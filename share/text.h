#ifndef TEXT_H
#define TEXT_H

#include <SDL.h>

#ifndef MAXSTR
#define MAXSTR 256
#endif

/*---------------------------------------------------------------------------*/

int text_add_char(Uint32, char *, int);
int text_del_char(char *);
int text_length(const char *);

/*---------------------------------------------------------------------------*/

char text_input[MAXSTR];

void text_input_start(void (*cb)(void));
void text_input_stop(void);
int  text_input_str(const char *);
int  text_input_char(int);
int  text_input_del(void);

#endif
