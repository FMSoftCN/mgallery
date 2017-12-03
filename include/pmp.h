/*
** $Id$
**
** pmp.h: Data structure & interface definitions for Feynman PMP
** solution.
**
** Copyright (C) 2007 Feynman Software, all rights reserved.
**
** Use of this source package is subject to specific license terms
** from Beijing Feynman Software Technology Co., Ltd.
**
** URL: http://www.minigui.com
**
** Current maintainer: liupeng.
*/

#ifndef _PMP_H
    #define _PMP_H

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include "../config.h"

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define MAINWINDOW_W              320
#define MAINWINDOW_H              240

/*----------------------menu------------------------------------------*/

#define PMP_MI_OP_DEF        0x00
#define PMP_MI_OP_PREV       0x01
#define PMP_MI_OP_NEXT       0x02
#define PMP_MI_OP_ENTER      0x03
#define PMP_MI_OP_CANCEL     0x04

#define PMP_MI_ST_NONE       0x10
#define PMP_MI_ST_UPDATED    0x11
#define PMP_MI_ST_SELECTED   0x12
#define PMP_MI_ST_UPDATED_OTHERS    0x13

#define LEN_ITEM_TEXT        60

typedef int (*CB_MENU_ITEM) (void * context, int id, int op, char *text_buff);

typedef struct _PMP_MENU_ITEM{
    int id;
    CB_MENU_ITEM cb_menu_item;
    BOOL dirty;
} PMP_MENU_ITEM;

/*
 * parent: The handle to the parent main window.
 * items: The pointer to the PMP_MENU_ITEM structure array.
 * context: The context will be passed to the callback function of 
 *          every menu item.
 * The return value of this function can be zero (ESCAPE key pressed) or 
 * the identifier of the menu item which leads to close the menu.
 */
int pmp_show_menu (HWND parent, const char* title, 
                PMP_MENU_ITEM *items, void *context);


/*---------------------info window--------------------------------*/
/*
 * parent: The handle to the parent main window.
 * info: The pointer to the text which will be showed.
 */
int pmp_show_info (HWND parent, const char* title, 
                const char *info, DWORD dwStyle);

/*---------------------caption bar---------------------------------*/
/*
 * Create the captionbar window.
 */
void pmp_create_captionbar (HWND hosting);

/*
 * Destroy the captionbar window.
 */
void pmp_destroy_captionbar (void);

/*
 * show_hide: The flag to show or hide caption bar.
 *   - TRUE: show caption bar.
 *   - FALSE: hide caption bar.
 */
void pmp_show_captionbar (BOOL show_hide);

/*
 * title: the pointer to the new title text.
 */
void pmp_set_captionbar_title (const char *title);

/*
 * Return the pointer to current title text.
 */
const char *pmp_get_captionbar_title (void);

/*-------------------------- tooltip window ----------------------------------*/

/*
 * Create the tooltip window.
 */
void pmp_create_tip (HWND hosting);

/*
 * Destroy the tooltip window.
 */
void pmp_destroy_tip (void);

/*
 * x: The new x coordiate of the left-top corner of the tooltip window.
 * y: The new y coordiate of the left-top corner of the tooltip window.
 * text: the pointer to the new string which will be displayed on 
 *      the tooltip window 
 */
void pmp_update_tip (int x, int y, const char *text);

/*
 * Force to show/hide the tooltip window.
 */
void pmp_show_tip (BOOL show);

/*****************************************************************/
/*
 * key: The key value to retrive .
 * Return the pointer to image data according to index .
 * This function will be called when the main window lost the focus.
 */ 
MG_RWops*  pmp_hash_retrive_ds (const char* path);


/*****************************************************************/
                                                
/* 
 * Acquires the data source object according to the image file name.
 * file: The name of resource file such as picture etc.
 * Return a MG_RWops object, NULL indicates error.
 */
MG_RWops* pmp_image_res_acquire_ds (const char *file);

/* 
 * Releases the data source object according to the image file name.
 * file: The name of resource file such as picture etc.
 * Return a MG_RWops object, NULL indicates error.
 */
void pmp_image_res_release_ds (const char *file, MG_RWops* ds);


/* 
 * Loads a BITMAP object according to the image file name.
 * file: The name of resource file such as picture etc.
 * Return a BITMAP object, NULL indicates error.
 */
BITMAP* pmp_image_res_acquire_bitmap (const char *file);


/* 
 * Releases the BITMAP object according to the image file name.
 * file: The name of resource file such as picture etc.
 * Return a BITMAP object, NULL indicates error.
 */
void pmp_image_res_release_bitmap (const char *file, BITMAP* bmp);


/* 
 * A helper to link path and filename to get full path.
 * full_path: the buffer to store the full path.
 * path: The  buffer to store path.
 * name: The buffer to store file name.
 * Return a pointer to the full path.
 */
char* pmp_get_full_path (char *full_path, const char *path, 
                const char* name);

/* 
 * A helper to return the suffix of a file name.
 * file: The name of resource file such as picture etc.
 * Return a BITMAP object, NULL indicates error.
 */
const char* pmp_get_file_suffix (const char *file);

#define LEN_RES_TOP_DIR     126
#define LEN_RES_FILE_NAME   126

int pmp_set_res_top_dir (const char *top_dir);

/*--------------------- Internationalization ---------------------------------*/

#define _(id) pmp_get_string_from_id (id)
#define N_(String) String

BOOL pmp_select_text_domain (const char *text_domain);
const char * pmp_get_string_from_id (int id);

/*---------------------The default PMP window proc ---------------------------*/

#ifdef BITMAPBK
void pmp_set_def_bmpbg(BITMAP* bmpbg);
#endif

void pmp_draw_background (HWND hwnd, HDC hdc, const RECT* clip, 
        const char* bg_img_file);

int pmp_def_wnd_proc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam);

/*---------------------SelectFileWindow---------------------------------------*/

#ifndef PMP_CFG_PATH
#define PMP_CFG_PATH                "/media/cfg/"
#endif

#ifndef PMP_MEDIA_TOP_DIR
#define PMP_MEDIA_TOP_DIR           "/media/"
#endif

#define PMP_MEDIA_NAME_MUSIC        "music/"
#define PMP_MEDIA_NAME_VIDEO        "video/"
#define PMP_MEDIA_NAME_EBOOK        "ebook/"
#define PMP_MEDIA_NAME_PICTURE      "picture/"
#define PMP_MEDIA_NAME_RECORDER     "recorder/"

#define PMP_BROWSER_MODE_LIST       0x00000040
#define PMP_BROWSER_MODE_THUMBNAIL  0x00000041
#define PMP_BROWSER_MODE_MULTIPLE   0x00000100
#define PMP_BROWSER_MODE_DIR        0x00001000

typedef BOOL (*PMP_CB_MEDIA_FILTER) (const char*path,
                const char*file_name);
typedef const BITMAP* (*PMP_CB_MEDIA_ICON) (const char*path,
                const char* file_name, DWORD mode);

typedef struct _PMP_MEDIA_FILE_LIST {
    char    dir [PATH_MAX+1];
    int     nr_files;

    /* Pointer to buffer which stores the name of selected files . 
     * If multiple files are selected, they are seperated by '\0' 
     * such as "a.mp3\0b.ogg\0c.wav".   
     */  
    char*   file_names;
} PMP_MEDIA_FILE_LIST;

typedef struct _PMP_DIR_SHOW_INFO {
    const char*     dir_name;
    const char*     show_text;
    const BITMAP*   icon;
} PMP_DIR_SHOW_INFO;

/*
 * parent: The handle to the parent main window.
 * root_dir: The root directory of the media file tree.
 * initial_dir: The initial directory relative to the root directory.
 * cb_media_filter: The filter to determine whether to list a specified 
 *              media file.
 * cb_media_icon: The filter to return the icon bitmap object of a specified 
 *              media file.
 * dir_show_info: A structure array to define the display text and icon for 
 *              different directory name.
 * mode: The browsing mode and selectiong mode, can be one of the following 
 *      value:
 *          - PMP_FILE_BROWSE_MODE_LIST
 *          - PMP_FILE_BROWSE_MODE_THUMBNAIL 
 *     and OR'd with:
 *          - PMP_BROWSER_MODE_MULTIPLE
 *
 * Return: A PMP_MEDIA_FILE_LIST structure which includes all selected 
 *          media files.
 * Note that the caller should take the full responsibility for releasing 
 *      the memory used by the structure.
 */

PMP_MEDIA_FILE_LIST * pmp_select_media_files (HWND parent, 
                const char *root_dir, const char* initial_dir, 
                PMP_CB_MEDIA_FILTER cb_media_filter, 
                PMP_CB_MEDIA_ICON cb_media_icon, 
                PMP_DIR_SHOW_INFO* dir_show_info, DWORD mode);

#define PMP_RELEASE_MEDIA_FILE_LIST(list) \
   do {free (list->file_names); free (list); list = NULL; } while(0)

/* 
 * A helper to display the LED-style digits.
 *
 * This function returns zero on success, non-zero on failure.
 */
int pmp_display_led_digits (HDC hdc, int x, int y, const char* digits);

/* Indices of the applications */
#define PMP_APP_DESKTOP         0
#define PMP_APP_MUSIC           1
#define PMP_APP_VIDEO           2
#define PMP_APP_PICTUE          3
#define PMP_APP_RECORDER        4
#define PMP_APP_RADIO           5
#define PMP_APP_EBOOK           6
#define PMP_APP_CALENDAR        7
#define PMP_APP_CALCULATOR      8
#define PMP_APP_STOPWATCH       9
#define PMP_APP_SETUP           10
#define PMP_APP_RUSSIA          11
#define PMP_APP_WORM            12
#define PMP_APP_HOUSEKEEPER     13 
    
#define PMP_APP_NUMBER          14
    
/* 
 * Notifies to the desktop window that the application opened or closed.
 *
 * These two functions are defined in src/main.c file.
 */
void pmp_app_notify_opened (int app_idx, HWND hwnd);
void pmp_app_notify_closed (int app_idx);

#if defined(__GNUC__)
#ifdef _DEBUG_MSG
#   define _MY_PRINTF(fmt...) fprintf (stderr, fmt)
#else
#   define _MY_PRINTF(fmt...)
#endif
#else /* __GNUC__ */
static inline void _MY_PRINTF(const char* fmt, ...)
{
#ifdef _DEBUG_MSG
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
#endif
}
#endif /* __GNUC__ */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _PMP_H */

