/*
** $Id$
**
** russia.c: The module of russia game for Feynman PMP solution.
**
** Copyright ( C ) 2007 Feynman Software, all rights reserved.
**
** Use of this source package is subject to specific license terms
** from Beijing Feynman Software Technology Co., Ltd.
**
** URL: http://www.minigui.com
**
** Current maintainer: Zhang Hua .
 */

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "pmp.h"

#ifdef PMP_APP_RUSSIA

#include <minigui/control.h>

#include "text_id.h"

static HWND hMainWnd = HWND_INVALID; 

#define RUSSIA_SCOREPATH        ( PMP_CFG_PATH "russia_score.conf" )
#define DIFFICULTY_HOP          5000
#define HIGHEST_SCORE           1000000
#define RUSSIA_SQUARE_TIMER1    600
#define RUSSIA_SQUARE_HEIGHT    22 
#define RUSSIA_SQUARE_WIDTH     16
#define RUSSIA_SQUARE_GRID      20

#define ACTION_INIT             0
#define ACTION_PUTIN            1
#define ACTION_LEFT             2
#define ACTION_RIGHT            3
#define ACTION_ROTATE           4
#define ACTION_DOWN             5

#define LINE_FULL               0
#define LINE_EMPTY              1
#define LINE_PARTLYFULL         2

/*The struct of the movingsquare*/
typedef struct { 
#define SQUARE_NOTHING          0
#define SQUARE_LONG             1 
#define SQUARE_SQUARE           2 
#define SQUARE_HILL             3
#define SQUARE_LEFT_DUCK        4
#define SQUARE_RIGHT_DUCK       5
#define SQUARE_LEFT_CHAIR       6
#define SQUARE_RIGHT_CHAIR      7
    int iType; 
#define DIR_NONE                0
#define DIR_UP                  1
#define DIR_DOWN                2
#define DIR_LEFT                3
#define DIR_RIGHT               4
    int iDir;
    /*first, get at a contour of the square, which is a rectangle,
    **then we get the cordinate of the point at the left up corner.
    **The coordinate of the origin
    */
    POINT poOrigin;
    /*The coordinate of the rightbottom*/
    POINT poRightBottom;
    /*The coorfinates of the four blocks*/
    POINT poAll[4];
 } MovingSquare;

/*The struct of the playingwindow*/
typedef struct {
    /*The rank of the game has 10 levels*/
    int iLevel;
    /*The score of the game*/
    int iScore;
    /*The movingsquare of the game*/
    MovingSquare * pMovingSquare;
    /*The precreatsquare of the game*/
    MovingSquare *pPreCreatedSquare;
    POINT poOrigin;
    int   iStateMatrix[RUSSIA_SQUARE_HEIGHT][RUSSIA_SQUARE_WIDTH];
    /*A param of the game is begin or not*/
    BOOL bBegin;
    /*A param of the game is dead or not*/
    BOOL bDead;
    /*A param of the game is pause or not*/
    BOOL bPause;
 } PlayingWindow; 

typedef struct {
    PlayingWindow *pPlayingWindow;
    /*initial difficulty, rank from 0 to 11; */
    int iDifficulty;
    int HighScore[5];
    char strEtcFilePath[MAX_PATH + 1];
    char strText[50];
    int  iTempStateMatrix[RUSSIA_SQUARE_HEIGHT][RUSSIA_SQUARE_WIDTH];
    BOOL bTease;
    BOOL bBusy;
 } RussiaSquare;

RussiaSquare theApp;
static RECT rect_condition = { 180, 25, 280, 45 };
static RECT rect_next = { 180, 45, 280, 80 };
static RECT rect_score = { 180, 75, 280, 130 };
static int TimerFreq[12] = { 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60 };
static int TestMyWinProc ( HWND hWnd, int message, WPARAM wParam, LPARAM lParam ); 
static void _russia_init_create_info ( PMAINWINCREATE pCreateInfo );
static int LineFullEmpty ( int *pLine );
static void OnCommandNewOne ( HWND hWnd );
static void OnClose ( HWND hWnd );
static void OnDraw ( HWND hWnd, HDC hDC );
static void DrawPlayingWindow ( HDC hDC, HWND hWnd, POINT poStart, \
        int iSideLength, PlayingWindow * pPlayingWindow );
static BOOL OnLeftKeyDown ( HWND hWnd, PlayingWindow *pPlayingWindow, int iWhich );
static BOOL OnRightKeyDown ( HWND hWnd, PlayingWindow *pPlayingWindow, int iWhich );
static BOOL OnRotateKeyDown ( HWND hWnd, PlayingWindow *pPlayingWindow, int iWhich );
static BOOL OnDownKeyDown ( HWND hWnd, PlayingWindow *pPlayingWindow, int iWhich );
static BOOL OnStartKeyDown ( HWND hWnd, PlayingWindow *pPlayingWindow, int iWhich );
static void OnTimer ( HWND hWnd, PlayingWindow *pPlayingWindow, int iWhich );
static BOOL MoveSquare ( PlayingWindow * pPlayingWindow, int iDir );
static BOOL MoveSquareLong ( PlayingWindow * pPlayingWindow, int iAction );
static BOOL MoveSquareSquare ( PlayingWindow * pPlayingWindow, int iAction );
static BOOL MoveSquareLeftDuck ( PlayingWindow * pPlayingWindow, int iAction );
static BOOL MoveSquareRightDuck ( PlayingWindow * pPlayingWindow, int iAction );
static BOOL MoveSquareHill ( PlayingWindow * pPlayingWindow, int iAction );
static BOOL MoveSquareLeftChair ( PlayingWindow * pPlayingWindow, int iAction );
static BOOL MoveSquareRightChair ( PlayingWindow * pPlayingWindow, int iAction );
static int  PutSquare ( PlayingWindow *pPlayingWindow, int *y, int *x );
static BOOL WillNotTouch ( PlayingWindow *pPlayingWindow );
static BOOL WillNotTouchLong ( PlayingWindow *pPlayingWindow );
static BOOL WillNotTouchSquare ( PlayingWindow *pPlayingWindow );
static BOOL WillNotTouchLeftDuck ( PlayingWindow *pPlayingWindow );
static BOOL WillNotTouchRightDuck ( PlayingWindow *pPlayingWindow );
static BOOL WillNotTouchHill ( PlayingWindow *pPlayingWindow );
static BOOL WillNotTouchLeftChair ( PlayingWindow *pPlayingWindow );
static BOOL WillNotTouchRightChair ( PlayingWindow *pPlayingWindow );
static int  RemoveFullLines ( PlayingWindow * pPlayingWindow, int iWhich );
static void ConvertRectCord ( HWND hWnd, int iWhich, RECT * pRect );
static void DrawLittleBrick ( HDC hDC, HWND hWnd, POINT poStart, int iSideLength, int iType );
static BOOL TestPosition ( PlayingWindow *pPlayingWindow , int i, int j );
static void UpdateHighScore ( HWND hWnd, PlayingWindow *pPlayingWindow );
static BOOL IncrementScore ( PlayingWindow *pPlayingWindow, int iLines );
static BOOL TeaseTheOtherPlayer ( int iWhich, int iBegin, int iLines );

/*when the player key the F2 down, then the lose_focuse will be used.*/
BOOL lose_focuse = 0;
typedef struct _RUSSIA_MENU_PARAMS {
    int level;
 } RUSSIA_MENU_PARAMS;
static int russia_menu_item_text_id [] = {
    IDS_PMP_RUSSIA_START,
    IDS_PMP_RUSSIA_SET_LEVEL,
    IDS_PMP_RUSSIA_SCORE_INFO
 }; 

RUSSIA_MENU_PARAMS russia_menu_params = { 1 }; 

static int _russia_menu_start_callback ( void *context, int id, int op, char *text_buff )
{
    switch ( op ) {

        case PMP_MI_OP_DEF:
            sprintf ( text_buff, _( russia_menu_item_text_id[0] ) );
            return PMP_MI_ST_UPDATED;

        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;
    }
    return PMP_MI_ST_NONE;
 }

static int _russia_menu_setlevel_callback ( void *context, int id, int op, char *text_buff )
{
    RUSSIA_MENU_PARAMS* my_params = ( RUSSIA_MENU_PARAMS* )context;

    switch ( op ) {

        case PMP_MI_OP_DEF:
            sprintf ( text_buff, _( russia_menu_item_text_id[1] ), my_params->level );
            return PMP_MI_ST_UPDATED;

        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;

        case PMP_MI_OP_PREV:
            if ( my_params->level > 1 )
                my_params->level--;
            sprintf ( text_buff, _( russia_menu_item_text_id[1] ), my_params->level );
            return PMP_MI_ST_UPDATED;

        case PMP_MI_OP_NEXT:
            if ( my_params->level < 10 )
                my_params->level++;
            sprintf ( text_buff, _( russia_menu_item_text_id[1] ), my_params->level );
            return PMP_MI_ST_UPDATED;
    }
    return PMP_MI_ST_NONE;
 }

static int _russia_menu_highscore_callback ( void *context, int id, int op, char *text_buff )
{
    switch ( op ) {
        case PMP_MI_OP_DEF:
            strcpy ( text_buff, _( russia_menu_item_text_id[2] ) );
            return PMP_MI_ST_UPDATED;

        case PMP_MI_OP_ENTER:
            return PMP_MI_ST_SELECTED;
    }

    return PMP_MI_ST_NONE;
 }

static PMP_MENU_ITEM russia_menu[] =
{
    { 1, _russia_menu_start_callback },
    { 2, _russia_menu_setlevel_callback },
    { 3, _russia_menu_highscore_callback },
    { 0, NULL }
};

/*if there is only one persion play the game then the window will*/
static void OnCommandNewOne ( HWND hWnd )
{
    PlayingWindow * pPlayingWindow;
    theApp.pPlayingWindow = ( PlayingWindow* )calloc ( 1, sizeof ( PlayingWindow ) );
    pPlayingWindow = theApp.pPlayingWindow;
    pPlayingWindow->iLevel = theApp.iDifficulty - 1;

#ifdef _TIMER_UNIT_10MS
    SetTimer ( hWnd, RUSSIA_SQUARE_TIMER1, 1000/TimerFreq[pPlayingWindow->iLevel] );
#else
    SetTimer ( hWnd, RUSSIA_SQUARE_TIMER1, TimerFreq[pPlayingWindow->iLevel] );
#endif

}

/*This founction has draw the start window*/
static void OnDraw ( HWND hWnd, HDC hDC )
{ 
    POINT poStart;
    int   iSideLength;
    RECT rect;

    GetClientRect ( hWnd, &rect );
    SetBkColor ( hDC, COLOR_black );
    SetTextColor ( hDC, COLOR_green );
    SetPenColor ( hDC, COLOR_green );
    /*The length of the little square.*/
    iSideLength = 8;
    assert ( iSideLength > 0 );
    if ( theApp.pPlayingWindow ) {
        poStart.x = rect.left + 45;
        poStart.y = rect.top + 10;
        DrawPlayingWindow ( hDC, hWnd, poStart, iSideLength, theApp.pPlayingWindow );
    }
}

static void OnClose ( HWND hWnd )
{
    if ( !hWnd ) return;
    russia_menu_params.level = 1;
    /*saving the high score, keys arrange...*/
    DestroyMainWindow ( hWnd );
    MainWindowCleanup ( hWnd );
    /*ShowWindow ( hMainWnd, SW_SHOWNORMAL );*/
    pmp_app_notify_closed ( PMP_APP_RUSSIA );
}

/*Draw the playing window*/
static void DrawPlayingWindow ( HDC hDC, HWND hWnd, POINT poStart, \
        int iSideLength, PlayingWindow * pPlayingWindow )
{
    RECT rect;
    char strText[50];
    int i, j;
    MovingSquare *pMovingSquare;
    BOOL bMeetBrick;
    POINT poStart1;

    assert ( hDC );
    assert ( pPlayingWindow );
    /*the rect of the frame*/
    rect.left = poStart.x-4;
    rect.top  = poStart.y;
    rect.right = poStart.x + iSideLength*RUSSIA_SQUARE_WIDTH + 6 + 92;
    rect.bottom = poStart.y + iSideLength*RUSSIA_SQUARE_HEIGHT + 4 + 4;
    Draw3DBorder ( hDC, rect.left, rect.top, rect.right, rect.bottom );
    rect.left += 2;
    rect.top += 2;
    rect.right -= ( 92 + 2 + 2 );
    rect.bottom -= 2;
    Draw3DBorder ( hDC, rect.left, rect.top, rect.right, rect.bottom );
    rect.left += iSideLength*RUSSIA_SQUARE_WIDTH + 4;
    rect.top  = rect.top;
    rect.right += 2 + 92 ;
    rect.bottom = rect.bottom;
    Draw3DBorder ( hDC, rect.left, rect.top, rect.right, rect.bottom );

    /*the score of the game*/
    snprintf ( strText, 50, _( IDS_PMP_RUSSIA_SCORE ), pPlayingWindow->iScore );
    TextOut ( hDC, poStart.x + 135, poStart.y + 75, strText );
    snprintf ( strText, 50, _( IDS_PMP_RUSSIA_LEVEL ), russia_menu_params.level );
    TextOut ( hDC, poStart.x + 135, poStart.y + 95, strText );
    if ( !pPlayingWindow->bBegin && pPlayingWindow->bDead == FALSE && pPlayingWindow->bPause == FALSE ){
        TextOut ( hDC, poStart.x + 135, poStart.y + 15, _( IDS_PMP_RUSSIA_START ) );
        return;
    } else if ( pPlayingWindow->bDead  == TRUE ){
        TextOut ( hDC, poStart.x + 135, poStart.y + 15, _( IDS_PMP_RUSSIA_DEAD ) );
        return;
    }else if ( pPlayingWindow->bPause == TRUE ){
        TextOut ( hDC, poStart.x + 135, poStart.y + 15, _( IDS_PMP_RUSSIA_PAUSED ) );
        return;
    }    poStart.y += ( RUSSIA_SQUARE_GRID +  20 );

    pMovingSquare = pPlayingWindow->pPreCreatedSquare;
    if ( pMovingSquare ){
        poStart1.x = poStart.x +  (  21 + pMovingSquare->poAll[0].x )*iSideLength;
        poStart1.y = poStart.y +  (  0 + pMovingSquare->poAll[0].y )*iSideLength;
        DrawLittleBrick ( hDC, hWnd, poStart1, iSideLength, pMovingSquare->iType );
        poStart1.x = poStart.x +  ( 21 +  pMovingSquare->poAll[1].x )*iSideLength;
        poStart1.y = poStart.y +  ( 0 + pMovingSquare->poAll[1].y )*iSideLength;
        DrawLittleBrick ( hDC, hWnd, poStart1, iSideLength, pMovingSquare->iType );
        poStart1.x = poStart.x +  (  21 + pMovingSquare->poAll[2].x )*iSideLength;
        poStart1.y = poStart.y +  ( 0 +  pMovingSquare->poAll[2].y )*iSideLength;
        DrawLittleBrick ( hDC, hWnd, poStart1, iSideLength, pMovingSquare->iType );
        poStart1.x = poStart.x +  (  21 + pMovingSquare->poAll[3].x )*iSideLength;
        poStart1.y = poStart.y +  ( 0 + pMovingSquare->poAll[3].y )*iSideLength;
        DrawLittleBrick ( hDC, hWnd, poStart1, iSideLength, pMovingSquare->iType );
    }

    poStart.y -=  ( RUSSIA_SQUARE_GRID + 15 );
    for ( i = RUSSIA_SQUARE_HEIGHT-1; i >= 0; i-- )
    {
        bMeetBrick = FALSE;
        for ( j = 0; j < RUSSIA_SQUARE_WIDTH; j++ )
            if ( pPlayingWindow->iStateMatrix[i][j] )
            {
                poStart1.x  = poStart.x + j*iSideLength;
                poStart1.y  = poStart.y + i*iSideLength; 
                DrawLittleBrick ( hDC, hWnd, poStart1, iSideLength, \
                        pPlayingWindow->iStateMatrix[i][j] );
                bMeetBrick = TRUE;
            }
    }
    pMovingSquare = pPlayingWindow->pMovingSquare;
    if ( pMovingSquare ){
        poStart1.x = poStart.x +  ( pMovingSquare->poOrigin.x +
                pMovingSquare->poAll[0].x )*iSideLength;
        poStart1.y = poStart.y +  ( pMovingSquare->poOrigin.y +
                pMovingSquare->poAll[0].y )*iSideLength;
        DrawLittleBrick ( hDC, hWnd, poStart1, iSideLength, pMovingSquare->iType );
        poStart1.x = poStart.x +  ( pMovingSquare->poOrigin.x +
                pMovingSquare->poAll[1].x )*iSideLength;
        poStart1.y = poStart.y +  ( pMovingSquare->poOrigin.y +
                pMovingSquare->poAll[1].y )*iSideLength;
        DrawLittleBrick ( hDC, hWnd, poStart1, iSideLength, pMovingSquare->iType );
        poStart1.x = poStart.x +  ( pMovingSquare->poOrigin.x +
                pMovingSquare->poAll[2].x )*iSideLength;
        poStart1.y = poStart.y +  ( pMovingSquare->poOrigin.y +
                pMovingSquare->poAll[2].y )*iSideLength;
        DrawLittleBrick ( hDC, hWnd, poStart1, iSideLength, pMovingSquare->iType );
        poStart1.x = poStart.x +  ( pMovingSquare->poOrigin.x +
                pMovingSquare->poAll[3].x )*iSideLength;
        poStart1.y = poStart.y +  ( pMovingSquare->poOrigin.y +
                pMovingSquare->poAll[3].y )*iSideLength;
        DrawLittleBrick ( hDC, hWnd, poStart1, iSideLength, pMovingSquare->iType );
    }
 }

static void DrawLittleBrick ( HDC hDC, HWND hWnd, POINT poStart, int iSideLength, int iType )
{
    gal_pixel iColor;
    iColor = COLOR_green;
    switch ( iType ) {
        case SQUARE_LONG:
            iColor = COLOR_green; 
            break; 
        case SQUARE_SQUARE: 
            iColor = COLOR_lightgray; 
            break; 
        case SQUARE_HILL:
            iColor = COLOR_red; 
            break; 
        case SQUARE_LEFT_DUCK:
            iColor = COLOR_yellow; 
            break; 
        case SQUARE_RIGHT_DUCK:
            iColor = COLOR_blue; 
            break; 
        case SQUARE_LEFT_CHAIR:
            iColor = COLOR_magenta; 
            break; 
        case SQUARE_RIGHT_CHAIR:
            iColor = COLOR_cyan; 
            break; 
    }

    Draw3DUpThickFrame ( hDC, poStart.x, poStart.y, poStart.x + iSideLength, 
            poStart.y + iSideLength, iColor ); 
 }

static BOOL OnLeftKeyDown ( HWND hWnd, PlayingWindow *pPlayingWindow, int iWhich )
{ 
    RECT rect1; 
    assert ( hWnd ); 

    if ( !pPlayingWindow ) return FALSE;  
    if ( !pPlayingWindow->bBegin ) return FALSE; 
    if ( pPlayingWindow->bPause ) return FALSE; 
    if ( pPlayingWindow->bDead ) return FALSE; 
    if ( MoveSquare ( pPlayingWindow, ACTION_LEFT ) ){ 
        rect1.left = pPlayingWindow->pMovingSquare->poOrigin.x; 
        rect1.top  = pPlayingWindow->pMovingSquare->poOrigin.y; 	
        rect1.right = rect1.left +  pPlayingWindow->pMovingSquare->poRightBottom.x + 2; 
        rect1.bottom = rect1.top +  pPlayingWindow->pMovingSquare->poRightBottom.y + 1; 
        ConvertRectCord ( hWnd, iWhich, &rect1 ); 
        InvalidateRect ( hWnd, &rect1, TRUE ); 
    }
    else
        return FALSE; 
    return TRUE; 
 }

static BOOL OnRightKeyDown ( HWND hWnd, PlayingWindow *pPlayingWindow, int iWhich )
{ 
    RECT rect1; 
    assert ( hWnd ); 
    if ( !pPlayingWindow ) return FALSE;
    if ( !pPlayingWindow->bBegin ) return 0;
    if ( pPlayingWindow->bPause ) return 0;
    if ( pPlayingWindow->bDead ) return 0;
    if ( MoveSquare ( pPlayingWindow, ACTION_RIGHT ) ){
        rect1.left = pPlayingWindow->pMovingSquare->poOrigin.x-1;
        rect1.top  = pPlayingWindow->pMovingSquare->poOrigin.y;
        rect1.right = rect1.left +  pPlayingWindow->pMovingSquare->poRightBottom.x + 2;
        rect1.bottom = rect1.top +  pPlayingWindow->pMovingSquare->poRightBottom.y + 1;
        ConvertRectCord ( hWnd, iWhich, &rect1 );
        InvalidateRect ( hWnd, &rect1, TRUE );
    }
    else
        return FALSE;
    return TRUE;
 }

static BOOL OnRotateKeyDown ( HWND hWnd, PlayingWindow *pPlayingWindow, int iWhich )
{
    RECT rect1;
    assert ( hWnd );

    if ( !pPlayingWindow ) return FALSE;
    if ( !pPlayingWindow->bBegin ) return 0;
    if ( pPlayingWindow->bPause ) return 0;
    if ( pPlayingWindow->bDead ) return 0;
    if ( MoveSquare ( pPlayingWindow, ACTION_ROTATE ) ){
        rect1.left = pPlayingWindow->pMovingSquare->poOrigin.x-1;
        rect1.top  = pPlayingWindow->pMovingSquare->poOrigin.y-1;
        rect1.right = rect1.left +  5;
        rect1.bottom = rect1.top +  5;
        ConvertRectCord ( hWnd, iWhich, &rect1 );
        InvalidateRect ( hWnd, &rect1, TRUE );
    }
    else
        return FALSE;
    return TRUE;
 }
static BOOL OnDownKeyDown ( HWND hWnd, PlayingWindow *pPlayingWindow, int iWhich )
{
    RECT rect1;
    int iScoreBefore;
    int iScoreAfter;
    int iLines;
    int y,x;
    assert ( hWnd );

    if ( !pPlayingWindow ) return FALSE;
    if ( !pPlayingWindow->bBegin ) return 0;
    if ( pPlayingWindow->bPause ) return 0;
    if ( pPlayingWindow->bDead ) return 0;
    MoveSquare ( pPlayingWindow, ACTION_DOWN );
    PutSquare ( pPlayingWindow, &y,&x );
    iScoreBefore = ( pPlayingWindow->iScore )%DIFFICULTY_HOP;
    iLines = RemoveFullLines ( pPlayingWindow, iWhich );
    if ( iLines ){
        iScoreAfter = ( pPlayingWindow->iScore )%DIFFICULTY_HOP;
        if ( iScoreBefore > iScoreAfter ){
            pPlayingWindow->iLevel = ( pPlayingWindow->iLevel +  1 );
            if ( pPlayingWindow->iLevel > 9 )
                pPlayingWindow->iLevel = 9;
        }
        rect1.left = 0;
        rect1.right = 16;
        rect1.top = 0;
        rect1.bottom = y + 1;
        ConvertRectCord ( hWnd, iWhich, &rect1 );
        InvalidateRect ( hWnd, &rect1, TRUE );
    }
    else
    {
        rect1.left = x - 2;
        rect1.right = x + 4;
        rect1.top = 0;
        rect1.bottom = y + 1;
        ConvertRectCord ( hWnd, iWhich, &rect1 );
        InvalidateRect ( hWnd, &rect1, TRUE );
    }
    InvalidateRect ( hWnd, &rect_score, TRUE );
    return TRUE;
 }

static BOOL OnStartKeyDown ( HWND hWnd, PlayingWindow *pPlayingWindow, int iWhich )
{
    int i, j;
    assert ( hWnd );

    if ( !pPlayingWindow ) return FALSE;
    if ( pPlayingWindow->bPause ) return FALSE;
    if ( theApp.bTease )
        return 0;

    /*reset the struct : pMoingSquare....*/
    if ( pPlayingWindow->pMovingSquare ){
        free ( pPlayingWindow->pMovingSquare );
        pPlayingWindow->pMovingSquare = NULL;
        free ( pPlayingWindow->pPreCreatedSquare );
        pPlayingWindow->pPreCreatedSquare = NULL;
    }
    for ( i = 0; i < RUSSIA_SQUARE_HEIGHT; i++ )
        for ( j = 0; j < RUSSIA_SQUARE_WIDTH; j++ )
            pPlayingWindow->iStateMatrix[i][j] = 0;

    pPlayingWindow->iScore = 0;
    pPlayingWindow->bBegin = TRUE;
    pPlayingWindow->bDead  = FALSE;
    return TRUE;
 }


static void OnTimer ( HWND hWnd, PlayingWindow *pPlayingWindow, int iWhich )
{
    RECT rect1;
    int iScoreBefore;
    int iScoreAfter;
    int iLines;
    int y,x;
    MovingSquare * pMovingSquare ;
    assert ( hWnd );

    y = 22;
    assert ( pPlayingWindow );
    if ( !pPlayingWindow->bBegin ) return;
    if ( pPlayingWindow->bPause ) return;	
    if ( pPlayingWindow->bDead ) return;
    /*if no moving square, create one.*/
    if ( !pPlayingWindow->pMovingSquare ){
        if ( !pPlayingWindow->pPreCreatedSquare ){
            pPlayingWindow->pPreCreatedSquare =
                ( MovingSquare* )calloc ( 1, sizeof ( MovingSquare ) );
            pMovingSquare = pPlayingWindow->pPreCreatedSquare;
            srandom ( time ( NULL ) + iWhich );
            pMovingSquare->iType = ( rand (  ) + 2 + iWhich )% 7  +  1;

            pMovingSquare->iDir  = ( rand (  ) + 2 + iWhich )% 4  +  1;
            pMovingSquare->poOrigin.y = 0;
            pMovingSquare->poOrigin.x = ( rand (  ) + 2 + iWhich ) % ( RUSSIA_SQUARE_WIDTH-3 );
            MoveSquare ( pPlayingWindow, ACTION_INIT );

        }

        pPlayingWindow->pMovingSquare = pPlayingWindow->pPreCreatedSquare;

        pPlayingWindow->pPreCreatedSquare =
            ( MovingSquare* )calloc ( 1, sizeof ( MovingSquare ) );
        pMovingSquare = pPlayingWindow->pPreCreatedSquare;
        srandom ( time ( NULL ) + iWhich );
        pMovingSquare->iType = ( rand (  ) + iWhich ) % 7 +  1;

        pMovingSquare->iDir  = ( rand (  ) + iWhich ) % 4 +  1;
        pMovingSquare->poOrigin.y = 0;
        pMovingSquare->poOrigin.x = ( rand (  ) + iWhich ) % ( RUSSIA_SQUARE_WIDTH-3 );
        MoveSquare ( pPlayingWindow, ACTION_INIT );
        if ( !MoveSquare ( pPlayingWindow, ACTION_PUTIN ) ){
            /*No where to put the created square, die.*/
            pPlayingWindow->bDead = TRUE;
            UpdateHighScore ( hWnd, pPlayingWindow );
            InvalidateRect ( hWnd, &rect_condition, TRUE );
            return;
        }
        if ( WillNotTouch ( pPlayingWindow ) ){
            rect1.left = pMovingSquare->poOrigin.x - 2;
            rect1.right = pMovingSquare->poOrigin.x + 4;
            rect1.top = pMovingSquare->poOrigin.y - 4;
            rect1.bottom = pMovingSquare->poOrigin.y + 4;
            ConvertRectCord ( hWnd, iWhich, &rect1 );
            InvalidateRect ( hWnd, &rect1, TRUE );
            InvalidateRect ( hWnd, &rect_next, TRUE );
            return;
        }
        /*Touch the bottom!! put it in the square.*/
        PutSquare ( pPlayingWindow, &y ,&x);
        /*Can remove at least one line, live.....*/

        /*can not, die....*/
        pPlayingWindow->bDead = TRUE;
        UpdateHighScore ( hWnd, pPlayingWindow );
        InvalidateRect ( hWnd, &rect_condition, TRUE );
        return;
    }
    /*
    ** if there exist one, move down one step
    ** possible case:
    ** 1.simple move down
    ** 2.move down and find touch one brick.kill the moving square
    */
    if ( WillNotTouch ( pPlayingWindow ) ){
        /*simple move down*/
        pMovingSquare = pPlayingWindow->pMovingSquare;
        pMovingSquare->poOrigin.y += 1;
        rect1.left = pMovingSquare->poOrigin.x;
        rect1.top = pMovingSquare->poOrigin.y-1;
        rect1.right = rect1.left + pMovingSquare->poRightBottom.x + 1;
        rect1.bottom = rect1.top + pMovingSquare->poRightBottom.y + 2;
        ConvertRectCord ( hWnd, iWhich, &rect1 );
        InvalidateRect ( hWnd, &rect1, TRUE );
        return;
    }
    /*Touch the bottom!! put it in the square.*/
    PutSquare ( pPlayingWindow, &y ,&x);
    iScoreBefore = ( pPlayingWindow->iScore )%DIFFICULTY_HOP;
    iLines = RemoveFullLines ( pPlayingWindow, iWhich );
    if ( iLines ){
        iScoreAfter = ( pPlayingWindow->iScore )%DIFFICULTY_HOP;
        if ( iScoreBefore > iScoreAfter ){
            pPlayingWindow->iLevel = ( pPlayingWindow->iLevel +  1 );
            if ( pPlayingWindow->iLevel > 9 )
                pPlayingWindow->iLevel = 9;
        }
        rect1.left = 0;
        rect1.right = 16;
        rect1.top = 0;
        rect1.bottom = y + 1;
        ConvertRectCord ( hWnd, iWhich, &rect1 );
        InvalidateRect ( hWnd, &rect1, TRUE );
    }
    else
    {
        rect1.left = x - 2;
        rect1.right = x + 4;
        rect1.top = 0;
        rect1.bottom = y + 1;
        ConvertRectCord ( hWnd, iWhich, &rect1 );
        InvalidateRect ( hWnd, &rect1, TRUE );
    }
    InvalidateRect ( hWnd, &rect_score, TRUE );
    return;
 }

/*
** first: Decide wheather can we move.
** if can , then
** second: Move the square.
*/
static BOOL MoveSquareLong ( PlayingWindow * pPlayingWindow, int iAction )
{
    MovingSquare *pMovingSquare;
    POINT *pPoint;

    if ( iAction == ACTION_INIT )
        pMovingSquare = pPlayingWindow->pPreCreatedSquare;
    else
        pMovingSquare = pPlayingWindow->pMovingSquare;
    pPoint = &pMovingSquare->poOrigin;

    switch ( pMovingSquare->iDir ){
        case DIR_UP:
        case DIR_DOWN:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare = pPlayingWindow->pPreCreatedSquare;
                    pMovingSquare->poRightBottom.y = 3;
                    pMovingSquare->poRightBottom.x = 0;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 0;
                    pMovingSquare->poAll[2].y = 2;
                    pMovingSquare->poAll[3].x = 0;
                    pMovingSquare->poAll[3].y = 3;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x-1 ) )
                        return FALSE;

                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x + 1 ) )
                        return FALSE;

                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x-1 ) )
                        return FALSE;

                    pPoint->x -= 1;
                    pPoint->y += 1;
                    pMovingSquare->iDir = DIR_LEFT;
                    pMovingSquare->poRightBottom.y = 0;
                    pMovingSquare->poRightBottom.x = 3;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 1;
                    pMovingSquare->poAll[1].y = 0;
                    pMovingSquare->poAll[2].x = 2;
                    pMovingSquare->poAll[2].y = 0;
                    pMovingSquare->poAll[3].x = 3;
                    pMovingSquare->poAll[3].y = 0;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 4, pPoint->x ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
        case DIR_LEFT:
        case DIR_RIGHT:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare = pPlayingWindow->pPreCreatedSquare;
                    pMovingSquare->poRightBottom.y = 0;
                    pMovingSquare->poRightBottom.x = 3;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 1;
                    pMovingSquare->poAll[1].y = 0;
                    pMovingSquare->poAll[2].x = 2;
                    pMovingSquare->poAll[2].y = 0;
                    pMovingSquare->poAll[3].x = 3;
                    pMovingSquare->poAll[3].y = 0;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 3 ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x-1 ) )
                        return FALSE;
                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 4 ) )
                        return FALSE;
                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y-1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y-1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 3 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 3 ) )
                        return FALSE;

                    pPoint->x += 1;
                    pPoint->y -= 1;

                    pMovingSquare->iDir = DIR_UP;

                    pMovingSquare->poRightBottom.y = 3;
                    pMovingSquare->poRightBottom.x = 0;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 0;
                    pMovingSquare->poAll[2].y = 2;
                    pMovingSquare->poAll[3].x = 0;
                    pMovingSquare->poAll[3].y = 3;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 3 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
    }
    return TRUE;
 }

static BOOL MoveSquareSquare ( PlayingWindow * pPlayingWindow, int iAction )
{
    MovingSquare *pMovingSquare;
    POINT *pPoint;

    if ( iAction == ACTION_INIT )
        pMovingSquare = pPlayingWindow->pPreCreatedSquare;
    else
        pMovingSquare = pPlayingWindow->pMovingSquare;
    pPoint = &pMovingSquare->poOrigin;
    switch ( iAction ){
        case ACTION_INIT:
            pMovingSquare->poRightBottom.y = 1;
            pMovingSquare->poRightBottom.x = 1;
            pMovingSquare->poAll[0].x = 0;
            pMovingSquare->poAll[0].y = 0;
            pMovingSquare->poAll[1].x = 1;
            pMovingSquare->poAll[1].y = 0;
            pMovingSquare->poAll[2].x = 0;
            pMovingSquare->poAll[2].y = 1;
            pMovingSquare->poAll[3].x = 1;
            pMovingSquare->poAll[3].y = 1;
            break;
        case ACTION_PUTIN:
            if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                return FALSE;
            break;
        case ACTION_LEFT:
            if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x-1 ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x-1 ) )
                return FALSE;

            pPoint->x -= 1;
            break;
        case ACTION_RIGHT:
            if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                return FALSE;

            pPoint->x += 1;
            break;
        case ACTION_ROTATE:
            break;
        case ACTION_DOWN:
            while ( TRUE ){
                if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                    return FALSE;
                if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                    return FALSE;
                pPoint->y += 1;
            }
            break;
    }
    return TRUE;
 }

static BOOL MoveSquareLeftDuck ( PlayingWindow * pPlayingWindow, int iAction )
{
    MovingSquare *pMovingSquare;
    POINT *pPoint;

    if ( iAction == ACTION_INIT )
        pMovingSquare = pPlayingWindow->pPreCreatedSquare;
    else
        pMovingSquare = pPlayingWindow->pMovingSquare;
    pPoint = &pMovingSquare->poOrigin;
    switch ( pMovingSquare->iDir ){
        case DIR_UP:
        case DIR_DOWN:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare->poRightBottom.y = 2;
                    pMovingSquare->poRightBottom.x = 1;
                    pMovingSquare->poAll[0].x = 1;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 0;
                    pMovingSquare->poAll[3].y = 2;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x-1 ) )
                        return FALSE;
                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;

                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    pMovingSquare->iDir = DIR_LEFT;
                    pMovingSquare->poRightBottom.y = 1;
                    pMovingSquare->poRightBottom.x = 2;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 1;
                    pMovingSquare->poAll[1].y = 0;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 2;
                    pMovingSquare->poAll[3].y = 1;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
        case DIR_LEFT:
        case DIR_RIGHT:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare->poRightBottom.y = 1;
                    pMovingSquare->poRightBottom.x = 2;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 1;
                    pMovingSquare->poAll[1].y = 0;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 2;
                    pMovingSquare->poAll[3].y = 1;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;

                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 3 ) )
                        return FALSE;

                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                        return FALSE;

                    pMovingSquare->iDir = DIR_UP;
                    pMovingSquare->poRightBottom.y = 2;
                    pMovingSquare->poRightBottom.x = 1;
                    pMovingSquare->poAll[0].x = 1;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 0;
                    pMovingSquare->poAll[3].y = 2;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
    }			
    return TRUE;
 }

static BOOL MoveSquareRightDuck ( PlayingWindow * pPlayingWindow, int iAction )
{
    MovingSquare *pMovingSquare;
    POINT *pPoint;

    if ( iAction == ACTION_INIT )
        pMovingSquare = pPlayingWindow->pPreCreatedSquare;
    else
        pMovingSquare = pPlayingWindow->pMovingSquare;
    pPoint = &pMovingSquare->poOrigin;
    switch ( pMovingSquare->iDir ){
        case DIR_UP:
        case DIR_DOWN:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare->poRightBottom.y = 2;
                    pMovingSquare->poRightBottom.x = 1;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 1;
                    pMovingSquare->poAll[3].y = 2;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                        return FALSE;
                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                        return FALSE;
                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    pMovingSquare->iDir = DIR_LEFT;
                    pMovingSquare->poRightBottom.y = 1;
                    pMovingSquare->poRightBottom.x = 2;
                    pMovingSquare->poAll[0].x = 1;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 2;
                    pMovingSquare->poAll[1].y = 0;
                    pMovingSquare->poAll[2].x = 0;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 1;
                    pMovingSquare->poAll[3].y = 1;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x + 1 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
        case DIR_LEFT:
        case DIR_RIGHT:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare->poRightBottom.y = 1;
                    pMovingSquare->poRightBottom.x = 2;
                    pMovingSquare->poAll[0].x = 1;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 2;
                    pMovingSquare->poAll[1].y = 0;
                    pMovingSquare->poAll[2].x = 0;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 1;
                    pMovingSquare->poAll[3].y = 1;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x-1 ) )
                        return FALSE;
                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 3 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                        return FALSE;
                    pMovingSquare->iDir = DIR_UP;
                    pMovingSquare->poRightBottom.y = 2;
                    pMovingSquare->poRightBottom.x = 1;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 1;
                    pMovingSquare->poAll[3].y = 2;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
    }
    return TRUE;
 }

static BOOL MoveSquareHill ( PlayingWindow * pPlayingWindow, int iAction )
{
    MovingSquare *pMovingSquare;
    POINT *pPoint;

    if ( iAction == ACTION_INIT )
        pMovingSquare = pPlayingWindow->pPreCreatedSquare;
    else
        pMovingSquare = pPlayingWindow->pMovingSquare;
    pPoint = &pMovingSquare->poOrigin;
    switch ( pMovingSquare->iDir ){
        case DIR_UP:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare->poRightBottom.y = 1;
                    pMovingSquare->poRightBottom.x = 2;
                    pMovingSquare->poAll[0].x = 1;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 2;
                    pMovingSquare->poAll[3].y = 1;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x-1 ) )
                        return FALSE;
                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 3 ) )
                        return FALSE;
                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                        return FALSE;
                    pMovingSquare->iDir = DIR_RIGHT;

                    pPoint->x += 1;
                    pMovingSquare->poRightBottom.y = 2;
                    pMovingSquare->poRightBottom.x = 1;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 0;
                    pMovingSquare->poAll[3].y = 2;

                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
        case DIR_DOWN:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare->poRightBottom.y = 1;
                    pMovingSquare->poRightBottom.x = 2;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 1;
                    pMovingSquare->poAll[1].y = 0;
                    pMovingSquare->poAll[2].x = 2;
                    pMovingSquare->poAll[2].y = 0;
                    pMovingSquare->poAll[3].x = 1;
                    pMovingSquare->poAll[3].y = 1;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;

                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 3 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;

                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y-1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y-1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    pPoint->y -= 1;
                    pMovingSquare->iDir = DIR_LEFT;
                    pMovingSquare->poRightBottom.y = 2;
                    pMovingSquare->poRightBottom.x = 1;
                    pMovingSquare->poAll[0].x = 1;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 1;
                    pMovingSquare->poAll[3].y = 2;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
        case DIR_LEFT:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare->poRightBottom.y = 2;
                    pMovingSquare->poRightBottom.x = 1;
                    pMovingSquare->poAll[0].x = 1;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 1;
                    pMovingSquare->poAll[3].y = 2;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                        return FALSE;

                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                        return FALSE;

                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                        return FALSE;

                    pMovingSquare->iDir = DIR_UP;
                    pMovingSquare->poRightBottom.y = 1;
                    pMovingSquare->poRightBottom.x = 2;
                    pMovingSquare->poAll[0].x = 1;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 2;
                    pMovingSquare->poAll[3].y = 1;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x + 1 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
        case DIR_RIGHT:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare->poRightBottom.y = 2;
                    pMovingSquare->poRightBottom.x = 1;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 0;
                    pMovingSquare->poAll[3].y = 2;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x-1 ) )
                        return FALSE;

                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;
                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;

                    pPoint->x -= 1;
                    pPoint->y += 1;
                    pMovingSquare->iDir = DIR_DOWN;

                    pMovingSquare->poRightBottom.y = 1;
                    pMovingSquare->poRightBottom.x = 2;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 1;
                    pMovingSquare->poAll[1].y = 0;
                    pMovingSquare->poAll[2].x = 2;
                    pMovingSquare->poAll[2].y = 0;
                    pMovingSquare->poAll[3].x = 1;
                    pMovingSquare->poAll[3].y = 1;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
    }
    return TRUE;
 }

static BOOL MoveSquareLeftChair ( PlayingWindow * pPlayingWindow, int iAction )
{
    MovingSquare *pMovingSquare;
    POINT *pPoint;
    if ( iAction == ACTION_INIT )
        pMovingSquare = pPlayingWindow->pPreCreatedSquare;
    else
        pMovingSquare = pPlayingWindow->pMovingSquare;
    pPoint = &pMovingSquare->poOrigin;
    switch ( pMovingSquare->iDir ){
        case DIR_UP:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare->poRightBottom.y = 1;
                    pMovingSquare->poRightBottom.x = 2;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 2;
                    pMovingSquare->poAll[3].y = 1;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x-1 ) )
                        return FALSE;

                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 3 ) )
                        return FALSE;
                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                        return FALSE;

                    pMovingSquare->iDir = DIR_RIGHT;
                    pMovingSquare->poRightBottom.y = 2;
                    pMovingSquare->poRightBottom.x = 1;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 1;
                    pMovingSquare->poAll[1].y = 0;
                    pMovingSquare->poAll[2].x = 0;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 0;
                    pMovingSquare->poAll[3].y = 2;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
        case DIR_DOWN:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare->poRightBottom.y = 1;
                    pMovingSquare->poRightBottom.x = 2;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 1;
                    pMovingSquare->poAll[1].y = 0;
                    pMovingSquare->poAll[2].x = 2;
                    pMovingSquare->poAll[2].y = 0;
                    pMovingSquare->poAll[3].x = 2;
                    pMovingSquare->poAll[3].y = 1;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 3 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 3 ) )
                        return FALSE;
                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;

                    pMovingSquare->iDir = DIR_LEFT;
                    pMovingSquare->poRightBottom.y = 2;
                    pMovingSquare->poRightBottom.x = 1;
                    pMovingSquare->poAll[0].x = 1;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 1;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 2;
                    pMovingSquare->poAll[3].x = 0;
                    pMovingSquare->poAll[3].y = 2;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
        case DIR_LEFT:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare->poRightBottom.y = 2;
                    pMovingSquare->poRightBottom.x = 1;
                    pMovingSquare->poAll[0].x = 1;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 1;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 2;
                    pMovingSquare->poAll[3].x = 0;
                    pMovingSquare->poAll[3].y = 2;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x-1 ) )
                        return FALSE;

                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                        return FALSE;
                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;

                    pMovingSquare->iDir = DIR_UP;
                    pMovingSquare->poRightBottom.y = 1;
                    pMovingSquare->poRightBottom.x = 2;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 2;
                    pMovingSquare->poAll[3].y = 1;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x + 1 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
        case DIR_RIGHT:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare->poRightBottom.y = 2;
                    pMovingSquare->poRightBottom.x = 1;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 1;
                    pMovingSquare->poAll[1].y = 0;
                    pMovingSquare->poAll[2].x = 0;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 0;
                    pMovingSquare->poAll[3].y = 2;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x-1 ) )
                        return FALSE;

                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;
                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;

                    pMovingSquare->iDir = DIR_DOWN;
                    pMovingSquare->poRightBottom.y = 1;
                    pMovingSquare->poRightBottom.x = 2;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 1;
                    pMovingSquare->poAll[1].y = 0;
                    pMovingSquare->poAll[2].x = 2;
                    pMovingSquare->poAll[2].y = 0;
                    pMovingSquare->poAll[3].x = 2;
                    pMovingSquare->poAll[3].y = 1;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
    }

    return TRUE;
 }

static BOOL MoveSquareRightChair ( PlayingWindow * pPlayingWindow, int iAction )
{
    MovingSquare *pMovingSquare;
    POINT *pPoint;

    if ( iAction == ACTION_INIT )
        pMovingSquare = pPlayingWindow->pPreCreatedSquare;
    else
        pMovingSquare = pPlayingWindow->pMovingSquare;
    pPoint = &pMovingSquare->poOrigin;
    switch ( pMovingSquare->iDir ){
        case DIR_UP:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare->poRightBottom.y = 1;
                    pMovingSquare->poRightBottom.x = 2;
                    pMovingSquare->poAll[0].x = 2;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 2;
                    pMovingSquare->poAll[3].y = 1;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x-1 ) )
                        return FALSE;

                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 3 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 3 ) )
                        return FALSE;
                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                        return FALSE;

                    pMovingSquare->iDir = DIR_RIGHT;

                    pMovingSquare->poRightBottom.y = 2;
                    pMovingSquare->poRightBottom.x = 1;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 0;
                    pMovingSquare->poAll[2].y = 2;
                    pMovingSquare->poAll[3].x = 1;
                    pMovingSquare->poAll[3].y = 2;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
        case DIR_DOWN:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare->poRightBottom.y = 1;
                    pMovingSquare->poRightBottom.x = 2;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 1;
                    pMovingSquare->poAll[1].y = 0;
                    pMovingSquare->poAll[2].x = 2;
                    pMovingSquare->poAll[2].y = 0;
                    pMovingSquare->poAll[3].x = 0;
                    pMovingSquare->poAll[3].y = 1;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x-1 ) )
                        return FALSE;

                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 3 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 3 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x + 2 ) )
                        return FALSE;

                    pMovingSquare->iDir = DIR_LEFT;
                    pMovingSquare->poRightBottom.y = 2;
                    pMovingSquare->poRightBottom.x = 1;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 1;
                    pMovingSquare->poAll[1].y = 0;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 1;
                    pMovingSquare->poAll[3].y = 2;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
        case DIR_LEFT:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare->poRightBottom.y = 2;
                    pMovingSquare->poRightBottom.x = 1;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 1;
                    pMovingSquare->poAll[1].y = 0;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 1;
                    pMovingSquare->poAll[3].y = 2;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                        return FALSE;

                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                        return FALSE;
                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                        return FALSE;

                    pMovingSquare->iDir = DIR_UP;
                    pMovingSquare->poRightBottom.y = 1;
                    pMovingSquare->poRightBottom.x = 2;
                    pMovingSquare->poAll[0].x = 2;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 1;
                    pMovingSquare->poAll[2].y = 1;
                    pMovingSquare->poAll[3].x = 2;
                    pMovingSquare->poAll[3].y = 1;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x + 1 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
        case DIR_RIGHT:
            switch ( iAction ){
                case ACTION_INIT:
                    pMovingSquare->poRightBottom.y = 2;
                    pMovingSquare->poRightBottom.x = 1;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 0;
                    pMovingSquare->poAll[1].y = 1;
                    pMovingSquare->poAll[2].x = 0;
                    pMovingSquare->poAll[2].y = 2;
                    pMovingSquare->poAll[3].x = 1;
                    pMovingSquare->poAll[3].y = 2;
                    break;
                case ACTION_PUTIN:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                        return FALSE;
                    break;
                case ACTION_LEFT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x-1 ) )
                        return FALSE;

                    pPoint->x -= 1;
                    break;
                case ACTION_RIGHT:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                        return FALSE;
                    pPoint->x += 1;
                    break;
                case ACTION_ROTATE:
                    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x-1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 1 ) )
                        return FALSE;
                    if ( !TestPosition ( pPlayingWindow, pPoint->y, pPoint->x + 2 ) )
                        return FALSE;

                    pMovingSquare->iDir = DIR_DOWN;
                    pMovingSquare->poRightBottom.y = 1;
                    pMovingSquare->poRightBottom.x = 2;
                    pMovingSquare->poAll[0].x = 0;
                    pMovingSquare->poAll[0].y = 0;
                    pMovingSquare->poAll[1].x = 1;
                    pMovingSquare->poAll[1].y = 0;
                    pMovingSquare->poAll[2].x = 2;
                    pMovingSquare->poAll[2].y = 0;
                    pMovingSquare->poAll[3].x = 0;
                    pMovingSquare->poAll[3].y = 1;
                    break;
                case ACTION_DOWN:
                    while ( TRUE ){
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x ) )
                            return FALSE;
                        if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x + 1 ) )
                            return FALSE;
                        pPoint->y += 1;
                    }
                    break;
            }
            break;
    }
    return TRUE;
 }

static BOOL MoveSquare ( PlayingWindow * pPlayingWindow, int iAction )
{
    MovingSquare *pMovingSquare;
    if ( iAction == ACTION_INIT )
        pMovingSquare = pPlayingWindow->pPreCreatedSquare;
    else
        pMovingSquare = pPlayingWindow->pMovingSquare;
    if ( !pMovingSquare ) return FALSE;
    switch ( pMovingSquare->iType ){
        case SQUARE_NOTHING:
            return FALSE;
        case SQUARE_LONG:
            return MoveSquareLong ( pPlayingWindow, iAction );
        case SQUARE_SQUARE:
            return MoveSquareSquare ( pPlayingWindow, iAction );
        case SQUARE_LEFT_DUCK:
            return MoveSquareLeftDuck ( pPlayingWindow, iAction );
        case SQUARE_RIGHT_DUCK:
            return MoveSquareRightDuck ( pPlayingWindow, iAction );
        case SQUARE_HILL:
            return MoveSquareHill ( pPlayingWindow, iAction );
        case SQUARE_LEFT_CHAIR:
            return MoveSquareLeftChair ( pPlayingWindow, iAction );
        case SQUARE_RIGHT_CHAIR:
            return MoveSquareRightChair ( pPlayingWindow, iAction );
    }
    return TRUE;
 }

static int  PutSquare ( PlayingWindow *pPlayingWindow, int *y,int *x )
{
    MovingSquare *pMovingSquare;
    int i, j;
    assert ( pPlayingWindow );
    if ( !pPlayingWindow->pMovingSquare )
        return -1;
    for ( i = 0; i < RUSSIA_SQUARE_HEIGHT; i++ )
        for ( j = 0; j < RUSSIA_SQUARE_WIDTH; j++ ){
            theApp.iTempStateMatrix[i][j] = pPlayingWindow->iStateMatrix[i][j];
        }
    pMovingSquare = pPlayingWindow->pMovingSquare;
    *x = pMovingSquare->poOrigin.x +  pMovingSquare->poAll[0].x;
    pPlayingWindow->iStateMatrix\
        [pMovingSquare->poOrigin.y + pMovingSquare->poAll[0].y] \
        [pMovingSquare->poOrigin.x + pMovingSquare->poAll[0].x] \
        = pMovingSquare->iType;
    pPlayingWindow->iStateMatrix\
        [pMovingSquare->poOrigin.y + pMovingSquare->poAll[1].y]\
        [pMovingSquare->poOrigin.x + pMovingSquare->poAll[1].x]\
        = pMovingSquare->iType;
    pPlayingWindow->iStateMatrix\
        [pMovingSquare->poOrigin.y + pMovingSquare->poAll[2].y]\
        [pMovingSquare->poOrigin.x + pMovingSquare->poAll[2].x]\
        = pMovingSquare->iType;
    *y = pMovingSquare->poOrigin.y +  pMovingSquare->poAll[3].y;
    pPlayingWindow->iStateMatrix\
        [pMovingSquare->poOrigin.y + pMovingSquare->poAll[3].y]\
        [pMovingSquare->poOrigin.x + pMovingSquare->poAll[3].x]\
        = pMovingSquare->iType;

    free ( pMovingSquare );
    pPlayingWindow->pMovingSquare = NULL;
    return 0;
 }

static BOOL WillNotTouch ( PlayingWindow *pPlayingWindow )
{
    MovingSquare *pMovingSquare;
    assert ( pPlayingWindow->pMovingSquare );
    pMovingSquare = pPlayingWindow->pMovingSquare;
    switch ( pMovingSquare->iType ){
        case SQUARE_NOTHING:
            return FALSE;
        case SQUARE_LONG:
            return WillNotTouchLong ( pPlayingWindow );
        case SQUARE_SQUARE:
            return WillNotTouchSquare ( pPlayingWindow );
        case SQUARE_LEFT_DUCK:
            return WillNotTouchLeftDuck ( pPlayingWindow );
        case SQUARE_RIGHT_DUCK:
            return WillNotTouchRightDuck ( pPlayingWindow );
        case SQUARE_HILL:
            return WillNotTouchHill ( pPlayingWindow );
        case SQUARE_LEFT_CHAIR:
            return WillNotTouchLeftChair ( pPlayingWindow );
        case SQUARE_RIGHT_CHAIR:
            return WillNotTouchRightChair ( pPlayingWindow );
    }
    return TRUE;
 }

static BOOL WillNotTouchLong ( PlayingWindow *pPlayingWindow )
{
    MovingSquare *pMovingSquare;
    POINT *pPoint;
    pMovingSquare = pPlayingWindow->pMovingSquare;
    pPoint = &pMovingSquare->poOrigin;

    switch ( pMovingSquare->iDir ){
        case DIR_UP:
        case DIR_DOWN:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 4, pPoint->x ) )
                return FALSE;
            break;
        case DIR_LEFT:
        case DIR_RIGHT:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 3 ) )
                return FALSE;
            break;
    }
    return TRUE;
 }

static BOOL WillNotTouchSquare ( PlayingWindow *pPlayingWindow )
{ 
    MovingSquare *pMovingSquare;
    POINT *pPoint;
    pMovingSquare = pPlayingWindow->pMovingSquare;
    pPoint = &pMovingSquare->poOrigin;
    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
        return FALSE;
    if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
        return FALSE;
    return TRUE;
 }

static BOOL WillNotTouchLeftDuck ( PlayingWindow *pPlayingWindow )
{
    MovingSquare *pMovingSquare;
    POINT *pPoint;
    pMovingSquare = pPlayingWindow->pMovingSquare;
    pPoint = &pMovingSquare->poOrigin;
    switch ( pMovingSquare->iDir ){
        case DIR_UP:
        case DIR_DOWN:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                return FALSE;
            break;
        case DIR_LEFT:
        case DIR_RIGHT:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                return FALSE;
            break;
    }    
    return TRUE;
}

static BOOL WillNotTouchRightDuck ( PlayingWindow *pPlayingWindow )
{ 
    MovingSquare *pMovingSquare;
    POINT *pPoint;
    pMovingSquare = pPlayingWindow->pMovingSquare;
    pPoint = &pMovingSquare->poOrigin;
    switch ( pMovingSquare->iDir ){
        case DIR_UP:
        case DIR_DOWN:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x + 1 ) )
                return FALSE;
            break;
        case DIR_LEFT:
        case DIR_RIGHT:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                return FALSE;
            break;
    }
    return TRUE;
}

static BOOL WillNotTouchHill ( PlayingWindow *pPlayingWindow )
{
    MovingSquare *pMovingSquare;
    POINT *pPoint;
    pMovingSquare = pPlayingWindow->pMovingSquare;
    pPoint = &pMovingSquare->poOrigin;
    switch ( pMovingSquare->iDir ){
        case DIR_UP:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                return FALSE;
            break;
        case DIR_DOWN:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                return FALSE;
            break;
        case DIR_LEFT:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x + 1 ) )
                return FALSE;
            break;
        case DIR_RIGHT:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                return FALSE;
            break;
    }
    return TRUE;
}

static BOOL WillNotTouchLeftChair ( PlayingWindow *pPlayingWindow )
{
    MovingSquare *pMovingSquare;
    POINT *pPoint;
    pMovingSquare = pPlayingWindow->pMovingSquare;
    pPoint = &pMovingSquare->poOrigin;
    switch ( pMovingSquare->iDir ){
        case DIR_UP:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                return FALSE;
            break;
        case DIR_DOWN:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                return FALSE;
            break;
        case DIR_LEFT:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x + 1 ) )
                return FALSE;
            break;
        case DIR_RIGHT:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                return FALSE;
            break;
    }
    return TRUE;
}

static BOOL WillNotTouchRightChair ( PlayingWindow *pPlayingWindow )
{
    MovingSquare *pMovingSquare;
    POINT *pPoint;
    pMovingSquare = pPlayingWindow->pMovingSquare;
    pPoint = &pMovingSquare->poOrigin;
    switch ( pMovingSquare->iDir ){
        case DIR_UP:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 1 ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x + 2 ) )
                return FALSE;
            break;
        case DIR_DOWN:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 2, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 1 ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x + 2 ) )
                return FALSE;
            break;
        case DIR_LEFT:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 1, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x + 1 ) )
                return FALSE;
            break;
        case DIR_RIGHT:
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x ) )
                return FALSE;
            if ( !TestPosition ( pPlayingWindow, pPoint->y + 3, pPoint->x + 1 ) )
                return FALSE;
            break;
    }
    return TRUE;
}

static int RemoveFullLines ( PlayingWindow * pPlayingWindow, int iWhich )
{
    int iConFullLines;
    int iTotFullLines;
    int i, j, k;

    assert ( pPlayingWindow );
    iConFullLines = 0;
    iTotFullLines = 0;
    for ( i = RUSSIA_SQUARE_HEIGHT-1; i >= 0; i-- ){
        switch ( LineFullEmpty ( pPlayingWindow->iStateMatrix[i] ) ){
            case LINE_FULL:
                iConFullLines++;
                /*Remove the line:*/
                for ( j = i++; j > 0; j-- ){
                    if ( LineFullEmpty ( pPlayingWindow->iStateMatrix\
                                [j-1] ) == LINE_EMPTY )
                        break;
                    for ( k = 0; k < RUSSIA_SQUARE_WIDTH; k++ )
                        pPlayingWindow->iStateMatrix[j]\
                            [k] = pPlayingWindow->iStateMatrix[j-1][k];
                }
                for ( k = 0; k < RUSSIA_SQUARE_WIDTH; k++ )
                    pPlayingWindow->iStateMatrix[j][k] = 0;

                break;
            case LINE_EMPTY:
                if ( iConFullLines ){
                    IncrementScore ( pPlayingWindow, iConFullLines );
                    if (  ( theApp.bTease )&& ( iConFullLines >= 2 ) ){
                        TeaseTheOtherPlayer ( 3-iWhich, i, iConFullLines );
                    }
                    iTotFullLines += iConFullLines;
                    iConFullLines = 0;
                }
                pPlayingWindow->iScore %= HIGHEST_SCORE;
                return iTotFullLines;
            case LINE_PARTLYFULL:
                if ( iConFullLines ){
                    IncrementScore ( pPlayingWindow, iConFullLines );
                    if ( iConFullLines >= 2 ){
                        TeaseTheOtherPlayer ( 3-iWhich, i, iConFullLines );
                    }
                    iTotFullLines += iConFullLines;
                    iConFullLines = 0;
                }
                break;
        }
    }
    return iTotFullLines;
} 

/*1:full, 0:empty, 2:partly filled.*/

static int LineFullEmpty ( int *pLine )
{ 
    BOOL bMeetZero;
    BOOL bMeetOne;
    int i;
    bMeetZero = FALSE;
    bMeetOne = FALSE;
    for ( i = 0; i < RUSSIA_SQUARE_WIDTH; i++ )	
    {
        if ( pLine[i] == 0 )
        {
            if ( bMeetOne )
                return LINE_PARTLYFULL;
            bMeetZero = TRUE;
        }
        else
        {
            if ( bMeetZero )
                return LINE_PARTLYFULL;
            bMeetOne = TRUE;
        }
    }
    if ( bMeetZero )
        return	LINE_EMPTY;
    return LINE_FULL;
 }

static void ConvertRectCord ( HWND hWnd, int iWhich, RECT * pRect )
{
    RECT rect;
    int iSideLength;
    POINT poStart;

    GetClientRect ( hWnd, &rect );
    iSideLength = 8;
    poStart.y = rect.top +  14;
    if ( theApp.pPlayingWindow ){
        poStart.x  = rect.left + 45;
    }else
        assert ( 0 );
    if (  ( pRect->left == 0 )&& ( pRect->right == 0 )&&\
            ( pRect->top == 0 )&& ( pRect->bottom == 0 ) ){
        pRect->left = poStart.x;
        pRect->top  = poStart.y + 2;
        pRect->right = poStart.x + iSideLength*RUSSIA_SQUARE_WIDTH + 92 + 4;
        pRect->bottom = poStart.y  +
            iSideLength*RUSSIA_SQUARE_HEIGHT;
        return;
    }
    if ( pRect->left < 0 )
        pRect->left = 0;
    if ( pRect->right > RUSSIA_SQUARE_WIDTH )
        pRect->right = RUSSIA_SQUARE_WIDTH;
    if ( pRect->top < 0 )
        pRect->top = 0;
    if ( pRect->bottom > RUSSIA_SQUARE_HEIGHT )
        pRect->bottom = RUSSIA_SQUARE_HEIGHT;
    poStart.y += 0*iSideLength;
    pRect->left = poStart.x +  pRect->left*iSideLength;
    pRect->right = poStart.x +  pRect->right*iSideLength;
    pRect->top = poStart.y +  pRect->top*iSideLength;
    pRect->bottom = poStart.y +  pRect->bottom*iSideLength;
    return;
 }
/*i: line j: column*/
static BOOL TestPosition ( PlayingWindow *pPlayingWindow , int i, int j )
{
    if ( i < 0 ) return FALSE;
    if ( i >= RUSSIA_SQUARE_HEIGHT ) return FALSE;
    if ( j < 0 ) return FALSE;
    if ( j >= RUSSIA_SQUARE_WIDTH ) return FALSE;
    if ( pPlayingWindow->iStateMatrix[i][j] != 0 ) return FALSE;
    return TRUE;
 }

static void UpdateHighScore ( HWND hWnd, PlayingWindow *pPlayingWindow )
{
    int iScore ;
    int i, j;
    iScore = pPlayingWindow->iScore;
    if ( theApp.bTease )
        return;
    for ( i = 0; i < 5; i++ )
        if ( iScore > theApp.HighScore[i] )
            break;
    if ( i == 5 ){
        return;
    }
    for ( j = 4; j > i; j-- ){
        theApp.HighScore[j] = theApp.HighScore[j-1];
    }
    theApp.bBusy  = TRUE;
    theApp.HighScore[i]  = iScore;
    theApp.bBusy  = FALSE;
 }

static BOOL IncrementScore ( PlayingWindow *pPlayingWindow, int iLines )
{
    switch ( iLines ){
        case 1:
            pPlayingWindow->iScore += 100 +  theApp.iDifficulty*100;
            break;
        case 2:
            pPlayingWindow->iScore += 300 +  theApp.iDifficulty*100;
            break;
        case 3:
            pPlayingWindow->iScore += 700 +  theApp.iDifficulty*100;
            break;
        case 4:
            pPlayingWindow->iScore += 1500 +  theApp.iDifficulty*100;
            break;
    }
    return TRUE;
 }

static BOOL TeaseTheOtherPlayer ( int iWhich, int iBegin, int iLines )
{
    PlayingWindow *pPlayingWindow;
    int i, j, k;
    assert (  ( iWhich == 1 ) || ( iWhich == 2 ) );
    assert ( iBegin >= 0 );
    assert (  ( iLines >= 2 ) && ( iLines <= 4 ) );
    assert ( iBegin-iLines + 1 >= 0 );

    if ( iWhich == 1 )
        pPlayingWindow = theApp.pPlayingWindow;
    else
        return FALSE;

    /*begin tease...*/
    free ( pPlayingWindow->pMovingSquare );
    pPlayingWindow->pMovingSquare = NULL;
    for ( i = iBegin-iLines + 1; i <= iBegin; i++ ){
        if ( LineFullEmpty ( pPlayingWindow->iStateMatrix[3] )!= LINE_EMPTY )
            break;
        j = 0;
        while ( j <= RUSSIA_SQUARE_HEIGHT-1 ){
            if ( LineFullEmpty ( pPlayingWindow->iStateMatrix[j] )!= LINE_EMPTY )
                break;
            j++;
        }
        while ( j <= RUSSIA_SQUARE_HEIGHT-1 ){
            for ( k = 0; k < RUSSIA_SQUARE_WIDTH; k++ ){
                pPlayingWindow->iStateMatrix[j-1][k] = pPlayingWindow->iStateMatrix[j][k];
            }
            j++;
        }
        for ( k = 0; k < RUSSIA_SQUARE_WIDTH; k++ ){
            pPlayingWindow->iStateMatrix[RUSSIA_SQUARE_HEIGHT-1][k] = theApp.iTempStateMatrix[i][k];
        }
    }
    return TRUE;
 }

static int TestMyWinProc ( HWND hWnd, int message, WPARAM wParam, LPARAM lParam )
{
    HDC hdc;
    char st[6][20];
    char str[100];
    FILE *stream;
    PlayingWindow * pPlayingWindow;

    switch ( message ){
        case MSG_CREATE:
            stream = fopen ( RUSSIA_SCOREPATH, "r+" );
            if ( stream == NULL )
            {
                fprintf ( stderr, "Can not open the file of %s to read something!\n", RUSSIA_SCOREPATH );
                theApp.HighScore[0] = 0;
                theApp.HighScore[1] = 0;
                theApp.HighScore[2] = 0;
                theApp.HighScore[3] = 0;
                theApp.HighScore[4] = 0;
            }
            else
            {
                fgets ( st[0], 100, stream );
                theApp.HighScore[0] = strtol ( st[0], NULL, 10 );
                fgets ( st[1], 100, stream );
                theApp.HighScore[1] = strtol ( st[1], NULL, 10 );
                fgets ( st[2], 100, stream );
                theApp.HighScore[2] = strtol ( st[2], NULL, 10 );
                fgets ( st[3], 100, stream );
                theApp.HighScore[3] = strtol ( st[3], NULL, 10 );
                fgets ( st[4], 100, stream );
                theApp.HighScore[4] = strtol ( st[4], NULL, 10 );
                fclose ( stream );
            }
            theApp.iDifficulty = russia_menu_params.level;
            theApp.pPlayingWindow = ( PlayingWindow* )calloc ( 1, sizeof ( PlayingWindow ) );
            pPlayingWindow = theApp.pPlayingWindow;
            break;
        case MSG_PAINT:
            hdc = BeginPaint ( hWnd );
            OnDraw ( hWnd, hdc );
            EndPaint ( hWnd, hdc );
            return 0;
        case MSG_KEYDOWN:
            switch ( wParam ) {
                case SCANCODE_F1:
                    KillTimer ( hWnd, RUSSIA_SQUARE_TIMER1 );
                    switch ( pmp_show_menu ( hWnd, _( IDS_PMP_RUSSIA_TITLE ), russia_menu, &russia_menu_params ) ){
                        case 0:
                            SetTimer ( hMainWnd, RUSSIA_SQUARE_TIMER1, 1000/TimerFreq[theApp.pPlayingWindow->iLevel] );
                            break;
                        case 1:
                            theApp.bTease = FALSE;
                            theApp.iDifficulty = russia_menu_params.level;
                            OnCommandNewOne ( hMainWnd );
                            InvalidateRect ( hMainWnd, NULL, TRUE );
                            return OnStartKeyDown ( hWnd, theApp.pPlayingWindow, 1 );
                            break;
                        case 2:
                            theApp.iDifficulty = russia_menu_params.level;
                            theApp.bTease = FALSE;
                            OnCommandNewOne ( hMainWnd );
                            InvalidateRect ( hMainWnd, NULL, TRUE );
                            return OnStartKeyDown ( hWnd, theApp.pPlayingWindow, 1 );
                            break;
                        case 3:
                            sprintf ( str, _( IDS_PMP_RUSSIA_SHOW_SCORE ),
                                    theApp.HighScore[0], theApp.HighScore[1],
                                    theApp.HighScore[2], theApp.HighScore[3],
                                    theApp.HighScore[4] );
                            pmp_show_info ( hWnd, _( IDS_PMP_RUSSIA_SCORE_INFO ), str, MB_ICONINFORMATION );
                            SetTimer ( hMainWnd, RUSSIA_SQUARE_TIMER1, 1000/TimerFreq[theApp.pPlayingWindow->iLevel] );
                            break;
                        default:
                            break;
                    }
                    InvalidateRect ( hMainWnd, NULL, TRUE );
                    break;
                case SCANCODE_ESCAPE:
                    if ( TRUE != theApp.pPlayingWindow->bDead )
                        UpdateHighScore ( hWnd, theApp.pPlayingWindow );
                    theApp.pPlayingWindow->iScore = 0;
                    stream = fopen (  RUSSIA_SCOREPATH, "w+" );
                    if ( stream == NULL )
                    {
                        fprintf ( stderr, "Can not open the file of %s to write something\n", RUSSIA_SCOREPATH );

                        theApp.pPlayingWindow->bPause = TRUE;
                        theApp.pPlayingWindow->bBegin = FALSE;
                        KillTimer ( hWnd, RUSSIA_SQUARE_TIMER1 );
                        InvalidateRect ( hMainWnd, &rect_condition, TRUE );
                        pmp_show_info ( hWnd, _( IDS_PMP_RUSSIA_TEXT ), _( IDS_PMP_RUSSIA_WRFILE_SHOW ), MB_ICONINFORMATION );
                    }
                    else
                    {
                        sprintf ( str, "%d\n", theApp.HighScore[0] );
                        fputs ( str, stream );
                        sprintf ( str, "%d\n", theApp.HighScore[1] );
                        fputs ( str, stream );
                        sprintf ( str, "%d\n", theApp.HighScore[2] );
                        fputs ( str, stream );
                        sprintf ( str, "%d\n", theApp.HighScore[3] );
                        fputs ( str, stream );
                        sprintf ( str, "%d\n", theApp.HighScore[4] );
                        fputs ( str, stream );
                        fclose ( stream );
                    }
                    PostMessage ( hWnd, MSG_CLOSE, 0, 0 );
                    break;
                case SCANCODE_CURSORBLOCKUP:
                    return OnRotateKeyDown ( hWnd, theApp.pPlayingWindow, 1 );
                case SCANCODE_CURSORBLOCKDOWN:
                    /*when the downkeydown , then the square will down fast*/
                    return OnDownKeyDown ( hWnd, theApp.pPlayingWindow, 1 );
                case SCANCODE_CURSORBLOCKLEFT:
                    return OnLeftKeyDown ( hWnd, theApp.pPlayingWindow, 1 );
                case SCANCODE_CURSORBLOCKRIGHT:
                    return OnRightKeyDown ( hWnd, theApp.pPlayingWindow, 1 );
                case SCANCODE_ENTER:
                    /*Pause the game when playing*/
                    if ( theApp.pPlayingWindow->bDead == FALSE && theApp.pPlayingWindow->bPause == FALSE && theApp.pPlayingWindow->bBegin == TRUE )
                    {
                        theApp.pPlayingWindow->bPause = TRUE;
                        theApp.pPlayingWindow->bBegin = FALSE;
                        KillTimer ( hWnd, RUSSIA_SQUARE_TIMER1 );
                        InvalidateRect ( hMainWnd, &rect_condition, TRUE );
                    }
                    /*Replay the game from pausing*/
                    else if ( theApp.pPlayingWindow->bDead == FALSE && theApp.pPlayingWindow->bPause == TRUE && theApp.pPlayingWindow->bBegin == FALSE )
                    {
                        theApp.pPlayingWindow->bPause = FALSE;
                        theApp.pPlayingWindow->bBegin = TRUE;
                        SetTimer ( hWnd, RUSSIA_SQUARE_TIMER1, 1000/TimerFreq[theApp.pPlayingWindow->iLevel] );
                        InvalidateRect ( hMainWnd, &rect_condition, TRUE );
                    }
                    else if ( theApp.pPlayingWindow->bDead == FALSE 
                            && theApp.pPlayingWindow->bPause == FALSE
                            && theApp.pPlayingWindow->bBegin == FALSE )
                   
                    /* Start play the game and when the game is died, 
                    ** then the game will start agine
                    */
                    {
                        theApp.bTease = FALSE;
                        OnCommandNewOne ( hMainWnd );
                        InvalidateRect ( hMainWnd, NULL, TRUE );
                        return OnStartKeyDown ( hWnd, theApp.pPlayingWindow, 1 );
                    }
                    else
                    {
                        InvalidateRect ( hMainWnd, NULL, TRUE );
                        return OnStartKeyDown ( hWnd, theApp.pPlayingWindow, 1 );
                    }
            }
            return 0;
        case MSG_TIMER:
            if ( theApp.bBusy )
                break;
            if ( wParam == RUSSIA_SQUARE_TIMER1 ){
                OnTimer ( hWnd, theApp.pPlayingWindow, 1 );
            }
            break;
        case MSG_SETFOCUS:
            pmp_set_captionbar_title ( _( IDS_PMP_RUSSIA_TITLE ) );
            break;
        case MSG_DESTROY:
            DestroyAllControls ( hWnd );
            hMainWnd = HWND_INVALID;
            return 0;
        case MSG_CLOSE:
            OnClose ( hWnd );
            return 0;
    }
    return DefaultMainWinProc ( hWnd, message, wParam, lParam );
}

/*Create the main window*/
static void _russia_init_create_info ( PMAINWINCREATE pCreateInfo )
{ 
    pCreateInfo->dwStyle = WS_BORDER | WS_SYSMENU |  WS_VISIBLE;
    pCreateInfo->dwExStyle = WS_EX_NONE;
    pCreateInfo->spCaption = _( IDS_PMP_RUSSIA_TITLE );
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor ( 0 );
    pCreateInfo->hIcon = 0;
    pCreateInfo->MainWindowProc = TestMyWinProc;
    pCreateInfo->lx = 0;
    pCreateInfo->ty = 20;
    pCreateInfo->rx = MAINWINDOW_W;
    pCreateInfo->by = MAINWINDOW_H;
    pCreateInfo->iBkColor = COLOR_black;
    pCreateInfo->dwAddData = 0;
    pCreateInfo->hHosting = HWND_DESKTOP;
 }

void ex_russia_show_window ( HWND parent )
{
    MAINWINCREATE CreateInfo;
    _russia_init_create_info ( &CreateInfo );
    CreateInfo.hHosting = parent;

    hMainWnd = CreateMainWindow ( &CreateInfo );

    ShowWindow ( hMainWnd, SW_SHOWNORMAL );
    pmp_app_notify_opened ( PMP_APP_RUSSIA, hMainWnd );
 }

#endif /* PMP_APP_RUSSIA */
