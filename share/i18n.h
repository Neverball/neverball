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

#ifndef LANGUAGE_H
#define LANGUAGE_H

#ifndef POSIX
#	include <libintl.h>
#	define _(String)            gettext(String)
#else
#	define _(String)            (String)
#endif

#define N_(String)           (String)

/*---------------------------------------------------------------------------*/

const char * gettextdbg(const char *);

/*---------------------------------------------------------------------------*/

void language_init(const char * domain, const char * locale_dir);
void language_set(int id);

/*---------------------------------------------------------------------------*/

int language_count();
int language_from_code(const char * code);
const char * language_get_name(int id);
const char * language_get_code(int id);

/*---------------------------------------------------------------------------*/

#endif
