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

#include <emscripten/fetch.h>

#include "fetch.h"
#include "common.h"
#include "list.h"
#include "fs.h"

static unsigned int last_fetch_id = 0;

struct fetch_info
{
    struct fetch_callback callback;

    emscripten_fetch_t *handle;
    char *dest_filename;
    unsigned int fetch_id;
};

static List fetch_list = NULL;

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
            emscripten_fetch_close(fi->handle);

        if (fi->dest_filename)
        {
            free(fi->dest_filename);
            fi->dest_filename = NULL;
        }

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

void fetch_init(void (*dispatch_event)(void *))
{
    /* Just compile with -s FETCH=1 */
}

void fetch_handle_event(void *data)
{
}

void fetch_quit(void)
{
    List l;

    for (l = fetch_list; l; l = list_rest(l))
        free_fetch_info(l->data);

    fetch_list = NULL;
}

static void fetch_success_func(emscripten_fetch_t *handle)
{
    struct fetch_info *fi = handle->userData;

    if (fi)
    {
        int success = 0;

        if (fi->dest_filename && *fi->dest_filename)
        {
            fs_file fp = fs_open_write(fi->dest_filename);

            if (fp)
            {
                if (fs_write(handle->data, handle->numBytes, fp) == handle->numBytes)
                    success = 1;

                fs_close(fp);
                fp = NULL;
            }
        }

        if (fi->callback.done)
        {
            struct fetch_done extra_data = { 0 };

            extra_data.success = !!success;

            fi->callback.done(fi->callback.data, &extra_data);
        }

        unlink_and_free_fetch_info(fi);
    }
}

static void fetch_error_func(emscripten_fetch_t *handle)
{
    struct fetch_info *fi = handle->userData;

    if (fi)
    {
        if (fi->callback.done)
        {
            struct fetch_done extra_data = { 0 };

            extra_data.success = 0;

            fi->callback.done(fi->callback.data, &extra_data);
        }

        unlink_and_free_fetch_info(fi);
    }
}

static void fetch_progress_func(emscripten_fetch_t *handle)
{
    struct fetch_info *fi = handle->userData;

    if (fi)
    {
        if (fi->callback.progress)
        {
            struct fetch_progress extra_data = { 0 };

            extra_data.now = (double) handle->dataOffset;
            extra_data.total = (double) handle->totalBytes;

            fi->callback.progress(fi->callback.data, &extra_data);
        }
    }
}

unsigned int fetch_file(const char *url, const char *dst, struct fetch_callback callback)
{
    unsigned int fetch_id = 0;
    struct fetch_info *fi = create_and_link_fetch_info();

    if (fi)
    {
        emscripten_fetch_attr_t attr;

        emscripten_fetch_attr_init(&attr);

        strcpy(attr.requestMethod, "GET");

        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
        attr.onsuccess = fetch_success_func;
        attr.onerror = fetch_error_func;
        attr.onprogress = fetch_progress_func;
        attr.userData = fi;

        fi->callback = callback;
        fi->dest_filename = strdup(dst);
        fi->handle = emscripten_fetch(&attr, url);

        if (fi->handle)
            fetch_id = fi->fetch_id;
        else
            unlink_and_free_fetch_info(fi);
    }

    return fetch_id;
}