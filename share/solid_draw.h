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

#ifndef SOLID_DRAW_H
#define SOLID_DRAW_H

#include "solid_base.h"
#include "solid_vary.h"

/*
 * Rendered solid data.
 */

/*---------------------------------------------------------------------------*/

struct d_mtrl
{
    const struct b_mtrl *base;

    GLuint o;                                  /* OpenGL texture object      */
};

struct d_body
{
    const struct b_body *base;

    GLuint ol;                                 /* opaque geometry list       */
    GLuint tl;                                 /* transparent geometry list  */
    GLuint rl;                                 /* reflective geometry list   */
    GLuint sl;                                 /* shadowed geometry list     */
};

struct s_draw
{
    const struct s_base *base;
    const struct s_vary *vary;

    int mc;
    int bc;

    struct d_mtrl *mv;
    struct d_body *bv;

    int reflective;
};

/*---------------------------------------------------------------------------*/

int  sol_load_draw(struct s_draw *, const struct s_vary *, int);
void sol_free_draw(struct s_draw *);

void sol_back(const struct s_draw *, float, float, float);
void sol_refl(const struct s_draw *);
void sol_draw(const struct s_draw *, int, int);
void sol_bill(const struct s_draw *, const float *, float);
void sol_shad(const struct s_draw *);

/*---------------------------------------------------------------------------*/

struct s_full
{
    struct s_base base;
    struct s_vary vary;
    struct s_draw draw;
};

int  sol_load_full(struct s_full *, const char *, int);
void sol_free_full(struct s_full *);

/*---------------------------------------------------------------------------*/

#endif
