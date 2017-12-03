#ifndef SETUP_H
#define SETUP_H

#include <minigui/common.h>

#define MAIN_OUT_NUM	5

#define LANG_ZH_CN 0
#define LANG_ZH_TW 1
#define LANG_EN_US 2


#define BKCOLOR_STATIC		COLOR_lightwhite
#define BKCOLOR_STATIC_SEL	COLOR_darkblue
#define FNCOLOR_STATIC		COLOR_black
#define FNCOLOR_STATIC_SEL	COLOR_green

#define EXIT_SET		0
#define CONTINUE_SET	1
///////////////////////////////////////////////////////
void add_volume(void);
void sub_volume(void);
int get_volume(void);
void out_volume(char*);
//////////////////////////////////////////////////////
void add_minute(void);
void sub_minute(void);
int get_minute(void);

void add_hour(void);
void sub_hour(void);
int get_hour(void);

void add_year(void);
void sub_year(void);
int get_year(void);

void add_month(void);
void sub_month(void);
int get_month(void);

void add_day(void);
void sub_day(void);
int get_day(void);
///////////////////////////////////////////////
void add_key_sound(void);
void sub_key_sound(void);
int get_key_sound(void);
void out_key_sound(char* buffer);

void add_bklight(void);
void sub_bklight(void);
int  get_bklight(void);
void out_bklight(char*);

void add_shutdown_time(void);
void sub_shutdown_time(void);
int  get_shutdown_time(void);
void out_shutdown_time(char*);

void add_language(void);
void sub_language(void);
int  get_language(void);
void out_language(char* );

/*
void set_bkpic(char* path);

void initial_bkcolor(void);
void add_bkcolor(void);
void sub_bkcolor(void);
int	 get_bkcolor(void);
void out_bkcolor(char *);
*/

void out_sysinfo(char* buffer, int buf_size);

extern int lang;
extern int background_index;

#endif
