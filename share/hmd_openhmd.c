/*
 * Copyright (C) 2013 Robert Kooima
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

#define OHMD_STATIC 1
#include <openhmd/openhmd.h>
#include <stdlib.h>
#include <stdio.h>

#include "hmd_common.h"
#include "config.h"
#include "glext.h"

static ohmd_context *ctx = NULL;
static ohmd_device  *dev = NULL;
static int           eye = 0;

static int hmd_hres = 0;
static int hmd_vres = 0;

/*---------------------------------------------------------------------------*/

void hmd_init()
{
    hmd_hres = config_get_d(CONFIG_WIDTH);
    hmd_vres = config_get_d(CONFIG_HEIGHT);

    /* Start up OpenHMD. */

    if ((ctx = ohmd_ctx_create()))
    {
        if (ohmd_ctx_probe(ctx) > 0)
        {
            if ((dev = ohmd_list_open_device(ctx, 0)))
            {
                /* Create the off-screen frame buffers. */

                ohmd_device_geti(dev, OHMD_SCREEN_HORIZONTAL_RESOLUTION, &hmd_hres);
                ohmd_device_geti(dev, OHMD_SCREEN_VERTICAL_RESOLUTION,   &hmd_vres);
            }
        }
    }

    hmd_common_init(hmd_hres, hmd_vres);
}

void hmd_free()
{
    if (ctx) ohmd_ctx_destroy(ctx);

    hmd_common_free();

    dev = NULL;
    ctx = NULL;
    eye =    0;
}

void hmd_step()
{
    if (ctx) ohmd_ctx_update(ctx);
}

void hmd_swap()
{
    static const float center = 0.151976f;
    static const float scale  = 1.714606f;

    static const float barrel_correction[] = { 1.00f, 0.22f, 0.24f };
    static const float chroma_correction[] = { 0.996f, -0.004f, 1.014f, 0.000f };

    hmd_common_swap(center, scale, barrel_correction, chroma_correction);
}

void hmd_prep_left()
{
    hmd_common_left();
    eye = 0;
}

void hmd_prep_right()
{
    hmd_common_right();
    eye = 1;
}

void hmd_persp(float n, float f)
{
    float M[16];

    if (eye)
    {
        glMatrixMode(GL_PROJECTION);
        ohmd_device_getf(dev, OHMD_RIGHT_EYE_GL_PROJECTION_MATRIX, M);
        glLoadMatrixf(M);

        glMatrixMode(GL_MODELVIEW);
        ohmd_device_getf(dev, OHMD_RIGHT_EYE_GL_MODELVIEW_MATRIX, M);
        glLoadMatrixf(M);
    }
    else
    {
        glMatrixMode(GL_PROJECTION);
        ohmd_device_getf(dev, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX, M);
        glLoadMatrixf(M);

        glMatrixMode(GL_MODELVIEW);
        ohmd_device_getf(dev, OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX, M);
        glLoadMatrixf(M);
    }
}

void hmd_ortho()
{
    hmd_persp(0.5f, 2.0f);

    glScalef    ( 1.25f / hmd_vres,  1.25f / hmd_vres,  1.0f);
    glTranslatef(-0.50f * hmd_hres, -0.50f * hmd_vres, -1.0f);
}

/*---------------------------------------------------------------------------*/
