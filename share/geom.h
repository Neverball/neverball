#ifndef GEOM_H
#define GEOM_H

#include "solid_draw.h"
#include "common.h"

/*---------------------------------------------------------------------------*/

#define IMG_SHAD "png/shadow.png"

#define BACK_DIST   256.0f
#define FAR_DIST    512.0f
#define JUMP_HEIGHT   2.00f
#define SWCH_HEIGHT   2.00f
#define GOAL_HEIGHT   3.00f
#define GOAL_SPARKS  64

/*---------------------------------------------------------------------------*/

void geom_init(void);
void geom_free(void);

void goal_draw(struct s_rend *, float);
void jump_draw(struct s_rend *, float, int);
void swch_draw(struct s_rend *, int, int);
void flag_draw(struct s_rend *);
void mark_draw(struct s_rend *);
void vect_draw(struct s_rend *);
void back_draw(struct s_rend *, float);

/*---------------------------------------------------------------------------*/

void back_init(const char *s);
void back_free(void);
void back_draw_easy(void);

/*---------------------------------------------------------------------------*/

void shad_init(void);
void shad_free(void);
void shad_draw_set(void);
void shad_draw_clr(void);

/*---------------------------------------------------------------------------*/

enum
{
    TEX_STAGE_TEXTURE,
    TEX_STAGE_SHADOW,
    TEX_STAGE_CLIP
};

struct tex_stage
{
    GLenum unit;
    int stage;
};

struct tex_env
{
    void (*conf)(int, int);
    size_t count;
    struct tex_stage stages[4];
};

extern const struct tex_env tex_env_default;
extern const struct tex_env tex_env_shadow;
extern const struct tex_env tex_env_shadow_clip;

void tex_env_select(const struct tex_env *, ...) NULL_TERMINATED;
void tex_env_active(const struct tex_env *);
int  tex_env_stage(int);

/*---------------------------------------------------------------------------*/

void fade_draw(float);

#endif
