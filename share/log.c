/*
 * Copyright (C) 2014 Neverball authors
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

#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "common.h"
#include "version.h"
#include "fs.h"

static char    log_header[MAXSTR];
static fs_file log_fp;

/*---------------------------------------------------------------------------*/

void log_printf(const char *fmt, ...)
{
    char *str;
    int len;

    va_list ap;

    va_start(ap, fmt);
    len = 1 + vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    if ((str = malloc(len)))
    {
        va_start(ap, fmt);
        vsnprintf(str, len, fmt, ap);
        va_end(ap);

        fputs(str, stderr);

        if (log_fp)
        {
            /* These are printfs to get us CRLF conversion. */

            if (log_header[0])
            {
                fs_printf(log_fp, "%s\n", log_header);
                log_header[0] = 0;
            }

            fs_printf(log_fp, "%s", str);

            fs_flush(log_fp);
        }

        free(str);
    }
}

/*---------------------------------------------------------------------------*/

void log_init(const char *name, const char *path)
{
    if (!log_fp)
    {
        if ((log_fp = fs_open(path, "w+")))
        {
            /* Printed on first message. */

            sprintf(log_header, "%s - %s %s",
                    date_to_str(time(NULL)),
                    name, VERSION);
        }
        else
        {
            fprintf(stderr, "Failure to open %s\n", path);
        }
    }
}

void log_quit(void)
{
    if (log_fp)
    {
        fs_close(log_fp);
        log_header[0] = 0;
    }
}

/*---------------------------------------------------------------------------*/
