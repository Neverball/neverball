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
#include "config.h"
#include "solid_gl.h"

/*---------------------------------------------------------------------------*/

static int has_solid = 0;
static int has_inner = 0;
static int has_outer = 0;

static struct s_file solid;
static struct s_file inner;
static struct s_file outer;

static int inner_pendulum;
static int outer_pendulum;

/*---------------------------------------------------------------------------*/

static int ball_flag(const struct s_file *fp, const char *key)
{
    int di;

    for (di = 0; di < fp->dc; ++di)
    {
        char *k = fp->av + fp->dv[di].ai;
        char *v = fp->av + fp->dv[di].aj;

        if (strcmp(k, key) == 0)
            return atoi(v);
    }

    return 0;
}

void ball_init(void)
{
    char solid_file[PATHMAX];
    char inner_file[PATHMAX];
    char outer_file[PATHMAX];

    config_get_s(CONFIG_BALL, solid_file, PATHMAX - 12);
    config_get_s(CONFIG_BALL, inner_file, PATHMAX - 12);
    config_get_s(CONFIG_BALL, outer_file, PATHMAX - 12);

    strcat(solid_file, "-solid.sol");
    strcat(inner_file, "-inner.sol");
    strcat(outer_file, "-outer.sol");

    has_solid = sol_load_gl(&solid,
                            config_data(solid_file),
                            config_get_d(CONFIG_TEXTURES), 0);
    has_inner = sol_load_gl(&inner,
                            config_data(inner_file),
                            config_get_d(CONFIG_TEXTURES), 0);
    has_outer = sol_load_gl(&outer,
                            config_data(outer_file),
                            config_get_d(CONFIG_TEXTURES), 0);

    inner_pendulum = (has_inner && ball_flag(&inner, "pendulum"));
    outer_pendulum = (has_outer && ball_flag(&outer, "pendulum"));
}

void ball_free(void)
{
    if (has_outer) sol_free_gl(&outer);
    if (has_inner) sol_free_gl(&inner);
    if (has_solid) sol_free_gl(&solid);

    has_solid = has_inner = has_outer = 0;
}

void ball_draw(const float *ball_M,
               const float *pend_M,
               const float *bill_M)
{
    /* Go to GREAT pains to ensure all layers are drawn back-to-front. */

    float ball_T[16], ball_bill_M[16];
    float pend_T[16], pend_bill_M[16];

    m_xps(ball_T, ball_M);
    m_xps(pend_T, pend_M);

    m_mult(ball_bill_M, ball_T, bill_M);
    m_mult(pend_bill_M, pend_T, bill_M);

    /* Draw the outer geometry behind the ball. */

    if (has_outer)
    {
        glPushMatrix();
        {
            if (outer_pendulum)
                glMultMatrixf(pend_M);

            glEnable(GL_CLIP_PLANE1);
            sol_draw(&outer);
            glDisable(GL_CLIP_PLANE1);
        }
        glPopMatrix();
    }

    /* Draw the back of the solid geometry */

    if (has_solid)
    {
        glPushMatrix();
        {
            glMultMatrixf(ball_M);

            glCullFace(GL_FRONT);
            sol_draw(&solid);
            glCullFace(GL_BACK);
        }
        glPopMatrix();
    }

    /* Draw the inner geometry. */

    if (has_inner)
    {
        glPushMatrix();
        {
            if (inner_pendulum)
                glMultMatrixf(pend_M);

            sol_draw(&inner);

            glDepthMask(GL_FALSE);
            glDisable(GL_LIGHTING);
            sol_bill(&inner, inner_pendulum ? pend_bill_M : bill_M);
            glEnable(GL_LIGHTING);
            glDepthMask(GL_TRUE);
        }
        glPopMatrix();
    }

    /* Draw the front of the solid geometry */

    if (has_solid)
    {
        glPushMatrix();
        {
            glMultMatrixf(ball_M);

            glDepthMask(GL_FALSE);
            glDisable(GL_LIGHTING);
            sol_bill(&solid, ball_bill_M);
            glEnable(GL_LIGHTING);
            glDepthMask(GL_TRUE);

            sol_draw(&solid);
        }
        glPopMatrix();
    }

    /* Draw the outer geometry in front of the ball. */

    if (has_outer)
    {
        glPushMatrix();
        {
            if (outer_pendulum)
                glMultMatrixf(pend_M);

            glEnable(GL_CLIP_PLANE2);
            sol_draw(&outer);
            glDisable(GL_CLIP_PLANE2);
        }
        glPopMatrix();
    }
}

/*---------------------------------------------------------------------------*/
