/*
** $Id$
**
** tip.c: Tip window for Feynman PMP solution.
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

#include <pmp.h>

#define PMP_TIP_TIMER_ID       100
#define PMP_TIP_TIMEOUT     300     /* 3s */

#define TIP_BK_NAME       "/res/desktop/tip.gif"

static HWND hTipWnd = HWND_INVALID;

static int TipProc (HWND hWnd, int message, 
                WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    
    switch (message)
    {
    case MSG_CREATE:
        SetTimer (hWnd, PMP_TIP_TIMER_ID, PMP_TIP_TIMEOUT);
        return 0;        

    case MSG_PAINT:        
        {
            RECT rc;
            rc.left = 1;
            rc.right = 102;
            rc.top = 1;
            rc.bottom = 44;
            hdc = BeginPaint(hWnd);
            SetBkMode (hdc, BM_TRANSPARENT);
            DrawText(hdc, GetWindowCaption(hWnd), -1, &rc, DT_VCENTER|DT_CENTER|DT_SINGLELINE);
            EndPaint (hWnd, hdc);
        }
        break;

    case MSG_ERASEBKGND:
        {
            const RECT* clip = (const RECT*) lParam;
            BOOL fGetDC = FALSE;
            RECT rcTemp;
#ifdef BITMAPBK
            BITMAP* bmp_bk;
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
            bmp_bk = GetWindowAdditionalData(hWnd);
            FillBoxWithBitmap(hdc, 0, 0, 0, 0, bmp_bk);
#else

            data =  pmp_image_res_acquire_ds (TIP_BK_NAME);
            PaintImageEx (hdc, 0, 0, data, 
                    pmp_get_file_suffix (TIP_BK_NAME));
            pmp_image_res_release_ds (TIP_BK_NAME, data);
#endif

            if (fGetDC)
                ReleaseDC (hdc);
            return 0;
        }        
        
    case MSG_TIMER:
        KillTimer (hWnd, PMP_TIP_TIMER_ID);
        ShowWindow (hTipWnd, SW_HIDE);
        break;

    case MSG_DESTROY:
        DestroyAllControls (hWnd);
        hTipWnd = HWND_INVALID;            
        return 0;
            
    case MSG_CLOSE:
#ifdef BITMAPBK
        pmp_image_res_release_bitmap(TIP_BK_NAME, 
                        (BITMAP*)GetWindowAdditionalData(hWnd));
#endif
        DestroyMainWindow (hWnd);
        MainWindowCleanup (hWnd);        
        return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

static void InitCreateInfo (PMAINWINCREATE pCreateInfo)
{
    pCreateInfo->dwStyle = WS_NONE;
    pCreateInfo->dwExStyle = WS_EX_TOOLWINDOW;
    pCreateInfo->spCaption = "Tooltip";
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor(0);
    pCreateInfo->hIcon = 0;
    pCreateInfo->MainWindowProc = TipProc;
    pCreateInfo->lx = 33;
    pCreateInfo->ty = 61;
    pCreateInfo->rx = 158;
    pCreateInfo->by = 124;
    pCreateInfo->iBkColor = RGB2Pixel(HDC_SCREEN, 0xFF, 0xFF, 0xE7);
    pCreateInfo->dwAddData = 0;
    pCreateInfo->hHosting = HWND_DESKTOP;
}

void pmp_create_tip (HWND hosting)
{
    MAINWINCREATE CreateInfo;
    
#ifdef BITMAPBK
    BITMAP* bmp_tip = pmp_image_res_acquire_bitmap(TIP_BK_NAME);
#endif
    
    
    
    if (hTipWnd == HWND_INVALID) {
        InitCreateInfo (&CreateInfo);    
        CreateInfo.hHosting = hosting;
#ifdef BITMAPBK
        CreateInfo.dwAddData = bmp_tip;
#endif
        hTipWnd = CreateMainWindow (&CreateInfo);        
    }
}

void pmp_destroy_tip (void)
{
    if (hTipWnd != HWND_INVALID)
        SendNotifyMessage (hTipWnd, MSG_CLOSE, 0, 0);
}

/*
 * x: The new x coordiate of the left-top corner of the tip window.
 * y: The new y coordiate of the left-top corner of the tip window.
 * text: the pointer to the new string which will be displayed on the tip window 
 */
void pmp_update_tip (int x, int y, const char *text)
{
    if (hTipWnd == HWND_INVALID)
        return;

    SetWindowCaption (hTipWnd, text);
    KillTimer (hTipWnd, PMP_TIP_TIMER_ID);
    SetTimer (hTipWnd, PMP_TIP_TIMER_ID, PMP_TIP_TIMEOUT);
    if (IsWindowVisible (hTipWnd))
    {
        ShowWindow (hTipWnd, SW_HIDE);
        ShowWindow (hTipWnd, SW_SHOWNORMAL);
    }
    else
    {
        ShowWindow (hTipWnd, SW_SHOWNORMAL);
    }

    return;    
}

void pmp_show_tip (BOOL show)
{
    if (hTipWnd == HWND_INVALID)
        return;

    if (show)
        ShowWindow (hTipWnd, SW_SHOWNORMAL);
    else
        ShowWindow (hTipWnd, SW_HIDE);
}

