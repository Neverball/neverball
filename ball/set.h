#ifndef SET_H
#define SET_H

#include "base_config.h"
#include "level.h"

/*---------------------------------------------------------------------------*/

#define SET_FILE "sets.txt"

#define MAXSET 16
#define MAXLVL 25

struct set
{
    char file[PATHMAX];

    char *id;                  /* Internal set identifier    */
    char *name;                /* Set name                   */
    char *desc;                /* Set description            */
    char *shot;                /* Set screen-shot            */

    char user_scores[PATHMAX]; /* User high-score file       */

    struct score time_score;   /* Challenge score            */
    struct score coin_score;   /* Challenge score            */

    /* Level stats */

    unsigned int count;        /* Number of levels           */
};

/*---------------------------------------------------------------------------*/

int  set_init(void);
void set_free(void);

int  set_exists(int);

const struct set *get_set(int);

int set_level_exists(const struct set *, int);

void                set_goto(int i);
const struct set *  curr_set(void);
const struct level *get_level(int);

/*---------------------------------------------------------------------------*/

void set_finish_level(struct level_game *, const char *);
void score_change_name(struct level_game *, const char *);

/*---------------------------------------------------------------------------*/

void level_snap(int);
void set_cheat(void);

/*---------------------------------------------------------------------------*/

#endif
