/*
** $Id$
**
** setup_proc.c: implement setup funtion for Feynman PMP solution.
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
#include <time.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <pmp.h>
#include <text_id.h>

extern struct msg_domain* pmp_domains[];

#define LANG_ZH_CN 0
#define LANG_EN_US 1

extern void get_disk_info(long* used_space, long* free_space);


static int lang=1;


struct tm tm;

void add_minute(void)
{
	tm.tm_min = (tm.tm_min + 1 + 60) % 60;

}

void sub_minute(void)
{
	tm.tm_min = (tm.tm_min - 1 + 60) % 60;

#ifdef DEBUG
	printf("add_tm.tm_min--tm.tm_min is %d\n", tm.tm_min);
#endif
}

int get_minute(void)
{
	return tm.tm_min;
}

void add_hour(void)
{
	tm.tm_hour = (tm.tm_hour + 1) % 24;
#ifdef DEBUG
	printf("add_tm.tm_hour--tm.tm_hour is %d\n", volume);
#endif

}
void sub_hour(void)
{
	tm.tm_hour  = (tm.tm_hour - 1 + 24) % 24;
#ifdef DEBUG
	printf("sub_tm.tm_hour--tm.tm_hour is %d\n", volume);
#endif
}

int get_hour(void)
{
	return tm.tm_hour;
}
///////////////////////////////////////////////////////////

						/* 0  1   2   3   4   5   6   7   8   9   10  11  12*/
static int days_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void add_year(void)
{
	/*add tm.tm_year*/
	if (tm.tm_year < 2030)
	{
		tm.tm_year++;
	}
	else
	{
		tm.tm_year = 2000;
	}

	/*adjust tm.tm_mday number in Feb*/
	if (tm.tm_year % 4 == 0)
	{
		days_month[2] = 29;
	}
	else
	{
		days_month[2] = 28;
	}
	
	/*abjust tm.tm_mday if this tm.tm_mon is Feb*/
	if (tm.tm_mon == 2 && tm.tm_mday > days_month[2])
	{
		tm.tm_mday = days_month[2];
	}
}

void sub_year(void)
{
	if (tm.tm_year > 2000)
	{
		tm.tm_year--;
	}
	else
	{
		tm.tm_year = 2030;
	}

	/*adjust tm.tm_mday number in Feb*/
	if (tm.tm_year % 4 == 0)
	{
		days_month[2] = 29;
	}
	else
	{
		days_month[2] = 28;
	}
	
	/*abjust tm.tm_mday if this tm.tm_mon is Feb*/
	if (tm.tm_mon == 2 && tm.tm_mday > days_month[2])
	{
		tm.tm_mday = days_month[2];
	}
}

int get_year(void)
{
	return tm.tm_year;
}
///////////////////////////////////////////////////////////////
void add_month(void)
{
	tm.tm_mon = tm.tm_mon % 12 + 1;

	/*abjust tm.tm_mday*/
	if (tm.tm_mday > days_month[tm.tm_mon])
	{
		tm.tm_mday = days_month[tm.tm_mon];
	}
}

void sub_month(void)
{
	tm.tm_mon = (tm.tm_mon + 12 - 2) % 12 + 1;

	/*abjust tm.tm_mday*/
	if (tm.tm_mday > days_month[tm.tm_mon])
	{
		tm.tm_mday = days_month[tm.tm_mon];
	}
}

int get_month(void)
{
	return tm.tm_mon;
}
/////////////////////////////////////////////////////////////////
void add_day(void)
{
	if (tm.tm_mday < days_month[tm.tm_mon])
	{
		tm.tm_mday++;
	}
	else
	{
		tm.tm_mday = 1;
	}
}

void sub_day(void)
{
	if (tm.tm_mday > 1)
	{
		tm.tm_mday--;
	}
	else
	{
		tm.tm_mday = days_month[tm.tm_mon];
	}
}

int get_day(void)
{
	return tm.tm_mday;
}

///////////////////////////////////////////////

static int is_key_sound = 0;

void add_key_sound(void)
{
	is_key_sound = !(is_key_sound);
}

void sub_key_sound(void)
{
	is_key_sound = !(is_key_sound);
}

int get_key_sound(void)
{
	return is_key_sound;
}

void out_key_sound(char* buffer)
{
	if (is_key_sound == 0)
	{
		strcpy(buffer, _(IDS_PMP_SETUP_ON));
	}
	else
	{
		strcpy(buffer, _(IDS_PMP_SETUP_OFF));
	}
}
////////////////////////////////////////////

static int bklight = 3;

void add_bklight(void)
{
	if (bklight < 5)
	{
		bklight++;
	}
}

void sub_bklight(void)
{
	if (bklight > 1)
	{
		bklight--;
	}
}
int  get_bklight(void)
{
	return bklight;
}

void out_bklight(char* buffer)
{
	sprintf(buffer, _(IDS_PMP_SETUP_MIN)/*"%dmin"*/, bklight);
}
/////////////////////////////////////////////////
static int shutdown_min = 5;
void add_shutdown_time(void)
{
	if (shutdown_min < 20)
	{
		shutdown_min++;
	}
}

void sub_shutdown_time(void)
{
	if (shutdown_min > 1)
	{
		shutdown_min--;
	}
}

int  get_shutdown_time(void)
{
	return shutdown_min;
}

void out_shutdown_time(char* buffer)
{
	sprintf(buffer, _(IDS_PMP_SETUP_MIN)/*"%d"*/, shutdown_min);
}
////////////////////////////////////////////////////
static char* lang_str[] = {"¼òÌåÖÐÎÄ","English"};

void add_language(void)
{
	lang = (lang + 1) % 2;
    /*
    if (lang == LANG_ZH_CN)
        pmp_select_text_domain("zh_cn");
    else
        pmp_select_text_domain("en");
        */
}

void sub_language(void)
{
	lang = (lang -1 + 2) % 2;
    
    /*
    if (lang == LANG_ZH_CN)
        pmp_select_text_domain("zh_cn");
    else
        pmp_select_text_domain("en");
        */
}

int get_language(void)
{
	return lang;
}

void out_language(char* buffer)
{
	switch (lang)
	{
		case LANG_ZH_CN:
			strcpy(buffer, lang_str[LANG_ZH_CN]);
			break;
		case LANG_EN_US:
			strcpy(buffer, lang_str[LANG_EN_US]);
			break;
		default:
			break;
	}
}


void set_bkpic(char* path)
{
#ifdef DEBUG
	printf("back ground pictrue is seted");
#endif
}
////////////////////////////////////////////
int background_index;

void add_bkcolor(void)
{	
	background_index = (background_index + 1) % 4;
}

void sub_bkcolor(void)
{
	background_index = (background_index - 1 + 4) % 4;
}

int get_bkcolor(void)
{
	return background_index;
}

void out_bkcolor(char* buffer)
{
	switch(background_index)
	{
		case 0:
			strcpy(buffer, ("pic 1"));
			break;
		case 1:
			strcpy(buffer, ("pic 2"));
			break;
		case 2:
			strcpy(buffer, ("pic 3"));
			break;
        case 3:
            strcpy(buffer, ("pic 4"));
            break;
		default:
			break;
	}
}

static int get_mem_size(void)
{
    int mem = 0;
    FILE *file = fopen("/proc/meminfo", "r");
    if (file)
    {
        char line[256];
        fgets(line, sizeof(line), file);
        if (strncmp(line, "MemTotal:", sizeof("MemTotal:")-1)==0)
        {
            char *p = line + sizeof("MemTotal:") -1;
            while (*p==' ')
                p++;
            mem = atoi(p);
        }
        fclose(file);
    }
    return mem;
}

//////////////////////////////////////////////////
static char* sys_info = "STAR--PMP\n"
					    "core store: %dK\n"
					    "disk space: %ldM\n"
						"free space: %ldM\n"
					    "firmware:20080808\n";
							 
void out_sysinfo(char* buffer, int buf_size)
{
	long space;
	long free_space;
	int mem;

	mem = get_mem_size();
	get_disk_info(&space, &free_space);
	snprintf(buffer, buf_size, sys_info/*_(IDS_PMP_SETUP_PMPINFO)*/, mem, 
			space, free_space);
}	
