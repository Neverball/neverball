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
	
    char init_levels[MAXSTR]; /* levels list file */
    char init_scores[MAXSTR]; /* levels intals score file */
    char user_scores[MAXSTR]; /* lever user highscore file */

    char shot[MAXSTR];        /* screenshot image file*/
    char name[MAXSTR];        /* set name */
    char desc[MAXSTR];        /* set description */

    struct score time_score;  /* challenge score */
    struct score coin_score;  /* challenge score */
    
    /* levels info */
    
    int count;	              /* number of levels */
    int limit;	              /* last opened level */
};

/*---------------------------------------------------------------------------*/

void set_init();

int  set_exists(int);
const struct set *get_set(int);

/*---------------------------------------------------------------------------*/

int  set_extra_bonus_opened(const struct set *);
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
