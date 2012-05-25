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

#ifndef SOLID_BASE_H
#define SOLID_BASE_H

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
 * The Xs are as documented by struct s_base:
 *
 *     f  File          (struct s_base)
 *     m  Material      (struct b_mtrl)
 *     v  Vertex        (struct b_vert)
 *     e  Edge          (struct b_edge)
 *     s  Side          (struct b_side)
 *     t  Texture coord (struct b_texc)
 *     g  Geometry      (struct b_geom)
 *     o  Offset        (struct b_offs)
 *     l  Lump          (struct b_lump)
 *     n  Node          (struct b_node)
 *     p  Path          (struct b_path)
 *     b  Body          (struct b_body)
 *     h  Item          (struct b_item)
 *     z  Goal          (struct b_goal)
 *     j  Jump          (struct b_jump)
 *     x  Switch        (struct b_swch)
 *     r  Billboard     (struct b_bill)
 *     u  User          (struct b_ball)
 *     w  Viewpoint     (struct b_view)
 *     d  Dictionary    (struct b_dict)
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
 * These prefixes are still available: c k q y.
 */

/*
 * Additionally, solid data is split into three main parts: static
 * data (base), simulation data (vary), and rendering data (draw).
 */

/*---------------------------------------------------------------------------*/

/* Material type flags */

#define M_ALPHA_TEST  (1 << 10)
#define M_SEMI_OPAQUE (1 <<  9)
#define M_REFLECTIVE  (1 <<  8)
#define M_TRANSPARENT (1 <<  7)
#define M_SHADOWED    (1 <<  6)
#define M_DECAL       (1 <<  5)
#define M_ENVIRONMENT (1 <<  4)
#define M_TWO_SIDED   (1 <<  3)
#define M_ADDITIVE    (1 <<  2)
#define M_CLAMP_S     (1 <<  1)
#define M_CLAMP_T     (1 <<  0)

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

struct b_mtrl
{
    float d[4];                                /* diffuse color              */
    float a[4];                                /* ambient color              */
    float s[4];                                /* specular color             */
    float e[4];                                /* emission color             */
    float h[1];                                /* specular exponent          */
    float angle;

    int fl;                                    /* material flags             */

    char   f[PATHMAX];                         /* texture file name          */

    /* M_SEMI_OPAQUE */
    float semi_opaque;                         /* alpha threshold            */

    /* M_ALPHA_TEST */
    float alpha_test;                          /* reference value            */
};

struct b_vert
{
    float p[3];                                /* vertex position            */
};

struct b_edge
{
    int vi;
    int vj;
};

struct b_side
{
    float n[3];                                /* plane normal vector        */
    float d;                                   /* distance from origin       */
};

struct b_texc
{
    float u[2];                                /* texture coordinate         */
};

struct b_offs
{
    int ti, si, vi;
};

struct b_geom
{
    int mi;
    int oi, oj, ok;
};

struct b_lump
{
    int fl;                                    /* lump flags                 */
    int v0, vc;
    int e0, ec;
    int g0, gc;
    int s0, sc;
};

struct b_node
{
    int si;
    int ni;
    int nj;
    int l0;
    int lc;
};

struct b_path
{
    float p[3];                                /* starting position          */
    float e[4];                                /* orientation (quaternion)   */
    float t;                                   /* travel time                */
    int   tm;                                  /* milliseconds               */

    int pi;
    int f;                                     /* enable flag                */
    int s;                                     /* smooth flag                */

    int fl;                                    /* flags                      */

    /* TODO: merge enable and smooth into flags. */
};

struct b_body
{
    int pi;
    int pj;

    int ni;
    int l0;
    int lc;
    int g0;
    int gc;
};

struct b_item
{
    float p[3];                                /* position                   */
    int   t;                                   /* type                       */
    int   n;                                   /* value                      */
};

struct b_goal
{
    float p[3];                                /* position                   */
    float r;                                   /* radius                     */
};

struct b_swch
{
    float p[3];                                /* position                   */
    float r;                                   /* radius                     */
    int  pi;                                   /* the linked path            */

    float t;                                   /* default timer              */
    int   tm;                                  /* milliseconds               */
    int   f;                                   /* default state              */
    int   i;                                   /* is invisible?              */
};

struct b_bill
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
};

struct b_jump
{
    float p[3];                                /* position                   */
    float q[3];                                /* target position            */
    float r;                                   /* radius                     */
};

struct b_ball
{
    float p[3];                                /* position vector            */
    float r;                                   /* radius                     */
};

struct b_view
{
    float p[3];
    float q[3];
};

struct b_dict
{
    int ai;
    int aj;
};

struct s_base
{
    int ac;
    int mc;
    int vc;
    int ec;
    int sc;
    int tc;
    int oc;
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

    char          *av;
    struct b_mtrl *mv;
    struct b_vert *vv;
    struct b_edge *ev;
    struct b_side *sv;
    struct b_texc *tv;
    struct b_offs *ov;
    struct b_geom *gv;
    struct b_lump *lv;
    struct b_node *nv;
    struct b_path *pv;
    struct b_body *bv;
    struct b_item *hv;
    struct b_goal *zv;
    struct b_jump *jv;
    struct b_swch *xv;
    struct b_bill *rv;
    struct b_ball *uv;
    struct b_view *wv;
    struct b_dict *dv;
    int           *iv;
};

/*---------------------------------------------------------------------------*/

int  sol_load_base(struct s_base *, const char *);
int  sol_load_meta(struct s_base *, const char *);
void sol_free_base(struct s_base *);
int  sol_stor_base(struct s_base *, const char *);

/*---------------------------------------------------------------------------*/

struct path
{
    char prefix[16];
    char suffix[8];
};

#define CONCAT_PATH(dst, path, name) do { \
    SAFECPY((dst), (path)->prefix);       \
    SAFECAT((dst), (name));               \
    SAFECAT((dst), (path)->suffix);       \
} while (0)

extern const struct path tex_paths[4];
extern const struct path mtrl_paths[2];

/*---------------------------------------------------------------------------*/

#endif
