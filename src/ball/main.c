#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <glw.h>
#include <vec.h>
#include <sol.h>

#include <GL/glu.h>

#include "ball.h"

/*--------------------------------------------------------------------*/

static int win_w = 640;
static int win_h = 480;

static double grav[3];
static double view[3];
static double fov = 45.0;

static double dy = 2.0;
static double dz = 4.0;

static double vx[3] = { 1.0, 0.0, 0.0 };
static double vy[3] = { 0.0, 1.0, 0.0 };
static double vz[3] = { 0.0, 0.0, 1.0 };

static double kx = 0.0;
static double ky = 0.0;

static double rx = 0.0;
static double rz = 0.0;

static struct s_file  f;
static struct s_ball *up;

static GLuint list_ball  = 0;

/*--------------------------------------------------------------------*/

static double last_time;

static double now(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (double) tv.tv_sec + (double) tv.tv_usec / 1000000.0;
}

/*--------------------------------------------------------------------*/

static void ball_draw(const struct s_file *fp)
{
	const struct s_ball *up = fp->uv;

	glPushMatrix();
	{
		double M[16];

		m_basis(M, up->e[0], up->e[1], up->e[2]);

		glTranslated(up->p[0], up->p[1], up->p[2]);
		glMultMatrixd(M);
		glScaled(up->r, up->r, up->r);
		glCallList(list_ball);
	}
	glPopMatrix();
}

/*--------------------------------------------------------------------*/

static float light_p[4] = { 8.0, 32.0, 8.0, 1.0 };

static int render(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLightfv(GL_LIGHT0, GL_POSITION, light_p);

	glPushMatrix();
	{
		gluLookAt(view[0],  view[1],  view[2],
		          up->p[0], up->p[1], up->p[2],
                          0.0,      1.0,      0.0);

		glTranslated(+up->p[0], +up->p[1], +up->p[2]);

		glRotated(-rz, vz[0], vz[1], vz[2]);
		glRotated(-rx, vx[0], vx[1], vx[2]);

		glTranslated(-up->p[0], -up->p[1], -up->p[2]);

		glPushAttrib(GL_ENABLE_BIT);
		glPushAttrib(GL_LIGHTING_BIT);
		{
			glEnable(GL_TEXTURE_2D);
			sol_render(&f);
		}
		glPopAttrib();
		glPopAttrib();

		glDepthMask(GL_FALSE);
		glPushAttrib(GL_ENABLE_BIT);
		{
			glEnable(GL_COLOR_MATERIAL);

			glCullFace(GL_FRONT);
			ball_draw(&f);
			glCullFace(GL_BACK);
			ball_draw(&f);
		}
		glPopAttrib();
		glDepthMask(GL_TRUE);
	}
	glPopMatrix();

	return 1;
}

static int update(void)
{
	double time = now();

	double X[16];
	double Z[16];
	double M[16];
	double x[3];
	double y[3] = { 0.0, 1.0, 0.0 };
	double z[3];
	double g[3] = { 0.0, -9.8, 0.0 };

	double a[3];
	double ay;
	double az;

	rx = ky * 30;
	rz = kx * 30;

	if (rx < -20.0) rx = -20.0;
	if (rx > +20.0) rx = +20.0;
	if (rz < -20.0) rz = -20.0;
	if (rz > +20.0) rz = +20.0;

	v_sub(z, view, up->p);
	v_crs(x, y, z);
	v_crs(z, x, y);
	v_nrm(x, x);
	v_nrm(z, z);

	m_rot(Z, z, V_RAD(rz));
	m_rot(X, x, V_RAD(rx));
	m_mult(M, Z, X);

	m_vxfm(grav, M, g);

	sol_update(&f, time - last_time, grav);

	v_sub(vz, view, up->p);
	v_crs(vx, vy, vz);
	v_crs(vz, vx, vy);
	v_nrm(vx, vx);
	v_nrm(vz, vz);

	v_sub(a, view, up->p);
	ay = v_dot(vy, a);
	az = v_dot(vz, a);

	view[0] = 0.0;
	view[1] = 0.0;
	view[2] = 0.0;

	v_mad(view, up->p, vy, ay < dy ? ay : dy);
	v_mad(view, view,  vz, az < dz ? az : dz);

	last_time = time;
	return 1;
}

/*--------------------------------------------------------------------*/

static int on_shape(int w, int h)
{
	GLdouble a = (GLdouble) w / (GLdouble) h;

	win_w = w;
	win_h = h;

	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	{
		glLoadIdentity();
		gluPerspective(fov, a, 1.0, 1000.0);
	}
	glMatrixMode(GL_MODELVIEW);

	return 1;
}

static int on_point(int x, int y)
{
	kx = +(2.0 * x / win_w - 1.0);
	ky = -(2.0 * y / win_h - 1.0);

	return 1;
}

static int on_click(int x, int y, int b, int d)
{
	return 1;
}

static int on_key(int k, int d)
{
	if (k == 27 && d == 0) exit(0);

	if (k == 32 && d == 0)
	{
		f.uv[0].p[0] = 0;
		f.uv[0].p[1] = f.uv[0].r + 0.001;
		f.uv[0].p[2] = 0;
		f.uv[0].v[0] = 0;
		f.uv[0].v[1] = 0;
		f.uv[0].v[2] = 0;
		f.uv[0].w[0] = 0;
		f.uv[0].w[1] = 0;
		f.uv[0].w[2] = 0;
	}

	return 0;
}

/*--------------------------------------------------------------------*/

static void init(void)
{
	glClearColor(0.0, 0.0, 0.2, 1.0);

	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);

	list_ball = make_ball(4);

	grav[0] =  0.0;
	grav[1] = -9.8;
	grav[2] =  0.0;

	view[0] = 0.0;
	view[1] =  dy;
	view[2] =  dz;
}

static int loop(int s)
{
	switch (glw_update(s, 0))
	{
	case GLW_RENDER:  return 0;
	case GLW_RESIZE:  return on_shape(glw_x(), glw_y());
	case GLW_MOTION:  return on_point(glw_x(), glw_y());
	case GLW_L_BTN_D: return on_click(glw_x(), glw_y(), 0, 1);
	case GLW_L_BTN_U: return on_click(glw_x(), glw_y(), 0, 0);
	case GLW_KEY_D:   return on_key(glw_x(), 1);
	case GLW_KEY_U:   return on_key(glw_x(), 0);
	case GLW_CLOSE:   return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int d = 0;
	int s = 0;

	if (argc > 1)
	{
		if (glw_create(argv[1], win_w, win_h, 1))
		{
			sol_load(&f, argv[1]);

			up = f.uv;

			last_time = now();
			init();

			while (d >= 0)
			{
				/* FIXME: buh? */

				while ((d = loop(s)) > 0)
					s = 0;

				    update();
				s = render();
			}

			glw_delete();
		}
	}

	return 0;
}

