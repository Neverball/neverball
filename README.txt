
                      * SUPER EMPTY BALL *

Super Empty Ball is an simulation of the physics of spheres  interacting
with  faceted solid geometry.  It has been tested only under  Linux  and
OSX. It requires:

    libjpeg
    libpng

To build:

    make install

To build with ALSA 0.9 sound:

    export CFLAGS="-DALSA"
    export ALSALIB="-lasound"
    make install

The executables will be copied into bin/. Environments will be processed
and copied into data/sol/.  The program must be run from within the base
seb directory in order for materials and textures to resolve correctly.

To run:

    bin/ball

Press space  to begin.  The  world tilts in  the direction of  the mouse
curser.  Escape pauses.  Control-X while paused exits.

The physics system starts to destablize at about 50Hz, so if your system
cannot  maintain  50 FPS  then  simulation  time  will run  slower  than
realtime.  If  your video card  is lacking in  power you can  shrink the
window  or  remove  the  definition  of  REFLECT  at  the  beginning  of
ball/game.c.  If  your processor is  lacking in power, there's  not much
you can do.

Any resemblance to any other software,  game,  or simulation related  to
super  forms of balls possibly non-empty potentially containing  monkeys
is purely coincidental.
