#ifndef STATE_H
#define STATE_H

#include <stdio.h>

/*---------------------------------------------------------------------------*/

struct state
{
    void (*enter)(void);
    void (*leave)(void);
    void (*paint)(void);
    int  (*timer)(float);
    int  (*point)(int, int, int, int);
    int  (*click)(int, int);
    int  (*keybd)(int);
};

void  init_state(void);
float time_state(void);
int   goto_state(struct state *);

void st_paint(void);
int  st_timer(float);
int  st_point(int, int, int, int);
int  st_click(int, int);
int  st_keybd(int);

/*---------------------------------------------------------------------------*/

extern struct state st_null;
extern struct state st_title;
extern struct state st_conf;
extern struct state st_party;
extern struct state st_next;
extern struct state st_flyby;
extern struct state st_stroke;
extern struct state st_roll;
extern struct state st_goal;
extern struct state st_stop;
extern struct state st_fall;
extern struct state st_over;

/*---------------------------------------------------------------------------*/

#endif
