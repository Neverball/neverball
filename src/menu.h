#ifndef MENU_H
#define MENU_H

/*---------------------------------------------------------------------------*/

void menu_init(int, int, int);
void menu_free(void);

void menu_item(int, int, int, int, int);
void menu_link(int, int, int, int, int);
void menu_stat(int, int);
void menu_text(int, int, int, const float *, const float *, const char *, int);

void menu_paint(void);
void menu_point(int, int);
void menu_stick(int, int);
int  menu_click(void);
int  menu_buttn(void);

/*---------------------------------------------------------------------------*/

#endif
