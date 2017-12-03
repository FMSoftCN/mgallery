/*
 ** $Id$
 **
 ** m_widget_picture.c: a widget of picture.
 **
 ** Copyright (C) 2006 Feynman Software.
 **
 ** All right reserved by Feynman Software.
 **
 ** Current maintainer: Wang Minggang.
 **
 ** Create date: 2007/04/15
 */

#include <stdio.h>
#include <string.h>

#undef _DEBUG_MSG
#include "pmp.h"

#include <minigui/control.h>

#include "picture.h"
//#undef EVB
static BITMAP    g_bitmap;
static FileInfor PictureInfo;
static char *ShowText;
static int offset_x, offset_y;
static int scale_ratio;
static int rotate_angle;

#ifdef EVB
#include "../include/decoder.h"
static img_info_t img_info;
static void *handle = NULL;     
static img_file_info_t file_info;

static BOOL init_bitmap (void)
{
    img_info.rect_width = MAINWINDOW_W;
    img_info.rect_height =MAINWINDOW_H;
    img_info.bpp =2;
    img_info.len = img_info.rect_width * img_info.rect_height * img_info.bpp;
    img_info.buf = (unsigned char *)malloc(img_info.len);
    img_info.formate = RGB;

    g_bitmap.bmType=BMP_TYPE_NORMAL;
    g_bitmap.bmBitsPerPixel=16;
    g_bitmap.bmBytesPerPixel=2;
    g_bitmap.bmWidth=img_info.img_width;
    g_bitmap.bmHeight=img_info.img_height;
    g_bitmap.bmPitch=2*img_info.img_width;
    g_bitmap.bmBits=img_info.buf;
    return TRUE;
}

static void load_bitmap (char * file_name)
{
    imageDecCmd(handle,SET_FILE,(unsigned int)file_name);
    img_info.rect_width = MAINWINDOW_W;
    img_info.rect_height =MAINWINDOW_H;
    img_info.bpp =2;
    img_info.len = img_info.rect_width * img_info.rect_height * img_info.bpp;
    img_info.formate = RGB;
    imageDecCmd(handle,IMG_DECODE,(unsigned int)&img_info); 

    g_bitmap.bmType=BMP_TYPE_NORMAL;
    g_bitmap.bmBitsPerPixel=16;
    g_bitmap.bmBytesPerPixel=2;
    g_bitmap.bmWidth=img_info.img_width;
    g_bitmap.bmHeight=img_info.img_height;
    g_bitmap.bmPitch=2*img_info.img_width;
    g_bitmap.bmBits=img_info.buf;

} 
static void destroy_bitmap (void)
{
    UnloadBitmap (&g_bitmap);
}
static void get_picture_info (const char *name )
{
    const char *postfix;

    postfix=strrchr(name, '.');
    postfix++;
    imageDecCmd(handle,GET_MEDIA_INFO,(unsigned int)&file_info);
    PictureInfo.FileType [0] = '\0';
    strcat(PictureInfo.FileType, postfix); 
    PictureInfo.Depth        = 16;
    PictureInfo.Width        = file_info.width;
    PictureInfo.Height       = file_info.height;
    PictureInfo.Size         = file_info.file_len; 

}
#else
static HDC       g_mem_dc;
static MG_RWops* g_area;
static BOOL init_bitmap_mem_dc (void)
{

    if (!InitBitmap (HDC_SCREEN, MAINWINDOW_W, MAINWINDOW_H, 0, NULL, &g_bitmap))
        return FALSE;

    g_mem_dc = CreateMemDCFromBitmap (HDC_SCREEN, &g_bitmap);
    if (g_mem_dc == 0) {
        UnloadBitmap (&g_bitmap);
        return FALSE;
    }

    return TRUE;
}

static void destroy_bitmap_mem_dc (void)
{
    DeleteMemDC (g_mem_dc);
    UnloadBitmap (&g_bitmap);
}

static MG_RWops* get_picture_info (const char *file_name)
{  
    MYBITMAP my_bmp;
    MG_RWops* area;
    RGB pal [256];
    void* load_info;
    const char* ext;

    if (!(area = MGUI_RWFromFile (file_name, "rb"))) {
        return NULL;
    }

    ext = pmp_get_file_suffix (file_name);
    load_info = InitMyBitmapSL (area, ext, &my_bmp, pal);
    if (load_info == NULL) {
        MGUI_RWclose (area);
        return NULL;
    }
    CleanupMyBitmapSL (&my_bmp, load_info);

    strncpy (PictureInfo.FileType, ext, 8);
    PictureInfo.FileType [7] = '\0';
    PictureInfo.Depth        = my_bmp.depth;
    PictureInfo.Width        = my_bmp.w;
    PictureInfo.Height       = my_bmp.h;
    PictureInfo.Size         = 0; 

    return area;
}


static void fill_bitmap_mem_dc (void)
{
    int w = PictureInfo.Width * scale_ratio / 100;
    int h = PictureInfo.Height * scale_ratio / 100;

    SetBrushColor (g_mem_dc, PIXEL_lightwhite);
    FillBox (g_mem_dc,0 ,0 ,MAINWINDOW_W, MAINWINDOW_H);

    MGUI_RWseek (g_area, 0, SEEK_SET);
    StretchPaintImageEx (g_mem_dc,(MAINWINDOW_W-w)/2+offset_x,(MAINWINDOW_H-h)/2+offset_y, w, h, 
                g_area, PictureInfo.FileType);

    _MY_PRINTF ("scaled info: %d, %d, %d, %d\n",
                offset_x, offset_y, w, h);
}

#endif
static void get_best_fit_ratio (void)
{
    offset_x = 0;
    offset_y = 0;
    rotate_angle = 0;

    if (PictureInfo.Width * MAINWINDOW_H > PictureInfo.Height * MAINWINDOW_W) {
        scale_ratio = MAINWINDOW_W * 100 / PictureInfo.Width;
    }
    else {
        scale_ratio = MAINWINDOW_H * 100 / PictureInfo.Height;
    }
}

static void show_picture (HWND hwnd, HDC hdc)
{

    SetBrushColor (hdc, PIXEL_black);
    FillBox (hdc,0 ,0 ,MAINWINDOW_W, MAINWINDOW_H);
    RotateBitmap (hdc, &g_bitmap,(MAINWINDOW_W-g_bitmap.bmWidth)/2 ,(MAINWINDOW_H-g_bitmap.bmHeight)/2, rotate_angle * 64);
    SetBkMode (hdc, BM_TRANSPARENT);
    SetTextColor (hdc, RGB2Pixel(hdc, 255, 255, 0));
    TextOut (hdc, 30, 30, ShowText);
}

static int WidgetPictureProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case MSG_CREATE:
        #ifdef EVB
            init_bitmap();
        #else 
            if (!init_bitmap_mem_dc ())
                return -1;
            g_area = NULL;
        #endif
            break;

        case MSG_DESTROY:
        #ifdef EVB
           destroy_bitmap(); 
        #else
            destroy_bitmap_mem_dc ();
            if (g_area) MGUI_RWclose (g_area);
        #endif
            break;

        case SET_SHOW_INFOR:
        {
            static RECT rc_text = {30, 30, MAINWINDOW_W, 50};
            ShowText = (char *)wParam;
            InvalidateRect (hwnd, &rc_text, FALSE);
            break;
        }

        case GET_FILEINFOR:
            return(int)(&PictureInfo);

        case MOVE_PICTURE:
        #ifdef EVB
            if ((int)wParam>0)
                imageDecCmd(handle,MOVE,RIGHT);
            else
            if ((int)wParam<0)
                imageDecCmd(handle,MOVE,LEFT);
            
            if ((int)lParam>0)
                imageDecCmd(handle,MOVE,DOWN);
            else
            if ((int)lParam<0)
                imageDecCmd(handle,MOVE,UP);

        #else
            offset_x += (int)wParam;
            offset_y += (int)lParam;
            rotate_angle = 0;
            fill_bitmap_mem_dc ();
        #endif
            InvalidateRect (hwnd, NULL, FALSE);
            break;

        case ROTATE_PICTURE:
            rotate_angle = (int)wParam % 360;
            InvalidateRect (hwnd, NULL, TRUE);
            break;

        case ZOOM_PICTURE:
            if (SCALE_MIN <= (wParam + scale_ratio)
                    && (wParam + scale_ratio) <= SCALE_MAX) {  
            #ifdef EVB
                if ((int)wParam>0)
                   imageDecCmd(handle,ZOOM,ZOOMIN); 
                else
                   imageDecCmd(handle,ZOOM,ZOOMOUT);
            #else
                scale_ratio += wParam;
                fill_bitmap_mem_dc ();
            #endif
                InvalidateRect (hwnd, NULL, FALSE);
                return 0;
            }
            return -1;

        case PIC_SET_FILE:
        {
        #ifdef EVB
            handle=(void *)lParam;
            load_bitmap ((char*) wParam);
            get_picture_info((const char*)wParam);
            get_best_fit_ratio ();
            InvalidateRect (hwnd, NULL, FALSE);
        #else
            
            if (g_area) {
                MGUI_RWclose (g_area);
                g_area = NULL;
            }
            
            g_area = get_picture_info ((const char *)wParam);

            if (g_area) {
                get_best_fit_ratio ();
                fill_bitmap_mem_dc ();
                InvalidateRect (hwnd, NULL, FALSE);
                return 0;
            }
        #endif
            return 1;
        }

        case MSG_PAINT:
        {
            HDC hdc = BeginPaint (hwnd);
            show_picture (hwnd, hdc);
            EndPaint (hwnd, hdc);
            return 0;
        }
    }

    return DefaultControlProc (hwnd, message, wParam, lParam);
}

BOOL RegisterWidgetPicture (void)
{
    WNDCLASS MyClass;

    MyClass.spClassName = WIDGET_PICTURE;
    MyClass.dwStyle     = WS_NONE;
    MyClass.dwExStyle   = WS_EX_NONE;
    MyClass.hCursor     = GetSystemCursor (IDC_ARROW);
    MyClass.iBkColor    = COLOR_lightwhite ;
    MyClass.WinProc     = WidgetPictureProc;

    return RegisterWindowClass (&MyClass);
}

void UnregisterWidgetPicture (void)
{
    UnregisterWindowClass (WIDGET_PICTURE);
}

