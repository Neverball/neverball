#ifndef KEY_H
#define KEY_H 1

#include <SDL_keycode.h>

/* Names for some hard-coded keys. */

#define KEY_EXIT       SDLK_ESCAPE

#define KEY_LOOKAROUND SDLK_F5
#define KEY_WIREFRAME  SDLK_F6
#define KEY_RESOURCES  SDLK_F7
#define KEY_LEVELSHOTS SDLK_F8

#define KEY_FPS        SDLK_F9
#define KEY_POSE       SDLK_F10
#define KEY_FULLSCREEN SDLK_F11
#define KEY_SCREENSHOT SDLK_F12

#if defined(__EMSCRIPTEN__)
#define KEY_IS_PAUSE(c) ((c) == KEY_EXIT || (c) == SDLK_SPACE)
#else
#define KEY_IS_PAUSE(c) ((c) == KEY_EXIT)
#endif

#endif
