#ifndef GEOM_H
#define GEOM_H

#include "gl.h"
#include "solid.h"

/*---------------------------------------------------------------------------*/

GLuint ball_init(int);
void   ball_draw(GLuint, double, const double[3], const double[3][3]);

/*---------------------------------------------------------------------------*/

GLuint coin_init(int);
void   coin_draw(GLuint, const struct s_coin *, int);
int    coin_test(const struct s_ball *, struct s_coin *, int);

/*---------------------------------------------------------------------------*/

void back_init(const char *);
void back_draw(void);
void back_free(void);

/*---------------------------------------------------------------------------*/

#endif
