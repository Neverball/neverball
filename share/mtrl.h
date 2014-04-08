#ifndef MTRL_H
#define MTRL_H

#include "glext.h"
#include "solid_base.h"

/*---------------------------------------------------------------------------*/

#define tobyte(f) ((GLubyte) (f * 255.0f))

#define touint(v) ((GLuint) (tobyte((v)[0])       | \
                             tobyte((v)[1]) << 8  | \
                             tobyte((v)[2]) << 16 | \
                             tobyte((v)[3]) << 24))

struct mtrl
{
    struct b_mtrl base;

    GLuint d;                              /* 32-bit diffuse color cache     */
    GLuint a;                              /* 32-bit ambient color cache     */
    GLuint s;                              /* 32-bit specular color cache    */
    GLuint e;                              /* 32-bit emissive color cache    */
    GLuint h;                              /* 32-bit specular exponent cache */
    GLuint o;                              /* OpenGL texture object          */

    unsigned int refc;
};

extern int default_mtrl;

void mtrl_init(void);
void mtrl_quit(void);

int  mtrl_cache(const struct b_mtrl *);
void mtrl_free (int);

struct mtrl *mtrl_get(int);

void mtrl_cache_sol(struct s_base *);
void mtrl_free_sol (struct s_base *);

void mtrl_load_objects(void);
void mtrl_free_objects(void);

void mtrl_reload(void);

/*---------------------------------------------------------------------------*/

GLenum mtrl_func(int);

/*---------------------------------------------------------------------------*/

#endif
