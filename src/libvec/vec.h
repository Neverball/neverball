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

#ifndef VEC_H
#define VEC_H

#define V_RAD(d) (d * 3.14159265358979323846 / 180.0)
#define V_DEG(r) (r * 180.0 / 3.14159265358979323846)

/*---------------------------------------------------------------------------*/

#define v_dot(u, v)  ((u)[0] * (v)[0] + (u)[1] * (v)[1] + (u)[2] * (v)[2])
#define v_len(u) sqrt((u)[0] * (u)[0] + (u)[1] * (u)[1] + (u)[2] * (u)[2])

#define v_cpy(u, v) { \
    (u)[0] = (v)[0];  \
    (u)[1] = (v)[1];  \
    (u)[2] = (v)[2];  \
}

#define v_inv(u, v) { \
    (u)[0] = -(v)[0]; \
    (u)[1] = -(v)[1]; \
    (u)[2] = -(v)[2]; \
}

#define v_scl(u, v, k) {   \
    (u)[0] = (v)[0] * (k); \
    (u)[1] = (v)[1] * (k); \
    (u)[2] = (v)[2] * (k); \
}

#define v_add(u, v, w) {      \
    (u)[0] = (v)[0] + (w)[0]; \
    (u)[1] = (v)[1] + (w)[1]; \
    (u)[2] = (v)[2] + (w)[2]; \
}

#define v_sub(u, v, w) {      \
    (u)[0] = (v)[0] - (w)[0]; \
    (u)[1] = (v)[1] - (w)[1]; \
    (u)[2] = (v)[2] - (w)[2]; \
}

#define v_mid(u, v, w) {              \
    (u)[0] = ((v)[0] + (w)[0]) / 2.0; \
    (u)[1] = ((v)[1] + (w)[1]) / 2.0; \
    (u)[2] = ((v)[2] + (w)[2]) / 2.0; \
}

#define v_mad(u, p, v, t) {         \
    (u)[0] = (p)[0] + (v)[0] * (t); \
    (u)[1] = (p)[1] + (v)[1] * (t); \
    (u)[2] = (p)[2] + (v)[2] * (t); \
}

/*---------------------------------------------------------------------------*/


void   v_nrm(double[3], const double[3]);
void   v_crs(double[3], const double[3], const double[3]);

void   m_cpy(double[16], const double[16]);
void   m_xps(double[16], const double[16]);
int    m_inv(double[16], const double[16]);

void   m_ident(double[16]);
void   m_basis(double[16], const double[3],
                           const double[3],
                           const double[3]);

void   m_xlt(double[16], const double[3]);
void   m_scl(double[16], const double[3]);
void   m_rot(double[16], const double[3], double);

void   m_mult(double[16], const double[16], const double[16]);
void   m_pxfm(double[3],  const double[16], const double[3]);
void   m_vxfm(double[3],  const double[16], const double[3]);

#endif
