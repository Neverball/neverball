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

#include "gui.h"
#include "hud.h"
#include "game.h"
#include "demo.h"
#include "levels.h"
#include "audio.h"
#include "config.h"
#include "st_shared.h"

#include "st_play.h"
#include "st_play_end.h"
#include "st_over.h"

/*---------------------------------------------------------------------------*/

static int view_rotate;

/*---------------------------------------------------------------------------*/

static int abort_play(void)
{
   if (curr_lg()->mode == MODE_SINGLE)
       return 0;
   else
       return goto_state(&st_over);
}

static int play_ready_enter(void)
{
    int id;

    if ((id = gui_label(0, _("Ready?"), GUI_LRG, GUI_ALL, 0, 0)))
    {
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    audio_play(AUD_READY, 1.0f);
    config_set_grab();

    return id;
}

static void play_ready_timer(int id, float dt)
{
    float t = time_state();

    game_set_fly(1.0f - 0.5f * t);

    if (dt > 0.0f && t > 1.0f)
        goto_state(&st_play_set);

    game_step_fade(dt);
    gui_timer(id, dt);
    audio_timer(dt);
}

static int play_ready_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_play_loop) : 1;
}

static int play_ready_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_play_loop);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return abort_play();
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int play_set_enter(void)
{
    int id;

    if ((id = gui_label(0, _("Set?"), GUI_LRG, GUI_ALL, 0, 0)))
    {
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    audio_play(AUD_SET, 1.f);

    return id;
}

static void play_set_timer(int id, float dt)
{
    float t = time_state();

    game_set_fly(0.5f - 0.5f * t);

    if (dt > 0.0f && t > 1.0f)
        goto_state(&st_play_loop);

    game_step_fade(dt);
    gui_timer(id, dt);
}

static int play_set_click(int b, int d)
{
    if (b < 0 && d == 1)
    {
        game_set_fly(0.0f);
        return goto_state(&st_play_loop);
    }
    return 1;
}

static int play_set_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_play_loop);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return abort_play();
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int nohud = 0;

static int play_loop_enter(void)
{
    int id;

    if ((id = gui_label(0, _("GO!"), GUI_LRG, GUI_ALL, gui_blu, gui_grn)))
    {
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    audio_play(AUD_GO, 1.f);

    game_set_fly(0.f);
    view_rotate = 0;
    
    hud_view_pulse(config_get_d(CONFIG_CAMERA));

    nohud = 0;

    hud_update(0);

    return id;
}

static void play_loop_paint(int id, float st)
{
    game_draw(0, st);
    if (!nohud)
        hud_paint();

    if (time_state() < 1.f)
        gui_paint(id);
}

static void play_loop_timer(int id, float dt)
{
    float k = (((SDL_GetModState() & KMOD_LSHIFT) ||
                (SDL_GetModState() & KMOD_RSHIFT)) ?
               (float) config_get_d(CONFIG_ROTATE_FAST) / 100.f:
               (float) config_get_d(CONFIG_ROTATE_SLOW) / 100.f);

    static float at = 0;

    float g[3] = { 0.0f, -9.8f, 0.0f };

    int state, state_value;
    
    at = (7 * at + dt) / 8;

    gui_timer(id, at);
    hud_timer(at);
    game_set_rot(view_rotate * k);

    state = game_step(g, at, &state_value);
    if (state)
    {
        level_stop(state, state_value, curr_clock(), curr_coins());
        goto_state(&st_play_end);
    }

    game_step_fade(dt);
    demo_play_step(at);
    audio_timer(dt);
}

static void play_loop_point(int id, int x, int y, int dx, int dy)
{
    game_set_pos(dx, dy);
}

static void play_loop_stick(int id, int a, int k)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        game_set_z(k);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        game_set_x(k);
}

static int play_loop_click(int b, int d)
{
    view_rotate = d ? b : 0;
    return 1;
}

static int play_loop_keybd(int c, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_KEY_CAMERA_R, c))
            view_rotate = +1;
        if (config_tst_d(CONFIG_KEY_CAMERA_L, c))
            view_rotate = -1;

        if (config_tst_d(CONFIG_KEY_CAMERA_1, c))
        {
            config_set_d(CONFIG_CAMERA, 0);
            hud_view_pulse(0);
        }
        if (config_tst_d(CONFIG_KEY_CAMERA_2, c))
        {
            config_set_d(CONFIG_CAMERA, 1);
            hud_view_pulse(1);
        }
        if (config_tst_d(CONFIG_KEY_CAMERA_3, c))
        {
            config_set_d(CONFIG_CAMERA, 2);
            hud_view_pulse(2);
        }
    }
    else
    {
        if (config_tst_d(CONFIG_KEY_CAMERA_R, c))
            view_rotate = 0;
        if (config_tst_d(CONFIG_KEY_CAMERA_L, c))
            view_rotate = 0;
    }

    if (d && c == SDLK_F12)
        return goto_state(&st_look);
    
    if (d && c == SDLK_F6)
        nohud = !nohud;
    
    /* Cheat */
    if (d && c == SDLK_c && config_get_d(CONFIG_CHEAT))
    {
        level_stop(GAME_GOAL, 0, curr_clock(), curr_coins());
        return goto_state(&st_play_end);
    }
    return 1;
}

static int play_loop_buttn(int b, int d)
{
    if (d == 1)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
        {
            level_stop(GAME_NONE, 0, curr_clock(), curr_coins());
            return abort_play();
        }

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_R, b))
            view_rotate = +1;
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_L, b))
            view_rotate = -1;

        if (config_tst_d(CONFIG_JOYSTICK_CAMERA_1, b))
        {
            config_set_d(CONFIG_CAMERA, 0);
            hud_view_pulse(0);
        }
        if (config_tst_d(CONFIG_JOYSTICK_CAMERA_2, b))
        {
            config_set_d(CONFIG_CAMERA, 1);
            hud_view_pulse(1);
        }
        if (config_tst_d(CONFIG_JOYSTICK_CAMERA_3, b))
        {
            config_set_d(CONFIG_CAMERA, 2);
            hud_view_pulse(2);
        }
    }
    else
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_R, b))
            view_rotate = 0;
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_L, b))
            view_rotate = 0;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static float phi;
static float theta;

static int look_enter(void)
{
    phi   = 0;
    theta = 0;
    return 0;
}

static void look_leave(int id)
{
}

static void look_paint(int id, float st)
{
    game_draw(0, st);
}

static void look_point(int id, int x, int y, int dx, int dy)
{
    phi   +=  90.0f * dy / config_get_d(CONFIG_HEIGHT);
    theta += 180.0f * dx / config_get_d(CONFIG_WIDTH);

    if (phi > +90.0f) phi = +90.0f;
    if (phi < -90.0f) phi = -90.0f;

    if (theta > +180.0f) theta -= 360.0f;
    if (theta < -180.0f) theta += 360.0f;

    game_look(phi, theta);
}

static int look_keybd(int c, int d)
{
    if (d && c == SDLK_F12)
        return goto_state(&st_play_loop);

    return 1;
}

static int look_buttn(int b, int d)
{
    if (d && config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
        return goto_state(&st_play_loop);

    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_play_ready = {
    play_ready_enter,
    shared_leave,
    shared_paint,
    play_ready_timer,
    NULL,
    NULL,
    play_ready_click,
    NULL,
    play_ready_buttn,
    1, 0
};

struct state st_play_set = {
    play_set_enter,
    shared_leave,
    shared_paint,
    play_set_timer,
    NULL,
    NULL,
    play_set_click,
    NULL,
    play_set_buttn,
    1, 0
};

struct state st_play_loop = {
    play_loop_enter,
    shared_leave,
    play_loop_paint,
    play_loop_timer,
    play_loop_point,
    play_loop_stick,
    play_loop_click,
    play_loop_keybd,
    play_loop_buttn,
    0, 0
};

struct state st_look = {
    look_enter,
    look_leave,
    look_paint,
    NULL,
    look_point,
    NULL,
    NULL,
    look_keybd,
    look_buttn,
    0, 0
};
