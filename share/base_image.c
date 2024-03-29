/*
 * Copyright (C) 2003 Robert Kooima
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

#include <png.h>
#include <jpeglib.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <setjmp.h>

#include "base_config.h"
#include "base_image.h"

#include "fs.h"
#include "fs_png.h"
#include "fs_jpg.h"

/*---------------------------------------------------------------------------*/

void image_size(int *W, int *H, int w, int h)
{
    /* Round the image size up to the next power-of-two. */

    *W = w ? 1 : 0;
    *H = h ? 1 : 0;

    while (*W < w) *W *= 2;
    while (*H < h) *H *= 2;
}

void image_near2(int *W, int *H, int w, int h)
{
    image_size(W, H, w, h);

    if (*W > 1 && (*W - w > w - (*W / 2)))
        *W /= 2;
    if (*H > 1 && (*H - h > h - (*H / 2)))
        *H /= 2;
}

/*---------------------------------------------------------------------------*/

static void *image_load_png(const char *filename, int *width,
                                                  int *height,
                                                  int *bytes)
{
    fs_file fh;

    png_structp readp = NULL;
    png_infop   infop = NULL;
    png_bytep  *bytep = NULL;
    unsigned char  *p = NULL;

    /* Initialize all PNG import data structures. */

    if (!(fh = fs_open_read(filename)))
        return NULL;

    if (!(readp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        return NULL;

    if (!(infop = png_create_info_struct(readp)))
        return NULL;

    /* Enable the default PNG error handler. */

    if (setjmp(png_jmpbuf(readp)) == 0)
    {
        int w, h, b, i;

        /* Read the PNG header. */

        png_set_read_fn(readp, fh, fs_png_read);
        png_read_info(readp, infop);

        png_set_expand(readp);
        png_set_strip_16(readp);
        png_set_packing(readp);

        png_read_update_info(readp, infop);

        /* Extract and check image properties. */

        w = (int) png_get_image_width (readp, infop);
        h = (int) png_get_image_height(readp, infop);

        switch (png_get_color_type(readp, infop))
        {
        case PNG_COLOR_TYPE_GRAY:       b = 1; break;
        case PNG_COLOR_TYPE_GRAY_ALPHA: b = 2; break;
        case PNG_COLOR_TYPE_RGB:        b = 3; break;
        case PNG_COLOR_TYPE_RGB_ALPHA:  b = 4; break;

        default: longjmp(png_jmpbuf(readp), -1);
        }

        if (!(bytep = png_malloc(readp, h * sizeof (png_bytep))))
            longjmp(png_jmpbuf(readp), -1);

        /* Allocate the final pixel buffer and read pixels there. */

        if ((p = (unsigned char *) malloc(w * h * b)))
        {
            for (i = 0; i < h; i++)
                bytep[i] = p + w * b * (h - i - 1);

            png_read_image(readp, bytep);
            png_read_end(readp, NULL);

            if (width)  *width  = w;
            if (height) *height = h;
            if (bytes)  *bytes  = b;
        }

        png_free(readp, bytep);
    }
    else p = NULL;

    /* Free all resources. */

    png_destroy_read_struct(&readp, &infop, NULL);
    fs_close(fh);

    return p;
}

/*
 * JPEG error manager wrapper so that we don't exit() on JPEG errors.
 */
struct image_jpg_error
{
    struct jpeg_error_mgr mgr;

    jmp_buf setjmp_buffer;
};

static void image_jpg_error_exit(j_common_ptr cinfo)
{
    struct image_jpg_error *err = (struct image_jpg_error *) cinfo->err;

    cinfo->err->output_message(cinfo);

    longjmp(err->setjmp_buffer, 1);
}

static void *image_load_jpg(const char *filename, int *width,
                                                  int *height,
                                                  int *bytes)
{
    unsigned char *p = NULL;
    fs_file fp;

    if ((fp = fs_open_read(filename)))
    {
        struct jpeg_decompress_struct cinfo;
        struct image_jpg_error err;

        int w, h, b, i = 0;

        /* Initialize the JPG decompressor. */

        memset(&cinfo, 0, sizeof (cinfo));

        /* Install our error_exit replacement. */

        cinfo.err = jpeg_std_error(&err.mgr);

        err.mgr.error_exit = image_jpg_error_exit;

        if (setjmp(err.setjmp_buffer) == 0)
        {
            jpeg_create_decompress(&cinfo);

            /* Set up a VFS source manager. */

            fs_jpg_src(&cinfo, fp);

            /* Grab the JPG header info. */

            jpeg_read_header(&cinfo, TRUE);
            jpeg_start_decompress(&cinfo);

            w = cinfo.output_width;
            h = cinfo.output_height;
            b = cinfo.output_components;

            /* Allocate the final pixel buffer and copy pixels there. */

            if ((p = (unsigned char *) malloc (w * h * b)))
            {
                while (cinfo.output_scanline < cinfo.output_height)
                {
                    unsigned char *buffer = p + w * b * (h - i - 1);
                    i += jpeg_read_scanlines(&cinfo, &buffer, 1);
                }

                if (width)  *width  = w;
                if (height) *height = h;
                if (bytes)  *bytes  = b;
            }

            jpeg_finish_decompress(&cinfo);
        }
        jpeg_destroy_decompress(&cinfo);

        fs_close(fp);
    }

    return p;
}

void *image_load(const char *filename, int *width,
                                       int *height,
                                       int *bytes)
{
    if (filename && strlen(filename) > 4)
    {
        const char *ext = filename + strlen(filename) - 4;

        if      (strcmp(ext, ".png") == 0 || strcmp(ext, ".PNG") == 0)
            return image_load_png(filename, width, height, bytes);
        else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".JPG") == 0)
            return image_load_jpg(filename, width, height, bytes);
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/

/*
 * Allocate and return a power-of-two image buffer with the given pixel buffer
 * centered within in.
 */
void *image_next2(const void *p, int w, int h, int b, int *w2, int *h2)
{
    unsigned char *src = (unsigned char *) p;
    unsigned char *dst = NULL;

    int W;
    int H;

    image_size(&W, &H, w, h);

    if ((dst = (unsigned char *) calloc(W * H * b, sizeof (unsigned char))))
    {
        const int dr = (H - h) / 2;
        const int dc = (W - w) / 2;

        int r;

        for (r = 0; r < h; ++r)
        {
            const int R = r + dr;
            const int C =     dc;

            memcpy(&dst[(R * W + C) * b], &src[(r * w) * b], w * b);
        }

        if (w2) *w2 = W;
        if (h2) *h2 = H;
    }

    return dst;
}

/*
 * Allocate and return a new down-sampled image buffer.
 */
void *image_scale(const void *p, int w, int h, int b, int *wn, int *hn, int n)
{
    unsigned char *src = (unsigned char *) p;
    unsigned char *dst = NULL;

    int W = w / n;
    int H = h / n;

    if ((dst = (unsigned char *) calloc(W * H * b, sizeof (unsigned char))))
    {
        int si, di;
        int sj, dj;
        int i;

        /* Iterate each component of each destination pixel. */

        for (di = 0; di < H; di++)
            for (dj = 0; dj < W; dj++)
                for (i = 0; i < b; i++)
                {
                    int c = 0;

                    /* Average the NxN source pixel block for each. */

                    for (si = di * n; si < (di + 1) * n; si++)
                        for (sj = dj * n; sj < (dj + 1) * n; sj++)
                            c += src[(si * w + sj) * b + i];

                    dst[(di * W + dj) * b + i] =
                        (unsigned char) (c / (n * n));
                }

        if (wn) *wn = W;
        if (hn) *hn = H;
    }

    return dst;
}

/*
 * Whiten the RGB channels of the given image without touching any alpha.
 */
void image_white(void *p, int w, int h, int b)
{
    unsigned char *s = (unsigned char *) p;

    int i;

    assert(b >= 1 && b <= 4);

    if (b == 1 || b == 3)
    {
        memset(s, 0xFF, w * h * b);
    }
    else if (b == 2)
    {
        for (i = 0; i < w * h * b; i += 2)
            s[i] = 0xFF;
    }
    else
    {
        for (i = 0; i < w * h * b; i += 4)
        {
            s[i + 0] = 0xFF;
            s[i + 1] = 0xFF;
            s[i + 2] = 0xFF;
        }
    }
}

/*
 * Allocate and return an image buffer of the given image flipped horizontally
 * and/or vertically.
 */
void *image_flip(const void *p, int w, int h, int b, int hflip, int vflip)
{
    unsigned char *q;

    assert(hflip || vflip);

    if (!p)
        return NULL;

    if ((q = malloc(w * b * h)))
    {
        int r, c, i;

        for (r = 0; r < h; r++)
            for (c = 0; c < w; c++)
                for (i = 0; i < b; i++)
                {
                    int pr = vflip ? h - r - 1 : r;
                    int pc = hflip ? w - c - 1 : c;

                    int qi = r  * w * b + c  * b + i;
                    int pi = pr * w * b + pc * b + i;

                    q[qi] = ((const unsigned char *) p)[pi];
                }
        return q;
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/
