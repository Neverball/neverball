#ifndef AUX_H
#define AUX_H

#include <GL/gl.h>

void   aux_size_jpg(const char *, int *, int *);
void  *aux_load_jpg(const char *, int *, int *, int *);
void  *aux_load_png(const char *, int *, int *, int *);

GLuint aux_make_tex(const void *, int, int, int);
void   aux_draw_tex(GLuint, double, double, double, double, double);

void   aux_proj_identity();

#endif
