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

    char id[MAXSTR];           /* Internal set identifier    */
    char name[MAXSTR];         /* Set name                   */
    char desc[MAXSTR];         /* Set description            */
    char shot[MAXSTR];         /* Set screen-shot            */

    char user_scores[PATHMAX]; /* User high-score file       */

    struct score time_score;   /* Challenge score            */
    struct score coin_score;   /* Challenge score            */

    /* Level stats */

    int count;                 /* Number of levels           */
    int locked;                /* Number of locked levels    */
    int completed;             /* Number of completed levels */
};

/*---------------------------------------------------------------------------*/

int set_init();
int set_exists(int);

const struct set *get_set(int);

int set_unlocked(const struct set *);
int set_completed(const struct set *);
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
