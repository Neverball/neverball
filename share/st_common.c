/*
 * Copyright (C) 2013 Neverball authors
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


#include "config.h"
#include "audio.h"
#include "video.h"
#include "geom.h"
#include "lang.h"
#include "gui.h"
#include "transition.h"

#include "st_common.h"

#define AUD_MENU "snd/menu.ogg"

/*---------------------------------------------------------------------------*/

/*
 * Conf screen GUI helpers.
 */

void conf_slider(int id, const char *text,
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

        gui_label(jd, text, GUI_SML, 0, 0);
    }
}

int conf_state(int id, const char *label, const char *text, int token)
{
    int jd, kd, rd = 0;

    if ((jd = gui_harray(id)) && (kd = gui_harray(jd)))
    {
        rd = gui_state(kd, text, GUI_SML, token, 0);
        gui_label(jd, label, GUI_SML, 0, 0);
    }

    return rd;
}

void conf_toggle(int id, const char *label, int token, int value,
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

        gui_label(jd, label, GUI_SML, 0, 0);
    }
}

void conf_header(int id, const char *text, int token)
{
    int jd;

    if ((jd = gui_harray(id)))
    {
        gui_label(jd, text, GUI_SML, 0, 0);
        gui_space(jd);
        gui_start(jd, _("Back"), GUI_SML, token, 0);
    }

    gui_space(id);
}

void conf_select(int id, const char *text, int token, int value,
                 const struct conf_option *opts, int num)
{
    int jd, kd, ld;
    int i;

    if ((jd = gui_harray(id)) && (kd = gui_harray(jd)))
    {
        for (i = 0; i < num; i++)
        {
            ld = gui_state(kd, _(opts[i].text), GUI_SML,
                           token, opts[i].value);

            gui_set_hilite(ld, (opts[i].value == value));
        }

        gui_label(jd, text, GUI_SML, 0, 0);
    }
}

/*---------------------------------------------------------------------------*/

/*
 * Code shared by most screens (not just conf screens).
 *
 * FIXME This probably makes ball/st_shared.c obsolete.
 */

static int (*common_action)(int tok, int val);

void common_init(int (*action_fn)(int, int))
{
    common_action = action_fn;
}

int common_leave(struct state *st, struct state *next, int id, int intent)
{
    return transition_slide(id, 0, intent);
}

void common_paint(int id, float st)
{
    gui_paint(id);
}

void common_timer(int id, float dt)
{
    gui_timer(id, dt);
}

void common_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

void common_stick(int id, int a, float v, int bump)
{
    gui_pulse(gui_stick(id, a, v, bump), 1.2f);
}

int common_click(int b, int d)
{
    if (gui_click(b, d))
    {
        int active = gui_active();
        return common_action(gui_token(active), gui_value(active));
    }
    return 1;
}

int common_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT)
            return common_action(GUI_BACK, 0);

        if (c == SDLK_LEFTBRACKET)
            return common_action(GUI_PREV, 0);

        if (c == SDLK_RIGHTBRACKET)
            return common_action(GUI_NEXT, 0);
    }

    return 1;
}

int common_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return common_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return common_action(GUI_BACK, 0);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_L1, b))
            return common_action(GUI_PREV, 0);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_R1, b))
            return common_action(GUI_NEXT, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

/*
 * Code shared by conf screens.
 */

void conf_common_init(int (*action_fn)(int, int))
{
    back_init("back/gui.png");
    audio_music_fade_to(0.5f, "bgm/inter.ogg");

    common_init(action_fn);
}

int conf_common_leave(struct state *st, struct state *next, int id, int intent)
{
    config_save();

    back_free();

    return transition_slide(id, 0, intent);
}

void conf_common_paint(int id, float t)
{
    video_push_persp((float) config_get_d(CONFIG_VIEW_FOV), 0.1f, FAR_DIST);
    {
        back_draw_easy();
    }
    video_pop_matrix();

    gui_paint(id);
}

/*---------------------------------------------------------------------------*/

enum
{
    VIDEO_FULLSCREEN = GUI_LAST,
    VIDEO_DISPLAY,
    VIDEO_RESOLUTION,
    VIDEO_REFLECTION,
    VIDEO_BACKGROUND,
    VIDEO_SHADOW,
    VIDEO_VSYNC,
    VIDEO_HMD,
    VIDEO_MULTISAMPLE
};

static struct state *video_back;

static int video_action(int tok, int val)
{
    int f = config_get_d(CONFIG_FULLSCREEN);
    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);
    int r = 1;

    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case GUI_BACK:
        exit_state(video_back);
        video_back = NULL;
        break;

    case VIDEO_FULLSCREEN:
        goto_state(&st_null);
        r = video_mode(val, w, h);
        goto_state(&st_video);
        break;

    case VIDEO_DISPLAY:
        goto_state(&st_display);
        break;

    case VIDEO_REFLECTION:
        goto_state(&st_null);
        config_set_d(CONFIG_REFLECTION, val);
        r = video_mode(f, w, h);
        goto_state(&st_video);
        break;

    case VIDEO_BACKGROUND:
        goto_state(&st_null);
        config_set_d(CONFIG_BACKGROUND, val);
        goto_state(&st_video);
        break;

    case VIDEO_SHADOW:
        goto_state(&st_null);
        config_set_d(CONFIG_SHADOW, val);
        goto_state(&st_video);
        break;

    case VIDEO_RESOLUTION:
        goto_state(&st_resol);
        break;

    case VIDEO_VSYNC:
        goto_state(&st_null);
        config_set_d(CONFIG_VSYNC, val);
        r = video_mode(f, w, h);
        goto_state(&st_video);
        break;

    case VIDEO_HMD:
        goto_state(&st_null);
        config_set_d(CONFIG_HMD, val);
        r = video_mode(f, w, h);
        goto_state(&st_video);
        break;

    case VIDEO_MULTISAMPLE:
        goto_state(&st_null);
        config_set_d(CONFIG_MULTISAMPLE, val);
        r = video_mode(f, w, h);
        goto_state(&st_video);
        break;
    }

    return r;
}

static int video_gui(void)
{
    static const struct conf_option multisample_opts[] = {
        { N_("Off"), 0 },
        { N_("2x"), 2 },
        { N_("4x"), 4 },
        { N_("8x"), 8 },
    };

    int id, jd;

    if ((id = gui_vstack(0)))
    {
        char resolution[sizeof ("12345678 x 12345678")];
        const char *display;
        int dpy = config_get_d(CONFIG_DISPLAY);

        sprintf(resolution, "%d x %d", video.window_w, video.window_h);

        if (!(display = SDL_GetDisplayName(dpy)))
            display = _("Unknown Display");

        conf_header(id, _("Graphics"), GUI_BACK);

        if ((jd = conf_state(id, _("Display"), "Longest Name", VIDEO_DISPLAY)))
        {
            gui_set_trunc(jd, TRUNC_TAIL);
            gui_set_label(jd, display);
        }

        conf_toggle(id, _("Fullscreen"),   VIDEO_FULLSCREEN,
                    config_get_d(CONFIG_FULLSCREEN), _("On"), 1, _("Off"), 0);

        if ((jd = conf_state (id, _("Resolution"), resolution,
                              VIDEO_RESOLUTION)))
        {
            /*
             * Because we always use the desktop display mode, disable
             * display mode switching in fullscreen.
             */

            if (config_get_d(CONFIG_FULLSCREEN))
            {
                gui_set_state(jd, GUI_NONE, 0);
                gui_set_color(jd, gui_gry, gui_gry);
            }
        }
#if ENABLE_HMD
        conf_toggle(id, _("HMD"),          VIDEO_HMD,
                    config_get_d(CONFIG_HMD),        _("On"), 1, _("Off"), 0);
#endif

        gui_space(id);

        conf_toggle(id, _("V-Sync"),       VIDEO_VSYNC,
                    config_get_d(CONFIG_VSYNC),      _("On"), 1, _("Off"), 0);
        conf_select(id, _("Antialiasing"), VIDEO_MULTISAMPLE,
                    config_get_d(CONFIG_MULTISAMPLE),
                    multisample_opts, ARRAYSIZE(multisample_opts));

        gui_space(id);

        conf_toggle(id, _("Reflection"),   VIDEO_REFLECTION,
                    config_get_d(CONFIG_REFLECTION), _("On"), 1, _("Off"), 0);
        conf_toggle(id, _("Background"),   VIDEO_BACKGROUND,
                    config_get_d(CONFIG_BACKGROUND), _("On"), 1, _("Off"), 0);
        conf_toggle(id, _("Shadow"),       VIDEO_SHADOW,
                    config_get_d(CONFIG_SHADOW),     _("On"), 1, _("Off"), 0);

        gui_layout(id, 0, 0);
    }

    return id;
}

static int video_enter(struct state *st, struct state *prev, int intent)
{
    if (!video_back)
        video_back = prev;

    conf_common_init(video_action);
    return transition_slide(video_gui(), 1, intent);
}

/*---------------------------------------------------------------------------*/

enum
{
    DISPLAY_SELECT = GUI_LAST
};

static struct state *display_back;

static int display_action(int tok, int val)
{
    int r = 1;

    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case GUI_BACK:
        exit_state(display_back);
        display_back = NULL;
        break;

    case DISPLAY_SELECT:
        if (val != config_get_d(CONFIG_DISPLAY))
        {
            goto_state(&st_null);
            config_set_d(CONFIG_DISPLAY, val);
            r = video_mode(config_get_d(CONFIG_FULLSCREEN),
                           config_get_d(CONFIG_WIDTH),
                           config_get_d(CONFIG_HEIGHT));
            goto_state(&st_display);
        }
        break;
    }

    return r;
}

static int display_gui(void)
{
    int id, jd;

    int i, n = SDL_GetNumVideoDisplays();

    if ((id = gui_vstack(0)))
    {
        conf_header(id, _("Display"), GUI_BACK);

        for (i = 0; i < n; i++)
        {
            const char *name = SDL_GetDisplayName(i);

            jd = gui_state(id, name, GUI_SML, DISPLAY_SELECT, i);
            gui_set_hilite(jd, (i == config_get_d(CONFIG_DISPLAY)));
        }

        gui_layout(id, 0, 0);
    }

    return id;
}

static int display_enter(struct state *st, struct state *prev, int intent)
{
    if (!display_back)
        display_back = prev;

    conf_common_init(display_action);
    return transition_slide(display_gui(), 1, intent);
}

/*---------------------------------------------------------------------------*/

struct mode
{
    int w;
    int h;
};

static const struct mode modes[] = {
    { 2560, 1440 },
    { 1920, 1200 },
    { 1920, 1080 },
    { 1680, 1050 },
    { 1600, 1200 },
    { 1600, 900 },
    { 1440, 900 },
    { 1366, 768 },
    { 1280, 1024 },
    { 1280, 800 },
    { 1280, 720 },
    { 1024, 768 },
    { 800, 600 },
    { 640, 480 },
    { 480, 320 },
    { 320, 240 }
};

enum
{
    RESOL_MODE = GUI_LAST
};

static struct state *resol_back;

static int resol_action(int tok, int val)
{
    int r = 1;

    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case GUI_BACK:
        exit_state(resol_back);
        resol_back = NULL;
        break;

    case RESOL_MODE:
        goto_state(&st_null);
        r = video_mode(config_get_d(CONFIG_FULLSCREEN),
                       modes[val].w,
                       modes[val].h);
        goto_state(&st_resol);
        break;
    }

    return r;
}

static int resol_gui(void)
{
    int id, jd, kd;

    if ((id = gui_vstack(0)))
    {
        const int W = config_get_d(CONFIG_WIDTH);
        const int H = config_get_d(CONFIG_HEIGHT);

        int i, j, n = ARRAYSIZE(modes);

        char buff[sizeof ("1234567890 x 1234567890")] = "";

        conf_header(id, _("Resolution"), GUI_BACK);

        for (i = 0; i < n; i += 4)
        {
            if ((jd = gui_harray(id)))
            {
                for (j = 3; j >= 0; j--)
                {
                    int m = i + j;

                    if (m < n)
                    {
                        sprintf(buff, "%d x %d", modes[m].w, modes[m].h);
                        kd = gui_state(jd, buff, GUI_SML, RESOL_MODE, m);
                        gui_set_hilite(kd, (modes[m].w == W &&
                                            modes[m].h == H));
                    }
                    else
                    {
                        gui_space(jd);
                    }
                }
            }
        }

        gui_layout(id, 0, 0);
    }

    return id;
}

static int resol_enter(struct state *st, struct state *prev, int intent)
{
    if (!resol_back)
        resol_back = prev;

    conf_common_init(resol_action);
    return transition_slide(resol_gui(), 1, intent);
}

/*---------------------------------------------------------------------------*/

#define LANG_STEP 10

static Array langs;
static int   first;

enum
{
    LANG_DEFAULT = GUI_LAST,
    LANG_SELECT
};

static struct state *lang_back;

static int lang_action(int tok, int val)
{
    int r = 1;

    struct lang_desc *desc;

    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
    case GUI_BACK:
        exit_state(lang_back);
        lang_back = NULL;
        break;

    case GUI_PREV:
        first -= LANG_STEP;
        exit_state(&st_lang);
        break;

    case GUI_NEXT:
        first += LANG_STEP;
        goto_state(&st_lang);
        break;

    case LANG_DEFAULT:
        /* HACK: Reload resources to load the localized font. */
        goto_state(&st_null);
        config_set_s(CONFIG_LANGUAGE, "");
        lang_init();
        goto_state(&st_lang);
        break;

    case LANG_SELECT:
        desc = LANG_GET(langs, val);
        goto_state(&st_null);
        config_set_s(CONFIG_LANGUAGE, desc->code);
        lang_init();
        goto_state(&st_lang);
        break;
    }

    return r;
}

static int lang_gui(void)
{
    const int step = (first == 0 ? LANG_STEP - 1 : LANG_STEP);

    int id, jd;
    int i;

    if ((id = gui_vstack(0)))
    {
        if ((jd = gui_hstack(id)))
        {
            gui_label(jd, _("Language"), GUI_SML, 0, 0);
            gui_space(jd);
            gui_space(jd);
            gui_navig(jd, array_len(langs), first, LANG_STEP);
        }

        gui_space(id);

        if (step < LANG_STEP)
        {
            int default_id;
            default_id = gui_state(id, _("Default"), GUI_SML, LANG_DEFAULT, 0);
            gui_set_hilite(default_id, !*config_get_s(CONFIG_LANGUAGE));
        }

        for (i = first; i < first + step; i++)
        {
            if (i < array_len(langs))
            {
                struct lang_desc *desc = LANG_GET(langs, i);

                int lang_id;

                lang_id = gui_state(id, " ", GUI_SML, LANG_SELECT, i);

                gui_set_hilite(lang_id, (strcmp(config_get_s(CONFIG_LANGUAGE),
                                                desc->code) == 0));

                /* Set font and rebuild texture. */

                gui_set_font(lang_id, desc->font);
                gui_set_label(lang_id, lang_name(desc));
            }
            else
            {
                gui_label(id, " ", GUI_SML, 0, 0);
            }
        }

        gui_layout(id, 0, 0);
    }

    return id;
}

static int lang_enter(struct state *st, struct state *prev, int intent)
{
    if (!langs)
    {
        langs = lang_dir_scan();
        first = 0;
    }

    if (!lang_back)
        lang_back = prev;

    conf_common_init(lang_action);
    return transition_slide(lang_gui(), 1, intent);
}

int lang_leave(struct state *st, struct state *next, int id, int intent)
{
    if (!(next == &st_lang || next == &st_null))
    {
        lang_dir_free(langs);
        langs = NULL;
    }

    return conf_common_leave(st, next, id, intent);
}

static int lang_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return lang_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return lang_action(GUI_BACK, 0);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_L1, b) && first > 0)
            return lang_action(GUI_PREV, 0);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_R1, b) && first + LANG_STEP < array_len(langs))
            return lang_action(GUI_NEXT, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

enum
{
    JOYSTICK_ASSIGN_BUTTON = GUI_LAST,
    JOYSTICK_ASSIGN_AXIS
};

static struct state *joystick_back;

static int joystick_modal_button_id;
static int joystick_modal_axis_id;

static int joystick_modal;

static int joystick_option_index;

static int *joystick_options[] = {
    &CONFIG_JOYSTICK_BUTTON_A,
    &CONFIG_JOYSTICK_BUTTON_B,
    &CONFIG_JOYSTICK_BUTTON_X,
    &CONFIG_JOYSTICK_BUTTON_Y,

    &CONFIG_JOYSTICK_BUTTON_L1,
    &CONFIG_JOYSTICK_BUTTON_R1,

    &CONFIG_JOYSTICK_BUTTON_SELECT,
    &CONFIG_JOYSTICK_BUTTON_START,

    NULL, /* Marker to separate buttons from axes. */

    &CONFIG_JOYSTICK_AXIS_X0,
    &CONFIG_JOYSTICK_AXIS_Y0,
    &CONFIG_JOYSTICK_AXIS_X1,
    &CONFIG_JOYSTICK_AXIS_Y1,
};

static const char *joystick_option_names[] = {
    N_("Button A"),
    N_("Button B"),
    N_("Button X"),
    N_("Button Y"),
    N_("Button L1"),
    N_("Button R1"),
    N_("Select"),
    N_("Start"),

    "",

    N_("X Axis 1"),
    N_("Y Axis 1"),
    N_("X Axis 2"),
    N_("Y Axis 2"),
};

static int joystick_option_ids[ARRAYSIZE(joystick_options)];

static void joystick_set_label(int id, int value)
{
    char str[8];

    sprintf(str, "%d", value % 100000);

    gui_set_label(id, str);
}

static void joystick_set_option(int index, int value)
{
    if (index < ARRAYSIZE(joystick_options))
    {
        int option = *joystick_options[index];

        config_set_d(option, value);

        joystick_set_label(joystick_option_ids[index], value);

        // Focus the next button.

        if (index < ARRAYSIZE(joystick_options) - 1)
        {
            /* Skip over marker, if any. */

            if (index < ARRAYSIZE(joystick_options) - 2 && joystick_options[index + 1] == NULL)
                gui_focus(joystick_option_ids[index + 2]);
            else
                gui_focus(joystick_option_ids[index + 1]);
        }
    }
}

static int joystick_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);

    switch (tok)
    {
        case GUI_BACK:
            if (joystick_modal)
            {
                joystick_modal = 0;
            }
            else
            {
                exit_state(joystick_back);
                joystick_back = NULL;
            }
            break;

        case JOYSTICK_ASSIGN_BUTTON:
        case JOYSTICK_ASSIGN_AXIS:
            joystick_modal = tok;
            joystick_option_index = val;
            break;
    }
    return 1;
}

static int joystick_gui(void)
{
    int id;

    if ((id = gui_vstack(0)))
    {
        int token = JOYSTICK_ASSIGN_BUTTON;
        int i;

        conf_header(id, _("Gamepad"), GUI_BACK);

        for (i = 0; i < ARRAYSIZE(joystick_options); ++i)
        {
            int btn_id;
            int value;

            /* Check for marker. */

            if (joystick_options[i] == NULL)
            {
                /* Switch the GUI token / assignment type. */
                token = JOYSTICK_ASSIGN_AXIS;

                gui_space(id);

                continue;
            }

            value = config_get_d(*joystick_options[i]);

            if ((btn_id = conf_state(id, _(joystick_option_names[i]), "99", 0)))
            {
                joystick_option_ids[i] = btn_id;

                gui_set_state(btn_id, token, i);

                joystick_set_label(btn_id, value);
            }
        }

        gui_layout(id, 0, 0);
    }

    return id;
}

static int joystick_modal_button_gui(void)
{
    int id;

    if ((id = gui_label(0, _("Press a button..."), GUI_MED, gui_wht, gui_wht)))
        gui_layout(id, 0, 0);

    return id;
}

static int joystick_modal_axis_gui(void)
{
    int id;

    if ((id = gui_label(0, _("Move a stick..."), GUI_MED, gui_wht, gui_wht)))
        gui_layout(id, 0, 0);

    return id;
}

static int joystick_enter(struct state *st, struct state *prev, int intent)
{
    if (!joystick_back)
        joystick_back = prev;

    conf_common_init(joystick_action);

    joystick_modal = 0;

    joystick_modal_button_id = joystick_modal_button_gui();
    joystick_modal_axis_id = joystick_modal_axis_gui();

    return transition_slide(joystick_gui(), 1, intent);
}

static int joystick_leave(struct state *st, struct state *next, int id, int intent)
{
    gui_delete(joystick_modal_button_id);
    gui_delete(joystick_modal_axis_id);

    return conf_common_leave(st, next, id, intent);
}

static void joystick_paint(int id, float t)
{
    conf_common_paint(id, t);

    if (joystick_modal == JOYSTICK_ASSIGN_BUTTON)
        gui_paint(joystick_modal_button_id);

    if (joystick_modal == JOYSTICK_ASSIGN_AXIS)
        gui_paint(joystick_modal_axis_id);
}

static int joystick_buttn(int b, int d)
{
    if (d)
    {
        if (joystick_modal == JOYSTICK_ASSIGN_BUTTON)
        {
            joystick_set_option(joystick_option_index, b);
            joystick_modal = 0;
            return 1;
        }
        else if (joystick_modal)
        {
            /* Allow backing out of other modal types with B. */

            if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
                joystick_modal = 0;

            return 1;
        }
    }

    return common_buttn(b, d);
}

static void joystick_stick(int id, int a, float v, int bump)
{
    if (joystick_modal == JOYSTICK_ASSIGN_AXIS)
    {
        if (bump)
        {
            joystick_set_option(joystick_option_index, a);
            joystick_modal = 0;
        }

        return;
    }
    else if (joystick_modal)
    {
        /* Ignore stick motion if another type of modal is active. */
        return;
    }

    gui_pulse(gui_stick(id, a, v, bump), 1.2f);
}

/*---------------------------------------------------------------------------*/

static int loading_gui(void)
{
    int id;

    if ((id = gui_vstack(0)))
    {
        gui_label(id, _("Loading..."), GUI_SML, gui_wht, gui_wht);
        gui_layout(id, 0, 0);
    }

    return id;
}

static int loading_enter(struct state *st, struct state *prev, int intent)
{
    return loading_gui();
}

static int loading_leave(struct state *st, struct state *next, int id, int intent)
{
    gui_delete(id);
    return 0;
}

static void loading_paint(int id, float t)
{
    gui_paint(id);
}

/*---------------------------------------------------------------------------*/

struct state st_video = {
    video_enter,
    conf_common_leave,
    conf_common_paint,
    common_timer,
    common_point,
    common_stick,
    NULL,
    common_click,
    common_keybd,
    common_buttn
};

struct state st_display = {
    display_enter,
    conf_common_leave,
    conf_common_paint,
    common_timer,
    common_point,
    common_stick,
    NULL,
    common_click,
    common_keybd,
    common_buttn
};

struct state st_resol = {
    resol_enter,
    conf_common_leave,
    conf_common_paint,
    common_timer,
    common_point,
    common_stick,
    NULL,
    common_click,
    common_keybd,
    common_buttn
};

struct state st_lang = {
    lang_enter,
    lang_leave,
    conf_common_paint,
    common_timer,
    common_point,
    common_stick,
    NULL,
    common_click,
    common_keybd,
    lang_buttn
};

struct state st_joystick = {
    joystick_enter,
    joystick_leave,
    joystick_paint,
    common_timer,
    common_point,
    joystick_stick,
    NULL,
    common_click,
    common_keybd,
    joystick_buttn
};

struct state st_loading = {
    loading_enter,
    loading_leave,
    loading_paint
};

/*---------------------------------------------------------------------------*/
