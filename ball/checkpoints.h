#ifndef CHECKPOINTS_H
#define CHECKPOINTS_H

extern int last_active;

extern float last_position_x;
extern float last_position_y;
extern float last_position_z;
extern float last_r;

extern int last_time;
extern int last_coins;
extern int last_goal;

extern int last_timer_down;

/*---------------------------------------------------------------------------*/

void checkpoints_stop(void);

void set_active_checkpoint(int);

void set_last_transform(float, float, float, float);
void set_last_data(int, int, int);
void set_last_timer_down(int);

#endif
