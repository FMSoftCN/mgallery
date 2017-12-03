#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <minigui/mgext.h>
#include <minigui/mywindows.h>
#include <minigui/filedlg.h>

#include "debug.h"

#define MAX_MCHAR_LEN 6

static int fd = -1;
static int fd_tag = -1;

static int* blk_stas = NULL;		//start of blocks
static int blk_num = 0;			//block num

static int blk_cur = -1;	// block displayed 
static int line_cur = 0;	//size of file

static char buf[1024];		//buffer where put file content
static int mchar_pos[1024];	//buffer where put mchar pos 
#define BUF_SIZE 1015	 

static int blk_tag;
static int line_tag;

static int file_size;


static int alloc_blk_stas(void)
{
	struct stat file_stat;

	FUN_START
	fstat(fd, &file_stat);

	file_size = file_stat.st_size;

	blk_num = (file_stat.st_size + BUF_SIZE - 1)  / BUF_SIZE;
	/*gather fragment of Mchar on tail of block*/
	blk_num += blk_num * MAX_MCHAR_LEN/BUF_SIZE + 1;
	
	blk_stas = (int*)malloc(blk_num*sizeof(int));
	blk_cur = -1;

	if (blk_stas==NULL)
	{
		blk_num = 0;
		return -1;
	}

	FUN_END

	return 0;
}

static int parse_file_to_blk(LOGFONT* font)
{
	int blk_par = 0;
	int rd_num = 0;
	int mchar_num_line;

	FUN_START

	blk_stas[0] = 0;

	lseek(fd, 0, SEEK_SET);
	
	while((rd_num = read(fd, buf, BUF_SIZE)) > 0)
	{
		buf[rd_num] = '\0';

		if (rd_num != strlen(buf))
		{
			return -1; 
		}
	
		if (rd_num == BUF_SIZE)
		{
			mchar_num_line = GetTextMCharInfo(font, buf, rd_num, mchar_pos);
		}
		else
		{
			//TEST_VAL(buf, %s);
			blk_stas[blk_par+1] = file_size;
			blk_num = blk_par+1;
			FUN_END
			return 0;
		}
		
		/*next block start = cur block start + cur block length*/
		buf[mchar_pos[mchar_num_line-1]] = '\0';
//		TEST_VAL(buf, %s);
//		TEST_VAL(strlen(buf), %d);

		blk_stas[blk_par+1] = blk_stas[blk_par] + mchar_pos[mchar_num_line-1]; 
//		TEST_VAL(blk_stas[blk_par+1], %d);
		lseek(fd, blk_stas[blk_par+1], SEEK_SET);
		blk_par++;

//		TEST_INFO("\n\n\n");
	}

	return -1;
}

static int load_tag(char* file_name)
{
	char tag_name[MAX_PATH];
	char* slash_p;

	FUN_START

	/*make name of tag file */
	strcpy(tag_name, file_name);
	slash_p = rindex(tag_name, '/');
	memmove(slash_p+1, slash_p, strlen(slash_p)+1);
	*(slash_p+1) = '.';

	strcat(tag_name, ".tmp");
    TEST_VAL(tag_name, %s);

	/*open tag file*/
	fd_tag = open(tag_name, O_RDWR);

	if (fd_tag == -1)
	{
		fd_tag = open(tag_name, O_CREAT|O_RDWR, S_IRWXU);
		TEST_INFO("creat tag file");
		TEST_VAL(tag_name, %s);
		return -1;
	}
	
	lseek(fd_tag, 0, SEEK_SET);
	read(fd_tag, &blk_tag, sizeof(blk_tag));
	read(fd_tag, &line_tag, sizeof(line_tag));
    TEST_VAL(blk_tag, %d);
    TEST_VAL(line_tag, %d);

	FUN_END
	
	return 0;
}

static void save_tag(void)
{
	FUN_START
	lseek(fd_tag, 0, SEEK_SET);
	TEST_VAL(blk_tag, %d);
	TEST_VAL(line_tag, %d);
	write(fd_tag, &blk_tag, sizeof(blk_tag));
	write(fd_tag, &line_tag, sizeof(line_tag));
	fsync(fd_tag);
	close(fd_tag);
	fd_tag = -1;
	FUN_END
}


int pmp_ebook_start_read(char* file_name, LOGFONT* font, int *line)
{
	FUN_START

	fd = open(file_name, O_RDONLY);
	if (fd == -1)
	{
		return -1;
	}
	
	/*alloc blk_stas*/
	if (alloc_blk_stas() == -1)
	{
		return -1;
	}

	if (parse_file_to_blk(font) == -1)
	{
		free(blk_stas);
		return -1;
	}
	
	if (load_tag(file_name) == 0)
	{
		TEST_INFO("have tag-------");
		TEST_VAL(blk_tag, %d);
		TEST_VAL(line_tag, %d);
	
		blk_cur = blk_tag;
		*line = line_tag;
	}
	else
	{
		blk_cur = 0;
		*line = 0;
	}

	FUN_END

	return 0;
}

static int my_read(int fd, char* buf, int leng)
{
	int rd_num = 0;
	int wr_num = 0;
	char ch;
	for(rd_num=0; rd_num<leng; rd_num++)
	{
		read(fd, &ch, 1);
		if (ch == '\r')
			continue;
		buf[wr_num] = ch;
		wr_num++;
	}

	return wr_num;
}

char* pmp_ebook_cur_read(void)
{
//	TEST_VAL(blk_cur, %d);
//	TEST_VAL(blk_num, %d);
	int num = 0;
	lseek(fd, blk_stas[blk_cur], SEEK_SET);
	num = my_read(fd, buf, blk_stas[blk_cur+1] - blk_stas[blk_cur]);
	buf[num] = '\0';
	TEST_VAL(buf, %s);
	strcat(buf+num-2, "\n\n");
	return buf;
}

char* pmp_ebook_next_read(void)
{
	blk_cur = (blk_cur + 1) % blk_num;
	return pmp_ebook_cur_read();
}

char* pmp_ebook_pre_read(void)
{
	blk_cur = (blk_cur - 1 + blk_num) % blk_num;
	return pmp_ebook_cur_read();
}

void pmp_ebook_end_read(void)
{
	save_tag();
	if (fd != -1)
	{
		close(fd);
		fd = -1;
	}
	if (fd_tag != -1) 
	{
		close(fd_tag);
		fd_tag = -1;
	}
	
	free(blk_stas);
	blk_stas = NULL;
	blk_num = 0;

	blk_cur = -1;	 
	line_cur = -1;
}

void pmp_ebook_tag(int line)
{
	blk_tag = blk_cur;
	line_tag = line;
	
	TEST_VAL(blk_tag, %d);
	TEST_VAL(line_tag, %d);
}
