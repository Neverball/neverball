#ifndef SPEED_H
#define SPEED_H

enum
{
    SPEED_NONE = 0,

    SPEED_SLOWEST,
    SPEED_SLOWER,
    SPEED_SLOW,
    SPEED_NORMAL,
    SPEED_FAST,
    SPEED_FASTER,
    SPEED_FASTEST,

    SPEED_MAX
};

extern float SPEED_FACTORS[];

#define SPEED_UP(s) MIN((s) + 1, SPEED_MAX - 1)
#define SPEED_DN(s) MAX((s) - 1, SPEED_NONE)

#endif
