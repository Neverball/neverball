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
#include <math.h>

#include "base_config.h"
#include "common.h"
#include "log.h"
#include "fs.h"

#ifdef _WIN32
#include <shlobj.h>
#endif

/*---------------------------------------------------------------------------*/

static const char *pick_data_path(const char *arg_data_path)
{
    static char dir[MAXSTR];
    char *env;

    if (arg_data_path)
        return arg_data_path;

    if ((env = getenv("NEVERBALL_DATA")))
        return env;

    if (path_is_abs(CONFIG_DATA))
        return CONFIG_DATA;

    SAFECPY(dir, fs_base_dir());
    SAFECAT(dir, "/");
    SAFECAT(dir, CONFIG_DATA);

    return dir;
}

static const char *pick_home_path(void)
{
#ifdef _WIN32
    static char path[MAX_PATH];

    if (SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, path) == S_OK)
    {
        static char gamepath[MAX_PATH];

        SAFECPY(gamepath, path);
        SAFECAT(gamepath, "\\My Games");

        if (dir_exists(gamepath) || dir_make(gamepath) == 0)
            return gamepath;

        return path;
    }
    else
        return fs_base_dir();
#else
    const char *path;

    return (path = getenv("HOME")) ? path : fs_base_dir();
#endif
}

void config_paths(const char *arg_data_path)
{
    const char *data, *home, *user;

    /*
     * Scan in turn the game data and user directories for archives,
     * adding each archive to the search path.  Archives with names
     * further down the alphabet take precedence.  After each scan,
     * add the directory itself, taking precedence over archives added
     * so far.
     */

    /* Data directory. */

    data = pick_data_path(arg_data_path);

    fs_add_path_with_archives(data);

    /* User directory. */

    home = pick_home_path();

#ifdef __EMSCRIPTEN__
    /* Force IndexedDB-backed location created during Module['preRun']. */
    user = strdup("/neverball");
#else
    user = concat_string(home, "/", CONFIG_USER, NULL);
#endif

    /* Set up directory for writing, create if needed. */

    if (!fs_set_write_dir(user))
    {
        int success = 0;

        log_printf("Failure to establish write directory. First run?\n");

        if (fs_set_write_dir(home))
            if (fs_mkdir(CONFIG_USER))
                if (fs_set_write_dir(user))
                    success = 1;

        if (success)
        {
            log_printf("Write directory established at %s\n", user);
        }
        else
        {
            log_printf("Write directory not established at %s\n", user);
            fs_set_write_dir(NULL);
        }
    }

    fs_add_path_with_archives(user);

    free((void *) user);
}

/*---------------------------------------------------------------------------*/
