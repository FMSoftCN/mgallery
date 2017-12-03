/*
 ** $Id$
 **
 ** stopwatch.c: Stopwatch module for Feynman PMP solution.
 **
 ** Copyright(C) 2007 Feynman Software, all rights reserved.
 **
 ** Use of this source package is subject to specific license terms
 ** from Beijing Feynman Software Technology Co., Ltd.
 **
 ** URL: http://www.minigui.com
 **
 ** Current maintainer: Zhang Hua .
 */

#include "pmp.h"

#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <minigui/control.h>

#include "text_id.h"
#define _ID_TIMER 100

static unsigned long times, start_times;
static int minute, second, msecond;
#ifdef BITMAPBK
static BITMAP *backbitmap;
#endif
static int start_watch = 0;
static int start_reset = 0;

#define X_TIME          120
#define Y_TIME          99
#define WIDTH_DIGIT     10
#define HEIGHT_DIGIT    15
#define BK_STOPWATCH    "/res/stopwatch/stopwatch.png"

static RECT time_rect = { X_TIME, Y_TIME, 
    X_TIME + WIDTH_DIGIT * 8, Y_TIME + HEIGHT_DIGIT };

static RECT number_rect [] ={
    { X_TIME + WIDTH_DIGIT * 6, Y_TIME, 
        X_TIME + WIDTH_DIGIT * 8, Y_TIME + HEIGHT_DIGIT },
    { X_TIME + WIDTH_DIGIT * 3, Y_TIME, 
        X_TIME + WIDTH_DIGIT * 8, Y_TIME + HEIGHT_DIGIT },
    { X_TIME, Y_TIME, 
        X_TIME + WIDTH_DIGIT * 8, Y_TIME + HEIGHT_DIGIT },
};

static char *get_time_text( void )
{
    static char buf[10];
    sprintf( buf, "%02d:%02d:%02d", minute, second, msecond );
    buf [8] = '\0';
    return buf;
}

static int parse_timer( void )
{
    int ret = 0;
    int m, s, ms;
    unsigned long tmp;

    /* Get minute */
    m = times / 60000000L;

    /* Get second */
    tmp = times%60000000L;
    s = tmp/1000000L;

    /* Get millisecond */
    tmp = tmp%1000000L;
    ms = tmp/10000L;

    if( ms != msecond ) {
        msecond = ms;
        ret = 0;
    }

    if( s != second ) {
        second = s;
        ret = 1;
    }

    if( m != minute ) {
        minute = m;
        ret = 2;
    }
    return ret;
}

static int StopWatchProc( HWND hWnd, int message, WPARAM wParam, LPARAM lParam )
{
    struct timeval tv;
    struct timezone tz;

    int i;
    switch( message ) {

        case MSG_NCCREATE:

#ifdef BITMAPBK
            backbitmap = pmp_image_res_acquire_bitmap ( BK_STOPWATCH ); 
            SetWindowAdditionalData2( hWnd, ( DWORD )backbitmap );
#else
            SetWindowAdditionalData2( hWnd, ( DWORD ) BK_STOPWATCH );
#endif
            break;

        case MSG_CREATE:
            times = 0;
            msecond = 0;
            second = 0;
            minute = 0;

            break;

        case MSG_DESTROY:
            DestroyAllControls( hWnd );
            return 0;

        case MSG_SETFOCUS:
            pmp_set_captionbar_title( _(IDS_PMP_STOPWATCH_TITLE) );
            break;

        case MSG_CLOSE:
#ifdef BITMAPBK
            pmp_image_res_release_bitmap ( BK_STOPWATCH , backbitmap);
#endif
            DestroyMainWindow( hWnd );
            MainWindowCleanup( hWnd );
            pmp_app_notify_closed( PMP_APP_STOPWATCH );
            return 0;

        case MSG_PAINT:
            {
                HDC hdc = BeginPaint( hWnd );
                if( minute == 60 )
                {
                    second = 0;
                    msecond = 0;
                    KillTimer( hWnd, _ID_TIMER );
                }
                pmp_display_led_digits( hdc, X_TIME, Y_TIME, get_time_text( ) );
                EndPaint( hWnd, hdc );
                break;
            }

        case MSG_TIMER:
            {
                static long t;
                static long old_times;

                if( t != start_times ) {
                    old_times = times;
                    t = start_times;
                }

                gettimeofday( &tv, &tz );
                times = old_times + ( tv.tv_usec + tv.tv_sec * 1000000L ) - start_times;

                i = parse_timer( );
                InvalidateRect( hWnd, &number_rect[i], FALSE );
                break;
            }

        case MSG_KEYDOWN:
            switch( wParam ) {
                case SCANCODE_ENTER:
                    /*if the enter keydown and the param is 0 , then the watch will start*/
                    start_watch = 1;
                    if( start_reset == 0 )
                    {
                        SetTimer( hWnd, _ID_TIMER, 1 );
                        gettimeofday( &tv, &tz );
                        start_times = tv.tv_usec + tv.tv_sec * 1000000L;
                        start_reset = 1;
                    }
                    /*Puash the watch*/
                    else
                    {
                        KillTimer( hWnd, _ID_TIMER );
                        start_reset = 0;
                    }
                    break;
                    /*Esc the watch*/
                case SCANCODE_ESCAPE:
                    KillTimer( hWnd, _ID_TIMER );
                    times = 0;
                    second = 0;
                    minute = 0;
                    start_reset = 0;
                    start_watch = 0;
                    PostMessage( hWnd, MSG_CLOSE, 0, 0 );
                    break;
                    /*Restart the watch*/
                case SCANCODE_CURSORBLOCKRIGHT:
                    {
                        KillTimer( hWnd, _ID_TIMER );

                        start_watch=0;
                        start_reset = 0;
                        times = 0;
                        msecond = 0;
                        second = 0;
                        minute = 0;
                        InvalidateRect( hWnd, &time_rect, FALSE );
                        break;
                    }
            }
            break;
    }
    return pmp_def_wnd_proc( hWnd, message, wParam, lParam );
}

static void _stopwatch_init_create_info( PMAINWINCREATE pCreateInfo )
{
    pCreateInfo->dwStyle = WS_BORDER | WS_VISIBLE;
    pCreateInfo->dwExStyle = WS_EX_NONE;
    pCreateInfo->spCaption = _(IDS_PMP_STOPWATCH_TITLE);
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor(0);
    pCreateInfo->hIcon = 0;
    pCreateInfo->MainWindowProc = StopWatchProc;
    pCreateInfo->lx = 0;
    pCreateInfo->ty = 20;
    pCreateInfo->rx = MAINWINDOW_W;
    pCreateInfo->by = MAINWINDOW_H;
    pCreateInfo->iBkColor = COLOR_lightgray;
    pCreateInfo->dwAddData = 0;
    pCreateInfo->hHosting = HWND_DESKTOP;
}

void ex_stopwatch_show_window( HWND parent )
{
    HWND hMainWnd = HWND_INVALID;
    MAINWINCREATE CreateInfo;

    _stopwatch_init_create_info( &CreateInfo );
    CreateInfo.hHosting = parent;

    hMainWnd = CreateMainWindow( &CreateInfo );
    ShowWindow( hMainWnd, SW_SHOWNORMAL );

    pmp_app_notify_opened( PMP_APP_STOPWATCH, hMainWnd );
}
