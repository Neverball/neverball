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

#ifndef SOL_H
#define SOL_H

#include <SDL/SDL.h>

#include "gl.h"

#define PATHMAX 44

/*---------------------------------------------------------------------------*/

struct s_imag
{
    SDL_Surface *S;
    GLuint       o;
};

struct s_list
{
    GLuint o;                                  /* opaque geometry list       */
    GLuint t;                                  /* transparent geometry list  */
};

struct s_mtrl
{
    float a[4];                                /* ambient color              */
    float d[4];                                /* diffuse color              */
    float s[4];                                /* specular color             */
    float e[4];                                /* emission color             */
    float h[1];                                /* specular exponent          */
    char  f[PATHMAX];                          /* texture file name          */
};

struct s_vert
{
    double p[3];                               /* vertex position            */
};

struct s_edge
{
    int vi;
    int vj;
};

struct s_side
{
    double n[3];                               /* plane normal vector        */
    double d;                                  /* distance from origin       */
};

struct s_texc
{
    double u[2];                               /* texture coordinate         */
};

struct s_geom
{
    int si;
    int mi;
    int ti, vi;
    int tj, vj;
    int tk, vk;
};

struct s_lump
{
    int fl;
    int v0;
    int vc;
    int e0;
    int ec;
    int g0;
    int gc;
    int s0;
    int sc;
};

struct s_node
{
    int si;
    int ni;
    int nj;
    int l0;
    int lc;
};

struct s_path
{
    double p[3];                               /* starting position          */
    double t;                                  /* travel time                */

    int pi;
};

struct s_body
{
    double t;                                  /* time on current path       */

    int pi;
    int ni;
    int l0;
    int lc;
};

struct s_coin
{
    double p[3];                               /* position                   */
    int    n;                                  /* value                      */
};

struct s_goal
{
    double p[3];                               /* position                   */
    double r;                                  /* radius                     */
};

struct s_ball
{
    double e[3][3];                            /* basis of orientation       */
    double p[3];                               /* position vector            */
    double v[3];                               /* velocity vector            */
    double w[3];                               /* angular velocity vector    */
    double r;                                  /* radius                     */
};

struct s_file
{
    int mc;
    int vc;
    int ec;
    int sc;
    int tc;
    int gc;
    int lc;
    int nc;
    int pc;
    int bc;
    int cc;
    int zc;
    int uc;
    int ic;

    struct s_imag *xv;
    struct s_list *dv;
    struct s_part *qv;

    struct s_mtrl *mv;
    struct s_vert *vv;
    struct s_edge *ev;
    struct s_side *sv;
    struct s_texc *tv;
    struct s_geom *gv;
    struct s_lump *lv;
    struct s_node *nv;
    struct s_path *pv;
    struct s_body *bv;
    struct s_coin *cv;
    struct s_goal *zv;
    struct s_ball *uv;
    int           *iv;
};

/*---------------------------------------------------------------------------*/

void sol_render(const struct s_file *);

int  sol_load(struct s_file *, const char *);
int  sol_stor(struct s_file *, const char *);
void sol_free(struct s_file *);

double sol_update(struct s_file *, const double[3], double);

#endif
