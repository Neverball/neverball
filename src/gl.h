#ifndef GL_H
#define GL_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#ifdef _WIN32
#include <GL/glext.h>
extern PFNGLACTIVETEXTUREPROC   glActiveTextureARB;
extern PFNGLMULTITEXCOORD2DPROC glMultiTexCoord2dARB;
#endif

#endif
