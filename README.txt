
                             * Neverball *


Tilt the  floor to roll a  ball through an obstacle  course within the
given  time.  If  the ball  falls  or time  expires, a  ball is  lost.
Collect 100 coins to earn an extra ball.  Red coins are worth 5.  Blue
coins are worth 10.

Neverball requires:

    SDL               http://www.libsdl.org/download-1.2.php
    SDL_image         http://www.libsdl.org/projects/SDL_image/
    SDL_mixer         http://www.libsdl.org/projects/SDL_mixer/
    SDL_ttf           http://www.libsdl.org/projects/SDL_ttf/



* BUILDING

    make                     (Unix)
or
    nmake /fMakefile-W32     (Windows with MSVC)

The executables  will be copied to  the base directory.   Maps will be
processed and  copied into  data/sol/.  The program  must be  run from
within  the base  directory in  order  for materials  and textures  to
resolve correctly.



* RUNNING

    ./neverball

Click Play  to begin.  The floor  tilts in the direction  of the mouse
pointer.  The left and right  mouse buttons rotate the view point left
and right about the ball.

Spacebar pauses  and resumes.  Escape ends  a game, and  escape at the
title screen exits.  F9 toggles an FPS meter.  F10 takes a screenshot.



* PERFORMANCE

By  default,  Neverball runs  in  a  "nice"  mode.  A  delay  function
executes after  each frame is  rendered, forcing a context  switch and
ensuring that the game does not  utilize 100% of the CPU.  Barring any
other  bottlenecks, this  causes the  frame-rate on  most  hardware to
appear to cap at about 90 FPS.

If this  is not fast enough  for you, or  you simply want to  test the
performance of  the game  on your hardware,  press F8 to  disable nice
mode.  This flag  will be stored in the  persistent configuration when
the game is exited.



* GAME CONTROLLERS

Gamepad and joystick input are supported.  The game will automatically
use the  first connected controller,  if any.  Gamepad input  does not
conflict with mouse  input, so there is no need  to choose between the
two.

However, gamepad input is  not currently configurable in-game.  If you
wish to  change the defaust axis  and button mappings,  you will first
need to determine the numbers assigned to each axis and button on your
controller.  You may do this  via the game control panel (jstest under
Linux) or simply by trial and error.

Functions are mapped to inputs in the global configuration file:

    data/config.txt

You may edit this  file freely, but keep in mind that  the game is not
guaranteed to run properly when given bad input.  Most key-value pairs
in config.txt  should be self-explanatory.  The  gamepad functions are
as follows:

    joy_axis_x         - Horizontal input axis number
    joy_axis_y         - Vertical input axis number

    joy_button_a       - Menu selection and activation
    joy_button_b       - Menu cancel and exit
    joy_button_r       - Counter-clockwise camera rotation
    joy_button_l       - Clockwise camera rotation
    joy_button_pause   - Game pause



<rlk@snth.net>
