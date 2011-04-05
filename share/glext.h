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

#ifndef GLEXT_H
#define GLEXT_H

/*---------------------------------------------------------------------------*/
/* Include the system OpenGL headers.                                        */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#ifdef _WIN32
#include <GL/glext.h>
#endif

/*---------------------------------------------------------------------------*/

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE                0x809D
#endif

#ifndef GL_TEXTURE0
#define GL_TEXTURE0                   0x84C0
#endif

#ifndef GL_TEXTURE1
#define GL_TEXTURE1                   0x84C1
#endif

#ifndef GL_TEXTURE2
#define GL_TEXTURE2                   0x84C2
#endif

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER               0x8892
#endif

#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER       0x8893
#endif

#ifndef GL_STATC_DRAW
#define GL_STATIC_DRAW                0x88E4
#endif

#ifndef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW               0x88E8
#endif

#ifndef GL_POINT_SPRITE
#define GL_POINT_SPRITE               0x8861
#endif

#ifndef GL_COORD_REPLACE
#define GL_COORD_REPLACE              0x8862
#endif

#ifndef GL_POINT_DISTANCE_ATTENUATIAN
#define GL_POINT_DISTANCE_ATTENUATION 0x8129
#endif

/*---------------------------------------------------------------------------*/

int  glext_check(const char *);
void glext_init(void);

/*---------------------------------------------------------------------------*/

/* Exercise extreme paranoia in defining a cross-platform OpenGL interface.  */
/* If we're compiling against OpenGL ES then we must assume native linkage   */
/* of the extensions we use. Otherwise, GetProc them regardless of whether   */
/* they need it or not.                                                      */

#if defined(GL_VERSION_ES_CM_1_0) || \
    defined(GL_VERSION_ES_CM_1_1) || \
    defined(GL_OES_VERSION_1_0)

#define CONF_OPENGLES 1

#define glClientActiveTexture_ glClientActiveTexture
#define glActiveTexture_       glActiveTexture
#define glGenBuffers_          glGenBuffers
#define glBindBuffer_          glBindBuffer
#define glBufferData_          glBufferData
#define glBufferSubData_       glBufferSubData
#define glDeleteBuffers_       glDeleteBuffers
#define glIsBuffer_            glIsBuffer
#define glPointParameterfv_    glPointParameterfv

#define glOrtho_               glOrtho

#else /* No native linkage?  Define the extension API. */

#define glOrtho_               glOrtho

/*---------------------------------------------------------------------------*/
/* ARB_multitexture                                                          */

typedef void (*PFNGLACTIVETEXTURE_PROC)(GLenum);
typedef void (*PFNGLCLIENTACTIVETEXTURE_PROC)(GLenum);

extern PFNGLCLIENTACTIVETEXTURE_PROC glClientActiveTexture_;
extern PFNGLACTIVETEXTURE_PROC       glActiveTexture_;

/*---------------------------------------------------------------------------*/
/* ARB_vertex_buffer_object                                                  */

typedef void      (*PFNGLGENBUFFERS_PROC)(GLsizei, GLuint *);
typedef void      (*PFNGLBINDBUFFER_PROC)(GLenum, GLuint);
typedef void      (*PFNGLBUFFERDATA_PROC)(GLenum, GLsizeiptr, const GLvoid *, GLenum);
typedef void      (*PFNGLBUFFERSUBDATA_PROC)(GLenum, GLintptr, GLsizeiptr, const GLvoid *);
typedef void      (*PFNGLDELETEBUFFERS_PROC)(GLsizei, const GLuint *);
typedef GLboolean (*PFNGLISBUFFER_PROC)(GLuint);

extern PFNGLGENBUFFERS_PROC    glGenBuffers_;
extern PFNGLBINDBUFFER_PROC    glBindBuffer_;
extern PFNGLBUFFERDATA_PROC    glBufferData_;
extern PFNGLBUFFERSUBDATA_PROC glBufferSubData_;
extern PFNGLDELETEBUFFERS_PROC glDeleteBuffers_;
extern PFNGLISBUFFER_PROC      glIsBuffer_;

/*---------------------------------------------------------------------------*/
/* ARB_point_parameters                                                      */

typedef void (*PFNGLPOINTPARAMETERFV_PROC)(GLenum, const GLfloat *);

extern PFNGLPOINTPARAMETERFV_PROC glPointParameterfv_;

/*---------------------------------------------------------------------------*/
#endif /* !CONF_OPENGLES */

void glClipPlane4f(GLenum, GLfloat, GLfloat, GLfloat, GLfloat);

/*---------------------------------------------------------------------------*/
#endif
