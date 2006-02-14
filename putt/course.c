/*   
 * Copyright (C) 2003 Robert Kooima
 *
 * NEVERPUTT is  free software; you can redistribute  it and/or modify
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
#include <string.h>

#include "config.h"
#include "course.h"
#include "hole.h"

/*---------------------------------------------------------------------------*/

struct course
{
    char holes[MAXSTR];

    char shot[MAXSTR];
    char desc[MAXSTR];
};

static int course_state = 0;

static int course;
static int count;

static struct course course_v[MAXCRS];

/*---------------------------------------------------------------------------*/

void course_init()
{
    FILE *fin;

    if (course_state)
        course_free();

    count = 0;

    if ((fin = fopen(config_data(COURSE_FILE), "r")))
    {
        while (fscanf(fin, "%s %s\n",
                      course_v[count].holes,
                      course_v[count].shot) == 2 &&
                fgets(course_v[count].desc, MAXSTR, fin))
        {
            char *q = course_v[count].desc + strlen(course_v[count].desc) - 1;

            if (*q == '\n') *q = 0;

            count++;
        }

        fclose(fin);

        course_state = 1;
    }
}

int course_exists(int i)
{
    return (0 <= i && i < count);
}

int course_count(void)
{
    return count;
}

void course_goto(int i)
{
    hole_init(course_v[i].holes);
    course = i;
}

int course_curr(void)
{
    return course;
}

void course_free(void)
{
    hole_free();
    course_state = 0;
}

void course_rand(void)
{
    course_goto(rand() % count);
    hole_goto(rand() % curr_count(), 4);
}

/*---------------------------------------------------------------------------*/

const char *course_desc(int i)
{
    return course_exists(i) ? course_v[i].desc : "";
}

const char *course_shot(int i)
{
    return course_exists(i) ? course_v[i].shot : course_v[0].shot;
}

/*---------------------------------------------------------------------------*/
