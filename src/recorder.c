/*
 ** $Id$
 **
 ** recorder.c: Recorder module for Feynman PMP solution.
 **
 ** Copyright (C ) 2007 Feynman Software, all rights reserved.
 **
 ** Use of this source package is subject to specific license terms
 ** from Beijing Feynman Software Technology Co., Ltd.
 **
 ** URL: http://www.minigui.com
 **
 ** Current maintainer: Zhang Hua .
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>

#include "pmp.h"

#ifdef PMP_APP_RECORDER

#include "encoder.h"
#include "text_id.h"

#define _ID_TIMER        500
#define PUSH_LONG        1
#define PUSH_SHORT       2
#define ID_STATUS_TIMER  100
#define ID_LAST_TIMER    200
#define BK_RECORDER      "/res/recorder/recorder.bmp"

#ifdef BITMAPBK
static BITMAP *backbitmap;
#endif

static HWND hMainWnd=HWND_INVALID;

static RECT rect_statu[6] = {
    { 183, 50, 206, 63 },
    { 183, 70, 215, 83 },
    { 183, 90, 206, 103 },
    { 183, 110, 206, 123 },
    { 183, 130, 225, 143 },
    { 183, 150, 206, 163 }
};

static char *forma_set_str[] = { "WAV",  "MP3" };
static char *source_set_str[] = { "MIC", "FM", "LINE_IN" };
static int silence_detect_set_str[] = { IDS_PMP_RECORDER_ON, IDS_PMP_RECORDER_OFF };
static int listen_setting_str[] = { IDS_PMP_RECORDER_ON, IDS_PMP_RECORDER_OFF };
static char *bit_perpoint_set_str = "24";
static int channel_num_set_str[] = { IDS_PMP_RECORDER_SINGLE, IDS_PMP_RECORDER_DOUBLE };
static char *sample_rate_set_str[] = { "8000", "16000", "24000", "32000", "48000" };
static char *bit_rate_mp3_str[] = { "64", "128", "192", "256" };
static char *bit_rate_wav_str[] = { "32", "64", "192", "384" };
static int encode_set_str[] = { IDS_PMP_RECORDER_STEREO, IDS_PMP_RECORDER_JOINT_STEREO,
    IDS_PMP_RECORDER_DUAL_CHANNLE, IDS_PMP_RECORDER_MONO };

typedef struct _RECORDER_SECOND_MENU_PARAMS {
    int channel_num;
    int sample_rate;
    int bit_rate_wav;
    int bit_rate_mp3;
    int encode_mode;
} RECORDER_SECOND_MENU_PARAMS;

static int recorder_second_menu_item_text_id[] = {
    IDS_PMP_RECORDER_MENU_BPP,
    IDS_PMP_RECORDER_MENU_CHANNEL,
    IDS_PMP_RECORDER_MENU_SAMPLE_RATE,
    IDS_PMP_RECORDER_MENU_BPS,
    IDS_PMP_RECORDER_MENU_ENCODE_MODE
};

RECORDER_SECOND_MENU_PARAMS recorder_second_menu_params = { 0, 1, 1, 0, 0 };

typedef struct _RECORDER_MENU_PARAMS {
    int forma;
    int source;
    int record_gain;
    int silence_detect;
    int vor_time;
    int volume_setting;
    int listen_setting;
    int listen_volume;
    int last_time;
} RECORDER_MENU_PARAMS;

static int recorder_menu_item_text_id [] = {
    IDS_PMP_RECORDER_MENU_FILE_FORMA,
    IDS_PMP_RECORDER_MENU_SOUND_SOURCE,    
    IDS_PMP_RECORDER_MENU_RECORDER_GAIN,
    IDS_PMP_RECORDER_MENU_SILENCE_DETECT,
    IDS_PMP_RECORDER_MENU_VOR_TIME,
    IDS_PMP_RECORDER_MENU_RECORD_VOLUME,
    IDS_PMP_RECORDER_MENU_LISTEN,
    IDS_PMP_RECORDER_MENU_LISTEN_VOLUME,
    IDS_PMP_RECORDER_MENU_LAST_TIME,
    IDS_PMP_RECORDER_MENU_AUDIO_PARAM
};

RECORDER_MENU_PARAMS recorder_menu_params = { 0, 1, 0, 0, 0, 0, 0, 0, 0 };

static int _recorder_menu_changeparam_callback ( void *context, int id, int op, char *text_buff );
static void _recorder_menu_wavchange ( RECORDER_SECOND_MENU_PARAMS* my_params );
static void _recorder_menu_mp3change ( RECORDER_SECOND_MENU_PARAMS* my_params );
static int _recorder_menu_setrate_callback (void *context, int id, int op, char *text_buff );
static int _recorder_menu_setmode_callback ( void *context, int id, int op, char *text_buff );

static PMP_MENU_ITEM recorder_second_menu[] =
{
    {1, _recorder_menu_changeparam_callback},
    {2, _recorder_menu_changeparam_callback},
    {3, _recorder_menu_changeparam_callback},
    {4, _recorder_menu_setrate_callback},
    {5, _recorder_menu_setmode_callback},
    {0, NULL}
};

static int _recorder_menu_changeparam_callback (void *context, int id, int op, char *text_buff )
{
    RECORDER_SECOND_MENU_PARAMS* my_params = (RECORDER_SECOND_MENU_PARAMS* )context;
    switch(op ) {
        case PMP_MI_OP_DEF:
            if ( 1 == id )
                sprintf (text_buff, _(recorder_second_menu_item_text_id[0]), bit_perpoint_set_str );
            if ( 2 == id )
                sprintf (text_buff, _(recorder_second_menu_item_text_id[1]), _(channel_num_set_str[my_params->channel_num]) );
            if ( 3 == id )
                sprintf (text_buff, _(recorder_second_menu_item_text_id[2]), sample_rate_set_str[my_params->sample_rate] );
            return PMP_MI_ST_UPDATED;
        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;
    }
    return PMP_MI_ST_NONE;
}

static void _recorder_menu_wavchange ( RECORDER_SECOND_MENU_PARAMS* my_params )
{
    if(my_params->bit_rate_wav==0 )
    {
        my_params->channel_num=0;
        my_params->sample_rate=0;
    }
    if(my_params->bit_rate_wav==1 )
    {
        my_params->channel_num=0;
        my_params->sample_rate=1;
    }
    if(my_params->bit_rate_wav==2 )
    {
        my_params->channel_num=1;
        my_params->sample_rate=2;
    }
    if(my_params->bit_rate_wav==3 )
    {
        my_params->channel_num=1;
        my_params->sample_rate=4;
    }
}

static void _recorder_menu_mp3change ( RECORDER_SECOND_MENU_PARAMS* my_params )
{
    my_params->channel_num=1;
    if(my_params->bit_rate_mp3==0 )
        my_params->sample_rate = 1;
    else
        my_params->sample_rate = 3;
}

static int _recorder_menu_setrate_callback (void *context, int id, int op, char *text_buff )
{
    RECORDER_SECOND_MENU_PARAMS* my_params = (RECORDER_SECOND_MENU_PARAMS* )context;

    switch(op ) {

        case PMP_MI_OP_DEF:
            if ( recorder_menu_params.forma == 1 )
                sprintf ( text_buff, _(recorder_second_menu_item_text_id[3]), bit_rate_mp3_str[my_params->bit_rate_mp3] );
            if ( recorder_menu_params.forma == 0 )
                sprintf (text_buff, _(recorder_second_menu_item_text_id[3]), bit_rate_wav_str[my_params->bit_rate_wav] );
            return PMP_MI_ST_UPDATED;

        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;

        case PMP_MI_OP_PREV:

            if ( recorder_menu_params.forma == 0 && my_params->bit_rate_wav > 0 )
            {
                my_params->bit_rate_wav--;
                _recorder_menu_wavchange ( my_params );
                sprintf (text_buff, _(recorder_second_menu_item_text_id[3]), bit_rate_wav_str[my_params->bit_rate_wav] );
                if ( my_params->bit_rate_wav == 1 )
                    recorder_second_menu[1].dirty = TRUE;
                recorder_second_menu[2].dirty =TRUE;
            } 
            if ( recorder_menu_params.forma == 1 && my_params->bit_rate_mp3 > 0 )
            {
                my_params->bit_rate_mp3--;
                _recorder_menu_mp3change ( my_params );
                sprintf ( text_buff, _(recorder_second_menu_item_text_id[3]), bit_rate_mp3_str[my_params->bit_rate_mp3] );
                if ( my_params->bit_rate_mp3 == 0 )
                    recorder_second_menu[2].dirty = TRUE;
            }
            recorder_second_menu[3].dirty = TRUE;
            return PMP_MI_ST_UPDATED_OTHERS;

        case PMP_MI_OP_NEXT:
            if ( recorder_menu_params.forma == 0 && my_params->bit_rate_wav < 3 )
            {
                my_params->bit_rate_wav++;
                _recorder_menu_wavchange ( my_params );
                sprintf (text_buff, _(recorder_second_menu_item_text_id[3]), bit_rate_wav_str[my_params->bit_rate_wav] );
                if ( my_params->bit_rate_wav == 2 )
                    recorder_second_menu[1].dirty = TRUE;
                recorder_second_menu[2].dirty =TRUE;
            } 
            if ( recorder_menu_params.forma == 1 && my_params->bit_rate_mp3 < 3 )
            {
                my_params->bit_rate_mp3++;
                _recorder_menu_mp3change ( my_params );
                sprintf ( text_buff, _(recorder_second_menu_item_text_id[3]), bit_rate_mp3_str[my_params->bit_rate_mp3] );
                if ( my_params->bit_rate_mp3 == 1 )
                    recorder_second_menu[2].dirty = TRUE;
            }
            recorder_second_menu[3].dirty = TRUE;
            return PMP_MI_ST_UPDATED_OTHERS;
    }
    return PMP_MI_ST_NONE;
}

static int _recorder_menu_setmode_callback ( void *context, int id, int op, char *text_buff )
{
    RECORDER_SECOND_MENU_PARAMS* my_params = ( RECORDER_SECOND_MENU_PARAMS* ) context;

    switch ( op ) {

        case PMP_MI_OP_DEF:
            sprintf ( text_buff, _(recorder_second_menu_item_text_id[4]), _(encode_set_str[ my_params->encode_mode ]) );
            return PMP_MI_ST_UPDATED;

        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;

        case PMP_MI_OP_PREV:
            if ( my_params->encode_mode > 0 )
                my_params->encode_mode-- ;
            sprintf ( text_buff, _(recorder_second_menu_item_text_id[4]), _(encode_set_str[ my_params->encode_mode ]) );
            return PMP_MI_ST_UPDATED;

        case PMP_MI_OP_NEXT:
            if ( my_params->encode_mode < 3 )
                my_params->encode_mode++ ;
            sprintf ( text_buff, _(recorder_second_menu_item_text_id[4]), _(encode_set_str[ my_params->encode_mode ]) );
            return PMP_MI_ST_UPDATED;
    }
    return PMP_MI_ST_NONE;

}

static int _recorder_menu_setstr_callback (void *context, int id, int op, char *text_buff )
{
    RECORDER_MENU_PARAMS* my_params = (RECORDER_MENU_PARAMS* )context;
    switch(op ) {

        case PMP_MI_OP_DEF:
            if ( 1 == id )
            {
                sprintf ( text_buff, _(recorder_menu_item_text_id[0]), forma_set_str[ my_params->forma ] );
                if ( my_params->forma == 0 )
                {
                    recorder_second_menu_params.bit_rate_wav = 1;
                    recorder_second_menu_params.sample_rate = 1;
                    recorder_second_menu_params.channel_num = 0;
                }        
            }
            if ( 2 == id )
                sprintf ( text_buff, _(recorder_menu_item_text_id[1]), source_set_str[ my_params->source ] );
            if ( 4 == id )
                sprintf ( text_buff, _(recorder_menu_item_text_id[3]), _(silence_detect_set_str[ my_params->silence_detect ]) );
            if ( 7 == id )
                sprintf ( text_buff, _(recorder_menu_item_text_id[6]), _(listen_setting_str[ my_params->listen_setting ]) );
            return PMP_MI_ST_UPDATED;

        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;

        case PMP_MI_OP_PREV:
            if( 1 == id )
            {
                if ( my_params->forma > 0 )
                {
                    my_params->forma = 0;
                    recorder_second_menu_params.bit_rate_wav = 1;
                    recorder_second_menu_params.sample_rate = 1;
                    recorder_second_menu_params.channel_num = 0;
                }
                sprintf( text_buff, _(recorder_menu_item_text_id[0]), forma_set_str[ my_params->forma ] );
            }
            if ( 2 == id )
            {   if ( my_params->source > 0 )
                my_params->source--;
                sprintf( text_buff, _(recorder_menu_item_text_id[1]), source_set_str[ my_params->source ] );
            }
            if ( 4 == id )
            {   if ( my_params->silence_detect > 0 )
                my_params->silence_detect = 0;
                sprintf( text_buff, _(recorder_menu_item_text_id[3]), _(silence_detect_set_str[ my_params->silence_detect ]) );
            }
            if( 7 == id )
            {
                if ( my_params->listen_setting > 0 )
                    my_params->listen_setting = 0;
                sprintf( text_buff, _(recorder_menu_item_text_id[6]), _(listen_setting_str[ my_params->listen_setting ]) );
            }
            return PMP_MI_ST_UPDATED;

        case PMP_MI_OP_NEXT:
            if( 1 == id )
            {
                if ( my_params->forma < 1 )
                {
                    my_params->forma = 1;
                    recorder_second_menu_params.bit_rate_mp3 = 0;
                    recorder_second_menu_params.sample_rate = 1;
                    recorder_second_menu_params.channel_num = 1;
                }
                sprintf( text_buff, _(recorder_menu_item_text_id[0]), forma_set_str[ my_params->forma ] );
            }
            if( 2 == id )
            {
                if ( my_params->source < 2 )
                    my_params->source ++;
                sprintf( text_buff, _(recorder_menu_item_text_id[1]), source_set_str[ my_params->source ] );
            }
            if( 4 == id )
            {
                if ( my_params->silence_detect < 1 )
                    my_params->silence_detect = 1;
                sprintf( text_buff, _(recorder_menu_item_text_id[3]), _(silence_detect_set_str[ my_params->silence_detect ]) );
            }
            if( 7 == id )
            {
                if ( my_params->listen_setting < 1 )
                    my_params->listen_setting = 1;
                sprintf( text_buff, _(recorder_menu_item_text_id[6]), _(listen_setting_str[ my_params->listen_setting ]) );
            }
            return PMP_MI_ST_UPDATED;            
    }

    return PMP_MI_ST_NONE;
}

int _recorder_menu_set_audioparam_callback (void *context, int id, int op, char *text_buff )
{
    switch(op ) {
        case PMP_MI_OP_DEF:
            sprintf( text_buff, _(recorder_menu_item_text_id[9]) );
            return PMP_MI_ST_UPDATED;

        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;
    }
    return PMP_MI_ST_NONE;
}

static int _recorder_menu_setint_callback (void *context, int id, int op, char *text_buff )
{
    RECORDER_MENU_PARAMS* my_params = ( RECORDER_MENU_PARAMS* )context;

    switch(op ) {
        case PMP_MI_OP_DEF:
            if( 3 == id )
                sprintf ( text_buff, _(recorder_menu_item_text_id[2]), my_params->record_gain );
            if( 5 == id )
                sprintf ( text_buff, _(recorder_menu_item_text_id[4]), my_params->vor_time );
            if( 6 == id )
                sprintf ( text_buff, _(recorder_menu_item_text_id[5]), my_params->volume_setting );
            if( 8 == id )
                sprintf ( text_buff, _(recorder_menu_item_text_id[7]), my_params->listen_volume );
            if( 9 == id )
                sprintf ( text_buff, _(recorder_menu_item_text_id[8]), my_params->last_time );
            return PMP_MI_ST_UPDATED;

        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;

        case PMP_MI_OP_PREV:
            if( 3 == id )
            {
                if ( my_params->record_gain > 0 )
                    my_params->record_gain--;
                sprintf( text_buff, _(recorder_menu_item_text_id[2]), my_params->record_gain );
            }
            if( 5 == id )
            {
                if ( my_params->vor_time > 0 )
                    my_params->vor_time--;
                sprintf( text_buff, _(recorder_menu_item_text_id[4]), my_params->vor_time );
            }
            if( 6 == id )
            {
                if (my_params->volume_setting > 0 )
                    my_params->volume_setting--;
                sprintf( text_buff, _(recorder_menu_item_text_id[5]), my_params->volume_setting );
            }
            if( 8 == id )
            {
                if (my_params->listen_volume > 0 )
                    my_params->listen_volume--;
                sprintf( text_buff, _(recorder_menu_item_text_id[7]), my_params->listen_volume );
            }
            if( 9 == id )
            {
                if ( my_params->last_time > 0 )
                    my_params->last_time--;
                sprintf( text_buff, _(recorder_menu_item_text_id[8]), my_params->last_time );
            }
            return PMP_MI_ST_UPDATED;

        case PMP_MI_OP_NEXT:
            if( 3 == id )
            {
                if ( my_params->record_gain < 10 )
                    my_params->record_gain++;
                sprintf( text_buff, _(recorder_menu_item_text_id[2]), my_params->record_gain );
            }
            if( 5 == id )
            {
                if ( my_params->vor_time < 10 )
                    my_params->vor_time++;
                sprintf( text_buff, _(recorder_menu_item_text_id[4]), my_params->vor_time );
            }
            if ( 6 == id )
            {
                if (my_params->volume_setting < 31 )
                    my_params->volume_setting++;
                sprintf( text_buff, _(recorder_menu_item_text_id[5]), my_params->volume_setting );
            }
            if ( 8 == id )
            {
                if ( my_params->listen_volume < 31 )
                    my_params->listen_volume++;
                sprintf( text_buff, _(recorder_menu_item_text_id[7]), my_params->listen_volume );
            }
            if ( 9 == id )
            {
                my_params->last_time++;
                sprintf( text_buff, _(recorder_menu_item_text_id[8]), my_params->last_time );
            }
            return PMP_MI_ST_UPDATED;            
    }
    return PMP_MI_ST_NONE;
}

static PMP_MENU_ITEM recorder_menu[] =
{
    {1, _recorder_menu_setstr_callback},
    {2, _recorder_menu_setstr_callback},
    {3, _recorder_menu_setint_callback},
    {4, _recorder_menu_setstr_callback},
    {5, _recorder_menu_setint_callback},
    {6, _recorder_menu_setint_callback},
    {7, _recorder_menu_setstr_callback},
    {8, _recorder_menu_setint_callback},
    {9, _recorder_menu_setint_callback},
    {10, _recorder_menu_set_audioparam_callback},
    {0, NULL}
};

static int RecorderProc ( HWND hWnd, int message, WPARAM wParam, LPARAM lParam )
{
    //FILE *stream;
    char buf[100];
    static void *recorderHandle;
    static unsigned int key_number;
    static BOOL LongPush = FALSE;
    static int start_record = 0;
    static int pause_or_continue = 0;
    static char record_filename_number[10];
    
    static unsigned int second;
    static char status[30];

    static record_status_t status_info;
    static disk_info_t disk_info;
    static audio_resource_t record_source = { 0, 1, 0 };
    static audio_param_t record_param;

    switch ( message ) {
        case MSG_NCCREATE:
#ifdef BITMAPBK
            backbitmap = pmp_image_res_acquire_bitmap ( BK_RECORDER ); 
            SetWindowAdditionalData2( hWnd, ( DWORD )backbitmap );
#else
            SetWindowAdditionalData2 (hWnd, ( DWORD ) BK_RECORDER );
#endif
            break;
        case MSG_CREATE:
            recorderHandle = audioEncOpen ( NULL );
            sprintf(status,"%s",_(IDS_PMP_RECORDER_STOP_RECORDER) );
            InvalidateRect ( hWnd, &rect_statu[0], TRUE );
            audioEncCmd ( recorderHandle, SET_SAVE_PATH, ( unsigned int ) "/media/recorder/" );
            audioEncCmd ( recorderHandle, GET_STATUS, ( unsigned int ) &status_info );
            audioEncCmd ( recorderHandle, GET_DISK_INFO, ( unsigned int ) &disk_info );
            SetTimer ( hWnd, ID_STATUS_TIMER, 100 );
            break;
        case MSG_SETFOCUS:
            
            /* When recorder is lanched by picture, pmp_show_captionbar(TRUE)
             * must firstly be called to show captionbar.  */            
            pmp_show_captionbar (TRUE);
            pmp_set_captionbar_title ( _(IDS_PMP_RECORDER_TITLE) );
            break;

        case MSG_DESTROY:
            DestroyAllControls ( hWnd );
            hMainWnd = HWND_INVALID;
            return 0; 

        case MSG_CLOSE:
            audioEncCmd ( recorderHandle, START_MONITOR, 0 );
            audioEncClose ( recorderHandle, NULL );
#ifdef BITMAPBK
            pmp_image_res_release_bitmap ( BK_RECORDER, backbitmap);
#endif
            DestroyMainWindow ( hWnd );
            MainWindowCleanup ( hWnd );
            pmp_app_notify_closed (PMP_APP_RECORDER );
            start_record = 0;
            pause_or_continue = 0;
            return 0;
        case MSG_PAINT:
            {
                HDC hdc = BeginPaint ( hWnd );
                SetBkMode ( hdc, BM_TRANSPARENT );
                SetBkMode ( hdc, BM_TRANSPARENT );
                SetTextColor ( hdc, RGB2Pixel ( hdc, 255, 255, 255 ) ); 
                sprintf ( buf, _(IDS_PMP_RECORDER_PRESENT_STATUS), status );
                TextOut ( hdc, 105, 50, buf );
                sprintf ( buf, _(IDS_PMP_RECORDER_PRESENT_TIME), status_info.time );
                TextOut ( hdc, 105, 70, buf );
                sprintf ( buf, _(IDS_PMP_RECORDER_PRESENT_FORMA), forma_set_str[recorder_menu_params.forma] );
                TextOut ( hdc, 105, 90, buf );
                sprintf ( buf, _(IDS_PMP_RECORDER_PRESENT_SOURCE), source_set_str[recorder_menu_params.source] );
                TextOut ( hdc,105,110,buf );
                sprintf( buf, _(IDS_PMP_RECORDER_REST_TIME), disk_info.info );
                TextOut ( hdc,105,130,buf );
                if ( recorder_menu_params.forma == 0 )
                    sprintf ( buf, _(IDS_PMP_RECORDER_PRESENT_BPS), bit_rate_wav_str[recorder_second_menu_params.bit_rate_wav] );
                if ( recorder_menu_params.forma == 1 )
                    sprintf ( buf, _(IDS_PMP_RECORDER_PRESENT_BPS), bit_rate_mp3_str[recorder_second_menu_params.bit_rate_mp3] );
                TextOut ( hdc, 105, 150, buf );
                EndPaint ( hWnd, hdc );
                break;
            }
        case MSG_TIMER:
            if( wParam == _ID_TIMER )
            {
                LongPush = TRUE;
                PostMessage ( hWnd, MSG_USER+1, key_number, PUSH_LONG );
                InvalidateRect ( hWnd, &rect_statu[0], TRUE );
            }
            else if ( wParam == ID_LAST_TIMER )
            {
                KillTimer ( hWnd, ID_LAST_TIMER );
                start_record = 0;
                pause_or_continue = 0;
                audioEncCmd ( recorderHandle, STOP_RECORDING, 0 );
                sprintf(status,"%s",_(IDS_PMP_RECORDER_STOP_RECORDER) );
                InvalidateRect ( hWnd, &rect_statu[0], TRUE );
            }
            else
            {
                audioEncCmd ( recorderHandle, GET_STATUS, ( unsigned int ) &status_info );
                audioEncCmd ( recorderHandle, GET_DISK_INFO, ( unsigned int ) &disk_info );
                InvalidateRect ( hWnd, &rect_statu[1], TRUE );
                InvalidateRect ( hWnd, &rect_statu[4], TRUE );
            }
            break;
        case MSG_KEYDOWN:
            switch ( wParam ) {
                case SCANCODE_F1:
                    if (start_record == 1 )
                        break;
                    switch ( pmp_show_menu ( hWnd, _(IDS_PMP_RECORDER_TITLE), recorder_menu, &recorder_menu_params ) ) {
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                        case 6:
                        case 7:
                        case 8:
                        case 9:
                            InvalidateRect ( hWnd, &rect_statu[2], TRUE );
                            InvalidateRect ( hWnd, &rect_statu[3], TRUE );
                            break;
                        case 10:
                            pmp_show_menu ( hWnd, _(IDS_PMP_RECORDER_TITLE), recorder_second_menu, &recorder_second_menu_params );
                            InvalidateRect ( hWnd, &rect_statu[5], TRUE );
                    }
                    break;
                case SCANCODE_CURSORBLOCKRIGHT:
                    key_number = wParam;
                    SetTimer ( hWnd, _ID_TIMER, 120 );
                    break;
                case SCANCODE_ESCAPE:
                    PostMessage ( hMainWnd, MSG_CLOSE, 0, 0 );
                    break;
            }
            break;

        case MSG_KEYUP:
            switch ( wParam ) {
                case SCANCODE_CURSORBLOCKRIGHT:
                    KillTimer ( hWnd, _ID_TIMER );
                    if(  LongPush == FALSE ) 
                    {
                        PostMessage ( hWnd, MSG_USER+1, key_number, PUSH_SHORT );
                    }
                    LongPush = FALSE;

                    break;
            }
            break;
        case MSG_USER+1:
            if ( lParam == PUSH_LONG && start_record == 1 )
            {
                start_record = 0;
                audioEncCmd ( recorderHandle, STOP_RECORDING, 0 );
                sprintf(status,"%s",_(IDS_PMP_RECORDER_STOP_RECORDER) );
                pause_or_continue = 0;
                InvalidateRect ( hWnd, &rect_statu[0], TRUE );
            }
            else if ( lParam == PUSH_SHORT )
            {
                if ( start_record == 0 && pause_or_continue == 0 )
                {
                    if ( recorder_menu_params.last_time != 0 )
                        SetTimer ( hWnd, ID_LAST_TIMER, 100*( recorder_menu_params.last_time ) );
                    audioEncCmd ( recorderHandle, SET_FILE_FMT, ( unsigned int ) forma_set_str[ recorder_menu_params.forma ] );
                    if ( recorder_menu_params.source == 0 )
                    { 
                        record_source.mic = 1;
                        record_source.fm = 0;
                        record_source.line_in = 0;
                    }
                    if ( recorder_menu_params.source == 1 )
                    {	
                        record_source.mic = 0;
                        record_source.fm = 1;
                        record_source.line_in = 0;
                    }
                    if ( recorder_menu_params.source == 2 )
                    {	
                        record_source.mic = 0;
                        record_source.fm = 0;
                        record_source.line_in = 1;
                    }
                    audioEncCmd ( recorderHandle, SET_AUDIO_SOURCE, ( unsigned int ) &record_source );
                    audioEncCmd ( recorderHandle, SET_VOR_TIME, recorder_menu_params.vor_time );
                    record_param.sample_rate = atoi( sample_rate_set_str[recorder_second_menu_params.sample_rate] );
                    record_param.bpp = 24;
                    if ( recorder_menu_params.forma == 0 )
                        record_param.bitrate = atoi( bit_rate_wav_str[recorder_second_menu_params.bit_rate_wav] );
                    else if ( recorder_menu_params.forma == 1 )
                        record_param.bitrate = atoi( bit_rate_mp3_str[recorder_second_menu_params.bit_rate_mp3] );
                    if ( recorder_second_menu_params.encode_mode == 0 )
                        record_param.encode_mode = STEREO_AUDIO;
                    if ( recorder_second_menu_params.encode_mode == 1 )
                        record_param.encode_mode = JOINT_STEREO_AUDIO;
                    if ( recorder_second_menu_params.encode_mode == 2 )
                        record_param.encode_mode = DUAL_CHANNEL_AUDIO;
                    if ( recorder_second_menu_params.encode_mode == 3 )
                        record_param.encode_mode = MONO_AUDIO;
                    if ( recorder_second_menu_params.channel_num == 0 )
                        record_param.channel = 1;
                    if ( recorder_second_menu_params.channel_num == 1 )
                        record_param.channel = 2;
                    audioEncCmd ( recorderHandle, SET_AUDIO_PARAM, ( unsigned int ) &record_param );
                    audioEncCmd ( recorderHandle, SET_ENERGY_LEVEL, ( unsigned int ) 4 );
                    audioEncCmd ( recorderHandle, SET_VOR_GAIN, ( int ) recorder_menu_params.record_gain );
                    /*946656093 if the seconds from 1970/1/1 00:00:00 to 2000/1/1 00:00:00*/
                    second = time( ( time_t * ) NULL ) - 946656093;
                    sprintf (record_filename_number, "%X", second );
                    audioEncCmd ( recorderHandle, START_RECORDING, ( unsigned int ) record_filename_number );

                    if( recorder_menu_params.listen_setting == 0 )
                        audioEncCmd ( recorderHandle, START_MONITOR, 0 );
                    if( recorder_menu_params.listen_setting == 1 )
                        audioEncCmd ( recorderHandle, STOP_MONITOR, 0 );
                    if( recorder_menu_params.silence_detect == 0 )
                        audioEncCmd ( recorderHandle, SILENCE_DECTECT_ON, 0 );
                    if( recorder_menu_params.silence_detect == 1 )
                        audioEncCmd ( recorderHandle, SILENCE_DECTECT_OFF, 0 );

                    sprintf(status,"%s",_(IDS_PMP_RECORDER_START_RECORDER) );
                    InvalidateRect ( hWnd, &rect_statu[0], TRUE );
                    InvalidateRect ( hWnd, &rect_statu[1], TRUE );
                    InvalidateRect ( hWnd, &rect_statu[4], TRUE );
                    start_record = 1;
                }
                else if ( start_record == 1 )
                {
                    if ( pause_or_continue == 0 )
                    {
                        sprintf(status, "%s", _(IDS_PMP_RECORDER_PAUSED_RECORDER) );
                        InvalidateRect ( hWnd, &rect_statu[0], TRUE );
                        audioEncCmd ( recorderHandle, PAUSE_RECORDING, 0 );
                    }
                    else
                    {
                        sprintf(status, "%s", _(IDS_PMP_RECORDER_START_RECORDER) );
                        InvalidateRect ( hWnd, &rect_statu[0], TRUE );
                        audioEncCmd ( recorderHandle, START_RECORDING, ( unsigned int ) record_filename_number );
                    }
                    pause_or_continue = !pause_or_continue;
                }
            }
            break;
    }
    return pmp_def_wnd_proc ( hWnd, message, wParam, lParam );
}

static void _recorder_init_create_info ( PMAINWINCREATE pCreateInfo )
{
    pCreateInfo->dwStyle = WS_BORDER | WS_VISIBLE;
    pCreateInfo->dwExStyle = WS_EX_NONE;
    pCreateInfo->spCaption = _(IDS_PMP_RECORDER_TITLE);
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor(0 );
    pCreateInfo->hIcon = 0;
    pCreateInfo->MainWindowProc = RecorderProc;
    pCreateInfo->lx = 0;
    pCreateInfo->ty = 20;
    pCreateInfo->rx = MAINWINDOW_W;
    pCreateInfo->by = MAINWINDOW_H;
    pCreateInfo->iBkColor = COLOR_lightgray;
    pCreateInfo->dwAddData = 0;
    pCreateInfo->hHosting = HWND_DESKTOP;
}

void ex_recorder_show_window ( HWND parent )
{
    MAINWINCREATE CreateInfo;

    _recorder_init_create_info ( &CreateInfo );
    CreateInfo.hHosting = parent;
    hMainWnd = CreateMainWindow ( &CreateInfo );
    pmp_app_notify_opened (PMP_APP_RECORDER, hMainWnd );

    ShowWindow ( hMainWnd, SW_SHOWNORMAL );
}

#endif /* PMP_APP_RECORDER */
