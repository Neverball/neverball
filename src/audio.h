#ifndef AUDIO_H
#define AUDIO_H

#define AUD_TITLE  0
#define AUD_MENU   1
#define AUD_LEVEL  2
#define AUD_READY  3
#define AUD_SET    4
#define AUD_GO     5
#define AUD_BALL   6
#define AUD_BUMP   7
#define AUD_COIN   8
#define AUD_TICK   9
#define AUD_GOAL  10
#define AUD_FALL  11
#define AUD_TIME  12
#define AUD_OVER  13
#define AUD_PAUSE 14
#define AUD_COUNT 15

int  audio_init(int, int);
void audio_play(int, float);
void audio_free(void);

#endif
