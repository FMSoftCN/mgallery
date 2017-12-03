
/* 
** $Id$
**
** Copyright (C) 2004 Feynman Software.
**
*/

#ifndef _MENU_H_
#define _MENU_H_

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>


#define ITEM_LAST	SYS_INFO 
#define ITEM_FIRST	KEY_SOUND
typedef enum
{
	KEY_SOUND = 0,
	LIGHT_TIME,   
	OFF_TIME,        
	LANGUAGE,
//	BKGND,  
	TIME_DATE, 
	SYS_INFO,
}menu_item_t;


#endif /* _MENU_H_ */
