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

#include "item.h"
#include "glext.h"
#include "vec3.h"
#include "solid.h"
#include "image.h"

#define PI 3.1415926535897932

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

            glTexCoord2f(+x * 0.5f + 0.5f, +y * 0.5f + 0.5f);
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

            glTexCoord2f(-x * 0.5f + 0.5f, +y * 0.5f + 0.5f);
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

