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

#include "solid.h"
#include "vec3.h"
#include "glext.h"
#include "ball.h"
#include "item.h"
#include "part.h"
#include "geom.h"
#include "solid_gl.h"
#include "config.h"
#include "back.h"
#include "video.h"

#include "game_draw.h"

/*---------------------------------------------------------------------------*/

static void game_draw_balls(const s_file *fp,
                            const float *bill_M, float t)
{
    float c[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    float ball_M[16];
    float pend_M[16];

    m_basis(ball_M, fp->uv[0].e[0], fp->uv[0].e[1], fp->uv[0].e[2]);
    m_basis(pend_M, fp->uv[0].E[0], fp->uv[0].E[1], fp->uv[0].E[2]);

    glPushAttrib(GL_LIGHTING_BIT);
    glPushMatrix();
    {
        glTranslatef(fp->uv[0].p[0],
                     fp->uv[0].p[1] + BALL_FUDGE,
                     fp->uv[0].p[2]);
        glScalef(fp->uv[0].r,
                 fp->uv[0].r,
                 fp->uv[0].r);

        glColor4fv(c);
        ball_draw(ball_M, pend_M, bill_M, t);
    }
    glPopMatrix();
    glPopAttrib();
}

static void game_draw_items(const s_file *fp, float t)
{
    float r = 360.f * t;
    int hi;

    glPushAttrib(GL_LIGHTING_BIT);
    {
        item_push(ITEM_COIN);
        {
            for (hi = 0; hi < fp->hc; hi++)

                if (fp->hv[hi].t == ITEM_COIN && fp->hv[hi].n > 0)
                {
                    glPushMatrix();
                    {
                        glTranslatef(fp->hv[hi].p[0],
                                     fp->hv[hi].p[1],
                                     fp->hv[hi].p[2]);
                        glRotatef(r, 0.0f, 1.0f, 0.0f);
                        item_draw(&fp->hv[hi], r);
                    }
                    glPopMatrix();
                }
        }
        item_pull();

        item_push(ITEM_SHRINK);
        {
            for (hi = 0; hi < fp->hc; hi++)

                if (fp->hv[hi].t == ITEM_SHRINK)
                {
                    glPushMatrix();
                    {
                        glTranslatef(fp->hv[hi].p[0],
                                     fp->hv[hi].p[1],
                                     fp->hv[hi].p[2]);
                        glRotatef(r, 0.0f, 1.0f, 0.0f);
                        item_draw(&fp->hv[hi], r);
                    }
                    glPopMatrix();
                }
        }
        item_pull();

        item_push(ITEM_GROW);
        {
            for (hi = 0; hi < fp->hc; hi++)

                if (fp->hv[hi].t == ITEM_GROW)
                {
                    glPushMatrix();
                    {
                        glTranslatef(fp->hv[hi].p[0],
                                     fp->hv[hi].p[1],
                                     fp->hv[hi].p[2]);
                        glRotatef(r, 0.0f, 1.0f, 0.0f);
                        item_draw(&fp->hv[hi], r);
                    }
                    glPopMatrix();
                }
        }
        item_pull();
    }
    glPopAttrib();
}

static void game_draw_goals(const struct game_draw *dr,
                            const s_file *fp,
                            const float *M, float t)
{
    if (dr->goal_e)
    {
        int zi;

        /* Draw the goal particles. */

        glEnable(GL_TEXTURE_2D);
        {
            for (zi = 0; zi < fp->zc; zi++)
            {
                glPushMatrix();
                {
                    glTranslatef(fp->zv[zi].p[0],
                                 fp->zv[zi].p[1],
                                 fp->zv[zi].p[2]);

                    part_draw_goal(M, fp->zv[zi].r, dr->goal_k, t);
                }
                glPopMatrix();
            }
        }
        glDisable(GL_TEXTURE_2D);

        /* Draw the goal column. */

        for (zi = 0; zi < fp->zc; zi++)
        {
            glPushMatrix();
            {
                glTranslatef(fp->zv[zi].p[0],
                             fp->zv[zi].p[1],
                             fp->zv[zi].p[2]);

                glScalef(fp->zv[zi].r,
                         dr->goal_k,
                         fp->zv[zi].r);

                goal_draw();
            }
            glPopMatrix();
        }
    }
}

static void game_draw_jumps(const struct game_draw *dr,
                            const s_file *fp,
                            const float *M, float t)
{
    int ji;

    glEnable(GL_TEXTURE_2D);
    {
        for (ji = 0; ji < fp->jc; ji++)
        {
            glPushMatrix();
            {
                glTranslatef(fp->jv[ji].p[0],
                             fp->jv[ji].p[1],
                             fp->jv[ji].p[2]);

                part_draw_jump(M, fp->jv[ji].r, 1.0f, t);
            }
            glPopMatrix();
        }
    }
    glDisable(GL_TEXTURE_2D);

    for (ji = 0; ji < fp->jc; ji++)
    {
        glPushMatrix();
        {
            glTranslatef(fp->jv[ji].p[0],
                         fp->jv[ji].p[1],
                         fp->jv[ji].p[2]);
            glScalef(fp->jv[ji].r,
                     1.0f,
                     fp->jv[ji].r);

            jump_draw(!dr->jump_e);
        }
        glPopMatrix();
    }
}

static void game_draw_swchs(const s_file *fp)
{
    int xi;

    for (xi = 0; xi < fp->xc; xi++)
    {
        if (fp->xv[xi].i)
            continue;

        glPushMatrix();
        {
            glTranslatef(fp->xv[xi].p[0],
                         fp->xv[xi].p[1],
                         fp->xv[xi].p[2]);
            glScalef(fp->xv[xi].r,
                     1.0f,
                     fp->xv[xi].r);

            swch_draw(fp->xv[xi].f, fp->xv[xi].e);
        }
        glPopMatrix();
    }
}

/*---------------------------------------------------------------------------*/

static void game_draw_tilt(const struct game_draw *dr, int d)
{
    const struct game_tilt *tilt = &dr->tilt;
    const float *ball_p = dr->file.uv->p;

    /* Rotate the environment about the position of the ball. */

    glTranslatef(+ball_p[0], +ball_p[1] * d, +ball_p[2]);
    glRotatef(-tilt->rz * d, tilt->z[0], tilt->z[1], tilt->z[2]);
    glRotatef(-tilt->rx * d, tilt->x[0], tilt->x[1], tilt->x[2]);
    glTranslatef(-ball_p[0], -ball_p[1] * d, -ball_p[2]);
}

static void game_refl_all(const struct game_draw *dr)
{
    glPushMatrix();
    {
        game_draw_tilt(dr, 1);

        /* Draw the floor. */

        sol_refl(&dr->file);
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

static void game_draw_back(const struct game_draw *dr, int pose, int d, float t)
{
    if (pose == POSE_BALL)
        return;

    glPushMatrix();
    {
        if (d < 0)
        {
            const struct game_tilt *tilt = &dr->tilt;

            glRotatef(tilt->rz * 2, tilt->z[0], tilt->z[1], tilt->z[2]);
            glRotatef(tilt->rx * 2, tilt->x[0], tilt->x[1], tilt->x[2]);
        }

        glTranslatef(dr->view.p[0], dr->view.p[1] * d, dr->view.p[2]);

        if (config_get_d(CONFIG_BACKGROUND))
        {
            /* Draw all background layers back to front. */

            sol_back(&dr->back, BACK_DIST, FAR_DIST,  t);
            back_draw(0);
            sol_back(&dr->back,         0, BACK_DIST, t);
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

static void game_clip_ball(const struct game_draw *dr, int d, const float *p)
{
    GLdouble r, c[3], pz[4], nz[4];

    /* Compute the plane giving the front of the ball, as seen from view.p. */

    c[0] = p[0];
    c[1] = p[1] * d;
    c[2] = p[2];

    pz[0] = dr->view.p[0] - c[0];
    pz[1] = dr->view.p[1] - c[1];
    pz[2] = dr->view.p[2] - c[2];

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

static void game_draw_fore(const struct game_draw *dr,
                           int pose, const float *M,
                           int d, float t)
{
    const float *ball_p = dr->file.uv->p;
    const float  ball_r = dr->file.uv->r;

    const s_file *fp = &dr->file;

    glPushMatrix();
    {
        /* Rotate the environment about the position of the ball. */

        game_draw_tilt(dr, d);

        /* Compute clipping planes for reflection and ball facing. */

        game_clip_refl(d);
        game_clip_ball(dr, d, ball_p);

        if (d < 0)
            glEnable(GL_CLIP_PLANE0);

        switch (pose)
        {
        case POSE_LEVEL:
            sol_draw(fp, 0, 1);
            break;

        case POSE_NONE:
            /* Draw the coins. */

            game_draw_items(fp, t);

            /* Draw the floor. */

            sol_draw(fp, 0, 1);

            /* Fall through. */

        case POSE_BALL:

            /* Draw the ball shadow. */

            if (d > 0 && config_get_d(CONFIG_SHADOW))
            {
                shad_draw_set(ball_p, ball_r);
                sol_shad(fp);
                shad_draw_clr();
            }

            /* Draw the ball. */

            game_draw_balls(fp, M, t);

            break;
        }

        /* Draw the particles and light columns. */

        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDepthMask(GL_FALSE);
        {
            glColor3f(1.0f, 1.0f, 1.0f);

            sol_bill(fp, M, t);
            part_draw_coin(M, t);

            glDisable(GL_TEXTURE_2D);
            {
                game_draw_goals(dr, fp, M, t);
                game_draw_jumps(dr, fp, M, t);
                game_draw_swchs(fp);
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

void game_draw(const struct game_draw *dr, int pose, float t)
{
    float fov = (float) config_get_d(CONFIG_VIEW_FOV);

    if (dr->jump_b) fov *= 2.f * fabsf(dr->jump_dt - 0.5);

    if (dr->state)
    {
        const struct game_view *view = &dr->view;

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

            if (dr->reflective && config_get_d(CONFIG_REFLECTION))
            {
                glEnable(GL_STENCIL_TEST);
                {
                    /* Draw the mirrors only into the stencil buffer. */

                    glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
                    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    glDepthMask(GL_FALSE);

                    game_refl_all(dr);

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
                        game_draw_back(dr, pose,    -1, t);
                        game_draw_fore(dr, pose, U, -1, t);
                    }
                    glPopMatrix();
                    glFrontFace(GL_CCW);
                }
                glDisable(GL_STENCIL_TEST);
            }

            /* Draw the scene normally. */

            game_draw_light();

            if (dr->reflective)
            {
                if (config_get_d(CONFIG_REFLECTION))
                {
                    /* Draw background while preserving reflections. */

                    glEnable(GL_STENCIL_TEST);
                    {
                        glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
                        game_draw_back(dr, pose, +1, t);
                    }
                    glDisable(GL_STENCIL_TEST);

                    /* Draw mirrors. */

                    game_refl_all(dr);
                }
                else
                {
                    /* Draw background. */

                    game_draw_back(dr, pose, +1, t);

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
                        game_refl_all(dr);
                        glColor4f(1.0,  1.0,  1.0,  1.0);
                    }
                    glDisable(GL_COLOR_MATERIAL);

                    game_refl_all(dr);
                }
            }
            else
            {
                game_draw_back(dr, pose, +1, t);
                game_refl_all(dr);
            }

            game_draw_fore(dr, pose, T, +1, t);
        }
        glPopMatrix();
        video_pop_matrix();

        /* Draw the fade overlay. */

        fade_draw(dr->fade_k);
    }
}

/*---------------------------------------------------------------------------*/
