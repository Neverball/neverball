/*
 * Copyright (C) 2021-2025 Jānis Rūcis
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

#ifndef PACKAGE_H
#define PACKAGE_H 1

#include "fetch.h"
#include "array.h"

enum package_status
{
    PACKAGE_NONE = 0,
    PACKAGE_AVAILABLE,
    PACKAGE_UPDATE,
    PACKAGE_DOWNLOADING,
    PACKAGE_INSTALLED,
    PACKAGE_ERROR
};

enum package_image_status
{
    PACKAGE_IMAGE_NONE = 0,
    PACKAGE_IMAGE_DOWNLOADING
};

struct package
{
    unsigned int size;

    char id[64];
    char type[64];
    char filename[256];
    char files[1024];
    char name[64];
    char desc[256];
    char shot[64];

    enum package_status status;
    enum package_image_status image_status;
};

void package_init(void);
void package_quit(void);

unsigned int package_refresh(struct fetch_callback, cmp_fn);

int package_count(void);

int package_search(const char *file);
int package_search_id(const char *package_id);
int package_next(const char *type, int start);

unsigned int package_fetch(int, struct fetch_callback);
unsigned int package_fetch_image(int, struct fetch_callback);

enum package_status package_get_status(int);

const char *package_get_id(int);
const char *package_get_type(int);
const char *package_get_name(int);
const char *package_get_desc(int);
const char *package_get_shot(int);
const char *package_get_files(int);

const char *package_get_shot_filename(int);
const char *package_get_formatted_type(int);

#endif