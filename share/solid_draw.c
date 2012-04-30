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

/*
 * Included and excluded material flags for each rendering pass.
 */

static const struct
{
    int in;
    int ex;
} passes[PASS_MAX] = {
    { 0, M_REFLECTIVE | M_TRANSPARENT | M_DECAL },
    { M_DECAL, M_REFLECTIVE | M_TRANSPARENT },
    { M_DECAL | M_TRANSPARENT, M_REFLECTIVE },
    { M_TRANSPARENT, M_REFLECTIVE | M_DECAL },
    { M_REFLECTIVE, 0 }
};

/*---------------------------------------------------------------------------*/

static void sol_transform(const struct s_vary *vary,
                          const struct v_body *bp, int ui)
{
    float a;
    float e[4];
    float p[3];
    float v[3];

    /* Apply the body position and rotation to the model-view matrix. */

    sol_body_p(p, vary, bp, 0.0f);
    sol_body_e(e, vary, bp, 0.0f);

    q_as_axisangle(e, v, &a);

    if (!(p[0] == 0 && p[1] == 0 && p[2] == 0))
        glTranslatef(p[0], p[1], p[2]);

    if (!((v[0] == 0 && v[1] == 0 && v[2] == 0) || a == 0))
        glRotatef(V_DEG(a), v[0], v[1], v[2]);

    /* Apply the shadow transform to the texture matrix. */

    if (ui >= 0 && ui < vary->uc && vary->uv[ui].r > 0.0f)
    {
        struct v_ball *up = vary->uv + ui;

        if (tex_env_stage(TEX_STAGE_SHADOW))
        {
            glMatrixMode(GL_TEXTURE);
            {
                float k = 0.25f / up->r;

                glLoadIdentity();

                /* Center the shadow texture on the ball. */

                glTranslatef(0.5f, 0.5f, 0.0f);

                /* Transform ball XZ position to ST texture coordinate. */

                glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

                /* Scale the shadow texture to the radius of the ball. */

                glScalef(k, k, k);

                /* Move the shadow texture under the ball. */

                glTranslatef(-up->p[0], -up->p[1], -up->p[2]);

                /* Apply the body position and rotation. */

                glTranslatef(p[0], p[1], p[2]);
                glRotatef(V_DEG(a), v[0], v[1], v[2]);

                /* Vertically center clipper texture on ball position. */

                if (tex_env_stage(TEX_STAGE_CLIP))
                {
                    glLoadIdentity();
                    glTranslatef(p[0] - up->p[0],
                                 p[1] - up->p[1] + 0.5f,
                                 p[2] - up->p[2]);
                    glRotatef(V_DEG(a), v[0], v[1], v[2]);

                }
            }
            glMatrixMode(GL_MODELVIEW);

            tex_env_stage(TEX_STAGE_TEXTURE);
        }
    }
}

/*---------------------------------------------------------------------------*/

static void sol_load_bill(struct s_draw *draw)
{
    static const GLfloat data[] = {
        0.0f,  0.0f, -0.5f,  0.0f,
        1.0f,  0.0f,  0.5f,  0.0f,
        0.0f,  1.0f, -0.5f,  1.0f,
        1.0f,  1.0f,  0.5f,  1.0f,

        0.0f,  0.0f, -0.5f, -0.5f,
        1.0f,  0.0f,  0.5f, -0.5f,
        0.0f,  1.0f, -0.5f,  0.5f,
        1.0f,  1.0f,  0.5f,  0.5f,
    };

    /* Initialize a vertex buffer object for billboard drawing. */

    glGenBuffers_(1,              &draw->bill);
    glBindBuffer_(GL_ARRAY_BUFFER, draw->bill);
    glBufferData_(GL_ARRAY_BUFFER, sizeof (data), data, GL_STATIC_DRAW);
    glBindBuffer_(GL_ARRAY_BUFFER, 0);
}

static void sol_free_bill(struct s_draw *draw)
{
    glDeleteBuffers_(1, &draw->bill);
}

static void sol_draw_bill(GLboolean edge)
{
    if (edge)
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    else
        glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
}

/*---------------------------------------------------------------------------*/

/* NOTE: The state management here presumes that billboard rendering is      */
/* NESTED within a wider SOL rendering process. That is: sol_draw_enable     */
/* has been called and sol_draw_disable will be called in the future.        */
/* Thus the "default" VBO state retained by billboard rendering is the       */
/* state appropriate for normal SOL rendering.                               */

static void sol_bill_enable(const struct s_draw *draw)
{
    const size_t s = sizeof (GLfloat);

    glBindBuffer_(GL_ARRAY_BUFFER, draw->bill);

    glDisableClientState(GL_NORMAL_ARRAY);

    glTexCoordPointer(2, GL_FLOAT, s * 4, (GLvoid *) (    0));
    glVertexPointer  (2, GL_FLOAT, s * 4, (GLvoid *) (s * 2));
}

static void sol_bill_disable(void)
{
    glEnableClientState(GL_NORMAL_ARRAY);

    glBindBuffer_(GL_ARRAY_BUFFER, 0);
}

/*---------------------------------------------------------------------------*/

#define tobyte(f) ((GLubyte) (f * 255.0f))

static struct b_mtrl default_base_mtrl =
{
    { 0.8f, 0.8f, 0.8f, 1.0f },
    { 0.2f, 0.2f, 0.2f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f }, 0.0f, 0, ""
};

/* Nasty. */

static struct d_mtrl default_draw_mtrl =
{
    &default_base_mtrl,
    0xffcccccc,
    0xff333333,
    0xff000000,
    0xff000000,
    0x00000000,
    0
};

#if DEBUG_MTRL
static void check_mtrl(const char *name, GLenum pname, GLuint curr)
{
    static char buff[64];

    GLuint real;
    GLfloat v[4];

    glGetMaterialfv(GL_FRONT, pname, v);

    if (pname != GL_SHININESS)
        real = (tobyte(v[0])       |
                tobyte(v[1]) << 8  |
                tobyte(v[2]) << 16 |
                tobyte(v[3]) << 24);
    else
        real = (tobyte(v[0]));

    if (real != curr)
    {
        sprintf(buff, "%s mismatch (0x%08X -> 0x%08X)", name, real, curr);
        glStringMarker_(buff);
    }
}

static void assert_mtrl(const struct d_mtrl *mp)
{
    if (glIsEnabled(GL_COLOR_MATERIAL))
        return;

    check_mtrl("ambient",   GL_AMBIENT,   mp->a);
    check_mtrl("diffuse",   GL_DIFFUSE,   mp->d);
    check_mtrl("specular",  GL_SPECULAR,  mp->s);
    check_mtrl("emission",  GL_EMISSION,  mp->e);
    check_mtrl("shininess", GL_SHININESS, mp->h);
}
#endif

void sol_color_mtrl(struct s_rend *rend, int enable)
{
    if (enable)
    {
        glEnable(GL_COLOR_MATERIAL);

        rend->color_mtrl = 1;
    }
    else
    {
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glDisable(GL_COLOR_MATERIAL);

        /* This keeps material tracking synchronized with GL state. */

        rend->mtrl.d = 0xffffffff;
        rend->mtrl.a = 0xffffffff;

        rend->color_mtrl = 0;
    }
}

void sol_apply_mtrl(const struct d_mtrl *mp_draw, struct s_rend *rend)
{
    const struct b_mtrl *mp_base =  mp_draw->base;
    const struct d_mtrl *mq_draw = &rend->mtrl;

    int mp_flags = rend->shadow ? mp_base->fl : mp_base->fl & ~M_SHADOWED;
    int mq_flags = rend->flags;

#if DEBUG_MTRL
    assert_mtrl(&rend->mtrl);
#endif

    /* Bind the texture. */

    if (mp_draw->o != mq_draw->o)
        glBindTexture(GL_TEXTURE_2D, mp_draw->o);

    /* Set material properties. */

    if (mp_draw->d != mq_draw->d && !rend->color_mtrl)
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mp_base->d);
    if (mp_draw->a != mq_draw->a && !rend->color_mtrl)
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   mp_base->a);
    if (mp_draw->s != mq_draw->s)
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mp_base->s);
    if (mp_draw->e != mq_draw->e)
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  mp_base->e);
    if (mp_draw->h != mq_draw->h)
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mp_base->h);

    /* Ball shadow. */

    if ((mp_flags & M_SHADOWED) ^ (mq_flags & M_SHADOWED))
    {
        if (mp_flags & M_SHADOWED)
            shad_draw_set();
        else
            shad_draw_clr();
    }

    /* Environment mapping. */

#if !ENABLE_OPENGLES
    if ((mp_flags & M_ENVIRONMENT) ^ (mq_flags & M_ENVIRONMENT))
    {
        if (mp_flags & M_ENVIRONMENT)
        {
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);

            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        }
        else
        {
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
    }
#endif

    /* Additive blending. */

    if ((mp_flags & M_ADDITIVE) ^ (mq_flags & M_ADDITIVE))
    {
        if (mp_flags & M_ADDITIVE)
            glBlendFunc(GL_ONE, GL_ONE);
        else
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    /* Visibility-from-behind. */

    if ((mp_flags & M_TWO_SIDED) ^ (mq_flags & M_TWO_SIDED))
    {
        if (mp_flags & M_TWO_SIDED)
        {
            glDisable(GL_CULL_FACE);
            glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1);
        }
        else
        {
            glEnable(GL_CULL_FACE);
            glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 0);
        }
    }

    /* Decal offset. */

    if ((mp_flags & M_DECAL) ^ (mq_flags & M_DECAL))
    {
        if (mp_flags & M_DECAL)
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-1.0f, -2.0f);
        }
        else
            glDisable(GL_POLYGON_OFFSET_FILL);
    }

    rend->mtrl = *mp_draw;
    rend->flags = mp_flags;
}

static GLuint sol_find_texture(const char *name)
{
    char path[MAXSTR];
    GLuint o;
    int i;

    for (i = 0; i < ARRAYSIZE(tex_paths); i++)
    {
        CONCAT_PATH(path, &tex_paths[i], name);

        if ((o = make_image_from_file(path, IF_MIPMAP)))
            return o;
    }
    return 0;
}

void sol_load_mtrl(struct d_mtrl *mp, const struct b_mtrl *mq)
{
    mp->base = mq;

    if ((mp->o = sol_find_texture(_(mq->f))))
    {
        /* Set the texture to clamp or repeat based on material type. */

        if (mq->fl & M_CLAMP_S)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        else
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

        if (mq->fl & M_CLAMP_T)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        else
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    /* Cache the 32-bit material values for quick comparison. */

    mp->d = (tobyte(mq->d[0]))
        |   (tobyte(mq->d[1]) <<  8)
        |   (tobyte(mq->d[2]) << 16)
        |   (tobyte(mq->d[3]) << 24);
    mp->a = (tobyte(mq->a[0]))
        |   (tobyte(mq->a[1]) <<  8)
        |   (tobyte(mq->a[2]) << 16)
        |   (tobyte(mq->a[3]) << 24);
    mp->s = (tobyte(mq->s[0]))
        |   (tobyte(mq->s[1]) <<  8)
        |   (tobyte(mq->s[2]) << 16)
        |   (tobyte(mq->s[3]) << 24);
    mp->e = (tobyte(mq->e[0]))
        |   (tobyte(mq->e[1]) <<  8)
        |   (tobyte(mq->e[2]) << 16)
        |   (tobyte(mq->e[3]) << 24);
    mp->h = (tobyte(mq->h[0]));
}

void sol_free_mtrl(struct d_mtrl *mp)
{
    glDeleteTextures(1, &mp->o);
}

static int sol_test_mtrl(const struct d_mtrl *mp, int p)
{
    /* Test whether the material flags match inclusion rules. */

    return ((mp->base->fl & passes[p].in) == passes[p].in &&
            (mp->base->fl & passes[p].ex) == 0);
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

static int sol_count_mesh(const struct d_body *bp, int p)
{
    int mi, c = 0;

    /* Count the body meshes matching the given material flags. */

    for (mi = 0; mi < bp->mc; ++mi)
        if (sol_test_mtrl(bp->mv[mi].mp, p))
            c++;

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

        glGenBuffers_(1, &mp->vbo);
        glBindBuffer_(GL_ARRAY_BUFFER,         mp->vbo);
        glBufferData_(GL_ARRAY_BUFFER,         vn * vs, vv, GL_STATIC_DRAW);
        glBindBuffer_(GL_ARRAY_BUFFER,         0);

        glGenBuffers_(1, &mp->ebo);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, mp->ebo);
        glBufferData_(GL_ELEMENT_ARRAY_BUFFER, gn * gs, gv, GL_STATIC_DRAW);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, 0);

        mp->mp  = draw->mv + mi;
        mp->ebc = gn * 3;
    }

    free(iv);
    free(gv);
    free(vv);
}

static void sol_free_mesh(struct d_mesh *mp)
{
    glDeleteBuffers_(1, &mp->ebo);
    glDeleteBuffers_(1, &mp->vbo);
}

void sol_draw_mesh(const struct d_mesh *mp, struct s_rend *rend, int p)
{
    /* If this mesh has material matching the given flags... */

    if (sol_test_mtrl(mp->mp, p))
    {
        const size_t s = sizeof (struct d_vert);
        const GLenum T = GL_FLOAT;

        /* Apply the material state. */

        sol_apply_mtrl(mp->mp, rend);

        /* Bind the mesh data. */

        glBindBuffer_(GL_ARRAY_BUFFER,         mp->vbo);
        glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, mp->ebo);

        glVertexPointer  (3, T, s, (GLvoid *) offsetof (struct d_vert, p));
        glNormalPointer  (   T, s, (GLvoid *) offsetof (struct d_vert, n));

        if (tex_env_stage(TEX_STAGE_SHADOW))
        {
            glTexCoordPointer(3, T, s, (GLvoid *) offsetof (struct d_vert, p));

            if (tex_env_stage(TEX_STAGE_CLIP))
                glTexCoordPointer(3, T, s, (GLvoid *) offsetof (struct d_vert, p));

            tex_env_stage(TEX_STAGE_TEXTURE);
        }
        glTexCoordPointer(2, T, s, (GLvoid *) offsetof (struct d_vert, t));

        /* Draw the mesh. */

        glDrawElements(GL_TRIANGLES, mp->ebc, GL_UNSIGNED_SHORT, 0);
    }
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

    /* Cache a mesh count for each pass. */

    bp->pass[0] = sol_count_mesh(bp, 0);
    bp->pass[1] = sol_count_mesh(bp, 1);
    bp->pass[2] = sol_count_mesh(bp, 2); 
    bp->pass[3] = sol_count_mesh(bp, 3); 
    bp->pass[4] = sol_count_mesh(bp, 4);
}

static void sol_free_body(struct d_body *bp)
{
    int mi;

    for (mi = 0; mi < bp->mc; ++mi)
        sol_free_mesh(bp->mv + mi);

    free(bp->mv);
}

static void sol_draw_body(const struct d_body *bp, struct s_rend *rend, int p)
{
    int i;

    for (i = 0; i < bp->mc; ++i)
        sol_draw_mesh(bp->mv + i, rend, p);
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
            {
                sol_load_mtrl(draw->mv + i, draw->base->mv + i);

                /* If at least one material is reflective, mark it. */

                if (draw->base->mv[i].fl & M_REFLECTIVE)
                    draw->reflective = 1;
            }
        }
    }

    /* Initialize shadow state. */

    draw->shadow_ui = -1;
    draw->shadowed = s;

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

    sol_load_bill(draw);

    return 1;
}

void sol_free_draw(struct s_draw *draw)
{
    int i;

    sol_free_bill(draw);

    for (i = 0; i < draw->mc; i++)
        sol_free_mtrl(draw->mv + i);
    for (i = 0; i < draw->bc; i++)
        sol_free_body(draw->bv + i);

    free(draw->mv);
    free(draw->bv);
}

/*---------------------------------------------------------------------------*/

static void sol_draw_all(const struct s_draw *draw, struct s_rend *rend, int p)
{
    int bi;

    /* Draw all meshes of all bodies matching the given material flags. */

    for (bi = 0; bi < draw->bc; ++bi)
        if (draw->bv[bi].pass[p])
        {
            glPushMatrix();
            {
                sol_transform(draw->vary, draw->vary->bv + bi, draw->shadow_ui);
                sol_draw_body(draw->bv + bi, rend, p);
            }
            glPopMatrix();
        }
}

void sol_draw_enable(struct s_rend *rend)
{
    memset(rend, 0, sizeof (*rend));

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindTexture(GL_TEXTURE_2D, 0);

    rend->mtrl = default_draw_mtrl;
    rend->flags = default_base_mtrl.fl;
}

void sol_draw_disable(struct s_rend *rend)
{
    sol_apply_mtrl(&default_draw_mtrl, rend);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

/*---------------------------------------------------------------------------*/

void sol_draw(const struct s_draw *draw, struct s_rend *rend, int mask, int test)
{
    /* Pass use-shadow-flag to material tracking. */

    rend->shadow = draw->shadowed;

    /* Render all opaque geometry, decals last. */

    sol_draw_all(draw, rend, PASS_OPAQUE);
    sol_draw_all(draw, rend, PASS_OPAQUE_DECAL);

    /* Render all transparent geometry, decals first. */

    if (!test) glDisable(GL_DEPTH_TEST);
    if (!mask) glDepthMask(GL_FALSE);
    {
        sol_draw_all(draw, rend, PASS_TRANSPARENT_DECAL);
        sol_draw_all(draw, rend, PASS_TRANSPARENT);
    }
    if (!mask) glDepthMask(GL_TRUE);
    if (!test) glEnable(GL_DEPTH_TEST);

    /* Revert the buffer object state. */

    glBindBuffer_(GL_ARRAY_BUFFER,         0);
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, 0);

    rend->shadow = 0;
}

void sol_refl(const struct s_draw *draw, struct s_rend *rend)
{
    /* Pass use-shadow-flag to material tracking. */

    rend->shadow = draw->shadowed;

    /* Render all reflective geometry. */

    sol_draw_all(draw, rend, PASS_REFLECTIVE);

    /* Revert the buffer object state. */

    glBindBuffer_(GL_ARRAY_BUFFER,         0);
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, 0);

    rend->shadow = 0;
}

void sol_back(const struct s_draw *draw,
              struct s_rend *rend,
              float n, float f, float t)
{
    if (!(draw && draw->base && draw->base->rc))
        return;

    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);

    sol_bill_enable(draw);
    {
        int ri;

        /* Consider each billboard. */

        for (ri = 0; ri < draw->base->rc; ri++)
        {
            const struct b_bill *rp = draw->base->rv + ri;

            /* Render only billboards at distances between n and f. */

            if (n <= rp->d && rp->d < f)
            {
                float T = (rp->t > 0.0f) ? (fmodf(t, rp->t) - rp->t / 2) : 0;

                float w = rp->w[0] + rp->w[1] * T + rp->w[2] * T * T;
                float h = rp->h[0] + rp->h[1] * T + rp->h[2] * T * T;

                /* Render only billboards facing the viewer. */

                if (w > 0 && h > 0)
                {
                    float rx = rp->rx[0] + rp->rx[1] * T + rp->rx[2] * T * T;
                    float ry = rp->ry[0] + rp->ry[1] * T + rp->ry[2] * T * T;
                    float rz = rp->rz[0] + rp->rz[1] * T + rp->rz[2] * T * T;

                    sol_apply_mtrl(draw->mv + rp->mi, rend);

                    glPushMatrix();
                    {
                        if (ry) glRotatef(ry, 0.0f, 1.0f, 0.0f);
                        if (rx) glRotatef(rx, 1.0f, 0.0f, 0.0f);

                        glTranslatef(0.0f, 0.0f, -rp->d);

                        if (rp->fl & B_FLAT)
                        {
                            glRotatef(-rx - 90.0f, 1.0f, 0.0f, 0.0f);
                            glRotatef(-ry,         0.0f, 0.0f, 1.0f);
                        }
                        if (rp->fl & B_EDGE)
                            glRotatef(-rx,         1.0f, 0.0f, 0.0f);

                        if (rz) glRotatef(rz, 0.0f, 0.0f, 1.0f);

                        glScalef(w, h, 1.0f);

                        sol_draw_bill(rp->fl & B_EDGE);
                    }
                    glPopMatrix();
                }
            }
        }
    }
    sol_bill_disable();

    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
}

void sol_bill(const struct s_draw *draw,
              struct s_rend *rend, const float *M, float t)
{
    if (!(draw && draw->base && draw->base->rc))
        return;

    sol_bill_enable(draw);
    {
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

            sol_apply_mtrl(draw->mv + rp->mi, rend);

            glPushMatrix();
            {
                glTranslatef(rp->p[0], rp->p[1], rp->p[2]);

                if (M && ((rp->fl & B_NOFACE) == 0)) glMultMatrixf(M);

                if (fabsf(rx) > 0.0f) glRotatef(rx, 1.0f, 0.0f, 0.0f);
                if (fabsf(ry) > 0.0f) glRotatef(ry, 0.0f, 1.0f, 0.0f);
                if (fabsf(rz) > 0.0f) glRotatef(rz, 0.0f, 0.0f, 1.0f);

                glScalef(w, h, 1.0f);

                sol_draw_bill(GL_FALSE);
            }
            glPopMatrix();
        }
    }
    sol_bill_disable();
}

void sol_fade(const struct s_draw *draw, struct s_rend *rend, float k)
{
    if (k > 0.0f)
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        {
            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_TEXTURE_2D);

            glColor4f(0.0f, 0.0f, 0.0f, k);

            sol_bill_enable(draw);
            sol_apply_mtrl(&default_draw_mtrl, rend);
            glScalef(2.0f, 2.0f, 1.0f);
            sol_draw_bill(GL_FALSE);
            sol_bill_disable();

            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

            glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_LIGHTING);
        }
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
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
