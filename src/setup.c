/* 
** $Id$
**
** menu.c: menu for PMP 
**
** Copyright (C) 2004 Feynman Software.
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mgext.h>

#include <text_id.h>
#include "../include/pmp.h"
#include "setup_menu.h"
#include "setup.h"

#define ITEM_RECT_H 16
#define ITEM_RECT_W	240

#define ITEM_START_L	ITEM_RECT_H
#define ITEM_START_H	ITEM_RECT_H

#define MENU_WND_H	(ITEM_LAST+3)*ITEM_RECT_H
#define MENU_WND_W	(ITEM_RECT_W + 2 * ITEM_RECT_H)
#define MENU_WND_L	(320-MENU_WND_W)/2
#define MENU_WND_T	((220-MENU_WND_H)/2 + 20)

#define EQ_TEXT(a)	((a)*100/255) 

#define BUFFER_SIZE 256
	

extern int timedate_setup(HWND h_host);
//extern void setup_show_info(HWND h_host);


//static RECT rect;
static HWND hMainWnd;
BOOL wnd_creat = TRUE;
static RECT item_rect;

#define DEBUG
//menu items
static int menu_items[] = 
{
    IDS_PMP_SETUP_KEYSOUND, //N_("key sound"), 
    IDS_PMP_SETUP_BKLIGHTTIME,//N_("back light time"),
    IDS_PMP_SETUP_OFFTIME, //N_("poweroff time"), 
    IDS_PMP_SETUP_LANGUAGE, //N_("language"), 
    IDS_PMP_SETUP_DATE, //N_("time and date"),
    IDS_PMP_SETUP_SYSINFO, //N_("system infomation"),
};

static menu_item_t cur_menu_item = 0;
static menu_item_t old_menu_item;

/*
void draw_bkground(HWND hwnd, WPARAM wparam, LPARAM lparam)
{

	HDC hdc = (HDC)wparam;
	BOOL f_get_dc = FALSE;
	RECT* clip = (RECT*)lparam;
	RECT rect;
	if (hdc == 0)
	{
		hdc = GetClientDC(hwnd);
		f_get_dc = TRUE;
	}

	if (clip != NULL)
	{
		rect = *clip;
		ScreenToWindow(hwnd, &rect.left, &rect.top);
		ScreenToWindow(hwnd, &rect.right, &rect.bottom);
		IncludeClipRect(hdc, &rect);
	}

	if (!IsControl(hwnd))
	{
		FillBoxWithBitmap(hdc, 0, 0, 0, 0, bitmap_back);
	}
	else
	{
		RECT ctrl_in_par;
		GetWindowRect(hwnd, &ctrl_in_par);
		HWND h_parent = GetParent(hwnd);
		
		ScreenToWindow(h_parent, &(ctrl_in_par.left), &(ctrl_in_par.top));
		ScreenToWindow(h_parent, &(ctrl_in_par.right), &(ctrl_in_par.bottom));
		
		BOOL suc;
		suc = FillBoxWithBitmapPart(hdc, 0, 0, 0, 0, 0,0 ,bitmap_back, ctrl_in_par.left, ctrl_in_par.top);
		
		if (suc == FALSE)
		{
			printf("draw contral back pic FAILED\n");
			if (f_get_dc)
			{
				ReleaseDC(hdc);
			}
			return;

		}
	}
	
	printf("m==========================\n");
	if (f_get_dc)
	{
		ReleaseDC(hdc);
	}
}
*/

void make_text_rect(menu_item_t menu_item)
{
	item_rect.left = ITEM_START_L;
	item_rect.right = ITEM_START_L+ITEM_RECT_W;

	item_rect.top = ITEM_START_H + (ITEM_RECT_H)*menu_item;
	item_rect.bottom = item_rect.top + ITEM_RECT_H;
}

/*
static int get_item_value(menu_item_t item)
{
	switch(item) 
	{
		case KEY_SOUND:
			return get_key_sound();
		case LIGHT_TIME:
			return get_bklight();	
		case OFF_TIME:
			return get_shutdown_time();	
		case LANGUAGE:
			return get_language();	
		default:
			assert(0);
			return -1;
	}
}
*/

static void draw_menu_text(HDC hdc, int i)
{
	char str[20];

#ifdef DEBUG
	printf("draw_menu_text{{{\n");
#endif
	make_text_rect(i);

	/*out item name text*/
	DrawText(hdc, _(menu_items[i]), -1, &item_rect, DT_NOCLIP|DT_LEFT);

	/*out item value text*/
	memset(str, 0 , 10);
	switch(i) 
	{
		case KEY_SOUND:                
			out_key_sound(str);
			break;
		case LIGHT_TIME:                
			out_bklight(str);
			break;
		case OFF_TIME:               
			out_shutdown_time(str);
			break;
		case LANGUAGE:             
		   out_language(str);	
			break;
		default:	//TIME_DATE and SYS_INFO
			return;
			break;
	}
	DrawText(hdc, str, -1, &item_rect, DT_NOCLIP|DT_RIGHT);

#ifdef DEBUG
	printf("}}}draw_menu_text\n");
#endif
}

static void draw_menu_wnd(HWND hWnd)
{
    HDC hdc = BeginPaint(hWnd);
    int i;

#ifdef DEBUG
    printf("draw_menu_wnd{{{\n");
#endif

    SelectFont(hdc, GetSystemFont(SYSLOGFONT_WCHAR_DEF));
    SetBkMode(hdc, BM_TRANSPARENT);


    SetTextColor(hdc, RGB2Pixel(hdc, 255, 255, 255)); 

    for (i=ITEM_FIRST; i<=ITEM_LAST; i++)
    {
        if (i == cur_menu_item)
        {
            continue;
        }
        draw_menu_text(hdc, i);
    }

    SetTextColor(hdc, RGB2Pixel(hdc, 255, 255, 0));
    draw_menu_text(hdc, cur_menu_item);
    wnd_creat = FALSE;

    EndPaint(hWnd, hdc);
#ifdef DEBUG
    printf("}}}draw_menu_wnd\n");
#endif
}


static void cur_item_sub(void)
{
	switch(cur_menu_item) 
	{
		case KEY_SOUND:            
			sub_key_sound();
			break;

		case LIGHT_TIME:            
			sub_bklight();
			break;
    
		case OFF_TIME:           
			sub_shutdown_time();
			break;

		case LANGUAGE:           
			sub_language();
			break;

//		case BKGND:          
//			sub_bkcolor();
//			break;
		default:
			break;
	}
}


static void cur_item_add(void)
{
	switch(cur_menu_item) 
	{
		case KEY_SOUND:            
			add_key_sound();
			break;

		case LIGHT_TIME:            
			add_bklight();
			break;

		case OFF_TIME:           
			add_shutdown_time();
			break;

		case LANGUAGE:           
			add_language();
			break;

//		case BKGND:          
//			add_bkcolor();
//			break;
		default:
			break;
	}
}

/*
void erase_bkgnd(HWND hwnd, WPARAM wparam, LPARAM lparam)
{
#ifdef DEBUG
	printf("erase_bkgnd{{{\n");
#endif
	HDC hdc = (HDC)wparam;
	BOOL f_get_dc = FALSE;
	RECT* clip = (RECT*)lparam;
	RECT rect;
	if (hdc == 0)
	{
		hdc = GetClientDC(hwnd);
		f_get_dc = TRUE;
	}

	if (clip != NULL)
	{
		rect = *clip;
		ScreenToWindow(hwnd, &rect.left, &rect.top);
		ScreenToWindow(hwnd, &rect.right, &rect.bottom);
		IncludeClipRect(hdc, &rect);
	}


	BitBlt(mem_back, 0, 0, MENU_WND_W, MENU_WND_H, hdc, 0, 0, 0);
	
	if (f_get_dc)
	{
		ReleaseDC(hdc);
	}
#ifdef DEBUG
	printf("}}}erase_bkgnd\n");
#endif
}
*/

static char _setup_sysinfo[BUFFER_SIZE];

static int setup_proc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
        case MSG_NCCREATE:
            SetWindowAdditionalData2 (hWnd, 0);
            break;

		case MSG_CREATE:
			return 0;        
			break;


        case MSG_SETFOCUS:
            wnd_creat = TRUE;
            pmp_set_captionbar_title(_(IDS_PMP_SETUP_TITLE)/*"setup"*/);
            break;

		case MSG_KEYDOWN:
			switch (wParam) 
			{
				case SCANCODE_CURSORBLOCKUP:
					if(cur_menu_item>ITEM_FIRST)
					{
						old_menu_item = cur_menu_item;
						cur_menu_item--;
						
						make_text_rect(old_menu_item);
						InvalidateRect(hWnd, &item_rect, TRUE);
						make_text_rect(cur_menu_item);
						InvalidateRect(hWnd, &item_rect, TRUE);
					}
					else
					{
						return 0;
					}
					break;

				case SCANCODE_CURSORBLOCKDOWN:
					if(cur_menu_item<ITEM_LAST)
					{
						old_menu_item = cur_menu_item;
						cur_menu_item++;
						
						make_text_rect(old_menu_item);
						InvalidateRect(hWnd, &item_rect, TRUE);
						make_text_rect(cur_menu_item);
						InvalidateRect(hWnd, &item_rect, TRUE);
					}
					else
					{
						return 0;
					}
					break;

				case SCANCODE_CURSORBLOCKLEFT: 
					{
						cur_item_sub();
						InvalidateRect(hWnd, &item_rect, TRUE);
						break;
					}
				case SCANCODE_CURSORBLOCKRIGHT: 
					{
						cur_item_add();
						InvalidateRect(hWnd, &item_rect, TRUE);
						break;
					}
					break;

				case SCANCODE_ENTER:
#ifdef DEBUG
					printf("id = %d\n", cur_menu_item);
#endif
                    switch (cur_menu_item)
                    {
                        case LANGUAGE:
                            if (get_language() == LANG_ZH_CN)
                                pmp_select_text_domain("zh_cn");
                            else
                                pmp_select_text_domain("en");
                            
                            InvalidateRect(hWnd, NULL, TRUE);
                            break;

                        case TIME_DATE:
                            timedate_setup(hWnd);
                            break;

                        case SYS_INFO:
                            out_sysinfo(_setup_sysinfo, BUFFER_SIZE);
                            pmp_show_info(hWnd, _(IDS_PMP_SETUP_SYSINFO)/*"system info"*/, _setup_sysinfo, MB_ICONINFORMATION);
                            wnd_creat = TRUE;
                        //    InvalidateRect(hWnd, NULL, FALSE);
                            break;

                        default:
                            break;
                    }
					return 0;

				case SCANCODE_ESCAPE:
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
					break;
			}
			/*display the current setup*/
			break;        

		case MSG_PAINT:
#ifdef DEBUG
			printf("get MSG_PAINT\n");
#endif
			draw_menu_wnd(hWnd);
			break;

		case MSG_CLOSE:
			DestroyAllControls (hWnd); 
			DestroyMainWindow (hWnd);
			MainWindowCleanup (hWnd);        
			wnd_creat = TRUE;
            pmp_app_notify_closed(PMP_APP_SETUP);
			
			return 0;
	}

    return pmp_def_wnd_proc(hWnd, message, wParam, lParam);
}

static MAINWINCREATE CreateInfo;
void ex_setup_show_window(HWND hwnd)
{

   	CreateInfo.dwStyle = WS_VISIBLE;
    CreateInfo.spCaption = "";
    CreateInfo.dwExStyle = WS_EX_NONE ;
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = setup_proc;
    CreateInfo.lx = 0; 
    CreateInfo.ty = 20; 
    CreateInfo.rx = 320;
    CreateInfo.by = 240;
    CreateInfo.iBkColor = PIXEL_lightgray;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = hwnd;

    hMainWnd = CreateMainWindow (&CreateInfo);
	if (hMainWnd == HWND_INVALID)
	{
		return ;
	}
	ShowWindow(hMainWnd, SW_SHOWNORMAL);
    pmp_app_notify_opened(PMP_APP_SETUP, hMainWnd);


	return ;
}



