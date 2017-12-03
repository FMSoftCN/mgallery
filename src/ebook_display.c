#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <minigui/mgext.h>
#include <minigui/mywindows.h>
#include <minigui/filedlg.h>
#include "../include/pmp.h"
#include "../include/text_id.h"

#include "ebook_scrolled.h"
#include "ebook_text.h"
#include "ebook_scrollview_impl.h"
#include "ebook_textedit_impl.h"
#include "ebook_listmodel.h"

#define ID_TIMER	100
#define ptescr          (&ptedata->svdata.scrdata)

#define MIID_BASE_EBOOK 1

typedef enum
{
	AUTO_PAGE = 0,
	PAGE_TIME,
	MARK_TAG,
}menu_item_t;


typedef struct{
	BOOL auto_page;
	int page_time;
}book_status_t; 


extern struct msg_domain *pmp_domains[];

extern int pmp_ebook_start_read(char* file_name, LOGFONT* font, int* line);
extern void pmp_ebook_end_read(void);
extern char* pmp_ebook_pre_read(void);
extern char* pmp_ebook_next_read(void);
extern char* pmp_ebook_cur_read(void);
extern int  parse_file_to_blk(LOGFONT* font);
extern void pmp_ebook_tag(int line);
//extern int select_page_time(HWND h_book);
//static BOOL open_file(HWND hMLEditWnd, char* filefullname);

BOOL is_scroll_top(HWND hwnd);
BOOL is_scroll_bottom(HWND hwnd);
void set_scroll_bottom(HWND hMLEditWnd);
static BOOL have_scoll_bar(HWND hwnd);
static int pmp_edit_proc(HWND hwnd, int message, WPARAM wparam, LPARAM lparam);
static void _ebook_show_menu(HWND parent, HWND hMLEditWnd, book_status_t* book_status);

static PMP_MEDIA_FILE_LIST* _ebook_show_files(HWND hwnd);

static int line;
static WNDPROC old_edit_proc;

static char fullname[PATH_MAX];

static  book_status_t book_status =
{
	FALSE,
	5,
};

static int ebookWinProc(HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
    static HWND hMLEditWnd;
	static char* buffer;
#if 0
	static PLOGFONT font_gb;
	static PLOGFONT font_utf8;
	static PLOGFONT font_big5;
	static PLOGFONT font_arry[3];
	static int font_flag;
#endif
	static int close_stat = 0;
	static PMP_MEDIA_FILE_LIST* file_list = NULL;
    switch (message) 
	{
		case MSG_CREATE:
            pmp_app_notify_opened(PMP_APP_EBOOK, hwnd);
			
            file_list = _ebook_show_files(hwnd);
			if (file_list == NULL)
			{
				SendMessage(hwnd, MSG_CLOSE, 0, 0);
				return 0;
			}

            /*
			while (file_list->file_names == NULL
					||*(file_list->file_names) == '\0')
			{
				PMP_RELEASE_MEDIA_FILE_LIST(file_list);
				file_list = _ebook_show_files(hwnd);
			}
            */

			strcpy(fullname, file_list->dir);
			strcat(fullname, file_list->file_names);
			printf("%s\n", fullname);

			hMLEditWnd = CreateWindowEx (CTRL_MEDIT, "",  
				//	WS_CHILD|ES_BASELINE|WS_VSCROLL|ES_AUTOWRAP|ES_READONLY, 
					WS_BORDER|WS_CHILD|WS_VISIBLE|ES_BASELINE|WS_VSCROLL|ES_AUTOWRAP|ES_READONLY, 
					0, 
					0, 
					0, 0, 320, 218, 
					hwnd, 0);
			old_edit_proc = SetWindowCallbackProc(hMLEditWnd, pmp_edit_proc);
			
			SendMessage(hMLEditWnd, EM_SETLINEHEIGHT, 20, 0);

#if 0
			font_utf8 = GetSystemFont(SYSLOGFONT_WCHAR_DEF);

			font_gb = CreateLogFont (NULL, "song", "GB2312",
					FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, 
					FONT_FLIP_NIL, FONT_OTHER_NIL, 
					FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
					4, 0);
			
			font_utf8 = CreateLogFont (NULL, "song", "utf-8",
					FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, 
					FONT_FLIP_NIL, FONT_OTHER_NIL, 
					FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
					4, 0);
			font_big5 = CreateLogFont (NULL, "song", "big5",
					FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, 
					FONT_FLIP_NIL, FONT_OTHER_NIL, 
					FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
					4, 0);

			font_arry[0]=font_gb;
			font_arry[1]=font_big5;
			font_arry[2]=font_utf8;
			font_flag = 0;
			
			SetWindowFont(hMLEditWnd, font_arry[font_flag]);
#endif

			if (pmp_ebook_start_read(fullname, GetSystemFont(SYSLOGFONT_WCHAR_DEF), &line) == -1)
			{
				close_stat = 1;
                pmp_show_info(hwnd, _(IDS_PMP_EBOOK_ERROR), _(IDS_PMP_EBOOK_ERRORINFO),MB_ICONHAND);
				printf("pmp_ebook_start_read error\n");
				SendMessage(hwnd, MSG_CLOSE, 0, 0);
				return 0;
			}

			if ((buffer = pmp_ebook_cur_read()) == NULL)
			{
				close_stat = 1;
				printf("pmp_ebook_next_read error\n");
				SendMessage(hwnd, MSG_CLOSE, 0, 0);
				return 0;
			}

			SendMessage(hMLEditWnd, MSG_SETTEXT, 0, (UINT)buffer);
			EnableScrollBar(hMLEditWnd, SB_VERT, TRUE);
			if (have_scoll_bar(hMLEditWnd))
			{
				int svm_x = SendMessage(hMLEditWnd, SVM_GETCONTENTX, 0, 0);
				SendNotifyMessage(hMLEditWnd, SVM_SETCONTPOS, svm_x, line);
			}
			printf("\n\n");
			
			break;
        case MSG_SETFOCUS:
            pmp_set_captionbar_title(_(IDS_PMP_EBOOK_TITLE)/*"ebook"*/);
            break;

		case MSG_KEYDOWN:
			if (file_list == NULL) return 0;
			switch(wparam)
			{
				case SCANCODE_CURSORBLOCKUP:
					if (!have_scoll_bar(hMLEditWnd)
						||is_scroll_top(hMLEditWnd))
					{ 
#ifdef DEBUG
						printf("scroll to top\n");
#endif
						buffer = pmp_ebook_pre_read();

						SendMessage(hMLEditWnd, MSG_SETTEXT, 0, (UINT)buffer);
						set_scroll_bottom(hMLEditWnd);
					}
					else
					{
						SendMessage(hMLEditWnd, MSG_VSCROLL, SB_LINEUP, 0);
					}
					return 0;
					break;
				case SCANCODE_CURSORBLOCKDOWN:
					if (!have_scoll_bar(hMLEditWnd)
						||is_scroll_bottom(hMLEditWnd))
					{
#ifdef DEBUG
						printf("scroll to bottom\n");
#endif
						buffer = pmp_ebook_next_read();
						SendMessage(hMLEditWnd, MSG_SETTEXT, 0, (UINT)buffer);

					}
					else
					{
						SendMessage(hMLEditWnd, MSG_VSCROLL, SB_LINEDOWN, 0);
					}
					return 0;
					break;
				case SCANCODE_CURSORBLOCKLEFT:
					if (!have_scoll_bar(hMLEditWnd)
						||is_scroll_top(hMLEditWnd))
					{ 
#ifdef DEBUG
						printf("scroll to top\n");
#endif
						buffer = pmp_ebook_pre_read();
						
						SendMessage(hMLEditWnd, MSG_SETTEXT, 0, (UINT)buffer);
						set_scroll_bottom(hMLEditWnd);
					}
					else
					{
						SendMessage(hMLEditWnd, MSG_VSCROLL, SB_PAGEUP, 0);
					}
					return 0;
					break;

				case SCANCODE_CURSORBLOCKRIGHT:
					if (!have_scoll_bar(hMLEditWnd)
						||is_scroll_bottom(hMLEditWnd))
					{ 
#ifdef DEBUG
						printf("scroll to top\n");
#endif
						buffer = pmp_ebook_next_read();
						
						SendMessage(hMLEditWnd, MSG_SETTEXT, 0, (UINT)buffer);
					}
					else
					{
						SendMessage(hMLEditWnd, MSG_VSCROLL, SB_PAGEDOWN, 0);
					}
					return 0;
					break;

				case SCANCODE_F1:
					{
						_ebook_show_menu(hwnd, hMLEditWnd, &book_status);
					}
					break;

				case SCANCODE_ESCAPE:
                    PostMessage(hwnd, MSG_CLOSE, 0, 0);
                    return 0;

					break;

				default:
					break;
			}	
			return 0;
			break;	//MSG_KEYDOWN
		case MSG_TIMER:
			if (wparam == ID_TIMER)
			{
				SendMessage(hwnd, MSG_KEYDOWN, SCANCODE_CURSORBLOCKRIGHT, 0);
			}
			break;

        case MSG_ERASEBKGND:
            return 0;

		case MSG_CLOSE:
			{
				printf("get MSG_CLOSE+++++++++++++++\n");

				if (file_list) 
				{
					PMP_RELEASE_MEDIA_FILE_LIST (file_list);
				}

				if (close_stat == 0)
				{
					pmp_ebook_end_read();
				}

				if (IsTimerInstalled(hwnd, ID_TIMER))
				{
					KillTimer(hwnd, ID_TIMER);
				}

                book_status.auto_page = FALSE;

				DestroyAllControls(hwnd);
				DestroyMainWindow(hwnd);
				MainWindowThreadCleanup(hwnd);
                pmp_app_notify_closed(PMP_APP_EBOOK);
				return 0;
			}
			break;
	}

    SetWindowAdditionalData2(hwnd, 0);
    return pmp_def_wnd_proc(hwnd, message, wparam, lparam);
}


static void InitebookInfo (PMAINWINCREATE pCreateInfo, UINT add_date, HWND h_host)
{
    pCreateInfo->dwStyle =  WS_BORDER | WS_VISIBLE;     
    pCreateInfo->dwExStyle = 0;
    pCreateInfo->spCaption = "";
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor(0);
    pCreateInfo->hIcon = 0; 
    pCreateInfo->MainWindowProc = ebookWinProc;
    pCreateInfo->lx = 0; 
    pCreateInfo->ty = 20;
    pCreateInfo->rx = 320;
    pCreateInfo->by = 240;
    pCreateInfo->iBkColor = COLOR_lightgray; 
    pCreateInfo->dwAddData = add_date;
    pCreateInfo->hHosting = h_host;
}

static MAINWINCREATE create_info;

void ex_ebook_display_book(HWND h_host)
{
	HWND hwnd;
	InitebookInfo(&create_info, (UINT)fullname, h_host);
	hwnd = CreateMainWindow(&create_info);
}


static int nPos_old = -1;

BOOL is_scroll_top(HWND hwnd)
{
	int nPos;
	GetScrollPos(hwnd, SB_VERT, &nPos);
#if 0 //def DEBUG
	printf("sv_info = %d\n", nPos);
#endif 
	nPos_old = nPos;

	return (nPos == 0);
}

BOOL is_scroll_bottom(HWND hwnd)
{
	int nPos;
	GetScrollPos(hwnd, SB_VERT, &nPos);
#if 0 //def DEBUG
	printf("sv_info = %d\n", nPos);
#endif 

	if (nPos == nPos_old && nPos != 0)
	{
		return TRUE;
	}
	else
	{
		nPos_old = nPos;
		return FALSE;
	}
}

void set_scroll_bottom(HWND hwnd)
{
	int count = 0;
	count = SendMessage(hwnd, SVM_GETITEMCOUNT, 0, 0);
#if 0 //def DEBUG
	printf("the count of edit is %d\n", count);
#endif 
	SendMessage(hwnd, SVM_SHOWITEM, count-1, 0);
}


static BOOL have_scoll_bar(HWND hwnd)
{
	int cont_height = 0;
	int visi_height = 0;
	cont_height = SendMessage(hwnd, SVM_GETCONTHEIGHT, 0, 0);
	visi_height = SendMessage(hwnd, SVM_GETVISIBLEHEIGHT, 0, 0);
	if (visi_height < cont_height)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static void tePaint(HWND hwnd, HDC hdc, RECT *rcDraw)
{
    RECT *rc = rcDraw;
    PTEDATA ptedata;
    int h, indent = 0;

    ptedata = (PTEDATA)GetWindowAdditionalData2(hwnd);
    h = ptedata->nLineHeight - 1;

    if (GetWindowStyle(hwnd) & ES_BASELINE) {
        SetPenColor (hdc, GetWindowElementColorEx (hwnd, FGC_CONTROL_NORMAL));
        while (h < RECTHP(rc)) {
            indent = (h == ptedata->nLineHeight - 1) ? ptedata->titleIndent : 0;
            DrawHDotLine (hdc, rc->left + indent, rc->top+h, RECTWP(rc)-indent);
            h += ptedata->nLineHeight;
        }
    }
}

void scrollview_draw (HWND hwnd, HDC hdc, PSVDATA psvdata)
{
    list_t *me;
    PSVITEMDATA pci;
    RECT rcDraw;
    int h = 0;
    RECT rcVis;
    PSVLIST psvlist = &psvdata->svlist;

    rcDraw.left = 0;
    rcDraw.top = 0;
    rcDraw.right = psvscr->nContWidth;
    rcDraw.bottom = psvscr->nContHeight;

    scrolled_content_to_window (psvscr, &rcDraw.left, &rcDraw.top);
    scrolled_content_to_window (psvscr, &rcDraw.right, &rcDraw.bottom);

    scrolled_get_visible_rect (psvscr, &rcVis);
    ClipRectIntersect (hdc, &rcVis);

    mglist_for_each (me, psvlist) {
        pci = (PSVITEMDATA)mglist_entry (me);
        rcDraw.top += h;
        rcDraw.bottom = rcDraw.top + pci->nItemHeight;
        if (rcDraw.bottom < rcVis.top) {
            h = pci->nItemHeight;
            continue;
        }
        if (rcDraw.top > rcVis.bottom)
            break;
        if (((MgList *)psvlist)->iop.drawItem && pci->nItemHeight > 0) {
            ((MgList *)psvlist)->iop.drawItem (hwnd, (HSVITEM)pci, hdc, &rcDraw);
        }
        h = pci->nItemHeight;
    }
}

static int pmp_edit_proc(HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
	if (message == MSG_PAINT)
    {
        RECT rcDraw, rcVis;
		PTEDATA ptedata = NULL;
        HDC hdc = BeginPaint (hwnd);
		HDC mem_dc = CreateCompatibleDC(hdc);

		SetBrushColor(mem_dc, COLOR_lightwhite);
		FillBox(mem_dc, 0, 0, 320, 220);

        ptedata = (PTEDATA) GetWindowAdditionalData2 (hwnd);
        
		scrolled_get_visible_rect (ptescr, &rcVis);
        ClipRectIntersect (mem_dc, &rcVis);

        scrolled_get_content_rect (ptescr, &rcDraw);
        scrolled_visible_to_window (ptescr, &rcDraw.left, &rcDraw.top);
        scrolled_visible_to_window (ptescr, &rcDraw.right, &rcDraw.bottom);

        tePaint (hwnd, mem_dc, &rcDraw);
        scrollview_draw (hwnd, mem_dc, &ptedata->svdata);

        if ((ptedata->ex_flags & TEST_EX_SETFOCUS)){
            ptedata->ex_flags &= ~TEST_EX_SETFOCUS;
        }
        else if (ptedata->ex_flags & TEST_EX_KILLFOCUS) {
            ptedata->ex_flags &= ~TEST_EX_KILLFOCUS;
        }

		BitBlt(mem_dc, 0, 0 ,0, 0, hdc, 0, 0, 0);
		DeleteMemDC(mem_dc);
        EndPaint (hwnd, hdc);
        return 0;
    }
	else if (message == MSG_ERASEBKGND)
	{
		return 0;
	}
	return old_edit_proc(hwnd, message, wparam, lparam);
}



static int auto_page_str[] = { 
    IDS_PMP_EBOOK_OFF,
    IDS_PMP_EBOOK_ON, 
};  /*N_("OFF"), N_("ON") };*/

static int _ebook_autopage_callback (void* context, int id, int op, char *text_buff)
{
	book_status_t* book_status = (book_status_t*)context;
	switch (op) {
		case PMP_MI_OP_DEF:
			sprintf(text_buff, _(IDS_PMP_EBOOK_AUTOPAGE)/*"auto page\t%s"*/, _(auto_page_str[book_status->auto_page]));
			return PMP_MI_ST_UPDATED;

		case PMP_MI_OP_PREV:
			book_status->auto_page = (book_status->auto_page + 2 - 1) % 2;
			sprintf(text_buff, _(IDS_PMP_EBOOK_AUTOPAGE)/*"auto page\t%s"*/, _(auto_page_str[book_status->auto_page]));
			return PMP_MI_ST_UPDATED;

		case PMP_MI_OP_NEXT:
			book_status->auto_page = (book_status->auto_page + 1) % 2;
			sprintf(text_buff, _(IDS_PMP_EBOOK_AUTOPAGE)/*"auto page\t%s"*/, _(auto_page_str[book_status->auto_page]));
			return PMP_MI_ST_UPDATED;

		case PMP_MI_OP_ENTER:
			return PMP_MI_ST_SELECTED;
	}

	return PMP_MI_ST_NONE;
}

static int _ebook_pagetime_callback(void* context, int id, int op, char* text_buff)
{
	book_status_t* book_status = (book_status_t*)context;

	switch (op) {
		case PMP_MI_OP_DEF:
			sprintf(text_buff, _(IDS_PMP_EBOOK_PAGETIME)/*"page time\t%ds"*/, book_status->page_time);
			return PMP_MI_ST_UPDATED;

		case PMP_MI_OP_PREV:
			if (book_status->page_time > 5)
				book_status->page_time--;
			sprintf(text_buff, _(IDS_PMP_EBOOK_PAGETIME)/*"page time\t%ds"*/, book_status->page_time);
			return PMP_MI_ST_UPDATED;

		case PMP_MI_OP_NEXT:
			if (book_status->page_time < 20)
				book_status->page_time++;
			sprintf(text_buff, _(IDS_PMP_EBOOK_PAGETIME)/*"page time\t%ds"*/, book_status->page_time);
			return PMP_MI_ST_UPDATED;

		case PMP_MI_OP_ENTER:
			return PMP_MI_ST_SELECTED;
	}

	return PMP_MI_ST_NONE;
}


static int _ebook_marktag_callback(void* context, int id, int op, char* text_buff)
{

	switch (op) {
		case PMP_MI_OP_DEF:
			strcpy(text_buff, _(IDS_PMP_EBOOK_MARKTAG)); /*mark tag*/
			return PMP_MI_ST_UPDATED;

		case PMP_MI_OP_ENTER:
			return PMP_MI_ST_SELECTED;
	}

	return PMP_MI_ST_NONE;
}

static PMP_MENU_ITEM __ebook_menu_items[] =
{
	{AUTO_PAGE + MIID_BASE_EBOOK, _ebook_autopage_callback},
	{PAGE_TIME + MIID_BASE_EBOOK, _ebook_pagetime_callback},
	{MARK_TAG  + MIID_BASE_EBOOK, _ebook_marktag_callback},
	{0, NULL},
};

static void _ebook_show_menu(HWND parent, HWND hMLEditWnd, book_status_t* book_status)
{
	int i;
	int line;
	book_status_t menu_context = *book_status;
	i = pmp_show_menu(parent, _(IDS_PMP_EBOOK_OPTION)/*"ebook options"*/, __ebook_menu_items, &menu_context);
	i -= MIID_BASE_EBOOK;
	switch (i)
	{
		case AUTO_PAGE:
			book_status->auto_page = menu_context.auto_page;
			if (book_status->auto_page == TRUE)
			{
				if (!IsTimerInstalled(parent, ID_TIMER))
				{
					SetTimer(parent, ID_TIMER, 100*(book_status->page_time));
				}
			}
			else
			{
				if (IsTimerInstalled(parent, ID_TIMER))
				{
					KillTimer(parent, ID_TIMER);
				}
			}
			break;
	
		case PAGE_TIME:
			book_status->page_time = menu_context.page_time;
			if (book_status->auto_page == TRUE)
			{
				if (IsTimerInstalled(parent, ID_TIMER))
				{
					ResetTimer(parent, ID_TIMER, 100*(book_status->page_time));
				}
				else
				{
					SetTimer(parent, ID_TIMER, 100*(book_status->page_time));
				}
			}
			else
			{
				if (IsTimerInstalled(parent, ID_TIMER))
				{
					KillTimer(parent, ID_TIMER);
				}
			}
			break;

		case MARK_TAG:
			line = SendMessage(hMLEditWnd, SVM_GETCONTENTY, 0, 0);
            printf("ebook--to mark a tag\n");
			pmp_ebook_tag(line);	
			break;
	}
}

static BOOL _ebook_filter(const char *path, const char* file_name)
{
    char* filter_str = ".txt";
    const char* postfix = rindex(file_name, '.');
    if (postfix)
    {
        if (strcasecmp(postfix, filter_str) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;

}


static BITMAP* _ebook_file_icon;
static const BITMAP* _ebook_get_file_icon (const char *path, const char* file_name, DWORD mode)
{
	return _ebook_file_icon;
}

static PMP_MEDIA_FILE_LIST* _ebook_show_files(HWND hwnd)
{
    
    BITMAP* _ebook_folder_icon;
    PMP_MEDIA_FILE_LIST* my_list;
    PMP_DIR_SHOW_INFO dir_show_info [] = 
    {
        {"music", _(IDS_PMP_MUSIC_DIRNAME), NULL},
        {"video", _(IDS_PMP_VIDEO_DIR_ALIAS), NULL},
        {"ebook", _(IDS_PMP_EBOOK_DIRNAME), NULL},
        {"picture", _(IDS_PMP_PIC_DIR_ALIAS), NULL},
        {"recorder", _(IDS_PMP_RECORDER_DIRNAME), NULL},
        {"..", _(IDS_PMP_DOTDOT), NULL},
        {"*", "*", NULL},
        {NULL, NULL, NULL},
    };



    _ebook_folder_icon = pmp_image_res_acquire_bitmap("/res/ebook/folder.png");
    _ebook_file_icon = pmp_image_res_acquire_bitmap("/res/ebook/txt.png");

    dir_show_info[0].icon = _ebook_folder_icon;
    dir_show_info[1].icon = _ebook_folder_icon;
    dir_show_info[2].icon = _ebook_folder_icon;
    dir_show_info[3].icon = _ebook_folder_icon;
    dir_show_info[4].icon = _ebook_folder_icon;
    dir_show_info[5].icon = _ebook_folder_icon;
    dir_show_info[6].icon = _ebook_folder_icon;

    my_list = pmp_select_media_files (hwnd, PMP_MEDIA_TOP_DIR, PMP_MEDIA_NAME_EBOOK, _ebook_filter, _ebook_get_file_icon, dir_show_info, PMP_BROWSER_MODE_LIST);

    pmp_image_res_release_bitmap("/res/ebook/folder.png", _ebook_folder_icon);
    pmp_image_res_release_bitmap("/res/ebook/txt.png", _ebook_file_icon);

    return my_list;

}
