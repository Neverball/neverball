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

#include "config.h"
#include "glext.h"
#include "glsl.h"
#include "fbo.h"

/*---------------------------------------------------------------------------*/

static glsl distortion = { 0, 0, 0 };

static fbo    L_fbo = { 0, 0, 0 };
static fbo    R_fbo = { 0, 0, 0 };

static GLuint L_vbo = 0;
static GLuint R_vbo = 0;

struct point
{
    GLfloat x;
    GLfloat y;
    GLfloat u;
    GLfloat v;
};

static const struct point L_rect[4] = {
    { -1, -1,  0,  0 }, {  0, -1,  1,  0 },
    { -1,  1,  0,  1 }, {  0,  1,  1,  1 }
};
static const struct point R_rect[4] = {
    {  0, -1,  0,  0 }, {  1, -1,  1,  0 },
    {  0,  1,  0,  1 }, {  1,  1,  1,  1 }
};

/*---------------------------------------------------------------------------*/

static const char *hmd_vert[] = {
    "void main()\n",
    "{\n",
        "gl_TexCoord[0] = gl_MultiTexCoord0;\n",
        "gl_Position    = gl_Vertex;\n",
    "}\n",
};

#if 0

static const char *hmd_frag[] = {
    "#version 120\n",

    "uniform sampler2D warpTexture;\n",
    "uniform vec2      LensCenter;\n",
    "uniform vec4      DistortionK;\n",
    "uniform vec2      ScaleIn;\n",
    "uniform vec2      ScaleOut;\n",

    "void main()\n",
    "{\n",
        "vec2  v  = (gl_TexCoord[0].xy - LensCenter) * ScaleIn;\n",
        "float rr = v.x * v.x + v.y * v.y;\n",

        "vec2 w = v * (DistortionK.x + ",
                      "DistortionK.y * rr + ",
                      "DistortionK.z * rr * rr + ",
                      "DistortionK.w * rr * rr * rr);\n",

        "vec2 t = LensCenter + ScaleOut * w;\n",
        "vec2 b = step(vec2(0.0), t) * step(t, vec2(1.0));\n",

        "gl_FragColor = b.x * b.y * texture2D(warpTexture, t);\n",
    "}\n",
};

#else

static const char *hmd_frag[] = {
    "#version 120\n",

    "uniform sampler2D warpTexture;\n",
    "uniform vec2      LensCenter;\n",
    "uniform vec4      DistortionK;\n",
    "uniform vec4      ChromAbParam;\n",
    "uniform vec2      ScaleIn;\n",
    "uniform vec2      ScaleOut;\n",

    "vec2 GetOut(vec2 v)\n",
    "{\n",
        "return LensCenter + ScaleOut * v;\n",
    "}\n",

    "void main()\n",
    "{\n",
        "vec2  v  = (gl_TexCoord[0].xy - LensCenter) * ScaleIn;\n",
        "float rr = v.x * v.x + v.y * v.y;\n",

        "vec2 w = v * (DistortionK.x + ",
                      "DistortionK.y * rr + ",
                      "DistortionK.z * rr * rr + ",
                      "DistortionK.w * rr * rr * rr);\n",

        "vec2 tb = GetOut(w * (ChromAbParam.z + ChromAbParam.w * rr));\n",
        "vec2 tr = GetOut(w * (ChromAbParam.x + ChromAbParam.y * rr));\n",
        "vec2 tg = GetOut(w);\n",

        "vec2 b = step(vec2(0.0), tb) * step(tb, vec2(1.0));\n",

        "gl_FragColor = b.x * b.y * vec4(texture2D(warpTexture, tr).r,",
                                        "texture2D(warpTexture, tg).g,",
                                        "texture2D(warpTexture, tb).b, 1.0);\n",
    "}\n",
};

#endif

void hmd_common_init(int w, int h)
{
    /* Create the off-screen frame buffers. */

    fbo_create(&L_fbo, 5 * w / 4 / 2, 5 * h / 4);
    fbo_create(&R_fbo, 5 * w / 4 / 2, 5 * h / 4);

    /* Create and initialize the distortion shader. */

    glsl_create(&distortion, sizeof (hmd_vert) / sizeof (char *), hmd_vert,
                             sizeof (hmd_frag) / sizeof (char *), hmd_frag);

    /* Initialize VBOs for the on-screen rectangles. */

    glGenBuffers_(1, &L_vbo);
    glBindBuffer_(GL_ARRAY_BUFFER, L_vbo);
    glBufferData_(GL_ARRAY_BUFFER, sizeof (L_rect), L_rect, GL_STATIC_DRAW);
    glGenBuffers_(1, &R_vbo);
    glBindBuffer_(GL_ARRAY_BUFFER, R_vbo);
    glBufferData_(GL_ARRAY_BUFFER, sizeof (R_rect), R_rect, GL_STATIC_DRAW);
    glBindBuffer_(GL_ARRAY_BUFFER, 0);
}

void hmd_common_left()
{
    glBindFramebuffer_(GL_FRAMEBUFFER, L_fbo.framebuffer);
    glViewport(0, 0, L_fbo.width, L_fbo.height);
}

void hmd_common_right()
{
    glBindFramebuffer_(GL_FRAMEBUFFER, R_fbo.framebuffer);
    glViewport(0, 0, R_fbo.width, R_fbo.height);
}

void hmd_common_draw(GLuint texture, GLuint buffer)
{
    if (buffer)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindBuffer_(GL_ARRAY_BUFFER, buffer);

        glVertexPointer  (2, GL_FLOAT, sizeof (struct point), (GLvoid *) 0);
        glTexCoordPointer(2, GL_FLOAT, sizeof (struct point), (GLvoid *) 8);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindBuffer_(GL_ARRAY_BUFFER, 0);
    }
}

void hmd_common_swap(float center,
                     float scale,
                     const float *barrel_correction,
                     const float *chroma_correction)
{
    int w = config_get_d(CONFIG_WIDTH);
    int h = config_get_d(CONFIG_HEIGHT);

    float a = (float) w / (float) h / 2;

    /* Prepare to draw a screen-filling pair of rectangles. */

    glBindFramebuffer_(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, w, h);

    glDisable(GL_BLEND);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glUseProgram_(distortion.program);
    {
        /* Set distortion parameters for both eyes. */

        glsl_uniform2f(&distortion, "ScaleOut", 0.5f / scale, 0.5f * a / scale);
        glsl_uniform2f(&distortion, "ScaleIn",  2.0f,         2.0f / a);
        glsl_uniform4f(&distortion, "DistortionK",  barrel_correction[0],
                                                    barrel_correction[1],
                                                    barrel_correction[2],
                                                    barrel_correction[3]);
        glsl_uniform4f(&distortion, "ChromAbParam", chroma_correction[0],
                                                    chroma_correction[1],
                                                    chroma_correction[2],
                                                    chroma_correction[3]);
        /* Draw the left eye. */

        glsl_uniform2f(&distortion, "LensCenter", 0.5 + 0.5f * center, 0.5);
        hmd_common_draw(L_fbo.color_texture, L_vbo);

        /* Draw the right eye.*/

        glsl_uniform2f(&distortion, "LensCenter", 0.5 - 0.5f * center, 0.5);
        hmd_common_draw(R_fbo.color_texture, R_vbo);
    }
    glUseProgram_(0);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_BLEND);
}

void hmd_common_free()
{
    fbo_delete(&L_fbo);
    fbo_delete(&R_fbo);

    glsl_delete(&distortion);

    if (L_vbo) glDeleteBuffers_(1, &L_vbo);
    if (R_vbo) glDeleteBuffers_(1, &R_vbo);
}

/*---------------------------------------------------------------------------*/
