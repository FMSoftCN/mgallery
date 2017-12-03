/*
** $Id$
**
** sel_win_list.c: select file window in list mode  for Feynman PMP solution.
**
** Copyright (C) 2007 Feynman Software, all rights reserved.
**
** Use of this source package is subject to specific license terms
** from Beijing Feynman Software Technology Co., Ltd.
**
** URL: http://www.minigui.com
**
** Current maintainer: wangminggang  .
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "pmp.h"
#include <minigui/control.h>

#ifndef _ADV_2DAPI
#define RoundRect(hdc, lx, ty, rx, by, r1, r2) Rectangle(hdc, lx, ty, rx, by)
#endif

#define STATIC_LX                 0
#define STATIC_LY                 20
#define STATIC_H                  18

#define IDC_BROWSER_STATIC        100
#define IDC_BROWSER_SCROLLVIEW    101

#define SCROLLVIEW_LX             0
#define SCROLLVIEW_LY             (STATIC_H+20)

#define NAME_LEN_MAX              64
#define PATH_LEN_MAX              512
#define FULL_PATH_LEN_MAX         NAME_LEN_MAX+PATH_LEN_MAX



typedef struct _PMP_WINDOW_ADD{
     char initial_dir[PATH_LEN_MAX];
     char full_path[PATH_LEN_MAX];
     const  char *root_dir;
     PMP_DIR_SHOW_INFO* dir_show_info;
     PMP_CB_MEDIA_FILTER cb_media_filter;
     PMP_CB_MEDIA_ICON cb_media_icon;
     DWORD mode;
     PMP_MEDIA_FILE_LIST *back_file;
}PMP_WINDOW_ADD;

typedef struct _PMP_DIR_INFO{
    char   name[NAME_LEN_MAX];
    char   *show_text;
    PMP_WINDOW_ADD *add;
    const  BITMAP * icon;
    BOOL   select;
}PMP_DIR_INFO;
   

static int _pmp_file_is_dir (PMP_WINDOW_ADD *add, const char *file_name)
{
    struct stat attr;

    if (strlen(add->root_dir)+strlen(add->initial_dir)>PATH_LEN_MAX-2||strlen(file_name)>NAME_LEN_MAX-1)
        return -1;
    add->full_path[0] ='\0';
    strcat(add->full_path,add->root_dir);
    strcat(add->full_path,add->initial_dir);
    strcat(add->full_path,file_name);
    if (stat (add->full_path, &attr) < 0)
        return -1;
    if (S_ISDIR (attr.st_mode))
        return 1;
    else
        return  0;
}

static void _pmp_file_last_dir (char * s)
{
    char *p;
    int   n=0;

    if(0!=(p=strstr(s,"../"))){
        *p='\0';
        p--;

        for(;p-s>0;p--){
            if(*p=='/')
                n++;
            if(n==2)
                break;
        }

        if(n==2)
            *(p+1)='\0';
        else
            *p='\0';   
    }
}

static const BITMAP* _pmp_file_find_dir_icon (PMP_WINDOW_ADD *add, const char *dir)
{
    PMP_DIR_SHOW_INFO* show_info;

    show_info=add->dir_show_info;

    while(show_info->dir_name!=NULL){
        if(strcmp(dir,show_info->dir_name)==0)
            return show_info->icon;

        ++show_info;

    } 
    return (show_info-2)->icon;

}

static char* _pmp_file_find_dir_name (PMP_WINDOW_ADD *add, const char *dir)
{
    PMP_DIR_SHOW_INFO* show_info;
    
    show_info=add->dir_show_info;
    while(show_info->dir_name!=NULL){
        if(strcmp(dir,show_info->dir_name)==0)
            return (char *)show_info->show_text;

        ++show_info;

    } 
    return (char *)dir;
}


static void _free_scrollview_add_data (HWND hWnd)
{
    int idex;
    PMP_DIR_INFO *p; 
    idex=SendMessage(GetDlgItem(hWnd,IDC_BROWSER_SCROLLVIEW),SVM_GETITEMCOUNT,0,0); 
    while(idex){   
        p=(PMP_DIR_INFO*)SendMessage(GetDlgItem(hWnd,IDC_BROWSER_SCROLLVIEW),SVM_GETITEMADDDATA,--idex,0);
        free(p);   
    } 
}


static int _compitem (HSVITEM hsvi1, HSVITEM hsvi2)
{
    const PMP_DIR_INFO *item1 = (const PMP_DIR_INFO *) scrollview_get_item_adddata (hsvi1);
    const PMP_DIR_INFO *item2 = (const PMP_DIR_INFO *) scrollview_get_item_adddata (hsvi2);

    if (strcmp (item1->name, "..") == 0)
        return -1;
    if (strcmp (item2->name, "..") == 0)
        return 1;
    if (_pmp_file_is_dir(item1->add, item1->name) > _pmp_file_is_dir(item2->add, item2->name))
        return -1;
    if (_pmp_file_is_dir(item1->add, item1->name) < _pmp_file_is_dir(item2->add, item2->name))
        return 1;
    return strcmp (item1->name, item2->name);

}


static void _browser_drawitem (HWND hwnd, HSVITEM hsvi, HDC hdc, RECT *rcDraw)
{
    const PMP_DIR_INFO *data = (const PMP_DIR_INFO*)scrollview_get_item_adddata (hsvi);
    RECT tmp;
    int  ret;
    PMP_WINDOW_ADD *add;

    add =(PMP_WINDOW_ADD*)GetWindowAdditionalData(GetParent(hwnd));
    sprintf(add->full_path,"%s%s",add->root_dir ,add->initial_dir);
    ret=SetBkMode(hdc, BM_TRANSPARENT);
    SetTextColor(hdc, PIXEL_black);

    if (scrollview_is_item_hilight(hwnd, hsvi)) {
        SetBrushColor (hdc,RGB2Pixel(HDC_SCREEN, 71, 156, 210));
        SetPenColor (hdc, PIXEL_yellow);
        SetTextColor (hdc, PIXEL_yellow);
        RoundRect(hdc,rcDraw->left+1,rcDraw->top+1,rcDraw->right-2,rcDraw->bottom-1,2,2);

        if(_pmp_file_is_dir(add, data->name)==1){
            tmp.left = rcDraw->left + 20 + 5;
            tmp.top = rcDraw->top;
            tmp.right = tmp.left + 240;
            tmp.bottom = rcDraw->bottom;
            DrawText(hdc,"-->", -1, &tmp, DT_RIGHT | DT_WORDBREAK);
        }
    }

    if (data->select) {
        SetBrushColor (hdc, PIXEL_blue);
        FillBox (hdc, rcDraw->left+1, rcDraw->top+2, RECTWP(rcDraw)-4, RECTHP(rcDraw)-3);
        SetBkColor (hdc, PIXEL_blue);
        SetTextColor (hdc, PIXEL_lightwhite);
    }

    if(data->icon!=NULL) 
        FillBoxWithBitmap(hdc, rcDraw->left, rcDraw->top,15,15, data->icon );
    else 
        FillBoxWithBitmap(hdc, rcDraw->left, rcDraw->top,15,15,add->cb_media_icon(add->full_path,data->name,add->mode));

    tmp.left = rcDraw->left + 20 + 5;
    tmp.top = rcDraw->top;
    tmp.right = tmp.left + 240;
    tmp.bottom = rcDraw->bottom;
    DrawText(hdc,data->show_text, -1, &tmp, DT_LEFT | DT_WORDBREAK);

    return;
}


static void 
_select_add_items (HWND hParent)
{
    HWND hscroll;
    SVITEMINFO svii;
    DIR* handle = NULL;
    struct dirent* os_item = NULL;
    unsigned int ret;
    PMP_DIR_INFO   *p;
    PMP_WINDOW_ADD *add;

    add =(PMP_WINDOW_ADD*)GetWindowAdditionalData(hParent);
    hscroll = GetDlgItem (hParent, IDC_BROWSER_SCROLLVIEW);
    _pmp_file_last_dir(add->initial_dir); 
back:
    sprintf(add->full_path,"%s%s",add->root_dir ,add->initial_dir);
    handle = opendir (add->full_path);

    if (handle == NULL)
    {
        GetWindowText(GetDlgItem (hParent, IDC_BROWSER_STATIC), add->initial_dir, FULL_PATH_LEN_MAX);
        _MY_PRINTF("Open dir error.\n");
        if (add->initial_dir[0]=='\0')
            return;
        goto back;
    }


    SetWindowText (GetDlgItem (hParent, IDC_BROWSER_STATIC),add->initial_dir);
    _free_scrollview_add_data(hParent); 
    SendMessage (hscroll, SVM_RESETCONTENT, 0, 0);
    SendMessage (hscroll, MSG_FREEZECTRL, TRUE, 0);

    while ((os_item = readdir (handle)) != NULL) {
        if (os_item->d_name[0] == '.' && os_item->d_name[1] != '.')
            continue;
        if (os_item->d_name[0] == '.' && os_item->d_name[1] == '.' && strlen(add->initial_dir)==0)
            continue;

        ret=_pmp_file_is_dir(add, os_item->d_name);
        if( ret<0)
            continue;
        else
            p=malloc(sizeof(PMP_DIR_INFO));
            p->add=add;
        if(ret>0){ /*is dir*/
            p->name[0]='\0';
            p->icon= _pmp_file_find_dir_icon(add, os_item->d_name);
            p->select=FALSE;
            strcat(p->name,os_item->d_name);
            p->show_text=_pmp_file_find_dir_name(add, p->name); 
        }else
        {
            if(!add->cb_media_filter(add->full_path,os_item->d_name))
            {
                free(p);
                continue;
            }

            p->name[0]='\0';
            p->icon=NULL;
            p->select=FALSE;
            strcat(p->name,os_item->d_name); 
            p->show_text=p->name; 
        }   
        svii.nItemHeight = 15;
        svii.addData =(DWORD)p;
        svii.nItem = -1;
        SendMessage (hscroll, SVM_ADDITEM, 0, (LPARAM)&svii);
    }

    SendMessage (hscroll, MSG_FREEZECTRL, FALSE, 0);
    SetFocus(hscroll);
    SendMessage (hscroll, SVM_SETCURSEL, 0, 1);
    closedir(handle);
}

static void
_create_controls (HWND hParent)
{
    HWND hScrollView,hStatic;

    hStatic     = CreateWindowEx (CTRL_STATIC, "q",
            WS_CHILD |WS_VISIBLE |SS_LEFT, 0,
            IDC_BROWSER_STATIC,
            STATIC_LX,STATIC_LY,g_rcScr.right,STATIC_H,
            hParent, 0);


    hScrollView = CreateWindow ("scrollview", "",
            WS_CHILD |WS_VISIBLE |WS_VSCROLL | SVS_AUTOSORT |WS_BORDER,
            IDC_BROWSER_SCROLLVIEW ,
            SCROLLVIEW_LX, SCROLLVIEW_LY,MAINWINDOW_W,MAINWINDOW_H-STATIC_H-20,
            hParent, 0);

    SetWindowBkColor (hScrollView, RGB2Pixel(HDC_SCREEN, 71, 156, 210));   
    SendMessage (hScrollView, SVM_SETITEMCMP, 0, (LPARAM) _compitem);
    SendMessage (hScrollView, SVM_SETITEMDRAW, 0, (LPARAM)_browser_drawitem);

    _select_add_items(hParent);
    UpdateWindow (hScrollView, TRUE);
    SendMessage (hScrollView, SVM_SETCURSEL, 0, 1);
}

static void
_pmp_return_files (HWND hWnd)
{
    int idex;
    char *s;
    PMP_DIR_INFO *p;
    PMP_WINDOW_ADD *add;

    add =(PMP_WINDOW_ADD*)GetWindowAdditionalData(hWnd);
    if(add->mode&PMP_BROWSER_MODE_MULTIPLE){
        idex=SendMessage(GetDlgItem(hWnd,IDC_BROWSER_SCROLLVIEW),SVM_GETITEMCOUNT,0,0);    

        add->back_file=malloc(sizeof(PMP_MEDIA_FILE_LIST));
        add->back_file->nr_files=0;
        sprintf(add->back_file->dir,"%s%s",add->root_dir,add->initial_dir);
        add->back_file->file_names=malloc(1024);
        *(add->back_file->file_names)='\0';
        s=add->back_file->file_names;

        p=(PMP_DIR_INFO*)SendMessage(GetDlgItem(hWnd,IDC_BROWSER_SCROLLVIEW),SVM_GETITEMADDDATA,idex-1,0);   
        while(idex!=0){
            if(p->select==TRUE){
                sprintf(s,"%s",p->name);
                s+=strlen(s)+1; 
                add->back_file->nr_files++;
            }
            idex--;
            p=(PMP_DIR_INFO*)SendMessage(GetDlgItem(hWnd,IDC_BROWSER_SCROLLVIEW),SVM_GETITEMADDDATA,idex-1,0);   
        }

    }else
    {
        idex=SendMessage(GetDlgItem(hWnd,IDC_BROWSER_SCROLLVIEW),SVM_GETCURSEL,0,0);    
        p=(PMP_DIR_INFO*)SendMessage(GetDlgItem(hWnd,IDC_BROWSER_SCROLLVIEW),SVM_GETITEMADDDATA,idex,0);   
        add->back_file=malloc(sizeof(PMP_MEDIA_FILE_LIST));
        add->back_file->nr_files=1;
        sprintf(add->back_file->dir,"%s%s",add->root_dir,add->initial_dir);
        add->back_file->file_names=malloc(strlen(p->name)+1);
        sprintf(add->back_file->file_names,"%s",p->name);

    }
    if (add->back_file->nr_files > 0) 
    {
        SendMessage(hWnd,MSG_CLOSE,0,0); 
    }else
    {
        free(add->back_file);
        add->back_file=NULL;
    }
}

static void
_pmp_do_with_keydown (HWND hWnd,int key)
{
    int idex;
    PMP_DIR_INFO *p;
    PMP_WINDOW_ADD *add;

    add =(PMP_WINDOW_ADD*)GetWindowAdditionalData(hWnd);
    idex= SendMessage (GetDlgItem (hWnd, IDC_BROWSER_SCROLLVIEW), SVM_GETCURSEL, 0, 0);    
    p=(PMP_DIR_INFO*)SendMessage (GetDlgItem (hWnd, IDC_BROWSER_SCROLLVIEW), SVM_GETITEMADDDATA, idex,0);

    switch(key)
    {
        case SCANCODE_ENTER:
            if(1==_pmp_file_is_dir( add, p->name)){
                if(!(add->mode&PMP_BROWSER_MODE_DIR))
                    break;
                add->back_file=malloc(sizeof(PMP_MEDIA_FILE_LIST));
                add->back_file->nr_files=0;
                sprintf(add->back_file->dir,"%s%s%s",add->root_dir,add->initial_dir,p->name);
                add->back_file->file_names=NULL;
                SendMessage(hWnd,MSG_CLOSE,0,0); 
            }else if(0==_pmp_file_is_dir(add, p->name))
            {
                if(add->mode&PMP_BROWSER_MODE_DIR)
                    break;
                _pmp_return_files(hWnd);
            }
            break;
        case SCANCODE_CURSORBLOCKRIGHT:
            if(1==_pmp_file_is_dir(add, p->name)){
                strcat(add->initial_dir,p->name);
                strcat(add->initial_dir,"/");
                _select_add_items(hWnd);
            }
            break;
        case SCANCODE_CURSORBLOCKLEFT:
            if(0==_pmp_file_is_dir(add, p->name)){
                if(p->select==TRUE)
                    p->select=FALSE;
                else
                    p->select=TRUE;
            }
            break;
    }

}
static int
pmp_select_file_window_list_proc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    PMP_WINDOW_ADD *add;
    add =(PMP_WINDOW_ADD*)GetWindowAdditionalData(hWnd);

    switch (message)
    {
        case MSG_INITDIALOG:
            SetWindowAdditionalData(hWnd,(DWORD)lParam);
            _create_controls (hWnd); 
            break;
        case MSG_KEYDOWN:
            switch (LOWORD(wParam)){
                case SCANCODE_ENTER:
                case SCANCODE_CURSORBLOCKRIGHT:
                case SCANCODE_CURSORBLOCKLEFT:
                    _pmp_do_with_keydown(hWnd,LOWORD(wParam));
                    SendMessage(GetDlgItem(hWnd,IDC_BROWSER_SCROLLVIEW),MSG_PAINT,0,0);
                    break; 
                case SCANCODE_ESCAPE:
                    add->back_file=NULL;
                    SendMessage(hWnd,MSG_CLOSE,0,0); 
                    break;
            }
            break;
        case MSG_CLOSE:
            _free_scrollview_add_data(hWnd); 
            EndDialog(hWnd,(int)add->back_file);
            break;
    }
    return DefaultMainWinProc (hWnd,message,wParam,lParam);

}

PMP_MEDIA_FILE_LIST*
pmp_select_media_files_list(HWND parent,
        const char *root_dir,
        const char* initial_dir,
        PMP_CB_MEDIA_FILTER cb_media_filter,
        PMP_CB_MEDIA_ICON cb_media_icon,
        PMP_DIR_SHOW_INFO* dir_show_info,
        DWORD mode 
        )
{

 DLGTEMPLATE select_win = {
      //  WS_VISIBLE|WS_VSCROLL,
        WS_VISIBLE,
        WS_EX_NONE,
        0, 0,
        MAINWINDOW_W, MAINWINDOW_H,
        "",  /* caption */
        0, /* icon */
        0, /* menu */
        0, /* number of controls */
        NULL, 0
    };
 PMP_WINDOW_ADD window_add;

    window_add.initial_dir[0] = '\0';
    strcat(window_add.initial_dir, initial_dir);
    window_add.root_dir = root_dir;
    window_add.cb_media_filter = cb_media_filter;
    window_add.cb_media_icon =cb_media_icon;
    window_add.dir_show_info =dir_show_info;
    window_add.mode =mode;

    return (PMP_MEDIA_FILE_LIST*)DialogBoxIndirectParam (&select_win, parent,
             pmp_select_file_window_list_proc, (LPARAM)&window_add);

}

