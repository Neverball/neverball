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
#include <string.h>
#include <errno.h>

#include "fs.h"
#include "dir.h"
#include "array.h"
#include "list.h"
#include "common.h"
#include "log.h"
#include "zip.h"

/*
 * This file implements the low-level virtual file system routines
 * using stdio.
 */

/*---------------------------------------------------------------------------*/

enum fs_path_type
{
    FS_PATH_DIRECTORY,
    FS_PATH_ZIP,
};

struct fs_file_s
{
    FILE *handle;

    void *zip_file_data;
    size_t zip_file_pos;
    size_t zip_file_size;

    enum fs_path_type path_type;
};

struct fs_path_item
{
    void *data;
    char *path;
    enum fs_path_type type;
};

static struct fs_path_item *create_path_item(void)
{
    return calloc(sizeof (struct fs_path_item), 1);
}

static void free_path_item(struct fs_path_item *path_item)
{
    if (path_item)
    {
        if (path_item->path)
        {
            free(path_item->path);
            path_item->path = NULL;
        }

        if (path_item->type == FS_PATH_ZIP)
        {
            mz_zip_archive *zip = path_item->data;

            if (zip)
            {
                mz_zip_reader_end(zip);
                free(zip);
                zip = NULL;
            }

            path_item->data = NULL;
        }

        free(path_item);
        path_item = NULL;
    }
}

static char *fs_dir_base;
static char *fs_dir_write;
static List  fs_path;
static int   fs_logging = 1;

int fs_init(const char *argv0)
{
    fs_dir_base  = strdup(argv0 && *argv0 ? dir_name(argv0) : ".");
    fs_dir_write = NULL;
    fs_path      = NULL;
    fs_logging   = 1;

    return 1;
}

void fs_set_logging(int logging)
{
    fs_logging = logging;
}

int fs_quit(void)
{
    if (fs_dir_base)
    {
        free(fs_dir_base);
        fs_dir_base = NULL;
    }

    if (fs_dir_write)
    {
        free(fs_dir_write);
        fs_dir_write = NULL;
    }

    while (fs_path)
    {
        struct fs_path_item *path_item = fs_path->data;

        if (path_item)
        {
            free_path_item(path_item);
            path_item = NULL;
        }

        fs_path = list_rest(fs_path);
    }

    fs_cache_quit();

    return 1;
}

const char *fs_error(void)
{
    return strerror(errno);
}

/*---------------------------------------------------------------------------*/

const char *fs_base_dir(void)
{
    return fs_dir_base;
}

int fs_add_path(const char *path)
{
    struct fs_path_item *path_item;

    List l;

    if (!(path && *path))
        return 0;

    for (l = fs_path; l; l = l->next)
    {
        struct fs_path_item *test_item = l->data;

        if (strcmp(path, test_item->path) == 0)
            return 0;
    }

    path_item = create_path_item();

    if (!path_item)
        return 0;

    if (dir_exists(path))
    {
        if (fs_logging)
            log_printf("FS: reading from \"%s\" (directory)\n", path);

        path_item->type = FS_PATH_DIRECTORY;
        path_item->path = strdup(path);
        path_item->data = NULL;

        fs_path = list_cons(path_item, fs_path);

        return 1;
    }
    else
    {
        mz_zip_archive *zip;

        if ((zip = malloc(sizeof (*zip))))
        {
            mz_zip_zero_struct(zip);

            if (mz_zip_reader_init_file(zip, path, 0))
            {
                if (fs_logging)
                    log_printf("FS: reading from \"%s\" (zip)\n", path);

                path_item->type = FS_PATH_ZIP;
                path_item->path = strdup(path);
                path_item->data = zip;

                fs_path = list_cons(path_item, fs_path);

                return 1;
            }
            else if (fs_logging)
            {
                mz_zip_error err = mz_zip_get_last_error(zip);
                const char *str = mz_zip_get_error_string(err);
                log_printf("FS: skipping \"%s\" (%s)\n", path, str);
            }

            free(zip);
            zip = NULL;
        }

    }

    free_path_item(path_item);
    path_item = NULL;

    return 0;
}

void fs_remove_path(const char *path)
{
    List l, p;

    for (p = NULL, l = fs_path; l; p = l, l = l->next)
    {
        struct fs_path_item *path_item = l->data;

        if (strcmp(path_item->path, path) == 0)
        {
            if (fs_logging)
                log_printf("FS: unmounting \"%s\" (%s)\n", path, path_item->type == FS_PATH_DIRECTORY ? "directory" : "zip");

            free_path_item(path_item);
            path_item = NULL;
            l->data = NULL;

            if (p)
            {
                p->next = list_rest(l);
                l = p;
            }
            else
            {
                fs_path = list_rest(l);
                l = fs_path;
            }
        }
    }
}

int fs_set_write_dir(const char *path)
{
    if (dir_exists(path))
    {
        if (fs_dir_write)
        {
            free(fs_dir_write);
            fs_dir_write = NULL;
        }

        if (fs_logging)
            log_printf("FS: writing to \"%s\"\n", path);

        fs_dir_write = strdup(path);
        return 1;
    }
    return 0;
}

const char *fs_get_write_dir(void)
{
    return fs_dir_write;
}

/*---------------------------------------------------------------------------*/

/*
 * Like dir_list_files, but for ZIP archives.
 */
static List zip_list_files(mz_zip_archive *zip, const char *path)
{
    List files = NULL;

    if (zip)
    {
        mz_zip_archive_file_stat file_stat;

        unsigned int i, n = mz_zip_reader_get_num_files(zip);

        for (i = 0; i < n; ++i)
            if (mz_zip_reader_file_stat(zip, i, &file_stat))
            {
                const char *dn = dir_name(file_stat.m_filename);

                if (strcmp(dn, ".") == 0)
                {
                    /* dir_name() returns "." when it perhaps shouldn't. */
                    dn = "";
                }

                /* Very ape? */

                if (strcmp(path, dn) == 0)
                {
                    char *copy = strdup(base_name(file_stat.m_filename));
                    files = list_cons(copy, files);
                }
            }
    }

    return files;
}

static void zip_list_free(List files)
{
    while (files)
    {
        free(files->data);
        files = list_rest(files);
    }
}

/*---------------------------------------------------------------------------*/

/*
 * Uniquely insert strings from a List into another List.
 *
 * Target List is assumed to be pre-sorted.
 *
 * String data is assumed to be heap-allocated.
 */
static void insert_strings_into_list(List *items, List strings)
{
    List str;

    for (str = strings; str; str = str->next)
    {
        int skip = 0;
        List p, l;

        /* "Inspired" by PhysicsFS file enumeration code. */

        for (p = NULL, l = *items; l; p = l, l = l->next)
        {
            int cmp;

            if ((cmp = strcmp(l->data, str->data)) >= 0)
            {
                skip = (cmp == 0);
                break;
            }
        }

        if (!skip)
        {
            if (p)
                p->next = list_cons(str->data, p->next);
            else
                *items = list_cons(str->data, *items);

            /* We will free the string data ourselves. */

            str->data = NULL;
        }
    }
}

/*
 * Enumerate the files at `path` for every FS path location. FS path priority
 * is observed. Duplicates are skipped. Returns a List of allocated filenames.
 */
static List list_files(const char *path)
{
    List all_files = NULL;
    List p;

    for (p = fs_path; p; p = p->next)
    {
        struct fs_path_item *path_item = p->data;

        List path_files = NULL;

        if (path_item->type == FS_PATH_DIRECTORY)
        {
            char *real = path_join(path_item->path, path);
            path_files = dir_list_files(real);
            free(real);
        }
        else if (path_item->type == FS_PATH_ZIP)
        {
            mz_zip_archive *zip = path_item->data;
            path_files = zip_list_files(zip, path);
        }

        if (path_files)
            insert_strings_into_list(&all_files, path_files);

        /* Free any leftover string data and List cells. */

        if (path_item->type == FS_PATH_DIRECTORY)
            dir_list_free(path_files);
        else if (path_item->type == FS_PATH_ZIP)
            zip_list_free(path_files);
    }

    return all_files;
}

/*
 * Free the List of allocated filenames.
 */
static void free_files(List files)
{
    while (files)
    {
        free(files->data);
        files = list_rest(files);
    }
}

/*
 * Enumerate files in the given FS directory. Returns an Array of struct dir_item.
 */
Array fs_dir_scan(const char *path, int (*filter)(struct dir_item *))
{
    return dir_scan(path, filter, list_files, free_files);
}

/*
 * Free the Array of struct dir_item.
 */
void fs_dir_free(Array items)
{
    dir_free(items);
}

/*---------------------------------------------------------------------------*/

fs_file fs_open_read(const char *path)
{
    fs_file fh;

    if ((fh = calloc(1, sizeof (*fh))))
    {
        int opened = 0;
        List p;

        for (p = fs_path; p && !opened; p = p->next)
        {
            struct fs_path_item *path_item = p->data;

            if (path_item->type == FS_PATH_DIRECTORY)
            {
                char *real = path_join(path_item->path, path);

                if ((fh->handle = fopen(real, "rb")))
                {
                    fh->path_type = FS_PATH_DIRECTORY;
                    opened = 1;
                }

                free(real);
            }
            else if (path_item->type == FS_PATH_ZIP)
            {
                mz_zip_archive *zip = path_item->data;

                fh->zip_file_data = mz_zip_reader_extract_file_to_heap(zip, path, &fh->zip_file_size, 0);

                if (fh->zip_file_data)
                {
                    fh->zip_file_pos = 0;
                    fh->path_type = FS_PATH_ZIP;
                    opened = 1;
                }
            }
        }

        if (!opened)
        {
            free(fh);
            fh = NULL;
        }
    }
    return fh;
}

static fs_file fs_open_write_flags(const char *path, int append)
{
    fs_file fh = NULL;

    if (fs_dir_write && path && *path)
    {
        if ((fh = calloc(1, sizeof (*fh))))
        {
            char *real;

            if ((real = path_join(fs_dir_write, path)))
            {
                fh->handle = fopen(real, append ? "ab" : "wb");
                fh->path_type = FS_PATH_DIRECTORY;
                free(real);
            }

            if (!fh->handle)
            {
                free(fh);
                fh = NULL;
            }
        }
    }
    return fh;
}

fs_file fs_open_write(const char *path)
{
    return fs_open_write_flags(path, 0);
}

fs_file fs_open_append(const char *path)
{
    return fs_open_write_flags(path, 1);
}

int fs_close(fs_file fh)
{
    int closed = 0;

    if (fh)
    {
        if (fh->handle)
        {
            if (fclose(fh->handle))
                closed = 1;
        }

        if (fh->zip_file_data)
        {
            free(fh->zip_file_data);

            fh->zip_file_data = NULL;
            fh->zip_file_pos = 0;
            fh->zip_file_size = 0;

            closed = 1;
        }

        free(fh);
    }

    return closed;
}

/*----------------------------------------------------------------------------*/

/*
 * Create a directory in the FS write location.
 */
int fs_mkdir(const char *path)
{
    int success = 0;

    if (fs_dir_write)
    {
        char *real = path_join(fs_dir_write, path);
        success = dir_make(real) == 0;
        free((void *) real);
    }

    return success;
}

int fs_exists(const char *path)
{
    fs_file fh;

    if ((fh = fs_open_read(path)))
    {
        fs_close(fh);
        return 1;
    }
    return 0;
}

int fs_remove(const char *path)
{
    int success = 0;

    if (fs_dir_write)
    {
        char *real = path_join(fs_dir_write, path);
        success = (remove(real) == 0);
        free(real);
    }

    return success;
}

/*---------------------------------------------------------------------------*/

int fs_read(void *data, int bytes, fs_file fh)
{
    if (fh->handle)
        return fread(data, 1, bytes, fh->handle);

    if (fh->zip_file_data)
    {
        size_t left = fh->zip_file_size - fh->zip_file_pos;
        size_t read = MIN(left, bytes);

        memcpy(data, ((unsigned char *) fh->zip_file_data) + fh->zip_file_pos, read);

        fh->zip_file_pos += read;

        return read;
    }

    return 0;
}

int fs_write(const void *data, int bytes, fs_file fh)
{
    if (fh->handle)
        return fwrite(data, 1, bytes, fh->handle);

    /* ZIP writing is not available. */

    return 0;
}

int fs_flush(fs_file fh)
{
    if (fh->handle)
        return fflush(fh->handle);

    /* ZIP writing is not available. */

    return 0;
}

long fs_tell(fs_file fh)
{
    if (fh->handle)
        return ftell(fh->handle);

    if (fh->zip_file_data)
        return fh->zip_file_pos;

    return -1;
}

int fs_seek(fs_file fh, long offset, int whence)
{
    if (fh->handle)
        return fseek(fh->handle, offset, whence);

    if (fh->zip_file_data)
    {
        size_t pos = fh->zip_file_pos;

        if (whence == SEEK_CUR) {
            pos = fh->zip_file_pos + offset;
        } else if (whence == SEEK_SET) {
            pos = offset;
        } else if (whence == SEEK_END) {
            pos = fh->zip_file_size + offset;
        }

        pos = CLAMP(0, pos, fh->zip_file_size);

        fh->zip_file_pos = pos;

        return 0;
    }

    return -1;
}

int fs_eof(fs_file fh)
{
    /*
     * Unlike PhysicsFS, stdio does not register EOF unless we have
     * actually attempted to read past the end of the file.  Nothing
     * is done to mitigate this: instead, code that relies on
     * PhysicsFS behavior should be fixed not to.
     */
    if (fh->handle)
        return feof(fh->handle);


    if (fh->zip_file_data)
        return fh->zip_file_pos >= fh->zip_file_size;

    return 1;
}

int fs_size(const char *path)
{
    List p;

    for (p = fs_path; p; p = p->next)
    {
        struct fs_path_item *path_item = p->data;

        if (path_item->type == FS_PATH_DIRECTORY)
        {
            char *real = path_join(path_item->path, path);

            if (real)
            {
                if (file_exists(real))
                {
                    int s = file_size(real);
                    free(real);
                    real = NULL;
                    return s;
                }

                free(real);
                real = NULL;
            }
        }
        else if (path_item->type == FS_PATH_ZIP)
        {
            mz_zip_archive *zip = path_item->data;
            int file_index = mz_zip_reader_locate_file(zip, path, NULL, 0);

            if (file_index >= 0)
            {
                mz_zip_archive_file_stat file_stat;

                if (mz_zip_reader_file_stat(zip, file_index, &file_stat))
                    return file_stat.m_uncomp_size;
            }
        }
    }

    return 0;
}

/*---------------------------------------------------------------------------*/
