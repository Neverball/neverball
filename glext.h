#ifndef GLEXT_H
#define GLEXT_H

/*---------------------------------------------------------------------------*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define GL_GLEXT_LEGACY

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#ifdef _WIN32
#include <GL/glext.h>
extern PFNGLACTIVETEXTUREPROC   glActiveTextureARB;
extern PFNGLMULTITEXCOORD2DPROC glMultiTexCoord2dARB;
#endif

/* Certain buggy drivers don't handle multitexture well.  Comment this in. */
/*
#undef GL_ARB_multitexture
*/
/*---------------------------------------------------------------------------*/

void glext_init(void);

/*---------------------------------------------------------------------------*/

#endif
