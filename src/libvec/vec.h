#ifndef VEC_H
#define VEC_H

#define V_RAD(d) (d * 3.14159265358979323846 / 180.0)
#define V_DEG(r) (r * 180.0 / 3.14159265358979323846)

double v_dot(const double[3], const double[3]);
double v_len(const double[3]);

void   v_cpy(double[3], const double[3]);
void   v_inv(double[3], const double[3]);
void   v_nrm(double[3], const double[3]);

void   v_scl(double[3], const double[3], double);
void   v_add(double[3], const double[3], const double[3]);
void   v_sub(double[3], const double[3], const double[3]);
void   v_crs(double[3], const double[3], const double[3]);
void   v_mid(double[3], const double[3], const double[3]);
void   v_mad(double[3], const double[3], const double[3], double);

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
