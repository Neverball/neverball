#include <GL/gl.h>
#include <math.h>

#include "glv.h"

/*--------------------------------------------------------------------*/

#define XOR(a, b) (((a) || (b)) && !((a) && (b)))

#define RIGHT_P(v)	(v & GLV_RIGHT)
#define LEFT_P(v)	(v & GLV_LEFT)
#define UP_P(v)		(v & GLV_UP)
#define DOWN_P(v)	(v & GLV_DOWN)
#define BACK_P(v)	(v & GLV_BACK)
#define FORE_P(v)	(v & GLV_FORE)
#define POS_RHO_P(v)	(v & GLV_POS_RHO)
#define NEG_RHO_P(v)	(v & GLV_NEG_RHO)
#define POS_PHI_P(v)	(v & GLV_POS_PHI)
#define NEG_PHI_P(v)	(v & GLV_NEG_PHI)
#define POS_THETA_P(v)	(v & GLV_POS_THETA)
#define NEG_THETA_P(v)	(v & GLV_NEG_THETA)
#define ZOOM_IN_P(v)	(v & GLV_ZOOM_IN)
#define ZOOM_OUT_P(v)	(v & GLV_ZOOM_OUT)

#define X_P(v)		XOR(RIGHT_P(v),     LEFT_P(v))
#define Y_P(v)		XOR(UP_P(v),        DOWN_P(v))
#define Z_P(v)		XOR(BACK_P(v),	    FORE_P(v))
#define RHO_P(v)	XOR(POS_RHO_P(v),   NEG_RHO_P(v))
#define PHI_P(v)	XOR(POS_PHI_P(v),   NEG_PHI_P(v))
#define THETA_P(v)	XOR(POS_THETA_P(v), NEG_THETA_P(v))
#define ZOOM_P(v)	XOR(ZOOM_IN_P(v),   ZOOM_OUT_P(v))

#define TRANSLATE_P(v)	(X_P(v)   || Y_P(v)   || Z_P(v))
#define ROTATE_P(v)	(RHO_P(v) || PHI_P(v) || THETA_P(v))

/*====================================================================*/

static double o[3] = { 0.0, 0.0, 0.0 };	/* Origin                     */
static double s[3] = { 0.0, 0.0, 0.0 };	/* Orientation                */

static double n  =   1.0;		/* Near plane distance        */
static double f  = 512.0;		/* Far plane distance         */
static double e  =   0.125;		/* Interocular distance       */
static double z  =   0.5;		/* Zoom coefficient           */

static int    w  = 512;			/* Viewport width             */
static int    h  = 512;			/* Viewport height            */

static double ts =  10.0;		/* Translation speed          */
static double rs = 180.0;		/* Rotate speed	              */
static double zs =   2.0;		/* Zoom speed                 */

/*--------------------------------------------------------------------*/

static void origin(int var, double d, double o[3])
{
	const double sp = sin(s[1] * 3.14159265358979323846 / 180.0);
	const double cp = cos(s[1] * 3.14159265358979323846 / 180.0);
	const double st = sin(s[2] * 3.14159265358979323846 / 180.0);
	const double ct = cos(s[2] * 3.14159265358979323846 / 180.0);

	double v[3], k;

	v[0] = (RIGHT_P(var) ? 1.0 : 0.0) - (LEFT_P(var) ? 1.0 : 0.0);
	v[1] = (UP_P(var)    ? 1.0 : 0.0) - (DOWN_P(var) ? 1.0 : 0.0);
	v[2] = (BACK_P(var)  ? 1.0 : 0.0) - (FORE_P(var) ? 1.0 : 0.0);

	k = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	v[0] *= d * ts / k;
	v[1] *= d * ts / k;
	v[2] *= d * ts / k;

	o[0] = o[0] + ct * v[0] + sp * st * v[1] + cp * st * v[2];
	o[1] = o[1]             + cp      * v[1] - sp      * v[2];
	o[2] = o[2] - st * v[0] + sp * ct * v[1] + cp * ct * v[2];
}

static void sphere(int var, double d, double s[3])
{
	s[0] += (POS_RHO_P(var))   ? d * ts : 0.0f;
	s[0] -= (NEG_RHO_P(var))   ? d * ts : 0.0f;
	s[1] += (POS_PHI_P(var))   ? d * rs : 0.0f;
	s[1] -= (NEG_PHI_P(var))   ? d * rs : 0.0f;
	s[2] += (POS_THETA_P(var)) ? d * rs : 0.0f;
	s[2] -= (NEG_THETA_P(var)) ? d * rs : 0.0f;

	if (s[0] < 0.0f) s[0] = 0.0f;

	if (s[1] <  -90.0f) s[1] =  -90.0f;
	if (s[1] >  +90.0f) s[1] =  +90.0f;

	if (s[2] < -180.0f) s[2] += 360.0f;
	if (s[2] > +180.0f) s[2] -= 360.0f;
}

static void zoom(int var, double d, double *z)
{
	if (ZOOM_IN_P(var))  *z /= 1.0f + (d * zs);
	if (ZOOM_OUT_P(var)) *z *= 1.0f + (d * zs);

	if (*z < 0.00001f) *z = 0.00001f;
}

/*====================================================================*/

void glv_apply_ortho(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, (GLdouble) w, 0, (GLdouble) h, n, f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void glv_apply_persp(void)
{
	GLdouble a = (GLdouble) w / (GLdouble) h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-a * z, +a * z, -z, +z, n, f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void glv_apply_model(void)
{
	glTranslated(0.0, 0.0, -s[0]);

	glRotated(-s[1], 1.0, 0.0, 0.0);
	glRotated(-s[2], 0.0, 1.0, 0.0);

	glTranslated(-o[0], -o[1], -o[2]);
}

/*--------------------------------------------------------------------*/

void glv_apply_left(void)
{
	double r = asin(e / n) * 180.0 / 3.14159265358979323846;

	glDrawBuffer(GL_BACK_LEFT);

	glTranslated(0.0, 0.0, -n);
	glRotated(+r, 0.0, 1.0, 0.0);
	glTranslated(0.0, 0.0, +n);
}

void glv_apply_right(void)
{
	double r = asin(e / n) * 180.0 / 3.14159265358979323846;

	glDrawBuffer(GL_BACK_RIGHT);
	
	glTranslatef(0.0, 0.0, -n);
	glRotatef(-r, 0.0, 1.0, 0.0);
	glTranslatef(0.0, 0.0, +n);
}

/*--------------------------------------------------------------------*/

void glv_apply_billboard(void)
{
	glRotated(+s[2], 0.0, 1.0, 0.0);
	glRotated(+s[1], 1.0, 0.0, 0.0);
}

void glv_apply_backdrop(void)
{
	glTranslated(o[0], o[1], o[2]);
	glTranslated(0.0,  0.0,  s[0]);
}

/*--------------------------------------------------------------------*/

static int drag_x = GLV_NONE;
static int drag_y = GLV_NONE;

static int drag_x0;
static int drag_y0;

static double o0[3];
static double s0[3];
static double z0;

int glv_drag_begin(int x, int y, int x_var, int y_var)
{
	drag_x0 = x;
	drag_y0 = y;

	if (TRANSLATE_P(x_var) || TRANSLATE_P(y_var))
	{
		o0[0] = o[0];
		o0[1] = o[1];
		o0[2] = o[2];
	}
	if (ROTATE_P(x_var) || ROTATE_P(y_var))
	{
		s0[0] = s[0];
		s0[1] = s[1];
		s0[2] = s[2];
	}
	if (ZOOM_P(x_var) || ZOOM_P(y_var))
	{
		z0 = z;
	}

	drag_x = x_var;
	drag_y = y_var;

	return GL_FALSE;
}

int glv_drag_step(int x, int y)
{
	int b = 0;

	if (TRANSLATE_P(drag_x) || TRANSLATE_P(drag_y))
	{
		o[0] = o0[0];
		o[1] = o0[1];
		o[2] = o0[2];

		origin(drag_x, (double) (x - drag_x0) / h, o);
		origin(drag_y, (double) (y - drag_y0) / h, o);

		b = 1;
	}
	if (ROTATE_P(drag_x) || ROTATE_P(drag_y))
	{
		s[0] = s0[0];
		s[1] = s0[1];
		s[2] = s0[2];

		sphere(drag_x, (double) (x - drag_x0) / h, s);
		sphere(drag_y, (double) (y - drag_y0) / h, s);

		b = 1;
	}
	if (ZOOM_P(drag_x) || ZOOM_P(drag_y))
	{
		z = z0;

		zoom(drag_x, (double) (x - drag_x0) / h, &z);
		zoom(drag_y, (double) (y - drag_y0) / h, &z);

		b = 1;
	}

	return b;
}

int glv_drag_end(int x, int y)
{
	int b = glv_drag_step(x, y);

	drag_x = GLV_NONE;
	drag_y = GLV_NONE;

	return b;
}

/*--------------------------------------------------------------------*/

static int time_var = GLV_NONE;

int glv_time_begin(double t, int var)
{
	time_var |= var;

	return glv_time_step(t);
}

int glv_time_step(double t1)
{
	static double t0 = 0.0;
	int b = 0;
				
	if (TRANSLATE_P(time_var))
	{
		origin(time_var, (t1 - t0), o);
		b = 1;
	}
	if (ROTATE_P(time_var))
	{
		sphere(time_var, (t1 - t0), s);
		b = 1;
	}
	if (ZOOM_P(time_var))
	{
		zoom(time_var, (t1 - t0), &z);
		b = 1;
	}

	t0 = t1;

	return b;
}

int glv_time_end(double t, int var)
{
	int b = glv_time_step(t);

	time_var &= ~var;

	return b;
}

/*--------------------------------------------------------------------*/

void glv_set_shape(int width, int height)
{
	w = width;
	h = height;

	glViewport(0, 0, w, h);
}

void glv_set_range(double near_dist, double far_dist, double eye_dist)
{
	n = near_dist;
	f = far_dist;
	e = eye_dist;
}

void glv_set_speed(double translate, double rotate, double zoom)
{
	ts = translate;
	rs = rotate;
	zs = zoom;
}

/*--------------------------------------------------------------------*/

void glv_set(double origin[3], double sphere[3], double zoom)
{
	o[0] = origin[0]; o[1] = origin[1]; o[2] = origin[2];
	s[0] = sphere[0]; s[1] = sphere[1]; s[2] = sphere[2];

	z = zoom;
}

double glv_get(double origin[3], double sphere[3])
{
	origin[0] = o[0]; origin[1] = o[1]; origin[2] = o[2];
	sphere[0] = s[0]; sphere[1] = s[1]; sphere[2] = s[2];

	return z;
}

/*====================================================================*/

static void cross(double u[3], const double v[3], const double w[3])
{
	u[0] = v[1] * w[2] - v[2] * w[1];
	u[1] = v[2] * w[0] - v[0] * w[2];
	u[2] = v[0] * w[1] - v[1] * w[0];
}

static void norm(double v[3])
{
	double d = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	v[0] /= d;
	v[1] /= d;
	v[2] /= d;
}

/*--------------------------------------------------------------------*/

void glv_get_point(double p[3], double e[3][3], int x, int y)
{
	double v[3], u[3], q[3], f[3][3];

	v[0] = -2.0f * z * (w / 2 - x) / h;
	v[1] = +2.0f * z * (h / 2 - y) / h;
	v[2] = -n;

	glv_get_space(q, f);

	u[0] = f[0][0] * v[0] + f[1][0] * v[1] + f[2][0] * v[2];
	u[1] = f[0][1] * v[0] + f[1][1] * v[1] + f[2][1] * v[2];
	u[2] = f[0][2] * v[0] + f[1][2] * v[1] + f[2][2] * v[2];

	p[0] = q[0] + u[0];
	p[1] = q[1] + u[1];
	p[2] = q[2] + u[2];

	e[2][0] = -u[0];
	e[2][1] = -u[1];
	e[2][2] = -u[2];

	norm(e[2]);
	cross(e[0], f[1], e[2]);
	norm(e[0]);
	cross(e[1], e[2], e[0]);
	norm(e[1]);
}

void glv_get_space(double p[3], double e[3][3])
{
	const double sp = sin(s[1] * 3.14159265358979323846 / 180.0);
	const double cp = cos(s[1] * 3.14159265358979323846 / 180.0);
	const double st = sin(s[2] * 3.14159265358979323846 / 180.0);
	const double ct = cos(s[2] * 3.14159265358979323846 / 180.0);

	p[0]	= o[0] - s[0] * cp * st;
	p[1]	= o[1] + s[0] * sp;
	p[2]	= o[2] - s[0] * cp * ct;

	e[0][0] =  ct;
	e[0][1] =  0.0f;
	e[0][2] = -st;

	e[1][0] =  sp * st;
	e[1][1] =  cp;
	e[1][2] =  sp * ct;

	e[2][0] =  cp * st;
	e[2][1] = -sp;
	e[2][2] =  cp * ct;
}

void glv_get_frust(double d[6][4])
{
	const double k = (double) h / (w * z);

	double p[3], e[3][3];

	glv_get_space(p, e);

	d[0][0] = -e[2][0];
	d[0][1] = -e[2][1];
	d[0][2] = -e[2][2];
	d[0][3] = d[0][0] * p[0] + d[0][1] * p[1] + d[0][2] * p[2] + n;

	d[1][0] = +e[2][0];
	d[1][1] = +e[2][1];
	d[1][2] = +e[2][2];
	d[1][3] = d[1][0] * p[0] + d[1][1] * p[1] + d[1][2] * p[2] - f;

	d[2][0] = -(e[2][0] * n + e[0][0] * k);
	d[2][1] = -(e[2][1] * n + e[0][1] * k);
	d[2][2] = -(e[2][2] * n + e[0][2] * k);
	norm(d[2]);
	d[2][3] = d[2][0] * p[0] + d[2][1] * p[1] + d[2][2] * p[2];

	d[3][0] = -(e[2][0] * n - e[0][0] * k);
	d[3][1] = -(e[2][1] * n - e[0][1] * k);
	d[3][2] = -(e[2][2] * n - e[0][2] * k);
	norm(d[3]);
	d[3][3] = d[3][0] * p[0] + d[3][1] * p[1] + d[3][2] * p[2];

	d[4][0] = -(e[2][0] * n + e[1][0] / z);
	d[4][1] = -(e[2][1] * n + e[1][1] / z);
	d[4][2] = -(e[2][2] * n + e[1][2] / z);
	norm(d[4]);
	d[4][3] = d[4][0] * p[0] + d[4][1] * p[1] + d[4][2] * p[2];

	d[5][0] = -(e[2][0] * n - e[1][0] / z);
	d[5][1] = -(e[2][1] * n - e[1][1] / z);
	d[5][2] = -(e[2][2] * n - e[1][2] / z);
	norm(d[5]);
	d[5][3] = d[5][0] * p[0] + d[5][1] * p[1] + d[5][2] * p[2];
}

/*====================================================================*/

