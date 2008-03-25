#ifndef TEXT_H
#define TEXT_H

#include <SDL.h>

/*---------------------------------------------------------------------------*/

void text_init(void);

char *text_from_locale(char *);
char *text_to_locale(char *);

int text_add_char(Uint32, char *, int, int);
int text_del_char(char *);
int text_length(const char *);

void text_quit(void);

/*---------------------------------------------------------------------------*/

#endif
