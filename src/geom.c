/*   
 *   Copyright (C) 2003 Robert Kooima
 *
 *   SUPER EMPTY BALL is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by the
 *   Free Software Foundation;  either version 2 of the  License, or (at your
 *   option) any later version.
 *
 *   This program  is distributed  in the  hope that it  will be  useful, but
 *   WITHOUT   ANY   WARRANTY;  without   even   the   implied  warranty   of
 *   MERCHANTABILITY  or  FITNESS FOR  A  PARTICULAR  PURPOSE.   See the  GNU
 *   General Public License for more details.
 */

#include <math.h>

#include "gl.h"
#include "main.h"
#include "vec3.h"
#include "solid.h"
#include "state.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

#define CR 0.15
#define DZ 0.01
#define PI 3.1415926

#define COIN_S "data/png/coin.png"

static struct image coin_i, *coin_p = &coin_i;

/*---------------------------------------------------------------------------*/

static void section(int d,
                    const double p0[3], const double c0[4],
                    const double p1[3], const double c1[4],
                    const double p2[3], const double c2[4])
{
    if (d == 0)
    {
        glColor4dv(c0);
        glNormal3dv(p0);
        glVertex3dv(p0);

        glColor4dv(c1);
        glNormal3dv(p1);
        glVertex3dv(p1);

        glColor4dv(c2);
        glNormal3dv(p2);
        glVertex3dv(p2);
    }
    else
    {
        double p01[3], p12[3], p20[3];
        double c01[4], c12[4], c20[4];

        v_mid(p01, p0, p1);
        v_mid(p12, p1, p2);
        v_mid(p20, p2, p0);

        v_nrm(p01, p01);
        v_nrm(p12, p12);
        v_nrm(p20, p20);

        c01[0] = (c0[0] + c1[0]) / 2.0;
        c01[1] = (c0[1] + c1[1]) / 2.0;
        c01[2] = (c0[2] + c1[2]) / 2.0;
        c01[3] = (c0[3] + c1[3]) / 2.0;

        c12[0] = (c1[0] + c2[0]) / 2.0;
        c12[1] = (c1[1] + c2[1]) / 2.0;
        c12[2] = (c1[2] + c2[2]) / 2.0;
        c12[3] = (c1[3] + c2[3]) / 2.0;

        c20[0] = (c2[0] + c0[0]) / 2.0;
        c20[1] = (c2[1] + c0[1]) / 2.0;
        c20[2] = (c2[2] + c0[2]) / 2.0;
        c20[3] = (c2[3] + c0[3]) / 2.0;

        section(d - 1, p0,  c0,  p01, c01, p20, c20);
        section(d - 1, p01, c01, p1,  c1,  p12, c12);
        section(d - 1, p20, c20, p12, c12, p2,  c2);
        section(d - 1, p01, c01, p12, c12, p20, c20);
    }
}

GLuint ball_init(int d)
{
    static const float s[3] = { 1.0f, 1.0f, 1.0f };

    static const double p[6][3] = {
        { +1.0,  0.0,  0.0 },
        { -1.0,  0.0,  0.0 },
        {  0.0, +1.0,  0.0 },
        {  0.0, -1.0,  0.0 },
        {  0.0,  0.0, +1.0 },
        {  0.0,  0.0, -1.0 },
    };
    static const double c[2][4] = {
        { 0.0, 0.0, 0.0, 0.5 },
        { 1.0, 1.0, 1.0, 0.5 },
    };

    GLuint list = glGenLists(1);

    glNewList(list, GL_COMPILE);
    glPushAttrib(GL_LIGHTING_BIT);
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  s);
        glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 64.0f);

        glEnable(GL_COLOR_MATERIAL);

        glBegin(GL_TRIANGLES);
        {
            section(d, p[4], c[0], p[0], c[0], p[2], c[0]);
            section(d, p[0], c[1], p[5], c[1], p[2], c[1]);
            section(d, p[5], c[0], p[1], c[0], p[2], c[0]);
            section(d, p[1], c[1], p[4], c[1], p[2], c[1]);

            section(d, p[0], c[1], p[4], c[1], p[3], c[1]);
            section(d, p[5], c[0], p[0], c[0], p[3], c[0]);
            section(d, p[1], c[1], p[5], c[1], p[3], c[1]);
            section(d, p[4], c[0], p[1], c[0], p[3], c[0]);
        }
        glEnd();
    }
    glPopAttrib();
    glEndList();

    return list;
}

void ball_draw(GLuint list, double r,
               const double p[3],
               const double e[3][3])
{
    double M[16];

    m_basis(M, e[0], e[1], e[2]);

    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    {
        glDepthMask(GL_FALSE);

        glDisable(GL_TEXTURE_2D);
        glPushMatrix();
        {
            glTranslated(p[0], p[1], p[2]);
            glMultMatrixd(M);
            glScaled(r, r, r);

            glCullFace(GL_FRONT);
            glCallList(list);
            glCullFace(GL_BACK);
            glCallList(list);
        }
        glPopMatrix();
        glEnable(GL_TEXTURE_2D);
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

GLuint coin_init(int n)
{
    GLuint list = glGenLists(1);

    image_load(coin_p, COIN_S);

    glNewList(list, GL_COMPILE);
    {
        double x;
        double y;
        int i;

        image_bind(coin_p);

        glBegin(GL_TRIANGLE_FAN);
        {
            glNormal3d(0.0, 0.0, +1.0);

            for (i = 0; i < n; i++)
            {
                x = cos(+2.0 * PI * i / n);
                y = sin(+2.0 * PI * i / n);

                glTexCoord2d(-0.5 * x + 0.5, 0.5 * y + 0.5);
                glVertex3d(CR * x, CR * y, +DZ);
            }
        }
        glEnd();

        glBegin(GL_TRIANGLE_FAN);
        {
            glNormal3d(0.0, 0.0, -1.0);
            
            for (i = 0; i < n; i++)
            {
                x = cos(-2.0 * PI * i / n);
                y = sin(-2.0 * PI * i / n);

                glTexCoord2d(+0.5 * x + 0.5, 0.5 * y + 0.5);
                glVertex3d(CR * x, CR * y, +DZ);
            }
        }
        glEnd();

        glBegin(GL_QUAD_STRIP);
        {
            for (i = 0; i <= n; i++)
            {
                x = cos(+2.0 * PI * i / n);
                y = sin(+2.0 * PI * i / n);

                glNormal3d(x, y, 0.0);
                glVertex3d(CR * x, CR * y, +DZ);
                glVertex3d(CR * x, CR * y, -DZ);
            }
        }
        glEnd();
    }
    glEndList();

    return list;
}

void coin_draw(GLuint list, const struct s_coin *cv, int cc)
{
    static const float c1[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
    static const float c2[4] = { 1.0f, 0.2f, 0.2f, 1.0f };
    static const float c3[4] = { 0.2f, 0.2f, 1.0f, 1.0f };
    static const float  s[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    double r = 360.0 * fmod(time_state(), 1.0);
    int i;

    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s);

    for (i = 0; i < cc; i++)
    {
        if (0 < cv[i].n && cv[i].n < 5)
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, c1);

        if (4 < cv[i].n && cv[i].n < 10)
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, c2);

        if (9 < cv[i].n)
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, c3);

        if (0 < cv[i].n)
        {
            glPushMatrix();
            {
                glTranslated(cv[i].p[0],
                             cv[i].p[1],
                             cv[i].p[2]);
                glRotated(r, 0.0, 1.0, 0.0);
                glCallList(list);
            }
            glPopMatrix();
        }
    }
}

int coin_test(const struct s_ball *up, struct s_coin *cv, int cc)
{
    int i, n = 0;

    for (i = 0; i < cc; i++)
    {
        double r[3];

        v_sub(r, up->p, cv[i].p);

        if (v_len(r) < CR + up->r)
        {
            n += cv[i].n;
            cv[i].n = 0;
        }
    }

    return n;
}

/*---------------------------------------------------------------------------*/

static struct image back_u;
static struct image back_n;
static struct image back_s;
static struct image back_w;
static struct image back_e;
static struct image back_d;

void back_init(const char *s)
{
    char filename[256];

    sprintf(filename, "%s_u.jpg", s);
    image_load(&back_u, filename);
    
    sprintf(filename, "%s_n.jpg", s);
    image_load(&back_n, filename);
    
    sprintf(filename, "%s_s.jpg", s);
    image_load(&back_s, filename);
    
    sprintf(filename, "%s_w.jpg", s);
    image_load(&back_w, filename);
    
    sprintf(filename, "%s_e.jpg", s);
    image_load(&back_e, filename);
    
    sprintf(filename, "%s_d.jpg", s);
    image_load(&back_d, filename);
}

void back_draw(void)
{
    glPushMatrix();
    glPushAttrib(GL_ENABLE_BIT);
    { 
        glDisable(GL_LIGHTING);

        glScalef(500.0f, 500.0f, 500.0f);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        image_bind(&back_u);
        glBegin(GL_QUADS);
        {
            glTexCoord2i(0, 1); glVertex3i(-1, +1, -1);
            glTexCoord2i(1, 1); glVertex3i(+1, +1, -1);
            glTexCoord2i(1, 0); glVertex3i(+1, +1, +1);
            glTexCoord2i(0, 0); glVertex3i(-1, +1, +1);
        }
        glEnd();

        image_bind(&back_n);
        glBegin(GL_QUADS);
        {
            glTexCoord2i(0, 1); glVertex3i(-1, -1, -1);
            glTexCoord2i(1, 1); glVertex3i(+1, -1, -1);
            glTexCoord2i(1, 0); glVertex3i(+1, +1, -1);
            glTexCoord2i(0, 0); glVertex3i(-1, +1, -1);
        }
        glEnd();

        image_bind(&back_s);
        glBegin(GL_QUADS);
        {
            glTexCoord2i(0, 1); glVertex3i(+1, -1, +1);
            glTexCoord2i(1, 1); glVertex3i(-1, -1, +1);
            glTexCoord2i(1, 0); glVertex3i(-1, +1, +1);
            glTexCoord2i(0, 0); glVertex3i(+1, +1, +1);
        }
        glEnd();

        image_bind(&back_w);
        glBegin(GL_QUADS);
        {
            glTexCoord2i(0, 1); glVertex3i(-1, -1, +1);
            glTexCoord2i(1, 1); glVertex3i(-1, -1, -1);
            glTexCoord2i(1, 0); glVertex3i(-1, +1, -1);
            glTexCoord2i(0, 0); glVertex3i(-1, +1, +1);
        }
        glEnd();

        image_bind(&back_e);
        glBegin(GL_QUADS);
        {
            glTexCoord2i(0, 1); glVertex3i(+1, -1, -1);
            glTexCoord2i(1, 1); glVertex3i(+1, -1, +1);
            glTexCoord2i(1, 0); glVertex3i(+1, +1, +1);
            glTexCoord2i(0, 0); glVertex3i(+1, +1, -1);
        }
        glEnd();

        image_bind(&back_d);
        glBegin(GL_QUADS);
        {
            glTexCoord2i(0, 1); glVertex3i(-1, -1, +1);
            glTexCoord2i(1, 1); glVertex3i(+1, -1, +1);
            glTexCoord2i(1, 0); glVertex3i(+1, -1, -1);
            glTexCoord2i(0, 0); glVertex3i(-1, -1, -1);
        }
        glEnd();
    }
    glPopAttrib();
    glPopMatrix();
}

void back_free(void)
{
    image_free(&back_d);
    image_free(&back_e);
    image_free(&back_w);
    image_free(&back_s);
    image_free(&back_n);
    image_free(&back_u);
}

/*---------------------------------------------------------------------------*/
