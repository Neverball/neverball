#ifndef STATE_H
#define STATE_H

#include <stdio.h>

/*---------------------------------------------------------------------------*/

struct state
{
    void (*enter)(void);
    void (*leave)(void);
    void (*paint)(void);
    int  (*timer)(double);
    int  (*point)(int, int, int, int);
    int  (*click)(int, int);
    int  (*keybd)(int);
    int  (*stick)(int, int);
    int  (*buttn)(int, int);
};

void   init_state(void);
double time_state(void);
int    goto_state(struct state *);

void st_paint(void);
int  st_timer(double);
int  st_point(int, int, int, int);
int  st_click(int, int);
int  st_keybd(int);
int  st_stick(int, int);
int  st_buttn(int, int);

/*---------------------------------------------------------------------------*/

extern FILE *record_fp;
extern FILE *replay_fp;

extern struct state st_null;
extern struct state st_title;
extern struct state st_start;
extern struct state st_help;
extern struct state st_conf;
extern struct state st_level;
extern struct state st_poser;
extern struct state st_ready;
extern struct state st_set;
extern struct state st_play;
extern struct state st_demo;
extern struct state st_goal;
extern struct state st_score;
extern struct state st_fall;
extern struct state st_time;
extern struct state st_omed;
extern struct state st_over;
extern struct state st_done;

/*---------------------------------------------------------------------------*/

#endif
