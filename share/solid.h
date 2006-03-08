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

#ifndef SOL_H
#define SOL_H

#include <SDL.h>
#include "glext.h"
#include "base_config.h"

/*
 * Some might  be taken  aback at  the terseness of  the names  of the
 * structure  members and  the variables  used by  the  functions that
 * access them.  Yes, yes, I know:  readability.  I  contend that once
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
 *     f  File          (struct s_file)
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
 *     j  Jump          (struct s_jump)
 *     x  Switch        (struct s_swch)
 *     r  Billboard     (struct s_bill)
 *     u  User          (struct s_ball)
 *     w  Viewpoint     (struct s_view)
 *     d  Dictionary    (struct s_dict)
 *     i  Index         (int)
 *     a  Text          (char)
 *
 * The Ys are as follows:
 *
 *     c  Counter
 *     p  Pointer
 *     v  Vector (array)
 *     0  Index of the first
 *     i  Index
 *     j  Subindex
 *     k  Subsubindex
 *
 * Thus "up" is a pointer to  a user structure.  "lc" is the number of
 * lumps.  "ei" and "ej" are  edge indices into some "ev" edge vector.
 * An edge is  defined by two vertices, so  an edge structure consists
 * of "vi" and "vj".  And so on.
 * 
 * Those members that do not conform to this convention are explicitly
 * documented with a comment.
 *
 * These prefixes are still available: h k o q y.
 */

/*---------------------------------------------------------------------------*/

/* Material type flags */

#define M_OPAQUE       1
#define M_TRANSPARENT  2
#define M_REFLECTIVE   4
#define M_ENVIRONMENT  8
#define M_ADDITIVE    16
#define M_CLAMPED     32
#define M_SHADOWED (M_OPAQUE | M_REFLECTIVE | M_ENVIRONMENT)

/* Billboard types. */

#define B_EDGE     1
#define B_FLAT     2
#define B_ADDITIVE 4

/* Lump flags. */

#define L_DETAIL   1

/*---------------------------------------------------------------------------*/

struct s_mtrl
{
    float a[4];                                /* ambient color              */
    float d[4];                                /* diffuse color              */
    float s[4];                                /* specular color             */
    float e[4];                                /* emission color             */
    float h[1];                                /* specular exponent          */

    int fl;                                    /* material flags             */

    GLuint o;                                  /* OpenGL texture object      */
    char   f[PATHMAX];                         /* texture file name          */
};

struct s_vert
{
    float p[3];                                /* vertex position            */
};

struct s_edge
{
    int vi;
    int vj;
};

struct s_side
{
    float n[3];                                /* plane normal vector        */
    float d;                                   /* distance from origin       */
};

struct s_texc
{
    float u[2];                                /* texture coordinate         */
};

struct s_geom
{
    int mi;
    int ti, si, vi;
    int tj, sj, vj;
    int tk, sk, vk;
};

struct s_lump
{
    int fl;                                    /* lump flags                 */
    int v0, vc;
    int e0, ec;
    int g0, gc;
    int s0, sc;
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
    float p[3];                                /* starting position          */
    float t;                                   /* travel time                */

    int pi;
    int f;                                     /* enable flag                */
};

struct s_body
{
    float t;                                   /* time on current path       */

    GLuint ol;                                 /* opaque geometry list       */
    GLuint tl;                                 /* transparent geometry list  */
    GLuint rl;                                 /* reflective geometry list   */
    GLuint sl;                                 /* shadowed geometry list     */

    int pi;
    int ni;
    int l0;
    int lc;
    int g0;
    int gc;
};

struct s_coin
{
    float p[3];                                /* position                   */
    int   n;                                   /* value                      */
};

struct s_goal
{
    float p[3];                                /* position                   */
    float r;                                   /* radius                     */
    int   s;                                   /* level skiped               */
};

struct s_swch
{
    float p[3];                                /* position                   */
    float r;                                   /* radius                     */
    int  pi;

    float t0;                                  /* default timer              */
    float t;                                   /* current timer              */
    int   f0;                                  /* default state              */
    int   f;                                   /* current state              */
    int   i;                                   /* is invisible?              */
};

struct s_bill
{
    int  fl;
    int  mi;
    float t;                                   /* repeat time interval       */
    float d;                                   /* distance                   */

    float w[3];                                /* width coefficients         */
    float h[3];                                /* height coefficients        */

    float rx[3];                               /* X rotation coefficients    */
    float ry[3];                               /* Y rotation coefficients    */
    float rz[3];                               /* Z rotation coefficients    */
};

struct s_jump
{
    float p[3];                                /* position                   */
    float q[3];                                /* target position            */
    float r;                                   /* radius                     */
};

struct s_ball
{
    float e[3][3];                             /* basis of orientation       */
    float p[3];                                /* position vector            */
    float v[3];                                /* velocity vector            */
    float w[3];                                /* angular velocity vector    */
    float r;                                   /* radius                     */
};

struct s_view
{
    float p[3];
    float q[3];
};

struct s_dict
{
    int ai;
    int aj;
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
    int jc;
    int xc;
    int rc;
    int uc;
    int wc;
    int dc;
    int ac;
    int ic;

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
    struct s_jump *jv;
    struct s_swch *xv;
    struct s_bill *rv;
    struct s_ball *uv;
    struct s_view *wv;
    int           *iv;
    char          *av;
};

/*---------------------------------------------------------------------------*/

int   sol_load_only_file(struct s_file *, const char *);
int   sol_load(struct s_file *, const char *, int, int);
int   sol_stor(struct s_file *, const char *);
void  sol_free(struct s_file *);

void  sol_back(const struct s_file *, float, float, float);
void  sol_refl(const struct s_file *);
void  sol_draw(const struct s_file *);
void  sol_shad(const struct s_file *);

float sol_step(struct s_file *, const float *, float, int, int *);

int   sol_coin_test(struct s_file *, float *, float);
int   sol_goal_test(struct s_file *, float *, int);
int   sol_jump_test(struct s_file *, float *, int);
int   sol_swch_test(struct s_file *, int, int);

/*---------------------------------------------------------------------------*/

void put_file_state(FILE *, struct s_file *);
void get_file_state(FILE *, struct s_file *);

/*---------------------------------------------------------------------------*/

#endif
