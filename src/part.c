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

#include "gl.h"
#include "part.h"
#include "vec3.h"
#include "text.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

struct part
{
    double t;
    double a;
    double w;
    double c[3];
    double p[3];
    double v[3];
};

static struct part part_coin[PART_MAX_COIN];
static struct part part_goal[PART_MAX_GOAL];
static GLuint      part_text;
static GLuint      part_list;

/*---------------------------------------------------------------------------*/

#define PI 3.1415926535897932

static double rnd(double l, double h)
{
    return l + (h - l) * rand() / RAND_MAX;
}

/*---------------------------------------------------------------------------*/

static void part_init_goal(double h)
{
    int i;

    for (i = 0; i < PART_MAX_GOAL; i++)
    {
        double t = rnd(0.1, 1.0);
        double a = rnd(-1.0 * PI, +1.0 * PI);
        double w = rnd(-2.0 * PI, +2.0 * PI);

        part_goal[i].t = t;
        part_goal[i].a = V_DEG(a);
        part_goal[i].w = V_DEG(w);

        part_goal[i].c[0] = c_yellow[0];
        part_goal[i].c[1] = c_yellow[1];
        part_goal[i].c[2] = c_yellow[2];

        part_goal[i].p[0] = 0.65 * sin(a);
        part_goal[i].p[1] = (1.0 - t) * h;
        part_goal[i].p[2] = 0.65 * cos(a);

        part_goal[i].v[0] = 0.0;
        part_goal[i].v[1] = 0.0;
        part_goal[i].v[2] = 0.0;
    }
}

void part_init(double h)
{
    memset(part_coin, 0, PART_MAX_COIN * sizeof (struct part));
    memset(part_goal, 0, PART_MAX_GOAL * sizeof (struct part));

    part_text = make_image_from_file(NULL, NULL, IMG_PART);
    part_list = glGenLists(1);

    glNewList(part_list, GL_COMPILE);
    {
        glBegin(GL_QUADS);
        {
            glTexCoord2d(0.0, 0.0);
            glVertex2d(-PART_SIZE, -PART_SIZE);

            glTexCoord2d(1.0, 0.0);
            glVertex2d(+PART_SIZE, -PART_SIZE);

            glTexCoord2d(1.0, 1.0);
            glVertex2d(+PART_SIZE, +PART_SIZE);

            glTexCoord2d(0.0, 1.0);
            glVertex2d(-PART_SIZE, +PART_SIZE);
        }
        glEnd();
    }
    glEndList();

    part_init_goal(h);
}

void part_free(void)
{
    if (glIsList(part_list))
        glDeleteLists(part_list, 1);

    if (glIsTexture(part_text))
        glDeleteTextures(1, &part_text);
}

/*---------------------------------------------------------------------------*/

void part_burst(const double *p, const double *c)
{
    int i, n = 0;

    for (i = 0; n < 10 && i < PART_MAX_COIN; i++)
        if (part_coin[i].t <= 0.0)
        {
            double a = rnd(-1.0 * PI, +1.0 * PI);
            double b = rnd(+0.3 * PI, +0.5 * PI);
            double w = rnd(-4.0 * PI, +4.0 * PI);

            part_coin[i].p[0] = p[0];
            part_coin[i].p[1] = p[1];
            part_coin[i].p[2] = p[2];

            part_coin[i].v[0] = 4.0 * cos(a) * cos(b);
            part_coin[i].v[1] = 4.0 *          sin(b);
            part_coin[i].v[2] = 4.0 * sin(a) * cos(b);

            part_coin[i].c[0] = c[0];
            part_coin[i].c[1] = c[1];
            part_coin[i].c[2] = c[2];

            part_coin[i].t = 1.0;
            part_coin[i].a = 0.0;
            part_coin[i].w = V_DEG(w);

            n++;
        }
}

/*---------------------------------------------------------------------------*/

static void part_fall(struct part *part, int n, const double *g, double dt)
{
    int i;

    for (i = 0; i < n; i++)
        if (part[i].t > 0.0)
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

static void part_spin(struct part *part, int n, const double *g, double dt)
{
    int i;

    for (i = 0; i < n; i++)
        if (part[i].t > 0.0)
        {
            part[i].a += 30.0 * dt;

            part[i].p[0] = 0.65 * sin(V_RAD(part[i].a));
            part[i].p[2] = 0.65 * cos(V_RAD(part[i].a));
        }
}

void part_step(const double *g, double dt)
{
    part_fall(part_coin, PART_MAX_COIN, g, dt);

    if (g[1] > 0.0)
        part_fall(part_goal, PART_MAX_GOAL, g, dt);
    else
        part_spin(part_goal, PART_MAX_GOAL, g, dt);
}

/*---------------------------------------------------------------------------*/

static void part_draw(const double p[3], const double c[3],
                      double a, double rx, double ry, double rz)
{
    glPushMatrix();
    {
        glTranslated(p[0], p[1], p[2]);
        glRotated(ry, 0.0, 1.0, 0.0);
        glRotated(rx, 1.0, 0.0, 0.0);
        glRotated(rz, 0.0, 0.0, 1.0);

        glColor4d(c[0], c[1], c[2], a);

        glCallList(part_list);
    }
    glPopMatrix();
}

void part_draw_coin(double rx, double ry)
{
    double r = (double) SDL_GetTicks() / 1000.0;
    int i;

    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    {
        glDisable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);

        glDepthMask(GL_FALSE);
        glBindTexture(GL_TEXTURE_2D, part_text);

        for (i = 0; i < PART_MAX_COIN; i++)
            if (part_coin[i].t > 0.0)
                part_draw(part_coin[i].p, part_coin[i].c,
                          part_coin[i].t, rx, ry, r * part_coin[i].w);
    }
    glPopAttrib();
    glPopAttrib();
}

void part_draw_goal(double rx, double ry)
{
    double r = (double) SDL_GetTicks() / 1000.0;
    int i;

    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    {
        glDisable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);

        glDepthMask(GL_FALSE);
        glBindTexture(GL_TEXTURE_2D, part_text);

        for (i = 0; i < PART_MAX_GOAL; i++)
            if (part_goal[i].t > 0.0)
                part_draw(part_goal[i].p, part_goal[i].c,
                          part_goal[i].t, rx, ry, r * part_goal[i].w);
    }
    glPopAttrib();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/
