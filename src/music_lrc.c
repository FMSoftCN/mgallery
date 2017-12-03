/*
** $Id$
**
** music_lrc.c: reading lrc file in music for Feynman PMP solution.
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
#include <ctype.h>
#include <string.h>
#include <assert.h>

//#define DEBUG
#include "debug.h"
#define MAX_LINE_LEN 256

typedef struct Lrc_Time_Tag Lrc_Time_Tag;
struct Lrc_Time_Tag
{
	int time;
	int lrc_pos;
};

static char cur_line[MAX_LINE_LEN];

/**
 * _music_getline_file: 
 *		read a line char from file "fp",
 *		put the chars to "buf" 
 *return: 
 *		the number of character in current line
 *			(include '\n')
 *		EOF: to end of file
 */ 
int _music_getline_file (FILE*fp, char* buf)
{
#if 0
	int pos = -1;

	/*read to '\n' or end of file*/
	do
	{
		pos++;
		buf[pos] = fgetc(fp);
	}while (buf[pos] != '\n' && buf[pos] != EOF);

	
	/*only end of file*/
	if (buf[0] == EOF)
	{
		buf[0] = '\0';
		return EOF;
	}
	
	/*to '\n'*/
	if (buf[pos] == '\n')
	{
		buf[pos+1] = '\0';
		return pos+1;
	}

	/*readed chars but to end*/
	if (buf[pos] == EOF)
	{
		buf[pos] = '\0';
		return pos;
	}

    assert(0);
    return -1;
#else
    if (fgets (buf, MAX_LINE_LEN - 1, fp)) {
        int len = strlen (buf);
        if (buf [len - 1] != '\n') {
            buf [len - 1] = '\n';
            buf [len] = '\0';
            len ++;
        }
        return len;
    }

    return 0;
#endif
}

/**
 *lrc_tt_compar: 
 *		compare two Lrc_Time_Tag.
 *return: 
 *		1 -- tt1->time > tt2->time, 
 *		0 -- tt1->time = tt2->time, 
 *		-1 -- tt1->time < tt2->time, 
 */ 
static int lrc_tt_compar(const void* tt1, const void* tt2)
{
	Lrc_Time_Tag* tt_1 = (Lrc_Time_Tag*)tt1;
	Lrc_Time_Tag* tt_2 = (Lrc_Time_Tag*)tt2;

	assert(tt1);
	assert(tt2);

	if (tt_1->time > tt_2->time)
	{
		return 1;
	}
	else if (tt_1->time < tt_2->time )
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

/**
 *skip_over_lrc_head: 
 *		skip over lrc head(not lyric) in file "fp"
 *return: 
 *		the first lrc line in file 
 */ 
static int skip_over_lrc_head(FILE* fp)
{
	int line_pos_in_file = 0;
	int line_len = 0;

	FUN_START
	assert(fp);

	fseek(fp, 0, SEEK_SET);
	
	do{
		/*get line header*/
		line_pos_in_file = ftell(fp);
		line_len = _music_getline_file(fp, cur_line);
		
		/*skip over the file*/
		if (line_len <= 0)
		{
			return EOF;
		}
		/*current line have no tag*/
		else if (cur_line[0] != '[')
		{
			continue;
		}
		/*meet time tag*/
		else if (isdigit(cur_line[1]))
		{
			break;
		}
	}while(1);


	TEST_VAL(cur_line, %s);
	FUN_END
	return line_pos_in_file;
}

/**
 *parse_lrc_line: 
 *		parse one lrc line "cur_line",
 *		put Lrc_Time_Tags to buffer "lrc_tts",
 *		put lyric positon to lrc_pos.
 * return:
 *		the number of Lrc_Time_Tags in the lrc line.
 */ 
static int parse_lrc_line(Lrc_Time_Tag* lrc_tts, char* cur_line, int* lrc_pos)
{

	char* tag_start_cp = cur_line;
	char* tag_end_cp = NULL;
	int	tag_len = 0;

	int min;
	int sec;

	int cur_tag = 0;

	char tag_str[12];
	
	if (cur_line[0] != '[')
	{
		*lrc_pos = 0;
		return 0;
	}

	while ((*tag_start_cp == '[') 
			&& (tag_end_cp = memchr(tag_start_cp, ']', MAX_LINE_LEN)))
	{
		int scan_num;
		bzero(tag_str, 12);
		tag_len = tag_end_cp - tag_start_cp + 1;
		strncpy(tag_str, tag_start_cp, tag_len);

		scan_num = sscanf(tag_str, "[%d:%d", &min, &sec);
		/*if not a valid tag*/
		if (scan_num < 2)
		{
			break;
		}

		TEST_VAL(min, %d);
		TEST_VAL(sec, %d);
		
		lrc_tts[cur_tag].time = min * 60 + sec;
		TEST_VAL(lrc_tts[cur_tag].time, %d);
		
		cur_tag++;
		tag_start_cp = tag_end_cp + 1;
	}

	/*index of lyric first character*/
	*lrc_pos = tag_start_cp - cur_line;
	TEST_VAL(*lrc_pos, %d);
	TEST_VAL(cur_tag, %d);

	TEST_INFO("\n");
	return cur_tag;

}


/**
 *_music_parse_lrc_line: 
 *		parse a line in lrc file "fp",
 *		put time tag in the file to "lrc_tts".
 *return: 
 *		the number of time tag in the file fp
 */ 
int _music_parse_lrc_file(FILE* fp, Lrc_Time_Tag* lrc_tts)
{
	int line_pos_in_file = 0;		//rw pointer of fp
	int tt_num_file = 0;	//number of lrc_time_tag in file fp
	int tt_num_line = 0;	//number of lrc_time_tag in cur line
	int lrc_pos_line = 0;	//lyric pos in current line
	int read_num = 0;

	assert(fp);
	assert(lrc_tts);

	//skip over the head of lrc file
	line_pos_in_file = skip_over_lrc_head(fp);
	/*skiped over the file*/
	if (line_pos_in_file == EOF)
	{
		return -1;
	}

	fseek(fp, line_pos_in_file, SEEK_SET);

//	bzero(cur_line, MAX_LINE_LEN);
	while ((read_num = _music_getline_file(fp, cur_line)) > 0)	
	{
		int i;

		/*only '\n'*/
		if (read_num == 1)
		{
			continue;
		}
		
		//TEST_VAL(cur_line, %s);
		tt_num_line = parse_lrc_line(lrc_tts+tt_num_file, cur_line, &lrc_pos_line);

		tt_num_file += tt_num_line;

		//set lrc_pos of new tags to rw_pos of cur line header
		for (i=1; i<=tt_num_line; i++)
		{
			lrc_tts[tt_num_file-i].lrc_pos = line_pos_in_file + lrc_pos_line;
		}

		//set rw_pos to header of next line
		line_pos_in_file = ftell(fp);
	
//		bzero(cur_line, MAX_LINE_LEN);
	}

	qsort(lrc_tts, tt_num_file, sizeof(*lrc_tts), lrc_tt_compar);	
	return tt_num_file;
}
