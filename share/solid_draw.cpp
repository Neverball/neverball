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
#include <cmath>

#include "glext.h"
#include "video.h"
#include "vec3.h"
#include "geom.h"
#include "image.h"
#include "base_image.h"
#include "config.h"
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
    { 0,                       M_REFLECTIVE | M_TRANSPARENT | M_DECAL },
    { M_DECAL,                 M_REFLECTIVE | M_TRANSPARENT },
    { M_DECAL | M_TRANSPARENT, M_REFLECTIVE },
    { M_TRANSPARENT,           M_REFLECTIVE | M_DECAL },
    { M_REFLECTIVE,            0 }
};

/*---------------------------------------------------------------------------*/

static void sol_transform(const struct s_vary *vary,
                          const struct v_body *bp, int ui)
{
    float a;
    float e[4];
    float p[3];
    float v[3];

    sol_body_p(p, vary, bp, 0.0f);
    sol_body_e(e, vary, bp, 0.0f);

    q_as_axisangle(e, v, &a);

    if (!(p[0] == 0 && p[1] == 0 && p[2] == 0)) {
        //glTranslatef(p[0], p[1], p[2]);
        ptransformer->translate(p[0], p[1], p[2]);
    }

    if (!((v[0] == 0 && v[1] == 0 && v[2] == 0) || a == 0)) {
        //glRotatef(V_DEG(a), v[0], v[1], v[2]);
        ptransformer->rotate(V_DEG(a), v[0], v[1], v[2]);
    }
}

/*---------------------------------------------------------------------------*/

static void sol_load_bill(struct s_draw *draw)
{
    draw->billVert = glcontext->createBuffer<pgl::floatv>()->data(std::vector<pgl::floatv>({
        -0.5f, 0.0f, 0.0f,
        0.5f, 0.0f, 0.0f,
        -0.5f, 1.0f, 0.0f,
        
        -0.5f, 1.0f, 0.0f,
        0.5f, 0.0f, 0.0f,
        0.5f, 1.0f, 0.0f,

        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,

        -0.5f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
    }));

    draw->billTex = glcontext->createBuffer<pgl::floatv>()->data(std::vector<pgl::floatv>({
         0.0f,  0.0f,
         1.0f,  0.0f, 
         0.0f,  1.0f,

         0.0f,  1.0f,
         1.0f,  0.0f,
         1.0f,  1.0f,

         0.0f,  0.0f,
         1.0f,  0.0f,
         0.0f,  1.0f,

         0.0f,  1.0f,
         1.0f,  0.0f,
         1.0f,  1.0f
    }));

}

static void sol_free_bill(const s_draw *draw)
{
    //glDeleteBuffers_(1, &draw->bill);
}

static void sol_draw_bill(const s_draw *draw, const int mi, GLboolean edge)
{
    //GLfloat model[16];
    //glGetFloatv(GL_MODELVIEW_MATRIX, model);

    //voffsetAccum += 0.001f;

    meshloader->setColorModifier(pgl::floatv4(1.0f));
    meshloader->setVerticeOffset(voffsetAccum);
    meshloader->setVertices(draw->billVert);
    meshloader->setTexcoords(draw->billTex);
    meshloader->setNormals(glcontext->createBuffer<pgl::floatv>()->storage(3));
    meshloader->setIndexed(false);
    //meshloader->setTransform(*(glm::mat4 *)(model));
    meshloader->setTransform(ptransformer->getCurrent());
    meshloader->setMaterialOffset(mi);
    meshloader->triangleCount = 2;

    //voffsetAccum += 0.01f;

    if (edge) {
        meshloader->setLoadingOffset(0);
    }
    else {
        meshloader->setLoadingOffset(2);
    }

    

    currentIntersector->loadMesh(meshloader);
    meshloader->setVerticeOffset(0.0f);
}

/*---------------------------------------------------------------------------*/

/* NOTE: The state management here presumes that billboard rendering is      */
/* NESTED within a wider SOL rendering process. That is: r_draw_enable       */
/* has been called and r_draw_disable will be called in the future.          */
/* Thus the "default" VBO state retained by billboard rendering is the       */
/* state appropriate for normal SOL rendering.                               */

static void sol_bill_enable(const struct s_draw *draw)
{

}

static void sol_bill_disable(void)
{

}

/*---------------------------------------------------------------------------*/

static int sol_test_mtrl(int mi, int p)
{
    const struct mtrl *mp = mtrl_get(mi);
    return ((mp->base.fl & passes[p].in) == passes[p].in &&
            (mp->base.fl & passes[p].ex) == 0);
}

/*---------------------------------------------------------------------------*/

static int sol_count_geom(const struct s_base *base, int g0, int gc, int mi)
{
    int gi, c = 0;
    for (gi = 0; gi < gc; gi++)
        if (base->gv[base->iv[g0 + gi]].mi == mi)
            c++;

    return c;
}

static int sol_count_body(const struct b_body *bp,
                          const struct s_base *base, int mi)
{
    int li, c = 0;
    for (li = 0; li < bp->lc; li++)
        c += sol_count_geom(base, base->lv[bp->l0 + li].g0,
                                  base->lv[bp->l0 + li].gc, mi);

    c += sol_count_geom(base, bp->g0, bp->gc, mi);
    return c;
}

static int sol_count_mesh(const struct d_body *bp, int p)
{
    int mi, c = 0;
    for (mi = 0; mi < bp->mc; ++mi)
        if (sol_test_mtrl(bp->mv[mi].mtrl, p))
            c++;

    return c;
}


/// NOTE
// base->ov are indexes to texcoord, normals, vertices array (ti - texcoords, si - normals, vi - vertices)
// base->gi are vertice indexes of primitives (oi - first, oj - second, ok - third)
// vv (d_vert) are re-grouped vertices
// gv (d_geom) are re-grouped triangles data (indices)
// i.e. mesh loader loading from raw data and resort to total array

/*---------------------------------------------------------------------------*/


//#ifdef T2
static void sol_mesh_vert(pgl::floatv *vp, pgl::floatv *np, pgl::floatv *tp, 
    const struct s_base *base, int oi)
{
    /* Gather all vertex attributes for the given offs. */

    const struct b_texc *tq = base->tv + base->ov[oi].ti;
    const struct b_side *sq = base->sv + base->ov[oi].si;
    const struct b_vert *vq = base->vv + base->ov[oi].vi;

    vp[0] = vq->p[0];
    vp[1] = vq->p[1];
    vp[2] = vq->p[2];

    np[0] = sq->n[0];
    np[1] = sq->n[1];
    np[2] = sq->n[2];

    tp[0] = tq->u[0];
    tp[1] = tq->u[1];
}

static void sol_mesh_geom(pgl::floatv *vv, pgl::floatv *nv, pgl::floatv *tv, int *vn,
    struct d_geom *gv, int *gn,
    const struct s_base *base, int *iv, int g0, int gc, int mi)
{
    int gi;
    for (gi = 0; gi < gc; gi++)
    {
        const struct b_geom *gq = base->gv + base->iv[g0 + gi];

        if (gq->mi == mi)
        {
            if (iv[gq->oi] == -1)
            {
                iv[gq->oi] = *vn;
                sol_mesh_vert(vv + (*vn) * 3, nv + (*vn) * 3, tv + (*vn) * 2, base, gq->oi);
                (*vn)++;
            }
            if (iv[gq->oj] == -1)
            {
                iv[gq->oj] = *vn;
                sol_mesh_vert(vv + (*vn) * 3, nv + (*vn) * 3, tv + (*vn) * 2, base, gq->oj);
                (*vn)++;
            }
            if (iv[gq->ok] == -1)
            {
                iv[gq->ok] = *vn;
                sol_mesh_vert(vv + (*vn) * 3, nv + (*vn) * 3, tv + (*vn) * 2, base, gq->ok);
                (*vn)++;
            }

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
    const size_t vs = sizeof(struct d_vert);
    const size_t gs = sizeof(struct d_geom);

    int oc = draw->base->oc;
    int vn = 0;
    int gn = 0;

    const int gc = sol_count_body(bp, draw->base, mi);

    std::vector<pgl::intv> idc(gc * 3);
    std::vector<pgl::floatv> vert(oc * 3);
    std::vector<pgl::floatv> norm(oc * 3);
    std::vector<pgl::floatv> tex(oc * 2);
    std::vector<pgl::intv> remidc(oc);

    pgl::floatv * vv = vert.data();
    pgl::floatv * nv = norm.data();
    pgl::floatv * tv = tex.data();
    d_geom * gv = (d_geom *)idc.data();
    pgl::intv * iv = (pgl::intv *)remidc.data();

    for (int i = 0; i < oc; ++i) iv[i] = -1;

    for (int li = 0; li < bp->lc; li++)
        sol_mesh_geom(vv, nv, tv, &vn, gv, &gn, draw->base, iv,
            draw->base->lv[bp->l0 + li].g0,
            draw->base->lv[bp->l0 + li].gc, mi);

    sol_mesh_geom(vv, nv, tv, &vn, gv, &gn, draw->base, iv, bp->g0, bp->gc, mi);

    pgl::Buffer<pgl::intv> ridcBuf = glcontext->createBuffer<pgl::intv>()->data(remidc);
    pgl::Buffer<pgl::intv> idcBuf = glcontext->createBuffer<pgl::intv>()->data(idc);
    pgl::Buffer<pgl::floatv> vertBuf = glcontext->createBuffer<pgl::floatv>()->data(vert);
    pgl::Buffer<pgl::floatv> normBuf = glcontext->createBuffer<pgl::floatv>()->data(norm);
    pgl::Buffer<pgl::floatv> texBuf = glcontext->createBuffer<pgl::floatv>()->data(tex);

    mp->mtrl = draw->base->mtrls[mi];
    mp->ridcBuf = ridcBuf;
    mp->idcBuf = idcBuf;
    mp->vertBuf = vertBuf;
    mp->normBuf = normBuf;
    mp->texBuf = texBuf;
    mp->ebc = gn; 
    mp->vbc = vn; 
}

static void sol_free_mesh(struct d_mesh *mp)
{
    //glDeleteBuffers_(1, &mp->ebo);
    //glDeleteBuffers_(1, &mp->vbo);
}



void sol_draw_mesh(const struct d_mesh *mp, struct s_rend *rend, int p)
{
    if (sol_test_mtrl(mp->mtrl, p))
    {
        //GLfloat model[16];
        //glGetFloatv(GL_MODELVIEW_MATRIX, model);

        pgl::floatv4 cmod;
        glGetFloatv(GL_CURRENT_COLOR, (float *)&cmod);

        meshloader->setColorModifier(cmod);
        meshloader->setVerticeOffset((PASS_OPAQUE_DECAL == p || PASS_TRANSPARENT_DECAL == p) ? 0.002f : 0.0f);
        meshloader->setMaterialOffset(mp->mtrl);
        meshloader->setVertices(mp->vertBuf);
        meshloader->setTexcoords(mp->texBuf);
        meshloader->setNormals(mp->normBuf);
        meshloader->setIndices(mp->idcBuf);
        //meshloader->setTransform(*(glm::mat4 *)(model));
        meshloader->setTransform(ptransformer->getCurrent());
        meshloader->setLoadingOffset(0);
        meshloader->triangleCount = mp->ebc;

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        if (mtrl_get(mp->mtrl)->base.fl & M_PARTICLE) {
            meshloader->setIndexed(false);
        }
        else {
            meshloader->setIndexed(true);
            currentIntersector->loadMesh(meshloader);
        }

        meshloader->setVerticeOffset(0.0f);

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

    for (mi = 0; mi < draw->base->mc; ++mi)
        if (sol_count_body(bq, draw->base, mi))
            bp->mc++;

    if ((bp->mv = (struct d_mesh *) calloc(bp->mc, sizeof (struct d_mesh))))
    {
        int mj = 0;

        for (mi = 0; mi < draw->base->mc; ++mi)
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

int sol_load_draw(struct s_draw *draw, struct s_vary *vary, int s)
{
    int i;

    memset(draw, 0, sizeof (struct s_draw));

    draw->vary = vary;
    draw->base = vary->base;

    /* Determine whether this file has reflective materials. */

    for (i = 0; i < draw->base->mc; i++)
        if (draw->base->mv[i].fl & M_REFLECTIVE)
        {
            draw->reflective = 1;
            break;
        }

    /* Cache all materials for this file. */

    mtrl_cache_sol(draw->base);

    /* Initialize shadow state. */

    draw->shadow_ui = -1;
    draw->shadowed = s;

    /* Initialize all bodies for this file. */

    if (draw->base->bc)
    {
        if ((draw->bv = (d_body *)calloc(draw->base->bc, sizeof (*draw->bv))))
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

    mtrl_free_sol(draw->base);

    sol_free_bill(draw);

    for (i = 0; i < draw->bc; i++)
        sol_free_body(draw->bv + i);

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
            //glPushMatrix();
            //{
                ptransformer->push();
                sol_transform(draw->vary, draw->vary->bv + bi, draw->shadow_ui);
                sol_draw_body(draw->bv + bi, rend, p);
                ptransformer->pop();

            //}
            //glPopMatrix();
        }
}

/*---------------------------------------------------------------------------*/

void sol_draw(const struct s_draw *draw, struct s_rend *rend, int mask, int test)
{
    rend->skip_flags |= (draw->shadowed ? 0 : M_SHADOWED);
    sol_draw_all(draw, rend, PASS_OPAQUE);
    sol_draw_all(draw, rend, PASS_OPAQUE_DECAL);
    sol_draw_all(draw, rend, PASS_TRANSPARENT_DECAL);
    sol_draw_all(draw, rend, PASS_TRANSPARENT);
    rend->skip_flags = 0;
}

void sol_refl(const struct s_draw *draw, struct s_rend *rend)
{
    /* Disable shadowed material setup if not requested. */

    rend->skip_flags |= (draw->shadowed ? 0 : M_SHADOWED);

    /* Render all reflective geometry. */

    sol_draw_all(draw, rend, PASS_REFLECTIVE);

    /* Revert the buffer object state. */

    glBindBuffer_(GL_ARRAY_BUFFER,         0);
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, 0);

    rend->skip_flags = 0;
}

void sol_back(const struct s_draw *draw,
              struct s_rend *rend,
              float n, float f, float t)
{
    if (!(draw && draw->base && draw->base->rc)) return;

    int ri;

    //for (ri = 0; ri < draw->base->rc; ri++)
    for (ri = draw->base->rc - 1;ri >= 0;ri--) 
    {
        const struct b_bill *rp = draw->base->rv + ri;
        if (n <= rp->d && rp->d < f)
        {
            float T = (rp->t > 0.0f) ? (fmodf(t, rp->t) - rp->t / 2) : 0;

            float w = rp->w[0] + rp->w[1] * T + rp->w[2] * T * T;
            float h = rp->h[0] + rp->h[1] * T + rp->h[2] * T * T;

            if (w > 0 && h > 0)
            {
                float rx = rp->rx[0] + rp->rx[1] * T + rp->rx[2] * T * T;
                float ry = rp->ry[0] + rp->ry[1] * T + rp->ry[2] * T * T;
                float rz = rp->rz[0] + rp->rz[1] * T + rp->rz[2] * T * T;

                //glMatrixMode(GL_MODELVIEW);
                //glLoadIdentity();
                //glPushMatrix();

                ptransformer->push();
                {
                    const int mi = draw->base->mtrls[rp->mi];
                    const mtrl *mp = mtrl_get(mi);

                    //if (ry) glRotatef(ry, 0.0f, 1.0f, 0.0f);
                    //if (rx) glRotatef(rx, 1.0f, 0.0f, 0.0f);
                    //glTranslatef(0.0f, 0.0f, -rp->d);
                    //if (rp->fl & B_FLAT)
                    //{
                    //    glRotatef(-rx - 90.0f, 1.0f, 0.0f, 0.0f);
                    //    glRotatef(-ry, 0.0f, 0.0f, 1.0f);
                    //}
                    //if (rp->fl & B_EDGE) glRotatef(-rx, 1.0f, 0.0f, 0.0f);
                    //if (rz) glRotatef(rz, 0.0f, 0.0f, 1.0f);
                    //glScalef(w, h, 1.0f);

                    if (ry) ptransformer->rotate(ry, 0.0f, 1.0f, 0.0f);
                    if (rx) ptransformer->rotate(rx, 1.0f, 0.0f, 0.0f);
                    ptransformer->translate(0.0f, 0.0f, -rp->d);
                    if (rp->fl & B_FLAT)
                    {
                        ptransformer->rotate(-rx - 90.0f, 1.0f, 0.0f, 0.0f);
                        ptransformer->rotate(-ry, 0.0f, 0.0f, 1.0f);
                    }
                    if (rp->fl & B_EDGE) ptransformer->rotate(-rx, 1.0f, 0.0f, 0.0f);
                    if (rz) ptransformer->rotate(rz, 0.0f, 0.0f, 1.0f);
                    ptransformer->scale(w, h, 1.0f);

                    sol_draw_bill(draw, mi, rp->fl & B_EDGE);
                }
                ptransformer->pop();

                //glPopMatrix();
            }
        }
    }
}

void sol_bill(const struct s_draw *draw,
              struct s_rend *rend, const float *M, float t)
{
    if (!(draw && draw->base && draw->base->rc)) return;

    int ri;

    for (ri = draw->base->rc - 1;ri >= 0;ri--)
    {
        const struct b_bill *rp = draw->base->rv + ri;

        float T = rp->t * t;
        float S = fsinf(T);

        float w = rp->w[0] + rp->w[1] * T + rp->w[2] * S;
        float h = rp->h[0] + rp->h[1] * T + rp->h[2] * S;
        float rx = rp->rx[0] + rp->rx[1] * T + rp->rx[2] * S;
        float ry = rp->ry[0] + rp->ry[1] * T + rp->ry[2] * S;
        float rz = rp->rz[0] + rp->rz[1] * T + rp->rz[2] * S;

        //glMatrixMode(GL_MODELVIEW);
        //glLoadIdentity();
        //glPushMatrix();
        ptransformer->push();
        {
            //glTranslatef(rp->p[0], rp->p[1], rp->p[2]);
            //if (M && ((rp->fl & B_NOFACE) == 0)) glMultMatrixf(M);
            //if (fabsf(rx) > 0.0f) glRotatef(rx, 1.0f, 0.0f, 0.0f);
            //if (fabsf(ry) > 0.0f) glRotatef(ry, 0.0f, 1.0f, 0.0f);
            //if (fabsf(rz) > 0.0f) glRotatef(rz, 0.0f, 0.0f, 1.0f);
            //glScalef(w, h, 1.0f);

            ptransformer->translate(rp->p[0], rp->p[1], rp->p[2]);
            if (M && ((rp->fl & B_NOFACE) == 0)) ptransformer->multiply(M);
            if (fabsf(rx) > 0.0f) ptransformer->rotate(rx, 1.0f, 0.0f, 0.0f);
            if (fabsf(ry) > 0.0f) ptransformer->rotate(ry, 0.0f, 1.0f, 0.0f);
            if (fabsf(rz) > 0.0f) ptransformer->rotate(rz, 0.0f, 0.0f, 1.0f);
            ptransformer->scale(w, h, 1.0f);

            sol_draw_bill(draw, draw->base->mtrls[rp->mi], GL_FALSE);
        }
        ptransformer->pop();
        //glPopMatrix();
    }
}

void sol_fade(const struct s_draw *draw, struct s_rend *rend, float k)
{
}

/*---------------------------------------------------------------------------*/

int sol_load_full(struct s_full *full, const char *filename, int s)
{
    if (full)
    {
        memset(full, 0, sizeof (*full));
        if (sol_load_base(&full->base, filename))
        {
            sol_load_vary(&full->vary, &full->base);
            sol_load_draw(&full->draw, &full->vary, s);
            return 1;
        }
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

static void assert_mtrl(const struct mtrl *mp)
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

void r_color_mtrl(struct s_rend *rend, int enable)
{
}

void r_apply_mtrl(struct s_rend *rend, int mi)
{
}

void r_draw_enable(struct s_rend *rend)
{
}

void r_draw_disable(struct s_rend *rend)
{
}

/*---------------------------------------------------------------------------*/
