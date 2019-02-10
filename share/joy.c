#include <SDL_events.h>
#include <SDL_joystick.h>

#include "joy.h"
#include "state.h"
#include "common.h"
#include "log.h"

#define JOY_MAX 16

static struct
{
    SDL_Joystick *joy;
    SDL_JoystickID id;
} joysticks[JOY_MAX];

static SDL_JoystickID joy_curr = -1;

/*
 * Initialize joystick state.
 */
void joy_init()
{
    size_t i = 0;

    for (i = 0; i < ARRAYSIZE(joysticks); ++i)
    {
        joysticks[i].joy = NULL;
        joysticks[i].id = -1;
    }

    SDL_JoystickEventState(SDL_ENABLE);
}

/*
 * Close all joysticks.
 */
void joy_quit()
{
    size_t i = 0;

    for (i = 0; i < ARRAYSIZE(joysticks); ++i)
        if (joysticks[i].joy)
            joy_remove(joysticks[i].id);
}

/*
 * Handle joystick add event.
 */
void joy_add(int device)
{
    log_printf("Joystick added (device %d)\n", device);

    SDL_Joystick *joy = SDL_JoystickOpen(device);

    if (joy)
    {
        size_t i = 0;

        for (i = 0; i < ARRAYSIZE(joysticks); ++i)
        {
            if (joysticks[i].joy == NULL)
            {
                joysticks[i].joy = joy;
                joysticks[i].id = SDL_JoystickInstanceID(joy);
                log_printf("Joystick opened (instance %d)\n", joysticks[i].id);

                joy_curr = joysticks[i].id;
                log_printf("Joystick %d made current via device addition\n", joy_curr);

                break;
            }
        }

        if (i == ARRAYSIZE(joysticks))
        {
            SDL_JoystickClose(joy);
            log_printf("Joystick %d not opened, %ud open joysticks reached\n", device, ARRAYSIZE(joysticks));
        }
    }
}

/*
 * Handle joystick remove event.
 */
void joy_remove(int instance)
{
    size_t i;

    log_printf("Joystick removed (instance %d)\n", instance);

    for (i = 0; i < ARRAYSIZE(joysticks); ++i)
    {
        if (joysticks[i].id == instance)
        {
            SDL_JoystickClose(joysticks[i].joy);
            joysticks[i].joy = NULL;
            joysticks[i].id = -1;
            log_printf("Joystick closed (instance %d)\n", instance);
        }
    }
}

/*
 * Handle joystick button event.
 */
int joy_button(int instance, int b, int d)
{
    if (joy_curr == instance)
    {
        /* Process button event normally. */
        return st_buttn(b, d);
    }
    else
    {
        /* Do not process button event, but make joystick current. */
        joy_curr = instance;
        log_printf("Joystick %d made current via button press\n", joy_curr);
        return 1;
    }
}

/*
 * Handle joystick axis event.
 */
void joy_axis(int instance, int a, float v)
{
    if (joy_curr == instance)
    {
        /* Process axis events from current joystick only. */
        st_stick(a, v);
    }
}