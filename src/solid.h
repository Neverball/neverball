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

/*
 * Some might  be taken  aback at  the terseness of  the names  of the
 * structure  members and  the variables  used by  the  functions that
 * access them.  Yes,  yes, I know, readability.  I  contend that once
 * the naming  convention is embraced, the names  become more readable
 * than any  verbose alternative, and their brevity  and uniformity do
 * more to augment readability than longVariableNames ever could.
 *
 * Members  and variables  are named  XY.   X determines  the type  of
 * structure to which the variable  refers.  Y determines the usage of
 * the variable.
 *
 * The Xs are as documented by struct s_file:
 * 
 *     x  Image         (struct s_imag)
 *     d  Display list  (struct s_list)
 *     m  Material      (struct s_mtrl)
 *     v  Vertex        (struct s_vert)
 *     e  Edge          (struct s_edge)
 *     s  Side          (struct s_side)
 *     t  Texture coord (struct s_texc)
 *     g  Geometry      (struct s_geom)
 *     l  Lump          (struct s_lump)
 *     n  Node          (struct s_node)
 *     p  Path          (struct s_path)
 *     b  Body          (struct s_body)
 *     c  Coin          (struct s_coin)
 *     z  Goal          (struct s_goal)
 *     u  User          (struct s_ball)
 *     i  Index         (int)
 *     
 * The Ys are as follows:
 *
 *     c  Counter
 *     p  Pointer
 *     v  Vector (array)
 *     0  Index of the first of block
 *     i  Index
 *     j  Subindex
 *     k  Subsubindex
 *
 * Thus "vp" is  a pointer to a vertex.  "lc" is  the number of lumps.
 * "ei" and "ej" are edge indices  into some "ev" edge vector.  And so
 * on.
 * 
 * Those members that do not conform to this convention are explicitly
 * documented with a comment.
 */

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
    int fl;                                    /* lump flags                 */
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
