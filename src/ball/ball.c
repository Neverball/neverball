#include <GL/gl.h>
#include <vec.h>

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

GLuint make_ball(int d)
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

		glEnable(GL_COLOR_MATERIAL);

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

GLuint make_sky(int d, double k)
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

		glBegin(GL_TRIANGLES);
		{
			section(d, p[4], c[1], p[2], c[0], p[0], c[1]);
			section(d, p[0], c[1], p[2], c[0], p[5], c[1]);
			section(d, p[5], c[1], p[2], c[0], p[1], c[1]);
			section(d, p[1], c[1], p[2], c[0], p[4], c[1]);

			section(d, p[0], c[1], p[3], c[2], p[4], c[1]);
			section(d, p[5], c[1], p[3], c[2], p[0], c[1]);
			section(d, p[1], c[1], p[3], c[2], p[5], c[1]);
			section(d, p[4], c[1], p[3], c[2], p[1], c[1]);
		}
		glEnd();
	}
	glPopMatrix();
	glPopAttrib();
	glEndList();

	return list;
}

/*--------------------------------------------------------------------*/

