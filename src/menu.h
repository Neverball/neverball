#ifndef MENU_H
#define MENU_H

#include "image.h"

/*---------------------------------------------------------------------------*/

struct item
{
    int x;
    int y;
    int w;
    int h;
    int lit;
    int val;
};

struct menu
{
    struct image image;
    struct item  *item;
    int x;
    int y;
    int w;
    int h;
    int num;
    int lit;
};

/*---------------------------------------------------------------------------*/

void menu_init(struct menu *, const char *, struct item *, int, int);
void menu_free(struct menu *);

void menu_paint(struct menu *, double);
void menu_point(struct menu *, int, int);
int  menu_click(struct menu *, int);

/*---------------------------------------------------------------------------*/

#endif
