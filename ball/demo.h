#ifndef DEMO_H
#define DEMO_H

/*---------------------------------------------------------------------------*/

int  demo_exists(void);

void demo_play_init(void);
void demo_play_step(float);

void demo_replay_init(void);
int  demo_replay_step(float *);

void demo_finish(void);

/*---------------------------------------------------------------------------*/

#endif
