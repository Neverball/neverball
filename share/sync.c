/*
 * Copyright (C) 2003 Robert Kooima
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

/*---------------------------------------------------------------------------*/

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif

void sync_init(void)
{
#ifdef __APPLE__
    long swap = 1;
    CGLSetParameter(CGLGetCurrentContext(),  kCGLCPSwapInterval, &swap);
#endif
    return;
}

/*---------------------------------------------------------------------------*/
