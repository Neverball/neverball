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
#include "transition.h"
#include "hud.h"
#include "geom.h"
#include "ball.h"
#include "part.h"
#include "audio.h"
#include "config.h"
#include "video.h"
#include "common.h"
#include "version.h"
#include "lang.h"

#include "game_common.h"
#include "game_client.h"
#include "game_server.h"

#include "st_conf.h"
#include "st_title.h"
#include "st_common.h"
#include "st_name.h"
#include "st_ball.h"
#include "st_shared.h"

extern const char TITLE[];
extern const char ICON[];

/*---------------------------------------------------------------------------*/

enum
{
    CONF_VIDEO = GUI_LAST,
    CONF_LANGUAGE,
    CONF_MOUSE_SENSE,
    CONF_JOYSTICK_RESPONSE,
    CONF_JOYSTICK,
    CONF_SOUND_VOLUME,
    CONF_MUSIC_VOLUME,
    CONF_PLAYER,
    CONF_BALL
};

static int mouse_id[11];
static int music_id[11];
static int sound_id[11];
static int joystick_id[11];

/*
 * This maps mouse_sense 300 (default) to the 7th of an 11 button
 * series. Effectively there are more options for a lower-than-default
 * sensitivity than for a higher one.
 */

#define MOUSE_RANGE_MIN  100
#define MOUSE_RANGE_INC  50
#define MOUSE_RANGE_MAX (MOUSE_RANGE_MIN + (MOUSE_RANGE_INC * 10))

/*
 * Map mouse_sense values to [0, 10]. A higher mouse_sense value means
 * lower sensitivity, thus counter-intuitively, 0 maps to the higher
 * value.
 */

#define MOUSE_RANGE_MAP(m) \
    CLAMP(0, (MOUSE_RANGE_MAX - m) / MOUSE_RANGE_INC, 10)

#define MOUSE_RANGE_UNMAP(i) \
    (MOUSE_RANGE_MAX - (i * MOUSE_RANGE_INC))

/*
 * Tilt responsiveness configuration: [50, 75 .. 300].
 */

#define JOYSTICK_RANGE_MIN  50
#define JOYSTICK_RANGE_INC  25
#define JOYSTICK_RANGE_MAX (JOYSTICK_RANGE_MIN + (JOYSTICK_RANGE_INC * 10))

#define JOYSTICK_RANGE_MAP(m) \
    CLAMP(0, (JOYSTICK_RANGE_MAX - m) / JOYSTICK_RANGE_INC, 10)

#define JOYSTICK_RANGE_UNMAP(i) \
    (JOYSTICK_RANGE_MAX - (i * JOYSTICK_RANGE_INC))

static int conf_action(int tok, int val)
{
    int sound = config_get_d(CONFIG_SOUND_VOLUME);
    int music = config_get_d(CONFIG_MUSIC_VOLUME);
    int mouse = MOUSE_RANGE_MAP(config_get_d(CONFIG_MOUSE_SENSE));
    int joystick = JOYSTICK_RANGE_MAP(config_get_d(CONFIG_JOYSTICK_RESPONSE));
    int r = 1;

    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case GUI_BACK:
        exit_state(&st_title);
        break;

    case CONF_VIDEO:
        goto_state(&st_video);
        break;

    case CONF_JOYSTICK:
        goto_state(&st_joystick);
        break;

    case CONF_LANGUAGE:
        goto_state(&st_lang);
        break;

    case CONF_PLAYER:
        goto_name(&st_conf, &st_conf, 1);
        break;

    case CONF_BALL:
        goto_state(&st_ball);
        break;

    case CONF_MOUSE_SENSE:
        config_set_d(CONFIG_MOUSE_SENSE, MOUSE_RANGE_UNMAP(val));

        gui_toggle(mouse_id[val]);
        gui_toggle(mouse_id[mouse]);
        break;

    case CONF_JOYSTICK_RESPONSE:
        config_set_d(CONFIG_JOYSTICK_RESPONSE, JOYSTICK_RANGE_UNMAP(val));

        gui_toggle(joystick_id[val]);
        gui_toggle(joystick_id[joystick]);
        break;

    case CONF_SOUND_VOLUME:
        config_set_d(CONFIG_SOUND_VOLUME, val);
        audio_volume(val, music);
        audio_play(AUD_BUMPM, 1.f);

        gui_toggle(sound_id[val]);
        gui_toggle(sound_id[sound]);
        break;

    case CONF_MUSIC_VOLUME:
        config_set_d(CONFIG_MUSIC_VOLUME, val);
        audio_volume(sound, val);
        audio_play(AUD_BUMPM, 1.f);

        gui_toggle(music_id[val]);
        gui_toggle(music_id[music]);

        break;
    }

    return r;
}

static int conf_gui(void)
{
    int root_id;

    /* Initialize the configuration GUI. */

    if ((root_id = gui_root()))
    {
        int id;

        if ((id = gui_vstack(root_id)))
        {
            int sound = config_get_d(CONFIG_SOUND_VOLUME);
            int music = config_get_d(CONFIG_MUSIC_VOLUME);
            int mouse = MOUSE_RANGE_MAP(config_get_d(CONFIG_MOUSE_SENSE));
            int joystick = JOYSTICK_RANGE_MAP(config_get_d(CONFIG_JOYSTICK_RESPONSE));

            const char *player = config_get_s(CONFIG_PLAYER);
            const char *ball   = config_get_s(CONFIG_BALL_FILE);

            int name_id, ball_id, lang_id;

            conf_header(id, _("Options"), GUI_BACK);

            conf_state(id, _("Graphics"), _("Configure"), CONF_VIDEO);

            gui_space(id);

            conf_slider(id, _("Mouse Sensitivity"), CONF_MOUSE_SENSE, mouse,
                        mouse_id, ARRAYSIZE(mouse_id));

            conf_slider(id, _("Tilt Responsiveness"), CONF_JOYSTICK_RESPONSE, joystick,
                        joystick_id, ARRAYSIZE(joystick_id));

            gui_space(id);

            conf_state(id, _("Gamepad"), _("Configure"), CONF_JOYSTICK);

            gui_space(id);

            conf_slider(id, _("Sound Volume"), CONF_SOUND_VOLUME, sound,
                        sound_id, ARRAYSIZE(sound_id));
            conf_slider(id, _("Music Volume"), CONF_MUSIC_VOLUME, music,
                        music_id, ARRAYSIZE(music_id));

            gui_space(id);

            name_id = conf_state(id, _("Player Name"), " ", CONF_PLAYER);
            ball_id = conf_state(id, _("Ball Model"), " ", CONF_BALL);
            lang_id = conf_state(id, _("Language"), " ", CONF_LANGUAGE);

            gui_layout(id, 0, 0);

            gui_set_trunc(lang_id, TRUNC_TAIL);
            gui_set_trunc(name_id, TRUNC_TAIL);
            gui_set_trunc(ball_id, TRUNC_TAIL);

            gui_set_label(name_id, player);
            gui_set_label(ball_id, base_name(ball));

            if (*config_get_s(CONFIG_LANGUAGE))
                gui_set_label(lang_id, lang_name(&curr_lang));
            else
                gui_set_label(lang_id, _("Default"));
        }

        if ((id = gui_vstack(root_id)))
        {
            gui_label(id, "Neverball " VERSION, GUI_TNY, gui_wht, gui_wht);
            gui_multi(id, _(
                "Copyright © 2025 Neverball authors\n"
                "Neverball is free software available under the terms of GPL v2 or later."
            ), GUI_TNY, gui_wht, gui_wht);

            gui_clr_rect(id);
            gui_layout(id, 0, -1);
        }
    }

    return root_id;
}

static int conf_enter(struct state *st, struct state *prev, int intent)
{
    game_client_free(NULL);
    conf_common_init(conf_action);
    return transition_slide(conf_gui(), 1, intent);
}

static int conf_leave(struct state *st, struct state *next, int id, int intent)
{
    return conf_common_leave(st, next, id, intent);
}
/*---------------------------------------------------------------------------*/

static int null_enter(struct state *st, struct state *prev, int intent)
{
    hud_free();
    transition_quit();
    gui_free();
    geom_free();
    ball_free();
    shad_free();
    part_free();
    mtrl_free_objects();

    return 0;
}

static int null_leave(struct state *st, struct state *next, int id, int intent)
{
    mtrl_load_objects();
    part_init();
    shad_init();
    ball_init();
    geom_init();
    gui_init();
    transition_init();
    hud_init();
    return 0;
}

/*---------------------------------------------------------------------------*/

 struct state st_conf = {
    conf_enter,
    conf_leave,
    conf_common_paint,
    common_timer,
    common_point,
    common_stick,
    NULL,
    common_click,
    common_keybd,
    common_buttn
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
