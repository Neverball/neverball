#ifndef PUTT_H
#define PUTT_H

/*---------------------------------------------------------------------------*/

#define AUD_BIRDIE  "snd/birdie.ogg"
#define AUD_BOGEY   "snd/bogey.ogg"
#define AUD_BUMP    "snd/bink.ogg"
#define AUD_DOUBLE  "snd/double.ogg"
#define AUD_EAGLE   "snd/eagle.ogg"
#define AUD_JUMP    "snd/jump.ogg"
#define AUD_MENU    "snd/menu.ogg"
#define AUD_ONE     "snd/one.ogg"
#define AUD_PAR     "snd/par.ogg"
#define AUD_PENALTY "snd/penalty.ogg"
#define AUD_PLAYER1 "snd/player1.ogg"
#define AUD_PLAYER2 "snd/player2.ogg"
#define AUD_PLAYER3 "snd/player3.ogg"
#define AUD_PLAYER4 "snd/player4.ogg"
#define AUD_SWITCH  "snd/switch.ogg"
#define AUD_SUCCESS "snd/success.ogg"

/*---------------------------------------------------------------------------*/

#define MAX_DT  (1.0f / 60.0f)         /* Maximum physics update cycle       */
#define MAX_DN  16                     /* Maximum subdivisions of dt         */
#define FOV     50.00f                 /* Field of view                      */
#define RESPONSE 0.05f                 /* Input smoothing time               */

#define PUTT_NONE 0
#define PUTT_STOP 1
#define PUTT_GOAL 2
#define PUTT_FALL 3

/*---------------------------------------------------------------------------*/

int   putt_init(const char *);
void  putt_free(void);
void  putt_free_objects(void);
void  putt_load_objects(void);

void  putt_draw(int, float);
void  putt_putt(void);
int   putt_step(const float[3], float);

void  putt_update_view(float);

void  putt_set_rot(int);
void  putt_clr_mag(void);
void  putt_set_mag(int);
void  putt_set_fly(float);

void  putt_ball(int);
void  putt_set_pos(float[3], float[3][3]);
void  putt_get_pos(float[3], float[3][3]);

/*---------------------------------------------------------------------------*/

#endif
