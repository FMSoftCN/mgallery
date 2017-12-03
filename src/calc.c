/*
** $Id$
**
** calc.c:  Implement the GUI of normal and science calc.  
**
** Copyright (C) 2006 Feynman Software.
**
** All right reserved by Feynman Software.
**
** Current maintainer: Wang Xuguang.
**
** Create date: 2006/11/17
*/

#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mgext.h>

#include "../include/text_id.h"
#include "../include/pmp.h"


#undef  _LARGE_SCREEN
#define PDA_SIZE_320x240

#include "calc_type.h"
#include "calc_size.h"
#include "calc_btn_code.h"
//--------------------------------------------------------------


#define IDC_STATIC_DP	200
#define IDC_CHK_INV		202
#define IDC_CHK_HYP		203
#define IDC_BTN_BASE	204
#define IDC_BTN_ANG		205
//#define IDC_BTN_UNIT	206
//#define IDC_BTN_EXIT	207
//--------------------------------------------------------------

#define CALC_COMPACT    1
#define CALC_SCIEN      2


#define ROW		5
#define COL     6
#define ROW_SC  7
#define COL_SC  6

char calc_strdisp[50];//FIXME ADD

static BITMAP * pbackground;
/*caption in buttions of simple calculator*/
static char *Caption[ROW][COL] = {
    {"BS", "CE", "AC", "", "", ""}, 
    {"7", "8", "9", "/", "^/", "MC"}, 
    {"4", "5", "6", "x", "%", "MR"}, 
    {"1", "2", "3", "-", "1/x", "MS"}, 
    {".", "0", "=", "+", "+/-", "M+"} 
};

/*caption in buttons of scientific calculator*/
static char *Caption_Sc[ROW_SC][COL_SC] = {
    {"BS", "CE", "AC", "", "", ""}, 
    {"M+", "MS", "MR", "MC", "", ""}, 
    {"And", "Or", "Not", "Lsh", "Log", "Ln"},
    {"Sin", "Cos", "Tan", "/", "^/", "x^2"}, 
    {"D", "E", "F", "x", "%", "x^y"}, 
    {"A", "B", "C", "-", "1/x", "n!"}, 
    {"(", ")", "=", "+", "+/-", "Mod"} 
};

/*button IDs of simple calculator*/
static int BtId[ROW][COL] = {
    {CODE_BS, CODE_CE, CODE_AC, 0, 0, 0},
    {CODE_7, CODE_8, CODE_9, CODE_DIVIDE, CODE_SQRT, CODE_MC},
    {CODE_4, CODE_5, CODE_6, CODE_MULTIPLY, CODE_PERCENT, CODE_MR},
    {CODE_1, CODE_2, CODE_3, CODE_SUBTRACT, CODE_RECIP, CODE_MS},
    {CODE_POINT, CODE_0, CODE_EQ, CODE_ADD, CODE_PLUSMINUS, CODE_MPLUS}    
};

/*button IDs of scientific calculator*/
static int BtId_Sc[ROW_SC][COL_SC] = {
    {CODE_BS, CODE_CE, CODE_AC, 0, 0, 0},
    {CODE_MPLUS, CODE_MS, CODE_MR, CODE_MC, 0, 0},
    {CODE_AND, CODE_OR, CODE_NOT, CODE_LSH, CODE_LOG, CODE_LN},
    {CODE_SIN, CODE_COS, CODE_TAN, CODE_DIVIDE, CODE_SQRT, CODE_SQ},
    {CODE_D, CODE_E, CODE_F, CODE_MULTIPLY, CODE_PERCENT, CODE_XY},
    {CODE_A, CODE_B, CODE_C, CODE_SUBTRACT, CODE_RECIP, CODE_FAC},
    {CODE_LEFTPAREN, CODE_RIGHTPAREN, CODE_EQ, CODE_ADD, CODE_PLUSMINUS, CODE_MOD}    
};

HWND calc_hWndStaticM;
RECT calc_rcArrow;
static int b_h, b_w, hd, vd, begin_x, begin_y, row, col;
static int cur_row, cur_col;
static BOOL cur_isctrl;


int calc_Status = CALC_COMPACT;
Num_Base calc_cur_base = BASE_DEC;
Angle_Type calc_cur_angle = ANG_DEG;
BOOL calc_hyp_mode = FALSE, calc_inv_mode = FALSE, calc_percent_mod = FALSE;

extern int display_error;
extern DIGIT_STACK digit_stack;


int calc_pre_btnid = -1;

extern FLOAT last_opnd;
extern FLOAT dispnow;
int calc_last_optr;

extern BOOL UnitDialogBox (HWND);

static HWND  hChkInv, hChkHyp, hBtnAng, hBtnBase, hBtnExit;
//static RECT rcBtn;
//static BOOL bt_down = FALSE;
void InputProc (HWND hwnd, int btnid);
void clearzero (char*);
BOOL digit_stack_empty(void);
void InitCalculator (void);
void FreeStack(void);
int longint_bin(char*, long, int);
void clear_digit_stack (void);
//---------------------------------------------------------------------------
//
// get id of the pressed button

inline void calc_InvalidDiaplayRect(HWND hwnd) 
{
    RECT rc;
    rc.left = DISPLAY_L;
    rc.right = DISPLAY_R;
    rc.top = DISPLAY_T;
    rc.bottom = DISPLAY_B;

    InvalidateRect(hwnd, &rc, FALSE);
}

static int GetIdFromPos (int cur_row, int cur_col)
{
    if (calc_Status == CALC_COMPACT)
        return BtId[cur_row][cur_col];
    else if (calc_Status == CALC_SCIEN)
        return BtId_Sc[cur_row][cur_col];
    else 
        return -1;
}

static int GetIDCtrlFromPos(int cur_row, int cur_col)
{
	if (calc_Status == CALC_COMPACT)
	{
		return 0;
	}

	if (calc_Status == CALC_SCIEN)
	{
		switch (cur_row)
		{
			case 0:
			{
				switch(cur_col)
				{
			//		case 3:
			//			return IDC_BTN_EXIT;
					case 4:
						return IDC_CHK_INV;
					case 5:
						return IDC_CHK_HYP;
					default:
						return 0;
				}
			}
			case 1:
			{
				switch(cur_col)
				{
					case 4:
						return IDC_BTN_BASE;
					case 5:
						return IDC_BTN_ANG;
					default:
						return 0;
				}
			}

			default:
			{
				return 0;
			}
		}
	}

    assert(0);
    return 0;
    
}

/*坐标》按钮上的字*/
static int GetCaptFromPos (int cur_row, int cur_col, char *caption)
{
    if (!caption)
        return -1;
    if (cur_row >= row || cur_col >= col) 
        return -1;
    if (calc_Status == CALC_COMPACT)
        strcpy (caption, Caption[cur_row][cur_col]);
    else if (calc_Status == CALC_SCIEN)
        strcpy (caption, Caption_Sc[cur_row][cur_col]);
    else 
        return -1;

    return 0;
}
static void move_up(void)
{
	/*up, while is not butten and is not control*/
	do
	{
		cur_row = (cur_row - 1 + row) % row;
	}
	while(GetIdFromPos(cur_row, cur_col)==0 && 
			GetIDCtrlFromPos(cur_row, cur_col)==0);

	/*certain it is control or not*/
	if (GetIDCtrlFromPos(cur_row, cur_col) > 0)
	{
		cur_isctrl = TRUE;
	}
	else
	{
		cur_isctrl = FALSE;
	}

}


static void move_down(void)
{
	/*down, while is not butten and is not control*/
	do
	{
		cur_row = (cur_row + 1) % row;
	}
	while(GetIdFromPos(cur_row, cur_col)==0 && 
			GetIDCtrlFromPos(cur_row, cur_col)==0);

	/*certain it is control or not*/
	if (GetIDCtrlFromPos(cur_row, cur_col) > 0)
	{
		cur_isctrl = TRUE;
	}
	else
	{
		cur_isctrl = FALSE;
	}

}


static void move_left(void)
{
	/*left, while is not butten and is not control*/
	do
	{
		cur_col = (cur_col - 1 + col) % col;
	}
	while(GetIdFromPos(cur_row, cur_col)==0 && 
			GetIDCtrlFromPos(cur_row, cur_col)==0);

	/*certain it is control or not*/
	if (GetIDCtrlFromPos(cur_row, cur_col) > 0)
	{
		cur_isctrl = TRUE;
	}
	else
	{
		cur_isctrl = FALSE;
	}

}


static void move_right(void)
{
	/*up, while is not butten and is not control*/
	do
	{
		cur_col = (cur_col + 1) % col;
	}
	while(GetIdFromPos(cur_row, cur_col)==0 && 
			GetIDCtrlFromPos(cur_row, cur_col)==0);

	/*certain it is control or not*/
	if (GetIDCtrlFromPos(cur_row, cur_col) > 0)
	{
		cur_isctrl = TRUE;
	}
	else
	{
		cur_isctrl = FALSE;
	}

}



static int GetRectByPos (RECT *prcBtn, int cur_row, int cur_col)
{
    if (cur_row >= row || cur_col >= col) return -1;

    if (calc_Status == CALC_COMPACT) 
	{
        if (strcmp (Caption[cur_row][cur_col], "") == 0) return -1;
    }
    else if (calc_Status == CALC_SCIEN)
    {
        if (strcmp (Caption_Sc[cur_row][cur_col], "") == 0) return -1;
    }

    prcBtn->top = begin_y + (b_h + vd) * cur_row;
    prcBtn->left = begin_x + (b_w + hd) * cur_col;

    prcBtn->bottom = prcBtn->top + b_h;
    prcBtn->right = prcBtn->left + b_w;

    return 0;
}


/*decide layout of calculatora*/
int DecideSize (void)
{
	/*simpule calculator*/
	if (calc_Status == CALC_COMPACT) {
        b_h = B_H;
        vd = VD;
        b_w = B_W;
        hd = HD;
        begin_x = BEGIN_X;
        begin_y = BEGIN_Y;
        row = ROW;
        col = COL;
    }
	/*scientific calculator*/
    else {
        b_h = B_H_SC;
        vd = VD_SC;
        b_w = B_W_SC;
        hd = HD_SC;
        begin_x = BEGIN_X_SC;
        begin_y = BEGIN_Y_SC;
        row = ROW_SC;
        col = COL_SC;
    }

    return 0;
}

/* 
 * TextOutCenter: Output string in the center of a rectangle.
 * Params       : hdc - the handle of  device context
 *                preText - the rectangle where the string will be outputed
 * Return       : void
 */ 
static void TextOutCenter (HDC hdc, RECT* prcText, char* pchText)
{
    int bkMode;
	/*output the string*/
    bkMode = SetBkMode (hdc, BM_TRANSPARENT);
    //if (pchText) TextOut (hdc, x, y, pchText);
    if (pchText) DrawText (hdc, pchText, -1, prcText, DT_SINGLELINE|DT_VCENTER|DT_CENTER|DT_NOCLIP);
    SetBkMode (hdc, bkMode);
}

/*
 * DrawButton: draw the button. 
 *			  called when the button is pressed and loosen
 * Params	: hdc - handle of device context
 *			  prcBtn - the point of button rectangle
 *			  status - the button is pressed or loosen
 * return	: void
 */
static void  DrawButton (HDC hdc, RECT *prcBtn, int status)
{
    int w = 2;
    SetPenColor (hdc , PIXEL_black);    

    MoveTo (hdc, prcBtn->left + w, prcBtn->top);
    LineTo (hdc, prcBtn->right - w, prcBtn->top);
    LineTo (hdc, prcBtn->right, prcBtn->top + w);
    LineTo (hdc, prcBtn->right, prcBtn->bottom - w);
    LineTo (hdc, prcBtn->right - w, prcBtn->bottom);
    LineTo (hdc, prcBtn->left + w, prcBtn->bottom);
    LineTo (hdc, prcBtn->left, prcBtn->bottom - w);
    LineTo (hdc, prcBtn->left, prcBtn->top + w);
    LineTo (hdc, prcBtn->left + w, prcBtn->top);

    if (status == 1) { //pressed
        MoveTo (hdc, prcBtn->left + 1, prcBtn->bottom - w);
        LineTo (hdc, prcBtn->left + 1, prcBtn->top + w);
        LineTo (hdc, prcBtn->left + w, prcBtn->top + 1);
        LineTo (hdc, prcBtn->right - w + 1, prcBtn->top + 1);
    }
    else if (status == 0) { //loosen
       MoveTo (hdc, prcBtn->right+1, prcBtn->top + w + 1);
        LineTo (hdc, prcBtn->right+1, prcBtn->bottom - w);
        LineTo (hdc, prcBtn->right - w, prcBtn->bottom + 1);
        LineTo (hdc, prcBtn->left + w + 1, prcBtn->bottom + 1);
    }
}
/***change1*/
/*坐标－》按键－》按键的标题*/
static int GetCaptFromPos (int cur_row, int cur_col, char *caption);


static void HiliteButton (HDC hdc, int cur_row, int cur_col)
{
    RECT rcDown, rcText;
    char caption[10];
	int top =  begin_y + (b_h + vd) * cur_row;
	int left = begin_x + (b_w + hd) * cur_col;
	int bottom = top + b_h;
	int right = left + b_w;

    SetRect (&rcDown, left, top, right+1, bottom+1);
    SetRect (&rcText, left+1, top+1, right+1, bottom+1);

    SetBrushColor (hdc, RGB2Pixel(hdc, 0xd3, 0xc6, 0xef));
    FillBox (hdc, left, top, (right-left+1), (bottom-top+1));

    DrawButton (hdc, &rcDown, -1);

    GetCaptFromPos (cur_row, cur_col, caption);
    TextOutCenter (hdc, &rcText, caption);
}

static void select_cur_color(HWND hwnd, HDC hdc, 
				gal_pixel btn_color, gal_pixel ctrl_color)
{
    char caption[10];
	RECT btn_rect;

	if (cur_isctrl == FALSE)
	{
		GetRectByPos(&btn_rect, cur_row, cur_col);

		/*fill box with color*/
		SetBrushColor(hdc, btn_color);
		FillBox(hdc, btn_rect.left, btn_rect.top,
			   	(btn_rect.right - btn_rect.left), 
				(btn_rect.bottom - btn_rect.top));

		DrawButton (hdc, &btn_rect, 0);

		GetCaptFromPos (cur_row, cur_col, caption);
	    TextOutCenter (hdc, &btn_rect, caption);
	}
	else
	{
		HWND hctrl = GetDlgItem(hwnd, 
						GetIDCtrlFromPos(cur_row, cur_col));
		SetWindowElementColorEx(hctrl, FGC_BUTTON_NORMAL, ctrl_color);
		UpdateWindow(hctrl, TRUE);
	}
}

static void select_cur(HWND hwnd, HDC hdc)
{
    gal_pixel pixel = RGB2Pixel(hdc, 71, 156, 210);

	select_cur_color(hwnd, hdc, pixel, pixel);   //COLOR_blue, COLOR_blue);
}

static void unselect_cur(HWND hwnd, HDC hdc)
{
	select_cur_color(hwnd, hdc, COLOR_lightwhite, COLOR_black);
}

void DrawTabButton (HDC hdc)
{
    SetPenColor (hdc, PIXEL_black);

    MoveTo (hdc, calc_rcArrow.left, calc_rcArrow.top);
    LineTo (hdc, calc_rcArrow.right-3, calc_rcArrow.top);
    LineTo (hdc, calc_rcArrow.right-3, calc_rcArrow.bottom-3);
    LineTo (hdc, calc_rcArrow.left, calc_rcArrow.bottom-3);
    LineTo (hdc, calc_rcArrow.left, calc_rcArrow.top);
    
    MoveTo (hdc, calc_rcArrow.right-3, calc_rcArrow.top+3);
    LineTo (hdc, calc_rcArrow.right, calc_rcArrow.top+3);
    LineTo (hdc, calc_rcArrow.right, calc_rcArrow.bottom);
    LineTo (hdc, calc_rcArrow.left+3, calc_rcArrow.bottom);
    LineTo (hdc, calc_rcArrow.left+3, calc_rcArrow.bottom-3);

}

static int DrawInterface (HDC hdc)
{
    RECT rcBtn;
    int i, j, w, h;
    
    DrawTabButton (hdc);

    h = b_h + vd;
    w = b_w + hd;
        
    for (i = 0; i < row; i++) {
        for (j = 0; j < col; j++) {
        char *cap = NULL;
        rcBtn.top = begin_y + i * h;
        rcBtn.left = begin_x + j * w;
        rcBtn.right = rcBtn.left + b_w;
        rcBtn.bottom = rcBtn.top + b_h;
        if (calc_Status == CALC_COMPACT)
            cap = Caption[i][j];
        else if (calc_Status == CALC_SCIEN)
            cap = Caption_Sc[i][j];
        if (strlen (cap) > 0) {
			SetBrushColor(hdc, COLOR_lightwhite);
			FillBox(hdc, rcBtn.left, rcBtn.top, (rcBtn.right-rcBtn.left), (rcBtn.bottom-rcBtn.top));
            DrawButton (hdc, &rcBtn, 0);
            TextOutCenter (hdc, &rcBtn, cap);
        }
        }
    }

    return 0;
}


// show the value of last_opnd
void show_var (HWND hwnd)        
{
//FIXME DEL    char calc_strdisp[50];
    

    if (display_error) {
        strcpy(calc_strdisp, "E"); //FIXME ADD
        calc_InvalidDiaplayRect(hwnd);
        return; 
    }

    if (calc_cur_base != BASE_DEC) {
        FLOAT i_opnd;

        MODF (last_opnd, &i_opnd);
        if (i_opnd < LONG_MIN || i_opnd > ULONG_MAX) {
            fprintf (stderr, "show_var: %f outside the ulong scope\n", i_opnd);
            display_error = 1;
        }
        else if (i_opnd > LONG_MAX) {
            last_opnd = (long) (i_opnd + LONG_MIN - LONG_MAX - 1);
        }
        else {
            last_opnd = (long) i_opnd;
        }
    }
    
    if (display_error) {
        strcpy(calc_strdisp, "E");
        calc_InvalidDiaplayRect(hwnd);
        return;
    }

    if (calc_cur_base == BASE_DEC) {
        char outformat[20];
        FLOAT abs_last_opnd = FABS (last_opnd);
        if (abs_last_opnd >= 1e12 || (abs_last_opnd > 0 && abs_last_opnd < 1e-12))
            sprintf (outformat, "%%.%dE", EFF_NUM - 1);
        else if (abs_last_opnd < 1){
            sprintf (outformat, "%%.%df", EFF_NUM);
        }
        else {
            char buff[EFF_NUM + 5];
            sprintf (buff, "%d", (int) abs_last_opnd);  // to do 
            sprintf (outformat, "%%.%df", EFF_NUM - strlen (buff));
        }
        sprintf (calc_strdisp, outformat, last_opnd);
        clearzero (calc_strdisp);
    }
    else { //to do
        if (calc_cur_base == BASE_BIN) {
            if (longint_bin (calc_strdisp, last_opnd, 17) == -1) {
                display_error = 1;
            }
        }
        else if (calc_cur_base == BASE_OCT) {
            sprintf (calc_strdisp, "%lo", (long)last_opnd);    
        }
        else if (calc_cur_base == BASE_HEX) {
            sprintf (calc_strdisp, "%lX", (long)last_opnd);    
        }
    }

    if (display_error)
    {
        strcpy(calc_strdisp, "E");
    }
    else
    {
        strcpy(calc_strdisp, calc_strdisp);
    }

    calc_InvalidDiaplayRect(hwnd);
    dispnow = last_opnd;
}

// show the digit stack---show input
void show_digit_stack(HWND hwnd)
{
    char strzero[50], *pstrzero;
    int *ptmp;

    if (display_error) {
        strcpy(calc_strdisp, "E");
        calc_InvalidDiaplayRect(hwnd);
        return;
    }
    if (digit_stack_empty ()) {
        dispnow = 0;
        strcpy(calc_strdisp, "0");
        calc_InvalidDiaplayRect(hwnd);
        return;
    }
    if (digit_stack.eNum != 0) {
        show_var (hwnd);
        return;    
    }    

    pstrzero = strzero;    
    for(ptmp = digit_stack.base; ptmp <= digit_stack.top - 1; 
                    ptmp++, pstrzero++) {
        if (ptmp == digit_stack.base && *ptmp == DIGIT_POINT) {
            *pstrzero = '0';
            pstrzero++;
        }
        if (*ptmp >= 0 && *ptmp <= 9) *pstrzero = *ptmp + 48;
        if (*ptmp >= 10 && *ptmp <= 15) *pstrzero = *ptmp + 55;
        else if (*ptmp == DIGIT_POINT) *pstrzero = '.';
    }
    *pstrzero = '\0';
    strcpy(calc_strdisp, strzero);
    calc_InvalidDiaplayRect(hwnd);
    dispnow = last_opnd;
}


void enter_key(HWND hwnd)
{
	/*drawed button*/
	if (GetIdFromPos(cur_row, cur_col) > 0)
	{
		HDC hdc = GetClientDC (hwnd);
		HiliteButton (hdc, cur_row, cur_col);
		InputProc(hwnd, GetIdFromPos(cur_row, cur_col));
		ReleaseDC (hdc);
	}
	/*control button*/
	else
	{
		int idc = GetIDCtrlFromPos(cur_row, cur_col);
		int hbtn = GetDlgItem(hwnd, idc);
		if (idc == IDC_BTN_BASE || idc == IDC_BTN_ANG) 
		//		idc == IDC_BTN_EXIT)
		{
			SendMessage(hbtn, BM_CLICK, 0, 0);
		}
		else
		{
			if (SendMessage(hbtn, BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				SendMessage(hbtn, BM_SETCHECK, BST_UNCHECKED, 0);
			}
			else
			{
				SendMessage(hbtn, BM_SETCHECK, BST_CHECKED, 0);
			}
				
		}

	}
}
// ------------------------------------------------------------------------

static BOOL calcOnCreate (HWND hWnd)
{

    strcpy(calc_strdisp, "0");
    calc_InvalidDiaplayRect(hWnd);

    calc_hWndStaticM = CreateWindowEx ("static", "", 
            WS_VISIBLE | SS_LEFT, WS_EX_TRANSPARENT, IDC_STATIC_DP,
            SM_X, SM_Y, SM_W, SM_H, hWnd, 0);
    SetWindowBkColor (calc_hWndStaticM, PIXEL_lightwhite);    


    hChkInv = CreateWindow ("button", "Inv",
            WS_CHILD 
            | BS_AUTOCHECKBOX | BS_RIGHT,
            IDC_CHK_INV,
            CHK_INV_X, CHK_INV_Y,
            CHK_INV_W, CHK_INV_H+4,
            hWnd, 0);

    hChkHyp = CreateWindow ("button", "Hyp",
            WS_CHILD
            | BS_AUTOCHECKBOX | BS_RIGHT,
            IDC_CHK_HYP,
            CHK_HYP_X, CHK_HYP_Y,
            CHK_HYP_W, CHK_HYP_H+4,
            hWnd, 0);

    hBtnBase = CreateWindow ("button", "Dec",
            WS_CHILD | BS_PUSHBUTTON,
            IDC_BTN_BASE,
            BTN_BASE_X+2, BTN_BASE_Y,
            BTN_BASE_W, BTN_BASE_H,
            hWnd, 0);

    hBtnAng = CreateWindow ("button", "Deg",
            WS_CHILD | BS_PUSHBUTTON,
            IDC_BTN_ANG,
            BTN_ANG_X, BTN_ANG_Y,
            BTN_ANG_W, BTN_ANG_H,
            hWnd, 0);
    InitCalculator ();

    return TRUE;
}

static BOOL calcOnDestroy (HWND hWnd)
{
    FreeStack ();

    return TRUE;
}

void trans_status(HWND hWnd)
{
	switch (calc_Status) 
	{
        case CALC_COMPACT:
            SetWindowCaption (hWnd, ("Scientific"));
            ShowWindow (hChkInv, SW_SHOW);
            ShowWindow (hChkHyp, SW_SHOW);
            ShowWindow (hBtnBase, SW_SHOW);
            ShowWindow (hBtnAng, SW_SHOW);
			ShowWindow (hBtnExit, SW_SHOW);
            calc_Status = CALC_SCIEN;
            break;
        case CALC_SCIEN:
            SetWindowCaption (hWnd, ("Simple"));
            ShowWindow (hChkInv, SW_HIDE);
            ShowWindow (hChkHyp, SW_HIDE);
            ShowWindow (hBtnBase, SW_HIDE);
            ShowWindow (hBtnAng, SW_HIDE);
			ShowWindow (hBtnExit, SW_HIDE);
            calc_Status = CALC_COMPACT;
            break;
    }
    DecideSize ();
    UpdateWindow (hWnd, TRUE);
}
// ------------------------------------------------------------------------
static int CalcWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;

    switch (message) {
        case MSG_NCCREATE:
            SetWindowAdditionalData2 (hWnd, 0);
            break;

        case MSG_CREATE:

            pmp_app_notify_opened (PMP_APP_CALCULATOR, hWnd);

            calcOnCreate(hWnd); 
            cur_row = 0;
            cur_col = 0;
            UpdateWindow(hWnd, TRUE);
            /*select the first*/
            hdc = GetClientDC(hWnd);
            select_cur(hWnd, hdc);
            ReleaseDC(hdc);
            break;

        case MSG_SETFOCUS:
            pmp_set_captionbar_title(_(IDS_PMP_CALC_TITLE)/*"calc"*/);
            break;

        case MSG_KEYUP:
            {
                if (wParam == SCANCODE_ENTER)
                {
                    HDC hdc = GetClientDC(hWnd);
                    select_cur(hWnd, hdc);
                    ReleaseDC(hdc);
                }
            }
            break;
        case MSG_KEYDOWN:
            switch(wParam)
            {
                HDC hdc;

                case SCANCODE_CURSORBLOCKUP:
                hdc = GetClientDC(hWnd);
                unselect_cur(hWnd, hdc);
                move_up();
                select_cur(hWnd, hdc);	
                ReleaseDC(hdc);
                return 0;
                break;

                case SCANCODE_CURSORBLOCKDOWN:
                hdc = GetClientDC(hWnd);
                unselect_cur(hWnd, hdc);
                move_down();
                select_cur(hWnd, hdc);	
                ReleaseDC(hdc);
                return 0;
                break;

                case SCANCODE_CURSORBLOCKLEFT:
                hdc = GetClientDC(hWnd);
                unselect_cur(hWnd, hdc);
                move_left();
                select_cur(hWnd, hdc);	
                ReleaseDC(hdc);
                return 0;
                break;

                case SCANCODE_CURSORBLOCKRIGHT:
                hdc = GetClientDC(hWnd);
                unselect_cur(hWnd, hdc);
                move_right();
                select_cur(hWnd, hdc);	
                ReleaseDC(hdc);
                return 0;
                break;

                case SCANCODE_F1:
                cur_row = 0;
                cur_col = 0;
                trans_status(hWnd);
                hdc = GetClientDC(hWnd);
                select_cur(hWnd, hdc);
                ReleaseDC(hdc);
                break;

                case SCANCODE_ENTER:
                enter_key(hWnd);
                break;
                case SCANCODE_ESCAPE:
                calcOnDestroy(hWnd);
                DestroyMainWindow (hWnd);
                MainWindowThreadCleanup (hWnd);
                pmp_image_res_release_bitmap("/res/calc/calc.png", pbackground);
                pmp_app_notify_closed (PMP_APP_CALCULATOR);
                return 0;
            }
            break;

        case MSG_PAINT:
            {
                int i;
                //char* p_e;
                HDC hdc = BeginPaint(hWnd);
                DrawInterface (hdc);
                select_cur(hWnd, hdc);	
#ifdef DEBUG
                printf("DrawInterface\n");
#endif
                printf("m==========================\n");
                printf("get MSG_PAINT\n");

                
                printf("calc_strdisp = <%s>\n", calc_strdisp);
                if (strcmp(calc_strdisp, "inf") == 0
                        ||strcmp(calc_strdisp, "INF") ==0)
                {
                    strcpy(calc_strdisp, "E");
                }

                i = strlen(calc_strdisp);
                assert(i<=18);
                
                printf("calc_strdisp = <%s>", calc_strdisp);
                memmove(calc_strdisp+(18-i), calc_strdisp, i+1);
                memset(calc_strdisp, ' ',  18-i);
                printf("calc_strdisp = <%s>\n\n", calc_strdisp);

                pmp_display_led_digits(hdc, DISPLAY_L, DISPLAY_T, calc_strdisp);
                EndPaint(hWnd, hdc);
            }
            break;

        case MSG_COMMAND:
            {
                int id, code;
                id = LOWORD (wParam);
                code = HIWORD (wParam);    

                switch (id) {
                    case IDC_CHK_INV:
                        switch (code) {
                            case BN_CLICKED:
                                calc_inv_mode = !calc_inv_mode;
                                break;
                        }
                        break;
                    case IDC_CHK_HYP:
                        switch (code) {
                            case BN_CLICKED:
                                calc_hyp_mode = !calc_hyp_mode;
                                break;
                        }
                        break;
                        /*
                           case IDC_BTN_EXIT:
                           {
                           calcOnDestroy(hWnd);
                           DestroyMainWindow (hWnd);
                           PostQuitMessage (hWnd);
                           return 0;
                           }
                         */
                    case IDC_BTN_BASE:
                        {
                            switch (calc_cur_base) {
                                case BASE_DEC:
                                    SetWindowText (hBtnBase, "Hex");
                                    calc_cur_base = BASE_HEX;
                                    break;
                                case BASE_HEX:
                                    SetWindowText (hBtnBase, "Oct");
                                    calc_cur_base = BASE_OCT;
                                    break;
                                case BASE_OCT:
                                    SetWindowText (hBtnBase, "Bin");
                                    calc_cur_base = BASE_BIN;
                                    break;
                                case BASE_BIN:
                                    SetWindowText (hBtnBase, "Dec");
                                    calc_cur_base = BASE_DEC;
                                    break;
                            }        
                            clear_digit_stack ();
                            show_var (hWnd);
                        }
                        break;

                    case IDC_BTN_ANG:
                        {
                            switch (calc_cur_angle) {
                                case ANG_DEG:
                                    SetWindowText (hBtnAng, "Rad");
                                    calc_cur_angle = ANG_RAD;
                                    break;
                                case ANG_RAD:
                                    SetWindowText (hBtnAng, "Gra");
                                    calc_cur_angle = ANG_GRA;
                                    break;
                                case ANG_GRA:
                                    SetWindowText (hBtnAng, "Deg");
                                    calc_cur_angle = ANG_DEG;
                                    break;
                            }
                        }
                        break;
                }
            }
            break;        
    }

    return pmp_def_wnd_proc (hWnd, message, wParam, lParam);
}

static HWND create_calc_window (HWND hosting)
{
    MAINWINCREATE CreateInfo;

    CreateInfo.dwStyle = WS_VISIBLE ; //| WS_BORDER ;
    CreateInfo.spCaption= _(IDS_PMP_CALC_TITLE);
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = CalcWinProc;

    CreateInfo.lx = 0;//fh_mainwindow_lx;
    CreateInfo.ty = 20;//fh_mainwindow_ty;
    CreateInfo.rx = 320;//fh_mainwindow_rx;
    CreateInfo.by = 240;//fh_mainwindow_by;
    
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = hosting;

    return CreateMainWindow(&CreateInfo);
}

//FHAS_APP_STD_INDIRECT_INTL(calc, create_calc_window, fhas_pda_domains)
void  ex_calc_show_window (HWND hwnd)
{
    HWND hMainWnd;
	
    printf("DISPLAY_L = %d\n", DISPLAY_L);
    printf("DISPLAY_R = %d\n", DISPLAY_R);
    printf("DISPLAY_T = %d\n", DISPLAY_T);
    printf("DISPLAY_B = %d\n", DISPLAY_B);

    pbackground = pmp_image_res_acquire_bitmap("/res/calc/calc.png");
    
    hMainWnd = create_calc_window(hwnd);
    if (hMainWnd == HWND_INVALID)
	{
        pmp_image_res_release_bitmap("/res/calc/calc.png", pbackground);
		return ;
	}


    ShowWindow(hMainWnd, SW_SHOWNORMAL);

}
