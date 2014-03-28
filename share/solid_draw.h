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

#include "glext.h"
#include "solid_base.h"
#include "solid_vary.h"
#include "mtrl.h"

/*
 * Rendered solid data.
 */

/*---------------------------------------------------------------------------*/

/* Named rendering pass indices. */

enum
{
    PASS_OPAQUE = 0,
    PASS_OPAQUE_DECAL,
    PASS_TRANSPARENT_DECAL,
    PASS_TRANSPARENT,
    PASS_REFLECTIVE,
    PASS_MAX
};

/*---------------------------------------------------------------------------*/

struct d_vert
{
    float p[3];
    float n[3];
    float t[2];
};

struct d_geom
{
    GLushort i;
    GLushort j;
    GLushort k;
};

/*---------------------------------------------------------------------------*/

struct d_mesh
{
    int mtrl;                                  /* Cached material            */

    GLuint vbo;                                /* Vertex  buffer object      */
    GLuint vbc;                                /* Vertex  buffer count       */
    GLuint ebo;                                /* Element buffer object      */
    GLuint ebc;                                /* Element buffer count       */
};

struct d_body
{
    const struct b_body *base;

    int pass[PASS_MAX];
    int mc;

    struct d_mesh *mv;
};

struct s_draw
{
    struct s_base *base;
    struct s_vary *vary;

    int bc;

    struct d_body *bv;

    GLuint bill;

    unsigned int reflective:1;
    unsigned int shadowed:1;

    int shadow_ui;
};

/*---------------------------------------------------------------------------*/

/*
 * This structure holds rendering state shared between separate
 * SOLs. I am aware that the name leaves much to be desired.
 */

struct s_rend
{
    struct mtrl curr_mtrl;              /* Current material state            */

    int skip_flags;                     /* Ignored material flags            */

    unsigned int color_mtrl:1;          /* Color material flag               */
};

void r_draw_enable(struct s_rend *);
void r_draw_disable(struct s_rend *);

void r_color_mtrl(struct s_rend *, int);
void r_apply_mtrl(struct s_rend *, int);

/*---------------------------------------------------------------------------*/

int  sol_load_draw(struct s_draw *, struct s_vary *, int);
void sol_free_draw(struct s_draw *);

void sol_back(const struct s_draw *, struct s_rend *, float, float, float);
void sol_refl(const struct s_draw *, struct s_rend *);
void sol_draw(const struct s_draw *, struct s_rend *, int, int);
void sol_bill(const struct s_draw *, struct s_rend *, const float *, float);
void sol_fade(const struct s_draw *, struct s_rend *, float);

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
