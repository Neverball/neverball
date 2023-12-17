#ifndef JOY_H
#define JOY_H

void joy_init(void);
void joy_quit(void);

void joy_add(int device);
void joy_remove(int instance);
int  joy_button(int instance, int b, int d);
void joy_axis(int instance, int a, float v);

#endif