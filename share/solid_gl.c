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
#include "solid_gl.h"
#include "base_config.h"

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
                          const struct s_body *bp, int fl, int decal)
{
    int mi, li, gi;

    if (decal)
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.0f, -2.0f);
    }

    /* Iterate all materials of the correct opacity. */

    for (mi = 0; mi < fp->mc; mi++)
        if ((fp->mv[mi].fl & fl) && (fp->mv[mi].fl & M_DECAL) == decal)
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

    if (decal)
        glDisable(GL_POLYGON_OFFSET_FILL);
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

    glPushAttrib(GL_ENABLE_BIT       |
                 GL_TEXTURE_BIT      |
                 GL_LIGHTING_BIT     |
                 GL_COLOR_BUFFER_BIT |
                 GL_DEPTH_BUFFER_BIT);
    {
        /* Render all opaque geometry into the color and depth buffers. */

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
                          const struct s_body *bp, int fl, int decal)
{
    int mi, li, gi;

    if (decal)
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.0f, -2.0f);
    }

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

    if (decal)
        glDisable(GL_POLYGON_OFFSET_FILL);
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

    /* Here we sort geometry into display lists by material type. */

    for (i = 0; i < fp->bc; i++)
    {
        struct s_body *bp = fp->bv + i;

        /* Draw all opaque geometry, decals last. */

        if (sol_enum_body(fp, bp, M_OPAQUE))
        {
            fp->bv[i].ol = glGenLists(1);

            glNewList(fp->bv[i].ol, GL_COMPILE);
            {
                sol_draw_body(fp, fp->bv+i, M_OPAQUE, 0);
                sol_draw_body(fp, fp->bv+i, M_OPAQUE, M_DECAL);
            }
            glEndList();
        }
        else fp->bv[i].ol = 0;

        /* Draw all translucent geometry, decals first. */

        if (sol_enum_body(fp, bp, M_TRANSPARENT))
        {
            fp->bv[i].tl = glGenLists(1);

            glNewList(fp->bv[i].tl, GL_COMPILE);
            {
                sol_draw_body(fp, fp->bv+i, M_TRANSPARENT, M_DECAL);
                sol_draw_body(fp, fp->bv+i, M_TRANSPARENT, 0);
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
                sol_draw_body(fp, fp->bv+i, M_REFLECTIVE, 0);
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
                sol_shad_body(fp, fp->bv+i, M_SHADOWED, 0);
                sol_shad_body(fp, fp->bv+i, M_SHADOWED, M_DECAL);
            }
            glEndList();
        }
        else fp->bv[i].sl = 0;
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

static void sol_load_textures(struct s_file *fp, int k)
{
    int i;

    /* Load the image referenced by each material. */

    for (i = 0; i < fp->mc; i++)
        if ((fp->mv[i].o = sol_find_texture(fp->mv[i].f)))
        {
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
        }
}

/*---------------------------------------------------------------------------*/

int sol_load_gl(struct s_file *fp, const char *filename, int k, int s)
{
    if (sol_load_only_file(fp, filename))
    {
        sol_load_textures(fp, k);
        sol_load_objects (fp, s);
        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void sol_free_gl(struct s_file *fp)
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

    sol_free(fp);
}

/*---------------------------------------------------------------------------*/
