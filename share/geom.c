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
#include "solid.h"
#include "image.h"
#include "config.h"

#define PI 3.1415926535897932

/*---------------------------------------------------------------------------*/

static GLUquadric  *ball_quad = NULL;
static GLuint       ball_list = 0;
static GLuint       ball_text = 0;

void ball_init(int b)
{
    static const float  s[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    static const float  e[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    static const float  h[1] = { 64.0f };

    ball_text = make_image_from_file(NULL, NULL, IMG_BALL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if ((ball_quad = gluNewQuadric()))
    {
        int slices = b ? 64 : 16;
        int stacks = b ? 32 :  8;

        gluQuadricOrientation(ball_quad, GLU_OUTSIDE);
        gluQuadricNormals(ball_quad, GLU_SMOOTH);
        gluQuadricTexture(ball_quad, GL_TRUE);

        ball_list = glGenLists(1);
    
        glNewList(ball_list, GL_COMPILE);
        {
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  s);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  e);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, h);

            glBindTexture(GL_TEXTURE_2D, ball_text);

            gluSphere(ball_quad, 1.0, slices, stacks);
        }
    }
    glEndList();
}

void ball_free(void)
{
    if (glIsList(ball_list))
        glDeleteLists(ball_list, 1);

    gluDeleteQuadric(ball_quad);

    if (glIsTexture(ball_text))
        glDeleteTextures(1, &ball_text);
}

void ball_draw(void)
{
    glPushAttrib(GL_POLYGON_BIT);
    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    {
        glEnable(GL_COLOR_MATERIAL);

        glBindTexture(GL_TEXTURE_2D, ball_text);

        /* Render the ball back to front in case it is translucent. */

        glDepthMask(GL_FALSE);

        glCullFace(GL_FRONT);
        glCallList(ball_list);
        glCullFace(GL_BACK);
        glCallList(ball_list);

        /* Render the ball into the depth buffer. */

        glDepthMask(GL_TRUE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        {
            glCallList(ball_list);
        }
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        /* Ensure the ball is visible even when obscured by geometry. */

        glDisable(GL_DEPTH_TEST);

        glColor4f(1.0f, 1.0f, 1.0f, 0.1f);
        glCallList(ball_list);
    }
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

static GLUquadric *mark_quad = NULL;
static GLuint      mark_list;

void mark_init(int b)
{
    if ((mark_quad = gluNewQuadric()))
    {
        int slices = b ? 32 : 16;

        gluQuadricOrientation(mark_quad, GLU_OUTSIDE);
        gluQuadricTexture(mark_quad, GL_TRUE);

        mark_list = glGenLists(1);
    
        glNewList(mark_list, GL_COMPILE);
        {
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            gluDisk(mark_quad, 0.0, 1.0, slices, 1);
        }
        glEndList();
    }
}

void mark_draw(void)
{
    glPushAttrib(GL_TEXTURE_BIT);
    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    {
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_TEXTURE_2D);
        glDepthMask(GL_FALSE);

        glCallList(mark_list);
    }
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
}

void mark_free(void)
{
    if (glIsList(mark_list))
        glDeleteLists(mark_list, 1);

    gluDeleteQuadric(mark_quad);
}

/*---------------------------------------------------------------------------*/

static GLuint coin_text = 0;
static GLuint coin_list = 0;

static void coin_face(int n, double radius, double thick)
{
    int i;

    glBegin(GL_TRIANGLE_FAN);
    {
        glNormal3d(0.0, 0.0, +1.0);

        for (i = 0; i < n; i++)
        {
            double x = cos(2.0 * PI * i / n);
            double y = sin(2.0 * PI * i / n);

            glTexCoord2d(-x * 0.5 + 0.5, +y * 0.5 + 0.5);
            glVertex3d(radius * x, radius * y, thick);
        }
    }
    glEnd();
}

static void coin_edge(int n, double radius, double thick)
{
    int i;

    glBegin(GL_QUAD_STRIP);
    {
        for (i = 0; i <= n; i++)
        {
            double x = cos(2.0 * PI * i / n);
            double y = sin(2.0 * PI * i / n);

            glNormal3d(x, y, 0.0);
            glVertex3d(radius * x, radius * y, +thick);
            glVertex3d(radius * x, radius * y, -thick);
        }
    }
    glEnd();
}

void coin_color(double *c, int n)
{
    if (n >= 1)
    {
        c[0] = 1.0;
        c[1] = 1.0;
        c[2] = 0.2;
    }
    if (n >= 5)
    {
        c[0] = 1.0;
        c[1] = 0.2;
        c[2] = 0.2;
    }
    if (n >= 10)
    {
        c[0] = 0.2;
        c[1] = 0.2;
        c[2] = 1.0;
    }
}

void coin_init(int b)
{
    static const float  a[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    static const float  s[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    static const float  e[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    static const float  h[1] = { 32.0f };

    int n = b ? 32 : 8;

    coin_text = make_image_from_file(NULL, NULL, IMG_COIN);
    coin_list = glGenLists(1);

    glNewList(coin_list, GL_COMPILE);
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   a);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  s);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  e);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, h);

        glPushMatrix();
        {
            coin_edge(n, COIN_RADIUS, COIN_THICK);
            coin_face(n, COIN_RADIUS, COIN_THICK);

            glRotated(180.0, 0.0, 1.0, 0.0);
            coin_face(n, COIN_RADIUS, COIN_THICK);
        }
        glPopMatrix();
    }
    glEndList();
}

void coin_free(void)
{
    if (glIsList(coin_list))
        glDeleteLists(coin_list, 1);

    if (glIsTexture(coin_text))
        glDeleteTextures(1, &coin_text);
}

void coin_draw(int n)
{
    double c[3], r = 360.0 * SDL_GetTicks() / 1000.0;

    glPushMatrix();
    glPushAttrib(GL_LIGHTING_BIT);
    {
        glEnable(GL_COLOR_MATERIAL);
        glBindTexture(GL_TEXTURE_2D, coin_text);

        coin_color(c, n);

        glRotated(r, 0.0, 1.0, 0.0);
        glColor3dv(c);
        glCallList(coin_list);
    }
    glPopAttrib();
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

static GLuint goal_list = 0;

void goal_init(int b)
{
    int i, n = b ? 32 : 8;

    goal_list = glGenLists(1);

    glNewList(goal_list, GL_COMPILE);
    {
        glBegin(GL_QUAD_STRIP);
        {
            for (i = 0; i <= n; i++)
            {
                double x = cos(2.0 * PI * i / n);
                double y = sin(2.0 * PI * i / n);
            
                glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
                glVertex3d(x, 0.0, y);
                glColor4f(1.0f, 1.0f, 0.0f, 0.0f);
                glVertex3d(x, GOAL_HEIGHT, y);
            }
        }
        glEnd();
    }
    glEndList();
}

void goal_free(void)
{
    if (glIsList(goal_list))
        glDeleteLists(goal_list, 1);
}

void goal_draw(void)
{
    glPushAttrib(GL_TEXTURE_BIT);
    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    {
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDepthMask(GL_FALSE);

        glCallList(goal_list);
    }
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

static GLuint jump_list = 0;

void jump_init(int b)
{
    int i, n = b ? 32 : 8;

    jump_list = glGenLists(1);

    glNewList(jump_list, GL_COMPILE);
    {
        glBegin(GL_QUAD_STRIP);
        {
            for (i = 0; i <= n; i++)
            {
                double x = cos(2.0 * PI * i / n);
                double y = sin(2.0 * PI * i / n);
            
                glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
                glVertex3d(x, 0.0, y);
                glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
                glVertex3d(x, JUMP_HEIGHT, y);
            }
        }
        glEnd();
    }
    glEndList();
}

void jump_free(void)
{
    if (glIsList(jump_list))
        glDeleteLists(jump_list, 1);
}

void jump_draw(void)
{
    glPushAttrib(GL_TEXTURE_BIT);
    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    {
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDepthMask(GL_FALSE);

        glCallList(jump_list);
    }
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

static GLuint swch_list = 0;

void swch_init(int b)
{
    int i, n = b ? 32 : 8;

    swch_list = glGenLists(2);

    glNewList(swch_list, GL_COMPILE);
    {
        glBegin(GL_QUAD_STRIP);
        {
            for (i = 0; i <= n; i++)
            {
                double x = cos(2.0 * PI * i / n);
                double y = sin(2.0 * PI * i / n);
            
                glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
                glVertex3d(x, 0.0, y);
                glColor4f(1.0f, 0.0f, 0.0f, 0.0f);
                glVertex3d(x, SWCH_HEIGHT, y);
            }
        }
        glEnd();
    }
    glEndList();

    glNewList(swch_list + 1, GL_COMPILE);
    {
        glBegin(GL_QUAD_STRIP);
        {
            for (i = 0; i <= n; i++)
            {
                double x = cos(2.0 * PI * i / n);
                double y = sin(2.0 * PI * i / n);
            
                glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
                glVertex3d(x, 0.0, y);
                glColor4f(0.0f, 1.0f, 0.0f, 0.0f);
                glVertex3d(x, SWCH_HEIGHT, y);
            }
        }
        glEnd();
    }
    glEndList();
}

void swch_free(void)
{
    if (glIsList(swch_list))
        glDeleteLists(swch_list, 1);
}

void swch_draw(int b)
{
    glPushAttrib(GL_TEXTURE_BIT);
    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    {
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDepthMask(GL_FALSE);

        if (b)
            glCallList(swch_list + 1);
        else
            glCallList(swch_list);
    }
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

static GLuint flag_list = 0;

void flag_init(int b)
{
    int i, n = b ? 8 : 4;

    flag_list = glGenLists(1);

    glNewList(flag_list, GL_COMPILE);
    {
        glBegin(GL_QUAD_STRIP);
        {
            for (i = 0; i <= n; i++)
            {
                double x = cos(2.0 * PI * i / n) * 0.01;
                double y = sin(2.0 * PI * i / n) * 0.01;
            
                glColor3f(1.0f, 1.0f, 1.0f);
                glVertex3d(x, 0.0,         y);
                glVertex3d(x, GOAL_HEIGHT, y);
            }
        }
        glEnd();

        glBegin(GL_TRIANGLES);
        {
            glColor3f(1.0f, 0.0f, 0.0f);

            glVertex3d(              0.0, GOAL_HEIGHT,        0.0);
            glVertex3d(GOAL_HEIGHT * 0.2, GOAL_HEIGHT * 0.90, 0.0);
            glVertex3d(              0.0, GOAL_HEIGHT * 0.80, 0.0);
        }
        glEnd();
    }
    glEndList();
}

void flag_free(void)
{
    if (glIsList(flag_list))
        glDeleteLists(flag_list, 1);
}

void flag_draw(void)
{
    glPushAttrib(GL_TEXTURE_BIT);
    glPushAttrib(GL_LIGHTING_BIT);
    {
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        glCallList(flag_list);
    }
    glPopAttrib();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/
