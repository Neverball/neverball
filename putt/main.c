#include <glw.h>
#include <glv.h>
#include <vec.h>
#include <sol.h>

#include <time.h>
#include <sys/time.h>

/*--------------------------------------------------------------------*/

static struct ball drop;
static struct ball ball;
static struct body body;

static GLuint list_ball  = 0;
static GLuint list_sky   = 0;

/*--------------------------------------------------------------------*/

static double init_time;
static double last_time;

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

static void ball_init(void)
{
	ball.p[0] = 0.0;
	ball.p[1] = 0.0;
	ball.p[2] = 0.0;

	ball.v[0] = 0.0;
	ball.v[1] = 0.0;
	ball.v[2] = 0.0;

	ball.w[0] = 0.0;
	ball.w[1] = 0.0;
	ball.w[2] = 0.0;

	ball.r = 1.0;
}

static int ball_drop(int x, int y)
{
	double p[3], e[3][3], s = 2.0;

	glv_get_point(p, e, x, y);

	v_cpy(drop.e[0], e[0]);
	v_cpy(drop.e[1], e[1]);
	v_cpy(drop.e[2], e[2]);

	drop.p[0] = p[0] - e[2][0] * 2.0;
	drop.p[1] = p[1] - e[2][1] * 2.0;
	drop.p[2] = p[2] - e[2][2] * 2.0;

	drop.v[0] = -e[2][0] * s;
	drop.v[1] = -e[2][1] * s;
	drop.v[2] = -e[2][2] * s;

	drop.w[0] = 0.0;
	drop.w[1] = 0.0;
	drop.w[2] = 0.0;

	drop.r =  1.0;

	ball = drop;

	return 1;
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

static void cube(void)
{
	static const double v[8][3] = {
		{ -1.0, -1.0, -1.0 }, { +1.0, -1.0, -1.0 },
		{ -1.0, +1.0, -1.0 }, { +1.0, +1.0, -1.0 },
		{ -1.0, -1.0, +1.0 }, { +1.0, -1.0, +1.0 },
		{ -1.0, +1.0, +1.0 }, { +1.0, +1.0, +1.0 }
	};
	static const int p[6][4] = {
		{ 5, 1, 3, 7 }, { 0, 4, 6, 2 }, { 6, 7, 3, 2 },
		{ 0, 1, 5, 4 }, { 4, 5, 7, 6 }, { 1, 0, 2, 3 }
	};
	static const double n[6][3] = {
		{ +1.0,  0.0,  0.0 }, { -1.0,  0.0,  0.0 },
		{  0.0, +1.0,  0.0 }, {  0.0, -1.0,  0.0 },
		{  0.0,  0.0, +1.0 }, {  0.0,  0.0, -1.0 },
	};

	GLint i, j;

	glBegin(GL_QUADS);
	{
		glColor3d(1.0, 0.5, 0.0);

		for (i = 0; i < 6; i++)
		{
			if (i == 2) continue;

			glNormal3dv(n[i]);

			for (j = 0; j < 4; j++)
				glVertex3dv(v[p[i][j]]);
		}
	}
	glEnd();
}
static int render(void)
{
	float p[4] = { 3.0, 8.0, 3.0, 1.0 };

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

	sol_draw(&body);

	glDepthMask(GL_FALSE);
	{

		glCullFace(GL_FRONT);
		ball_draw();
		glCullFace(GL_BACK);
		ball_draw();
	}
	glDepthMask(GL_TRUE);

	return 1;
}

static int update(void)
{
	struct ball next;

	double time = now();
	double nt;
	double dt = time - last_time;

	while (dt > (nt = sol_ball(&next, &ball, &body)))
	{
		ball = next;
		dt  -= nt;
	}

	sol_step(&ball, &ball, dt);

	ball.v[1] -= 9.8 * dt;

	last_time = time;
	return glv_time_step(time);
}

/*--------------------------------------------------------------------*/

static int on_shape(int w, int h)
{
	glv_set_shape(w, h);
	return 1;
}

static int on_point(int x, int y)
{
	return glv_drag_step(x, y);
}

static int on_click(int x, int y, int b, int d)
{
	if (b == 0 && d != 0)
		return glv_drag_begin(x, y, GLV_NEG_THETA, GLV_NEG_PHI);
	if (b == 0 && d == 0)
		return glv_drag_end(x, y);

	if (b == 1 && d != 0)
		return ball_drop(x, y);

	return 0;
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

	case '1': ball.r = 1.0; break;
	case '2': ball.r = 0.5; break;
	case '3': ball.r = 2.0; break;
	case '4': ball.r = 0.05; break;

	case ' ':
		if (d) ball = drop;
		break;
	}

	if (d)
		return glv_time_begin(now(), v);
	else
		return glv_time_end(now(), v);
}

/*--------------------------------------------------------------------*/

static void init(void)
{
	double o[3] = {  0.0,  4.0,  8.0 };
	double s[3] = {  0.0,  0.0,  0.0 };

	glv_set(o, s, 0.5);
	glv_set_speed(5.0, 180.0, 2.0);

	glEnable(GL_NORMALIZE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);

	list_ball  = make_ball(4);
	list_sky   = make_sky(3, 64.0);

	sol_load(&body);
	ball_init();
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
	case GLW_KEY_D:   return on_key(glw_x(), 1);
	case GLW_KEY_U:   return on_key(glw_x(), 0);
	case GLW_CLOSE:   return -1;
	}

	return 0;
}

int main(void)
{
	int d = 0;

	if (glw_create("Putt", 640, 480, 1))
	{
		init_time = last_time = now();
		init();

		while ((d = loop(d)) >= 0)
			d = render();

		glw_delete();
	}

	return 0;
}

