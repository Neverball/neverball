#ifndef HOLE_H
#define HOLE_H

/*---------------------------------------------------------------------------*/

#define MAXHOL 19
#define MAXPLY 5

#define HOLE_FILE "holes-putt.txt"

void  hole_init(void);
void  hole_free(void);
int   hole_exists(int);

char *hole_player(int);
char *hole_score(int, int);
char *hole_tot(int);
char *hole_out(int);
char *hole_in(int);

int  curr_hole(void);
int  curr_party(void);
int  curr_player(void);
int  curr_stroke(void);
int  curr_count(void);

void hole_goto(int, int);
int  hole_next(void);
void hole_goal(void);
void hole_stop(void);
void hole_fall(void);

void hole_song(void);

/*---------------------------------------------------------------------------*/

#endif
