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

#include <string.h>

#include "gui.h"
#include "hud.h"
#include "set.h"
#include "game.h"
#include "demo.h"
#include "levels.h"
#include "audio.h"
#include "solid.h"
#include "config.h"
#include "st_shared.h"
#include "util.h"

#include "st_demo.h"
#include "st_title.h"

/*---------------------------------------------------------------------------*/

#define DEMO_LINE 4
#define DEMO_STEP 8

static int first = 0;
static int total = 0;

static float replay_time;
static float global_time;

/*---------------------------------------------------------------------------*/

static int demo_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case GUI_BACK:
        return goto_state(&st_title);

    case GUI_NEXT:
        first += DEMO_STEP;
        return goto_state(&st_demo);
        break;

    case GUI_PREV:
        first -= DEMO_STEP;
        return goto_state(&st_demo);
        break;

    case GUI_NULL:
        return 1;
        break;

    default:
        if (level_replay(demo_get(i)->filename))
            demo_play_goto(0);
            return goto_state(&st_demo_play);
    }
    return 1;
}

static void demo_replay(int id, int i)
{
    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);
    int jd;

    char nam[MAXNAM + 3];

    if ((jd = gui_vstack(id)))
    {
        gui_space(jd);
        gui_image(jd, demo_get(i)->shot, w / 6, h / 6);

        nam[MAXNAM - 1] = '\0';
        strncpy(nam, demo_get(i)->name, MAXNAM);
        if (nam[MAXNAM - 1] != '\0')
        {
            nam[MAXNAM - 2] = '.';
            nam[MAXNAM - 1] = '.';
            nam[MAXNAM + 0] = '.';
            nam[MAXNAM + 1] = '\0';
        }
        gui_state(jd, nam, GUI_SML, i, 0);

        gui_active(jd, i, 0);
    }
}

static int name_id;
static int time_id;
static int coin_id;
static int date_id;
static int mode_id;
static int state_id;
static int player_id;

/* Create a layout for some demo info.  If d is NULL, try to reserve enough
 * space. */

static int gui_demo_status(int id, const struct demo *d)
{
    char noname[MAXNAM];
    const char *mode, *state;
    int i, j, k;
    int jd, kd, ld, md;

    if (d == NULL)
    {
        /* Build a long name */
        memset(noname, 'M', MAXNAM - 1);
        noname[MAXNAM - 1] = '\0';

        /* Get a long mode */
        mode = mode_to_str(0, 0);
        j = strlen(mode);
        for (i = 1; i <= MODE_SINGLE; i++)
        {
            k = strlen(mode_to_str(i, 0));
            if (k > j)
            {
                j = k;
                mode = mode_to_str(i, 0);
            }
        }

        /* Get a long state */
        state = state_to_str(0);
        j = strlen(state);
        for (i = 1; i <= GAME_FALL; i++)
        {
            k = strlen(state_to_str(i));
            if (k > j)
            {
                j = k;
                state = state_to_str(i);
            }
        }
    }
    else
    {
        mode = mode_to_str(d->mode, 0);
        state = state_to_str(d->state);
    }

    if ((jd = gui_hstack(id)))
    {
        if ((kd = gui_vstack(jd)))
        {
            if ((ld = gui_harray(kd)))
            {
                if ((md = gui_vstack(ld)))
                {
                    player_id = gui_label(md, (d ? d->player : noname),
                                          GUI_SML, GUI_RGT, 0, 0);
                    coin_id = gui_count(md, (d ? d->coins : 100),
                                        GUI_SML, GUI_RGT);
                    state_id = gui_label(md, state, GUI_SML, GUI_RGT,
                                         gui_red, gui_red);
                }
                if ((md = gui_vstack(ld)))
                {
                    gui_label(md, _("Player"), GUI_SML, GUI_LFT,
                              gui_wht, gui_wht);
                    gui_label(md, _("Coins"), GUI_SML, GUI_LFT,
                              gui_wht, gui_wht);
                    gui_label(md, _("State"), GUI_SML, GUI_LFT,
                              gui_wht, gui_wht);
                }
                if ((md = gui_vstack(ld)))
                {
                    name_id = gui_label(md, (d ? d->name : noname),
                                        GUI_SML, GUI_RGT, 0, 0);
                    time_id = gui_clock(md, (d ? d->timer : 35000),
                                        GUI_SML, GUI_RGT);
                    mode_id = gui_label(md, mode, GUI_SML, GUI_RGT, 0, 0);
                }
            }
            date_id = gui_label(kd, (d ? date_to_str(d->date) : "M"),
                                GUI_SML, GUI_RGT, 0, 0);
        }
        if ((kd = gui_vstack(jd)))
        {
            gui_label(kd, _("Replay"), GUI_SML, GUI_LFT, gui_wht, gui_wht);
            gui_label(kd, _("Time"),   GUI_SML, GUI_LFT, gui_wht, gui_wht);
            gui_label(kd, _("Mode"),   GUI_SML, GUI_LFT, gui_wht, gui_wht);
            gui_label(kd, _("Date"),   GUI_SML, GUI_LFT, gui_wht, gui_wht);
        }
        if (d && (d->state == GAME_GOAL || d->state == GAME_SPEC))
            gui_set_color(state_id, gui_grn, gui_grn);
    }
    return jd;
}

static void gui_demo_update_status(int i)
{
    const struct demo *d = demo_get(i);

    gui_set_label(name_id,   d->name);
    gui_set_label(date_id,   date_to_str(d->date));
    gui_set_label(player_id, d->player);
    gui_set_label(mode_id,   mode_to_str(d->mode, 0));

    if (d->state == GAME_GOAL || d->state == GAME_SPEC)
        gui_set_color(state_id, gui_grn, gui_grn);
    else
        gui_set_color(state_id, gui_red, gui_red);

    gui_set_label(state_id, state_to_str(d->state));
    gui_set_count(coin_id, d->coins);
    gui_set_clock(time_id, d->timer);
}

static int demo_enter(void)
{
    int i, j;
    int id, jd, kd;

    id = gui_vstack(0);

    if ((total = demo_scan()))
    {
        if ((jd = gui_hstack(id)))
        {

            gui_label(jd, _("Select Replay"), GUI_SML, GUI_ALL, 0,0);
            gui_filler(jd);
            gui_back_prev_next(jd, first > 0, first + DEMO_STEP < total);
        }

        if ((jd = gui_varray(id)))
            for (i = first; i < first + DEMO_STEP ; i += DEMO_LINE)
                if ((kd = gui_harray(jd)))
                {
                    for (j = i + DEMO_LINE - 1; j >= i; j--)
                        if (j < total)
                            demo_replay(kd, j);
                        else
                            gui_space(kd);
                }
        gui_filler(id);
        gui_demo_status(id, NULL);
        gui_layout(id, 0, 0);
        gui_demo_update_status(0);
    }
    else
    {
        gui_label(id, _("No Replays"), GUI_MED, GUI_ALL, 0,0);
        gui_filler(id);
        gui_start(id, _("Back"), GUI_SML, GUI_BACK, 0);
        gui_layout(id, 0, 0);
    }

    audio_music_fade_to(0.5f, "bgm/inter.ogg");

    return id;
}

static void demo_point(int id, int x, int y, int dx, int dy)
{
    int jd = shared_point_basic(id, x, y);
    int i  = gui_token(jd);

    if (jd && i >= 0)
        gui_demo_update_status(i);
}

static void demo_stick(int id, int a, int v)
{
    int jd = shared_stick_basic(id, a, v);
    int i  = gui_token(jd);

    if (jd && i >= 0)
        gui_demo_update_status(i);
}

static int demo_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return demo_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return demo_action(GUI_BACK);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int simple_play; /* play demo from command line */

static int demo_paused;

void demo_play_goto(int simple)
{
    simple_play = simple;
}

static int demo_play_enter(void)
{
    int id;

    if (demo_paused)
    {
        demo_paused = 0;
        audio_music_fade_in(0.5f);
        return 0;
    }

    if ((id = gui_vstack(0)))
    {
        gui_label(id, _("Replay"), GUI_LRG, GUI_ALL, gui_blu, gui_grn);
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    global_time = -1.f;
    replay_time =  0.f;

    hud_update(0);

    game_set_fly(0.f);

    return id;
}

static void demo_play_paint(int id, float st)
{
    game_draw(0, st);
    hud_paint();

    if (time_state() < 1.f)
        gui_paint(id);
}

static void demo_play_timer(int id, float dt)
{
    float t;

    game_step_fade(dt);
    gui_timer(id, dt);
    audio_timer(dt);

    global_time += dt;
    hud_timer(dt);

    /* Spin or skip depending on how fast the demo wants to run. */

    while (replay_time < global_time)
        if (demo_replay_step(&t))
        {
            replay_time += t;
        }
        else
        {
            demo_paused = 0;
            goto_state(&st_demo_end);
            break;
        }
}

static int demo_play_keybd(int c, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_KEY_PAUSE, c))
        {
            demo_paused = 1;
            return goto_state(&st_demo_end);
        }
    }
    return 1;
}

static int demo_play_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
        {
            if (! (SDL_GetModState() & (KMOD_SHIFT | KMOD_CTRL | KMOD_ALT | KMOD_META)))
                demo_paused = 1;
            return goto_state(&st_demo_end);
        }
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

#define DEMO_KEEP    0
#define DEMO_DEL     1
#define DEMO_QUIT    2
#define DEMO_REPLAY  3
#define DEMO_CONTINUE  4

static int demo_end_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case DEMO_DEL:
        return goto_state(&st_demo_del);
    case DEMO_KEEP:
        demo_replay_stop(0);
        return goto_state(&st_demo);
    case DEMO_QUIT:
        demo_replay_stop(0);
        return 0;
    case DEMO_REPLAY:
        demo_replay_stop(0);
        level_replay(curr_demo_replay()->filename);
        return goto_state(&st_demo_play);
    case DEMO_CONTINUE:
        return goto_state(&st_demo_play);
    }
    return 1;
}

static int demo_end_enter(void)
{
    int id, jd, kd;

    if ((id = gui_vstack(0)))
    {
        if (demo_paused)
            kd = gui_label(id, _("Replay Paused"), GUI_LRG, GUI_ALL,
                           gui_gry, gui_red);
        else
            kd = gui_label(id, _("Replay Ends"), GUI_LRG, GUI_ALL,
                           gui_gry, gui_red);

        if ((jd = gui_harray(id)))
        {
            if (demo_paused)
                gui_start(jd, _("Continue"), GUI_SML, DEMO_CONTINUE, 0);
            else
                gui_start(jd, _("Replay Again"), GUI_SML, DEMO_REPLAY, 0);

            if (simple_play)
                gui_start(jd, _("OK"),       GUI_SML, DEMO_QUIT,   1);
            else
            {
                gui_start(jd, _("Keep"),     GUI_SML, DEMO_KEEP,   1);
                gui_state(jd, _("Delete"),   GUI_SML, DEMO_DEL,    0);
            }
        }

        gui_filler(id);

        if ((jd = gui_hstack(id)))
        {
            gui_filler(jd);
            gui_demo_status(jd, curr_demo_replay());
            gui_filler(jd);
        }

        gui_pulse(kd, 1.2f);
        gui_layout(id, 0, 0);
    }

    audio_music_fade_out(demo_paused ? 0.2f : 2.0f);

    return id;
}

void demo_end_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);

    if (demo_paused)
        hud_paint();
}

static void demo_end_timer(int id, float dt)
{
    float gg[3] = { 0.0f,  9.8f, 0.0f };
    float gf[3] = { 0.0f, -9.8f, 0.0f };
    int state = curr_demo_replay()->state;

    if (time_state() < 2.f)
    {
        if (replay_time < global_time)
        {
            if (state != GAME_NONE && state != GAME_TIME)
                game_step(state == GAME_GOAL ? gg : gf, dt, NULL);
        }
    }

    gui_timer(id, dt);
    audio_timer(dt);
}

static int demo_end_keybd(int c, int d)
{
    if (d)
    {
        if (demo_paused && config_tst_d(CONFIG_KEY_PAUSE, c))
            return demo_end_action(DEMO_CONTINUE);
    }
    return 1;
}

static int demo_end_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return demo_end_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return demo_end_action(demo_paused
                                   ? DEMO_CONTINUE
                                   : (simple_play ? DEMO_QUIT : DEMO_KEEP));
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int demo_del_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    demo_replay_stop(i == DEMO_DEL);
    return goto_state(&st_demo);
}

static int demo_del_enter(void)
{
    int id, jd, kd;

    if ((id = gui_vstack(0)))
    {
        kd = gui_label(id, _("Delete Replay?"), GUI_MED, GUI_ALL, gui_red, gui_red);

        if ((jd = gui_harray(id)))
        {
            gui_start(jd, _("No"),  GUI_SML, DEMO_KEEP, 1);
            gui_state(jd, _("Yes"), GUI_SML, DEMO_DEL,  0);
        }

        gui_pulse(kd, 1.2f);
        gui_layout(id, 0, 0);
    }
    audio_music_fade_out(2.0f);

    return id;
}

static int demo_del_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return demo_del_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return demo_del_action(DEMO_KEEP);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_demo = {
    demo_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    demo_point,
    demo_stick,
    shared_click,
    NULL,
    demo_buttn,
    0
};

struct state st_demo_play = {
    demo_play_enter,
    shared_leave,
    demo_play_paint,
    demo_play_timer,
    NULL,
    NULL,
    NULL,
    demo_play_keybd,
    demo_play_buttn,
    0
};

struct state st_demo_end = {
    demo_end_enter,
    shared_leave,
    demo_end_paint,
    demo_end_timer,
    shared_point,
    shared_stick,
    shared_click,
    demo_end_keybd,
    demo_end_buttn,
    1, 0
};

struct state st_demo_del = {
    demo_del_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_click,
    NULL,
    demo_del_buttn,
    1, 0
};
