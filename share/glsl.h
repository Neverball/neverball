/*
 * Copyright (C) 2013 Robert Kooima
 *
 * NEVERBALL is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

#ifndef GLSL_H
#define GLSL_H

#include "glext.h"

/*----------------------------------------------------------------------------*/

struct glsl
{
    GLuint vert_shader;
    GLuint frag_shader;
    GLuint program;
};

typedef struct glsl glsl;

GLboolean glsl_create(glsl *, int, const char *const *,
	                          int, const char *const *);
void      glsl_delete(glsl *);

void      glsl_uniform1f(glsl *, const char *, GLfloat);
void      glsl_uniform2f(glsl *, const char *, GLfloat, GLfloat);
void      glsl_uniform3f(glsl *, const char *, GLfloat, GLfloat, GLfloat);
void      glsl_uniform4f(glsl *, const char *, GLfloat, GLfloat, GLfloat, GLfloat);

/*----------------------------------------------------------------------------*/

#endif
