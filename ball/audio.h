#ifndef AUDIO_H
#define AUDIO_H

/*---------------------------------------------------------------------------*/

#define AUD_MENU   1
#define AUD_START  2
#define AUD_READY  3
#define AUD_SET    4
#define AUD_GO     5
#define AUD_BALL   6
#define AUD_BUMP   7
#define AUD_COIN   8
#define AUD_TICK   9
#define AUD_TOCK   10
#define AUD_SWITCH 11
#define AUD_JUMP   12
#define AUD_GOAL   13
#define AUD_SCORE  14
#define AUD_FALL   15
#define AUD_TIME   16
#define AUD_OVER   17
#define AUD_COUNT  18

/*---------------------------------------------------------------------------*/

void audio_init(void);
void audio_play(int, float);
void audio_free(void);

void audio_music_queue(const char *);
void audio_music_play(const char *);
void audio_music_stop(void);

void audio_music_fade_to(float, const char *);
void audio_music_fade_in(float);
void audio_music_fade_out(float);

void audio_timer(float);
void audio_volume(int, int);

/*---------------------------------------------------------------------------*/

#endif
