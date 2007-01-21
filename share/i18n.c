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

#include "i18n.h"


static const char *languages[][2] = {
    { "de", N_("German")  },
    { "en", N_("English") },
    { "es", N_("Spanish") },
    { "fr", N_("French")  },
    { "lv", N_("Latvian") },
};

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
        putenv("LANGUAGE");
    else
    {
        static char e[25];

        strcpy(e, "LANGUAGE=");
        strncat(e, languages[l - 1][0], 25 - 9);

        putenv(e);
    }

    /* Force to update gettext. */
    setlocale(LC_ALL, "");
}

int language_count(void)
{
    return sizeof (languages) / sizeof (languages[0]);
}

int language_from_code(const char *code)
{
    int i;

    for (i = 0; i < language_count(); i++)
        if (strcmp(languages[i][0], code) == 0)
            return i + 1;

    return 0;
}

const char *language_name(int id)
{
    return id == 0 ? N_("System Default") : languages[id - 1][1];
}


const char *language_code(int id)
{
    return id > 0  ? languages[id - 1][0] : "";
}

/*---------------------------------------------------------------------------*/

const char *sgettext(const char *msgid)
{
    const char *msgval = gettext(msgid);

    if (msgval == msgid)
    {
        if ((msgval = strrchr(msgid, '^')))
            msgval++;
        else
            msgval = msgid;
    }
    return msgval;
}

/*---------------------------------------------------------------------------*/
