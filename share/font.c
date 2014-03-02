/*
 * Copyright (C) 2014 Neverball authors
 *
 * NEVERBALL is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

#include <string.h>
#include <stdlib.h>

#include <SDL_ttf.h>
#include <SDL_rwops.h>

#include "font.h"
#include "common.h"
#include "fs.h"

/*---------------------------------------------------------------------------*/

int font_load(struct font *ft, const char *path, int sizes[3])
{
    if (ft && path && *path)
    {
        memset(ft, 0, sizeof (*ft));

        if ((ft->data = fs_load(path, &ft->datalen)))
        {
            int i;

            SAFECPY(ft->path, path);

            ft->rwops = SDL_RWFromConstMem(ft->data, ft->datalen);

            for (i = 0; i < ARRAYSIZE(ft->ttf); i++)
            {
                SDL_RWseek(ft->rwops, 0, SEEK_SET);
                ft->ttf[i] = TTF_OpenFontRW(ft->rwops, 0, sizes[i]);
            }
            return 1;
        }
    }
    return 0;
}

void font_free(struct font *ft)
{
    if (ft)
    {
        int i;

        for (i = 0; i < ARRAYSIZE(ft->ttf); i++)
            if (ft->ttf[i])
                TTF_CloseFont(ft->ttf[i]);

        if (ft->rwops)
            SDL_RWclose(ft->rwops);

        if (ft->data)
            free(ft->data);

        memset(ft, 0, sizeof (*ft));
    }
}

int font_init(void)
{
    return (TTF_Init() == 0);
}

void font_quit(void)
{
    TTF_Quit();
}

/*---------------------------------------------------------------------------*/
