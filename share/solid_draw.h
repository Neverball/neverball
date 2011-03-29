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

struct d_vert
{
    float p[3];
    float n[3];
    float t[2];
    float u[2];
};

struct d_geom
{
    GLushort i;
    GLushort j;
    GLushort k;
};

/*---------------------------------------------------------------------------*/

struct d_mtrl
{
    const struct b_mtrl *base;

    GLuint o;                                  /* OpenGL texture object      */
};

struct d_mesh
{
    const struct d_mtrl *mp;

    GLuint vbo;                                /* Vertex  buffer object      */
    GLuint ebo;                                /* Element buffer object      */
    GLuint ebc;                                /* Element buffer count       */
};

struct d_body
{
    const struct b_body *base;

    int mc;

    struct d_mesh *mv;
};

struct s_draw
{
    const struct s_base *base;
    const struct s_vary *vary;

    int mc;
    int bc;

    struct d_mtrl *mv;
    struct d_body *bv;

    GLuint bill;

    int reflective;
};

/*---------------------------------------------------------------------------*/

int  sol_load_draw(struct s_draw *, const struct s_vary *, int);
void sol_free_draw(struct s_draw *);

void sol_back(const struct s_draw *, float, float, float);
void sol_refl(const struct s_draw *);
void sol_draw(const struct s_draw *, int, int);
void sol_bill(const struct s_draw *, const float *, float);
void sol_shad(const struct s_draw *, int);
void sol_fade(const struct s_draw *, float);

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
