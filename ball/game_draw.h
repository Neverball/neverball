#ifndef GAME_DRAW_H
#define GAME_DRAW_H

#include "solid_draw.h"
#include "game_common.h"

struct game_draw
{
    int state;

    struct s_full file;
    struct s_full back;

    int reflective;                     /* Reflective geometry used?         */

    struct game_tilt tilt;              /* Floor rotation                    */
    struct game_view view;              /* Current view                      */

    int   goal_e;                       /* Goal enabled flag                 */
    float goal_k;                       /* Goal animation                    */

    int   jump_e;                       /* Jumping enabled flag              */
    int   jump_b;                       /* Jump-in-progress flag             */
    float jump_dt;                      /* Jump duration                     */

    float fade_k;                       /* Fade in/out level                 */
    float fade_d;                       /* Fade in/out direction             */
};

/* FIXME: this is just for POSE_* constants. */
#include "game_client.h"

void game_draw(const struct game_draw *, int, float);

#endif
