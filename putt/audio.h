#ifndef AUDIO_H
#define AUDIO_H

/*---------------------------------------------------------------------------*/

#define AUD_BIRDIE   0
#define AUD_BOGEY    1
#define AUD_BUMP     2
#define AUD_DOUBLE   3
#define AUD_EAGLE    4
#define AUD_JUMP     5
#define AUD_MENU     6
#define AUD_ONE      7
#define AUD_PAR      8
#define AUD_PENALTY  9
#define AUD_PLAYER1 10
#define AUD_PLAYER2 11
#define AUD_PLAYER3 12
#define AUD_PLAYER4 13
#define AUD_SUCCESS 14
#define AUD_COUNT   15

/*---------------------------------------------------------------------------*/

#ifdef _WIN32
#define AUD_BUFF_HI 4096
#define AUD_BUFF_LO 2048
#else
#define AUD_BUFF_HI 2048
#define AUD_BUFF_LO 1024
#endif

/*---------------------------------------------------------------------------*/

void audio_init(void);
void audio_play(int, float);
void audio_free(void);

void audio_music_play(const char *);
void audio_music_fade(float);
void audio_music_stop(void);

void audio_volume(int, int);

/*---------------------------------------------------------------------------*/

#endif
