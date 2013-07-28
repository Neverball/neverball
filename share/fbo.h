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

#ifndef FBO_H
#define FBO_H

#include "glext.h"

/*----------------------------------------------------------------------------*/

struct fbo
{
    GLuint framebuffer;
    GLuint color_texture;
    GLuint depth_texture;
};

typedef struct fbo fbo;

GLboolean fbo_create(fbo *, int, int);
void      fbo_delete(fbo *);

/*----------------------------------------------------------------------------*/

#endif
