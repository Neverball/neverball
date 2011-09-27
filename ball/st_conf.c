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
#include "geom.h"
#include "item.h"
#include "ball.h"
#include "part.h"
#include "audio.h"
#include "config.h"
#include "video.h"
#include "common.h"

#include "game_common.h"
#include "game_client.h"
#include "game_server.h"

#include "st_conf.h"
#include "st_title.h"
#include "st_resol.h"
#include "st_name.h"
#include "st_ball.h"
#include "st_shared.h"

extern const char TITLE[];
extern const char ICON[];

/*---------------------------------------------------------------------------*/

static void conf_slider(int id, const char *text,
                        int token, int value,
                        int *ids, int num)
{
    int jd, kd, i;

    if ((jd = gui_harray(id)) && (kd = gui_harray(jd)))
    {
        /* A series of empty buttons forms a "slider". */

        for (i = num - 1; i >= 0; i--)
        {
            ids[i] = gui_state(kd, NULL, GUI_SML, token, i);

            gui_set_hilite(ids[i], (i == value));
        }

        gui_label(jd, text, GUI_SML, GUI_ALL, 0, 0);
    }
}

static int conf_state(int id, const char *label, const char *text, int token)
{
    int jd, kd, rd = 0;

    if ((jd = gui_harray(id)) && (kd = gui_harray(jd)))
    {
        rd = gui_state(kd, text, GUI_SML, token, 0);
        gui_label(jd, label, GUI_SML, GUI_ALL, 0, 0);
    }

    return rd;
}

static void conf_toggle(int id, const char *label, int token, int value,
                        const char *text1, int value1,
                        const char *text0, int value0)
{
    int jd, kd;

    if ((jd = gui_harray(id)) && (kd = gui_harray(jd)))
    {
        int btn0, btn1;

        btn0 = gui_state(kd, text0, GUI_SML, token, value0);
        btn1 = gui_state(kd, text1, GUI_SML, token, value1);

        gui_set_hilite(btn0, (value == value0));
        gui_set_hilite(btn1, (value == value1));

        gui_label(jd, label, GUI_SML, GUI_ALL, 0, 0);
    }
}

static void conf_header(int id, const char *text, int token)
{
    int jd;

    if ((jd = gui_harray(id)))
    {
        gui_label(jd, text, GUI_SML, GUI_ALL, 0, 0);
        gui_space(jd);
        gui_start(jd, _("Back"), GUI_SML, token, 0);
    }

    gui_space(id);
}

/*---------------------------------------------------------------------------*/

enum
{
    CONF_SHARED_BACK = GUI_LAST         /* Shared GUI token.                 */
};

static int (*conf_shared_action)(int tok, int val);

static void conf_shared_init(int (*action_fn)(int, int))
{
    conf_shared_action = action_fn;

    game_client_free(NULL);
    back_init("back/gui.png");
    audio_music_fade_to(0.5f, "bgm/inter.ogg");
}

static void conf_shared_leave(struct state *st, struct state *next, int id)
{
    back_free();
    gui_delete(id);
}

static void conf_shared_paint(int id, float t)
{
    video_push_persp((float) config_get_d(CONFIG_VIEW_FOV), 0.1f, FAR_DIST);
    {
        back_draw_easy();
    }
    video_pop_matrix();
    gui_paint(id);
}

static int conf_shared_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return conf_shared_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return conf_shared_action(CONF_SHARED_BACK, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

enum
{
    CONF_BACK = CONF_SHARED_BACK,
    CONF_VIDEO,
    CONF_SOUND_VOLUME,
    CONF_MUSIC_VOLUME,
    CONF_PLAYER,
    CONF_BALL
};

static int music_id[11];
static int sound_id[11];

static int conf_action(int tok, int val)
{
    int s = config_get_d(CONFIG_SOUND_VOLUME);
    int m = config_get_d(CONFIG_MUSIC_VOLUME);
    int r = 1;

    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case CONF_BACK:
        goto_state(&st_title);
        break;

    case CONF_VIDEO:
        goto_state(&st_conf_video);
        break;

    case CONF_PLAYER:
        goto_name(&st_conf, &st_conf, 1);
        break;

    case CONF_BALL:
        goto_state(&st_ball);
        break;

    case CONF_SOUND_VOLUME:
        config_set_d(CONFIG_SOUND_VOLUME, val);
        audio_volume(val, m);
        audio_play(AUD_BUMPM, 1.f);

        gui_toggle(sound_id[val]);
        gui_toggle(sound_id[s]);
        break;

    case CONF_MUSIC_VOLUME:
        config_set_d(CONFIG_MUSIC_VOLUME, val);
        audio_volume(s, val);
        audio_play(AUD_BUMPM, 1.f);

        gui_toggle(music_id[val]);
        gui_toggle(music_id[m]);

        break;
    }

    return r;
}

static int conf_gui(void)
{
    int id;

    /* Initialize the configuration GUI. */

    if ((id = gui_vstack(0)))
    {
        int s = config_get_d(CONFIG_SOUND_VOLUME);
        int m = config_get_d(CONFIG_MUSIC_VOLUME);

        const char *player = config_get_s(CONFIG_PLAYER);
        const char *ball   = config_get_s(CONFIG_BALL_FILE);

        int name_id = 0, ball_id = 0;

        conf_header(id, _("Options"), CONF_BACK);

        conf_state(id, _("Graphics"), _("Configure"), CONF_VIDEO);

        gui_space(id);

        conf_slider(id, _("Sound Volume"), CONF_SOUND_VOLUME, s,
                    sound_id, ARRAYSIZE(sound_id));
        conf_slider(id, _("Music Volume"), CONF_MUSIC_VOLUME, m,
                    music_id, ARRAYSIZE(music_id));

        gui_space(id);

        name_id = conf_state(id, _("Player Name"), " ", CONF_PLAYER);
        ball_id = conf_state(id, _("Ball Model"), " ", CONF_BALL);

        gui_layout(id, 0, 0);

        gui_set_trunc(name_id, TRUNC_TAIL);
        gui_set_trunc(ball_id, TRUNC_TAIL);

        gui_set_label(name_id, player);
        gui_set_label(ball_id, base_name(ball));
    }

    return id;
}

static int conf_enter(struct state *st, struct state *prev)
{
    conf_shared_init(conf_action);
    return conf_gui();
}

/*---------------------------------------------------------------------------*/

enum
{
    CONF_VIDEO_BACK = CONF_SHARED_BACK,
    CONF_VIDEO_FULL,
    CONF_VIDEO_RES,
    CONF_VIDEO_TEX,
    CONF_VIDEO_REF,
    CONF_VIDEO_BAC,
    CONF_VIDEO_SHD
};

static int conf_video_action(int tok, int val)
{
    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);
    int r = 1;

    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case CONF_VIDEO_FULL:
        goto_state(&st_null);
        r = video_mode(val, w, h);
        goto_state(&st_conf_video);
        break;

    case CONF_VIDEO_TEX:
        goto_state(&st_null);
        config_set_d(CONFIG_TEXTURES, val);
        goto_state(&st_conf_video);
        break;

    case CONF_VIDEO_REF:
        goto_state(&st_null);
        config_set_d(CONFIG_REFLECTION, val);

        if (val)
        {
            /* Ensure we have a stencil buffer. */
            r = video_init(TITLE, ICON);
        }

        goto_state(&st_conf_video);
        break;

    case CONF_VIDEO_BAC:
        goto_state(&st_null);
        config_set_d(CONFIG_BACKGROUND, val);
        goto_state(&st_conf_video);
        break;

    case CONF_VIDEO_SHD:
        goto_state(&st_null);
        config_set_d(CONFIG_SHADOW, val);
        goto_state(&st_conf_video);
        break;

    case CONF_VIDEO_BACK:
        goto_state(&st_conf);
        break;

    case CONF_VIDEO_RES:
        goto_state(&st_resol);
        break;
    }

    return r;
}

static int conf_video_gui(void)
{
    int id;

    if ((id = gui_vstack(0)))
    {
        int f = config_get_d(CONFIG_FULLSCREEN);
        int t = config_get_d(CONFIG_TEXTURES);
        int r = config_get_d(CONFIG_REFLECTION);
        int b = config_get_d(CONFIG_BACKGROUND);
        int s = config_get_d(CONFIG_SHADOW);

        char resolution[sizeof ("12345678 x 12345678")];

        sprintf(resolution, "%d x %d",
                config_get_d(CONFIG_WIDTH),
                config_get_d(CONFIG_HEIGHT));

        conf_header(id, _("Graphics Options"), CONF_VIDEO_BACK);

        conf_toggle(id, _("Fullscreen"), CONF_VIDEO_FULL, f,
                    _("Yes"), 1, _("No"), 0);

        conf_state(id, _("Resolution"), resolution, CONF_VIDEO_RES);

        gui_space(id);

        conf_toggle(id, _("Textures"), CONF_VIDEO_TEX, t,
                    _("High"), 1, _("Low"), 2);

        conf_toggle(id, _("Reflection"), CONF_VIDEO_REF, r,
                    _("On"), 1, _("Off"), 0);

        conf_toggle(id, _("Background"), CONF_VIDEO_BAC, b,
                    _("On"), 1, _("Off"), 0);

        conf_toggle(id, _("Shadow"), CONF_VIDEO_SHD, s,
                    _("On"), 1, _("Off"), 0);

        gui_layout(id, 0, 0);
    }

    return id;
}

static int conf_video_enter(struct state *st, struct state *prev)
{
    conf_shared_init(conf_video_action);
    return conf_video_gui();
}

/*---------------------------------------------------------------------------*/

static int null_enter(struct state *st, struct state *prev)
{
    hud_free();
    gui_free();
    geom_free();
    item_free();
    ball_free();
    shad_free();
    part_free();

    return 0;
}

static void null_leave(struct state *st, struct state *next, int id)
{
    part_init();
    shad_init();
    ball_init();
    item_init();
    geom_init();
    gui_init();
    hud_init();
}

/*---------------------------------------------------------------------------*/

struct state st_conf = {
    conf_enter,
    conf_shared_leave,
    conf_shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    NULL,
    conf_shared_buttn
};

struct state st_conf_video = {
    conf_video_enter,
    conf_shared_leave,
    conf_shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_angle,
    shared_click,
    NULL,
    conf_shared_buttn
};

struct state st_null = {
    null_enter,
    null_leave,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
