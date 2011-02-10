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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "glext.h"
#include "vec3.h"
#include "image.h"
#include "base_image.h"
#include "base_config.h"
#include "lang.h"

#include "solid_draw.h"
#include "solid_all.h"

/*---------------------------------------------------------------------------*/

static int sol_enum_mtrl(const struct s_base *base,
                         const struct b_body *bp, int mi)
{
    int li, gi, c = 0;

    /* Count all lump geoms with this material. */

    for (li = 0; li < bp->lc; li++)
    {
        int g0 = base->lv[bp->l0 + li].g0;
        int gc = base->lv[bp->l0 + li].gc;

        for (gi = 0; gi < gc; gi++)
            if (base->gv[base->iv[g0 + gi]].mi == mi)
                c++;
    }

    /* Count all body geoms with this material. */

    for (gi = 0; gi < bp->gc; gi++)
        if (base->gv[base->iv[bp->g0 + gi]].mi == mi)
            c++;

    return c;
}

static int sol_enum_body(const struct s_base *base,
                         const struct b_body *bp, int fl)
{
    int mi, c = 0;

    /* Count all geoms with this flag. */

    for (mi = 0; mi < base->mc; mi++)
        if (base->mv[mi].fl & fl)
            c = c + sol_enum_mtrl(base, bp, mi);

    return c;
}

/*---------------------------------------------------------------------------*/

int sol_reflective(const struct s_draw *draw)
{
    int bi;

    for (bi = 0; bi < draw->bc; bi++)
        if (draw->bv[bi].rl)
            return 1;

    return 0;
}

/*---------------------------------------------------------------------------*/

#define tobyte(f) ((GLubyte) (f * 255.0f))

#define color_cmp(a, b) (tobyte((a)[0]) == tobyte((b)[0]) && \
                         tobyte((a)[1]) == tobyte((b)[1]) && \
                         tobyte((a)[2]) == tobyte((b)[2]) && \
                         tobyte((a)[3]) == tobyte((b)[3]))

static struct b_mtrl default_base_mtrl =
{
    { 0.8f, 0.8f, 0.8f, 1.0f },
    { 0.2f, 0.2f, 0.2f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f }, 0.0f, M_OPAQUE, ""
};

static struct d_mtrl default_draw_mtrl =
{
    &default_base_mtrl, 0
};

static const struct d_mtrl *sol_draw_mtrl(const struct s_draw *draw,
                                          const struct d_mtrl *mp_draw,
                                          const struct d_mtrl *mq_draw)
{
    const struct b_mtrl *mp_base = mp_draw->base;
    const struct b_mtrl *mq_base = mq_draw->base;

    /* Change material properties only as needed. */

    if (!color_cmp(mp_base->a, mq_base->a))
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   mp_base->a);
    if (!color_cmp(mp_base->d, mq_base->d))
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mp_base->d);
    if (!color_cmp(mp_base->s, mq_base->s))
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mp_base->s);
    if (!color_cmp(mp_base->e, mq_base->e))
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  mp_base->e);
    if (tobyte(mp_base->h[0]) != tobyte(mq_base->h[0]))
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mp_base->h);

    /* Bind the texture. */

    if (mp_draw->o != mq_draw->o)
        glBindTexture(GL_TEXTURE_2D, mp_draw->o);

    /* Enable environment mapping. */

    if ((mp_base->fl & M_ENVIRONMENT) && !(mq_base->fl & M_ENVIRONMENT))
    {
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);

        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    }

    /* Disable environment mapping. */

    if ((mq_base->fl & M_ENVIRONMENT) && !(mp_base->fl & M_ENVIRONMENT))
    {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
    }

    /* Enable additive blending. */

    if ((mp_base->fl & M_ADDITIVE) && !(mq_base->fl & M_ADDITIVE))
        glBlendFunc(GL_ONE, GL_ONE);

    /* Enable standard blending. */

    if ((mq_base->fl & M_ADDITIVE) && !(mp_base->fl & M_ADDITIVE))
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Enable visibility-from-behind. */

    if ((mp_base->fl & M_TWO_SIDED) && !(mq_base->fl & M_TWO_SIDED))
    {
        glDisable(GL_CULL_FACE);
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
    }

    /* Disable visibility-from-behind. */

    if ((mq_base->fl & M_TWO_SIDED) && !(mp_base->fl & M_TWO_SIDED))
    {
        glEnable(GL_CULL_FACE);
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
    }

    /* Enable decal offset. */

    if ((mp_base->fl & M_DECAL) && !(mq_base->fl & M_DECAL))
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.0f, -2.0f);
    }

    /* Disable decal offset. */

    if ((mq_base->fl & M_DECAL) && !(mp_base->fl & M_DECAL))
        glDisable(GL_POLYGON_OFFSET_FILL);

    return mp_draw;
}

static const struct d_mtrl *sol_back_bill(const struct s_draw *draw,
                                           const struct b_bill *rp,
                                           const struct d_mtrl *mp,
                                           float t)
{
    float T = (rp->t > 0.0f) ? (fmodf(t, rp->t) - rp->t / 2) : 0.0f;

    float w = rp->w[0] + rp->w[1] * T + rp->w[2] * T * T;
    float h = rp->h[0] + rp->h[1] * T + rp->h[2] * T * T;

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

            mp = sol_draw_mtrl(draw, draw->mv + rp->mi, mp);

            glBegin(GL_QUADS);
            {
                glTexCoord2f(0.0f, 0.0f); glVertex2f(-w / 2, y0);
                glTexCoord2f(1.0f, 0.0f); glVertex2f(+w / 2, y0);
                glTexCoord2f(1.0f, 1.0f); glVertex2f(+w / 2, y1);
                glTexCoord2f(0.0f, 1.0f); glVertex2f(-w / 2, y1);
            }
            glEnd();
        }
        glPopMatrix();
    }

    return mp;
}

/*---------------------------------------------------------------------------*/

void sol_back(const struct s_draw *draw, float n, float f, float t)
{
    const struct d_mtrl *mp = &default_draw_mtrl;

    int ri;

    /* Render all billboards in the given range. */

    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    {
        for (ri = 0; ri < draw->base->rc; ri++)
            if (n <= draw->base->rv[ri].d && draw->base->rv[ri].d < f)
                mp = sol_back_bill(draw, draw->base->rv + ri, mp, t);

        mp = sol_draw_mtrl(draw, &default_draw_mtrl, mp);
    }
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
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

static void sol_draw_geom(const struct s_base *base,
                          const struct b_geom *gp, int mi)
{
    if (gp->mi == mi)
    {
        const float *ui = base->tv[gp->ti].u;
        const float *uj = base->tv[gp->tj].u;
        const float *uk = base->tv[gp->tk].u;

        const float *ni = base->sv[gp->si].n;
        const float *nj = base->sv[gp->sj].n;
        const float *nk = base->sv[gp->sk].n;

        const float *vi = base->vv[gp->vi].p;
        const float *vj = base->vv[gp->vj].p;
        const float *vk = base->vv[gp->vk].p;

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

static void sol_draw_lump(const struct s_base *base,
                          const struct b_lump *lp, int mi)
{
    int i;

    for (i = 0; i < lp->gc; i++)
        sol_draw_geom(base, base->gv + base->iv[lp->g0 + i], mi);
}

static const struct d_mtrl *sol_draw_body(const struct s_draw *draw,
                                          const struct b_body *bp,
                                          const struct d_mtrl *mp,
                                          int fl, int decal)
{
    const struct s_base *base = draw->base;

    int mi, li, gi;

    /* Iterate all materials of the correct opacity. */

    for (mi = 0; mi < draw->mc; mi++)
    {
        struct d_mtrl *mq = draw->mv + mi;

        if ((mq->base->fl & fl) && (mq->base->fl & M_DECAL) == decal)
        {
            if (sol_enum_mtrl(draw->base, bp, mi))
            {
                /* Set the material state. */

                mp = sol_draw_mtrl(draw, mq, mp);

                /* Render all geometry of that material. */

                glBegin(GL_TRIANGLES);
                {
                    for (li = 0; li < bp->lc; li++)
                        sol_draw_lump(draw->base,
                                      base->lv + bp->l0 + li,
                                      mi);
                    for (gi = 0; gi < bp->gc; gi++)
                        sol_draw_geom(draw->base,
                                      base->gv + base->iv[bp->g0 + gi],
                                      mi);
                }
                glEnd();
            }
        }
    }

    return mp;
}

static void sol_draw_list(const struct s_vary *vary,
                          const struct v_body *bp, GLuint list)
{
    float p[3], e[4], u[3], a;

    sol_body_p(p, vary, bp->pi, bp->t);
    sol_body_e(e, vary, bp, 0);

    q_as_axisangle(e, u, &a);
    a = V_DEG(a);

    glPushMatrix();
    {
        /* Translate and rotate a moving body. */

        glTranslatef(p[0], p[1], p[2]);
        glRotatef(a, u[0], u[1], u[2]);

        /* Draw the body. */

        glCallList(list);
    }
    glPopMatrix();
}

void sol_draw(const struct s_draw *draw, int depthmask, int depthtest)
{
    int bi;

    /* Render all opaque geometry into the color and depth buffers. */

    for (bi = 0; bi < draw->bc; bi++)
        if (draw->bv[bi].ol)
            sol_draw_list(draw->vary, draw->vary->bv + bi, draw->bv[bi].ol);

    /* Render all translucent geometry into only the color buffer. */

    if (depthtest == 0) glDisable(GL_DEPTH_TEST);
    if (depthmask == 0) glDepthMask(GL_FALSE);
    {
        for (bi = 0; bi < draw->bc; bi++)
            if (draw->bv[bi].tl)
                sol_draw_list(draw->vary, draw->vary->bv + bi, draw->bv[bi].tl);
    }
    if (depthmask == 0) glDepthMask(GL_TRUE);
    if (depthtest == 0) glEnable(GL_DEPTH_TEST);
}

void sol_bill(const struct s_draw *draw, const float *M, float t)
{
    const struct d_mtrl *mp = &default_draw_mtrl;

    int ri;

    for (ri = 0; ri < draw->base->rc; ++ri)
    {
        const struct b_bill *rp = draw->base->rv + ri;

        float T = rp->t * t;
        float S = fsinf(T);

        float w  = rp->w [0] + rp->w [1] * T + rp->w [2] * S;
        float h  = rp->h [0] + rp->h [1] * T + rp->h [2] * S;
        float rx = rp->rx[0] + rp->rx[1] * T + rp->rx[2] * S;
        float ry = rp->ry[0] + rp->ry[1] * T + rp->ry[2] * S;
        float rz = rp->rz[0] + rp->rz[1] * T + rp->rz[2] * S;

        mp = sol_draw_mtrl(draw, draw->mv + rp->mi, mp);

        glPushMatrix();
        {
            glTranslatef(rp->p[0], rp->p[1], rp->p[2]);

            if (M && ((rp->fl & B_NOFACE) == 0)) glMultMatrixf(M);

            if (fabsf(rx) > 0.0f) glRotatef(rx, 1.0f, 0.0f, 0.0f);
            if (fabsf(ry) > 0.0f) glRotatef(ry, 0.0f, 1.0f, 0.0f);
            if (fabsf(rz) > 0.0f) glRotatef(rz, 0.0f, 0.0f, 1.0f);

            glBegin(GL_QUADS);
            {
                glTexCoord2f(0.0f, 0.0f); glVertex2f(-w / 2, -h / 2);
                glTexCoord2f(1.0f, 0.0f); glVertex2f(+w / 2, -h / 2);
                glTexCoord2f(1.0f, 1.0f); glVertex2f(+w / 2, +h / 2);
                glTexCoord2f(0.0f, 1.0f); glVertex2f(-w / 2, +h / 2);
            }
            glEnd();
        }
        glPopMatrix();
    }

    mp = sol_draw_mtrl(draw, &default_draw_mtrl, mp);
}

void sol_refl(const struct s_draw *draw)
{
    int bi;

    /* Render all reflective geometry into the color and depth buffers. */

    for (bi = 0; bi < draw->bc; bi++)
        if (draw->bv[bi].rl)
            sol_draw_list(draw->vary, draw->vary->bv + bi, draw->bv[bi].rl);
}

/*---------------------------------------------------------------------------*/

static void sol_shad_geom(const struct s_base *base,
                          const struct b_geom *gp, int mi)
{
    if (gp->mi == mi)
    {
        const float *vi = base->vv[gp->vi].p;
        const float *vj = base->vv[gp->vj].p;
        const float *vk = base->vv[gp->vk].p;

        glTexCoord2f(vi[0], vi[2]);
        glVertex3fv(vi);

        glTexCoord2f(vj[0], vj[2]);
        glVertex3fv(vj);

        glTexCoord2f(vk[0], vk[2]);
        glVertex3fv(vk);
    }
}

static void sol_shad_lump(const struct s_base *base,
                          const struct b_lump *lp, int mi)
{
    int i;

    for (i = 0; i < lp->gc; i++)
        sol_shad_geom(base, base->gv + base->iv[lp->g0 + i], mi);
}

static void sol_shad_body(const struct s_base *base,
                          const struct b_body *bp,
                          int fl, int decal)
{
    int mi, li, gi;

    if (decal)
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.0f, -2.0f);
    }

    glBegin(GL_TRIANGLES);
    {
        for (mi = 0; mi < base->mc; mi++)
        {
            struct b_mtrl *mp = base->mv + mi;

            if ((mp->fl & fl) && (mp->fl & M_DECAL) == decal)
            {
                for (li = 0; li < bp->lc; li++)
                    sol_shad_lump(base, base->lv + bp->l0 + li, mi);
                for (gi = 0; gi < bp->gc; gi++)
                    sol_shad_geom(base, base->gv + base->iv[bp->g0 + gi], mi);
            }
        }
    }
    glEnd();

    if (decal)
        glDisable(GL_POLYGON_OFFSET_FILL);
}

static void sol_shad_list(const struct s_vary *vary,
                          const struct v_body *bp, GLuint list)
{
    float p[3], e[4], u[3], a;

    sol_body_p(p, vary, bp->pi, bp->t);
    sol_body_e(e, vary, bp, 0);

    q_as_axisangle(e, u, &a);
    a = V_DEG(a);

    glPushMatrix();
    {
        /* Translate and rotate a moving body. */

        glTranslatef(p[0], p[1], p[2]);
        glRotatef(a, u[0], u[1], u[2]);

        /* Translate the shadow on a moving body. */

        glMatrixMode(GL_TEXTURE);
        {
            glPushMatrix();
            glTranslatef(p[0], p[2], 0.0f);
            glRotatef(-a, u[0], u[2], u[1]);
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

void sol_shad(const struct s_draw *draw)
{
    int bi;

    /* Render all shadowed geometry. */

    glDepthMask(GL_FALSE);
    {
        for (bi = 0; bi < draw->bc; bi++)
            if (draw->bv[bi].sl)
                sol_shad_list(draw->vary, draw->vary->bv + bi, draw->bv[bi].sl);
    }
    glDepthMask(GL_TRUE);
}

/*---------------------------------------------------------------------------*/

static void sol_load_objects(struct s_draw *draw, int s)
{
    int i;

    /* Here we sort geometry into display lists by material type. */

    for (i = 0; i < draw->bc; i++)
    {
        struct d_body *bp = draw->bv + i;

        int on = sol_enum_body(draw->base, bp->base, M_OPAQUE);
        int tn = sol_enum_body(draw->base, bp->base, M_TRANSPARENT);
        int rn = sol_enum_body(draw->base, bp->base, M_REFLECTIVE);
        int dn = sol_enum_body(draw->base, bp->base, M_DECAL);
        int sn = sol_enum_body(draw->base, bp->base, M_SHADOWED);

        /* Draw all opaque geometry, decals last. */

        if (on)
        {
            bp->ol = glGenLists(1);

            glNewList(bp->ol, GL_COMPILE);
            {
                const struct d_mtrl *mp = &default_draw_mtrl;

                mp = sol_draw_body(draw, bp->base, mp, M_OPAQUE, 0);
                mp = sol_draw_body(draw, bp->base, mp, M_OPAQUE, M_DECAL);
                mp = sol_draw_mtrl(draw, &default_draw_mtrl, mp);
            }
            glEndList();
        }
        else bp->ol = 0;

        /* Draw all translucent geometry, decals first. */

        if (tn)
        {
            bp->tl = glGenLists(1);

            glNewList(bp->tl, GL_COMPILE);
            {
                const struct d_mtrl *mp = &default_draw_mtrl;

                mp = sol_draw_body(draw, bp->base, mp, M_TRANSPARENT, M_DECAL);
                mp = sol_draw_body(draw, bp->base, mp, M_TRANSPARENT, 0);
                mp = sol_draw_mtrl(draw, &default_draw_mtrl, mp);
            }
            glEndList();
        }
        else bp->tl = 0;

        /* Draw all reflective geometry. */

        if (rn)
        {
            bp->rl = glGenLists(1);

            glNewList(bp->rl, GL_COMPILE);
            {
                const struct d_mtrl *mp = &default_draw_mtrl;

                mp = sol_draw_body(draw, bp->base, mp, M_REFLECTIVE, 0);
                mp = sol_draw_mtrl(draw, &default_draw_mtrl, mp);
            }
            glEndList();
        }
        else bp->rl = 0;

        /* Draw all shadowed geometry. */

        if (s && (on || rn || sn))
        {
            bp->sl = glGenLists(1);

            glNewList(bp->sl, GL_COMPILE);
            {
                if (on) sol_shad_body(draw->base, bp->base, M_OPAQUE, 0);
                if (rn) sol_shad_body(draw->base, bp->base, M_REFLECTIVE, 0);
                if (dn) sol_shad_body(draw->base, bp->base, M_OPAQUE, M_DECAL);
                if (sn)
                {
                    /* Transparent shadowed geometry hack. */

                    if (dn)
                        sol_shad_body(draw->base, bp->base, M_SHADOWED, M_DECAL);

                    sol_shad_body(draw->base, bp->base, M_SHADOWED, 0);
                }
            }
            glEndList();
        }
        else bp->sl = 0;
    }
}

static GLuint sol_find_texture(const char *name)
{
    char png[MAXSTR];
    char jpg[MAXSTR];

    GLuint o;

    /* Prefer a lossless copy of the texture over a lossy compression. */

    strncpy(png, name, PATHMAX); strcat(png, ".png");
    strncpy(jpg, name, PATHMAX); strcat(jpg, ".jpg");

    /* Check for a PNG. */

    if ((o = make_image_from_file(png)))
        return o;

    /* Check for a JPG. */

    if ((o = make_image_from_file(jpg)))
        return o;

    return 0;
}

static void sol_load_textures(struct s_draw *draw)
{
    int i;

    /* Load the image referenced by each material. */

    for (i = 0; i < draw->mc; i++)
    {
        struct d_mtrl *mp = draw->mv + i;

        if ((mp->o = sol_find_texture(_(mp->base->f))))
        {
            /* Set the texture to clamp or repeat based on material type. */

            if (mp->base->fl & M_CLAMPED)
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            }
            else
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

int sol_load_draw(struct s_draw *draw, const struct s_vary *vary, int s)
{
    int i;

    memset(draw, 0, sizeof (*draw));

    draw->vary = vary;
    draw->base = draw->vary->base;

    if (draw->base->mc)
    {
        draw->mv = calloc(draw->base->mc, sizeof (*draw->mv));
        draw->mc = draw->base->mc;

        for (i = 0; i < draw->base->mc; i++)
        {
            struct d_mtrl *mp = draw->mv + i;
            struct b_mtrl *mq = draw->base->mv + i;

            mp->base = mq;
        }
    }

    if (draw->base->bc)
    {
        draw->bv = calloc(draw->base->bc, sizeof (*draw->bv));
        draw->bc = draw->base->bc;

        for (i = 0; i < draw->base->bc; i++)
        {
            struct d_body *bp = draw->bv + i;
            struct b_body *bq = draw->base->bv + i;

            bp->base = bq;
        }
    }

    sol_load_textures(draw);
    sol_load_objects (draw, s);

    return 1;
}

void sol_free_draw(struct s_draw *draw)
{
    int i;

    for (i = 0; i < draw->mc; i++)
    {
        if (glIsTexture(draw->mv[i].o))
            glDeleteTextures(1, &draw->mv[i].o);
    }

    for (i = 0; i < draw->bc; i++)
    {
        if (glIsList(draw->bv[i].ol))
            glDeleteLists(draw->bv[i].ol, 1);
        if (glIsList(draw->bv[i].tl))
            glDeleteLists(draw->bv[i].tl, 1);
        if (glIsList(draw->bv[i].rl))
            glDeleteLists(draw->bv[i].rl, 1);
        if (glIsList(draw->bv[i].sl))
            glDeleteLists(draw->bv[i].sl, 1);
    }

    free(draw->mv);
    free(draw->bv);

    memset(draw, 0, sizeof (*draw));
}

/*---------------------------------------------------------------------------*/

int sol_load_full(struct s_full *full, const char *filename, int s)
{
    if (sol_load_base(&full->base, filename))
    {
        sol_load_vary(&full->vary, &full->base);
        sol_load_draw(&full->draw, &full->vary, s);

        return 1;
    }

    return 0;
}

void sol_free_full(struct s_full *full)
{
    sol_free_draw(&full->draw);
    sol_free_vary(&full->vary);
    sol_free_base(&full->base);
}

/*---------------------------------------------------------------------------*/
