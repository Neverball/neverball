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
#include "set.h"
#include "game.h"
#include "demo.h"
#include "levels.h"
#include "audio.h"
#include "solid.h"
#include "config.h"
#include "st_shared.h"

#include "st_demo.h"
#include "st_title.h"

extern struct state st_demo_play;
extern struct state st_demo_end;
extern struct state st_demo_del;

/*---------------------------------------------------------------------------*/

#define DEMO_BACK -1
#define DEMO_NEXT -2
#define DEMO_PREV -3

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
    case DEMO_BACK:
        return goto_state(&st_title);

    case DEMO_NEXT:
        first += DEMO_STEP;
        return goto_state(&st_demo);
        break;

    case DEMO_PREV:
        first -= DEMO_STEP;
        return goto_state(&st_demo);
        break;

    default:
        if (level_replay(demo_filename(i)))
            return goto_demo_play(0);
    }
    return 1;
}

static void demo_replay(int id, int i)
{
    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);
    int jd;

    if ((jd = gui_vstack(id)))
    {
        gui_space(jd);

        gui_image(jd, demo_shot(i), w / 6, h / 6);
        gui_state(jd, demo_name(i), GUI_SML, i, 0);

        gui_active(jd, i, 0);
    }
}

static int name_id;
static int time_id;
static int coin_id;
static int date_id;
static int mode_id;
static int player_id;

static void demo_status(int i)
{
    char ds[MAXSTR];
    demo_str_date(i, ds, MAXSTR);
    gui_set_label(name_id,   demo_name(i));
    gui_set_label(date_id,   ds);
    gui_set_label(player_id, demo_player(i));
    gui_set_label(mode_id,   mode_to_str(demo_mode(i)));
    gui_set_count(coin_id,   demo_coins(i));
    gui_set_clock(time_id,   demo_clock(i));
}

static int demo_enter(void)
{
    int i, j;
    int id, jd, kd, ld;

    total = demo_scan();

    id = gui_vstack(0);
    if (total == 0)
    {
	    gui_label(id, _("No Replay"), GUI_MED, GUI_ALL, 0,0);
	    gui_filler(id);
	    gui_multi(id, _("You can save replay of you games.\\Currently, there is no replay saved."), GUI_SML, GUI_ALL, gui_wht, gui_wht);
	    gui_filler(id);
	    gui_start(id, _("Back"), GUI_SML, DEMO_BACK, 0);
            gui_layout(id, 0, 0);
    }
    else
    {
	if ((jd = gui_hstack(id)))
        {

            ld = gui_label(jd, _("Select Replay"), GUI_SML, GUI_ALL, 0,0);
            gui_filler(jd);

	    
	    
	    if (first + DEMO_STEP < total)
		gui_state(jd, _("Next"), GUI_SML, DEMO_NEXT, 0);
	    else
		gui_label(jd, _("Next"), GUI_SML, GUI_ALL, gui_gry, gui_gry);
	    
	    gui_start(jd, _("Back"), GUI_SML, DEMO_BACK, 0);

	    if (first > 0)
                gui_state(jd, _("Prev"), GUI_SML, DEMO_PREV, 0);
	    else
		gui_label(jd, _("Prev"), GUI_SML, GUI_ALL, gui_gry, gui_gry);
	    
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
	if ((jd = gui_hstack(id)))
	{
	    if((kd = gui_vstack(jd)))
	    {
		if ((ld = gui_harray(kd)))
		{
		    coin_id = gui_count(ld, 100,          GUI_SML, GUI_RGT);
		    gui_label(ld, _("Coins"),             GUI_SML, GUI_LFT, gui_wht, gui_wht);
		    time_id = gui_clock(ld, 35000,        GUI_SML, GUI_RGT);
		    gui_label(ld, _("Time"),              GUI_SML, GUI_LFT, gui_wht, gui_wht);
		    name_id = gui_label(ld, demo_name(0), GUI_SML, GUI_RGT, 0, 0);
		}
		if ((ld = gui_harray(kd)))
		{
		    mode_id = gui_label(ld, "..............", GUI_SML, GUI_RGT, 0, 0);
		    gui_label(ld, _("Mode"),                  GUI_SML, GUI_LFT, gui_wht, gui_wht);
		    player_id = gui_label(ld, demo_player(0), GUI_SML, GUI_RGT, 0, 0);
		}
		date_id = gui_label(kd, "X",     GUI_SML, GUI_RGT, 0, 0);
	    }
	    if((kd = gui_vstack(jd)))
	    {
		gui_label(kd, _("Name"), GUI_SML, GUI_LFT, gui_wht, gui_wht);
		gui_label(kd, _("Player"), GUI_SML, GUI_LFT, gui_wht, gui_wht);
		gui_label(kd, _("Date"), GUI_SML, GUI_LFT, gui_wht, gui_wht);
	    }
	}
	gui_layout(id, 0, 0);
        demo_status(0);
    }

    audio_music_fade_to(0.5f, "bgm/inter.ogg");

    return id;
}

static void demo_point(int id, int x, int y, int dx, int dy)
{
    int jd = shared_point_basic(id, x, y);
    int i  = gui_token(jd);
    if (jd && i>=0)
	demo_status(i);
}

static void demo_stick(int id, int a, int v)
{
    int jd = shared_stick_basic(id, a, v);
    int i  = gui_token(jd);
    if (jd && i>=0)
	demo_status(i);
}

static int demo_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return demo_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return demo_action(DEMO_BACK);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int simple_play;
int goto_demo_play(int simple)
{
    simple_play = simple;
    return goto_state(&st_demo_play);
}

static int demo_play_enter(void)
{
    int id;

    if ((id = gui_label(0, _("Replay"), GUI_LRG, GUI_ALL, gui_blu, gui_grn)))
    {
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
            goto_state(&st_demo_end);
            break;
        }
}

static int demo_play_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_demo_end);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

#define DEMO_KEEP  0
#define DEMO_DEL   1
#define DEMO_QUIT  2

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
    }
    return 1;
}

static int demo_end_enter(void)
{
    int id, jd, kd;

    if ((id = gui_vstack(0)))
    {
        kd = gui_label(id, _("Replay Ends"), GUI_MED, GUI_ALL, gui_gry, gui_red);

        if ((jd = gui_harray(id)))
        {
	    if (simple_play)
                gui_start(jd, _("OK"),   GUI_SML,   DEMO_QUIT, 1);
	    else
	    {
                gui_state(jd, _("Delete"), GUI_SML, DEMO_DEL,  0);
                gui_start(jd, _("Keep"),   GUI_SML, DEMO_KEEP, 1);
	    }
        }

        gui_pulse(kd, 1.2f);
        gui_layout(id, 0, 0);
    }
    audio_music_fade_out(2.0f);

    return id;
}

static void demo_end_timer(int id, float dt)
{
    float g[3] = { 0.0f, -9.8f, 0.0f };

    if (time_state() < 2.f)
	game_step(g, dt, 0);
		
    gui_timer(id, dt);
    audio_timer(dt);
}

static int demo_end_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return demo_end_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return demo_end_action(simple_play ? DEMO_QUIT : DEMO_KEEP);
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
            gui_state(jd, _("Yes"), GUI_SML, DEMO_DEL,  0);
            gui_start(jd, _("No"),  GUI_SML, DEMO_KEEP, 1);
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
    NULL,
    demo_play_buttn,
    0
};

struct state st_demo_end = {
    demo_end_enter,
    shared_leave,
    shared_paint,
    demo_end_timer,
    shared_point,
    shared_stick,
    shared_click,
    NULL,
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
