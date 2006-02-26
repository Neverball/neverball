#ifndef LEVEL_H
#define LEVEL_H

/*---------------------------------------------------------------------------*/

#define MODE_CHALLENGE	1
#define MODE_NORMAL	2
#define MODE_PRACTICE	3

const char *level_shot(int);
const char *level_coin_n(int, int);
const char *level_time_n(int, int);
int         level_coin_c(int, int);
int         level_time_c(int, int);
int         level_coin_t(int, int);
int         level_time_t(int, int);

void level_init(const char *, const char *, const char *);
void level_free(void);

int  level_exists(int);
int  level_opened(int);
int  level_locked(int);
int  level_extra_bonus(int i);
int  level_extra_bonus_opened(void);
int  level_set_completed(void);

int  curr_times_total(void);
int  curr_coins_total(void);
int  curr_count(void);
int  curr_score(void);
int  curr_coins(void);
int  curr_balls(void);
int  curr_level(void);
int  curr_goal (void);

int  level_replay(const char *);
void level_play(int, int);
int  level_play_go(void);
void level_next(void);

void level_stop(int);
int  level_dead(void);
int  level_last(void);
int  level_sort(int *, int *);
int  level_done(int *, int *);
int  level_score(int);
int  level_count(void);

void level_name(int, const char *, int, int);
void level_snap(int);
void level_song(void);

int level_mode(void);

void level_cheat(void);

/*---------------------------------------------------------------------------*/

#endif
