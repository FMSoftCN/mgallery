/*
** $Id$
**
** timedate.c: window set the time and date.  
**
** Copyright (C) 2006 Feynman Software.
**
** All right reserved by Feynman Software.
**
** Current maintainer: Wang Xuguang.
**
** Create date: 2006/11/17
*/

#define COLOR_SEL COLOR_green

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mgext.h>

#include <pmp.h>
#include "setup.h"

#define IDC_STATIC_HOUR			101
#define IDC_STATIC_SEPARATOR	102
#define IDC_STATIC_MINUTE		103
#define IDC_STATIC_24HOUR		104

#define IDC_STATIC_YEARVAL		105
#define IDC_STATIC_YEAR			106

#define IDC_STATIC_MONTHVAL		107
#define IDC_STATIC_MONTH		108

#define IDC_STATIC_DAYVAL		109
#define IDC_STATIC_DAY			110

extern struct tm tm;
extern struct tm* get_time(void);
extern BOOL wnd_creat;
extern void write_time(struct tm* tm);
//extern void draw_bkground(HWND hwnd, WPARAM wparam, LPARAM lparam);
//static int (*old_static_proc)(HWND, int, WPARAM, LPARAM);
/*
static int new_static_proc(HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{

	if (message == MSG_ERASEBKGND)
	{
		if (GetWindowBkColor(hwnd) == COLOR_green)
		{
			printf("control backcolor <COLOR_green>-%d\n",hwnd);
			return old_static_proc(hwnd, message, wparam, lparam);
		}

		draw_bkground(hwnd, wparam, lparam);
		printf("control backcolor <picture>_%d\n", hwnd);
		return 0;
	}
	return old_static_proc(hwnd, message, wparam, lparam);
}

void change_proc(HWND hwnd)
{
	int control_id;
	HWND h_contol;

	h_contol = GetDlgItem(hwnd, IDC_STATIC_DAY);
	old_static_proc = GetWindowCallbackProc(h_contol);

	for (control_id=IDC_STATIC_HOUR;
		   	control_id<=IDC_STATIC_DAY; 
			control_id++)
	{
		h_contol = GetDlgItem(hwnd, control_id);
		SetWindowCallbackProc(h_contol, new_static_proc);
	}
}
*/
static DLGTEMPLATE dlg_timedata_set =
{
	WS_VISIBLE,
	WS_EX_NOCLOSEBOX,
	0, 20, 320, 220,

#ifdef DEBUG
	"TIME DATE SETUP dialog",
#else
	"TIME DATA SETUP",
#endif

	0,
	0,
	9,	
	NULL,
	0,
};

static CTRLDATA ctrl_timedate_set[] =
{
	/*hour value*/
	{
		CTRL_STATIC,
		WS_VISIBLE | WS_CHILD | WS_THICKFRAME | WS_TABSTOP | SS_RIGHT,
		80, 64, 32, 20,
		IDC_STATIC_HOUR,
		"",
		0,
		WS_EX_TRANSPARENT,
	},
	/*separtor ( hour :  minute)*/
	{
		CTRL_STATIC,
		WS_VISIBLE | WS_CHILD,
		112, 64, 8, 20,
		IDC_STATIC_SEPARATOR,
		":",
		0,
		WS_EX_TRANSPARENT,
	},
	/*minute*/
	{
		CTRL_STATIC,
		WS_VISIBLE | WS_CHILD | WS_THICKFRAME | WS_TABSTOP | SS_RIGHT,
		120, 64, 32, 20,
		IDC_STATIC_MINUTE,
		"",
		0,
		WS_EX_TRANSPARENT,
	},
	/*display: "24 hours"*/
	{
		CTRL_STATIC,
		WS_VISIBLE | WS_CHILD,
		160, 64, 32, 20,
		IDC_STATIC_24HOUR,
		"24H",
		0,
		WS_EX_TRANSPARENT,
	},
	/*year value*/
	{
		CTRL_STATIC,
		WS_VISIBLE | WS_CHILD | WS_THICKFRAME | WS_TABSTOP | SS_RIGHT,
		80, 116, 32, 20,
		IDC_STATIC_YEARVAL,
		"",
		0,
		WS_EX_TRANSPARENT,
	},
	/*display "year"*/
	{
		CTRL_STATIC,
		WS_VISIBLE | WS_CHILD,
		112, 116, 8, 20,
		IDC_STATIC_YEAR,
		".",
		0,
		WS_EX_TRANSPARENT,
	},
	/*month value*/
	{
		CTRL_STATIC,
		WS_VISIBLE | WS_CHILD | WS_THICKFRAME | WS_TABSTOP | SS_RIGHT,
		120, 116, 32, 20,
		IDC_STATIC_MONTHVAL,
		"",
		0,
		WS_EX_TRANSPARENT,
	},
	/*display: "month"*/
	{
		CTRL_STATIC,
		WS_VISIBLE | WS_CHILD,
		152, 116, 8, 20,
		IDC_STATIC_MONTH,
		".",
		0,
		WS_EX_TRANSPARENT,
	},
	/*day value*/
	{
		CTRL_STATIC,
		WS_VISIBLE | WS_CHILD | WS_THICKFRAME | WS_TABSTOP | SS_RIGHT,
		160, 116, 32, 20,
		IDC_STATIC_DAYVAL,
		"",
		0,
		WS_EX_TRANSPARENT,
	},
	/*display: " day" */
    /*
	{
		CTRL_STATIC,
		WS_VISIBLE | WS_CHILD,
		224, 116, 16, 20,
		IDC_STATIC_DAY,
		"",
		0,
		WS_EX_TRANSPARENT,
	},
    */
	
};

static HWND set_hwnds[5];

static void (* add_proc[5]) (void); 

static void (* sub_proc[5]) (void);

static int  (* get_proc[5]) (void);

static char buffer[10];
/*
 *dlg_proc_timedata:
 *		callback process of setup dialogue of time and date
 *params:
 *		hdlg_timedate - handle of setup dialogue of time and date
 *		message - message type
 *		wparam - the first parameter
 *		lparam - the sencond parameter
 *return:
 *		
 */ 
static int dlg_proc_timedata(HWND hdlg_timedate, int message, WPARAM wparam, LPARAM lparam);


/*
 *timedate_setup: 
 *		set time and date
 *params:
 *		hdlg_main_set - handle of main setup dialogue
 *return:
 *		void
 */
int timedate_setup(HWND hdlg_main_set)
{
#ifdef DEBUG
	{
		static int call_num;
		call_num++;
		printf("timedate_setup is called %d\n", call_num);
	}
#endif

	struct tm* tm_p = get_time();
	tm = *tm_p;
	tm.tm_year += 1900;
	tm.tm_mon += 1;

	add_proc[0]	= add_hour;
	add_proc[1] = add_minute;
	add_proc[2]	= add_year;
	add_proc[3]	= add_month;
	add_proc[4]	= add_day;

	sub_proc[0] = sub_hour; 
	sub_proc[1] = sub_minute; 
	sub_proc[2] = sub_year; 
	sub_proc[3] = sub_month; 
	sub_proc[4] = sub_day; 

	get_proc[0] = get_hour;
	get_proc[1] = get_minute;
	get_proc[2] = get_year;
	get_proc[3] = get_month;
	get_proc[4] = get_day;

	dlg_timedata_set.controls = ctrl_timedate_set;

	DialogBoxIndirectParam(&dlg_timedata_set, hdlg_main_set, dlg_proc_timedata, 0L);

	return 0;
}

/*
 *dlg_proc_timedata:
 *		callback process of setup dialogue of time and date
 *params:
 *		hdlg_timedate - handle of setup dialogue of time and date
 *		message - message type
 *		wparam - the first parameter
 *		lparam - the sencond parameter
 *return:
 *		
 */ 
int dlg_proc_timedata(HWND hdlg_timedate, int message, WPARAM wparam, LPARAM lparam)

{

	static int cur_set = 0;
	static int bkcolor_def;

	switch(message)
	{
        case MSG_INITDIALOG:
			{
				int i;

				printf("GetHosting is %d\n", GetHosting(hdlg_timedate));

				set_hwnds[0] = GetDlgItem(hdlg_timedate,IDC_STATIC_HOUR);
				set_hwnds[1] = GetDlgItem(hdlg_timedate,IDC_STATIC_MINUTE);

				set_hwnds[2] = GetDlgItem(hdlg_timedate, IDC_STATIC_YEARVAL);
				set_hwnds[3] = GetDlgItem(hdlg_timedate, IDC_STATIC_MONTHVAL);
				set_hwnds[4] = GetDlgItem(hdlg_timedate, IDC_STATIC_DAYVAL);

				bkcolor_def = COLOR_blue;
				SetWindowElementColorEx(set_hwnds[cur_set], 
						WEC_FRAME_NORMAL, COLOR_SEL);

				/*show current time and date*/
				for (i=0; i<5; i++)
				{
					SetWindowElementColorEx(set_hwnds[i], 
							FGC_CONTROL_NORMAL, COLOR_lightwhite);

					if (i!=cur_set)
					{
						SetWindowElementColorEx(set_hwnds[i], 
								WEC_FRAME_NORMAL, bkcolor_def);
					}
					sprintf(buffer, "%d", get_proc[i]());
					SetWindowText(set_hwnds[i], buffer);
				}

			}
			break;

		case MSG_ERASEBKGND:
#ifdef BITMAPBK
            pmp_draw_background_withbmp(hdlg_timedate,
                    (HDC)wparam, (const RECT*)lparam, NULL);
#else
			pmp_draw_background (hdlg_timedate, 
                    (HDC)wparam, (const RECT*)lparam, NULL);
#endif
			return 0;

		case MSG_KEYDOWN:
			{
				/*select which to select*/
				if (wparam == SCANCODE_CURSORBLOCKLEFT
					|| wparam == SCANCODE_CURSORBLOCKRIGHT)
				{

					/*recovery bkcolor to old control*/
				SetWindowElementColorEx(set_hwnds[cur_set], WEC_FRAME_NORMAL, bkcolor_def);
					UpdateWindow(set_hwnds[cur_set], TRUE);

					if (wparam == SCANCODE_CURSORBLOCKLEFT)
					{
						cur_set = (cur_set - 1 + 5) % 5;
					}
					if (wparam == SCANCODE_CURSORBLOCKRIGHT)
					{
						cur_set = (cur_set + 1) % 5;
					}

				SetWindowElementColorEx(set_hwnds[cur_set], WEC_FRAME_NORMAL, COLOR_SEL);
					UpdateWindow(set_hwnds[cur_set], TRUE);

				}

				/*set the item selected*/
				else if (wparam == SCANCODE_CURSORBLOCKUP
						|| wparam == SCANCODE_CURSORBLOCKDOWN)
				{
					if (wparam == SCANCODE_CURSORBLOCKUP)
					{
						add_proc[cur_set]();
					}
					else
					{
						sub_proc[cur_set]();
					}
					sprintf(buffer, "%d", get_proc[cur_set]());
					SetWindowText(set_hwnds[cur_set], buffer);

					/*if cur_set is month or year, update day*/
					if (cur_set==2 || cur_set==3)
					{
						sprintf(buffer, "%d", get_proc[4]());
						SetWindowText(set_hwnds[4], buffer);
					}
				}

				/*end of timedate setup*/
				else if (wparam == SCANCODE_ENTER)
				{
					write_time(&tm);
					
					DestroyAllControls(hdlg_timedate);
					EndDialog(hdlg_timedate, IDC_STATIC_HOUR);
					
					wnd_creat = TRUE;
					return 0;
				}
				else if (wparam == SCANCODE_ESCAPE)
				{
					DestroyAllControls(hdlg_timedate);
					EndDialog(hdlg_timedate, IDC_STATIC_HOUR);
					
					wnd_creat = TRUE;
					return 0;
				}
			}//end of MSG_KEYDOWN
			break;
	}//end of switch


    return DefaultDialogProc (hdlg_timedate, message, wparam, lparam);
}


