/*
** $Id$
**
** video.h: Data structure & interface definitions for video app
** of Feynman PMP solution.
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

#ifndef _VIDEO_H
    #define _VIDEO_H

#include "pmp.h"
#include "decoder.h"

#define MENU_VOLUME_ID    1
#define MENU_LOOP_ID      2
#define MENU_SKIP_ID      3
#define MENU_PROGBAR_ID   4 
#define MENU_FILEINFO_ID  5

#define IDC_TIMER                       400
#define TIMER_INTERVAL                  100

typedef struct _VIDEO_INFO {
    void *dec_handle;
    PMP_MEDIA_FILE_LIST* file_list;
    int cur_idx;
} VIDEO_INFO;

typedef struct _VIDEO_MENU_PARAMS {
    int volume;
    loop_mode_t loop_mode;
    int skip_to;
    int progress_bar;
} VIDEO_MENU_PARAMS;


#endif /*End of _VIDEO_H */


