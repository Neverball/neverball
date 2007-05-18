#ifndef UTIL_H
#define UTIL_H

#include "set.h"

/*---------------------------------------------------------------------------*/

#define GUI_NULL_MASK 0x1000
#define GUI_NULL -100
#define GUI_BACK -101
#define GUI_PREV -102
#define GUI_NEXT -103
#define GUI_BS   -104
#define GUI_CL   -105

void gui_most_coins(int, int);
void set_most_coins(const struct score *, int);
void gui_best_times(int, int);
void set_best_times(const struct score *, int, int);

void gui_keyboard(int);
void gui_keyboard_lock(void);
char gui_keyboard_char(char);

int  gui_back_prev_next(int, int, int);
int  gui_maybe(int, const char *, int, int);

/*---------------------------------------------------------------------------*/

#endif
