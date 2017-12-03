/*
** $Id$
**
** win_test.c: Test case for tip window and menu of Feynman PMP solution.
**
** Copyright (C) 2007 Feynman Software, all rights reserved.
**
** Use of this source package is subject to specific license terms
** from Beijing Feynman Software Technology Co., Ltd.
**
** URL: http://www.minigui.com
**
** Current maintainer: liupeng .
*/

#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/mywindows.h>

#include "pmp.h"

typedef struct _MY_PARAMS {
    int len;
    int skip_to;
    int volume;
} MY_PARAMS;

static char *menu_item_text [] = {
    "Fast Forward",
    "skip to \t %d min",
    "volume \t %d",
    "file info"
};

MY_PARAMS my_params = {5, 1, 10};

static int menu_ff_callback(void *context, int id, int op, char *text_buff)
{
    switch(op) {
        
        case PMP_MI_OP_DEF:
            strcpy (text_buff, menu_item_text[0]);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;
    }
        
    return PMP_MI_ST_NONE;
}

static int menu_skipto_callback (void *context, int id, int op, char *text_buff)
{
    MY_PARAMS* my_params = (MY_PARAMS*)context;
    
    switch(op) {
        
        case PMP_MI_OP_DEF:
/*             strcpy (text_buff, "hello"); */
            sprintf(text_buff, menu_item_text[1], my_params->skip_to);
            printf("skip to text = %s\n", text_buff);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;
            
        case PMP_MI_OP_PREV:
            if (my_params->skip_to > 0)
                my_params->skip_to--;
            sprintf(text_buff, menu_item_text[1], my_params->skip_to);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_NEXT:
            if (my_params->skip_to < my_params->len)
                my_params->skip_to++;
            sprintf(text_buff, menu_item_text[1], my_params->skip_to);
            return PMP_MI_ST_UPDATED;
    }

    return PMP_MI_ST_NONE;
}

static int menu_volume_callback (void *context, int id, int op, char *text_buff)
{
    MY_PARAMS *my_params = (MY_PARAMS*)context;

    switch(op) {
        
        case PMP_MI_OP_DEF:
            sprintf(text_buff, menu_item_text[2], my_params->volume);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_ENTER:
            return 2;
            
        case PMP_MI_OP_PREV:
            if (my_params->volume > 0)
                my_params->volume --;
            sprintf(text_buff, menu_item_text[2], my_params->volume);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_NEXT:
            if (my_params->volume < 20)
                my_params->volume ++;
            sprintf(text_buff, menu_item_text[2], my_params->volume);
            return PMP_MI_ST_UPDATED;            
    }

    return PMP_MI_ST_NONE;
}

static int menu_fileinfo_callback (void *context, int id, int op, char *text_buff)
{
    switch(op) {
        case PMP_MI_OP_DEF:
            
            strcpy(text_buff, menu_item_text[3]);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_ENTER:
            return 2;
    }

    return PMP_MI_ST_NONE;
}

static PMP_MENU_ITEM my_menu[] =
{
    {1, menu_ff_callback},
    {2, menu_skipto_callback},
    {3, menu_volume_callback},
    {4, menu_fileinfo_callback},
    {0, NULL}
};


static int HelloWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    int ret = 0;


    switch (message) {
        case MSG_PAINT:
            hdc = BeginPaint (hWnd);

/* 	    TextOut(hdc, 20, 20, "menu1"); */
/* 	    TextOut(hdc, 20, 50, "menu2"); */
            
            EndPaint (hWnd, hdc);
            return 0;


        case MSG_KEYDOWN:
            switch (wParam) {
                
                case SCANCODE_F1:
                    ret = pmp_show_menu (hWnd, "options", my_menu, &my_params);
                    printf("ret = %d\n", my_params.skip_to);
                    break;

                case SCANCODE_F2:
                    pmp_show_info (hWnd, "title", "hello.\n I am a test.\n", MB_ICONHAND);
                    break;

                case SCANCODE_F3:
                    pmp_show_captionbar (TRUE);
                    SetActiveWindow (hWnd);
                    break;

                case SCANCODE_F4:
                    pmp_update_tip (0, 0, "hello");
                    break;                    
            }
            
            return 0;

/*
        case MSG_KEYUP:
            sprintf (msg_text, "The %d key released", wParam);
            InvalidateRect (hWnd, &msg_rc, TRUE);
            return 0;
*/
        case MSG_CLOSE:
            //KillTimer (hWnd, 100);
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
            return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

#ifdef _MISC_MOUSECALIBRATE
static void mouse_calibrate (void)
{
    POINT src_pts [5] = {{5, 10}, {600, 20}, {620, 450}, {20, 470}, {310, 234}};
    POINT dst_pts [5] = {{0, 0}, {639, 0}, {639, 479}, {0, 479}, {320, 240}};

    SetMouseCalibrationParameters (src_pts, dst_pts);
}
#endif /* !_MISC_MOUSECALIBRATE */

int MiniGUIMain (int argc, const char* argv[])
{
    MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , "helloworld" , 0 , 0);
#endif

#ifdef _MISC_MOUSECALIBRATE
    mouse_calibrate ();
#endif
    
    SetKeyLongPressTime(200);

    CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "Hello, world!";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = HelloWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 20;
    CreateInfo.rx = g_rcScr.right;
    CreateInfo.by = g_rcScr.bottom;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;
    
    hMainWnd = CreateMainWindow (&CreateInfo);
    
    if (hMainWnd == HWND_INVALID)
        return -1;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);

    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup (hMainWnd);

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

