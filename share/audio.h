#ifndef AUDIO_H
#define AUDIO_H

/*---------------------------------------------------------------------------*/

#define MAXSND 32

void audio_init(void);
void audio_free(void);
void audio_bind(int, int, const char *);
void audio_play(int, float);

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
