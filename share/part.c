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
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "config.h"
#include "glext.h"
#include "part.h"
#include "vec3.h"
#include "image.h"
#include "geom.h"
#include "hmd.h"
#include "video.h"

/*---------------------------------------------------------------------------*/

struct part
{
    float v[3];               /* Velocity                                    */
    float w;                  /* Angular velocity (degrees)                  */
    float p[3];               /* Position                                    */
    float c[3];               /* Color                                       */
    float t;                  /* Time until death. Doubles as opacity.       */
};

static struct part coin_part[PART_MAX_COIN];

static GLuint coin_vbo;
static GLuint coin_ebo;

/*---------------------------------------------------------------------------*/

static struct b_mtrl coin_base_mtrl =
{
    { 0.8f, 0.8f, 0.8f, 1.0f },
    { 0.2f, 0.2f, 0.2f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f }, 0.0f, M_TRANSPARENT, IMG_PART_STAR
};

static int coin_mtrl;

/*---------------------------------------------------------------------------*/

#define PI 3.1415927f

static float rnd(float l, float h)
{
    return l + (h - l) * rand() / (float) RAND_MAX;
}

/*---------------------------------------------------------------------------*/

#define CURR 0
#define PREV 1

struct part_lerp
{
    float p[2][3];
};

static struct part_lerp part_lerp_coin[PART_MAX_COIN];

void part_lerp_copy(void)
{
    int i;

    for (i = 0; i < PART_MAX_COIN; i++)
        v_cpy(part_lerp_coin[i].p[PREV],
              part_lerp_coin[i].p[CURR]);
}

void part_lerp_init(void)
{
}

void part_lerp_burst(int i)
{
    if (coin_part[i].t >= 1.0f)
    {
        v_cpy(part_lerp_coin[i].p[PREV], coin_part[i].p);
        v_cpy(part_lerp_coin[i].p[CURR], coin_part[i].p);
    }
}

void part_lerp_apply(float a)
{
    int i;

    for (i = 0; i < PART_MAX_COIN; i++)
        if (coin_part[i].t > 0.0f)
            v_lerp(coin_part[i].p,
                   part_lerp_coin[i].p[PREV],
                   part_lerp_coin[i].p[CURR], a);
}

/*---------------------------------------------------------------------------*/

void part_reset(void)
{
    int i;

    for (i = 0; i < PART_MAX_COIN; i++)
        coin_part[i].t = 0.0f;

    part_lerp_init();
}

void part_init(void)
{
    static const GLfloat verts[4][5] = {
        { -0.5f, -0.5f, 0.0f, 0.0f, 0.0f },
        { +0.5f, -0.5f, 0.0f, 1.0f, 0.0f },
        { -0.5f, +0.5f, 0.0f, 0.0f, 1.0f },
        { +0.5f, +0.5f, 0.0f, 1.0f, 1.0f },
    };

    static const GLushort elems[4] = {
        0u, 1u, 2u, 3u
    };

    coin_mtrl = mtrl_cache(&coin_base_mtrl);

    memset(coin_part, 0, sizeof (coin_part));

    glGenBuffers_(1,              &coin_vbo);
    glBindBuffer_(GL_ARRAY_BUFFER, coin_vbo);
    glBufferData_(GL_ARRAY_BUFFER, sizeof (verts), verts, GL_STATIC_DRAW);
    glBindBuffer_(GL_ARRAY_BUFFER, 0);

    glGenBuffers_(1, &coin_ebo);
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, coin_ebo);
    glBufferData_(GL_ELEMENT_ARRAY_BUFFER, sizeof (elems), elems, GL_STATIC_DRAW);
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, 0);

    part_reset();
}

void part_free(void)
{
    glDeleteBuffers_(1, &coin_vbo);

    mtrl_free(coin_mtrl);
    coin_mtrl = 0;
}

/*---------------------------------------------------------------------------*/

void part_burst(const float *p, const float *c)
{
    int i, n = 0;

    for (i = 0; n < 10 && i < PART_MAX_COIN; i++)
        if (coin_part[i].t <= 0.f)
        {
            float a = rnd(-1.0f * PI, +1.0f * PI);
            float b = rnd(+0.3f * PI, +0.5f * PI);
            float w = rnd(-4.0f * PI, +4.0f * PI);

            coin_part[i].c[0] = c[0];
            coin_part[i].c[1] = c[1];
            coin_part[i].c[2] = c[2];

            coin_part[i].p[0] = p[0];
            coin_part[i].p[1] = p[1];
            coin_part[i].p[2] = p[2];

            coin_part[i].v[0] = 4.f * fcosf(a) * fcosf(b);
            coin_part[i].v[1] = 4.f *            fsinf(b);
            coin_part[i].v[2] = 4.f * fsinf(a) * fcosf(b);

            coin_part[i].w = V_DEG(w);

            coin_part[i].t = 1.f;

            part_lerp_burst(i);

            n++;
        }
}

/*---------------------------------------------------------------------------*/

static void part_fall(const float *g, float dt)
{
    int i;

    for (i = 0; i < PART_MAX_COIN; i++)
        if (coin_part[i].t > 0.f)
        {
            coin_part[i].t -= dt;

            v_mad(coin_part[i].v, coin_part[i].v, g, dt);

            v_mad(part_lerp_coin[i].p[CURR], part_lerp_coin[i].p[CURR], coin_part[i].v, dt);
        }
        else coin_part[i].t = 0.0f;
}

void part_step(const float *g, float dt)
{
    part_lerp_copy();
    part_fall(g, dt);
}

/*---------------------------------------------------------------------------*/

void part_draw_coin(const struct s_draw *draw, struct s_rend *rend, const float *M, float t)
{
    int i;

    r_apply_mtrl(rend, coin_mtrl);

    glBindBuffer_(GL_ARRAY_BUFFER, coin_vbo);
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, coin_ebo);

    glDisableClientState(GL_NORMAL_ARRAY);
    {
        glVertexPointer  (3, GL_FLOAT, sizeof (GLfloat) * 5, (GLvoid *) (                   0u));
        glTexCoordPointer(2, GL_FLOAT, sizeof (GLfloat) * 5, (GLvoid *) (sizeof (GLfloat) * 3u));

        for (i = 0; i < PART_MAX_COIN; ++i)
            if (coin_part[i].t > 0.0f)
            {
                glColor4f(coin_part[i].c[0], coin_part[i].c[1], coin_part[i].c[2], coin_part[i].t);

                glPushMatrix();
                {
                    glTranslatef(coin_part[i].p[0], coin_part[i].p[1], coin_part[i].p[2]);

                    if (M)
                        glMultMatrixf(M);

                    glScalef(PART_SIZE * 2.0f, PART_SIZE * 2.0f, 1.0f);
                    glRotatef(t * coin_part[i].w, 0.0f, 0.0f, 1.0f);

                    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (GLvoid *) 0u);
                }
                glPopMatrix();
            }
    }
    glEnableClientState(GL_NORMAL_ARRAY);

    glBindBuffer_(GL_ARRAY_BUFFER, 0);
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, 0);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

/*---------------------------------------------------------------------------*/
