#ifndef MAIN_H
#define MAIN_H

/*---------------------------------------------------------------------------*/

#define MIN(a, b) ((a) < (b) ? (a) : (b))

struct state
{
    void (*enter)(void);
    void (*leave)(void);
    void (*paint)(void);
    int  (*point)(int, int);
    int  (*click)(int);
    int  (*keybd)(int);
};

void   goto_state(struct state *);
double time_state(void);

/*---------------------------------------------------------------------------*/

#endif
