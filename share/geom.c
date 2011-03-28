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
#include <math.h>

#include "glext.h"
#include "geom.h"
#include "part.h"
#include "vec3.h"
#include "image.h"
#include "config.h"
#include "video.h"

#include "solid_draw.h"

#define PI 3.1415926535897932

/*---------------------------------------------------------------------------*/

static struct s_full beam;
static struct s_full flag;

void geom_init(void)
{
    sol_load_full(&beam, "geom/beam/beam.sol", 0);
    sol_load_full(&flag, "geom/flag/flag.sol", 0);
}

void geom_free(void)
{
    sol_free_full(&flag);
    sol_free_full(&beam);
}

/*---------------------------------------------------------------------------*/

void goal_draw(void)
{
    glPushMatrix();
    {
        glScalef(1.0f, 3.0f, 1.0f);

        glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
        sol_draw(&beam.draw, 1, 1);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    glPopMatrix();
}

void jump_draw(int highlight)
{
    static GLfloat jump_colors[4][4] = {
        { 0.75f, 0.5f, 1.0f, 0.5f },
        { 0.75f, 0.5f, 1.0f, 0.8f },
    };

    glPushMatrix();
    {
        glScalef(1.0f, 2.0f, 1.0f);

        glColor4fv(jump_colors[highlight]);
        sol_draw(&beam.draw, 1, 1);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    glPopMatrix();
}

void swch_draw(int b, int e)
{
    static GLfloat swch_colors[4][4] = {
        { 1.0f, 0.0f, 0.0f, 0.5f }, /* red out */
        { 1.0f, 0.0f, 0.0f, 0.8f }, /* red in */
        { 0.0f, 1.0f, 0.0f, 0.5f }, /* green out */
        { 0.0f, 1.0f, 0.0f, 0.8f }, /* green in */
    };

    glPushMatrix();
    {
        glScalef(1.0f, 2.0f, 1.0f);

        glColor4fv(swch_colors[b * 2 + e]);
        sol_draw(&beam.draw, 1, 1);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    glPopMatrix();
}

void flag_draw(void)
{
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    sol_draw(&flag.draw, 1, 1);
}

/*---------------------------------------------------------------------------*/

static GLuint mark_list;

void mark_init(void)
{
    int i, slices = 32;

    mark_list = glGenLists(1);

    glNewList(mark_list, GL_COMPILE);
    {
        glBegin(GL_TRIANGLE_FAN);
        {
            glNormal3f(0.f, 1.f, 0.f);

            for (i = 0; i < slices; i++)
            {
                float x = fcosf(-2.f * PI * i / slices);
                float y = fsinf(-2.f * PI * i / slices);

                glVertex3f(x, 0, y);
            }
        }
        glEnd();
    }
    glEndList();
}

void mark_draw(void)
{
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_FALSE);
    {
        glCallList(mark_list);
    }
    glDepthMask(GL_TRUE);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_COLOR_MATERIAL);
}

void mark_free(void)
{
    if (glIsList(mark_list))
        glDeleteLists(mark_list, 1);

    mark_list = 0;
}

/*---------------------------------------------------------------------------*/
/*
static GLuint flag_list;

void flag_init(void)
{
    int i, n = 8;

    flag_list = glGenLists(1);

    glNewList(flag_list, GL_COMPILE);
    {
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        {
            glBegin(GL_TRIANGLES);
            {
                glColor3f(1.0f, 0.0f, 0.0f);

                glVertex3f(              0.0f, GOAL_HEIGHT,        0.0f);
                glVertex3f(GOAL_HEIGHT * 0.2f, GOAL_HEIGHT * 0.9f, 0.0f);
                glVertex3f(              0.0f, GOAL_HEIGHT * 0.8f, 0.0f);

                glVertex3f(              0.0f, GOAL_HEIGHT,        0.0f);
                glVertex3f(              0.0f, GOAL_HEIGHT * 0.8f, 0.0f);
                glVertex3f(GOAL_HEIGHT * 0.2f, GOAL_HEIGHT * 0.9f, 0.0f);
            }
            glEnd();

            glBegin(GL_QUAD_STRIP);
            {
                for (i = 0; i <= n; i++)
                {
                    float x = fcosf(2.f * PI * i / n) * 0.01f;
                    float y = fsinf(2.f * PI * i / n) * 0.01f;

                    glColor3f(1.0f, 1.0f, 1.0f);
                    glVertex3f(x, 0.0f,        y);
                    glVertex3f(x, GOAL_HEIGHT, y);
                }
            }
            glEnd();
        }
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        glDisable(GL_COLOR_MATERIAL);
    }
    glEndList();
}

void flag_free(void)
{
    if (glIsList(flag_list))
        glDeleteLists(flag_list, 1);

    flag_list = 0;
}

void flag_draw(void)
{
    glCallList(flag_list);
}
*/
/*---------------------------------------------------------------------------*/

static GLuint clip_text;

static GLubyte clip_data[] = { 0xff, 0xff, 0x0, 0x0 };

void clip_init(void)
{
    if (!glActiveTexture_)
        return;

    glActiveTexture_(GL_TEXTURE1);
    {
        glGenTextures(1, &clip_text);
        glBindTexture(GL_TEXTURE_1D, clip_text);

        glTexImage1D(GL_TEXTURE_1D, 0,
                     GL_LUMINANCE_ALPHA, 2, 0,
                     GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, clip_data);

        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    }
    glActiveTexture_(GL_TEXTURE0);
}

void clip_free(void)
{
    if (glIsTexture(clip_text))
        glDeleteTextures(1, &clip_text);
}

void clip_draw_set(void)
{
    if (!glActiveTexture_)
        return;

    glActiveTexture_(GL_TEXTURE1);
    {
        glBindTexture(GL_TEXTURE_1D, clip_text);

        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_1D);
    }
    glActiveTexture_(GL_TEXTURE0);
}

void clip_draw_clr(void)
{
    if (!glActiveTexture_)
        return;

    glActiveTexture_(GL_TEXTURE1);
    {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_1D);
    }
    glActiveTexture_(GL_TEXTURE0);
}

/*---------------------------------------------------------------------------*/

/*
 * A note about lighting and shadow: technically speaking, it's wrong.
 * The  light  position  and   shadow  projection  behave  as  if  the
 * light-source rotates with the  floor.  However, the skybox does not
 * rotate, thus the light should also remain stationary.
 *
 * The  correct behavior  would eliminate  a significant  3D  cue: the
 * shadow of  the ball indicates  the ball's position relative  to the
 * floor even  when the ball is  in the air.  This  was the motivating
 * idea  behind the  shadow  in  the first  place,  so correct  shadow
 * projection would only magnify the problem.
 */

static GLuint shad_text;

void shad_init(void)
{
    shad_text = make_image_from_file(IMG_SHAD);

    if (config_get_d(CONFIG_SHADOW) == 2)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }

#if 0
    clip_init();
#endif
}

void shad_free(void)
{
    if (glIsTexture(shad_text))
        glDeleteTextures(1, &shad_text);
#if 0
    clip_free();
#endif
}

void shad_draw_set(void)
{
    glActiveTexture_(GL_TEXTURE1);
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, shad_text);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    }
    glActiveTexture_(GL_TEXTURE0);


#if 0
    glMatrixMode(GL_TEXTURE);
    {
        glLoadIdentity();
    }
    glMatrixMode(GL_MODELVIEW);

    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);

    clip_draw_set();
#endif
}

void shad_draw_clr(void)
{
    glActiveTexture_(GL_TEXTURE1);
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }
    glActiveTexture_(GL_TEXTURE0);

#if 0
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);

    clip_draw_clr();
#endif
}

/*---------------------------------------------------------------------------*/

void fade_draw(float k)
{
    if (k > 0.0f)
    {
        int w = config_get_d(CONFIG_WIDTH);
        int h = config_get_d(CONFIG_HEIGHT);

        video_push_ortho();
        {
            glEnable(GL_COLOR_MATERIAL);
            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_TEXTURE_2D);

            glColor4f(0.0f, 0.0f, 0.0f, k);

            glBegin(GL_QUADS);
            {
                glVertex2i(0, 0);
                glVertex2i(w, 0);
                glVertex2i(w, h);
                glVertex2i(0, h);
            }
            glEnd();

            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

            glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_LIGHTING);
            glDisable(GL_COLOR_MATERIAL);
        }
        video_pop_matrix();
    }
}

/*---------------------------------------------------------------------------*/
