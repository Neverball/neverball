#ifndef GAME_H
#define GAME_H

/*---------------------------------------------------------------------------*/

#define IMG_SHADOW "png/shadow.png"

#define MAX_DT   0.01666666            /* Maximum physics update cycle       */
#define MAX_DN  16                     /* Maximum subdivisions of dt         */
#define FOV     50.00f                 /* Field of view                      */
#define RESPONSE 0.05f                 /* Input smoothing time               */

#define GAME_NONE 0
#define GAME_STOP 1
#define GAME_GOAL 2
#define GAME_FALL 3

/*---------------------------------------------------------------------------*/

void  game_init(const char *);
void  game_free(void);

void  game_draw(int);
void  game_putt(void);
int   game_step(const float[3], float);

void  game_update_view(float);

void  game_set_rot(int);
void  game_clr_mag(void);
void  game_set_mag(int);
void  game_set_fly(float);

void  game_ball(int);
void  game_set_pos(float[3], float[3][3]);
void  game_get_pos(float[3], float[3][3]);

/*---------------------------------------------------------------------------*/

#endif
