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

static GLuint view_text[3];
static GLuint view_list[3];
static int    view_w = 0;
static int    view_h = 0;

static GLuint large_text[10];           /* Large digit texture objects */
static GLuint large_list[10];           /* Large digit display lists   */
static int    large_w = 0;              /* Large digit layout size     */
static int    large_h = 0;

static GLuint small_text[11];           /* Small digit texture objects */
static GLuint small_list[11];           /* Small digit display lists   */
static int    small_w = 0;              /* Small digit layout size     */
static int    small_h = 0;

static GLuint coins_text;               /* Coins label texture object  */
static GLuint coins_list;               /* Coins label display list    */
static GLuint coins_rect;               /* Coins label background      */
static int    coins_w = 0;              /* Coins label layout size     */
static int    coins_h = 0;

static GLuint balls_text;               /* Balls label texture object  */
static GLuint balls_list;               /* Balls label display list    */
static GLuint balls_rect;               /* Balls label background      */
static int    balls_w = 0;              /* Balls label layout size     */
static int    balls_h = 0;

static int    space_w = 0;              /* HUD layout spacing          */
static GLuint timer_rect;               /* Timer label background      */

static float ball_k = 1.f;
static float time_k = 1.f;
static float coin_k = 1.f;
static float view_k = 0.f;
static int   view_c = 0;

/*---------------------------------------------------------------------------*/

void hud_init(void)
{
    const int W = config_w();
    const int C = config_w() / 2;

    char buf[2];
    int a, b, i;

    text_size("0",       TXT_MED, &small_w, &small_h);
    text_size("0",       TXT_LRG, &large_w, &large_h);
    text_size(STR_BALLS, TXT_SML, &balls_w, &balls_h);
    text_size(STR_COINS, TXT_SML, &coins_w, &coins_h);
    text_size(STR_VIEW2, TXT_SML, &view_w,  &view_h);

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

    a =      coins_w + 2 * small_w + 2 * space_w;
    b = (3 * large_w + 2 * small_w + 3 * space_w) / 2;

    small_text[10] = make_text(":", TXT_MED);
    small_list[10] = make_list(":", TXT_MED, c_yellow, c_red);

    balls_text = make_text(STR_BALLS, TXT_SML);
    coins_text = make_text(STR_COINS, TXT_SML);

    balls_list = make_list(STR_BALLS, TXT_SML, c_white, c_white);
    coins_list = make_list(STR_COINS, TXT_SML, c_white, c_white);

    balls_rect = make_rect(0,     0, a,     small_h);
    coins_rect = make_rect(W - a, 0, W,     small_h);
    timer_rect = make_rect(C - b, 0, C + b, large_h);

    view_text[0] = make_text(STR_VIEW0, TXT_SML);
    view_text[1] = make_text(STR_VIEW1, TXT_SML);
    view_text[2] = make_text(STR_VIEW2, TXT_SML);

    view_list[0] = make_list(STR_VIEW0, TXT_SML, c_white, c_white);
    view_list[1] = make_list(STR_VIEW1, TXT_SML, c_white, c_white);
    view_list[2] = make_list(STR_VIEW2, TXT_SML, c_white, c_white);
}

void hud_free(void)
{
    int i;

    for (i = 0; i < 3; i++)
    {
        glDeleteLists(view_list[i], 1);
        glDeleteTextures(1, view_text + i);
    }

    glDeleteLists(timer_rect, 1);
    glDeleteLists(coins_rect, 1);
    glDeleteLists(balls_rect, 1);

    glDeleteLists(coins_list, 1);
    glDeleteLists(balls_list, 1);

    glDeleteTextures(1, &balls_text);
    glDeleteTextures(1, &coins_text);

    glDeleteLists(small_list[10], 1);
    glDeleteTextures(1, small_text + 10);

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
        glTranslatef((float) x, (float) y, 0.f);

        glBindTexture(GL_TEXTURE_2D, small_text[d]);
        glCallList(small_list[d]);
    }
    glPopMatrix();
}

static void hud_draw_large(int d, int x, int y)
{
    glPushMatrix();
    {
        glTranslatef((float) x, (float) y, 0.f);

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
        glTranslatef((float) space_w, (float) y, 0.f);

        glBindTexture(GL_TEXTURE_2D, balls_text);
        glCallList(balls_list);
    }
    glPopMatrix();

    glPushMatrix();
    {
        glTranslatef((float) (W - coins_w - space_w), (float) y, 0.f);

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
    glDisable(GL_TEXTURE_2D);
    {
        glColor4fv(c_grey);

        glCallList(timer_rect);
        glCallList(balls_rect);
        glCallList(coins_rect);
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

    int now = SDL_GetTicks();

    if (now - then > 250)
    {
        fps   = count * 1000 / (now - then);
        then  = now;
        count = 0;
    }
    else count++;

    hud_draw_small((fps / 100),      0,           config_h() - small_h);
    hud_draw_small((fps % 100) / 10, small_w,     config_h() - small_h);
    hud_draw_small((fps % 100) % 10, small_w * 2, config_h() - small_h);
}

static void hud_draw_view(void)
{
    glPushMatrix();
    {
        GLfloat t = ((view_k > 2.f) ? 2.f : view_k) - 1.f;

        glTranslatef(-view_w + view_w * t, config_h() - small_h - view_h, 0.f);

        glBindTexture(GL_TEXTURE_2D, view_text[view_c]);
        glCallList(view_list[view_c]);
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

void hud_draw(void)
{
    const int W = config_w();
    const int C = config_w() / 2;

    const float clock = curr_clock();
    const int   balls = curr_balls();
    const int   coins = curr_coins();

    const int m = (int) floor(clock) / 60;
    const int s = (int) floor(clock) % 60;
    const int h = (int) (100.f * (clock - m * 60 - s));

    config_push_ortho();
    {
        glPushAttrib(GL_LIGHTING_BIT);
        glPushAttrib(GL_COLOR_BUFFER_BIT);
        glPushAttrib(GL_DEPTH_BUFFER_BIT);
        {
            const int tw = 3 * large_w + 2 * small_w + space_w;
            const int tx = C - tw / 2;

            const int bx =     balls_w + space_w;
            const int cx = W - coins_w - space_w - small_w * 2;
            const int hy = (large_h - small_h) / 2;

            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_COLOR_MATERIAL);
            glEnable(GL_BLEND);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            hud_draw_back();
            hud_draw_labels();

            /* Timer. */

            glPushMatrix();
            {
                glTranslatef(tx, 0.f, 0.f);

                glTranslatef(+tw / 2, +large_h / 2, 0.f);
                glScalef(time_k, time_k, 1.f);
                glTranslatef(-tw / 2, -large_h / 2, 0.f);

                hud_draw_large(m,          0,                            0);
                hud_draw_small(10,         large_w,                     hy);
                hud_draw_large(s / 10,     large_w + space_w,            0);
                hud_draw_large(s % 10, 2 * large_w + space_w,            0);
                hud_draw_small(h / 10, 3 * large_w + space_w,           hy);
                hud_draw_small(h % 10, 3 * large_w + space_w + small_w, hy);
            }
            glPopMatrix();

            /* Ball count. */

            glPushMatrix();
            {
                glTranslatef(bx, 0.f, 0.f);

                glTranslatef(+small_w, +small_h / 2, 0.f);
                glScalef(ball_k, ball_k, 1.f);
                glTranslatef(-small_w, -small_h / 2, 0.f);

                hud_draw_small((balls / 10), 0,       0);
                hud_draw_small((balls % 10), small_w, 0);
            }
            glPopMatrix();

            /* Coin count. */

            glPushMatrix();
            {
                glTranslatef(cx, 0.f, 0.f);

                glTranslatef(+small_w, +small_h / 2, 0.f);
                glScalef(coin_k, coin_k, 1.f);
                glTranslatef(-small_w, -small_h / 2, 0.f);

                hud_draw_small((coins / 10), 0,       0);
                hud_draw_small((coins % 10), small_w, 0);
            }
            glPopMatrix();

            if (view_k > 1.f) hud_draw_view();
            if (config_fps()) hud_draw_fps();
        }
        glPopAttrib();
        glPopAttrib();
        glPopAttrib();
    }
    config_pop_matrix();
}


void hud_step(float dt)
{
    if (dt < 0.25)
    {
        ball_k -= (ball_k - 1.f) * dt * 4;
        time_k -= (time_k - 1.f) * dt * 4;
        coin_k -= (coin_k - 1.f) * dt * 4;
        view_k -=  dt;
    }
    else
    {
        ball_k = 1.f;
        time_k = 1.f;
        coin_k = 1.f;
        view_k = 1.f;
    }

    if (ball_k < 0.f) ball_k =  0.f;
    if (time_k < 0.f) time_k =  0.f;
    if (coin_k < 0.f) coin_k =  0.f;
    if (view_k < 0.f) view_k =  0.f;
}

void hud_ball_pulse(float k) { ball_k = k; }
void hud_time_pulse(float k) { time_k = k; }
void hud_coin_pulse(float k) { coin_k = k; }
void hud_view_pulse(int c)   { view_k = 4.f; view_c = c; }

/*---------------------------------------------------------------------------*/
