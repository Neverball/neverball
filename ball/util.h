#ifndef UTIL_H
#define UTIL_H

#include "set.h"

/*---------------------------------------------------------------------------*/

enum
{
    GUI_NONE = 0,

    GUI_BACK,
    GUI_PREV,
    GUI_NEXT,
    GUI_BS,
    GUI_CL,
    GUI_CHAR,
    GUI_NAME,
    GUI_SCORE,

    GUI_LAST
};

#define GUI_SCORE_COIN  0x1
#define GUI_SCORE_TIME  0x2
#define GUI_SCORE_GOAL  0x4
#define GUI_SCORE_ALL  (GUI_SCORE_COIN | GUI_SCORE_TIME | GUI_SCORE_GOAL)

#define GUI_SCORE_NEXT(s) \
    ((((s) << 1) & GUI_SCORE_ALL) ? (s) << 1 : GUI_SCORE_COIN)

#define GUI_SCORE_PREV(s) \
    ((((s) >> 1) & GUI_SCORE_ALL) ? (s) >> 1 : GUI_SCORE_GOAL)

void gui_score_set(int);
int  gui_score_get(void);

void gui_score_board(int, unsigned int, int, int);
void set_score_board(const struct score *, int,
                     const struct score *, int,
                     const struct score *, int);

void gui_keyboard(int);
void gui_keyboard_lock(void);
char gui_keyboard_char(char);

int  gui_navig(int, int, int);
int  gui_maybe(int, const char *, int, int, int);

/*---------------------------------------------------------------------------*/

#endif
