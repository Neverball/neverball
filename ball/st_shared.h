#ifndef ST_SHARED_H
#define ST_SHARED_H

#include "state.h"
#include "util.h"

void shared_leave(struct state *, struct state *next, int id);
void shared_paint(int id, float st);
void shared_timer(int id, float dt);
int  shared_point_basic(int id, int x, int y);
void shared_point(int id, int x, int y, int dx, int dy);
int  shared_stick_basic(int id, int a, float v, int bump);
void shared_stick(int id, int a, float v, int bump);
void shared_angle(int id, float x, float z);
int  shared_click_basic(int b, int d);
int  shared_click(int b, int d);

#endif
