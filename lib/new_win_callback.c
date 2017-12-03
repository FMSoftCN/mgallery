/*
** $Id$
**
** new_win_callback.c: New window callback fun for PMP apps.
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

#include "pmp.h"

#ifdef BITMAPBK
static BITMAP* def_bmpbg = NULL;

void pmp_set_def_bmpbg(BITMAP* bmpbg)
{
    def_bmpbg = bmpbg;
}

void pmp_draw_background_withbmp (HWND hWnd, HDC hdc, const RECT* clip, 
        const BITMAP* bmp_bg)
{
    BOOL fGetDC = FALSE;
    RECT rcTemp;

    if (bmp_bg == NULL)
        bmp_bg = def_bmpbg;

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

    FillBoxWithBitmap(hdc, 0, 0, 0, 0, bmp_bg);
    
    if (fGetDC)
        ReleaseDC (hdc);
}

#else
void pmp_draw_background (HWND hWnd, HDC hdc, const RECT* clip, 
        const char* bg_img_file)
{
    BOOL fGetDC = FALSE;
    RECT rcTemp;
    MG_RWops *data = NULL;

    if (bg_img_file == NULL)
        bg_img_file = "/res/public/bk3.bmp";

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
    
    data =  pmp_image_res_acquire_ds (bg_img_file);
    PaintImageEx (hdc, 0, 0, data, pmp_get_file_suffix (bg_img_file));
    pmp_image_res_release_ds (bg_img_file, data);

    if (fGetDC)
        ReleaseDC (hdc);
}
#endif

int pmp_def_wnd_proc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    if (message == MSG_ERASEBKGND) {
#ifdef BITMAPBK
            pmp_draw_background_withbmp (hWnd, (HDC)wParam, (const RECT*)lParam, 
                    (const BITMAP*) GetWindowAdditionalData2 (hWnd));

#else
            pmp_draw_background (hWnd, (HDC)wParam, (const RECT*)lParam, 
                    (const char*) GetWindowAdditionalData2 (hWnd));
#endif
            return 0;
    }

    return DefaultMainWinProc (hWnd, message, wParam, lParam);
}
