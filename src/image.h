/*   
 *   Copyright (C) 2003 Robert Kooima
 *
 *   SUPER EMPTY BALL is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by the
 *   Free Software Foundation;  either version 2 of the  License, or (at your
 *   option) any later version.
 *
 *   This program  is distributed  in the  hope that it  will be  useful, but
 *   WITHOUT   ANY   WARRANTY;  without   even   the   implied  warranty   of
 *   MERCHANTABILITY  or  FITNESS FOR  A  PARTICULAR  PURPOSE.   See the  GNU
 *   General Public License for more details.
 */

#ifndef IMAGE_H
#define IMAGE_H

#include <SDL/SDL.h>

#include "gl.h"

/*---------------------------------------------------------------------------*/

struct image
{
    SDL_Surface *s;
    GLuint       o;
};

int  image_load(struct image *, const char *);
void image_free(struct image *);
void image_bind(struct image *);
void image_rect(struct image *, double, double, double, double, double);

/*---------------------------------------------------------------------------*/

#endif
