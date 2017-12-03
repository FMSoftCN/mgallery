#include <stdlib.h>
#include <string.h>

#include "pmp.h"
#include "decoder.h"

static char *loop_mode_str[] = 
	{ "SEQUENCE",
	  "REPEAT_ONE",
	  "REPEAT_ALL",
	  "RANDOM"};

typedef struct _VIDEO_MENU_PARAMS {
    int volume;
    loop_mode_t loop_mode;
    int skip_to;
    int progress_bar;
} VIDEO_MENU_PARAMS;

static char *video_menu_item_text [] = {
    "Volume \t %d",
    "Loop Play \t %s",    
    "Skip To \t %d min",
    "Progress \t %s",
    "File Info"
};

VIDEO_MENU_PARAMS video_menu_params = {20, SEQUENCE, 0, 0};

static int _video_menu_volume_callback (void *context, int id, int op, char *text_buff)
{
    VIDEO_MENU_PARAMS *my_params = (VIDEO_MENU_PARAMS*)context;

    switch(op) {
        
        case PMP_MI_OP_DEF:
            sprintf(text_buff, video_menu_item_text[0], my_params->volume);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;
            
        case PMP_MI_OP_PREV:
            if (my_params->volume > 0)
                my_params->volume --;
            sprintf(text_buff, video_menu_item_text[0], my_params->volume);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_NEXT:
            if (my_params->volume < 20)
                my_params->volume ++;
            sprintf(text_buff, video_menu_item_text[0], my_params->volume);
            return PMP_MI_ST_UPDATED;            
    }

    return PMP_MI_ST_NONE;
}

static int _video_menu_loop_callback(void *context, int id, int op, char *text_buff)
{
    VIDEO_MENU_PARAMS* my_params = (VIDEO_MENU_PARAMS*)context;
    
    switch(op) {
        
        case PMP_MI_OP_DEF:
            sprintf (text_buff, video_menu_item_text[1], loop_mode_str[my_params->loop_mode]);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;

        case PMP_MI_OP_PREV:
            if (my_params->loop_mode > SEQUENCE)
                my_params->loop_mode--;
            sprintf(text_buff, video_menu_item_text[1], loop_mode_str[my_params->loop_mode]);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_NEXT:
            if (my_params->loop_mode < RANDOM)
                my_params->loop_mode++;
            sprintf(text_buff, video_menu_item_text[1], loop_mode_str[my_params->loop_mode]);
            return PMP_MI_ST_UPDATED;            
    }
        
    return PMP_MI_ST_NONE;
}

static int _video_menu_skipto_callback (void *context, int id, int op, char *text_buff)
{
    VIDEO_MENU_PARAMS* my_params = (VIDEO_MENU_PARAMS*)context;
    
    switch(op) {
        
        case PMP_MI_OP_DEF:
            sprintf(text_buff, video_menu_item_text[2], my_params->skip_to);
            printf("skip to text = %s\n", text_buff);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;
            
        case PMP_MI_OP_PREV:
            if (my_params->skip_to > 0)
                my_params->skip_to--;
            sprintf(text_buff, video_menu_item_text[2], my_params->skip_to);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_NEXT:
            if (my_params->skip_to < 100)
                my_params->skip_to++;
            sprintf(text_buff, video_menu_item_text[2], my_params->skip_to);
            return PMP_MI_ST_UPDATED;
    }

    return PMP_MI_ST_NONE;
}

static char* off_on_str [] = {"OFF", "ON"};

static int _video_menu_prograss_callback(void *context, int id, int op, char *text_buff)
{
    VIDEO_MENU_PARAMS* my_params = (VIDEO_MENU_PARAMS*)context;
    
    switch(op) {
        
        case PMP_MI_OP_DEF:
            sprintf (text_buff, video_menu_item_text[3], off_on_str[my_params->progress_bar]);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;

        case PMP_MI_OP_PREV:
            if (my_params->progress_bar > 0)
                my_params->progress_bar = 0;
            sprintf(text_buff, video_menu_item_text[3], off_on_str[my_params->progress_bar]);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_NEXT:
            if (my_params->progress_bar < 1)
                my_params->progress_bar = 1;
            sprintf(text_buff, video_menu_item_text[3], off_on_str[my_params->progress_bar]);
            return PMP_MI_ST_UPDATED;            
    }
        
    return PMP_MI_ST_NONE;
}

static int _video_menu_fileinfo_callback (void *context, int id, int op, char *text_buff)
{
    switch(op) {
        case PMP_MI_OP_DEF:
            strcpy(text_buff, video_menu_item_text[4]);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;
    }

    return PMP_MI_ST_NONE;
}

static PMP_MENU_ITEM video_menu[] =
{
    {1, _video_menu_volume_callback},
    {2, _video_menu_loop_callback},
    {3, _video_menu_skipto_callback},
    {4, _video_menu_prograss_callback},
    {5, _video_menu_fileinfo_callback},
    {0, NULL}
};

static int HelloWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    int ret = 0;


    switch (message) {
        case MSG_PAINT:
            hdc = BeginPaint (hWnd);

/* 	    TextOut(hdc, 20, 20, "menu1"); */
/* 	    TextOut(hdc, 20, 50, "menu2"); */
            
            EndPaint (hWnd, hdc);
            return 0;


        case MSG_KEYDOWN:
            switch (wParam) {
                
                case SCANCODE_F1:
                    ret = pmp_show_menu (HWND_DESKTOP, "options", video_menu, &video_menu_params);                    
                    
                    printf("ret = %d\n", ret);
                    break;

                case SCANCODE_F2:
                    pmp_show_info (hWnd, "title", "hello.\n I am a test.\n", MB_ICONHAND);
                    break;

                case SCANCODE_F3:
                    pmp_show_captionbar (TRUE);
                    SetActiveWindow (hWnd);
                    break;

                case SCANCODE_F4:
                    pmp_update_tip (0, 0, "hello");
                    break;                    
            }
            
            return 0;

/*
        case MSG_KEYUP:
            sprintf (msg_text, "The %d key released", wParam);
            InvalidateRect (hWnd, &msg_rc, TRUE);
            return 0;
*/
        case MSG_CLOSE:
            //KillTimer (hWnd, 100);
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
            return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

#ifdef _MISC_MOUSECALIBRATE
static void mouse_calibrate (void)
{
    POINT src_pts [5] = {{5, 10}, {600, 20}, {620, 450}, {20, 470}, {310, 234}};
    POINT dst_pts [5] = {{0, 0}, {639, 0}, {639, 479}, {0, 479}, {320, 240}};

    SetMouseCalibrationParameters (src_pts, dst_pts);
}
#endif /* !_MISC_MOUSECALIBRATE */

int MiniGUIMain (int argc, const char* argv[])
{
    MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , "helloworld" , 0 , 0);
#endif

#ifdef _MISC_MOUSECALIBRATE
    mouse_calibrate ();
#endif
    
    SetKeyLongPressTime(200);

    CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "Hello, world!";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = HelloWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 20;
    CreateInfo.rx = 320;
    CreateInfo.by = 240;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;
    
    hMainWnd = CreateMainWindow (&CreateInfo);
    
    if (hMainWnd == HWND_INVALID)
        return -1;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);

    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup (hMainWnd);

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

