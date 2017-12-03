#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>

#include"pmp.h"


extern PMP_MEDIA_FILE_LIST*
pmp_select_media_files_list(HWND parent,
        const char *root_dir,
        const char* initial_dir,
        PMP_CB_MEDIA_FILTER cb_media_filter,
        PMP_CB_MEDIA_ICON cb_media_icon,
        PMP_DIR_SHOW_INFO* dir_show_info,
        DWORD mode
        );

extern PMP_MEDIA_FILE_LIST*
pmp_select_media_files_thumbscrew(HWND parent,
        const char *root_dir,
        const char* initial_dir,
        PMP_CB_MEDIA_FILTER cb_media_filter,
        PMP_CB_MEDIA_ICON cb_media_icon,
        PMP_DIR_SHOW_INFO* dir_show_info,
        DWORD mode
        );




PMP_MEDIA_FILE_LIST*
pmp_select_media_files (HWND parent,
        const char *root_dir,
        const char* initial_dir, 
        PMP_CB_MEDIA_FILTER cb_media_filter,
        PMP_CB_MEDIA_ICON cb_media_icon, 
        PMP_DIR_SHOW_INFO* dir_show_info,
        DWORD mode
        )
{
    if((PMP_BROWSER_MODE_THUMBNAIL & mode)==PMP_BROWSER_MODE_THUMBNAIL)
        return pmp_select_media_files_thumbscrew(parent,
                root_dir,
                initial_dir, 
                cb_media_filter,
                cb_media_icon, 
                dir_show_info,
                mode
                );

    else 
        if((PMP_BROWSER_MODE_LIST & mode)==PMP_BROWSER_MODE_LIST)
            return  pmp_select_media_files_list(parent,
                    root_dir,
                    initial_dir, 
                    cb_media_filter,
                    cb_media_icon, 
                    dir_show_info,
                    mode
                    );
        else
            _MY_PRINTF("Invalid parameter for mode! ");
    return NULL; 

}
