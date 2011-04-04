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

/*---------------------------------------------------------------------------*/

static struct s_full beam;
static struct s_full jump;
static struct s_full goal;
static struct s_full flag;
static struct s_full mark;
static struct s_full vect;
static struct s_full back;

static int back_state = 0;

/*---------------------------------------------------------------------------*/

void geom_init(void)
{
    sol_load_full(&beam, "geom/beam/beam.sol", 0);
    sol_load_full(&jump, "geom/jump/jump.sol", 0);
    sol_load_full(&goal, "geom/goal/goal.sol", 0);
    sol_load_full(&flag, "geom/flag/flag.sol", 0);
    sol_load_full(&mark, "geom/mark/mark.sol", 0);
    sol_load_full(&vect, "geom/vect/vect.sol", 0);
}

void geom_free(void)
{
    sol_free_full(&vect);
    sol_free_full(&mark);
    sol_free_full(&flag);
    sol_free_full(&goal);
    sol_free_full(&jump);
    sol_free_full(&beam);
}

/*---------------------------------------------------------------------------*/

void back_init(const char *name)
{
    if (back_state)
        back_free();

    /* Load the background SOL and modify its material in-place to use the   */
    /* named gradient texture.                                               */

    sol_load_full(&back, "geom/back/back.sol", 0);
    back.draw.mv[0].o = make_image_from_file(name);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    back_state = 1;
}

void back_free(void)
{
    if (back_state)
        sol_free_full(&back);
    
    back_state = 0;
}

/*---------------------------------------------------------------------------*/

static const struct d_mtrl *jump_part_draw(const struct d_mtrl *mq,
                                           GLfloat s, GLfloat a)
{
    glMatrixMode(GL_TEXTURE);
    glTranslatef(s, 0.0f, 0.0f);
    glMatrixMode(GL_MODELVIEW);

    glRotatef(a, 0.0f, 1.0f, 0.0f);
    mq = sol_draw(&jump.draw, mq, 1, 1);
    glScalef(0.9f, 0.9f, 0.9f);

    return mq;
}

static const struct d_mtrl *goal_part_draw(const struct d_mtrl *mq, GLfloat s)
{
    glMatrixMode(GL_TEXTURE);
    glTranslatef(0.0f, -s, 0.0f);
    glMatrixMode(GL_MODELVIEW);

    mq = sol_draw(&goal.draw, mq, 1, 1);
    glScalef(0.8f, 1.1f, 0.8f);

    return mq;
}

/*---------------------------------------------------------------------------*/

const struct d_mtrl *goal_draw(const struct d_mtrl *mq, float t)
{
    glPushMatrix();
    {
        glScalef(1.0f, 3.0f, 1.0f);
        glColor4f(1.0f, 1.0f, 0.0f, 0.5f);

        mq = sol_draw(&beam.draw, mq, 1, 1);

        mq = goal_part_draw(mq, t * 0.10f);
        mq = goal_part_draw(mq, t * 0.10f);
        mq = goal_part_draw(mq, t * 0.10f);
        mq = goal_part_draw(mq, t * 0.10f);

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    glPopMatrix();

    return mq;
}

const struct d_mtrl *jump_draw(const struct d_mtrl *mq, float t, int h)
{
    static GLfloat jump_colors[4][4] = {
        { 0.75f, 0.5f, 1.0f, 0.5f },
        { 0.75f, 0.5f, 1.0f, 0.8f },
    };

    glPushMatrix();
    {
        glColor4fv(jump_colors[h]);

        glScalef(1.0f, 2.0f, 1.0f);

        mq = sol_draw(&beam.draw, mq, 1, 1);

        mq = jump_part_draw(mq, t * 0.15f, t * 360.0f);
        mq = jump_part_draw(mq, t * 0.20f, t * 360.0f);
        mq = jump_part_draw(mq, t * 0.25f, t * 360.0f);

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    glPopMatrix();

    return mq;
}

const struct d_mtrl *swch_draw(const struct d_mtrl *mq, int b, int e)
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
        mq = sol_draw(&beam.draw, mq, 1, 1);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    glPopMatrix();

    return mq;
}

const struct d_mtrl *flag_draw(const struct d_mtrl *mq)
{
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    return sol_draw(&flag.draw, mq, 1, 1);
}

const struct d_mtrl *mark_draw(const struct d_mtrl *mq)
{
    return sol_draw(&mark.draw, mq, 1, 1);
}

const struct d_mtrl *vect_draw(const struct d_mtrl *mq)
{
    mq = sol_draw(&vect.draw, mq, 0, 1);
    mq = sol_draw(&vect.draw, mq, 0, 0);
    return mq;
}

const struct d_mtrl *back_draw(const struct d_mtrl *mq, float t)
{
    glPushMatrix();
    {
        GLfloat dx =  60.0f * fsinf(t / 10.0f);
        GLfloat dz = 180.0f * fsinf(t / 12.0f);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDepthMask(GL_FALSE);
        {
            glScalef(-BACK_DIST, BACK_DIST, -BACK_DIST);
            if (t) glRotatef(dz, 0.0f, 0.0f, 1.0f);
            if (t) glRotatef(dx, 1.0f, 0.0f, 0.0f);

            mq = sol_draw(&back.draw, mq, 1, 1);
        }
        glDepthMask(GL_TRUE);
        glEnable(GL_LIGHTING);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
    }
    glPopMatrix();

    return mq;
}

void back_draw_easy(void)
{
    sol_draw_disable(back_draw(sol_draw_enable(), 0.0f));
}

/*---------------------------------------------------------------------------*/
/*
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

        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
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
*/
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

void shad_free(void)
{
    if (glIsTexture(shad_text))
        glDeleteTextures(1, &shad_text);
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
}

void shad_draw_clr(void)
{
    glActiveTexture_(GL_TEXTURE1);
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }
    glActiveTexture_(GL_TEXTURE0);
}

/*---------------------------------------------------------------------------*/
