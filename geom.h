#ifndef GEOM_H
#define GEOM_H

/*---------------------------------------------------------------------------*/

#define IMG_COIN "png/coin.png"
#define IMG_BALL "png/ball.png"
#define IMG_DONE "png/eyeball.png"

#define BALL_FUDGE    0.01
#define COIN_RADIUS   0.15
#define COIN_THICK    0.01
#define JUMP_HEIGHT   2.00
#define GOAL_HEIGHT   3.00
#define GOAL_SPARKS  64

/*---------------------------------------------------------------------------*/

void ball_init(int);
void ball_free(void);
void ball_draw(void);

/*---------------------------------------------------------------------------*/

void coin_color(double *, int);
void coin_init(int);
void coin_free(void);
void coin_draw(int);

/*---------------------------------------------------------------------------*/

void goal_init(int);
void goal_free(void);
void goal_draw(void);

/*---------------------------------------------------------------------------*/

void jump_init(int);
void jump_free(void);
void jump_draw(void);

/*---------------------------------------------------------------------------*/

#endif
