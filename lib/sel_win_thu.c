/*
 ** $Id$
 **
 ** sel_win_thu.c: select file window in  THUMBNAIL mode  for Feynman PMP solution.
 **
 ** Copyright (C) 2007 Feynman Software, all rights reserved.
 **
 ** Use of this source package is subject to specific license terms
 ** from Beijing Feynman Software Technology Co., Ltd.
 **
 ** URL: http://www.minigui.com
 **
 ** Current maintainer: wangminggang wangxuguang 
 */

#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "pmp.h"

#ifndef _ADV_2DAPI
#define RoundRect(hdc, lx, ty, rx, by, r1, r2) Rectangle(hdc, lx, ty, rx, by)
#endif

#define PATH_LEN_MAX              512
#define NAME_LEN_MAX              64
#define FULL_PATH_LEN_MAX         PATH_LEN_MAX+NAME_LEN_MAX  

typedef struct _FILE_BROWSER_LIST_NODE{
    char   name[NAME_LEN_MAX];
    char   *alias_name;
    const  BITMAP * icon;
    BOOL   is_selected;
    BOOL   is_dir;
    struct _FILE_BROWSER_LIST_NODE *prev;
    struct _FILE_BROWSER_LIST_NODE *next;
}FILE_BROWSER_LIST_NODE;


typedef struct _FILE_BROWSER_LIST{
    const char *root_dir;
    char initial_dir[PATH_LEN_MAX];
    char full_path[FULL_PATH_LEN_MAX];
    PMP_DIR_SHOW_INFO*  dir_show_info;
    PMP_CB_MEDIA_FILTER cb_media_filter;
    PMP_CB_MEDIA_ICON   cb_media_icon;
    DWORD               mode;
    FILE_BROWSER_LIST_NODE *root_dir_list;
    FILE_BROWSER_LIST_NODE *current_show;
    FILE_BROWSER_LIST_NODE *load_icon_node;
    SCROLLINFO scrollbar;
    FILE_BROWSER_LIST_NODE* active_file;
    int active_file_idex;
    int active_file_idex_back;
    int load_icon_time;
    PMP_MEDIA_FILE_LIST *back_file;

}FILE_BROWSER_LIST;

static RECT rect_icons[5]={
    {0,0,160,120},
    {160,0,320,120},
    {0,118,160,240},
    {160,118,320,240},
    {0,0,0,0},
};

static const BITMAP* 
_filebrs_file_find_dir_icon (FILE_BROWSER_LIST *add, const char *dir)
{
    PMP_DIR_SHOW_INFO* show_info;

    show_info =add->dir_show_info;
    while(show_info->dir_name!=NULL)
    {
        if(strcmp(dir,show_info->dir_name)==0)
            return show_info->icon;
        ++show_info;
    }
    return (show_info-2)->icon;

}


static char* 
_filebrs_file_find_dir_name (FILE_BROWSER_LIST *add, const char *dir)
{
    PMP_DIR_SHOW_INFO* show_info;

    show_info=add->dir_show_info;
    while (show_info->dir_name!=NULL)
    {
        if (strcmp (dir,show_info->dir_name)==0)
            return (char *)show_info->show_text;
        ++show_info;
    }
    return (char *)dir;

}


static void 
_filebrs_file_true_dir (char *dir )
{
    char *p;
    int   n=0;

    if(0!=(p=strstr(dir,"../")))
    {
        *p= '\0';
        p--;
        for (; p-dir>0; p--)
        {
            if (*p=='/')
                n++;
            if (n==2)
                break;
        }
        if (n==2)
            *(p+1)='\0';
        else
            *p='\0';   
    }
}
static int 
_filebrs_file_is_dir (FILE_BROWSER_LIST* add, const char *file_name)
{
    struct stat attr;

    if(strlen(add->root_dir)+strlen(add->initial_dir)>PATH_LEN_MAX-2||strlen(file_name)>NAME_LEN_MAX-1)
        return -1;
    add->full_path[0]='\0';
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


static void
_filebrs_destroy_file_list(FILE_BROWSER_LIST *add)
{
    FILE_BROWSER_LIST_NODE*p, *n;

    p=add->root_dir_list;
    if(p==NULL)
        return;
    while (p->next!=NULL)
    {
        if (!p->is_dir)
        {
            if (p->icon!=NULL)
            {
                UnloadBitmap((BITMAP*)p->icon);
                free((BITMAP*)p->icon);
            }
        }
        n=p->next;
        free(p);
        p=n;
    }

    if ((p->icon!=NULL)&&(!p->is_dir))
    {
        UnloadBitmap ((BITMAP*)p->icon);
        free ((BITMAP*)p->icon);
    }
    free (p);
    add->root_dir_list=NULL;
}

static void 
_filebrs_move_show_nodes (FILE_BROWSER_LIST *add, int n)
{
    FILE_BROWSER_LIST_NODE*p;
    int i;


    if (0==n)
        return ;
    else
        if (n>0)
        {
            while(n)
            {
                if(add->active_file->next==NULL)
                    break;
                add->active_file=add->active_file->next;
                add->active_file_idex++;
                add->scrollbar.nPos++;
                n--;
            }
        }else
        {
            while(n)
            {
                if(add->active_file->prev==NULL)
                    break;
                add->active_file=add->active_file->prev;
                add->active_file_idex--;
                add->scrollbar.nPos--;
                n++;
            }
        }

        add->current_show=add->root_dir_list;
        n=(add->active_file_idex/4)*4;

        while(n>0)
        {
            if(add->current_show->next==NULL)
                break;
            add->current_show=add->current_show->next;
            n--;
        }

        i=0; 
        n=(add->active_file_idex/4)*4;
        p=add->root_dir_list;

        while(1)
        {
            i++;
            if((i-n)<=4&&(i-n)>0)
            {
                goto next;
            }
            if((!p->is_dir)&&(p->icon!=NULL))
            {
                UnloadBitmap((BITMAP*)p->icon);
                free((BITMAP*)p->icon);
                p->icon=NULL;
            }

    next:
            if(p->next==NULL)
                break;
            p=p->next;

        }


}

static int compitem (FILE_BROWSER_LIST *add, const char*item1,const char*item2)
{
    if (strcmp (item1, "..") == 0)
        return -1;
    if (strcmp (item2, "..") == 0)
        return 1;
    if (_filebrs_file_is_dir(add, item1) > _filebrs_file_is_dir(add, item2))
        return -1;
    if (_filebrs_file_is_dir(add,item1) < _filebrs_file_is_dir(add, item2))
        return 1;
    return strcmp (item1, item2);

}

static void
_filebrs_list_insert_items (FILE_BROWSER_LIST *add, FILE_BROWSER_LIST_NODE*insert)
{
    FILE_BROWSER_LIST_NODE *p,*next;

    p=add->root_dir_list;
    if (p==NULL)
    {
        insert->prev = NULL;
        insert->next = NULL;
        add->root_dir_list = insert;
    }else
    {
        while (p->next!=NULL)
        {
            if (1==compitem(add ,p->name, insert->name))
                break;   
            p =p->next; 
        }

        if (p->next==NULL)
        {
            p->next=insert;
            insert->prev=p;
        }else
        {
            next=p;
            if(p->prev!=NULL)
            {
                p=p->prev;
                p->next=insert;
                insert->prev=p;
                insert->next=next;
                next->prev=insert;
            }
            else
            {
                add->root_dir_list=insert;
                insert->next=next;
                next->prev=insert;
            }
        }
    }
}
static void
_filebrs_init_scrollbar(FILE_BROWSER_LIST* list)
{   
    list->scrollbar.nMax =0 ;
    list->scrollbar.nMin =0 ;
    list->scrollbar.nPage=10;
    list->scrollbar.nPos =0 ;
}
static void
_filebrs_init_list_node(FILE_BROWSER_LIST_NODE * node,
        FILE_BROWSER_LIST *list, char *name, BOOL is_dir)
{
    node->name[0] = '\0';
    node->prev =NULL;
    node->next =NULL;
    
    if (is_dir)
    {
        node->icon = _filebrs_file_find_dir_icon (list, name);
        strcat (node->name, name);
        node->is_dir =is_dir;
        node->alias_name =_filebrs_file_find_dir_name (list, node->name);
    }else
    {
        node->icon = NULL ;
        strcat (node->name, name);
        node->is_dir =is_dir;
        node->alias_name =node->name;
    }
}
static void 
_filebrs_add_file_item (HWND hParent)
{
    DIR    *handle         = NULL;
    int    ret;
    struct dirent *os_item = NULL;
    FILE_BROWSER_LIST_NODE  *p;
    FILE_BROWSER_LIST *add;

    add =(FILE_BROWSER_LIST*)GetWindowAdditionalData (hParent);
    _filebrs_file_true_dir (add->initial_dir); 
back:
    sprintf (add->full_path, "%s%s", add->root_dir, add->initial_dir);
    handle = opendir (add->full_path);

    if (handle == NULL)
    {  
        GetWindowText(hParent, add->initial_dir, FULL_PATH_LEN_MAX);
        _MY_PRINTF("Open dir error.\n");
        if(add->initial_dir[0]=='q')
            return ;
        goto back ;
    }
    SetWindowText(hParent,add->initial_dir);
    _filebrs_init_scrollbar (add);
    _filebrs_destroy_file_list (add);


    while ((os_item = readdir (handle)) != NULL)
    {
        if (os_item->d_name[0] == '.' && os_item->d_name[1] != '.')
            continue;
        if ((os_item->d_name[0] == '.') && (os_item->d_name[1] == '.') 
                && (strlen (add->initial_dir)==0))
            continue;

        ret = _filebrs_file_is_dir (add, os_item->d_name);

        if (ret==-1)
            continue ;
        else if (ret==1)
        {  
           p =malloc (sizeof (FILE_BROWSER_LIST_NODE)); 
           add->scrollbar.nMax++; 
        }else
        {
           if (!add->cb_media_filter (add->full_path, os_item->d_name))
                continue;
           p =malloc (sizeof (FILE_BROWSER_LIST_NODE)); 
           add->scrollbar.nMax++; 
        }
        _filebrs_init_list_node (p, add, os_item->d_name,ret);
        _filebrs_list_insert_items (add, p);   
    }
    closedir (handle);

    add->current_show = add->root_dir_list;
    add->active_file = add->current_show;
    add->active_file_idex =0; 
    add->active_file_idex_back =5; 
    add->scrollbar.nMax--;
    SetScrollInfo (hParent,SB_VERT ,&(add->scrollbar),TRUE);
}

    static void
_filebrs_draw_icon (HWND hwnd, HDC hdc, int left,int top, FILE_BROWSER_LIST_NODE* p)
{
    RECT tmp;
    FILE_BROWSER_LIST *add;
    int ret;

    ret=SetBkMode(hdc, BM_TRANSPARENT);
    add =(FILE_BROWSER_LIST*)GetWindowAdditionalData(hwnd);

    if(add->active_file==p)
    {
        SetPenColor (hdc,RGB2Pixel(HDC_SCREEN, 255, 255, 255));
        SetBrushColor (hdc,RGB2Pixel(HDC_SCREEN, 71, 156, 210));
        RoundRect(hdc,left-5,top-2,left+95+5,top+95+2,2,2);
    }

    if(NULL==p->icon)
    {
        SetPenColor (hdc, PIXEL_yellow);
        SetBrushColor (hdc,RGB2Pixel(HDC_SCREEN, 71, 156, 210));
        RoundRect(hdc,left,top,left+95,top+95,6,6);
    }else
        FillBoxWithBitmap(hdc,left,top,95,95,p->icon);
    tmp.left   = left-5 ;
    tmp.top    = top+95;
    tmp.right  = tmp.left + 95+5;
    tmp.bottom = tmp.top+16;
    DrawText(hdc,p->alias_name, -1, &tmp, DT_CENTER | DT_WORDBREAK);

}

static void 
_filebrs_initial_file_list (FILE_BROWSER_LIST* list, 
        const char         *root_dir,
        const char         *initial_dir,
        PMP_CB_MEDIA_FILTER cb_media_filter,
        PMP_CB_MEDIA_ICON   cb_media_icon,
        PMP_DIR_SHOW_INFO*  dir_show_info,
        DWORD mode)
{
    list->root_dir        =root_dir;
    list->initial_dir[0]  ='\0'; 
    strcat (list->initial_dir, initial_dir);
    list->cb_media_filter =cb_media_filter;
    list->cb_media_icon   =cb_media_icon;
    list->dir_show_info   =dir_show_info;
    list->mode            =mode;
    list->root_dir_list   =NULL;
    list->load_icon_time  =0;
}

static void 
_filebrs_browser_drawitem (HWND hwnd)
{
    HDC hdc;
    FILE_BROWSER_LIST_NODE *p;
    FILE_BROWSER_LIST *add;

    add =(FILE_BROWSER_LIST*)GetWindowAdditionalData (hwnd);
    p=add->current_show;
    hdc = BeginPaint (hwnd);

    _filebrs_draw_icon (hwnd, hdc,35,5,p);
    if(p->next==NULL)
        goto over;
    p=p->next;  
    _filebrs_draw_icon (hwnd, hdc,165,5,p);
    if (p->next==NULL)
        goto over;
    p=p->next;  
    _filebrs_draw_icon (hwnd, hdc,35,120,p);
    if(p->next==NULL)
        goto over;
    p=p->next;  
    _filebrs_draw_icon (hwnd, hdc,165,120,p);

over:
    EndPaint (hwnd,hdc);
    if (0==add->scrollbar.nMax)
        SetScrollPos (hwnd, SB_VERT, add->scrollbar.nPos);
    else 
        SetScrollPos (hwnd, SB_VERT, add->scrollbar.nPos*100/add->scrollbar.nMax);

}

    static void
_filebrs_do_with_keydown (HWND hWnd, int key)
{
    FILE_BROWSER_LIST *add;

    add =(FILE_BROWSER_LIST*)GetWindowAdditionalData(hWnd);
    switch(key)
    {
        case SCANCODE_ENTER:
            if(1==_filebrs_file_is_dir(add, add->active_file->name))
            {
                strcat (add->initial_dir,add->active_file->name);
                strcat (add->initial_dir,"/");
                _filebrs_add_file_item (hWnd);
            }else if(0==_filebrs_file_is_dir(add, add->active_file->name))
            {
                add->back_file = malloc(sizeof(PMP_MEDIA_FILE_LIST));
                add->back_file->nr_files = 1;
                sprintf(add->back_file->dir,"%s%s",add->root_dir,add->initial_dir);
                add->back_file->file_names = malloc(strlen(add->active_file->name)+1);
                sprintf (add->back_file->file_names,"%s",add->active_file->name);
                SendMessage (hWnd,MSG_CLOSE,0,0); 
            }
            break;
        case SCANCODE_CURSORBLOCKRIGHT:
            _filebrs_move_show_nodes (add,  1);
            break;
        case SCANCODE_CURSORBLOCKLEFT:
            _filebrs_move_show_nodes (add, -1);
            break;
        case SCANCODE_CURSORBLOCKUP:
            _filebrs_move_show_nodes (add, -2);
            break;
        case SCANCODE_CURSORBLOCKDOWN:
            _filebrs_move_show_nodes (add,  2);
            break;
    }
}

static void 
_filebrs_loadicon_in_cpufree (FILE_BROWSER_LIST *add ,HWND hWnd)
{
    FILE_BROWSER_LIST_NODE** p= &(add->load_icon_node);

    sprintf(add->full_path,"%s%s",add->root_dir,add->initial_dir);

    if(add->load_icon_time>3)
        return ; 

    if(add->load_icon_time==0)
        *p=add->current_show;

    while ((*p)->next!=NULL)
    {
        if ((*p)->icon==NULL)
        {
            (*p)->icon=add->cb_media_icon(add->full_path,(*p)->name,add->mode);
            (*p) =(*p)->next;
            add->load_icon_time++;
            return;
        }else{
            (*p) =(*p)->next;
            add->load_icon_time++;
        }
    }

    if ((*p)->icon==NULL)
        (*p)->icon=add->cb_media_icon(add->full_path,(*p)->name,add->mode);
    add->load_icon_time++ ;
        
}
    static int
_select_file_window_thumbscrew_proc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    FILE_BROWSER_LIST *add;

    add =(FILE_BROWSER_LIST*)GetWindowAdditionalData(hWnd);
    switch (message)
    {
        case MSG_KEYDOWN:
            switch (LOWORD(wParam))
            {
                case SCANCODE_ESCAPE:
                    add->back_file=NULL;
                    SendMessage(hWnd,MSG_CLOSE,0,0); 
                    break;
                case SCANCODE_CURSORBLOCKRIGHT:
                case SCANCODE_CURSORBLOCKLEFT:
                case SCANCODE_CURSORBLOCKDOWN:
                case SCANCODE_CURSORBLOCKUP:
                case SCANCODE_ENTER:
                    add->load_icon_time=0;
                    _filebrs_do_with_keydown(hWnd,LOWORD(wParam));
                    break;
            }

            if (add->active_file_idex/4!=add->active_file_idex_back/4) 
            {
                 InvalidateRect (hWnd,NULL,TRUE);
            }
            else
            {
                InvalidateRect (hWnd,&rect_icons[add->active_file_idex_back%4],TRUE);
                InvalidateRect (hWnd,&rect_icons[add->active_file_idex%4],TRUE);
            }
            add->active_file_idex_back =add->active_file_idex ;
            return 0; 
        case MSG_INITDIALOG:
            SetWindowBkColor (hWnd,RGB2Pixel(HDC_SCREEN, 71, 156, 210) );
            SetWindowAdditionalData(hWnd,(DWORD)lParam);
            _filebrs_add_file_item(hWnd);
            EnableScrollBar(hWnd,SB_VERT,FALSE);
            break;
        case MSG_CLOSE:
            _filebrs_destroy_file_list(add);
            EndDialog(hWnd,(int)(add->back_file));
            return 0;
        case MSG_PAINT:
            _filebrs_browser_drawitem(hWnd);
            break;
        case MSG_IDLE:
            _filebrs_loadicon_in_cpufree(add, hWnd);
            InvalidateRect (hWnd,&rect_icons[add->load_icon_time%4-1],TRUE);
            break;
    }
    return DefaultMainWinProc (hWnd,message,wParam,lParam);

}

PMP_MEDIA_FILE_LIST*
pmp_select_media_files_thumbscrew(HWND parent,
        const char           *root_dir,
        const char           *initial_dir,
        PMP_CB_MEDIA_FILTER   cb_media_filter,
        PMP_CB_MEDIA_ICON     cb_media_icon,
        PMP_DIR_SHOW_INFO    *dir_show_info,
        DWORD mode
        )
{
    DLGTEMPLATE select_win = {
        WS_VISIBLE| WS_VSCROLL,
        WS_EX_NONE,
        0, 0,
        MAINWINDOW_W, MAINWINDOW_H,
        "q",  /* caption */
        0, /* icon */
        0, /* menu */
        0, /* number of controls */
        NULL, 0
    };
    FILE_BROWSER_LIST list;

    _filebrs_initial_file_list(&list, root_dir, 
            initial_dir, cb_media_filter,
            cb_media_icon, dir_show_info, mode);

    return (PMP_MEDIA_FILE_LIST*)DialogBoxIndirectParam (&select_win, parent,
            _select_file_window_thumbscrew_proc, (LPARAM)&list);

}

