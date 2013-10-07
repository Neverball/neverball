#ifndef ST_COMMON_H
#define ST_COMMON_H

#include "state.h"

/*---------------------------------------------------------------------------*/

struct conf_option
{
    char text[8];
    int  value;
};

void conf_slider(int id, const char *text,
                 int token, int value,
                 int *ids, int num);
int  conf_state (int id, const char *label, const char *text, int token);
void conf_toggle(int id, const char *label, int token, int value,
                 const char *text1, int value1,
                 const char *text0, int value0);
void conf_header(int id, const char *text, int token);
void conf_select(int id, const char *text, int token, int value,
                 const struct conf_option *opts, int num);

/*---------------------------------------------------------------------------*/

void common_init(int (*action_fn)(int, int));
void common_leave(struct state *st, struct state *next, int id);
void common_paint(int id, float st);
void common_timer(int id, float dt);
void common_point(int id, int x, int y, int dx, int dy);
void common_stick(int id, int a, float v, int bump);
int  common_click(int b, int d);
int  common_keybd(int c, int d);
int  common_buttn(int b, int d);


/*---------------------------------------------------------------------------*/

void conf_common_init(int (*action_fn)(int, int));
void conf_common_leave(struct state *st, struct state *next, int id);
void conf_common_paint(int id, float t);

/*---------------------------------------------------------------------------*/

/*
 * This is only a common declaration, this module does not implement
 * this state. Check out ball/st_conf.c and putt/st_conf.c instead.
 */
extern struct state st_null;

/*
 * These are actually implemented by this module.
 */
extern struct state st_video;
extern struct state st_display;
extern struct state st_resol;

/*---------------------------------------------------------------------------*/

#endif
