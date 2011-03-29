#ifndef GEOM_H
#define GEOM_H

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

void goal_draw(void);
void jump_draw(int);
void swch_draw(int, int);
void flag_draw(void);
void mark_draw(void);

/*---------------------------------------------------------------------------*/

void back_init(const char *s);
void back_free(void);
void back_draw(float);

/*---------------------------------------------------------------------------*/

void shad_init(void);
void shad_free(void);
void shad_draw_set(void);
void shad_draw_clr(void);

/*---------------------------------------------------------------------------*/

void fade_draw(float);

#endif
