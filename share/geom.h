#ifndef GEOM_H
#define GEOM_H

/*---------------------------------------------------------------------------*/

#define IMG_SHAD "png/shadow.png"

#define BALL_FUDGE    0.001f
#define COIN_RADIUS   0.15f
#define COIN_THICK    0.01f
#define JUMP_HEIGHT   2.00f
#define SWCH_HEIGHT   2.00f
#define GOAL_HEIGHT   3.00f
#define GOAL_SPARKS  64

/*---------------------------------------------------------------------------*/

void ball_init(int);
void ball_free(void);
void ball_draw(void);

/*---------------------------------------------------------------------------*/

void mark_init(int);
void mark_free(void);
void mark_draw(void);

/*---------------------------------------------------------------------------*/

void coin_color(float *, int);
void coin_init(int);
void coin_free(void);

void coin_push(void);
void coin_draw(int, float);
void coin_pull(void);

/*---------------------------------------------------------------------------*/

void goal_init(int);
void goal_free(void);
void goal_draw(void);

/*---------------------------------------------------------------------------*/

void jump_init(int);
void jump_free(void);
void jump_draw(void);

/*---------------------------------------------------------------------------*/

void swch_init(int);
void swch_free(void);
void swch_draw(int);

/*---------------------------------------------------------------------------*/

void flag_init(int);
void flag_free(void);
void flag_draw(void);

/*---------------------------------------------------------------------------*/

void shad_init(void);
void shad_free(void);
void shad_draw_set(const float *, float);
void shad_draw_clr(void);

/*---------------------------------------------------------------------------*/

void fade_draw(float);

#endif
