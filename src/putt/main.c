#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <glw.h>
#include <glv.h>
#include <vec.h>
#include <sol.h>

#include "ball.h"

/*--------------------------------------------------------------------*/

static int dragging = 0;
static double drag[2][3];

static struct s_file f;

GLuint list_ball = 0;
GLuint list_sky  = 0;

/*--------------------------------------------------------------------*/

static double init_time;
static double last_time;
static double draw_time;

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

static int render(void)
{
	float p[4] = { 8.0, 32.0, 8.0, 1.0 };

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glv_apply_persp();
	glv_apply_model();

	glLightfv(GL_LIGHT0, GL_POSITION, p);

	glPushMatrix();
	{
		glScaled(64.0, 64.0, 64.0);
		glCallList(list_sky);
	}
	glPopMatrix();

	sol_render(&f);

	glDepthMask(GL_FALSE);
	{
		glCullFace(GL_FRONT);
		ball_draw(&f);
		glCullFace(GL_BACK);
		ball_draw(&f);
	}
	glDepthMask(GL_TRUE);

	if (dragging)
	{
		glPushAttrib(GL_ENABLE_BIT);
		{
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_LIGHTING);

			glLineWidth(4.0);
			glBegin(GL_LINES);
			{
				glColor3d(1.0, 1.0, 0.0);
				glVertex3dv(drag[1]);
				glColor3d(1.0, 0.5, 0.0);
				glVertex3dv(drag[0]);
			}
			glEnd();
		}
		glPopAttrib();
	}


	return 1;
}

static int update(void)
{
	double g[3] = { +0.0, -9.8, +0.0 };
	double time = now();

	sol_update(&f, time - last_time, g);

	last_time = time;

	return glv_time_step(time);
}

/*--------------------------------------------------------------------*/

static void do_drag(int x, int y, int d)
{
	double e[3][3];
	double p[3];
	double r[3];

	if (d == 1) dragging = 1;
	if (d == 0) dragging = 0;

	glv_get_point(p, e, x, y);

	if (d == 1)
		v_cpy(drag[0], f.uv[0].p);
	else
		v_mad(drag[1], p, e[2], (f.uv[0].p[1] - p[1]) / e[2][1]);

	if (d == 0)
	{
		v_sub(r, drag[0], drag[1]);
		v_mad(f.uv[0].v, f.uv[0].v, r, 8.0);
	}
}

/*--------------------------------------------------------------------*/

static int on_shape(int w, int h)
{
	glv_set_shape(w, h);
	return 1;
}

static int on_point(int x, int y)
{
	do_drag(x, y, -1);
	return glv_drag_step(x, y);
}

static int on_click(int x, int y, int b, int d)
{
	if (b == 1 && d != 0)
		return glv_drag_begin(x, y, GLV_NEG_THETA, GLV_NEG_PHI);
	if (b == 1 && d == 0)
		return glv_drag_end(x, y);

	if (b == 0)
		do_drag(x, y, d);

	return 0;
}

static int on_wheel(int d)
{
	double o[3], s[3], z;
	double p[3], e[3][3];

	z = glv_get(o, s);

	glv_get_space(p, e);

	v_mad(o, o, e[2], d ? -0.5 : +0.5);

	glv_set(o, s, z);

	return 1;
}

static int on_key(int k, int d)
{
	int v = GLV_NONE;

	switch (k)
	{
	case 'a': v = GLV_LEFT;  break;
	case 'u': v = GLV_RIGHT; break;
	case ',': v = GLV_UP;    break;
	case 'o': v = GLV_DOWN;  break;
	case '.': v = GLV_FORE;  break;
	case 'e': v = GLV_BACK;  break;

	/*
	case '1': ball_init(0.05); break;
	case '2': ball_init(0.10); break;
	case '3': ball_init(0.25); break;
	case '4': ball_init(0.50); break;
	case '5': ball_init(1.00); break;
	case '6': ball_init(2.00); break;
	*/

	case '\t':
		if (d)
		{
			f.uv[0].v[0] = 0.0;
			f.uv[0].v[1] = 0.0;
			f.uv[0].v[2] = 0.0;
		}
		break;
	case ' ':
		if (d)
		{
			f.uv[0].p[0] = 2.0;
			f.uv[0].p[1] = 2.0;
			f.uv[0].p[2] = -0.5;
			f.uv[0].v[0] = 0.0;
			f.uv[0].v[1] = 0.0;
			f.uv[0].v[2] = 0.0;
		}
		break;

	case 27: exit(0);
	}

	if (d)
		return glv_time_begin(now(), v);
	else
		return glv_time_end(now(), v);
}

/*--------------------------------------------------------------------*/

static void init(void)
{
	double o[3] = {  0.0,   2.0,  2.0 };
	double s[3] = {  0.0, -30.0,  0.0 };

	glv_set(o, s, 0.05);
	glv_set_speed(2.0, 180.0, 2.0);
	glv_set_range(0.1, 100.0, 0.0);

	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);

	list_ball = make_ball(4);
	list_sky  = make_sky(3);
}

static int loop(int d)
{
	switch (glw_update(d, 0))
	{
	case 0:           return update();
	case GLW_RENDER:  return +1;
	case GLW_RESIZE:  return on_shape(glw_x(), glw_y());
	case GLW_MOTION:  return on_point(glw_x(), glw_y());
	case GLW_L_BTN_D: return on_click(glw_x(), glw_y(), 0, 1);
	case GLW_L_BTN_U: return on_click(glw_x(), glw_y(), 0, 0);
	case GLW_R_BTN_D: return on_click(glw_x(), glw_y(), 1, 1);
	case GLW_R_BTN_U: return on_click(glw_x(), glw_y(), 1, 0);
	case GLW_WHEEL_D: return on_wheel(1);
	case GLW_WHEEL_U: return on_wheel(0);
	case GLW_KEY_D:   return on_key(glw_x(), 1);
	case GLW_KEY_U:   return on_key(glw_x(), 0);
	case GLW_CLOSE:   return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	FILE *fin;
	int d = 0;

	if (argc > 1 && (fin = fopen(argv[1], "r")))
	{
		sol_load(&f, fin);
		fclose(fin);

		if (glw_create(argv[1], 640, 480, 1))
		{
			init_time = draw_time = last_time = now();
			init();

			while ((d = loop(d)) >= 0)
				d = render();

			glw_delete();
		}
	}

	return 0;
}

