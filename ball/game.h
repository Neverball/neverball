#ifndef GAME_H
#define GAME_H

/*---------------------------------------------------------------------------*/

#define IMG_SHADOW "png/shadow.png"

#define MAX_DT   0.01666666            /* Maximum physics update cycle       */
#define MAX_DN  16                     /* Maximum subdivisions of dt         */
#define FOV     45.00f                 /* Field of view                      */
#define RESPONSE 0.05f                 /* Input smoothing time               */

#define GAME_NONE 0
#define GAME_TIME 1
#define GAME_GOAL 2
#define GAME_FALL 3

/*---------------------------------------------------------------------------*/

void  game_init(const char *, int);
void  game_free(void);

float curr_clock(void);
char *curr_intro(void);

void  game_draw(int, float);
int   game_step(const float[3], float, int);

void  game_set_pos(int, int);
void  game_set_x  (int);
void  game_set_z  (int);
void  game_set_rot(int);
void  game_set_fly(float);

int game_put(FILE *);
int game_get(FILE *);

/*---------------------------------------------------------------------------*/

#endif
