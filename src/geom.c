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
#include <stdlib.h>
#include <string.h>

#include "gl.h"
#include "main.h"
#include "vec3.h"
#include "solid.h"
#include "state.h"
#include "image.h"

#define PI 3.1415926535897932

#define BALL_FUDGE    0.01
#define GOAL_HEIGHT   3.00
#define GOAL_SPARKS  64
#define COIN_RADIUS   0.15
#define COIN_THICK    0.01
#define BACK_DIST   100.0
#define PART_COUNT  128
#define PART_SIZE     0.1

#define IMG_COIN "data/png/coin.png"
#define IMG_PART "data/png/part.png"

/*---------------------------------------------------------------------------*/

static double rnd(double l, double h)
{
    return l + (h - l) * rand() / RAND_MAX;
}

/*---------------------------------------------------------------------------*/

static GLuint ball_list;

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

void ball_init(void)
{
    static const float  a[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    static const float  s[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    static const float  e[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    static const float  h[1] = { 64.0f };

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
    int d = main_geom ? 4 : 2;

    ball_list = glGenLists(1);

    glNewList(ball_list, GL_COMPILE);
    glPushAttrib(GL_LIGHTING_BIT);
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   a);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  s);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  e);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, h);

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
}

void ball_free(void)
{
    glDeleteLists(ball_list, 1);
}

void ball_draw(double r,
               const double p[3],
               const double e[3][3])
{
    double M[16];

    m_basis(M, e[0], e[1], e[2]);

    glPushAttrib(GL_ENABLE_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glPushAttrib(GL_COLOR_BUFFER_BIT);
    {
        glDisable(GL_TEXTURE_2D);

        glPushMatrix();
        {
            glTranslated(p[0], p[1] + BALL_FUDGE, p[2]);
            glMultMatrixd(M);
            glScaled(r, r, r);

            glDepthMask(GL_FALSE);

            glCullFace(GL_FRONT);
            glCallList(ball_list);
            glCullFace(GL_BACK);
            glCallList(ball_list);

            glDepthMask(GL_TRUE);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

            glCallList(ball_list);
        }
        glPopMatrix();
    }
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

struct s_part
{
    GLfloat t;
    GLfloat a;
    GLfloat w;
    GLfloat c[3];
    GLfloat p[3];
    GLfloat v[3];
};

static struct image  part_img;
static struct s_part part[PART_COUNT];

void part_init(void)
{
    memset(part, 0, PART_COUNT * sizeof (struct s_part));
    image_load(&part_img, IMG_PART);
}

void part_free(void)
{
    image_free(&part_img);
}

static void particle(const GLfloat p[3], const GLfloat c[3],
                     GLfloat a, GLfloat rx, GLfloat ry, GLfloat rz)
{
    glPushMatrix();
    {
        glTranslatef(p[0], p[1], p[2]);
        glRotatef(ry, 0.0f, 1.0f, 0.0f);
        glRotatef(rx, 1.0f, 0.0f, 0.0f);
        glRotatef(rz, 0.0f, 0.0f, 1.0f);

        glBegin(GL_QUADS);
        {
            glColor4f(c[0], c[1], c[2], a);

            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(-PART_SIZE, -PART_SIZE);

            glTexCoord2f(1.0f, 0.0f);
            glVertex2f(+PART_SIZE, -PART_SIZE);

            glTexCoord2f(1.0f, 1.0f);
            glVertex2f(+PART_SIZE, +PART_SIZE);

            glTexCoord2f(0.0f, 1.0f);
            glVertex2f(-PART_SIZE, +PART_SIZE);
        }
        glEnd();
    }
    glPopMatrix();
}

static void part_make(const GLfloat p[3], const GLfloat v[3],
                      const GLfloat c[3], GLfloat t, GLfloat a, GLfloat w)
{
    int i;

    for (i = 0; i < PART_COUNT; i++)
        if (part[i].t <= 0.0)
        {
            part[i].p[0] = p[0];
            part[i].p[1] = p[1];
            part[i].p[2] = p[2];
            part[i].v[0] = v[0];
            part[i].v[1] = v[1];
            part[i].v[2] = v[2];
            part[i].c[0] = c[0];
            part[i].c[1] = c[1];
            part[i].c[2] = c[2];

            part[i].t = t;
            part[i].a = a;
            part[i].w = w;

            return;
        }
}

void part_draw(GLfloat rx, GLfloat ry)
{
    int i;

    glPushAttrib(GL_ENABLE_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    {
        glDisable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);

        glDepthMask(GL_FALSE);
        image_bind(&part_img);

        for (i = 0; i < PART_COUNT; i++)
            if (part[i].t > 0.0)
                particle(part[i].p, part[i].c, part[i].t, rx, ry, part[i].a);
    }
    glPopAttrib();
    glPopAttrib();
}

void part_update_grav(const double g[3], double dt)
{
    int i;

    for (i = 0; i < PART_COUNT; i++)
        if (part[i].t > 0.0)
        {
            part[i].v[0] += (GLfloat) (g[0] * dt);
            part[i].v[1] += (GLfloat) (g[1] * dt);
            part[i].v[2] += (GLfloat) (g[2] * dt);

            part[i].p[0] += (GLfloat) (part[i].v[0] * dt);
            part[i].p[1] += (GLfloat) (part[i].v[1] * dt);
            part[i].p[2] += (GLfloat) (part[i].v[2] * dt);

            part[i].t -= (GLfloat) dt;
            part[i].a += (GLfloat) (part[i].w * dt);
        }
}

/*---------------------------------------------------------------------------*/

static GLuint  goal_list;

static int     spark_n;
static GLfloat spark_a[GOAL_SPARKS];
static GLfloat spark_y[GOAL_SPARKS];

static void get_spark(double t, int i, GLfloat p[3], GLfloat *a, GLfloat *rz)
{
    double k = spark_a[i] + t / 5.0;

    p[0] = 0.9 * cos(k);
    p[1] = spark_y[i];
    p[2] = 0.9 * sin(k);

    *a  = 1.0 - spark_y[i] / GOAL_HEIGHT;
    *rz = k * 720.0;
}

void goal_init(void)
{
    int i, n = main_geom ? 32 : 16;

    spark_n = GOAL_SPARKS;

    for (i = 0; i < spark_n; i++)
    {
        spark_a[i] = (GLfloat) rnd(0.0, 2.0 * PI);
        spark_y[i] = (GLfloat) rnd(0.0, GOAL_HEIGHT);
    }

    goal_list = glGenLists(1);

    glNewList(goal_list, GL_COMPILE);
    glPushAttrib(GL_ENABLE_BIT);
    {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE);

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
    glPopAttrib();
    glEndList();
}

void goal_free(void)
{
    glDeleteLists(goal_list, 1);
}

void goal_draw(GLfloat rx, GLfloat ry, const struct s_goal *zv, int zc)
{
    static const GLfloat c[3] = { 1.0f, 1.0f, 0.0f };
    GLfloat p[3], a, rz;

    double t = time_state();
    int zi, j;

    for (zi = 0; zi < zc; zi++)
    {
        const struct s_goal *zp = zv + zi;

        glPushMatrix();
        glPushAttrib(GL_ENABLE_BIT);
        glPushAttrib(GL_DEPTH_BUFFER_BIT);
        {
            glEnable(GL_COLOR_MATERIAL);
            glDisable(GL_LIGHTING);
            glDepthMask(GL_FALSE);

            glTranslated(zp->p[0], zp->p[1], zp->p[2]);
            glScaled(zp->r, 1.0, zp->r);
            glCallList(goal_list);

            image_bind(&part_img);

            for (j = 0; j < spark_n; j++)
            {
                get_spark(t, j, p, &a, &rz);
                particle(p, c, a, rx, ry, rz);
            }
        }
        glPopAttrib();
        glPopAttrib();
        glPopMatrix();
    }
}

static void goal_part(struct s_goal *zv, int zc)
{
    static const GLfloat v[3] = { 0.0f, 0.0f, 0.0f };
    static const GLfloat c[3] = { 1.0f, 1.0f, 0.0f };
    GLfloat p[3], a, rz;

    double t = time_state();
    int zi, j;

    for (zi = 0; zi < zc; zi++)
    {
        for (j = 0; j < spark_n; j++)
        {
            get_spark(t, j, p, &a, &rz);
        
            p[0] = p[0] * zv[zi].r + zv[zi].p[0];
            p[1] = p[1]            + zv[zi].p[1];
            p[2] = p[2] * zv[zi].r + zv[zi].p[2];

            part_make(p, v, c, a, rz, 0.0);
        }
    }

    spark_n = 0;
}

int goal_test(const struct s_ball *up, struct s_goal *zv, int zc)
{
    double r[3];
    int zi;

    for (zi = 0; zi < zc; zi++)
    {
        r[0] = up->p[0] - zv[zi].p[0];
        r[1] = up->p[2] - zv[zi].p[2];
        r[2] = 0;

        if (v_len(r) < zv[zi].r - up->r && up->p[1] > zv[zi].p[1])
        {
            goal_part(zv, zc);
            return 1;
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static struct image coin_img;
static GLuint       coin_list;

static const GLfloat coin_color[11][3] = {
    { 0.0f, 0.0f, 0.0f },
    { 1.0f, 1.0f, 0.0f },
    { 1.0f, 1.0f, 0.0f },
    { 1.0f, 1.0f, 0.0f },
    { 1.0f, 1.0f, 0.0f },
    { 1.0f, 0.2f, 0.2f },
    { 1.0f, 0.2f, 0.2f },
    { 1.0f, 0.2f, 0.2f },
    { 1.0f, 0.2f, 0.2f },
    { 1.0f, 0.2f, 0.2f },
    { 0.2f, 0.2f, 1.0f },
};

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

void coin_init(void)
{
    static const float  a[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    static const float  s[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    static const float  e[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    static const float  h[1] = { 32.0f };

    int n = main_geom ? 32 : 16;

    coin_list = glGenLists(1);

    image_load(&coin_img, IMG_COIN);

    glNewList(coin_list, GL_COMPILE);
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   a);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  s);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  e);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, h);

        glPushMatrix();
        {
            image_bind(&coin_img);

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
    glDeleteLists(coin_list, 1);
    image_free(&coin_img);
}

void coin_draw(const struct s_coin *cv, int cc)
{
    double r = 360.0 * fmod(time_state(), 1.0);
    int i;

    glPushAttrib(GL_ENABLE_BIT);
    {
        glEnable(GL_COLOR_MATERIAL);

        for (i = 0; i < cc; i++)
            if (0 < cv[i].n)
            {
                glPushMatrix();
                {
                    if (0 < cv[i].n && cv[i].n < 11)
                        glColor3fv(coin_color[cv[i].n]);

                    glTranslated(cv[i].p[0], cv[i].p[1], cv[i].p[2]);
                    glRotated(r, 0.0, 1.0, 0.0);

                    glCallList(coin_list);
                }
                glPopMatrix();
            }
        
    }
    glPopAttrib();
}

int coin_test(const struct s_ball *up, struct s_coin *cv, int cc)
{
    double  r[3];
    GLfloat p[3];
    GLfloat v[3];

    int i, j, k;
    int n = 0;

    for (i = 0; i < cc; i++)
    {
        v_sub(r, up->p, cv[i].p);

        if (cv[i].n > 0 && v_len(r) < up->r + COIN_RADIUS)
        {
            for (j = 0; j < 10; j++)
            {
                double a = rnd(-1.0 * PI, +1.0 * PI);
                double b = rnd(+0.3 * PI, +0.5 * PI);
                double c = rnd(-4.0 * PI, +4.0 * PI);

                p[0] = (GLfloat) cv[i].p[0];
                p[1] = (GLfloat) cv[i].p[1];
                p[2] = (GLfloat) cv[i].p[2];

                v[0] = (GLfloat) (4.0 * cos(a) * cos(b));
                v[1] = (GLfloat) (4.0 *          sin(b));
                v[2] = (GLfloat) (4.0 * sin(a) * cos(b));

                k = (cv[i].n < 11) ? cv[i].n : 10;

                part_make(p, v, coin_color[k], 1.0, 0.0, V_DEG(c));
            }

            n += cv[i].n;
            cv[i].n = 0;
        }
    }

    return n;
}

/*---------------------------------------------------------------------------*/

static GLuint back_list;

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

    back_list = glGenLists(1);

    glNewList(back_list, GL_COMPILE);
    glPushAttrib(GL_ENABLE_BIT);
    { 
        glDisable(GL_LIGHTING);

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
    glEndList();
}

void back_free(void)
{
    glDeleteLists(back_list, 1);

    image_free(&back_d);
    image_free(&back_e);
    image_free(&back_w);
    image_free(&back_s);
    image_free(&back_n);
    image_free(&back_u);
}

void back_draw(void)
{
    glPushMatrix();
    {
        glScalef(BACK_DIST, BACK_DIST, BACK_DIST);
        glCallList(back_list);
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/
