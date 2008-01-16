#ifndef PROGRESS_H
#define PROGRESS_H

/*---------------------------------------------------------------------------*/

void progress_init(int);

int  progress_play(int level);
void progress_stat(int status);
void progress_stop(void);
void progress_exit(int status);

int  progress_next_avail(void);
int  progress_next(void);
int  progress_same_avail(void);
int  progress_same(void);

void progress_rename();

int  progress_replay(const char *);

int  progress_dead(void);
int  progress_done(void);

int  progress_lvl_high(void);
int  progress_set_high(void);

int  progress_time_rank(void);
int  progress_goal_rank(void);
int  progress_coin_rank(void);

int  progress_times_rank(void);
int  progress_score_rank(void);

int  curr_level(void);
int  curr_balls(void);
int  curr_score(void);
int  curr_mode (void);

int count_extra_balls(int, int);

/*---------------------------------------------------------------------------*/

#define MODE_CHALLENGE 1
#define MODE_NORMAL    2
#define MODE_PRACTICE  3

#define MODE_COUNT 3

const char *mode_to_str(int, int);

/*---------------------------------------------------------------------------*/

#endif
