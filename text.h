#ifndef TEXT_H
#define TEXT_H

#include "glext.h"

/*---------------------------------------------------------------------------*/

#define TEXT_FACE "ttf/VeraBd.ttf"

#define TXT_SML 0
#define TXT_MED 1
#define TXT_LRG 2

extern const GLfloat c_white[4];
extern const GLfloat c_black[4];
extern const GLfloat c_red[4];
extern const GLfloat c_yellow[4];
extern const GLfloat c_green[4];
extern const GLfloat c_blue[4];
extern const GLfloat c_grey[4];

/*---------------------------------------------------------------------------*/

int    text_init(int);
void   text_free(void);

void   text_size(const char *, int, int *, int *);
GLuint make_list(const char *, int, const float *, const float *);
GLuint make_text(const char *, int);
GLuint make_rect(int, int, int, int, int);

/*---------------------------------------------------------------------------*/

#endif
