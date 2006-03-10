#ifndef BASE_IMAGE_H
#define BASE_IMAGE_H

#include <SDL.h>

/*---------------------------------------------------------------------------*/

void         image_swab (SDL_Surface *);
void         image_white(SDL_Surface *);
SDL_Surface *image_scale(SDL_Surface *, int);

/*---------------------------------------------------------------------------*/

#endif
