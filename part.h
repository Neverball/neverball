#ifndef PART_H
#define PART_H

/*---------------------------------------------------------------------------*/

#define IMG_PART "png/part.png"

#define PART_MAX_COIN 256
#define PART_MAX_GOAL  64
#define PART_SIZE       0.1

/*---------------------------------------------------------------------------*/

void part_init(double);
void part_free(void);

void part_burst(const double *, const double *);
void part_step(const double *, double);

void part_draw_coin(double, double);
void part_draw_goal(double, double);

/*---------------------------------------------------------------------------*/

#endif
