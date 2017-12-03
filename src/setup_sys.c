/*
** $Id$
**
** setup_sys.c: setup funtions depend on OS for Feynman PMP solution.
**
** Copyright (C) 2007 Feynman Software, all rights reserved.
**
** Use of this source package is subject to specific license terms
** from Beijing Feynman Software Technology Co., Ltd.
**
** URL: http://www.minigui.com
**
** Current maintainer: WangXuguang.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/vfs.h>

#include "pmp.h"

#define MAX_STR_LENGTH 200
#define DEBUG
/*return head of part str,  part_len length of part*/

#if 0
static char* find_str_part(char* src, int* part_len)
{
	char* cp = NULL;
	int i = 0; //
	int len = 0;

	//cp++ to find the first unspace char.
	cp = src;
	len = strlen(cp);
#ifdef DEBUG
		printf("find_str_part{{{\n");
#endif
	for (i=0; isspace(cp[i])&&i<MAX_STR_LENGTH; i++)
	{
		if (cp[i] == '\0')
		{
			*part_len = 0;
			return NULL;
		}

	}

	if (i==MAX_STR_LENGTH)
	{
		return NULL;
	}

#ifdef DEBUG
	printf("	part-start = <%d>\n", i);
#endif
	//continue to find first space char
	cp = src + i;
	for (i=0; !isspace(cp[i]) && i<MAX_STR_LENGTH; i++)
	{
		if (cp[i] == '\0')
		{
			*part_len = i;

#ifdef DEBUG
			{
				char tmp[50];
				bzero(tmp, 50);
				strncpy(tmp, cp, i);
				printf("	part out= <%d>\n", i);
				printf("	str_part = <%s>\n", tmp);
				printf("}}}find_str_part\n");

			}
#endif
			return cp;
		}

	}
	if (i == MAX_STR_LENGTH)
	{
		return NULL;
	}
	else
	{
		*part_len = i;
#ifdef DEBUG
			{
				char tmp[50];
				bzero(tmp, 50);
				strncpy(tmp, cp, i);
				printf("	part out= <%d>\n", i);
				printf("	str_part = <%s>\n", tmp);
				printf("}}}find_str_part\n");

			}
#endif
		return cp;
	}
}
#endif


//used space, free space of disk
void get_disk_info(long* space, long* free_space)
{
#define FLASH_MOUNT_DIR "/"
    struct statfs stat_fs;
    statfs(FLASH_MOUNT_DIR, &stat_fs);
    *space = (stat_fs.f_blocks * stat_fs.f_bsize) >> 20;
    *free_space = (stat_fs.f_bfree * stat_fs.f_bsize) >> 20;
}

/*get hours minute, day mouth year*/
struct tm* get_time(void)
{
    time_t t;
    struct tm* tm;

    time (&t);
    tm = localtime (&t);

#ifdef DEBUG
	printf("get_time{{{%04d-%02d-%02d, %02d:%02d:%02d}}}\n", 
			(tm->tm_year+1900), tm->tm_mon, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec);
#endif

    return tm;
}

/*set time to clock*/
void write_time(struct tm* tm)
{
    tm->tm_year -= 1900;
    
    time_t t = mktime(tm);
    
    if (stime(&t) == -1)
        perror("write_time--");
}
