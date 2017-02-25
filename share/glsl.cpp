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

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "glext.h"
#include "glsl.h"
#include "log.h"

/*----------------------------------------------------------------------------*/

#if ENABLE_OPENGLES

/* OpenGL ES support in Neverball is targeted toward OpenGL ES version 1.1.   */
/* This version of ES has no support for programmable shading.                */

GLboolean glsl_create(glsl *G, int vertc, const char *const *vertv,
                               int fragc, const char *const *fragv)
{
    return GL_FALSE;
}

void glsl_delete(glsl *G) { }

void glsl_uniform1f(glsl *G, const char *name, GLfloat a) { }
void glsl_uniform2f(glsl *G, const char *name, GLfloat a, GLfloat b) { }
void glsl_uniform3f(glsl *G, const char *name, GLfloat a, GLfloat b, GLfloat c) { }
void glsl_uniform4f(glsl *G, const char *name, GLfloat a, GLfloat b, GLfloat c, GLfloat d) { }

#else

/*----------------------------------------------------------------------------*/

static int check_shader_log(GLuint shader)
{
    char *p = 0;
    GLint s = 0;
    GLint n = 0;

    /* Check the shader compile status.  If failed, print the log. */

    glGetShaderiv_(shader, GL_COMPILE_STATUS,  &s);
    glGetShaderiv_(shader, GL_INFO_LOG_LENGTH, &n);

    if (s == 0)
    {
        if ((p = (char *) calloc(n + 1, 1)))
        {
            glGetShaderInfoLog_(shader, n, NULL, p);

            log_printf("OpenGL Shader Error:\n%s", p);
            free(p);
        }
        return 0;
    }
    return 1;
}

static int check_program_log(GLuint program)
{
    char *p = 0;
    GLint s = 0;
    GLint n = 0;

    /* Check the program link status.  If failed, print the log. */

    glGetProgramiv_(program, GL_LINK_STATUS,     &s);
    glGetProgramiv_(program, GL_INFO_LOG_LENGTH, &n);

    if (s == 0)
    {
        if ((p = (char *) calloc(n + 1, 1)))
        {
            glGetProgramInfoLog_(program, n, NULL, p);

            log_printf("OpenGL Program Error:\n%s", p);
            free(p);
        }
        return 0;
    }
    return 1;
}

/*----------------------------------------------------------------------------*/

static GLuint glsl_init_shader(GLenum type, int strc, const char *const *strv)
{
    /* Compile a new shader with the given source. */

    GLuint shader = glCreateShader_(type);

    glShaderSource_ (shader, strc, strv, NULL);
    glCompileShader_(shader);

    /* If the shader is valid, return it.  Else, delete it. */

    if (check_shader_log(shader))
        return shader;
    else
        glDeleteShader_(shader);

    return 0;
}

static GLuint glsl_init_program(GLuint shader_vert,
                                GLuint shader_frag)
{
    /* Link a new program object. */

    GLuint program = glCreateProgram_();

    glAttachShader_(program, shader_vert);
    glAttachShader_(program, shader_frag);

    glLinkProgram_(program);

    /* If the program is valid, return it.  Else, delete it. */

    if (check_program_log(program))
        return program;
    else
        glDeleteProgram_(program);

    return 0;
}

/*----------------------------------------------------------------------------*/

GLboolean glsl_create(glsl *G, int vertc, const char *const *vertv,
                               int fragc, const char *const *fragv)
{
    if (gli.shader_objects == 0) return GL_FALSE;

    /* Compile the shaders. */

    G->vert_shader = glsl_init_shader(GL_VERTEX_SHADER,   vertc, vertv);
    G->frag_shader = glsl_init_shader(GL_FRAGMENT_SHADER, fragc, fragv);

    /* Link the program. */

    if (G->vert_shader && G->frag_shader)
    {
        G->program = glsl_init_program(G->vert_shader, G->frag_shader);
        return GL_TRUE;
    }
    return GL_FALSE;
}

void glsl_delete(glsl *G)
{
    if (gli.shader_objects == 0) return;

    /* Delete the program and shaders. */

    if (G->program)     glDeleteProgram_(G->program);
    if (G->frag_shader) glDeleteShader_ (G->frag_shader);
    if (G->vert_shader) glDeleteShader_ (G->vert_shader);

    memset(G, 0, sizeof (glsl));
}

void glsl_uniform1f(glsl *G, const char *name, GLfloat a)
{
    if (gli.shader_objects)
        glUniform1f_(glGetUniformLocation_(G->program, name), a);
}

void glsl_uniform2f(glsl *G, const char *name, GLfloat a, GLfloat b)
{
    if (gli.shader_objects)
        glUniform2f_(glGetUniformLocation_(G->program, name), a, b);
}

void glsl_uniform3f(glsl *G, const char *name, GLfloat a, GLfloat b, GLfloat c)
{
    if (gli.shader_objects)
        glUniform3f_(glGetUniformLocation_(G->program, name), a, b, c);
}

void glsl_uniform4f(glsl *G, const char *name, GLfloat a, GLfloat b, GLfloat c, GLfloat d)
{
    if (gli.shader_objects)
        glUniform4f_(glGetUniformLocation_(G->program, name), a, b, c, d);
}

#endif

