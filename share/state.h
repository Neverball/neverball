#ifndef STATE_H
#define STATE_H

#include <stdlib.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*/

struct state
{
    int  (*enter)(void);
    void (*leave)(int dt);
    void (*paint)(int id, float st);
    void (*timer)(int id, float dt);
    void (*point)(int id, int x, int y, int dx, int dy);
    void (*stick)(int id, int a, int v);
    int  (*click)(int b,  int d);
    int  (*keybd)(int c,  int d);
    int  (*buttn)(int b,  int d);

    int pointer;
    int gui_id;
};

float time_state(void);
void  init_state(struct state *);
int   goto_state(struct state *);

void st_paint(void);
void st_timer(float);
void st_point(int, int, int, int);
void st_stick(int, int);
int  st_click(int, int);
int  st_keybd(int, int);
int  st_buttn(int, int);

/*---------------------------------------------------------------------------*/

#endif
