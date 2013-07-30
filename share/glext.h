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

/* Windows calling convention cruft. */

#ifndef APIENTRY
#define APIENTRY
#endif

#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
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

#ifndef GL_STATIC_DRAW
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
#ifndef GL_POINT_DISTANCE_ATTENUATION
#define GL_POINT_DISTANCE_ATTENUATION 0x8129
#endif

#ifndef GL_SRC0_RGB
#define GL_SRC0_RGB                   0x8580
#endif
#ifndef GL_SRC1_RGB
#define GL_SRC1_RGB                   0x8581
#endif
#ifndef GL_SRC2_RGB
#define GL_SRC2_RGB                   0x8582
#endif
#ifndef GL_SRC0_ALPHA
#define GL_SRC0_ALPHA                 0x8588
#endif

#ifndef GL_DEPTH_STENCIL
#define GL_DEPTH_STENCIL              0x84F9
#endif
#ifndef GL_DEPTH24_STENCIL8
#define GL_DEPTH24_STENCIL8           0x88F0
#endif
#ifndef GL_UNSIGNED_INT_24_8
#define GL_UNSIGNED_INT_24_8          0x84FA
#endif
#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER                0x8D40
#endif
#ifndef GL_COLOR_ATTACHMENT0
#define GL_COLOR_ATTACHMENT0          0x8CE0
#endif
#ifndef GL_DEPTH_ATTACHMENT
#define GL_DEPTH_ATTACHMENT           0x8D00
#endif
#ifndef GL_STENCIL_ATTACHMENT
#define GL_STENCIL_ATTACHMENT         0x8D20
#endif
#ifndef GL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_COMPLETE       0x8CD5
#endif

#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER              0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER            0x8B30
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS                0x8B82
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS             0x8B81
#endif
#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH            0x8B84
#endif

/*---------------------------------------------------------------------------*/

int glext_check(const char *);
int glext_init(void);

/*---------------------------------------------------------------------------*/

/* Exercise extreme paranoia in defining a cross-platform OpenGL interface.  */
/* If we're compiling against OpenGL ES then we must assume native linkage   */
/* of the extensions we use. Otherwise, GetProc them regardless of whether   */
/* they need it or not.                                                      */

#if defined(GL_VERSION_ES_CM_1_0) || \
    defined(GL_VERSION_ES_CM_1_1) || \
    defined(GL_OES_VERSION_1_0)

#define ENABLE_OPENGLES 1

#define glClientActiveTexture_ glClientActiveTexture
#define glActiveTexture_       glActiveTexture
#define glGenBuffers_          glGenBuffers
#define glBindBuffer_          glBindBuffer
#define glBufferData_          glBufferData
#define glBufferSubData_       glBufferSubData
#define glDeleteBuffers_       glDeleteBuffers
#define glIsBuffer_            glIsBuffer
#define glPointParameterfv_    glPointParameterfv

#define glOrtho_               glOrthof

#define glStringMarker_(s) ((void) (s))

#else /* No native linkage?  Define the extension API. */

#define glOrtho_               glOrtho

/*---------------------------------------------------------------------------*/
/* ARB_multitexture                                                          */

typedef void (APIENTRYP PFNGLACTIVETEXTURE_PROC)(GLenum);
typedef void (APIENTRYP PFNGLCLIENTACTIVETEXTURE_PROC)(GLenum);

extern PFNGLCLIENTACTIVETEXTURE_PROC glClientActiveTexture_;
extern PFNGLACTIVETEXTURE_PROC       glActiveTexture_;

/*---------------------------------------------------------------------------*/
/* ARB_vertex_buffer_object                                                  */

typedef void      (APIENTRYP PFNGLGENBUFFERS_PROC)(GLsizei, GLuint *);
typedef void      (APIENTRYP PFNGLBINDBUFFER_PROC)(GLenum, GLuint);
typedef void      (APIENTRYP PFNGLBUFFERDATA_PROC)(GLenum, long, const GLvoid *, GLenum);
typedef void      (APIENTRYP PFNGLBUFFERSUBDATA_PROC)(GLenum, long, long, const GLvoid *);
typedef void      (APIENTRYP PFNGLDELETEBUFFERS_PROC)(GLsizei, const GLuint *);
typedef GLboolean (APIENTRYP PFNGLISBUFFER_PROC)(GLuint);

extern PFNGLGENBUFFERS_PROC    glGenBuffers_;
extern PFNGLBINDBUFFER_PROC    glBindBuffer_;
extern PFNGLBUFFERDATA_PROC    glBufferData_;
extern PFNGLBUFFERSUBDATA_PROC glBufferSubData_;
extern PFNGLDELETEBUFFERS_PROC glDeleteBuffers_;
extern PFNGLISBUFFER_PROC      glIsBuffer_;

/*---------------------------------------------------------------------------*/
/* ARB_point_parameters                                                      */

typedef void (APIENTRYP PFNGLPOINTPARAMETERFV_PROC)(GLenum, const GLfloat *);

extern PFNGLPOINTPARAMETERFV_PROC glPointParameterfv_;

/*---------------------------------------------------------------------------*/
/* OpenGL Shading Language                                                   */

typedef void   (APIENTRYP PFNGLGETSHADERIV_PROC)(GLuint, GLenum, GLint *);
typedef void   (APIENTRYP PFNGLGETSHADERINFOLOG_PROC)(GLuint, GLsizei, GLsizei *, char *);
typedef void   (APIENTRYP PFNGLGETPROGRAMIV_PROC)(GLuint, GLenum, GLint *);
typedef void   (APIENTRYP PFNGLGETPROGRAMINFOLOG_PROC)(GLuint, GLsizei, GLsizei *, char *);
typedef GLuint (APIENTRYP PFNGLCREATESHADER_PROC)(GLenum);
typedef GLuint (APIENTRYP PFNGLCREATEPROGRAM_PROC)(void);
typedef void   (APIENTRYP PFNGLSHADERSOURCE_PROC)(GLuint, GLsizei, const char * const *, const GLint *);
typedef void   (APIENTRYP PFNGLCOMPILESHADER_PROC)(GLuint);
typedef void   (APIENTRYP PFNGLDELETESHADER_PROC)(GLuint);
typedef void   (APIENTRYP PFNGLDELETEPROGRAM_PROC)(GLuint);
typedef void   (APIENTRYP PFNGLATTACHSHADER_PROC)(GLuint, GLuint);
typedef void   (APIENTRYP PFNGLLINKPROGRAM_PROC)(GLuint);
typedef void   (APIENTRYP PFNGLUSEPROGRAM_PROC)(GLuint);
typedef GLint  (APIENTRYP PFNGLGETUNIFORMLOCATION_PROC)(GLuint, const char *);
typedef void   (APIENTRYP PFNGLUNIFORM1F_PROC)(GLint, GLfloat);
typedef void   (APIENTRYP PFNGLUNIFORM2F_PROC)(GLint, GLfloat, GLfloat);
typedef void   (APIENTRYP PFNGLUNIFORM3F_PROC)(GLint, GLfloat, GLfloat, GLfloat);
typedef void   (APIENTRYP PFNGLUNIFORM4F_PROC)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);

extern PFNGLGETSHADERIV_PROC         glGetShaderiv_;
extern PFNGLGETSHADERINFOLOG_PROC    glGetShaderInfoLog_;
extern PFNGLGETPROGRAMIV_PROC        glGetProgramiv_;
extern PFNGLGETPROGRAMINFOLOG_PROC   glGetProgramInfoLog_;
extern PFNGLCREATESHADER_PROC        glCreateShader_;
extern PFNGLCREATEPROGRAM_PROC       glCreateProgram_;
extern PFNGLSHADERSOURCE_PROC        glShaderSource_;
extern PFNGLCOMPILESHADER_PROC       glCompileShader_;
extern PFNGLDELETESHADER_PROC        glDeleteShader_;
extern PFNGLDELETEPROGRAM_PROC       glDeleteProgram_;
extern PFNGLATTACHSHADER_PROC        glAttachShader_;
extern PFNGLLINKPROGRAM_PROC         glLinkProgram_;
extern PFNGLUSEPROGRAM_PROC          glUseProgram_;
extern PFNGLGETUNIFORMLOCATION_PROC  glGetUniformLocation_;
extern PFNGLUNIFORM1F_PROC           glUniform1f_;
extern PFNGLUNIFORM2F_PROC           glUniform2f_;
extern PFNGLUNIFORM3F_PROC           glUniform3f_;
extern PFNGLUNIFORM4F_PROC           glUniform4f_;

/*---------------------------------------------------------------------------*/
/* ARB_framebuffer_object                                                    */

typedef void (APIENTRYP PFNGLBINDFRAMEBUFFER_PROC)(GLenum, GLuint);
typedef void (APIENTRYP PFNGLDELETEFRAMEBUFFERS_PROC)(GLsizei, const GLuint *);
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERS_PROC)(GLsizei, GLuint *);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2D_PROC)(GLenum, GLenum, GLenum, GLuint, GLint);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUS_PROC)(GLenum);

extern PFNGLBINDFRAMEBUFFER_PROC        glBindFramebuffer_;
extern PFNGLDELETEFRAMEBUFFERS_PROC     glDeleteFramebuffers_;
extern PFNGLGENFRAMEBUFFERS_PROC        glGenFramebuffers_;
extern PFNGLFRAMEBUFFERTEXTURE2D_PROC   glFramebufferTexture2D_;
extern PFNGLCHECKFRAMEBUFFERSTATUS_PROC glCheckFramebufferStatus_;

/*---------------------------------------------------------------------------*/
/* GREMEDY_string_marker                                                     */

typedef void (APIENTRYP PFNGLSTRINGMARKERGREMEDY_PROC)(GLsizei, const void *);

extern PFNGLSTRINGMARKERGREMEDY_PROC glStringMarkerGREMEDY_;

#define glStringMarker_(s)      \
    if (glStringMarkerGREMEDY_) \
        glStringMarkerGREMEDY_(0, (s))

/*---------------------------------------------------------------------------*/
#endif /* !ENABLE_OPENGLES */

void glClipPlane4f_(GLenum, GLfloat, GLfloat, GLfloat, GLfloat);

/*---------------------------------------------------------------------------*/

struct gl_info
{
    GLint max_texture_units;
    GLint max_texture_size;

    unsigned int multitexture:1;
    unsigned int vertex_buffer_object:1;
    unsigned int point_parameters:1;
    unsigned int shader_objects:1;
    unsigned int framebuffer_object:1;
};

extern struct gl_info gli;

/*---------------------------------------------------------------------------*/
#endif
