
                              * PUTTNIK *

Puttnik is a small test program that simulates the physics of a sphere
interacting with faceted solid geometry.  It has been tested only under
Linux.

To build:

    make install

Libraries and binaires will be copied into puttnik/lib and puttnik/bin.
To run, puttnik/lib must be included in LD_LIBRARY_PATH.  Specify a .sol
file as a command line parameter.

    export LD_LIBRARY_PATH=lib

    bin/putt map/plinko.sol
    bin/putt map/u.sol

If the .sol files do not exists, use map2sol to create them.  This must
be done from the map/ directory in order for the materials to resolve
correctly.

    cd map/
    ../bin/map2sol plinko.map plinko.sol

Right drag rotates the view in a Quakish manner.  Left drag hits the
ball.  The mouse wheel moves the viewpoint forward and back.

The flight controls are dvorakish.  This may be changed in putt/main.c
in the function on_key.

    esc 1 2 3 4 5
    tab    , .
         a o e u

             spacebar

  .  - move forward
  e  - move backward
  ,  - move up
  o  - move down
  a  - strafe left
  u  - strafe right

Spacebar redrops the ball back at the origin.  Tab toggles performance
statistics, numbers 1-5 alter the size of the ball.  Escape exits.

