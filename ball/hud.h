#ifndef HUD_H
#define HUD_H

/*---------------------------------------------------------------------------*/

#define STR_VIEW0 "Chase"
#define STR_VIEW1 "Lazy"
#define STR_VIEW2 "Manual"

/*---------------------------------------------------------------------------*/

void hud_init(void);
void hud_free(void);

void hud_paint(void);
void hud_timer(float);

void hud_ball_pulse(float);
void hud_time_pulse(float);
void hud_coin_pulse(float);
void hud_goal_pulse(float);
void hud_view_pulse(int);

/*---------------------------------------------------------------------------*/

#endif
