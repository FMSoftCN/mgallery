bin_PROGRAMS = startpmp

startpmp_SOURCES = main.c \
                   music_play.c music_lrc.c music_menu.c \
                   music_menu.h pmp_music.h debug.h \
                   ebook_display.c ebook_read_file.c ebook_edit_impl.h \
				      ebook_list.h ebook_listmodel.h ebook_scrolled.h \
					  ebook_scrollview.h ebook_scrollview_impl.h ebook_text.h \
					  ebook_textedit_impl.h \
                   video.c video.h\
                   worm.c worm_level.h\
                   housekeeper.c housekeeper_map.c \
                   housekeeper_map.h housekeeper_data.h \
                   recorder.c \
                   russia.c   \
                   calendar.c monthcalendar.c monthcal.h monthcalendar_impl.h \
                   stopwatch.c \
                   picture_widget.c picture.c picture.h \
                   setup.c setup_proc.c setup_sys.c setup_timedate.c \
                   setup.h setup_menu.h \
                   calc.c calc_compute.c calc_btn_code.h calc_size.h calc_type.h \
                   radio.c     
if EVB
startpmp_LDADD = -lengine ../lib/libpmp.a
else
startpmp_LDADD = ../lib/libpmp.a 
endif

INCLUDES = -I../include
