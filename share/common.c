/*
 *  Copyright (C) 2007  Neverball contributors
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "common.h"

#define MAXSTR 256

/*---------------------------------------------------------------------------*/

int read_line(char **dst, FILE *fin)
{
    char buffer[MAXSTR] = "";
    int  buffer_size    = 0;

    char *store      = NULL;
    char *store_new  = NULL;
    int   store_size = 0;

    int seen_newline = 0;

    while (!seen_newline)
    {
        if (fgets(buffer, sizeof (buffer), fin) == NULL)
        {
            if (store_size > 0)
                break;
            else
            {
                *dst = NULL;
                return 0;
            }
        }

        buffer_size = strlen(buffer) + 1;

        /* Erase trailing newline. */

        if (buffer[buffer_size - 2] == '\n')
        {
            seen_newline = 1;
            buffer[buffer_size - 2] = '\0';
            buffer_size--;
        }

        /* Allocate or reallocate space for the buffer. */

        if ((store_new = (char *) realloc(store, store_size + buffer_size)))
        {
            /* Avoid passing garbage to string functions. */

            if (store == NULL)
                store_new[0] = '\0';

            store       = store_new;
            store_size += buffer_size;

            store_new = NULL;
        }
        else
        {
            fprintf(stderr, "Failed to allocate memory.\n");

            free(store);
            *dst = NULL;
            return 0;
        }

        strncat(store, buffer, buffer_size);
    }

    *dst = store;

    return 1;
}

char *strip_newline(char *str)
{
    char *c = str + strlen(str) - 1;

    while (c >= str && (*c == '\n' || *c =='\r'))
        *c-- = '\0';

    return str;
}

time_t make_time_from_utc(struct tm *tm)
{
    struct tm local, *utc;
    time_t t;

    t = mktime(tm);

    local = *localtime(&t);
    utc   =  gmtime(&t);

    local.tm_year += local.tm_year - utc->tm_year;
    local.tm_mon  += local.tm_mon  - utc->tm_mon ;
    local.tm_mday += local.tm_mday - utc->tm_mday;
    local.tm_hour += local.tm_hour - utc->tm_hour;
    local.tm_min  += local.tm_min  - utc->tm_min ;
    local.tm_sec  += local.tm_sec  - utc->tm_sec ;

    return mktime(&local);
}

const char *date_to_str(time_t i)
{
    static char str[sizeof ("YYYY-mm-dd HH:MM:SS")];
    strftime(str, sizeof (str), "%Y-%m-%d %H:%M:%S", localtime(&i));
    return str;
}

int file_exists(const char *name)
{
    FILE *fp;

    if ((fp = fopen(name, "r")))
    {
        fclose(fp);
        return 1;
    }
    return 0;
}

int file_rename(const char *src, const char *dst)
{
#ifdef _WIN32
    if (file_exists(dst))
        remove(dst);
#endif
    return rename(src, dst);
}

void file_copy(FILE *fin, FILE *fout)
{
    char   buff[MAXSTR];
    size_t size;

    while ((size = fread(buff, 1, sizeof (buff), fin)) > 0)
        fwrite(buff, 1, size, fout);
}

char *base_name(const char *name, const char *suffix)
{
    static char buf[MAXSTR];
    char *base;

    if (!name)
        return NULL;

    /* Remove the directory part. */

    base = strrchr(name, '/');

#ifdef _WIN32
    if (!base)
        base = strrchr(name, '\\');
    else
    {
        char *tmp;

        if ((tmp = strrchr(base, '\\')))
            base = tmp;
    }
#endif

    strncpy(buf, base ? base + 1 : name, sizeof (buf));

    /* Remove the suffix. */

    if (suffix)
    {
        int l = strlen(buf) - strlen(suffix);

        if (l >= 0 && strcmp(buf + l, suffix) == 0)
            buf[l] = '\0';
    }

    return buf;
}

/*---------------------------------------------------------------------------*/
