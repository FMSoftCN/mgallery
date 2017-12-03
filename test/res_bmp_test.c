/*
** $Id$
**
** res_cache_test.c: Test case for resource cache interface
** of Feynman PMP solution.
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
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/mywindows.h>

#include "pmp.h"

#include <mcheck.h>

#define IMAGE_VIDEO_NAME    "/res/public/bg.gif"
#define IMAGE_MUSIC_NAME    "/res/desktop/bk.gif"
#define IMAGE_NONINCORE     "/res/recorder/recorder.bmp"

static BITMAP *video_bmp;
static BITMAP *music_bmp;
static BITMAP *bmp_array[5];

static int HelloWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    
    switch (message) {
        
        case MSG_CREATE:
            break;
            
        case MSG_PAINT: {
            hdc = BeginPaint (hWnd);
            TextOut (hdc, 0, 80, "This bitmap is load by pmp_image_res_acquire_bitmap");
            FillBoxWithBitmap(hdc, 0, 100, 0, 0, video_bmp);
            FillBoxWithBitmap(hdc, 0, 200, 0, 0, music_bmp);                        
            EndPaint (hWnd, hdc);
            return 0;
        }
    case MSG_ERASEBKGND:
        {
            const RECT* clip = (const RECT*) lParam;
            BOOL fGetDC = FALSE;
            RECT rcTemp;

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

/*             TextOut (hdc, 0, 0, "This bitmap is load by pmp_image_res_acquire_ds"); */
/*             data =  pmp_image_res_acquire_ds (IMAGE_VIDEO_NAME); */
/*             PaintImageEx (hdc, 0, 20, data, pmp_get_file_suffix (IMAGE_VIDEO_NAME)); */
/*             pmp_image_res_release_ds (IMAGE_VIDEO_NAME, data);  */
            
            if (fGetDC)
                ReleaseDC (hdc);
            return 0;
        }

        case MSG_KEYDOWN:
            switch (wParam) {                
                case SCANCODE_F1:
                    video_bmp = pmp_image_res_acquire_bitmap (IMAGE_VIDEO_NAME);
                    InvalidateRect (hWnd, NULL, TRUE);
                    break;

                case SCANCODE_F2:
                    pmp_image_res_release_bitmap (IMAGE_VIDEO_NAME, video_bmp);
                    video_bmp = NULL;
                    break;

                case SCANCODE_F3:
                    music_bmp = pmp_image_res_acquire_bitmap (IMAGE_MUSIC_NAME);
                    InvalidateRect (hWnd, NULL, TRUE);
                    break;

                case SCANCODE_F4:
                    pmp_image_res_release_bitmap (IMAGE_MUSIC_NAME, music_bmp);
                    music_bmp = NULL;
                    break;

                case SCANCODE_F5: {
                    int i = 0;
                    for (i = 0; i < 5; i++)
                        bmp_array[i] = pmp_image_res_acquire_bitmap (IMAGE_NONINCORE);    
                    break;
                }
                case SCANCODE_F6: {
                    int i = 0;
                    for (i = 0; i < 5; i++) {
                        pmp_image_res_release_bitmap (IMAGE_NONINCORE, bmp_array[i]);
                        bmp_array[i] = NULL;
                    }
                    break;                    
                }

            }
            
            return 0;

        case MSG_CLOSE:
/*             pmp_image_res_release_bitmap (IMAGE_VIDEO_NAME, video_bmp); */
/*             pmp_image_res_release_bitmap (IMAGE_MUSIC_NAME, music_bmp); */
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
            return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int MiniGUIMain (int argc, const char* argv[])
{
    MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;
    char path [MAX_PATH + 1];

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , "desktop" , 0 , 0);
#endif

    getcwd (path, MAX_PATH);
    strcat (path, "/..");
    if (pmp_set_res_top_dir (path)) {
        printf ("pmp_set_res_top_dir failed.\n");
        return 1;
    }

    SetKeyLongPressTime(200);

    CreateInfo.dwStyle = WS_VISIBLE ;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "desktop";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = HelloWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = g_rcScr.right;
    CreateInfo.by = g_rcScr.bottom;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;
    
    mtrace ();
    hMainWnd = CreateMainWindow (&CreateInfo);
    
    if (hMainWnd == HWND_INVALID)
        return 2;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);

    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup (hMainWnd);
    muntrace ();

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

