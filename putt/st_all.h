#ifndef ST_ALL_H
#define ST_ALL_H

#include "state.h"

extern struct state st_putt_title;
extern struct state st_putt_course;
extern struct state st_putt_party;
extern struct state st_putt_next;
extern struct state st_putt_poser;
extern struct state st_putt_flyby;
extern struct state st_putt_stroke;
extern struct state st_putt_roll;
extern struct state st_putt_goal;
extern struct state st_putt_stop;
extern struct state st_putt_fall;
extern struct state st_putt_score;
extern struct state st_putt_over;
extern struct state st_putt_pause;

int goto_putt_pause(struct state *);

#endif
