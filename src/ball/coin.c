/*   Copyright (C) 2003  Robert Kooima                                       */
/*                                                                           */
/*   SUPER EMPTY BALL  is  free software; you  can redistribute  it and/or   */
/*   modify  it under  the  terms  of  the  GNU General Public License  as   */
/*   published by  the Free Software Foundation;  either version 2  of the   */
/*   License, or (at your option) any later version.                         */
/*                                                                           */
/*   This program is  distributed in the hope that it  will be useful, but   */
/*   WITHOUT  ANY   WARRANTY;  without   even  the  implied   warranty  of   */
/*   MERCHANTABILITY  or FITNESS FOR  A PARTICULAR  PURPOSE.  See  the GNU   */
/*   General Public License for more details.                                */

#include <GL/gl.h>
#include <math.h>

#include <vec.h>
#include <sol.h>

#include "main.h"

#define CR 0.15
#define DZ 0.01
#define PI 3.1415926

/*---------------------------------------------------------------------------*/

GLuint coin_init(int n)
{
    GLuint list = glGenLists(1);

    glNewList(list, GL_COMPILE);
    {
        int i;

        glBegin(GL_TRIANGLE_FAN);
        {
            glNormal3d(0.0, 0.0, +1.0);

            for (i = 0; i < n; i++)
                glVertex3d(CR * cos(+2.0 * PI * i / n),
                           CR * sin(+2.0 * PI * i / n), +DZ);
        }
        glEnd();

        glBegin(GL_TRIANGLE_FAN);
        {
            glNormal3d(0.0, 0.0, -1.0);
            
            for (i = 0; i < n; i++)
                glVertex3d(CR * cos(-2.0 * PI * i / n),
                           CR * sin(-2.0 * PI * i / n), -DZ);
        }
        glEnd();

        glBegin(GL_QUAD_STRIP);
        {
            for (i = 0; i <= n; i++)
            {
                double x = cos(2.0 * PI * i / n);
                double y = sin(2.0 * PI * i / n);

                glNormal3d(x, y, 0.0);
                glVertex3d(CR * x, CR * y, +DZ);
                glVertex3d(CR * x, CR * y, -DZ);
            }
        }
        glEnd();
    }
    glEndList();

    return list;
}

void coin_draw(GLuint list, const struct s_coin *cv, int cc)
{
    static const float c1[4] = { 1.0, 1.0, 0.0, 1.0 };
    static const float c2[4] = { 1.0, 0.2, 0.2, 1.0 };
    static const float c3[4] = { 0.2, 0.2, 1.0, 1.0 };
    static const float  s[4] = { 1.0, 1.0, 1.0, 1.0 };

    double r = 360.0 * fmod(time_state(), 1.0);
    int i;

    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s);

    for (i = 0; i < cc; i++)
    {
        if (0 < cv[i].n && cv[i].n < 5)
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, c1);

        if (4 < cv[i].n && cv[i].n < 10)
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, c2);

        if (9 < cv[i].n)
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, c3);

        if (0 < cv[i].n)
        {
            glPushMatrix();
            {
                glTranslated(cv[i].p[0],
                             cv[i].p[1],
                             cv[i].p[2]);
                glRotated(r, 0.0, 1.0, 0.0);
                glCallList(list);
            }
            glPopMatrix();
        }
    }
}

int coin_test(const struct s_ball *up, struct s_coin *cv, int cc)
{
    int i, n = 0;

    for (i = 0; i < cc; i++)
    {
        double r[3];

        v_sub(r, up->p, cv[i].p);

        if (v_len(r) < CR + up->r)
        {
            n += cv[i].n;
            cv[i].n = 0;
        }
    }

    return n;
}

/*---------------------------------------------------------------------------*/
