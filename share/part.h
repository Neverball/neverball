#ifndef PART_H
#define PART_H

/*---------------------------------------------------------------------------*/

#define IMG_PART "png/part.png"

#define PART_MAX_COIN 256
#define PART_MAX_GOAL  64
#define PART_SIZE       0.1f

/*---------------------------------------------------------------------------*/

void part_reset(float);
void part_init(float);
void part_free(void);

void part_burst(const float *, const float *);
void part_step(const float *, float);

void part_draw_coin(float, float);
void part_draw_goal(float, float, float, float);

/*---------------------------------------------------------------------------*/

#endif
