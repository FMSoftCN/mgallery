/*
** $Id$
**
** calc_size.h: define the buttons layout of calc
**
** Copyright (C) 2006 Feynman Software.
**
** All right reserved by Feynman Software.
**
** Current maintainer: Wang Xuguang.
**
** Create date: 2006/11/17
*/


#define CALCWINDOW_LX        0//fh_mainwindow_lx
#define CALCWINDOW_TY        0//fh_mainwindow_ty
#define CALCWINDOW_RX        320//fh_mainwindow_rx
#define CALCWINDOW_BY        (240-28)//fh_mainwindow_by
#define CALCWINDOW_WIDTH     (CALCWINDOW_RX - CALCWINDOW_LX)
#define CALCWINDOW_HEIGHT    (CALCWINDOW_BY - CALCWINDOW_TY)

//the distance between bottons 
    #define BD                  19	//border distance 
    #define HD                  5 //H-distance
    #define HD_SC               5
    #define VD_SC               4  //V-distance
    #define VD                  6

//the size of bottons
    #define B_W                 42 //button width
    #define B_H                 26 //button height
    #define B_W_SC              42
    #define B_H_SC              20

//static box displaying result
    #define DISPLAY_T           (VD + 1)
    #define DISPLAY_R           (CALCWINDOW_WIDTH - BD - 2)
    #define DISPLAY_L           (DISPLAY_R - 180)
    #define DISPLAY_B           (16 + DISPLAY_T)

#define ARROW_LEFT              (BD)
#define ARROW_TOP               VD
#define ARROW_RIGHT             (ARROW_LEFT + 12)
#define ARROW_BOTTOM            (ARROW_TOP + 10)

#define BEGIN_X             (BD)
#define BEGIN_X_SC          (BD)


#define BEGIN_Y             (DISPLAY_B + 5)
#define BEGIN_Y_SC          (DISPLAY_B + 5)

//static box display "M" 
#define SM_W                13
#define SM_H                12
#define SM_X                (B_W + BD + 3)        
#define SM_Y                (VD)                

#define CHK_INV_W		 50	 
#define CHK_INV_H        17

#define CHK_INV_X        (CHK_HYP_X - HD - CHK_INV_W)
#define CHK_INV_Y        BEGIN_Y_SC

#define CHK_HYP_W        50
#define CHK_HYP_H        17
#define CHK_HYP_X        (CALCWINDOW_WIDTH - BD - CHK_HYP_W)
#define CHK_HYP_Y        BEGIN_Y_SC

#define BTN_ANG_W        (B_W_SC + 3)
#define BTN_ANG_H        B_H_SC
#define BTN_ANG_X        (CALCWINDOW_WIDTH - BD - 3 - BTN_ANG_W)
#define BTN_ANG_Y        (BEGIN_Y_SC + B_H_SC + VD)

#define BTN_BASE_W       BTN_ANG_W
#define BTN_BASE_H       B_H_SC
#define BTN_BASE_X       (BTN_ANG_X - BTN_BASE_W - HD + 1)
#define BTN_BASE_Y       BTN_ANG_Y


#define BTN_UNIT_W        86
#define BTN_UNIT_H        21
#define BTN_UNIT_X        (CALCWINDOW_WIDTH - BD - BTN_UNIT_W - BD - 1)
#define BTN_UNIT_Y        BEGIN_Y

