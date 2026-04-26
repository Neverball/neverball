#ifndef HUD_H
#define HUD_H

#include "SDL_events.h"

/*---------------------------------------------------------------------------*/

void hud_init(void);
void hud_free(void);

void hud_paint(void);
void hud_timer(float);
void hud_update(int);
void hud_lvlname(const char *);

void hud_show(float delay);
void hud_hide(void);

int hud_touch(const SDL_TouchFingerEvent *);

void hud_cam_pulse(int);
void hud_cam_timer(float);
void hud_cam_paint(void);

void hud_speed_pulse(int);
void hud_speed_timer(float);
void hud_speed_paint(void);

void hud_touch_timer(float);
void hud_touch_paint(void);

/*---------------------------------------------------------------------------*/

#endif
