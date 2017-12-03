#include <stdio.h>
#include <stdlib.h>

#include "pmp.h"
#include <minigui/control.h>

HWND hwnd;
BITMAP bmp,bmp1;
static PMP_DIR_SHOW_INFO my_dir_show_info[] = {
    {"source", "111",&bmp1},
    {"linux_c", "111",&bmp1},
    {"design", "222",&bmp1},
    {"Desktop", "desktop",&bmp1},
    {"../", "back", &bmp1},
    {"*", "333", &bmp1},
    {NULL, "333", &bmp1},
};

static DLGTEMPLATE DlgYourTaste =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    0, 0, 370, 280,
    "Creat MainWindow with Dialog",
    0, 0,
    2, NULL,
    0
};

static CTRLDATA CtrlYourTaste[] =
{ 
    {
        "button",
        WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP | WS_GROUP,
        70, 120, 70, 28,
        IDOK, 
        "OK",
        0
    },
    {
        "button",
        WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        200, 120, 70, 28,
        IDCANCEL,
        "Cancel",
        0
    },
};

const BITMAP* cb_media_icon (const char*path,const char* file_name, DWORD mode)
{
  BITMAP* pbmp;
  
  pbmp=malloc(sizeof(BITMAP));
  printf("load map!!!!\n");
  LoadBitmap (HDC_SCREEN, pbmp,"select_file_window1.bmp"); 
  return pbmp;
  return &bmp;
}

BOOL cb_media_filter(const char*path,const char*file_name)
{
     return TRUE;
}
static int DialogBoxProc2 (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
  PMP_MEDIA_FILE_LIST*file;
  switch(message){      
    case MSG_COMMAND:
        switch (wParam) {
        case IDOK:
          //  file=pmp_select_media_files (hDlg, "/home/mgwang/" ,"Desktop/",cb_media_filter,cb_media_icon,my_dir_show_info,PMP_BROWSER_MODE_LIST|PMP_BROWSER_MODE_MULTIPLE);
          file=pmp_select_media_files (hDlg, "/home/liupeng/" ,"Desktop/",cb_media_filter,cb_media_icon,my_dir_show_info,PMP_BROWSER_MODE_DIR|PMP_BROWSER_MODE_LIST);
         //   file=pmp_select_media_files (hDlg, "/home/mgwang/" ,"Desktop/",cb_media_filter,cb_media_icon,my_dir_show_info,PMP_BROWSER_MODE_LIST);
/*              file=pmp_select_media_files (hDlg, "/home/mgwang/" ,"Desktop/",cb_media_filter,cb_media_icon,my_dir_show_info,PMP_BROWSER_MODE_THUMBNAIL); */
             if(file!=NULL){
             printf("%s%s\n",file->dir,file->file_names);
             PMP_RELEASE_MEDIA_FILE_LIST(file);
             }
             break;
        case IDCANCEL:
            DestroyMainWindowIndirect (hDlg);
            break;
        }
        break;
        
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

int MiniGUIMain (int argc, const char* argv[])
{ 
    MSG Msg;
     DlgYourTaste.controls = CtrlYourTaste;

     LoadBitmap (HDC_SCREEN, &bmp,"select_file_window1.bmp"); 
     LoadBitmap (HDC_SCREEN, &bmp1,"select_file_window2.png"); 

     hwnd = CreateMainWindowIndirect (&DlgYourTaste, HWND_DESKTOP,  DialogBoxProc2);
     if (hwnd == HWND_INVALID){
        return -1;}

    while (GetMessage (&Msg, hwnd)) {
        TranslateMessage (&Msg);
        DispatchMessage (&Msg);
    }
    DestroyMainWindowIndirect (hwnd); 

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

