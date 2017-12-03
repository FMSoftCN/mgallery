/*
** $Id$
**
** resource.c: Resource interface for Feynman PMP solution.
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

#include <string.h>
#include <stdlib.h>

#undef _DEBUG_MSG

#include "pmp.h"
#include "res_hash.h"

static char res_top_dir [LEN_RES_TOP_DIR + 2];

/* Look up file in the incore resource hash table and get the BITMAP object.
 * file: The pointer to file name.
 * bmp: The pointer to the BITMAP object.
 * Return: if found, return 1; if find nothing, return 0; if error, return -1. 
 * */
static BITMAP* get_bmp_from_incore (const char *file)
{
    PMP_RES_HASH_UNIT *image;
    int nc = 0;
    
    image = pmp_res_hash_retrive_unit (file);
    if (image) {
        if (image->ref_count > 0) {
            image->ref_count++;
            _MY_PRINTF ("incore bmp ref_count++:%d\n", image->ref_count);
        }
        else {
            image->ref_count = 0; 
            if (!(image->bmp = (BITMAP *)malloc (sizeof(BITMAP))))             
                return (BITMAP*)-1;
        
            if (LoadBitmapFromMem (HDC_SCREEN, image->bmp,
                    (const void *)image->data, image->data_size,
                    pmp_get_file_suffix (file))) {
                
                fprintf (stderr, "LoadSystemBitmapEx: Load bitmap "
                            "error: %s: %d!\n", file, nc);
                free (image->bmp);
                image->bmp = NULL;
                return (BITMAP*) -1;
            }

            image->ref_count ++; 
            _MY_PRINTF ("load bmp from incore: %s\n", file);
        }

        return image->bmp;
    }
    
    return NULL;     
}

static int release_bmp_from_incore (const char *file)
{
    PMP_RES_HASH_UNIT *image;
    
    image = pmp_res_hash_retrive_unit (file);
    if (image) {
        if (image->ref_count > 1) {
            image->ref_count--;
            _MY_PRINTF ("incore bmp ref_count--:%d\n", image->ref_count);
        }
        else {
            image->ref_count = 0;
            UnloadBitmap (image->bmp);
            free(image->bmp);
            image->bmp = NULL;
            _MY_PRINTF ("Unload bmp from incore: %s\n", file);
        }
        return 1;
    }
    
    return 0;    
}

static BITMAP *get_bmp_from_file (const char *file)
{
    BITMAP *bmp = NULL;
    char path[LEN_RES_TOP_DIR + 2 + LEN_RES_FILE_NAME + 2];

    if (!(bmp = (BITMAP *)malloc (sizeof(BITMAP))))
        return NULL;

    strcpy (path, res_top_dir);
    strcat (path, file);

    _MY_PRINTF ("load path = %s\n", path);
    
    if (LoadBitmap (HDC_SCREEN, bmp, path)) {
        free (bmp);
        _MY_PRINTF ("can not load %s\n", path);
        return NULL;
    }

    return bmp;
}

/******************res cache *******************************/

#define NR_NODES    24
#define MD5_LENTH   16
    
#define FIRST_NODE res_cache[0]
#define LAST_NODE res_cache[NR_NODES-1]    

typedef struct _PMP_RES_CACHE_NODE {
    unsigned char key_md5[MD5_LENTH];   /* hash value */
    BITMAP *bmp;             /* The pointer to BITMAP object*/
    int ref_count;
} PMP_RES_CACHE_NODE;

void pmp_get_md5 (const unsigned char *string, unsigned char digest[16]);

static PMP_RES_CACHE_NODE res_cache[NR_NODES];

/*Warn: memset (node_usage, 0xFF, sizeof (node_usage)) is a good substitute. */
static unsigned char node_usage [(NR_NODES + 7)/8] = {0xFF, 0xFF, 0xFF};

static int slot_is_free (unsigned char* bitmap, int index)
{
    bitmap += index >> 3;
    if (*bitmap & (0x80 >> (index % 8)))
        return 1;
    
    return 0;
}

int __mg_lookfor_unused_slot (unsigned char* bitmap, int len_bmp, int set);
int __mg_slot_clear_use (unsigned char* bitmap, int index);

/* Insert a node in the cache.  
 * key: the name of resouce file */
static void pmp_res_cache_insert (unsigned char md5_value[MD5_LENTH], BITMAP *bmp)
{
    int id = -1;    

    id =  __mg_lookfor_unused_slot (node_usage, sizeof (node_usage), 1);

    if (id >= 0) {
        memcpy(res_cache[id].key_md5, md5_value, MD5_LENTH);
        res_cache[id].bmp = bmp;
        res_cache[id].ref_count = 1;
    }
        
    return;
}

/* 
 * Caculate the key's md5 value and search in the cache. If found, return the 
 * pointer to the cache node; otherwise return NULL.
 */
static int pmp_res_cache_lookup (unsigned char md5_value[MD5_LENTH])
{
    PMP_RES_CACHE_NODE *p;
    
    for (p = &FIRST_NODE; p <= &LAST_NODE; ++p)
        if (!slot_is_free(node_usage, p-&FIRST_NODE) &&
            memcmp (md5_value, p->key_md5, MD5_LENTH) == 0) 
            return  (p-&FIRST_NODE);
    
    return -1;
}

/* Search in the cache and get a BITMAP object. If found, then return the pointer to 
 * BITMAP object; otherwise insert a node into the cache. If the cache is full, 
 *  load and return  a BITMAP object from filesystem directly.
 * key: the key of the cache.    
 */
static BITMAP *pmp_res_cache_req (const char *key)
{
    unsigned char md5_value[16];
    BITMAP *bmp = NULL;
    int id = -1;

    pmp_get_md5 ((const unsigned char*)key, md5_value);

    id = pmp_res_cache_lookup (md5_value);

    if (id >= 0) {
        res_cache[id].ref_count++;
        _MY_PRINTF ("ref_count++: ref_count = %d\n", res_cache[id].ref_count);

        return res_cache[id].bmp;
    }
    else {
        if ((bmp = get_bmp_from_file (key))!=NULL) {    
            pmp_res_cache_insert (md5_value, bmp);
            _MY_PRINTF ("insert a bitmap object to cache: %s\n ", key);
        }

        return bmp;
    }
    
}

/* Search the node according to the key, if ref_count>0, the ref_count--; 
 * if ref_count = 0, unload the bitmap object and free the node.     */
static int pmp_res_cache_release (const char *key)
{
    unsigned char md5_value[16];
    int id = -1;

    pmp_get_md5 ((const unsigned char*)key, md5_value);

    id = pmp_res_cache_lookup (md5_value);
    _MY_PRINTF ("release key=%s, id = %d\n", key, id);    
    if (id >= 0) {
        if (res_cache[id].ref_count > 1) {
            res_cache[id].ref_count--;
            _MY_PRINTF ("ref_count--: ref_count = %d\n", res_cache[id].ref_count);
        }
        else {
            UnloadBitmap (res_cache[id].bmp);
            free (res_cache[id].bmp);
            res_cache[id].bmp = NULL;
            __mg_slot_clear_use (node_usage, id);
            _MY_PRINTF ("Unload bmp object. id =%d. \n", id);
        }
        return 1;
    }
      
    return 0;
}


/* 
 * Loads a BITMAP object according to the image file name.
 * file: The name of resource file such as picture etc.
 * Return a BITMAP object, NULL indicates error.
 */
BITMAP *pmp_image_res_acquire_bitmap (const char *file)
{
    BITMAP *bmp = get_bmp_from_incore (file);

    if (bmp == NULL) {
        /* not found in incore resouce pool */
        bmp = pmp_res_cache_req (file);
    }
    else if (bmp == (BITMAP*) -1) {
        /* found in incore resouce pool but load failed */
        return NULL;
    }

    return bmp;
}

/* 
 * Releases the BITMAP object according to the image file name.
 * file: The name of resource file such as picture etc.
 * Return a BITMAP object, NULL indicates error.
 */
void pmp_image_res_release_bitmap (const char *file, BITMAP* bmp)
{
    if (!bmp)
        return;
    
    if (!release_bmp_from_incore (file) && !pmp_res_cache_release (file)) {
        _MY_PRINTF ("Not incore and cached. \n");
        UnloadBitmap (bmp);
        free (bmp);
    }
    
    return;
}

static MG_RWops *get_ds (const char *file)
{
    MG_RWops *rwops = NULL;
    const PMP_RES_HASH_UNIT *image;
    char path[LEN_RES_TOP_DIR + 2 + LEN_RES_FILE_NAME + 2];

    image = pmp_res_hash_retrive_unit (file);

    if (image) {
        rwops = MGUI_RWFromMem ((void *)image->data, image->data_size);
    }
    else {
        strcpy (path, res_top_dir);
        strcat (path, file);

        rwops = MGUI_RWFromFile (path, "rb");
    }

    return rwops;           
}

/* 
 * Acquires the data source object according to the image file name.
 * file: The name of resource file such as picture etc.
 * Return a MG_RWops object, NULL indicates error.
 */
MG_RWops* pmp_image_res_acquire_ds (const char *file)
{
    return get_ds (file);    

}

/* 
 * Releases the data source object according to the image file name.
 * file: The name of resource file such as picture etc.
 * No return .
 */
void pmp_image_res_release_ds (const char *file, MG_RWops* ds)
{
    if (ds) 
        MGUI_RWclose (ds);
}

/* 
 * A helper to return the suffix of a file name.
 * file: The name of resource file such as picture etc.
 * Return a BITMAP object, NULL indicates error.
 */
const char* pmp_get_file_suffix (const char *file)
{
    const char* ext;

    if (file == NULL)
        return NULL;

    ext = strrchr (file, '.');
    
    if (ext)
        return ext + 1;
    
    return NULL;
}

int pmp_set_res_top_dir (const char *top_dir)
{
    if (strlen (top_dir) > LEN_RES_TOP_DIR)
        return 1;

    strcpy (res_top_dir, top_dir);
    if (strrchr (top_dir, '/') == NULL) {
        strcat (res_top_dir, "/");
    }

    return 0;
}


