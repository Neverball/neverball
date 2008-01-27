#ifndef SET_H
#define SET_H

#include "base_config.h"
#include "level.h"

#define SET_FILE "sets.txt"

#define MAXSET 16
#define MAXLVL 25

/*---------------------------------------------------------------------------*/

int  set_init(void);
void set_free(void);

/*---------------------------------------------------------------------------*/

int  set_exists(int);
void set_goto(int);

int  curr_set(void);

const char         *set_name(int);
const char         *set_desc(int);
const char         *set_shot(int);
const struct score *set_time_score(int);
const struct score *set_coin_score(int);


/*---------------------------------------------------------------------------*/

int set_level_exists(int, int);
const struct level *get_level(int);

void set_finish_level(struct level_game *, const char *);
void score_change_name(struct level_game *, const char *);

void level_snap(int);
void set_cheat(void);

/*---------------------------------------------------------------------------*/

#endif
