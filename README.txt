
                              * PUTTNIK *

Puttnik  is a small test program that simulates the physics of a  sphere
interacting  with faceted solid geomtry.  It has been tested only  under
Linux. The build requires that the following directories exist:

    $HOME/bin
    $HOME/lib
    $HOME/include

To build:

    make install

To run:

    $HOME/bin/putt

The  putt  flight  controls  are  dvorakish.  This  may  be  changed  in
putt/main.c in the function on_key.

      , .
    a o e u

  .  - move forward
  e  - move backward
  ,  - move up
  o  - move down
  a  - strafe left
  u  - strafe right

Left  drag rotates the view in a Quakish manner.  Right click drops  the
ball.  The initial parameters of the ball may be changed in  putt/main.c
in ball_drop. drop.r is the radius, drop.w is the angular velocity, s is
the linear speed.

No,  you can't actually interact with the ball once it has been  set  in
motion.  You may redrop it at any time.

Spacebar  will redrop the ball on the same trajectory as the last  right
click.  This  is useful if you want to see the same interaction  from  a
different angle or with a different radius.

The radius of the ball may be changed during the simulation:

  1  - radius 1.00 m
  2  - radius 0.50 m
  3  - radius 2.00 m
  4  - radius 0.05 m

If  the  ball is within a solid when it is dropped,  or too close  to  a
solid when resized, it may pass through that solid.  This is your fault.

