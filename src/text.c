/*   
 *   Copyright (C) 2003 Robert Kooima
 *
 *   SUPER EMPTY BALL is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by the
 *   Free Software Foundation;  either version 2 of the  License, or (at your
 *   option) any later version.
 *
 *   This program  is distributed  in the  hope that it  will be  useful, but
 *   WITHOUT   ANY   WARRANTY;  without   even   the   implied  warranty   of
 *   MERCHANTABILITY  or  FITNESS FOR  A  PARTICULAR  PURPOSE.   See the  GNU
 *   General Public License for more details.
 */

#include "gl.h"
#include "text.h"
#include "image.h"

/*---------------------------------------------------------------------------*/

#define IMG_DIG0 "data/png/digit0.png"
#define IMG_DIG1 "data/png/digit1.png"
#define IMG_DIG2 "data/png/digit2.png"
#define IMG_DIG3 "data/png/digit3.png"
#define IMG_DIG4 "data/png/digit4.png"
#define IMG_DIG5 "data/png/digit5.png"
#define IMG_DIG6 "data/png/digit6.png"
#define IMG_DIG7 "data/png/digit7.png"
#define IMG_DIG8 "data/png/digit8.png"
#define IMG_DIG9 "data/png/digit9.png"

#define IMG_COINS "data/png/coins.png"
#define IMG_BALLS "data/png/balls.png"

static struct image digit_img[10];
static struct image label_img[2];

/*---------------------------------------------------------------------------*/

void text_init(void)
{
    image_load(digit_img + 0, IMG_DIG0);
    image_load(digit_img + 1, IMG_DIG1);
    image_load(digit_img + 2, IMG_DIG2);
    image_load(digit_img + 3, IMG_DIG3);
    image_load(digit_img + 4, IMG_DIG4);
    image_load(digit_img + 5, IMG_DIG5);
    image_load(digit_img + 6, IMG_DIG6);
    image_load(digit_img + 7, IMG_DIG7);
    image_load(digit_img + 8, IMG_DIG8);
    image_load(digit_img + 9, IMG_DIG9);

    image_load(label_img + TXT_COINS, IMG_COINS);
    image_load(label_img + TXT_BALLS, IMG_BALLS);
}

void text_free(void)
{
    image_free(label_img + TXT_BALLS);
    image_free(label_img + TXT_COINS);

    image_free(digit_img + 9);
    image_free(digit_img + 8);
    image_free(digit_img + 7);
    image_free(digit_img + 6);
    image_free(digit_img + 5);
    image_free(digit_img + 4);
    image_free(digit_img + 3);
    image_free(digit_img + 2);
    image_free(digit_img + 1);
    image_free(digit_img + 0);
}

/*---------------------------------------------------------------------------*/

void text_digit(int d, double x0, double y0, double x1, double y1)
{
    image_bind(digit_img + d);

    glBegin(GL_QUADS);
    {
        glTexCoord2d(0.0, 1.0); glVertex2d(x0, y0);
        glTexCoord2d(1.0, 1.0); glVertex2d(x1, y0);
        glTexCoord2d(1.0, 0.0); glVertex2d(x1, y1);
        glTexCoord2d(0.0, 0.0); glVertex2d(x0, y1);
    }
    glEnd();
}

void text_label(int l, double x0, double y0, double x1, double y1)
{
    image_bind(label_img + l);

    glBegin(GL_QUADS);
    {
        glTexCoord2d(0.0, 1.0); glVertex2d(x0, y0);
        glTexCoord2d(1.0, 1.0); glVertex2d(x1, y0);
        glTexCoord2d(1.0, 0.0); glVertex2d(x1, y1);
        glTexCoord2d(0.0, 0.0); glVertex2d(x0, y1);
    }
    glEnd();
}

/*---------------------------------------------------------------------------*/
