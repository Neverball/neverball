#ifndef MENU_H
#define MENU_H

#include "gl.h"

/*---------------------------------------------------------------------------*/

struct text
{
    GLuint text;
    GLuint list;

    int x, y;
};

struct item
{
    int state;

    int x0, y0;
    int x1, y1;

    int U, D;
    int L, R;
};

struct menu
{
    struct text *text;
    struct item *item;

    int ntext;
    int nitem;
    int value;
};

/*---------------------------------------------------------------------------*/

void menu_init(struct menu *, int, int, int);
void menu_free(struct menu *);

void menu_item(struct menu *, int, int, int, int, int);
void menu_link(struct menu *, int, int, int, int, int);
void menu_stat(struct menu *, int, int);
void menu_text(struct menu *, int, int, int,
               const float *, const float *, const char *, int);

void menu_paint(struct menu *, double);
void menu_point(struct menu *, int, int);
int  menu_click(struct menu *);
void menu_stick(struct menu *, int, int);
int  menu_buttn(struct menu *);

/*---------------------------------------------------------------------------*/

#endif
