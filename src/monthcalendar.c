/*
** $Id$
**
** monthcalendar.c: A part of calendar module for Feynman PMP solution.
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pmp.h"
#include <minigui/control.h>
#include "text_id.h"
#include "monthcal.h"
#include "monthcalendar_impl.h"
/*The width and the hight of the miniwindow rect*/
#define MINWNDRECT_W         200
#define MINWNDRECT_H         120
/*The height of the week file*/
#define WEEKFIELD_H(hwnd)    (GetWindowFont(hwnd)->size+4)
/*The width from border to weektitle*/
#define WEEK_BORDER          5 
/*The hight from borfer to weektitle*/
#define WEEK_VBORDER1        2
#define WEEK_VBORDER2        3

/*Something about the rect of arrow*/
#define ARROW_H              7
#define ARROW_W              (ARROW_H+1)/2
#define ARROWRECT_W          (ARROW_W+4)
/*Something about the rect of month*/
#define MON_WIDTH            40
#define MON_HEIGHT           14
#define MON_BORDER           15
/*Something about the rect of year*/
#define YEAR_WIDTH           40
#define YEAR_HEIGHT          14
#define YEAR_BORDER          15
#define ARROW_BORDER         10

/*color info*/
#define MCCLR_DF_TITLEBK         RGB2Pixel(HDC_SCREEN, 71, 156, 210) //0xD6, 0xD3, 0xCE) 
#define MCCLR_DF_TITLETEXT       PIXEL_black
#define MCCLR_DF_ARROW           PIXEL_black
#define MCCLR_DF_ARROWHIBK       RGB2Pixel(HDC_SCREEN, 71, 156, 210);
#define MCCLR_DF_DAYBK           RGB2Pixel(HDC_SCREEN, 71, 156, 210);
#define MCCLR_DF_DAYHIBK         PIXEL_black
#define MCCLR_DF_DAYTEXT         PIXEL_black
#define MCCLR_DF_TRAILINGTEXT    PIXEL_lightwhite
#define MCCLR_DF_DAYHITEXT       GetWindowElementColorEx(hWnd, FGC_MENUITEM_HILITE)
#define MCCLR_DF_WEEKCAPTBK      RGB2Pixel(HDC_SCREEN, 71, 84, 130) 
#define MCCLR_DF_WEEKCAPTTEXT    PIXEL_lightwhite

static int MonthCalendarCtrlProc ( HWND hWnd, int message, WPARAM wParam, LPARAM lParam );

BOOL RegisterMonthCalendarControl ( void )
{
    WNDCLASS WndClass;

    WndClass.spClassName = "monthcalendar";
    WndClass.dwStyle = WS_NONE;
    WndClass.dwExStyle = WS_EX_NONE;
    WndClass.hCursor = GetSystemCursor ( IDC_ARROW );
    WndClass.iBkColor = RGB2Pixel ( HDC_SCREEN, 71, 156, 210 );
    WndClass.WinProc = MonthCalendarCtrlProc;
    return RegisterWindowClass ( &WndClass );
}

void MonthCalendarControlCleanup ( void )
{
    UnregisterWindowClass ( CTRL_MONTHCALENDAR );
    return;
}

/* draw page arrow */
static void mcDrawPageArrow ( PMONCALDDATA mc_data, HDC hdc, RECT* prcArrow, 
        BOOL bFaceR, BOOL bHilight )
{
    int arrow_w, arrow_h = ARROW_H;
    int line_x, line_h, line_y;
    int stepx;
    int rc_w, rc_h;
    PMCCOLORINFO pmcci;

    pmcci = ( PMCCOLORINFO ) mc_data->dwClrData;
    arrow_w = ( arrow_h + 1 )/2;
    rc_w = prcArrow->right - prcArrow->left;
    rc_h = prcArrow->bottom - prcArrow->top;
    SetPenColor ( hdc, pmcci->clr_arrow );
    if ( bHilight ) {
        SetBrushColor ( hdc, pmcci->clr_arrowHibk );
        FillBox ( hdc, prcArrow->left, prcArrow->top, rc_w, rc_h );
    }
    else {
        SetBrushColor ( hdc, pmcci->clr_titlebk );
        FillBox ( hdc, prcArrow->left, prcArrow->top, rc_w, rc_h );
    }
    line_y = prcArrow->top + ( rc_h-arrow_h )/2;
    line_h = arrow_h;
    if ( bFaceR ) {
        stepx = 1;
        line_x = prcArrow->left + ( rc_w - arrow_w )/2;
    }
    else {
        stepx = -1;
        line_x = prcArrow->right - ( rc_w - arrow_w )/2 - 1;
    }
    while ( line_h > 0 ) {
        MoveTo ( hdc, line_x, line_y );
        LineTo ( hdc, line_x, line_y+line_h );
        line_x += stepx;
        line_y++;
        line_h -= 2;
    }
}

/*Cleanup the monthcalendar*/
static void mcMonthCalendarCleanUp ( MONCALDDATA* mc_data )
{
    free ( ( PMCCOLORINFO ) mc_data->dwClrData );
    return;
}

/*find out whether a year is a leap year*/
static BOOL IsLeapYear ( int year )
{
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) return TRUE;
    else return FALSE;
}


/*get month length from year and month*/
static int GetMonLen ( int year, int month )
{
    int mon_len;

    if ( month < 1 || month > 12 )
        return -1;
    if ( ( month <= 7 && month % 2 == 1 ) || ( month >= 8 && month % 2 == 0 ) )
        mon_len = 31;
    else if ( month == 2 ){
        if ( IsLeapYear ( year ) )
            mon_len = 29;
        else 
            mon_len = 28;
    }
    else
        mon_len = 30;
    return mon_len;
}

/*get previous month length from year and month*/
static int GetPMonLen ( int year, int month )
{
    if ( month > 1 && month <= 12 )
        return GetMonLen ( year, month - 1 );
    else if ( month == 1 ) 
        return GetMonLen ( year - 1, 12 );
    else
        return -1;
}

/*get weekday from date*/
static int GetWeekDay (int year, int month, int day)
{
    struct tm nowday;

    if ( year < 1970 )
    {
        return -1;
    }
    if ( month < 1 || month > 12 )
        return -1;
    if ( day < 1 || day > 31 ) 
        return -1;

    if ( year > 2020 )
    {
        return -1;
    }
    nowday.tm_sec = 0;
    nowday.tm_min = 0;
    nowday.tm_hour = 0;
    nowday.tm_mday = day;
    nowday.tm_mon = month-1;
    nowday.tm_year = year-1900;

    if ( __mg_mktime ( &nowday ) == -1 )
        return -1;
    else
        return nowday.tm_wday;    
}

/*get line and weekday from date according to weekday1*/
static void mcGetCurDayPos ( PMONCALDDATA mc_data, int day, int* pline, int* pWeekDay )
{
    *pWeekDay = ( mc_data->WeekDay1 + day - 1 ) % 7;
    *pline = ( day + 6 - *pWeekDay - 1 ) / 7;
}

/*get rects of arrows*/
static void mcGetArrowRects ( RECT* prcMonth, RECT* prcYear, 
        RECT* prcAML, RECT* prcAMR, 
        RECT* prcAYL, RECT* prcAYR )
{
    prcAML->right = prcMonth->left - 1;
    prcAML->left = prcAML->right - ARROWRECT_W;
    prcAML->top = prcMonth->top;
    prcAML->bottom = prcMonth->bottom;

    prcAMR->left = prcMonth->right + 2;    
    prcAMR->right = prcAMR->left + ARROWRECT_W;
    prcAMR->top = prcMonth->top;
    prcAMR->bottom = prcMonth->bottom;

    prcAYL->right = prcYear->left - 2;
    prcAYL->left = prcAYL->right - ARROWRECT_W;
    prcAYL->top = prcYear->top;
    prcAYL->bottom = prcYear->bottom;

    prcAYR->left = prcYear->right + 2;    
    prcAYR->right = prcAYR->left + ARROWRECT_W;
    prcAYR->top = prcYear->top;
    prcAYR->bottom = prcYear->bottom;
}

/*get rect of month day area*/
static void mcGetMDayRect (HWND hwnd, RECT* prcClient, RECT* prcMDay)
{
    prcMDay->left = prcClient->left + WEEK_BORDER;
    prcMDay->right = prcClient->right - WEEK_BORDER;
    prcMDay->top = prcClient->top + 2 + DATECAP_H(hwnd) + 2*WEEK_VBORDER2 + WEEKFIELD_H(hwnd);
    prcMDay->bottom = prcClient->bottom - WEEK_VBORDER2;
}

/*get rects */
static void mcGetRects ( HWND hWnd,  RECT* prcClient,
        RECT* prcMonth,
        RECT* prcYear,
        RECT* prcMDay )
{
    RECT rcClient;

    GetClientRect ( hWnd, &rcClient );
    rcClient.right --;
    rcClient.bottom --;


    if ( prcClient ) {
        *prcClient = rcClient;
    }    

    if ( prcMonth ) 
        SetRect ( prcMonth, rcClient.left + MON_BORDER,
                rcClient.top + 6,
                rcClient.left + MON_BORDER + MON_WIDTH,
                rcClient.top + 2 + DATECAP_H ( hWnd ) - 4 );

    if ( prcYear )
        SetRect ( prcYear, rcClient.right - YEAR_BORDER - YEAR_WIDTH,
                rcClient.top + 6,
                rcClient.right - YEAR_BORDER,
                rcClient.top + 2 + DATECAP_H ( hWnd ) - 4 );

    if ( prcMDay )
        mcGetMDayRect ( hWnd, &rcClient, prcMDay );
}

/*get current item rect from mc_data*/
static void mcGetCurRect ( RECT* prcMDay, RECT* prcItem, PMONCALDDATA mc_data )
{
    prcItem->left = prcMDay->left + mc_data->cur_WeekDay*mc_data->item_w;    
    prcItem->top = prcMDay->top + mc_data->cur_line*mc_data->item_h;
    prcItem->right = prcItem->left + mc_data->item_w;
    prcItem->bottom = prcItem->top + mc_data->item_h;
}

/*textout in a rect center style*/
static void mcTextOutCenter ( HWND hWnd, HDC hdc, RECT* prcText, const char* pchText )
{

    if ( pchText ) {
        int bkMode;

        bkMode = SetBkMode ( hdc, BM_TRANSPARENT );
        SetBkColor ( hdc, GetWindowBkColor ( hWnd ) );
        DrawText ( hdc, pchText, -1, prcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP );
        SetBkMode ( hdc, bkMode );
    }
}

/*up the rect which is not highlight*/
static void mcUnHilightRect ( HWND hWnd, PMONCALDDATA mc_data, HDC hdc, RECT* prcItem, int day )
{
    int item_w, item_h;
    char daytext[3];
    PMCCOLORINFO pmcci;

    pmcci = ( PMCCOLORINFO ) mc_data->dwClrData;
    item_w = prcItem->right - prcItem->left;
    item_h = prcItem->bottom - prcItem->top;
    SetBrushColor ( hdc, pmcci->clr_daybk );
    FillBox ( hdc, prcItem->left, prcItem->top, item_w, item_h );
    if ( day < 10 )
        sprintf ( daytext, " %d", day );
    else
        sprintf ( daytext, "%d", day );
    SetBkColor ( hdc, pmcci->clr_dayHibk );
    SetTextColor ( hdc, pmcci->clr_daytext );
    mcTextOutCenter ( hWnd, hdc, prcItem, daytext );
}

/*draw change day*/
static void mcDrawDay ( HWND hWnd, HDC hdc, PMONCALDDATA mc_data, RECT* prcMDay, 
        int newday )
{
    RECT rcPItemDay, rcItemDay;
    PMCCOLORINFO pmcci;

    pmcci = (PMCCOLORINFO) mc_data->dwClrData;
    mcGetCurRect (prcMDay, &rcPItemDay, mc_data);
    mcUnHilightRect (hWnd, mc_data, hdc, &rcPItemDay, mc_data->cur_day);    

    mc_data->cur_day = newday;
    mcGetCurDayPos ( mc_data, mc_data->cur_day, &mc_data->cur_line, 
            &mc_data->cur_WeekDay ); 
    mcGetCurRect ( prcMDay, &rcItemDay, mc_data );
}

// draw month day area
static void mcDrawMonthDay ( HWND hWnd, HDC hdc, RECT* prcMDay, PMONCALDDATA mc_data )
{
    int i, WeekDayPM = 0, LineIndex = 0;
    int mdaypm = 0, MonLenPM, mdaynm;
    int iWeekDay = 0;
    char chMonthDay[3];
    RECT rcMonthDay;
    PMCCOLORINFO pmcci;

    pmcci = ( PMCCOLORINFO ) mc_data->dwClrData;
    SetBkColor ( hdc, pmcci->clr_daybk ); 
    SetTextColor ( hdc, pmcci->clr_daytext );
    for ( i = 1; i <= mc_data->monlen; i++ ) {
        if ( i < 10 ) 
            sprintf ( chMonthDay, " %d", i );
        else
            sprintf ( chMonthDay, "%d", i );
        iWeekDay = ( mc_data->WeekDay1 + i - 1 ) % 7;
        rcMonthDay.left = prcMDay->left + iWeekDay*mc_data->item_w;
        rcMonthDay.right = rcMonthDay.left + mc_data->item_w;
        LineIndex = ( mc_data->WeekDay1 + i - 1 ) / 7;
        rcMonthDay.top = prcMDay->top + mc_data->item_h*LineIndex;
        rcMonthDay.bottom = rcMonthDay.top + mc_data->item_h;
        if ( i == mc_data->cur_day ) {
#ifndef _PHONE_WINDOW_STYLE
            SetTextColor ( hdc, pmcci->clr_dayHitext );
            SetBkColor ( hdc, pmcci->clr_dayHibk );
            SetBrushColor ( hdc, pmcci->clr_dayHibk );
            FillBox ( hdc, rcMonthDay.left, rcMonthDay.top, 
                    mc_data->item_w, mc_data->item_h );
#else
            SetTextColor ( hdc, pmcci->clr_daytext );
#endif
            SetPenColor ( hdc, GetWindowElementColor ( FGC_MENUITEM_FRAME ) );
            Rectangle ( hdc, rcMonthDay.left, rcMonthDay.top, 
                    rcMonthDay.left+mc_data->item_w-1 , 
                    rcMonthDay.top+mc_data->item_h-1 );
        }
        else {
            SetBkColor ( hdc, pmcci->clr_daybk );
            SetTextColor ( hdc, pmcci->clr_daytext );
        }

        if ( mc_data->customed_day & ( 1 << i ) ) {
            SetTextColor ( hdc, mc_data->customed_day_color[i] );
        }
        mcTextOutCenter ( hWnd, hdc, &rcMonthDay, chMonthDay );
    }

    SetTextColor ( hdc, pmcci->clr_trailingtext );

    LineIndex += ( iWeekDay + 1 ) / 7;
    iWeekDay = ( iWeekDay + 1 ) % 7;
    mdaynm = 1;
    while ( LineIndex <= 5 ) {
        if ( mdaynm < 10 ) 
            sprintf ( chMonthDay, " %d", mdaynm );
        else
            sprintf ( chMonthDay, "%d", mdaynm );
        rcMonthDay.left = prcMDay->left + iWeekDay*mc_data->item_w;
        rcMonthDay.right = rcMonthDay.left + mc_data->item_w;
        rcMonthDay.top = prcMDay->top + mc_data->item_h*LineIndex;
        rcMonthDay.bottom = rcMonthDay.top + mc_data->item_h;
        mcTextOutCenter ( hWnd, hdc, &rcMonthDay, chMonthDay );
        mdaynm++;
        iWeekDay++;
        if ( iWeekDay == 7 ) {
            iWeekDay = 0;
            LineIndex++;
        }
    }

    WeekDayPM = mc_data->WeekDay1 - 1;
    if ( WeekDayPM >= 0 ) {
        rcMonthDay.top = prcMDay->top;
        rcMonthDay.bottom = rcMonthDay.top + mc_data->item_h;
        rcMonthDay.left = prcMDay->left + WeekDayPM*mc_data->item_w;
        rcMonthDay.right = rcMonthDay.left + mc_data->item_w;
        MonLenPM = GetPMonLen ( mc_data->cur_year, mc_data->cur_month );
        mdaypm = MonLenPM;
    }
    while ( WeekDayPM >= 0 ) {
        sprintf ( chMonthDay, "%d", mdaypm );
        mcTextOutCenter ( hWnd, hdc, &rcMonthDay, chMonthDay );
        OffsetRect( &rcMonthDay, -mc_data->item_w, 0 );
        mdaypm--;
        WeekDayPM--;
    }
}

/*draw change month*/
static const int chMon_C[] = { IDS_PMP_CALENDAR_JAN, IDS_PMP_CALENDAR_FEB, 
                IDS_PMP_CALENDAR_MAR, IDS_PMP_CALENDAR_APR, IDS_PMP_CALENDAR_MAY, 
                IDS_PMP_CALENDAR_JUNE, IDS_PMP_CALENDAR_JULY, IDS_PMP_CALENDAR_AUG,
                IDS_PMP_CALENDAR_SEP, IDS_PMP_CALENDAR_OCT, IDS_PMP_CALENDAR_NOV,
                IDS_PMP_CALENDAR_DEC };
static const int chWeek_C[] = { IDS_PMP_CALENDAR_SUN, IDS_PMP_CALENDAR_MON, 
                IDS_PMP_CALENDAR_TUE, IDS_PMP_CALENDAR_WED, IDS_PMP_CALENDAR_THU, 
                IDS_PMP_CALENDAR_FRI, IDS_PMP_CALENDAR_SAT }; 

/*draw the whole calendar*/
static void mcDrawCalendar ( HWND hWnd, HDC hdc, PMONCALDDATA mc_data )
{
    RECT rcClient, rcMonth, rcYear, rcMDay;
    RECT rcAML, rcAMR, rcAYL, rcAYR;
    DWORD dwStyle;
    PMCCOLORINFO pmcci;
    UINT uFormat;
    pmcci = ( PMCCOLORINFO ) mc_data->dwClrData;
    dwStyle = GetWindowStyle ( hWnd );
    mcGetRects ( hWnd, &rcClient, &rcMonth, &rcYear, &rcMDay );

    Draw3DThickFrameEx ( hdc, hWnd, rcClient.left, rcClient.top, rcClient.right,
            rcClient.bottom, DF_3DBOX_PRESSED | DF_3DBOX_FILL, pmcci->clr_daybk );

    if ( DATECAP_H(hWnd) > 0 ) {
        Draw3DThickFrameEx ( hdc, hWnd, rcClient.left+2, rcClient.top+2,
                rcClient.right-2, rcClient.top+2+DATECAP_H ( hWnd ), 
                DF_3DBOX_NORMAL | DF_3DBOX_FILL, pmcci->clr_titlebk );
    }

    if ( DATECAP_H ( hWnd ) > 0 ) {

        mcGetArrowRects ( &rcMonth, &rcYear, &rcAML, &rcAMR, &rcAYL, &rcAYR );
        mcDrawPageArrow ( mc_data, hdc, &rcAML, FALSE, FALSE );
        mcDrawPageArrow ( mc_data, hdc, &rcAMR, TRUE, FALSE );
        mcDrawPageArrow ( mc_data, hdc, &rcAYL, FALSE, FALSE );
        mcDrawPageArrow ( mc_data, hdc, &rcAYR, TRUE, FALSE );

        {
            char chYear[5];
            const char* pchMon;
            int oldcolor;

            uFormat = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
            pchMon = _(chMon_C [ mc_data->cur_month-1 ]);

            oldcolor = SetTextColor ( hdc, pmcci->clr_titletext );
            mcTextOutCenter ( hWnd, hdc, &rcMonth, pchMon );

            sprintf ( chYear, "%d", mc_data->cur_year );
            mcTextOutCenter ( hWnd, hdc, &rcYear, chYear );
            SetTextColor ( hdc, oldcolor );
        }

    } /*no year mon*/

    /* draw weekday caption*/
    {
        int i, weekitem_w;
        const char *pchDay;
        RECT rcWeek, rcWeekItem;

        rcWeek.left = rcClient.left + WEEK_BORDER;
        rcWeek.right = rcClient.right-WEEK_BORDER;
        rcWeek.top = rcClient.top + 2 + WEEK_VBORDER1 + DATECAP_H ( hWnd );
        rcWeek.bottom = rcWeek.top + WEEKFIELD_H ( hWnd );
        weekitem_w = ( rcWeek.right - rcWeek.left ) / 7;
        rcWeekItem.left = rcWeek.left;
        rcWeekItem.right = rcWeekItem.left + weekitem_w;
        rcWeekItem.top = rcWeek.top;
        rcWeekItem.bottom = rcWeek.bottom;

        SetBrushColor ( hdc, pmcci->clr_weekcaptbk );
        FillBox ( hdc, rcWeek.left, rcWeek.top, rcWeek.right-rcWeek.left,
                rcWeek.bottom-rcWeek.top );

        SetBkColor ( hdc, pmcci->clr_weekcaptbk );
        SetTextColor ( hdc, pmcci->clr_weekcapttext );
        for ( i = 0; i <= 6; i++ ) {
            pchDay = _(chWeek_C[i]);
            TextOut ( hdc, rcWeekItem.left+15, rcWeekItem.top+1, pchDay );
            OffsetRect ( &rcWeekItem, weekitem_w, 0 );
        }
    }
    /*draw month day text*/
    mcDrawMonthDay ( hWnd, hdc, &rcMDay, mc_data );
}

/*initialize mc_data*/
static BOOL mcInitMonthCalendarData ( HWND hWnd, MONCALDDATA* mc_data )
{
    time_t nowtime;
    struct tm *pnt;
    RECT rcClient, rcMDay;
    PMCCOLORINFO pmcci;

    __mg_time( &nowtime );
    pnt = ( struct tm * )__mg_localtime ( &nowtime );
    mc_data->sys_year = mc_data->cur_year = pnt->tm_year + 1900;
    mc_data->sys_month = mc_data->cur_month = pnt->tm_mon + 1;
    mc_data->sys_day = mc_data->cur_day = pnt->tm_mday;

    mcGetRects ( hWnd, &rcClient, NULL, NULL, &rcMDay );
    mc_data->item_w = ( rcMDay.right - rcMDay.left ) / 7;
    mc_data->item_h = ( rcMDay.bottom - rcMDay.top ) / 6;
    mc_data->WeekDay1 = GetWeekDay ( mc_data->cur_year, mc_data->cur_month, 1 );
    mcGetCurDayPos ( mc_data, mc_data->cur_day,
            &mc_data->cur_line, &mc_data->cur_WeekDay );    
    mc_data->sys_WeekDay = mc_data->cur_WeekDay;    
    mc_data->monlen = GetMonLen ( mc_data->cur_year, mc_data->cur_month );

    mc_data->state = 0;

    /*color info*/
    pmcci = ( PMCCOLORINFO ) mc_data->dwClrData;
    pmcci->clr_titlebk = MCCLR_DF_TITLEBK;
    pmcci->clr_titletext = MCCLR_DF_TITLETEXT;
    pmcci->clr_arrow = MCCLR_DF_ARROW;
    pmcci->clr_arrowHibk = MCCLR_DF_ARROWHIBK;
    pmcci->clr_daybk = MCCLR_DF_DAYBK;
    pmcci->clr_dayHibk = MCCLR_DF_DAYHIBK;
    pmcci->clr_daytext = MCCLR_DF_DAYTEXT;
    pmcci->clr_trailingtext = MCCLR_DF_TRAILINGTEXT;
    pmcci->clr_dayHitext = MCCLR_DF_DAYHITEXT;
    pmcci->clr_weekcaptbk = MCCLR_DF_WEEKCAPTBK;
    pmcci->clr_weekcapttext = MCCLR_DF_WEEKCAPTTEXT;

    mc_data->customed_day = 0;
    return TRUE;
}

/*Set the curday*/
static void mcSetCurDay ( HWND hWnd, PMONCALDDATA mc_data, int newday )
{
    RECT rcClient, rcMonth, rcMDay, rcYear;
    HDC hdc;

    mcGetRects ( hWnd, &rcClient, &rcMonth, &rcYear, &rcMDay );
    hdc = GetClientDC ( hWnd );
    mcDrawDay ( hWnd, hdc, mc_data, &rcMDay, newday );
    ReleaseDC ( hdc );
}

/*Shift the year and the month*/
static void mcShiftYM ( HWND hWnd, PMONCALDDATA mc_data, BOOL bMonth, BOOL bRight )
{
    RECT rcClient, rcMonth, rcMDay, rcYear;
    mcGetRects ( hWnd, &rcClient, &rcMonth, &rcYear, &rcMDay );
    if ( GetWindowStyle ( hWnd ) & MCS_NOTIFY )
        NotifyParent ( hWnd, GetDlgCtrlID ( hWnd ), MCN_DATECHANGE );        

    if ( bMonth && bRight ) 
    {
        if ( mc_data->cur_month != 12 ) 
        {
            mc_data->cur_month++;
            InvalidateRect ( hWnd, &rcMonth, FALSE );
            InvalidateRect ( hWnd, &rcMDay, FALSE );
        }
        else
        {
            if ( mc_data->cur_year >= 2020 )
            {
                return;
            }
            mc_data->cur_month = 1;
            mc_data->cur_year++;
            InvalidateRect ( hWnd, &rcYear, FALSE );
            InvalidateRect ( hWnd, &rcMonth, FALSE );
            InvalidateRect ( hWnd, &rcMDay, FALSE );
        }
    }
    else if ( bMonth ) 
    {
        if ( mc_data->cur_month != 1 ) 
        {
            mc_data->cur_month--;
            InvalidateRect ( hWnd, &rcMonth, FALSE );
            InvalidateRect ( hWnd, &rcMDay, FALSE );
        }
        else
        {
            if ( mc_data->cur_year <= 1970 )
            {
                return;
            }
            mc_data->cur_month = 12;
            mc_data->cur_year--;
            InvalidateRect ( hWnd, &rcYear, FALSE );
            InvalidateRect ( hWnd, &rcMonth, FALSE );
            InvalidateRect ( hWnd, &rcMDay, FALSE );
        }
    }
    else if (bRight) 
    {
        if ( mc_data->cur_year >= 2020 )
        {
            return;
        }
        mc_data->cur_year++;
        InvalidateRect ( hWnd, &rcYear, FALSE );
        InvalidateRect ( hWnd, &rcMDay, FALSE );
    }
    else 
    {
        if ( mc_data->cur_year <= 1970 )
        {
            return;
        }
        mc_data->cur_year--;
        InvalidateRect ( hWnd, &rcYear, FALSE );
        InvalidateRect ( hWnd, &rcMDay, FALSE );
    }

    mc_data->monlen = GetMonLen ( mc_data->cur_year, mc_data->cur_month );
    while ( mc_data->cur_day > mc_data->monlen ) {
        mc_data->cur_day--;
    }
    mc_data->WeekDay1 = GetWeekDay ( mc_data->cur_year, mc_data->cur_month, 1 );
    mcGetCurDayPos ( mc_data, mc_data->cur_day,
            &mc_data->cur_line, &mc_data->cur_WeekDay );    
    SendNotifyMessage ( GetParent ( hWnd ), MSG_COMMAND, 0, 0 );
}

#ifdef _KEY_OP_SUPPORT
/*When the keys down what  will be done*/
static void mcKeyOperations ( HWND hWnd, PMONCALDDATA mc_data, WPARAM wParam )
{
    int day = -1;
    RECT rcMDay, rcClient;
    RECT  rcPItemDay, rcItemDay;
    PMONCALDDATA pmcci;

    mcGetRects ( hWnd, &rcClient, NULL, NULL, &rcMDay );
    switch ( wParam ){

        case SCANCODE_CURSORBLOCKLEFT:
        case SCANCODE_CURSORBLOCKUP:
            if ( CURFOCUS == MST_YEAR_FOCUS ){
                mcShiftYM ( hWnd, mc_data, FALSE, FALSE );
                return;
            }
            else if ( CURFOCUS == MST_MONTH_FOCUS ){
                mcShiftYM ( hWnd, mc_data, TRUE, FALSE );
                return;
            }
            else
                day = mc_data->cur_day - ( wParam == SCANCODE_CURSORBLOCKLEFT ? 1 : 7 );
            SendNotifyMessage ( GetParent ( hWnd ), MSG_COMMAND, 0, 0 );
            break;
        case SCANCODE_CURSORBLOCKRIGHT:
        case SCANCODE_CURSORBLOCKDOWN:
            if ( CURFOCUS == MST_YEAR_FOCUS ) {
                mcShiftYM ( hWnd, mc_data, FALSE, TRUE );
                return;
            }
            else if ( CURFOCUS == MST_MONTH_FOCUS ) {
                mcShiftYM ( hWnd, mc_data, TRUE, TRUE );
                return;
            }
            else
                day = mc_data->cur_day + ( wParam == SCANCODE_CURSORBLOCKRIGHT ? 1 : 7 );
            SendNotifyMessage ( GetParent ( hWnd ), MSG_COMMAND, 0, 0 );
            break;

        default:
            return;
    }

    if ( day > mc_data->monlen ) {
        mc_data->cur_day = day - mc_data->monlen;
        mcShiftYM ( hWnd, mc_data, TRUE, TRUE );
    }
    else if ( day < 1 ) {
        mcShiftYM ( hWnd, mc_data, TRUE, FALSE );
        mcSetCurDay ( hWnd, mc_data, mc_data->monlen + day );
    }
    else {
        mcGetCurRect ( &rcMDay, &rcPItemDay, mc_data );

        pmcci = ( PMONCALDDATA )mc_data;
        pmcci->cur_day = day;
        mcGetCurDayPos ( pmcci, pmcci->cur_day, &pmcci->cur_line, 
                &pmcci->cur_WeekDay );    
        mcGetCurRect ( &rcMDay, &rcItemDay, pmcci );
        mcSetCurDay ( hWnd, mc_data, day );
        InvalidateRect ( hWnd, &rcPItemDay, FALSE );
        InvalidateRect ( hWnd, &rcItemDay, FALSE );
    }
}

#endif

/*The proc of the monthcalendar window*/

static int MonthCalendarCtrlProc ( HWND hWnd, int message, WPARAM wParam, LPARAM lParam )
{
    HDC hdc;
    PMONCALDDATA    mc_data = NULL;

    if ( message != MSG_CREATE )
        mc_data = ( PMONCALDDATA ) GetWindowAdditionalData2 ( hWnd );

    switch ( message ) {
        case MSG_CREATE:
            {
                RECT rcWnd;
                int rc_w, rc_h;
                PMCCOLORINFO pmcci;

                GetWindowRect (hWnd, &rcWnd);
                rc_w = rcWnd.right - rcWnd.left;
                rc_h = rcWnd.bottom - rcWnd.top;
                if ( rc_w < MINWNDRECT_W ) {
                    rc_w = MINWNDRECT_W;
                    MoveWindow ( hWnd, rcWnd.left, rcWnd.top, rc_w, 
                            rc_h, FALSE );
                }
                if ( rc_h < MINWNDRECT_H ) {
                    rc_h = MINWNDRECT_H;
                    MoveWindow ( hWnd, rcWnd.left, rcWnd.top, rc_w, 
                            rc_h, FALSE );
                }

                if ( ( mc_data = ( MONCALDDATA * ) malloc ( sizeof ( MONCALDDATA ) ) ) == NULL )
                    return -1;
                SetWindowAdditionalData2 ( hWnd, ( DWORD ) mc_data );
                if ( ( pmcci = ( MCCOLORINFO* ) malloc ( sizeof ( MCCOLORINFO ) ) ) == NULL ) {
                    free ( mc_data );
                    return -1;
                }
                mc_data->dwClrData = ( DWORD ) pmcci;

                if ( !mcInitMonthCalendarData ( hWnd, mc_data ) ) {
                    free ( pmcci );
                    free ( mc_data );
                    return -1;
                }
            }
            break;

        case MCM_GETCURDATE:
            {
                PSYSTEMTIME pcurdate = NULL;

                pcurdate = ( PSYSTEMTIME ) lParam;
                pcurdate->year = mc_data->cur_year;
                pcurdate->month = mc_data->cur_month;
                pcurdate->day = mc_data->cur_day;
                pcurdate->weekday = mc_data->cur_WeekDay;
                return 0;
            }

        case MSG_DESTROY:
            mcMonthCalendarCleanUp ( mc_data );
            free ( mc_data );
            break;

#ifdef _KEY_OP_SUPPORT
        case MSG_KEYDOWN:
            if ( wParam == SCANCODE_F1 ) {
                int focus = ( CURFOCUS+1 ) % FOCUS_NR;
                SETFOCUS ( focus );
            }
            else
                mcKeyOperations ( hWnd, mc_data, wParam );
            break;
#endif
        case MSG_PAINT:
            hdc = BeginPaint ( hWnd );
            mcDrawCalendar ( hWnd, hdc, mc_data );
            EndPaint ( hWnd, hdc );
            return 0;

        default:
            break;
    }

    return DefaultControlProc ( hWnd, message, wParam, lParam );
}
