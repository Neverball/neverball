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
#include <math.h>

#include "glext.h"
#include "hud.h"
#include "text.h"
#include "game.h"
#include "level.h"
#include "config.h"

/*---------------------------------------------------------------------------*/

static GLuint large_text[10];           /* Large digit texture objects */
static GLuint large_list[10];           /* Large digit display lists   */
static int    large_w = 0;              /* Large digit layout size     */
static int    large_h = 0;

static GLuint small_text[10];           /* Small digit texture objects */
static GLuint small_list[10];           /* Small digit display lists   */
static int    small_w = 0;              /* Small digit layout size     */
static int    small_h = 0;

static GLuint coins_text;               /* Coins label texture object  */
static GLuint coins_list;               /* Coins label display list    */
static int    coins_w = 0;              /* Coins label layout size     */
static int    coins_h = 0;

static GLuint balls_text;               /* Balls label texture object  */
static GLuint balls_list;               /* Balls label display list    */
static int    balls_w = 0;              /* Balls label layout size     */
static int    balls_h = 0;

static int    space_w = 0;              /* HUD layout spacing          */

/*---------------------------------------------------------------------------*/

void hud_init(void)
{
    char buf[2];
    int i;

    text_size("0",       TXT_MED, &small_w, &small_h);
    text_size("0",       TXT_LRG, &large_w, &large_h);
    text_size(STR_BALLS, TXT_SML, &balls_w, &balls_h);
    text_size(STR_COINS, TXT_SML, &coins_w, &coins_h);

    space_w = small_w / 2;

    for (i = 0; i < 10; i++)
    {
        buf[0] = '0' + i;
        buf[1] =  0;

        small_text[i] = make_text(buf, TXT_MED);
        large_text[i] = make_text(buf, TXT_LRG);

        small_list[i] = make_list(buf, TXT_MED, c_yellow, c_red);
        large_list[i] = make_list(buf, TXT_LRG, c_yellow, c_red);
    }

    balls_text = make_text(STR_BALLS, TXT_SML);
    coins_text = make_text(STR_COINS, TXT_SML);

    balls_list = make_list(STR_BALLS, TXT_SML, c_white, c_white);
    coins_list = make_list(STR_COINS, TXT_SML, c_white, c_white);
}

void hud_free(void)
{
    int i;

    glDeleteLists(coins_list, 1);
    glDeleteLists(balls_list, 1);

    glDeleteTextures(1, &balls_text);
    glDeleteTextures(1, &coins_text);

    for (i = 0; i < 10; i++)
    {
        glDeleteLists(large_list[i], 1);
        glDeleteLists(small_list[i], 1);

        glDeleteTextures(1, large_text + i);
        glDeleteTextures(1, small_text + i);
    }
}

/*---------------------------------------------------------------------------*/

static void hud_draw_small(int d, int x, int y)
{
    glPushMatrix();
    {
        glTranslated((GLdouble) x, (GLdouble) y, 0.0);

        glBindTexture(GL_TEXTURE_2D, small_text[d]);
        glCallList(small_list[d]);
    }
    glPopMatrix();
}

static void hud_draw_large(int d, int x, int y)
{
    glPushMatrix();
    {
        glTranslated((GLdouble) x, (GLdouble) y, 0.0);

        glBindTexture(GL_TEXTURE_2D, large_text[d]);
        glCallList(large_list[d]);
    }
    glPopMatrix();
}

static void hud_draw_labels(void)
{
    int W = config_w();
    int y = (small_h - coins_h) / 2;

    glPushMatrix();
    {
        glTranslated((GLdouble) space_w, (GLdouble) y, 0.0);

        glBindTexture(GL_TEXTURE_2D, balls_text);
        glCallList(balls_list);
    }
    glPopMatrix();

    glPushMatrix();
    {
        glTranslated((GLdouble) (W - coins_w - space_w), (GLdouble) y, 0.0);

        glBindTexture(GL_TEXTURE_2D, coins_text);
        glCallList(coins_list);
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

/*
 * Draw the transparent grey rectangles behind the HUD.
 */
static void hud_draw_back(void)
{
    const int W = config_w();
    const int C = config_w() / 2;

    glDisable(GL_TEXTURE_2D);
    {
        const int a = 2 * space_w + 2 * small_w + coins_w;
        const int b =     space_w +     large_w + small_w;

        glColor4fv(c_grey);

        glRecti(C - b, 0, C + b, large_h);
        glRecti(0,     0, a,     small_h);
        glRecti(W - a, 0, W,     small_h);
    }
    glEnable(GL_TEXTURE_2D);
}

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

/*---------------------------------------------------------------------------*/

void hud_draw(void)
{
    const int W = config_w();
    const int C = config_w() / 2;

    const double clock = curr_clock();
    const int    balls = curr_balls();
    const int    coins = curr_coins();

    const int s = (int) floor(clock);
    const int h = (int) (100.0 * (clock - s));

    config_push_ortho();
    {
        glPushAttrib(GL_LIGHTING_BIT);
        glPushAttrib(GL_DEPTH_BUFFER_BIT);
        {
            const int tx = C - large_w - small_w;
            const int bx =     balls_w + space_w;
            const int cx = W - coins_w - space_w - small_w * 2;
            const int hy = (large_h - small_h) / 2;

            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_COLOR_MATERIAL);

            hud_draw_back();
            hud_draw_labels();

            hud_draw_large((s / 10), tx,           0);
            hud_draw_large((s % 10), tx + large_w, 0);
            hud_draw_small((h / 10), tx + large_w + large_w,           hy);
            hud_draw_small((h % 10), tx + large_w + large_w + small_w, hy);

            hud_draw_small((balls / 10), bx,           0);
            hud_draw_small((balls % 10), bx + small_w, 0);

            hud_draw_small((coins / 10), cx,           0);
            hud_draw_small((coins % 10), cx + small_w, 0);

            if (config_fps()) hud_draw_fps();
        }
        glPopAttrib();
        glPopAttrib();
    }
    config_pop_matrix();
}

/*---------------------------------------------------------------------------*/
