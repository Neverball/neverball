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

#ifndef SOL_GL_H
#define SOL_GL_H

#include "solid.h"

/*---------------------------------------------------------------------------*/

int   sol_load_gl(s_file *, const char *, int);
void  sol_free_gl(s_file *);

int   sol_reflective(const s_file *);

void  sol_back(const s_file *, float, float, float);
void  sol_refl(const s_file *);
void  sol_draw(const s_file *, int, int);
void  sol_bill(const s_file *, const float *, float);
void  sol_shad(const s_file *);

/*---------------------------------------------------------------------------*/

#endif
