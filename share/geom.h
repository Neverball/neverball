#ifndef GEOM_H
#define GEOM_H

#include "solid_draw.h"

/*---------------------------------------------------------------------------*/

#define IMG_SHAD "png/shadow.png"

#define BACK_DIST   256.0f
#define FAR_DIST    512.0f
#define JUMP_HEIGHT   2.00f
#define SWCH_HEIGHT   2.00f
#define GOAL_HEIGHT   3.00f
#define GOAL_SPARKS  64

/*---------------------------------------------------------------------------*/

void geom_init(void);
void geom_free(void);

const struct d_mtrl *goal_draw(const struct d_mtrl *, float);
const struct d_mtrl *jump_draw(const struct d_mtrl *, float, int);
const struct d_mtrl *swch_draw(const struct d_mtrl *, int, int);
const struct d_mtrl *flag_draw(const struct d_mtrl *);
const struct d_mtrl *mark_draw(const struct d_mtrl *);
const struct d_mtrl *vect_draw(const struct d_mtrl *);
const struct d_mtrl *back_draw(const struct d_mtrl *, float);

/*---------------------------------------------------------------------------*/

void back_init(const char *s);
void back_free(void);
void back_draw_easy(void);

/*---------------------------------------------------------------------------*/

void shad_init(void);
void shad_free(void);
void shad_draw_set(void);
void shad_draw_clr(void);

/*---------------------------------------------------------------------------*/

void fade_draw(float);

#endif
