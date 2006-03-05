#ifndef UTIL_H
#define UTIL_H

#include "set.h"

/*---------------------------------------------------------------------------*/

#define GUI_NULL -100
#define GUI_BACK -101
#define GUI_PREV -102
#define GUI_NEXT -103
#define GUI_BS   -104

void gui_most_coins(int, int);
void set_most_coins(const struct score *, int);
void gui_best_times(int, int);
void set_best_times(const struct score *, int, int);

void gui_keyboard(int);
int gui_back_prev_next(int, int, int);

/*---------------------------------------------------------------------------*/

#endif
