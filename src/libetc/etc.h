#ifndef ETC_H
#define ETC_H

#include <GL/gl.h>

void   etc_size_jpg(const char *, int *, int *);
void  *etc_load_jpg(const char *, int *, int *, int *);
void  *etc_load_png(const char *, int *, int *, int *);

GLuint etc_make_tex(const void *, int, int, int);
void   etc_draw_tex(GLuint, double, double, double, double, double);

void   etc_proj_identity(void);

#endif
