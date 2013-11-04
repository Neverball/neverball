/*
 * Copyright (C) 2006 Jean Privat
 *
 * this file is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

#ifndef LANG_H
#define LANG_H

#if ENABLE_NLS
#include <libintl.h>

#define _(String)   gettext(String)
#define L_(String)  get_local_text(String)

#else

#define _(String)   (String)
#define L_(String)  (String)

#define ngettext(msgid, msgid_plural, n) ((n) == 1 ? (msgid) : (msgid_plural))

#endif /* ENABLE_NLS */

/* No-op, useful for marking up strings for extraction-only. */
#define N_(String)   String

/*---------------------------------------------------------------------------*/

void lang_init(const char *domain, const char *pref);

const char *sgettext(const char *);
const char *get_local_text(const char *);

/*---------------------------------------------------------------------------*/

#include "common.h"
#include "array.h"
#include "dir.h"

struct lang_desc
{
    char code[32];

    char name1[MAXSTR];
    char name2[MAXSTR];
};

#define lang_name(desc) (*(desc)->name2 ? (desc)->name2 : (desc)->name1)

const char *lang_path(const char *);
const char *lang_code(const char *);

int  lang_load(struct lang_desc *, const char *);
void lang_free(struct lang_desc *);

#define LANG_GET(a, i) ((struct lang_desc *) DIR_ITEM_GET((a), (i))->data)

Array lang_dir_scan(void);
void  lang_dir_free(Array);

/*---------------------------------------------------------------------------*/

#endif
