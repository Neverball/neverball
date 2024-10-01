/*
 * Copyright (C) 2021-2024 Jānis Rūcis
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

#include "package.h"
#include "array.h"
#include "common.h"
#include "fetch.h"
#include "fs.h"
#include "lang.h"

enum package_image_status
{
    PACKAGE_IMAGE_NONE = 0,
    PACKAGE_IMAGE_DOWNLOADING
};

struct package
{
    unsigned int size;

    char id[64];
    char type[64];
    char filename[MAXSTR];
    char files[1024];
    char name[64];
    char desc[MAXSTR];
    char shot[64];

    enum package_status status;
    enum package_image_status image_status;
};

static Array available_packages;

#define PACKAGE_GET(a, i) ((struct package *) array_get((a), (i)))

#define PACKAGE_DIR "Downloads"

/*---------------------------------------------------------------------------*/

/*
 * Get a download URL.
 */
static const char *get_package_url(const char *filename)
{
    if (filename && *filename)
    {
        static char url[MAXSTR];

        memset(url, 0, sizeof (url));

#ifdef __EMSCRIPTEN__
        /* Same origin. */
        SAFECPY(url, "packages/");
#else
        SAFECPY(url, "https://play.neverball.org/packages/");
#endif
        SAFECAT(url, filename);

        return url;
    }

    return NULL;
}

/*
 * Get a download filename.
 */
static const char *get_package_path(const char *filename)
{
    if (filename && *filename)
    {
        static char path[MAXSTR];

        memset(path, 0, sizeof (path));

        SAFECPY(path, PACKAGE_DIR "/");
        SAFECAT(path, filename);

        return path;
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/

/*
 * We keep a separate list of installed packages, so that we don't have to scan
 * the package directory and figure out which ZIP files can be added to the FS
 * and which ones can't.
 */

struct local_package
{
    char id[64];
    char filename[MAXSTR];
};

static List installed_packages;

static struct local_package *create_local_package(const char *package_id, const char *filename)
{
    struct local_package *lpkg = calloc(sizeof (*lpkg), 1);

    if (lpkg)
    {
        SAFECPY(lpkg->id, package_id);
        SAFECPY(lpkg->filename, filename);
    }

    return lpkg;
}

static void free_local_package(struct local_package **lpkg)
{
    if (lpkg && *lpkg)
    {
        free(*lpkg);
        *lpkg = NULL;
    }
}

/*
 * Add package file to FS path.
 */
static int mount_package_file(const char *filename)
{
    const char *write_dir = fs_get_write_dir();
    int added = 0;

    if (filename && *filename && write_dir)
    {
        char *path = concat_string(write_dir, "/" PACKAGE_DIR "/", filename, NULL);

        if (path)
        {
            added = fs_add_path(path);

            free(path);
            path = NULL;
        }
    }

    return added;
}

/*
 * Remove package file from the FS read path.
 */
static void unmount_package_file(const char *filename)
{
    const char *write_dir = fs_get_write_dir();

    if (filename && *filename && write_dir)
    {
        char *path = concat_string(write_dir, "/" PACKAGE_DIR "/", filename, NULL);

        if (path)
        {
            fs_remove_path(path);

            free(path);
            path = NULL;
        }
    }
}

/*
 * Unmount and uninstall other instances of the given local package.
 */
static void unmount_duplicate_local_packages(const struct local_package *keep_lpkg)
{
    List p, l;

    /* Unmount and uninstall other instances of this package ID. */

    for (p = NULL, l = installed_packages; l; p = l, l = l->next)
    {
        struct local_package *test_lpkg = l->data;

        if (test_lpkg != keep_lpkg && strcmp(test_lpkg->id, keep_lpkg->id) == 0)
        {
            unmount_package_file(test_lpkg->filename);

            free_local_package(&test_lpkg);

            l->data = NULL;

            if (p)
            {
                p->next = list_rest(l);
                l = p;
            }
            else
            {
                installed_packages = list_rest(l);
                l = installed_packages;
            }
        }
    }
}

static int save_installed_packages(void);

/*
 * Add a package to the FS path and to the list, if not yet added.
 */
static int mount_local_package(struct local_package *lpkg)
{
    if (lpkg && mount_package_file(lpkg->filename))
    {
        installed_packages = list_cons(lpkg, installed_packages);
        unmount_duplicate_local_packages(lpkg);
        save_installed_packages();
        return 1;
    }

    return 0;
}

/*
 * Load the list of installed packages.
 */
static int load_installed_packages(void)
{
    fs_file fp = fs_open_read(get_package_path("installed-packages.txt"));

    if (fp)
    {
        char line[MAXSTR] = "";

        Array pkgs = array_new(sizeof (struct local_package));
        struct local_package *lpkg = NULL;
        int i, n;

        while (fs_gets(line, sizeof (line), fp))
        {
            strip_newline(line);

            if (strncmp(line, "package ", 8) == 0)
            {
                lpkg = array_add(pkgs);

                if (lpkg)
                    SAFECPY(lpkg->id, line + 8);
            }
            else if (strncmp(line, "filename ", 9) == 0)
            {
                if (lpkg)
                    SAFECPY(lpkg->filename, line + 9);
            }
            else if (fs_exists(get_package_path(line)))
            {
                /* Backward compatibility: the entire line is the filename. */

                if ((lpkg = array_add(pkgs)))
                {
                    char *delim;

                    SAFECPY(lpkg->filename, line);

                    /* Extract package ID from the filename. */

                    if ((delim = strrchr(lpkg->filename, '-')))
                    {
                        size_t len = delim - lpkg->filename;
                        memcpy(lpkg->id, lpkg->filename, MIN(sizeof (lpkg->id) - 1, len));
                    }

                    lpkg = NULL;
                }
            }
        }

        for (i = 0, n = array_len(pkgs); i < n; ++i)
        {
            const struct local_package *src = array_get(pkgs, i);
            struct local_package *dst = create_local_package(src->id, src->filename);

            if (!mount_local_package(dst))
                free_local_package(&dst);
        }

        if (pkgs)
        {
            array_free(pkgs);
            pkgs = NULL;
        }

        fs_close(fp);
        fp = NULL;

        return 1;
    }

    return 0;
}

/*
 * Save the list of installed packages.
 */
static int save_installed_packages(void)
{
    if (installed_packages)
    {
        fs_file fp = fs_open_write(get_package_path("installed-packages.txt"));

        if (fp)
        {
            List l;

            for (l = installed_packages; l; l = l->next)
            {
                struct local_package *lpkg = l->data;

                if (lpkg)
                    fs_printf(fp, "package %s\nfilename %s\n", lpkg->id, lpkg->filename);
            }

            fs_close(fp);
            fp = NULL;

            return 1;
        }
    }

    return 0;
}

/*
 * Free the list of installed packages.
 */
static void free_installed_packages(void)
{
    List l = installed_packages;

    while (l)
    {
        struct local_package *lpkg = l->data;

        free_local_package(&lpkg);

        l = list_rest(l);
    }

    installed_packages = NULL;
}

/*---------------------------------------------------------------------------*/

/*
 * Figure out package statuses.
 *
 * Packages that are found to exist are marked as installed and added to the FS.
 */
static void load_package_statuses(Array packages)
{
    if (packages)
    {
        int i, n;

        for (i = 0, n = array_len(packages); i < n; ++i)
        {
            struct package *pkg = array_get(packages, i);

            List l;

            pkg->status = PACKAGE_AVAILABLE;

            for (l = installed_packages; l; l = l->next)
            {
                struct local_package *lpkg = l->data;

                if (strcmp(pkg->id, lpkg->id) == 0)
                {
                    pkg->status = PACKAGE_UPDATE;

                    if (strcmp(pkg->filename, lpkg->filename) == 0)
                    {
                        pkg->status = PACKAGE_INSTALLED;
                        break;
                    }
                }
            }
        }
    }
}

/*
 * Load an array of packages from a manifest file.
 */
static Array load_packages_from_file(const char *filename)
{
    Array packages = array_new(sizeof (struct package));
    fs_file fp;

    if (!packages)
        return NULL;

    if ((fp = fs_open_read(filename)))
    {
        struct package *pkg = NULL;
        char line[MAXSTR];

        while (fs_gets(line, sizeof (line), fp))
        {
            strip_newline(line);

            if (strncmp(line, "package ", 8) == 0)
            {
                /* Start reading a new package. */

                pkg = array_add(packages);

                if (pkg)
                {
                    size_t prefix_len;

                    memset(pkg, 0, sizeof (*pkg));

                    SAFECPY(pkg->id, line + 8);

                    prefix_len = strcspn(pkg->id, "-");

                    strncpy(pkg->type, pkg->id, MIN(sizeof (pkg->type) - 1, prefix_len));
                }
            }
            else if (strncmp(line, "filename ", 9) == 0)
            {
                if (pkg)
                    SAFECPY(pkg->filename, line + 9);
            }
            else if (strncmp(line, "size ", 5) == 0)
            {
                if (pkg)
                    sscanf(line + 5, "%u", &pkg->size);
            }
            else if (strncmp(line, "files ", 6) == 0)
            {
                if (pkg)
                    SAFECPY(pkg->files, line + 6);
            }
            else if (strncmp(line, "name ", 5) == 0)
            {
                if (pkg)
                    SAFECPY(pkg->name, line + 5);
            }
            else if (strncmp(line, "desc ", 5) == 0)
            {
                if (pkg)
                {
                    char *s = NULL;

                    SAFECPY(pkg->desc, line + 5);

                    /* Replace "\\n" with "\r\n" in place. I really just need the "\n", but don't want to move bytes around. */

                    for (s = pkg->desc; (s = strstr(s, "\\n")); s += 2)
                    {
                        s[0] = '\r';
                        s[1] = '\n';
                    }
                }
            }
            else if (strncmp(line, "shot ", 5) == 0)
            {
                if (pkg)
                    SAFECPY(pkg->shot, line + 5);
            }
        }

        fs_close(fp);
    }

    load_package_statuses(packages);

    return packages;
}

/*
 * Free a loaded array of packages.
 */
static void free_packages(Array packages)
{
    if (packages)
    {
        array_free(packages);
        packages = NULL;
    }
}

/*---------------------------------------------------------------------------*/

struct package_image_info
{
    struct fetch_callback callback;
    struct package *pkg;
};

static struct package_image_info *create_pii(struct fetch_callback callback, struct package *pkg)
{
    struct package_image_info *pii = calloc(sizeof (*pii), 1);

    if (pii)
    {
        pii->callback = callback;
        pii->pkg = pkg;
    }

    return pii;
}

static void free_pii(struct package_image_info **pii)
{
    if (pii && *pii)
    {
        free(*pii);
        *pii = NULL;
    }
}

static void package_image_done(void *data, void *extra_data)
{
    struct package_image_info *pii = data;
    struct fetch_done *fd = extra_data;

    if (pii)
    {
        if (fd && fd->finished && pii->pkg)
            pii->pkg->image_status = PACKAGE_IMAGE_NONE;

        if (pii->callback.done)
            pii->callback.done(pii->callback.data, extra_data);

        free_pii(&pii);
    }
}

/*
 * Queue missing package images for download.
 */
unsigned int package_fetch_image(int pi, struct fetch_callback nested_callback)
{
    unsigned int fetch_id = 0;

    if (available_packages && pi >= 0 && pi < array_len(available_packages))
    {
        struct package *pkg = array_get(available_packages, pi);
        const char *filename = package_get_shot_filename(pi);

        if (filename && *filename && !fs_exists(filename) && !pkg->image_status)
        {
            const char *url = get_package_url(pkg->shot);

            if (url)
            {
                struct fetch_callback callback = { 0 };
                struct package_image_info *pii = create_pii(nested_callback, pkg);

                callback.data = pii;
                callback.done = package_image_done;

                fetch_id = fetch_file(url, filename, callback);

                if (fetch_id)
                    pkg->image_status = PACKAGE_IMAGE_DOWNLOADING;
                else
                {
                    free_pii(&pii);
                    callback.data = NULL;
                }
            }
        }
    }

    return fetch_id;
}

/*---------------------------------------------------------------------------*/

struct package_list_info
{
    struct fetch_callback callback;
};

static struct package_list_info *create_pli(struct fetch_callback nested_callback)
{
    struct package_list_info *pli = calloc(sizeof (*pli), 1);

    if (pli)
        pli->callback = nested_callback;

    return pli;
}

static void free_pli(struct package_list_info **pli)
{
    if (pli && *pli)
    {
        free(*pli);
        *pli = NULL;
    }
}

/*
 * Load the list of available packages and initiate image downloads.
 */
static void available_packages_done(void *data, void *extra_data)
{
    struct package_list_info *pli = data;
    struct fetch_done *fd = extra_data;

    if (fd && fd->finished)
    {
        const char *filename = get_package_path("available-packages.txt");

        if (filename)
        {
            Array packages = load_packages_from_file(filename);

            if (packages)
                available_packages = packages;
        }
    }

    if (pli)
    {
        if (pli->callback.done)
            pli->callback.done(pli->callback.data, extra_data);

        free_pli(&pli);
    }
}

/*
 * Download the package list.
 */
static unsigned int fetch_available_packages(struct fetch_callback nested_callback)
{
    unsigned int fetch_id = 0;
    const char *url = get_package_url("available-packages.txt");

    if (url)
    {
        const char *filename = get_package_path("available-packages.txt");

        if (filename)
        {
            struct fetch_callback callback = { 0 };

            struct package_list_info *pli = create_pli(nested_callback);

            callback.data = pli;
            callback.done = available_packages_done;

            fetch_id = fetch_file(url, filename, callback);

            if (!fetch_id)
            {
                free_pli(&pli);
                callback.data = NULL;
            }
        }
    }

    return fetch_id;
}

/*---------------------------------------------------------------------------*/

/*
 * Initialize package stuff.
 */
void package_init(void)
{
    const char *write_dir;

    /* Create the downloads directory. */

    write_dir = fs_get_write_dir();

    if (write_dir)
    {
        char *package_dir = concat_string(write_dir, "/", PACKAGE_DIR, NULL);

        if (package_dir)
        {
            if (!dir_exists(package_dir))
                fs_mkdir(PACKAGE_DIR);

            free(package_dir);
            package_dir = NULL;
        }
    }

    /* Load the list of installed packages. */

    load_installed_packages();
}

/*
 * Download the list of available packages.
 */
unsigned int package_refresh(struct fetch_callback callback)
{
    return fetch_available_packages(callback);
}

void package_quit(void)
{
    if (available_packages)
    {
        free_packages(available_packages);
        available_packages = NULL;
    }

    save_installed_packages();
    free_installed_packages();
}

int package_count(void)
{
    return available_packages ? array_len(available_packages) : 0;
}

/*
 * Find a package that has FILE in its "files" string.
 */
int package_search(const char *file)
{
    if (available_packages)
    {
        int i, n;

        for (i = 0, n = array_len(available_packages); i < n; ++i)
        {
            struct package *pkg = array_get(available_packages, i);

            if (pkg && strstr(pkg->files, file) != NULL)
                return i;
        }
    }

    return -1;
}

/*
 * Find a package by ID.
 */
int package_search_id(const char *package_id)
{
    if (available_packages)
    {
        int i, n;

        for (i = 0, n = array_len(available_packages); i < n; ++i)
        {
            struct package *pkg = array_get(available_packages, i);

            if (pkg && strcmp(pkg->id, package_id) == 0)
                return i;
        }
    }

    return -1;
}

/*
 * Find next package of TYPE (aka the first part of package ID).
 */
int package_next(const char *type, int start)
{
    if (available_packages)
    {
        int i, n;

        for (i = MAX(0, start + 1), n = array_len(available_packages); i < n; ++i)
        {
            struct package *pkg = array_get(available_packages, i);
            size_t prefix_len = strcspn(pkg->id, "-");

            if (strncmp(pkg->id, type, prefix_len) == 0)
                return i;
        }
    }

    return -1;
}

enum package_status package_get_status(int pi)
{
    if (pi >= 0 && pi < array_len(available_packages))
        return PACKAGE_GET(available_packages, pi)->status;

    return PACKAGE_NONE;
}

const char *package_get_id(int pi)
{
    if (pi >= 0 && pi < array_len(available_packages))
        return PACKAGE_GET(available_packages, pi)->id;

    return NULL;
}

const char *package_get_type(int pi)
{
    if (pi >= 0 && pi < array_len(available_packages))
        return PACKAGE_GET(available_packages, pi)->type;

    return NULL;
}

const char *package_get_name(int pi)
{
    if (pi >= 0 && pi < array_len(available_packages))
        return PACKAGE_GET(available_packages, pi)->name;

    return NULL;
}

const char *package_get_desc(int pi)
{
    if (pi >= 0 && pi < array_len(available_packages))
        return PACKAGE_GET(available_packages, pi)->desc;

    return NULL;
}

const char *package_get_shot(int pi)
{
    if (pi >= 0 && pi < array_len(available_packages))
        return PACKAGE_GET(available_packages, pi)->shot;

    return NULL;
}

const char *package_get_files(int pi)
{
    if (pi >= 0 && pi < array_len(available_packages))
        return PACKAGE_GET(available_packages, pi)->files;

    return NULL;
}

/*
 * Construct a package image filename relative to the write dir.
 */
const char *package_get_shot_filename(int pi)
{
    return get_package_path(package_get_shot(pi));
}

const char *package_get_formatted_type(int pi)
{
    const char *type = package_get_type(pi);

    if (type)
    {
        if (strcmp(type, "set") == 0)
            return _("Level Set");
        else if (strcmp(type, "ball") == 0)
            return _("Ball");
        else if (strcmp(type, "course") == 0)
            return _("Course");
        else if (strcmp(type, "base") == 0)
            return _("Base");
        else if (strcmp(type, "gui") == 0)
            return _("Theme");
    }

    return type;
}

/*---------------------------------------------------------------------------*/

struct package_fetch_info
{
    struct fetch_callback callback;
    char *temp_filename;
    char *dest_filename;
    struct package *pkg;
};

static struct package_fetch_info *create_pfi(struct package *pkg)
{
    struct package_fetch_info *pfi = malloc(sizeof (*pfi));

    if (pfi)
    {
        memset(pfi, 0, sizeof (*pfi));

        pfi->temp_filename = concat_string(get_package_path(pkg->filename), ".tmp", NULL);
        pfi->dest_filename = strdup(get_package_path(pkg->filename));

        pfi->pkg = pkg;
    }

    return pfi;
}

static void free_pfi(struct package_fetch_info *pfi)
{
    if (pfi)
    {
        if (pfi->temp_filename)
        {
            free(pfi->temp_filename);
            pfi->temp_filename = NULL;
        }

        if (pfi->dest_filename)
        {
            free(pfi->dest_filename);
            pfi->dest_filename = NULL;
        }

        free(pfi);
    }
}

/*
 * Just call the caller's callback.
 */
static void package_fetch_progress(void *data, void *data2)
{
    struct package_fetch_info *pfi = data;

    if (pfi)
    {
        if (pfi->callback.progress)
            pfi->callback.progress(pfi->callback.data, data2);
    }
}

/*
 * Add downloaded package to FS.
 */
static void package_fetch_done(void *data, void *extra_data)
{
    struct package_fetch_info *pfi = data;
    struct fetch_done *dn = extra_data;

    if (pfi)
    {
        struct package *pkg = pfi->pkg;

        /* Always prepare for worst. */
        pkg->status = PACKAGE_ERROR;

        if (dn->finished)
        {
            struct local_package *lpkg = create_local_package(pkg->id, pkg->filename);

            /* Rename from temporary name to destination name. */

            if (pfi->temp_filename && pfi->dest_filename)
                fs_rename(pfi->temp_filename, pfi->dest_filename);

            /* Add package to installed packages and to FS. */

            if (lpkg)
            {
                if (mount_local_package(lpkg))
                    pkg->status = PACKAGE_INSTALLED;
                else
                    free_local_package(&lpkg);

                lpkg = NULL;
            }

        }

        if (pfi->callback.done)
            pfi->callback.done(pfi->callback.data, extra_data);

        free_pfi(pfi);
        pfi = NULL;
    }
}

unsigned int package_fetch(int pi, struct fetch_callback callback)
{
    unsigned int fetch_id = 0;

    if (pi >= 0 && pi < array_len(available_packages))
    {
        struct package *pkg = array_get(available_packages, pi);
        const char *url = get_package_url(pkg->filename);

        if (url)
        {
            struct package_fetch_info *pfi = create_pfi(pkg);

            if (pfi)
            {
                /* Store passed callback. */

                pfi->callback = callback;

                /* Reuse variable to pass our callbacks. */

                callback.progress = package_fetch_progress;
                callback.done = package_fetch_done;
                callback.data = pfi;

                fetch_id = fetch_file(url, pfi->temp_filename, callback);

                if (fetch_id)
                {
                    pkg->status = PACKAGE_DOWNLOADING;
                }
                else
                {
                    free_pfi(pfi);
                    pfi = NULL;
                    callback.data = NULL;
                }
            }
        }
    }

    return fetch_id;
}