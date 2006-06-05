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

#include "glext.h"
#include "part.h"
#include "vec3.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

struct part
{
    float t;
    float a;
    float w;
    float c[3];
    float p[3];
    float v[3];
};

static struct part part_coin[PART_MAX_COIN];
static struct part part_goal[PART_MAX_GOAL];
static GLuint      part_text;
static GLuint      part_list;

/*---------------------------------------------------------------------------*/

#define PI 3.1415927f

static float rnd(float l, float h)
{
    return l + (h - l) * rand() / RAND_MAX;
}

/*---------------------------------------------------------------------------*/

static GLfloat gcoltab[] = {0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

void part_reset(float h)
{
    int i;

    for (i = 0; i < PART_MAX_GOAL; i++)
    {
        float t = rnd(+0.1f,      +1.0f);
        float a = rnd(-1.0f * PI, +1.0f * PI);
        float w = rnd(-2.0f * PI, +2.0f * PI);
        int k = i % 6;

        part_goal[i].t = t;
        part_goal[i].a = V_DEG(a);
        part_goal[i].w = V_DEG(w);

        part_goal[i].c[0] = gcoltab[k];
        part_goal[i].c[1] = gcoltab[k+1];
        part_goal[i].c[2] = gcoltab[k+2];

        part_goal[i].p[0] = fsinf(a);
        part_goal[i].p[1] = (1.f - t) * h;
        part_goal[i].p[2] = fcosf(a);

        part_goal[i].v[0] = 0.f;
        part_goal[i].v[1] = 0.f;
        part_goal[i].v[2] = 0.f;

        part_coin[i].t    = 0.0f;
    }
}

void part_init(float h)
{
    memset(part_coin, 0, PART_MAX_COIN * sizeof (struct part));
    memset(part_goal, 0, PART_MAX_GOAL * sizeof (struct part));

    part_text = make_image_from_file(NULL, NULL, NULL, NULL, IMG_PART);
    part_list = glGenLists(1);

    glNewList(part_list, GL_COMPILE);
    {
        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.f, 0.f);
            glVertex2f(-PART_SIZE, -PART_SIZE);

            glTexCoord2f(1.f, 0.f);
            glVertex2f(+PART_SIZE, -PART_SIZE);

            glTexCoord2f(1.f, 1.f);
            glVertex2f(+PART_SIZE, +PART_SIZE);

            glTexCoord2f(0.f, 1.f);
            glVertex2f(-PART_SIZE, +PART_SIZE);
        }
        glEnd();
    }
    glEndList();

    part_reset(h);
}

void part_free(void)
{
    if (glIsList(part_list))
        glDeleteLists(part_list, 1);

    if (glIsTexture(part_text))
        glDeleteTextures(1, &part_text);
}

/*---------------------------------------------------------------------------*/

void part_burst(const float *p, const float *c)
{
    int i, n = 0;

    for (i = 0; n < 10 && i < PART_MAX_COIN; i++)
        if (part_coin[i].t <= 0.f)
        {
            float a = rnd(-1.0f * PI, +1.0f * PI);
            float b = rnd(+0.3f * PI, +0.5f * PI);
            float w = rnd(-4.0f * PI, +4.0f * PI);

            part_coin[i].p[0] = p[0];
            part_coin[i].p[1] = p[1];
            part_coin[i].p[2] = p[2];

            part_coin[i].v[0] = 4.f * fcosf(a) * fcosf(b);
            part_coin[i].v[1] = 4.f *            fsinf(b);
            part_coin[i].v[2] = 4.f * fsinf(a) * fcosf(b);

            part_coin[i].c[0] = c[0];
            part_coin[i].c[1] = c[1];
            part_coin[i].c[2] = c[2];

            part_coin[i].t = 1.f;
            part_coin[i].a = 0.f;
            part_coin[i].w = V_DEG(w);

            n++;
        }
}

/*---------------------------------------------------------------------------*/

static void part_fall(struct part *part, int n, const float *g, float dt)
{
    int i;

    for (i = 0; i < n; i++)
        if (part[i].t > 0.f)
        {
            part[i].t -= dt;

            part[i].v[0] += (g[0] * dt);
            part[i].v[1] += (g[1] * dt);
            part[i].v[2] += (g[2] * dt);

            part[i].p[0] += (part[i].v[0] * dt);
            part[i].p[1] += (part[i].v[1] * dt);
            part[i].p[2] += (part[i].v[2] * dt);
        }
}

static void part_spin(struct part *part, int n, const float *g, float dt)
{
    int i;

    for (i = 0; i < n; i++)
        if (part[i].t > 0.f)
        {
            part[i].a += 30.f * dt;

            part[i].p[0] = fsinf(V_RAD(part[i].a));
            part[i].p[2] = fcosf(V_RAD(part[i].a));
        }
}

void part_step(const float *g, float dt)
{
    part_fall(part_coin, PART_MAX_COIN, g, dt);

    if (g[1] > 0.f)
        part_fall(part_goal, PART_MAX_GOAL, g, dt);
    else
        part_spin(part_goal, PART_MAX_GOAL, g, dt);
}

/*---------------------------------------------------------------------------*/

static void part_draw(const float p[3], const float c[3],
                      float a, float r, float rx, float ry, float rz)
{
    glPushMatrix();
    {
        glTranslatef(r * p[0], p[1], r * p[2]);
        glRotatef(ry, 0.f, 1.f, 0.f);
        glRotatef(rx, 1.f, 0.f, 0.f);
        glRotatef(rz, 0.f, 0.f, 1.f);

        glColor4f(c[0], c[1], c[2], a);

        glCallList(part_list);
    }
    glPopMatrix();
}

void part_draw_coin(float rx, float ry)
{
    float r = (float) SDL_GetTicks() / 1000.f;
    int i;

    glPushAttrib(GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT);
    {
        glDisable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);

        glDepthMask(GL_FALSE);
        glBindTexture(GL_TEXTURE_2D, part_text);

        for (i = 0; i < PART_MAX_COIN; i++)
            if (part_coin[i].t > 0.f)
                part_draw(part_coin[i].p,
                          part_coin[i].c,
                          part_coin[i].t,
                          1.f, rx, ry, r * part_coin[i].w);
    }
    glPopAttrib();
}

void part_draw_goal(float rx, float ry, float radius, float a, int spe)
{
    float r = (float) SDL_GetTicks() / 1000.f;
    int i;
    GLfloat yel[3] = {1.0f, 1.0f, 0.0f};

    glPushAttrib(GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT);
    {
        glDisable(GL_LIGHTING);

        glEnable(GL_COLOR_MATERIAL);

        glDepthMask(GL_FALSE);
        glBindTexture(GL_TEXTURE_2D, part_text);

        for (i = 0; i < PART_MAX_GOAL; i++)
            if (part_goal[i].t > 0.f)
                part_draw(part_goal[i].p,
                          spe ? part_goal[i].c : yel,
                          a,
                          radius - 0.05f, rx, ry, r * part_goal[i].w);
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/
