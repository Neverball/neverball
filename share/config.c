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

#include "config.h"
#include "glext.h"
#include "geom.h"
#include "text.h"

/*---------------------------------------------------------------------------*/

/* Define the chdir symbol. */

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

/*---------------------------------------------------------------------------*/

static int mode         = SDL_OPENGL | SDL_FULLSCREEN;
static int width        = CONFIG_DEF_WIDTH;
static int height       = CONFIG_DEF_HEIGHT;
static int stereo       = CONFIG_DEF_STEREO;
static int camera       = CONFIG_DEF_CAMERA;
static int textures     = CONFIG_DEF_TEXTURES;
static int geometry     = CONFIG_DEF_GEOMETRY;
static int reflection   = CONFIG_DEF_REFLECTION;
static int audio_rate   = CONFIG_DEF_AUDIO_RATE;
static int audio_buff   = CONFIG_DEF_AUDIO_BUFF;
static int mouse_sense  = CONFIG_DEF_MOUSE_SENSE;
static int mouse_inv    = CONFIG_DEF_MOUSE_INV;
static int niceness     = CONFIG_DEF_NICE;
static int fps          = CONFIG_DEF_FPS;
static int joy          = CONFIG_DEF_JOY;
static int joy_device   = CONFIG_DEF_JOY_DEVICE;
static int sound_vol    = CONFIG_DEF_SOUND_VOL;
static int music_vol    = CONFIG_DEF_MUSIC_VOL;

static int axis_x       = CONFIG_DEF_AXIS_X;
static int axis_y       = CONFIG_DEF_AXIS_Y;
static int button_a     = CONFIG_DEF_BUTTON_A;
static int button_b     = CONFIG_DEF_BUTTON_B;
static int button_r     = CONFIG_DEF_BUTTON_R;
static int button_l     = CONFIG_DEF_BUTTON_L;
static int button_exit  = CONFIG_DEF_BUTTON_EXIT;

static int key_cam_1    = CONFIG_DEF_KEY_CAM_1;
static int key_cam_2    = CONFIG_DEF_KEY_CAM_2;
static int key_cam_3    = CONFIG_DEF_KEY_CAM_3;
static int key_cam_r    = CONFIG_DEF_KEY_CAM_R;
static int key_cam_l    = CONFIG_DEF_KEY_CAM_L;

char player[MAXNAM]     = DEFAULT_NAME;

/*---------------------------------------------------------------------------*/

/*
 * Convert the given file name to  an absolute path name in the user's
 * home  directory.   If the  home  directory  cannot be  established,
 * return false.
 *
 * HACK: under Windows just assume the user has permission to write to
 * the data  directory.  This is  more reliable than trying  to devine
 * anything reasonable from the environment.
 */
int config_home(char *dst, const char *src, size_t n)
{
#ifdef _WIN32

    strncpy(dst, src, n);
    return 1;

#else

    char *dir;

    if ((dir = getenv("HOME")))
    {
        size_t d = strlen(dir);

        strncpy(dst, dir, n - 1);
        strncat(dst, "/", n - d - 1);
        strncat(dst, src, n - d - 2);

        return 1;
    }
    return 0;

#endif
}

/*
 * Game  assets are  accessed  via relative  paths.   Set the  current
 * directory to the root of the asset hierarchy.  Confirm the location
 * by checking for the presence of the named file.
 */
int config_path(const char *path, const char *test)
{
    FILE *fp;

    chdir(path);

    if ((fp = fopen(test, "r")))
    {
        fclose(fp);
        return 1;
    }
    return 0;
}

/*
 * Confirm the existence of a readable replay file.
 */
int config_demo(void)
{
    char path[MAXSTR];
    FILE *fp;

    if (config_home(path, USER_REPLAY_FILE, MAXSTR))
        if ((fp = fopen(path, FMODE_RB)))
        {
            fclose(fp);
            return 1;
        }

    return 0;
}

/*---------------------------------------------------------------------------*/

int config_key(const char *s, int d)
{
    int c;

    for (c = 0; c < SDLK_LAST; c++)
        if (strcmp(s, SDL_GetKeyName(c)) == 0)
            return c;

    return d;
}

/*---------------------------------------------------------------------------*/

void config_load(void)
{
    char  path[MAXSTR];
    FILE *fp;

    if (config_home(path, USER_CONFIG_FILE, MAXSTR) && (fp = fopen(path, "r")))
    {
        char buf[MAXSTR];
        char key[MAXSTR];
        char str[MAXSTR];
        int  val;

        while (fgets(buf, MAXSTR, fp))
        {
            if (sscanf(buf, "%s %d", key, &val) == 2)
            {
                if (strcmp(key, "fullscreen") == 0)
                    mode = SDL_OPENGL | (val ? SDL_FULLSCREEN : 0);

                if (strcmp(key, "width")        == 0) width        = val;
                if (strcmp(key, "height")       == 0) height       = val;
                if (strcmp(key, "stereo")       == 0) stereo       = val;
                if (strcmp(key, "camera")       == 0) camera       = val;
                if (strcmp(key, "textures")     == 0) textures     = val;
                if (strcmp(key, "geometry")     == 0) geometry     = val;
                if (strcmp(key, "reflection")   == 0) reflection   = val;
                if (strcmp(key, "audio_rate")   == 0) audio_rate   = val;
                if (strcmp(key, "audio_buff")   == 0) audio_buff   = val;
                if (strcmp(key, "mouse_sense")  == 0) mouse_sense  = val;
                if (strcmp(key, "mouse_inv")    == 0) mouse_inv    = val;
                if (strcmp(key, "niceness")     == 0) niceness     = val;
                if (strcmp(key, "fps")          == 0) fps          = val;
                if (strcmp(key, "joy")          == 0) joy          = val;
                if (strcmp(key, "joy_device")   == 0) joy_device   = val;
                if (strcmp(key, "sound_vol")    == 0) sound_vol    = val;
                if (strcmp(key, "music_vol")    == 0) music_vol    = val;

                if (strcmp(key, "axis_x")       == 0) axis_x       = val;
                if (strcmp(key, "axis_y")       == 0) axis_y       = val;
                if (strcmp(key, "button_a")     == 0) button_a     = val;
                if (strcmp(key, "button_b")     == 0) button_b     = val;
                if (strcmp(key, "button_r")     == 0) button_r     = val;
                if (strcmp(key, "button_l")     == 0) button_l     = val;
                if (strcmp(key, "button_exit")  == 0) button_exit  = val;

                if (strcmp(key, "shadow")       == 0) glext_set_shadow(val);
            }

            else if (sscanf(buf, "%s %s", key, str) == 2)
            {
                if (strcmp(key, "player") == 0)
                    strncpy(player, str, MAXNAM);

                if (strcmp(key, "key_cam_1")  == 0)
                    key_cam_1 = config_key(str, CONFIG_DEF_KEY_CAM_1);
                if (strcmp(key, "key_cam_2")  == 0)
                    key_cam_2 = config_key(str, CONFIG_DEF_KEY_CAM_2);
                if (strcmp(key, "key_cam_3")  == 0)
                    key_cam_3 = config_key(str, CONFIG_DEF_KEY_CAM_3);
                if (strcmp(key, "key_cam_r")  == 0)
                    key_cam_r = config_key(str, CONFIG_DEF_KEY_CAM_R);
                if (strcmp(key, "key_cam_l")  == 0)
                    key_cam_l = config_key(str, CONFIG_DEF_KEY_CAM_L);
            }
        }

        fclose(fp);
    }
}

void config_store(void)
{
    char  path[MAXSTR];
    FILE *fp;

    if (config_home(path, USER_CONFIG_FILE, MAXSTR) && (fp = fopen(path, "w")))
    {
        fprintf(fp, "fullscreen %d\n",  (mode & SDL_FULLSCREEN) ? 1 : 0);
        fprintf(fp, "width %d\n",        width);
        fprintf(fp, "height %d\n",       height);
        fprintf(fp, "stereo %d\n",       stereo);
        fprintf(fp, "camera %d\n",       camera);
        fprintf(fp, "textures %d\n",     textures);
        fprintf(fp, "geometry %d\n",     geometry);
        fprintf(fp, "reflection %d\n",   reflection);
        fprintf(fp, "audio_rate %d\n",   audio_rate);
        fprintf(fp, "audio_buff %d\n",   audio_buff);
        fprintf(fp, "mouse_sense %d\n",  mouse_sense);
        fprintf(fp, "mouse_inv %d\n",    mouse_inv);
        fprintf(fp, "player %s\n",       player);
        fprintf(fp, "niceness %d\n",     niceness);
        fprintf(fp, "fps %d\n",          fps);
        fprintf(fp, "joy %d\n",          joy);
        fprintf(fp, "joy_device %d\n",   joy_device);
        fprintf(fp, "sound_vol %d\n",    sound_vol);
        fprintf(fp, "music_vol %d\n",    music_vol);

        fprintf(fp, "axis_x %d\n",       axis_x);
        fprintf(fp, "axis_y %d\n",       axis_y);
        fprintf(fp, "button_r %d\n",     button_r);
        fprintf(fp, "button_l %d\n",     button_l);
        fprintf(fp, "button_a %d\n",     button_a);
        fprintf(fp, "button_b %d\n",     button_b);
        fprintf(fp, "button_exit %d\n",  button_exit);

        fprintf(fp, "key_cam_1 %s\n",    SDL_GetKeyName(key_cam_1));
        fprintf(fp, "key_cam_2 %s\n",    SDL_GetKeyName(key_cam_2));
        fprintf(fp, "key_cam_3 %s\n",    SDL_GetKeyName(key_cam_3));
        fprintf(fp, "key_cam_r %s\n",    SDL_GetKeyName(key_cam_r));
        fprintf(fp, "key_cam_l %s\n",    SDL_GetKeyName(key_cam_l));

        fprintf(fp, "shadow %d\n",       glext_shadow());

        fclose(fp);
    }
}

/*---------------------------------------------------------------------------*/

int config_mode(void) { return mode; }
int config_w   (void) { return width;  }
int config_h   (void) { return height; }
int config_view(void) { return camera; }
int config_text(void) { return textures; }
int config_geom(void) { return geometry; }
int config_refl(void) { return reflection; }
int config_rate(void) { return audio_rate; }
int config_buff(void) { return audio_buff; }
int config_sens(void) { return mouse_sense; }
int config_inv (void) { return mouse_inv; }
int config_nice(void) { return niceness; }
int config_fps (void) { return fps; }
int config_joy_device(void) { return joy_device; }
int config_sound(void) { return sound_vol; }
int config_music(void) { return music_vol; }
int config_stereo(void) { return stereo; }

int config_axis_x(int a)   { return (joy && a == axis_x); }
int config_axis_y(int a)   { return (joy && a == axis_y); }
int config_button_a(int b) { return (joy && b == button_a); }
int config_button_b(int b) { return (joy && b == button_b); }
int config_button_r(int b) { return (joy && b == button_r); }
int config_button_l(int b) { return (joy && b == button_l); }
int config_button_X(int b) { return (joy && b == button_exit); }

int config_key_cam_1(int b)    { return (b == key_cam_1); }
int config_key_cam_2(int b)    { return (b == key_cam_2); }
int config_key_cam_3(int b)    { return (b == key_cam_3); }
int config_key_cam_r(int b)    { return (b == key_cam_r); }
int config_key_cam_l(int b)    { return (b == key_cam_l); }

char *config_get_key_cam_1(void) { return SDL_GetKeyName(key_cam_1); }
char *config_get_key_cam_2(void) { return SDL_GetKeyName(key_cam_2); }
char *config_get_key_cam_3(void) { return SDL_GetKeyName(key_cam_3); }

/*---------------------------------------------------------------------------*/

int config_set_mode(int w, int h, int m)
{
    if (SDL_SetVideoMode(w, h, 0, m))
    {
        mode   = m;
        width  = w;
        height = h;

        glViewport(0, 0, w, h);
        glClearColor(0.0f, 0.8f, 1.0f, 0.0f);

        glEnable(GL_NORMALIZE);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);

        return 1;
    }
    return 0;
}

void config_set_text(int t)
{
    textures = t;
}

void config_set_geom(int g)
{
    geometry = g;
}

void config_set_refl(int r)
{
    reflection = r;
}

void config_set_audio(int r, int b)
{
    audio_rate = r;
    audio_buff = b;
}

void config_set_sound(int n)
{
    if (0 <= n && n <= 10)
        sound_vol = n;
}

void config_set_music(int n)
{
    if (0 <= n && n <= 10)
        music_vol = n;
}

void config_set_view(int c)
{
    camera = c;
}

void config_tog_nice(void)
{
    niceness = niceness ? 0 : 1;
}

void config_tog_fps(void)
{
    fps = fps ? 0 : 1;
}

/*---------------------------------------------------------------------------*/

void config_push_persp(float fov, float n, float f)
{
    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();
        gluPerspective(fov, (double) width / height, n, f);
    }
    glMatrixMode(GL_MODELVIEW);
}

void config_push_ortho(void)
{
    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, width, 0.0, height, -1.0, +1.0);
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

/*---------------------------------------------------------------------------*/
