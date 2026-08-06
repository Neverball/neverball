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
#include <string.h>

#include "glext.h"
#include "hud.h"
#include "gui.h"
#include "progress.h"
#include "config.h"
#include "video.h"
#include "audio.h"

#include "game_common.h"
#include "game_client.h"

#include "st_pause.h"

/*---------------------------------------------------------------------------*/

static int Lhud_id;
static int Rhud_id;
static int time_id;
static int Touch_id;

static int coin_id;
static int ball_id;
static int scor_id;
static int goal_id;
static int cam_id;
static int fps_id;
static int lvlname_id = 0;

static int speed_id;
static int speed_ids[SPEED_MAX];

static const char *speed_labels[SPEED_MAX] = {
    "", "8", "4", "2", "1", "2", "4", "8"
};

static float cam_timer;
static float speed_timer;
static float touch_timer;

static void hud_fps(void)
{
    gui_set_count(fps_id, video_perf());
}

void hud_init(void)
{
    int id;
    const char *str_cam;
    int v;

    if ((Rhud_id = gui_hstack(0)))
    {
        if ((id = gui_vstack(Rhud_id)))
        {
            gui_label(id, _("Coins"), GUI_SML, gui_wht, gui_wht);
            gui_label(id, _("Goal"),  GUI_SML, gui_wht, gui_wht);
        }
        if ((id = gui_vstack(Rhud_id)))
        {
            coin_id = gui_count(id, 100, GUI_SML);
            goal_id = gui_count(id, 10,  GUI_SML);
        }
        gui_set_rect(Rhud_id, GUI_NW);
        gui_layout(Rhud_id, +1, -1);
    }

    if ((Lhud_id = gui_hstack(0)))
    {
        if ((id = gui_vstack(Lhud_id)))
        {
            ball_id = gui_count(id, 10,   GUI_SML);
            scor_id = gui_count(id, 1000, GUI_SML);
        }
        if ((id = gui_vstack(Lhud_id)))
        {
            gui_label(id, _("Balls"), GUI_SML, gui_wht, gui_wht);
            gui_label(id, _("Score"), GUI_SML, gui_wht, gui_wht);
        }
        gui_set_rect(Lhud_id, GUI_NE);
        gui_layout(Lhud_id, -1, -1);
    }

    if ((Touch_id = gui_vstack(0)))
    {
        gui_space(Touch_id);

        if ((id = gui_hstack(Touch_id)))
        {
            /* Poor man's pause symbol. */
            gui_state(id, GUI_ROMAN_2, GUI_TCH, GUI_BACK, 0);

            gui_space(id);

            gui_state(id, GUI_FISHEYE, GUI_TCH, GUI_CAMERA, 0);

            gui_space(id);
        }

        // HACK: hide by default.
        gui_set_slide(Touch_id, GUI_N, 0, 0, 0);

        gui_layout(Touch_id, -1, +1);
    }

    if ((time_id = gui_clock(0, 59999, GUI_MED)))
    {
        gui_set_rect(time_id, GUI_TOP);
        gui_layout(time_id, 0, -1);
    }

    /* Find the longest camera name. */

    for (str_cam = "", v = CAM_NONE + 1; v < CAM_MAX; v++)
        if (strlen(cam_to_str(v)) > strlen(str_cam))
            str_cam = cam_to_str(v);

    if ((cam_id = gui_label(0, str_cam, GUI_SML, gui_wht, gui_wht)))
    {
        gui_set_rect(cam_id, GUI_SW);
        gui_layout(cam_id, 1, 1);
    }

    if ((fps_id = gui_count(0, 1000, GUI_SML)))
    {
        gui_set_rect(fps_id, GUI_BOT);
        gui_layout(fps_id, 0, 1);
    }

    if ((speed_id = gui_varray(0)))
    {
        int i;

        for (i = SPEED_MAX - 1; i > SPEED_NONE; i--)
            speed_ids[i] = gui_label(speed_id, speed_labels[i], GUI_SML, 0, 0);

        gui_set_rect(speed_id, GUI_LFT);
        gui_layout(speed_id, +1, 0);
    }
}

void hud_free(void)
{
    int i;

    gui_delete(Rhud_id);
    gui_delete(Lhud_id);
    gui_delete(Touch_id);
    gui_delete(time_id);
    gui_delete(cam_id);
    gui_delete(fps_id);

    gui_delete(speed_id);

    for (i = SPEED_NONE + 1; i < SPEED_MAX; i++)
        gui_delete(speed_ids[i]);
}

void hud_paint(void)
{
    if (curr_mode() == MODE_CHALLENGE)
        gui_paint(Lhud_id);

    gui_paint(Rhud_id);
    gui_paint(time_id);
    gui_paint(lvlname_id);

    if (config_get_d(CONFIG_FPS))
        gui_paint(fps_id);

    hud_cam_paint();
    hud_speed_paint();
    hud_touch_paint();
}

void hud_update(int pulse)
{
    int clock = curr_clock();
    int coins = curr_coins();
    int goal  = curr_goal();
    int balls = curr_balls();
    int score = curr_score();

    int c_id;
    int last;

    if (!pulse)
    {
        /* reset the hud */

        gui_pulse(ball_id, 0.f);
        gui_pulse(time_id, 0.f);
        gui_pulse(coin_id, 0.f);

        speed_timer = 0.0f;
    }

    /* time and tick-tock */

    if (clock != (last = gui_value(time_id)))
    {
        gui_set_clock(time_id, clock);

        if (pulse)
        {
            if (last > clock)
            {
                if (clock <= 1000 && (last / 100) > (clock / 100))
                {
                    audio_play(AUD_TICK, 1.f);
                    gui_pulse(time_id, 1.50);
                }
                else if (clock < 500 && (last / 50) > (clock / 50))
                {
                    audio_play(AUD_TOCK, 1.f);
                    gui_pulse(time_id, 1.25);
                }
            }
            else if (last > 0)
            {
                if (clock > last + 2950)
                    gui_pulse(time_id, 2.00);
                else if (clock > last + 1450)
                    gui_pulse(time_id, 1.50);
                else if (clock > last + 450)
                    gui_pulse(time_id, 1.25);
            }
        }
    }

    /* balls and score + select coin widget */

    switch (curr_mode())
    {
    case MODE_CHALLENGE:
        if (gui_value(ball_id) != balls) gui_set_count(ball_id, balls);
        if (gui_value(scor_id) != score) gui_set_count(scor_id, score);

        c_id = coin_id;
        break;

    default:
        c_id = coin_id;
        break;
    }


    /* coins and pulse */

    if (coins != (last = gui_value(c_id)))
    {
        last = coins - last;

        gui_set_count(c_id, coins);

        if (pulse && last > 0)
        {
            if      (last >= 10) gui_pulse(coin_id, 2.00f);
            else if (last >=  5) gui_pulse(coin_id, 1.50f);
            else                 gui_pulse(coin_id, 1.25f);

            if (goal > 0)
            {
                if      (last >= 10) gui_pulse(goal_id, 2.00f);
                else if (last >=  5) gui_pulse(goal_id, 1.50f);
                else                 gui_pulse(goal_id, 1.25f);
            }
        }
    }

    /* goal and pulse */

    if (goal != (last = gui_value(goal_id)))
    {
        gui_set_count(goal_id, goal);

        if (pulse && goal == 0 && last > 0)
            gui_pulse(goal_id, 2.00f);
    }

    if (config_get_d(CONFIG_FPS))
        hud_fps();
}

void hud_timer(float dt)
{
    hud_update(1);

    gui_timer(Rhud_id, dt);
    gui_timer(Lhud_id, dt);
    gui_timer(Touch_id, dt);
    gui_timer(time_id, dt);

    hud_cam_timer(dt);
    hud_speed_timer(dt);
    hud_touch_timer(dt);
}

void hud_show(float delay)
{
    gui_slide(Lhud_id, GUI_S | GUI_EASE_BACK, delay + 0.0f, 0.3f, 0);
    gui_slide(time_id, GUI_S | GUI_EASE_BACK, delay + 0.1f, 0.3f, 0);
    gui_slide(Rhud_id, GUI_S | GUI_EASE_BACK, delay + 0.2f, 0.3f, 0);
}

void hud_hide(void)
{
    gui_slide(Lhud_id, GUI_S | GUI_EASE_BACK | GUI_BACKWARD, 0, 0.3f, 0);
    gui_slide(time_id, GUI_S | GUI_EASE_BACK | GUI_BACKWARD, 0, 0.3f, 0);
    gui_slide(Rhud_id, GUI_S | GUI_EASE_BACK | GUI_BACKWARD, 0, 0.3f, 0);

    if (touch_timer > 0.0f)
    {
        touch_timer = 0.0f;
        gui_slide(Touch_id, GUI_N | GUI_EASE_BACK | GUI_BACKWARD, 0, 0.3f, 0);
    }
}

int hud_touch(const SDL_TouchFingerEvent *event)
{
    if (touch_timer == 0.0f)
        gui_slide(Touch_id, GUI_N | GUI_EASE_BACK, 0, 0.3f, 0);

    touch_timer = 5.0f;

    if (event->type == SDL_FINGERUP)
    {
        const int x = (int) ((float) video.device_w * event->x);
        const int y = (int) ((float) video.device_h * (1.0f - event->y));

        return gui_point(Touch_id, x, y);
    }

    return 0;
}

void hud_lvlname(const char *name)
{
    gui_delete(lvlname_id);

    if ((lvlname_id = gui_label(0, name, GUI_SML, gui_yel, gui_wht)))
    {
        gui_set_rect(lvlname_id, GUI_SE);
        gui_layout(lvlname_id, -1, +1);
    }
}

/*---------------------------------------------------------------------------*/

void hud_cam_pulse(int c)
{
    gui_set_label(cam_id, cam_to_str(c));
    gui_slide(cam_id, GUI_N | GUI_EASE_BACK, 0, 0.2f, 0);
    cam_timer = 0.0f;
}

void hud_cam_timer(float dt)
{
    gui_timer(cam_id, dt);

    if (cam_timer < 2.0f)
    {
        cam_timer += dt;

        if (cam_timer >= 2.0f)
        {
            cam_timer = 2.0f;
            gui_slide(cam_id, GUI_N | GUI_EASE_BACK | GUI_BACKWARD, 0, 0.2f, 0);
        }
    }
}

void hud_cam_paint(void)
{
    gui_paint(cam_id);
}

/*---------------------------------------------------------------------------*/

void hud_speed_pulse(int speed)
{
    int i;

    for (i = SPEED_NONE + 1; i < SPEED_MAX; i++)
    {
        const GLubyte *c = gui_gry;

        if (speed != SPEED_NONE)
        {
            if      (i > SPEED_NORMAL && i <= speed)
                c = gui_grn;
            else if (i < SPEED_NORMAL && i >= speed)
                c = gui_red;
            else if (i == SPEED_NORMAL)
                c = gui_wht;
        }

        gui_set_color(speed_ids[i], c, c);

        if (i == speed)
            gui_pulse(speed_ids[i], 1.2f);
    }

    speed_timer = 2.0f;
}

void hud_speed_timer(float dt)
{
    speed_timer -= dt;
    gui_timer(speed_id, dt);
}

void hud_speed_paint(void)
{
    if (speed_timer > 0.0f)
        gui_paint(speed_id);
}

/*---------------------------------------------------------------------------*/

void hud_touch_timer(float dt)
{
    if (touch_timer > 0.0f)
    {
        touch_timer -= dt;

        if (touch_timer <= 0.0f)
        {
            touch_timer = 0.0f;
            gui_slide(Touch_id, GUI_N | GUI_EASE_BACK | GUI_BACKWARD, 0, 0.3f, 0);
        }
    }

    gui_timer(Touch_id, dt);
}

void hud_touch_paint(void)
{
    gui_paint(Touch_id);
}

/*---------------------------------------------------------------------------*/
