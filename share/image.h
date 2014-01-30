#ifndef IMAGE_H
#define IMAGE_H

#include <SDL.h>
#include <SDL_ttf.h>

#include "glext.h"
#include "base_image.h"

/*---------------------------------------------------------------------------*/

#define IF_MIPMAP 0x01

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RMASK 0xFF000000
#define GMASK 0x00FF0000
#define BMASK 0x0000FF00
#define AMASK 0x000000FF
#else
#define RMASK 0x000000FF
#define GMASK 0x0000FF00
#define BMASK 0x00FF0000
#define AMASK 0xFF000000
#endif

void   image_snap(const char *);

GLuint make_image_from_file(const char *, int);
GLuint make_image_from_font(int *, int *,
                            int *, int *, const char *, TTF_Font *, int);
GLuint make_texture(const void *, int, int, int, int);

SDL_Surface *load_surface(const char *);

/*---------------------------------------------------------------------------*/

#endif
