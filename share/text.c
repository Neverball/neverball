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

#include <stdio.h>
#include <string.h>
#include <errno.h>

#if ENABLE_NLS
#include <iconv.h>
#endif

#include "text.h"

/*---------------------------------------------------------------------------*/

#define MAXSTR 256

/*---------------------------------------------------------------------------*/

#if ENABLE_NLS
static iconv_t conv_from_locale = 0;
static iconv_t conv_to_locale = 0;
#endif

void text_init(void)
{
#if ENABLE_NLS
    if ((conv_from_locale = iconv_open("UTF-8", "")) == (iconv_t) -1)
        fprintf(stderr, "Error: %s\n", strerror(errno));

    if ((conv_to_locale = iconv_open("", "UTF-8")) == (iconv_t) -1)
        fprintf(stderr, "Error: %s\n", strerror(errno));
#else
    return;
#endif
}

void text_quit(void)
{
#if ENABLE_NLS
    if (conv_from_locale != (iconv_t) -1)
        iconv_close(conv_from_locale);

    if (conv_to_locale != (iconv_t) -1)
        iconv_close(conv_to_locale);
#else
    return;
#endif
}

/*---------------------------------------------------------------------------*/

char *text_from_locale(char *str0)
{
#if ENABLE_NLS
    static char buffer[MAXSTR * 2];

    char *str0p = str0;
    char *str1p = buffer;

    size_t l0 = strlen(str0);
    size_t l1 = sizeof (buffer);

    if (conv_from_locale == (iconv_t) -1)
        return str0;

    if (iconv(conv_from_locale, &str0p, &l0, &str1p, &l1) == (size_t) -1)
        fprintf(stderr, "Error while converting to UTF-8: %s\n",
                strerror(errno));

    *str1p = '\0';

    return buffer;
#else
    return str0;
#endif
}

char *text_to_locale(char *str0)
{
#if ENABLE_NLS
    static char buffer[MAXSTR * 2];

    char *str0p = str0;
    char *str1p = buffer;

    size_t l0 = strlen(str0);
    size_t l1 = sizeof (buffer);

    if (conv_to_locale == (iconv_t) -1)
        return str0;

    if (iconv(conv_to_locale, &str0p, &l0, &str1p, &l1) == (size_t) -1)
        fprintf(stderr, "Error while converting from UTF-8: %s\n",
                strerror(errno));

    *str1p = '\0';

    return buffer;
#else
    return str0;
#endif
}

/*---------------------------------------------------------------------------*/

int text_add_char(Uint32 unicode, char *string, int maxbytes, int maxchars)
{
    size_t pos = strlen(string);
    int l;

    if      (unicode < 0x80)    l = 1;
    else if (unicode < 0x0800)  l = 2;
    else if (unicode < 0x10000) l = 3;
    else                        l = 4;

    if ((pos + l >= maxbytes) || (text_length(string) + 1 >= maxchars))
        return 0;

    if (unicode < 0x80)
        string[pos++] = (char) unicode;
    else if (unicode < 0x0800)
    {
        string[pos++] = (char) ((unicode >> 6) | 0xC0);
        string[pos++] = (char) ((unicode & 0x3F) | 0x80);
    }
    else if (unicode < 0x10000)
    {
        string[pos++] = (char) ((unicode >> 12) | 0xE0);
        string[pos++] = (char) (((unicode >> 6) & 0x3F) | 0x80);
        string[pos++] = (char) ((unicode & 0x3F) | 0x80);
    }
    else
    {
        string[pos++] = (char) ((unicode >> 18) | 0xF0);
        string[pos++] = (char) (((unicode >> 12) & 0x3F) | 0x80);
        string[pos++] = (char) (((unicode >> 6) & 0x3F) | 0x80);
        string[pos++] = (char) ((unicode & 0x3F) | 0x80);
    }

    string[pos++] = 0;

    return l;
}

int text_del_char(char *string)
{
    int pos = (int) strlen(string) - 1;

    while (pos >= 0 && ((string[pos] & 0xC0) == 0x80))
        string[pos--] = 0;

    if (pos >= 0)
    {
        string[pos] = 0;
        return 1;
    }

    return 0;
}

int text_length(const char *string)
{
    int result = 0;

    while (*string != '\0')
        if ((*string++ & 0xC0) != 0x80)
            result++;

    return result;
}

/*---------------------------------------------------------------------------*/
