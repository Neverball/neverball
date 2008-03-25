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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include "base_config.h"
#include "glext.h"
#include "vec3.h"

/*---------------------------------------------------------------------------*/

/* Define the mkdir symbol. */

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

/*---------------------------------------------------------------------------*/

static char data_path[MAXSTR];
static char user_path[MAXSTR];

/*
 * Given  a path  and a  file name  relative to  that path,  create an
 * absolute path name and return a temporary pointer to it.
 */
static const char *config_file(const char *path, const char *file)
{
    static char absolute[MAXSTR];

    size_t d = strlen(path);

    strncpy(absolute, path, MAXSTR - 1);
    strncat(absolute, "/",  MAXSTR - d - 1);
    strncat(absolute, file, MAXSTR - d - 2);

    return absolute;
}

static int config_test(const char *path, const char *file)
{
    if (file)
    {
        FILE *fp;

        if ((fp = fopen(config_file(path, file), "r")))
        {
            fclose(fp);
            return 1;
        }
        return 0;
    }
    return 1;
}

const char *config_data(const char *file)
{
    return config_file(data_path, file);
}

const char *config_user(const char *file)
{
    return config_file(user_path, file);
}

/*---------------------------------------------------------------------------*/

/*
 * Attempt to find  the game data directory.  Search  the command line
 * parameter,  the environment,  and the  hard-coded default,  in that
 * order.  Confirm it by checking for presence of the named file.
 */
int config_data_path(const char *path, const char *file)
{
    char *dir;

    if (path && config_test(path, file))
    {
        strncpy(data_path, path, MAXSTR);
        return 1;
    }

    if ((dir = getenv("NEVERBALL_DATA")) && config_test(dir, file))
    {
        strncpy(data_path, dir, MAXSTR);
        return 1;
    }

    if (config_test(CONFIG_DATA, file))
    {
        strncpy(data_path, CONFIG_DATA, MAXSTR);
        return 1;
    }

    return 0;
}

/*
 * Determine the location of  the user's home directory.  Ensure there
 * is a  directory there for  storing configuration, high  scores, and
 * replays.
 *
 * Under Windows check the APPDATA environment variable and if that's
 * not set, just assume the user has permission to write to the data
 * directory.
 */
int config_user_path(const char *file)
{
#ifdef _WIN32
    char *dir;

    if ((dir = getenv("APPDATA")) || (dir = data_path))
    {
        size_t d = strlen(dir);

        strncpy(user_path, dir,         MAXSTR - 1);
        strncat(user_path, "\\",        MAXSTR - d - 1);
        strncat(user_path, CONFIG_USER, MAXSTR - d - 2);
    }

    if ((mkdir(user_path) == 0) || (errno == EEXIST))
        if (config_test(user_path, file))
            return 1;
#else
    char *dir;

    if ((dir = getenv("HOME")))
    {
        size_t d = strlen(dir);

        strncpy(user_path, dir,         MAXSTR - 1);
        strncat(user_path, "/",         MAXSTR - d - 1);
        strncat(user_path, CONFIG_USER, MAXSTR - d - 2);
    }

    if ((mkdir(user_path, 0777) == 0) || (errno == EEXIST))
        if (config_test(user_path, file))
            return 1;
#endif

    return 0;
}

/*---------------------------------------------------------------------------*/
