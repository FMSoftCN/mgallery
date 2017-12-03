/*
** $Id$
**
** music_menu.c:
**
** Copyright (C) 2006 Feynman Software.
**
** All right reserved by Feynman Software.
**
** Current maintainer: Wang Xuguang.
**
** Create date: 2007/05/17
*/

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mgext.h>

#include "decoder.h"
#include "pmp.h"
#include "text_id.h"

#include "music_menu.h"
#include "pmp_music.h"
//#include "../include/music.h"

extern void* music_audio_player;

static int __music_menu_texts[] = 
{
    IDS_PMP_MUSIC_VOLUME,//	N_("volume\t%d"), 
	IDS_PMP_MUSIC_PLAYTYPE, //N_("play type\t%s"),
	IDS_PMP_MUSIC_LOOPTYPE,//N_("loop type\t%s"), 
	IDS_PMP_MUSIC_OPERATOR,//N_("operator\t%s"), 
	IDS_PMP_MUSIC_SPEED, //N_("speed\t%d"),
	IDS_PMP_MUSIC_FADE, //N_("fade\t%s"),	
	IDS_PMP_MUSIC_ABWAITTIME, //N_("AB wait time\t%d"),
	IDS_PMP_MUSIC_ABCOUNT,//N_("AB count\t%d"),
	IDS_PMP_MUSIC_EQMODE, //N_("EQ mode\t%s"),
    IDS_PMP_MUSIC_EQUSR1, //
	IDS_PMP_MUSIC_EQUSR2, //N_("   EQ USR1\t%d"),	
	IDS_PMP_MUSIC_EQUSR3, //_("   EQ USR3\t%d"),	
};

static int __music_playtype_texts[] = 
{
	IDS_PMP_MUSIC_LIST, //N_("list"),
	IDS_PMP_MUSIC_DIR, //N_("dir"),
	IDS_PMP_MUSIC_PREVIEW, //N_("preview"),
};

static int __music_looptype_texts[] =
{
	IDS_PMP_MUSIC_ONLYONE, //N_("only one"),
	IDS_PMP_MUSIC_RANDOM, //N_("random"),
	IDS_PMP_MUSIC_INORDER, //N_("in order"),
    IDS_PMP_MUSIC_ALL, //	N_("all"),
};

static int __music_operator_texts[] =
{
	IDS_PMP_MUSIC_ABPLAY, //N_("AB play"),
/*	
	N_("AB restore"),
	N_("tag play"),
	N_("BPT play"),
	N_("mark tag"),
	N_("alter speed"),
	*/
};

static int __music_eqmode_texts[] =
{
	IDS_PMP_MUSIC_NORMAL, //_("NORMAL"),
	IDS_PMP_MUSIC_ROCK, //N_("ROCK"),
	IDS_PMP_MUSIC_POP, //N_("POP"),
	IDS_PMP_MUSIC_CLASSIC, //N_("CLASSIC"),
	IDS_PMP_MUSIC_SOFT, //N_("SOFT"),
	IDS_PMP_MUSIC_JAZZ, //_("JAZZ"),
	IDS_PMP_MUSIC_DBB, //N_("DBB"),
	IDS_PMP_MUSIC_SRS, //N_("SRS"),
	IDS_PMP_MUSIC_WOW, //N_("WOW"),
	IDS_PMP_MUSIC_BASS, //N_("BASS"),
	IDS_PMP_MUSIC_USER, //N_("USER")
};

static int __music_fade_str[] = 
{ 
    IDS_PMP_MUSIC_OFF, /*("OFF"),*/ 
    IDS_PMP_MUSIC_ON, //N_("ON") 
};



static void _music_setdsp_volume(void* param)
{
	menu_context_t* menu_cx = (menu_context_t*)param;
	audioDecCmd(music_audio_player, SET_VOLUME, (UINT)(menu_cx->volume.value));
}

static void _music_setdsp_eq(void* param)
{
	menu_context_t* menu_cx = (menu_context_t*)param;
	
	eq_set_t eq_set;
	eq_set.eq_mode = menu_cx->eq_mode.value;
	eq_set.param[0] = menu_cx->eq_user_1.value;
	eq_set.param[1] = menu_cx->eq_user_2.value;
	eq_set.param[2] = menu_cx->eq_user_3.value;

	audioDecCmd(music_audio_player, SET_EQ, (UINT)&(eq_set));
}

menu_context_t ex__music_menu_cx =
{
	{20, 0, 31, FALSE, NULL, _music_setdsp_volume}, //volume
	{LIST_PLAY, LIST_PLAY, PREVIEW_PLAY, TRUE, __music_playtype_texts, NULL}, //play_type 
	{IN_ORDER, ONLY_ONE, IN_ORDER, TRUE, __music_looptype_texts, NULL}, //loop_type
	{AB_PLAY, AB_PLAY, AB_PLAY, TRUE, __music_operator_texts, NULL}, //operator

	{0, -2, 4, FALSE, NULL, NULL},//speed
	{0, 0, 1,  TRUE, __music_fade_str, NULL}, //fade
	{1, 1, 10, FALSE, NULL, NULL}, //ab_wait_time
	{8, 1, 10, FALSE, NULL, NULL}, //ab_count

	{NORMAL, NORMAL, USR_MODE, TRUE, __music_eqmode_texts, _music_setdsp_eq}, //eq
	{5, 0, 5, FALSE, NULL, _music_setdsp_eq}, //eq_usr1
	{5, 0, 5, FALSE, NULL, _music_setdsp_eq}, //eq_usr2
	{5, 0, 5, FALSE, NULL, _music_setdsp_eq}, //eq_usr3
};


static inline void _update_menu_item(char* text_buff, mi_context_t* mi_contexts, int i)
{
	if (mi_contexts[i].msg_ids)
	{
		sprintf(text_buff, _(__music_menu_texts[i]), 
				_(mi_contexts[i].msg_ids[ mi_contexts[i].value ]));
	}
	else
	{
		sprintf(text_buff, _(__music_menu_texts[i]), mi_contexts[i].value); 
	}
	printf("%s\n", text_buff);
}

static int _menu_item_callback (void* context, int id, int op, char *text_buff)
{
	mi_context_t* mi_contexts = (mi_context_t*)context;
	menu_context_t* menu_cx = (menu_context_t*)context;
	int i = id - MIID_BASE_MUSIC;

	switch (op) {
		case PMP_MI_OP_DEF:
			_update_menu_item(text_buff, mi_contexts, i);
			return PMP_MI_ST_UPDATED;

		case PMP_MI_OP_PREV:
			if (i == EQ_USR1 || i == EQ_USR2 || i == EQ_USR3)
			{
				if (menu_cx->eq_mode.value != USR_MODE)
				{
					_update_menu_item(text_buff, mi_contexts, i);
					return PMP_MI_ST_UPDATED;
				}
			}

			if (mi_contexts[i].is_roll)
			{
				int cur = mi_contexts[i].value - mi_contexts[i].min;
				int range = mi_contexts[i].max - mi_contexts[i].min + 1;
				cur = (cur + range -1) % range;
				mi_contexts[i].value = mi_contexts[i].min + cur; 
			}
			else
			{
				if (mi_contexts[i].value > mi_contexts[i].min)
					mi_contexts[i].value --;
			}

			_update_menu_item(text_buff, mi_contexts, i);

			if (mi_contexts[i].set_dsp)
			{
				mi_contexts[i].set_dsp(menu_cx);
			}
			return PMP_MI_ST_UPDATED;

		case PMP_MI_OP_NEXT:
			if (i == EQ_USR1 || i == EQ_USR2 || i == EQ_USR3)
			{
				if (menu_cx->eq_mode.value != USR_MODE)
				{
					_update_menu_item(text_buff, mi_contexts, i);
					return PMP_MI_ST_UPDATED;
				}
			}

			if (mi_contexts[i].is_roll)
			{
				int cur = mi_contexts[i].value - mi_contexts[i].min;
				int range = mi_contexts[i].max - mi_contexts[i].min + 1;
				cur = (cur + 1) % range;
				mi_contexts[i].value = mi_contexts[i].min + cur; 
			}
			else
			{
				if (mi_contexts[i].value < mi_contexts[i].max)
					mi_contexts[i].value ++;
			}
			_update_menu_item(text_buff, mi_contexts, i);
			
			if (mi_contexts[i].set_dsp)
			{
				mi_contexts[i].set_dsp(menu_cx);
			}
			return PMP_MI_ST_UPDATED;

		case PMP_MI_OP_ENTER:
			return PMP_MI_ST_SELECTED;
	}

	return PMP_MI_ST_NONE;
}

PMP_MENU_ITEM ex__music_menu_item[] = 
{
    {VOLUME+MIID_BASE_MUSIC, _menu_item_callback},
    {PLAY_TYPE+MIID_BASE_MUSIC, _menu_item_callback},
    {LOOP_TYPE+MIID_BASE_MUSIC, _menu_item_callback},
    {OPERATOR+MIID_BASE_MUSIC, _menu_item_callback},

    {SPEED+MIID_BASE_MUSIC, _menu_item_callback},
    {FADE_IN_OUT+MIID_BASE_MUSIC, _menu_item_callback},
    {AB_WAIT_TIME+MIID_BASE_MUSIC, _menu_item_callback},
    {AB_COUNT+MIID_BASE_MUSIC, _menu_item_callback},

    {EQ_MODE+MIID_BASE_MUSIC, _menu_item_callback},
    {EQ_USR1+MIID_BASE_MUSIC, _menu_item_callback},
    {EQ_USR2+MIID_BASE_MUSIC, _menu_item_callback},
    {EQ_USR3+MIID_BASE_MUSIC, _menu_item_callback},

    {0, NULL}
};

