/*
** $Id$
**
** main.c: Desktop window for Feynman PMP solution.
**
** Copyright (C) 2007 Feynman Software, all rights reserved.
**
** Use of this source package is subject to specific license terms
** from Beijing Feynman Software Technology Co., Ltd.
**
** URL: http://www.minigui.com
**
** Current maintainer: liupeng.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/mywindows.h>

#include "pmp.h"
#include "text_id.h"

#define ICON_TOP_MARGIN  6
#define ICON_SPACE       12
#define ICON_LEFT        182
#define ICON_WIDTH       32
#define ICON_HEIGHT      32

#define DESKTOP_BK_NAME       "/res/desktop/bk3.bmp"
#define DESKTOP_SELREC_NAME   "/res/desktop/selected_rec.bmp"              

static char *pmp_desktop_icon_names [] = {
    "/res/desktop/music.png",
    "/res/desktop/video.png",
    "/res/desktop/picture.png",
    "/res/desktop/recorder.png",
    "/res/desktop/radio.png",
    "/res/desktop/ebook.png",
    "/res/desktop/calendar.png",
    "/res/desktop/calc.png",
    "/res/desktop/stopwatch.png",
    "/res/desktop/setup.png",
    "/res/desktop/russia.png",
    "/res/desktop/worm.png",
    "/res/desktop/housekeeper.png"  
};


static int pmp_desktop_icon_tip_id [] = {
    IDS_PMP_TIP_MUSIC,          
    IDS_PMP_TIP_VIDEO,          
    IDS_PMP_TIP_PICTURE,        
    IDS_PMP_TIP_RECORDER,       
    IDS_PMP_TIP_RADIO,          
    IDS_PMP_TIP_EBOOK,          
    IDS_PMP_TIP_CALENDAR,       
    IDS_PMP_TIP_CALC,           
    IDS_PMP_TIP_STOPWATCH,      
    IDS_PMP_TIP_SETUP,          
    IDS_PMP_TIP_RUSSIA,         
    IDS_PMP_TIP_WORM,           
    IDS_PMP_TIP_HOUSEKEEPER        
};


static BITMAP *pmp_desktop_icons [TABLESIZE(pmp_desktop_icon_names)];

static int top_icon_id = 0;

static int pmp_hook (void* context, HWND dst_wnd, int message, WPARAM wParam, LPARAM lParam);

static void update_icons (HWND hWnd)
{
    RECT rc;

    rc.left = ICON_LEFT;
    rc.right = rc.left + ICON_WIDTH;
    rc.top = ICON_TOP_MARGIN;
    rc.bottom = rc.top + (ICON_HEIGHT+ICON_SPACE)*5;;
    InvalidateRect (hWnd, &rc, TRUE);       

    return;
}

/*if flag=TRUE, load icons; if FALSE, unload icons */
static int load_icons (BOOL flag)
{
    int i;

    if (flag) 
        for (i = 0; i < TABLESIZE(pmp_desktop_icon_names); i++) 
            pmp_desktop_icons[i] = pmp_image_res_acquire_bitmap (pmp_desktop_icon_names[i]);
    else 
         for (i = 0; i < TABLESIZE(pmp_desktop_icon_names); i++)
             pmp_image_res_release_bitmap (pmp_desktop_icon_names [i], pmp_desktop_icons[i]);    
    
    return 0;
}

static HWND app_hwnds [PMP_APP_NUMBER];

static int pmp_hook (void* context, HWND dst_wnd, int message, WPARAM wParam, LPARAM lParam)
{
    if (message == MSG_KEYDOWN && wParam == SCANCODE_F2) {
        ShowWindow (app_hwnds [0],  SW_SHOWNORMAL);
    }

    if (message == MSG_KEYLONGPRESS && wParam == SCANCODE_ESCAPE)
        ExitGUISafely(0);

    return HOOK_GOON;
}

void pmp_app_notify_opened (int app_idx, HWND hwnd)
{
    if (app_idx <= 0 || app_idx >= PMP_APP_NUMBER)
        return; /* do nothing */

    app_hwnds [app_idx] = hwnd;
}

void pmp_app_notify_closed (int app_idx)
{
    if (app_idx <= 0 || app_idx >= PMP_APP_NUMBER)
        return; /* do nothing */

    ShowWindow(GetHosting(app_hwnds [app_idx]), SW_SHOWNORMAL);
    app_hwnds [app_idx] = 0;
}

typedef void (*PMP_APP_ENTRY) (HWND hwnd);

void ex_music_play_music (HWND hosting);
void ex_video_show_window (HWND hosting);
void ex_recorder_show_window (HWND hosting);
void ex_picture_show (HWND hosting);
void ex_radio_show (HWND hosting);
void ex_ebook_display_book (HWND hosting);
void ex_calendar_show_window (HWND hosting);
void ex_calc_show_window (HWND hosting);
void ex_stopwatch_show_window (HWND hosting);
void ex_setup_show_window (HWND hosting);
void ex_housekeeper_show_window (HWND hosting);
void ex_worm_show_window (HWND hosting);
void ex_russia_show_window (HWND hosting);

static PMP_APP_ENTRY app_entries [] = {
    NULL,
    ex_music_play_music,
    ex_video_show_window,
    ex_picture_show,
    ex_recorder_show_window,
    ex_radio_show,
    ex_ebook_display_book,
    ex_calendar_show_window,
    ex_calc_show_window,
    ex_stopwatch_show_window,
    ex_setup_show_window,
    ex_russia_show_window,
    ex_worm_show_window,
    ex_housekeeper_show_window,
};

#define MUTUAL_APP_NR    3
static int mut_id[MUTUAL_APP_NR] = {PMP_APP_MUSIC,
                                    PMP_APP_VIDEO,
                                    PMP_APP_RADIO};

/* Only one app among music, video, radio exist in system. */
static int is_igore (int app_idx)
{
    int i = 0;
    
    for (i = 0; i < MUTUAL_APP_NR; i++)
        if(app_idx == mut_id[i])
            break;

    if (i >= MUTUAL_APP_NR)
        return 0;

    for (i = 0; i < MUTUAL_APP_NR; i++ ) {
        if(app_hwnds [mut_id[i]])
            break;
    }
    
    if (i >= MUTUAL_APP_NR)
        return 0;
    else
        return 1; 
}

void start_app (int app_idx)
{
    if (app_idx <= 0 || app_idx >= PMP_APP_NUMBER)
        return; /* do nothing */

    if (app_hwnds [app_idx]) {
        HWND hosted;

        ShowWindow (app_hwnds [app_idx], SW_SHOWNORMAL);

        hosted = GetFirstHosted (app_hwnds [app_idx]);
        while (hosted) {
            ShowWindow (hosted, SW_SHOWNORMAL);
            hosted = GetNextHosted (app_hwnds [app_idx], hosted);
        }
    }
    else {
        if (!is_igore(app_idx) && app_entries [app_idx])
            app_entries [app_idx] (app_hwnds [0]);
    }
}

static int DesktopProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    
    switch (message) {
        
        case MSG_CREATE: {
            RegisterKeyMsgHook ((void*)hWnd, pmp_hook);
            load_icons (TRUE);
            break;
        }
        case MSG_PAINT: {
            int i;

            hdc = BeginPaint (hWnd);
            for (i=0; i<5; i++) {
                if ((i + top_icon_id)<0)
                    continue;
                
                FillBoxWithBitmap(hdc,
                                  ICON_LEFT,
                                  ICON_TOP_MARGIN + (ICON_HEIGHT+ICON_SPACE)*i,
                                  ICON_WIDTH,
                                  ICON_HEIGHT,
                                  pmp_desktop_icons[i + top_icon_id]);
            }

            EndPaint (hWnd, hdc);
            return 0;
        }
    case MSG_ERASEBKGND:
        {
            const RECT* clip = (const RECT*) lParam;
            BOOL fGetDC = FALSE;
            RECT rcTemp;
#ifdef BITMAPBK 
            BITMAP** bmp_bg = (BITMAP**)GetWindowAdditionalData(hWnd);
#else
            MG_RWops *data = NULL;
#endif

            hdc = (HDC)wParam;
            if (hdc == 0) {
                hdc = GetClientDC (hWnd);
                fGetDC = TRUE;
            }

            if (clip) {
                rcTemp = *clip;
                ScreenToClient (hWnd, &rcTemp.left, &rcTemp.top);
                ScreenToClient (hWnd, &rcTemp.right, &rcTemp.bottom);
                IncludeClipRect (hdc, &rcTemp);
            }

#ifdef BITMAPBK
            FillBoxWithBitmap(hdc, 0, 0, 0, 0, *bmp_bg);
            FillBoxWithBitmap(hdc, 168, 91, 0, 0, *(bmp_bg+1));
#else
            data =  pmp_image_res_acquire_ds (DESKTOP_BK_NAME);
            PaintImageEx (hdc, 0, 0, data, pmp_get_file_suffix (DESKTOP_BK_NAME));
            pmp_image_res_release_ds (DESKTOP_BK_NAME, data);      

            data =  pmp_image_res_acquire_ds (DESKTOP_SELREC_NAME);
            PaintImageEx (hdc, 168, 91, data, pmp_get_file_suffix (DESKTOP_SELREC_NAME));
            pmp_image_res_release_ds (DESKTOP_SELREC_NAME, data);
#endif

            if (fGetDC)
                ReleaseDC (hdc);
            return 0;
        }

    case MSG_KEYDOWN:
        switch (wParam) {
            case SCANCODE_ENTER:
                start_app (top_icon_id + 3);
                break;

            case SCANCODE_CURSORBLOCKUP:
                if ((top_icon_id) > -2) {
                    top_icon_id--;
                    pmp_update_tip (0, 0, _(pmp_desktop_icon_tip_id[top_icon_id+2]));
                    update_icons (hWnd);
                }
                break;
                
            case SCANCODE_CURSORBLOCKDOWN:
                if (top_icon_id < 10) {
                    top_icon_id++;
                    pmp_update_tip (0, 0, _(pmp_desktop_icon_tip_id[top_icon_id+2]));
                    update_icons (hWnd);
                }
                break;                    
        }
        return 0;

        case MSG_SETFOCUS:
            if (GetActiveWindow () == hWnd) {
                pmp_show_captionbar (TRUE);
                pmp_set_captionbar_title (_(IDS_PMP_DESKTOP_CAPTION));
            }
            break;

        case MSG_KILLFOCUS:
            pmp_show_tip (FALSE);
            break;

        case MSG_CLOSE:
            load_icons (FALSE);
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
            return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int MiniGUIMain (int argc, const char* argv[])
{
    MSG Msg;
    MAINWINCREATE CreateInfo;

#ifdef BITMAPBK 
    BITMAP* bmp_bg[2];
    char *bg_img_file = "/res/public/bk3.bmp";
    BITMAP* def_bmpbg = NULL;
#endif

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , "PMP" , 0 , 0);
#endif

#ifdef RES_TOP_DIR
    if (pmp_set_res_top_dir (RES_TOP_DIR)) {
        _MY_PRINTF ("pmp_set_res_top_dir failed.\n");
        return 1;
    }
#else
    {
        char path [MAX_PATH + 1];
        getcwd (path, MAX_PATH);
        strcat (path, "/..");
        if (pmp_set_res_top_dir (path)) {
            _MY_PRINTF ("pmp_set_res_top_dir failed.\n");
            return 1;
        }
    }
#endif

#ifdef BITMAPBK
    bmp_bg[0] = pmp_image_res_acquire_bitmap(DESKTOP_BK_NAME);
    bmp_bg[1] = pmp_image_res_acquire_bitmap(DESKTOP_SELREC_NAME);
    def_bmpbg = pmp_image_res_acquire_bitmap(bg_img_file);
    pmp_set_def_bmpbg(def_bmpbg);
#endif

    SetKeyLongPressTime(100);
    pmp_select_text_domain ("en");
    
    CreateInfo.dwStyle = WS_VISIBLE ;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "FMSoft's PMP Solution";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = DesktopProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 20;
    CreateInfo.rx = MAINWINDOW_W;
    CreateInfo.by = MAINWINDOW_H;
    CreateInfo.iBkColor = COLOR_lightwhite;

#ifdef BITMAPBK
    CreateInfo.dwAddData = (DWORD)bmp_bg;
#else
    CreateInfo.dwAddData = 0;
#endif
    CreateInfo.hHosting = HWND_DESKTOP;
    
    app_hwnds [0] = CreateMainWindow (&CreateInfo);
    
    if (app_hwnds [0] == HWND_INVALID)
        return -1;

    pmp_create_captionbar (app_hwnds [0]);
    pmp_create_tip (app_hwnds [0]);
    pmp_show_captionbar (TRUE);
    ShowWindow(app_hwnds [0], SW_SHOWNORMAL);

    while (GetMessage(&Msg, app_hwnds [0])) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    pmp_destroy_tip ();
    pmp_destroy_captionbar ();
#ifdef BITMAPBK
    pmp_image_res_release_bitmap(DESKTOP_BK_NAME, bmp_bg[0]);
    pmp_image_res_release_bitmap(DESKTOP_SELREC_NAME, bmp_bg[1]);
    pmp_image_res_release_bitmap(bg_img_file, def_bmpbg);
#endif
    MainWindowThreadCleanup (app_hwnds [0]);



    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

