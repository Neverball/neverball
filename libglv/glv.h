#ifndef GLV_H
#define GLV_H

/*--------------------------------------------------------------------*/

#define GLV_NONE	0x0000

#define GLV_RIGHT	0x0001
#define GLV_LEFT	0x0002
#define GLV_UP		0x0004
#define GLV_DOWN	0x0008
#define GLV_BACK	0x0010
#define GLV_FORE	0x0020

#define GLV_POS_RHO	0x0040
#define GLV_NEG_RHO	0x0080
#define GLV_POS_PHI	0x0100
#define GLV_NEG_PHI	0x0200
#define GLV_POS_THETA	0x0400
#define GLV_NEG_THETA	0x0800

#define GLV_ZOOM_IN	0x1000
#define GLV_ZOOM_OUT	0x2000

/*--------------------------------------------------------------------*/

void   glv_apply_ortho(void);
void   glv_apply_persp(void);
void   glv_apply_model(void);

void   glv_apply_left(void);
void   glv_apply_right(void);

void   glv_apply_backdrop(void);
void   glv_apply_billboard(void);

int    glv_drag_begin(int, int, int, int);
int    glv_drag_step (int, int);
int    glv_drag_end  (int, int);

int    glv_time_begin(double, int);
int    glv_time_step (double);
int    glv_time_end  (double, int);

void   glv_set_shape(int, int);
void   glv_set_range(double, double, double);
void   glv_set_speed(double, double, double);

void   glv_set(double[3], double[3], double);
double glv_get(double[3], double[3]);

void   glv_get_point(double[3], double[3][3], int, int);
void   glv_get_space(double[3], double[3][3]);
void   glv_get_frust(double[6][4]);

/*--------------------------------------------------------------------*/

#endif
