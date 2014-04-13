#ifndef THEME_H
#define THEME_H

#include "base_config.h"
#include "glext.h"

#define THEME_IMAGES_MAX 4

struct theme
{
    GLuint tex[THEME_IMAGES_MAX];

    GLfloat t[4];
    GLfloat s[4];
};

int  theme_load(struct theme *, const char *name);
void theme_free(struct theme *);

#endif
