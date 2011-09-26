/*
 *  Copyright (C) 2007  Neverball authors
 *
 *  This  program is  free software;  you can  redistribute  it and/or
 *  modify it  under the  terms of the  GNU General Public  License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program  is distributed in the  hope that it  will be useful,
 *  but  WITHOUT ANY WARRANTY;  without even  the implied  warranty of
 *  MERCHANTABILITY or FITNESS FOR  A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a  copy of the GNU General Public License
 *  along  with this  program;  if  not, write  to  the Free  Software
 *  Foundation,  Inc.,   59  Temple  Place,  Suite   330,  Boston,  MA
 *  02111-1307 USA
 */

#ifndef COMMON_H
#define COMMON_H

#include <time.h>
#include <stdio.h>
#include "fs.h"

/* Random stuff. */

#ifdef __GNUC__
#define NULL_TERMINATED __attribute__ ((__sentinel__))
#else
#define NULL_TERMINATED
#endif

/* Math. */

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define SIGN(n) ((n) < 0 ? -1 : ((n) ? +1 : 0))
#define ROUND(f) ((int) ((f) + 0.5f * SIGN(f)))

#define TIME_TO_MS(t) ROUND((t) * 1000.0f)
#define MS_TO_TIME(m) ((m) * 0.001f)

int rand_between(int low, int high);

/* Arrays and strings. */

#define ARRAYSIZE(a) (sizeof (a) / sizeof ((a)[0]))
#define MAXSTRLEN(a) (sizeof (a) - 1)

#define SAFECPY(dst, src) \
    (strncpy((dst), (src), MAXSTRLEN(dst)))
#define SAFECAT(dst, src) \
    (strncat((dst), (src), MAXSTRLEN(dst) - MIN(strlen(dst), MAXSTRLEN(dst))))

int   read_line(char **, fs_file);
char *strip_newline(char *);

char *dupe_string(const char *);
char *concat_string(const char *first, ...) NULL_TERMINATED;

#ifdef strdup
#undef strdup
#endif
#define strdup dupe_string

#define str_starts_with(s, h) (strncmp((s), (h), strlen(h)) == 0)
#define str_ends_with(s, t) (strcmp((s) + strlen(s) - strlen(t), (t)) == 0)

/* Time. */

time_t make_time_from_utc(struct tm *);
const char *date_to_str(time_t);

/* Files. */

int  file_exists(const char *);
int  file_rename(const char *, const char *);
void file_copy(FILE *fin, FILE *fout);

/* Paths. */

int path_is_sep(int);
int path_is_abs(const char *);

char *path_join(const char *, const char *);

const char *path_last_sep(const char *);
const char *path_next_sep(const char *);

const char *base_name(const char *name);
const char *base_name_sans(const char *name, const char *suffix);
const char *dir_name(const char *name);

#endif
