#define WIDGET_PICTURE "widget_picture"


#define PIC_SET_FILE        MSG_USER+1
#define GET_FILEINFOR   MSG_USER+2
#define SLIDE_SHOW      MSG_USER+3
#define MOVE_PICTURE    MSG_USER+4
#define MATCH_SHOW      MSG_USER+5
#define ROTATE_PICTURE  MSG_USER+6
#define FLIP_PICTURE    MSG_USER+7
#define ZOOM_PICTURE    MSG_USER+8 
#define SET_SHOW_INFOR  MSG_USER+9 

#define SCALE_MAX      2000
#define SCALE_MIN      10 
typedef struct {
    char  FileType [8];
    int   Depth;
    int   Width;
    int   Height;
    int   Size;
}FileInfor;

typedef enum   {FlipN=0,
    FlipV=1<<0,
    FlipH=1<<1,
    FlipA=FlipH|FlipV
}FlipType;
extern int     RegisterWidgetPicture  (void);
extern void     UnregisterWidgetPicture(void);

