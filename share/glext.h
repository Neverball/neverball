#ifndef GLEXT_H
#define GLEXT_H

/*---------------------------------------------------------------------------*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN /* Ha ha. */
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

#ifndef GL_ARB_multitexture
#define GL_TEXTURE0_ARB                   0x84C0
#define GL_TEXTURE1_ARB                   0x84C1

typedef void (*PFNGLACTIVETEXTUREARBPROC)(GLenum);
#endif

extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB_;

/*---------------------------------------------------------------------------*/

#endif
