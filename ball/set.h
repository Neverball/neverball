#ifndef SET_H
#define SET_H

#include "base_config.h"
#include "level.h"
#include "package.h"

#define SET_FILE "sets.txt"
#define SET_MISC "set-misc.txt"

#define MAXLVL 25

/*---------------------------------------------------------------------------*/

int  set_init(void);
void set_quit(void);

/*---------------------------------------------------------------------------*/

int  set_exists(int);
void set_goto(int);
int  set_find(const char *);

int  curr_set(void);

const char         *set_id(int);
const char         *set_name(int);
const char         *set_file(int);
const char         *set_desc(int);
const char         *set_shot(int);
const struct score *set_score(int, int);

int  set_score_update (int, int, int *, int *);
void set_rename_player(int, int, const char *);

void set_store_hs(void);

/*---------------------------------------------------------------------------*/

struct level *get_level(int);

void level_snap(int, const char *);
void set_cheat(void);

/*---------------------------------------------------------------------------*/

#endif
