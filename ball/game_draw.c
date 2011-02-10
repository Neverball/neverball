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
#include "back.h"
#include "video.h"

#include "solid_draw.h"

#include "game_draw.h"

/*---------------------------------------------------------------------------*/

static void game_draw_balls(const struct s_vary *vary,
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
        ball_draw(ball_M, pend_M, bill_M, t);
    }
    glPopMatrix();
    glPopAttrib();
}

static void game_draw_items(const struct s_vary *vary, float t)
{
    float r = 360.f * t;
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
                        glRotatef(r, 0.0f, 1.0f, 0.0f);
                        item_draw(&vary->hv[hi], r);
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
                        glRotatef(r, 0.0f, 1.0f, 0.0f);
                        item_draw(&vary->hv[hi], r);
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
                        glRotatef(r, 0.0f, 1.0f, 0.0f);
                        item_draw(&vary->hv[hi], r);
                    }
                    glPopMatrix();
                }
        }
        item_pull();
    }
    glPopAttrib();
}

static void game_draw_goals(const struct game_draw *gd,
                            const float *M, float t)
{
    const struct s_base *base = &gd->file.base;

    if (gd->goal_e)
    {
        int zi;

        /* Draw the goal particles. */

        glEnable(GL_TEXTURE_2D);
        {
            for (zi = 0; zi < base->zc; zi++)
            {
                glPushMatrix();
                {
                    glTranslatef(base->zv[zi].p[0],
                                 base->zv[zi].p[1],
                                 base->zv[zi].p[2]);

                    part_draw_goal(M, base->zv[zi].r, gd->goal_k, t);
                }
                glPopMatrix();
            }
        }
        glDisable(GL_TEXTURE_2D);

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

                goal_draw();
            }
            glPopMatrix();
        }
    }
}

static void game_draw_jumps(const struct game_draw *gd,
                            const float *M, float t)
{
    const struct s_base *base = &gd->file.base;

    int ji;

    glEnable(GL_TEXTURE_2D);
    {
        for (ji = 0; ji < base->jc; ji++)
        {
            glPushMatrix();
            {
                glTranslatef(base->jv[ji].p[0],
                             base->jv[ji].p[1],
                             base->jv[ji].p[2]);

                part_draw_jump(M, base->jv[ji].r, 1.0f, t);
            }
            glPopMatrix();
        }
    }
    glDisable(GL_TEXTURE_2D);

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

            jump_draw(!gd->jump_e);
        }
        glPopMatrix();
    }
}

static void game_draw_swchs(const struct s_vary *vary)
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

            swch_draw(xp->f, xp->e);
        }
        glPopMatrix();
    }
}

/*---------------------------------------------------------------------------*/

static void game_draw_tilt(const struct game_draw *gd, int d)
{
    const struct game_tilt *tilt = &gd->tilt;
    const float *ball_p = gd->file.vary.uv[0].p;

    /* Rotate the environment about the position of the ball. */

    glTranslatef(+ball_p[0], +ball_p[1] * d, +ball_p[2]);
    glRotatef(-tilt->rz * d, tilt->z[0], tilt->z[1], tilt->z[2]);
    glRotatef(-tilt->rx * d, tilt->x[0], tilt->x[1], tilt->x[2]);
    glTranslatef(-ball_p[0], -ball_p[1] * d, -ball_p[2]);
}

static void game_refl_all(const struct game_draw *gd)
{
    glPushMatrix();
    {
        game_draw_tilt(gd, 1);

        /* Draw the floor. */

        sol_refl(&gd->file.draw);
    }
    glPopMatrix();
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

static void game_draw_back(const struct game_draw *gd, int pose, int d, float t)
{
    if (pose == POSE_BALL)
        return;

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

            sol_back(&gd->back.draw, BACK_DIST, FAR_DIST,  t);
            back_draw(0);
            sol_back(&gd->back.draw,         0, BACK_DIST, t);
        }
        else back_draw(0);
    }
    glPopMatrix();
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

static void game_draw_fore(const struct game_draw *gd,
                           int pose, const float *M,
                           int d, float t)
{
    const float *ball_p = gd->file.vary.uv[0].p;
    const float  ball_r = gd->file.vary.uv[0].r;

    const struct s_draw *draw = &gd->file.draw;

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
            sol_draw(draw, 0, 1);
            break;

        case POSE_NONE:
            /* Draw the coins. */

            game_draw_items(draw->vary, t);

            /* Draw the floor. */

            sol_draw(draw, 0, 1);

            /* Fall through. */

        case POSE_BALL:

            /* Draw the ball shadow. */

            if (d > 0 && config_get_d(CONFIG_SHADOW))
            {
                shad_draw_set(ball_p, ball_r);
                sol_shad(draw);
                shad_draw_clr();
            }

            /* Draw the ball. */

            game_draw_balls(draw->vary, M, t);

            break;
        }

        /* Draw the particles and light columns. */

        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDepthMask(GL_FALSE);
        {
            glColor3f(1.0f, 1.0f, 1.0f);

            sol_bill(draw, M, t);
            part_draw_coin(M, t);

            glDisable(GL_TEXTURE_2D);
            {
                game_draw_goals(gd, M, t);
                game_draw_jumps(gd, M, t);
                game_draw_swchs(draw->vary);
            }
            glEnable(GL_TEXTURE_2D);

            glColor3f(1.0f, 1.0f, 1.0f);
        }
        glDepthMask(GL_TRUE);
        glEnable(GL_LIGHTING);
        glDisable(GL_COLOR_MATERIAL);

        if (d < 0)
            glDisable(GL_CLIP_PLANE0);
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

void game_draw(const struct game_draw *gd, int pose, float t)
{
    float fov = (float) config_get_d(CONFIG_VIEW_FOV);

    if (gd->jump_b) fov *= 2.f * fabsf(gd->jump_dt - 0.5);

    if (gd->state)
    {
        const struct game_view *view = &gd->view;

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

            if (gd->reflective && config_get_d(CONFIG_REFLECTION))
            {
                glEnable(GL_STENCIL_TEST);
                {
                    /* Draw the mirrors only into the stencil buffer. */

                    glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
                    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    glDepthMask(GL_FALSE);

                    game_refl_all(gd);

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
                        game_draw_back(gd, pose,    -1, t);
                        game_draw_fore(gd, pose, U, -1, t);
                    }
                    glPopMatrix();
                    glFrontFace(GL_CCW);
                }
                glDisable(GL_STENCIL_TEST);
            }

            /* Draw the scene normally. */

            game_draw_light();

            if (gd->reflective)
            {
                if (config_get_d(CONFIG_REFLECTION))
                {
                    /* Draw background while preserving reflections. */

                    glEnable(GL_STENCIL_TEST);
                    {
                        glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
                        game_draw_back(gd, pose, +1, t);
                    }
                    glDisable(GL_STENCIL_TEST);

                    /* Draw mirrors. */

                    game_refl_all(gd);
                }
                else
                {
                    /* Draw background. */

                    game_draw_back(gd, pose, +1, t);

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
                        game_refl_all(gd);
                        glColor4f(1.0,  1.0,  1.0,  1.0);
                    }
                    glDisable(GL_COLOR_MATERIAL);

                    game_refl_all(gd);
                }
            }
            else
            {
                game_draw_back(gd, pose, +1, t);
                game_refl_all(gd);
            }

            game_draw_fore(gd, pose, T, +1, t);
        }
        glPopMatrix();
        video_pop_matrix();

        /* Draw the fade overlay. */

        fade_draw(gd->fade_k);
    }
}

/*---------------------------------------------------------------------------*/
