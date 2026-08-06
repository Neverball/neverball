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

#include <math.h>

#include "hud.h"
#include "geom.h"
#include "gui.h"
#include "transition.h"
#include "vec3.h"
#include "game.h"
#include "hole.h"
#include "audio.h"
#include "course.h"
#include "config.h"
#include "video.h"
#include "version.h"
#include "lang.h"
#include "key.h"

#include "st_all.h"
#include "st_conf.h"

/*---------------------------------------------------------------------------*/

static char *number(int i)
{
    static char str[MAXSTR];

    sprintf(str, "%02d", i);

    return str;
}

static int score_card(const char  *title,
                      const GLubyte *c0,
                      const GLubyte *c1)
{
    int id, jd, kd, ld;

    int p1 = (curr_party() >= 1) ? 1 : 0;
    int p2 = (curr_party() >= 2) ? 1 : 0;
    int p3 = (curr_party() >= 3) ? 1 : 0;
    int p4 = (curr_party() >= 4) ? 1 : 0;

    int i;
    int n = curr_count() - 1;
    int m = curr_count() / 2;

    if ((id = gui_vstack(0)))
    {
        gui_label(id, title, GUI_MED, c0, c1);
        gui_space(id);

        if ((jd = gui_hstack(id)))
        {
            if ((kd = gui_varray(jd)))
            {
                if (p1) gui_label(kd, _("O"),      GUI_SML, 0, 0);
                if (p1) gui_label(kd, hole_out(0), GUI_SML, gui_wht, gui_wht);
                if (p1) gui_label(kd, hole_out(1), GUI_SML, gui_red, gui_wht);
                if (p2) gui_label(kd, hole_out(2), GUI_SML, gui_grn, gui_wht);
                if (p3) gui_label(kd, hole_out(3), GUI_SML, gui_blu, gui_wht);
                if (p4) gui_label(kd, hole_out(4), GUI_SML, gui_yel, gui_wht);

                gui_set_rect(kd, GUI_RGT);
            }

            if ((kd = gui_harray(jd)))
            {
                for (i = m; i > 0; i--)
                    if ((ld = gui_varray(kd)))
                    {
                        if (p1) gui_label(ld, number(i), GUI_SML, 0, 0);
                        if (p1) gui_label(ld, hole_score(i, 0), GUI_SML, gui_wht, gui_wht);
                        if (p1) gui_label(ld, hole_score(i, 1), GUI_SML, gui_red, gui_wht);
                        if (p2) gui_label(ld, hole_score(i, 2), GUI_SML, gui_grn, gui_wht);
                        if (p3) gui_label(ld, hole_score(i, 3), GUI_SML, gui_blu, gui_wht);
                        if (p4) gui_label(ld, hole_score(i, 4), GUI_SML, gui_yel, gui_wht);
                    }

                gui_set_rect(kd, GUI_LFT);
            }

            if ((kd = gui_vstack(jd)))
            {
                gui_space(kd);

                if ((ld = gui_varray(kd)))
                {
                    if (p1) gui_label(ld, _("Par"), GUI_SML, gui_wht, gui_wht);
                    if (p1) gui_label(ld, _("P1"),  GUI_SML, gui_red, gui_wht);
                    if (p2) gui_label(ld, _("P2"),  GUI_SML, gui_grn, gui_wht);
                    if (p3) gui_label(ld, _("P3"),  GUI_SML, gui_blu, gui_wht);
                    if (p4) gui_label(ld, _("P4"),  GUI_SML, gui_yel, gui_wht);

                    gui_set_rect(ld, GUI_ALL);
                }
            }
        }

        gui_space(id);

        if ((jd = gui_hstack(id)))
        {
            if ((kd = gui_varray(jd)))
            {
                if (p1) gui_label(kd, _("Tot"),    GUI_SML, 0, 0);
                if (p1) gui_label(kd, hole_tot(0), GUI_SML, gui_wht, gui_wht);
                if (p1) gui_label(kd, hole_tot(1), GUI_SML, gui_red, gui_wht);
                if (p2) gui_label(kd, hole_tot(2), GUI_SML, gui_grn, gui_wht);
                if (p3) gui_label(kd, hole_tot(3), GUI_SML, gui_blu, gui_wht);
                if (p4) gui_label(kd, hole_tot(4), GUI_SML, gui_yel, gui_wht);

                gui_set_rect(kd, GUI_ALL);
            }

            if ((kd = gui_varray(jd)))
            {
                if (p1) gui_label(kd, _("I"),     GUI_SML, 0, 0);
                if (p1) gui_label(kd, hole_in(0), GUI_SML, gui_wht, gui_wht);
                if (p1) gui_label(kd, hole_in(1), GUI_SML, gui_red, gui_wht);
                if (p2) gui_label(kd, hole_in(2), GUI_SML, gui_grn, gui_wht);
                if (p3) gui_label(kd, hole_in(3), GUI_SML, gui_blu, gui_wht);
                if (p4) gui_label(kd, hole_in(4), GUI_SML, gui_yel, gui_wht);

                gui_set_rect(kd, GUI_RGT);
            }

            if ((kd = gui_harray(jd)))
            {
                for (i = n; i > m; i--)
                    if ((ld = gui_varray(kd)))
                    {
                        if (p1) gui_label(ld, number(i), GUI_SML, 0, 0);
                        if (p1) gui_label(ld, hole_score(i, 0), GUI_SML, gui_wht, gui_wht);
                        if (p1) gui_label(ld, hole_score(i, 1), GUI_SML, gui_red, gui_wht);
                        if (p2) gui_label(ld, hole_score(i, 2), GUI_SML, gui_grn, gui_wht);
                        if (p3) gui_label(ld, hole_score(i, 3), GUI_SML, gui_blu, gui_wht);
                        if (p4) gui_label(ld, hole_score(i, 4), GUI_SML, gui_yel, gui_wht);
                    }

                gui_set_rect(kd, GUI_LFT);
            }

            if ((kd = gui_vstack(jd)))
            {
                gui_space(kd);

                if ((ld = gui_varray(kd)))
                {
                    if (p1) gui_label(ld, _("Par"), GUI_SML, gui_wht, gui_wht);
                    if (p1) gui_label(ld, _("P1"),  GUI_SML, gui_red, gui_wht);
                    if (p2) gui_label(ld, _("P2"),  GUI_SML, gui_grn, gui_wht);
                    if (p3) gui_label(ld, _("P3"),  GUI_SML, gui_blu, gui_wht);
                    if (p4) gui_label(ld, _("P4"),  GUI_SML, gui_yel, gui_wht);

                    gui_set_rect(ld, GUI_ALL);
                }
            }
        }

        gui_layout(id, 0, 0);
    }

    return id;
}

/*---------------------------------------------------------------------------*/

static int shared_stick_basic(int id, int a, float v, int bump)
{
    int jd;

    if ((jd = gui_stick(id, a, v, bump)))
        gui_pulse(jd, 1.2f);

    return jd;
}

static void shared_stick(int id, int a, float v, int bump)
{
    shared_stick_basic(id, a, v, bump);
}

/*---------------------------------------------------------------------------*/

#define TITLE_PLAY 1
#define TITLE_CONF 2
#define TITLE_EXIT 3

static int title_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case TITLE_PLAY: return goto_state(&st_course);
    case TITLE_CONF: return goto_state(&st_conf);
    case TITLE_EXIT: return 0;
    }
    return 1;
}

static int title_enter(struct state *st, struct state *prev, int intent)
{
    int id, jd, kd;
    int root_id;

    /* Build the title GUI. */

    if ((root_id = gui_root()))
    {
        if ((id = gui_vstack(root_id)))
        {
            gui_label(id, "Neverputt", GUI_LRG, 0, 0);
            gui_space(id);

            if ((jd = gui_harray(id)))
            {
                gui_filler(jd);

                if ((kd = gui_varray(jd)))
                {
                    gui_start(kd, gt_prefix("menu^Play"),    GUI_MED, TITLE_PLAY, 1);
                    gui_state(kd, gt_prefix("menu^Options"), GUI_MED, TITLE_CONF, 0);
                    gui_state(kd, gt_prefix("menu^Exit"),    GUI_MED, TITLE_EXIT, 0);
                }

                gui_filler(jd);
            }
            gui_layout(id, 0, 0);
        }

#if ENABLE_VERSION
        if ((id = gui_label(root_id, "Neverputt " VERSION, GUI_TNY, gui_wht2, gui_wht2)))
        {
            gui_clr_rect(id);
            gui_layout(id, -1, -1);
        }
#endif
    }

    course_init();
    course_rand();

    return transition_slide(root_id, 1, intent);
}

static int title_leave(struct state *st, struct state *next, int id, int intent)
{
    if (next == &st_conf)
    {
        /*
         * This is ugly, but better than stupidly deleting stuff using
         * object names from a previous GL context.
         */
        course_free();
    }

    return transition_slide(id, 0, intent);
}

static void title_paint(int id, float t)
{
    game_draw(0, t);
    gui_paint(id);
}

static void title_timer(int id, float dt)
{
    float g[3] = { 0.f, 0.f, 0.f };

    game_step(g, dt);
    game_set_fly(fcosf(time_state() / 10.f));

    gui_timer(id, dt);
}

static void title_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static int title_click(int b, int d)
{
    return gui_click(b, d) ? title_action(gui_token(gui_active())) : 1;
}

static int title_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return title_action(gui_token(gui_active()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return title_action(TITLE_EXIT);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int desc_id;
static int shot_id;

#define COURSE_BACK -1

static int course_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    if (course_exists(i))
    {
        course_goto(i);
        goto_state(&st_party);
    }
    if (i == COURSE_BACK)
        exit_state(&st_title);

    return 1;
}

static int comp_size(int n, int s)
{
    return n <= s * s ? s : comp_size(n, s + 1);
}

static int comp_cols(int n)
{
    return comp_size(n, 1);
}

static int comp_rows(int n)
{
    int s = comp_size(n, 1);

    return n <= s * (s - 1) ? s - 1 : s;
}

static int course_enter(struct state *st, struct state *prev, int intent)
{
    int w = video.device_w;
    int h = video.device_h;

    int id, jd, kd, ld, md;

    int i, j, r, c, n;

    n = course_count();

    r = comp_rows(n);
    c = comp_cols(n);

    if ((id = gui_vstack(0)))
    {
        gui_label(id, _("Select Course"), GUI_MED, 0, 0);
        gui_space(id);

        if ((jd = gui_hstack(id)))
        {
            shot_id = gui_image(jd, course_shot(0), w / 3, h / 3);

            gui_filler(jd);

            if ((kd = gui_varray(jd)))
            {
                for(i = 0; i < r; i++)
                {
                    if ((ld = gui_harray(kd)))
                    {
                        for (j = c - 1; j >= 0; j--)
                        {
                            int k = i * c + j;

                            if (k < n)
                            {
                                md = gui_image(ld, course_shot(k),
                                               w / 3 / c, h / 3 / r);
                                gui_set_state(md, k, 0);

                                if (k == 0)
                                    gui_focus(md);
                            }
                            else
                                gui_space(ld);
                        }
                    }
                }
            }
        }

        gui_space(id);
        desc_id = gui_multi(id, _(course_desc(0)), GUI_SML, gui_yel, gui_wht);
        gui_space(id);

        if ((jd = gui_hstack(id)))
        {
            gui_filler(jd);
            gui_state(jd, _("Back"), GUI_SML, COURSE_BACK, 0);
        }

        gui_layout(id, 0, 0);
    }

    audio_music_fade_to(0.5f, "bgm/inter.ogg");

    return transition_slide(id, 1, intent);
}

static int course_leave(struct state *st, struct state *next, int id, int intent)
{
    return transition_slide(id, 0, intent);
}

static void course_paint(int id, float t)
{
    game_draw(0, t);
    gui_paint(id);
}

static void course_timer(int id, float dt)
{
    gui_timer(id, dt);
}

static void course_point(int id, int x, int y, int dx, int dy)
{
    int jd;

    if ((jd = gui_point(id, x, y)))
    {
        int i = gui_token(jd);

        if (course_exists(i))
        {
            gui_set_image(shot_id, course_shot(i));
            gui_set_multi(desc_id, _(course_desc(i)));
        }
        gui_pulse(jd, 1.2f);
    }
}

static void course_stick(int id, int a, float v, int bump)
{
    int jd;

    if ((jd = shared_stick_basic(id, a, v, bump)))
    {
        int i = gui_token(jd);

        if (course_exists(i))
        {
            gui_set_image(shot_id, course_shot(i));
            gui_set_multi(desc_id, _(course_desc(i)));
        }
        gui_pulse(jd, 1.2f);
    }
}

static int course_click(int b, int d)
{
    return gui_click(b, d) ? course_action(gui_token(gui_active())) : 1;
}

static int course_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return course_action(gui_token(gui_active()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return course_action(COURSE_BACK);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

#define PARTY_T 0
#define PARTY_1 1
#define PARTY_2 2
#define PARTY_3 3
#define PARTY_4 4
#define PARTY_B 5

static int party_action(int i)
{
    switch (i)
    {
    case PARTY_1:
        audio_play(AUD_MENU, 1.f);
        if (hole_goto(1, 1))
            goto_state(&st_next);
        break;
    case PARTY_2:
        audio_play(AUD_MENU, 1.f);
        if (hole_goto(1, 2))
            goto_state(&st_next);
        break;
    case PARTY_3:
        audio_play(AUD_MENU, 1.f);
        if (hole_goto(1, 3))
            goto_state(&st_next);
        break;
    case PARTY_4:
        audio_play(AUD_MENU, 1.f);
        if (hole_goto(1, 4))
            goto_state(&st_next);
        break;
    case PARTY_B:
        audio_play(AUD_MENU, 1.f);
        exit_state(&st_course);
        break;
    }
    return 1;
}

static int party_enter(struct state *st, struct state *prev, int intent)
{
    int id, jd;

    if ((id = gui_vstack(0)))
    {
        gui_label(id, _("Players?"), GUI_MED, 0, 0);
        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            int p4 = gui_state(jd, "4", GUI_LRG, PARTY_4, 0);
            int p3 = gui_state(jd, "3", GUI_LRG, PARTY_3, 0);
            int p2 = gui_state(jd, "2", GUI_LRG, PARTY_2, 0);
            int p1 = gui_state(jd, "1", GUI_LRG, PARTY_1, 0);

            gui_set_color(p1, gui_red, gui_wht);
            gui_set_color(p2, gui_grn, gui_wht);
            gui_set_color(p3, gui_blu, gui_wht);
            gui_set_color(p4, gui_yel, gui_wht);

            gui_focus(p1);
        }

        gui_space(id);

        if ((jd = gui_hstack(id)))
        {
            gui_filler(jd);
            gui_state(jd, _("Back"), GUI_SML, PARTY_B, 0);
        }

        gui_layout(id, 0, 0);
    }

    return transition_slide(id, 1, intent);
}

static int party_leave(struct state *st, struct state *next, int id, int intent)
{
    return transition_slide(id, 0, intent);
}

static void party_paint(int id, float t)
{
    game_draw(0, t);
    gui_paint(id);
}

static void party_timer(int id, float dt)
{
    gui_timer(id, dt);
}

static void party_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static int party_click(int b, int d)
{
    return gui_click(b, d) ? party_action(gui_token(gui_active())) : 1;
}

static int party_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return party_action(gui_token(gui_active()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return party_action(PARTY_B);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int paused = 0;

static struct state *st_continue;
static struct state *st_quit;

#define PAUSE_CONTINUE 1
#define PAUSE_QUIT     2
#define PAUSE_SKIP     3

int goto_pause(struct state *s)
{
    if (curr_state() == &st_pause)
        return 1;

    st_continue = curr_state();
    st_quit = s;
    paused = 1;

    return goto_state(&st_pause);
}

static int pause_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    switch(i)
    {
    case PAUSE_CONTINUE:
        return goto_state(st_continue ? st_continue : &st_title);

    case PAUSE_SKIP:
        hole_skip();
        return goto_state(&st_score);

    case PAUSE_QUIT:
        return goto_state(st_quit);
    }
    return 1;
}

static int pause_enter(struct state *st, struct state *prev, int intent)
{
    int id, jd, td;

    audio_music_fade_out(0.2f);

    if ((id = gui_vstack(0)))
    {
        td = gui_label(id, _("Paused"), GUI_LRG, 0, 0);
        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_state(jd, _("Quit"), GUI_SML, PAUSE_QUIT, 0);
            gui_state(jd, _("Skip"), GUI_SML, PAUSE_SKIP, 1);
            gui_start(jd, _("Continue"), GUI_SML, PAUSE_CONTINUE, 2);
        }

        gui_pulse(td, 1.2f);
        gui_layout(id, 0, 0);
    }

    hud_init();
    return transition_slide(id, 1, intent);
}

static int pause_leave(struct state *st, struct state *next, int id, int intent)
{
    hud_free();
    audio_music_fade_in(0.5f);
    return transition_slide(id, 0, intent);
}

static void pause_paint(int id, float t)
{
    game_draw(0, t);
    gui_paint(id);
    hud_paint();
}

static void pause_timer(int id, float dt)
{
    gui_timer(id, dt);
}

static void pause_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static int pause_click(int b, int d)
{
    return gui_click(b, d) ? pause_action(gui_token(gui_active())) : 1;
}

static int pause_keybd(int c, int d)
{
    if (d && c == KEY_EXIT)
        return pause_action(PAUSE_CONTINUE);
    return 1;
}

static int pause_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return pause_action(gui_token(gui_active()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b) ||
            config_tst_d(CONFIG_JOYSTICK_BUTTON_START, b))
            return pause_action(PAUSE_CONTINUE);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int shared_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT)
            return goto_pause(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int num = 0;

static int next_enter(struct state *st, struct state *prev, int intent)
{
    int id, jd;
    char str[MAXSTR];

    sprintf(str, _("Hole %02d"), curr_hole());

    if ((id = gui_vstack(0)))
    {
        gui_label(id, str, GUI_MED, 0, 0);
        gui_space(id);

        if ((jd = gui_vstack(id)))
        {
            gui_label(jd, _("Player"), GUI_SML, 0, 0);

            switch (curr_player())
            {
            case 1:
                gui_label(jd, "1", GUI_LRG, gui_red, gui_wht);
                if (curr_party() > 1) audio_play(AUD_PLAYER1, 1.f);
                break;
            case 2:
                gui_label(jd, "2", GUI_LRG, gui_grn, gui_wht);
                if (curr_party() > 1) audio_play(AUD_PLAYER2, 1.f);
                break;
            case 3:
                gui_label(jd, "3", GUI_LRG, gui_blu, gui_wht);
                if (curr_party() > 1) audio_play(AUD_PLAYER3, 1.f);
                break;
            case 4:
                gui_label(jd, "4", GUI_LRG, gui_yel, gui_wht);
                if (curr_party() > 1) audio_play(AUD_PLAYER4, 1.f);
                break;
            }

            gui_set_rect(jd, GUI_ALL);
        }
        gui_layout(id, 0, 0);
    }

    hud_init();
    game_set_fly(1.f);

    if (paused)
        paused = 0;

    return transition_slide(id, 1, intent);
}

static int next_leave(struct state *st, struct state *next, int id, int intent)
{
    hud_free();
    return transition_slide(id, 0, intent);
}

static void next_paint(int id, float t)
{
    game_draw(0, t);
    hud_paint();
    gui_paint(id);
}

static void next_timer(int id, float dt)
{
    gui_timer(id, dt);
}

static void next_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static int next_click(int b, int d)
{
    return (d && b == SDL_BUTTON_LEFT) ? goto_state(&st_flyby) : 1;
}

static int next_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_POSE)
            return goto_state(&st_poser);
        if (c == KEY_EXIT)
            return goto_pause(&st_over);
        if ('0' <= c && c <= '9')
            num = num * 10 + c - '0';
    }
    return 1;
}

static int next_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
        {
            if (num > 0)
            {
                if (hole_goto(num, -1))
                {
                    num = 0;
                    return goto_state(&st_next);
                }
                else
                {
                    num = 0;
                    return 1;
                }
            }
            return goto_state(&st_flyby);
        }
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_pause(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int poser_enter(struct state *st, struct state *prev, int intent)
{
    game_set_fly(-1.f);
    return 0;
}

static void poser_paint(int id, float t)
{
    game_draw(1, t);
}

static int poser_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_next);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_next);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int flyby_enter(struct state *st, struct state *prev, int intent)
{
    video_hide_cursor();

    if (paused)
        paused = 0;
    else
        hud_init();

    return 0;
}

static int flyby_leave(struct state *st, struct state *next, int id, int intent)
{
    video_show_cursor();
    hud_free();
    return 0;
}

static void flyby_paint(int id, float t)
{
    game_draw(0, t);
    hud_paint();
}

static void flyby_timer(int id, float dt)
{
    float t = time_state();

    if (dt > 0.f && t > 1.f)
        goto_state(&st_stroke);
    else
        game_set_fly(1.f - t);

    gui_timer(id, dt);
}

static int flyby_click(int b, int d)
{
    if (d && b == SDL_BUTTON_LEFT)
    {
        game_set_fly(0.f);
        return goto_state(&st_stroke);
    }
    return 1;
}

static int flyby_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
        {
            game_set_fly(0.f);
            return goto_state(&st_stroke);
        }
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b) ||
            config_tst_d(CONFIG_JOYSTICK_BUTTON_START, b))
            return goto_pause(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int stroke_rotate = 0;
static int stroke_rotate_alt = 0;
static int stroke_mag = 0;

static int stroke_enter(struct state *st, struct state *prev, int intent)
{
    hud_init();
    game_clr_mag();
    config_set_d(CONFIG_CAMERA, 2);
    video_set_grab(1);

    if (paused)
        paused = 0;

    return 0;
}

static int stroke_leave(struct state *st, struct state *next, int id, int intent)
{
    hud_free();
    video_clr_grab();
    config_set_d(CONFIG_CAMERA, 0);
    stroke_rotate = 0.0f;
    stroke_mag = 0.0f;
    return 0;
}

static void stroke_paint(int id, float t)
{
    game_draw(0, t);
    hud_paint();
}

static void stroke_timer(int id, float dt)
{
    float g[3] = { 0.f, 0.f, 0.f };

    float k;

    if (SDL_GetModState() & KMOD_SHIFT || stroke_rotate_alt)
        k = 0.25;
    else
        k = 1.0;

    game_set_rot(stroke_rotate * k);
    game_set_mag(stroke_mag * k);

    game_update_view(dt);
    game_step(g, dt);
}

static void stroke_point(int id, int x, int y, int dx, int dy)
{
    game_set_rot(dx);
    game_set_mag(dy);
}

static void stroke_stick(int id, int a, float v, int bump)
{
    if      (config_tst_d(CONFIG_JOYSTICK_AXIS_X0, a))
        stroke_rotate = 6 * v;
    else if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y0, a))
        stroke_mag = -6 * v;
}

static int stroke_click(int b, int d)
{
    return (d && b == SDL_BUTTON_LEFT) ? goto_state(&st_roll) : 1;
}

static int stroke_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_X, b))
            stroke_rotate_alt = 1;
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_roll);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_START, b))
            return goto_pause(&st_over);
    }
    else
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_X, b))
            stroke_rotate_alt = 0;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int roll_enter(struct state *st, struct state *prev, int intent)
{
    video_hide_cursor();
    hud_init();

    if (paused)
        paused = 0;
    else
        game_putt();

    return 0;
}

static int roll_leave(struct state *st, struct state *next, int id, int intent)
{
    video_show_cursor();
    hud_free();
    return 0;
}

static void roll_paint(int id, float t)
{
    game_draw(0, t);
    hud_paint();
}

static void roll_timer(int id, float dt)
{
    float g[3] = { 0.0f, -9.8f, 0.0f };

    switch (game_step(g, dt))
    {
    case GAME_STOP: goto_state(&st_stop); break;
    case GAME_GOAL: goto_state(&st_goal); break;
    case GAME_FALL: goto_state(&st_fall); break;
    }
}

static int roll_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b) ||
            config_tst_d(CONFIG_JOYSTICK_BUTTON_START, b))
            return goto_pause(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int goal_enter(struct state *st, struct state *prev, int intent)
{
    int id;

    if ((id = gui_label(0, _("It's In!"), GUI_MED, gui_grn, gui_grn)))
        gui_layout(id, 0, 0);

    if (paused)
        paused = 0;
    else
        hole_goal();

    hud_init();

    return transition_slide(id, 1, intent);
}

static int goal_leave(struct state *st, struct state *next, int id, int intent)
{
    hud_free();
    return transition_slide(id, 0, intent);
}

static void goal_paint(int id, float t)
{
    game_draw(0, t);
    gui_paint(id);
    hud_paint();
}

static void goal_timer(int id, float dt)
{
    gui_timer(id, dt);

    if (time_state() > 3)
    {
        if (hole_next())
            goto_state(&st_next);
        else
            goto_state(&st_score);
    }
}

static int goal_click(int b, int d)
{
    if (b == SDL_BUTTON_LEFT && d == 1)
    {
        if (hole_next())
            goto_state(&st_next);
        else
            goto_state(&st_score);
    }
    return 1;
}

static int goal_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
        {
            if (hole_next())
                goto_state(&st_next);
            else
                goto_state(&st_score);
        }
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_pause(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int stop_enter(struct state *st, struct state *prev, int intent)
{
    if (paused)
        paused = 0;
    else
        hole_stop();

    hud_init();

    return 0;
}

static int stop_leave(struct state *st, struct state *next, int id, int intent)
{
    hud_free();
    return 0;
}

static void stop_paint(int id, float t)
{
    game_draw(0, t);
    hud_paint();
}

static void stop_timer(int id, float dt)
{
    float g[3] = { 0.f, 0.f, 0.f };

    game_update_view(dt);
    game_step(g, dt);

    if (time_state() > 1)
    {
        if (hole_next())
            goto_state(&st_next);
        else
            goto_state(&st_score);
    }
}

static int stop_click(int b, int d)
{
    if (b == SDL_BUTTON_LEFT && d == 1)
    {
        if (hole_next())
            goto_state(&st_next);
        else
            goto_state(&st_score);
    }
    return 1;
}

static int stop_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
        {
            if (hole_next())
                goto_state(&st_next);
            else
                goto_state(&st_score);
        }
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b) ||
            config_tst_d(CONFIG_JOYSTICK_BUTTON_START, b))
            return goto_pause(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int fall_enter(struct state *st, struct state *prev, int intent)
{
    int id;

    if ((id = gui_label(0, _("1 Stroke Penalty"), GUI_MED, gui_blk, gui_red)))
        gui_layout(id, 0, 0);

    if (paused)
        paused = 0;
    else
    {
        hole_fall();
/*        game_draw(0);*/ /*TODO: is this call ok? */  /* No, it's not. */
    }

    hud_init();

    return transition_slide(id, 1, intent);
}

static int fall_leave(struct state *st, struct state *next, int id, int intent)
{
    hud_free();
    return transition_slide(id, 0, intent);
}

static void fall_paint(int id, float t)
{
    game_draw(0, t);
    gui_paint(id);
    hud_paint();
}

static void fall_timer(int id, float dt)
{
    gui_timer(id, dt);

    if (time_state() > 3)
    {
        if (hole_next())
            goto_state(&st_next);
        else
            goto_state(&st_score);
    }
}

static int fall_click(int b, int d)
{
    if (b == SDL_BUTTON_LEFT && d == 1)
    {
        if (hole_next())
            goto_state(&st_next);
        else
            goto_state(&st_score);
    }
    return 1;
}

static int fall_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
        {
            if (hole_next())
                goto_state(&st_next);
            else
                goto_state(&st_score);
        }
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_pause(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int score_enter(struct state *st, struct state *prev, int intent)
{
    audio_music_fade_out(2.f);

    if (paused)
        paused = 0;

    return transition_slide(score_card(_("Scores"), gui_yel, gui_red), 1, intent);
}

static int score_leave(struct state *st, struct state *next, int id, int intent)
{
    return transition_slide(id, 0, intent);
}

static void score_paint(int id, float t)
{
    game_draw(0, t);
    gui_paint(id);
}

static void score_timer(int id, float dt)
{
    gui_timer(id, dt);
}

static int score_click(int b, int d)
{
    if (b == SDL_BUTTON_LEFT && d == 1)
    {
        if (hole_move())
            return goto_state(&st_next);
        else
            return goto_state(&st_title);
    }
    return 1;
}

static int score_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
        {
            if (hole_move())
                goto_state(&st_next);
            else
                goto_state(&st_title);
        }
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_pause(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int over_enter(struct state *st, struct state *prev, int intent)
{
    audio_music_fade_out(2.f);
    return transition_slide(score_card(_("Final Scores"), gui_yel, gui_red), 1, intent);
}

static int over_leave(struct state *st, struct state *next, int id, int intent)
{
    return transition_slide(id, 0, intent);
}

static void over_paint(int id, float t)
{
    game_draw(0, t);
    gui_paint(id);
}

static void over_timer(int id, float dt)
{
    gui_timer(id, dt);
}

static int over_click(int b, int d)
{
    return (d && b == SDL_BUTTON_LEFT) ? goto_state(&st_title) : 1;
}

static int over_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_title);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return goto_state(&st_title);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_title = {
    title_enter,
    title_leave,
    title_paint,
    title_timer,
    title_point,
    shared_stick,
    NULL,
    title_click,
    NULL,
    title_buttn
};

struct state st_course = {
    course_enter,
    course_leave,
    course_paint,
    course_timer,
    course_point,
    course_stick,
    NULL,
    course_click,
    NULL,
    course_buttn
};

struct state st_party = {
    party_enter,
    party_leave,
    party_paint,
    party_timer,
    party_point,
    shared_stick,
    NULL,
    party_click,
    NULL,
    party_buttn
};

struct state st_next = {
    next_enter,
    next_leave,
    next_paint,
    next_timer,
    next_point,
    shared_stick,
    NULL,
    next_click,
    next_keybd,
    next_buttn
};

struct state st_poser = {
    poser_enter,
    NULL,
    poser_paint,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    poser_buttn
};

struct state st_flyby = {
    flyby_enter,
    flyby_leave,
    flyby_paint,
    flyby_timer,
    NULL,
    NULL,
    NULL,
    flyby_click,
    shared_keybd,
    flyby_buttn
};

struct state st_stroke = {
    stroke_enter,
    stroke_leave,
    stroke_paint,
    stroke_timer,
    stroke_point,
    stroke_stick,
    NULL,
    stroke_click,
    shared_keybd,
    stroke_buttn
};

struct state st_roll = {
    roll_enter,
    roll_leave,
    roll_paint,
    roll_timer,
    NULL,
    NULL,
    NULL,
    NULL,
    shared_keybd,
    roll_buttn
};

struct state st_goal = {
    goal_enter,
    goal_leave,
    goal_paint,
    goal_timer,
    NULL,
    NULL,
    NULL,
    goal_click,
    shared_keybd,
    goal_buttn
};

struct state st_stop = {
    stop_enter,
    stop_leave,
    stop_paint,
    stop_timer,
    NULL,
    NULL,
    NULL,
    stop_click,
    shared_keybd,
    stop_buttn
};

struct state st_fall = {
    fall_enter,
    fall_leave,
    fall_paint,
    fall_timer,
    NULL,
    NULL,
    NULL,
    fall_click,
    shared_keybd,
    fall_buttn
};

struct state st_score = {
    score_enter,
    score_leave,
    score_paint,
    score_timer,
    NULL,
    NULL,
    NULL,
    score_click,
    shared_keybd,
    score_buttn
};

struct state st_over = {
    over_enter,
    over_leave,
    over_paint,
    over_timer,
    NULL,
    NULL,
    NULL,
    over_click,
    NULL,
    over_buttn
};

struct state st_pause = {
    pause_enter,
    pause_leave,
    pause_paint,
    pause_timer,
    pause_point,
    shared_stick,
    NULL,
    pause_click,
    pause_keybd,
    pause_buttn
};
