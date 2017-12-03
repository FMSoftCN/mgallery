/*
** $Id$
**
**monthcalendar_impl.h: head file of monthcalendar.
**
** Copyright (C) 2007 Feynman Software, all rights reserved.
**
** Use of this source package is subject to specific license terms
** from Beijing Feynman Software Technology Co., Ltd.
**
** URL: http://www.minigui.com
**
** Current maintainer: Zhang Hua.
*/

#ifndef __MONTHCALENDAR_IMPL_H_
#define __MONTHCALENDAR_IMPL_H_

#ifdef  __cplusplus
extern  "C" {
#endif

#define _KEY_OP_SUPPORT      1


#ifdef _KEY_OP_SUPPORT

#define MST_FOCUS_MASK       0x00000003
#define FOCUS_NR             3

#define MST_DAY_FOCUS        0
#define MST_MONTH_FOCUS      1
#define MST_YEAR_FOCUS       2

#define CURFOCUS             (mc_data->state & MST_FOCUS_MASK)
#define SETFOCUS(i)          (mc_data->state = (mc_data->state & \
                                  (~MST_FOCUS_MASK)) | (i & MST_FOCUS_MASK) )
#endif


#define MCS_LANG 0x0003L

static inline int DATECAP_H(HWND hwnd)
{
    if (GetWindowStyle(hwnd)&MCS_NOYEARMON)
        return 0;
    return (GetWindowFont(hwnd)->size+8);
}

// structs 
typedef struct tagMonthCalendarData
{
    int     sys_month;
    int     sys_year;
    int     sys_day;
    int     sys_WeekDay;

    int     cur_month;
    int     cur_year;
    int     cur_day;

    int     cur_line;
    int     cur_WeekDay;
    int     item_w;
    int     item_h;
    int     WeekDay1;
    int     monlen;

    DWORD   dwClrData;

    UINT    state;

    UINT    customed_day;
    gal_pixel customed_day_color[32];

} MONCALDDATA;
typedef MONCALDDATA* PMONCALDDATA;

BOOL RegisterMonthCalendarControl (void);
void MonthCalendarControlCleanup (void);

time_t __mg_time (time_t * timer);
time_t __mg_mktime (struct tm * timeptr);
struct tm *__mg_localtime (const time_t * timer);

#ifdef  __cplusplus
}
#endif

#endif    // __MONTHCALENDAR_IMPL_H__
