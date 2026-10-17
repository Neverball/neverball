#ifndef ST_LEVEL_H
#define ST_LEVEL_H

#include "state.h"

extern struct state st_level;
extern struct state st_poser;
extern struct state st_nodemo;
extern struct state st_abandon;

int goto_exit(void);
int goto_abandon(void);

#endif
