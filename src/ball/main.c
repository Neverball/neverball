#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <glw.h>
#include <glv.h>
#include <vec.h>
#include <sol.h>

/*--------------------------------------------------------------------*/

static int win_w = 800;
static int win_h = 600;

static double rot_x0 = 0.0, rot_x1 = 0.0;
static double rot_z0 = 0.0, rot_z1 = 0.0;

static int dragging = 0;
static int drag_x0, drag_x1;
static int drag_y0, drag_y1;

static double gravity[3];

static struct sol  S;
static struct ball drop;
static struct ball ball;

static GLuint list_ball  = 0;
static GLuint list_sky   = 0;
static GLuint list_sol  = 0;

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

static void section(int d, const double p0[3], const double c0[4],
                           const double p1[3], const double c1[4],
                           const double p2[3], const double c2[4])
{
	if (d == 0)
	{
		glColor4dv(c0);
		glNormal3dv(p0);
		glVertex3dv(p0);

		glColor4dv(c1);
		glNormal3dv(p1);
		glVertex3dv(p1);

		glColor4dv(c2);
		glNormal3dv(p2);
		glVertex3dv(p2);
	}
	else
	{
		double p01[3], p12[3], p20[3];
		double c01[3], c12[3], c20[3];

		v_mid(p01, p0, p1);
		v_mid(p12, p1, p2);
		v_mid(p20, p2, p0);

		v_nrm(p01, p01);
		v_nrm(p12, p12);
		v_nrm(p20, p20);

		c01[0] = (c0[0] + c1[0]) / 2.0;
		c01[1] = (c0[1] + c1[1]) / 2.0;
		c01[2] = (c0[2] + c1[2]) / 2.0;
		c01[3] = (c0[3] + c1[3]) / 2.0;

		c12[0] = (c1[0] + c2[0]) / 2.0;
		c12[1] = (c1[1] + c2[1]) / 2.0;
		c12[2] = (c1[2] + c2[2]) / 2.0;
		c12[3] = (c1[3] + c2[3]) / 2.0;

		c20[0] = (c2[0] + c0[0]) / 2.0;
		c20[1] = (c2[1] + c0[1]) / 2.0;
		c20[2] = (c2[2] + c0[2]) / 2.0;
		c20[3] = (c2[3] + c0[3]) / 2.0;

		section(d - 1, p0,  c0,  p01, c01, p20, c20);
		section(d - 1, p01, c01, p1,  c1,  p12, c12);
		section(d - 1, p20, c20, p12, c12, p2,  c2);
		section(d - 1, p01, c01, p12, c12, p20, c20);
	}
}

/*--------------------------------------------------------------------*/

static GLuint make_sol(const struct sol *Sp)
{
	GLuint list = glGenLists(1);

	glNewList(list, GL_COMPILE);
	{
		sol_draw(Sp, 0);

		glDepthMask(GL_FALSE);
		{
			sol_draw(Sp, 1);
		}
		glDepthMask(GL_TRUE);
	}
	glEndList();

	return list;
}

static GLuint make_ball(int d)
{
	static const float s[3] = { 1.0f, 1.0f, 1.0f };

	static const double p[6][3] = {
		{ +1.0,  0.0,  0.0 },
		{ -1.0,  0.0,  0.0 },
		{  0.0, +1.0,  0.0 },
		{  0.0, -1.0,  0.0 },
		{  0.0,  0.0, +1.0 },
		{  0.0,  0.0, -1.0 },
	};
	static const double c[2][4] = {
		{ 0.0, 0.0, 0.0, 0.5 },
		{ 1.0, 1.0, 1.0, 0.5 },
	};

	GLuint list = glGenLists(1);

	glNewList(list, GL_COMPILE);
	glPushAttrib(GL_LIGHTING_BIT);
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  s);
		glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 64.0f);

		glBegin(GL_TRIANGLES);
		{
			section(d, p[4], c[0], p[0], c[0], p[2], c[0]);
			section(d, p[0], c[1], p[5], c[1], p[2], c[1]);
			section(d, p[5], c[0], p[1], c[0], p[2], c[0]);
			section(d, p[1], c[1], p[4], c[1], p[2], c[1]);

			section(d, p[0], c[1], p[4], c[1], p[3], c[1]);
			section(d, p[5], c[0], p[0], c[0], p[3], c[0]);
			section(d, p[1], c[1], p[5], c[1], p[3], c[1]);
			section(d, p[4], c[0], p[1], c[0], p[3], c[0]);
		}
		glEnd();
	}
	glPopAttrib();
	glEndList();

	return list;
}

static GLuint make_sky(int d, double k)
{
	static const double p[6][3] = {
		{ +1.0,  0.0,  0.0 },
		{ -1.0,  0.0,  0.0 },
		{  0.0, +1.0,  0.0 },
		{  0.0, -1.0,  0.0 },
		{  0.0,  0.0, +1.0 },
		{  0.0,  0.0, -1.0 },
	};
	static const double c[3][4] = {
		{ 0.0, 0.5, 1.0, 1.0 },
		{ 0.0, 0.3, 0.6, 1.0 },
		{ 0.0, 0.0, 0.2, 1.0 },
	};

	GLuint list = glGenLists(1);

	glNewList(list, GL_COMPILE);
	glPushAttrib(GL_ENABLE_BIT);
	glPushMatrix();
	{
		glDisable(GL_LIGHTING);

		glScaled(k, k, k);

		glBegin(GL_TRIANGLES);
		{
			section(d, p[4], c[1], p[0], c[1], p[2], c[0]);
			section(d, p[0], c[1], p[5], c[1], p[2], c[0]);
			section(d, p[5], c[1], p[1], c[1], p[2], c[0]);
			section(d, p[1], c[1], p[4], c[1], p[2], c[0]);

			section(d, p[0], c[1], p[4], c[1], p[3], c[2]);
			section(d, p[5], c[1], p[0], c[1], p[3], c[2]);
			section(d, p[1], c[1], p[5], c[1], p[3], c[2]);
			section(d, p[4], c[1], p[1], c[1], p[3], c[2]);
		}
		glEnd();
	}
	glPopMatrix();
	glPopAttrib();
	glEndList();

	return list;
}

/*--------------------------------------------------------------------*/

static void ball_init(double r)
{
	double e[3][3] = {
		{ 1.0, 0.0, 0.0 },
		{ 0.0, 1.0, 0.0 },
		{ 0.0, 0.0, 1.0 },
	};

	ball.r = r;

	v_cpy(ball.e[0], e[0]);
	v_cpy(ball.e[1], e[1]);
	v_cpy(ball.e[2], e[2]);

	ball.p[0] =  0.0;
	ball.p[1] =  ball.r + 1.0;
	ball.p[2] =  0.0;

	ball.v[0] = 0.0;
	ball.v[1] = 0.0;
	ball.v[2] = 0.0;

	ball.w[0] = 0.0;
	ball.w[1] = 0.0;
	ball.w[2] = 0.0;

	drop = ball;
}

static void ball_draw(void)
{
	glPushMatrix();
	{
		double M[16];

		m_basis(M, ball.e[0], ball.e[1], ball.e[2]);

		glTranslated(ball.p[0], ball.p[1], ball.p[2]);
		glMultMatrixd(M);
		glScaled(ball.r, ball.r, ball.r);
		glCallList(list_ball);
	}
	glPopMatrix();
}

/*--------------------------------------------------------------------*/

static int render(void)
{
	float  p[4] = {  8.0,  32.0, 8.0, 1.0 };
	double o[3] = {  0.0, 0.0, 0.0 };
	double s[3] = {  5.0, -30.0, 0.0 };

	glv_set(o, s, 0.5);


	glClear(GL_DEPTH_BUFFER_BIT);

	glv_apply_persp();
	glv_apply_model();

	glLightfv(GL_LIGHT0, GL_POSITION, p);

	glPushMatrix();
	{
		glv_apply_backdrop();
		glScaled(-1.0, -1.0, -1.0);
		glCallList(list_sky);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glRotated(rot_z1, 0.0, 0.0, 1.0);
		glRotated(rot_x1, 1.0, 0.0, 0.0);

		glTranslated(-ball.p[0], -ball.p[1], -ball.p[2]);
		glPushAttrib(GL_LIGHTING_BIT);
		{
			glCallList(list_sol);
		}
		glPopAttrib();

		glDepthMask(GL_FALSE);
		glPushAttrib(GL_ENABLE_BIT);
		{
			glEnable(GL_COLOR_MATERIAL);

			glCullFace(GL_FRONT);
			ball_draw();
			glCullFace(GL_BACK);
			ball_draw();
		}
		glPopAttrib();
		glDepthMask(GL_TRUE);
	}
	glPopMatrix();

	return 1;
}

static int update(void)
{
	struct ball next;

	double time = now(), nt, dt = time - last_time;

	while (dt >= (nt = sol_ball(&next, &ball, &S, NULL)))
	{
		ball = next;
		dt -= nt;
	}

	next = ball;
	sol_step(&ball, &next, dt);

/*	v_mad(ball.v, ball.v, gravity, dt);*/

	last_time = time;
	return glv_time_step(time);
}

/*--------------------------------------------------------------------*/

static void do_drag(int x, int y)
{
	if (dragging)
	{
/*		double h[3], g[3] = { 0.0, -9.8, 0.0 };*/
		double h[3], g[3] = { 0.0, -20.0, 0.0 };
		double A[16], B[16];

		double x[3] = { 1.0, 0.0, 0.0 };
/*		double y[3] = { 0.0, 1.0, 0.0 }; */
		double z[3] = { 0.0, 0.0, 1.0 };

		int dx = drag_x0 - drag_x1;
		int dy = drag_y1 - drag_y0;

		rot_z1 = rot_z0 + 60.0 * dx / win_w;
		rot_x1 = rot_x0 + 60.0 * dy / win_h;

		m_rot(A, x, -V_RAD(rot_x1));
		m_rot(B, z, -V_RAD(rot_z1));

		m_vxfm(h, A, g);
		m_vxfm(gravity, B, h);
	}
}

/*--------------------------------------------------------------------*/

static int on_shape(int w, int h)
{
	win_w = w;
	win_h = h;

	glv_set_shape(w, h);
	return 1;
}

static int on_point(int x, int y)
{
	drag_x1 = x;
	drag_y1 = y;

	do_drag(x, y);

	return 1;
}

static int on_click(int x, int y, int b, int d)
{
	if (b == 0)
	{
		dragging = d;

		drag_x0  = x;
		drag_y0  = y;

		rot_x0 = rot_x1;
		rot_z0 = rot_z1;
	}

	return 1;
}

static int on_key(int k, int d)
{
	if (k == 27 && d == 0) exit(0);

	return 0;
}

/*--------------------------------------------------------------------*/

static void init(const struct sol *Sp, double r)
{
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);

	list_ball = make_ball(4);
	list_sky  = make_sky(3, 64.0);
	list_sol  = make_sol(Sp);

	ball_init(r);

	gravity[0] =  0.0;
	gravity[1] = -9.8;
	gravity[2] =  0.0;
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
	case GLW_KEY_D:   return on_key(glw_x(), 1);
	case GLW_KEY_U:   return on_key(glw_x(), 0);
	case GLW_CLOSE:   return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	FILE *fp;
	int d = 0;

	if (argc > 1 && (fp = fopen(argv[1], "r")))
	{
		sol_load(&S, fp);
		fclose(fp);

		if (glw_create(argv[1], 640, 480, 1))
		{
			init_time = draw_time = last_time = now();
			init(&S, 0.5);

			while ((d = loop(d)) >= 0)
				d = render();

			glw_delete();
		}
	}

	return 0;
}

