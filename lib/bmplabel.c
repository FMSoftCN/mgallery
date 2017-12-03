/*
** $Id$
**
** bmplable.c: Draw label with a bitmap.
**
** Copyright (C) 2007 Feynman Software, all rights reserved.
**
** Use of this source package is subject to specific license terms
** from Beijing Feynman Software Technology Co., Ltd.
**
** URL: http://www.minigui.com
**
** Current maintainer: Liu Peng.
*/

#include <string.h>

#include "pmp.h"

typedef struct _PMP_BMPLABEL_INFO {
    BITMAP* bmp_chars;
    int nr_label_chars;
    const char* label_chars;
} PMP_BMPLABEL_INFO;

#define MAX_COL  30
#define SetValue(Var, Value)    if (Var) {*Var = Value;}

/* get bmp char's width and height, and total col and row */
static void get_char_bmp_size (const PMP_BMPLABEL_INFO *info, int *w, int *h, 
                int *col, int *row)
{
    int _w, _h, _col, _row;

    /* get total number of col and row */
    if (info->nr_label_chars > MAX_COL) {
        _col =  MAX_COL;
        _row = (info->nr_label_chars - 1) / MAX_COL + 1;
    }
    else {
        _col = info->nr_label_chars;
        _row = 1;
    }
    _w = info->bmp_chars->bmWidth / _col;
    _h = info->bmp_chars->bmHeight/ _row;

    SetValue (w, _w);
    SetValue (h, _h);
    SetValue (col, _col);
    SetValue (row, _row);
}

static int get_char_bmp_pos (const PMP_BMPLABEL_INFO* info, char ch, 
        int* x, int* y, int* w, int* h)
{
    int i;
    int row, col;

    /* searches for the ch */
    for (i = 0 ; info->label_chars[i] != '\0' ; i++) {
        if (info->label_chars[i] == ch)
            break;
    }
    if (i >= info->nr_label_chars)    /* not found */
        return 0;

    get_char_bmp_size (info, w, h, &col, &row);

    *x = (i % col) * (*w);
    *y = (i / col) * (*h);

    return 1;
}

static void pmp_display_bmp_label (HDC hdc, const PMP_BMPLABEL_INFO *info, 
                int x, int y, const char* label)
{
    int _x, _y, _w, _h;
    const char *p;

    if (!label) return;

    p = label;
    while (p && *p != '\0') {
        if (get_char_bmp_pos (info, *p, &_x, &_y, &_w, &_h)) {
            FillBoxWithBitmapPart (hdc, x, y, _w, _h, 0, 0, 
                    info->bmp_chars, _x, _y);
            x += _w;
        }
        p++;
    }
}

#define PMP_DIGITS_IMAGE_FILE "/res/public/digits.gif"
static PMP_BMPLABEL_INFO digits_bmp_info = {NULL, 21, "0123456789ABCDEF:.-+ "};

int pmp_display_led_digits (HDC hdc, int x, int y, const char* digits)
{

    if (digits_bmp_info.bmp_chars == NULL) {
        digits_bmp_info.bmp_chars 
                = pmp_image_res_acquire_bitmap (PMP_DIGITS_IMAGE_FILE);
#if 0
        digits_bmp_info.bmp_chars->bmType = BMP_TYPE_COLORKEY;
        digits_bmp_info.bmp_chars->bmColorKey = 
                GetPixelInBitmap (digits_bmp_info.bmp_chars, 0, 0);
#endif
    }

    if (digits_bmp_info.bmp_chars == NULL)
        return -1;

    pmp_display_bmp_label (hdc, &digits_bmp_info, x, y, digits);
    return 0;
}

