#ifndef PART_H
#define PART_H

/*---------------------------------------------------------------------------*/

#define IMG_PART "png/part.png"
#define IMG_SQUIGGLE "png/squiggle.png"

#define PART_MAX_COIN 256
#define PART_MAX_GOAL  64
#define PART_MAX_JUMP  64

#define PART_SIZE       0.1f

/*---------------------------------------------------------------------------*/

void part_reset(float);
void part_init(float);
void part_free(void);

void part_burst(const float *, const float *);
void part_step(const float *, float);

void part_draw_coin(const float *, float);
void part_draw_goal(const float *, float, float, float);
void part_draw_jump(const float *, float, float, float);

/*---------------------------------------------------------------------------*/

#endif
