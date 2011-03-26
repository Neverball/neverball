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
#include "geom.h"
#include "image.h"
#include "base_image.h"
#include "base_config.h"
#include "lang.h"

#include "solid_draw.h"
#include "solid_all.h"

/*---------------------------------------------------------------------------*/

static void sol_transform(const struct s_vary *vary,
                          const struct v_body *bp)
{
    float a;
    float d[4];
    float e[4];
    float p[3];
    float v[3];

    /* Compute the body transform. */

    sol_body_p(p, vary, bp->pi, bp->t);
    sol_body_e(e, vary, bp, 0);

    q_as_axisangle(e, v, &a);

    glTranslatef(p[0], p[1], p[2]);
    glRotatef(V_DEG(a), v[0], v[1], v[2]);

    /* Compute the shadow texture transform */

    v_sub(d, vary->uv->p, p);

    glActiveTexture_(GL_TEXTURE1);
    glMatrixMode(GL_TEXTURE);
    {
        float k = 0.25f / vary->uv->r;

        glLoadIdentity();
        glTranslatef(0.5f - k * d[0],
                     0.5f - k * d[2], 0.0f);
        glScalef(k, k, 0.0f);
    }
    glMatrixMode(GL_MODELVIEW);
    glActiveTexture_(GL_TEXTURE0);
}

/*---------------------------------------------------------------------------*/

void sol_back(const struct s_draw *draw, float n, float f, float t)
{
}

/*---------------------------------------------------------------------------*/

void sol_bill(const struct s_draw *draw, const float *M, float t)
{
}

/*---------------------------------------------------------------------------*/

void sol_shad(const struct s_draw *draw, int ui)
{
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

static const struct d_mtrl *sol_apply_mtrl(const struct d_mtrl *mp_draw,
                                           const struct d_mtrl *mq_draw)
{
    const struct b_mtrl *mp_base = mp_draw->base;
    const struct b_mtrl *mq_base = mq_draw->base;

    /* Bind the texture. */

    if (mp_draw->o != mq_draw->o)
        glBindTexture(GL_TEXTURE_2D, mp_draw->o);

    /* Set material properties. */

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

    /* Enable ball shadow. */

    if ((mp_base->fl & M_SHADOWED) && !(mq_base->fl & M_SHADOWED))
    {
        shad_draw_set();
    }

    /* Disable ball shadow. */

    if (!(mp_base->fl & M_SHADOWED) && (mq_base->fl & M_SHADOWED))
    {
        shad_draw_clr();
    }

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

static void sol_load_mtrl(struct d_mtrl *mp,
                    const struct b_mtrl *mq,
                          struct s_draw *draw)
{
    mp->base = mq;

    if ((mp->o = sol_find_texture(_(mq->f))))
    {
        /* Set the texture to clamp or repeat based on material type. */

        if (mq->fl & M_CLAMPED)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }

        /* If at least one material is reflective, mark it in the SOL. */

        if (mq->fl & M_REFLECTIVE)
            draw->reflective = 1;
    }
}

static void sol_free_mtrl(struct d_mtrl *mp)
{
    if (glIsTexture(mp->o))
        glDeleteTextures(1, &mp->o);
}

static int sol_test_mtrl(const struct d_mtrl *mp, int f0, int f1)
{
    /* Test whether the material flags exclude f0 and include f1. */

    return ((mp->base->fl & f1) == f1 &&
            (mp->base->fl & f0) ==  0);
}

/*---------------------------------------------------------------------------*/

static int sol_count_geom(const struct s_base *base, int g0, int gc, int mi)
{
    int gi, c = 0;

    /* The arguments g0 and gc specify a range of the index array. These     */
    /* indices refer to geoms. Determine how many of these geoms use the     */
    /* given material                                                        */

    for (gi = 0; gi < gc; gi++)
        if (base->gv[base->iv[g0 + gi]].mi == mi)
            c++;

    return c;
}

static int sol_count_body(const struct b_body *bp,
                          const struct s_base *base, int mi)
{
    int li, c = 0;

    /* Count all lump geoms with the given material. */

    for (li = 0; li < bp->lc; li++)
        c += sol_count_geom(base, base->lv[bp->l0 + li].g0,
                                  base->lv[bp->l0 + li].gc, mi);

    /* Count all body geoms with the given material. */

    c += sol_count_geom(base, bp->g0, bp->gc, mi);

    return c;
}

/*---------------------------------------------------------------------------*/

static void sol_mesh_vert(struct d_vert *vp,
                    const struct s_base *base, int oi)
{
    /* Gather all vertex attributes for the given offs. */

    const struct b_texc *tq = base->tv + base->ov[oi].ti;
    const struct b_side *sq = base->sv + base->ov[oi].si;
    const struct b_vert *vq = base->vv + base->ov[oi].vi;

    vp->p[0] = vq->p[0];
    vp->p[1] = vq->p[1];
    vp->p[2] = vq->p[2];

    vp->n[0] = sq->n[0];
    vp->n[1] = sq->n[1];
    vp->n[2] = sq->n[2];

    vp->t[0] = tq->u[0];
    vp->t[1] = tq->u[1];

    vp->u[0] = vq->p[0];
    vp->u[1] = vq->p[2];
}

static void sol_mesh_geom(struct d_vert *vv,   int *vn,
                          struct d_geom *gv,   int *gn,
                    const struct s_base *base, int *iv, int g0, int gc, int mi)
{
    int gi;

    /* Insert all geoms with material mi into the vertex and element data. */

    for (gi = 0; gi < gc; gi++)
    {
        const struct b_geom *gq = base->gv + base->iv[g0 + gi];

        if (gq->mi == mi)
        {
            /* Insert a d_vert into the VBO data for each referenced b_off. */

            if (iv[gq->oi] == -1)
            {
                iv[gq->oi] = *vn;
                sol_mesh_vert(vv + (*vn)++, base, gq->oi);
            }
            if (iv[gq->oj] == -1)
            {
                iv[gq->oj] = *vn;
                sol_mesh_vert(vv + (*vn)++, base, gq->oj);
            }
            if (iv[gq->ok] == -1)
            {
                iv[gq->ok] = *vn;
                sol_mesh_vert(vv + (*vn)++, base, gq->ok);
            }

            /* Populate the EBO data using remapped b_off indices. */

            gv[*gn].i = iv[gq->oi];
            gv[*gn].j = iv[gq->oj];
            gv[*gn].k = iv[gq->ok];

            (*gn)++;
        }
    }
}

static void sol_load_mesh(struct d_mesh *mp,
                    const struct b_body *bp,
                    const struct s_draw *draw, int mi)
{
    const size_t vs = sizeof (struct d_vert);
    const size_t gs = sizeof (struct d_geom);

    struct d_vert *vv = 0;
    struct d_geom *gv = 0;
    int           *iv = 0;

    int oc = draw->base->oc;
    int vn = 0;
    int gn = 0;

    const int gc = sol_count_body(bp, draw->base, mi);

    /* Get temporary storage for vertex and element array creation. */

    if ((vv = (struct d_vert *) calloc(oc, vs)) &&
        (gv = (struct d_geom *) calloc(gc, gs)) &&
        (iv = (int           *) calloc(oc, sizeof (int))))
    {
        int li, i;

        /* Initialize the index remapping. */

        for (i = 0; i < oc; ++i) iv[i] = -1;

        /* Include all matching lump geoms in the arrays. */

        for (li = 0; li < bp->lc; li++)
            sol_mesh_geom(vv, &vn, gv, &gn, draw->base, iv,
                          draw->base->lv[bp->l0 + li].g0,
                          draw->base->lv[bp->l0 + li].gc, mi);

        /* Include all matching body geoms in the arrays. */

        sol_mesh_geom(vv, &vn, gv, &gn, draw->base, iv, bp->g0, bp->gc, mi);

        /* Initialize buffer objects for all data. */

        glGenBuffers(1, &mp->vbo);
        glBindBuffer(GL_ARRAY_BUFFER,         mp->vbo);
        glBufferData(GL_ARRAY_BUFFER,         vn * vs, vv, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,         0);

        glGenBuffers(1, &mp->ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mp->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, gn * gs, gv, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        mp->mp  = draw->mv + mi;
        mp->ebc = gn * 3;
    }

    free(iv);
    free(gv);
    free(vv);
}

static void sol_free_mesh(struct d_mesh *mp)
{
    if (glIsBuffer(mp->ebo))
        glDeleteBuffers(1, &mp->ebo);
    if (glIsBuffer(mp->vbo))
        glDeleteBuffers(1, &mp->vbo);
}

static const struct d_mtrl *sol_draw_mesh(const struct d_mesh *mp,
                                          const struct d_mtrl *mq,
                                          int f0, int f1)
{
    /* If this mesh has material matching the given flags... */

    if (sol_test_mtrl(mp->mp, f0, f1))
    {
        const size_t s = sizeof (struct d_vert);
        const GLenum T = GL_FLOAT;

        /* Apply the material state. */

        mq = sol_apply_mtrl(mp->mp, mq);

        /* Bind the mesh data. */

        glBindBuffer(GL_ARRAY_BUFFER,         mp->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mp->ebo);

        glVertexPointer  (3, T, s, (GLvoid *) offsetof (struct d_vert, p));
        glNormalPointer  (   T, s, (GLvoid *) offsetof (struct d_vert, n));

        glClientActiveTexture(GL_TEXTURE1);
        glTexCoordPointer(2, T, s, (GLvoid *) offsetof (struct d_vert, u));
        glClientActiveTexture(GL_TEXTURE0);
        glTexCoordPointer(2, T, s, (GLvoid *) offsetof (struct d_vert, t));

        /* Draw the mesh. */

        glDrawElements(GL_TRIANGLES, mp->ebc, GL_UNSIGNED_SHORT, 0);
    }

    return mq;
}

/*---------------------------------------------------------------------------*/

static void sol_load_body(struct d_body *bp,
                    const struct b_body *bq,
                    const struct s_draw *draw)
{
    int mi;

    bp->base = bq;
    bp->mc   =  0;

    /* Determine how many materials this body uses. */

    for (mi = 0; mi < draw->mc; ++mi)
        if (sol_count_body(bq, draw->base, mi))
            bp->mc++;

    /* Allocate and initialize a mesh for each material. */

    if ((bp->mv = (struct d_mesh *) calloc(bp->mc, sizeof (struct d_mesh))))
    {
        int mj = 0;

        for (mi = 0; mi < draw->mc; ++mi)
            if (sol_count_body(bq, draw->base, mi))
                sol_load_mesh(bp->mv + mj++, bq, draw, mi);
    }
}

static void sol_free_body(struct d_body *bp)
{
    int mi;

    for (mi = 0; mi < bp->mc; ++mi)
        sol_free_mesh(bp->mv + mi);

    free(bp->mv);
}

static const struct d_mtrl *sol_draw_body(const struct d_body *bp,
                                          const struct d_mtrl *mq,
                                          int f0, int f1)
{
    int i;

    for (i = 0; i < bp->mc; ++i)
        mq = sol_draw_mesh(bp->mv + i, mq, f0, f1);

    return mq;
}

/*---------------------------------------------------------------------------*/

int sol_load_draw(struct s_draw *draw, const struct s_vary *vary, int s)
{
    int i;

    memset(draw, 0, sizeof (struct s_draw));

    draw->vary = vary;
    draw->base = vary->base;

    /* Initialize all materials for this file. */

    if (draw->base->mc)
    {
        if ((draw->mv = calloc(draw->base->mc, sizeof (*draw->mv))))
        {
            draw->mc = draw->base->mc;

            for (i = 0; i < draw->mc; i++)
                sol_load_mtrl(draw->mv + i, draw->base->mv + i, draw);
        }
    }

    /* Initialize all bodies for this file. */

    if (draw->base->bc)
    {
        if ((draw->bv = calloc(draw->base->bc, sizeof (*draw->bv))))
        {
            draw->bc = draw->base->bc;

            for (i = 0; i < draw->bc; i++)
                sol_load_body(draw->bv + i, draw->base->bv + i, draw);
        }
    }

    return 1;
}

void sol_free_draw(struct s_draw *draw)
{
    int i;

    for (i = 0; i < draw->bc; i++)
        sol_free_body(draw->bv + i);
    for (i = 0; i < draw->mc; i++)
        sol_free_mtrl(draw->mv + i);
}

/*---------------------------------------------------------------------------*/

static const struct d_mtrl *sol_draw_all(const struct s_draw *draw,
                                         const struct d_mtrl *mq,
                                         int f0, int f1)
{
    int bi;

    /* Draw all meshes of all bodies matching the given material flags. */

    for (bi = 0; bi < draw->bc; ++bi)
    {
        glPushMatrix();
        {
            sol_transform(draw->vary, draw->vary->bv + bi);
            mq = sol_draw_body(draw->bv + bi, mq, f0, f1);
        }
        glPopMatrix();
    }
    return mq;
}

void sol_draw_enable(void)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glClientActiveTexture(GL_TEXTURE1);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void sol_draw_disable(void)
{
    glClientActiveTexture(GL_TEXTURE1);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glClientActiveTexture(GL_TEXTURE0);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void sol_draw(const struct s_draw *draw, int mask, int test)
{
    sol_draw_enable();
    {
        const struct d_mtrl *mq = &default_draw_mtrl;

        /* Render all opaque geometry. */

        mq = sol_draw_all(draw, mq, M_TRANSPARENT | M_REFLECTIVE, 0);

        /* Render all transparent geometry. */

        if (!test) glDisable(GL_DEPTH_TEST);
        if (!mask) glDepthMask(GL_FALSE);
        {
            sol_draw_all(draw, mq, M_REFLECTIVE, M_TRANSPARENT);
        }
        if (!mask) glDepthMask(GL_TRUE);
        if (!test) glEnable(GL_DEPTH_TEST);

        mq = sol_apply_mtrl(&default_draw_mtrl, mq);

        /* Revert the buffer object state. */

        glBindBuffer(GL_ARRAY_BUFFER,         0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    sol_draw_disable();
}

void sol_refl(const struct s_draw *draw)
{
    sol_draw_enable();
    {
        const struct d_mtrl *mq = &default_draw_mtrl;

        /* Render all reflective geometry. */

        mq = sol_draw_all(draw, mq, 0, M_REFLECTIVE);
        mq = sol_apply_mtrl(&default_draw_mtrl, mq);

        /* Revert the buffer object state. */

        glBindBuffer(GL_ARRAY_BUFFER,         0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    sol_draw_disable();
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
