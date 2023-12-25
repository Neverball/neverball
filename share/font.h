#ifndef FONT_H
#define FONT_H

#include <SDL_ttf.h>
#include <SDL_rwops.h>

#include "base_config.h"

enum
{
    FONT_SIZE_MIN = 0,

    FONT_SIZE_TNY = FONT_SIZE_MIN,
    FONT_SIZE_SML,
    FONT_SIZE_TCH,
    FONT_SIZE_MED,
    FONT_SIZE_LRG,

    FONT_SIZE_MAX
};

struct font
{
    char path[PATHMAX];

    TTF_Font  *ttf[FONT_SIZE_MAX];
    SDL_RWops *rwops;
    void      *data;
    int        datalen;
};

int  font_load(struct font *, const char *path, int sizes[FONT_SIZE_MAX]);
void font_free(struct font *);

int  font_init(void);
void font_quit(void);

#endif
