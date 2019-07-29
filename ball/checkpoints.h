#ifndef CHECKPOINTS_H
#define CHECKPOINTS_H

extern int checkpoints_busy;
extern int last_active;

extern struct s_vary last_vary;
extern struct game_view last_view;

extern float last_position_x;
extern float last_position_y;
extern float last_position_z;
extern float last_orientation_x;
extern float last_orientation_y;
extern float last_orientation_z;
extern float last_r;

extern float last_time;
extern int last_coins;
extern int last_goal;

extern int last_timer_down;

/*---------------------------------------------------------------------------*/

void checkpoints_save_spawnpoint(struct s_vary, struct game_view);

void checkpoints_respawn(void);
void checkpoints_respawn_done(void);

void checkpoints_stop(void);

void set_last_transform(float[], float);
void set_last_data(float, int);
void set_last_timer_down(int);

#endif
