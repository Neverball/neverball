#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>

#include <sol.h>
#include <vec.h>
#include <aio.h>

#include "main.h"
#include "ball.h"

#define TINY  0.0000000005

/*---------------------------------------------------------------------------*/

short *bump_buf = NULL;
int    bump_len =    0;
short *roll_buf = NULL;
int    roll_len =    0;
int    roll_aio =    0;

static GLuint list_ball = 0;

static struct s_file file;

static double game_rx = 0.0;
static double game_rz = 0.0;

static double view_y = 2.0;
static double view_z = 4.0;
static double view_p[3];
static double view_e[3][3];

/*---------------------------------------------------------------------------*/

static void render(const struct s_file *fp,
                   double p[3],
                   double e[3][3], double rx, double rz)
{
    const float light_p[4]  = { 8.0, 32.0, 8.0, 1.0 };
    const struct s_ball *up = fp->uv;
    
    GLint vp[4];

    glMatrixMode(GL_PROJECTION);
    {
        glGetIntegerv(GL_VIEWPORT, vp);
        glLoadIdentity();
        gluPerspective(30.0, (GLdouble) vp[2] / (GLdouble) vp[3], 0.1, 100.0);
    }
    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();
    {
        gluLookAt(p[0], p[1], p[2], up->p[0], up->p[1], up->p[2], 0, 1, 0);

        glLightfv(GL_LIGHT0, GL_POSITION, light_p);

        /* Rotate the world about the position of the ball */

        glTranslated(+up->p[0], +up->p[1], +up->p[2]);
        glRotated(-rz, e[2][0], e[2][1], e[2][2]);
        glRotated(-rx, e[0][0], e[0][1], e[0][2]);
        glTranslated(-up->p[0], -up->p[1], -up->p[2]);

        /* Render the world */

        glPushAttrib(GL_ENABLE_BIT);
        glPushAttrib(GL_LIGHTING_BIT);
        {
            glEnable(GL_TEXTURE_2D);
            sol_render(fp);
        }
        glPopAttrib();
        glPopAttrib();

        /* Render the ball. */

        ball_draw(list_ball, up->r, up->p, up->e);
    }
    glPopMatrix();
}

static void update(struct s_file *fp,
                   double p[3],
                   double e[3][3], double rx, double rz, double dt)
{
    double X[16], Z[16], M[16];
    double x[3], y[3] = { 0.0, 1.0, 0.0 }, z[3];
    double a[3], g[3] = { 0.0, -9.8, 0.0 }, h[3];
    double bump, roll;
    double ay, az;

    /* Compute the gravity vector from the given world rotations. */

    v_sub(z, p, fp->uv->p);
    v_crs(x, y, z);
    v_crs(z, x, y);
    v_nrm(x, x);
    v_nrm(z, z);

    m_rot (Z, z, V_RAD(rz));
    m_rot (X, x, V_RAD(rx));
    m_mult(M, Z, X);
    m_vxfm(h, M, g);

    /* Run the sim. */

    sol_update(fp, dt, h, &bump, &roll);

    /* Create the sound of the ball. */

    if (bump > 0.25)
        aio_play(bump_buf, bump_len, MIN(bump / 2.0, 1.0));

    aio_freq(roll_aio, roll);
    aio_ampl(roll_aio, MIN(roll / 50.0, 0.3));

    /* Orthonormalize the basis of the view of the ball in its new position. */

    v_sub(e[2], view_p, fp->uv->p);
    v_crs(e[0], e[1], e[2]);
    v_crs(e[2], e[0], e[1]);
    v_nrm(e[0], e[0]);
    v_nrm(e[2], e[2]);

    v_sub(a, view_p, fp->uv->p);
    ay = v_dot(e[1], a);
    az = v_dot(e[2], a);

    /* Compute the new view position. */

    p[0] = p[1] = p[2] = 0.0;

    v_mad(p, fp->uv->p, e[1], ay < view_y ? ay : view_y);
    v_mad(p,         p, e[2], az < view_z ? az : view_z);
}

/*---------------------------------------------------------------------------*/

void game_init(void)
{
    glClearColor(0.0, 0.0, 0.2, 1.0);

    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);

    list_ball = ball_init(4);

    view_p[0] =    0.0;
    view_p[1] = view_y;
    view_p[2] = view_z;

    view_e[0][0] = 1.0;
    view_e[0][1] = 0.0;
    view_e[0][2] = 0.0;
    view_e[1][0] = 0.0;
    view_e[1][1] = 1.0;
    view_e[1][2] = 0.0;
    view_e[2][0] = 0.0;
    view_e[2][1] = 0.0;
    view_e[2][2] = 1.0;

    bump_buf = aio_load("data/sw/bump.sw", &bump_len);
    roll_buf = aio_load("data/sw/roll.sw", &roll_len);

    roll_aio = aio_loop(roll_buf, roll_len, 0.25);

    sol_load(&file, "data/sol/move.sol");
}

void game_step(double dt)
{
    update(&file, view_p, view_e, game_rx, game_rz, dt);
}

/*---------------------------------------------------------------------------*/

void game_paint(void)
{
    render(&file, view_p, view_e, game_rx, game_rz);
}

int game_point(int x, int y)
{
    double bound = 20.0;

    game_rx -= 40.0 * y / 500;
    game_rz += 40.0 * x / 500;

    if (game_rx > +bound) game_rx = +bound;
    if (game_rx < -bound) game_rx = -bound;
    if (game_rz > +bound) game_rz = +bound;
    if (game_rz < -bound) game_rz = -bound;

    return 1;
}

int game_click(int d)
{
    return 0;
}

int game_keybd(int c)
{
    if (c == 27)
        return -1;
    else
        return 0;
}
