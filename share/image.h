#ifndef IMAGE_H
#define IMAGE_H

#include <SDL.h>
#include <SDL_ttf.h>

#include "glext.h"

/*---------------------------------------------------------------------------*/

void   image_snap(char *);
void   image_size(int *, int *, int, int);

void         image_swab (SDL_Surface *);
void         image_white(SDL_Surface *);
SDL_Surface *image_scale(SDL_Surface *, int);

GLuint make_image_from_surf(int *, int *, SDL_Surface *);
GLuint make_image_from_file(int *, int *,
                            int *, int *, const char *);
GLuint make_image_from_font(int *, int *,
                            int *, int *, const char *, TTF_Font *, int);

/*---------------------------------------------------------------------------*/

#endif
