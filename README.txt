
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
processed and copied into data/sol/.  By default, an uninstalled build
may be executed in place.



* RUNNING

    ./neverball

Click Play  to begin.  The floor  tilts in the direction  of the mouse
pointer.  The left and right  mouse buttons rotate the view point left
and right about the ball.

Spacebar pauses  and resumes.  Escape ends  a game, and  escape at the
title  screen  exits.   F9  toggles  a frame  counter.   F10  takes  a
screenshot.



* INSTALLATION

The game executable may be moved as desired.

However, game  assets are described  using relative path  names.  Thus
the game must  run within the data directory in  order for asset names
to resolve correctly.  Upon starting,  the game will attempt to change
into this directory, as defined in config.h.

If the data directory is to  be renamed or moved for installation, the
CONFIG_PATH variable in config.h must be changed to match.

Alternatively, the CONFIG_PATH variable may  be set to "." if the game
is started from within the data directory.



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

Gamepad and  joystick input are supported.  However,  gamepad input is
not currently  configurable in-game.  To enable  gamepad control, edit
the configuration  file, .neverballrc.  This file is  created when the
game exits, and should be found in your home directory.

Set the following keyword / value pair:

    joy 1

Note that the  game may still be controlled with  the mouse even while
gamepad  control is  enabled.  However,  random noise  from  an analog
controller at rest can disrupt normal mouse input.

If you wish  to change the defaust axis and  button mappings, you will
first need to  determine the numbers assigned to  each axis and button
on  your controller.   You may  do this  via the  game  control panel,
jstest under Linux, or simply by trial and error.

The gamepad configuration keyword / value pairs areas follows:

    axis_x         - Horizontal input axis number
    axis_y         - Vertical input axis number

    button_a       - Menu selection and activation
    button_b       - Menu cancel and exit
    button_r       - Counter-clockwise camera rotation
    button_l       - Clockwise camera rotation
    button_pause   - Game pause



<rlk@snth.net>
