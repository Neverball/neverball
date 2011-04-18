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

void goal_draw(struct s_rend *, float);
void jump_draw(struct s_rend *, float, int);
void swch_draw(struct s_rend *, int, int);
void flag_draw(struct s_rend *);
void mark_draw(struct s_rend *);
void vect_draw(struct s_rend *);
void back_draw(struct s_rend *, float);

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
