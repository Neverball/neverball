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
#include "demo.h"
#include "progress.h"
#include "audio.h"
#include "config.h"
#include "video.h"
#include "cmd.h"

#include "game_common.h"
#include "game_server.h"
#include "game_proxy.h"
#include "game_client.h"

#include "st_play.h"
#include "st_goal.h"
#include "st_fail.h"
#include "st_pause.h"
#include "st_level.h"
#include "st_shared.h"

/*---------------------------------------------------------------------------*/

static int pause_or_exit(void)
{
    if (config_tst_d(CONFIG_KEY_PAUSE, SDLK_ESCAPE))
    {
        return goto_state(&st_pause);
    }
    else
    {
        progress_stat(GAME_NONE);
        progress_stop();

        video_clr_grab();

        return goto_state(&st_exit);
    }
}

/*---------------------------------------------------------------------------*/

static void set_camera(int c)
{
    config_set_d(CONFIG_CAMERA, c);
    hud_view_pulse(c);
}

static void toggle_camera(void)
{
    int cam = (config_tst_d(CONFIG_CAMERA, VIEW_MANUAL) ?
               VIEW_CHASE : VIEW_MANUAL);

    set_camera(cam);
}

static void keybd_camera(int c)
{
    if (config_tst_d(CONFIG_KEY_CAMERA_1, c))
        set_camera(VIEW_CHASE);
    if (config_tst_d(CONFIG_KEY_CAMERA_2, c))
        set_camera(VIEW_LAZY);
    if (config_tst_d(CONFIG_KEY_CAMERA_3, c))
        set_camera(VIEW_MANUAL);

    if (config_tst_d(CONFIG_KEY_CAMERA_TOGGLE, c))
        toggle_camera();

    if (config_cheat() && c == SDLK_4)
        set_camera(config_tst_d(CONFIG_CAMERA, VIEW_TEST1) ?
                   VIEW_TEST2 : VIEW_TEST1);
}

static void click_camera(int b)
{
    if (config_tst_d(CONFIG_MOUSE_CAMERA_1, b))
        set_camera(VIEW_CHASE);
    if (config_tst_d(CONFIG_MOUSE_CAMERA_2, b))
        set_camera(VIEW_LAZY);
    if (config_tst_d(CONFIG_MOUSE_CAMERA_3, b))
        set_camera(VIEW_MANUAL);

    if (config_tst_d(CONFIG_MOUSE_CAMERA_TOGGLE, b))
        toggle_camera();
}

static void buttn_camera(int b)
{
    if (config_tst_d(CONFIG_JOYSTICK_CAMERA_1, b))
        set_camera(VIEW_CHASE);
    if (config_tst_d(CONFIG_JOYSTICK_CAMERA_2, b))
        set_camera(VIEW_LAZY);
    if (config_tst_d(CONFIG_JOYSTICK_CAMERA_3, b))
        set_camera(VIEW_MANUAL);

    if (config_tst_d(CONFIG_JOYSTICK_CAMERA_TOGGLE, b))
        toggle_camera();
}

/*---------------------------------------------------------------------------*/

static int play_ready_gui(void)
{
    int id;

    if ((id = gui_label(0, _("Ready?"), GUI_LRG, 0, 0)))
    {
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    return id;
}

static int play_ready_enter(struct state *st, struct state *prev)
{
    audio_play(AUD_READY, 1.0f);
    video_set_grab(1);

    hud_view_pulse(config_get_d(CONFIG_CAMERA));

    return play_ready_gui();
}

static void play_ready_paint(int id, float t)
{
    game_client_draw(0, t);
    hud_view_paint();
    gui_paint(id);
}

static void play_ready_timer(int id, float dt)
{
    float t = time_state();

    game_client_fly(1.0f - 0.5f * t);

    if (dt > 0.0f && t > 1.0f)
        goto_state(&st_play_set);

    game_step_fade(dt);
    hud_view_timer(dt);
    gui_timer(id, dt);
}

static int play_ready_click(int b, int d)
{
    if (d)
    {
        click_camera(b);

        if (b == SDL_BUTTON_LEFT)
            goto_state(&st_play_loop);
    }
    return 1;
}

static int play_ready_keybd(int c, int d)
{
    if (d)
    {
        keybd_camera(c);

        if (config_tst_d(CONFIG_KEY_PAUSE, c))
            goto_state(&st_pause);
    }
    return 1;
}

static int play_ready_buttn(int b, int d)
{
    if (d)
    {
        buttn_camera(b);

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_play_loop);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return pause_or_exit();
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int play_set_gui(void)
{
    int id;

    if ((id = gui_label(0, _("Set?"), GUI_LRG, 0, 0)))
    {
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    return id;
}

static int play_set_enter(struct state *st, struct state *prev)
{
    audio_play(AUD_SET, 1.f);

    return play_set_gui();
}

static void play_set_paint(int id, float t)
{
    game_client_draw(0, t);
    hud_view_paint();
    gui_paint(id);
}

static void play_set_timer(int id, float dt)
{
    float t = time_state();

    game_client_fly(0.5f - 0.5f * t);

    if (dt > 0.0f && t > 1.0f)
        goto_state(&st_play_loop);

    game_step_fade(dt);
    hud_view_timer(dt);
    gui_timer(id, dt);
}

static int play_set_click(int b, int d)
{
    if (d)
    {
        click_camera(b);

        if (b == SDL_BUTTON_LEFT)
            goto_state(&st_play_loop);
    }
    return 1;
}

static int play_set_keybd(int c, int d)
{
    if (d)
    {
        keybd_camera(c);

        if (config_tst_d(CONFIG_KEY_PAUSE, c))
            goto_state(&st_pause);
    }
    return 1;
}

static int play_set_buttn(int b, int d)
{
    if (d)
    {
        buttn_camera(b);

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_play_loop);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return pause_or_exit();
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct
{
    float R;
    float L;

    enum
    {
        DIR_R = 1,
        DIR_L
    } d;
} view_rotate;

#define VIEWR_SET_R(v) do {                                    \
    view_rotate.R = (v);                                       \
    view_rotate.d = (v) ? DIR_R : (view_rotate.L ? DIR_L : 0); \
} while (0)

#define VIEWR_SET_L(v) do {                                    \
    view_rotate.L = (v);                                       \
    view_rotate.d = (v) ? DIR_L : (view_rotate.R ? DIR_R : 0); \
} while (0)

static int fast_rotate;
static int show_hud;

static int play_loop_gui(void)
{
    int id;

    if ((id = gui_label(0, _("GO!"), GUI_LRG, gui_blu, gui_grn)))
    {
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    return id;
}

static int play_loop_enter(struct state *st, struct state *prev)
{
    VIEWR_SET_R(0);
    VIEWR_SET_L(0);
    fast_rotate = 0;

    if (prev == &st_pause)
        return 0;

    audio_play(AUD_GO, 1.f);

    game_client_fly(0.0f);

    show_hud = 1;
    hud_update(0);

    return play_loop_gui();
}

static void play_loop_paint(int id, float t)
{
    game_client_draw(0, t);

    if (show_hud)
        hud_paint();

    if (time_state() < 1.f)
        gui_paint(id);
}

static void play_loop_timer(int id, float dt)
{
    float k = (fast_rotate ?
               (float) config_get_d(CONFIG_ROTATE_FAST) / 100.0f :
               (float) config_get_d(CONFIG_ROTATE_SLOW) / 100.0f);

    gui_timer(id, dt);
    hud_timer(dt);
    game_set_rot(view_rotate.d == DIR_R ?
                 view_rotate.R * k :
                 view_rotate.L * k);
    game_set_cam(config_get_d(CONFIG_CAMERA));

    game_step_fade(dt);

    game_server_step(dt);
    game_client_sync(demo_fp);
    game_client_blend(game_server_blend());

    switch (curr_status())
    {
    case GAME_GOAL:
        progress_stat(GAME_GOAL);
        goto_state(&st_goal);
        break;

    case GAME_FALL:
        progress_stat(GAME_FALL);
        goto_state(&st_fail);
        break;

    case GAME_TIME:
        progress_stat(GAME_TIME);
        goto_state(&st_fail);
        break;

    default:
        progress_step();
        break;
    }
}

static void play_loop_point(int id, int x, int y, int dx, int dy)
{
    game_set_pos(dx, dy);
}

static void play_loop_stick(int id, int a, float v, int bump)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        game_set_z(v);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        game_set_x(v);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_U, a))
    {
        VIEWR_SET_R(0);
        VIEWR_SET_L(0);

        if (v > 0) VIEWR_SET_R(v);
        if (v < 0) VIEWR_SET_L(v);
    }
}

static int play_loop_click(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_MOUSE_CAMERA_R, b))
            VIEWR_SET_R(+1);
        if (config_tst_d(CONFIG_MOUSE_CAMERA_L, b))
            VIEWR_SET_L(-1);

        click_camera(b);
    }
    else
    {
        if (config_tst_d(CONFIG_MOUSE_CAMERA_R, b))
            VIEWR_SET_R(0);
        if (config_tst_d(CONFIG_MOUSE_CAMERA_L, b))
            VIEWR_SET_L(0);
    }

    return 1;
}

static int play_loop_keybd(int c, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_KEY_CAMERA_R, c))
            VIEWR_SET_R(+1);
        if (config_tst_d(CONFIG_KEY_CAMERA_L, c))
            VIEWR_SET_L(-1);
        if (config_tst_d(CONFIG_KEY_ROTATE_FAST, c))
            fast_rotate = 1;

        keybd_camera(c);

        if (config_tst_d(CONFIG_KEY_RESTART, c) &&
            progress_same_avail())
        {
            if (progress_same())
                goto_state(&st_play_ready);
        }
        if (config_tst_d(CONFIG_KEY_PAUSE, c))
            goto_state(&st_pause);
    }
    else
    {
        if (config_tst_d(CONFIG_KEY_CAMERA_R, c))
            VIEWR_SET_R(0);
        if (config_tst_d(CONFIG_KEY_CAMERA_L, c))
            VIEWR_SET_L(0);
        if (config_tst_d(CONFIG_KEY_ROTATE_FAST, c))
            fast_rotate = 0;
    }

    if (d && c == KEY_LOOKAROUND && config_cheat())
        return goto_state(&st_look);

    if (d && c == KEY_POSE)
        show_hud = !show_hud;

    if (d && c == SDLK_c && config_cheat())
    {
        progress_stat(GAME_GOAL);
        return goto_state(&st_goal);
    }
    return 1;
}

static int play_loop_buttn(int b, int d)
{
    if (d == 1)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            pause_or_exit();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_R, b))
            VIEWR_SET_R(+1);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_L, b))
            VIEWR_SET_L(-1);
        if (config_tst_d(CONFIG_JOYSTICK_ROTATE_FAST, b))
            fast_rotate = 1;

        buttn_camera(b);
    }
    else
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_R, b))
            VIEWR_SET_R(0);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_L, b))
            VIEWR_SET_L(0);
        if (config_tst_d(CONFIG_JOYSTICK_ROTATE_FAST, b))
            fast_rotate = 0;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static float phi;
static float theta;

static int look_enter(struct state *st, struct state *prev)
{
    phi   = 0;
    theta = 0;
    return 0;
}

static void look_leave(struct state *st, struct state *next, int id)
{
}

static void look_paint(int id, float t)
{
    game_client_draw(0, t);
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
    if (d && c == KEY_LOOKAROUND)
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
    play_ready_paint,
    play_ready_timer,
    NULL,
    NULL,
    NULL,
    play_ready_click,
    play_ready_keybd,
    play_ready_buttn
};

struct state st_play_set = {
    play_set_enter,
    shared_leave,
    play_set_paint,
    play_set_timer,
    NULL,
    NULL,
    NULL,
    play_set_click,
    play_set_keybd,
    play_set_buttn
};

struct state st_play_loop = {
    play_loop_enter,
    shared_leave,
    play_loop_paint,
    play_loop_timer,
    play_loop_point,
    play_loop_stick,
    shared_angle,
    play_loop_click,
    play_loop_keybd,
    play_loop_buttn
};

struct state st_look = {
    look_enter,
    look_leave,
    look_paint,
    NULL,
    look_point,
    NULL,
    NULL,
    NULL,
    look_keybd,
    look_buttn
};
