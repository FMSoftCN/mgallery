/*
** $Id$
**
** music_play.c:
**
** Copyright (C) 2006 Feynman Software.
**
** All right reserved by Feynman Software.
**
** Current maintainer: Wang Xuguang.
**
** Create date: 2007/05/14
*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pmp.h>
#include <text_id.h>

#include "decoder.h"
#include "pmp_music.h"
#include "music_menu.h"

//#define DEBUG
#include "debug.h"

#define GET_LINE(fp ,cur_line) \
	cur_line[_music_getline_file(fp, cur_line)] = '\0'

#define IDC_AB		110
#define IDC_LOOP	101
#define IDC_EQ		102
#define IDC_PLAY_OP	103
#define IDC_STRIP	104
#define IDC_SLIDE	105

#define MY_PATH_MAX 512

typedef struct Lrc_Time_Tag Lrc_Time_Tag;
struct Lrc_Time_Tag
{
	int time;
	int lrc_pos;
};
Lrc_Time_Tag lrc_tts[256];

extern int _music_getline_file(FILE* fp, char* cur_line);
extern int _music_parse_lrc_file(FILE* fp, Lrc_Time_Tag* lrc_tts);

static void	display_lrc(HDC hdc);
static FILE* fopen_lrc_from_music(char* music_name);

static void twinkle_hwnd(HWND hwnd);

static int brow_sec = 0;				//seconds used in browse play
void* music_audio_player;				//handle of DSP
static int dsp_status;					//status (PLAYING, FFING, FBING-- fade),

static BOOL refresh_time = TRUE;
static HDC mem_dc_save;
static HDC mem_dc_put;
static HDC mem_dc_save_lrc;
static HDC mem_dc_put_lrc;

static PMP_MEDIA_FILE_LIST* file_list;
static BOOL file_is_selected = FALSE;

static play_status_t play_status = 
{
	20, //volume
	LIST_PLAY,	//play mode
	IN_ORDER,	//loop mode

	AB_PLAY,	//operator
	0,			//cur speed			
	0,			//no fade in and out

	1,			//ab_wait_time
	8,			//ab_count

	NORMAL,		//eq_mode
	255,		// 3 param of user eq_mode
	255,
	255
};
/*info of playing window*/
static MAINWINCREATE create_info;	

//fullname used in setting file
static char fullname[MY_PATH_MAX];
static char lrc_name[MY_PATH_MAX];
static FILE* lrc_file;
static char lrc_str[200];
static int tt_num_file;
static int cur_tt;
static int ab_used_flag = 0;
static BOOL soft_paused = FALSE;
static int last_file;				//first file index in play_type "all"

static tag_info_t tag_info;
//path and name used in tag play;
static char path_tag[MY_PATH_MAX];
static char name_tag[NAME_MAX];
static tag_info_t tag_tag_info;

//path and name used in bpt_restore_play
static char path_exit[MY_PATH_MAX];
static char name_exit[NAME_MAX];
static tag_info_t exit_tag_info;

//path and name used in ab_restore_play
static char path_ab[MY_PATH_MAX];
static char name_ab[NAME_MAX];
static tag_info_t ab_tag_info;

static music_file_info_t music_file_info;
static music_status_t music_status;

static LOGFONT* lrc_font;

typedef struct tagBMPINFO
{
    char* file_name;
    BITMAP* bmp;

}BMPINFO;

static BMPINFO bmpinfo_play_op[2] = 
{
    {"/res/music/playing.png", NULL},
    {"/res/music/paused.png", NULL}
};

static BMPINFO bmpinfo_loop[3] = 
{
    {"/res/music/loop_one.png",NULL}, 
    {"/res/music/random.png", NULL},
    {"/res/music/inorder.png", NULL},
};

static BMPINFO bmpinfo_eq[11] = 
{
    {"/res/music/normal.png", NULL},
    {"/res/music/rock.png", NULL}, 
    {"/res/music/pop.png", NULL},

    {"/res/music/clas.png", NULL},
    {"/res/music/soft.png", NULL},
    {"/res/music/jazz.png", NULL},

    {"/res/music/dbb.png", NULL},
    {"/res/music/srs.png", NULL},
    {"/res/music/wow.png", NULL},

    {"/res/music/bass.png", NULL},
    {"/res/music/user.png", NULL},
};

static BMPINFO bmpinfo_ab[5]	= 
{
    {"/res/music/ab-void.png", NULL},
    {"/res/music/ab-a.png", NULL},
    {"/res/music/ab-a-.png", NULL},
    {"/res/music/a-b.png", NULL},
    {"/res/music/ab-replay.png", NULL},
};

static BMPINFO bmpinfo_trackbar[2] = 
{
    {"/res/music/strip_1.png", NULL},
    {"/res/music/slide_1.png", NULL}
};

static BMPINFO bmpinfo_back[1] = 
{
    {"/res/music/music.png", NULL}
};

//******************************************************
/*proc in playing window*/
static int play_proc(HWND hwnd, int message, WPARAM wparam, LPARAM lparam);

/*in normal playing(not AB), key_down, key_up, key_always_pressed*/
static void deal_key_normal(HWND hwnd, int message, WPARAM wparam, void* music_audio_player);

/*in ab playing key_down ,key_up*/
static int deal_key_ab(HWND hwnd, int message, WPARAM wparam, void* music_audio_player);

/*in normal play, key_down*/
static void norm_key_down(HWND hwnd, WPARAM wparam, void* music_audio_player);

/*in normal play, key_alwayspress*/
//static void norm_key_alwayspress(HWND hwnd, WPARAM wparam, void* music_audio_player);

/*in normal play, key_up*/
static void norm_key_up(HWND hwnd, WPARAM wparam, void* music_audio_player);

/*select file based loop_mode and is_dir_play*/
static int select_file(PMP_MEDIA_FILE_LIST* file_list, int loop_mode, int select_mod, int cur, char* fullname);

static PMP_MEDIA_FILE_LIST* _music_show_files(HWND hwnd);

static void display_time(HDC hdc);

/*load tag info from files*/
static BOOL load_exit_info(void);
static BOOL load_tag_info(void);
static BOOL load_ab_info(void);

/*save tag info from file */
static void save_exit_info(void);
static void save_ab_info(void);

/*load unload bmp resource*/
static inline void unload_bitmap (BMPINFO* bmp_info);
static inline void load_bitmap (BMPINFO* bmp_info);
static void unload_bitmap_array(BMPINFO* bmp_info, int num);

/*change bmp on static_box to displaying status*/
static void change_bmp_on_control(HWND hwnd, BMPINFO* bmp_info);

/*set position of slide in strip*/
static void set_slide(HWND h_strip, int pos);

/*creat list of music*/
//extern void pmp_music_list(HWND hwnd);

/*display the infomation of file being played*/
static void	display_music_info(HDC hdc, music_file_info_t* music_file_info);

/*refresh the rect displaying the file infomation*/
static void invalidate_music_info(HWND hwnd);
//*************************************************************

static void _music_show_menu(HWND parent, const char* title);


static void refresh_lrc(HWND h_play, int cur_time);

void compose_path_name(char* fullname, const char* path, const char* name);

void divide_fullname(const char* fullname, char* path, char* name);
	
/*initial creat_info of playing window*/
static void init_creatinfo(MAINWINCREATE* pCreateInfo, HWND h_host)
{
    pCreateInfo->dwStyle = WS_VISIBLE;
    pCreateInfo->dwExStyle = WS_EX_NONE;
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor(0);
	pCreateInfo->spCaption = "";
    pCreateInfo->hIcon = 0;
    pCreateInfo->MainWindowProc = play_proc;
    pCreateInfo->lx = 0;
    pCreateInfo->ty = 20;
    pCreateInfo->rx = 320;
    pCreateInfo->by = 240;
    pCreateInfo->iBkColor = COLOR_lightgray;
    pCreateInfo->dwAddData = 0;
    pCreateInfo->hHosting = h_host;

}

static int cur;			//file index
static HWND h_host;		//hosting window(window having file_scrollview)
//	static int down_num;
static int norm_ab;	//normal playing or AB playing (key diffrence)
	
static play_param_t play_param;	
	
static BOOL have_exit_info;		//there is tag_info of break point playing or not
static BOOL have_tag_info;		//           tag_info of tag palying
static BOOL have_ab_info;		//           tag_info of ab_play 

static HWND h_play;
static HWND h_loop;				//static box display loop_mod
static HWND h_play_op;			//static box display PLAY or PAUSE
static HWND h_eq;				//static box display eq_mode
static HWND h_ab;				//static box display AB status
static HWND h_strip;			//static box display strip

static int slide_pos = SLIDE_START;	//position of slide on strip
static int cur_time;				//cur time of music being played
static int total_time;				//total time of music being played

static void create_control(void)
{
	/*creat static boxes*/
	h_ab = CreateWindowEx("static", "", 
			WS_CHILD|WS_VISIBLE|SS_BITMAP, WS_EX_TRANSPARENT,
			IDC_AB,
			100, 20, 40, 20,
			h_play, 0); 

	h_loop = CreateWindowEx("static", "", 
			WS_CHILD|WS_VISIBLE|SS_BITMAP, WS_EX_TRANSPARENT,
			IDC_LOOP,
			160, 20, 40, 20,
			h_play, (UINT)0);
    change_bmp_on_control(h_loop, bmpinfo_loop+play_status.loop_type);

	h_eq = CreateWindowEx("static", "", 
			WS_CHILD|WS_VISIBLE|SS_BITMAP, WS_EX_TRANSPARENT,
			IDC_EQ,
			220, 20, 40, 20,
			h_play, (UINT)0);
    change_bmp_on_control(h_eq, bmpinfo_eq + play_status.eq_mode);

	h_play_op = CreateWindowEx("static", "", 
			WS_CHILD|WS_VISIBLE|SS_BITMAP, WS_EX_TRANSPARENT,
			IDC_PLAY_OP,
			280, 20, 20, 20,
			h_play, (UINT)0);
    change_bmp_on_control(h_play_op, bmpinfo_play_op);

	h_strip = CreateWindowEx("static", "", 
			WS_CHILD|WS_VISIBLE|SS_BITMAP|SS_REALSIZEIMAGE, 
			WS_EX_TRANSPARENT,
			IDC_STRIP,
			STRIP_L, STRIP_T, STRIP_W, STRIP_H,
			h_play, (UINT)0);
    change_bmp_on_control(h_strip, bmpinfo_trackbar);
    bmpinfo_trackbar[1].bmp = pmp_image_res_acquire_bitmap(bmpinfo_trackbar[1].file_name);

	/*put slide on strip*/
	set_slide(h_strip, SLIDE_START);
	slide_pos = SLIDE_START;
}

static void play_specially(play_mode_t play_mode)
{
	if (lrc_file != NULL)
	{
		fclose(lrc_file);
		lrc_file = NULL;
	}

	lrc_file = fopen_lrc_from_music(fullname);
	if (lrc_file != NULL)
	{
		tt_num_file = _music_parse_lrc_file(lrc_file, lrc_tts);
		cur_tt = 0;
	}
	bzero(lrc_str, 200);
	audioDecCmd(music_audio_player, SET_FILE, (UINT)fullname);
    TEST_VAL(fullname, %s);
	
	audioDecCmd(music_audio_player, GET_MEDIA_INFO, (UINT)&music_file_info);
	total_time = music_file_info.total_time;
	invalidate_music_info(h_play);
//	display_music_info(h_play, &music_file_info);

	play_param.mode = play_mode;
	play_param.param = (UINT)&tag_info;
	if (play_status.is_fade)
	{
		TEST_INFO("PLAY fade_in fade_in fade_in fade_in%%%%%%%%%%%%%%%%%%%%%%%%");
		audioDecCmd(music_audio_player, FADE_IN, 0);
	}
	audioDecCmd(music_audio_player, PLAY, (UINT)&play_param);
	change_bmp_on_control(h_ab, NULL);

	brow_sec = 0;
	play_status.speed = 0;
	dsp_status = DSP_STAT_PLAY;
	change_bmp_on_control(h_play_op, bmpinfo_play_op);
	
	SetTimer(h_play, ID_TIMER_NORM, 100);

}

static void normal_play(void)
{
	RECT rc;

	if (play_status.is_fade)
	{
#ifdef DEBUG
		printf("fadein\n");
#endif 
		audioDecCmd(music_audio_player, STOP, 0);
		audioDecCmd(music_audio_player, FADE_IN, 0);	
	}

	play_specially(NORMAL_PLAY);

	rc.left = TT_TIME_LEFT;
	rc.right = TT_TIME_RIGHT;
	rc.top = TT_TIME_TOP;
	rc.bottom = TT_TIME_BOTTOM;
	InvalidateRect(h_play, &rc, TRUE);
	
	norm_ab = STAT_NORM_PLAY;
///*old( right )FIXME	
	if (play_status.is_fade && dsp_status == DSP_STAT_PLAY)
	{
#ifdef DEBUG
		printf("fadein\n");
#endif 
		audioDecCmd(music_audio_player, FADE_IN, 0);	
	}
//	*/
}

static void play_first_file(void)
{
	/*select the first music file*/
	cur = select_file(file_list, IN_ORDER, NEXT_FILE, -1, fullname);
#ifdef DEBUG
	printf("start select %s\n", fullname);
#endif
	normal_play();

#ifdef DEBUG				
	printf("start play %s..\n", fullname);
#endif
	/*todo display status in control and file info*/

}


static int open_inital_dsp(void)
{
	/*open DSP*/
	music_audio_player = audioDecOpen(NULL);
    printf("open music device+++++++++++++++++++++++++++++++\n");
	if (music_audio_player == NULL)
		return -1;
	/*initial ab_wait_time, ab_count volume*/
	audioDecCmd(music_audio_player, SET_AB_WAIT_TIME, 
			play_status.ab_wait_time);
	audioDecCmd(music_audio_player, SET_AB_COUNT, 
			play_status.ab_count);
	audioDecCmd(music_audio_player, SET_VOLUME, 
			play_status.volume);
	audioDecCmd(music_audio_player, SET_FFB_STEP, 30);
	return 0;
}


static BOOL preview_toend(void)
{
	if (play_status.play_type == PREVIEW_PLAY)
	{
		brow_sec++;

		if (brow_sec > 5)
		{
#ifdef DEBUG
			printf("%s browse next\n", fullname);
#endif
			return TRUE;
				/*play next file during playing cur file*/
		}
	}
	return FALSE;
}


static void playing_timer(void) //get MSG_TIMER, while playing
{
	RECT rc;

	/*refresh slide on strip*/
	total_time = music_file_info.total_time;
	cur_time = music_status.cur_time;			
#ifdef DEBUG
//	printf("%s is playing....\n", fullname);
//	printf("tol-%u, cur-%u, mode-%u\n",total_time, cur_time, music_status.flag.AB_play_flag);
#endif
	slide_pos = SLIDE_START + (SLIDE_END - SLIDE_START)*cur_time/total_time;
	set_slide(h_strip, slide_pos);
	refresh_lrc(h_play, cur_time);

	rc.left = CUR_TIME_LEFT;
	rc.right = CUR_TIME_RIGHT;
	rc.top = CUR_TIME_TOP;
	rc.bottom = CUR_TIME_BOTTOM;
	refresh_time = TRUE;
	InvalidateRect(h_play, &rc, FALSE);

	/*if the ab_play is auto stop(DSP is normal playing),
	  change norm_ab to STAT_NORM_PLAY*/
	if(music_status.flag.AB_play_flag==0
			&&norm_ab != STAT_NORM_TO_AB)
	{
		norm_ab = STAT_NORM_PLAY;
		if (ab_used_flag == 2)
		{
			ab_used_flag = 0;
			change_bmp_on_control(h_ab, NULL);
		}
	}

	/*if near the end, fade_out*/
	if((music_file_info.total_time - music_status.cur_time < 10)
			&&(play_status.is_fade == TRUE) 
			&& (dsp_status == DSP_STAT_PLAY))
	{
#ifdef DEBUG
		printf("fade out\n");
#endif
		audioDecCmd(music_audio_player, FADE_OUT, 0);
	}
	
	/*if backforward to start, STOP */
	if (cur_time <= 4 && dsp_status == -1)
	{
		audioDecCmd(music_audio_player, STOP, 0);		
		set_slide(h_strip, SLIDE_START);
#ifdef DEBUG
		printf("BACK_FORWARD to start of song\n");
#endif
		//audioDecCmd(music_audio_player, PAUSE, 0);		
	}
}

#if 0
static void draw_bkground(WPARAM wparam, LPARAM lparam)
{
	HDC hdc = (HDC)wparam;
	BOOL f_get_dc = FALSE;
	RECT* clip = (RECT*)lparam;
	RECT rect;
	if (hdc == 0)
	{
		hdc = GetClientDC(h_play);
		f_get_dc = TRUE;
	}

	if (clip != NULL)
	{
		rect = *clip;
		ScreenToWindow(h_play, &rect.left, &rect.top);
		ScreenToWindow(h_play, &rect.right, &rect.bottom);
		IncludeClipRect(hdc, &rect);
	}


	FillBoxWithBitmap(hdc, 0, 0, 0, 0, bmpinfo_back->bmp);
	printf("m==========================\n");
	if (f_get_dc)
	{
		ReleaseDC(hdc);
	}
}
#endif

static void deal_timer(void)
{
	/*get DSP status*/
	audioDecCmd(music_audio_player, GET_STATUS, (UINT)&music_status);
	audioDecCmd(music_audio_player, GET_MEDIA_INFO, (UINT)&music_file_info);

	/*if playing, refresh slide, ab auto_stop, fade_out*/
	if (music_status.status == PLAYING)
	{
		playing_timer();
	}

	else if (music_status.status == PAUSED 
			|| soft_paused == TRUE)
	{

	}

	/*playing to the end , play next file, 
	  or stop. (FF to end, ignore it )*/
	else if ((music_status.status == STOPPED)
			&& dsp_status == DSP_STAT_PLAY)
	{
#ifdef DEBUG						
		printf("auto stopped\n");
#endif
		play_status.speed = 0;
		/*reset slide */	
		set_slide(h_strip, SLIDE_START);	
		slide_pos = SLIDE_START;	

		/*if in file playing , stop when to the end*/
		if (play_status.play_type == LIST_PLAY //FIXME
                ||play_status.play_type == DIR_PLAY)
		{
			SendMessage(h_play, MSG_PLAY_NEXT_FILE, 0, 0);
		}
		else if (play_status.loop_type == ALL_PLAY)
		{
			if (cur != last_file)
			{
				SendMessage(h_play, MSG_PLAY_NEXT_FILE, 0, 0);
			}
			else
			{
				change_bmp_on_control(h_play_op, bmpinfo_play_op+1);
				/* //jump to the first songs
				change_bmp_on_control(h_play_op, bmpinfo_play_op[1]);
				audioDecCmd(music_audio_player, SET_FILE, fullname);
				TEST_INFO("all return to first++++++++++");
				music_file_info_t music_file_info;
				audioDecCmd(music_audio_player, GET_MEDIA_INFO, (UINT)&music_file_info);
			
				invalidate_music_info(h_play);
				display_music_info(h_play, &music_file_info);
				TEST_INFO("all to stop play first++++++++++++");
				*/
			}
		}
		else if (play_status.loop_type == ONLY_ONE)
		{
			SendMessage(h_play, MSG_PLAY_NEXT_FILE, 0, 0);
		}
		else
		{

			/*in dir playing, set next file*/
#ifdef DEBUG
			printf("{{{loop}}} next file,--%s\n", fullname);
#endif
			change_bmp_on_control(h_play_op, bmpinfo_play_op+1);
		}

	}
	else 
    {
        if (music_status.status == STOPPED)
        {
            play_status.speed = 0;
            /*reset slide */	
            set_slide(h_strip, SLIDE_START);	
            slide_pos = SLIDE_START;	

            printf("back to head of file\n");
        }
    }
}


/*
static  int ab_restore_play(void)
{
	if (!have_ab_info)
	{
		return -1;
	}
	compose_path_name(fullname, path_ab, name_ab);
#ifdef DEBUG
	printf("ab restore play start...\n ab file is %s\n", fullname);
#endif
	memmove(&tag_info, &ab_tag_info, sizeof(tag_info_t));
	
	play_specially(AB_RESTORE_PLAY);

	norm_ab = STAT_AB_PLAY;
	return 0;
}
*/


#if 0
static int tag_play(void)
{
	//is_brow_play = FALSE;
	if (play_status.play_type == PREVIEW_PLAY)
	{
		play_status.play_type = LIST_PLAY;
	}
	if (!have_tag_info)
	{
		return -1;
	}
	compose_path_name(fullname, path_tag, name_tag);
#ifdef DEBUG
	printf("TAG play start...\n TAG file is %s\n", fullname);
#endif

	memmove(&tag_info, &tag_tag_info, sizeof(tag_info_t));
	
	play_specially(TAG_PLAY);
	norm_ab = STAT_NORM_PLAY;

	return 0;
}

static int bpt_play(void)
{
	if (play_status.play_type == PREVIEW_PLAY)
	{
		play_status.play_type = LIST_PLAY;
	}
	if (!have_exit_info)
	{
		return -1;
	}
	compose_path_name(fullname, path_exit, name_exit);
#ifdef DEBUG
	printf("break point play start...\n bpt file is %s\n", fullname);
#endif
	memmove(&tag_info, &exit_tag_info, sizeof(tag_info_t));
	play_specially(BPT_RESTORE_PLAY);
	return 0;
}
#endif

static void close_window(void)//FIXME
{
    printf("00000000000---<%s>\n", __FUNCTION__);
    printf("11111111111---<%s>\n", __FUNCTION__);
    if (file_is_selected)
    {
        audioDecClose (music_audio_player, 0);
        printf("close music device-------------------------------\n");
        if (norm_ab == STAT_NORM_PLAY)
        {
            divide_fullname(fullname, path_exit, name_exit);
            memmove(&exit_tag_info, &tag_info, sizeof(tag_info_t));
            save_exit_info();
#ifdef DEBUG
            printf("normal save tag %s-%s\n", path_exit, name_exit);
#endif	
        }
        else
        {
            divide_fullname(fullname, path_ab, name_ab);
            memmove(&ab_tag_info, &tag_info, sizeof(tag_info_t));
            save_ab_info();
#ifdef DEBUG
            printf("normal save tag %s-%s\n", path_ab, name_ab);
#endif	
        }
    }
	
    printf("22222222222---<%s>\n", __FUNCTION__);
	DestroyAllControls(h_play);
	DestroyMainWindow(h_play);
	MainWindowThreadCleanup(h_play);
}


static int  dealwith_operat(LPARAM lparam)
{
	switch (lparam)
	{
		case AB_PLAY:
			norm_ab = STAT_NORM_TO_AB;
			if (play_status.play_type == PREVIEW_PLAY)
			{
				play_status.play_type = LIST_PLAY;
			}

			SetTimer(h_play, ID_TIMER_TWINKLE, 50);
#ifdef DEBUG
			printf("AB_PLAY start...\n");
#endif
			break;
            /*
		case MUSIC_AB_RESTORE_PLAY:
			if (ab_restore_play() == -1)
			{
				return -1;
			}
#ifdef DEBUG
			printf("ab_restore_play\n");
#endif
			break;

		case MUSIC_TAG_PLAY:
			if (tag_play() == -1)
			{
				return -1;
			}
#ifdef DEBUG
			printf("tag_play\n");
#endif
			break;

		case MARK_TAG:
			divide_fullname(fullname, path_tag, name_tag);
			audioDecCmd(music_audio_player, MARK_A_TAG, (UINT)&tag_tag_info);
			have_tag_info = TRUE;
			save_tag_info();
#ifdef DEBUG
			printf("MARK TAGed \n");
#endif
			break;

		case ALERT_SPEED:
#ifdef DEBUG
			printf("alter speed to %d\n", (UINT)(play_status.speed));
#endif
			audioDecCmd(music_audio_player, ALTER_PLAY_SPEED, (UINT)(play_status.speed));
			break;
		case BPT_PLAY:
			if (bpt_play() == -1)
			{
				return -1;
			}
#ifdef DEBUG
			printf("bpt play\n");
#endif
			break;
            */
		default:
			return -1;

	}
	return 0;
}
/*
static int dealwith_command(WPARAM wparam, LPARAM lparam)
{
	switch (wparam)
	{
		case VOLUME:
#ifdef DEBUG
			printf("set volume <%d>\n", (UINT)(play_status.volume));
#endif
			audioDecCmd(music_audio_player, SET_VOLUME, (UINT)(play_status.volume));
			break;	

		case OPERATOR:
			{
				return dealwith_operat(lparam);
			}
		case AB_WAIT_TIME:
#ifdef DEBUG
			printf("set ab_wait_time <%d>\n", (UINT)(play_status.ab_wait_time));
#endif
			audioDecCmd(music_audio_player, SET_AB_WAIT_TIME, 
					(UINT)(play_status.ab_wait_time));
			break;
		case AB_COUNT:
#ifdef DEBUG
			printf("set ab_count <%d>\n", (UINT)(play_status.ab_count));
#endif
			audioDecCmd(music_audio_player, SET_AB_COUNT, 
					(UINT)(play_status.ab_count));
			break;
		case EQ_MODE:
#ifdef DEBUG
			printf("set eq_mode <%d>\n", (UINT)(play_status.eq_mode));
#endif
			change_bmp_on_control(h_eq, bitmap_eq+lparam);
			audioDecCmd(music_audio_player, SET_EQ, 
					(UINT)&(play_status.eq_mode));
			break;
		case LOOP_TYPE:
			if (play_status.loop_type == ALL_PLAY)
			{
				last_file = select_file(h_host, IN_ORDER, PRE_FILE, cur, fullname);
				TEST_INFO("start all play++++++++++++");
				TEST_VAL(last_file, %d);
				select_file(h_host, IN_ORDER, NEXT_FILE, last_file, fullname);
			}
			change_bmp_on_control(h_loop, bitmap_loop+lparam);
			break;
	}
	return 0;
}
*/


/*proc in play window*/
static int play_proc(HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
    h_play = hwnd;

    switch (message) {
        case MSG_NCCREATE:
        SetWindowAdditionalData2 (hwnd, 0);
        break;

        /*creat control, open dsp, load 3 path_name_tag*/
        case MSG_CREATE:
            {

                /*load tag_info*/
                pmp_app_notify_opened(PMP_APP_MUSIC, hwnd);
               

                create_control();
                h_host = GetHosting(hwnd);


                lrc_font = CreateLogFont (NULL, "song", "GB2312",
                        FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN, 
                        FONT_SETWIDTH_NORMAL, FONT_SPACING_CHARCELL, 
                        FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
                        7, 0);

                {
                    HDC hdc = GetDC(h_play);
                    mem_dc_save = CreateCompatibleDCEx(hdc, CUR_TIME_RIGHT-CUR_TIME_LEFT,
                            CUR_TIME_BOTTOM - CUR_TIME_TOP);
                    mem_dc_put = CreateCompatibleDCEx(hdc, CUR_TIME_RIGHT-CUR_TIME_LEFT,
                            CUR_TIME_BOTTOM - CUR_TIME_TOP);

                    mem_dc_save_lrc = CreateCompatibleDCEx(hdc, LRC_RIGHT-LRC_LEFT,
                            LRC_BOTTOM - LRC_TOP);
                    mem_dc_put_lrc = CreateCompatibleDCEx(hdc, LRC_RIGHT - LRC_LEFT,
                            LRC_BOTTOM - LRC_TOP);

                    BitBlt(hdc, CUR_TIME_LEFT, CUR_TIME_TOP, 
                            CUR_TIME_RIGHT - CUR_TIME_LEFT,
                            CUR_TIME_BOTTOM - CUR_TIME_TOP,
                            mem_dc_save, 0, 0, 0);
                    BitBlt(hdc, CUR_TIME_LEFT, CUR_TIME_TOP, 
                            CUR_TIME_RIGHT - CUR_TIME_LEFT,
                            CUR_TIME_BOTTOM - CUR_TIME_TOP,
                            mem_dc_put, 0, 0, 0);

                    BitBlt(hdc, LRC_LEFT, LRC_TOP, 
                            LRC_RIGHT - LRC_LEFT,
                            LRC_BOTTOM - LRC_TOP,
                            mem_dc_put_lrc, 0, 0, 0);
                    BitBlt(hdc, LRC_LEFT, LRC_TOP, 
                            LRC_RIGHT - LRC_LEFT,
                            LRC_BOTTOM - LRC_TOP,
                            mem_dc_save_lrc, 0, 0, 0);
                    ReleaseDC(hdc);
                    TEST_INFO("<<<<<<<<<<<save to mem_dc>>>>>>>>. ");
                }

                file_list = _music_show_files(hwnd);
                printf("11111111--%s\n", __FUNCTION__);

                if (file_list == NULL) 
                {
                    SendMessage(hwnd, MSG_CLOSE, 0, 0);
                    return 0;
                }
                else
                {
                    file_is_selected = TRUE;
                    have_exit_info = load_exit_info();
                    have_tag_info = load_tag_info();
                    have_ab_info = load_ab_info();
                }
                
#ifndef EVB
                return 0; /*for PC*/
#endif
                if (open_inital_dsp() == -1)
                {
                    MessageBox(hwnd, _(IDS_PMP_MUSIC_DSPERROR), "", 0);
                    PostMessage(hwnd, MSG_CLOSE, 0 ,0);
                    return 0;
                }

                    printf("22222222--%s\n", __FUNCTION__);
                play_first_file();
                    printf("33333333--%s\n", __FUNCTION__);

                if (play_status.loop_type == ALL_PLAY)
                {
                    last_file = select_file(file_list, IN_ORDER, PRE_FILE, cur, fullname);
                    printf("44444444--%s\n", __FUNCTION__);
                    TEST_INFO("start all play++++++++++++");
                    TEST_VAL(last_file, %d);
                    select_file(file_list, IN_ORDER, NEXT_FILE, last_file, fullname);
                }
                    printf("55555555--%s\n", __FUNCTION__);
                /*timer used to refresh slide and count time in browse playing*/
                return 0;

            }
            break;

        case MSG_SETFOCUS:
            
            /* When music is lanch by picture, pmp_show_captionbar(TRUE) must firstly be
             * called to show captionbar.  */
            pmp_show_captionbar (TRUE);
            pmp_set_captionbar_title(_(IDS_PMP_MUSIC_TITLE)/*"music"*/);
            break;
   
        case MSG_PAINT:
            {
                HDC hdc = BeginPaint(hwnd);
                LOGFONT* old_font;
#ifdef DEBUG
                printf("get MSG_PAINT\n");
#endif
                /*display music info(textOut on playing window)*/
                if (!file_is_selected)
                {
                    EndPaint(hwnd, hdc);
                    return 0;
                }
             //   printf("deal with MSG_PAINT\n");
                audioDecCmd(music_audio_player, GET_MEDIA_INFO, (UINT)&music_file_info);
                SetTextColor(hdc, COLOR_green);
                display_music_info(hdc, &music_file_info);
                old_font = SelectFont(hdc, lrc_font);

                SetTextColor(hdc, COLOR_yellow);
                display_lrc(hdc);
                SelectFont(hdc, old_font);
                display_time(hdc);
                EndPaint(hwnd, hdc);
                set_slide(h_strip, slide_pos);
                return 0;

            }
            break;

        case MSG_KEYDOWN:
        case MSG_KEYUP:
            {
                if (norm_ab == STAT_NORM_PLAY)
                {
                    /*deal key in normal play*/
                    deal_key_normal(hwnd, message, wparam, music_audio_player);
                }
                else
                {
#ifdef DEBUG
                    printf("deal AB play KEY\n");
#endif
                    /*deal key in ab playing*/
                    if (message == MSG_KEYDOWN)
                    {
                        norm_ab = deal_key_ab(hwnd, message, wparam, music_audio_player);
                    }
                }
            }
            break;

            /*refresh huakuai and stop playing in browse*/	
        case MSG_TIMER:
#ifndef EVB
            return 0; /*FIXME PC*/
#endif
            if (wparam == ID_TIMER_NORM)
            {
#ifdef DEBUG
                //				printf("get MSG_TIMER\n");
#endif

                if (preview_toend() == TRUE)
                {
                    SendMessage(hwnd, MSG_PLAY_NEXT_FILE, 0, 0);
                    return 0;
                }

                deal_timer();
            }
            else if (wparam == ID_TIMER_TWINKLE)
            {
                if (norm_ab == STAT_NORM_TO_AB)
                {
                    twinkle_hwnd(h_ab);
                }
            }
            break;

        case MSG_PLAY_NEXT_FILE:
            cur = select_file(file_list, play_status.loop_type, NEXT_FILE, cur, fullname);
#ifdef DEBUG
            printf("next---select %s\n", fullname);
#endif

            normal_play();
            break;

        case MSG_PLAY_PRE_FILE:
            cur = select_file(file_list, play_status.loop_type, PRE_FILE, cur, fullname);
#ifdef DEBUG
            printf("pre---select %s\n", fullname);
#endif
            normal_play();
            break;

        case MSG_CLOSE:


            unload_bitmap_array(bmpinfo_play_op, 
                    sizeof(bmpinfo_play_op)/sizeof(bmpinfo_play_op[0]));
            unload_bitmap_array(bmpinfo_loop, 
                    sizeof(bmpinfo_loop)/sizeof(bmpinfo_loop[0]));
            unload_bitmap_array(bmpinfo_ab, 
                    sizeof(bmpinfo_ab)/sizeof(bmpinfo_ab[0]));
            unload_bitmap_array(bmpinfo_eq, 
                    sizeof(bmpinfo_eq)/sizeof(bmpinfo_eq[0]));
            unload_bitmap_array(bmpinfo_trackbar, 
                    sizeof(bmpinfo_trackbar)/sizeof(bmpinfo_trackbar[0]));
            unload_bitmap_array(bmpinfo_back, 
                    sizeof(bmpinfo_back)/sizeof(bmpinfo_back[0]));

            DestroyLogFont(lrc_font);

            DeleteCompatibleDC(mem_dc_save);
            DeleteCompatibleDC(mem_dc_put);
            DeleteCompatibleDC(mem_dc_put_lrc);
            DeleteCompatibleDC(mem_dc_save_lrc);
            if (file_list) PMP_RELEASE_MEDIA_FILE_LIST(file_list);
            file_list = NULL;

            pmp_app_notify_closed(PMP_APP_MUSIC);
            close_window();
            
            file_is_selected = FALSE;
            return 0;
            break;

        case MSG_START_MENU:
            _music_show_menu(hwnd, "");
            break;
        default:
            break;
    }

    return pmp_def_wnd_proc(hwnd, message, wparam, lparam);
}

void ex_music_play_music(HWND h_host)
{

	HWND hwnd;

    bmpinfo_back[0].bmp = pmp_image_res_acquire_bitmap(bmpinfo_back[0].file_name);
	
    init_creatinfo(&create_info, h_host);
	hwnd = CreateMainWindow(&create_info);

    if (hwnd == HWND_INVALID)
	{
		printf("creat window failed\n");
		return;
	}

	ShowWindow(hwnd, SW_SHOWNORMAL);
}


static void deal_key_normal(HWND hwnd, int message, WPARAM wparam, void* music_audio_player)
{
//	music_status_t music_status;
//	audioDecCmd(music_audio_player, GET_STATUS, (UINT)&music_status);
	switch (message)
	{
		case MSG_KEYDOWN:
			{
				norm_key_down(hwnd, wparam, music_audio_player);
			}
			break;
		case MSG_KEYLONGPRESS:
			{
//				norm_key_alwayspress(hwnd, wparam, music_audio_player);
			}
			break;
		case MSG_KEYUP:
			{
#ifdef DEBUG
				printf("--------------get MSG_KEYUP--------------\n");
#endif
				norm_key_up(hwnd, wparam, music_audio_player);
			}
			break;
		default:
			break;

	}
}


static void norm_key_down(HWND hwnd, WPARAM wparam, void* music_audio_player)
{
    music_status_t music_status;
    int h_play_op;

    audioDecCmd(music_audio_player, GET_STATUS, (UINT)&music_status);
    h_play_op = GetDlgItem(hwnd, IDC_PLAY_OP);

    switch (wparam)
    {
        case SCANCODE_ENTER:
            if (music_status.status == PLAYING)
            {
                //				HDC hdc;
                change_bmp_on_control(h_play_op, bmpinfo_play_op+1);
                if (play_status.is_fade)
                {
                    audioDecCmd(music_audio_player, MARK_A_TAG, (UINT)&tag_info);
                    TEST_VAL(tag_info.time, %d);
                    audioDecCmd(music_audio_player, FADE_OUT, 0);
                    audioDecCmd(music_audio_player, STOP, 0);
                    soft_paused = TRUE;
                    TEST_INFO("soft pause<<<<<<<<<<<");
                }
                else
                {
                    audioDecCmd(music_audio_player, PAUSE, 0);
                }
#ifdef DEBUG
                printf("status is %u, paused\n",music_status.status);
#endif
                //				KillTimer(hwnd, ID_TIMER_NORM);
            }
            else if (music_status.status == PAUSED
                    || music_status.status == STOPPED)
            {
                change_bmp_on_control(h_play_op, bmpinfo_play_op);
                if (music_status.status == PAUSED)
                {
                    audioDecCmd(music_audio_player, CONTINUE, 0);
                }
                else
                {
                    if (soft_paused == TRUE)
                    {
                        play_param_t play_param;

                        TEST_VAL(tag_info.time, %d);
                        play_param.mode = TAG_PLAY;
                        play_param.param = (UINT)&tag_info;
                        audioDecCmd(music_audio_player, FADE_IN, 0);
                        audioDecCmd(music_audio_player, PLAY, (UINT)&play_param);
                        audioDecCmd(music_audio_player, ALTER_PLAY_SPEED, play_status.speed);
                        soft_paused = FALSE;
                        TEST_INFO("soft continue>>>>>>>>>>>>>\n");
                    }
                    else
                    {
                        if (play_status.loop_type == ALL_PLAY
                                && cur == last_file)
                        {
                            cur = select_file(file_list, IN_ORDER, NEXT_FILE, cur, fullname);
                            normal_play();
                        }
                        else
                        {
                            play_param_t play_param;
                            tag_info_t tag_info;
                            play_param.mode = NORMAL_PLAY;
                            play_param.param = (UINT)&tag_info;

                            if (play_status.is_fade)
                            {
                                TEST_INFO("FF fade_in fade_in fade_in fade_in fade_in%%%%%%%%%%%");
                                audioDecCmd(music_audio_player, FADE_IN, 0);
                            }
                            audioDecCmd(music_audio_player, PLAY, (UINT)&play_param);

                            brow_sec = 0;
                            dsp_status = DSP_STAT_PLAY;
                        }
                    }
                }
#ifdef DEBUG
                printf("status is %u, continued\n",music_status.status);
#endif
            }
            break;

        case SCANCODE_CURSORBLOCKUP:
            cur = select_file(file_list, IN_ORDER, PRE_FILE, cur, fullname);
            normal_play();
            break;
        case SCANCODE_CURSORBLOCKDOWN:
            cur = select_file(file_list, IN_ORDER, NEXT_FILE, cur, fullname);
            normal_play();
            break;

        case SCANCODE_CURSORBLOCKRIGHT:
            {
                if (play_status.play_type == PREVIEW_PLAY)
                {
                    return ;
                }
                if (dsp_status == 0)
                {
                    audioDecCmd(music_audio_player, FAST_FORWARD, 0);
#ifdef DEBUG
                    printf("FAST_FORWARD start..\n");
#endif
                    dsp_status = 1;
                    return;
                }
            }
            break;
        case SCANCODE_CURSORBLOCKLEFT:
            {
                if (play_status.play_type == PREVIEW_PLAY)
                {
                    return;
                }
                
                music_status_t music_status;
	            audioDecCmd(music_audio_player, GET_STATUS, (UINT)&music_status);
                if (music_status.status == STOPPED)
                    return;

                if (dsp_status == 0)
                {

                    /*fastward to start*/
                    if (music_status.cur_time <= -1)//FIXME
                    {
                        tag_info_t tag;
                        audioDecCmd(music_audio_player, STOP, (UINT)&tag);
#ifdef DEBUG
                        printf("fast back to 10s........\n");
#endif
                        //audioDecCmd(music_audio_player, PAUSE, 0);
                    }
                    else
                    {
                        audioDecCmd(music_audio_player, FAST_BACKWARD, 0);
                    }

                    dsp_status = -1;
#ifdef DEBUG
                    printf("BACK_FORWARD start..\n");
#endif
                    return;
                }
            }
            break;
        case SCANCODE_ESCAPE:
            PostMessage(hwnd, MSG_CLOSE, 0, 0);

            dsp_status = 0;
            break;

        case SCANCODE_F1:
#ifdef DEBUG
            printf("F1 is pressed\n");
#endif
            PostMessage(hwnd, MSG_START_MENU, 0, 0);
            break;
        default:
            break;


    }
}
/*
static void norm_key_alwayspress(HWND hwnd, WPARAM wparam, void* music_audio_player)
{
	music_status_t music_status;
	audioDecCmd(music_audio_player, GET_STATUS, (UINT)&music_status);
	switch (wparam)
	{
		case SCANCODE_CURSORBLOCKLEFT:
			audioDecCmd(music_audio_player, FAST_BACKWARD, 0);
			break;

		case SCANCODE_CURSORBLOCKRIGHT:
			audioDecCmd(music_audio_player, FAST_FORWARD, 0);
#ifdef DEBUG
			printf("FAST_FORWARD start..\n");
#endif
			break;

		case SCANCODE_ENTER:
			if (music_status.status == PLAYING)
			{
				audioDecCmd(music_audio_player, STOP, (UINT)&tag_info);
				KillTimer(hwnd, ID_TIMER_NORM);
			}

			else if (music_status.status == STOPPED
					|| music_status.status == ERROR)
			{
				PostMessage(hwnd, MSG_PLAY_NEXT_FILE, 0, 0);
			}
			break;

		default:
			break;

	}
}
*/


static void norm_key_up(HWND hwnd, WPARAM wparam, void* music_audio_player)
{
	music_status_t music_status;
	audioDecCmd(music_audio_player, GET_STATUS, (UINT)&music_status);

#ifdef DEBUG
	printf("++++++++++++++++++++++++++key_up=====\n");
#endif
	switch (wparam)
	{
		case SCANCODE_CURSORBLOCKLEFT:
			if (dsp_status == -1)
			{
				if (music_status.status == STOPPED)
				{
					play_param_t play_param;
					tag_info_t tag_info;
					play_param.mode = NORMAL_PLAY;
					play_param.param = (UINT)&tag_info;
					
					if (play_status.is_fade)
					{
						TEST_INFO("FF fade_in fade_in fade_in fade_in fade_in%%%%%%%%%%%");
						audioDecCmd(music_audio_player, FADE_IN, 0);
					}
					audioDecCmd(music_audio_player, PLAY, (UINT)&play_param);
					
					brow_sec = 0;
					play_status.speed = 0;
					dsp_status = DSP_STAT_PLAY;
					change_bmp_on_control(h_play_op, bmpinfo_play_op);
					change_bmp_on_control(h_ab, NULL);
				}
				else
				{
					audioDecCmd(music_audio_player, CANCEL_FB, 0);
#ifdef DEBUG
					printf("...BACK_FORWARD end\n");
#endif
					dsp_status = 0;
				}
			}
			break;

		case SCANCODE_CURSORBLOCKRIGHT:
			if (dsp_status == 1)
			{
				audioDecCmd(music_audio_player, CANCEL_FF, 0);
#ifdef DEBUG
				printf("...FAST_FORWARD end\n");
#endif
				dsp_status = 0;
			}
			break;
		default:
			break;
	}
}

static int deal_key_ab(HWND hwnd, int message, WPARAM wparam, void* music_audio_player)
{
	music_status_t music_status;
	if (wparam != SCANCODE_ENTER)
	{
#ifdef DEBUG
			printf("(key not enter)\n");
#endif
		return STAT_NORM_TO_AB;
	}
	
	audioDecCmd(music_audio_player, GET_STATUS, (UINT)&music_status);

	if (music_status.status != PLAYING)
	{
		change_bmp_on_control(h_ab, NULL);
		audioDecCmd(music_audio_player, CLEAR_AB, 0);
		ab_used_flag = 0;
#ifdef DEBUG
		printf("AB_PLAY end(status is not playing)\n");
#endif
		return STAT_NORM_PLAY;
	}

	else
	{
		if (0 == ab_used_flag)
		{
			audioDecCmd(music_audio_player, SET_A, 0);
//			change_bmp_on_control(h_ab, bmpinfo_ab[0]);
#ifdef DEBUG
			printf("A is setted\n");
#endif
			ab_used_flag = 1;
			return STAT_NORM_TO_AB;
		}
		else if (1 == ab_used_flag)
		{
			audioDecCmd(music_audio_player, SET_B, 0);
			change_bmp_on_control(h_ab, bmpinfo_ab+4);
#ifdef DEBUG
			printf("B is setted\n");
#endif

			ab_used_flag = 2;
			return STAT_AB_PLAY;
		}
		else
		{
			audioDecCmd(music_audio_player, MARK_A_TAG, (UINT)&ab_tag_info);	
			audioDecCmd(music_audio_player, CLEAR_AB, 0);
			
			change_bmp_on_control(h_ab, NULL);
			KillTimer(hwnd, ID_TIMER_TWINKLE);
			ab_used_flag = 0;
#ifdef DEBUG
		printf("AB_PLAY end(cleared AB)\n");
#endif
			return STAT_NORM_PLAY;
		}
	}
}

/*
static void change_file_browser(HWND hwnd, char* path)
{
	char* path_buf;
	path_buf = (char *)GetWindowAdditionalData(hwnd);
	strcpy(path_buf, path);
	get_items_by_path(hwnd);
}
*/
static int select_file(PMP_MEDIA_FILE_LIST* list, int loop_mode, int select_mod, 
		int cur, char* fullname)
{
	int i;
	char* file_name;
	if (cur < 0)
	{
		strcpy(fullname, list->dir);
		strcat(fullname, list->file_names);
        TEST_VAL(fullname, %s);
		return 0;
	}

	if (loop_mode == ONLY_ONE)
	{
		return cur;
	}
	else
	{
		if (select_mod == NEXT_FILE)
		{
			if (loop_mode == IN_ORDER
				||loop_mode == ALL_PLAY)
			{
				cur = (cur + 1) % list->nr_files;
			}
			else if (loop_mode == RANDOM_SEL)
			{
				cur = (cur + rand()) % list->nr_files;
			}
		}
		else if (select_mod == PRE_FILE)
		{
			cur = (cur + list->nr_files - 1) % list->nr_files;
		}

		file_name = list->file_names;
		for (i=0; i<cur; i++)
		{
            printf("%s\n", file_name);
			file_name = file_name + strlen(file_name) + 1;
		}
        printf(" <%d>--<%s>--<sum %d>-----%s\n", cur, file_name, list->nr_files, __FUNCTION__);
		strcpy(fullname, list->dir);
		strcat(fullname, file_name);
        TEST_VAL(fullname, %s);
		return cur;
	}
}


static BOOL load_exit_info()
{
	FILE* file;
	int fd;
	file = fopen (PMP_CFG_PATH "exit_info", "r");

	if (file == NULL)
	{
		return FALSE;
	}
	else
	{
		FILE* music_file = NULL;
		char fullname[MY_PATH_MAX];
		fd = fileno(file);
		read(fd, path_exit,	MY_PATH_MAX);
		read(fd, name_exit, NAME_MAX);
		read(fd, &exit_tag_info, sizeof(tag_info_t));
		close(fd);
		
		compose_path_name (fullname, path_exit, name_exit);
		music_file = fopen (fullname, "r");
		if (music_file == NULL)
		{
			return FALSE;
		}
		else
		{
			fclose(music_file);
			return TRUE;
		}
	}

}

static BOOL load_tag_info()
{
	FILE* file;
	int fd;
	file = fopen ("tag_info", "r");

	if (file == NULL)
	{
		return FALSE;
	}
	else
	{
		FILE* music_file = NULL;
		char fullname[MY_PATH_MAX];
		
		fd = fileno(file);
		read(fd, path_tag,	MY_PATH_MAX);
		read(fd, name_tag, NAME_MAX);
		read(fd, &tag_tag_info, sizeof(tag_info_t));
		close(fd);
		
		compose_path_name(fullname, path_tag, name_tag);
		music_file = fopen(fullname, "r");
		if (music_file == NULL)
		{
			return FALSE;
		}
		else
		{
			fclose(music_file);
			return TRUE;
		}
	}

}


static BOOL load_ab_info()
{
	FILE* file;
	int fd;
	file = fopen(PMP_CFG_PATH "ab_info", "r");

	if (file == NULL)
	{
		return FALSE;
	}
	else
	{
		FILE* music_file = NULL;
		char fullname[MY_PATH_MAX];
		
		fd = fileno(file);
		read(fd, path_ab,	MY_PATH_MAX);
		read(fd, name_ab, NAME_MAX);
		read(fd, &ab_tag_info, sizeof(tag_info_t));
		close(fd);
		
		compose_path_name(fullname, path_ab, name_ab);
		music_file = fopen(fullname, "r");
		if (music_file == NULL)
		{
			return FALSE;
		}
		else
		{
			fclose(music_file);
			return TRUE;
		}
	}
}

static void save_exit_info()
{
	FILE* file;
	int fd;
	file = fopen(PMP_CFG_PATH "exit_info", "w");
    if (file == NULL)
    {
        perror("save_exit_info--");
        return;
    }

	fd = fileno(file);
	write(fd, path_exit, MY_PATH_MAX);
	write(fd, name_exit, NAME_MAX);
	write(fd, &exit_tag_info, sizeof(tag_info_t));
	close(fd);
}

#if 0
static void save_tag_info(void);
static void save_tag_info()
{
	FILE* file;
	int fd;
	file = fopen(PMP_CFG_PATH "tag_info", "w");
    if (file == NULL)
    {
        perror("save_tag_info--");
        return;
    }

	fd = fileno(file);
	write(fd, path_tag, MY_PATH_MAX);
	write(fd, name_tag, NAME_MAX);
	write(fd, &tag_tag_info, sizeof(tag_info_t));
	close(fd);
}
#endif

static void save_ab_info()
{
	FILE* file;
	int fd;
	file = fopen(PMP_CFG_PATH "ab_info", "w");
    if (file == NULL)
    {
        perror("save_ab_info--");
        return;
    }
	
    fd = fileno(file);
	write(fd, path_ab, MY_PATH_MAX);
	write(fd, name_ab, NAME_MAX);
	write(fd, &ab_tag_info, sizeof(tag_info_t));
	close(fd);
}

static inline void load_bitmap (BMPINFO* bmp_info)
{
    if (bmp_info->bmp == NULL)
    {
        bmp_info->bmp = pmp_image_res_acquire_bitmap(bmp_info->file_name);
    }
}

static inline void unload_bitmap (BMPINFO* bmp_info)
{
    if (bmp_info->bmp != NULL)
    {
        pmp_image_res_release_bitmap(bmp_info->file_name, bmp_info->bmp);
        bmp_info->bmp = NULL;
    }
}

static void unload_bitmap_array(BMPINFO* bmp_info, int num)
{
    int i;
    for (i=0; i<num; i++)
    {
        if (bmp_info->bmp != NULL)
        {
            pmp_image_res_release_bitmap(bmp_info->file_name, bmp_info->bmp);
            bmp_info->bmp = NULL;
        }
        bmp_info++;
    }
}

static void change_bmp_on_control(HWND hwnd, BMPINFO* bmp_info)
{
	static HWND old_hwnd;
	static BMPINFO* pre_bmp_info;
    BMPINFO* old_info = (BMPINFO*)GetWindowAdditionalData(hwnd);

	if (pre_bmp_info == bmp_info && old_hwnd == hwnd)
	{
		return;
	}
	
    if (old_info != NULL)
    {
        unload_bitmap(old_info);
    }

    if (bmp_info == NULL)
    {
        SetWindowAdditionalData(hwnd, (DWORD)0);
	    SetWindowAdditionalData2(hwnd, (DWORD)0);
        UpdateWindow(hwnd, FALSE);
        return;
    }

    load_bitmap(bmp_info);

    SetWindowAdditionalData(hwnd, (DWORD)bmp_info);
	SetWindowAdditionalData2(hwnd, (DWORD)(bmp_info->bmp));
	
    UpdateWindow(hwnd, FALSE);
	
    old_hwnd = hwnd;
	pre_bmp_info = bmp_info;
}

static int old_pos = SLIDE_START;
static void set_slide(HWND h_strip, int pos)
{
	HDC hdc;
	BOOL suc;
	hdc = GetDC(h_strip);
	suc = FillBoxWithBitmapPart(hdc, old_pos, 0, SLIDE_W, bmpinfo_trackbar->bmp->bmHeight, 0, 0, bmpinfo_trackbar->bmp, old_pos, 0);
	
    FillBoxWithBitmap(hdc, pos, 0, SLIDE_W, SLIDE_H, (bmpinfo_trackbar+1)->bmp);
	old_pos = pos;
	ReleaseDC(hdc);
}

static void invalidate_music_info(HWND hwnd)
{
	RECT rc;
	
	rc.left = INFO_LEFT;
	rc.right = INFO_RIGHT;
	rc.top = INFO_TOP;
	rc.bottom = INFO_BOTTOM;	
	
	InvalidateRect(hwnd, &rc, TRUE);

}

static void	display_music_info(HDC hdc, music_file_info_t* music_file_info)
{
	RECT rc;
	char info_str[200];

	rc.left = INFO_LEFT;
	rc.right = INFO_RIGHT;
	rc.top = INFO_TOP;
	rc.bottom = INFO_BOTTOM;	
/*"%s\n[bit/s]:%u\n[sample_rate]:%u\n[channel]:%u\n%s\n%s"*/
	snprintf (info_str, 200, _(IDS_PMP_MUSIC_SONGINFO),
			music_file_info->filename, music_file_info->bitrate,
			music_file_info->sample_rate,  music_file_info->channels,
			music_file_info->license_info, (char*)music_file_info->id3_info);


	SetBkMode(hdc, BM_TRANSPARENT);
	DrawText(hdc, info_str, -1, &rc, DT_CENTER|DT_WORDBREAK|DT_NOCLIP);
}


static void	display_lrc(HDC hdc)
{
	RECT rc;
	//gal_pixel old_color;

	/*area to display lrc*/
	rc.left = 0;
	rc.right = LRC_RIGHT - LRC_LEFT;
	rc.top = 0;
	rc.bottom = LRC_BOTTOM - LRC_TOP;	

	BitBlt(mem_dc_save_lrc, 0, 0, 0, 0, mem_dc_put_lrc, 0, 0, 0);
	
	
	/*display lrc*/
	SelectFont(mem_dc_put_lrc, lrc_font);
	SetBkMode(mem_dc_put_lrc, BM_TRANSPARENT);
	
	TEST_INFO("display lrc----------\n");
	SetTextColor(mem_dc_put_lrc, COLOR_yellow);
	DrawText(mem_dc_put_lrc, lrc_str, -1, &rc, 
			DT_SINGLELINE|DT_VCENTER|DT_CENTER|DT_WORDBREAK|DT_NOCLIP);
	BitBlt(mem_dc_put_lrc, 0, 0, 0, 0, hdc, LRC_LEFT, LRC_TOP, 0);
}

FILE* fopen_lrc_from_music(char* music_name)
{
	FILE* fp;
	char* dot_pos;

	strcpy(lrc_name, music_name);
	dot_pos = rindex(lrc_name, '.');
	
	if (dot_pos != NULL)
	{
		*dot_pos = '\0';
	}
	
	strcat(lrc_name, ".lrc");
	TEST_VAL(lrc_name, %s);

	fp = fopen(lrc_name, "r");
	return fp;
}

void move_cur_tt(int time)
{
	/*to [cur_tt+1].time > time*/
	while ((cur_tt < tt_num_file-1)
		&&(lrc_tts[cur_tt+1].time <= time ))
	{
		cur_tt++;
	}

	/*to [cur_tt].time <= time*/
	while ((cur_tt > 0)
		&& (lrc_tts[cur_tt].time > time))
	{
		cur_tt--;
	}
}
void refresh_lrc(HWND hwnd, int time)
{
	int old_cur_tt = cur_tt;

	if (lrc_file == NULL)
	{
		return;
	}

	TEST_VAL(time, %d);
	TEST_VAL(lrc_tts[cur_tt-1].time, %d);
	TEST_VAL(lrc_tts[cur_tt].time, %d);
	TEST_VAL(lrc_tts[cur_tt+1].time, %d);


	move_cur_tt(time);

	if (old_cur_tt != cur_tt)
	{

		RECT rect;
		rect.left = LRC_LEFT;
		rect.right = LRC_RIGHT;
		rect.top = LRC_TOP;
		rect.bottom = LRC_BOTTOM;

		fseek(lrc_file, lrc_tts[cur_tt].lrc_pos, SEEK_SET);
		GET_LINE(lrc_file, lrc_str);
		
		InvalidateRect(h_play, &rect, FALSE);
	}
}

void twinkle_hwnd(HWND hwnd)
{
	static BOOL flag1 = TRUE;
	static BOOL flag2 = TRUE;

	TEST_VAL(ab_used_flag, %d);

	if (ab_used_flag == 0)
	{
		if (flag1 == FALSE)
		{
            change_bmp_on_control(hwnd, bmpinfo_ab);
			UpdateWindow(hwnd, FALSE);
			
			flag1 = TRUE;
			TEST_INFO("twinkle A0000000000");
		}
		else
		{
            change_bmp_on_control(hwnd, bmpinfo_ab+1);
			UpdateWindow(hwnd, FALSE);

			flag1 = FALSE;
			TEST_INFO("twinkle A1111111111");
		}
	}
	else if (ab_used_flag == 1)
	{
		if (flag2 == FALSE)
		{
            change_bmp_on_control(hwnd, bmpinfo_ab+2);
			UpdateWindow(hwnd, FALSE);
		
			flag2 = TRUE;
			TEST_INFO("twinkle B000000000");
		}
		else
		{
            change_bmp_on_control(hwnd, bmpinfo_ab+3);
			UpdateWindow(hwnd, FALSE);

			flag2 = FALSE;
			TEST_INFO("twinkle B111111111");
		}

	}
}

void display_time(HDC hdc)
{
	char time_str[4];
	RECT rc;

	FUN_START

	BitBlt(mem_dc_save, 0, 0, 0, 0, mem_dc_put, 0, 0, 0);

	rc.left = TT_TIME_LEFT;
	rc.right = TT_TIME_RIGHT;
	rc.top = TT_TIME_TOP;
	rc.bottom = TT_TIME_BOTTOM;
	sprintf(time_str, "%d", total_time);
	DrawText(hdc, time_str, -1, &rc, DT_CENTER|DT_WORDBREAK|DT_NOCLIP);
	

	rc.left = 0;
	rc.right = CUR_TIME_RIGHT - CUR_TIME_LEFT;
	rc.top = 0;
	rc.bottom = CUR_TIME_BOTTOM - CUR_TIME_TOP;
	sprintf(time_str, "%d", cur_time);
	//DrawText(hdc, time_str, -1, &rc, DT_CENTER|DT_WORDBREAK|DT_NOCLIP);
	SetTextColor(mem_dc_put, COLOR_yellow);
	SetBkMode(mem_dc_put, BM_TRANSPARENT);
	DrawText(mem_dc_put, time_str, -1, &rc, DT_CENTER|DT_WORDBREAK|DT_NOCLIP);
	
	BitBlt(mem_dc_put, 0, 0, CUR_TIME_RIGHT-CUR_TIME_LEFT, CUR_TIME_BOTTOM-CUR_TIME_TOP, 
			hdc, CUR_TIME_LEFT, CUR_TIME_TOP, 0);
	refresh_time = FALSE;
	FUN_END

}


extern PMP_MENU_ITEM ex__music_menu_item[];
extern menu_context_t ex__music_menu_cx;
static void _music_show_menu(HWND parent, const char* title)
{
	int i;
	ex__music_menu_cx.volume.value = play_status.volume;
	ex__music_menu_cx.play_type.value = play_status.play_type;
	ex__music_menu_cx.loop_type.value = play_status.loop_type;
	ex__music_menu_cx.operator_id.value = play_status.operator_index;
	ex__music_menu_cx.speed.value = play_status.speed;
	ex__music_menu_cx.is_fade.value = play_status.is_fade;
	ex__music_menu_cx.ab_wait_time.value = play_status.ab_wait_time;
	ex__music_menu_cx.ab_count.value = play_status.ab_count;
	ex__music_menu_cx.eq_mode.value = play_status.eq_mode;
	ex__music_menu_cx.eq_user_1.value = play_status.eq_user_1 / 51;
	ex__music_menu_cx.eq_user_2.value = play_status.eq_user_2 / 51;
	ex__music_menu_cx.eq_user_3.value = play_status.eq_user_3 / 51;

	i = pmp_show_menu (parent, _(IDS_PMP_MUSIC_MENUTITLE)/*"music options"*/, ex__music_menu_item, &ex__music_menu_cx);

	play_status.volume = ex__music_menu_cx.volume.value;
	
	play_status.eq_mode = ex__music_menu_cx.eq_mode.value;
	change_bmp_on_control(h_eq, bmpinfo_eq+play_status.eq_mode);

	play_status.eq_user_1 = ex__music_menu_cx.eq_user_1.value * 51;
	play_status.eq_user_2 = ex__music_menu_cx.eq_user_2.value * 51;
	play_status.eq_user_3 = ex__music_menu_cx.eq_user_3.value * 51;

	switch (i) 
	{
		case 0:
			/* ESCAPE key pressed, cacel all changes */
			break;

		case PLAY_TYPE + MIID_BASE_MUSIC:
			play_status.play_type = ex__music_menu_cx.play_type.value;
			break;

		case LOOP_TYPE + MIID_BASE_MUSIC:
			play_status.loop_type = ex__music_menu_cx.loop_type.value;

			if (play_status.loop_type == ALL_PLAY)
			{
				last_file = select_file(file_list, IN_ORDER, PRE_FILE, cur, fullname);
				TEST_INFO("start all play++++++++++++");
				TEST_VAL(last_file, %d);
				select_file(file_list, IN_ORDER, NEXT_FILE, last_file, fullname);
			}
			change_bmp_on_control(h_loop, bmpinfo_loop+play_status.loop_type);
			/* The changed parameters go into effect. */
			break;

		case OPERATOR + MIID_BASE_MUSIC:
			play_status.operator_index = ex__music_menu_cx.operator_id.value;
			dealwith_operat(play_status.operator_index);
			/* Display the file info. */
			break;
		case SPEED + MIID_BASE_MUSIC:
			play_status.speed = ex__music_menu_cx.speed.value;
			audioDecCmd(music_audio_player, ALTER_PLAY_SPEED, (UINT)(play_status.speed*2));
			break;
		case FADE_IN_OUT + MIID_BASE_MUSIC:
			play_status.is_fade = ex__music_menu_cx.is_fade.value;
			break;

		case AB_WAIT_TIME + MIID_BASE_MUSIC:
			play_status.ab_wait_time = ex__music_menu_cx.ab_wait_time.value;
			audioDecCmd(music_audio_player, SET_AB_WAIT_TIME, 
					(UINT)(play_status.ab_wait_time));

			break;
		case AB_COUNT + MIID_BASE_MUSIC:
			play_status.ab_count = ex__music_menu_cx.ab_count.value;
			audioDecCmd(music_audio_player, SET_AB_COUNT, 
					(UINT)(play_status.ab_count));
			break;
	} 
}



void compose_path_name(char* fullname, const char* path, const char* name)
{
	int i;

	bzero(fullname, MY_PATH_MAX);
	strcpy(fullname, path);

	i = strlen(path);
	fullname[i] = '/';

	strcpy(fullname+i+1, name);

#ifdef DEBUG
	printf("compose_path_name{{{\n");
	printf("\t%s + %s = %s\n", path, name, fullname);
	printf("}}}compose_path_name\n");
#endif
}



void divide_fullname(const char* fullname, char* path, char* name)
{
	char* cur = NULL;
	char* res = NULL;

	bzero(path, MY_PATH_MAX);
	bzero(name, NAME_MAX);

	cur = (char*)fullname;
	while (cur)
	{
		res = cur;
		cur = strchr(cur+1, '/');
	}
	
	//copy chars before the last '/' (not the '/')
	strncpy(path, fullname, (res-fullname));
	strcpy(name, res+1);
#ifdef DEBUG
	printf("divide_fullname{{{\n");
	printf("\t%s = %s + %s\n", fullname, path, name);
	printf("}}}divide_fullname\n");
#endif
}



static BOOL _music_filter(const char *path, const char* file_name)
{
	int i;
	char* a_filter_str[] = {".mp3",".ogg",  
		".wav",".asf", ".ape", ".wma", ".wmv"};

	const char* postfix = rindex(file_name, '.');
    if (postfix)
    {
        for (i=0; i < sizeof(a_filter_str)/sizeof(a_filter_str[0]); i++)
        {
            if (strcasecmp(postfix, a_filter_str[i]) == 0) return TRUE;
        }
    }

	return FALSE;
}


static BITMAP* bmp_file_icon;
static const BITMAP* _music_get_file_icon (const char *path, const char* file_name, DWORD mode)
{
	return bmp_file_icon;
}


static PMP_MEDIA_FILE_LIST* _music_show_files(HWND hwnd)
{
	PMP_DIR_SHOW_INFO dir_show_info [] = 
	{
        {"music", _(IDS_PMP_MUSIC_DIRNAME), NULL},
        {"video", _(IDS_PMP_VIDEO_DIR_ALIAS), NULL},
        {"ebook", _(IDS_PMP_EBOOK_DIRNAME), NULL},
        {"picture", _(IDS_PMP_PIC_DIR_ALIAS), NULL},
        {"recorder", _(IDS_PMP_RECORDER_DIRNAME), NULL},
        {"..", _(IDS_PMP_DOTDOT), NULL},
        {"*", "*", NULL},
        {NULL, NULL, NULL},
	};
	PMP_MEDIA_FILE_LIST* my_list;
  
    BITMAP* bmp_folder_icon = pmp_image_res_acquire_bitmap("/res/music/folder.png");
    bmp_file_icon = pmp_image_res_acquire_bitmap("/res/music/music_icon.png");

    dir_show_info[0].icon = bmp_folder_icon;
    dir_show_info[1].icon = bmp_folder_icon;
    dir_show_info[2].icon = bmp_folder_icon;
    dir_show_info[3].icon = bmp_folder_icon;
    dir_show_info[4].icon = bmp_folder_icon;
    dir_show_info[5].icon = bmp_folder_icon;
    dir_show_info[6].icon = bmp_folder_icon;

	my_list = pmp_select_media_files (hwnd, PMP_MEDIA_TOP_DIR, 
                    PMP_MEDIA_NAME_MUSIC, _music_filter, 
                    _music_get_file_icon, dir_show_info, 
                    PMP_BROWSER_MODE_LIST|PMP_BROWSER_MODE_MULTIPLE);

    printf("111111111111-<%s>\n", __FUNCTION__);

    if (bmp_folder_icon != NULL)
    {
        pmp_image_res_release_bitmap("/res/music/folder.png",bmp_folder_icon );
        bmp_folder_icon = NULL;
    }

    printf("222222222222-<%s>\n", __FUNCTION__);

    if (bmp_file_icon != NULL)
    {
        pmp_image_res_release_bitmap("/res/music/music_icon.png", bmp_file_icon);
        bmp_file_icon = NULL;
    }

    printf("333333333333-<%s>\n", __FUNCTION__);

	return my_list;
}
