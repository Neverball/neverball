#ifndef FETCH_H
#define FETCH_H 1

#define FETCH_MAX 5

/*
 * Progress callback extra_data.
 */
struct fetch_progress
{
    double total;
    double now;
};

/*
 * Done callback extra_data.
 */
struct fetch_done
{
    unsigned int finished:1;
};

/*
 * Callbacks.
 */
struct fetch_callback
{
    void (*progress)(void *data, void *progress_data);
    void (*done)(void *data, void *done_data);
    void *data;
};

void fetch_init(void (*dispatch_event)(void *));
void fetch_handle_event(void *);
void fetch_quit(void);

unsigned int fetch_url(const char *url,
                       const char *dst,
                       struct fetch_callback);

#endif