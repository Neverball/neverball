#ifndef LEVEL_H
#define LEVEL_H

#include "base_config.h"

/*---------------------------------------------------------------------------*/

struct level
{
    /* (data) means that the file is relative from the data file */
    char file[MAXSTR];    /* sol main file */
    char back[MAXSTR];    /* sol background file (data) */
    char grad[MAXSTR];    /* gradiant backgound image (data) */
    char shot[MAXSTR];    /* screenshot image (data)*/
    char song[MAXSTR];    /* song file (data) */
    int  time;            /* time limit */
    int  goal;            /* coins needed */
};

int level_load(const char *, struct level *);

void level_dump_info(const struct level *);

/*---------------------------------------------------------------------------*/

#endif
