/* 
** $Id$
**
** res_hash.h: Data structure & interface declaration for incore resource.
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

#ifndef _RES_HASH_H
#define _RES_HASH_H

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define MAX_DUP_KEYS    10
#define NR_HASH_ENTRIES 20

/*
 * An element of res hash table.
 *
 */ 
typedef struct _PMP_RES_HASH_UNIT 
{
    const char* key;
    const unsigned char* data;
    unsigned int data_size;
    BITMAP *bmp;
    int ref_count;
} PMP_RES_HASH_UNIT;

PMP_RES_HASH_UNIT* pmp_res_hash_retrive_unit (const char* path);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _RES_HASH_H */

