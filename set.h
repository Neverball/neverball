#ifndef SET_H
#define SET_H

/*---------------------------------------------------------------------------*/

#define SET_FILE "sets.txt"
#define MAXSET 8

void set_init();
void set_free();

int  set_exists(int);
void set_goto(int);
int  set_curr(void);

const char *set_name(int);
const char *set_desc(int);
void        set_shot(int);

/*---------------------------------------------------------------------------*/

#endif
