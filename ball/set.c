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

#include "glext.h"
#include "config.h"
#include "image.h"
#include "level.h"
#include "set.h"

/*---------------------------------------------------------------------------*/

struct set
{
    char init_levels[MAXSTR];
    char init_scores[MAXSTR];
    char user_scores[MAXSTR];

    char shot[MAXSTR];
    char name[MAXSTR];
    char desc[MAXSTR];

    GLuint text;
};

static int set;
static int count;

static struct set set_v[MAXSET];

/*---------------------------------------------------------------------------*/

void set_init()
{
    FILE *fin = fopen(SET_FILE, "r");

    count = 0;

    if (fin)
    {
        while (fscanf(fin, "%s %s %s %s\n",
                      set_v[count].init_levels,
                      set_v[count].init_scores,
                      set_v[count].user_scores,
                      set_v[count].shot) == 4 &&
               fgets(set_v[count].name, MAXSTR, fin) &&
               fgets(set_v[count].desc, MAXSTR, fin))
        {
            char *p = set_v[count].name + strlen(set_v[count].name) - 1;
            char *q = set_v[count].desc + strlen(set_v[count].desc) - 1;

            if (*p == '\n') *p = 0;
            if (*q == '\n') *q = 0;

            count++;
        }

        fclose(fin);
    }
}

int  set_exists(int i)
{
    return (0 <= i && i < count);
}

void set_goto(int i)
{
    level_init(set_v[i].init_levels,
               set_v[i].init_scores,
               set_v[i].user_scores);
    set = i;
}

int set_curr(void)
{
    return set;
}

void set_free()
{
    int i;

    for (i = 0; i < count; i++)
        if (glIsTexture(set_v[i].text))
        {
            glDeleteTextures(1, &set_v[i].text);
            set_v[i].text = 0;
        }

    level_free(set_v[set].user_scores);
}

/*---------------------------------------------------------------------------*/

const char *set_name(int i)
{
    return (0 <= i && i < count) ? set_v[i].name : "";
}

const char *set_desc(int i)
{
    return (0 <= i && i < count) ? set_v[i].desc : "";
}

const char *set_shot(int i)
{
    if (0 <= i && i < count)
        return set_v[i].shot;
    else
        return set_v[0].shot;
}

/*---------------------------------------------------------------------------*/
