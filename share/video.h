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

#ifndef VIDEO_H
#define VIDEO_H

#include "glext.h"

/*---------------------------------------------------------------------------*/

struct video
{
    int device_w, device_h;
    int window_w, window_h;

    float device_scale;
};

extern PathTracer::Tracer * ptracer;
extern PathTracer::Mesh * meshloader;
extern PathTracer::Intersector * currentIntersector;
extern PathTracer::Intersector * intersector;
extern PathTracer::Intersector * intersectorBillboard;
extern PathTracer::Intersector * intersectorBall;
extern PathTracer::Material * pmaterials;
extern PathTracer::Transformer * ptransformer;

extern struct video video;

int video_init(void);

/*---------------------------------------------------------------------------*/

int  video_mode(int, int, int);

void video_snap(const char *);
int  video_perf(void);
void video_swap(void);

void video_show_cursor(void);
void video_hide_cursor(void);

void video_set_grab(int w);
void video_clr_grab(void);
int  video_get_grab(void);

int  video_display();

/*---------------------------------------------------------------------------*/

void video_calc_view(float *, const float *,
                              const float *,
                              const float *);

void video_push_persp(float, float, float);
void video_push_ortho(void);
void video_push_FX(void);
void video_pop_matrix(void);
void video_clear(void);

/*---------------------------------------------------------------------------*/

#endif
