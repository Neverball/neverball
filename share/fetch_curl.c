/*
 * Copyright (C) 2021-2024 Jānis Rūcis
 *
 * NEVERBALL is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

#include "fetch.h"
#include "common.h"
#include "version.h"
#include "list.h"
#include "log.h"
#include "fs.h"

#include <curl/curl.h>

#include <SDL_mutex.h>
#include <SDL_thread.h>

/*
 * The thing that lets us do async transfers in a single thread.
 */
static CURLM *multi_handle = NULL;

/*
 * This serves as a unique reference to an ongoing transfer. We could
 * have returned the fetch_info pointer directly, but that might lead
 * to dangling pointers. Integers are safer.
 */
static unsigned int last_fetch_id = 0;

/*
 * Stuff we need to attach to a transfer.
 */
struct fetch_info
{
    struct fetch_callback callback;

    CURL *handle;
    char *dest_filename;
    fs_file dest_file;
    unsigned int fetch_id;
};

/*
 * List of transfers.
 *
 * This is a linked list, so you can start as many as you want, but we limit
 * the curl multi handle total connections, because unlimited connections will
 * make all of the connections slow, while limited connections means all of the
 * connections will make progress.
 */
static List fetch_list = NULL;

/*---------------------------------------------------------------------------*/

/*
 * Here's a bit of odd decision making:
 *
 * I was very impressed with the download speed gains of libcurl running
 * in its own separate thread. It is massively faster than the "async"
 * piece-wise approach of doing curl_multi_performs from the main loop.
 * This is due to the main loop being largely occupied by rendering work.
 *
 * Downloads are better with callbacks. I want to know download progress
 * and when downloads are done, and show this to the user.
 *
 * But when you're attempting to communicate information across threads,
 * you have to be very careful about timing. Another thread may be reading
 * the value that you are currently overwriting. The safest approach is using
 * a lock (mutex) in both threads, to make sure that any operations on shared
 * data are performed by one thread at a time.
 *
 * It's very safe, but annoying. You make the locks and use them everywhere,
 * and when something doesn't work, you use them in more places. You constantly
 * have to think about how threads interact. Are they waiting? Will they be
 * accessing this data at the same time?
 *
 * So, here's the dispatch approach:
 *
 * Callbacks are not executed immediately. Instead, they are wrapped up into
 * an "event" structure that is "dispatched" onto the event queue of the main
 * thread. As part of its own event loop, the main thread passes the wrapped
 * callback back to here via fetch_handle_event, but this time we are on the
 * main thread, so we can safely execute the callback. No locks necessary.
 *
 * (Except the builtin lock of the SDL event queue, but hey.)
 */
struct fetch_event
{
    void (*callback)(void *, void *);
    void *callback_data;
    void *extra_data;
};

/*
 * Dispatch a wrapped callback to the thread that calls fetch_handle_event.
 */
static void (*fetch_dispatch_event)(void *) = NULL;

/*
 * Create extra_data for a progress callback.
 */
static struct fetch_progress *create_extra_progress(double total, double now)
{
    struct fetch_progress *pr = calloc(sizeof (*pr), 1);

    if (pr)
    {
        pr->total = total;
        pr->now = now;
    }

    return pr;
}

/*
 * Create extra_data for a done callback.
 */
static struct fetch_done *create_extra_done(int finished)
{
    struct fetch_done *dn = calloc(sizeof (*dn), 1);

    if (dn)
        dn->finished = !!finished;

    return dn;
}

/*
 * Free previously created extra_data.
 */
static void free_extra_data(void *extra_data)
{
    if (extra_data)
    {
        free(extra_data);
        extra_data = NULL;
    }
}

/*
 * Allocate a callback wrapper structure.
 */
static struct fetch_event *create_fetch_event(void)
{
    struct fetch_event *fe = calloc(sizeof (*fe), 1);

    return fe;
}

static void free_fetch_event(struct fetch_event *fe)
{
    if (fe)
    {
        free_extra_data(fe->extra_data);
        fe->extra_data = NULL;

        free(fe);
        fe = NULL;
    }
}

/*
 * Invoke a wrapped callback. This should happen on the main thread.
 */
void fetch_handle_event(void *data)
{
    struct fetch_event *fe = data;

    if (fe->callback)
        fe->callback(fe->callback_data, fe->extra_data);

    free_fetch_event(fe);
}

/*---------------------------------------------------------------------------*/

/*
 * Count transfers in the linked list.
 *
 * Seems less error-prone than keeping a count variable in sync.
 */
static int count_active_transfers(void)
{
    int n = 0;
    List l;

    for (l = fetch_list; l; l = l->next)
        n++;

    return n;
}

/*
 * Allocate a new fetch_info struct.
 */
static struct fetch_info *create_fetch_info(void)
{
    struct fetch_info *fi = calloc(sizeof (*fi), 1);

    if (fi)
        fi->fetch_id = ++last_fetch_id;

    return fi;
}

/*
 * Allocate a new fetch_info struct and add it to the transfer list.
 */
static struct fetch_info *create_and_link_fetch_info(void)
{
    struct fetch_info *fi = create_fetch_info();

    if (fi)
        fetch_list = list_cons(fi, fetch_list);

    return fi;
}

/*
 * Clean up a fetch_info struct and associated resources.
 */
static void free_fetch_info(struct fetch_info *fi)
{
    if (fi)
    {
        if (fi->handle)
            curl_easy_cleanup(fi->handle);

        if (fi->dest_filename)
        {
            free(fi->dest_filename);
            fi->dest_filename = NULL;
        }

        if (fi->dest_file)
            fs_close(fi->dest_file);

        free(fi);
        fi = NULL;
    }
}

/*
 * Remove a fetch_info from the transfer list and then free it.
 */
static void unlink_and_free_fetch_info(struct fetch_info *fi)
{
    if (fi)
    {
        /* First, remove from the linked list. */

        List l, p;

        for (p = NULL, l = fetch_list; l; p = l, l = l->next)
            if (l->data == fi)
            {
                if (p)
                    p->next = list_rest(l);
                else
                    fetch_list = list_rest(l);

                break;
            }

        /* Then, do clean up. */

        free_fetch_info(fi);
        fi = NULL;
    }
}

/*
 * Clean up all fetch_info structs and the linked list.
 *
 * This should only be called AFTER the fetch thread has exited.
 */
static void free_all_fetch_infos(void)
{
    List l;

    for (l = fetch_list; l; l = list_rest(l))
    {
        free_fetch_info(l->data);
        l->data = NULL;
    }

    fetch_list = NULL;
}

/*---------------------------------------------------------------------------*/

/*
 * Curl write function.
 */
static size_t fetch_write_func(void *buffer, size_t size, size_t nmemb, void *user_data)
{
    struct fetch_info *fi = user_data;

    if (fi)
    {
        if (!fi->dest_file)
        {
            /* Open file on first write. TODO: write to a temporary file. */

            if (fi->dest_filename && *fi->dest_filename)
                fi->dest_file = fs_open_write(fi->dest_filename);
        }

        if (fi->dest_file)
            return fs_write(buffer, size * nmemb, fi->dest_file);
    }

    return 0;
}

/*
 * Curl progress function.
 */
static int fetch_progress_func(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    struct fetch_info *fi = clientp;

    if (fi && fi->callback.progress)
    {
        struct fetch_event *fe = create_fetch_event();

        if (fe)
        {
            fe->callback = fi->callback.progress;
            fe->callback_data = fi->callback.data;
            fe->extra_data = create_extra_progress(dltotal, dlnow);

            fetch_dispatch_event(fe);
        }
    }

    return 0;
}

/*
 * Progress all transfers.
 */
static void fetch_step(void)
{
    if (multi_handle)
    {
        int running_handles = 0;

        curl_multi_perform(multi_handle, &running_handles);

        if (running_handles < count_active_transfers())
        {
            CURLMsg *message;
            int remaining_messages = 0;

            while ((message = curl_multi_info_read(multi_handle, &remaining_messages)))
                if (message->msg == CURLMSG_DONE)
                {
                    struct fetch_info *fi;

                    int finished;

                    CURL *handle = message->easy_handle;
                    CURLcode code = message->data.result;

                    curl_easy_getinfo(handle, CURLINFO_PRIVATE, &fi);

                    if (code != CURLE_OK)
                    {
                        if (code == CURLE_ABORTED_BY_CALLBACK)
                            log_printf("Transfer %u aborted\n", fi->fetch_id);
                        else
                            log_printf("Transfer %u error: %s\n", fi->fetch_id, curl_easy_strerror(code));

                        finished = 0;
                    }
                    else
                    {
                        finished = 1;
                    }

                    curl_multi_remove_handle(multi_handle, handle);

                    /* Close file before done callback. */

                    if (fi->dest_file)
                    {
                        fs_close(fi->dest_file);
                        fi->dest_file = NULL;
                    }

                    if (fi->callback.done)
                    {
                        struct fetch_event *fe = create_fetch_event();

                        if (fe)
                        {
                            fe->callback = fi->callback.done;
                            fe->callback_data = fi->callback.data;
                            fe->extra_data = create_extra_done(finished);

                            fetch_dispatch_event(fe);
                        }
                    }

                    log_printf("Stopping transfer %u\n", fi->fetch_id);

                    unlink_and_free_fetch_info(fi);
                }
        }
    }
}

/*---------------------------------------------------------------------------*/

/*
 * Thread stuff.
 */

static SDL_mutex *fetch_mutex;
static SDL_Thread *fetch_thread;

static SDL_atomic_t fetch_thread_running;

/*
 * Fetch thread entry point.
 */
static int fetch_thread_func(void *data)
{
    /* Loop infinitely unless poll fails or requested to quit. */

    log_printf("Starting fetch thread\n");

    while (SDL_AtomicGet(&fetch_thread_running))
    {
        CURLMcode code;

        code = curl_multi_poll(multi_handle, NULL, 0u, 1000 / 30, NULL);

        if (code == CURLM_OK)
        {
            SDL_LockMutex(fetch_mutex);
            fetch_step();
            SDL_UnlockMutex(fetch_mutex);
        }
        else
        {
            log_printf("libcurl poll failure: %s\n", curl_multi_strerror(code));
            SDL_AtomicSet(&fetch_thread_running, 0);
        }
    };

    log_printf("Stopping fetch thread\n");

    return 0;
}

/*
 * Start the thread.
 */
static void fetch_thread_init(void)
{
    SDL_AtomicSet(&fetch_thread_running, 1);
    fetch_mutex = SDL_CreateMutex();
    fetch_thread = SDL_CreateThread(fetch_thread_func, "fetch", NULL);
}

/*
 * Wait for thread to exit and do cleanup.
 */
static void fetch_thread_quit(void)
{
    SDL_AtomicSet(&fetch_thread_running, 0);

    SDL_WaitThread(fetch_thread, NULL);
    fetch_thread = NULL;

    SDL_DestroyMutex(fetch_mutex);
    fetch_mutex = NULL;
}

/*
 * Gain thread access to shared data.
 */
static int fetch_lock_mutex(void)
{
    if (multi_handle)
    {
        /* Wake from curl_multi_poll first. */
        curl_multi_wakeup(multi_handle);
    }

    /* Then, attempt to acquire mutex. */
    return SDL_LockMutex(fetch_mutex);
}

/*
 * Give up thread access to shared.
 */
static int fetch_unlock_mutex(void)
{
    return SDL_UnlockMutex(fetch_mutex);
}

/*---------------------------------------------------------------------------*/

/*
 * Initialize the CURL.
 */
void fetch_init(void (*dispatch_event)(void *))
{
    curl_version_info_data *info;

    curl_global_init(CURL_GLOBAL_ALL);

    info = curl_version_info(CURLVERSION_NOW);

    log_printf("libcurl %s\n", info->version);

    multi_handle = curl_multi_init();

    if (!multi_handle)
    {
        log_printf("Failure to create a CURL multi handle\n");
        return;
    }

    /* Process FETCH_MAX connections in parallel, while the rest wait in a queue. */

    curl_multi_setopt(multi_handle, CURLMOPT_MAX_TOTAL_CONNECTIONS, FETCH_MAX);

    fetch_dispatch_event = dispatch_event;

    fetch_thread_init();
}

/*
 * Shut down the CURL.
 */
void fetch_quit(void)
{
    fetch_thread_quit();

    free_all_fetch_infos();

    if (multi_handle)
    {
        curl_multi_cleanup(multi_handle);
        multi_handle = NULL;
    }

    curl_global_cleanup();
}

/*
 * Download from URL into FILENAME.
 */
unsigned int fetch_file(const char *url,
                        const char *filename,
                        struct fetch_callback callback)
{
    unsigned int fetch_id = 0;
    CURL *handle;

    fetch_lock_mutex();

    handle = curl_easy_init();

    if (handle)
    {
        struct fetch_info *fi = create_and_link_fetch_info();

        if (fi)
        {
            log_printf("Starting transfer %u\n", fi->fetch_id);

            log_printf("Downloading from %s\n", url);
            log_printf("Saving to %s\n", filename);

            fi->callback = callback;
            fi->handle = handle;
            fi->dest_filename = strdup(filename);

            curl_easy_setopt(handle, CURLOPT_PRIVATE, fi);
            curl_easy_setopt(handle, CURLOPT_URL, url);
            curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, fetch_write_func);
            curl_easy_setopt(handle, CURLOPT_WRITEDATA, fi);

            curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, fetch_progress_func);
            curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, fi);
            curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0);

            curl_easy_setopt(handle, CURLOPT_BUFFERSIZE, 102400L);
            curl_easy_setopt(handle, CURLOPT_USERAGENT, "neverball/" VERSION);
            curl_easy_setopt(handle, CURLOPT_ACCEPT_ENCODING, "");
            curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);

            #if defined(_WIN32) && defined(CURLSSLOPT_NATIVE_CA)
            curl_easy_setopt(handle, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
            #endif

            /* curl_easy_setopt(handle, CURLOPT_VERBOSE, 1); */

            curl_multi_add_handle(multi_handle, handle);

            fetch_id = fi->fetch_id;
        }
        else curl_easy_cleanup(handle);
    }

    fetch_unlock_mutex();

    return fetch_id;
}
