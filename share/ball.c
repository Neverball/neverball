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

/*---------------------------------------------------------------------------*/

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
}

void ball_free(void)
{
    if (has_outer) sol_free_gl(&outer);
    if (has_inner) sol_free_gl(&inner);
    if (has_solid) sol_free_gl(&solid);

    has_solid = has_inner = has_outer = 0;
}

void ball_draw(const float *M,
               const float *P, float rx, float ry)
{
    /* Go to GREAT pains to ensure all layers are drawn back-to-front. */

    float T[16];
    float U[16];

    m_xps(T, M);
    m_xps(U, P);

    /* Position clipping planes to cut through the center of the ball. */

    if (has_outer)
    {
        glPushMatrix();
        {
            GLdouble nz[4] = { 0.0, 0.0, -1.0, 0.0 };
            GLdouble pz[4] = { 0.0, 0.0, +1.0, 0.0 };

            glMultMatrixf(T);
            glRotatef(ry, 0.0f, 1.0f, 0.0f);
            glRotatef(rx, 1.0f, 0.0f, 0.0f);
            glClipPlane(GL_CLIP_PLANE1, nz);
            glClipPlane(GL_CLIP_PLANE2, pz);
        }
        glPopMatrix();
    }

    /* Draw the outer geometry behind the ball. */

    if (has_outer)
    {
        glPushMatrix();
        {
            glMultMatrixf(T);
            glMultMatrixf(P);
            glEnable(GL_CLIP_PLANE1);
            sol_draw(&outer, rx, ry);
            glDisable(GL_CLIP_PLANE1);
        }
        glPopMatrix();
    }

    /* Draw the back of the solid geometry */

    if (has_solid)
    {
        glCullFace(GL_FRONT);
        sol_draw(&solid, rx, ry);
        glCullFace(GL_BACK);
    }

    /* Draw the inner geometry. */

    if (has_inner)
    {
        glPushMatrix();
        {
            glMultMatrixf(T);
            glMultMatrixf(P);
            sol_draw(&inner, rx, ry);

            glDepthMask(GL_FALSE);
            glDisable(GL_LIGHTING);
            sol_bill(&inner, rx, ry, U);
            glEnable(GL_LIGHTING);
            glDepthMask(GL_TRUE);
        }
        glPopMatrix();
    }

    /* Draw the front of the solid geometry */

    if (has_solid)
    {
        glDepthMask(GL_FALSE);
        glDisable(GL_LIGHTING);
        sol_bill(&solid, rx, ry, T);
        glEnable(GL_LIGHTING);
        glDepthMask(GL_TRUE);

        sol_draw(&solid, rx, ry);
    }

    /* Draw the outer geometry in front of the ball. */

    if (has_outer)
    {
        glPushMatrix();
        {
            glMultMatrixf(T);
            glMultMatrixf(P);
            glEnable(GL_CLIP_PLANE2);
            sol_draw(&outer, rx, ry);
            glDisable(GL_CLIP_PLANE2);
        }
        glPopMatrix();
    }

}

/*---------------------------------------------------------------------------*/
