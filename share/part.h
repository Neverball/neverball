#ifndef PART_H
#define PART_H

#include "solid_draw.h"

/*---------------------------------------------------------------------------*/

#define IMG_PART_STAR     "png/part.png"
#define IMG_PART_SQUIGGLE "png/squiggle.png"

#define PART_MAX_COIN 256
#define PART_MAX_GOAL  64
#define PART_MAX_JUMP  64

#define PART_SIZE       0.1f

/*---------------------------------------------------------------------------*/

void part_reset(void);
void part_init(void);
void part_free(void);

void part_burst(const float *, const float *);
void part_step(const float *, float);

const struct d_mtrl *part_draw_coin(const struct d_mtrl *);

void part_lerp_apply(float);

/*---------------------------------------------------------------------------*/

#endif
