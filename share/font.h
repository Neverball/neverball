#ifndef FONT_H
#define FONT_H

#if _WIN32
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_rwops.h>
#else
#include <SDL_ttf.h>
#include <SDL_rwops.h>
#endif

#include "base_config.h"

struct font
{
    char path[PATHMAX];

    TTF_Font  *ttf[3];
    SDL_RWops *rwops;
    void      *data;
    int        datalen;
};

int  font_load(struct font *, const char *path, int sizes[3]);
void font_free(struct font *);

int  font_init(void);
void font_quit(void);

#endif
