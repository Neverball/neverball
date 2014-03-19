/*
 * Copyright (C) 2006 Jean Privat
 * Part of the Neverball Project http://icculus.org/neverball/
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

#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "lang.h"
#include "common.h"
#include "config.h"
#include "base_config.h"
#include "fs.h"

/*---------------------------------------------------------------------------*/

#define GT_CODESET "UTF-8"

void gt_init(const char *domain, const char *pref)
{
#if ENABLE_NLS
    static char default_lang[MAXSTR];
    static int  default_lang_init;

    char *dir = strdup(getenv("NEVERBALL_LOCALE"));

    /* Select the location of message catalogs. */

    if (!dir)
    {
        if (path_is_abs(CONFIG_LOCALE))
            dir = strdup(CONFIG_LOCALE);
        else
            dir = concat_string(fs_base_dir(), "/", CONFIG_LOCALE, NULL);
    }

    /* Set up locale. */

    errno = 0;

    if (!setlocale(LC_ALL, ""))
    {
        log_printf("Failure to set LC_ALL to native locale (%s)\n",
                   errno ? strerror(errno) : "Unknown error");
    }

    /* The C locale is guaranteed (sort of) to be available. */

    setlocale(LC_NUMERIC, "C");

    /* Tell gettext of our language preference. */

    if (!default_lang_init)
    {
        const char *env;

        if ((env = getenv("LANGUAGE")))
            SAFECPY(default_lang, env);

        default_lang_init = 1;
    }

    if (pref && *pref)
        set_env_var("LANGUAGE", pref);
    else
        set_env_var("LANGUAGE", default_lang);

    /* Set up gettext. */

    bindtextdomain(domain, dir);
    bind_textdomain_codeset(domain, GT_CODESET);
    textdomain(domain);

    free(dir);
#else
    return;
#endif
}

const char *gt_prefix(const char *msgid)
{
#if ENABLE_NLS
    const char *msgval = gettext(msgid);
#else
    const char *msgval = msgid;
#endif

    if (msgval == msgid)
    {
        if ((msgval = strrchr(msgid, '^')))
            msgval++;
        else msgval = msgid;
    }
    return msgval;
}

/*---------------------------------------------------------------------------*/

const char *lang_path(const char *code)
{
    static char path[MAXSTR];

    SAFECPY(path, "lang/");
    SAFECAT(path, code);
    SAFECAT(path, ".txt");

    return path;
}

const char *lang_code(const char *path)
{
    return base_name_sans(path, ".txt");
}

int lang_load(struct lang_desc *desc, const char *path)
{
    if (desc && path && *path)
    {
        fs_file fp;

        memset(desc, 0, sizeof (*desc));

        if ((fp = fs_open(path, "r")))
        {
            char buf[MAXSTR];

            SAFECPY(desc->code, base_name_sans(path, ".txt"));

            while (fs_gets(buf, sizeof (buf), fp))
            {
                strip_newline(buf);

                if (str_starts_with(buf, "name1 "))
                    SAFECPY(desc->name1, buf + 6);
                else if (str_starts_with(buf, "name2 "))
                    SAFECPY(desc->name2, buf + 6);
                else if (str_starts_with(buf, "font "))
                    SAFECPY(desc->font, buf + 5);
            }

            fs_close(fp);

            if (*desc->name1)
                return 1;
        }
    }
    return 0;
}

void lang_free(struct lang_desc *desc)
{
}

/*---------------------------------------------------------------------------*/

static int scan_item(struct dir_item *item)
{
    if (str_ends_with(item->path, ".txt"))
    {
        struct lang_desc *desc;

        if ((desc = calloc(1, sizeof (*desc))))
        {
            if (lang_load(desc, item->path))
            {
                item->data = desc;
                return 1;
            }

            free(desc);
        }
    }
    return 0;
}

static void free_item(struct dir_item *item)
{
    if (item && item->data)
    {
        lang_free(item->data);

        free(item->data);
        item->data = NULL;
    }
}

static int cmp_items(const void *A, const void *B)
{
    const struct dir_item *a = A, *b = B;
    return strcmp(a->path, b->path);
}

Array lang_dir_scan(void)
{
    Array items;

    if ((items = fs_dir_scan("lang", scan_item)))
        array_sort(items, cmp_items);

    return items;
}

void lang_dir_free(Array items)
{
    int i;

    for (i = 0; i < array_len(items); i++)
        free_item(array_get(items, i));

    dir_free(items);
}

/*---------------------------------------------------------------------------*/

struct lang_desc curr_lang;

static int lang_status;

void lang_init(void)
{
    lang_quit();
    lang_load(&curr_lang, lang_path(config_get_s(CONFIG_LANGUAGE)));
    gt_init("neverball", curr_lang.code);
    lang_status = 1;
}

void lang_quit(void)
{
    if (lang_status)
    {
        lang_free(&curr_lang);
        lang_status = 0;
    }
}

/*---------------------------------------------------------------------------*/
