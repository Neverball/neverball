
                      * SUPER EMPTY BALL *

Super Empty Ball is an simulation of the physics of spheres  interacting
with  faceted solid geometry.  It has been tested only under  Linux  and
OSX. It requires libjpeg.

To build:

    make install

The  executables will be copied into puttnik/bin.  All programs must  be
run  from within the map/  directory in order for materials and textures
to resolve correctly.

Before they can be viewed, .map files must be converted to .sol files.

    cd map/
    ../bin/map2sol move.map move.sol
    ../bin/map2sol flat.map flat.sol

To view, specify a .sol file as a command line parameter to ball.

    ../bin/ball move.sol
    ../bin/ball flat.sol

The world tilts in the direction of the mouse curser.  Spacebar  redrops
the ball back at the origin.  Escape exits.

Any resemblance to any other software,  game,  or simulation related  to
super  forms of balls possibly non-empty potentially containing  monkeys
is purely coincidental.

