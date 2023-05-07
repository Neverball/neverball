#include "package.h"
#include "array.h"
#include "common.h"
#include "fetch.h"
#include "fs.h"
#include "lang.h"

struct package
{
    unsigned int size;

    char id[64];
    char type[64];
    char filename[MAXSTR];
    char files[MAXSTR];
    char name[64];
    char desc[MAXSTR];
    char shot[64];

    enum package_status status;
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
        SAFECPY(url, "/packages/");
#else
        SAFECPY(url, "https://neverball.github.io/packages/");
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
static List installed_packages;

/*
 * Add package file to FS path.
 */
static int mount_package(const char *filename)
{
    const char *write_dir = fs_get_write_dir();
    int added = 0;

    if (write_dir)
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
 * Add a package to the FS path and to the list, if not yet added.
 */
static int mount_installed_package(const char *filename)
{
    List l;

    /* Avoid double addition. */

    for (l = installed_packages; l; l = l->next)
        if (l->data && strcmp(l->data, filename) == 0)
            return 1;

    /* Attempt addition. */

    if (mount_package(filename))
    {
        installed_packages = list_cons(strdup(filename), installed_packages);
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

        while (fs_gets(line, sizeof (line), fp))
        {
            strip_newline(line);

            if (fs_exists(get_package_path(line)))
                mount_installed_package(line);
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
                if (l->data)
                    fs_printf(fp, "%s\n", l->data);

            fs_close(fp);
            fp = NULL;

            fs_persistent_sync();

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
        if (l->data)
            free(l->data);

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
            const char *dest_filename = get_package_path(pkg->filename);

            pkg->status = PACKAGE_AVAILABLE;

            if (dest_filename && fs_exists(dest_filename))
            {
                pkg->status = PACKAGE_PARTIAL;

                if (fs_size(dest_filename) == pkg->size)
                {
                    pkg->status = PACKAGE_INSTALLED;

                    mount_installed_package(pkg->filename);
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
                    SAFECPY(pkg->desc, line + 5);
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

/*
 * Queue missing package images for download.
 */
static void fetch_package_images(Array packages)
{
    if (packages)
    {
        int i, n = array_len(packages);

        for (i = 0; i < n; ++i)
        {
            struct package *pkg = array_get(packages, i);
            const char *filename = package_get_shot_filename(i);

            if (filename && *filename && !fs_exists(filename))
            {
                const char *url = get_package_url(pkg->shot);

                if (url)
                {
                    struct fetch_callback callback = { 0 };

                    fetch_url(url, filename, callback);
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

/*
 * Load the list of available packages and initiate image downloads.
 */
static void available_packages_done(void *data, void *extra_data)
{
    struct fetch_done *fd = extra_data;

    if (fd && fd->finished)
    {
        const char *filename = get_package_path("available-packages.txt");

        if (filename)
        {
            Array packages = load_packages_from_file(filename);

            if (packages)
            {
                available_packages = packages;

                /* TODO: notify the player somehow about this fact. */

                fetch_package_images(available_packages);
            }
        }
    }
}

/*
 * Download the package list.
 */
static void fetch_available_packages(void)
{
#ifdef __EMSCRIPTEN__
    const char *url = get_package_url("available-packages-emscripten.txt");
#else
    const char *url = get_package_url("available-packages.txt");
#endif

    if (url)
    {
        const char *filename = get_package_path("available-packages.txt");

        if (filename)
        {
            struct fetch_callback callback = { 0 };

            callback.done = available_packages_done;

            fetch_url(url, filename, callback);
        }
    }
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

    /* Download package list. */

    fetch_available_packages();
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
    return available_packages ? array_len(available_packages) : -1;
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

            if (pkg && strcmp(pkg->files, file) == 0)
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
            /* Rename from temporary name to destination name. */

            if (pfi->temp_filename && pfi->dest_filename)
                fs_rename(pfi->temp_filename, pfi->dest_filename);

            /* Add package to installed packages and to FS. */

            if (mount_installed_package(pkg->filename))
                pkg->status = PACKAGE_INSTALLED;
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

            /* Store passed callback. */

            pfi->callback = callback;

            /* Reuse variable to pass our callbacks. */

            callback.progress = package_fetch_progress;
            callback.done = package_fetch_done;
            callback.data = pfi;

            fetch_id = fetch_url(url, pfi->temp_filename, callback);

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

    return fetch_id;
}