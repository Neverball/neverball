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

/*---------------------------------------------------------------------------*/

/* Define the chdir symbol. */

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

/*---------------------------------------------------------------------------*/

static int  option[CONFIG_OPTION_COUNT];
static char player[MAXNAM];

/*---------------------------------------------------------------------------*/

static void config_key(const char *s, int i, int d)
{
    int c;

    option[i] = d;

    for (c = 0; c < SDLK_LAST; c++)
        if (strcmp(s, SDL_GetKeyName(c)) == 0)
        {
            option[i] = c;
            break;
        }
}

/*---------------------------------------------------------------------------*/

void config_init(void)
{
    option[CONFIG_FULLSCREEN]        = DEFAULT_FULLSCREEN;
    option[CONFIG_WIDTH]             = DEFAULT_WIDTH;
    option[CONFIG_HEIGHT]            = DEFAULT_HEIGHT;
    option[CONFIG_STEREO]            = DEFAULT_STEREO;
    option[CONFIG_CAMERA]            = DEFAULT_CAMERA;
    option[CONFIG_TEXTURES]          = DEFAULT_TEXTURES;
    option[CONFIG_GEOMETRY]          = DEFAULT_GEOMETRY;
    option[CONFIG_REFLECTION]        = DEFAULT_REFLECTION;
    option[CONFIG_SHADOW]            = DEFAULT_SHADOW;
    option[CONFIG_AUDIO_RATE]        = DEFAULT_AUDIO_RATE;
    option[CONFIG_AUDIO_BUFF]        = DEFAULT_AUDIO_BUFF;
    option[CONFIG_MOUSE_SENSE]       = DEFAULT_MOUSE_SENSE;
    option[CONFIG_MOUSE_INVERT]      = DEFAULT_MOUSE_INVERT;
    option[CONFIG_NICE]              = DEFAULT_NICE;
    option[CONFIG_FPS]               = DEFAULT_FPS;
    option[CONFIG_SOUND_VOLUME]      = DEFAULT_SOUND_VOLUME;
    option[CONFIG_MUSIC_VOLUME]      = DEFAULT_MUSIC_VOLUME;
    option[CONFIG_JOYSTICK]          = DEFAULT_JOYSTICK;
    option[CONFIG_JOYSTICK_DEVICE]   = DEFAULT_JOYSTICK_DEVICE;
    option[CONFIG_JOYSTICK_AXIS_X]   = DEFAULT_JOYSTICK_AXIS_X;
    option[CONFIG_JOYSTICK_AXIS_Y]   = DEFAULT_JOYSTICK_AXIS_Y;
    option[CONFIG_JOYSTICK_BUTTON_A] = DEFAULT_JOYSTICK_BUTTON_A;
    option[CONFIG_JOYSTICK_BUTTON_B] = DEFAULT_JOYSTICK_BUTTON_B;
    option[CONFIG_JOYSTICK_BUTTON_L] = DEFAULT_JOYSTICK_BUTTON_L;
    option[CONFIG_JOYSTICK_BUTTON_R] = DEFAULT_JOYSTICK_BUTTON_R;
    option[CONFIG_KEY_CAMERA_1]      = DEFAULT_KEY_CAMERA_1;
    option[CONFIG_KEY_CAMERA_2]      = DEFAULT_KEY_CAMERA_2;
    option[CONFIG_KEY_CAMERA_3]      = DEFAULT_KEY_CAMERA_3;
    option[CONFIG_KEY_CAMERA_R]      = DEFAULT_KEY_CAMERA_R;
    option[CONFIG_KEY_CAMERA_L]      = DEFAULT_KEY_CAMERA_L;
    option[CONFIG_VIEW_FOV]          = DEFAULT_VIEW_FOV;
    option[CONFIG_VIEW_DP]           = DEFAULT_VIEW_DP;
    option[CONFIG_VIEW_DC]           = DEFAULT_VIEW_DC;
    option[CONFIG_VIEW_DZ]           = DEFAULT_VIEW_DZ;

    strcpy(player, DEFAULT_PLAYER);
}

void config_load(void)
{
    char  path[MAXSTR];
    FILE *fp;

    if (config_home(path, USER_CONFIG_FILE, MAXSTR) && (fp = fopen(path, "r")))
    {
        char buf[MAXSTR];
        char key[MAXSTR];
        char val[MAXSTR];

        while (fgets(buf, MAXSTR, fp))
            if (sscanf(buf, "%s %s", key, val) == 2)
            {
                if      (strcmp(key, "fullscreen")            == 0)
                    option[CONFIG_FULLSCREEN]           = atoi(val);
                else if (strcmp(key, "width")                 == 0)
                    option[CONFIG_WIDTH]                = atoi(val);
                else if (strcmp(key, "height")                == 0)
                    option[CONFIG_HEIGHT]               = atoi(val);
                else if (strcmp(key, "stereo")                == 0)
                    option[CONFIG_STEREO]               = atoi(val);
                else if (strcmp(key, "camera")                == 0)
                    option[CONFIG_CAMERA]               = atoi(val);
                else if (strcmp(key, "textures")              == 0)
                    option[CONFIG_TEXTURES]             = atoi(val);
                else if (strcmp(key, "geometry")              == 0)
                    option[CONFIG_GEOMETRY]             = atoi(val);
                else if (strcmp(key, "reflection")            == 0)
                    option[CONFIG_REFLECTION]           = atoi(val);
                else if (strcmp(key, "shadow")                == 0)
                    option[CONFIG_SHADOW]               = atoi(val);
                else if (strcmp(key, "audio_rate")            == 0)
                    option[CONFIG_AUDIO_RATE]           = atoi(val);
                else if (strcmp(key, "audio_buff")            == 0)
                    option[CONFIG_AUDIO_BUFF]           = atoi(val);
                else if (strcmp(key, "mouse_sense")           == 0)
                    option[CONFIG_MOUSE_SENSE]          = atoi(val);
                else if (strcmp(key, "mouse_invert")          == 0)
                    option[CONFIG_MOUSE_INVERT]         = atoi(val);
                else if (strcmp(key, "nice")                  == 0)
                    option[CONFIG_NICE]                 = atoi(val);
                else if (strcmp(key, "fps")                   == 0)
                    option[CONFIG_FPS]                  = atoi(val);
                else if (strcmp(key, "sound_volume")          == 0)
                    option[CONFIG_SOUND_VOLUME]         = atoi(val);
                else if (strcmp(key, "music_volume")          == 0)
                    option[CONFIG_MUSIC_VOLUME]         = atoi(val);
                else if (strcmp(key, "joystick")              == 0)
                    option[CONFIG_JOYSTICK]             = atoi(val);
                else if (strcmp(key, "joystick_device")       == 0)
                    option[CONFIG_JOYSTICK_DEVICE]      = atoi(val);
                else if (strcmp(key, "joystick_axis_x")       == 0)
                    option[CONFIG_JOYSTICK_AXIS_X]      = atoi(val);
                else if (strcmp(key, "joystick_axis_y")       == 0)
                    option[CONFIG_JOYSTICK_AXIS_Y]      = atoi(val);
                else if (strcmp(key, "joystick_button_a")     == 0)
                    option[CONFIG_JOYSTICK_BUTTON_A]    = atoi(val);
                else if (strcmp(key, "joystick_button_b")     == 0)
                    option[CONFIG_JOYSTICK_BUTTON_B]    = atoi(val);
                else if (strcmp(key, "joystick_button_r")     == 0)
                    option[CONFIG_JOYSTICK_BUTTON_R]    = atoi(val);
                else if (strcmp(key, "joystick_button_l")     == 0)
                    option[CONFIG_JOYSTICK_BUTTON_L]    = atoi(val);
                else if (strcmp(key, "joystick_button_exit")  == 0)
                    option[CONFIG_JOYSTICK_BUTTON_EXIT] = atoi(val);
                else if (strcmp(key, "view_fov")              == 0)
                    option[CONFIG_VIEW_FOV]             = atoi(val);
                else if (strcmp(key, "view_dp")               == 0)
                    option[CONFIG_VIEW_DP]              = atoi(val);
                else if (strcmp(key, "view_dc")               == 0)
                    option[CONFIG_VIEW_DC]              = atoi(val);
                else if (strcmp(key, "view_dz")               == 0)
                    option[CONFIG_VIEW_DZ]              = atoi(val);
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
                    strncpy(player, val, MAXNAM);
            }

        fclose(fp);
    }
}

void config_save(void)
{
    char  path[MAXSTR];
    FILE *fp;

    if (config_home(path, USER_CONFIG_FILE, MAXSTR) && (fp = fopen(path, "w")))
    {
        fprintf(fp, "fullscreen           %d\n",
                option[CONFIG_FULLSCREEN]);
        fprintf(fp, "width                %d\n",
                option[CONFIG_WIDTH]);
        fprintf(fp, "height               %d\n",
                option[CONFIG_HEIGHT]);
        fprintf(fp, "stereo               %d\n",
                option[CONFIG_STEREO]);
        fprintf(fp, "camera               %d\n",
                option[CONFIG_CAMERA]);
        fprintf(fp, "textures             %d\n",
                option[CONFIG_TEXTURES]);
        fprintf(fp, "geometry             %d\n",
                option[CONFIG_GEOMETRY]);
        fprintf(fp, "reflection           %d\n",
                option[CONFIG_REFLECTION]);
        fprintf(fp, "shadow               %d\n",
                option[CONFIG_SHADOW]);
        fprintf(fp, "audio_rate           %d\n",
                option[CONFIG_AUDIO_RATE]);
        fprintf(fp, "audio_buff           %d\n",
                option[CONFIG_AUDIO_BUFF]);
        fprintf(fp, "mouse_sense          %d\n",
                option[CONFIG_MOUSE_SENSE]);
        fprintf(fp, "mouse_invert         %d\n",
                option[CONFIG_MOUSE_INVERT]);
        fprintf(fp, "nice                 %d\n",
                option[CONFIG_NICE]);
        fprintf(fp, "fps                  %d\n",
                option[CONFIG_FPS]);
        fprintf(fp, "sound_volume         %d\n",
                option[CONFIG_SOUND_VOLUME]);
        fprintf(fp, "music_volume         %d\n",
                option[CONFIG_MUSIC_VOLUME]);
        fprintf(fp, "joystick             %d\n",
                option[CONFIG_JOYSTICK]);
        fprintf(fp, "joystick_device      %d\n",
                option[CONFIG_JOYSTICK_DEVICE]);
        fprintf(fp, "joystick_axis_x      %d\n",
                option[CONFIG_JOYSTICK_AXIS_X]);
        fprintf(fp, "joystick_axis_y      %d\n",
                option[CONFIG_JOYSTICK_AXIS_Y]);
        fprintf(fp, "joystick_button_r    %d\n",
                option[CONFIG_JOYSTICK_BUTTON_R]);
        fprintf(fp, "joystick_button_l    %d\n",
                option[CONFIG_JOYSTICK_BUTTON_L]);
        fprintf(fp, "joystick_button_a    %d\n",
                option[CONFIG_JOYSTICK_BUTTON_A]);
        fprintf(fp, "joystick_button_b    %d\n",
                option[CONFIG_JOYSTICK_BUTTON_B]);
        fprintf(fp, "joystick_button_exit %d\n",
                option[CONFIG_JOYSTICK_BUTTON_EXIT]);
        fprintf(fp, "view_fov             %d\n",
                option[CONFIG_VIEW_FOV]);
        fprintf(fp, "view_dp              %d\n",
                option[CONFIG_VIEW_DP]);
        fprintf(fp, "view_dc              %d\n",
                option[CONFIG_VIEW_DC]);
        fprintf(fp, "view_dz              %d\n",
                option[CONFIG_VIEW_DZ]);
        fprintf(fp, "key_camera_1         %s\n",
                SDL_GetKeyName(option[CONFIG_KEY_CAMERA_1]));
        fprintf(fp, "key_camera_2         %s\n",
                SDL_GetKeyName(option[CONFIG_KEY_CAMERA_2]));
        fprintf(fp, "key_camera_3         %s\n",
                SDL_GetKeyName(option[CONFIG_KEY_CAMERA_3]));
        fprintf(fp, "key_camera_r         %s\n",
                SDL_GetKeyName(option[CONFIG_KEY_CAMERA_R]));
        fprintf(fp, "key_camrea_l         %s\n",
                SDL_GetKeyName(option[CONFIG_KEY_CAMERA_L]));
        fprintf(fp, "player               %s\n", player);

        fclose(fp);
    }
}

/*---------------------------------------------------------------------------*/

int config_mode(int f, int w, int h)
{
    if (SDL_SetVideoMode(w, h, 0, SDL_OPENGL | (f ? SDL_FULLSCREEN : 0)))
    {
        option[CONFIG_FULLSCREEN] = f;
        option[CONFIG_WIDTH]      = w;
        option[CONFIG_HEIGHT]     = h;
        option[CONFIG_SHADOW]     = (option[CONFIG_SHADOW] & glext_init());

        glViewport(0, 0, w, h);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        glEnable(GL_NORMALIZE);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);

        return 1;
    }
    return 0;
}

/*
 * Convert the given file name to  an absolute path name in the user's
 * home  directory.   If the  home  directory  cannot be  established,
 * return false.
 *
 * HACK: under Windows just assume the user has permission to write to
 * the data  directory.  This is  more reliable than trying  to devine
 * anything reasonable from the environment.
 */
int config_home(char *dst, char *src, size_t n)
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
int config_path(char *path, char *test)
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

/*---------------------------------------------------------------------------*/

void config_set(int i, int d)
{
    option[i] = d;
}

void config_tgl(int i)
{
    option[i] = (option[i] ? 0 : 1);
}

int config_tst(int i, int d)
{
    return (option[i] == d) ? 1 : 0;
}

int config_get(int i)
{
    return option[i];
}

void config_set_name(char *src)
{
    strncpy(player, src, MAXNAM);
}

void config_get_name(char *dst)
{
    strncpy(dst, player, MAXNAM);
}

/*---------------------------------------------------------------------------*/

void config_push_persp(float fov, float n, float f)
{
    GLdouble w = (GLdouble) option[CONFIG_WIDTH];
    GLdouble h = (GLdouble) option[CONFIG_HEIGHT];

    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();
        gluPerspective(fov, w / h, n, f);
    }
    glMatrixMode(GL_MODELVIEW);
}

void config_push_ortho(void)
{
    GLdouble w = (GLdouble) option[CONFIG_WIDTH];
    GLdouble h = (GLdouble) option[CONFIG_HEIGHT];

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
    if (option[CONFIG_REFLECTION])
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT);
    else
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);
}

/*---------------------------------------------------------------------------*/
