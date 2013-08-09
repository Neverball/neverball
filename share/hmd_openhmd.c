/*
 * Copyright (C) 2013 Robert Kooima
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

#define OHMD_STATIC 1
#include <openhmd/openhmd.h>
#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "glext.h"
#include "glsl.h"
#include "fbo.h"
#include "hmd.h"

static ohmd_context *ctx = NULL;
static ohmd_device  *dev = NULL;
static int           eye = 0;

static int hmd_hres = 0;
static int hmd_vres = 0;
static int fbo_hres = 0;
static int fbo_vres = 0;

static fbo L_fbo   = { 0, 0, 0 };
static fbo R_fbo   = { 0, 0, 0 };

static glsl L_glsl = { 0, 0, 0 };
static glsl R_glsl = { 0, 0, 0 };

static GLuint L_vbo = 0;
static GLuint R_vbo = 0;

/*---------------------------------------------------------------------------*/

/* I (rlk) am not very happy with this distortion shader. It's not sufficiently
   general because it's written to conform to the current state of OpenHMD.

   It's an adaptation of the Oculus SDK shader, however Oculus uses one texture
   for both eyes, while the OpenHMD example uses two textures. Thus this shader
   is a two-texture shader hacked to behave like there's only one texture.

   If it used texture coordinates instead of gl_FragCoord then it wouldn't have
   to know the FBO size.

   Also, the values of Scale and ScaleIn should be dynamically computed, but
   currently they have hard-coded values needed to match the FOV given by OHMD.
*/

static const char *hmd_vert[] = {
    "void main()\n",
    "{\n",
        "gl_Position = ftransform();\n",
    "}\n",
};

static const char *hmd_frag[] = {
    "#version 120\n",

    "uniform sampler2D warpTexture;\n",
    "uniform vec2      LensCenter;\n",
    "uniform vec2      ScreenCenter;\n",
    "uniform vec2      ScreenSize;\n",

    "const vec2 Scale   = vec2(0.1469278, 0.2350845);\n",
    "const vec2 ScaleIn = vec2(4, 2.5);\n",
    "const vec4 HMDWarpParam = vec4(1, 0.22, 0.24, 0);\n",

    "vec2 HMDWarp(vec2 in01)\n",
    "{\n",
        "vec2 theta = (in01 - LensCenter) * ScaleIn;\n",
        "float rSq = theta.x * theta.x + theta.y * theta.y;\n",
        "vec2 rvector = theta * (HMDWarpParam.x + "
                                "HMDWarpParam.y * rSq + "
                                "HMDWarpParam.z * rSq * rSq + "
                                "HMDWarpParam.w * rSq * rSq * rSq);\n",
        "return LensCenter + Scale * rvector;\n",
    "}\n",

    "void main()\n",
    "{\n",
         "vec2 tc = HMDWarp(gl_FragCoord.xy / ScreenSize);\n",
         "tc.x = gl_FragCoord.x < ScreenSize.x / 2.0 ? (2.0 * tc.x) : (2.0 * (tc.x - 0.5));\n",
         "vec2 tt = step(vec2(0.0), tc) * step(tc, vec2(1.0));\n",
         "gl_FragColor = tt.x * tt.y * texture2D(warpTexture, tc);\n",
    "}\n",
};

static const GLfloat L_rect[4][2] = {
    { -1, -1 }, {  0, -1 }, { -1,  1 }, {  0,  1 }
};
static const GLfloat R_rect[4][2] = {
    {  0, -1 }, {  1, -1 }, {  0,  1 }, {  1,  1 }
};

/*---------------------------------------------------------------------------*/

void hmd_init()
{
    hmd_hres = config_get_d(CONFIG_WIDTH);
    hmd_vres = config_get_d(CONFIG_HEIGHT);

    /* Start up OpenHMD. */

    if ((ctx = ohmd_ctx_create()))
    {
        if (ohmd_ctx_probe(ctx) > 0)
        {
            if ((dev = ohmd_list_open_device(ctx, 0)))
            {
                /* Create the off-screen frame buffers. */

                ohmd_device_geti(dev, OHMD_SCREEN_HORIZONTAL_RESOLUTION, &hmd_hres);
                ohmd_device_geti(dev, OHMD_SCREEN_VERTICAL_RESOLUTION,   &hmd_vres);
            }
        }
    }

    fbo_hres = 5 * hmd_hres / 8;
    fbo_vres = 5 * hmd_vres / 4;

    fbo_create(&L_fbo,  fbo_hres, fbo_vres);
    fbo_create(&R_fbo, fbo_hres, fbo_vres);

    /* Create and initialize the distortion shader. */

    glsl_create(&L_glsl,  sizeof (hmd_vert) / sizeof (char *), hmd_vert,
                             sizeof (hmd_frag) / sizeof (char *), hmd_frag);
    glsl_create(&R_glsl, sizeof (hmd_vert) / sizeof (char *), hmd_vert,
                             sizeof (hmd_frag) / sizeof (char *), hmd_frag);

    glUseProgram_  (L_glsl.program);
    glsl_uniform2f(&L_glsl, "LensCenter", 0.2863248, 0.5);
    glsl_uniform2f(&L_glsl, "ScreenCenter", 0.25, 0.5);
    glsl_uniform2f(&L_glsl, "ScreenSize", hmd_hres, hmd_vres);

    glUseProgram_  (R_glsl.program);
    glsl_uniform2f(&R_glsl, "LensCenter", 0.7136753, 0.5);
    glsl_uniform2f(&R_glsl, "ScreenCenter", 0.75, 0.5);
    glsl_uniform2f(&R_glsl, "ScreenSize", hmd_hres, hmd_vres);

    glUseProgram_(0);

    /* Initialize VBOs for the on-screen rectangles. */

    glGenBuffers_(1, &L_vbo);
    glBindBuffer_(GL_ARRAY_BUFFER, L_vbo);
    glBufferData_(GL_ARRAY_BUFFER, sizeof (L_rect), L_rect, GL_STATIC_DRAW);
    glGenBuffers_(1, &R_vbo);
    glBindBuffer_(GL_ARRAY_BUFFER, R_vbo);
    glBufferData_(GL_ARRAY_BUFFER, sizeof (R_rect), R_rect, GL_STATIC_DRAW);
    glBindBuffer_(GL_ARRAY_BUFFER, 0);
}

void hmd_free()
{
    if (ctx) ohmd_ctx_destroy(ctx);

    fbo_delete (&L_fbo);
    fbo_delete (&R_fbo);

    glsl_delete(&L_glsl);
    glsl_delete(&R_glsl);

    if (L_vbo)  glDeleteBuffers_(1, &L_vbo);
    if (R_vbo) glDeleteBuffers_(1, &R_vbo);

    dev = NULL;
    ctx = NULL;
    eye =    0;
}

void hmd_step()
{
    if (ctx) ohmd_ctx_update(ctx);
}

void hmd_swap()
{
    glDisable(GL_BLEND);
    glEnableClientState(GL_VERTEX_ARRAY);

    /* Prepare to draw a screen-filling pair of rectangles. */

    glBindFramebuffer_(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, hmd_hres, hmd_vres);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Draw the left eye view, distorted, to the screen. */

    glUseProgram_(L_glsl.program);
    glBindTexture(GL_TEXTURE_2D, L_fbo.color_texture);
    glBindBuffer_(GL_ARRAY_BUFFER, L_vbo);
    glVertexPointer(2, GL_FLOAT, 0, (GLvoid *) 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    /* Draw the right eye view, distorted, to the screen. */

    glUseProgram_(R_glsl.program);
    glBindTexture(GL_TEXTURE_2D, R_fbo.color_texture);
    glBindBuffer_(GL_ARRAY_BUFFER, R_vbo);
    glVertexPointer(2, GL_FLOAT, 0, (GLvoid *) 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    /* Revert that state. */

    glBindBuffer_(GL_ARRAY_BUFFER, 0);
    glUseProgram_(0);
    glEnable(GL_BLEND);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void hmd_prep_left()
{
    glBindFramebuffer_(GL_FRAMEBUFFER, L_fbo.framebuffer);
    glViewport(0, 0, fbo_hres, fbo_vres);
    eye = 0;
}

void hmd_prep_right()
{
    glBindFramebuffer_(GL_FRAMEBUFFER, R_fbo.framebuffer);
    glViewport(0, 0, fbo_hres, fbo_vres);
    eye = 1;
}

void hmd_persp(float n, float f)
{
    float M[16];

    if (eye)
    {
        glMatrixMode(GL_PROJECTION);
        ohmd_device_getf(dev, OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX, M);
        glLoadMatrixf(M);

        glMatrixMode(GL_MODELVIEW);
        ohmd_device_getf(dev, OHMD_RIGHT_EYE_GL_MODELVIEW_MATRIX, M);
        glLoadMatrixf(M);
    }
    else
    {
        glMatrixMode(GL_PROJECTION);
        ohmd_device_getf(dev, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX, M);
        glLoadMatrixf(M);

        glMatrixMode(GL_MODELVIEW);
        ohmd_device_getf(dev, OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX, M);
        glLoadMatrixf(M);
    }
}

void hmd_ortho()
{
    hmd_persp(0.5f, 2.0f);

    glScalef    ( 1.25f / hmd_vres,  1.25f / hmd_vres,  1.0f);
    glTranslatef(-0.50f * hmd_hres, -0.50f * hmd_vres, -1.0f);
}

/*---------------------------------------------------------------------------*/
