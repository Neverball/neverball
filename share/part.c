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

/*---------------------------------------------------------------------------*/

struct part_vary
{
    GLfloat v[3];             /* Velocity                                    */
};

struct part_draw
{
    GLfloat p[3];             /* Position                                    */
    GLfloat c[3];             /* Color                                       */
    GLfloat t;                /* Time until death. Doubles as opacity.       */
};

static struct part_vary coin_vary[PART_MAX_COIN];
static struct part_draw coin_draw[PART_MAX_COIN];

static GLuint part_text_star;
static GLuint coin_vbo;

/*---------------------------------------------------------------------------*/

#define PI 3.1415927f

static float rnd(float l, float h)
{
    return l + (h - l) * rand() / RAND_MAX;
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
    if (coin_draw[i].t >= 1.0f)
    {
        v_cpy(part_lerp_coin[i].p[PREV], coin_draw[i].p);
        v_cpy(part_lerp_coin[i].p[CURR], coin_draw[i].p);
    }
}

void part_lerp_apply(float a)
{
    int i;

    for (i = 0; i < PART_MAX_COIN; i++)
        if (coin_draw[i].t > 0.0f)
            v_lerp(coin_draw[i].p,
                   part_lerp_coin[i].p[PREV],
                   part_lerp_coin[i].p[CURR], a);
}

/*---------------------------------------------------------------------------*/

void part_reset(void)
{
    int i;

    for (i = 0; i < PART_MAX_COIN; i++)
        coin_draw[i].t = 0.0f;

    part_lerp_init();
}

void part_init(void)
{
    part_text_star = make_image_from_file(IMG_PART_STAR);

    memset(coin_vary, 0, PART_MAX_COIN * sizeof (struct part_vary));
    memset(coin_draw, 0, PART_MAX_COIN * sizeof (struct part_draw));

    glGenBuffers(1,              &coin_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, coin_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof (coin_draw),
                                          coin_draw, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    part_reset();
}

void part_free(void)
{
    if (glIsBuffer(coin_vbo))
        glDeleteBuffers(1, &coin_vbo);

    if (glIsTexture(part_text_star))
        glDeleteTextures(1, &part_text_star);
}

/*---------------------------------------------------------------------------*/

void part_burst(const float *p, const float *c)
{
    int i, n = 0;

    for (i = 0; n < 10 && i < PART_MAX_COIN; i++)
        if (coin_draw[i].t <= 0.f)
        {
            float a = rnd(-1.0f * PI, +1.0f * PI);
            float b = rnd(+0.3f * PI, +0.5f * PI);

            coin_draw[i].c[0] = c[0];
            coin_draw[i].c[1] = c[1];
            coin_draw[i].c[2] = c[2];

            coin_draw[i].p[0] = p[0];
            coin_draw[i].p[1] = p[1];
            coin_draw[i].p[2] = p[2];

            coin_vary[i].v[0] = 4.f * fcosf(a) * fcosf(b);
            coin_vary[i].v[1] = 4.f *            fsinf(b);
            coin_vary[i].v[2] = 4.f * fsinf(a) * fcosf(b);

            coin_draw[i].t = 1.f;

            part_lerp_burst(i);

            n++;
        }
}

/*---------------------------------------------------------------------------*/

static void part_fall(struct part_lerp *lerp,
                      struct part_vary *vary,
                      struct part_draw *draw,
                      int n, const float *g, float dt)
{
    int i;

    for (i = 0; i < n; i++)
        if (draw[i].t > 0.f)
        {
            draw[i].t -= dt;

            v_mad(vary[i].v, vary[i].v, g, dt);

            v_mad(lerp[i].p[CURR], lerp[i].p[CURR], vary[i].v, dt);
        }
        else draw[i].t = 0.0f;
}

void part_step(const float *g, float dt)
{
    part_lerp_copy();
    part_fall(part_lerp_coin, coin_vary, coin_draw, PART_MAX_COIN, g, dt);
}

/*---------------------------------------------------------------------------*/

void part_draw_coin(const float *M, float t)
{
    const GLfloat c[3] = { 0.0f, 1.0f, 0.0f };
    GLint s = config_get_d(CONFIG_HEIGHT) / 8;

    glBindTexture(GL_TEXTURE_2D, part_text_star);

    /* Upload the current state of the particles. It would be best to limit  */
    /* this upload to only active particles, but it's more important to do   */
    /* it all in a single call.                                              */

    glBindBuffer   (GL_ARRAY_BUFFER, coin_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof (coin_draw), coin_draw);

    /* Draw the entire buffer.  Dead particles have zero opacity anyway. */

    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    {
        glColorPointer (4, GL_FLOAT, sizeof (struct part_draw),
                        (GLvoid *) offsetof (struct part_draw, c));
        glVertexPointer(3, GL_FLOAT, sizeof (struct part_draw),
                        (GLvoid *) offsetof (struct part_draw, p));

        glEnable(GL_POINT_SPRITE);
        {
            glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
            glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, c);
            glPointSize(s);

            glDrawArrays(GL_POINTS, 0, PART_MAX_COIN);
        }
        glDisable(GL_POINT_SPRITE);
    }
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

/*---------------------------------------------------------------------------*/
