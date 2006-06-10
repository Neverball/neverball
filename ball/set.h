#ifndef SET_H
#define SET_H

#include "base_config.h"
#include "level.h"

/*---------------------------------------------------------------------------*/

#define SET_FILE "sets.txt"
#define MAXSET 16
#define MAXLVL 25

/* A pack of levels */
struct set
{
    /* set global info */
    int number;               /* number of the set */

    char file[MAXSTR];        /* set file description */
    char user_scores[MAXSTR]; /* user highscore file */
    char setname[MAXSTR];     /* internal set name */

    char name[MAXSTR];        /* set name */
    char desc[MAXSTR];        /* set description */
    char shot[MAXSTR];        /* screenshot image file*/

    struct score time_score;  /* challenge score */
    struct score coin_score;  /* challenge score */

    /* levels info */

    int count;                /* number of levels */
    int locked;               /* number of locked levels */
    int completed;            /* number of completed levels */
};

/*---------------------------------------------------------------------------*/

void set_init();

int  set_exists(int);
const struct set *get_set(int);

/*---------------------------------------------------------------------------*/

int  set_unlocked(const struct set *);
int  set_completed(const struct set *);

int  set_level_exists(const struct set *, int);

/*---------------------------------------------------------------------------*/

void set_goto(int i);
const struct set *curr_set(void);

const struct level *get_level(int);

/*---------------------------------------------------------------------------*/

void set_finish_level(struct level_game *, const char *);
void score_change_name(struct level_game *, const char *);

/*---------------------------------------------------------------------------*/

void level_snap(int);

void set_cheat(void);

/*---------------------------------------------------------------------------*/

#endif
