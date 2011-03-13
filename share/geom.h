#ifndef GEOM_H
#define GEOM_H

/*---------------------------------------------------------------------------*/

#define IMG_SHAD "png/shadow.png"

#define JUMP_HEIGHT   2.00f
#define SWCH_HEIGHT   2.00f
#define GOAL_HEIGHT   3.00f
#define GOAL_SPARKS  64

/*---------------------------------------------------------------------------*/

void mark_init(void);
void mark_free(void);
void mark_draw(void);

/*---------------------------------------------------------------------------*/

void goal_init(void);
void goal_free(void);
void goal_draw(void);

/*---------------------------------------------------------------------------*/

void jump_init(void);
void jump_free(void);
void jump_draw(int);

/*---------------------------------------------------------------------------*/

void swch_init(void);
void swch_free(void);
void swch_draw(int, int);

/*---------------------------------------------------------------------------*/

void flag_init(void);
void flag_free(void);
void flag_draw(void);

/*---------------------------------------------------------------------------*/

void shad_init(void);
void shad_free(void);
void shad_draw_set(void);
void shad_draw_clr(void);

/*---------------------------------------------------------------------------*/

void fade_draw(float);

#endif
