/*   
 * Copyright (C) 2003 Robert Kooima
 *
 * NEVERBALL is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "glext.h"
#include "vec3.h"
#include "geom.h"
#include "solid.h"
#include "config.h"

#define SMALL 1.0e-5f
#define LARGE 1.0e+5f

/*---------------------------------------------------------------------------*/

static float erp(float t)
{
    return 3.0f * t * t - 2.0f * t * t * t;
}

static float derp(float t)
{
    return 6.0f * t     - 6.0f * t * t;
}

static void sol_body_v(float v[3],
                       const struct s_file *fp,
                       const struct s_body *bp)
{
    if (bp->pi >= 0 && fp->pv[bp->pi].f)
    {
        const struct s_path *pp = fp->pv + bp->pi;
        const struct s_path *pq = fp->pv + pp->pi;

        v_sub(v, pq->p, pp->p);
        v_scl(v, v, 1.0f / pp->t);

        v_scl(v, v, derp(bp->t / pp->t));
    }
    else
    {
        v[0] = 0.0f;
        v[1] = 0.0f;
        v[2] = 0.0f;
    }
}

static void sol_body_p(float p[3],
                       const struct s_file *fp,
                       const struct s_body *bp)
{
    float v[3];

    if (bp->pi >= 0)
    {
        const struct s_path *pp = fp->pv + bp->pi;
        const struct s_path *pq = fp->pv + pp->pi;

        v_sub(v, pq->p, pp->p);
        v_mad(p, pp->p, v, erp(bp->t / pp->t));
    }
    else
    {
        p[0] = 0.0f;
        p[1] = 0.0f;
        p[2] = 0.0f;
    }
}

/*---------------------------------------------------------------------------*/

static short sol_enum_mtrl(const struct s_file *fp,
                           const struct s_body *bp, short mi)
{
    short li, gi, c = 0;

    /* Count all lump geoms with this material. */

    for (li = 0; li < bp->lc; li++)
    {
        short g0 = fp->lv[bp->l0 + li].g0;
        short gc = fp->lv[bp->l0 + li].gc;

        for (gi = 0; gi < gc; gi++)
            if (fp->gv[fp->iv[g0 + gi]].mi == mi)
                c++;
    }
                    
    /* Count all body geoms with this material. */

    for (gi = 0; gi < bp->gc; gi++)
        if (fp->gv[fp->iv[bp->g0 + gi]].mi == mi)
            c++;

    return c;
}

static short sol_enum_body(const struct s_file *fp,
                           const struct s_body *bp, short fl)
{
    short mi, c = 0;

    /* Count all geoms with this flag. */

    for (mi = 0; mi < fp->mc; mi++)
        if (fp->mv[mi].fl & fl)
            c = c + sol_enum_mtrl(fp, bp, mi);

    return c;
}

/*---------------------------------------------------------------------------*/
/*
 * The  following code  renders a  body in  a  ludicrously inefficient
 * manner.  It iterates the materials and scans the data structure for
 * geometry using each.  This  has the effect of absolutely minimizing
 * material  changes,  texture  bindings,  and  Begin/End  pairs,  but
 * maximizing trips through the data.
 *
 * However, this  is only done once  for each level.   The results are
 * stored in display lists.  Thus, it is well worth it.
 */

static void sol_draw_mtrl(const struct s_file *fp, short i)
{
    const struct s_mtrl *mp = fp->mv + i;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   mp->a);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mp->d);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mp->s);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  mp->e);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mp->h);

    if (mp->fl & M_ENVIRONMENT)
    {
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);

        glBindTexture(GL_TEXTURE_2D, mp->o);

        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    }
    else
    {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);

        glBindTexture(GL_TEXTURE_2D, mp->o);
    }
}

static void sol_draw_geom(const struct s_file *fp,
                          const struct s_geom *gp, short mi, int s)
{
    if (gp->mi == mi)
    {
        const float *ui = fp->tv[gp->ti].u;
        const float *uj = fp->tv[gp->tj].u;
        const float *uk = fp->tv[gp->tk].u;

        const float *ni = fp->sv[gp->si].n;
        const float *nj = fp->sv[gp->sj].n;
        const float *nk = fp->sv[gp->sk].n;

        const float *vi = fp->vv[gp->vi].p;
        const float *vj = fp->vv[gp->vj].p;
        const float *vk = fp->vv[gp->vk].p;

        if (s)
        {
            glMultiTexCoord2f(GL_TEXTURE0, ui[0], ui[1]);
            glMultiTexCoord2f(GL_TEXTURE1, vi[0], vi[2]);
            glNormal3fv(ni);
            glVertex3fv(vi);

            glMultiTexCoord2f(GL_TEXTURE0, uj[0], uj[1]);
            glMultiTexCoord2f(GL_TEXTURE1, vj[0], vj[2]);
            glNormal3fv(nj);
            glVertex3fv(vj);

            glMultiTexCoord2f(GL_TEXTURE0, uk[0], uk[1]);
            glMultiTexCoord2f(GL_TEXTURE1, vk[0], vk[2]);
            glNormal3fv(nk);
            glVertex3fv(vk);
        }
        else
        {
            glTexCoord2fv(ui);
            glNormal3fv(ni);
            glVertex3fv(vi);

            glTexCoord2fv(uj);
            glNormal3fv(nj);
            glVertex3fv(vj);

            glTexCoord2fv(uk);
            glNormal3fv(nk);
            glVertex3fv(vk);
        }
    }
}

static void sol_draw_lump(const struct s_file *fp,
                          const struct s_lump *lp, short mi, int s)
{
    short i;

    for (i = 0; i < lp->gc; i++)
    {
        short gi = fp->iv[lp->g0 + i];
        
        sol_draw_geom(fp, fp->gv + gi, mi, s);
    }
}

static void sol_draw_body(const struct s_file *fp,
                          const struct s_body *bp, short fl, int s)
{
    short mi, li, gi;

    /* Iterate all materials of the correct opacity. */

    for (mi = 0; mi < fp->mc; mi++)
        if (fp->mv[mi].fl & fl)
        {
            if (sol_enum_mtrl(fp, bp, mi))
            {
                /* Set the material state. */

                sol_draw_mtrl(fp, mi);

                /* Render all geometry of that material. */

                glBegin(GL_TRIANGLES);
                {
                    for (li = 0; li < bp->lc; li++)
                        sol_draw_lump(fp, fp->lv + bp->l0 + li, mi, s);
                    for (gi = 0; gi < bp->gc; gi++)
                        sol_draw_geom(fp, fp->gv + fp->iv[bp->g0 + gi], mi, s);
                }
                glEnd();
            }
        }
}

static void sol_draw_bill(const struct s_file *fp,
                          const struct s_bill *rp, const float p[3])
{
    float rx;
    float ry;
    float rz = rp->z;
    float sz = rp->r;
    float v[3];

    glPushMatrix();
    {
        glTranslatef(rp->p[0], rp->p[1], rp->p[2]);

        v_sub(v, p, rp->p);

        /* FIXME: make this not suck. */

        rx = V_DEG(fatan2f(-v[1], fsqrtf(v[0] * v[0] + v[2] * v[2])));
        ry = V_DEG(fatan2f(+v[0], v[2]));

        glRotatef(ry, 0.f, 1.f, 0.f);
        glRotatef(rx, 1.f, 0.f, 0.f);
        glRotatef(rz, 0.f, 0.f, 1.f);

        sol_draw_mtrl(fp, rp->mi);

        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(-sz, -sz);
            glTexCoord2f(1.0f, 0.0f);
            glVertex2f(+sz, -sz);
            glTexCoord2f(1.0f, 1.0f);
            glVertex2f(+sz, +sz);
            glTexCoord2f(0.0f, 1.0f);
            glVertex2f(-sz, +sz);
        }
        glEnd();
    }
    glPopMatrix();
}

/*---------------------------------------------------------------------------*/

static void sol_draw_list(const struct s_file *fp,
                          const struct s_body *bp, GLuint list, int s)
{
    float p[3];

    sol_body_p(p, fp, bp);

    glPushMatrix();
    {
        /* Translate a moving body. */

        glTranslatef(p[0], p[1], p[2]);

        /* Translate the shadow on a moving body. */

        if (s)
        {
            glActiveTexture(GL_TEXTURE1);
            glMatrixMode(GL_TEXTURE);
            {
                glPushMatrix();
                glTranslatef(p[0], p[2], 0.0f);
            }
            glMatrixMode(GL_MODELVIEW);
            glActiveTexture(GL_TEXTURE0);
        }
        
        /* Draw the body. */

        glCallList(list);

        /* Pop the shadow translation. */

        if (s)
        {
            glActiveTexture(GL_TEXTURE1);
            glMatrixMode(GL_TEXTURE);
            {
                glPopMatrix();
            }
            glMatrixMode(GL_MODELVIEW);
            glActiveTexture(GL_TEXTURE0);
        }
    }
    glPopMatrix();
}

void sol_draw(const struct s_file *fp, int s, const float p[3])
{
    short i;

    glPushAttrib(GL_TEXTURE_BIT);
    glPushAttrib(GL_LIGHTING_BIT);
    glPushAttrib(GL_COLOR_BUFFER_BIT);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    {
        /* Render all reflective geometry into the color and depth buffers. */

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (i = 0; i < fp->bc; i++)
            if (fp->bv[i].rl)
                sol_draw_list(fp, fp->bv + i, fp->bv[i].rl, s);

        /* Render all obaque geometry into the color and depth buffers. */

        glDisable(GL_BLEND);

        for (i = 0; i < fp->bc; i++)
            if (fp->bv[i].ol)
                sol_draw_list(fp, fp->bv + i, fp->bv[i].ol, s);

        glEnable(GL_BLEND);

        /* Render all translucent geometry into only the color buffer. */

        glDepthMask(GL_FALSE);

        for (i = 0; i < fp->bc; i++)
            if (fp->bv[i].tl)
                sol_draw_list(fp, fp->bv + i, fp->bv[i].tl, s);

        for (i = 0; i < fp->rc; i++)
            sol_draw_bill(fp, fp->rv + i, p);
    }
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
}

void sol_refl(const struct s_file *fp)
{
    float p[3];
    short bi;

    glDepthMask(GL_FALSE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    {
        for (bi = 0; bi < fp->bc; bi++)
            if (fp->bv[bi].rl)
            {
                sol_body_p(p, fp, fp->bv + bi);

                glPushMatrix();
                {
                    glTranslatef(p[0], p[1], p[2]);
                    glCallList(fp->bv[bi].rl);
                }
                glPopMatrix();
            }
    }
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
}

/*---------------------------------------------------------------------------*/

static void sol_load_objects(struct s_file *fp, int s)
{
    short i;

    for (i = 0; i < fp->bc; i++)
    {
        struct s_body *bp = fp->bv + i;

        /* Draw all opaque geometry. */

        if (sol_enum_body(fp, bp, M_OPAQUE | M_ENVIRONMENT))
        {
            fp->bv[i].ol = glGenLists(1);
            
            glNewList(fp->bv[i].ol, GL_COMPILE);
            {
                sol_draw_body(fp, fp->bv + i, M_OPAQUE | M_ENVIRONMENT, s);
            }
            glEndList();
        }
        else fp->bv[i].ol = 0;

        /* Draw all translucent geometry. */

        if (sol_enum_body(fp, bp, M_TRANSPARENT))
        {
            fp->bv[i].tl = glGenLists(1);

            glNewList(fp->bv[i].tl, GL_COMPILE);
            {
                sol_draw_body(fp, fp->bv + i, M_TRANSPARENT, s);
            }
            glEndList();
        }
        else fp->bv[i].tl = 0;

        /* Draw all reflective geometry. */

        if (sol_enum_body(fp, bp, M_REFLECTIVE))
        {
            fp->bv[i].rl = glGenLists(1);

            glNewList(fp->bv[i].rl, GL_COMPILE);
            {
                sol_draw_body(fp, fp->bv + i, M_REFLECTIVE, s);
            }
            glEndList();
        }
        else fp->bv[i].rl = 0;
    }
}

static void sol_load_textures(struct s_file *fp, int k)
{
    SDL_Surface *s;
    SDL_Surface *d;
    char tga[64];
    char jpg[64];

    int i;

    for (i = 0; i < fp->mc; i++)
    {
        strncpy(tga, fp->mv[i].f, PATHMAX);
        strcat(tga, ".tga");
        strncpy(jpg, fp->mv[i].f, PATHMAX);
        strcat(jpg, ".jpg");

        /* Prefer a lossless copy of the texture over a lossy compression. */

        if ((s = IMG_Load(tga)) || (s = IMG_Load(jpg)))
        {
            GLenum f0 = (s->format->BitsPerPixel == 32) ? GL_RGBA : GL_RGB;
            GLenum f1 = (s->format->BitsPerPixel == 32) ? GL_BGRA : GL_RGB;

            glGenTextures(1, &fp->mv[i].o);
            glBindTexture(GL_TEXTURE_2D, fp->mv[i].o);

            if (k > 1)
            {
                /* Create a new buffer and copy the scaled image to it. */

                d = SDL_CreateRGBSurface(SDL_SWSURFACE, s->w / k, s->h / k,
                                         s->format->BitsPerPixel,
                                         RMASK, GMASK, BMASK, AMASK);
                if (d)
                {
                    SDL_LockSurface(s);
                    SDL_LockSurface(d);
                    {
                        gluScaleImage(f1,
                                      s->w, s->h, GL_UNSIGNED_BYTE, s->pixels,
                                      d->w, d->h, GL_UNSIGNED_BYTE, d->pixels);
                    }
                    SDL_UnlockSurface(d);
                    SDL_UnlockSurface(s);

                    /* Load the scaled image. */

                    glTexImage2D(GL_TEXTURE_2D, 0, f0, d->w, d->h,
                                 0, f1, GL_UNSIGNED_BYTE, d->pixels);

                    SDL_FreeSurface(d);
                }
            }
            else
                glTexImage2D(GL_TEXTURE_2D, 0, f0, s->w, s->h,
                             0, f1, GL_UNSIGNED_BYTE, s->pixels);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            SDL_FreeSurface(s);
        }
    }
}

int sol_load(struct s_file *fp, const char *filename, int k, int s)
{
    FILE *fin;

    memset(fp, 0, sizeof (struct s_file));

    if ((fin = fopen(filename, FMODE_RB)))
    {
        short n[19];

        fread(n, sizeof (short), 19, fin);

        fp->mc = n[0];
        fp->vc = n[1];
        fp->ec = n[2];
        fp->sc = n[3];
        fp->tc = n[4];
        fp->gc = n[5];
        fp->lc = n[6];
        fp->nc = n[7];
        fp->pc = n[8];
        fp->bc = n[9];
        fp->cc = n[10];
        fp->zc = n[11];
        fp->jc = n[12];
        fp->xc = n[13];
        fp->rc = n[14];
        fp->uc = n[15];
        fp->wc = n[16];
        fp->ac = n[17];
        fp->ic = n[18];

        fp->mv = (struct s_mtrl *) calloc(n[0],  sizeof (struct s_mtrl));
        fp->vv = (struct s_vert *) calloc(n[1],  sizeof (struct s_vert));
        fp->ev = (struct s_edge *) calloc(n[2],  sizeof (struct s_edge));
        fp->sv = (struct s_side *) calloc(n[3],  sizeof (struct s_side));
        fp->tv = (struct s_texc *) calloc(n[4],  sizeof (struct s_texc));
        fp->gv = (struct s_geom *) calloc(n[5],  sizeof (struct s_geom));
        fp->lv = (struct s_lump *) calloc(n[6],  sizeof (struct s_lump));
        fp->nv = (struct s_node *) calloc(n[7],  sizeof (struct s_node));
        fp->pv = (struct s_path *) calloc(n[8],  sizeof (struct s_path));
        fp->bv = (struct s_body *) calloc(n[9],  sizeof (struct s_body));
        fp->cv = (struct s_coin *) calloc(n[10], sizeof (struct s_coin));
        fp->zv = (struct s_goal *) calloc(n[11], sizeof (struct s_goal));
        fp->jv = (struct s_jump *) calloc(n[12], sizeof (struct s_jump));
        fp->xv = (struct s_swch *) calloc(n[13], sizeof (struct s_swch));
        fp->rv = (struct s_bill *) calloc(n[14], sizeof (struct s_bill));
        fp->uv = (struct s_ball *) calloc(n[15], sizeof (struct s_ball));
        fp->wv = (struct s_view *) calloc(n[16], sizeof (struct s_view));
        fp->av = (char          *) calloc(n[17], sizeof (char));
        fp->iv = (short         *) calloc(n[18], sizeof (short));

        fread(fp->mv, sizeof (struct s_mtrl), n[0],  fin);
        fread(fp->vv, sizeof (struct s_vert), n[1],  fin);
        fread(fp->ev, sizeof (struct s_edge), n[2],  fin);
        fread(fp->sv, sizeof (struct s_side), n[3],  fin);
        fread(fp->tv, sizeof (struct s_texc), n[4],  fin);
        fread(fp->gv, sizeof (struct s_geom), n[5],  fin);
        fread(fp->lv, sizeof (struct s_lump), n[6],  fin);
        fread(fp->nv, sizeof (struct s_node), n[7],  fin);
        fread(fp->pv, sizeof (struct s_path), n[8],  fin);
        fread(fp->bv, sizeof (struct s_body), n[9],  fin);
        fread(fp->cv, sizeof (struct s_coin), n[10], fin);
        fread(fp->zv, sizeof (struct s_goal), n[11], fin);
        fread(fp->jv, sizeof (struct s_jump), n[12], fin);
        fread(fp->xv, sizeof (struct s_swch), n[13], fin);
        fread(fp->rv, sizeof (struct s_bill), n[14], fin);
        fread(fp->uv, sizeof (struct s_ball), n[15], fin);
        fread(fp->wv, sizeof (struct s_view), n[16], fin);
        fread(fp->av, sizeof (char),          n[17], fin);
        fread(fp->iv, sizeof (short),         n[18], fin);

        fclose(fin);

        sol_load_textures(fp, k);
        sol_load_objects (fp, s);

        return 1;
    }
    return 0;
}

int sol_stor(struct s_file *fp, const char *filename)
{
    FILE *fout;

    if ((fout = fopen(filename, FMODE_WB)))
    {
        short n[19];

        n[0]  = fp->mc;
        n[1]  = fp->vc;
        n[2]  = fp->ec;
        n[3]  = fp->sc;
        n[4]  = fp->tc;
        n[5]  = fp->gc;
        n[6]  = fp->lc;
        n[7]  = fp->nc;
        n[8]  = fp->pc;
        n[9]  = fp->bc;
        n[10] = fp->cc;
        n[11] = fp->zc;
        n[12] = fp->jc;
        n[13] = fp->xc;
        n[14] = fp->rc;
        n[15] = fp->uc;
        n[16] = fp->wc;
        n[17] = fp->ac;
        n[18] = fp->ic;

        fwrite(n, sizeof (short), 19, fout);

        fwrite(fp->mv, sizeof (struct s_mtrl), n[0],  fout);
        fwrite(fp->vv, sizeof (struct s_vert), n[1],  fout);
        fwrite(fp->ev, sizeof (struct s_edge), n[2],  fout);
        fwrite(fp->sv, sizeof (struct s_side), n[3],  fout);
        fwrite(fp->tv, sizeof (struct s_texc), n[4],  fout);
        fwrite(fp->gv, sizeof (struct s_geom), n[5],  fout);
        fwrite(fp->lv, sizeof (struct s_lump), n[6],  fout);
        fwrite(fp->nv, sizeof (struct s_node), n[7],  fout);
        fwrite(fp->pv, sizeof (struct s_path), n[8],  fout);
        fwrite(fp->bv, sizeof (struct s_body), n[9],  fout);
        fwrite(fp->cv, sizeof (struct s_coin), n[10], fout);
        fwrite(fp->zv, sizeof (struct s_goal), n[11], fout);
        fwrite(fp->jv, sizeof (struct s_jump), n[12], fout);
        fwrite(fp->xv, sizeof (struct s_swch), n[13], fout);
        fwrite(fp->rv, sizeof (struct s_bill), n[14], fout);
        fwrite(fp->uv, sizeof (struct s_ball), n[15], fout);
        fwrite(fp->wv, sizeof (struct s_view), n[16], fout);
        fwrite(fp->av, sizeof (char),          n[17], fout);
        fwrite(fp->iv, sizeof (short),         n[18], fout);

        fclose(fout);

        return 1;
    }
    return 0;
}

void sol_free(struct s_file *fp)
{
    short i;

    for (i = 0; i < fp->mc; i++)
    {
        if (glIsTexture(fp->mv[i].o))
            glDeleteTextures(1, &fp->mv[i].o);
    }

    for (i = 0; i < fp->bc; i++)
    {
        if (glIsList(fp->bv[i].ol))
            glDeleteLists(fp->bv[i].ol, 1);
        if (glIsList(fp->bv[i].tl))
            glDeleteLists(fp->bv[i].tl, 1);
        if (glIsList(fp->bv[i].rl))
            glDeleteLists(fp->bv[i].rl, 1);
    }

    if (fp->mv) free(fp->mv);
    if (fp->vv) free(fp->vv);
    if (fp->ev) free(fp->ev);
    if (fp->sv) free(fp->sv);
    if (fp->tv) free(fp->tv);
    if (fp->gv) free(fp->gv);
    if (fp->lv) free(fp->lv);
    if (fp->nv) free(fp->nv);
    if (fp->pv) free(fp->pv);
    if (fp->bv) free(fp->bv);
    if (fp->cv) free(fp->cv);
    if (fp->zv) free(fp->zv);
    if (fp->jv) free(fp->jv);
    if (fp->xv) free(fp->xv);
    if (fp->rv) free(fp->rv);
    if (fp->uv) free(fp->uv);
    if (fp->wv) free(fp->wv);
    if (fp->av) free(fp->av);
    if (fp->iv) free(fp->iv);

    memset(fp, 0, sizeof (struct s_file));
}

/*---------------------------------------------------------------------------*/
/* Solves (p + v * t) . (p + v * t) == r * r for smallest t.                 */

static float v_sol(const float p[3], const float v[3], float r)
{
    float a = v_dot(v, v);
    float b = v_dot(v, p) * 2.0f;
    float c = v_dot(p, p) - r * r;
    float d = b * b - 4.0f * a * c;

    if (a == 0.0f) return LARGE;
    if (d <  0.0f) return LARGE;

    if (d == 0.0f)
        return -b * 0.5f / a;
    else
    {
        float t0 = 0.5f * (-b - fsqrtf(d)) / a;
        float t1 = 0.5f * (-b + fsqrtf(d)) / a;
        float t  = (t0 < t1) ? t0 : t1;

        return (t < 0.0f) ? LARGE : t;
    }
}

/*---------------------------------------------------------------------------*/

/*
 * Compute the  earliest time  and position of  the intersection  of a
 * sphere and a vertex.
 *
 * The sphere has radius R and moves along vector V from point P.  The
 * vertex moves  along vector  W from point  Q in a  coordinate system
 * based at O.
 */
static float v_vert(float Q[3],
                    const float o[3],
                    const float q[3],
                    const float w[3],
                    const float p[3],
                    const float v[3], float r)
{
    float O[3], P[3], V[3];
    float t = LARGE;

    v_add(O, o, q);
    v_sub(P, p, O);
    v_sub(V, v, w);

    if (v_dot(P, V) < 0.0f)
    {
        t = v_sol(P, V, r);

        if (t < LARGE)
            v_mad(Q, O, w, t);
    }
    return t;
}

/*
 * Compute the  earliest time  and position of  the intersection  of a
 * sphere and an edge.
 *
 * The sphere has radius R and moves along vector V from point P.  The
 * edge moves along vector W from point Q in a coordinate system based
 * at O.  The edge extends along the length of vector U.
 */
static float v_edge(float Q[3],
                    const float o[3],
                    const float q[3],
                    const float u[3],
                    const float w[3],
                    const float p[3],
                    const float v[3], float r)
{
    float d[3], e[3];
    float P[3], V[3];
    float du, eu, uu, s, t;

    v_sub(d, p, o);
    v_sub(d, d, q);
    v_sub(e, v, w);

    du = v_dot(d, u);
    eu = v_dot(e, u);
    uu = v_dot(u, u);

    v_mad(P, d, u, -du / uu);
    v_mad(V, e, u, -eu / uu);

    t = v_sol(P, V, r);
    s = (du + eu * t) / uu;

    if (0.0f < t && t < LARGE && 0.0f < s && s < 1.0f)
    {
        v_mad(d, o, w, t);
        v_mad(e, q, u, s);
        v_add(Q, e, d);
    }
    else
        t = LARGE;

    return t;
}

/*
 * Compute  the earlist  time and  position of  the intersection  of a
 * sphere and a plane.
 *
 * The sphere has radius R and moves along vector V from point P.  The
 * plane  oves  along  vector  W.   The  plane has  normal  N  and  is
 * positioned at distance D from the origin O along that normal.
 */
static float v_side(float Q[3],
                    const float o[3],
                    const float w[3],
                    const float n[3], float d,
                    const float p[3],
                    const float v[3], float r)
{
    float vn = v_dot(v, n);
    float wn = v_dot(w, n);
    float t  = LARGE;

    if (vn - wn <= 0.0f)
    {
        float on = v_dot(o, n);
        float pn = v_dot(p, n);

        float u = (r + d + on - pn) / (vn - wn);
        float a = (    d + on - pn) / (vn - wn);

        if (0.0f <= u)
        {
            t = u;

            v_mad(Q, p, v, +t);
            v_mad(Q, Q, n, -r);
        }
        else if (0.0f <= a)
        {
            t = 0;

            v_mad(Q, p, v, +t);
            v_mad(Q, Q, n, -r);
        }
    }
    return t;
}

/*---------------------------------------------------------------------------*/

/*
 * Compute the new  linear and angular velocities of  a bouncing ball.
 * Q  gives the  position  of the  point  of impact  and  W gives  the
 * velocity of the object being impacted.
 */
static float sol_bounce(struct s_ball *up,
                        const float q[3],
                        const float w[3], float dt)
{
    const float kb = 1.10f;
    const float ke = 0.70f;
    const float km = 0.20f;

    float n[3], r[3], d[3], u[3], vn, wn, xn, yn;
    float *p = up->p;
    float *v = up->v;

    /* Find the normal of the impact. */

    v_sub(r, p, q);
    v_sub(d, v, w);
    v_nrm(n, r);

    /* Find the new angular velocity. */

    v_crs(up->w, d, r);
    v_scl(up->w, up->w, -1.0f / (up->r * up->r));

    /* Find the new linear velocity. */

    vn = v_dot(v, n);
    wn = v_dot(w, n);
    xn = (vn < 0.0f) ? -vn * ke : vn;
    yn = (wn > 0.0f) ?  wn * kb : wn;

    v_mad(u, w, n, -wn);
    v_mad(v, v, n, -vn);
    v_mad(v, v, u, +km * dt);
    v_mad(v, v, n, xn + yn); 

    v_mad(p, q, n, up->r);

    /* Return the "energy" of the impact, to determine the sound amplitude. */

    return fabsf(v_dot(n, d));
}

/*---------------------------------------------------------------------------*/

/*
 * Compute the states of all switches after DT seconds have passed.
 */
static void sol_swch_step(struct s_file *fp, float dt)
{
    short xi;

    for (xi = 0; xi < fp->xc; xi++)
    {
        struct s_swch *xp = fp->xv + xi;

        if (xp->t > 0)
        {
            xp->t -= dt;

            if (xp->t <= 0)
            {
                short pi = xp->pi;
                short pj = xp->pi;

                do  /* Tortoise and hare cycle traverser. */
                {
                    fp->pv[pi].f = xp->f0;
                    fp->pv[pj].f = xp->f0;

                    pi = fp->pv[pi].pi;
                    pj = fp->pv[pj].pi;
                    pj = fp->pv[pj].pi;
                }
                while (pi != pj);

                xp->f = xp->f0;
            }
        }
    }
}

/*
 * Compute the positions of all bodies after DT seconds have passed.
 */
static void sol_body_step(struct s_file *fp, float dt)
{
    short i;

    for (i = 0; i < fp->bc; i++)
    {
        struct s_body *bp = fp->bv + i;
        struct s_path *pp = fp->pv + bp->pi;

        if (bp->pi >= 0 && pp->f)
        {
            bp->t += dt;

            if (bp->t >= pp->t)
            {
                bp->t -= pp->t;
                bp->pi = pp->pi;
            }
        }
    }
}

/*
 * Compute the positions of all balls after DT seconds have passed.
 */
static void sol_ball_step(struct s_file *fp, float dt)
{
    short i;

    for (i = 0; i < fp->uc; i++)
    {
        struct s_ball *up = fp->uv + i;

        v_mad(up->p, up->p, up->v, dt);

        if (v_len(up->w) > 0.0f)
        {
            float M[16];
            float w[3];
            float e[3][3];

            v_nrm(w, up->w);
            m_rot(M, w, v_len(up->w) * dt);

            m_vxfm(e[0], M, up->e[0]);
            m_vxfm(e[1], M, up->e[1]);
            m_vxfm(e[2], M, up->e[2]);

            v_crs(up->e[2], e[0], e[1]);
            v_crs(up->e[1], e[2], e[0]);
            v_crs(up->e[0], e[1], e[2]);

            v_nrm(up->e[0], up->e[0]);
            v_nrm(up->e[1], up->e[1]);
            v_nrm(up->e[2], up->e[2]);
        }
    }
}

/*---------------------------------------------------------------------------*/

static float sol_test_vert(float dt,
                           float T[3],
                           const struct s_ball *up,
                           const struct s_vert *vp,
                           const float o[3],
                           const float w[3])
{
    return v_vert(T, o, vp->p, w, up->p, up->v, up->r);
}

static float sol_test_edge(float dt,
                           float T[3],
                           const struct s_ball *up,
                           const struct s_file *fp,
                           const struct s_edge *ep,
                           const float o[3],
                           const float w[3])
{
    float q[3];
    float u[3];

    v_cpy(q, fp->vv[ep->vi].p);
    v_sub(u, fp->vv[ep->vj].p,
          fp->vv[ep->vi].p);

    return v_edge(T, o, q, u, w, up->p, up->v, up->r);
}

static float sol_test_side(float dt,
                           float T[3],
                           const struct s_ball *up,
                           const struct s_file *fp,
                           const struct s_lump *lp,
                           const struct s_side *sp,
                           const float o[3],
                           const float w[3])
{
    float t = v_side(T, o, w, sp->n, sp->d, up->p, up->v, up->r);
    short i;

    if (t < dt)
        for (i = 0; i < lp->sc; i++)
        {
            const struct s_side *sq = fp->sv + fp->iv[lp->s0 + i];

            if (sp != sq &&
                v_dot(T, sq->n) -
                v_dot(o, sq->n) -
                v_dot(w, sq->n) * t > sq->d)
                return LARGE;
        }
    return t;
}

/*---------------------------------------------------------------------------*/

static float sol_test_fore(float dt,
                           const struct s_ball *up,
                           const struct s_side *sp,
                           const float o[3],
                           const float w[3])
{
    float q[3];

    /* If the ball is not behind the plane, the test passes. */

    v_sub(q, up->p, o);

    if (v_dot(q, sp->n) - sp->d + up->r >= 0)
        return 1;

    /* if the ball is behind the plane but will hit before dt, test passes. */
    /*
      if (v_side(q, o, w, sp->n, sp->d, up->p, up->v, up->r) < dt)
      return 1;
    */
    /* If the ball is behind but moving toward the plane, test passes. */

    if (v_dot(up->v, sp->n) > 0)
        return 1;


    /* Else, test fails. */

    return 0;
}

static float sol_test_back(float dt,
                           const struct s_ball *up,
                           const struct s_side *sp,
                           const float o[3],
                           const float w[3])
{
    float q[3];

    /* If the ball is not in front of the plane, the test passes. */

    v_sub(q, up->p, o);

    if (v_dot(q, sp->n) - sp->d - up->r <= 0)
        return 1;

    /* if the ball is behind the plane but will hit before dt, test passes. */
    /*
      if (v_side(q, o, w, sp->n, sp->d, up->p, up->v, up->r) < dt)
      return 1;
    */
    /* If the ball is in front but moving toward the plane, test passes. */

    if (v_dot(up->v, sp->n) < 0)
        return 1;


    /* Else, test fails. */

    return 0;
}

/*---------------------------------------------------------------------------*/

static float sol_test_lump(float dt,
                           float T[3],
                           const struct s_ball *up,
                           const struct s_file *fp,
                           const struct s_lump *lp,
                           const float o[3],
                           const float w[3])
{
    float U[3], u, t = dt;
    short i;

    /* Short circuit a non-solid lump. */

    if (lp->fl & L_DETAIL) return t;

    /* Test all verts */

    if (up->r > 0.0f)
        for (i = 0; i < lp->vc; i++)
        {
            const struct s_vert *vp = fp->vv + fp->iv[lp->v0 + i];

            if ((u = sol_test_vert(t, U, up, vp, o, w)) < t)
            {
                v_cpy(T, U);
                t = u;
            }
        }
 
    /* Test all edges */

    if (up->r > 0.0f)
        for (i = 0; i < lp->ec; i++)
        {
            const struct s_edge *ep = fp->ev + fp->iv[lp->e0 + i];

            if ((u = sol_test_edge(t, U, up, fp, ep, o, w)) < t)
            {
                v_cpy(T, U);
                t = u;
            }
        }

    /* Test all sides */

    for (i = 0; i < lp->sc; i++)
    {
        const struct s_side *sp = fp->sv + fp->iv[lp->s0 + i];

        if ((u = sol_test_side(t, U, up, fp, lp, sp, o, w)) < t)
        {
            v_cpy(T, U);
            t = u;
        }
    }
    return t;
}

static float sol_test_node(float dt,
                           float T[3],
                           const struct s_ball *up,
                           const struct s_file *fp,
                           const struct s_node *np,
                           const float o[3],
                           const float w[3])
{
    float U[3], u, t = dt;
    short i;

    /* Test all lumps */

    for (i = 0; i < np->lc; i++)
    {
        const struct s_lump *lp = fp->lv + np->l0 + i;

        if ((u = sol_test_lump(t, U, up, fp, lp, o, w)) < t)
        {
            v_cpy(T, U);
            t = u;
        }
    }

    /* Test in front of this node */

    if (np->ni >= 0 && sol_test_fore(t, up, fp->sv + np->si, o, w))
    {
        const struct s_node *nq = fp->nv + np->ni;

        if ((u = sol_test_node(t, U, up, fp, nq, o, w)) < t)
        {
            v_cpy(T, U);
            t = u;
        }
    }

    /* Test behind this node */

    if (np->nj >= 0 && sol_test_back(t, up, fp->sv + np->si, o, w))
    {
        const struct s_node *nq = fp->nv + np->nj;

        if ((u = sol_test_node(t, U, up, fp, nq, o, w)) < t)
        {
            v_cpy(T, U);
            t = u;
        }
    }

    return t;
}

static float sol_test_body(float dt,
                           float T[3], float V[3],
                           const struct s_ball *up,
                           const struct s_file *fp,
                           const struct s_body *bp)
{
    float U[3], O[3], W[3], u, t = dt;

    const struct s_node *np = fp->nv + bp->ni;

    sol_body_p(O, fp, bp);
    sol_body_v(W, fp, bp);

    if ((u = sol_test_node(t, U, up, fp, np, O, W)) < t)
    {
        v_cpy(T, U);
        v_cpy(V, W);
        t = u;
    }
    return t;
}

static float sol_test_file(float dt,
                           float T[3], float V[3],
                           const struct s_ball *up,
                           const struct s_file *fp)
{
    float U[3], W[3], u, t = dt;
    short i;

    for (i = 0; i < fp->bc; i++)
    {
        const struct s_body *bp = fp->bv + i;

        if ((u = sol_test_body(t, U, W, up, fp, bp)) < t)
        {
            v_cpy(T, U);
            v_cpy(V, W);
            t = u;
        }
    }
    return t;
}

/*---------------------------------------------------------------------------*/

/*
 * Step the physics forward DT  seconds under the influence of gravity
 * vector G.  If the ball gets pinched between two moving solids, this
 * loop might not terminate.  It  is better to do something physically
 * impossible than  to lock up the game.   So, if we make  more than C
 * iterations, punt it.
 */

float sol_step(struct s_file *fp, const float *g, float dt, short ui, int *m)
{
    float P[3], V[3], v[3], r[3], d, e, nt, b = 0.0f, tt = dt;
    int c = 16;

    struct s_ball *up = fp->uv + ui;

    /* If the ball is in contact with a surface, apply friction. */

    v_cpy(v, up->v);
    v_cpy(up->v, g);

    if (m && sol_test_file(tt, P, V, up, fp) < 0.0005f)
    {
        v_cpy(up->v, v);
        v_sub(r, P, up->p);

        if ((d = v_dot(r, g) / (v_len(r) * v_len(g))) > 0.999f)
        {
            if ((e = (v_len(up->v) - dt)) > 0.0f)
            {
                /* Scale the linear velocity. */

                v_nrm(up->v, up->v);
                v_scl(up->v, up->v, e);

                /* Scale the angular velocity. */

                v_sub(v, V, up->v);
                v_crs(up->w, v, r);
                v_scl(up->w, up->w, -1.0f / (up->r * up->r));
            }
            else
            {
                /* Friction has brought the ball to a stop. */

                up->v[0] = 0.0f;
                up->v[1] = 0.0f;
                up->v[2] = 0.0f;

                (*m)++;
            }
        }
        else v_mad(up->v, v, g, tt);
    }
    else v_mad(up->v, v, g, tt);

    /* Test for collision. */

    while (c > 0 && tt > 0 && tt > (nt = sol_test_file(tt, P, V, up, fp)))
    {
        sol_body_step(fp, nt);
        sol_swch_step(fp, nt);
        sol_ball_step(fp, nt);

        tt -= nt;

        if (b < (d = sol_bounce(up, P, V, nt)))
            b = d;

        c--;
    }

    sol_body_step(fp, tt);
    sol_swch_step(fp, tt);
    sol_ball_step(fp, tt);

    return b;
}

/*---------------------------------------------------------------------------*/

int sol_coin_test(struct s_file *fp, float *p, float coin_r)
{
    const float *ball_p = fp->uv->p;
    const float  ball_r = fp->uv->r;
    short ci, n;

    for (ci = 0; ci < fp->cc; ci++)
    {
        float r[3];

        r[0] = ball_p[0] - fp->cv[ci].p[0];
        r[1] = ball_p[1] - fp->cv[ci].p[1];
        r[2] = ball_p[2] - fp->cv[ci].p[2];

        if (fp->cv[ci].n > 0 && v_len(r) < ball_r + coin_r)
        {
            p[0] = fp->cv[ci].p[0];
            p[1] = fp->cv[ci].p[1];
            p[2] = fp->cv[ci].p[2];

            n = fp->cv[ci].n;
            fp->cv[ci].n = 0;

            return n;
        }
    }
    return 0;
}

int sol_goal_test(struct s_file *fp, float *p, short ui)
{
    const float *ball_p = fp->uv[ui].p;
    const float  ball_r = fp->uv[ui].r;
    short zi;

    for (zi = 0; zi < fp->zc; zi++)
    {
        float r[3];

        r[0] = ball_p[0] - fp->zv[zi].p[0];
        r[1] = ball_p[2] - fp->zv[zi].p[2];
        r[2] = 0;

        if (v_len(r) < fp->zv[zi].r * 1.1 - ball_r &&
            ball_p[1] > fp->zv[zi].p[1] &&
            ball_p[1] < fp->zv[zi].p[1] + GOAL_HEIGHT / 2)
        {
            p[0] = fp->zv[zi].p[0];
            p[1] = fp->zv[zi].p[1];
            p[2] = fp->zv[zi].p[2];

            return 1;
        }
    }
    return 0;
}

int sol_jump_test(struct s_file *fp, float *p, short ui)
{
    const float *ball_p = fp->uv[ui].p;
    const float  ball_r = fp->uv[ui].r;
    short ji;

    for (ji = 0; ji < fp->jc; ji++)
    {
        float r[3];

        r[0] = ball_p[0] - fp->jv[ji].p[0];
        r[1] = ball_p[2] - fp->jv[ji].p[2];
        r[2] = 0;

        if (v_len(r) < fp->jv[ji].r - ball_r &&
            ball_p[1] > fp->jv[ji].p[1] &&
            ball_p[1] < fp->jv[ji].p[1] + JUMP_HEIGHT / 2)
        {
            p[0] = fp->jv[ji].q[0] + (ball_p[0] - fp->jv[ji].p[0]);
            p[1] = fp->jv[ji].q[1] + (ball_p[1] - fp->jv[ji].p[1]);
            p[2] = fp->jv[ji].q[2] + (ball_p[2] - fp->jv[ji].p[2]);

            return 1;
        }
    }
    return 0;
}

int sol_swch_test(struct s_file *fp, int flag, short ui)
{
    const float *ball_p = fp->uv[ui].p;
    const float  ball_r = fp->uv[ui].r;
    short xi;
    int f = 1;

    for (xi = 0; xi < fp->xc; xi++)
    {
        struct s_swch *xp = fp->xv + xi;

        if (xp->t0 == 0 || xp->f == xp->f0)
        {
            float r[3];

            r[0] = ball_p[0] - xp->p[0];
            r[1] = ball_p[2] - xp->p[2];
            r[2] = 0;

            if (v_len(r)  < xp->r - ball_r &&
                ball_p[1] > xp->p[1] &&
                ball_p[1] < xp->p[1] + SWCH_HEIGHT / 2)
            {
                if (flag)
                {
                    short pi = xp->pi;
                    short pj = xp->pi;

                    /* Toggle the state, update the path. */

                    xp->f = xp->f ? 0 : 1;

                    do  /* Tortoise and hare cycle traverser. */
                    {
                        fp->pv[pi].f = xp->f;
                        fp->pv[pj].f = xp->f;

                        pi = fp->pv[pi].pi;
                        pj = fp->pv[pj].pi;
                        pj = fp->pv[pj].pi;
                    }
                    while (pi != pj);

                    /* It toggled to non-default state, start the timer. */

                    if (xp->f != xp->f0)
                        xp->t  = xp->t0;
                }
                f = 0;
            }
        }
    }
    return f;
}

/*---------------------------------------------------------------------------*/

int float_put(FILE *fout, float *d)
{
    return (fwrite(d, sizeof (float), 1, fout) == 1);
}

int float_get(FILE *fin, float *d)
{
    return (fread(d, sizeof (float), 1, fin) == 1);
}

int vector_put(FILE *fout, float v[3])
{
    return (fwrite(v, sizeof (float), 3, fout) == 3);
}

int vector_get(FILE *fin, float v[3])
{
    return (fread(v, sizeof (float), 3, fin) == 3);
}

/*---------------------------------------------------------------------------*/

int sol_put(FILE *fout, struct s_file *fp)
{
    return (vector_put(fout, fp->uv[0].p)    &&
            vector_put(fout, fp->uv[0].e[0]) &&
            vector_put(fout, fp->uv[0].e[1]));
}

int sol_get(FILE *fin, struct s_file *fp)
{
    if (vector_get(fin, fp->uv[0].p)    &&
        vector_get(fin, fp->uv[0].e[0]) &&
        vector_get(fin, fp->uv[0].e[1]))
    {
        v_crs(fp->uv[0].e[2], fp->uv[0].e[0], fp->uv[0].e[1]);
        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

