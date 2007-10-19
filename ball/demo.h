#ifndef DEMO_H
#define DEMO_H

#include <time.h>

#include "level.h"

/*---------------------------------------------------------------------------*/

struct demo
{
    char   name[PATHMAX];      /* demo basename */
    char   filename[MAXSTR];  /* demo path */

    int    timer;           /* elapsed time */
    int    coins;           /* coin number */
    int    status;          /* how the replay end */
    int    mode;            /* game mode */
    time_t date;            /* date of creation */
    char   player[MAXNAM];  /* player name */
    char   shot[PATHMAX];   /* image filename */
    char   file[PATHMAX];   /* level filename */
    char   back[PATHMAX];   /* level bg filename */
    char   grad[PATHMAX];   /* level gradient filename */
    char   song[PATHMAX];   /* level song filename */
    int    time;            /* time limit (! training mode) */
    int    goal;            /* coin to open the goal (! training mode) */
    int    score;           /* sum of coins (challenge mode) */
    int    balls;           /* number of balls (challenge mode) */
    int    times;           /* total time (challenge mode) */
};

/*---------------------------------------------------------------------------*/

#define MAXDEMO 256

int                demo_scan(void);
const char        *demo_pick(void);
const struct demo *demo_get(int);

const char *date_to_str(time_t);

int  demo_exists(const char *);
void demo_unique(char *);

/*---------------------------------------------------------------------------*/

int  demo_play_init(const char *, const struct level *,
                    const struct level_game *);
void demo_play_step(float);
void demo_play_stat(const struct level_game *);
void demo_play_stop(void);

int  demo_saved (void);
void demo_rename(const char *);

/*---------------------------------------------------------------------------*/

int  demo_replay_init(const char *, struct level_game *);
int  demo_replay_step(float *);
void demo_replay_stop(int);
void demo_replay_dump_info(void);

const struct demo *curr_demo_replay(void);

/*---------------------------------------------------------------------------*/

#endif
