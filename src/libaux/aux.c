#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>

#include <jpeglib.h>
#include <png.h>

/*---------------------------------------------------------------------------*/

void aux_size_jpg(const char *s, int *w, int *h)
{
    FILE *fin;

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr         jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    if ((fin = fopen(s, "r")))
    {
        jpeg_stdio_src(&cinfo, fin);
        jpeg_read_header(&cinfo, TRUE);

        *w = cinfo.image_width;
        *h = cinfo.image_height;
        
        fclose(fin);
    }

    jpeg_destroy_decompress(&cinfo);
}

/*---------------------------------------------------------------------------*/

void *aux_load_jpg(const char *s, int *w, int *h, int *b)
{
    unsigned char *p = NULL;
    unsigned char *q = NULL;
    FILE *fin;

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr         jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    if ((fin = fopen(s, "r")))
    {
        jpeg_stdio_src(&cinfo, fin);
        jpeg_read_header(&cinfo, TRUE);
        jpeg_start_decompress(&cinfo);

        p = (unsigned char *) malloc(cinfo.output_width *
                                     cinfo.output_height *
                                     cinfo.output_components);

        while (p && cinfo.output_scanline < cinfo.output_height)
        {
            q = p +
                cinfo.output_scanline *
                cinfo.output_width    *
                cinfo.output_components;

            jpeg_read_scanlines(&cinfo, &q, 1);
        }

        *w = cinfo.output_width;
        *h = cinfo.output_height;
        *b = cinfo.output_components;

        jpeg_finish_decompress(&cinfo);

        fclose(fin);
    }

    jpeg_destroy_decompress(&cinfo);

    return (void *) p;
}

/*---------------------------------------------------------------------------*/

static void aux_copy_png(png_structp structp, png_infop infop,
                         png_bytep p, int w, int h, int b)
{
    png_bytep *rows;

    if ((rows = png_get_rows(structp, infop)) != NULL)
    {
        int r, c, i;

        for (r = 0; r < h; r++)
            for (c = 0; c < w; c++)
                for (i = 0; i < b; i++)
                    p[r * w * b + c * b + i] = rows[h - r - 1][c * b + i];
    }
}

static void *aux_read_png(png_structp structp, png_infop infop,
                          FILE *fp, int *w, int *h, int *b)
{
    void *p = NULL;

    if (setjmp(png_jmpbuf(structp)) == 0)
    {
        png_init_io(structp, fp);
        png_read_png(structp, infop,
                     PNG_TRANSFORM_STRIP_16 |
                     PNG_TRANSFORM_PACKING, NULL);

        *w = (int) png_get_image_width (structp, infop);
        *h = (int) png_get_image_height(structp, infop);
        *b = 0;

        switch (png_get_color_type(structp, infop))
        {
        case PNG_COLOR_TYPE_GRAY:       *b = 1; break;
        case PNG_COLOR_TYPE_GRAY_ALPHA: *b = 2; break;
        case PNG_COLOR_TYPE_RGB:        *b = 3; break;
        case PNG_COLOR_TYPE_RGB_ALPHA:  *b = 4; break;
        }

        if ((p = malloc((*w) * (*h) * (*b))) != NULL)
            aux_copy_png(structp, infop, (png_bytep) p, *w, *h, *b);
    }

    return p;
}

static void *aux_png_init(FILE *fp, int *w, int *h, int *b)
{
    void *p = NULL;

    png_structp structp;
    png_infop   infop;

    if ((structp = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                          NULL, NULL, NULL)) != NULL)
    {
        if ((infop = png_create_info_struct(structp)) != NULL)
            p = aux_read_png(structp, infop, fp, w, h, b);
    }

    png_destroy_read_struct(&structp, &infop, NULL);

    return p;
}

void *aux_load_png(const char *s, int *w, int *h, int *b)
{
    FILE *fp;
    void *p = NULL;

    if ((fp = fopen(s, "r")) != NULL)
    {
        p = aux_png_init(fp, w, h, b);
        fclose(fp);
    }

    return p;
}

/*---------------------------------------------------------------------------*/

GLuint aux_make_tex(const void *p, int w, int h, int b)
{
    const GLenum f[5] = {
        0, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA
    };
    GLuint o;

    glGenTextures(1, &o);
    glBindTexture(GL_TEXTURE_2D, o);

    glTexImage2D(GL_TEXTURE_2D, 0, b, w, h, 0, f[b], GL_UNSIGNED_BYTE, p);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return o;
}

/*---------------------------------------------------------------------------*/

void aux_draw_tex(GLuint o,
                  double x0, double y0,
                  double x1, double y1, double a)
{
    glBindTexture(GL_TEXTURE_2D, o);

    glPushAttrib(GL_ENABLE_BIT);
    {
        glDisable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);

        glBegin(GL_QUADS);
        {
            glColor4d(1.0, 1.0, 1.0, a);

            glTexCoord2d(0.0, 0.0);
            glVertex2d(x0, y0);
            glTexCoord2d(1.0, 0.0);
            glVertex2d(x1, y0);
            glTexCoord2d(1.0, 1.0);
            glVertex2d(x1, y1);
            glTexCoord2d(0.0, 1.0);
            glVertex2d(x0, y1);
        }
        glEnd();
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/

void aux_proj_identity(void)
{
    glMatrixMode(GL_PROJECTION);
    {
        glLoadIdentity();
        glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
    }
    glMatrixMode(GL_MODELVIEW);
}


