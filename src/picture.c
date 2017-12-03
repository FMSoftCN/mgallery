#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "pmp.h"
#include "text_id.h"
#include "picture.h"

#define  ID_TIMER     500
#ifdef EVB
#include "decoder.h"
#endif
//#undef EVB
enum {
    e_norm=-1,
    e_zoom,
    e_move,
    e_rotate
} mode_enum = e_norm;

static char *model[]={"zoom","move","rotate"};
static HWND picture_widget;
static BITMAP* bmp_folder;
static int slide_show_time=4;
#ifdef EVB
static void *handle = NULL;     
#endif

static int menu_item_list_id[]={
    IDS_PMP_PIC_ZOOM,
    IDS_PMP_PIC_MOVE,
    IDS_PMP_PIC_ROTATE,
    IDS_PMP_PIC_INFO,
    IDS_PMP_PIC_MUSIC,
    IDS_PMP_PIC_MATCH,
    IDS_PMP_PIC_SLIDE,
};

static PMP_DIR_SHOW_INFO my_dir_show_info[] = {
    {"..", "..", NULL},
    {"music", NULL, NULL},    
    {"video", NULL, NULL},
    {"ebook", NULL, NULL},    
    {"picture", NULL, NULL},
    {"recorder", NULL, NULL},    
    {"*", "*", NULL},
    {NULL, "", NULL},
};

static const BITMAP* cb_media_icon (const char* path, 
                const char* file_name, DWORD mode)
{
    static  char full_path [PATH_MAX + 1];
    BITMAP *icon;
#ifdef EVB
    img_info_t img_info; 
#else
    HDC mem_dc;
#endif
    icon = (BITMAP*) malloc (sizeof (BITMAP));
    if (icon == NULL)
        return NULL;
    strcpy (full_path, path);
    strcat (full_path, file_name);
#ifdef EVB
    imageDecCmd(handle,SET_FILE,(unsigned int)full_path);
    img_info.rect_width = 95;
    img_info.rect_height =95;
    img_info.bpp =2;
    img_info.len = img_info.rect_width * img_info.rect_height * img_info.bpp;
    img_info.buf = (unsigned char *)malloc(img_info.len);
    img_info.formate = RGB;
    imageDecCmd(handle,IMG_DECODE,(unsigned int)&img_info);
    icon->bmType=BMP_TYPE_NORMAL;
    icon->bmBitsPerPixel=16;
    icon->bmBytesPerPixel=2;
    icon->bmWidth=img_info.img_width;
    icon->bmHeight=img_info.img_height;
    icon->bmPitch=2*img_info.img_width;
    icon->bmBits=img_info.buf;
#else
    InitBitmap (HDC_SCREEN, 95, 95, 0, NULL, icon);
    mem_dc = CreateMemDCFromBitmap (HDC_SCREEN, icon);

    StretchPaintImageFromFile (mem_dc, 0, 0, 95, 95, (const char *)full_path);
    DeleteMemDC (mem_dc);
#endif
    return icon;
}

static BOOL cb_media_filter(const char*path, const char*file_name)
{
    const char *postfix;

    postfix = strrchr (file_name, '.');
    if (postfix == NULL)
        return FALSE;
    postfix++;
    if (strcasecmp (postfix, "jpg") == 0)
        goto ok;
    else if (strcasecmp (postfix, "jpeg") == 0)
        goto ok;
    else if (strcasecmp (postfix, "bmp") == 0)
        goto ok;
    else if (strcasecmp (postfix, "gif") == 0)
        goto ok;
#ifndef EVB 
    else if (strcasecmp (postfix, "png") == 0)
        goto ok;
#endif

    return FALSE;
ok: 
    return TRUE;   
}

static int pic_op_callback (void *context, int id, int op, char *text_buff)
{
    switch (op) {
        case PMP_MI_OP_DEF:
            strncpy (text_buff, _(menu_item_list_id[id - 1]), LEN_ITEM_TEXT);
            text_buff [LEN_ITEM_TEXT - 1] = '\0';
            return PMP_MI_ST_NONE;

        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;

        case PMP_MI_OP_PREV:
            return PMP_MI_ST_NONE;

        case PMP_MI_OP_NEXT:
            return PMP_MI_ST_NONE;            
    }
    return PMP_MI_ST_NONE;
}

static int pic_op_slide_callback (void *context, int id, int op, char *text_buff)
{
    switch (op) {
        case PMP_MI_OP_DEF:
           sprintf (text_buff, _(menu_item_list_id[id - 1]), slide_show_time);
            return PMP_MI_ST_NONE;

        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;

        case PMP_MI_OP_PREV:
            if(slide_show_time>3)
            sprintf(text_buff,_(menu_item_list_id[id - 1]), --slide_show_time);
            return PMP_MI_ST_UPDATED;

        case PMP_MI_OP_NEXT:
            if(slide_show_time<10)
            sprintf(text_buff,"%s\t%ds",_(menu_item_list_id[id - 1]),++slide_show_time);
            return PMP_MI_ST_UPDATED;            
    }
    return PMP_MI_ST_NONE;
}


static PMP_MENU_ITEM pic_callback_menu[] =
{
    {1, pic_op_callback},
    {2, pic_op_callback},
    {3, pic_op_callback},
    {4, pic_op_callback},
    {5, pic_op_callback},
    {6, pic_op_callback},
    {7, pic_op_slide_callback},
    {0, NULL}
};

static struct dirent **namelist=NULL;
static int file_count = 0;
static int selected_entry;
static PMP_MEDIA_FILE_LIST* media_file_list;

static int pmp_filter (const struct dirent * entry)
{
    if (cb_media_filter (media_file_list->dir, entry->d_name)) {
        file_count ++;
        return 1;
    }

    return 0;
}

static void free_dir_entry_list (void)
{
    while (file_count--) {
        free (namelist [file_count]);
    }
    free (namelist);
    namelist = NULL;
}

static void
make_dir_entry_list (void)
{
    file_count = 0;
    selected_entry = -1;

    scandir (media_file_list->dir, &namelist, pmp_filter, alphasort);
    
    for (selected_entry =0; selected_entry <file_count; selected_entry++){
        if (strcmp (namelist[selected_entry]->d_name, media_file_list->file_names) == 0) 
            break;
    } 
}

static void do_picture_operation (HWND hwnd, int op)
{
    FileInfor* fil_infor ;
    char message[100];
    switch (op){
        case 1:
        case 2:
        case 3:
            mode_enum = op - 1;
            SendMessage (picture_widget, SET_SHOW_INFOR, (WPARAM)model[mode_enum], 0);
            break;

        case 4:
            fil_infor = (FileInfor*)SendMessage(picture_widget, GET_FILEINFOR, 0, 0);
            sprintf(message,"\n%s\n%dX%d\n%d",
                    fil_infor->FileType,
                    fil_infor->Width,
                    fil_infor->Height,
                    fil_infor->Size);
            pmp_show_info (hwnd, _(IDS_PMP_PIC_INFOCAPT), message, MB_ICONINFORMATION | MB_OK); 
            break;

        case 5:
            start_app(PMP_APP_MUSIC);
        case 6:
            break;

        case 7:
            pmp_show_captionbar (FALSE);
            SetTimer(hwnd,ID_TIMER,slide_show_time*100);
            SendMessage (picture_widget, SLIDE_SHOW, TRUE, 0);
            break;
    }
}

static void set_picture_file (const char* dir, const char* file)
{
    char file_name [MAX_PATH + 1]; 
    strcpy (file_name, dir);
    strcat (file_name, file);
#ifdef EVB
    SendMessage (picture_widget, PIC_SET_FILE, (WPARAM)file_name, (LPARAM)handle);
#else
    SendMessage (picture_widget, PIC_SET_FILE, (WPARAM)file_name, 1);
#endif
}

static void do_picture_key_down (HWND hwnd ,int key)
{
    static int y = 0;
    switch (mode_enum){
    case e_norm:
        switch(key){
            case SCANCODE_CURSORBLOCKUP:
                if (selected_entry > 0) {
                    selected_entry --;
                    set_picture_file (media_file_list->dir, 
                        namelist [selected_entry]->d_name);
                }
                break;

            case SCANCODE_CURSORBLOCKDOWN:
                if (selected_entry < file_count - 1) {
                    selected_entry ++;
                    set_picture_file (media_file_list->dir, 
                        namelist [selected_entry]->d_name);
                }
                break;
        }
        break;

    case e_zoom:
        switch(key) {
            case SCANCODE_CURSORBLOCKUP:
                SendMessage (picture_widget, ZOOM_PICTURE, 5, MAKELONG (160, 120));
                break;
            case SCANCODE_CURSORBLOCKDOWN:
                SendMessage (picture_widget, ZOOM_PICTURE, -5, MAKELONG (160, 120)); 
                break;
        }
        break;

    case e_move:
        switch(key){
            case SCANCODE_CURSORBLOCKUP:
                SendMessage (picture_widget, MOVE_PICTURE, 0, (LPARAM)(-5));
                break;
            case SCANCODE_CURSORBLOCKDOWN:
                SendMessage (picture_widget, MOVE_PICTURE, 0, (LPARAM)(5));
                break;
            case SCANCODE_CURSORBLOCKRIGHT:
                SendMessage (picture_widget, MOVE_PICTURE, (LPARAM)(5), 0);
                break;
            case SCANCODE_CURSORBLOCKLEFT:
                SendMessage(picture_widget, MOVE_PICTURE, (LPARAM)(-5), 0);
                break;
        }
        break;

    case e_rotate:
        switch(key){
            case SCANCODE_CURSORBLOCKRIGHT:
                y+=90;
                break;
            case SCANCODE_CURSORBLOCKLEFT :
                y-=90;
                break;
        }
        SendMessage (picture_widget, ROTATE_PICTURE, (WPARAM)(y), 0);

        break;
    }
}

static void select_image_file (HWND hWnd)
{ 
    int i;

    if (media_file_list != NULL){
        PMP_RELEASE_MEDIA_FILE_LIST (media_file_list);
        media_file_list = NULL;
    }

    pmp_show_captionbar (FALSE);

    i = 0;
    while (my_dir_show_info [i].dir_name) {
        my_dir_show_info [i].icon = bmp_folder;
        i++;
    }

    my_dir_show_info[0].show_text = _(IDS_PMP_DOTDOT);                     
    my_dir_show_info[1].show_text = _(IDS_PMP_MUSIC_DIRNAME);                     
    my_dir_show_info[2].show_text = _(IDS_PMP_VIDEO_DIR_ALIAS);
    my_dir_show_info[3].show_text = _(IDS_PMP_EBOOK_DIRNAME);                     
    my_dir_show_info[4].show_text = _(IDS_PMP_PIC_DIR_ALIAS);
    my_dir_show_info[5].show_text = _(IDS_PMP_RECORDER_DIRNAME);                     

    media_file_list = pmp_select_media_files (hWnd, 
            PMP_MEDIA_TOP_DIR, PMP_MEDIA_NAME_PICTURE, 
            cb_media_filter, cb_media_icon, my_dir_show_info, 
            PMP_BROWSER_MODE_THUMBNAIL);

    pmp_show_captionbar (FALSE);

    if (namelist)
        free_dir_entry_list ();

    if (media_file_list) {
        make_dir_entry_list ();
        set_picture_file (media_file_list->dir, 
                media_file_list->file_names);
    }
    else
        SendNotifyMessage (hWnd, MSG_CLOSE, 0, 0);
}

#define PMP_IMAGE_FILE_FOLDER   "/res/picture/file.png"

static int picture_win_proc (HWND hWnd, int message, 
                WPARAM wParam, LPARAM lParam)
{
    int rc;

    switch (message) {
        case MSG_CREATE:
            bmp_folder = pmp_image_res_acquire_bitmap (PMP_IMAGE_FILE_FOLDER);
            if (bmp_folder == NULL) {
                return -1;
            }

            picture_widget= CreateWindow (WIDGET_PICTURE, "", WS_VISIBLE, 504, 
                    0, 0, MAINWINDOW_W, MAINWINDOW_H, hWnd, 0);

            pmp_app_notify_opened (PMP_APP_PICTUE, hWnd);
            break;
        case MSG_KILLFOCUS:
            KillTimer(hWnd,ID_TIMER);
            break;
        case MSG_SETFOCUS:
           pmp_show_captionbar (FALSE);
            break;       
        case MSG_TIMER:
           pmp_show_captionbar (FALSE);
           ++selected_entry;
           if (selected_entry == file_count)
                selected_entry=0;
            
            set_picture_file(media_file_list->dir,
                    namelist[selected_entry]->d_name);
            break;     

        case MSG_KEYDOWN:
            switch (LOWORD(wParam)) {
                case SCANCODE_ESCAPE:
                    mode_enum = e_norm;
                    SendMessage (picture_widget, SET_SHOW_INFOR, (WPARAM)NULL, 0);
                    KillTimer(hWnd,ID_TIMER);
                    SendMessage (picture_widget, SLIDE_SHOW, FALSE, 0);
                    select_image_file (hWnd);
                    break;
                    
                case SCANCODE_F1:
                    KillTimer(hWnd,ID_TIMER);
                    SendMessage(picture_widget, SLIDE_SHOW, FALSE, 0);
                    if(mode_enum != e_norm){
                        mode_enum = e_norm;
                        SendMessage (picture_widget, SET_SHOW_INFOR, (WPARAM)NULL, 0);
                        break;
                    }

                    rc = pmp_show_menu (hWnd, _(IDS_PMP_PIC_OPTION), pic_callback_menu, NULL);
                    pmp_show_captionbar (FALSE);
                    if (rc)
                        do_picture_operation (hWnd, rc);
                    break;

                case SCANCODE_CURSORBLOCKUP:
                case SCANCODE_CURSORBLOCKDOWN:
                case SCANCODE_CURSORBLOCKRIGHT:
                case SCANCODE_CURSORBLOCKLEFT :
                    do_picture_key_down (hWnd, LOWORD(wParam));
                    break;
            }
            break;

        case MSG_CLOSE:
            if (namelist)
                free_dir_entry_list ();

            if (media_file_list!=NULL) {
                PMP_RELEASE_MEDIA_FILE_LIST (media_file_list);
                media_file_list=NULL;
            }

            pmp_image_res_release_bitmap (PMP_IMAGE_FILE_FOLDER, bmp_folder);
            bmp_folder = NULL;
        #ifdef EVB
            imageDecClose(handle,NULL);
        #endif
            DestroyMainWindow (hWnd);
            MainWindowCleanup (hWnd);
            pmp_app_notify_closed (PMP_APP_PICTUE);
            return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

void ex_picture_show (HWND parent)
{
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;

    RegisterWidgetPicture ();

    CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "PMP>Picture";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = picture_win_proc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = MAINWINDOW_W;
    CreateInfo.by = MAINWINDOW_H; 
    CreateInfo.iBkColor =0;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = parent;
    
#ifdef EVB
    handle = imageDecOpen(NULL);
    if(handle == NULL){
        _MY_PRINTF("imageDecOpen err !\n");
        return ;
    }
#endif

    hMainWnd = CreateMainWindow (&CreateInfo);

    if (hMainWnd != HWND_INVALID) {
        select_image_file (hMainWnd);
    }

    }


