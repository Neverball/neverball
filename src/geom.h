#ifndef GEOM_H
#define GEOM_H

#include "gl.h"
#include "solid.h"

/*---------------------------------------------------------------------------*/

void ball_init(void);
void ball_free(void);
void ball_draw(double, const double[3], const double[3][3]);

/*---------------------------------------------------------------------------*/

void part_init(void);
void part_free(void);
void part_draw(GLfloat, GLfloat);
void part_update_grav(const double[3], double);

/*---------------------------------------------------------------------------*/

void goal_init(void);
void goal_free(void);
void goal_draw(GLfloat, GLfloat, const struct s_goal *, int);
int  goal_test(const struct s_ball *, struct s_goal *, int);

/*---------------------------------------------------------------------------*/

void coin_init(void);
void coin_free(void);
void coin_draw(const struct s_coin *, int);
int  coin_test(const struct s_ball *, struct s_coin *, int);

/*---------------------------------------------------------------------------*/

void back_init(const char *);
void back_free(void);
void back_draw(void);

/*---------------------------------------------------------------------------*/

#endif
