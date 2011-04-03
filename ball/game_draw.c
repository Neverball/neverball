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

#include "vec3.h"
#include "glext.h"
#include "ball.h"
#include "item.h"
#include "part.h"
#include "geom.h"
#include "config.h"
#include "video.h"

#include "solid_draw.h"

#include "game_draw.h"

/*---------------------------------------------------------------------------*/

static const struct d_mtrl *game_draw_balls(const struct d_mtrl *mq,
                                            const struct s_vary *vary,
                                            const float *bill_M, float t)
{
    float c[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    float ball_M[16];
    float pend_M[16];

    m_basis(ball_M, vary->uv[0].e[0], vary->uv[0].e[1], vary->uv[0].e[2]);
    m_basis(pend_M, vary->uv[0].E[0], vary->uv[0].E[1], vary->uv[0].E[2]);

    glPushAttrib(GL_LIGHTING_BIT);
    glPushMatrix();
    {
        glTranslatef(vary->uv[0].p[0],
                     vary->uv[0].p[1] + BALL_FUDGE,
                     vary->uv[0].p[2]);
        glScalef(vary->uv[0].r,
                 vary->uv[0].r,
                 vary->uv[0].r);

        glColor4fv(c);
        mq = ball_draw(mq, ball_M, pend_M, bill_M, t);
    }
    glPopMatrix();
    glPopAttrib();

    return mq;
}

static const struct d_mtrl *game_draw_items(const struct d_mtrl *mq,
                                            const struct s_vary *vary,
                                            const float *bill_M, float t)
{
    int hi;

    glPushAttrib(GL_LIGHTING_BIT);
    {
        item_push(ITEM_COIN);
        {
            for (hi = 0; hi < vary->hc; hi++)

                if (vary->hv[hi].t == ITEM_COIN && vary->hv[hi].n > 0)
                {
                    glPushMatrix();
                    {
                        glTranslatef(vary->hv[hi].p[0],
                                     vary->hv[hi].p[1],
                                     vary->hv[hi].p[2]);
                        mq = item_draw(mq, &vary->hv[hi], bill_M, t);
                    }
                    glPopMatrix();
                }
        }
        item_pull();

        item_push(ITEM_SHRINK);
        {
            for (hi = 0; hi < vary->hc; hi++)

                if (vary->hv[hi].t == ITEM_SHRINK)
                {
                    glPushMatrix();
                    {
                        glTranslatef(vary->hv[hi].p[0],
                                     vary->hv[hi].p[1],
                                     vary->hv[hi].p[2]);
                        mq = item_draw(mq, &vary->hv[hi], bill_M, t);
                    }
                    glPopMatrix();
                }
        }
        item_pull();

        item_push(ITEM_GROW);
        {
            for (hi = 0; hi < vary->hc; hi++)

                if (vary->hv[hi].t == ITEM_GROW)
                {
                    glPushMatrix();
                    {
                        glTranslatef(vary->hv[hi].p[0],
                                     vary->hv[hi].p[1],
                                     vary->hv[hi].p[2]);
                        mq = item_draw(mq, &vary->hv[hi], bill_M, t);
                    }
                    glPopMatrix();
                }
        }
        item_pull();
    }
    glPopAttrib();

    return mq;
}

static const struct d_mtrl *game_draw_goals(const struct d_mtrl *mq,
                                            const struct game_draw *gd,
                                            const float *M, float t)
{
    const struct s_base *base = gd->vary.base;

    if (gd->goal_e)
    {
        int zi;

        /* Draw the goal column. */

        for (zi = 0; zi < base->zc; zi++)
        {
            glPushMatrix();
            {
                glTranslatef(base->zv[zi].p[0],
                             base->zv[zi].p[1],
                             base->zv[zi].p[2]);

                glScalef(base->zv[zi].r,
                         gd->goal_k,
                         base->zv[zi].r);

                mq = goal_draw(mq, t);
            }
            glPopMatrix();
        }
    }
    return mq;
}

static const struct d_mtrl *game_draw_jumps(const struct d_mtrl *mq,
                                            const struct game_draw *gd,
                                            const float *M, float t)
{
    const struct s_base *base = gd->vary.base;

    int ji;

    for (ji = 0; ji < base->jc; ji++)
    {
        glPushMatrix();
        {
            glTranslatef(base->jv[ji].p[0],
                         base->jv[ji].p[1],
                         base->jv[ji].p[2]);
            glScalef(base->jv[ji].r,
                     1.0f,
                     base->jv[ji].r);

            mq = jump_draw(mq, t, !gd->jump_e);
        }
        glPopMatrix();
    }
    return mq;
}

static const struct d_mtrl *game_draw_swchs(const struct d_mtrl *mq,
                                            const struct s_vary *vary)
{
    int xi;

    for (xi = 0; xi < vary->xc; xi++)
    {
        struct v_swch *xp = vary->xv + xi;

        if (xp->base->i)
            continue;

        glPushMatrix();
        {
            glTranslatef(xp->base->p[0],
                         xp->base->p[1],
                         xp->base->p[2]);
            glScalef(xp->base->r,
                     1.0f,
                     xp->base->r);

            mq = swch_draw(mq, xp->f, xp->e);
        }
        glPopMatrix();
    }
    return mq;
}

/*---------------------------------------------------------------------------*/

static void game_draw_tilt(const struct game_draw *gd, int d)
{
    const struct game_tilt *tilt = &gd->tilt;
    const float *ball_p = gd->vary.uv[0].p;

    /* Rotate the environment about the position of the ball. */

    glTranslatef(+ball_p[0], +ball_p[1] * d, +ball_p[2]);
    glRotatef(-tilt->rz * d, tilt->z[0], tilt->z[1], tilt->z[2]);
    glRotatef(-tilt->rx * d, tilt->x[0], tilt->x[1], tilt->x[2]);
    glTranslatef(-ball_p[0], -ball_p[1] * d, -ball_p[2]);
}

static const struct d_mtrl *game_refl_all(const struct d_mtrl *mq,
                                          const struct game_draw *gd)
{
    glPushMatrix();
    {
        game_draw_tilt(gd, 1);

        /* Draw the floor. */

        mq = sol_refl(&gd->draw, mq);
    }
    glPopMatrix();

    return mq;
}

/*---------------------------------------------------------------------------*/

static void game_draw_light(void)
{
    const float light_p[2][4] = {
        { -8.0f, +32.0f, -8.0f, 0.0f },
        { +8.0f, +32.0f, +8.0f, 0.0f },
    };
    const float light_c[2][4] = {
        { 1.0f, 0.8f, 0.8f, 1.0f },
        { 0.8f, 1.0f, 0.8f, 1.0f },
    };

    /* Configure the lighting. */

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_p[0]);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_c[0]);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_c[0]);

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_POSITION, light_p[1]);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  light_c[1]);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_c[1]);
}

static const struct d_mtrl *game_draw_back(const struct d_mtrl *mq,
                                           const struct game_draw *gd,
                                           int pose, int d, float t)
{
    if (pose == POSE_BALL)
        return mq;

    glPushMatrix();
    {
        const struct game_view *view = &gd->view;

        if (d < 0)
        {
            const struct game_tilt *tilt = &gd->tilt;

            glRotatef(tilt->rz * 2, tilt->z[0], tilt->z[1], tilt->z[2]);
            glRotatef(tilt->rx * 2, tilt->x[0], tilt->x[1], tilt->x[2]);
        }

        glTranslatef(view->p[0], view->p[1] * d, view->p[2]);

        if (config_get_d(CONFIG_BACKGROUND))
        {
            /* Draw all background layers back to front. */

            mq = sol_back(&gd->back.draw, mq, BACK_DIST, FAR_DIST,  t);
            mq = back_draw(mq, 0);
            mq = sol_back(&gd->back.draw, mq,         0, BACK_DIST, t);
        }
        else back_draw(mq, t);
    }
    glPopMatrix();

    return mq;
}

static void game_clip_refl(int d)
{
    /* Fudge to eliminate the floor from reflection. */

    GLdouble e[4], k = -0.00001;

    e[0] = 0;
    e[1] = 1;
    e[2] = 0;
    e[3] = k;

    glClipPlane(GL_CLIP_PLANE0, e);
}

static void game_clip_ball(const struct game_draw *gd, int d, const float *p)
{
    GLdouble r, c[3], pz[4], nz[4];

    /* Compute the plane giving the front of the ball, as seen from view.p. */

    c[0] = p[0];
    c[1] = p[1] * d;
    c[2] = p[2];

    pz[0] = gd->view.p[0] - c[0];
    pz[1] = gd->view.p[1] - c[1];
    pz[2] = gd->view.p[2] - c[2];

    r = sqrt(pz[0] * pz[0] + pz[1] * pz[1] + pz[2] * pz[2]);

    pz[0] /= r;
    pz[1] /= r;
    pz[2] /= r;
    pz[3] = -(pz[0] * c[0] +
              pz[1] * c[1] +
              pz[2] * c[2]);

    /* Find the plane giving the back of the ball, as seen from view.p. */

    nz[0] = -pz[0];
    nz[1] = -pz[1];
    nz[2] = -pz[2];
    nz[3] = -pz[3];

    /* Reflect these planes as necessary, and store them in the GL state. */

    pz[1] *= d;
    nz[1] *= d;

    glClipPlane(GL_CLIP_PLANE1, nz);
    glClipPlane(GL_CLIP_PLANE2, pz);
}

static const struct d_mtrl *game_draw_fore(const struct d_mtrl *mq,
                                           const struct game_draw *gd,
                                           int pose, const float *M,
                                           int d, float t)
{
    const float *ball_p = gd->vary.uv[0].p;

    const struct s_draw *draw = &gd->draw;

    glPushMatrix();
    {
        /* Rotate the environment about the position of the ball. */

        game_draw_tilt(gd, d);

        /* Compute clipping planes for reflection and ball facing. */

        game_clip_refl(d);
        game_clip_ball(gd, d, ball_p);

        if (d < 0)
            glEnable(GL_CLIP_PLANE0);

        switch (pose)
        {
        case POSE_LEVEL:
            mq = sol_draw(draw, mq, 0, 1);
            break;

        case POSE_NONE:
            /* Draw the floor. */

            mq = sol_draw(draw, mq, 0, 1);

            /* Draw the coins. */

            mq = game_draw_items(mq, draw->vary, M, t);

            /* Fall through. */

        case POSE_BALL:

            /* Draw the ball. */

            mq = game_draw_balls(mq, draw->vary, M, t);

            break;
        }

        /* Draw the billboards, entities, and  particles. */

        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDepthMask(GL_FALSE);
        {
            mq = sol_bill(draw, mq, M, t);

            mq = game_draw_goals(mq, gd, M, t);
            mq = game_draw_jumps(mq, gd, M, t);
            mq = game_draw_swchs(mq, draw->vary);

            mq = part_draw_coin(mq);
        }
        glDepthMask(GL_TRUE);
        glEnable(GL_LIGHTING);
        glDisable(GL_COLOR_MATERIAL);

        if (d < 0)
            glDisable(GL_CLIP_PLANE0);
    }
    glPopMatrix();

    return mq;
}

/*---------------------------------------------------------------------------*/

void game_draw(const struct game_draw *gd, int pose, float t)
{
    float fov = (float) config_get_d(CONFIG_VIEW_FOV);

    if (gd->jump_b) fov *= 2.f * fabsf(gd->jump_dt - 0.5);

    if (gd->state)
    {
        const struct game_view *view = &gd->view;
        const struct d_mtrl *mq = sol_draw_enable();

        video_push_persp(fov, 0.1f, FAR_DIST);
        glPushMatrix();
        {
            float T[16], U[16], M[16], v[3];

            /* Compute direct and reflected view bases. */

            v[0] = +view->p[0];
            v[1] = -view->p[1];
            v[2] = +view->p[2];

            m_view(T, view->c, view->p, view->e[1]);
            m_view(U, view->c, v,       view->e[1]);

            m_xps(M, T);

            /* Apply the current view. */

            v_sub(v, view->c, view->p);

            glTranslatef(0.f, 0.f, -v_len(v));
            glMultMatrixf(M);
            glTranslatef(-view->c[0], -view->c[1], -view->c[2]);

            if (gd->draw.reflective && config_get_d(CONFIG_REFLECTION))
            {
                glEnable(GL_STENCIL_TEST);
                {
                    /* Draw the mirrors only into the stencil buffer. */

                    glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
                    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    glDepthMask(GL_FALSE);

                    mq = game_refl_all(mq, gd);

                    glDepthMask(GL_TRUE);
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                    glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFF);

                    /* Draw the scene reflected into color and depth buffers. */

                    glFrontFace(GL_CW);
                    glPushMatrix();
                    {
                        glScalef(+1.0f, -1.0f, +1.0f);

                        game_draw_light();

                        mq = game_draw_back(mq, gd, pose,    -1, t);
                        mq = game_draw_fore(mq, gd, pose, U, -1, t);
                    }
                    glPopMatrix();
                    glFrontFace(GL_CCW);
                }
                glDisable(GL_STENCIL_TEST);
            }

            /* Draw the scene normally. */

            game_draw_light();

            if (gd->draw.reflective)
            {
                if (config_get_d(CONFIG_REFLECTION))
                {
                    /* Draw background while preserving reflections. */

                    glEnable(GL_STENCIL_TEST);
                    {
                        glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
                        mq = game_draw_back(mq, gd, pose, +1, t);
                    }
                    glDisable(GL_STENCIL_TEST);

                    /* Draw mirrors. */

                    mq = game_refl_all(mq, gd);
                }
                else
                {
                    /* Draw background. */

                    mq = game_draw_back(mq, gd, pose, +1, t);

                    /*
                     * Draw mirrors, first fully opaque with a custom
                     * material color, then blending normally with the
                     * opaque surfaces using their original material
                     * properties.  (Keeps background from showing
                     * through.)
                     */

                    glEnable(GL_COLOR_MATERIAL);
                    {
                        glColor4f(0.0, 0.0, 0.05, 1.0);
                        mq = game_refl_all(mq, gd);
                        glColor4f(1.0,  1.0,  1.0,  1.0);
                    }
                    glDisable(GL_COLOR_MATERIAL);

                    mq = game_refl_all(mq, gd);
                }
            }
            else
            {
                mq = game_draw_back(mq, gd, pose, +1, t);
                mq = game_refl_all(mq, gd);
            }

            mq = game_draw_fore(mq, gd, pose, T, +1, t);
        }
        glPopMatrix();
        video_pop_matrix();

        sol_draw_disable(mq);

        /* Draw the fade overlay. */

        sol_fade(&gd->draw, gd->fade_k);
    }
}

/*---------------------------------------------------------------------------*/

#define CURR 0
#define PREV 1

void game_lerp_init(struct game_lerp *gl, struct game_draw *gd)
{
    gl->alpha = 1.0f;

    sol_load_lerp(&gl->lerp, &gd->vary);

    gl->tilt[PREV] = gl->tilt[CURR] = gd->tilt;
    gl->view[PREV] = gl->view[CURR] = gd->view;

    gl->goal_k[PREV] = gl->goal_k[CURR] = gd->goal_k;
    gl->jump_dt[PREV] = gl->jump_dt[CURR] = gd->jump_dt;
}

void game_lerp_free(struct game_lerp *gl)
{
    sol_free_lerp(&gl->lerp);
}

void game_lerp_copy(struct game_lerp *gl)
{
    sol_lerp_copy(&gl->lerp);

    gl->tilt[PREV] = gl->tilt[CURR];
    gl->view[PREV] = gl->view[CURR];

    gl->goal_k[PREV] = gl->goal_k[CURR];
    gl->jump_dt[PREV] = gl->jump_dt[CURR];
}

void game_lerp_apply(struct game_lerp *gl, struct game_draw *gd)
{
    float a = gl->alpha;

    /* Solid. */

    sol_lerp_apply(&gl->lerp, a);

    /* Particles. */

    part_lerp_apply(a);

    /* Tilt. */

    v_lerp(gd->tilt.x, gl->tilt[PREV].x, gl->tilt[CURR].x, a);
    v_lerp(gd->tilt.z, gl->tilt[PREV].z, gl->tilt[CURR].z, a);

    gd->tilt.rx = (gl->tilt[PREV].rx * (1.0f - a) + gl->tilt[CURR].rx * a);
    gd->tilt.rz = (gl->tilt[PREV].rz * (1.0f - a) + gl->tilt[CURR].rz * a);

    /* View. */

    v_lerp(gd->view.c, gl->view[PREV].c, gl->view[CURR].c, a);
    v_lerp(gd->view.p, gl->view[PREV].p, gl->view[CURR].p, a);
    e_lerp(gd->view.e, gl->view[PREV].e, gl->view[CURR].e, a);

    /* Effects. */

    gd->goal_k = (gl->goal_k[PREV] * (1.0f - a) + gl->goal_k[CURR] * a);
    gd->jump_dt = (gl->jump_dt[PREV] * (1.0f - a) + gl->jump_dt[CURR] * a);
}

/*---------------------------------------------------------------------------*/
