/*
 ** $Id$
 **
 ** calendar.c: Calendar module for Feynman PMP solution.
 **
 ** Copyright (C) 2007 Feynman Software, all rights reserved.
 **
 ** Use of this source package is subject to specific license terms
 ** from Beijing Feynman Software Technology Co., Ltd.
 **
 ** URL: http://www.minigui.com
 **
 ** Current maintainer: Zhang Hua .
 */

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "pmp.h"
#include "text_id.h"
#include <minigui/control.h>

#include "monthcal.h"
#include "monthcalendar_impl.h"

#define IDC_CALENDAR     200
#define BK_CALENDAR      "/res/calendar/calendar.bmp"
static HWND calendar_hwnd;

#ifdef BITMAPBK
static BITMAP *backbitmap;
#endif

static RECT rect = {70, 168, 300, 220};

static char *GetDayOf(PSYSTEMTIME pSt)
{ 
    static const char *cTianGan[] = {"¼×", "ÒÒ", "±û", "¶¡", "Îì", "¼º", "¸ý", "ÐÁ", "ÈÉ", "¹ï"};
    static const char *cDiZhi[] = {"×Ó", "³ó", "Òú", "Ã®", "³½", "ËÈ", "Îç", "Î´",
        "Éê", "ÓÏ", "Ðç", "º¥"};
    static const char *cShuXiang[] = {"Êó", "Å£", "»¢", "ÍÃ", "Áú", "Éß", "Âí", "Ñò", "ºï", 
        "¼¦", "¹·", "Öí"};
    static const char *cDayName[] = {"*", "³õÒ»", "³õ¶þ", "³õÈý", "³õËÄ", "³õÎå", 
        "³õÁù", "³õÆß", "³õ°Ë", "³õ¾Å", "³õÊ®", "Ê®Ò»", "Ê®¶þ", "Ê®Èý", 
        "Ê®ËÄ", "Ê®Îå", "Ê®Áù", "Ê®Æß", "Ê®°Ë", "Ê®¾Å", "¶þÊ® ", "Ø¥Ò»",
        "Ø¥¶þ", "Ø¥Èý", "Ø¥ËÄ", "Ø¥Îå", "Ø¥Áù", "Ø¥Æß", "Ø¥°Ë", "Ø¥¾Å", "ÈýÊ®"};
    static const char *cMonName[] = {"*", "Õý", "¶þ", "Èý", "ËÄ", "Îå", "Áù", "Æß", 
        "°Ë", "¾Å", "Ê®", "Ê®Ò»", "À°"};

    static const int wMonthAdd[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    static const int wNongliData[100] = {2635, 333387, 1701, 1748, 267701, 694, 2391,
        133423, 1175, 396438, 3402, 3749, 331177, 1453, 694, 201326, 2350,
        465197, 3221, 3402, 400202, 2901, 1386, 267611, 605, 2349, 137515,
        2709, 464533, 1738, 2901, 330421, 1242, 2651, 199255, 1323, 529706, 
        3733, 1706, 398762, 2741, 1206, 267438, 2647, 1318, 204070, 3477, 
        461653, 1386, 2413, 330077, 1197, 2637, 268877, 3365, 531109, 2900, 
        2922, 398042, 2395, 1179, 267415, 2635, 661067, 1701, 1748, 398772,
        2742, 2391, 330031, 1175, 1611, 200010, 3749, 527717, 1452, 2742, 
        332397, 2350, 3222, 268949, 3402, 3493, 133973, 1386, 464219, 605, 
        2349, 334123, 2709, 2890, 267946, 2773, 592565, 1210, 2651, 395863, 
        1323, 2707, 265877};

    int wCurYear, wCurMonth, wCurDay;
    int nTheDate, nIsEnd, m, k, n, i, nBit;
    static char szNongli[60];
    char szNongliDay[20], szShuXiang[20];
    
    wCurYear = pSt->year;
    wCurMonth = pSt->month;
    wCurDay = pSt->day;
    
    nTheDate = ( wCurYear - 1921 ) * 365 + ( wCurYear - 1921 ) / 4 + wCurDay + wMonthAdd[wCurMonth - 1] - 38;
    if ( ( ! ( wCurYear % 4 ) ) && ( wCurMonth > 2 ) )
        nTheDate = nTheDate + 1;

    nIsEnd = 0;
    m = 0;
    while ( nIsEnd != 1 )
    {
        if ( wNongliData[m] < 4095 )
            k = 11;
        else
            k = 12;
        n = k;
        while ( n >= 0 )
        {
            nBit = wNongliData[m];
            for ( i = 1; i < n + 1; i++ )
                nBit = nBit/2;

            nBit = nBit % 2;

            if ( nTheDate <= ( 29 + nBit ) )
            {
                nIsEnd = 1;
                break;
            }

            nTheDate = nTheDate - 29 - nBit;
            n = n - 1;
        }
        if ( nIsEnd )
            break;
        m = m + 1;
    }
    wCurYear = 1921 + m;
    wCurMonth = k - n + 1;
    wCurDay = nTheDate;
    if ( k == 12 )
    {
        if ( wCurMonth == wNongliData[m] / 65536 + 1 )
            wCurMonth = 1 - wCurMonth;
        else if ( wCurMonth > wNongliData[m] / 65536 + 1 )
            wCurMonth = wCurMonth - 1;
    }

    sprintf ( szShuXiang, "%s", cShuXiang[ ( ( wCurYear - 4 ) % 60 ) % 12 ] );
    sprintf ( szNongli, "%s(%s%s)Äê", szShuXiang, cTianGan [ ( ( wCurYear - 4 ) % 60 ) % 10 ], cDiZhi [ ( ( wCurYear - 4 ) % 60 ) % 12 ] );
    
    if ( wCurMonth < 1 )
        sprintf ( szNongliDay, "Èò%s", cMonName[-1 * wCurMonth] );
    else
        strcpy ( szNongliDay, cMonName[wCurMonth] );

    strcat ( szNongliDay, "ÔÂ" );
    strcat ( szNongliDay, cDayName[wCurDay] );
    strcat ( szNongli, szNongliDay );
    return szNongli;
}

static int CalendarProc ( HWND hWnd, int message, WPARAM wParam, LPARAM lParam )
{
    switch ( message ) {
        case MSG_NCCREATE:

#ifdef BITMAPBK
            backbitmap = pmp_image_res_acquire_bitmap ( BK_CALENDAR ); 
            SetWindowAdditionalData2( hWnd, ( DWORD )backbitmap );
#else
            SetWindowAdditionalData2 ( hWnd, ( DWORD ) BK_CALENDAR );
#endif

            break;

        case MSG_CREATE:
            {
                MCCOLORINFO color;

                calendar_hwnd = CreateWindow ( "monthcalendar", "PLAY", 
                        WS_CHILD | WS_VISIBLE, 
                        IDC_CALENDAR, 0, 0, 320, 150, hWnd, 0 );
                
                /*COLOR_green;*/
                color.clr_titlebk = RGB2Pixel ( HDC_SCREEN, 71, 156, 210 );
                color.clr_daybk = RGB2Pixel ( HDC_SCREEN, 71, 156, 210 );
                SetFocusChild ( calendar_hwnd );
                break;
            }

        case MSG_SETFOCUS:
            pmp_set_captionbar_title ( _(IDS_PMP_CALENDAR_TITLE) );
            break;

        case MSG_CLOSE:

#ifdef BITMAPBK
            pmp_image_res_release_bitmap ( BK_CALENDAR, backbitmap);
#endif
            DestroyAllControls ( hWnd );
            DestroyMainWindow ( hWnd );
            MainWindowCleanup ( hWnd );
            MonthCalendarControlCleanup ();
            pmp_app_notify_closed ( PMP_APP_CALENDAR );
            return 0;

        case MSG_COMMAND:
            InvalidateRect ( hWnd, &rect, TRUE );
            break;

        case MSG_PAINT:
            {
                SYSTEMTIME date;
                HDC hdc = BeginPaint ( hWnd );
                SendMessage ( GetDlgItem ( hWnd, IDC_CALENDAR ), MCM_GETCURDATE, 0, ( LPARAM ) &date );
                SetBkMode ( hdc, BM_TRANSPARENT );
                TextOut ( hdc, 76, 168, GetDayOf ( &date ) );
                EndPaint ( hWnd, hdc );
                return 0;
            }  

        case MSG_KEYDOWN:
            if ( wParam == SCANCODE_ESCAPE )
                PostMessage ( hWnd, MSG_CLOSE, 0, 0 );
            break;
    }

    return pmp_def_wnd_proc ( hWnd, message, wParam, lParam );
}

static void _calendar_init_create_info ( PMAINWINCREATE pCreateInfo )
{
    pCreateInfo->dwStyle = WS_BORDER | WS_VISIBLE;
    pCreateInfo->dwExStyle = WS_EX_NONE;
    pCreateInfo->spCaption = _(IDS_PMP_CALENDAR_TITLE);
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor ( 0 );
    pCreateInfo->hIcon = 0;
    pCreateInfo->MainWindowProc = CalendarProc;
    pCreateInfo->lx = 0;
    pCreateInfo->ty = 20;
    pCreateInfo->rx = MAINWINDOW_W;
    pCreateInfo->by = MAINWINDOW_H;
    pCreateInfo->iBkColor = COLOR_lightgray;
    pCreateInfo->dwAddData = 0;
    pCreateInfo->hHosting = HWND_DESKTOP;
}

void ex_calendar_show_window ( HWND parent )
{
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;

    RegisterMonthCalendarControl();
    _calendar_init_create_info ( &CreateInfo );

    CreateInfo.hHosting = parent;
    hMainWnd = CreateMainWindow ( &CreateInfo );

    pmp_app_notify_opened ( PMP_APP_CALENDAR, hMainWnd );
}
