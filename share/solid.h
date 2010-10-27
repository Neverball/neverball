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
 * The Xs are as documented by s_file:
 *
 *     f  File          (s_file)
 *     m  Material      (s_mtrl)
 *     v  Vertex        (s_vert)
 *     e  Edge          (s_edge)
 *     s  Side          (s_side)
 *     t  Texture coord (s_texc)
 *     g  Geometry      (s_geom)
 *     l  Lump          (s_lump)
 *     n  Node          (s_node)
 *     p  Path          (s_path)
 *     b  Body          (s_body)
 *     h  Item          (s_item)
 *     z  Goal          (s_goal)
 *     j  Jump          (s_jump)
 *     x  Switch        (s_swch)
 *     r  Billboard     (s_bill)
 *     u  User          (s_ball)
 *     w  Viewpoint     (s_view)
 *     d  Dictionary    (s_dict)
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
 * These prefixes are still available: c k o q y.
 */

/*---------------------------------------------------------------------------*/

/* Material type flags */

#define M_OPAQUE       1
#define M_TRANSPARENT  2
#define M_REFLECTIVE   4
#define M_ENVIRONMENT  8
#define M_ADDITIVE    16
#define M_CLAMPED     32
#define M_DECAL       64
#define M_TWO_SIDED  128

/* Billboard types. */

#define B_EDGE     1
#define B_FLAT     2
#define B_ADDITIVE 4
#define B_NOFACE   8

/* Lump flags. */

#define L_DETAIL   1

/* Item types. */

#define ITEM_NONE       0
#define ITEM_COIN       1
#define ITEM_GROW       2
#define ITEM_SHRINK     3

/* Path flags. */

#define P_ORIENTED 1

/*---------------------------------------------------------------------------*/

typedef struct
{
    float d[4];                                /* diffuse color              */
    float a[4];                                /* ambient color              */
    float s[4];                                /* specular color             */
    float e[4];                                /* emission color             */
    float h[1];                                /* specular exponent          */
    float angle;

    int fl;                                    /* material flags             */

    GLuint o;                                  /* OpenGL texture object      */
    char   f[PATHMAX];                         /* texture file name          */
} s_mtrl;

typedef struct
{
    float p[3];                                /* vertex position            */
} s_vert;

typedef struct
{
    int vi;
    int vj;
} s_edge;

typedef struct
{
    float n[3];                                /* plane normal vector        */
    float d;                                   /* distance from origin       */
} s_side;

typedef struct
{
    float u[2];                                /* texture coordinate         */
} s_texc;

typedef struct
{
    int mi;
    int ti, si, vi;
    int tj, sj, vj;
    int tk, sk, vk;
} s_geom;

typedef struct
{
    int fl;                                    /* lump flags                 */
    int v0, vc;
    int e0, ec;
    int g0, gc;
    int s0, sc;
} s_lump;

typedef struct
{
    int si;
    int ni;
    int nj;
    int l0;
    int lc;
} s_node;

typedef struct
{
    float p[3];                                /* starting position          */
    float e[4];                                /* orientation (quaternion)   */
    float t;                                   /* travel time                */

    int pi;
    int f;                                     /* enable flag                */
    int s;                                     /* smooth flag                */

    int fl;                                    /* flags                      */

    /* TODO: merge enable and smooth into flags. */
} s_path;

typedef struct
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
} s_body;

typedef struct
{
    float p[3];                                /* position                   */
    int   t;                                   /* type                       */
    int   n;                                   /* value                      */
} s_item;

typedef struct
{
    float p[3];                                /* position                   */
    float r;                                   /* radius                     */
} s_goal;

typedef struct
{
    float p[3];                                /* position                   */
    float r;                                   /* radius                     */
    int  pi;                                   /* the linked path            */

    float t0;                                  /* default timer              */
    float t;                                   /* current timer              */
    int   f0;                                  /* default state              */
    int   f;                                   /* current state              */
    int   i;                                   /* is invisible?              */
    int   e;                                   /* is a ball inside it?       */
} s_swch;

typedef struct
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

    float p[3];
} s_bill;

typedef struct
{
    float p[3];                                /* position                   */
    float q[3];                                /* target position            */
    float r;                                   /* radius                     */
} s_jump;

typedef struct
{
    float e[3][3];                             /* basis of orientation       */
    float p[3];                                /* position vector            */
    float v[3];                                /* velocity vector            */
    float w[3];                                /* angular velocity vector    */
    float E[3][3];                             /* basis of pendulum          */
    float W[3];                                /* angular pendulum velocity  */
    float r;                                   /* radius                     */
} s_ball;

typedef struct
{
    float p[3];
    float q[3];
} s_view;

typedef struct
{
    int ai;
    int aj;
} s_dict;

typedef struct
{
    int ac;
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
    int hc;
    int zc;
    int jc;
    int xc;
    int rc;
    int uc;
    int wc;
    int dc;
    int ic;

    char   *av;
    s_mtrl *mv;
    s_vert *vv;
    s_edge *ev;
    s_side *sv;
    s_texc *tv;
    s_geom *gv;
    s_lump *lv;
    s_node *nv;
    s_path *pv;
    s_body *bv;
    s_item *hv;
    s_goal *zv;
    s_jump *jv;
    s_swch *xv;
    s_bill *rv;
    s_ball *uv;
    s_view *wv;
    s_dict *dv;
    int    *iv;
} s_file;

/*---------------------------------------------------------------------------*/

int   sol_load_only_file(s_file *, const char *);
int   sol_load_only_head(s_file *, const char *);
int   sol_stor(s_file *, const char *);
void  sol_free(s_file *);

/*---------------------------------------------------------------------------*/

#endif
