#ifndef GLEXT_H
#define GLEXT_H

/*---------------------------------------------------------------------------*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN /* Ha ha. */
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
/* #include "/usr/share/doc/NVIDIA_GLX-1.0/include/GL/gl.h" */
#include <GL/glu.h>
#endif

#ifdef _WIN32
#include <GL/glext.h>
extern PFNGLACTIVETEXTUREPROC   glActiveTexture;
extern PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2f;
#endif

/*---------------------------------------------------------------------------*/

int glext_init(void);

/*---------------------------------------------------------------------------*/

#endif
