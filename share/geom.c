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
#include "geom.h"
#include "part.h"
#include "vec3.h"
#include "solid.h"
#include "image.h"
#include "config.h"

#define PI 3.1415926535897932

#define OCTA_BALL 1

/*---------------------------------------------------------------------------*/

static GLuint ball_list;
static GLuint ball_text;

/*---------------------------------------------------------------------------*/
#ifdef OCTA_BALL

/* These are the faces of an octahedron in positive longitude/latitude. */

static float octahedron[8][3][2] = {
    {{   0.0f,  90.0f }, {   0.0f, 0.0f }, {  90.0f, 0.0f }},
    {{  90.0f,  90.0f }, {  90.0f, 0.0f }, { 180.0f, 0.0f }},
    {{ 180.0f,  90.0f }, { 180.0f, 0.0f }, { 270.0f, 0.0f }},
    {{ 270.0f,  90.0f }, { 270.0f, 0.0f }, { 360.0f, 0.0f }},
    {{   0.0f, -90.0f }, {  90.0f, 0.0f }, {   0.0f, 0.0f }},
    {{  90.0f, -90.0f }, { 180.0f, 0.0f }, {  90.0f, 0.0f }},
    {{ 180.0f, -90.0f }, { 270.0f, 0.0f }, { 180.0f, 0.0f }},
    {{ 270.0f, -90.0f }, { 360.0f, 0.0f }, { 270.0f, 0.0f }},
};

static void midpoint(float *P, const float *A, const float *B)
{
    float D[2];

    /* The haversine midpoint method. */

    D[0] = fcosf(B[1]) * fcosf(B[0] - A[0]);
    D[1] = fcosf(B[1]) * fsinf(B[0] - A[0]);

    P[0] = A[0] + fatan2f(D[1], fcosf(A[1]) + D[0]);

    P[1] = fatan2f(fsinf(A[1]) +
                   fsinf(B[1]),
                   fsqrtf((fcosf(A[1]) + D[0]) *
                          (fcosf(A[1]) + D[0]) + D[1] * D[1])); 
}

static void ball_vertex(const float *p)
{
    /* Draw a vertex with normal and texture coordinate at the given lon/lat. */

    const float x = fsinf(p[0]) * fcosf(p[1]);
    const float y =               fsinf(p[1]);
    const float z = fcosf(p[0]) * fcosf(p[1]);

    glTexCoord2f((+p[0]               ) / V_RAD(360.0f),
                 (-p[1] + V_RAD(90.0f)) / V_RAD(180.0f));

    glNormal3f(x, y, z);
    glVertex3f(x, y, z);
}

static void ball_subdiv(const float *a,
                        const float *b,
                        const float *c, int D)
{
    if (D > 0)
    {
        /* Recursively subdivide the given triangle. */

        float d[2];
        float e[2];
        float f[2];

        midpoint(d, a, b);
        midpoint(e, b, c);
        midpoint(f, c, a);

        ball_subdiv(a, d, f, D - 1);
        ball_subdiv(d, b, e, D - 1);
        ball_subdiv(f, e, c, D - 1);
        ball_subdiv(d, e, f, D - 1);
    }
    else
    {
        /* Draw the given triangle. */

        ball_vertex(a);
        ball_vertex(b);
        ball_vertex(c);
    }
}

#endif /* OCTA_BALL */
/*---------------------------------------------------------------------------*/

void ball_init(int b)
{
    char name[MAXSTR];

    config_get_s(CONFIG_BALL, name, MAXSTR);

    if ((ball_text = make_image_from_file(name)))
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    ball_list = glGenLists(1);

    glNewList(ball_list, GL_COMPILE);
    {
#ifdef OCTA_BALL
        int i, d = b ? 4 : 3;

        glBegin(GL_TRIANGLES);
        {
            for (i = 0; i < 8; ++i)
            {
                float a[2];
                float b[2];
                float c[2];

                a[0] = V_RAD(octahedron[i][0][0]);
                a[1] = V_RAD(octahedron[i][0][1]);

                b[0] = V_RAD(octahedron[i][1][0]);
                b[1] = V_RAD(octahedron[i][1][1]);

                c[0] = V_RAD(octahedron[i][2][0]);
                c[1] = V_RAD(octahedron[i][2][1]);

                ball_subdiv(a, b, c, d);
            }
        }
        glEnd();
#else
        int i, slices = b ? 32 : 16;
        int j, stacks = b ? 16 :  8;

        for (i = 0; i < stacks; i++)
        {
            float k0 = (float)  i      / stacks;
            float k1 = (float) (i + 1) / stacks;

            float s0 = fsinf(V_PI * (k0 - 0.5));
            float c0 = fcosf(V_PI * (k0 - 0.5));
            float s1 = fsinf(V_PI * (k1 - 0.5));
            float c1 = fcosf(V_PI * (k1 - 0.5));

            glBegin(GL_QUAD_STRIP);
            {
                for (j = 0; j <= slices; j++)
                {
                    float k = (float) j / slices;
                    float s = fsinf(V_PI * k * 2.0);
                    float c = fcosf(V_PI * k * 2.0);

                    glTexCoord2f(k, k0);
                    glNormal3f(s * c0, c * c0, s0);
                    glVertex3f(s * c0, c * c0, s0);

                    glTexCoord2f(k, k1);
                    glNormal3f(s * c1, c * c1, s1);
                    glVertex3f(s * c1, c * c1, s1);
                }
            }
            glEnd();
        }
#endif /* OCTA_BALL */
    }
    glEndList();
}

void ball_free(void)
{
    if (glIsList(ball_list))
        glDeleteLists(ball_list, 1);

    if (glIsTexture(ball_text))
        glDeleteTextures(1, &ball_text);

    ball_list = 0;
    ball_text = 0;
}

void ball_draw(void)
{
    static const float a[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    static const float s[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    static const float e[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    static const float h[1] = { 20.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   a);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  s);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  e);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, h);

    glEnable(GL_COLOR_MATERIAL);
    {
        glBindTexture(GL_TEXTURE_2D, ball_text);

        /* Render the ball back to front in case it is translucent. */

        glDepthMask(GL_FALSE);
        {
            glCullFace(GL_FRONT);
            glCallList(ball_list);
            glCullFace(GL_BACK);
            glCallList(ball_list);
        }
        glDepthMask(GL_TRUE);

        /* Render the ball into the depth buffer. */

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        {
            glCallList(ball_list);
        }
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        /* Ensure the ball is visible even when obscured by geometry. */

        glDisable(GL_DEPTH_TEST);
        {
            glColor4f(1.0f, 1.0f, 1.0f, 0.1f);
            glCallList(ball_list);
        }
        glEnable(GL_DEPTH_TEST);
    }
    glDisable(GL_COLOR_MATERIAL);
}

/*---------------------------------------------------------------------------*/

static GLuint mark_list;

void mark_init(int b)
{
    int i, slices = b ? 32 : 16;

    mark_list = glGenLists(1);

    glNewList(mark_list, GL_COMPILE);
    {
        glBegin(GL_TRIANGLE_FAN);
        {
            glNormal3f(0.f, 1.f, 0.f);

            for (i = 0; i < slices; i++)
            {
                float x = fcosf(-2.f * PI * i / slices);
                float y = fsinf(-2.f * PI * i / slices);

                glVertex3f(x, 0, y);
            }
        }
        glEnd();
    }
    glEndList();
}

void mark_draw(void)
{
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_FALSE);
    {
        glCallList(mark_list);
    }
    glDepthMask(GL_TRUE);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_COLOR_MATERIAL);
}

void mark_free(void)
{
    if (glIsList(mark_list))
        glDeleteLists(mark_list, 1);

    mark_list = 0;
}

/*---------------------------------------------------------------------------*/

static void coin_head(int n, float radius, float thick)
{
    int i;

    glBegin(GL_TRIANGLE_FAN);
    {
        glNormal3f(0.f, 0.f, +1.f);

        for (i = 0; i < n; i++)
        {
            float x = fcosf(+2.f * PI * i / n);
            float y = fsinf(+2.f * PI * i / n);

            glTexCoord2f(-x * 0.5f + 0.5f, +y * 0.5f + 0.5f);
            glVertex3f(radius * x, radius * y, +thick);
        }
    }
    glEnd();
}

static void coin_tail(int n, float radius, float thick)
{
    int i;

    glBegin(GL_TRIANGLE_FAN);
    {
        glNormal3f(0.f, 0.f, -1.f);

        for (i = 0; i < n; i++)
        {
            float x = fcosf(-2.f * PI * i / n);
            float y = fsinf(-2.f * PI * i / n);

            glTexCoord2f(+x * 0.5f + 0.5f, +y * 0.5f + 0.5f);
            glVertex3f(radius * x, radius * y, -thick);
        }
    }
    glEnd();
}

static void coin_edge(int n, float radius, float thick)
{
    int i;

    glBegin(GL_QUAD_STRIP);
    {
        for (i = 0; i <= n; i++)
        {
            float x = fcosf(2.f * PI * i / n);
            float y = fsinf(2.f * PI * i / n);

            glNormal3f(x, y, 0.0f);
            glVertex3f(radius * x, radius * y, +thick);
            glVertex3f(radius * x, radius * y, -thick);
        }
    }
    glEnd();
}

/*---------------------------------------------------------------------------*/

static GLuint item_coin_text;
static GLuint item_grow_text;
static GLuint item_shrink_text;
static GLuint item_list;

void item_color(const struct s_item *hp, float *c)
{
    switch (hp->t)
    {

    case ITEM_COIN:

        if (hp->n >= 1)
        {
            c[0] = 1.0f;
            c[1] = 1.0f;
            c[2] = 0.2f;
        }
        if (hp->n >= 5)
        {
            c[0] = 1.0f;
            c[1] = 0.2f;
            c[2] = 0.2f;
        }
        if (hp->n >= 10)
        {
            c[0] = 0.2f;
            c[1] = 0.2f;
            c[2] = 1.0f;
        }
        break;

    case ITEM_GROW:
    case ITEM_SHRINK:

    default:

        c[0] = 1.0f;
        c[1] = 1.0f;
        c[2] = 1.0f;

        break;
    }
}

void item_init(int b)
{
    int n = b ? 32 : 8;

    item_coin_text   = make_image_from_file(IMG_ITEM_COIN);
    item_grow_text   = make_image_from_file(IMG_ITEM_GROW);
    item_shrink_text = make_image_from_file(IMG_ITEM_SHRINK);

    item_list = glGenLists(1);

    glNewList(item_list, GL_COMPILE);
    {
        glDisable(GL_TEXTURE_2D);
        coin_edge(n, COIN_RADIUS, COIN_THICK);
        glEnable (GL_TEXTURE_2D);
        coin_head(n, COIN_RADIUS, COIN_THICK);
        coin_tail(n, COIN_RADIUS, COIN_THICK);
    }
    glEndList();
}

void item_free(void)
{
    if (glIsList(item_list))
        glDeleteLists(item_list, 1);

    if (glIsTexture(item_coin_text))
        glDeleteTextures(1, &item_coin_text);

    if (glIsTexture(item_grow_text))
        glDeleteTextures(1, &item_grow_text);

    if (glIsTexture(item_shrink_text))
        glDeleteTextures(1, &item_shrink_text);

    item_list = 0;
    item_coin_text = 0;
    item_grow_text = 0;
    item_shrink_text = 0;
}

void item_push(int type)
{
    static const float  a[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    static const float  s[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    static const float  e[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    static const float  h[1] = { 10.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   a);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  s);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  e);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, h);

    glEnable(GL_COLOR_MATERIAL);

    switch (type)
    {
    case ITEM_COIN:
        glBindTexture(GL_TEXTURE_2D, item_coin_text);
        break;

    case ITEM_GROW:
        glBindTexture(GL_TEXTURE_2D, item_grow_text);
        break;

    case ITEM_SHRINK:
        glBindTexture(GL_TEXTURE_2D, item_shrink_text);
        break;
    }
}

void item_draw(const struct s_item *hp, float r)
{
    float c[3];

    item_color(hp, c);

    glColor3fv(c);
    glCallList(item_list);
}

void item_pull(void)
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glDisable(GL_COLOR_MATERIAL);
}

/*---------------------------------------------------------------------------*/

static GLuint goal_list;

void goal_init(int b)
{
    int i, n = b ? 32 : 8;

    goal_list = glGenLists(1);

    glNewList(goal_list, GL_COMPILE);
    {
        glBegin(GL_QUAD_STRIP);
        {
            for (i = 0; i <= n; i++)
            {
                float x = fcosf(2.f * PI * i / n);
                float y = fsinf(2.f * PI * i / n);

                glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
                glVertex3f(x, 0.0f, y);

                glColor4f(1.0f, 1.0f, 0.0f, 0.0f);
                glVertex3f(x, GOAL_HEIGHT, y);
            }
        }
        glEnd();
    }
    glEndList();
}

void goal_free(void)
{
    if (glIsList(goal_list))
        glDeleteLists(goal_list, 1);

    goal_list = 0;
}

void goal_draw(void)
{
    glCallList(goal_list);
}

/*---------------------------------------------------------------------------*/

static GLuint jump_list;

void jump_init(int b)
{
    int k, i, n = b ? 32 : 8;

    jump_list = glGenLists(2);

    for (k = 0; k < 2; k++)
    {
        glNewList(jump_list + k, GL_COMPILE);
        {
            glBegin(GL_QUAD_STRIP);
            {
                for (i = 0; i <= n; i++)
                {
                    float x = fcosf(2.f * PI * i / n);
                    float y = fsinf(2.f * PI * i / n);

                    glColor4f(1.0f, 1.0f, 1.0f, (k == 0 ? 0.5f : 0.8f));
                    glVertex3f(x, 0.0f, y);

                    glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
                    glVertex3f(x, JUMP_HEIGHT, y);
                }
            }
            glEnd();
        }
        glEndList();
    }
}

void jump_free(void)
{
    glDeleteLists(jump_list, 2);
    jump_list = 0;
}

void jump_draw(int highlight)
{
    glCallList(jump_list + highlight);
}

/*---------------------------------------------------------------------------*/

static GLuint swch_list;

static GLfloat swch_colors[8][4] = {
    { 1.0f, 0.0f, 0.0f, 0.5f }, /* red out */
    { 1.0f, 0.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f, 0.8f }, /* red in */
    { 1.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 0.5f }, /* green out */
    { 0.0f, 1.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 0.8f }, /* green in */
    { 0.0f, 1.0f, 0.0f, 0.0f }};

void swch_init(int b)
{
    int k, i, n = b ? 32 : 8;

    swch_list = glGenLists(4);

    /* Create the display lists. */

    for (k = 0; k < 4; k++)
    {
        glNewList(swch_list + k, GL_COMPILE);
        {
            glBegin(GL_QUAD_STRIP);
            {
                for (i = 0; i <= n; i++)
                {
                    float x = fcosf(2.f * PI * i / n);
                    float y = fsinf(2.f * PI * i / n);

                    glColor4fv(swch_colors[2 * k + 0]);
                    glVertex3f(x, 0.0f, y);

                    glColor4fv(swch_colors[2 * k + 1]);
                    glVertex3f(x, SWCH_HEIGHT, y);
                }
            }
            glEnd();
        }
        glEndList();
    }
}

void swch_free(void)
{
    if (glIsList(swch_list))
        glDeleteLists(swch_list, 2);

    swch_list = 0;
}

void swch_draw(int b, int e)
{
    glCallList(swch_list + b * 2 + e);
}

/*---------------------------------------------------------------------------*/

static GLuint flag_list;

void flag_init(int b)
{
    int i, n = b ? 8 : 4;

    flag_list = glGenLists(1);

    glNewList(flag_list, GL_COMPILE);
    {
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        {
            glBegin(GL_TRIANGLES);
            {
                glColor3f(1.0f, 0.0f, 0.0f);

                glVertex3f(              0.0f, GOAL_HEIGHT,        0.0f);
                glVertex3f(GOAL_HEIGHT * 0.2f, GOAL_HEIGHT * 0.9f, 0.0f);
                glVertex3f(              0.0f, GOAL_HEIGHT * 0.8f, 0.0f);

                glVertex3f(              0.0f, GOAL_HEIGHT,        0.0f);
                glVertex3f(              0.0f, GOAL_HEIGHT * 0.8f, 0.0f);
                glVertex3f(GOAL_HEIGHT * 0.2f, GOAL_HEIGHT * 0.9f, 0.0f);
            }
            glEnd();

            glBegin(GL_QUAD_STRIP);
            {
                for (i = 0; i <= n; i++)
                {
                    float x = fcosf(2.f * PI * i / n) * 0.01f;
                    float y = fsinf(2.f * PI * i / n) * 0.01f;

                    glColor3f(1.0f, 1.0f, 1.0f);
                    glVertex3f(x, 0.0f,        y);
                    glVertex3f(x, GOAL_HEIGHT, y);
                }
            }
            glEnd();
        }
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        glDisable(GL_COLOR_MATERIAL);
    }
    glEndList();
}

void flag_free(void)
{
    if (glIsList(flag_list))
        glDeleteLists(flag_list, 1);

    flag_list = 0;
}

void flag_draw(void)
{
    glCallList(flag_list);
}

/*---------------------------------------------------------------------------*/
/*
 * A note about lighting and shadow: technically speaking, it's wrong.
 * The  light  position  and   shadow  projection  behave  as  if  the
 * light-source rotates with the  floor.  However, the skybox does not
 * rotate, thus the light should also remain stationary.
 *
 * The  correct behavior  would eliminate  a significant  3D  cue: the
 * shadow of  the ball indicates  the ball's position relative  to the
 * floor even  when the ball is  in the air.  This  was the motivating
 * idea  behind the  shadow  in  the first  place,  so correct  shadow
 * projection would only magnify the problem.
 */

static GLuint shad_text;

void shad_init(void)
{
    shad_text = make_image_from_file(IMG_SHAD);

    if (config_get_d(CONFIG_SHADOW) == 2)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
}

void shad_free(void)
{
    if (glIsTexture(shad_text))
        glDeleteTextures(1, &shad_text);
}

void shad_draw_set(const float *p, float r)
{
    glMatrixMode(GL_TEXTURE);
    {
        float k = 0.25f / r;

        glBindTexture(GL_TEXTURE_2D, shad_text);

        glLoadIdentity();
        glTranslatef(0.5f - k * p[0],
                     0.5f - k * p[2], 0.f);
        glScalef(k, k, 1.0f);
    }
    glMatrixMode(GL_MODELVIEW);
}

void shad_draw_clr(void)
{
    glMatrixMode(GL_TEXTURE);
    {
        glLoadIdentity();
    }
    glMatrixMode(GL_MODELVIEW);
}

/*---------------------------------------------------------------------------*/

void fade_draw(float k)
{
    if (k > 0.0f)
    {
        int w = config_get_d(CONFIG_WIDTH);
        int h = config_get_d(CONFIG_HEIGHT);

        config_push_ortho();
        {
            glEnable(GL_COLOR_MATERIAL);
            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_TEXTURE_2D);

            glColor4f(0.0f, 0.0f, 0.0f, k);

            glBegin(GL_QUADS);
            {
                glVertex2i(0, 0);
                glVertex2i(w, 0);
                glVertex2i(w, h);
                glVertex2i(0, h);
            }
            glEnd();

            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

            glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_LIGHTING);
            glDisable(GL_COLOR_MATERIAL);
        }
        config_pop_matrix();
    }
}

/*---------------------------------------------------------------------------*/
