/*
 * Copyright (C) 2003-2011 Neverball authors
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
#include <stdio.h>

#include "glext.h"
#include "log.h"

struct gl_info gli;

/*---------------------------------------------------------------------------*/

#if !ENABLE_OPENGLES && !defined(__EMSCRIPTEN__)

PFNGLCLIENTACTIVETEXTURE_PROC    glClientActiveTexture_;
PFNGLACTIVETEXTURE_PROC          glActiveTexture_;

PFNGLGENBUFFERS_PROC             glGenBuffers_;
PFNGLBINDBUFFER_PROC             glBindBuffer_;
PFNGLBUFFERDATA_PROC             glBufferData_;
PFNGLBUFFERSUBDATA_PROC          glBufferSubData_;
PFNGLDELETEBUFFERS_PROC          glDeleteBuffers_;
PFNGLISBUFFER_PROC               glIsBuffer_;

PFNGLPOINTPARAMETERF_PROC        glPointParameterf_;
PFNGLPOINTPARAMETERFV_PROC       glPointParameterfv_;

PFNGLGETSHADERIV_PROC            glGetShaderiv_;
PFNGLGETSHADERINFOLOG_PROC       glGetShaderInfoLog_;
PFNGLGETPROGRAMIV_PROC           glGetProgramiv_;
PFNGLGETPROGRAMINFOLOG_PROC      glGetProgramInfoLog_;
PFNGLCREATESHADER_PROC           glCreateShader_;
PFNGLCREATEPROGRAM_PROC          glCreateProgram_;
PFNGLSHADERSOURCE_PROC           glShaderSource_;
PFNGLCOMPILESHADER_PROC          glCompileShader_;
PFNGLDELETESHADER_PROC           glDeleteShader_;
PFNGLDELETEPROGRAM_PROC          glDeleteProgram_;
PFNGLATTACHSHADER_PROC           glAttachShader_;
PFNGLLINKPROGRAM_PROC            glLinkProgram_;
PFNGLUSEPROGRAM_PROC             glUseProgram_;
PFNGLGETUNIFORMLOCATION_PROC     glGetUniformLocation_;
PFNGLUNIFORM1F_PROC              glUniform1f_;
PFNGLUNIFORM2F_PROC              glUniform2f_;
PFNGLUNIFORM3F_PROC              glUniform3f_;
PFNGLUNIFORM4F_PROC              glUniform4f_;

PFNGLBINDFRAMEBUFFER_PROC        glBindFramebuffer_;
PFNGLDELETEFRAMEBUFFERS_PROC     glDeleteFramebuffers_;
PFNGLGENFRAMEBUFFERS_PROC        glGenFramebuffers_;
PFNGLFRAMEBUFFERTEXTURE2D_PROC   glFramebufferTexture2D_;
PFNGLCHECKFRAMEBUFFERSTATUS_PROC glCheckFramebufferStatus_;

PFNGLSTRINGMARKERGREMEDY_PROC    glStringMarkerGREMEDY_;

#endif

/*---------------------------------------------------------------------------*/

int glext_check(const char *needle)
{
    const GLubyte *haystack, *c;

    /* Search for the given string in the OpenGL extension strings. */

    for (haystack = glGetString(GL_EXTENSIONS); *haystack; haystack++)
    {
        for (c = (const GLubyte *) needle; *c && *haystack; c++, haystack++)
            if (*c != *haystack)
                break;

        if ((*c == 0) && (*haystack == ' ' || *haystack == '\0'))
            return 1;
    }

    return 0;
}

int glext_assert(const char *ext)
{
    if (!glext_check(ext))
    {
        log_printf("Missing required OpenGL extension (%s)\n", ext);
        return 0;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

#define SDL_GL_GFPA(fun, str) do {                       \
    ptr = SDL_GL_GetProcAddress(str);                    \
    if (!ptr)                                            \
        log_printf("Missing OpenGL function %s\n", str); \
    memcpy(&fun, &ptr, sizeof (void *));                 \
} while(0)

/*---------------------------------------------------------------------------*/

static void log_opengl(void)
{
    log_printf("GL vendor: %s\n"
               "GL renderer: %s\n"
               "GL version: %s\n"
               "GL extensions: %s\n",
               glGetString(GL_VENDOR),
               glGetString(GL_RENDERER),
               glGetString(GL_VERSION),
               glGetString(GL_EXTENSIONS));
}

int glext_fail(const char *title, const char *message)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, message, NULL);
    return 0;
}

int glext_init(void)
{
    memset(&gli, 0, sizeof (struct gl_info));

    /* Common init. */

    glGetIntegerv(GL_MAX_TEXTURE_SIZE,  &gli.max_texture_size);
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &gli.max_texture_units);

    if (glext_check("GL_EXT_texture_filter_anisotropic"))
        gli.texture_filter_anisotropic = 1;

    /* Desktop init. */

#if !ENABLE_OPENGLES && !defined(__EMSCRIPTEN__)
    void *ptr = 0;

    if (glext_assert("ARB_multitexture"))
    {
        SDL_GL_GFPA(glClientActiveTexture_, "glClientActiveTextureARB");
        SDL_GL_GFPA(glActiveTexture_,       "glActiveTextureARB");
    }

    if (glext_assert("ARB_vertex_buffer_object"))
    {
        SDL_GL_GFPA(glGenBuffers_,          "glGenBuffersARB");
        SDL_GL_GFPA(glBindBuffer_,          "glBindBufferARB");
        SDL_GL_GFPA(glBufferData_,          "glBufferDataARB");
        SDL_GL_GFPA(glBufferSubData_,       "glBufferSubDataARB");
        SDL_GL_GFPA(glDeleteBuffers_,       "glDeleteBuffersARB");
        SDL_GL_GFPA(glIsBuffer_,            "glIsBufferARB");
    }

    if (glext_assert("ARB_point_parameters"))
    {
        SDL_GL_GFPA(glPointParameterf_,    "glPointParameterfARB");
        SDL_GL_GFPA(glPointParameterfv_,   "glPointParameterfvARB");
    }

    if (glext_check("ARB_shader_objects"))
    {
        SDL_GL_GFPA(glGetShaderiv_,        "glGetShaderiv");
        SDL_GL_GFPA(glGetShaderInfoLog_,   "glGetShaderInfoLog");
        SDL_GL_GFPA(glGetProgramiv_,       "glGetProgramiv");
        SDL_GL_GFPA(glGetProgramInfoLog_,  "glGetProgramInfoLog");
        SDL_GL_GFPA(glCreateShader_,       "glCreateShader");
        SDL_GL_GFPA(glCreateProgram_,      "glCreateProgram");
        SDL_GL_GFPA(glShaderSource_,       "glShaderSource");
        SDL_GL_GFPA(glCompileShader_,      "glCompileShader");
        SDL_GL_GFPA(glDeleteShader_,       "glDeleteShader");
        SDL_GL_GFPA(glDeleteProgram_,      "glDeleteProgram");
        SDL_GL_GFPA(glAttachShader_,       "glAttachShader");
        SDL_GL_GFPA(glLinkProgram_,        "glLinkProgram");
        SDL_GL_GFPA(glUseProgram_,         "glUseProgram");
        SDL_GL_GFPA(glGetUniformLocation_, "glGetUniformLocation");
        SDL_GL_GFPA(glUniform1f_,          "glUniform1f");
        SDL_GL_GFPA(glUniform2f_,          "glUniform2f");
        SDL_GL_GFPA(glUniform3f_,          "glUniform3f");
        SDL_GL_GFPA(glUniform4f_,          "glUniform4f");

        gli.shader_objects = 1;
    }

    if (glext_check("ARB_framebuffer_object"))
    {
        SDL_GL_GFPA(glBindFramebuffer_,        "glBindFramebuffer");
        SDL_GL_GFPA(glDeleteFramebuffers_,     "glDeleteFramebuffers");
        SDL_GL_GFPA(glGenFramebuffers_,        "glGenFramebuffers");
        SDL_GL_GFPA(glFramebufferTexture2D_,   "glFramebufferTexture2D");
        SDL_GL_GFPA(glCheckFramebufferStatus_, "glCheckFramebufferStatus");

        gli.framebuffer_object = 1;
    }

    if (glext_check("GREMEDY_string_marker"))
        SDL_GL_GFPA(glStringMarkerGREMEDY_, "glStringMarkerGREMEDY");

#endif

    log_opengl();

    return 1;
}

/*---------------------------------------------------------------------------*/

void glClipPlane4f_(GLenum p, GLfloat a, GLfloat b, GLfloat c, GLfloat d)
{
#if ENABLE_OPENGLES

    GLfloat v[4];

    v[0] = a;
    v[1] = b;
    v[2] = c;
    v[3] = d;

    glClipPlanef(p, v);

#else

    GLdouble v[4];

    v[0] = a;
    v[1] = b;
    v[2] = c;
    v[3] = d;

    glClipPlane(p, v);

#endif
}

/*---------------------------------------------------------------------------*/
