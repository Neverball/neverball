#ifndef CONFIG_H
#define CONFIG_H

#include <SDL.h>

/*---------------------------------------------------------------------------*/

#define DEFAULT_NAME "Player"

/*
 * Changing the  working directory to CONFIG_PATH must  place the game
 * at  the root of  the game  asset hierarchy.   By default,  the game
 * expects to be run within its build tree, so it simply chdirs to the
 * local data directory.
 *
 * If the game is to be installed globally, change this value.
 */
#define CONFIG_PATH "./data"

/*
 * Global settings are stored in USER_CONFIG_FILE.  Replays are stored
 * in  USER_REPLAY_FILE.  These files  are placed  in the  user's home
 * directory as given by the HOME environment var.  If the config file
 * is deleted, it will be recreated using the defaults.
 */
#define USER_CONFIG_FILE ".neverballrc"
#define USER_REPLAY_FILE ".neverballrp"

#define CONFIG_DEF_WIDTH       800
#define CONFIG_DEF_HEIGHT      600
#define CONFIG_DEF_STEREO      0
#define CONFIG_DEF_CAMERA      0
#define CONFIG_DEF_TEXTURES    1
#define CONFIG_DEF_GEOMETRY    1
#define CONFIG_DEF_REFLECTION  1
#define CONFIG_DEF_AUDIO_RATE  44100
#define CONFIG_DEF_MOUSE_SENSE 300
#define CONFIG_DEF_MOUSE_INV   0
#define CONFIG_DEF_NICE        1
#define CONFIG_DEF_DONE        0
#define CONFIG_DEF_FPS         0
#define CONFIG_DEF_JOY         0
#define CONFIG_DEF_JOY_DEVICE  0
#define CONFIG_DEF_SOUND_VOL   10
#define CONFIG_DEF_MUSIC_VOL   6

#define CONFIG_DEF_AXIS_X      0
#define CONFIG_DEF_AXIS_Y      1
#define CONFIG_DEF_BUTTON_A    0
#define CONFIG_DEF_BUTTON_B    1
#define CONFIG_DEF_BUTTON_R    2
#define CONFIG_DEF_BUTTON_L    3
#define CONFIG_DEF_BUTTON_EXIT 4

#define CONFIG_DEF_KEY_CAM_1   SDLK_F1
#define CONFIG_DEF_KEY_CAM_2   SDLK_F2
#define CONFIG_DEF_KEY_CAM_3   SDLK_F3
#define CONFIG_DEF_KEY_CAM_R   SDLK_RIGHT
#define CONFIG_DEF_KEY_CAM_L   SDLK_LEFT

/*---------------------------------------------------------------------------*/

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RMASK 0xFF000000
#define GMASK 0x00FF0000
#define BMASK 0x0000FF00
#define AMASK 0x000000FF
#else
#define RMASK 0x000000FF
#define GMASK 0x0000FF00
#define BMASK 0x00FF0000
#define AMASK 0xFF000000
#endif

#ifdef _WIN32
#define CONFIG_DEF_AUDIO_BUFF  4096
#define FMODE_RB "rb"
#define FMODE_WB "wb"
#else
#define CONFIG_DEF_AUDIO_BUFF  2048
#define FMODE_RB "r"
#define FMODE_WB "w"
#endif

/*---------------------------------------------------------------------------*/

#define JOY_MAX 32767
#define JOY_MID 16383

#define MAXSTR 256
#define MAXLVL 26
#define MAXNAM 8

/*---------------------------------------------------------------------------*/

int  config_home(char *, const char *, size_t);
int  config_path(const char *, const char *);
int  config_demo(void);

void config_load(void);
void config_store(void);

/*---------------------------------------------------------------------------*/

int  config_mode(void);
int  config_w   (void);
int  config_h   (void);
int  config_view(void);
int  config_text(void);
int  config_geom(void);
int  config_refl(void);
int  config_rate(void);
int  config_buff(void);
int  config_sens(void);
int  config_inv(void);
int  config_nice(void);
int  config_fps (void);
int  config_sound(void);
int  config_music(void);
int  config_stereo(void);
int  config_joy_device(void);

int  config_axis_x(int);
int  config_axis_y(int);
int  config_button_a(int);
int  config_button_b(int);
int  config_button_r(int);
int  config_button_l(int);
int  config_button_X(int);

int  config_key_cam_1(int);
int  config_key_cam_2(int);
int  config_key_cam_3(int);
int  config_key_cam_r(int);
int  config_key_cam_l(int);

char *config_get_key_cam_1(void);
char *config_get_key_cam_2(void);
char *config_get_key_cam_3(void);

int  config_set_mode(int, int, int);
void config_set_text(int);
void config_set_geom(int);
void config_set_refl(int);
void config_set_high(int);
void config_set_view(int);
void config_set_audio(int, int);
void config_set_sound(int);
void config_set_music(int);

void config_tog_nice(void);
void config_tog_fps(void);

/*---------------------------------------------------------------------------*/

void config_push_persp(double, double, double);
void config_push_ortho(void);
void config_pop_matrix(void);

/*---------------------------------------------------------------------------*/

#endif
