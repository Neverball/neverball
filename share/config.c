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
#include <SDL_mixer.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include "config.h"
#include "glext.h"
#include "vec3.h"

/*---------------------------------------------------------------------------*/

/* Define the mkdir symbol. */

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

/*---------------------------------------------------------------------------*/

static int   option_d[CONFIG_OPTION_D_COUNT];
static char *option_s[CONFIG_OPTION_S_COUNT];

static int dirty = 0;

/*---------------------------------------------------------------------------*/

static void config_key(const char *s, int i, int d)
{
    int c;

    config_set_d(i, d);

    for (c = 0; c < SDLK_LAST; c++)
        if (strcmp(s, SDL_GetKeyName(c)) == 0)
        {
            config_set_d(i, c);
            break;
        }
}

/*---------------------------------------------------------------------------*/

void config_init(void)
{
    memset(option_d, 0, CONFIG_OPTION_D_COUNT * sizeof (int));
    memset(option_s, 0, CONFIG_OPTION_S_COUNT * sizeof (char *));

    config_set_d(CONFIG_FULLSCREEN,           DEFAULT_FULLSCREEN);
    config_set_d(CONFIG_WIDTH,                DEFAULT_WIDTH);
    config_set_d(CONFIG_HEIGHT,               DEFAULT_HEIGHT);
    config_set_d(CONFIG_STEREO,               DEFAULT_STEREO);
    config_set_d(CONFIG_CAMERA,               DEFAULT_CAMERA);
    config_set_d(CONFIG_TEXTURES,             DEFAULT_TEXTURES);
    config_set_d(CONFIG_GEOMETRY,             DEFAULT_GEOMETRY);
    config_set_d(CONFIG_REFLECTION,           DEFAULT_REFLECTION);
    config_set_d(CONFIG_BACKGROUND,           DEFAULT_BACKGROUND);
    config_set_d(CONFIG_SHADOW,               DEFAULT_SHADOW);
    config_set_d(CONFIG_AUDIO_RATE,           DEFAULT_AUDIO_RATE);
    config_set_d(CONFIG_AUDIO_BUFF,           DEFAULT_AUDIO_BUFF);
    config_set_d(CONFIG_MOUSE_SENSE,          DEFAULT_MOUSE_SENSE);
    config_set_d(CONFIG_MOUSE_INVERT,         DEFAULT_MOUSE_INVERT);
    config_set_d(CONFIG_NICE,                 DEFAULT_NICE);
    config_set_d(CONFIG_FPS,                  DEFAULT_FPS);
    config_set_d(CONFIG_SOUND_VOLUME,         DEFAULT_SOUND_VOLUME);
    config_set_d(CONFIG_MUSIC_VOLUME,         DEFAULT_MUSIC_VOLUME);
    config_set_d(CONFIG_JOYSTICK,             DEFAULT_JOYSTICK);
    config_set_d(CONFIG_JOYSTICK_DEVICE,      DEFAULT_JOYSTICK_DEVICE);
    config_set_d(CONFIG_JOYSTICK_AXIS_X,      DEFAULT_JOYSTICK_AXIS_X);
    config_set_d(CONFIG_JOYSTICK_AXIS_Y,      DEFAULT_JOYSTICK_AXIS_Y);
    config_set_d(CONFIG_JOYSTICK_BUTTON_A,    DEFAULT_JOYSTICK_BUTTON_A);
    config_set_d(CONFIG_JOYSTICK_BUTTON_B,    DEFAULT_JOYSTICK_BUTTON_B);
    config_set_d(CONFIG_JOYSTICK_BUTTON_L,    DEFAULT_JOYSTICK_BUTTON_L);
    config_set_d(CONFIG_JOYSTICK_BUTTON_R,    DEFAULT_JOYSTICK_BUTTON_R);
    config_set_d(CONFIG_JOYSTICK_BUTTON_EXIT, DEFAULT_JOYSTICK_BUTTON_EXIT);
    config_set_d(CONFIG_JOYSTICK_CAMERA_1,    DEFAULT_JOYSTICK_CAMERA_1);
    config_set_d(CONFIG_JOYSTICK_CAMERA_2,    DEFAULT_JOYSTICK_CAMERA_2);
    config_set_d(CONFIG_JOYSTICK_CAMERA_3,    DEFAULT_JOYSTICK_CAMERA_3);
    config_set_d(CONFIG_KEY_CAMERA_1,         DEFAULT_KEY_CAMERA_1);
    config_set_d(CONFIG_KEY_CAMERA_2,         DEFAULT_KEY_CAMERA_2);
    config_set_d(CONFIG_KEY_CAMERA_3,         DEFAULT_KEY_CAMERA_3);
    config_set_d(CONFIG_KEY_CAMERA_R,         DEFAULT_KEY_CAMERA_R);
    config_set_d(CONFIG_KEY_CAMERA_L,         DEFAULT_KEY_CAMERA_L);
    config_set_d(CONFIG_VIEW_FOV,             DEFAULT_VIEW_FOV);
    config_set_d(CONFIG_VIEW_DP,              DEFAULT_VIEW_DP);
    config_set_d(CONFIG_VIEW_DC,              DEFAULT_VIEW_DC);
    config_set_d(CONFIG_VIEW_DZ,              DEFAULT_VIEW_DZ);
    config_set_d(CONFIG_ROTATE_FAST,          DEFAULT_ROTATE_FAST);
    config_set_d(CONFIG_ROTATE_SLOW,          DEFAULT_ROTATE_SLOW);
    config_set_d(CONFIG_LAST_SET,             DEFAULT_LAST_SET);
    config_set_s(CONFIG_PLAYER,               DEFAULT_PLAYER);
    config_set_s(CONFIG_BALL,                 DEFAULT_BALL);
    config_set_s(CONFIG_COIN,                 DEFAULT_COIN);
    config_set_s(CONFIG_LANG,                 DEFAULT_LANG);
}

void config_load(void)
{
    FILE *fp;

    if ((fp = fopen(config_user(USER_CONFIG_FILE), "r")))
    {
        char buf[MAXSTR];
        char key[MAXSTR];
        char val[MAXSTR];

        while (fgets(buf, MAXSTR, fp))
            if (sscanf(buf, "%s %s", key, val) == 2)
            {
                if      (strcmp(key, "fullscreen")            == 0)
                    config_set_d(CONFIG_FULLSCREEN,           atoi(val));
                else if (strcmp(key, "width")                 == 0)
                    config_set_d(CONFIG_WIDTH,                atoi(val));
                else if (strcmp(key, "height")                == 0)
                    config_set_d(CONFIG_HEIGHT,               atoi(val));
                else if (strcmp(key, "stereo")                == 0)
                    config_set_d(CONFIG_STEREO,               atoi(val));
                else if (strcmp(key, "camera")                == 0)
                    config_set_d(CONFIG_CAMERA,               atoi(val));
                else if (strcmp(key, "textures")              == 0)
                    config_set_d(CONFIG_TEXTURES,             atoi(val));
                else if (strcmp(key, "geometry")              == 0)
                    config_set_d(CONFIG_GEOMETRY,             atoi(val));
                else if (strcmp(key, "reflection")            == 0)
                    config_set_d(CONFIG_REFLECTION,           atoi(val));
                else if (strcmp(key, "background")            == 0)
                    config_set_d(CONFIG_BACKGROUND,           atoi(val));
                else if (strcmp(key, "shadow")                == 0)
                    config_set_d(CONFIG_SHADOW,               atoi(val));
                else if (strcmp(key, "audio_rate")            == 0)
                    config_set_d(CONFIG_AUDIO_RATE,           atoi(val));
                else if (strcmp(key, "audio_buff")            == 0)
                    config_set_d(CONFIG_AUDIO_BUFF,           atoi(val));
                else if (strcmp(key, "mouse_sense")           == 0)
                    config_set_d(CONFIG_MOUSE_SENSE,          atoi(val));
                else if (strcmp(key, "mouse_invert")          == 0)
                    config_set_d(CONFIG_MOUSE_INVERT,         atoi(val));
                else if (strcmp(key, "nice")                  == 0)
                    config_set_d(CONFIG_NICE,                 atoi(val));
                else if (strcmp(key, "fps")                   == 0)
                    config_set_d(CONFIG_FPS,                  atoi(val));
                else if (strcmp(key, "sound_volume")          == 0)
                    config_set_d(CONFIG_SOUND_VOLUME,         atoi(val));
                else if (strcmp(key, "music_volume")          == 0)
                    config_set_d(CONFIG_MUSIC_VOLUME,         atoi(val));
                else if (strcmp(key, "joystick")              == 0)
                    config_set_d(CONFIG_JOYSTICK,             atoi(val));
                else if (strcmp(key, "joystick_device")       == 0)
                    config_set_d(CONFIG_JOYSTICK_DEVICE,      atoi(val));
                else if (strcmp(key, "joystick_axis_x")       == 0)
                    config_set_d(CONFIG_JOYSTICK_AXIS_X,      atoi(val));
                else if (strcmp(key, "joystick_axis_y")       == 0)
                    config_set_d(CONFIG_JOYSTICK_AXIS_Y,      atoi(val));
                else if (strcmp(key, "joystick_button_a")     == 0)
                    config_set_d(CONFIG_JOYSTICK_BUTTON_A,    atoi(val));
                else if (strcmp(key, "joystick_button_b")     == 0)
                    config_set_d(CONFIG_JOYSTICK_BUTTON_B,    atoi(val));
                else if (strcmp(key, "joystick_button_r")     == 0)
                    config_set_d(CONFIG_JOYSTICK_BUTTON_R,    atoi(val));
                else if (strcmp(key, "joystick_button_l")     == 0)
                    config_set_d(CONFIG_JOYSTICK_BUTTON_L,    atoi(val));
                else if (strcmp(key, "joystick_button_exit")  == 0)
                    config_set_d(CONFIG_JOYSTICK_BUTTON_EXIT, atoi(val));
                else if (strcmp(key, "joystick_camera_1")     == 0)
                    config_set_d(CONFIG_JOYSTICK_CAMERA_1,    atoi(val));
                else if (strcmp(key, "joystick_camera_2")     == 0)
                    config_set_d(CONFIG_JOYSTICK_CAMERA_2,    atoi(val));
                else if (strcmp(key, "joystick_camera_3")     == 0)
                    config_set_d(CONFIG_JOYSTICK_CAMERA_3,    atoi(val));
                else if (strcmp(key, "view_fov")              == 0)
                    config_set_d(CONFIG_VIEW_FOV,             atoi(val));
                else if (strcmp(key, "view_dp")               == 0)
                    config_set_d(CONFIG_VIEW_DP,              atoi(val));
                else if (strcmp(key, "view_dc")               == 0)
                    config_set_d(CONFIG_VIEW_DC,              atoi(val));
                else if (strcmp(key, "view_dz")               == 0)
                    config_set_d(CONFIG_VIEW_DZ,              atoi(val));
                else if (strcmp(key, "rotate_fast")           == 0)
                    config_set_d(CONFIG_ROTATE_FAST,          atoi(val));
                else if (strcmp(key, "rotate_slow")           == 0)
                    config_set_d(CONFIG_ROTATE_SLOW,          atoi(val));
                else if (strcmp(key, "last_set")              == 0)
                    config_set_d(CONFIG_LAST_SET,             atoi(val));

                else if (strcmp(key, "key_camera_1")  == 0)
                    config_key(val, CONFIG_KEY_CAMERA_1, DEFAULT_KEY_CAMERA_1);
                else if (strcmp(key, "key_camera_2")  == 0)
                    config_key(val, CONFIG_KEY_CAMERA_2, DEFAULT_KEY_CAMERA_2);
                else if (strcmp(key, "key_camera_3")  == 0)
                    config_key(val, CONFIG_KEY_CAMERA_3, DEFAULT_KEY_CAMERA_3);
                else if (strcmp(key, "key_camera_r")  == 0)
                    config_key(val, CONFIG_KEY_CAMERA_R, DEFAULT_KEY_CAMERA_R);
                else if (strcmp(key, "key_camera_l")  == 0)
                    config_key(val, CONFIG_KEY_CAMERA_L, DEFAULT_KEY_CAMERA_L);

                else if (strcmp(key, "player") == 0)
                    config_set_s(CONFIG_PLAYER, val);
                else if (strcmp(key, "ball")   == 0)
                    config_set_s(CONFIG_BALL,   val);
                else if (strcmp(key, "coin")   == 0)
                    config_set_s(CONFIG_COIN,   val);
                else if (strcmp(key, "lang")   == 0)
                    config_set_s(CONFIG_LANG,   val);
            }

        fclose(fp);

        dirty = 0;
    }
}

void config_save(void)
{
    FILE *fp;

    if (dirty && (fp = fopen(config_user(USER_CONFIG_FILE), "w")))
    {
        fprintf(fp, "fullscreen           %d\n",
                option_d[CONFIG_FULLSCREEN]);
        fprintf(fp, "width                %d\n",
                option_d[CONFIG_WIDTH]);
        fprintf(fp, "height               %d\n",
                option_d[CONFIG_HEIGHT]);
        fprintf(fp, "stereo               %d\n",
                option_d[CONFIG_STEREO]);
        fprintf(fp, "camera               %d\n",
                option_d[CONFIG_CAMERA]);
        fprintf(fp, "textures             %d\n",
                option_d[CONFIG_TEXTURES]);
        fprintf(fp, "geometry             %d\n",
                option_d[CONFIG_GEOMETRY]);
        fprintf(fp, "reflection           %d\n",
                option_d[CONFIG_REFLECTION]);
        fprintf(fp, "background           %d\n",
                option_d[CONFIG_BACKGROUND]);
        fprintf(fp, "shadow               %d\n",
                option_d[CONFIG_SHADOW]);
        fprintf(fp, "audio_rate           %d\n",
                option_d[CONFIG_AUDIO_RATE]);
        fprintf(fp, "audio_buff           %d\n",
                option_d[CONFIG_AUDIO_BUFF]);
        fprintf(fp, "mouse_sense          %d\n",
                option_d[CONFIG_MOUSE_SENSE]);
        fprintf(fp, "mouse_invert         %d\n",
                option_d[CONFIG_MOUSE_INVERT]);
        fprintf(fp, "nice                 %d\n",
                option_d[CONFIG_NICE]);
        fprintf(fp, "fps                  %d\n",
                option_d[CONFIG_FPS]);
        fprintf(fp, "sound_volume         %d\n",
                option_d[CONFIG_SOUND_VOLUME]);
        fprintf(fp, "music_volume         %d\n",
                option_d[CONFIG_MUSIC_VOLUME]);
        fprintf(fp, "joystick             %d\n",
                option_d[CONFIG_JOYSTICK]);
        fprintf(fp, "joystick_device      %d\n",
                option_d[CONFIG_JOYSTICK_DEVICE]);
        fprintf(fp, "joystick_axis_x      %d\n",
                option_d[CONFIG_JOYSTICK_AXIS_X]);
        fprintf(fp, "joystick_axis_y      %d\n",
                option_d[CONFIG_JOYSTICK_AXIS_Y]);
        fprintf(fp, "joystick_button_a    %d\n",
                option_d[CONFIG_JOYSTICK_BUTTON_A]);
        fprintf(fp, "joystick_button_b    %d\n",
                option_d[CONFIG_JOYSTICK_BUTTON_B]);
        fprintf(fp, "joystick_button_r    %d\n",
                option_d[CONFIG_JOYSTICK_BUTTON_R]);
        fprintf(fp, "joystick_button_l    %d\n",
                option_d[CONFIG_JOYSTICK_BUTTON_L]);
        fprintf(fp, "joystick_button_exit %d\n",
                option_d[CONFIG_JOYSTICK_BUTTON_EXIT]);
        fprintf(fp, "joystick_camera_1    %d\n",
                option_d[CONFIG_JOYSTICK_CAMERA_1]);
        fprintf(fp, "joystick_camera_2    %d\n",
                option_d[CONFIG_JOYSTICK_CAMERA_2]);
        fprintf(fp, "joystick_camera_3    %d\n",
                option_d[CONFIG_JOYSTICK_CAMERA_3]);
        fprintf(fp, "view_fov             %d\n",
                option_d[CONFIG_VIEW_FOV]);
        fprintf(fp, "view_dp              %d\n",
                option_d[CONFIG_VIEW_DP]);
        fprintf(fp, "view_dc              %d\n",
                option_d[CONFIG_VIEW_DC]);
        fprintf(fp, "view_dz              %d\n",
                option_d[CONFIG_VIEW_DZ]);
        fprintf(fp, "rotate_fast          %d\n",
                option_d[CONFIG_ROTATE_FAST]);
        fprintf(fp, "rotate_slow          %d\n",
                option_d[CONFIG_ROTATE_SLOW]);
        fprintf(fp, "last_set             %d\n",
                option_d[CONFIG_LAST_SET]);

        fprintf(fp, "key_camera_1         %s\n",
                SDL_GetKeyName(option_d[CONFIG_KEY_CAMERA_1]));
        fprintf(fp, "key_camera_2         %s\n",
                SDL_GetKeyName(option_d[CONFIG_KEY_CAMERA_2]));
        fprintf(fp, "key_camera_3         %s\n",
                SDL_GetKeyName(option_d[CONFIG_KEY_CAMERA_3]));
        fprintf(fp, "key_camera_r         %s\n",
                SDL_GetKeyName(option_d[CONFIG_KEY_CAMERA_R]));
        fprintf(fp, "key_camera_l         %s\n",
                SDL_GetKeyName(option_d[CONFIG_KEY_CAMERA_L]));

        fprintf(fp, "player               %s\n", option_s[CONFIG_PLAYER]);
        fprintf(fp, "ball                 %s\n", option_s[CONFIG_BALL]);
        fprintf(fp, "coin                 %s\n", option_s[CONFIG_COIN]);
        fprintf(fp, "lang                 %s\n", option_s[CONFIG_LANG]);

        fclose(fp);
    }

    dirty = 0;
}

/*---------------------------------------------------------------------------*/

int config_mode(int f, int w, int h)
{
    int stereo  = config_get_d(CONFIG_STEREO)     ? 1 : 0;
    int stencil = config_get_d(CONFIG_REFLECTION) ? 1 : 0;

    SDL_GL_SetAttribute(SDL_GL_STEREO,       stereo);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, stencil);

    /* Try to set the currently specified mode. */

    if (SDL_SetVideoMode(w, h, 0, SDL_OPENGL | (f ? SDL_FULLSCREEN : 0)))
    {
        option_d[CONFIG_FULLSCREEN] = f;
        option_d[CONFIG_WIDTH]      = w;
        option_d[CONFIG_HEIGHT]     = h;
        option_d[CONFIG_SHADOW]     = option_d[CONFIG_SHADOW];

        glViewport(0, 0, w, h);
        glClearColor(0.0f, 0.0f, 0.1f, 0.0f);

        glEnable(GL_NORMALIZE);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);

        return 1;
    }

    /* If the mode failed, try it without stereo. */

    else if (stereo)
    {
        config_set_d(CONFIG_STEREO, 0);
        return config_mode(f, w, h);
    }

    /* If that mode failed, try it without reflections. */

    else if (stencil)
    {
        config_set_d(CONFIG_REFLECTION, 0);
        return config_mode(f, w, h);
    }

    /* If THAT mode failed, punt. */

    return 0;
}

/*---------------------------------------------------------------------------*/

static char data_path[MAXSTR];
static char user_path[MAXSTR];

/*
 * Given  a path  and a  file name  relative to  that path,  create an
 * absolute path name and return a temporary pointer to it.
 */
static const char *config_file(const char *path, const char *file)
{
    static char absolute[MAXSTR];

    size_t d = strlen(path);

    strncpy(absolute, path, MAXSTR - 1);
    strncat(absolute, "/",  MAXSTR - d - 1);
    strncat(absolute, file, MAXSTR - d - 2);

    return absolute;
}

static int config_test(const char *path, const char *file)
{
    if (file)
    {
        FILE *fp;

        if ((fp = fopen(config_file(path, file), "r")))
        {
            fclose(fp);
            return 1;
        }
        return 0;
    }
    return 1;
}

const char *config_data(const char *file)
{
    return config_file(data_path, file);
}

const char *config_user(const char *file)
{
    return config_file(user_path, file);
}

/*---------------------------------------------------------------------------*/

/*
 * Attempt to find  the game data directory.  Search  the command line
 * paramater,  the environment,  and the  hard-coded default,  in that
 * order.  Confirm it by checking for presense of the named file.
 */
int config_data_path(const char *path, const char *file)
{
    char *dir;

    if (path && config_test(path, file))
    {
        strncpy(data_path, path, MAXSTR);
        return 1;
    }

    if ((dir = getenv("NEVERBALL_DATA")) && config_test(dir, file))
    {
        strncpy(data_path, dir, MAXSTR);
        return 1;
    }

    if (CONFIG_DATA && config_test(CONFIG_DATA, file))
    {
        strncpy(data_path, CONFIG_DATA, MAXSTR);
        return 1;
    }

    return 0;
}

/*
 * Determine the location of  the user's home directory.  Ensure there
 * is a  directory there for  storing configuration, high  scores, and
 * replays.
 *
 * HACK: under Windows just assume the user has permission to write to
 * the data  directory.  This is  more reliable than trying  to devine
 * anything reasonable from the environment.
 */
int config_user_path(const char *file)
{
#ifdef _WIN32
    size_t d = strlen(CONFIG_USER);

    strncpy(user_path, data_path,   MAXSTR - 1);
    strncat(user_path, "\\",        MAXSTR - d - 1);
    strncat(user_path, CONFIG_USER, MAXSTR - d - 2);

    if ((mkdir(user_path) == 0) || (errno = EEXIST))
        if (config_test(user_path, file))
            return 1;
#else
    char *dir;

    if ((dir = getenv("HOME")))
    {
        size_t d = strlen(dir);

        strncpy(user_path, getenv("HOME"), MAXSTR - 1);
        strncat(user_path, "/",            MAXSTR - d - 1);
        strncat(user_path, CONFIG_USER,    MAXSTR - d - 2);
    }

    if ((mkdir(user_path, 0777) == 0) || (errno = EEXIST))
        if (config_test(user_path, file))
            return 1;
#endif

    return 0;
}

/*---------------------------------------------------------------------------*/

void config_set_d(int i, int d)
{
    option_d[i] = d;
    dirty = 1;
}

void config_tgl_d(int i)
{
    option_d[i] = (option_d[i] ? 0 : 1);
}

int config_tst_d(int i, int d)
{
    return (option_d[i] == d) ? 1 : 0;
}

int config_get_d(int i)
{
    return option_d[i];
}

/*---------------------------------------------------------------------------*/

void config_set_s(int i, const char *src)
{
    int len = (int) strlen(src);

    if (option_s[i])
        free(option_s[i]);

    if ((option_s[i] = (char *) malloc(len + 1)))
        strncpy(option_s[i], src, len + 1);

    dirty = 1;
}

void config_get_s(int i, char *dst, int len)
{
    strncpy(dst, option_s[i], len);
}

const char * config_simple_get_s(int i)
{
	return option_s[i];
}

/*---------------------------------------------------------------------------*/

static int grabbed = 0;
static int paused  = 0;

void config_set_grab(void)
{
    SDL_WarpMouse(config_get_d(CONFIG_WIDTH)  / 2,
                  config_get_d(CONFIG_HEIGHT) / 2);
    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_ShowCursor(SDL_DISABLE);
    grabbed = 1;
}

void config_clr_grab(void)
{
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_ShowCursor(SDL_ENABLE);
    grabbed = 0;
}

int  config_get_grab(void)
{
    return grabbed;
}

int  config_get_pause(void)
{
    return paused;
}

void config_set_pause(void)
{
    Mix_PauseMusic();
    paused = 1;

    if (grabbed)
    {
        SDL_ShowCursor(SDL_ENABLE);
        SDL_WM_GrabInput(SDL_GRAB_OFF);
    }
}

void config_clr_pause(void)
{
    Mix_ResumeMusic();
    paused = 0;

    if (grabbed)
    {
        SDL_WM_GrabInput(SDL_GRAB_ON);
        SDL_ShowCursor(SDL_DISABLE);
    }
}

void config_tgl_pause(void)
{
    if (paused)
        config_clr_pause();
    else
        config_set_pause();
}
/*---------------------------------------------------------------------------*/

void config_push_persp(float fov, float n, float f)
{
    GLdouble m[4][4];

    GLdouble r = fov / 2 * V_PI / 180;
    GLdouble s = sin(r);
    GLdouble c = cos(r) / s;

    GLdouble a = ((GLdouble) option_d[CONFIG_WIDTH] / 
                  (GLdouble) option_d[CONFIG_HEIGHT]);

    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();

        m[0][0] =  c/a;
        m[0][1] =  0.0;
        m[0][2] =  0.0;
        m[0][3] =  0.0;
        m[1][0] =  0.0;
        m[1][1] =    c;
        m[1][2] =  0.0;
        m[1][3] =  0.0;
        m[2][0] =  0.0;
        m[2][1] =  0.0;
        m[2][2] = -(f + n) / (f - n);
        m[2][3] = -1.0;
        m[3][0] =  0.0;
        m[3][1] =  0.0;
        m[3][2] = -2.0 * n * f / (f - n);
        m[3][3] =  0.0;

        glMultMatrixd(&m[0][0]);
    }
    glMatrixMode(GL_MODELVIEW);
}

void config_push_ortho(void)
{
    GLdouble w = (GLdouble) option_d[CONFIG_WIDTH];
    GLdouble h = (GLdouble) option_d[CONFIG_HEIGHT];

    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, w, 0.0, h, -1.0, +1.0);
    }
    glMatrixMode(GL_MODELVIEW);
}

void config_pop_matrix(void)
{
    glMatrixMode(GL_PROJECTION);
    {
        glPopMatrix();
    }
    glMatrixMode(GL_MODELVIEW);
}

void config_clear(void)
{
    if (option_d[CONFIG_REFLECTION])
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT);
    else
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);
}

/*---------------------------------------------------------------------------*/
