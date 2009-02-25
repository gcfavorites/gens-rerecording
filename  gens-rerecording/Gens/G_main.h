#ifndef G_MAIN_H
#define G_MAIN_H

#define X_RES 320
#define Y_RES 240

#define TIMER_ID    1
#define TIMER_RATE  30

#include "stdio.h"
//#include "hackdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

extern HINSTANCE ghInstance;
extern WNDCLASS WndClass;
extern HWND HWnd;
extern HWND RamWatchHWnd;
extern HWND RamSearchHWnd;
extern HMENU Gens_Menu;

extern int Paused;
extern int Net_Play;
extern int Current_State;
extern int Full_Screen;
extern int Fast_Blur;
extern int Render_W;
extern int Render_FS;
extern int Setting_Render;
extern int Show_FPS;
extern int Show_Message;
extern int Show_LED;
extern int Auto_Pause;
extern int Auto_Fix_CS;
extern int Language;
extern int Country;
extern int Country_Order[3];
extern int WinNT_Flag;
extern int Gens_Priority;
extern int Kaillera_Client_Running;
extern int Intro_Style;
extern int SegaCD_Accurate;
extern int DialogsOpen;
extern int SlowDownMode; //Modif
extern int VideoLatencyCompensation; // Modif N.
extern int disableVideoLatencyCompensationCount;

extern BOOL AutoFireKeyDown;	//Modif N.
extern BOOL AutoHoldKeyDown;	//Modif N.
extern BOOL AutoClearKeyDown;	//Modif N.
extern BOOL PauseKeyDown; //Modif
extern BOOL FrameAdvanceKeyDown; //Modif
extern BOOL FastForwardKeyDown; // Modif N.

extern int SlowDownSpeed;	//Modif
extern int SkipKeyIsPressed; //Modif

extern int Disable_Blue_Screen; //Modif
extern int Never_Skip_Frame; //Modif
extern int FrameCounterEnabled; //Modif
extern int FrameCounterFrames; //Modif N.
extern int LagCounterEnabled; //Modif
extern int LagCounterFrames; //Modif N.
extern int LeftRightEnabled; //Modif
extern int NumLoadEnabled; //Modif N.
extern int ShowInputEnabled; //Modif
extern int AutoBackupEnabled; //Modif
#define FRAME_COUNTER_TOP_LEFT 5392
#define FRAME_COUNTER_TOP_RIGHT 5650
#define FRAME_COUNTER_BOTTOM_LEFT 67216
#define FRAME_COUNTER_BOTTOM_RIGHT 67470
extern int FrameCounterPosition; //Modif
extern int MustUpdateMenu; // Modif
extern unsigned char StateSelectCfg;
extern unsigned long SpliceFrame;
extern unsigned long SeekFrame;
extern char *TempName;
extern char SpliceMovie[1024];

extern POINT Window_Pos;

extern char Str_Tmp[1024];
extern char Gens_Path[1024];
extern char Language_Path[1024];
extern char CGOffline_Path[1024];
extern char Manual_Path[1024];

#define MAX_RECENT_SCRIPTS 15
extern char Recent_Scripts[MAX_RECENT_SCRIPTS][1024];

#ifdef __cplusplus
};
#endif

void CloseRamWindows(); // So the close rom process can close the Ram Search and watch windows (thus preventing freezeout)
int Set_Render(HWND hWnd, int Full, int Num, int Force);
int Change_Layer(HWND hWnd, int Num); //Nitsuja added this
int Change_68K_Type(HWND hWnd, int Num, int Reset_SND);
int Change_Sound(HWND hWnd);
HMENU Build_Main_Menu(void);
void Update_RAM_Search();
void Update_RAM_Cheats();
void ReopenRamWindows();
int SaveFlags();
void LoadFlags(int flags);
void init_list_box(HWND Box, const char* Strs[], int numColumns, int *columnWidths);
int IsVideoLatencyCompensationOn();
#ifdef __cplusplus
const char* GensOpenScript(const char* filename, const char* extraDirToCheck=0);
const char* GensPlayMovie(const char* filename, bool silent=false);
#endif

#endif