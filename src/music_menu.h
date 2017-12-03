/*
** $Id$
**
** music_menu.h: music menu item for Feynman PMP solution.
**
** Copyright (C) 2007 Feynman Software, all rights reserved.
**
** Use of this source package is subject to specific license terms
** from Beijing Feynman Software Technology Co., Ltd.
**
** URL: http://www.minigui.com
**
** Current maintainer: WangXuguang.
*/


#ifndef _MENU_H_
#define _MENU_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>


#define ITEM_LAST EQ_USR3
#define ITEM_FIRST	VOLUME
#define MIID_BASE_MUSIC 5 

typedef enum
{
	VOLUME = 0,
	PLAY_TYPE,   
	LOOP_TYPE,        
	OPERATOR,
	SPEED,  
	FADE_IN_OUT, 
	AB_WAIT_TIME,
	AB_COUNT, 
	EQ_MODE,
	EQ_USR1,
	EQ_USR2,
	EQ_USR3
}menu_index_t;

typedef enum
{
//	FILE_PLAY = 0,
	LIST_PLAY = 0,
	DIR_PLAY,
	PREVIEW_PLAY,
	PLAY_TYPE_NUM
}play_type_t;

typedef enum
{
	ONLY_ONE = 0,
	RANDOM_SEL,         
	IN_ORDER,                 
	ALL_PLAY,
	LOOP_TYPE_NUM,
}loop_type_t;

typedef enum
{
	AB_PLAY = 0,
	MUSIC_AB_RESTORE_PLAY,
	MUSIC_TAG_PLAY,
	BPT_PLAY,
	MARK_TAG,
	ALERT_SPEED
}
operator_t;

typedef struct
{
	int value;
	int min;
	int max;
	BOOL is_roll;
	int* msg_ids;
	void (*set_dsp)(void* param);
}mi_context_t;

//typedef mi_context_t menu_context_t[12];
typedef struct{
	mi_context_t volume;
	mi_context_t play_type;         
	mi_context_t loop_type;
	
	mi_context_t operator_id;
	mi_context_t speed;
	
	mi_context_t is_fade;
	mi_context_t ab_wait_time;
	mi_context_t ab_count;
	
	mi_context_t eq_mode;
	mi_context_t eq_user_1;
	mi_context_t eq_user_2;
	mi_context_t eq_user_3;

}menu_context_t; 

typedef struct{
    int volume;
    int play_type;    
    int loop_type;
    
    int operator_index;
    int speed;
    
    int is_fade;
    int ab_wait_time;
    int ab_count;
    
    unsigned int eq_mode;
    unsigned char eq_user_1;
    unsigned char eq_user_2;
    unsigned char eq_user_3;

}play_status_t; 

#endif /* _MENU_H_ */
