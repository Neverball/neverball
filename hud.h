#ifndef HUD_H
#define HUD_H

/*---------------------------------------------------------------------------*/

#define STR_BALLS "Balls"
#define STR_COINS "Coins"

/*---------------------------------------------------------------------------*/

void hud_init(void);
void hud_free(void);
void hud_draw(void);
void hud_step(double);

void hud_ball_pulse(double);
void hud_time_pulse(double);
void hud_coin_pulse(double);

/*---------------------------------------------------------------------------*/

#endif
