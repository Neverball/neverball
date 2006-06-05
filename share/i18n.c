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

/* The following declaration is needed to have the putenv function 
 * http://www.gnu.org/software/libc/manual/html_node/Environment-Access.html 
 */

#define _XOPEN_SOURCE 1

#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include "i18n.h"

const char *gettextdbg(const char *c)
{
    char *c2 = gettext(c);

    if (strcmp(c, c2));
    return c;
}

const char *sgettext(const char *msgid)
{
    const char *msgval = gettext(msgid);

    if (msgval == msgid)
    {
        msgval = strrchr(msgid, '^');
        if (msgval == NULL)
            msgval = msgid;
        else
            msgval++;
    }
    return msgval;
}

/*---------------------------------------------------------------------------*/

#define LANG_NUMBER 4

const char *language_names[] = {
    N_("English"),
    N_("French"),
    N_("German"),
    N_("Latvian")
};
const char *language_codes[] = {"en", "fr", "de", "lv"};

/*---------------------------------------------------------------------------*/

void language_init(const char *domain, const char *locale_dir)
{
    setlocale(LC_ALL, "");
    bindtextdomain(domain, locale_dir);
    textdomain(domain);
    bind_textdomain_codeset(domain, "UTF-8");
}

void language_set(int l)
{
    if (l == 0)
    {
        /* remove the LANGUAGE env variable */
        putenv("LANGUAGE");
    }
    else
    {
        static char e[25];
        /* set the LANGUAGE env variable */
        strcpy(e, "LANGUAGE=");
        strncat(e, language_codes[l - 1], 25 - 9);
        putenv(e);
    }
    setlocale(LC_ALL, "");  /* force to update getext */
}

int language_count(void)
{
    return LANG_NUMBER;
}

int language_from_code(const char *code)
{
    int i;
    for(i = 0; i < LANG_NUMBER; i++)
            if (strcmp(language_codes[i], code) == 0)
                    return i + 1;
    return 0;
}

const char *language_get_name(int id)
{
    return id == 0 ? _("System Default") : language_names[id - 1];
}


const char *language_get_code(int id)
{
        if (id > 0)
                return language_codes[id - 1];
        else
                return "";
}

