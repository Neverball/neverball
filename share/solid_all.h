#ifndef SOLID_ALL_H
#define SOLID_ALL_H

#include "solid.h"

void sol_body_p(float p[3], const s_file *fp, int pi, float t);
void sol_body_v(float v[3], const s_file *fp, int pi, float t, float dt);
void sol_body_e(float e[3],
                const s_file *fp,
                const s_body *bp, float dt);
void sol_body_w(float w[3], const s_file *fp, const s_body *bp);

void sol_rotate(float e[3][3], const float w[3], float dt);

void sol_pendulum(s_ball *up,
                  const float a[3],
                  const float g[3], float dt);

void sol_swch_step(s_file *fp, float dt);
void sol_body_step(s_file *fp, float dt);
void sol_ball_step(s_file *fp, float dt);

int            sol_item_test(s_file *fp, float *p, float item_r);
s_goal *sol_goal_test(s_file *fp, float *p, int ui);
int            sol_jump_test(s_file *fp, float *p, int ui);
int            sol_swch_test(s_file *fp, int ui);

#endif
