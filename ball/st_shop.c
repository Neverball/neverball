/*
 * Copyright (C) 2003 Robert Kooima
 *
 * NEVERBALL is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

#include <string.h>
#include <assert.h>

#include "config.h"
#include "gui.h"
#include "vec3.h"
#include "audio.h"
#include "image.h"
#include "video.h"

#include "game_common.h"

#include "st_shop.h"
#include "st_title.h"
#include "st_shared.h"

/*---------------------------------------------------------------------------*/

static int productkey;

static int coinwallet;

static int evalue;
static int fvalue;
static int svalue;

// The enumerations was merged together into products: Extra Levels, Extra Balls, Bonus Pack, Mediation
// incl. (Earninator, Floatifier, Speedifier)
static const char products[][16] = {
    "extralevels",
    "onlineballs",
    "bonuspack",
    "mediation",
    "earninator",
    "floatifier",
    "speedifier",
};

enum
{
    SHOP_BACK = GUI_LAST,
    SHOP_GETCOINS,
    SHOP_BUY,
    SHOP_PURCHASED
};

static int shop_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);
    
    switch (tok)
    {
    case SHOP_BACK:
        return goto_state(&st_title);
        break;
    case SHOP_GETCOINS:
        return goto_state(&st_shop_getcoins);
        break;
    case SHOP_BUY:
        set_product_key(val);
	if (strlen(config_get_s(CONFIG_PLAYER)) == 0)
        return goto_state(&st_shop_unregistered);
	else
	return goto_state(&st_shop_buy);
        break;
    }
    return 1;
}

static int shop_gui(void)
{
	coinwallet = config_get_d(CONFIG_ACCOUNT_WALLET);

	evalue = config_get_d(CONSUMEABLE_ACCOUNT_EARNINATOR);
	fvalue = config_get_d(CONSUMEABLE_ACCOUNT_FLOATIFIER);
	svalue = config_get_d(CONSUMEABLE_ACCOUNT_SPEEDIFIER);

	int w = video.device_w;
	int h = video.device_h;

    int p0 = (config_get_d(PRODUCT_ACCOUNT_LEVELS) == 1) ? gui_gry : gui_wht;
    int p1 = (config_get_d(PRODUCT_ACCOUNT_BALLS) == 1) ? gui_gry : gui_wht;
    int p2 = (config_get_d(PRODUCT_ACCOUNT_BONUS) == 1) ? gui_gry : gui_wht;
    int p3 = (config_get_d(PRODUCT_ACCOUNT_MEDIATION) == 1) ? gui_gry : gui_wht;

	int id, jd, kd, ld;
	if ((id = gui_vstack(0)))
	{
		if ((jd = gui_hstack(id)))
		{
			gui_state(jd, "+", GUI_SML, SHOP_GETCOINS, 0);
			char coinsattr[MAXSTR];
			sprintf(coinsattr, "%s: %i", _("Coins"), coinwallet);
			gui_label(jd, coinsattr, GUI_SML, gui_wht, gui_yel);
			gui_filler(jd);
			gui_state(jd, _("Back"), GUI_SML, SHOP_BACK, 0);
		}

		/*gui_space(id);

		if ((jd = gui_hstack(id)))
		{
			gui_label(jd, _("Select Product to buy"), GUI_SML, gui_yel, gui_red);
		}*/

		
		if ((jd = gui_hstack(id)))
		{
			// Consumeables
			if ((kd = gui_vstack(jd)))
			{
				if ((ld = gui_vstack(kd)))
				{
					gui_space(ld);
					gui_state(ld, _("Speedifier"), GUI_SML, 0, 0);
					gui_image(ld, "gui/shop/consum_speedifier.jpg", w / 7, h / 6);
					gui_filler(ld);
					gui_set_state(ld, SHOP_BUY, 6);
				}
			}

			if ((kd = gui_vstack(jd)))
			{
				if ((ld = gui_vstack(kd)))
				{
					gui_space(ld);
					gui_state(ld, _("Earninator"), GUI_SML, 0, 0);
					gui_image(ld, "gui/shop/consum_earninator.jpg", w / 7, h / 6);
					gui_filler(ld);
					gui_set_state(ld, SHOP_BUY, 4);
				}

				if ((ld = gui_vstack(kd)))
				{
					gui_space(ld);
					gui_state(ld, _("Floatifier"), GUI_SML, 0, 0);
					gui_image(ld, "gui/shop/consum_floatifier.jpg", w / 7, h / 6);
					gui_filler(ld);
					gui_set_state(ld, SHOP_BUY, 5);
				}
			}

			gui_space(jd);

			// Bonus Pack and Mediation
			if ((kd = gui_vstack(jd)))
			{
				if ((ld = gui_vstack(kd)))
				{
					gui_space(ld);
					if (config_get_d(PRODUCT_ACCOUNT_BONUS) == 0) gui_state(ld, _("Bonus Pack"), GUI_SML, 0, 0); else gui_label(ld, _("Bonus Pack"), GUI_SML, gui_gry, gui_gry);
					gui_image(ld, "gui/shop/bonus.jpg", w / 6, h / 6);
					gui_filler(ld);
					gui_set_state(ld, ((config_get_d(PRODUCT_ACCOUNT_BONUS) == 1) ? SHOP_PURCHASED : SHOP_BUY), 2);
				}

				if ((ld = gui_vstack(kd)))
				{
					gui_space(ld);
                    if (config_get_d(PRODUCT_ACCOUNT_MEDIATION) == 0) gui_state(ld, _("Mediation"), GUI_SML, 0, 0); else gui_label(ld, _("Mediation"), GUI_SML, gui_gry, gui_gry);
					gui_image(ld, "gui/shop/mediation.jpg", w / 6, h / 6);
					gui_filler(ld);
					gui_set_state(ld, ((config_get_d(PRODUCT_ACCOUNT_MEDIATION) == 1) ? SHOP_PURCHASED : SHOP_BUY), 3);
				}
			}

			// Extra levels and online balls
			if ((kd = gui_vstack(jd)))
			{
				if ((ld = gui_vstack(kd)))
				{
					gui_space(ld);
                    if (config_get_d(PRODUCT_ACCOUNT_LEVELS) == 0) gui_state(ld, _("Extra Levels"), GUI_SML, 0, 0); else gui_label(ld, _("Extra Levels"), GUI_SML, gui_gry, gui_gry);
					gui_image(ld, "gui/shop/levels.jpg", w / 6, h / 6);
					gui_filler(ld);
					gui_set_state(ld, ((config_get_d(PRODUCT_ACCOUNT_LEVELS) == 1) ? SHOP_PURCHASED : SHOP_BUY), 0);
				}

				if ((ld = gui_vstack(kd)))
				{
					gui_space(ld);
                    if (config_get_d(PRODUCT_ACCOUNT_BALLS) == 0) gui_state(ld, _("Online Balls"), GUI_SML, 0, 0); else gui_label(ld, _("Online Balls"), GUI_SML, gui_gry, gui_gry);
					gui_image(ld, "gui/shop/balls.jpg", w / 6, h / 6);
					gui_filler(ld);
					gui_set_state(ld, ((config_get_d(PRODUCT_ACCOUNT_BALLS) == 1) ? SHOP_PURCHASED : SHOP_BUY), 1);
				}
			}
		}

		gui_layout(id, 0, 0);
	}
	return id;
}

static int shop_enter(struct state *st, struct state *prev)
{
        coinwallet = config_get_d(CONFIG_ACCOUNT_WALLET);
	return shop_gui();
}

static int shop_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT)
            return shop_action(SHOP_BACK, 0);
    }
    return 1;
}

static int shop_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return shop_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return shop_action(SHOP_BACK, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

enum
{
	SHOP_UNREGISTERED_YES,
	SHOP_UNREGISTERED_CANCEL = GUI_LAST
};

static int shop_unregistered_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);
    
    switch (tok)
    {
    case SHOP_UNREGISTERED_YES:
        return goto_name(&st_shop_buy, &st_shop, 0);
        break;
    case SHOP_UNREGISTERED_CANCEL:
        return goto_state(&st_shop);
        break;
    }
    return 1;
}

static int shop_unregistered_gui(void)
{
	int id, jd;
	if ((id = gui_vstack(0)))
	{
		gui_label(id, _("Unregistered!"), GUI_MED, gui_gry, gui_red);

		gui_space(id);		

		gui_multi(id, _("You didn't registered your player name yet!\\Would you like register now before you buy?"), GUI_SML, gui_wht, gui_wht);

		gui_space(id);

		if ((jd = gui_harray(id)))
		{
			gui_state(jd, _("No, thanks!"), GUI_SML, SHOP_UNREGISTERED_CANCEL, 0);
			gui_state(jd, _("Yes, register now!"), GUI_SML, SHOP_UNREGISTERED_YES, 0);
		}

		gui_layout(id, 0, 0);
	}
	return id;
}

static int shop_unregistered_enter(struct state *st, struct state *prev)
{
	return shop_unregistered_gui();
}

static int shop_unregistered_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT)
            return shop_unregistered_action(SHOP_UNREGISTERED_CANCEL, 0);
    }
    return 1;
}

static int shop_unregistered_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return shop_unregistered_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return shop_unregistered_action(SHOP_UNREGISTERED_CANCEL, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

// IAP included (25 coins = 1,29 €)
static const char iapcoin[][16] = {
    "tier1",
    "tier2",
    "tier3",
    "tier4",
    "tier5"
};

static const char iaplabel[][16] = {
    "3.69 $",
    "11.19 $",
    "17.59 $",
    "35.19 $",
    "79.99 $"
};

enum
{
	SHOP_GETCOINS_BUY,
	SHOP_GETCOINS_CANCEL = GUI_LAST
};

static int shop_getcoins_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);
    
    switch (tok)
    {
    case SHOP_GETCOINS_CANCEL:
        return goto_state(&st_shop);
        break;
    }
    return 1;
}

static int shop_getcoins_gui(void)
{
	int w = video.device_w;
	int h = video.device_h;

	int multiply;

	int id, jd, kd;
	if ((id = gui_vstack(0)))
	{
		if ((jd = gui_hstack(id)))
		{
			char walletattr[MAXSTR];
			sprintf(walletattr, _("You have %i %s!"), coinwallet, _("Coins"));

			gui_label(jd, walletattr, GUI_SML, gui_yel, gui_red);
			gui_filler(jd);
			gui_state(jd, _("Back"), GUI_SML, SHOP_BACK, 0);
		}

		gui_space(id);

		if ((jd = gui_hstack(id)))
		{
			for (multiply = 5; multiply > 0 /*sizeof iapcoin*/; multiply--)
			{
				if ((kd = gui_vstack(jd)))
				{
					char iapattr[MAXSTR];
					sprintf(iapattr, "%s", iaplabel[multiply - 1]);
					
					gui_image(kd, "gui/shop/iap/placeholder.jpg", w / 7, h / 6);
					gui_state(kd, iapattr, GUI_SML, 0, 0);
					gui_filler(kd);
					gui_set_state(kd, SHOP_GETCOINS_BUY, multiply - 1);
				}
			}
		}
		gui_layout(id, 0, 0);
	}
	return id;
}

static int shop_getcoins_enter(struct state *st, struct state *prev)
{
	return shop_getcoins_gui();
}

static int shop_getcoins_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT)
            return shop_getcoins_action(SHOP_GETCOINS_CANCEL, 0);
    }
    return 1;
}

static int shop_getcoins_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return shop_getcoins_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return shop_getcoins_action(SHOP_GETCOINS_CANCEL, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

void set_product_key(int newkey) {
	productkey = newkey;
}

enum
{
	SHOP_BUY_YES,
	SHOP_BUY_CANCEL = GUI_LAST
};

static int shop_buy_action(int tok, int val)
{
    audio_play(AUD_MENU, 1.0f);
    
    switch (tok)
    {
    case SHOP_BUY_YES:
        audio_play("snd/buyproduct.ogg", 1.0f);
        coinwallet -= val;
        config_set_d(CONFIG_ACCOUNT_WALLET, coinwallet);

        switch (productkey)
        {
            case 0:
                config_set_d(PRODUCT_ACCOUNT_LEVELS, 1);
                return goto_state(&st_shop);
                break;

            case 1:
                config_set_d(PRODUCT_ACCOUNT_BALLS, 1);
                return goto_state(&st_shop);
                break;

            case 2:
                config_set_d(PRODUCT_ACCOUNT_BONUS, 1);
                return goto_state(&st_shop);
                break;

            case 3:
                config_set_d(PRODUCT_ACCOUNT_MEDIATION, 1);
                return goto_state(&st_shop);
                break;

            case 4:
                evalue += 1;
                config_set_d(CONSUMEABLE_ACCOUNT_EARNINATOR, evalue);
                return goto_state(&st_shop);
                break;

            case 5:
                fvalue += 1;
                config_set_d(CONSUMEABLE_ACCOUNT_FLOATIFIER, fvalue);
                return goto_state(&st_shop);
                break;

            case 6:
                svalue += 1;
                config_set_d(CONSUMEABLE_ACCOUNT_SPEEDIFIER, svalue);
                return goto_state(&st_shop);
                break;
        }
        break;

    case SHOP_BUY_CANCEL:
        return goto_state(&st_shop);
        break;
    }
    return 1;
}

static int has_enough_coins(int wprodcost) {
	int enough = 0;
        int currentwallet = config_get_d(CONFIG_ACCOUNT_WALLET);
        if (currentwallet >= wprodcost) { enough = 1; }

	return enough;
}

static int shop_buy_gui(void)
{
	int id, jd;
	if ((id = gui_vstack(0)))
	{
		gui_label(id, _("Buy Products?"), GUI_MED, gui_yel, gui_red);

		gui_space(id);		

		char * prodname;
		int prodcost;
		switch (productkey)
		{
		case 0: prodname = _("Extra Levels"); prodcost = 250; break;
		case 1: prodname = _("Online Balls"); prodcost = 250; break;
		case 2: prodname = _("Bonus Pack"); prodcost = 120; break;
		case 3: prodname = _("Mediation"); prodcost = 120; break;
		case 4: prodname = _("Earninator"); prodcost = 75; break;
		case 5: prodname = _("Floatifier"); prodcost = 75; break;
		case 6: prodname = _("Speedifier"); prodcost = 75; break;
		default: prodname = "(proddata)"; prodcost = 0; break;
		}

		char prodattr[MAXSTR];
		if (has_enough_coins(prodcost))
			sprintf(prodattr, _("Would you like buy this Products?\\%s costs %i coins."), prodname, prodcost);
		else
			sprintf(prodattr, _("You need at least %i coins\\to buy %s!"), prodcost, prodname);

		gui_multi(id, prodattr, GUI_SML, gui_wht, gui_wht);

		gui_space(id);

		if ((jd = gui_harray(id)))
		{
			if (has_enough_coins(prodcost)) {
				gui_state(jd, _("No, thanks!"), GUI_SML, SHOP_BUY_CANCEL, 0);
				gui_state(jd, _("Yes, buy products!"), GUI_SML, SHOP_BUY_YES, prodcost);
			} else {
				gui_state(jd, _("Back"), GUI_SML, SHOP_BUY_CANCEL, 0);
			}
		}

		gui_layout(id, 0, 0);
	}
	return id;
}

static int shop_buy_enter(struct state *st, struct state *prev)
{
	return shop_buy_gui();
}

static int shop_buy_keybd(int c, int d)
{
    if (d)
    {
        if (c == KEY_EXIT)
            return shop_buy_action(SHOP_BUY_CANCEL, 0);
    }
    return 1;
}

static int shop_buy_buttn(int b, int d)
{
    if (d)
    {
        int active = gui_active();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return shop_buy_action(gui_token(active), gui_value(active));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_B, b))
            return shop_buy_action(SHOP_BUY_CANCEL, 0);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/
struct state st_shop = {
	shop_enter,
	shared_leave,
	shared_paint,
	shared_timer,
	shared_point,
	shared_stick,
	shared_angle,
	shared_click,
	shop_keybd,
	shop_buttn
};

struct state st_shop_unregistered = {
	shop_unregistered_enter,
	shared_leave,
	shared_paint,
	shared_timer,
	shared_point,
	shared_stick,
	shared_angle,
	shared_click,
	shop_unregistered_keybd,
	shop_unregistered_buttn
};

struct state st_shop_getcoins = {
	shop_getcoins_enter,
	shared_leave,
	shared_paint,
	shared_timer,
	shared_point,
	shared_stick,
	shared_angle,
	shared_click,
	shop_getcoins_keybd,
	shop_getcoins_buttn
};

struct state st_shop_buy = {
	shop_buy_enter,
	shared_leave,
	shared_paint,
	shared_timer,
	shared_point,
	shared_stick,
	shared_angle,
	shared_click,
	shop_buy_keybd,
	shop_buy_buttn
};
