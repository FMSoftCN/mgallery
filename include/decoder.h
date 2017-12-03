#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef EVB
#include "act_decoder.h"
#else
/*
********************************************************************************
*                       linux213x                
*         actions decoder interface structure
*                (c) Copyright 2002-2003, Actions Co,Ld.                       
*                        All Right Reserved                               
*
* File   : act_decoder.h 
* by	 : cs/fw
* Version: 1> v1.00     first version     02.24.2006
********************************************************************************
*/
#ifndef ACT_DECODER_H
#define 	ACT_DECODER_H

#ifdef __cplusplus
extern "C" {
#endif

/**********************decoder公用的数据结构及常量*********************/
/*******常量定义，值域0x01~~~~0x30***********/
#define			PLAY					0x01
#define			STOP					0x02
#define			PAUSE					0x03
#define			CONTINUE				0x04
#define			SEEK					0x05

#define			FAST_FORWARD			0x06	
#define			FAST_BACKWARD			0x07
#define			CANCEL_FF				0x08
#define			CANCEL_FB				0x09

#define			SET_A					0x0a		
#define			SET_B					0x0b
#define			CLEAR_AB				0x0c
#define			SET_AB_COUNT			0x0d		//设置ab复读的次数
#define			SET_AB_WAIT_TIME		0x0e		//设置到达b点时需要等待的状态

#define			ALTER_PLAY_SPEED		0x0f	

#define			NEXT_FILE				0x10
#define			PRE_FILE				0x11

#define 		MARK_A_TAG				0x12

#define			GET_MEDIA_INFO			0x13
#define			GET_STATUS				0x14
//#define			GET_ERROR_NO			0x13	//获取错误id号,当检测到中间件出错时,可发送此命令获取错误代号

#define			SET_INDEX				0x15		//设置列表播放的index		
#define			SET_FILE				0x16		//在指定单曲播放时,为文件名(包含路径);
#define			SET_LOOP_MODE			0x17
#define			SET_FFB_STEP			0x18
#define			SET_VOLUME				0x19	

#define			GET_VOLUME				0x20

/******************数据结构定义******************/
typedef enum{
	    SEQUENCE,									//顺序
	    REPEAT_ONE,									//重复播放一首
	    REPEAT_ALL,									//列表重复播放
	    RANDOM,										//随机选择播放
}loop_mode_t;		

typedef struct{			
		unsigned int time;
		char buf[64];
}tag_info_t;										//标签信息

typedef enum{
		PLAYING,
		PAUSED,
		STOPPED,
		ERROR,
		SEARCHING									//在radio播放时用到此状态
}status_constant_t;									//当前decoder状态信息的常量

typedef enum{
		NORMAL_PLAY,								//正常播放				
		AB_RESTORE_PLAY,							//恢复上一次的AB播放状态
		BPT_RESTORE_PLAY,							//从上一次退出点开始播放
		TAG_PLAY									//TAG播放
}play_mode_t;										//启动时的播放模式	

typedef struct{
		play_mode_t mode;							//当前的播放模式
		unsigned int param;							//不同播放模式下的输入参数
}play_param_t;										//不同播放模式时的输入参数

/*******************与video decoder相关的常量、数据结构和函数接口*******************/
/*************常量定义，值域:0x31~~~~~0x50*****/
#define		SET_DISPLAY_AREA	0x31			//设置当前video的显示区域
#define		SET_TRACK_MODE		0x32			//双语功能选择
#define		YUV_CHANNEL_OFF		0x33			//关掉当前的视频输出
#define		YUV_CHANNEL_ON		0x34			//开启当前的视频输出
#define		FLUSH_SCREEN		0x35			//将当前的输出帧重新刷屏
#define		GET_NEW_FRAME		0x36			//将当前的输出帧重新刷屏
#define		VIDEO_TV_OUT		0x37			//将video电视输出
/*John Yi 2006.10.10*/
/*For ap gui demand, param is unsigned char* pointing to a buffer big enough
  to hold the rgb raw data*/
#define		PRINT_SCREEN		0x38			//catch current display and save it as rgb raw

/************数据结构定义*********************/
typedef struct{
		/*文件名*/			
		char filepath[256];
		char filename[256];
		unsigned int file_index;

		/*媒体信息*/
		unsigned int media_type;
		unsigned int total_time;
		unsigned int bitrate;		
		unsigned int sample_rate;					//音频有关信息
		unsigned int bpp;
		unsigned int channels;
		unsigned int frame_rate;					//视频有关信息
		unsigned int width;
		unsigned int height;
		unsigned int drm_flag;

		/*id3信息*/
		char id3_info[20];
}video_file_info_t;

typedef struct{
		unsigned int xpos;
		unsigned int ypos;
		unsigned int width;
		unsigned int height;
}display_area_t;

typedef enum{
		DOUBLE_TRACK,
    		LEFT_TRACK,
	    	RIGHT_TRACK
}select_track_t;

typedef struct{
		int file_index;							//当前播放歌曲的索引
		int cur_time;							//歌曲当前的播放时间
		status_constant_t status;				//codec当前的状态
		unsigned int ab_flag;
		int err_no;								//当前的出错状态
}video_status_t;								//decoder当前需要返回给ap的状态信息

/**********函数接口定义***********************/
void *videoDecOpen(void *param);
int videoDecCmd(void *handle,unsigned int cmd,unsigned int param);
int videoDecClose(void *handle,void *param);


/*******************与music player相关的常量、数据结构和函数接口*********************/
/*********常量定义，值域0x31~~~~~~0x50*******/
#define			FADE_IN				0x31
#define 		FADE_OUT			0x32
#define			SET_EQ				0x33
#define			SET_ENERGY_DOOR		0x34
#define			SET_ENERGY_STEP		0x35
#define			GET_SINGER_PHOTO	0x36		//获取歌曲的照片用于ap的显示//
#define			AUDIO_FM_OUT		0x37

/*********数据结构定义**********************/
typedef struct{
		/*文件名*/	
		char filepath[256];
		char filename[256];
		unsigned int file_index;

		/*媒体信息*/
		unsigned int total_time;
		unsigned int bitrate;		
		unsigned int sample_rate;			
		unsigned int channels;

		/*其他信息*/
		char drm_flag;							//是否是drm歌曲		
		char photo_flag;						//是否含有歌手的照片信息
		char license_info[50];					//歌曲的lincense信息

		/*id3信息*/
		void *id3_info;		
}music_file_info_t;

typedef enum{	
		NORMAL,
		ROCK,
		POP,
		CLASSIC,
		SOFT,
		JAZZ,
		DBB,
		SRS,
		WOW,
		TRUE_BASS,
		USR_MODE
}eq_mode_t;										//当前支持的eq模式

typedef struct{
		char eq_mode;			
		char param[3];							//在用户模式时用户需要设置 的参数
}eq_set_t;										//设置eq时的参数

typedef struct{
		unsigned fade_out_flag:1;				//当前是否在淡出状态
		unsigned silence_flag:1;				//当前是静音状态；
		unsigned AB_play_flag:1;				//当前处于ab播放状态
		unsigned reach_B_flag:1;				//当前已到达B点
		unsigned return_A_flag:1;				//在B处的等待时间已到，要返回到A点
}status_flag_t;									//这些状态flag均是只读数据

typedef struct{
		int file_index;								//当前播放歌曲的索引
		int cur_time;							//歌曲当前的播放时间		
		status_constant_t status;				//codec当前的状态		
		unsigned int err_no;
		status_flag_t flag;						//ap会用到的状态flag
		unsigned int cur_bps;					//当前歌曲的比特率信息
		unsigned int cur_power;					//当前输出的能量值
}music_status_t;								//decoder当前需要返回给ap的状态信息,这些状态flag均是只读数据


/*函数接口*/
void *audioDecOpen(void *param);
int audioDecCmd(void *handle,unsigned int cmd,unsigned int param);
int audioDecClose(void *handle,void *param);


/***********************与img decoder相关的常量、数据结构和函数接口定义**************/
/******常量定义，值域0x31~~~~~0x50********/		
#define			IMG_DECODE		0x31
#define			RESIZE			0x32
#define			MOVE			0x33
#define			ZOOM			0x34
#define			ROTATE			0x35
#define			MIRROR			0x36
#define			SAVE			0x37

#define			SET_CONTRAST	0x39	
#define			SET_BACKGROUND	0x3a	

#define			GET_ERR			0x3b

#define			IMG_TV_OUT		0x3c
#define			IMG_PRINT_OUT	0x3d

#define			YUV 			0x3e
#define			RGB 			0x3f

/*****数据结构定义************************/
typedef enum{
		ZOOMIN,    								//放大
		ZOOMOUT    								//缩小
}image_zoom_t;    								//图像缩放模式：放大or缩小 

typedef enum{
 		LEFT90,   								//顺时针旋转90度
		RIGHT90, 		 						//逆时针旋转90度	
		R180									//旋转180度	
}image_rotation_t;   							//旋转方向

typedef enum{
		UP_DOWN,    							//上下颠倒
		LEFT_RIGHT   							//左右颠倒（镜像）
}image_mirror_t;

typedef enum{
		UP,
		DOWN,
		LEFT,
		RIGHT
}image_move_direction_t;  						//移动图片

typedef struct{
		unsigned int year;
		unsigned int month;
		unsigned int day;
}img_date_t;

#define MAX_ASCII_LEN  100

typedef struct {
		
	char   make[MAX_ASCII_LEN];                  // manufactory name
	char   model[MAX_ASCII_LEN];                 // equitment mode
	char   software[MAX_ASCII_LEN];              // software note
	char   datetime[20];                         // phototaking date
	char   artist[MAX_ASCII_LEN];                // photo taker
	char   copyright[MAX_ASCII_LEN];             // mannufactory copyright note
	char   imageDescription[MAX_ASCII_LEN];      // photo description

	char   makernote[MAX_ASCII_LEN];            // maker noter
	char   exposuretime[MAX_ASCII_LEN];         // exposure time       
	char   F_number[MAX_ASCII_LEN];             // aperture
	char   exposureProg[MAX_ASCII_LEN];         // exposure Programme
	short  iso;                                 // iso
	char   componentConfig[5];                  // components description       
	char   shutterSpeed[MAX_ASCII_LEN];            // shutter speed 
	char   meterModel[MAX_ASCII_LEN];              // meter model
	char   lightSource[MAX_ASCII_LEN];
     
	char   flash[MAX_ASCII_LEN];     
	char   focalLength[MAX_ASCII_LEN] ;   
	long   width;                               // image width
	long   height;                              // image height
	char   exifVersion[4];                      // exif version

	char   GPSVersion[4];                       // GPS version
	char   GPSLatitudeRef[4];                   // GPS latitude reference. 'N' for North, 's' for South       
	char   GPSLatitude[MAX_ASCII_LEN];
	char   GPSLongtitudeRef[4];                 // GPS longtitude reference. 'E' for East, 'W' for West
	char   GPSLongtitude[MAX_ASCII_LEN];

} exif_info_t;

typedef struct{
		unsigned int file_index;
		char *file_name;
		unsigned int file_len;			
		unsigned int width;
		unsigned int height;
		img_date_t date;		
		exif_info_t *exif;
}img_file_info_t;

typedef struct{
		unsigned int bpp;
		unsigned int formate;
		unsigned int rect_width; 				//要求解码输出的图片的大小 
		unsigned int rect_height;			
		unsigned int img_width;					//解码输出图片的实际大小
		unsigned int img_height;
		unsigned char *buf;						//输出图片的buffer地址
		unsigned int len;					//buffer地址的长度
		unsigned int time_stamp;						
}img_info_t;	

/*******函数接口定义***************************/
void *imageDecOpen(void *param);
int imageDecCmd(void *handle,unsigned int cmd,unsigned int param);
int imageDecClose(void *handle,void *param);

/***********************与radio相关的常量、数据结构和函数接口定义**************/
/******常量定义，值域0x31~~~~~0x50********/	
#define SET_RADIO_MODE		0x31				//设置电台频段模式
#define SET_RADIO_FREQ		0x32    			//设置播放频率
#define SET_RADIO_INTENSITY 0x33				//设置电台的强度
#define GET_RADIO_INFO		0X34				//获取当前radio的一些状态信息
#define SEARCH				0x35				//搜索下一个电台频率

/*John Yi & Hou 2006.10.11*/
/* get radio's signal intensity, we can recognise whether there is a 
   radio station by it's value. */
#define GET_RADIO_INTENSITY		0x800	
/* set radio's info, param is type of (radio_info_t *) */
#define SET_RADIO_INFO		0x801

typedef enum{
		US = 0,    								
		JAPAN    								
}radio_mode_t;  

typedef struct{
		status_constant_t status;				//当前radio的状态
		unsigned int freq;     					//当前正在播放的电台频率
		unsigned int search_step;				//当前电台的搜索步长	
		unsigned int stereo_status;				//当前播放的立体声信息
		unsigned int cur_mode; 					//当前电台的频段模式
		unsigned int cur_volume;				//当前的音量值
		unsigned int intensity;         		//当前电台的强度
}radio_info_t;

/*******函数接口定义***************************/
void *radioOpen(void *param);
int radioCmd(void *handle,unsigned int cmd,unsigned int param);
int radioClose(void *handle,void *param); 
/***********************end of the file****************************************/
#ifdef __cplusplus
}
#endif

#endif

#endif
