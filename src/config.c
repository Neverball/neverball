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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "gl.h"
#include "config.h"
#include "level.h"
#include "geom.h"
#include "text.h"
#include "hud.h"

/*---------------------------------------------------------------------------*/

static int mode         = SDL_OPENGL;
static int width        = CONFIG_DEF_WIDTH;
static int height       = CONFIG_DEF_HEIGHT;
static int textures     = CONFIG_DEF_TEXTURES;
static int geometry     = CONFIG_DEF_GEOMETRY;
static int audio_rate   = CONFIG_DEF_AUDIO_RATE;
static int audio_buff   = CONFIG_DEF_AUDIO_BUFF;
static int mouse_sense  = CONFIG_DEF_MOUSE_SENSE;
static int nice         = CONFIG_DEF_NICE;
static int fps          = CONFIG_DEF_FPS;
static int joy          = CONFIG_DEF_JOY;

static int axis_x       = CONFIG_DEF_AXIS_X;
static int axis_y       = CONFIG_DEF_AXIS_Y;
static int button_a     = CONFIG_DEF_BUTTON_A;
static int button_b     = CONFIG_DEF_BUTTON_B;
static int button_r     = CONFIG_DEF_BUTTON_R;
static int button_l     = CONFIG_DEF_BUTTON_L;
static int button_pause = CONFIG_DEF_BUTTON_PAUSE;

/*---------------------------------------------------------------------------*/

/*
 * Convert the given file name to  an absolute path name in the user's
 * home  directory.   If the  home  directory  cannot be  established,
 * return false.
 */
int config_home(char *dst, const char *src)
{
    char *vol;
    char *dir;

    if ((dir = getenv("HOME")))
    {
        strcpy(dst, dir);
        strcat(dst, "/");
        strcat(dst, src);
        return 1;
    }

    if ((vol = getenv("HOMEDRIVE")) && (dir = getenv("HOMEPATH")))
    {
        strcpy(dst, vol);
        strcat(dst, dir);
        strcat(dst, "\\");
        strcat(dst, src);
        return 1;
    }

    return 0;
}

/*
 * Game  assets are  accessed  via relative  paths.   Set the  current
 * directory to the root of the asset hierarchy.  Confirm the location
 * by checking for the presence of the level catalog.
 */
int config_path(void)
{
    FILE *fp;

    chdir(CONFIG_PATH);

    if ((fp = fopen(LEVEL_FILE, "r")))
    {
        fclose(fp);
        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void config_load(void)
{
    char  path[STRMAX];
    FILE *fp;

    if (config_home(path, CONFIG_FILE) && (fp = fopen(path, "r")))
    {
        char key[STRMAX];
        int  val;

        while (fscanf(fp, "%s %d", key, &val) == 2)
        {
            if (strcmp(key, "fullscreen") == 0)
                mode = SDL_OPENGL | (val ? SDL_FULLSCREEN : 0);

            if (strcmp(key, "width")        == 0) width        = val;
            if (strcmp(key, "height")       == 0) height       = val;
            if (strcmp(key, "textures")     == 0) textures     = val;
            if (strcmp(key, "geometry")     == 0) geometry     = val;
            if (strcmp(key, "audio_rate")   == 0) audio_rate   = val;
            if (strcmp(key, "audio_buff")   == 0) audio_buff   = val;
            if (strcmp(key, "mouse_sense")  == 0) mouse_sense  = val;
            if (strcmp(key, "nice")         == 0) nice         = val;
            if (strcmp(key, "fps")          == 0) fps          = val;
            if (strcmp(key, "joy")          == 0) joy          = val;

            if (strcmp(key, "axis_x")       == 0) axis_x       = val;
            if (strcmp(key, "axis_y")       == 0) axis_y       = val;
            if (strcmp(key, "button_a")     == 0) button_a     = val;
            if (strcmp(key, "button_b")     == 0) button_b     = val;
            if (strcmp(key, "button_r")     == 0) button_r     = val;
            if (strcmp(key, "button_l")     == 0) button_l     = val;
            if (strcmp(key, "button_pause") == 0) button_pause = val;
        }

        fclose(fp);
    }
}

void config_store(void)
{
    char  path[STRMAX];
    FILE *fp;

    if (config_home(path, CONFIG_FILE) && (fp = fopen(path, "w")))
    {
        fprintf(fp, "fullscreen %d\n",  (mode & SDL_FULLSCREEN) ? 1 : 0);
        fprintf(fp, "width %d\n",        width);
        fprintf(fp, "height %d\n",       height);
        fprintf(fp, "textures %d\n",     textures);
        fprintf(fp, "geometry %d\n",     geometry);
        fprintf(fp, "audio_rate %d\n",   audio_rate);
        fprintf(fp, "audio_buff %d\n",   audio_buff);
        fprintf(fp, "mouse_sense %d\n",  mouse_sense);
        fprintf(fp, "nice %d\n",         nice);
        fprintf(fp, "fps %d\n",          fps);
        fprintf(fp, "joy %d\n",          joy);

        fprintf(fp, "axis_x %d\n",       axis_x);
        fprintf(fp, "axis_y %d\n",       axis_y);
        fprintf(fp, "button_r %d\n",     button_r);
        fprintf(fp, "button_l %d\n",     button_l);
        fprintf(fp, "button_a %d\n",     button_a);
        fprintf(fp, "button_b %d\n",     button_b);
        fprintf(fp, "button_pause %d\n", button_pause);

        fclose(fp);
    }
}

/*---------------------------------------------------------------------------*/

int config_mode(void) { return mode; }
int config_w   (void) { return width;  }
int config_h   (void) { return height; }
int config_text(void) { return textures; }
int config_geom(void) { return geometry; }
int config_rate(void) { return audio_rate; }
int config_buff(void) { return audio_buff; }
int config_sens(void) { return mouse_sense; }
int config_nice(void) { return nice; }
int config_fps (void) { return fps; }

int config_axis_x(int a)   { return (joy && a == axis_x); }
int config_axis_y(int a)   { return (joy && a == axis_y); }
int config_button_a(int b) { return (joy && b == button_a); }
int config_button_b(int b) { return (joy && b == button_b); }
int config_button_r(int b) { return (joy && b == button_r); }
int config_button_l(int b) { return (joy && b == button_l); }
int config_button_P(int b) { return (joy && b == button_pause); }

/*---------------------------------------------------------------------------*/

int config_set_mode(int w, int h, int m)
{
    if (SDL_SetVideoMode(w, h, 0, m))
    {
        hud_free();
        goal_free();
        coin_free();
        ball_free();
        text_free();

        mode   = m;
        width  = w;
        height = h;

        glViewport(0, 0, w, h);
        glClearColor(0.0f, 0.8f, 0.0f, 0.0f);

        glEnable(GL_NORMALIZE);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        text_init(h);
        ball_init(geometry);
        coin_init(geometry);
        goal_init(geometry);
        hud_init();

        return 1;
    }
    return 0;
}

int config_set_text(int t)
{
    textures = t;

    return 1;
}

int config_set_geom(int g)
{
    goal_free();
    coin_free();
    ball_free();

    geometry = g;

    ball_init(g);
    coin_init(g);
    goal_init(g);

    return 1;
}

int config_set_audio(int r, int b)
{
    audio_free();

    audio_rate = r;
    audio_buff = b;

    return audio_init(r, b);
}

void config_tog_nice(void)
{
    nice = nice ? 0 : 1;
}

void config_tog_fps(void)
{
    fps = fps ? 0 : 1;
}

/*---------------------------------------------------------------------------*/

void config_push_persp(double fov, double n, double f)
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
