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

/*---------------------------------------------------------------------------*/

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "version.h"
#include "glext.h"
#include "config.h"
#include "video.h"
#include "image.h"
#include "audio.h"
#include "demo.h"
#include "progress.h"
#include "gui.h"
#include "set.h"
#include "tilt.h"
#include "hmd.h"
#include "fs.h"
#include "common.h"
#include "text.h"
#include "mtrl.h"
#include "geom.h"
#include "joy.h"
#include "fetch.h"
#include "package.h"
#include "log.h"
#include "game_client.h"
#include "game_common.h"
#include "strbuf/substr.h"
#include "strbuf/joinstr.h"
#include "lang.h"

#include "st_conf.h"
#include "st_title.h"
#include "st_demo.h"
#include "st_level.h"
#include "st_pause.h"
#include "st_common.h"
#include "st_start.h"
#include "st_package.h"

const char TITLE[] = "Neverball";
const char ICON[] = "icon/neverball.png";

/*---------------------------------------------------------------------------*/

static char *opt_data;
static char *opt_replay;
static char *opt_level;
static char *opt_link;

#define opt_usage                                                     \
    "Usage: %s [options ...]\n"                                       \
    "Options:\n"                                                      \
    "  -h, --help                show this usage message.\n"          \
    "  -v, --version             show version.\n"                     \
    "  -d, --data <dir>          use 'dir' as game data directory.\n" \
    "  -r, --replay <file>       play the replay 'file'.\n"           \
    "  -l, --level <file>        load the level 'file'\n"             \
    "      --link <asset>        open the named asset\n"

#define opt_error(option) \
    fprintf(stderr, "Option '%s' requires an argument.\n", option)

static void opt_init(int argc, char **argv)
{
    int i;

    /* Scan argument list. */

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help")    == 0)
        {
            printf(opt_usage, argv[0]);
            exit(EXIT_SUCCESS);
        }

        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
        {
            printf("%s\n", VERSION);
            exit(EXIT_SUCCESS);
        }

        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--data")    == 0)
        {
            if (i + 1 == argc)
            {
                opt_error(argv[i]);
                exit(EXIT_FAILURE);
            }
            opt_data = argv[++i];
            continue;
        }

        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--replay")  == 0)
        {
            if (i + 1 == argc)
            {
                opt_error(argv[i]);
                exit(EXIT_FAILURE);
            }
            opt_replay = argv[++i];
            continue;
        }

        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--level")  == 0)
        {
            if (i + 1 == argc)
            {
                opt_error(argv[i]);
                exit(EXIT_FAILURE);
            }
            opt_level = argv[++i];
            continue;
        }

        if (strcmp(argv[i], "--link") == 0)
        {
            if (i + 1 == argc)
            {
                opt_error(argv[i]);
                exit(EXIT_FAILURE);
            }
            opt_link = argv[++i];
            continue;
        }

        /* Perform magic on a single unrecognized argument. */

        if (argc == 2)
        {
            size_t len = strlen(argv[i]);
            int level = 0;

            if (len > 4)
            {
                char *ext = argv[i] + len - 4;

                if (strcmp(ext, ".map") == 0)
                    strncpy(ext, ".sol", 4);

                if (strcmp(ext, ".sol") == 0)
                    level = 1;
            }

            if (level)
                opt_level = argv[i];
            else
                opt_replay = argv[i];

            break;
        }
    }
}

#undef opt_usage
#undef opt_error

static void opt_quit(void)
{
    opt_data = NULL;
    opt_replay = NULL;
    opt_level = NULL;
    opt_link = NULL;
}

/*---------------------------------------------------------------------------*/

static void shot(void)
{
    static char filename[MAXSTR];
    sprintf(filename, "Screenshots/screen%05d.png", config_screenshot());
    video_snap(filename);
}

/*---------------------------------------------------------------------------*/

static void toggle_wire(void)
{
    glToggleWireframe_();
}

/*---------------------------------------------------------------------------*/

/*
 * Track held direction keys.
 */
static char key_pressed[4];

static const int key_other[4] = { 1, 0, 3, 2 };

static const int *key_axis[4] = {
    &CONFIG_JOYSTICK_AXIS_Y0,
    &CONFIG_JOYSTICK_AXIS_Y0,
    &CONFIG_JOYSTICK_AXIS_X0,
    &CONFIG_JOYSTICK_AXIS_X0
};

static const float key_tilt[4] = { -1.0f, +1.0f, -1.0f, +1.0f };

static int handle_key_dn(SDL_Event *e)
{
    int d = 1;
    int c = e->key.keysym.sym;

    int dir = -1;

    /* SDL made me do it. */
#ifdef __APPLE__
    if (c == SDLK_q && e->key.keysym.mod & KMOD_GUI)
        return 0;
#endif
#ifdef _WIN32
    if (c == SDLK_F4 && e->key.keysym.mod & KMOD_ALT)
        return 0;
#endif

    switch (c)
    {
    case KEY_SCREENSHOT:
        shot();
        break;
    case KEY_FPS:
        config_tgl_d(CONFIG_FPS);
        break;
    case KEY_WIREFRAME:
        if (config_cheat())
            toggle_wire();
        break;
    case KEY_RESOURCES:
        if (config_cheat())
        {
            light_load();
            mtrl_reload();
        }
        break;
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 1);
        break;
    case KEY_FULLSCREEN:
        video_fullscreen(!config_get_d(CONFIG_FULLSCREEN));
        break;
    case KEY_EXIT:
        d = st_keybd(KEY_EXIT, 1);
        break;

    default:
        if (config_tst_d(CONFIG_KEY_FORWARD,  c))
            dir = 0;
        else if (config_tst_d(CONFIG_KEY_BACKWARD, c))
            dir = 1;
        else if (config_tst_d(CONFIG_KEY_LEFT, c))
            dir = 2;
        else if (config_tst_d(CONFIG_KEY_RIGHT, c))
            dir = 3;

        if (dir != -1)
        {
            /* Ignore auto-repeat on direction keys. */

            if (e->key.repeat)
                break;

            key_pressed[dir] = 1;
            st_stick(config_get_d(*key_axis[dir]), key_tilt[dir]);
        }
        else
            d = st_keybd(e->key.keysym.sym, 1);
    }

    return d;
}

static int handle_key_up(SDL_Event *e)
{
    int d = 1;
    int c = e->key.keysym.sym;

    int dir = -1;

    switch (c)
    {
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        d = st_buttn(config_get_d(CONFIG_JOYSTICK_BUTTON_A), 0);
        break;
    case KEY_EXIT:
        d = st_keybd(KEY_EXIT, 0);
        break;
    default:
        if (config_tst_d(CONFIG_KEY_FORWARD, c))
            dir = 0;
        else if (config_tst_d(CONFIG_KEY_BACKWARD, c))
            dir = 1;
        else if (config_tst_d(CONFIG_KEY_LEFT, c))
            dir = 2;
        else if (config_tst_d(CONFIG_KEY_RIGHT, c))
            dir = 3;

        if (dir != -1)
        {
            key_pressed[dir] = 0;

            if (key_pressed[key_other[dir]])
                st_stick(config_get_d(*key_axis[dir]), -key_tilt[dir]);
            else
                st_stick(config_get_d(*key_axis[dir]), 0.0f);
        }
        else
            d = st_keybd(e->key.keysym.sym, 0);
    }

    return d;
}

#ifdef __EMSCRIPTEN__

enum {
    USER_EVENT_BACK = -1,
    USER_EVENT_PAUSE = 0
};

void push_user_event(int code)
{
    SDL_Event event = { SDL_USEREVENT };
    event.user.code = code;
    SDL_PushEvent(&event);
}
#endif

/*---------------------------------------------------------------------------*/

static int goto_level(const char *path)
{
    /* HACK: must be around for the duration of the game. */
    static struct level level;

    if (path && level_load(path, &level))
    {
        progress_init(MODE_STANDALONE);

        if (progress_play(&level))
            return 1;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

/*
 * Handle the link option.
 *
 * This navigates to the appropriate screen, if the asset was found.
 *
 * Supported link types:
 *
 * --link set-easy
 * --link set-easy/peasy
 */
static int link_handle(const char *link)
{
    int processed = 0;

    if (!(link && *link))
        return 0;

    log_printf("Link: handling %s\n", link);

    if (str_starts_with(link, "set-"))
    {
        /* Search installed sets and package list. */

        const size_t prefix_len = strcspn(link, "/");

        const char *set_part = SUBSTR(link, 0, prefix_len);
        const char *map_part = SUBSTR(link, prefix_len + 1, 64);
        const char *set_file = JOINSTR(set_part, ".txt");

        int index;
        int found_level = 0;

        log_printf("Link: searching for set %s\n", set_file);

        set_init();

        if ((index = set_find(set_file)) >= 0)
        {
            log_printf("Link: found set match for %s\n", set_file);

            set_goto(index);

            if (map_part && *map_part)
            {
                /* Search for the given level. */

                const char *sol_basename = JOINSTR(map_part, ".sol");
                struct level *level;

                log_printf("Link: searching for level %s\n", sol_basename);

                if ((level = set_find_level(sol_basename)))
                {
                    log_printf("Link: found level match for %s\n", sol_basename);

                    progress_init(MODE_NORMAL);

                    if (progress_play(level))
                    {
                        goto_state(&st_level);
                        found_level = 1;
                        processed = 1;
                    }
                }
                else
                    log_printf("Link: no such level\n");
            }

            if (!found_level)
            {
                load_title_background();
                game_kill_fade();
                goto_state(&st_start);
                processed = 1;
            }
        }
        else if ((index = package_search(set_file)) >= 0)
        {
            log_printf("Link: found package match for %s\n", set_file);
            goto_package(index, &st_title);
            processed = 1;
        }
        else log_printf("Link: no such set or package\n", link);
    }

    return processed;
}

/*---------------------------------------------------------------------------*/

static void refresh_packages_done(void *data, void *extra_data)
{
    struct state *start_state = data;

    if (link_handle(opt_link))
        return;

    goto_state(start_state);
}

/*
 * Refresh package list, process links, go to starting screen.
 *
 * This is weird, I agree. Lots of parts moving together.
 */
static void main_preload(struct state *start_state)
{
    struct fetch_callback callback = { 0 };

    callback.data = start_state;
    callback.done = refresh_packages_done;

    goto_state(&st_loading);

    /* Link processing works best with a package list. */

    if (package_refresh(callback))
    {
        /* Callback takes care of link processing and starting screen. */
        return;
    }

    /* But attempt it even without a package list. */

    if (link_handle(opt_link))
    {
        /* Link processing navigates to the appropriate screen. */
        return;
    }

    /* Otherwise, go to the starting screen. */

    goto_state(start_state);
}

/*---------------------------------------------------------------------------*/

static int loop(void)
{
    SDL_Event e;
    int d = 1;

    int ax, ay, dx, dy;

#ifdef __EMSCRIPTEN__
    /* Since we are in the browser, and want to look good on every device,
     * naturally, we use CSS to do layout. The canvas element has two sizes:
     * the layout size ("window") and the drawing buffer size ("resolution").
     * Here, we get the canvas layout size and set the canvas resolution
     * to match. To update a bunch of internal state, we use SDL_SetWindowSize
     * to set the canvas resolution.
     */

    double clientWidth, clientHeight;

    int w, h;

    emscripten_get_element_css_size("#canvas", &clientWidth, &clientHeight);

    w = (int) clientWidth;
    h = (int) clientHeight;

    if (w != video.window_w || h != video.window_h)
        video_set_window_size(w, h);
#endif

    /* Process SDL events. */

    while (d && SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            return 0;

#ifdef __EMSCRIPTEN__
        case SDL_USEREVENT:
            switch (e.user.code)
            {
                case USER_EVENT_BACK:
                    d = st_keybd(KEY_EXIT, 1);
                    break;

                case USER_EVENT_PAUSE:
                    if (video_get_grab())
                        goto_state(&st_pause);
                    break;
            }
            break;
#endif

        case SDL_MOUSEMOTION:
            /* Convert to bottom-left origin. */

            ax = +e.motion.x;
            ay = -e.motion.y + video.window_h;
            dx = +e.motion.xrel;
            dy = (config_get_d(CONFIG_MOUSE_INVERT) ?
                  +e.motion.yrel : -e.motion.yrel);

            /* Scale to viewport pixels. */

            ax = ROUND(ax * video.device_scale);
            ay = ROUND(ay * video.device_scale);
            dx = ROUND(dx * video.device_scale);
            dy = ROUND(dy * video.device_scale);

            st_point(ax, ay, dx, dy);

            break;

        case SDL_MOUSEBUTTONDOWN:
            d = st_click(e.button.button, 1);
            break;

        case SDL_MOUSEBUTTONUP:
            d = st_click(e.button.button, 0);
            break;

        case SDL_FINGERDOWN:
        case SDL_FINGERUP:
        case SDL_FINGERMOTION:
            d = st_touch(&e.tfinger);
            break;

        case SDL_KEYDOWN:
            d = handle_key_dn(&e);
            break;

        case SDL_KEYUP:
            d = handle_key_up(&e);
            break;

        case SDL_WINDOWEVENT:
            switch (e.window.event)
            {
            case SDL_WINDOWEVENT_FOCUS_LOST:
                if (video_get_grab())
                    goto_state(&st_pause);
                break;

            case SDL_WINDOWEVENT_MOVED:
                if (config_get_d(CONFIG_DISPLAY) != video_display())
                    config_set_d(CONFIG_DISPLAY, video_display());
                break;

            case SDL_WINDOWEVENT_RESIZED:
                log_printf("Resize event (%u, %dx%d)\n",
                           e.window.windowID,
                           e.window.data1,
                           e.window.data2);
                break;

            case SDL_WINDOWEVENT_SIZE_CHANGED:
                log_printf("Size change event (%u, %dx%d)\n",
                           e.window.windowID,
                           e.window.data1,
                           e.window.data2);

                video_resize(e.window.data1, e.window.data2);
                gui_resize();

                break;
            }
            break;

        case SDL_TEXTINPUT:
            text_input_str(e.text.text, 1);
            break;

        case SDL_JOYAXISMOTION:
            joy_axis(e.jaxis.which, e.jaxis.axis, JOY_VALUE(e.jaxis.value));
            break;

        case SDL_JOYBUTTONDOWN:
            d = joy_button(e.jbutton.which, e.jbutton.button, 1);
            break;

        case SDL_JOYBUTTONUP:
            d = joy_button(e.jbutton.which, e.jbutton.button, 0);
            break;

        case SDL_JOYDEVICEADDED:
            joy_add(e.jdevice.which);
            break;

        case SDL_JOYDEVICEREMOVED:
            joy_remove(e.jdevice.which);
            break;

        case SDL_MOUSEWHEEL:
            st_wheel(e.wheel.x, e.wheel.y);
            break;

        default:
            if (e.type == FETCH_EVENT)
            {
                fetch_handle_event(e.user.data1);
            }
            break;
        }
    }

    /* Process events via the tilt sensor API. */

    if (tilt_stat())
    {
        int b;
        int s;

        st_angle(tilt_get_x(),
                 tilt_get_z());

        while (tilt_get_button(&b, &s))
        {
            const int X = config_get_d(CONFIG_JOYSTICK_AXIS_X0);
            const int Y = config_get_d(CONFIG_JOYSTICK_AXIS_Y0);
            const int L = config_get_d(CONFIG_JOYSTICK_DPAD_L);
            const int R = config_get_d(CONFIG_JOYSTICK_DPAD_R);
            const int U = config_get_d(CONFIG_JOYSTICK_DPAD_U);
            const int D = config_get_d(CONFIG_JOYSTICK_DPAD_D);

            if (b == L || b == R || b == U || b == D)
            {
                static int pad[4] = { 0, 0, 0, 0 };

                /* Track the state of the D-pad buttons. */

                if      (b == L) pad[0] = s;
                else if (b == R) pad[1] = s;
                else if (b == U) pad[2] = s;
                else if (b == D) pad[3] = s;

                /* Convert D-pad button events into joystick axis motion. */

                if      (pad[0] && !pad[1]) st_stick(X, -1.0f);
                else if (pad[1] && !pad[0]) st_stick(X, +1.0f);
                else                        st_stick(X,  0.0f);

                if      (pad[2] && !pad[3]) st_stick(Y, -1.0f);
                else if (pad[3] && !pad[2]) st_stick(Y, +1.0f);
                else                        st_stick(Y,  0.0f);
            }
            else d = st_buttn(b, s);
        }
    }

    return d;
}

/*---------------------------------------------------------------------------*/

static int is_replay(struct dir_item *item)
{
    return str_ends_with(item->path, ".nbr");
}

static int is_score_file(struct dir_item *item)
{
    return str_starts_with(item->path, "neverballhs-");
}

static void make_dirs_and_migrate(void)
{
    Array items;
    int i;

    const char *src;
    char *dst;

    if (fs_mkdir("Replays"))
    {
        if ((items = fs_dir_scan("", is_replay)))
        {
            for (i = 0; i < array_len(items); i++)
            {
                src = DIR_ITEM_GET(items, i)->path;
                dst = concat_string("Replays/", src, NULL);
                fs_rename(src, dst);
                free(dst);
            }

            fs_dir_free(items);
        }
    }

    if (fs_mkdir("Scores"))
    {
        if ((items = fs_dir_scan("", is_score_file)))
        {
            for (i = 0; i < array_len(items); i++)
            {
                src = DIR_ITEM_GET(items, i)->path;
                dst = concat_string("Scores/",
                                    src + sizeof ("neverballhs-") - 1,
                                    ".txt",
                                    NULL);
                fs_rename(src, dst);
                free(dst);
            }

            fs_dir_free(items);
        }
    }

    fs_mkdir("Screenshots");
}

/*---------------------------------------------------------------------------*/

static int handle_installed_action(int pi)
{
    /* Go to level set. */

    if (pi >= 0 && strcmp(package_get_type(pi), "set") == 0)
    {
        const char *package_id = package_get_id(pi);
        const char *file = JOINSTR(package_id, ".txt");
        int index = -1;

        set_init();

        index = set_find(file);

        return index >= 0 ? goto_start(index, &st_package) : 1;
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

static void main_quit(void);

struct main_loop
{
    Uint32 now;
    unsigned int done:1;
};

static void step(void *data)
{
    struct main_loop *mainloop = (struct main_loop *) data;

    int running = loop();

    if (running)
    {
        Uint32 now = SDL_GetTicks();
        Uint32 dt = (now - mainloop->now);

        if (0 < dt && dt < 1000)
        {
            /* Step the game state. */

            st_timer(0.001f * dt);

            /* Render. */

            hmd_step();
            st_paint(0.001f * now);
            video_swap();
        }

        mainloop->now = now;
    }

    mainloop->done = !running;

#ifdef __EMSCRIPTEN__
    /* On Emscripten, we never return to main(), so we have to do shutdown here. */

    if (mainloop->done)
    {
        emscripten_cancel_main_loop();
        main_quit();

        EM_ASM({
            Neverball.quit();
        });
    }
#endif
}

/*
 * Initialize all systems.
 */
static int main_init(int argc, char *argv[])
{
    if (!fs_init(argc > 0 ? argv[0] : NULL))
    {
        fprintf(stderr, "Failure to initialize file system (%s)\n", fs_error());
        return 0;
    }

    opt_init(argc, argv);

    config_paths(opt_data);
    log_init("Neverball " VERSION, "neverball.log");
    make_dirs_and_migrate();

    /* Initialize SDL. */

#ifdef SDL_HINT_TOUCH_MOUSE_EVENTS
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
#endif

    if (SDL_Init(SDL_INIT_VIDEO) == -1)
    {
        log_printf("Failure to initialize SDL (%s)\n", SDL_GetError());
        return 0;
    }

    /* Intitialize configuration. */

    config_init();
    config_load();

    fetch_enable(config_get_d(CONFIG_ONLINE));

    package_init();

    package_set_installed_action(handle_installed_action);

    /* Enable joystick events. */

    joy_init();

    /* Initialize localization. */

    lang_init();

    /* Initialize audio. */

    audio_init();
    audio_cache(AUD_MENU);
    audio_cache(AUD_BALL);
    audio_cache(AUD_BUMPS);
    audio_cache(AUD_BUMPM);
    audio_cache(AUD_BUMPL);
    audio_cache(AUD_COIN);
    audio_cache(AUD_TICK);
    audio_cache(AUD_TOCK);
    audio_cache(AUD_SWITCH);
    audio_cache(AUD_JUMP);
    audio_cache(AUD_GOAL);
    audio_cache(AUD_SCORE);
    audio_cache(AUD_FALL);
    audio_cache(AUD_TIME);
    audio_cache(AUD_OVER);
    audio_cache(AUD_GROW);
    audio_cache(AUD_SHRINK);
    audio_cache(AUD_JUMP);
    tilt_init();

    /* Initialize video. */

    if (!video_init())
        return 0;

    /* Material system. */

    mtrl_init();

    return 1;
}

/*
 * Shut down all systems.
 */
static void main_quit(void)
{
    config_save();

    /* Free loaded sets, in case of link processing. */

    set_quit();

    /* Free everything else. */

    goto_state(&st_null);

    mtrl_quit();
    video_quit();
    tilt_free();
    audio_free();
    lang_quit();
    joy_quit();
    config_quit();
    package_quit();
    fetch_enable(0);
    SDL_Quit();
    log_quit();
    fs_quit();
    opt_quit();
}

int main(int argc, char *argv[])
{
    struct main_loop mainloop = { 0 };

    struct state *start_state = &st_title;

    if (!main_init(argc, argv))
        return 1;

    /* Screen states. */

    init_state(&st_null);

    /* Initialize demo playback or load the level. */

    if (opt_replay && fs_add_path(dir_name(opt_replay)) && progress_replay(base_name(opt_replay)))
    {
        demo_play_goto(1);
        start_state = &st_demo_play;
    }
    else if (opt_level)
    {
        const char *path = fs_resolve(opt_level);

        if (goto_level(path))
            start_state = &st_level;
        else
            log_printf("File %s is not in game path\n", opt_level);
    }

    main_preload(start_state);

    /* Run the main game loop. */

    mainloop.now = SDL_GetTicks();

#ifdef __EMSCRIPTEN__
    /*
     * The Emscripten main loop is asynchronous. In other words,
     * emscripten_set_main_loop_arg() returns immediately. The fourth
     * parameter basically just determines what happens with main()
     * beyond this point:
     *
     *   0 = execution continues to the end of main().
     *   1 = execution stops here, the rest of main() is never executed.
     *
     * It's best not to put anything after this.
     */
    emscripten_set_main_loop_arg(step, (void *) &mainloop, 0, 1);
#else
    while (!mainloop.done)
        step(&mainloop);

    main_quit();
#endif

    return 0;
}

/*---------------------------------------------------------------------------*/

