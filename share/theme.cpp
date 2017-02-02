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

#include "theme.h"
#include "config.h"
#include "image.h"
#include "video.h"

/*---------------------------------------------------------------------------*/

static const char theme_images[THEME_IMAGES_MAX][PATHMAX] = {
    "back-plain.png",       /* off and inactive    */
    "back-plain-focus.png", /* off and   active    */
    "back-hilite.png",      /* on  and inactive    */
    "back-hilite-focus.png" /* on  and   active    */
};

static GLuint theme_image(const char *path)
{
    const int W = video.device_w;
    const int H = video.device_h;

    int W2, H2;

    int w, h, b;
    void *p;

    GLuint o = 0;

    /*
     * Disable mipmapping and do a manual downscale.  Heuristic for
     * downscaling the texture: assume target size to be roughly 1/16
     * of a full screen texture, smallest size being 32x32.
     */

    image_near2(&W2, &H2, W, H);

    W2 = MAX(W2 / 16, 32);
    H2 = MAX(H2 / 16, 32);

    if ((p = image_load(path, &w, &h, &b)))
    {
        void *q;

        /* Prefer a small scale factor. */

        int s = MAX(w, h) / MAX(W2, H2);

        if (s > 1 && (q = image_scale(p, w, h, b, &w, &h, s)))
        {
            free(p);
            p = q;
        }

        o = make_texture(p, w, h, b, 0);

        free(p);
        p = NULL;
    }

    return o;
}

static const char *theme_path(const char *name, const char *file)
{
    static char path[MAXSTR];

    if ((name && *name) && (file && *file))
    {
        SAFECPY(path, "gui/");
        SAFECAT(path, name);
        SAFECAT(path, "/");
        SAFECAT(path, file);
        return path;
    }
    return "";
}

int theme_load(struct theme *theme, const char *name)
{
    char buff[MAXSTR];
    fs_file fp;

    float s[4] = { 0.25f, 0.25f, 0.25f, 0.25f };

    int i;

    if (theme && name && *name)
    {
        memset(theme, 0, sizeof (*theme));

        /* Load description. */

        if ((fp = fs_open(theme_path(name, "theme.txt"), "r")))
        {
            while ((fs_gets(buff, sizeof (buff), fp)))
            {
                strip_newline(buff);

                if (strncmp(buff, "slice ", 6) == 0)
                    sscanf(buff + 6, "%f %f %f %f", &s[0], &s[1], &s[2], &s[3]);
            }

            fs_close(fp);
        }
        else
        {
            log_printf("Failure to open \"%s\" theme file\n", name);
        }

        theme->s[0] =  0.0f;
        theme->s[1] =  s[0];
        theme->s[2] = (1.0f - s[1]);
        theme->s[3] =  1.0f;

        theme->t[0] =  1.0f;
        theme->t[1] = (1.0f - s[2]);
        theme->t[2] =  s[3];
        theme->t[3] =  0.0f;

        /* Load textures. */

        for (i = 0; i < ARRAYSIZE(theme_images); i++)
            theme->tex[i] = theme_image(theme_path(name, theme_images[i]));

        return 1;
    }
    return 0;
}

void theme_free(struct theme *theme)
{
    int i;

    if (theme)
    {
        glDeleteTextures(THEME_IMAGES_MAX, theme->tex);

        for (i = 0; i < THEME_IMAGES_MAX; i++)
            theme->tex[i] = 0;
    }
}

/*---------------------------------------------------------------------------*/
