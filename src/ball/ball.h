#ifndef BALL_H
#define BALL_H

#include <GL/gl.h>

GLuint ball_init(int);
void   ball_draw(GLuint, double, const double[3], const double[3][3], int);

#endif
