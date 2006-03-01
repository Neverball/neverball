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

#include <SDL.h>
#include <SDL_rwops.h>
#include <SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "glext.h"
#include "vec3.h"
#include "geom.h"
#include "image.h"
#include "solid.h"
#include "base_config.h"
#include "binary.h"

#define LARGE 1.0e+5f

/*---------------------------------------------------------------------------*/

static float erp(float t)
{
    return 3.0f * t * t - 2.0f * t * t * t;
}

static float derp(float t)
{
    return 6.0f * t     - 6.0f * t * t;
}

static void sol_body_v(float v[3],
                       const struct s_file *fp,
                       const struct s_body *bp)
{
    if (bp->pi >= 0 && fp->pv[bp->pi].f)
    {
        const struct s_path *pp = fp->pv + bp->pi;
        const struct s_path *pq = fp->pv + pp->pi;

        v_sub(v, pq->p, pp->p);
        v_scl(v, v, 1.0f / pp->t);

        v_scl(v, v, derp(bp->t / pp->t));
    }
    else
    {
        v[0] = 0.0f;
        v[1] = 0.0f;
        v[2] = 0.0f;
    }
}

static void sol_body_p(float p[3],
                       const struct s_file *fp,
                       const struct s_body *bp)
{
    float v[3];

    if (bp->pi >= 0)
    {
        const struct s_path *pp = fp->pv + bp->pi;
        const struct s_path *pq = fp->pv + pp->pi;

        v_sub(v, pq->p, pp->p);
        v_mad(p, pp->p, v, erp(bp->t / pp->t));
    }
    else
    {
        p[0] = 0.0f;
        p[1] = 0.0f;
        p[2] = 0.0f;
    }
}

/*---------------------------------------------------------------------------*/

static int sol_enum_mtrl(const struct s_file *fp,
                         const struct s_body *bp, int mi)
{
    int li, gi, c = 0;

    /* Count all lump geoms with this material. */

    for (li = 0; li < bp->lc; li++)
    {
        int g0 = fp->lv[bp->l0 + li].g0;
        int gc = fp->lv[bp->l0 + li].gc;

        for (gi = 0; gi < gc; gi++)
            if (fp->gv[fp->iv[g0 + gi]].mi == mi)
                c++;
    }
                    
    /* Count all body geoms with this material. */

    for (gi = 0; gi < bp->gc; gi++)
        if (fp->gv[fp->iv[bp->g0 + gi]].mi == mi)
            c++;

    return c;
}

static int sol_enum_body(const struct s_file *fp,
                           const struct s_body *bp, int fl)
{
    int mi, c = 0;

    /* Count all geoms with this flag. */

    for (mi = 0; mi < fp->mc; mi++)
        if (fp->mv[mi].fl & fl)
            c = c + sol_enum_mtrl(fp, bp, mi);

    return c;
}

/*---------------------------------------------------------------------------*/

static void sol_draw_mtrl(const struct s_file *fp, int i)
{
    const struct s_mtrl *mp = fp->mv + i;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   mp->a);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mp->d);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mp->s);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  mp->e);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mp->h);

    if (mp->fl & M_ENVIRONMENT)
    {
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);

        glBindTexture(GL_TEXTURE_2D, mp->o);

        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    }
    else
    {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);

        glBindTexture(GL_TEXTURE_2D, mp->o);
    }

    if (mp->fl & M_ADDITIVE)
        glBlendFunc(GL_ONE, GL_ONE);
    else
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

static void sol_draw_bill(const struct s_file *fp,
                          const struct s_bill *rp, float t)
{
    float T  = fmodf(t, rp->t) - rp->t / 2;

    float w  = rp->w[0] + rp->w[1] * T + rp->w[2] * T * T;
    float h  = rp->h[0] + rp->h[1] * T + rp->h[2] * T * T;

    if (w > 0 && h > 0)
    {
        float rx = rp->rx[0] + rp->rx[1] * T + rp->rx[2] * T * T;
        float ry = rp->ry[0] + rp->ry[1] * T + rp->ry[2] * T * T;
        float rz = rp->rz[0] + rp->rz[1] * T + rp->rz[2] * T * T;

        glPushMatrix();
        {
            float y0 = (rp->fl & B_EDGE) ? 0 : -h / 2;
            float y1 = (rp->fl & B_EDGE) ? h : +h / 2;

            glRotatef(ry, 0.0f, 1.0f, 0.0f);
            glRotatef(rx, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, 0.0f, -rp->d);

            if (rp->fl & B_FLAT)
            {
                glRotatef(-rx - 90.0f, 1.0f, 0.0f, 0.0f);
                glRotatef(-ry,         0.0f, 0.0f, 1.0f);
            }
            if (rp->fl & B_EDGE)
                glRotatef(-rx,         1.0f, 0.0f, 0.0f);

            glRotatef(rz, 0.0f, 0.0f, 1.0f);

            sol_draw_mtrl(fp, rp->mi);

            glBegin(GL_QUADS);
            {
                glTexCoord2f(0.0f, 1.0f); glVertex2f(-w / 2, y0);
                glTexCoord2f(1.0f, 1.0f); glVertex2f(+w / 2, y0);
                glTexCoord2f(1.0f, 0.0f); glVertex2f(+w / 2, y1);
                glTexCoord2f(0.0f, 0.0f); glVertex2f(-w / 2, y1);
            }
            glEnd();
        }
        glPopMatrix();
    }
}

void sol_back(const struct s_file *fp, float n, float f, float t)
{
    int ri;

    glPushAttrib(GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT);
    {
        /* Render all billboards in the given range. */

        glDisable(GL_LIGHTING);
        glDepthMask(GL_FALSE);

        for (ri = 0; ri < fp->rc; ri++)
            if (n <= fp->rv[ri].d && fp->rv[ri].d < f)
                sol_draw_bill(fp, fp->rv + ri, t);
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/
/*
 * The  following code  renders a  body in  a  ludicrously inefficient
 * manner.  It iterates the materials and scans the data structure for
 * geometry using each.  This  has the effect of absolutely minimizing
 * material  changes,  texture  bindings,  and  Begin/End  pairs,  but
 * maximizing trips through the data.
 *
 * However, this  is only done once  for each level.   The results are
 * stored in display lists.  Thus, it is well worth it.
 */

static void sol_draw_geom(const struct s_file *fp,
                          const struct s_geom *gp, int mi)
{
    if (gp->mi == mi)
    {
        const float *ui = fp->tv[gp->ti].u;
        const float *uj = fp->tv[gp->tj].u;
        const float *uk = fp->tv[gp->tk].u;

        const float *ni = fp->sv[gp->si].n;
        const float *nj = fp->sv[gp->sj].n;
        const float *nk = fp->sv[gp->sk].n;

        const float *vi = fp->vv[gp->vi].p;
        const float *vj = fp->vv[gp->vj].p;
        const float *vk = fp->vv[gp->vk].p;

        glTexCoord2fv(ui);
        glNormal3fv(ni);
        glVertex3fv(vi);

        glTexCoord2fv(uj);
        glNormal3fv(nj);
        glVertex3fv(vj);

        glTexCoord2fv(uk);
        glNormal3fv(nk);
        glVertex3fv(vk);
    }
}

static void sol_draw_lump(const struct s_file *fp,
                          const struct s_lump *lp, int mi)
{
    int i;

    for (i = 0; i < lp->gc; i++)
        sol_draw_geom(fp, fp->gv + fp->iv[lp->g0 + i], mi);
}

static void sol_draw_body(const struct s_file *fp,
                          const struct s_body *bp, int fl)
{
    int mi, li, gi;

    /* Iterate all materials of the correct opacity. */

    for (mi = 0; mi < fp->mc; mi++)
        if (fp->mv[mi].fl & fl)
        {
            if (sol_enum_mtrl(fp, bp, mi))
            {
                /* Set the material state. */

                sol_draw_mtrl(fp, mi);

                /* Render all geometry of that material. */

                glBegin(GL_TRIANGLES);
                {
                    for (li = 0; li < bp->lc; li++)
                        sol_draw_lump(fp, fp->lv + bp->l0 + li, mi);
                    for (gi = 0; gi < bp->gc; gi++)
                        sol_draw_geom(fp, fp->gv + fp->iv[bp->g0 + gi], mi);
                }
                glEnd();
            }
        }
}

static void sol_draw_list(const struct s_file *fp,
                          const struct s_body *bp, GLuint list)
{
    float p[3];

    sol_body_p(p, fp, bp);

    glPushMatrix();
    {
        /* Translate a moving body. */

        glTranslatef(p[0], p[1], p[2]);

        /* Draw the body. */

        glCallList(list);
    }
    glPopMatrix();
}

void sol_draw(const struct s_file *fp)
{
    int bi;

    glPushAttrib(GL_TEXTURE_BIT      |
                 GL_LIGHTING_BIT     |
                 GL_COLOR_BUFFER_BIT |
                 GL_DEPTH_BUFFER_BIT);
    {
        /* Render all obaque geometry into the color and depth buffers. */

        for (bi = 0; bi < fp->bc; bi++)
            if (fp->bv[bi].ol)
                sol_draw_list(fp, fp->bv + bi, fp->bv[bi].ol);

        /* Render all translucent geometry into only the color buffer. */

        glDepthMask(GL_FALSE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (bi = 0; bi < fp->bc; bi++)
            if (fp->bv[bi].tl)
                sol_draw_list(fp, fp->bv + bi, fp->bv[bi].tl);
    }
    glPopAttrib();
}

void sol_refl(const struct s_file *fp)
{
    int bi;

    glPushAttrib(GL_LIGHTING_BIT);
    {
        /* Render all reflective geometry into the color and depth buffers. */

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (bi = 0; bi < fp->bc; bi++)
            if (fp->bv[bi].rl)
                sol_draw_list(fp, fp->bv + bi, fp->bv[bi].rl);
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

static void sol_shad_geom(const struct s_file *fp,
                          const struct s_geom *gp, int mi)
{
    if (gp->mi == mi)
    {
        const float *vi = fp->vv[gp->vi].p;
        const float *vj = fp->vv[gp->vj].p;
        const float *vk = fp->vv[gp->vk].p;

        glTexCoord2f(vi[0], vi[2]);
        glVertex3fv(vi);

        glTexCoord2f(vj[0], vj[2]);
        glVertex3fv(vj);

        glTexCoord2f(vk[0], vk[2]);
        glVertex3fv(vk);
    }
}

static void sol_shad_lump(const struct s_file *fp,
                          const struct s_lump *lp, int mi)
{
    int i;

    for (i = 0; i < lp->gc; i++)
        sol_shad_geom(fp, fp->gv + fp->iv[lp->g0 + i], mi);
}

static void sol_shad_body(const struct s_file *fp,
                          const struct s_body *bp, int fl)
{
    int mi, li, gi;

    glBegin(GL_TRIANGLES);
    {
        for (mi = 0; mi < fp->mc; mi++)
            if (fp->mv[mi].fl & fl)
            {
                for (li = 0; li < bp->lc; li++)
                    sol_shad_lump(fp, fp->lv + bp->l0 + li, mi);
                for (gi = 0; gi < bp->gc; gi++)
                    sol_shad_geom(fp, fp->gv + fp->iv[bp->g0 + gi], mi);
            }
    }
    glEnd();
}

static void sol_shad_list(const struct s_file *fp,
                          const struct s_body *bp, GLuint list)
{
    float p[3];

    sol_body_p(p, fp, bp);

    glPushMatrix();
    {
        /* Translate a moving body. */

        glTranslatef(p[0], p[1], p[2]);

        /* Translate the shadow on a moving body. */

        glMatrixMode(GL_TEXTURE);
        {
            glPushMatrix();
            glTranslatef(p[0], p[2], 0.0f);
        }
        glMatrixMode(GL_MODELVIEW);
        
        /* Draw the body. */

        glCallList(list);

        /* Pop the shadow translation. */

        glMatrixMode(GL_TEXTURE);
        {
            glPopMatrix();
        }
        glMatrixMode(GL_MODELVIEW);
    }
    glPopMatrix();
}

void sol_shad(const struct s_file *fp)
{
    int bi;

    glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT);
    {
        /* Render all shadowed geometry. */

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);

        for (bi = 0; bi < fp->bc; bi++)
            if (fp->bv[bi].sl)
                sol_shad_list(fp, fp->bv + bi, fp->bv[bi].sl);
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

static void sol_load_objects(struct s_file *fp, int s)
{
    int i;

    for (i = 0; i < fp->bc; i++)
    {
        struct s_body *bp = fp->bv + i;

        /* Draw all opaque geometry. */

        if (sol_enum_body(fp, bp, M_OPAQUE | M_ENVIRONMENT))
        {
            fp->bv[i].ol = glGenLists(1);
            
            glNewList(fp->bv[i].ol, GL_COMPILE);
            {
                sol_draw_body(fp, fp->bv + i, M_OPAQUE | M_ENVIRONMENT);
            }
            glEndList();
        }
        else fp->bv[i].ol = 0;

        /* Draw all translucent geometry. */

        if (sol_enum_body(fp, bp, M_TRANSPARENT))
        {
            fp->bv[i].tl = glGenLists(1);

            glNewList(fp->bv[i].tl, GL_COMPILE);
            {
                sol_draw_body(fp, fp->bv + i, M_TRANSPARENT);
            }
            glEndList();
        }
        else fp->bv[i].tl = 0;

        /* Draw all reflective geometry. */

        if (sol_enum_body(fp, bp, M_REFLECTIVE))
        {
            fp->bv[i].rl = glGenLists(1);

            glNewList(fp->bv[i].rl, GL_COMPILE);
            {
                sol_draw_body(fp, fp->bv + i, M_REFLECTIVE);
            }
            glEndList();
        }
        else fp->bv[i].rl = 0;

        /* Draw all shadowed geometry. */

        if (s && sol_enum_body(fp, bp, M_SHADOWED))
        {
            fp->bv[i].sl = glGenLists(1);

            glNewList(fp->bv[i].sl, GL_COMPILE);
            {
                sol_shad_body(fp, fp->bv + i, M_SHADOWED);
            }
            glEndList();
        }
        else fp->bv[i].sl = 0;
    }
}

static SDL_Surface *sol_find_texture(const char *name)
{
    char png[MAXSTR];
    char tga[MAXSTR];
    char jpg[MAXSTR];
    SDL_Surface *s;

    /* Prefer a lossless copy of the texture over a lossy compression. */

    strncpy(png, name, PATHMAX); strcat(png, ".png");
    strncpy(tga, name, PATHMAX); strcat(tga, ".tga");
    strncpy(jpg, name, PATHMAX); strcat(jpg, ".jpg");

    /* Check for a PNG. */

    if ((s = IMG_Load(config_data(png))))
        return s;

    /* Check for a TGA, swapping channels if found. */

    if ((s = IMG_Load(config_data(tga))))
    {
        image_swab(s);
        return s;
    }

    /* Check for a JPG. */

    if ((s = IMG_Load(config_data(jpg))))
        return s;

    return NULL;
}

static void sol_load_textures(struct s_file *fp, int k)
{
    SDL_Surface *s;
    SDL_Surface *d;

    int i;

    for (i = 0; i < fp->mc; i++)
        if ((s = sol_find_texture(fp->mv[i].f)))
        {
            GLenum f = (s->format->BitsPerPixel == 32) ? GL_RGBA : GL_RGB;

            glGenTextures(1, &fp->mv[i].o);
            glBindTexture(GL_TEXTURE_2D, fp->mv[i].o);

            if (k > 1)
            {
                /* Create a new buffer and copy the scaled image to it. */

                if ((d = image_scale(s, k)))
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, f, d->w, d->h, 0, f,
                                 GL_UNSIGNED_BYTE, d->pixels);
                    SDL_FreeSurface(d);
                }
            }
            else
                glTexImage2D(GL_TEXTURE_2D, 0, f, s->w, s->h, 0, f,
                             GL_UNSIGNED_BYTE, s->pixels);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            /* Set the texture to clamp or repeat based on material type. */

            if (fp->mv[i].fl & M_CLAMPED)
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            }
            else
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }

            SDL_FreeSurface(s);
        }
}

/*---------------------------------------------------------------------------*/

static void sol_load_mtrl(FILE *fin, struct s_mtrl *mp)
{
    get_array(fin,  mp->a, 4);
    get_array(fin,  mp->d, 4);
    get_array(fin,  mp->s, 4);
    get_array(fin,  mp->e, 4);
    get_array(fin,  mp->h, 1);
    get_index(fin, &mp->fl);

    fread(mp->f, 1, PATHMAX, fin);
}

static void sol_load_vert(FILE *fin, struct s_vert *vp)
{
    get_array(fin,  vp->p, 3);
}

static void sol_load_edge(FILE *fin, struct s_edge *ep)
{
    get_index(fin, &ep->vi);
    get_index(fin, &ep->vj);
}

static void sol_load_side(FILE *fin, struct s_side *sp)
{
    get_array(fin,  sp->n, 3);
    get_float(fin, &sp->d);
}

static void sol_load_texc(FILE *fin, struct s_texc *tp)
{
    get_array(fin,  tp->u, 2);
}

static void sol_load_geom(FILE *fin, struct s_geom *gp)
{
    get_index(fin, &gp->mi);
    get_index(fin, &gp->ti);
    get_index(fin, &gp->si);
    get_index(fin, &gp->vi);
    get_index(fin, &gp->tj);
    get_index(fin, &gp->sj);
    get_index(fin, &gp->vj);
    get_index(fin, &gp->tk);
    get_index(fin, &gp->sk);
    get_index(fin, &gp->vk);
}

static void sol_load_lump(FILE *fin, struct s_lump *lp)
{
    get_index(fin, &lp->fl);
    get_index(fin, &lp->v0);
    get_index(fin, &lp->vc);
    get_index(fin, &lp->e0);
    get_index(fin, &lp->ec);
    get_index(fin, &lp->g0);
    get_index(fin, &lp->gc);
    get_index(fin, &lp->s0);
    get_index(fin, &lp->sc);
}

static void sol_load_node(FILE *fin, struct s_node *np)
{
    get_index(fin, &np->si);
    get_index(fin, &np->ni);
    get_index(fin, &np->nj);
    get_index(fin, &np->l0);
    get_index(fin, &np->lc);
}

static void sol_load_path(FILE *fin, struct s_path *pp)
{
    get_array(fin,  pp->p, 3);
    get_float(fin, &pp->t);
    get_index(fin, &pp->pi);
    get_index(fin, &pp->f);
}

static void sol_load_body(FILE *fin, struct s_body *bp)
{
    get_index(fin, &bp->pi);
    get_index(fin, &bp->ni);
    get_index(fin, &bp->l0);
    get_index(fin, &bp->lc);
    get_index(fin, &bp->g0);
    get_index(fin, &bp->gc);
}

static void sol_load_coin(FILE *fin, struct s_coin *cp)
{
    get_array(fin,  cp->p, 3);
    get_index(fin, &cp->n);
}

static void sol_load_goal(FILE *fin, struct s_goal *zp)
{
    get_array(fin,  zp->p, 3);
    get_float(fin, &zp->r);
}

static void sol_load_swch(FILE *fin, struct s_swch *xp)
{
    get_array(fin,  xp->p, 3);
    get_float(fin, &xp->r);
    get_index(fin, &xp->pi);
    get_float(fin, &xp->t0);
    get_float(fin, &xp->t);
    get_index(fin, &xp->f0);
    get_index(fin, &xp->f);
}

static void sol_load_bill(FILE *fin, struct s_bill *rp)
{
    get_index(fin, &rp->fl);
    get_index(fin, &rp->mi);
    get_float(fin, &rp->t);
    get_float(fin, &rp->d);
    get_array(fin,  rp->w,  3);
    get_array(fin,  rp->h,  3);
    get_array(fin,  rp->rx, 3);
    get_array(fin,  rp->ry, 3);
    get_array(fin,  rp->rz, 3);
}

static void sol_load_jump(FILE *fin, struct s_jump *jp)
{
    get_array(fin,  jp->p, 3);
    get_array(fin,  jp->q, 3);
    get_float(fin, &jp->r);
}

static void sol_load_ball(FILE *fin, struct s_ball *bp)
{
    get_array(fin,  bp->e[0], 3);
    get_array(fin,  bp->e[1], 3);
    get_array(fin,  bp->e[2], 3);
    get_array(fin,  bp->p,    3);
    get_float(fin, &bp->r);
}

static void sol_load_view(FILE *fin, struct s_view *wp)
{
    get_array(fin,  wp->p, 3);
    get_array(fin,  wp->q, 3);
}

static void sol_load_file(FILE *fin, struct s_file *fp)
{
    int i;

    get_index(fin, &fp->mc);
    get_index(fin, &fp->vc);
    get_index(fin, &fp->ec);
    get_index(fin, &fp->sc);
    get_index(fin, &fp->tc);
    get_index(fin, &fp->gc);
    get_index(fin, &fp->lc);
    get_index(fin, &fp->nc);
    get_index(fin, &fp->pc);
    get_index(fin, &fp->bc);
    get_index(fin, &fp->cc);
    get_index(fin, &fp->zc);
    get_index(fin, &fp->jc);
    get_index(fin, &fp->xc);
    get_index(fin, &fp->rc);
    get_index(fin, &fp->uc);
    get_index(fin, &fp->wc);
    get_index(fin, &fp->ic);
    get_index(fin, &fp->ac);

    if (fp->mc)
        fp->mv = (struct s_mtrl *) calloc(fp->mc, sizeof (struct s_mtrl));
    if (fp->vc)
        fp->vv = (struct s_vert *) calloc(fp->vc, sizeof (struct s_vert));
    if (fp->ec)
        fp->ev = (struct s_edge *) calloc(fp->ec, sizeof (struct s_edge));
    if (fp->sc)
        fp->sv = (struct s_side *) calloc(fp->sc, sizeof (struct s_side));
    if (fp->tc)
        fp->tv = (struct s_texc *) calloc(fp->tc, sizeof (struct s_texc));
    if (fp->gc)
        fp->gv = (struct s_geom *) calloc(fp->gc, sizeof (struct s_geom));
    if (fp->lc)
        fp->lv = (struct s_lump *) calloc(fp->lc, sizeof (struct s_lump));
    if (fp->nc)
        fp->nv = (struct s_node *) calloc(fp->nc, sizeof (struct s_node));
    if (fp->pc)
        fp->pv = (struct s_path *) calloc(fp->pc, sizeof (struct s_path));
    if (fp->bc)
        fp->bv = (struct s_body *) calloc(fp->bc, sizeof (struct s_body));
    if (fp->cc)
        fp->cv = (struct s_coin *) calloc(fp->cc, sizeof (struct s_coin));
    if (fp->zc)
        fp->zv = (struct s_goal *) calloc(fp->zc, sizeof (struct s_goal));
    if (fp->jc)
        fp->jv = (struct s_jump *) calloc(fp->jc, sizeof (struct s_jump));
    if (fp->xc)
        fp->xv = (struct s_swch *) calloc(fp->xc, sizeof (struct s_swch));
    if (fp->rc)
        fp->rv = (struct s_bill *) calloc(fp->rc, sizeof (struct s_bill));
    if (fp->uc)
        fp->uv = (struct s_ball *) calloc(fp->uc, sizeof (struct s_ball));
    if (fp->wc)
        fp->wv = (struct s_view *) calloc(fp->wc, sizeof (struct s_view));
    if (fp->ic)
        fp->iv = (int           *) calloc(fp->ic, sizeof (int));
    if (fp->ac)
        fp->av = (char          *) calloc(fp->ac, sizeof (char));

    for (i = 0; i < fp->mc; i++) sol_load_mtrl(fin, fp->mv + i);
    for (i = 0; i < fp->vc; i++) sol_load_vert(fin, fp->vv + i);
    for (i = 0; i < fp->ec; i++) sol_load_edge(fin, fp->ev + i);
    for (i = 0; i < fp->sc; i++) sol_load_side(fin, fp->sv + i);
    for (i = 0; i < fp->tc; i++) sol_load_texc(fin, fp->tv + i);
    for (i = 0; i < fp->gc; i++) sol_load_geom(fin, fp->gv + i);
    for (i = 0; i < fp->lc; i++) sol_load_lump(fin, fp->lv + i);
    for (i = 0; i < fp->nc; i++) sol_load_node(fin, fp->nv + i);
    for (i = 0; i < fp->pc; i++) sol_load_path(fin, fp->pv + i);
    for (i = 0; i < fp->bc; i++) sol_load_body(fin, fp->bv + i);
    for (i = 0; i < fp->cc; i++) sol_load_coin(fin, fp->cv + i);
    for (i = 0; i < fp->zc; i++) sol_load_goal(fin, fp->zv + i);
    for (i = 0; i < fp->jc; i++) sol_load_jump(fin, fp->jv + i);
    for (i = 0; i < fp->xc; i++) sol_load_swch(fin, fp->xv + i);
    for (i = 0; i < fp->rc; i++) sol_load_bill(fin, fp->rv + i);
    for (i = 0; i < fp->uc; i++) sol_load_ball(fin, fp->uv + i);
    for (i = 0; i < fp->wc; i++) sol_load_view(fin, fp->wv + i);
    for (i = 0; i < fp->ic; i++) get_index(fin, fp->iv + i);

    if (fp->ac) fread(fp->av, 1, fp->ac, fin);
}

int sol_load(struct s_file *fp, const char *filename, int k, int s)
{
    FILE *fin;

    if ((fin = fopen(filename, FMODE_RB)))
    {
        sol_load_file(fin, fp);
        sol_load_textures(fp, k);
        sol_load_objects (fp, s);

        fclose(fin);

        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static void sol_stor_mtrl(FILE *fout, struct s_mtrl *mp)
{
    put_array(fout,  mp->a, 4);
    put_array(fout,  mp->d, 4);
    put_array(fout,  mp->s, 4);
    put_array(fout,  mp->e, 4);
    put_array(fout,  mp->h, 1);
    put_index(fout, &mp->fl);

    fwrite(mp->f, 1, PATHMAX, fout);
}

static void sol_stor_vert(FILE *fout, struct s_vert *vp)
{
    put_array(fout,  vp->p, 3);
}

static void sol_stor_edge(FILE *fout, struct s_edge *ep)
{
    put_index(fout, &ep->vi);
    put_index(fout, &ep->vj);
}

static void sol_stor_side(FILE *fout, struct s_side *sp)
{
    put_array(fout,  sp->n, 3);
    put_float(fout, &sp->d);
}

static void sol_stor_texc(FILE *fout, struct s_texc *tp)
{
    put_array(fout,  tp->u, 2);
}

static void sol_stor_geom(FILE *fout, struct s_geom *gp)
{
    put_index(fout, &gp->mi);
    put_index(fout, &gp->ti);
    put_index(fout, &gp->si);
    put_index(fout, &gp->vi);
    put_index(fout, &gp->tj);
    put_index(fout, &gp->sj);
    put_index(fout, &gp->vj);
    put_index(fout, &gp->tk);
    put_index(fout, &gp->sk);
    put_index(fout, &gp->vk);
}

static void sol_stor_lump(FILE *fout, struct s_lump *lp)
{
    put_index(fout, &lp->fl);
    put_index(fout, &lp->v0);
    put_index(fout, &lp->vc);
    put_index(fout, &lp->e0);
    put_index(fout, &lp->ec);
    put_index(fout, &lp->g0);
    put_index(fout, &lp->gc);
    put_index(fout, &lp->s0);
    put_index(fout, &lp->sc);
}

static void sol_stor_node(FILE *fout, struct s_node *np)
{
    put_index(fout, &np->si);
    put_index(fout, &np->ni);
    put_index(fout, &np->nj);
    put_index(fout, &np->l0);
    put_index(fout, &np->lc);
}

static void sol_stor_path(FILE *fout, struct s_path *pp)
{
    put_array(fout,  pp->p, 3);
    put_float(fout, &pp->t);
    put_index(fout, &pp->pi);
    put_index(fout, &pp->f);
}

static void sol_stor_body(FILE *fout, struct s_body *bp)
{
    put_index(fout, &bp->pi);
    put_index(fout, &bp->ni);
    put_index(fout, &bp->l0);
    put_index(fout, &bp->lc);
    put_index(fout, &bp->g0);
    put_index(fout, &bp->gc);
}

static void sol_stor_coin(FILE *fout, struct s_coin *cp)
{
    put_array(fout,  cp->p, 3);
    put_index(fout, &cp->n);
}

static void sol_stor_goal(FILE *fout, struct s_goal *zp)
{
    put_array(fout,  zp->p, 3);
    put_float(fout, &zp->r);
}

static void sol_stor_swch(FILE *fout, struct s_swch *xp)
{
    put_array(fout,  xp->p, 3);
    put_float(fout, &xp->r);
    put_index(fout, &xp->pi);
    put_float(fout, &xp->t0);
    put_float(fout, &xp->t);
    put_index(fout, &xp->f0);
    put_index(fout, &xp->f);
}

static void sol_stor_bill(FILE *fout, struct s_bill *rp)
{
    put_index(fout, &rp->fl);
    put_index(fout, &rp->mi);
    put_float(fout, &rp->t);
    put_float(fout, &rp->d);
    put_array(fout,  rp->w,  3);
    put_array(fout,  rp->h,  3);
    put_array(fout,  rp->rx, 3);
    put_array(fout,  rp->ry, 3);
    put_array(fout,  rp->rz, 3);
}

static void sol_stor_jump(FILE *fout, struct s_jump *jp)
{
    put_array(fout,  jp->p, 3);
    put_array(fout,  jp->q, 3);
    put_float(fout, &jp->r);
}

static void sol_stor_ball(FILE *fout, struct s_ball *bp)
{
    put_array(fout,  bp->e[0], 3);
    put_array(fout,  bp->e[1], 3);
    put_array(fout,  bp->e[2], 3);
    put_array(fout,  bp->p,    3);
    put_float(fout, &bp->r);
}

static void sol_stor_view(FILE *fout, struct s_view *wp)
{
    put_array(fout,  wp->p, 3);
    put_array(fout,  wp->q, 3);
}

static void sol_stor_file(FILE *fin, struct s_file *fp)
{
    int i;

    put_index(fin, &fp->mc);
    put_index(fin, &fp->vc);
    put_index(fin, &fp->ec);
    put_index(fin, &fp->sc);
    put_index(fin, &fp->tc);
    put_index(fin, &fp->gc);
    put_index(fin, &fp->lc);
    put_index(fin, &fp->nc);
    put_index(fin, &fp->pc);
    put_index(fin, &fp->bc);
    put_index(fin, &fp->cc);
    put_index(fin, &fp->zc);
    put_index(fin, &fp->jc);
    put_index(fin, &fp->xc);
    put_index(fin, &fp->rc);
    put_index(fin, &fp->uc);
    put_index(fin, &fp->wc);
    put_index(fin, &fp->ic);
    put_index(fin, &fp->ac);

    for (i = 0; i < fp->mc; i++) sol_stor_mtrl(fin, fp->mv + i);
    for (i = 0; i < fp->vc; i++) sol_stor_vert(fin, fp->vv + i);
    for (i = 0; i < fp->ec; i++) sol_stor_edge(fin, fp->ev + i);
    for (i = 0; i < fp->sc; i++) sol_stor_side(fin, fp->sv + i);
    for (i = 0; i < fp->tc; i++) sol_stor_texc(fin, fp->tv + i);
    for (i = 0; i < fp->gc; i++) sol_stor_geom(fin, fp->gv + i);
    for (i = 0; i < fp->lc; i++) sol_stor_lump(fin, fp->lv + i);
    for (i = 0; i < fp->nc; i++) sol_stor_node(fin, fp->nv + i);
    for (i = 0; i < fp->pc; i++) sol_stor_path(fin, fp->pv + i);
    for (i = 0; i < fp->bc; i++) sol_stor_body(fin, fp->bv + i);
    for (i = 0; i < fp->cc; i++) sol_stor_coin(fin, fp->cv + i);
    for (i = 0; i < fp->zc; i++) sol_stor_goal(fin, fp->zv + i);
    for (i = 0; i < fp->jc; i++) sol_stor_jump(fin, fp->jv + i);
    for (i = 0; i < fp->xc; i++) sol_stor_swch(fin, fp->xv + i);
    for (i = 0; i < fp->rc; i++) sol_stor_bill(fin, fp->rv + i);
    for (i = 0; i < fp->uc; i++) sol_stor_ball(fin, fp->uv + i);
    for (i = 0; i < fp->wc; i++) sol_stor_view(fin, fp->wv + i);
    for (i = 0; i < fp->ic; i++) put_index(fin, fp->iv + i);

    fwrite(fp->av, 1, fp->ac, fin);
}

/*---------------------------------------------------------------------------*/

int sol_stor(struct s_file *fp, const char *filename)
{
    FILE *fout;

    if ((fout = fopen(filename, FMODE_WB)))
    {
        sol_stor_file(fout, fp);
        fclose(fout);

        return 1;
    }
    return 0;
}

void sol_free(struct s_file *fp)
{
    int i;

    for (i = 0; i < fp->mc; i++)
    {
        if (glIsTexture(fp->mv[i].o))
            glDeleteTextures(1, &fp->mv[i].o);
    }

    for (i = 0; i < fp->bc; i++)
    {
        if (glIsList(fp->bv[i].ol))
            glDeleteLists(fp->bv[i].ol, 1);
        if (glIsList(fp->bv[i].tl))
            glDeleteLists(fp->bv[i].tl, 1);
        if (glIsList(fp->bv[i].rl))
            glDeleteLists(fp->bv[i].rl, 1);
    }

    if (fp->mv) free(fp->mv);
    if (fp->vv) free(fp->vv);
    if (fp->ev) free(fp->ev);
    if (fp->sv) free(fp->sv);
    if (fp->tv) free(fp->tv);
    if (fp->gv) free(fp->gv);
    if (fp->lv) free(fp->lv);
    if (fp->nv) free(fp->nv);
    if (fp->pv) free(fp->pv);
    if (fp->bv) free(fp->bv);
    if (fp->cv) free(fp->cv);
    if (fp->zv) free(fp->zv);
    if (fp->jv) free(fp->jv);
    if (fp->xv) free(fp->xv);
    if (fp->rv) free(fp->rv);
    if (fp->uv) free(fp->uv);
    if (fp->wv) free(fp->wv);
    if (fp->av) free(fp->av);
    if (fp->iv) free(fp->iv);

    memset(fp, 0, sizeof (struct s_file));
}

/*---------------------------------------------------------------------------*/
/* Solves (p + v * t) . (p + v * t) == r * r for smallest t.                 */

static float v_sol(const float p[3], const float v[3], float r)
{
    float a = v_dot(v, v);
    float b = v_dot(v, p) * 2.0f;
    float c = v_dot(p, p) - r * r;
    float d = b * b - 4.0f * a * c;

    if (a == 0.0f) return LARGE;
    if (d <  0.0f) return LARGE;

    if (d == 0.0f)
        return -b * 0.5f / a;
    else
    {
        float t0 = 0.5f * (-b - fsqrtf(d)) / a;
        float t1 = 0.5f * (-b + fsqrtf(d)) / a;
        float t  = (t0 < t1) ? t0 : t1;

        return (t < 0.0f) ? LARGE : t;
    }
}

/*---------------------------------------------------------------------------*/

/*
 * Compute the  earliest time  and position of  the intersection  of a
 * sphere and a vertex.
 *
 * The sphere has radius R and moves along vector V from point P.  The
 * vertex moves  along vector  W from point  Q in a  coordinate system
 * based at O.
 */
static float v_vert(float Q[3],
                    const float o[3],
                    const float q[3],
                    const float w[3],
                    const float p[3],
                    const float v[3], float r)
{
    float O[3], P[3], V[3];
    float t = LARGE;

    v_add(O, o, q);
    v_sub(P, p, O);
    v_sub(V, v, w);

    if (v_dot(P, V) < 0.0f)
    {
        t = v_sol(P, V, r);

        if (t < LARGE)
            v_mad(Q, O, w, t);
    }
    return t;
}

/*
 * Compute the  earliest time  and position of  the intersection  of a
 * sphere and an edge.
 *
 * The sphere has radius R and moves along vector V from point P.  The
 * edge moves along vector W from point Q in a coordinate system based
 * at O.  The edge extends along the length of vector U.
 */
static float v_edge(float Q[3],
                    const float o[3],
                    const float q[3],
                    const float u[3],
                    const float w[3],
                    const float p[3],
                    const float v[3], float r)
{
    float d[3], e[3];
    float P[3], V[3];
    float du, eu, uu, s, t;

    v_sub(d, p, o);
    v_sub(d, d, q);
    v_sub(e, v, w);

    du = v_dot(d, u);
    eu = v_dot(e, u);
    uu = v_dot(u, u);

    v_mad(P, d, u, -du / uu);
    v_mad(V, e, u, -eu / uu);

    t = v_sol(P, V, r);
    s = (du + eu * t) / uu;

    if (0.0f < t && t < LARGE && 0.0f < s && s < 1.0f)
    {
        v_mad(d, o, w, t);
        v_mad(e, q, u, s);
        v_add(Q, e, d);
    }
    else
        t = LARGE;

    return t;
}

/*
 * Compute  the earlist  time and  position of  the intersection  of a
 * sphere and a plane.
 *
 * The sphere has radius R and moves along vector V from point P.  The
 * plane  oves  along  vector  W.   The  plane has  normal  N  and  is
 * positioned at distance D from the origin O along that normal.
 */
static float v_side(float Q[3],
                    const float o[3],
                    const float w[3],
                    const float n[3], float d,
                    const float p[3],
                    const float v[3], float r)
{
    float vn = v_dot(v, n);
    float wn = v_dot(w, n);
    float t  = LARGE;

    if (vn - wn <= 0.0f)
    {
        float on = v_dot(o, n);
        float pn = v_dot(p, n);

        float u = (r + d + on - pn) / (vn - wn);
        float a = (    d + on - pn) / (vn - wn);

        if (0.0f <= u)
        {
            t = u;

            v_mad(Q, p, v, +t);
            v_mad(Q, Q, n, -r);
        }
        else if (0.0f <= a)
        {
            t = 0;

            v_mad(Q, p, v, +t);
            v_mad(Q, Q, n, -r);
        }
    }
    return t;
}

/*---------------------------------------------------------------------------*/

/*
 * Compute the new  linear and angular velocities of  a bouncing ball.
 * Q  gives the  position  of the  point  of impact  and  W gives  the
 * velocity of the object being impacted.
 */
static float sol_bounce(struct s_ball *up,
                        const float q[3],
                        const float w[3], float dt)
{
    const float kb = 1.10f;
    const float ke = 0.70f;
    const float km = 0.20f;

    float n[3], r[3], d[3], u[3], vn, wn, xn, yn;
    float *p = up->p;
    float *v = up->v;

    /* Find the normal of the impact. */

    v_sub(r, p, q);
    v_sub(d, v, w);
    v_nrm(n, r);

    /* Find the new angular velocity. */

    v_crs(up->w, d, r);
    v_scl(up->w, up->w, -1.0f / (up->r * up->r));

    /* Find the new linear velocity. */

    vn = v_dot(v, n);
    wn = v_dot(w, n);
    xn = (vn < 0.0f) ? -vn * ke : vn;
    yn = (wn > 0.0f) ?  wn * kb : wn;

    v_mad(u, w, n, -wn);
    v_mad(v, v, n, -vn);
    v_mad(v, v, u, +km * dt);
    v_mad(v, v, n, xn + yn); 

    v_mad(p, q, n, up->r);

    /* Return the "energy" of the impact, to determine the sound amplitude. */

    return fabsf(v_dot(n, d));
}

/*---------------------------------------------------------------------------*/

/*
 * Compute the states of all switches after DT seconds have passed.
 */
static void sol_swch_step(struct s_file *fp, float dt)
{
    int xi;

    for (xi = 0; xi < fp->xc; xi++)
    {
        struct s_swch *xp = fp->xv + xi;

        if (xp->t > 0)
        {
            xp->t -= dt;

            if (xp->t <= 0)
            {
                int pi = xp->pi;
                int pj = xp->pi;

                do  /* Tortoise and hare cycle traverser. */
                {
                    fp->pv[pi].f = xp->f0;
                    fp->pv[pj].f = xp->f0;

                    pi = fp->pv[pi].pi;
                    pj = fp->pv[pj].pi;
                    pj = fp->pv[pj].pi;
                }
                while (pi != pj);

                xp->f = xp->f0;
            }
        }
    }
}

/*
 * Compute the positions of all bodies after DT seconds have passed.
 */
static void sol_body_step(struct s_file *fp, float dt)
{
    int i;

    for (i = 0; i < fp->bc; i++)
    {
        struct s_body *bp = fp->bv + i;
        struct s_path *pp = fp->pv + bp->pi;

        if (bp->pi >= 0 && pp->f)
        {
            bp->t += dt;

            if (bp->t >= pp->t)
            {
                bp->t -= pp->t;
                bp->pi = pp->pi;
            }
        }
    }
}

/*
 * Compute the positions of all balls after DT seconds have passed.
 */
static void sol_ball_step(struct s_file *fp, float dt)
{
    int i;

    for (i = 0; i < fp->uc; i++)
    {
        struct s_ball *up = fp->uv + i;

        v_mad(up->p, up->p, up->v, dt);

        if (v_len(up->w) > 0.0f)
        {
            float M[16];
            float w[3];
            float e[3][3];

            v_nrm(w, up->w);
            m_rot(M, w, v_len(up->w) * dt);

            m_vxfm(e[0], M, up->e[0]);
            m_vxfm(e[1], M, up->e[1]);
            m_vxfm(e[2], M, up->e[2]);

            v_crs(up->e[2], e[0], e[1]);
            v_crs(up->e[1], e[2], e[0]);
            v_crs(up->e[0], e[1], e[2]);

            v_nrm(up->e[0], up->e[0]);
            v_nrm(up->e[1], up->e[1]);
            v_nrm(up->e[2], up->e[2]);
        }
    }
}

/*---------------------------------------------------------------------------*/

static float sol_test_vert(float dt,
                           float T[3],
                           const struct s_ball *up,
                           const struct s_vert *vp,
                           const float o[3],
                           const float w[3])
{
    return v_vert(T, o, vp->p, w, up->p, up->v, up->r);
}

static float sol_test_edge(float dt,
                           float T[3],
                           const struct s_ball *up,
                           const struct s_file *fp,
                           const struct s_edge *ep,
                           const float o[3],
                           const float w[3])
{
    float q[3];
    float u[3];

    v_cpy(q, fp->vv[ep->vi].p);
    v_sub(u, fp->vv[ep->vj].p,
          fp->vv[ep->vi].p);

    return v_edge(T, o, q, u, w, up->p, up->v, up->r);
}

static float sol_test_side(float dt,
                           float T[3],
                           const struct s_ball *up,
                           const struct s_file *fp,
                           const struct s_lump *lp,
                           const struct s_side *sp,
                           const float o[3],
                           const float w[3])
{
    float t = v_side(T, o, w, sp->n, sp->d, up->p, up->v, up->r);
    int i;

    if (t < dt)
        for (i = 0; i < lp->sc; i++)
        {
            const struct s_side *sq = fp->sv + fp->iv[lp->s0 + i];

            if (sp != sq &&
                v_dot(T, sq->n) -
                v_dot(o, sq->n) -
                v_dot(w, sq->n) * t > sq->d)
                return LARGE;
        }
    return t;
}

/*---------------------------------------------------------------------------*/

static float sol_test_fore(float dt,
                           const struct s_ball *up,
                           const struct s_side *sp,
                           const float o[3],
                           const float w[3])
{
    float q[3];

    /* If the ball is not behind the plane, the test passes. */

    v_sub(q, up->p, o);

    if (v_dot(q, sp->n) - sp->d + up->r >= 0)
        return 1;

    /* if the ball is behind the plane but will hit before dt, test passes. */
    /*
      if (v_side(q, o, w, sp->n, sp->d, up->p, up->v, up->r) < dt)
      return 1;
    */
    /* If the ball is behind but moving toward the plane, test passes. */

    if (v_dot(up->v, sp->n) > 0)
        return 1;


    /* Else, test fails. */

    return 0;
}

static float sol_test_back(float dt,
                           const struct s_ball *up,
                           const struct s_side *sp,
                           const float o[3],
                           const float w[3])
{
    float q[3];

    /* If the ball is not in front of the plane, the test passes. */

    v_sub(q, up->p, o);

    if (v_dot(q, sp->n) - sp->d - up->r <= 0)
        return 1;

    /* if the ball is behind the plane but will hit before dt, test passes. */
    /*
      if (v_side(q, o, w, sp->n, sp->d, up->p, up->v, up->r) < dt)
      return 1;
    */
    /* If the ball is in front but moving toward the plane, test passes. */

    if (v_dot(up->v, sp->n) < 0)
        return 1;


    /* Else, test fails. */

    return 0;
}

/*---------------------------------------------------------------------------*/

static float sol_test_lump(float dt,
                           float T[3],
                           const struct s_ball *up,
                           const struct s_file *fp,
                           const struct s_lump *lp,
                           const float o[3],
                           const float w[3])
{
    float U[3], u, t = dt;
    int i;

    /* Short circuit a non-solid lump. */

    if (lp->fl & L_DETAIL) return t;

    /* Test all verts */

    if (up->r > 0.0f)
        for (i = 0; i < lp->vc; i++)
        {
            const struct s_vert *vp = fp->vv + fp->iv[lp->v0 + i];

            if ((u = sol_test_vert(t, U, up, vp, o, w)) < t)
            {
                v_cpy(T, U);
                t = u;
            }
        }
 
    /* Test all edges */

    if (up->r > 0.0f)
        for (i = 0; i < lp->ec; i++)
        {
            const struct s_edge *ep = fp->ev + fp->iv[lp->e0 + i];

            if ((u = sol_test_edge(t, U, up, fp, ep, o, w)) < t)
            {
                v_cpy(T, U);
                t = u;
            }
        }

    /* Test all sides */

    for (i = 0; i < lp->sc; i++)
    {
        const struct s_side *sp = fp->sv + fp->iv[lp->s0 + i];

        if ((u = sol_test_side(t, U, up, fp, lp, sp, o, w)) < t)
        {
            v_cpy(T, U);
            t = u;
        }
    }
    return t;
}

static float sol_test_node(float dt,
                           float T[3],
                           const struct s_ball *up,
                           const struct s_file *fp,
                           const struct s_node *np,
                           const float o[3],
                           const float w[3])
{
    float U[3], u, t = dt;
    int i;

    /* Test all lumps */

    for (i = 0; i < np->lc; i++)
    {
        const struct s_lump *lp = fp->lv + np->l0 + i;

        if ((u = sol_test_lump(t, U, up, fp, lp, o, w)) < t)
        {
            v_cpy(T, U);
            t = u;
        }
    }

    /* Test in front of this node */

    if (np->ni >= 0 && sol_test_fore(t, up, fp->sv + np->si, o, w))
    {
        const struct s_node *nq = fp->nv + np->ni;

        if ((u = sol_test_node(t, U, up, fp, nq, o, w)) < t)
        {
            v_cpy(T, U);
            t = u;
        }
    }

    /* Test behind this node */

    if (np->nj >= 0 && sol_test_back(t, up, fp->sv + np->si, o, w))
    {
        const struct s_node *nq = fp->nv + np->nj;

        if ((u = sol_test_node(t, U, up, fp, nq, o, w)) < t)
        {
            v_cpy(T, U);
            t = u;
        }
    }

    return t;
}

static float sol_test_body(float dt,
                           float T[3], float V[3],
                           const struct s_ball *up,
                           const struct s_file *fp,
                           const struct s_body *bp)
{
    float U[3], O[3], W[3], u, t = dt;

    const struct s_node *np = fp->nv + bp->ni;

    sol_body_p(O, fp, bp);
    sol_body_v(W, fp, bp);

    if ((u = sol_test_node(t, U, up, fp, np, O, W)) < t)
    {
        v_cpy(T, U);
        v_cpy(V, W);
        t = u;
    }
    return t;
}

static float sol_test_file(float dt,
                           float T[3], float V[3],
                           const struct s_ball *up,
                           const struct s_file *fp)
{
    float U[3], W[3], u, t = dt;
    int i;

    for (i = 0; i < fp->bc; i++)
    {
        const struct s_body *bp = fp->bv + i;

        if ((u = sol_test_body(t, U, W, up, fp, bp)) < t)
        {
            v_cpy(T, U);
            v_cpy(V, W);
            t = u;
        }
    }
    return t;
}

/*---------------------------------------------------------------------------*/

/*
 * Step the physics forward DT  seconds under the influence of gravity
 * vector G.  If the ball gets pinched between two moving solids, this
 * loop might not terminate.  It  is better to do something physically
 * impossible than  to lock up the game.   So, if we make  more than C
 * iterations, punt it.
 */

float sol_step(struct s_file *fp, const float *g, float dt, int ui, int *m)
{
    float P[3], V[3], v[3], r[3], d, e, nt, b = 0.0f, tt = dt;
    int c = 16;

    if (ui < fp->uc)
    {
        struct s_ball *up = fp->uv + ui;

        /* If the ball is in contact with a surface, apply friction. */

        v_cpy(v, up->v);
        v_cpy(up->v, g);

        if (m && sol_test_file(tt, P, V, up, fp) < 0.0005f)
        {
            v_cpy(up->v, v);
            v_sub(r, P, up->p);

            if ((d = v_dot(r, g) / (v_len(r) * v_len(g))) > 0.999f)
            {
                if ((e = (v_len(up->v) - dt)) > 0.0f)
                {
                    /* Scale the linear velocity. */

                    v_nrm(up->v, up->v);
                    v_scl(up->v, up->v, e);

                    /* Scale the angular velocity. */

                    v_sub(v, V, up->v);
                    v_crs(up->w, v, r);
                    v_scl(up->w, up->w, -1.0f / (up->r * up->r));
                }
                else
                {
                    /* Friction has brought the ball to a stop. */

                    up->v[0] = 0.0f;
                    up->v[1] = 0.0f;
                    up->v[2] = 0.0f;

                    (*m)++;
                }
            }
            else v_mad(up->v, v, g, tt);
        }
        else v_mad(up->v, v, g, tt);

        /* Test for collision. */

        while (c > 0 && tt > 0 && tt > (nt = sol_test_file(tt, P, V, up, fp)))
        {
            sol_body_step(fp, nt);
            sol_swch_step(fp, nt);
            sol_ball_step(fp, nt);

            tt -= nt;

            if (b < (d = sol_bounce(up, P, V, nt)))
                b = d;

            c--;
        }

        sol_body_step(fp, tt);
        sol_swch_step(fp, tt);
        sol_ball_step(fp, tt);
    }
    return b;
}

/*---------------------------------------------------------------------------*/

int sol_coin_test(struct s_file *fp, float *p, float coin_r)
{
    const float *ball_p = fp->uv->p;
    const float  ball_r = fp->uv->r;
    int ci, n;

    for (ci = 0; ci < fp->cc; ci++)
    {
        float r[3];

        r[0] = ball_p[0] - fp->cv[ci].p[0];
        r[1] = ball_p[1] - fp->cv[ci].p[1];
        r[2] = ball_p[2] - fp->cv[ci].p[2];

        if (fp->cv[ci].n > 0 && v_len(r) < ball_r + coin_r)
        {
            p[0] = fp->cv[ci].p[0];
            p[1] = fp->cv[ci].p[1];
            p[2] = fp->cv[ci].p[2];

            n = fp->cv[ci].n;
            fp->cv[ci].n = 0;

            return n;
        }
    }
    return 0;
}

int sol_goal_test(struct s_file *fp, float *p, int ui)
{
    const float *ball_p = fp->uv[ui].p;
    const float  ball_r = fp->uv[ui].r;
    int zi;

    for (zi = 0; zi < fp->zc; zi++)
    {
        float r[3];

        r[0] = ball_p[0] - fp->zv[zi].p[0];
        r[1] = ball_p[2] - fp->zv[zi].p[2];
        r[2] = 0;

        if (v_len(r) < fp->zv[zi].r * 1.1 - ball_r &&
            ball_p[1] > fp->zv[zi].p[1] &&
            ball_p[1] < fp->zv[zi].p[1] + GOAL_HEIGHT / 2)
        {
            p[0] = fp->zv[zi].p[0];
            p[1] = fp->zv[zi].p[1];
            p[2] = fp->zv[zi].p[2];

            return 1;
        }
    }
    return 0;
}

int sol_jump_test(struct s_file *fp, float *p, int ui)
{
    const float *ball_p = fp->uv[ui].p;
    const float  ball_r = fp->uv[ui].r;
    int ji;

    for (ji = 0; ji < fp->jc; ji++)
    {
        float r[3];

        r[0] = ball_p[0] - fp->jv[ji].p[0];
        r[1] = ball_p[2] - fp->jv[ji].p[2];
        r[2] = 0;

        if (v_len(r) < fp->jv[ji].r - ball_r &&
            ball_p[1] > fp->jv[ji].p[1] &&
            ball_p[1] < fp->jv[ji].p[1] + JUMP_HEIGHT / 2)
        {
            p[0] = fp->jv[ji].q[0] + (ball_p[0] - fp->jv[ji].p[0]);
            p[1] = fp->jv[ji].q[1] + (ball_p[1] - fp->jv[ji].p[1]);
            p[2] = fp->jv[ji].q[2] + (ball_p[2] - fp->jv[ji].p[2]);

            return 1;
        }
    }
    return 0;
}

int sol_swch_test(struct s_file *fp, int flag, int ui)
{
    const float *ball_p = fp->uv[ui].p;
    const float  ball_r = fp->uv[ui].r;
    int xi;
    int f = 1;

    for (xi = 0; xi < fp->xc; xi++)
    {
        struct s_swch *xp = fp->xv + xi;

        if (xp->t0 == 0 || xp->f == xp->f0)
        {
            float r[3];

            r[0] = ball_p[0] - xp->p[0];
            r[1] = ball_p[2] - xp->p[2];
            r[2] = 0;

            if (v_len(r)  < xp->r - ball_r &&
                ball_p[1] > xp->p[1] &&
                ball_p[1] < xp->p[1] + SWCH_HEIGHT / 2)
            {
                if (flag)
                {
                    int pi = xp->pi;
                    int pj = xp->pi;

                    /* Toggle the state, update the path. */

                    xp->f = xp->f ? 0 : 1;

                    do  /* Tortoise and hare cycle traverser. */
                    {
                        fp->pv[pi].f = xp->f;
                        fp->pv[pj].f = xp->f;

                        pi = fp->pv[pi].pi;
                        pj = fp->pv[pj].pi;
                        pj = fp->pv[pj].pi;
                    }
                    while (pi != pj);

                    /* It toggled to non-default state, start the timer. */

                    if (xp->f != xp->f0)
                        xp->t  = xp->t0;
                }
                f = 0;
            }
        }
    }
    return f;
}

/*---------------------------------------------------------------------------*/

void put_file_state(FILE *fout, struct s_file *fp)
{
    /* Write the position and orientation of the ball. */

    put_array(fout, fp->uv[0].p,    3);
    put_array(fout, fp->uv[0].e[0], 3);
    put_array(fout, fp->uv[0].e[1], 3);
}

void get_file_state(FILE *fin, struct s_file *fp)
{
    /* Read the position and orientation of the ball. */

    get_array(fin, fp->uv[0].p,    3);
    get_array(fin, fp->uv[0].e[0], 3);
    get_array(fin, fp->uv[0].e[1], 3);

    /* Compute the 3rd vector of the ball orientatian basis. */

    v_crs(fp->uv[0].e[2], fp->uv[0].e[0], fp->uv[0].e[1]);
}

/*---------------------------------------------------------------------------*/

