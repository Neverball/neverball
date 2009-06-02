#ifndef HUD_H
#define HUD_H

/*---------------------------------------------------------------------------*/

#define STR_VIEW0 _("Chase")
#define STR_VIEW1 _("Lazy")
#define STR_VIEW2 _("Manual")
#define STR_VIEW3 _("Top-Down")

/*---------------------------------------------------------------------------*/

void hud_init(void);
void hud_free(void);

void hud_paint(void);
void hud_timer(float);
void hud_update(int);

void hud_view_pulse(int);

/*---------------------------------------------------------------------------*/

#endif
