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

#include <stdlib.h>
#include <string.h>

#include "vec3.h"
#include "glext.h"
#include "config.h"
#include "solid_gl.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

#define IMG_DEFAULT "ball/default.png"
#define IMG_ARBBALL "ball/arbball.png"

static int has_solid = 0;
static int has_inner = 0;
static int has_outer = 0;

static struct s_file solid;
static struct s_file inner;
static struct s_file outer;

#define F_PENDULUM   1
#define F_DRAWBACK   2
#define F_DRAWCLIP   4
#define F_DEPTHMASK  8
#define F_DEPTHTEST 16

static int solid_flags;
static int inner_flags;
static int outer_flags;

static float solid_alpha;
static float inner_alpha;
static float outer_alpha;

static GLuint oldball_list;
static GLuint oldball_text;
static GLuint arbball_list;
static GLuint arbball_text;

/*---------------------------------------------------------------------------*/

/* These are the faces of an octahedron in positive longitude/latitude. */

static float oldball_octahedron[8][3][2] = {
    {{   0.0f,  90.0f }, {   0.0f, 0.0f }, {  90.0f, 0.0f }},
    {{  90.0f,  90.0f }, {  90.0f, 0.0f }, { 180.0f, 0.0f }},
    {{ 180.0f,  90.0f }, { 180.0f, 0.0f }, { 270.0f, 0.0f }},
    {{ 270.0f,  90.0f }, { 270.0f, 0.0f }, { 360.0f, 0.0f }},
    {{   0.0f, -90.0f }, {  90.0f, 0.0f }, {   0.0f, 0.0f }},
    {{  90.0f, -90.0f }, { 180.0f, 0.0f }, {  90.0f, 0.0f }},
    {{ 180.0f, -90.0f }, { 270.0f, 0.0f }, { 180.0f, 0.0f }},
    {{ 270.0f, -90.0f }, { 360.0f, 0.0f }, { 270.0f, 0.0f }},
};

static void oldball_midpoint(float *P, const float *A, const float *B)
{
    float D[2];

    /* The haversine midpoint method. */

    D[0] = fcosf(B[1]) * fcosf(B[0] - A[0]);
    D[1] = fcosf(B[1]) * fsinf(B[0] - A[0]);

    P[0] = A[0] + fatan2f(D[1], fcosf(A[1]) + D[0]);

    P[1] = fatan2f(fsinf(A[1]) +
                   fsinf(B[1]),
                   fsqrtf((fcosf(A[1]) + D[0]) *
                          (fcosf(A[1]) + D[0]) + D[1] * D[1])); 
}

static void oldball_vertex(const float *p)
{
    /* Draw a vertex with normal and texture coordinate at the given lon/lat. */

    const float x = fsinf(p[0]) * fcosf(p[1]);
    const float y =               fsinf(p[1]);
    const float z = fcosf(p[0]) * fcosf(p[1]);

    glTexCoord2f((+p[0]               ) / V_RAD(360.0f),
                 (-p[1] + V_RAD(90.0f)) / V_RAD(180.0f));

    glNormal3f(x, y, z);
    glVertex3f(x, y, z);
}

static void oldball_subdiv(const float *a,
                        const float *b,
                        const float *c, int D)
{
    if (D > 0)
    {
        /* Recursively subdivide the given triangle. */

        float d[2];
        float e[2];
        float f[2];

        oldball_midpoint(d, a, b);
        oldball_midpoint(e, b, c);
        oldball_midpoint(f, c, a);

        oldball_subdiv(a, d, f, D - 1);
        oldball_subdiv(d, b, e, D - 1);
        oldball_subdiv(f, e, c, D - 1);
        oldball_subdiv(d, e, f, D - 1);
    }
    else
    {
        /* Draw the given triangle. */

        oldball_vertex(a);
        oldball_vertex(b);
        oldball_vertex(c);
    }
}

void oldball_init(int b)
{
    char name[MAXSTR];

    strncpy(name, IMG_DEFAULT, MAXSTR - 12);

    if ((oldball_text = make_image_from_file(name)))
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    oldball_list = glGenLists(1);

    glNewList(oldball_list, GL_COMPILE);
    {
#if 1
        int i, d = b ? 4 : 3;

        glBegin(GL_TRIANGLES);
        {
            for (i = 0; i < 8; ++i)
            {
                float a[2];
                float b[2];
                float c[2];

                a[0] = V_RAD(oldball_octahedron[i][0][0]);
                a[1] = V_RAD(oldball_octahedron[i][0][1]);

                b[0] = V_RAD(oldball_octahedron[i][1][0]);
                b[1] = V_RAD(oldball_octahedron[i][1][1]);

                c[0] = V_RAD(oldball_octahedron[i][2][0]);
                c[1] = V_RAD(oldball_octahedron[i][2][1]);

                oldball_subdiv(a, b, c, d);
            }
        }
        glEnd();
#else
        int i, slices = b ? 32 : 16;
        int j, stacks = b ? 16 :  8;

        for (i = 0; i < stacks; i++)
        {
            float k0 = (float)  i      / stacks;
            float k1 = (float) (i + 1) / stacks;

            float s0 = fsinf(V_PI * (k0 - 0.5));
            float c0 = fcosf(V_PI * (k0 - 0.5));
            float s1 = fsinf(V_PI * (k1 - 0.5));
            float c1 = fcosf(V_PI * (k1 - 0.5));

            glBegin(GL_QUAD_STRIP);
            {
                for (j = 0; j <= slices; j++)
                {
                    float k = (float) j / slices;
                    float s = fsinf(V_PI * k * 2.0);
                    float c = fcosf(V_PI * k * 2.0);

                    glTexCoord2f(k, k0);
                    glNormal3f(s * c0, c * c0, s0);
                    glVertex3f(s * c0, c * c0, s0);

                    glTexCoord2f(k, k1);
                    glNormal3f(s * c1, c * c1, s1);
                    glVertex3f(s * c1, c * c1, s1);
                }
            }
            glEnd();
        }
#endif
    }
    glEndList();

    strncpy(name, IMG_ARBBALL, MAXSTR - 12);

    if ((arbball_text = make_image_from_file(name)))
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    arbball_list = glGenLists(1);

    glNewList(arbball_list, GL_COMPILE);
    {
#if 1
        int i, d = b ? 4 : 3;

        glBegin(GL_TRIANGLES);
        {
            for (i = 0; i < 8; ++i)
            {
                float a[2];
                float b[2];
                float c[2];

                a[0] = V_RAD(oldball_octahedron[i][0][0]);
                a[1] = V_RAD(oldball_octahedron[i][0][1]);

                b[0] = V_RAD(oldball_octahedron[i][1][0]);
                b[1] = V_RAD(oldball_octahedron[i][1][1]);

                c[0] = V_RAD(oldball_octahedron[i][2][0]);
                c[1] = V_RAD(oldball_octahedron[i][2][1]);

                oldball_subdiv(a, b, c, d);
            }
        }
        glEnd();
#else
        int i, slices = b ? 32 : 16;
        int j, stacks = b ? 16 :  8;

        for (i = 0; i < stacks; i++)
        {
            float k0 = (float)  i      / stacks;
            float k1 = (float) (i + 1) / stacks;

            float s0 = fsinf(V_PI * (k0 - 0.5));
            float c0 = fcosf(V_PI * (k0 - 0.5));
            float s1 = fsinf(V_PI * (k1 - 0.5));
            float c1 = fcosf(V_PI * (k1 - 0.5));

            glBegin(GL_QUAD_STRIP);
            {
                for (j = 0; j <= slices; j++)
                {
                    float k = (float) j / slices;
                    float s = fsinf(V_PI * k * 2.0);
                    float c = fcosf(V_PI * k * 2.0);

                    glTexCoord2f(k, k0);
                    glNormal3f(s * c0, c * c0, s0);
                    glVertex3f(s * c0, c * c0, s0);

                    glTexCoord2f(k, k1);
                    glNormal3f(s * c1, c * c1, s1);
                    glVertex3f(s * c1, c * c1, s1);
                }
            }
            glEnd();
        }
#endif
    }
    glEndList();
}

void oldball_free(void)
{
    if (glIsList(oldball_list))
        glDeleteLists(oldball_list, 1);

    if (glIsTexture(oldball_text))
        glDeleteTextures(1, &oldball_text);

    if (glIsList(arbball_list))
        glDeleteLists(arbball_list, 1);

    if (glIsTexture(arbball_text))
        glDeleteTextures(1, &arbball_text);

    oldball_list = 0;
    oldball_text = 0;

    arbball_list = 0;
    arbball_text = 0;
}

void oldball_draw(int arb)
{
    static const float a[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    static const float s[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    static const float e[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    static const float h[1] = { 20.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   a);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  s);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  e);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, h);

    glEnable(GL_COLOR_MATERIAL);
    {
        glBindTexture(GL_TEXTURE_2D, (arb) ? (arbball_text) : (oldball_text));

        /* Render the ball back to front in case it is translucent. */

        glDepthMask(GL_FALSE);
        {
            glCullFace(GL_FRONT);
            glCallList((arb) ? (arbball_list) : (oldball_list));
            glCullFace(GL_BACK);
            glCallList(oldball_list);
        }
        glDepthMask(GL_TRUE);

        /* Render the ball into the depth buffer. */

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        {
            glCallList((arb) ? (arbball_list) : (oldball_list));
        }
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        /* Ensure the ball is visible even when obscured by geometry. */

        glDisable(GL_DEPTH_TEST);
        {
            glColor4f(1.0f, 1.0f, 1.0f, 0.1f);
            glCallList((arb) ? (arbball_list) : (oldball_list));
        }
        glEnable(GL_DEPTH_TEST);
    }
    glDisable(GL_COLOR_MATERIAL);
}

/*---------------------------------------------------------------------------*/

#define SET(B, v, b) ((v) ? ((B) | (b)) : ((B) & ~(b)))

static int ball_opts(const struct s_file *fp, float *alpha)
{
    int flags = F_DEPTHTEST;
    int di;

    for (di = 0; di < fp->dc; ++di)
    {
        char *k = fp->av + fp->dv[di].ai;
        char *v = fp->av + fp->dv[di].aj;

        if (strcmp(k, "pendulum")  == 0)
            flags = SET(flags, atoi(v), F_PENDULUM);
        if (strcmp(k, "drawback")  == 0)
            flags = SET(flags, atoi(v), F_DRAWBACK);
        if (strcmp(k, "drawclip")  == 0)
            flags = SET(flags, atoi(v), F_DRAWCLIP);
        if (strcmp(k, "depthmask") == 0)
            flags = SET(flags, atoi(v), F_DEPTHMASK);
        if (strcmp(k, "depthtest") == 0)
            flags = SET(flags, atoi(v), F_DEPTHTEST);
        if (strcmp(k, "alphatest") == 0)
            sscanf(v, "%f", alpha);
    }

    return flags;
}

void ball_init(void)
{
    int T = config_get_d(CONFIG_TEXTURES);

    char ball_file[PATHMAX];
    char solid_file[PATHMAX];
    char inner_file[PATHMAX];
    char outer_file[PATHMAX];

    config_get_s(CONFIG_BALL, ball_file, PATHMAX / 2 - 12);

    strncpy(solid_file, "ball/", PATHMAX);
    strncpy(inner_file, "ball/", PATHMAX);
    strncpy(outer_file, "ball/", PATHMAX);

    strcat(solid_file, ball_file);
    strcat(inner_file, ball_file);
    strcat(outer_file, ball_file);

    strcat(solid_file, "/");
    strcat(inner_file, "/");
    strcat(outer_file, "/");

    strcat(solid_file, ball_file);
    strcat(inner_file, ball_file);
    strcat(outer_file, ball_file);

    strcat(solid_file, "-solid.sol");
    strcat(inner_file, "-inner.sol");
    strcat(outer_file, "-outer.sol");

    solid_flags = 0;
    inner_flags = 0;
    outer_flags = 0;

    solid_alpha = 1.0f;
    inner_alpha = 1.0f;
    outer_alpha = 1.0f;

    if ((has_solid = sol_load_gl(&solid, config_data(solid_file), T, 0)))
        solid_flags = ball_opts(&solid, &solid_alpha);

    if ((has_inner = sol_load_gl(&inner, config_data(inner_file), T, 0)))
        inner_flags = ball_opts(&inner, &inner_alpha);

    if ((has_outer = sol_load_gl(&outer, config_data(outer_file), T, 0)))
        outer_flags = ball_opts(&outer, &outer_alpha);
}

void ball_free(void)
{
    if (has_outer) sol_free_gl(&outer);
    if (has_inner) sol_free_gl(&inner);
    if (has_solid) sol_free_gl(&solid);

    has_solid = has_inner = has_outer = 0;
}

/*---------------------------------------------------------------------------*/

static void ball_draw_solid(const float *ball_M,
                            const float *ball_bill_M, float t)
{
    if (has_solid)
    {
        const int mask = (solid_flags & F_DEPTHMASK);
        const int test = (solid_flags & F_DEPTHTEST);

        if (solid_alpha < 1.0f)
        {
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_GEQUAL, solid_alpha);
        }

        glPushMatrix();
        {
            /* Apply the ball rotation. */

            glMultMatrixf(ball_M);

            /* Draw the solid billboard geometry. */

            if (solid.rc)
            {
                if (test == 0) glDisable(GL_DEPTH_TEST);
                if (mask == 0) glDepthMask(GL_FALSE);
                glDisable(GL_LIGHTING);
                {
                    sol_bill(&solid, ball_bill_M, t);
                }
                glEnable(GL_LIGHTING);
                if (mask == 0) glDepthMask(GL_TRUE);
                if (test == 0) glEnable(GL_DEPTH_TEST);
            }

            /* Draw the solid opaque and transparent geometry. */

            sol_draw(&solid, mask, test);
        }
        glPopMatrix();

        if (solid_alpha < 1.0f)
            glDisable(GL_ALPHA_TEST);
    }
}

static void ball_draw_inner(const float *pend_M,
                            const float *bill_M,
                            const float *pend_bill_M, float t)
{
    if (has_inner)
    {
        const int pend = (inner_flags & F_PENDULUM);
        const int mask = (inner_flags & F_DEPTHMASK);
        const int test = (inner_flags & F_DEPTHTEST);

        if (inner_alpha < 1.0f)
        {
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_GEQUAL, inner_alpha);
        }

        /* Apply the pendulum rotation. */

        if (pend)
        {
            glPushMatrix();
            glMultMatrixf(pend_M);
        }

        /* Draw the inner opaque and transparent geometry. */

        sol_draw(&inner, mask, test);

        /* Draw the inner billboard geometry. */

        if (inner.rc)
        {
            if (test == 0) glDisable(GL_DEPTH_TEST);
            if (mask == 0) glDepthMask(GL_FALSE);
            glDisable(GL_LIGHTING);
            {
                if (pend)
                    sol_bill(&inner, pend_bill_M, t);
                else
                    sol_bill(&inner, bill_M,      t);
            }

            glEnable(GL_LIGHTING);
            if (mask == 0) glDepthMask(GL_TRUE);
            if (test == 0) glEnable(GL_DEPTH_TEST);
        }

        if (pend)
            glPopMatrix();

        if (inner_alpha < 1.0f)
            glDisable(GL_ALPHA_TEST);
    }
}

static void ball_draw_outer(const float *pend_M,
                            const float *bill_M,
                            const float *pend_bill_M, float t)
{
    if (has_outer)
    {
        const int pend = (outer_flags & F_PENDULUM);
        const int mask = (outer_flags & F_DEPTHMASK);
        const int test = (outer_flags & F_DEPTHTEST);

        if (outer_alpha < 1.0f)
        {
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_GEQUAL, outer_alpha);
        }

       /* Apply the pendulum rotation. */

        if (pend)
        {
            glPushMatrix();
            glMultMatrixf(pend_M);
        }

        /* Draw the outer opaque and transparent geometry. */

        sol_draw(&outer, mask, test);

        /* Draw the outer billboard geometry. */

        if (outer.rc)
        {
            if (test == 0) glDisable(GL_DEPTH_TEST);
            if (mask == 0) glDepthMask(GL_FALSE);
            glDisable(GL_LIGHTING);
            {
                if (pend)
                    sol_bill(&outer, pend_bill_M, t);
                else
                    sol_bill(&outer, bill_M,      t);
            }
            glEnable(GL_LIGHTING);
            if (mask == 0) glDepthMask(GL_TRUE);
            if (test == 0) glEnable(GL_DEPTH_TEST);
        }

        if (pend)
            glPopMatrix();

        if (outer_alpha < 1.0f)
            glDisable(GL_ALPHA_TEST);
    }
}

/*---------------------------------------------------------------------------*/

static void ball_pass_inner(const float *ball_M,
                            const float *pend_M,
                            const float *bill_M,
                            const float *ball_bill_M,
                            const float *pend_bill_M, float t)
{
    /* Sort the inner ball using clip planes. */

    if      (inner_flags & F_DRAWCLIP)
    {
        glEnable(GL_CLIP_PLANE1);
        ball_draw_inner(        pend_M, bill_M,              pend_bill_M, t);
        glDisable(GL_CLIP_PLANE1);
        
        glEnable(GL_CLIP_PLANE2);
        ball_draw_inner(        pend_M, bill_M,              pend_bill_M, t);
        glDisable(GL_CLIP_PLANE2);
    }

    /* Sort the inner ball using face culling. */

    else if (inner_flags & F_DRAWBACK)
    {
        glCullFace(GL_FRONT);
        ball_draw_inner(        pend_M, bill_M,              pend_bill_M, t);
        glCullFace(GL_BACK);
        ball_draw_inner(        pend_M, bill_M,              pend_bill_M, t);
    }

    /* Draw the inner ball normally. */

    else
    {
        ball_draw_inner(        pend_M, bill_M,              pend_bill_M, t);
    }
}

static void ball_pass_solid(const float *ball_M,
                            const float *pend_M,
                            const float *bill_M,
                            const float *ball_bill_M,
                            const float *pend_bill_M, float t)
{
    /* Sort the solid ball with the inner ball using clip planes. */

    if      (solid_flags & F_DRAWCLIP)
    {
        glEnable(GL_CLIP_PLANE1);
        ball_draw_solid(ball_M,                 ball_bill_M, t);
        glDisable(GL_CLIP_PLANE1);
        
        ball_pass_inner(ball_M, pend_M, bill_M, ball_bill_M, pend_bill_M, t);

        glEnable(GL_CLIP_PLANE2);
        ball_draw_solid(ball_M,                 ball_bill_M, t);
        glDisable(GL_CLIP_PLANE2);
    }

    /* Sort the solid ball with the inner ball using face culling. */

    else if (solid_flags & F_DRAWBACK)
    {
        glCullFace(GL_FRONT);
        ball_draw_solid(ball_M,                 ball_bill_M, t);
        glCullFace(GL_BACK);
        
        ball_pass_inner(ball_M, pend_M, bill_M, ball_bill_M, pend_bill_M, t);
        ball_draw_solid(ball_M,                 ball_bill_M, t);
    }

    /* Draw the solid ball after the inner ball. */

    else
    {
        ball_pass_inner(ball_M, pend_M, bill_M, ball_bill_M, pend_bill_M, t);
        ball_draw_solid(ball_M,                 ball_bill_M, t);
    }
}

static void ball_pass_outer(const float *ball_M,
                            const float *pend_M,
                            const float *bill_M,
                            const float *ball_bill_M,
                            const float *pend_bill_M, float t)
{
    /* Sort the outer ball with the solid ball using clip planes. */

    if      (outer_flags & F_DRAWCLIP)
    {
        glEnable(GL_CLIP_PLANE1);
        ball_draw_outer(        pend_M, bill_M,              pend_bill_M, t);
        glDisable(GL_CLIP_PLANE1);
        
        ball_pass_solid(ball_M, pend_M, bill_M, ball_bill_M, pend_bill_M, t);

        glEnable(GL_CLIP_PLANE2);
        ball_draw_outer(        pend_M, bill_M,              pend_bill_M, t);
        glDisable(GL_CLIP_PLANE2);
    }

    /* Sort the outer ball with the solid ball using face culling. */

    else if (outer_flags & F_DRAWBACK)
    {
        glCullFace(GL_FRONT);
        ball_draw_outer(        pend_M, bill_M,              pend_bill_M, t);
        glCullFace(GL_BACK);
        
        ball_pass_solid(ball_M, pend_M, bill_M, ball_bill_M, pend_bill_M, t);
        ball_draw_outer(        pend_M, bill_M,              pend_bill_M, t);
    }

    /* Draw the outer ball after the solid ball. */

    else
    {
        ball_pass_solid(ball_M, pend_M, bill_M, ball_bill_M, pend_bill_M, t);
        ball_draw_outer(        pend_M, bill_M,              pend_bill_M, t);
    }
}

/*---------------------------------------------------------------------------*/

void ball_draw(const float *ball_M,
               const float *pend_M,
               const float *bill_M, float t)
{
    /* Compute transforms for ball and pendulum billboards. */

    float ball_T[16], ball_bill_M[16];
    float pend_T[16], pend_bill_M[16];

    m_xps(ball_T, ball_M);
    m_xps(pend_T, pend_M);
    m_xps(pend_T, pend_M);

    m_mult(ball_bill_M, ball_T, bill_M);
    m_mult(pend_bill_M, pend_T, bill_M);

    /* Go to GREAT pains to ensure all layers are drawn back-to-front. */

    ball_pass_outer(ball_M, pend_M, bill_M, ball_bill_M, pend_bill_M, t);
}

/*---------------------------------------------------------------------------*/
