#ifndef TEXT_H
#define TEXT_H

#include "gl.h"

/*---------------------------------------------------------------------------*/

#define TXT_SML  0
#define TXT_MED 1
#define TXT_LRG  2

int    text_init(int);
void   text_free(void);

void   text_size(const char *, int, int *, int *);
GLuint make_list(const char *, int, const float *, const float *);
GLuint make_text(const char *, int);

/*---------------------------------------------------------------------------*/

void push_ortho(int, int);
void pop_ortho(void);

/*---------------------------------------------------------------------------*/

#endif
