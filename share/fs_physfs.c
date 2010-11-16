/*
 * Copyright (C) 2003-2010 Neverball authors
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
#include <assert.h>
#include <physfs.h>

#include "fs.h"
#include "dir.h"
#include "array.h"
#include "common.h"

/*
 * This file implements the low-level virtual file system routines
 * using the PhysicsFS 1.0 API.
 */

/*---------------------------------------------------------------------------*/

struct fs_file
{
    PHYSFS_file *handle;
};

int fs_init(const char *argv0)
{
    if (PHYSFS_init(argv0))
    {
        PHYSFS_permitSymbolicLinks(1);
        return 1;
    }

    return 0;
}

int fs_quit(void)
{
    return PHYSFS_deinit();
}

const char *fs_error(void)
{
    return PHYSFS_getLastError();
}

/*---------------------------------------------------------------------------*/

const char *fs_base_dir(void)
{
    return PHYSFS_getBaseDir();
}

int fs_add_path(const char *path)
{
    return PHYSFS_addToSearchPath(path, 0);
}

int fs_set_write_dir(const char *path)
{
    return PHYSFS_setWriteDir(path);
}

const char *fs_get_write_dir(void)
{
    return PHYSFS_getWriteDir();
}

/*---------------------------------------------------------------------------*/

static List list_files(const char *path)
{
    List list = NULL;
    char **files, **file;

    if ((files = PHYSFS_enumerateFiles(path)))
    {
        for (file = files; *file; file++)
            list = list_cons(strdup(*file), list);

        PHYSFS_freeList(files);
    }

    return list;
}

static void free_files(List list)
{
    while (list)
    {
        free(list->data);
        list = list_rest(list);
    }
}

Array fs_dir_scan(const char *path, int (*filter)(struct dir_item *))
{
    return dir_scan(path, filter, list_files, free_files);
}

void fs_dir_free(Array items)
{
    dir_free(items);
}

/*---------------------------------------------------------------------------*/

fs_file fs_open(const char *path, const char *mode)
{
    fs_file fh;

    assert((mode[0] == 'r' && !mode[1]) ||
           (mode[0] == 'w' && (!mode[1] || mode[1] == '+')));

    if ((fh = malloc(sizeof (*fh))))
    {
        switch (mode[0])
        {
        case 'r':
            fh->handle = PHYSFS_openRead(path);
            break;

        case 'w':
            fh->handle = (mode[1] == '+' ?
                          PHYSFS_openAppend(path) :
                          PHYSFS_openWrite(path));
            break;
        }

        if (fh->handle)
        {
            PHYSFS_setBuffer(fh->handle, 0x2000);
        }
        else
        {
            free(fh);
            fh = NULL;
        }
    }

    return fh;
}

int fs_close(fs_file fh)
{
    if (PHYSFS_close(fh->handle))
    {
        free(fh);
        return 1;
    }
    return 0;
}

/*----------------------------------------------------------------------------*/

int fs_mkdir(const char *path)
{
    return PHYSFS_mkdir(path);
}

int fs_exists(const char *path)
{
    return PHYSFS_exists(path);
}

int fs_remove(const char *path)
{
    return PHYSFS_delete(path);
}

/*---------------------------------------------------------------------------*/

int fs_read(void *data, int size, int count, fs_file fh)
{
    return PHYSFS_read(fh->handle, data, size, count);
}

int fs_write(const void *data, int size, int count, fs_file fh)
{
    return PHYSFS_write(fh->handle, data, size, count);
}

int fs_flush(fs_file fh)
{
    return PHYSFS_flush(fh->handle);
}

long fs_tell(fs_file fh)
{
    return PHYSFS_tell(fh->handle);
}

int fs_seek(fs_file fh, long offset, int whence)
{
    PHYSFS_uint64 pos = 0;
    PHYSFS_sint64 cur = PHYSFS_tell(fh->handle);
    PHYSFS_sint64 len = PHYSFS_fileLength(fh->handle);

    switch (whence)
    {
    case SEEK_SET:
        pos = offset;
        break;

    case SEEK_CUR:
        if (cur < 0)
            return -1;
        pos = cur + offset;
        break;

    case SEEK_END:
        if (len < 0)
            return -1;
        pos = len + offset;
        break;
    }

    return PHYSFS_seek(fh->handle, pos);
}

int fs_eof(fs_file fh)
{
    return PHYSFS_eof(fh->handle);
}

int fs_length(fs_file fh)
{
    return PHYSFS_fileLength(fh->handle);
}

/*---------------------------------------------------------------------------*/
