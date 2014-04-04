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
#include "image.h"
#include "config.h"

#include "solid_draw.h"

/*---------------------------------------------------------------------------*/

#define COIN_MAX 3

static struct s_full item_coin_file[COIN_MAX];
static struct s_full item_grow_file;
static struct s_full item_shrink_file;

/*
 * Colors for coin bursts. TODO, turn into a material/text file?
 */
void item_color(const struct v_item *hp, float *c)
{
    switch (hp->t)
    {
    case ITEM_COIN:

        if (hp->n >= 1)
        {
            c[0] = 1.0f;
            c[1] = 1.0f;
            c[2] = 0.2f;
            c[3] = 1.0f;
        }
        if (hp->n >= 5)
        {
            c[0] = 1.0f;
            c[1] = 0.2f;
            c[2] = 0.2f;
            c[3] = 1.0f;
        }
        if (hp->n >= 10)
        {
            c[0] = 0.2f;
            c[1] = 0.2f;
            c[2] = 1.0f;
            c[3] = 1.0f;
        }
        break;

    case ITEM_GROW:

        c[0] = 0.00f;
        c[1] = 0.51f;
        c[2] = 0.80f;
        c[3] = 1.00f;

        break;

    case ITEM_SHRINK:

        c[0] = 1.00f;
        c[1] = 0.76f;
        c[2] = 0.00f;
        c[3] = 1.00f;

        break;

    default:

        c[0] = 1.0f;
        c[1] = 1.0f;
        c[2] = 1.0f;
        c[3] = 1.0f;

        break;
    }
}

void item_init(void)
{
    static const char coin_sols[COIN_MAX][PATHMAX] = {
        "item/coin/coin.sol",
        "item/coin/coin5.sol",
        "item/coin/coin10.sol"
    };

    int i;

    for (i = 0; i < COIN_MAX; i++)
        sol_load_full(&item_coin_file[i], coin_sols[i], 0);

    sol_load_full(&item_grow_file,   "item/grow/grow.sol",     0);
    sol_load_full(&item_shrink_file, "item/shrink/shrink.sol", 0);
}

void item_free(void)
{
    int i;

    for (i = 0; i < COIN_MAX; i++)
        sol_free_full(&item_coin_file[i]);

    sol_free_full(&item_grow_file);
    sol_free_full(&item_shrink_file);
}

void item_draw(struct s_rend *rend,
               const struct v_item *hp,
               const GLfloat *M, float t)
{
    const GLfloat s = ITEM_RADIUS;

    struct s_draw *draw;

    switch (hp->t)
    {
    case ITEM_GROW:
        draw = &item_grow_file.draw;
        break;

    case ITEM_SHRINK:
        draw = &item_shrink_file.draw;
        break;

    default:
        if      (hp->n >= 10)
            draw = &item_coin_file[2].draw;
        else if (hp->n >= 5)
            draw = &item_coin_file[1].draw;
        else
            draw = &item_coin_file[0].draw;
        break;
    }

    glPushMatrix();
    {
        glScalef(s, s, s);

        glDepthMask(GL_FALSE);
        {
            sol_bill(draw, rend, M, t);
        }
        glDepthMask(GL_TRUE);

        glRotatef(360.0f * t, 0.0f, 1.0f, 0.0f);
        sol_draw(draw, rend, 0, 1);
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

