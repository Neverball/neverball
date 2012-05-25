#ifndef IMAGE_H
#define IMAGE_H

#include <SDL.h>
#include <SDL_ttf.h>

#include "glext.h"
#include "base_image.h"

/*---------------------------------------------------------------------------*/

#define IF_MIPMAP 0x01

void   image_snap(const char *);

GLuint make_image_from_file(const char *, int);
GLuint make_image_from_font(int *, int *,
                            int *, int *, const char *, TTF_Font *, int);
GLuint make_texture(const void *, int, int, int, int);

SDL_Surface *load_surface(const char *);

/*---------------------------------------------------------------------------*/

#endif
