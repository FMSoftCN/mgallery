#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "pmp.h"
#include "text_id.h"

#include "decoder.h"
static char ShowText[100];
static BOOL show_volem=FALSE;
static RECT rect_volem ={60,180,200+60,10+180};
static RECT rect_infor ={60,100,200+60,100+40};
static radio_info_t *radio_info_ptr;
static radio_info_t radio_info;

static int list_id[]={
    IDS_PMP_RADIO_MODE,
    IDS_PMP_RADIO_GAMSEAR,
    IDS_PMP_RADIO_FINETURN,
    IDS_PMP_RADIO_GAMLIST,
    IDS_PMP_RADIO_SAVEGAM,
    IDS_PMP_RADIO_DELGAM,
    IDS_PMP_RADIO_RECORD,    
};
static unsigned int game_list[2][50];
static game_idex=0;
static int model_id[]={
    IDS_PMP_RADIO_MODE_USA,
    IDS_PMP_RADIO_MODE_JAP
};

static int   current_mode=0;

static unsigned int volem=0;
static unsigned int frequency=10;
static void  do_get_radio_infof (void)
{
    radioCmd(radio_info_ptr, GET_RADIO_INFO, (unsigned int)&radio_info);
    frequency =radio_info.freq ;
    current_mode =radio_info.cur_mode;

}
static void read_game_frome_file (void)
{
    FILE * radio_cfg;
    int i,j;
    char number[10];
    radio_cfg=fopen("./radio.cfg","r");

    if(radio_cfg){
      for(j=0;j<2;j++)
      for(i=0;i<50;i++){
        if(!fgets(number,20,radio_cfg))
          break;
          game_list[j][i]=strtol(number,NULL,10);
      }

      fclose(radio_cfg);
    }

}
static void save_game_to_file (void)
{
    FILE * radio_cfg;
    int i,j;
    char number[10];
    radio_cfg=fopen("./radio.cfg","w+");
    if(radio_cfg){
      for(j=0;j<2;j++)
      for(i=0;i<50;i++){
       fprintf(radio_cfg,"%d\n",game_list[j][i]); }
       fclose(radio_cfg);
    }

}
static int mode_set_callback (void *context, int id, int op, char *text_buff)
{
    switch(op) {
        case PMP_MI_OP_DEF:
            sprintf(text_buff,"%s\t%s" ,_(list_id[0]),_(model_id[current_mode]) );
            return PMP_MI_ST_UPDATED;
        case PMP_MI_OP_ENTER:
            radioCmd(radio_info_ptr, SET_RADIO_MODE,current_mode);
            if(current_mode)
            {
                 radioCmd(radio_info_ptr, SET_RADIO_FREQ,76000);
            }else
            {
                 radioCmd(radio_info_ptr, SET_RADIO_FREQ,87000);
            }

            return PMP_MI_ST_SELECTED;
        case PMP_MI_OP_PREV:
        case PMP_MI_OP_NEXT:
            current_mode++;
            current_mode %= 2;
            return PMP_MI_ST_UPDATED;            
    }
    return PMP_MI_ST_NONE;
}

static int gam_sear_callback (void *context, int id, int op, char *text_buff)
{
    switch(op) {
        case PMP_MI_OP_DEF:
            sprintf(text_buff,"%s" ,_(list_id[1]) );
            return PMP_MI_ST_UPDATED;
        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;
        case PMP_MI_OP_PREV:
            return PMP_MI_ST_UPDATED;
        case PMP_MI_OP_NEXT:
            return PMP_MI_ST_UPDATED;            
    }
    return PMP_MI_ST_NONE;
}
static int fine_turn_callback (void *context, int id, int op, char *text_buff)
{
    switch(op) {
        case PMP_MI_OP_DEF:
            sprintf(text_buff,"%s\t%dKHz" ,_(list_id[2]),frequency);
            return PMP_MI_ST_UPDATED;
        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;
        case PMP_MI_OP_PREV:
            frequency-=10;
            radioCmd(radio_info_ptr, SET_RADIO_FREQ,frequency);
            do_get_radio_infof(); 
            return PMP_MI_ST_UPDATED;
        case PMP_MI_OP_NEXT:
            frequency+=10;
            radioCmd(radio_info_ptr, SET_RADIO_FREQ,frequency);
            do_get_radio_infof(); 
            return PMP_MI_ST_UPDATED;            
    }
    return PMP_MI_ST_NONE;
}
static int game_list_callback (void *context, int id, int op, char *text_buff)
{
    switch(op) {
        case PMP_MI_OP_DEF:
            sprintf(text_buff,"%s\t%dKHz" ,_(list_id[3]), game_list[current_mode][game_idex] );
            return PMP_MI_ST_UPDATED;
        case PMP_MI_OP_ENTER:
            radioCmd(radio_info_ptr, SET_RADIO_FREQ,game_list[current_mode][game_idex]);
            return PMP_MI_ST_SELECTED;
        case PMP_MI_OP_PREV:
            if (game_idex>1)
                game_idex--;
            return PMP_MI_ST_UPDATED;
        case PMP_MI_OP_NEXT:
            if (game_idex<game_list[current_mode][0])
                game_idex++;
            return PMP_MI_ST_UPDATED;            
    }
    return PMP_MI_ST_NONE;
}
static void do_save_game (void )
{
    int x;
    
    for (x =1;x<=game_list[current_mode][0]; x++)
    {
        if (abs(game_list[current_mode][x]-frequency)<100)
            return ;
    }
    if (x>49) 
        return ;
    game_list[current_mode][x] =frequency;
    game_list[current_mode][0]++;
}

static void do_dele_game (void)
{
    int x = game_idex;
    
    for (;x<game_list[current_mode][0]; x++)
    {
        game_list[current_mode][x] =game_list[current_mode][x+1];
    }

    game_list[current_mode][0]--;
}
static int game_save_callback (void *context, int id, int op, char *text_buff)
{
    switch(op) {
        case PMP_MI_OP_DEF:
            sprintf(text_buff,"%s" ,_(list_id[4]));
            return PMP_MI_ST_UPDATED;
        case PMP_MI_OP_ENTER:
            do_save_game ();
            return PMP_MI_ST_SELECTED;
        case PMP_MI_OP_PREV:
            return PMP_MI_ST_UPDATED;
        case PMP_MI_OP_NEXT:
            return PMP_MI_ST_UPDATED;            
    }
    return PMP_MI_ST_NONE;
}

static int game_dele_callback (void *context, int id, int op, char *text_buff)
{
    switch(op) {
        case PMP_MI_OP_DEF:
            sprintf(text_buff,"%s" ,_(list_id[5]));
            return PMP_MI_ST_UPDATED;
        case PMP_MI_OP_ENTER:
            do_dele_game ();
            return PMP_MI_ST_SELECTED;
        case PMP_MI_OP_PREV:
            return PMP_MI_ST_UPDATED;
        case PMP_MI_OP_NEXT:
            return PMP_MI_ST_UPDATED;            
    }
    return PMP_MI_ST_NONE;
}
static int fm_recod_callback (void *context, int id, int op, char *text_buff)
{
    switch(op) {
        case PMP_MI_OP_DEF:
            sprintf(text_buff,"%s" ,_(list_id[6]));
            return PMP_MI_ST_UPDATED;
        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;
        case PMP_MI_OP_PREV:
            return PMP_MI_ST_UPDATED;
        case PMP_MI_OP_NEXT:
            return PMP_MI_ST_UPDATED;            
    }
    return PMP_MI_ST_NONE;
}

static PMP_MENU_ITEM radio_callback_menu[] =
{
    {1, mode_set_callback },
    {2, gam_sear_callback },
    {3, fine_turn_callback},
    {4, game_list_callback},
    {5, game_save_callback},
    {6, game_dele_callback},
    {7, fm_recod_callback},
    {0, NULL}
};
static void sarch_game (HWND hWnd)
{
    int x=1;

    if(current_mode)
    {
         radioCmd(radio_info_ptr, SET_RADIO_FREQ,76000);
    }else
    {
         radioCmd(radio_info_ptr, SET_RADIO_FREQ,87000);
    }
    
    while(-1!= radioCmd(radio_info_ptr, SEARCH, 0)&& x<49)
    {
        do_get_radio_infof ();
        game_list[current_mode][x]=frequency;
        SendMessage(hWnd,MSG_PAINT,0,0);
        x++; 
    }
    game_list[current_mode][0] =--x;
    game_idex =1;
}
static void do_radio_operation (HWND hWnd,int rc)
{
    switch(rc)
    {
        case 2: 
            sarch_game(hWnd);
            break;
        case 7:
            start_app(PMP_APP_RECORDER); 
            break;
    }
    
}

static void do_vol_up(HWND hwnd)
{
int tmp;
    if (volem>200)
        return ;
    radioCmd(radio_info_ptr, GET_RADIO_INFO, (unsigned int)&radio_info);
    tmp =radio_info.cur_volume+1;
    radioCmd(radio_info_ptr, SET_VOLUME, tmp);
    radioCmd(radio_info_ptr, GET_RADIO_INFO, (unsigned int)&radio_info);
    volem =radio_info.cur_volume*200/31;
}

static void do_vol_down(HWND hwnd)
{
int tmp;
    if (volem<1)
        return ;
    radioCmd(radio_info_ptr, GET_RADIO_INFO, (unsigned int)&radio_info);
    tmp =radio_info.cur_volume-1;
    radioCmd(radio_info_ptr, SET_VOLUME, tmp);
    radioCmd(radio_info_ptr, GET_RADIO_INFO, (unsigned int)&radio_info);
    volem =radio_info.cur_volume*200/31;

}

static int pmp_radio_win_proc(HWND hWnd, int message, 
                WPARAM wParam, LPARAM lParam)
{
    int rc;
    HDC  hdc;
    switch (message) {
        case MSG_CREATE:
            show_volem=FALSE;
            pmp_app_notify_opened (PMP_APP_RADIO, hWnd);
            radio_info_ptr = radioOpen(NULL);
            radioCmd(radio_info_ptr, PLAY, 0);
            do_get_radio_infof ();
            read_game_frome_file ();
            break;
        case MSG_TIMER:
            show_volem=FALSE;
            KillTimer (hWnd, 100);
            InvalidateRect(hWnd,&rect_volem,TRUE);
            break;
        case MSG_SETFOCUS:
            pmp_set_captionbar_title (_(IDS_PMP_RADIO_CAPTION));
            break;            
        case MSG_PAINT:
            hdc = BeginPaint (hWnd);
            SetBkMode(hdc, BM_TRANSPARENT);
            SetTextColor(hdc, RGB2Pixel(hdc, 255, 255, 0));
            SetBrushColor (hdc,PIXEL_yellow);
            FillBox(hdc,60,100,200,40); 
            SetBrushColor (hdc,RGB2Pixel(HDC_SCREEN, 71, 156, 210));
            FillBox(hdc,61,101,198,38);
            sprintf(ShowText,"%s:  %d%s",_(model_id[current_mode]),frequency,"KHZ");
            TextOut(hdc,72,107,ShowText);
            if(show_volem)
            { 
                FillBox(hdc,60,180,200,10); 
                SetBrushColor (hdc,PIXEL_yellow);
                FillBox(hdc,61,181,volem,8); 
            }
            EndPaint (hWnd, hdc);
            break;
        case MSG_KEYDOWN:
            switch (LOWORD(wParam)){
                case SCANCODE_ESCAPE:
                    SendNotifyMessage(hWnd, MSG_CLOSE, 0, 0) ;
                    break;
                case SCANCODE_F1:
                    rc = pmp_show_menu (hWnd, _(IDS_PMP_RADIO_OPTION), 
                            radio_callback_menu, NULL);
                    if (rc)
                        do_radio_operation (hWnd, rc);
                    do_get_radio_infof();
                    break;

                case SCANCODE_CURSORBLOCKUP:
                    do_vol_up(hWnd);
                    show_volem=TRUE;
                    KillTimer (hWnd, 100);
                    SetTimer (hWnd, 100, 200);
                    do_get_radio_infof ();
                    InvalidateRect(hWnd,&rect_volem,FALSE);
                    break;
                case SCANCODE_CURSORBLOCKDOWN:
                    do_vol_down(hWnd);
                    show_volem=TRUE;
                    KillTimer (hWnd, 100);
                    SetTimer (hWnd, 100, 200);
                    do_get_radio_infof ();
                    InvalidateRect(hWnd,&rect_volem,FALSE);
                    break;
                case SCANCODE_CURSORBLOCKRIGHT:
                    radioCmd(radio_info_ptr, SEARCH, 0);
                    do_get_radio_infof ();
                    InvalidateRect(hWnd,&rect_infor,FALSE);
                    break;
                case SCANCODE_CURSORBLOCKLEFT :
                    radioCmd(radio_info_ptr, SEARCH, 1);
                    do_get_radio_infof ();
                    InvalidateRect(hWnd,&rect_infor,FALSE);
                    break;
            }
            break;

        case MSG_CLOSE:
            radioClose(radio_info_ptr, NULL);
            DestroyMainWindow (hWnd);
            MainWindowThreadCleanup (hWnd);
            pmp_app_notify_closed (PMP_APP_RADIO);
            save_game_to_file ();
            return 0;
    }

    return pmp_def_wnd_proc (hWnd, message, wParam, lParam);            
}

void ex_radio_show (HWND parent)
{
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;

    CreateInfo.dwStyle = WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "radio";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = pmp_radio_win_proc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 20;
    CreateInfo.rx = 320;
    CreateInfo.by = 240;
    CreateInfo.iBkColor =0;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = parent;

    hMainWnd = CreateMainWindow (&CreateInfo);

    if (hMainWnd == HWND_INVALID)
        return ;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);
}


