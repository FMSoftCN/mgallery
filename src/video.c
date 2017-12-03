/*
** $Id$
**
** video.c: Video module for Feynman PMP solution.
**
** Copyright (C) 2007 Feynman Software, all rights reserved.
**
** Use of this source package is subject to specific license terms
** from Beijing Feynman Software Technology Co., Ltd.
**
** URL: http://www.minigui.com
**
** Current maintainer: liupeng .
*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "video.h"
#include "text_id.h"

#define DECODER

static int loop_mode_id[] = {
    IDS_PMP_VIDEO_SEQUENCE,
    IDS_PMP_VIDEO_REAPEAT_ONE,
    IDS_PMP_VIDEO_REAPEAT_ALL,
    IDS_PMP_VIDEO_RANDOM
};

static int video_menu_item_text_id [] = {
    IDS_PMP_VIDEO_VOLUME,
    IDS_PMP_VIDEO_LOOP,
    IDS_PMP_VIDEO_SKIP,
    IDS_PMP_VIDEO_PROG,
    IDS_PMP_VIDEO_FILE_INFO
};

static BITMAP *file_bmp = NULL;

static VIDEO_MENU_PARAMS video_menu_params = {20, SEQUENCE, 0, 0};

static int _video_menu_volume_callback (void *context, int id, int op, char *text_buff)
{
    VIDEO_MENU_PARAMS *my_params = (VIDEO_MENU_PARAMS*)context;

    switch(op) {
        
        case PMP_MI_OP_DEF:
            sprintf(text_buff, _(video_menu_item_text_id[0]), my_params->volume);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;
            
        case PMP_MI_OP_PREV:
            if (my_params->volume > 0)
                my_params->volume --;
            sprintf(text_buff, _(video_menu_item_text_id[0]), my_params->volume);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_NEXT:
            if (my_params->volume < 20)
                my_params->volume ++;
            sprintf(text_buff, _(video_menu_item_text_id[0]), my_params->volume);
            return PMP_MI_ST_UPDATED;            
    }

    return PMP_MI_ST_NONE;
}

static int _video_menu_loop_callback(void *context, int id, int op, char *text_buff)
{
    VIDEO_MENU_PARAMS* my_params = (VIDEO_MENU_PARAMS*)context;
    
    switch(op) {        
        case PMP_MI_OP_DEF:
            sprintf (text_buff, _(video_menu_item_text_id[1]), _(loop_mode_id[my_params->loop_mode]));
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;

        case PMP_MI_OP_PREV:
            if (my_params->loop_mode > SEQUENCE)
                my_params->loop_mode--;
            sprintf(text_buff, _(video_menu_item_text_id[1]), _(loop_mode_id[my_params->loop_mode]));
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_NEXT:
            if (my_params->loop_mode < RANDOM)
                my_params->loop_mode++;
            sprintf(text_buff, _(video_menu_item_text_id[1]), _(loop_mode_id[my_params->loop_mode]));
            return PMP_MI_ST_UPDATED;            
    }
        
    return PMP_MI_ST_NONE;
}

static int _video_menu_skipto_callback (void *context, int id, int op, char *text_buff)
{
    VIDEO_MENU_PARAMS* my_params = (VIDEO_MENU_PARAMS*)context;
    
    switch(op) {
        
        case PMP_MI_OP_DEF:
            sprintf(text_buff, _(video_menu_item_text_id[2]), my_params->skip_to);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;
            
        case PMP_MI_OP_PREV:
            if (my_params->skip_to > 0)
                my_params->skip_to--;
            sprintf(text_buff, _(video_menu_item_text_id[2]), my_params->skip_to);
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_NEXT:
            if (my_params->skip_to < 100)
                my_params->skip_to++;
            sprintf(text_buff, _(video_menu_item_text_id[2]), my_params->skip_to);
            return PMP_MI_ST_UPDATED;
    }

    return PMP_MI_ST_NONE;
}

static int off_on_id [] = {IDS_PMP_VIDEO_OFF, IDS_PMP_VIDEO_ON};

static int _video_menu_prograss_callback(void *context, int id, int op, char *text_buff)
{
    VIDEO_MENU_PARAMS* my_params = (VIDEO_MENU_PARAMS*)context;
    
    switch(op) {
        
        case PMP_MI_OP_DEF:
            sprintf (text_buff, _(video_menu_item_text_id[3]), _(off_on_id[my_params->progress_bar]));
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;

        case PMP_MI_OP_PREV:
            if (my_params->progress_bar > 0)
                my_params->progress_bar = 0;
            sprintf(text_buff, _(video_menu_item_text_id[3]), _(off_on_id[my_params->progress_bar]));
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_NEXT:
            if (my_params->progress_bar < 1)
                my_params->progress_bar = 1;
            sprintf(text_buff, _(video_menu_item_text_id[3]), _(off_on_id[my_params->progress_bar]));
            return PMP_MI_ST_UPDATED;            
    }
        
    return PMP_MI_ST_NONE;
}

static int _video_menu_fileinfo_callback (void *context, int id, int op, char *text_buff)
{
    switch(op) {
        case PMP_MI_OP_DEF:
            
            strcpy(text_buff, _(video_menu_item_text_id[4]));
            return PMP_MI_ST_UPDATED;
            
        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;
    }

    return PMP_MI_ST_NONE;
}

static PMP_MENU_ITEM video_menu[] =
{
    {MENU_VOLUME_ID, _video_menu_volume_callback},
    {MENU_LOOP_ID, _video_menu_loop_callback},
    {MENU_SKIP_ID, _video_menu_skipto_callback},
    {MENU_PROGBAR_ID, _video_menu_prograss_callback},
    {MENU_FILEINFO_ID, _video_menu_fileinfo_callback},
    {0, NULL}
};

/* define the file filter */
static BOOL _video_filter (const char *path, const char* file_name)
{
    /* check the file's suffix to determine whether to list the file here */
    const char *filter = "Media File (*.avi;*.AVI;*.Avi)";
    char *p;

    p = rindex (file_name, '.');
    if (!p)
        return FALSE;
    
    if (strstr (filter, ++p))
        return TRUE;
    else
        return FALSE;
}

/* return the icon of the media */
static const BITMAP* _video_icon (const char *path, const char* file_name, DWORD mode)
{
    /* return the BITMAP object according to the media file and the browse mode */

    if (file_bmp)
        pmp_image_res_release_bitmap("/res/video/video_icon.bmp", file_bmp);

    file_bmp = pmp_image_res_acquire_bitmap("/res/video/video_icon.bmp");
    
    return file_bmp;  
}

static char *_get_file_name (PMP_MEDIA_FILE_LIST* list, int idx)
{
    char *p = list->file_names;
    int i = 0;

    if (idx < 0 || idx >= list->nr_files)
        return NULL;

    while (i < idx) {
        p = strchr (p, '\0');
        p++;
        i++;
    }

    return p;
}

#define MAINWIN_LX    0
#define MAINWIN_TY    20
static video_file_info_t file_info;
static display_area_t display_area;
static play_param_t play_param;
static video_status_t video_status;
#define VIDEO_FULLPATH_MAX    (512+64+1)
static char path[VIDEO_FULLPATH_MAX];
static RECT prog_rect = {0, 190, 320, 220};
static RECT cur_time_rect = {10, 195, 50, 205};
#define PROGBAR_HEIGHT                  30
static HDC mem_dc;

static void _play_me (VIDEO_INFO *info)
{
    memset (path, 0, VIDEO_FULLPATH_MAX);
    strcpy (path, info->file_list->dir);
    strcat (path, _get_file_name(info->file_list, info->cur_idx));
    _MY_PRINTF ("file = %s \n", path);


    videoDecCmd(info->dec_handle, SET_FILE,(unsigned int)path);
    videoDecCmd(info->dec_handle,GET_MEDIA_INFO,(unsigned int)&file_info);
    
    display_area.xpos = 0;
    if (video_menu_params.progress_bar == 1)
        display_area.ypos = PROGBAR_HEIGHT;
    else
        display_area.ypos = (g_rcScr.bottom - file_info.height)/2;
    display_area.width = file_info.width;
    display_area.height = file_info.height;
    videoDecCmd(info->dec_handle,SET_DISPLAY_AREA,(unsigned int)&display_area);
    
    videoDecCmd(info->dec_handle,YUV_CHANNEL_ON,0);
    videoDecCmd(info->dec_handle,FLUSH_SCREEN,0);
    play_param.mode = NORMAL_PLAY;
    videoDecCmd(info->dec_handle, PLAY, (unsigned int)&play_param);
    

    return;
}

static void _print_screen (void *handle, HWND hWnd)
{
    HDC hdc;
    BITMAP bmp;

    videoDecCmd (handle, GET_MEDIA_INFO, (unsigned int)&file_info);
    
    videoDecCmd (handle, PAUSE, 0);
    videoDecCmd (handle, YUV_CHANNEL_OFF, 0);

    if((file_info.width==0) || (file_info.height==0)) {
        _MY_PRINTF ("bad frame\n");
        return;
    }

    bmp.bmBits  = (unsigned char*)malloc(2 * file_info.width * file_info.height);
    videoDecCmd(handle, PRINT_SCREEN, (unsigned int)bmp.bmBits);

    bmp.bmType = BMP_TYPE_NORMAL;
    bmp.bmBitsPerPixel = 16;
    bmp.bmBytesPerPixel = 2;
    bmp.bmWidth = file_info.width;
    bmp.bmHeight = file_info.height;
    bmp.bmPitch = 2*file_info.width;
                       
    hdc = GetClientDC(hWnd);
    FillBoxWithBitmap(hdc,
                      (g_rcScr.right - file_info.width)/2,
                      (g_rcScr.bottom - file_info.height)/2 - MAINWIN_TY ,
                      file_info.width,
                      file_info.height,
                      &bmp);
    ReleaseDC(hdc);
    
    
    return;
}

static void _restore_play (void *handle)
{
    display_area.xpos = 0;
    if(video_menu_params.progress_bar == 0) 
        display_area.ypos = (MAINWINDOW_H-file_info.height)/2;
    else
        display_area.ypos = PROGBAR_HEIGHT;
    display_area.width = file_info.width; 
    display_area.height = file_info.height;  
    videoDecCmd(handle,SET_DISPLAY_AREA,(unsigned int)&display_area);     
    
    videoDecCmd(handle, YUV_CHANNEL_ON,0);
    videoDecCmd(handle,FLUSH_SCREEN,0);
    videoDecCmd(handle, CONTINUE, 0);

    return;
}

static void _draw_progressbar (HDC hdc, void *handle)
{
    char str[10];                //current time
    int total_time_min, total_time_sec;
    int cur_time_min, cur_time_sec;

    videoDecCmd(handle,GET_STATUS,(unsigned int)&video_status);

    cur_time_min = video_status.cur_time/1000/60;
    cur_time_sec = video_status.cur_time/1000%60;
    total_time_min = file_info.total_time/1000/60;
    total_time_sec = file_info.total_time/1000%60;
    
    SetBrushColor(hdc, RGB2Pixel(hdc, 17, 90, 136));
    FillBox(hdc, 60, 222-20, 200, 4 );
    
    SetBrushColor(hdc, RGB2Pixel(hdc, 130, 183, 216));
    FillBox(hdc,
            60,
            222-20,
            video_status.cur_time * 200 / file_info.total_time,
            4 );
    
    BitBlt(mem_dc, 0, 0, 0, 0, hdc, 10, 195, 0);    
    SetBkMode (hdc, BM_TRANSPARENT);
    SetTextColor(hdc, RGB2Pixel(hdc, 255, 255, 255));
    
    memset(str, 0, 10);
    sprintf(str, "%2d:%2d", cur_time_min, cur_time_sec);
    TextOut(hdc, 10, 215-20, str);
       
    memset(str, 0, 10);          //total time
    sprintf(str, "%2d:%2d", total_time_min, total_time_sec);
    TextOut(hdc, 265, 215-20 , str);

   
    return;
}

static void fill_mem_dc (HWND hWnd)
{
    HDC hdc;
    
    hdc = GetDC(hWnd);
    mem_dc = CreateCompatibleDCEx(hdc, 40, 10);            
    BitBlt (hdc, 10, 195, 40, 10, mem_dc, 0, 0, 0);                
    ReleaseDC(hdc);    
}

static PMP_MEDIA_FILE_LIST *get_video_file_list (HWND hWnd)
{
    PMP_MEDIA_FILE_LIST* list = NULL;
    BITMAP *folder_bmp = pmp_image_res_acquire_bitmap ("/res/public/folder.png");

            
    PMP_DIR_SHOW_INFO dir_show_info [] = 
        {
            {"..", _(IDS_PMP_DOTDOT), folder_bmp},                            
            {"music", _(IDS_PMP_MUSIC_DIRNAME), folder_bmp},                    
            {"video", _(IDS_PMP_VIDEO_DIR_ALIAS), folder_bmp},
            {"ebook", _(IDS_PMP_EBOOK_DIRNAME), folder_bmp},                    
            {"picture", _(IDS_PMP_PIC_DIR_ALIAS), folder_bmp},
            {"recorder", _(IDS_PMP_RECORDER_DIRNAME), folder_bmp},                    
            {"*", NULL,folder_bmp},
            {NULL, NULL, NULL},
        };
    list = pmp_select_media_files (hWnd, PMP_MEDIA_TOP_DIR, 
                                   PMP_MEDIA_NAME_VIDEO, 
                                   _video_filter, _video_icon, dir_show_info, 
                                   PMP_BROWSER_MODE_LIST | PMP_BROWSER_MODE_MULTIPLE );
        /* release bmp. */
    if (folder_bmp) {
        pmp_image_res_release_bitmap("/res/public/folder.png", folder_bmp);
        folder_bmp = NULL;
    }
    
    if (file_bmp) {
        pmp_image_res_release_bitmap("/res/video/video_icon.bmp", file_bmp);
        file_bmp = NULL;
    }

    return list;
}

static int VideoProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    VIDEO_INFO* video_info = (VIDEO_INFO*)GetWindowAdditionalData (hWnd);
    
    switch (message) {
        case MSG_NCCREATE:
#ifdef BITMAPBK
            SetWindowAdditionalData2 (hWnd, 0);
#else            
            SetWindowAdditionalData2 (hWnd, (DWORD)"/res/public/bg.gif");
#endif            
            break;

        case MSG_CREATE: {
            PMP_MEDIA_FILE_LIST* list = NULL;
            
            fill_mem_dc (hWnd);            
            pmp_app_notify_opened (PMP_APP_VIDEO, hWnd);
            list = get_video_file_list (hWnd);
            
            if (!list) {
                PostMessage(hWnd, MSG_CLOSE, 0, 0);
                return 0;
            }

            video_info = (VIDEO_INFO *)malloc(sizeof(VIDEO_INFO));
            if (video_info == NULL)
                return 0;
    
            video_info->file_list = list;            
            video_info->dec_handle = videoDecOpen(NULL);
            if (video_info->dec_handle == NULL) {
                free (video_info);
                PostMessage(hWnd, MSG_CLOSE, 0, 0);
                return 0;
            }
            video_info->cur_idx = 0;             

            SetWindowAdditionalData (hWnd, (DWORD)video_info);

            _play_me (video_info);    
            SetTimer (hWnd, IDC_TIMER, TIMER_INTERVAL);
            break;
        }
        case MSG_PAINT: {
            HDC hdc;
            
            hdc = BeginPaint (hWnd);
            if (video_menu_params.progress_bar == 1)
                _draw_progressbar (hdc, video_info->dec_handle);           
            
            
            EndPaint (hWnd, hdc);
            return 0;
        }
        case MSG_TIMER: {
            videoDecCmd(video_info->dec_handle,GET_STATUS,(unsigned int)&video_status);            
            if(video_status.status == STOPPED) {
                if( REPEAT_ALL == video_menu_params.loop_mode) {
                    ++(video_info->cur_idx) >= video_info->file_list->nr_files
                        ? video_info->cur_idx = 0 : 0;  
                    _play_me (video_info);
                }
                else if (REPEAT_ONE == video_menu_params.loop_mode) {
                    if(video_info->cur_idx < video_info->file_list->nr_files)
                        _play_me (video_info);
                }
                else if (RANDOM == video_menu_params.loop_mode)  {
                        video_info->cur_idx = rand() % (video_info->file_list->nr_files - 1);
                        _play_me (video_info);
                }
                else if (SEQUENCE == video_menu_params.loop_mode) {
                    if(++(video_info->cur_idx) < video_info->file_list->nr_files)
                        _play_me (video_info);                     
		    else {  //when sequence play over, return to pre win.
	                videoDecCmd(video_info->dec_handle,YUV_CHANNEL_OFF,0);
			KillTimer(hWnd, IDC_TIMER);
                        PostMessage(hWnd, MSG_CLOSE, 0, 0);
		    }     
                }                    
            }

            if (video_menu_params.progress_bar == 1 &&
                video_status.status == PLAYING)
            {        
                InvalidateRect (hWnd, &prog_rect, FALSE);
                InvalidateRect (hWnd, &cur_time_rect, FALSE);
            }
            
            break;
        }

        case MSG_KEYLONGPRESS:
            if (wParam == SCANCODE_CURSORBLOCKLEFT) 
                videoDecCmd(video_info->dec_handle, FAST_BACKWARD, 1);
            else if (wParam == SCANCODE_CURSORBLOCKRIGHT)
                videoDecCmd(video_info->dec_handle, FAST_FORWARD, 1);

            break;

        case MSG_KEYALWAYSPRESS: {
            videoDecCmd(video_info->dec_handle,GET_STATUS,(unsigned int)&video_status);
            
            if(wParam == SCANCODE_CURSORBLOCKRIGHT) {            
                if(video_status.status == PLAYING)
                    videoDecCmd(video_info->dec_handle,GET_STATUS,(unsigned int)&video_status);
            }
            else if(wParam == SCANCODE_CURSORBLOCKLEFT) {
                if(video_status.status == PLAYING)
                    videoDecCmd(video_info->dec_handle,GET_STATUS,(unsigned int)&video_status);
            }
            break;            
        }
        case MSG_KEYUP:          
            if(wParam == SCANCODE_CURSORBLOCKLEFT ||
               wParam == SCANCODE_CURSORBLOCKRIGHT)
                videoDecCmd(video_info->dec_handle,CANCEL_FF,0);
            
            break;
            
        case MSG_KEYDOWN: {
            switch (wParam)  {
                case SCANCODE_ESCAPE: 
                    PostMessage(hWnd, MSG_CLOSE, 0, 0);
                    break;
                
                case SCANCODE_ENTER: {
                    videoDecCmd(video_info->dec_handle,GET_STATUS,(unsigned int)&video_status);
                    if(video_status.status == PLAYING)
                        videoDecCmd(video_info->dec_handle,PAUSE,0);
                    else if(video_status.status == PAUSED)
                        videoDecCmd(video_info->dec_handle,CONTINUE,0);
                    break;
                }
                    
                case SCANCODE_CURSORBLOCKUP:      /* Pre Video */
                    if(video_info->cur_idx > 0) {
                        video_info->cur_idx--;
                        _play_me(video_info);
                    }
		    break;

                case SCANCODE_CURSORBLOCKDOWN:    /* Next  Video */
                    if(video_info->cur_idx < video_info->file_list->nr_files - 1) {
                        (video_info->cur_idx)++;
                        _play_me (video_info);
                    }
		    break;


                case SCANCODE_F1: {
                    _print_screen (video_info->dec_handle, hWnd);
                    switch (pmp_show_menu (hWnd,
                                           _(IDS_PMP_VIDEO_OPTION),
                                           video_menu,
                                           &video_menu_params)) {
                      
                        case MENU_VOLUME_ID:
                            _restore_play (video_info->dec_handle);
                            videoDecCmd(video_info->dec_handle,
                                        SET_VOLUME,
                                        (unsigned int)(video_menu_params.volume));                    
                            break;
                    
                        case MENU_LOOP_ID:
                            _restore_play (video_info->dec_handle);
                            break;
                            
                        case MENU_SKIP_ID:
                            _restore_play (video_info->dec_handle);
                            videoDecCmd(video_info->dec_handle,
                                        SEEK,
                                        video_menu_params.skip_to*1000);                                    
                            break;
                            
                        case MENU_PROGBAR_ID: {
                            InvalidateRect (hWnd, &prog_rect, TRUE);
                            _restore_play (video_info->dec_handle);
                      
                            break;
                        }
                        case MENU_FILEINFO_ID: {
                            char buf[1024];
                            unsigned int total_time= file_info.total_time/1000;
                            
                            sprintf (buf,
                                     _(IDS_PMP_VIDEO_INFO),
                                     file_info.filename,
                                     total_time/60000,
                                     total_time%60,
                                     file_info.bitrate,
                                     file_info.sample_rate,
                                     file_info.filepath,
                                     file_info.width,
                                     file_info.height);
                            pmp_show_info (hWnd, _(IDS_PMP_VIDEO_INFO_CAPT), buf, MB_ICONINFORMATION);                            
                            _restore_play (video_info->dec_handle);
                            break;
                        }
                            
                        case 0:
                            _restore_play (video_info->dec_handle);
                            break;
                            
                    }
                } /* End of SCANCODE_F1 */
            } /* End of switch(wParam) */
            break;
        }
        case MSG_SETFOCUS:
            pmp_set_captionbar_title (_(IDS_PMP_VIDEO_CAPTION));
            break;            

        case MSG_DESTROY: {
            video_menu_params.progress_bar = 0;
            if (video_info) {
                if (video_info->file_list) 
                    PMP_RELEASE_MEDIA_FILE_LIST (video_info->file_list);

                if (video_info->dec_handle)
                    videoDecClose(video_info->dec_handle,NULL);

                free (video_info);
            }            
            KillTimer(hWnd, IDC_TIMER);
            DeleteCompatibleDC(mem_dc);            
            DestroyAllControls (hWnd);            
            return 0;            
        }
        case MSG_CLOSE:
            DestroyMainWindow (hWnd);
            MainWindowCleanup (hWnd);
            pmp_app_notify_closed (PMP_APP_VIDEO);
            return 0;
    }

/*     return DefaultMainWinProc (hWnd, message, wParam, lParam); */

    return pmp_def_wnd_proc (hWnd, message, wParam, lParam);
}

static void _video_init_create_info (PMAINWINCREATE pCreateInfo)
{
    pCreateInfo->dwStyle = WS_VISIBLE;
    pCreateInfo->dwExStyle = WS_EX_NONE;
    pCreateInfo->spCaption = "video";
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor(0);
    pCreateInfo->hIcon = 20;
    pCreateInfo->MainWindowProc = VideoProc;
    pCreateInfo->lx = MAINWIN_LX;
    pCreateInfo->ty = MAINWIN_TY;
    pCreateInfo->rx = MAINWINDOW_W;
    pCreateInfo->by = MAINWINDOW_H;
    pCreateInfo->iBkColor = COLOR_lightwhite;
    pCreateInfo->dwAddData = 0;
    pCreateInfo->hHosting = HWND_DESKTOP;
}

void ex_video_show_window (HWND parent)
{
    HWND hVideoWnd;
    MAINWINCREATE CreateInfo;

    _video_init_create_info (&CreateInfo);
    CreateInfo.hHosting = parent;
    
    hVideoWnd = CreateMainWindow (&CreateInfo);
/*     pmp_app_notify_opened (PMP_APP_VIDEO, hVideoWnd); */
    
    return;
}


