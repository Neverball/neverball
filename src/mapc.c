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

/*
 * I'm not  particularly proud  of this chunk  of code.   It was not  so much
 * designed as it was accumulated.
 */

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vec3.h"
#include "solid.h"

#define MAXSTR 256
#define MAXKEY 16
#define SCALE  64.0
#define SMALL  0.0000000005

/*
 * The overall design  of this map converter is very  stupid, but very simple
 * It begins by  assuming that every mtrl, vert, edge, side,  and texc in the
 * map is unique.   It then makes an optimizing  pass that discards redundant
 * information.   The  result is  optimal,  though  the  process is  terribly
 * inefficient.
 */

/*---------------------------------------------------------------------------*/

/* Arbitrary limits woo! */

#define MAXM	4096
#define MAXV	4096
#define MAXE	4096
#define MAXS	2048
#define MAXT	4096
#define MAXG	4096
#define MAXL	1024
#define MAXN	1024
#define MAXP	512
#define MAXB	512
#define MAXC	1024
#define MAXZ    8
#define MAXU	16
#define MAXI	16384

static void init_file(struct s_file *fp)
{
    fp->mc = 0;
    fp->vc = 0;
    fp->ec = 0;
    fp->sc = 0;
    fp->tc = 0;
    fp->gc = 0;
    fp->lc = 0;
    fp->nc = 0;
    fp->pc = 0;
    fp->bc = 0;
    fp->cc = 0;
    fp->zc = 0;
    fp->uc = 0;
    fp->ic = 0;

    fp->mv = (struct s_mtrl *) calloc(MAXM, sizeof (struct s_mtrl));
    fp->vv = (struct s_vert *) calloc(MAXV, sizeof (struct s_vert));
    fp->ev = (struct s_edge *) calloc(MAXE, sizeof (struct s_edge));
    fp->sv = (struct s_side *) calloc(MAXS, sizeof (struct s_side));
    fp->tv = (struct s_texc *) calloc(MAXT, sizeof (struct s_texc));
    fp->gv = (struct s_geom *) calloc(MAXG, sizeof (struct s_geom));
    fp->lv = (struct s_lump *) calloc(MAXL, sizeof (struct s_lump));
    fp->nv = (struct s_node *) calloc(MAXN, sizeof (struct s_node));
    fp->pv = (struct s_path *) calloc(MAXP, sizeof (struct s_path));
    fp->bv = (struct s_body *) calloc(MAXB, sizeof (struct s_body));
    fp->cv = (struct s_coin *) calloc(MAXC, sizeof (struct s_coin));
    fp->zv = (struct s_goal *) calloc(MAXZ, sizeof (struct s_goal));
    fp->uv = (struct s_ball *) calloc(MAXU, sizeof (struct s_ball));
    fp->iv = (int           *) calloc(MAXI, sizeof (int));
}

/*---------------------------------------------------------------------------*/

/*
 * The following is  a small symbol table data  structure.  Symbols and their
 * integer values are collected in symv and valv.  References and pointers to
 * their  unsatisfied integer  values are  collected in  refv and  pntv.  The
 * resolve  procedure matches references  to symbols  and fills  waiting ints
 * with the proper values.
 */

#define MAXSYM 1024

static char symv[MAXSYM][MAXSTR];
static int  valv[MAXSYM];

static char refv[MAXSYM][MAXSTR];
static int *pntv[MAXSYM];

static int   strc = 0;
static int   refc = 0;

static void make_sym(const char *s, int  v)
{
    strcpy(symv[strc], s);
    valv[strc] = v;
    strc++;
}

static void make_ref(const char *r, int *p)
{
    strcpy(refv[refc], r);
    pntv[refc] = p;
    refc++;
}

static void resolve(void)
{
    int i, j;

    for (i = 0; i < refc; i++)
        for (j = 0; j < strc; j++)
            if (strcmp(refv[i], symv[j]) == 0)
            {
                *(pntv[i]) = valv[j];
                break;
            }
}

/*---------------------------------------------------------------------------*/

static void size_image(const char *s, int *w, int *h)
{
    SDL_Surface *S = IMG_Load(s);

    if (S)
    {
        *w = S->w;
        *h = S->h;

        SDL_FreeSurface(S);
    }
    else
    {
        *w = 0;
        *h = 0;
    }
}

/*---------------------------------------------------------------------------*/

static int path_0 = -1;

static double plane_d[MAXS];
static double plane_n[MAXS][3];
static double plane_p[MAXS][3];
static double plane_u[MAXS][3];
static double plane_v[MAXS][3];
static int    plane_f[MAXS];

static void make_plane(int pi, int x0, int y0, int z0,
                       int x1, int y1, int z1,
                       int x2, int y2, int z2,
                       int tu, int tv, int r,
                       float su, float sv, int fl, const char *s)
{
    static const double base[6][3][3] = {
        {{  0,  0,  1 }, {  1,  0,  0 }, {  0, -1,  0 }},
        {{  0,  0, -1 }, {  1,  0,  0 }, {  0, -1,  0 }},
        {{  1,  0,  0 }, {  0,  0, -1 }, {  0, -1,  0 }},
        {{ -1,  0,  0 }, {  0,  0, -1 }, {  0, -1,  0 }},
        {{  0,  1,  0 }, {  1,  0,  0 }, {  0,  0,  1 }},
        {{  0, -1,  0 }, {  1,  0,  0 }, {  0,  0,  1 }},
    };

    double R[16];
    double p0[3], p1[3], p2[3];
    double  u[3],  v[3],  p[3];
    double k, d = 0.0;
    int i, w, h, n = 0;
    char filename[MAXSTR];

    strcpy(filename, s);
    strcat(filename, ".jpg");

    size_image(filename, &w, &h);

    plane_f[pi] = fl;

    p0[0] = +(double) x0 / SCALE;
    p0[1] = +(double) z0 / SCALE;
    p0[2] = -(double) y0 / SCALE;

    p1[0]  = +(double) x1 / SCALE;
    p1[1]  = +(double) z1 / SCALE;
    p1[2]  = -(double) y1 / SCALE;

    p2[0] = +(double) x2 / SCALE;
    p2[1] = +(double) z2 / SCALE;
    p2[2] = -(double) y2 / SCALE;

    v_sub(u, p0, p1);
    v_sub(v, p2, p1);

    v_crs(plane_n[pi], u, v);
    v_nrm(plane_n[pi], plane_n[pi]);
	
    plane_d[pi] = v_dot(plane_n[pi], p1);

    for (i = 0; i < 6; i++)
        if ((k = v_dot(plane_n[pi], base[i][0])) >= d)
        {
            d = k;
            n = i;
        }

    p[0] = 0.0;
    p[1] = 0.0;
    p[2] = 0.0;

    m_rot(R, base[n][0], V_RAD(r));

    v_mad(p, p, base[n][1], su * tu / SCALE);
    v_mad(p, p, base[n][2], sv * tv / SCALE);

    m_vxfm(plane_u[pi], R, base[n][1]);
    m_vxfm(plane_v[pi], R, base[n][2]);
    m_vxfm(plane_p[pi], R, p);

    v_scl(plane_u[pi], plane_u[pi], SCALE / w);
    v_scl(plane_v[pi], plane_v[pi], SCALE / h);

    v_scl(plane_u[pi], plane_u[pi], 1.0 / su);
    v_scl(plane_v[pi], plane_v[pi], 1.0 / sv);
}

/*---------------------------------------------------------------------------*/

#define T_EOF 0
#define T_BEG 1
#define T_CLP 2
#define T_KEY 3
#define T_END 4
#define T_NOP 5

static int map_token(FILE *fin, int pi, char key[MAXSTR],
                     char val[MAXSTR])
{
    char buf[MAXSTR];

    if (fgets(buf, MAXSTR, fin))
    {
        char c;
        int x0, y0, z0;
        int x1, y1, z1;
        int x2, y2, z2;
        int tu, tv, r, fl;
        float su, sv;

        /* Scan the beginning or end of a block. */

        if (buf[0] == '{') return T_BEG;
        if (buf[0] == '}') return T_END;

        /* Scan a key-value pair. */

        if (buf[0] == '"')
        {
            strcpy(key, strtok(buf,  "\""));
            (void)      strtok(NULL, "\"");
            strcpy(val, strtok(NULL, "\""));

            return T_KEY;
        }

        /* Scan a plane. */

        if (sscanf(buf, "%c %d %d %d %c "
                   "%c %d %d %d %c "
                   "%c %d %d %d %c "
                   "%s %d %d %d %f %f %d",
                   &c, &x0, &y0, &z0, &c,
                   &c, &x1, &y1, &z1, &c,
                   &c, &x2, &y2, &z2, &c,
                   key, &tu, &tv, &r, &su, &sv, &fl) == 22)
        {
            make_plane(pi, x0, y0, z0,
                       x1, y1, z1,
                       x2, y2, z2,
                       tu, tv, r, su, sv, fl, key);
            return T_CLP;
        }

        /* If it's not recognized, it must be uninteresting. */

        return T_NOP;
    }
    return T_EOF;
}

/*---------------------------------------------------------------------------*/

/* Read the given material file, adding a new material to the solid.  */

static void read_mtrl(struct s_file *fp, const char *s)
{
    struct s_mtrl *mp = fp->mv + fp->mc++;
    FILE *fin;

    if ((fin = fopen(s, "r")))
    {
        fscanf(fin, "%f %f %f %f "
               "%f %f %f %f "
               "%f %f %f %f "
               "%f %f %f %f "
               "%f ",
               mp->d, mp->d + 1, mp->d + 2, mp->d + 3,
               mp->a, mp->a + 1, mp->a + 2, mp->a + 3,
               mp->s, mp->s + 1, mp->s + 2, mp->s + 3,
               mp->e, mp->e + 1, mp->e + 2, mp->e + 3,
               mp->h);
        fclose(fin);

        strcpy(mp->f, "data/");
        strcat(mp->f, s);
        strcat(mp->f, ".jpg");
    }
}

/*
 * Parse a lump  from the given file and  add it to the solid.   Note a small
 * hack here  in mapping  materials onto sides.   Material indices  cannot be
 * assigned until faces  are computed, so for now there is  assumed to be one
 * material  per side,  and that  a side  index equals  that  side's material
 * index.  See clip_lump and clip_geom.
 */
static void read_lump(struct s_file *fp, FILE *fin)
{
    char k[MAXSTR];
    char v[MAXSTR];
    int t;

    struct s_lump *lp = fp->lv + fp->lc++;

    lp->s0 = fp->ic;

    while ((t = map_token(fin, fp->sc, k, v)))
    {
        if (t == T_CLP)
        {
            fp->sv[fp->sc].n[0] = plane_n[fp->sc][0];
            fp->sv[fp->sc].n[1] = plane_n[fp->sc][1];
            fp->sv[fp->sc].n[2] = plane_n[fp->sc][2];
            fp->sv[fp->sc].d    = plane_d[fp->sc];

            read_mtrl(fp, k);

            fp->iv[fp->ic] = fp->sc;
            fp->ic++;
            fp->sc++;
            lp->sc++;
        }
        if (t == T_END)
            break;
    }
}

/*---------------------------------------------------------------------------*/

static void make_path(struct s_file *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], int c)
{
    int i, pi = fp->pc++;

    struct s_path *pp = fp->pv + pi;

    pp->p[0] = 0.0;
    pp->p[1] = 0.0;
    pp->p[2] = 0.0;
    pp->t    = 1.0;
    pp->pi   = pi;

    for (i = 0; i < c; i++)
    {
        if (!strcmp(k[i], "targetname"))
            make_sym(v[i], pi);

        if (!strcmp(k[i], "target"))
            make_ref(v[i], &pp->pi);

        if (!strcmp(k[i], "speed"))
            sscanf(v[i], "%lf", &pp->t);

        if (!strcmp(k[i], "origin"))
        {
            int x = 0, y = 0, z = 0;

            sscanf(v[i], "%d %d %d", &x, &y, &z);

            pp->p[0] = +(double) x / SCALE;
            pp->p[1] = +(double) z / SCALE;
            pp->p[2] = -(double) y / SCALE;
        }
    }
}

static void make_body(struct s_file *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], int c, int l0)
{
    int i, bi = fp->bc++;

    struct s_body *bp = fp->bv + bi;

    bp->t  = 0.0;
    bp->pi = -1;
    bp->ni = -1;
    bp->l0 = l0;
    bp->lc = fp->lc - l0;

    for (i = 0; i < c; i++)
        if (strcmp(k[i], "target") == 0)
            make_ref(v[i], &bp->pi);
}

static void make_coin(struct s_file *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], int c)
{
    int i, ci = fp->cc++;

    struct s_coin *cp = fp->cv + ci;

    cp->p[0] = 0.0;
    cp->p[1] = 0.0;
    cp->p[2] = 0.0;
    cp->n    = 1;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "light") == 0)
            sscanf(v[i], "%d", &cp->n);

        if (strcmp(k[i], "origin") == 0)
        {
            int x = 0, y = 0, z = 0;

            sscanf(v[i], "%d %d %d", &x, &y, &z);

            cp->p[0] = +(double) x / SCALE;
            cp->p[1] = +(double) z / SCALE;
            cp->p[2] = -(double) y / SCALE;
        }
    }
}

static void make_goal(struct s_file *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], int c)
{
    int i, zi = fp->zc++;

    struct s_goal *zp = fp->zv + zi;

    zp->p[0] = 0.0;
    zp->p[1] = 0.0;
    zp->p[2] = 0.0;
    zp->r    = 0.75;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "radius") == 0)
            sscanf(v[i], "%lf", &zp->r);

        if (strcmp(k[i], "origin") == 0)
        {
            int x = 0, y = 0, z = 0;

            sscanf(v[i], "%d %d %d", &x, &y, &z);

            zp->p[0] = +(double) x / SCALE;
            zp->p[1] = +(double) z / SCALE;
            zp->p[2] = -(double) y / SCALE;
        }
    }
}

static void make_ball(struct s_file *fp,
                      char k[][MAXSTR],
                      char v[][MAXSTR], int c)
{
    int i, ui = fp->uc++;

    struct s_ball *up = fp->uv + ui;

    up->p[0] = 0.0;
    up->p[1] = 0.0;
    up->p[2] = 0.0;
    up->r    = 0.25;

    up->e[0][0] = 1.0;
    up->e[0][1] = 0.0;
    up->e[0][2] = 0.0;
    up->e[1][0] = 0.0;
    up->e[1][1] = 1.0;
    up->e[1][2] = 0.0;
    up->e[2][0] = 0.0;
    up->e[2][1] = 0.0;
    up->e[2][2] = 1.0;

    up->v[0] = 0.0;
    up->v[1] = 0.0;
    up->v[2] = 0.0;
    up->w[0] = 0.0;
    up->w[1] = 0.0;
    up->w[2] = 0.0;

    for (i = 0; i < c; i++)
    {
        if (strcmp(k[i], "radius") == 0)
            sscanf(v[i], "%lf", &up->r);

        if (!strcmp(k[i], "target"))
            make_ref(v[i], &path_0);

        if (strcmp(k[i], "origin") == 0)
        {
            int x = 0, y = 0, z = 0;

            sscanf(v[i], "%d %d %d", &x, &y, &z);

            up->p[0] = +(double) (x)      / SCALE;
            up->p[1] = +(double) (z - 24) / SCALE;
            up->p[2] = -(double) (y)      / SCALE;
        }
    }

    up->p[1] += up->r + SMALL;
}

/*---------------------------------------------------------------------------*/

static void read_ent(struct s_file *fp, FILE *fin)
{
    char k[MAXKEY][MAXSTR];
    char v[MAXKEY][MAXSTR];
    int t, i = 0, c = 0;

    int l0 = fp->lc;

    while ((t = map_token(fin, -1, k[c], v[c])))
    {
        if (t == T_KEY)
        {
            if (strcmp(k[c], "classname") == 0)
                i = c;
            c++;
        }
        if (t == T_BEG)
            read_lump(fp, fin);
        if (t == T_END)
            break;
    }

    if (!strcmp(v[i], "light"))             make_coin(fp, k, v, c);
    if (!strcmp(v[i], "path_corner"))       make_path(fp, k, v, c);
    if (!strcmp(v[i], "info_player_start")) make_ball(fp, k, v, c);
    if (!strcmp(v[i], "target_teleporter")) make_goal(fp, k, v, c);

    if (!strcmp(v[i], "worldspawn"))  make_body(fp, k, v, c, l0);
    if (!strcmp(v[i], "func_train"))  make_body(fp, k, v, c, l0);
}

static void read_map(struct s_file *fp, FILE *fin)
{
    char k[MAXSTR];
    char v[MAXSTR];
    int t;

    while ((t = map_token(fin, -1, k, v)))
        if (t == T_BEG)
            read_ent(fp, fin);
}

/*---------------------------------------------------------------------------*/

/*
 * All bodies  with an associated  path are assumed  to be positioned  at the
 * beginning of that path.  These bodies must be moved to the origin in order
 * for their  path transforms to behave  correctly.  This is how  we get away
 * with defining func_trains with no origin specification.
 */

static void move_side(struct s_side *sp, const double p[3])
{
    sp->d -= v_dot(sp->n, p);
}

static void move_lump(struct s_file *fp,
                      struct s_lump *lp, const double p[3])
{
    int i;

    for (i = 0; i < lp->sc; i++)
        move_side(fp->sv + fp->iv[lp->s0 + i], p);
}

static void move_body(struct s_file *fp,
                      struct s_body *bp)
{
    int i;

    for (i = 0; i < bp->lc; i++)
        move_lump(fp, fp->lv + bp->l0 + i, fp->pv[bp->pi].p);
}

static void move_file(struct s_file *fp)
{
    int i;

    for (i = 0; i < fp->bc; i++)
        if (fp->bv[i].pi >= 0)
            move_body(fp, fp->bv + i);
}

/*---------------------------------------------------------------------------*/

/* Test the location of a point with respect to a side plane. */

static int fore_side(const double p[3], const struct s_side *sp)
{
    return (v_dot(p, sp->n) - sp->d > +SMALL) ? 1 : 0;
}

static int on_side(const double p[3], const struct s_side *sp)
{
    double d = v_dot(p, sp->n) - sp->d;

    return (-SMALL < d && d < +SMALL) ? 1 : 0;
}

/*---------------------------------------------------------------------------*/
/*
 * Confirm  that  the addition  of  a vert  would  not  result in  degenerate
 * geometry.
 */

static int ok_vert(const struct s_file *fp,
                   const struct s_lump *lp, const double p[3])
{
    double r[3];
    int i;

    for (i = 0; i < lp->vc; i++)
    {
        double *q = fp->vv[fp->iv[lp->v0 + i]].p;

        v_sub(r, p, q);

        if (v_len(r) < SMALL)
            return 0;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

/*
 * The following  functions take the set  of planes defining a  lump and find
 * the verts,  edges, and  geoms that describe  its boundaries.  To  do this,
 * they first find the verts, and then search these verts for valid edges and
 * geoms.  It  may be more efficient  to compute edges and  geoms directly by
 * clipping down  infinite line segments and  planes, but this  would be more
 * complex and prone to numerical error.
 */

/*
 * Given 3 side  planes, compute the point of  intersection, if any.  Confirm
 * that this point falls within the current lump, and that it is unique.  Add
 * it as a vert of the solid.
 */
static void clip_vert(struct s_file *fp,
                      struct s_lump *lp, int si, int sj, int sk)
{
    double M[16], X[16], I[16];
    double d[3],  p[3];
    int i;

    d[0] = fp->sv[si].d;
    d[1] = fp->sv[sj].d;
    d[2] = fp->sv[sk].d;

    m_basis(M, fp->sv[si].n, fp->sv[sj].n, fp->sv[sk].n);
    m_xps(X, M);
	
    if (m_inv(I, X))
    {
        m_vxfm(p, I, d);

        for (i = 0; i < lp->sc; i++)
        {
            int si = fp->iv[lp->s0 + i];

            if (fore_side(p, fp->sv + si))
                return;
        }

        if (ok_vert(fp, lp, p))
        {
            v_cpy(fp->vv[fp->vc].p, p);

            fp->iv[fp->ic] = fp->vc;
            fp->ic++;
            fp->vc++;
            lp->vc++;
        }
    }
}

/*
 * Given two side planes, find an  edge along their intersection by finding a
 * pair of vertices that fall on both planes.  Add it to the solid.
 */
static void clip_edge(struct s_file *fp,
                      struct s_lump *lp, int si, int sj)
{
    int i, j;

    for (i = 1; i < lp->vc; i++)
        for (j = 0; j < i; j++)
        {
            int vi = fp->iv[lp->v0 + i];
            int vj = fp->iv[lp->v0 + j];

            if (on_side(fp->vv[vi].p, fp->sv + si) &&
                on_side(fp->vv[vj].p, fp->sv + si) &&
                on_side(fp->vv[vi].p, fp->sv + sj) &&
                on_side(fp->vv[vj].p, fp->sv + sj))
            {
                fp->ev[fp->ec].vi = vi;
                fp->ev[fp->ec].vj = vj;

                fp->iv[fp->ic] = fp->ec;

                fp->ic++;
                fp->ec++;
                lp->ec++;
            }
        }
}

/*
 * Find all verts that  lie on the given side of the  lump.  Sort these verts
 * to have a counter-clockwise winding  about the plane normal.  Create geoms
 * to tessalate the resulting convex polygon.
 */
static void clip_geom(struct s_file *fp,
                      struct s_lump *lp, int si)
{
    int    m[16], t[16], d, i, j, n = 0;
    double u[3];
    double v[3];
    double w[3];

    struct s_side *sp = fp->sv + si;

    /* Find em. */

    for (i = 0; i < lp->vc; i++)
    {
        int vi = fp->iv[lp->v0 + i];

        if (on_side(fp->vv[vi].p, sp))
        {
            m[n] = vi;
            t[n] = fp->tc++;

            v_add(v, fp->vv[vi].p, plane_p[si]);

            fp->tv[t[n]].u[0] = v_dot(v, plane_u[si]);
            fp->tv[t[n]].u[1] = v_dot(v, plane_v[si]);

            n++;
        }
    }

    /* Sort em. */

    for (i = 1; i < n; i++)
        for (j = i + 1; j < n; j++)
        {
            v_sub(u, fp->vv[m[i]].p, fp->vv[m[0]].p);
            v_sub(v, fp->vv[m[j]].p, fp->vv[m[0]].p);
            v_crs(w, u, v);

            if (v_dot(w, sp->n) < 0.0)
            {
                d     = m[i];
                m[i]  = m[j];
                m[j]  =    d;

                d     = t[i];
                t[i]  = t[j];
                t[j]  =    d;
            }
        }

    /* Index em. */

    for (i = 0; i < n - 2; i++)
    {
        fp->gv[fp->gc].si = si;
        fp->gv[fp->gc].mi = si;

        fp->gv[fp->gc].ti = t[0];
        fp->gv[fp->gc].tj = t[i + 1];
        fp->gv[fp->gc].tk = t[i + 2];

        fp->gv[fp->gc].vi = m[0];
        fp->gv[fp->gc].vj = m[i + 1];
        fp->gv[fp->gc].vk = m[i + 2];

        fp->iv[fp->ic] = fp->gc;
        fp->ic++;
        fp->gc++;
        lp->gc++;
    }
}

/*
 * Iterate the sides  of the lump, attemping to generate a  new vert for each
 * trio of planes, a new edge for each  pair of planes, and a new set of geom
 * for each visible plane.
 */
static void clip_lump(struct s_file *fp, struct s_lump *lp)
{
    int i, j, k;

    lp->v0 = fp->ic;
    lp->vc = 0;

    for (i = 2; i < lp->sc; i++)
        for (j = 1; j < i; j++)
            for (k = 0; k < j; k++)
                clip_vert(fp, lp,
                          fp->iv[lp->s0 + i],
                          fp->iv[lp->s0 + j],
                          fp->iv[lp->s0 + k]);

    lp->e0 = fp->ic;
    lp->ec = 0;

    for (i = 1; i < lp->sc; i++)
        for (j = 0; j < i; j++)
            clip_edge(fp, lp,
                      fp->iv[lp->s0 + i],
                      fp->iv[lp->s0 + j]);

    lp->g0 = fp->ic;
    lp->gc = 0;

    for (i = 0; i < lp->sc; i++)
        if (fp->mv[fp->iv[lp->s0 + i]].d[3] > 0)
            clip_geom(fp, lp,
                      fp->iv[lp->s0 + i]);

    for (i = 0; i < lp->sc; i++)
        lp->fl |= (plane_f[fp->iv[lp->s0 + i]] ? 1 : 0);
}

static void clip_file(struct s_file *fp)
{
    int i;

    for (i = 0; i < fp->lc; i++)
        clip_lump(fp, fp->lv + i);
}

/*---------------------------------------------------------------------------*/

/*
 * For  each body element  type, determine  if element  'p' is  equivalent to
 * element 'q'.  This  is more than a simple  memory compare.  It effectively
 * snaps mtrls and verts togather, and  may reverse the winding of an edge or
 * a geom.  This is done in order to maximize the number of elements that can
 * be eliminated.
 */

static int comp_mtrl(const struct s_mtrl *mp, const struct s_mtrl *mq)
{
    if (fabs(mp->d[0] - mq->d[0]) > SMALL) return 0;
    if (fabs(mp->d[1] - mq->d[1]) > SMALL) return 0;
    if (fabs(mp->d[2] - mq->d[2]) > SMALL) return 0;
    if (fabs(mp->d[3] - mq->d[3]) > SMALL) return 0;

    if (fabs(mp->a[0] - mq->a[0]) > SMALL) return 0;
    if (fabs(mp->a[1] - mq->a[1]) > SMALL) return 0;
    if (fabs(mp->a[2] - mq->a[2]) > SMALL) return 0;
    if (fabs(mp->a[3] - mq->a[3]) > SMALL) return 0;

    if (fabs(mp->s[0] - mq->s[0]) > SMALL) return 0;
    if (fabs(mp->s[1] - mq->s[1]) > SMALL) return 0;
    if (fabs(mp->s[2] - mq->s[2]) > SMALL) return 0;
    if (fabs(mp->s[3] - mq->s[3]) > SMALL) return 0;

    if (fabs(mp->e[0] - mq->e[0]) > SMALL) return 0;
    if (fabs(mp->e[1] - mq->e[1]) > SMALL) return 0;
    if (fabs(mp->e[2] - mq->e[2]) > SMALL) return 0;
    if (fabs(mp->e[3] - mq->e[3]) > SMALL) return 0;

    if (fabs(mp->h[0] - mq->h[0]) > SMALL) return 0;

    if (strcmp(mp->f, mq->f)) return 0;

    return 1;
}

static int comp_vert(const struct s_vert *vp, const struct s_vert *vq)
{
    if (fabs(vp->p[0] - vq->p[0]) > SMALL) return 0;
    if (fabs(vp->p[1] - vq->p[1]) > SMALL) return 0;
    if (fabs(vp->p[2] - vq->p[2]) > SMALL) return 0;

    return 1;
}

static int comp_edge(const struct s_edge *ep, const struct s_edge *eq)
{
    if (ep->vi != eq->vi && ep->vi != eq->vj) return 0;
    if (ep->vj != eq->vi && ep->vj != eq->vj) return 0;

    return 1;
}

static int comp_side(const struct s_side *sp, const struct s_side *sq)
{
    if  (fabs(sp->d - sq->d) >       SMALL) return 0;
    if (v_dot(sp->n,  sq->n) < 1.0 - SMALL) return 0;

    return 1;
}

static int comp_texc(const struct s_texc *tp, const struct s_texc *tq)
{
    if (fabs(tp->u[0] - tq->u[0]) > SMALL) return 0;
    if (fabs(tp->u[1] - tq->u[1]) > SMALL) return 0;

    return 1;
}

static int comp_geom(const struct s_geom *gp, const struct s_geom *gq)
{
    if (gp->si != gq->si) return 0;
    if (gp->mi != gq->mi) return 0;

    /* FIXME: buh. */

    return 0;
}

/*---------------------------------------------------------------------------*/

/*
 * For each file  element type, replace all references to  element 'i' with a
 * reference to element 'j'.  These  are used when optimizing and sorting the
 * file.
 */

static void swap_mtrl(struct s_file *fp, int mi, int mj)
{
    int i;

    for (i = 0; i < fp->gc; i++)
        if (fp->gv[i].mi == mi) fp->gv[i].mi = mj;
}

static void swap_vert(struct s_file *fp, int vi, int vj)
{
    int i, j;

    for (i = 0; i < fp->ec; i++)
    {
        if (fp->ev[i].vi == vi) fp->ev[i].vi = vj;
        if (fp->ev[i].vj == vi) fp->ev[i].vj = vj;
    }

    for (i = 0; i < fp->gc; i++)
    {
        if (fp->gv[i].vi == vi) fp->gv[i].vi = vj;
        if (fp->gv[i].vj == vi) fp->gv[i].vj = vj;
        if (fp->gv[i].vk == vi) fp->gv[i].vk = vj;
    }

    for (i = 0; i < fp->lc; i++)
        for (j = 0; j < fp->lv[i].vc; j++)
            if (fp->iv[fp->lv[i].v0 + j] == vi)
                fp->iv[fp->lv[i].v0 + j]  = vj;
}

static void swap_edge(struct s_file *fp, int ei, int ej)
{
    int i, j;

    for (i = 0; i < fp->lc; i++)
        for (j = 0; j < fp->lv[i].ec; j++)
            if (fp->iv[fp->lv[i].e0 + j] == ei)
                fp->iv[fp->lv[i].e0 + j]  = ej;
}

static void swap_side(struct s_file *fp, int si, int sj)
{
    int i, j;

    for (i = 0; i < fp->gc; i++)
        if (fp->gv[i].si == si) fp->gv[i].si = sj;
    for (i = 0; i < fp->nc; i++)
        if (fp->nv[i].si == si) fp->nv[i].si = sj;

    for (i = 0; i < fp->lc; i++)
        for (j = 0; j < fp->lv[i].sc; j++)
            if (fp->iv[fp->lv[i].s0 + j] == si)
                fp->iv[fp->lv[i].s0 + j]  = sj;
}

static void swap_texc(struct s_file *fp, int ti, int tj)
{
    int i;

    for (i = 0; i < fp->gc; i++)
    {
        if (fp->gv[i].ti == ti) fp->gv[i].ti = tj;
        if (fp->gv[i].tj == ti) fp->gv[i].tj = tj;
        if (fp->gv[i].tk == ti) fp->gv[i].tk = tj;
    }
}

static void swap_geom(struct s_file *fp, int gi, int gj)
{
    int i, j;

    for (i = 0; i < fp->lc; i++)
        for (j = 0; j < fp->lv[i].gc; j++)
            if (fp->iv[fp->lv[i].g0 + j] == gi)
                fp->iv[fp->lv[i].g0 + j]  = gj;
}

static void swap_path(struct s_file *fp, int pi, int pj)
{
    int i;

    for (i = 0; i < fp->pc; i++)
        if (fp->pv[i].pi == pi) fp->pv[i].pi = pj;

    for (i = 0; i < fp->bc; i++)
        if (fp->bv[i].pi == pi) fp->bv[i].pi = pj;
}

/*---------------------------------------------------------------------------*/

static void uniq_mtrl(struct s_file *fp)
{
    int i, j, k = 0;

    for (i = 0; i < fp->mc; i++)
    {
        for (j = 0; j < i; j++)
            if (comp_mtrl(fp->mv + i, fp->mv + j))
            {
                swap_mtrl(fp, i, j);
                break;
            }

        if (i == j)
        {
            fp->mv[k] = fp->mv[i];
            swap_mtrl(fp, i, k);
            k++;
        }
    }

    fp->mc = k;
}

static void uniq_vert(struct s_file *fp)
{
    int i, j, k = 0;

    for (i = 0; i < fp->vc; i++)
    {
        for (j = 0; j < i; j++)
            if (comp_vert(fp->vv + i, fp->vv + j))
            {
                swap_vert(fp, i, j);
                break;
            }

        if (i == j)
        {
            fp->vv[k] = fp->vv[i];
            swap_vert(fp, i, k);
            k++;
        }
    }

    fp->vc = k;
}

static void uniq_edge(struct s_file *fp)
{
    int i, j, k = 0;

    for (i = 0; i < fp->ec; i++)
    {
        for (j = 0; j < i; j++)
            if (comp_edge(fp->ev + i, fp->ev + j))
            {
                swap_edge(fp, i, j);
                break;
            }

        if (i == j)
        {
            fp->ev[k] = fp->ev[i];
            swap_edge(fp, i, k);
            k++;
        }
    }

    fp->ec = k;
}

static void uniq_geom(struct s_file *fp)
{
    int i, j, k = 0;

    for (i = 0; i < fp->gc; i++)
    {
        for (j = 0; j < i; j++)
            if (comp_geom(fp->gv + i, fp->gv + j))
            {
                swap_geom(fp, i, j);
                break;
            }

        if (i == j)
        {
            fp->gv[k] = fp->gv[i];
            swap_geom(fp, i, k);
            k++;
        }
    }

    fp->gc = k;
}

static void uniq_texc(struct s_file *fp)
{
    int i, j, k = 0;

    for (i = 0; i < fp->tc; i++)
    {
        for (j = 0; j < i; j++)
            if (comp_texc(fp->tv + i, fp->tv + j))
            {
                swap_texc(fp, i, j);
                break;
            }
        if (i == j)
        {
            fp->tv[k] = fp->tv[i];
            swap_texc(fp, i, k);
            k++;
        }
    }

    fp->tc = k;
}

static void uniq_side(struct s_file *fp)
{
    int i, j, k = 0;

    for (i = 0; i < fp->sc; i++)
    {
        for (j = 0; j < i; j++)
            if (comp_side(fp->sv + i, fp->sv + j))
            {
                swap_side(fp, i, j);
                break;
            }
        if (i == j)
        {
            fp->sv[k] = fp->sv[i];
            swap_side(fp, i, k);
            k++;
        }
    }

    fp->sc = k;
}

static void uniq_file(struct s_file *fp)
{
    uniq_mtrl(fp);
    uniq_vert(fp);
    uniq_edge(fp);
    uniq_side(fp);
    uniq_texc(fp);
    uniq_geom(fp);
}

/*---------------------------------------------------------------------------*/

static void sort_file(struct s_file *fp)
{
    int i, j;

    struct s_geom g;
    struct s_side s;
    struct s_path p;
   
    /*
     * Sort the  geoms in  order of material  index.  This was  originally an
     * important state transition reduction hack, but is now merely cute.
     */
    for (i = 1; i < fp->gc; i++)
        for (j = 0; j < i; j++)
            if (fp->gv[i].mi < fp->gv[j].mi)
            {
                g         = fp->gv[i];
                fp->gv[i] = fp->gv[j];
                fp->gv[j] =         g;

                swap_geom(fp,  i, -1);
                swap_geom(fp,  j,  i);
                swap_geom(fp, -1,  j);
            }

    /*
     * Sort the sides so that the  most "floorish" side is 0.  Then any geoms
     * referring to side 0 may be given special treatment as "floor" geoms.
     */
    for (j = 1; j < fp->sc; j++)
        if (fp->sv[j].n[1]    >= fp->sv[0].n[1] &&
            fabs(fp->sv[j].d) <= fabs(fp->sv[0].d))
        {
            s         = fp->sv[0];
            fp->sv[0] = fp->sv[j];
            fp->sv[j] =         s;

            swap_side(fp,  0, -1);
            swap_side(fp,  j,  0);
            swap_side(fp, -1,  j);
        }

    /*
     * If path_0 is not -1, ensure that the path it refers to is 0.
     */
    if (path_0 > 0 && fp->pc > 0)
    {
        j = path_0;

        p         = fp->pv[0];
        fp->pv[0] = fp->pv[j];
        fp->pv[j] =         p;

        swap_path(fp,  0, -2);
        swap_path(fp,  j,  0);
        swap_path(fp, -2,  j);
    }
}

/*---------------------------------------------------------------------------*/

static void dump_head(void)
{
    printf(" mtrl vert edge side texc geom lump"
           " node path body coin goal ball indx\n");
}

static void dump_file(struct s_file *fp)
{
    printf("%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d\n",
           fp->mc, fp->vc, fp->ec, fp->sc, fp->tc, fp->gc, fp->lc,
           fp->nc, fp->pc, fp->bc, fp->cc, fp->zc, fp->uc, fp->ic);
}

int main(int argc, char *argv[])
{
    struct s_file f;
    FILE *fin;

    if (argc > 2)
    {
        if ((fin = fopen(argv[1], "r")))
        {
            init_file(&f);
            read_map(&f, fin);

            resolve();

            dump_head();
            move_file(&f);
            clip_file(&f);
            dump_file(&f);
            uniq_file(&f);
            sort_file(&f);
            dump_file(&f);

            sol_stor(&f, argv[2]);

            fclose(fin);
        }
    }
    return 0;
}

