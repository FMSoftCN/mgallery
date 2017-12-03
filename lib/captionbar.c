/*
** $Id$
**
** captionbar.c: Captionbar window for Feynman PMP solution.
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

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "pmp.h"

#define PMP_CAPTBAR_WIDTH             320
#define PMP_CAPTBAR_HEIGHT            20
#define PMP_CAPTBAR_TITLE_LEFT_X      80
#define PMP_CAPTBAR_TITLE_LEFT_Y      0
#define PMP_CAPTBAR_TITLE_RIGHT_X     200
#define PMP_CAPTBAR_TITLE_RIGHT_Y     16
#define PMP_CAPTBAR_TIME_WIDTH        52

#define PMP_LOGO_LEFTX                0    
#define PMP_LOGO_LEFTY                3
#define PMP_POWER_LEFTX               237
#define PMP_POWER_LEFTY               3

#define _ID_CAPTBAR_TIMER              500

static RECT title_rect = {
    PMP_CAPTBAR_TITLE_LEFT_X,
    PMP_CAPTBAR_TITLE_LEFT_Y,
    PMP_CAPTBAR_TITLE_RIGHT_X,
    PMP_CAPTBAR_TITLE_RIGHT_Y
};

static HWND hCaptionbarWnd = HWND_INVALID;
static int time_min = 0;

static struct tm *get_cur_time ()
{
    time_t t;

    time (&t);
    return (localtime (&t));
}

static void mk_time (char* buff)
{
    struct tm * tm;
        
    tm = get_cur_time();
    sprintf (buff, "%02d:%02d", tm->tm_hour, tm->tm_min);
    
    return ;
}

static int CaptionbarProc (HWND hWnd, int message, 
            WPARAM wParam, LPARAM lParam)
{
    HDC     hdc;
    
    switch (message) {
        case MSG_CREATE:
            SetTimer (hWnd, _ID_CAPTBAR_TIMER, 100);
            return 0;        

        case MSG_PAINT:
        {
            RECT rc_client;
            char time[10];

            hdc = BeginPaint (hWnd);

            SetBkMode (hdc, BM_TRANSPARENT);
            SetTextColor (hdc, RGB2Pixel(hdc, 255, 255, 255));
            DrawText (hdc, GetWindowCaption (hWnd), -1, 
                    &title_rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

            mk_time (time);
            GetClientRect (hWnd, &rc_client);
            rc_client.bottom -= 4;
            DrawText (hdc, time, -1, &rc_client,
                      DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
            
            EndPaint (hWnd, hdc);        
            break;
        }

        case MSG_TIMER:
        {
            RECT rc;
            struct tm * tm = get_cur_time ();

            if(tm->tm_min != time_min) {
                time_min = tm->tm_min;
                rc.left =  MAINWINDOW_W - PMP_CAPTBAR_TIME_WIDTH;
                rc.right = PMP_CAPTBAR_WIDTH;
                rc.top = 0;
                rc.bottom = PMP_CAPTBAR_HEIGHT;
                InvalidateRect (hWnd, &rc, TRUE);
            }

            break;
        }
            
        case MSG_ERASEBKGND:
        {
            const RECT* clip = (const RECT*) lParam;
            BOOL fGetDC = FALSE;
            RECT rcTemp;
            MG_RWops *data = NULL;
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

#define CAPBAR_BK_NAME       "/res/captionbar/bk.gif"
#define CAPBAR_LOGO_NAME     "/res/captionbar/logo.png"
#define CAPBAR_POWER_NAME    "/res/captionbar/power.png"            
            
            data =  pmp_image_res_acquire_ds (CAPBAR_BK_NAME);
            PaintImageEx (hdc, 0, 0, data, 
                    pmp_get_file_suffix (CAPBAR_BK_NAME));
            pmp_image_res_release_ds (CAPBAR_BK_NAME, data);

            data =  pmp_image_res_acquire_ds (CAPBAR_LOGO_NAME);
            PaintImageEx (hdc, PMP_LOGO_LEFTX, PMP_LOGO_LEFTY, data, 
                        pmp_get_file_suffix (CAPBAR_LOGO_NAME));
            pmp_image_res_release_ds (CAPBAR_LOGO_NAME, data);

            data =  pmp_image_res_acquire_ds (CAPBAR_POWER_NAME);
            PaintImageEx (hdc, PMP_POWER_LEFTX, PMP_POWER_LEFTY, data, 
                        pmp_get_file_suffix (CAPBAR_POWER_NAME));
            pmp_image_res_release_ds (CAPBAR_POWER_NAME, data);
            
            if (fGetDC)
                ReleaseDC (hdc);
            return 0;
        }                
        
    case MSG_DESTROY:
        KillTimer (hWnd, _ID_CAPTBAR_TIMER);
        DestroyAllControls (hWnd);
        hCaptionbarWnd = HWND_INVALID;
        return 0;
            
    case MSG_CLOSE:
        DestroyMainWindow (hWnd);
        MainWindowCleanup (hWnd);
        return 0;
    }

    return  DefaultMainWinProc(hWnd, message, wParam, lParam);
}

/*
 * show_hide: The flag to show or hide caption bar.
 *   - TRUE: show caption bar.
 *   - FALSE: hide caption bar.
 */
void pmp_show_captionbar (BOOL show_hide)  
{
    if (show_hide) 
        ShowWindow(hCaptionbarWnd, SW_SHOWNORMAL);
    else 
        ShowWindow(hCaptionbarWnd, SW_HIDE);
    
}

static void InitCreateInfo (PMAINWINCREATE pCreateInfo)
{
    pCreateInfo->dwStyle = WS_VISIBLE;
    pCreateInfo->dwExStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
    pCreateInfo->spCaption = "";
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor(0);
    pCreateInfo->hIcon = 0;
    pCreateInfo->MainWindowProc = CaptionbarProc;
    pCreateInfo->lx = 0;
    pCreateInfo->ty = 0;
    pCreateInfo->rx = PMP_CAPTBAR_WIDTH;
    pCreateInfo->by = PMP_CAPTBAR_HEIGHT;
    pCreateInfo->iBkColor = COLOR_lightgray;
    pCreateInfo->dwAddData = 0;
    pCreateInfo->hHosting = HWND_DESKTOP;
}

void pmp_create_captionbar (HWND hosting)
{
    MAINWINCREATE CreateInfo;

    if (hCaptionbarWnd == HWND_INVALID) {
        InitCreateInfo (&CreateInfo);    
        CreateInfo.hHosting = hosting;
        hCaptionbarWnd = CreateMainWindow (&CreateInfo);        
    }    
}

void pmp_destroy_captionbar (void)
{
    if (hCaptionbarWnd != HWND_INVALID)
        SendNotifyMessage (hCaptionbarWnd, MSG_CLOSE, 0, 0);
}

/*
 * title: the pointer to the new title text.
 */
void pmp_set_captionbar_title (const char *title)
{
    SetWindowCaption (hCaptionbarWnd, title);
    InvalidateRect (hCaptionbarWnd, &title_rect, TRUE);
}

/*
 * Return the pointer to current title text.
 */
const char *pmp_get_captionbar_title (void)
{
    if (hCaptionbarWnd != HWND_INVALID)
        return GetWindowCaption (hCaptionbarWnd);

    return NULL;
}

