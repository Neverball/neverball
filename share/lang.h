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

#ifndef POSIX
#include <libintl.h>
#define _(String)   gettext(String)
#else
#define _(String)   (String)
#endif

/* No-op, useful for marking up strings for extraction-only. */
#define N_(String)  (String)

/*---------------------------------------------------------------------------*/

void lang_init(const char *domain, const char *locale_dir);

const char *sgettext(const char *);

/*---------------------------------------------------------------------------*/

#endif
