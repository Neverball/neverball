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

#include "mtrl.h"
#include "array.h"
#include "common.h"
#include "image.h"
#include "lang.h"

/*---------------------------------------------------------------------------*/

/*
 * Material cache.
 *
 * These routines load materials from SOL files into a common cache
 * and create mappings from internal SOL indices to this cache. They
 * can also reload materials from material specification files.
 *
 * Some unresolved stuff:
 *
 * Duplicates are ignored. SOLs define materials internally, so
 * conflicts can arise between different SOLs. Right now materials
 * start out with the values from the first cached SOL. If the values
 * are bad, every subsequent SOL will use them. On the upside,
 * duplicates are uncommon.
 */

static Array mtrls;

static struct b_mtrl default_base_mtrl =
{
    { 0.8f, 0.8f, 0.8f, 1.0f },
    { 0.2f, 0.2f, 0.2f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f }, 0.0f, 0, ""
};

int default_mtrl;

/*
 * Obtain a mtrl ref by name.
 */
static int find_mtrl(const char *name)
{
    int i, c = array_len(mtrls);

    for (i = 0; i < c; i++)
    {
        struct mtrl *mp = array_get(mtrls, i);

        if (mp->refc > 0 && strcmp(name, mp->base.f) == 0)
            return i;
    }
    return -1;
}

/*
 * Load a material texture.
 */
static GLuint find_texture(const char *name)
{
    char path[MAXSTR];
    GLuint o;
    int i;

    for (i = 0; i < ARRAYSIZE(tex_paths); i++)
    {
        CONCAT_PATH(path, &tex_paths[i], name);

        if ((o = make_image_from_file(path, IF_MIPMAP)))
            return o;
    }
    return 0;
}

/*
 * Load a material from a base material.
 */
static void load_mtrl(struct mtrl *mp, const struct b_mtrl *base)
{
    /* Copy the base material. */

    memcpy(&mp->base, base, sizeof (struct b_mtrl));

    /* Load the texture. */

    if ((mp->o = find_texture(_(base->f))))
    {
        /* Set the texture to clamp or repeat based on material type. */

        if (base->fl & M_CLAMP_S)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        else
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

        if (base->fl & M_CLAMP_T)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        else
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    /* Cache the 32-bit material values for quick comparison. */

    mp->d = touint(base->d);
    mp->a = touint(base->a);
    mp->s = touint(base->s);
    mp->e = touint(base->e);
    mp->h = tobyte(base->h[0]);
}

/*
 * Free material resources.
 */
static void free_mtrl(struct mtrl *mp)
{
    glDeleteTextures(1, &mp->o);
}

/*
 * Cache a single material.
 */
int mtrl_cache(const struct b_mtrl *base)
{
    struct mtrl *mp;

    int mi = find_mtrl(base->f);

    if (mi < 0)
    {
        int i, c = array_len(mtrls);

        /* Find an empty slot. */

        for (i = 0; i < c; i++)
        {
            mp = array_get(mtrls, i);

            if (mp->refc == 0)
            {
                load_mtrl(mp, base);
                mp->refc++;
                return i;
            }
        }

        /* Allocate a new slot. */

        if ((mp = array_add(mtrls)))
        {
            memset(mp, 0, sizeof (*mp));
            load_mtrl(mp, base);
            mp->refc++;
            return array_len(mtrls) - 1;
        }
    }
    else
    {
        mp = array_get(mtrls, mi);
        mp->refc++;
    }

    return mi;
}

/*
 * Free a cached material.
 */
void mtrl_free(int mi)
{
    if (mtrls)
    {
        struct mtrl *mp = array_get(mtrls, mi);

        if (mp->refc > 0)
        {
            mp->refc--;

            if (mp->refc == 0)
                free_mtrl(mp);
        }
    }
}

/*
 * Obtain a material pointer.
 */
struct mtrl *mtrl_get(int mi)
{
    return mtrls ? array_get(mtrls, mi) : NULL;
}

/*
 * Cache SOL materials.
 */
void mtrl_cache_sol(struct s_base *fp)
{
    if (fp->mtrls)
    {
        free(fp->mtrls);
        fp->mtrls = NULL;
    }

    if ((fp->mtrls = calloc(fp->mc, sizeof (*fp->mtrls))))
    {
        int mi;

        for (mi = 0; mi < fp->mc; mi++)
            fp->mtrls[mi] = mtrl_cache(&fp->mv[mi]);
    }
}

/*
 * Free cached materials.
 */
void mtrl_free_sol(struct s_base *fp)
{
    if (fp)
        if (fp->mtrls)
        {
            int mi;

            for (mi = 0; mi < fp->mc; mi++)
                mtrl_free(fp->mtrls[mi]);

            free(fp->mtrls);
            fp->mtrls = NULL;
        }
}

/*
 * Initialize material cache.
 */
void mtrl_init(void)
{
    mtrl_quit();

    if ((mtrls = array_new(sizeof (struct mtrl))))
    {
        /* Cache the default material at index 0. */

        default_mtrl = mtrl_cache(&default_base_mtrl);
    }
}

/*
 * Destroy material cache.
 */
void mtrl_quit(void)
{
    if (mtrls)
    {
        int i, c = array_len(mtrls);

        for (i = 0; i < c; i++)
            free_mtrl(array_get(mtrls, i));

        array_free(mtrls);
        mtrls = NULL;
    }
}

