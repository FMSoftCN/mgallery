#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef EVB
#include "act_encoder.h"
#else
/*
********************************************************************************
*                       linux213x                
*         actions encoder interface structure
*                (c) Copyright 2002-2003, Actions Co,Ld.                       
*                        All Right Reserved                               
*
* File   : act_encoder.h 
* by	 : cs/fw
* Version: 1> v1.00     first version     02.24.2006
********************************************************************************
*/
#ifndef ACT_ENCODER_H
#define 	ACT_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************公用的数据结构及常量***************************************/
/************常量定义,值域0x00~~~~0x30*************/
#define			SET_DISPLAY_AREA		0x01		//设置取景时的显示区域

#define			SET_CONTRAST			0x02		//设置对比度
#define			SET_WHITE_BALANCE		0x03		//设置白平衡
#define			SET_SPECIAL_EFFECT		0x04		//设置特殊效果
#define			SET_EXPOSURE_MODE		0x05		//设置曝光模式
#define			SET_RESOLUTION			0x06		//设置dc或dv的分辨率

#define			SET_SAVE_PATH			0x07		//设置保存路径

#define			SET_FILE_FMT			0x08		//设置当前的编码格式,为字符串,如"mp3"
#define			SET_AUDIO_FMT			0x08		//设置音频格式
#define			SET_VIDEO_FMT			0x08		//设置视频格式

#define			SET_AUDIO_PARAM			0x09		//设置音频格式
#define 		SET_VIDEO_PARAM			0x0a		//设置视频录音的格式

#define			SET_AUDIO_SOURCE		0x0b		//设置audio源
#define			SET_VIDEO_SOURCE		0x0c		//设置video源
#define			SET_VOLUME				0x0d		//设置录音的音量

#define			ZOOM					0x0e		//数码变焦,此命令的参数为变焦的级别
#define			MARK_A_TAG				0x0f		//给当前的录像或录音打上tag
	
#define			GET_STATUS				0x10		//获取当前的状态
#define			GET_DISK_INFO			0x11		//获取磁盘信息，让用户知道当前磁盘可录相的时间或可照得照片数

#define			START_CAPTURE			0x12		//开始取景
#define			STOP_CAPTURE			0x13		//停止取景

#define			YUV_CHANNEL_ON			0x14		//开启yuv通道
#define			YUV_CHANNEL_OFF			0x15		//关闭yuv通道

#define			START_MONITOR			0x16		//开始录音监听
#define			STOP_MONITOR			0x17		//停止监听

/***********数据结构定义*************************/
typedef struct{
		unsigned int xpos;
		unsigned int ypos;
		unsigned int width;
		unsigned int height;
}display_area_t;									//设置取景显示位置时的参数

typedef enum{ 
		AUTO_BALANCE, 
		INDOOR_BALANCE, 
		OUTDOOR_BALANCE, 
		FLUORE_BALANCE 
}white_balance_t;									//白平衡设置参数

typedef enum{
    	NONE_SPECIAL, 
		WHITE_AND_BLACK, 
		BROWN, 
		SOLARIZE
}special_effect_t;									//特殊效果设置参数

typedef enum{
		AUTO_EXPOSURE, 
		INDOOR_EXPOSURE	 
}exposure_mode_t;									//曝光模式设置参数

typedef struct{
		unsigned int space_left;					//磁盘剩余空间
		unsigned int info;							//对于录像与录音此参数为剩余可录时间，对于dc此参数为可照得照片
}disk_info_t;										//磁盘信息参数

typedef struct{
		unsigned int year;
		unsigned int month;
		unsigned int day;
}date_t;											//录音的日期

typedef enum{
		CAPTURING,
		MONITORING,
		RECORDING,
		STOPPED,
		PAUSED,		
		ERROR
}status_constant_t;									//当前状态常量定义

typedef enum{
		STEREO_AUDIO,
		JOINT_STEREO_AUDIO,
		DUAL_CHANNEL_AUDIO,
		MONO_AUDIO
}encode_mode_t;

typedef struct{
		char bpp;									//音频采样点的精度，如8bit,16bit,24bit//
		char channel;								//音频采样的通道数如:1 单声道；2 双声道		
		unsigned int sample_rate;					//采样频率如8000，16000，44100等
		unsigned int bitrate;						//编码的码率
		unsigned int encode_mode;					//音频编码的模式
}audio_param_t;

typedef struct{
		unsigned mic:1;
		unsigned fm:1;
		unsigned line_in:1;
		unsigned iis:1;
		unsigned spdif:1;		
		unsigned tv:1;
		unsigned reserved:26;		
}audio_resource_t;									//音源输入选择，当有多路bit同时为1时是混合输入

/**************************dv相关的常量、数据结构和函数接口********************************/
/***********常量定义,值域0x31~~~~0x50***********/
#define			VIDEO_TIME_ON		0x31			//在录像时需要输出当前时间
#define			VIDEO_TIME_OFF		0x32			//在录像时不需要输出当前时间

#define			START_VIDEO			0x34			//开始录像,此时需要传入当前需要保存的文件名;
#define			STOP_VIDEO			0x35			//停止录像
#define			PAUSE_VIDEO			0x36			//暂停录像

/***********数据结构定义***********************/
typedef enum{
		DV160X120,
		DV320X240
}dv_resolution_t;									//dv时的分辨率设置参数

typedef struct{
    unsigned int time;
    status_constant_t status;
    unsigned int err_no;
}dv_status_t;										//当前dv时的状态

/**********函数接口定义***********************/
void *videoEncOpen(void *param);
int videoEncCmd(void *handle,unsigned int cmd,unsigned int param);
int videoEncClose(void *handle,void *param);

/*************************dc相关的常量、数据结构和函数接口**********************************/
/**********常量定义,值域0x31~~~~0x50**********/
#define			SET_EXIF_INFO		0x31			//设置图片的exif信息,如:制造商等	
#define			PHOTO_DATE_ON		0x32			//给照片打上日期
#define			PHOTO_DATE_OFF		0x33			//不需要给照片打上日期
#define			SNAPSHOT			0x34			//拍照,此时需要传入当前需要保存的文件名(文件名+扩展名)

/**********数据结构定义**********************/
typedef enum{
		DC320X240,
		DC640X480,
		DC1024X768,
		DC1280X960,
		DC1600X1200,
		DC2048X1536
}dc_resolution_t;									//dc时的分辨率设置参数

typedef struct{
		char manufacture[10];
		char model[10];
}exif_info_t;										//exif设置信息

/**********函数接口定义***********************/
void *imageEncOpen(void *param);
int imageEncCmd(void *handle,unsigned int cmd,unsigned int param);
int imageEncClose(void *handle,void *param);

/*************************record相关的常量、数据结构和函数接口**********************************/
/**********常量定义,值域0x31~~0x50***********/
#define			SET_VOR_TIME		0x31			//静音检测时的参数，确定某段时间为静音时静音至少保持的时间
#define			SET_VOR_GAIN		0x32			//静音检测时的参数
#define			SET_RECORD_TIME		0x33			//设置当前需要录音的时间，时间到时自动关闭
#define			SET_ENERGY_LEVEL	0x34			//设置需要显示的能量总级别数

#define			SILENCE_DECTECT_ON	0x35			//打开静音检测的功能；返回的静音状态可作自动分区或自动暂停录音功能
#define			SILENCE_DECTECT_OFF	0x36			//关闭静音检测的功能

#define			START_RECORDING		0x37			//开始录音
#define			STOP_RECORDING		0x38			//停止录音
#define			PAUSE_RECORDING		0x39			//暂停录音

/**********数据结构定义**********************/
typedef struct{
    unsigned int time;								//当前的录音时间
    status_constant_t status;						//当前codec的状态
	unsigned char energy_level;						//输出当前实时能量级别
	unsigned char silence_flag;	 					//当前是否是静音，在静音检测功能开启时有效
    unsigned int err_no;							//在出错状态时返回的错误号
}record_status_t;									//当前dv时的状态

typedef struct{			
		unsigned int time;
		char buf[64];
}tag_info_t;										//标签信息

/**********函数接口定义***********************/
void *audioEncOpen(void *param);					
int audioEncCmd(void *handle,unsigned int cmd,unsigned int param);
int audioEncClose(void *handle,void *param);

/*****************************end of the file************************************************/
#ifdef __cplusplus
}
#endif

#endif

#endif
