/*   
 * Copyright (C) 2003 Robert Kooima
 *
 * NEVERPUTT is  free software; you can redistribute  it and/or modify
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
#include <math.h>

#include "glext.h"
#include "hud.h"
#include "text.h"
#include "game.h"
#include "hole.h"
#include "config.h"

/*---------------------------------------------------------------------------*/

static GLuint small_text[10];           /* Small digit texture objects */
static GLuint small_list[10];           /* Small digit display lists   */
static int    small_w = 0;              /* Small digit layout size     */
static int    small_h = 0;

/*---------------------------------------------------------------------------*/

void hud_init(void)
{
    char buf[2];
    int i;

    text_size("0", TXT_MED, &small_w, &small_h);

    for (i = 0; i < 10; i++)
    {
        buf[0] = '0' + i;
        buf[1] =  0;

        small_text[i] = make_text(buf, TXT_MED);
        small_list[i] = make_list(buf, TXT_MED, c_yellow, c_red);
    }
}

void hud_free(void)
{
    int i;

    for (i = 0; i < 10; i++)
    {
        glDeleteLists(small_list[i], 1);
        glDeleteTextures(1, small_text + i);
    }
}

/*---------------------------------------------------------------------------*/

static void hud_draw_small(int d, int x, int y)
{
    glPushMatrix();
    {
        glTranslatef((float) x, (float) y, 0.f);

        glBindTexture(GL_TEXTURE_2D, small_text[d]);
        glCallList(small_list[d]);
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

/*
 * Measure  and render  the frames-per-second  counter.  Use  the call
 * frequency to estimate this value.  Average 10 executions to produce
 * a reasonably stable value.
 */
static void hud_draw_fps(void)
{
    static int fps   = 0;
    static int then  = 0;
    static int count = 0;

    if (count > 10)
    {
        int now = SDL_GetTicks();

        fps   = count * 1000 / (now - then);
        then  = now;
        count = 0;
    }
    else count++;

    hud_draw_small((fps / 100),      0,           config_h() - small_h);
    hud_draw_small((fps % 100) / 10, small_w,     config_h() - small_h);
    hud_draw_small((fps % 100) % 10, small_w * 2, config_h() - small_h);
}

void hud_draw(void)
{
    config_push_ortho();
    {
        glPushAttrib(GL_LIGHTING_BIT);
        glPushAttrib(GL_DEPTH_BUFFER_BIT);
        {
            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_COLOR_MATERIAL);

            if (config_fps()) hud_draw_fps();
        }
        glPopAttrib();
        glPopAttrib();
    }
    config_pop_matrix();
}

/*---------------------------------------------------------------------------*/
