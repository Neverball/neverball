#ifndef STATE_H
#define STATE_H

/*---------------------------------------------------------------------------*/

struct state
{
    void (*enter)(void);
    void (*leave)(void);
    void (*paint)(void);
    int  (*timer)(double);
    int  (*point)(int, int);
    int  (*click)(int, int);
    int  (*keybd)(int);
};

void   goto_state(struct state *);
double time_state(void);

void st_paint(void);
int  st_timer(double);
int  st_point(int, int);
int  st_click(int, int);
int  st_keybd(int);

/*---------------------------------------------------------------------------*/

extern struct state st_null;
extern struct state st_title;
extern struct state st_start;
extern struct state st_help;
extern struct state st_conf;
extern struct state st_level;
extern struct state st_ready;
extern struct state st_set;
extern struct state st_play;
extern struct state st_goal;
extern struct state st_fall;
extern struct state st_time;
extern struct state st_over;
extern struct state st_pause;

/*---------------------------------------------------------------------------*/

#endif
