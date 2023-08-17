/* Lunar Rescue since 1979-80 @Sottomarina di Chioggia
	12/8/23
	*/

// https://www.youtube.com/watch?v=6oeAsbC2xKA&t=56s
// https://www.youtube.com/watch?v=cSKQGtyaP6s
// https://www.youtube.com/watch?v=PdAZVsQqWX8
// per i PUNTI https://www.arcade-history.com/?n=lunar-rescue&page=detail&id=1418

#define APPNAME "Lunar Rescue"

// Windows Header Files:
#include <windows.h>
#include <string.h>

// Local Header Files
#include "LunarRescue.h"
#include "resource.h"

// Makes it easier to determine appropriate code paths:
#if defined (WIN32)
	#define IS_WIN32 TRUE
#else
	#define IS_WIN32 FALSE
#endif
#define IS_NT      IS_WIN32 && (BOOL)(GetVersion() < 0x80000000)
#define IS_WIN32S  IS_WIN32 && (BOOL)(!(IS_NT) && (LOBYTE(LOWORD(GetVersion()))<4))
#define IS_WIN95 (BOOL)(!(IS_NT) && !(IS_WIN32S)) && IS_WIN32

// Global Variables:
HINSTANCE g_hinst;
char szAppName[] = APPNAME; // The name of this application
char INIFile[] = APPNAME".ini";
char szTitle[]   = APPNAME; // The title bar text

#define APP_XSIZE (280+10)
#define APP_YSIZE (320+60)
int AppXSize=APP_XSIZE,AppYSize=APP_YSIZE,AppXSizeR,AppYSizeR;
BYTE doubleSize=1,bSuoni=1;
HWND ghWnd,hStatusWnd;
HBRUSH hBrush,hBrush2;
HPEN hPen1,hPen2;
HFONT hFont,hFont2,hFont3,hTitleFont;
DWORD playTime;		// conto cmq il tempo giocato :)
BYTE currQuadro;
int score[2],hiScore,totShips[2],credit;
BYTE maxBombeNow;
BYTE velocitaAlieni=10,velocitaAsteroidi=4;
int nauLChar=VK_LEFT,nauRChar=VK_RIGHT,nauFChar=VK_SPACE;
struct MOB myAsteroidi[MAX_ASTEROIDI+1];
struct MOB myAlieni[MAX_ALIENI+1];
struct MOB myMob[20];
struct MOB myOmini[7];
struct MOB myStars[3];
//BYTE bombs[MAX_BOMBE];
BYTE basi[3][3];
WORD fuel;
BYTE ominiRescued,ominiLost;
HDC hCompDC;
UINT hTimer;
WORD demoTime;

enum PLAY_STATE bPlayMode,bPausedPlayMode;
enum SUB_PLAY_STATE subPlayMode,pausedSubPlayMode;
DWORD subPlayModeTime;
DWORD subPlayData;

COLORREF colorPalette[16]={ RGB(128,128,128),RGB(255,0,0),RGB(0,255,0),RGB(0,0,255),	// in effetti 8 :)
	RGB(255,255,0),RGB(0,255,255),RGB(255,0,255),RGB(255,255,255),
	RGB(128,128,128),RGB(255,0,0),RGB(0,255,0),RGB(0,0,255),
	RGB(255,255,0),RGB(0,255,255),RGB(255,0,255),RGB(255,255,255)
	};

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MSG msg;
	HANDLE hAccelTable;

	if(!hPrevInstance) {
		if(!InitApplication(hInstance)) {
			return (FALSE);
		  }
	  }

	if(!InitInstance(hInstance, nCmdShow)) {
		return (FALSE);
  	}

	if(*lpCmdLine)
		PostMessage(ghWnd,WM_USER+1,0,(LPARAM)lpCmdLine);

	hAccelTable = LoadAccelerators (hInstance,MAKEINTRESOURCE(IDR_ACCELERATOR1));
	while(GetMessage(&msg, NULL, 0, 0)) {
		if(!TranslateAccelerator (msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			}
		}

  return msg.wParam;
	}

void Wait /*DoEvents*/(WORD millisec) {
  MSG msg;
  BOOL result;
	DWORD ti=timeGetTime()+millisec;

  while(ti>timeGetTime()) {
		if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) ) {
			result =GetMessage(&msg, NULL, 0, 0);
			if(result == 0) {		// WM_QUIT
				PostQuitMessage(msg.wParam);
				break;
				}
			else if(result == -1) {
				// Handle errors/exit application, etc.
				}
			else {
				if(msg.message == WM_PAINT) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					}
	      }
      }
	  }
	}

void myTimerProc(HWND hWnd,UINT unnamedParam2,UINT_PTR unnamedParam3,DWORD unnamedParam4) {

	SendMessage(hWnd,WM_TIMER,unnamedParam2,unnamedParam3);
	}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId,wmEvent;
	PAINTSTRUCT ps;
	HDC hDC;
 	POINT pnt;
	HMENU hMenu;
 	BOOL bGotHelp;
	int i,j,k;
	long l;
	char myBuf[128];
	LOGBRUSH br;
	RECT rc;
	SIZE mySize;
	static int TimerState=-1,TimerCnt;
	HFONT hOldFont;
	HPEN hOldPen;
	struct MOB *mp;

	switch(message) { 
		case WM_COMMAND:
			wmId    = LOWORD(wParam); // Remember, these are...
			wmEvent = HIWORD(wParam); // ...different for Win32!

			switch(wmId) {
				case ID_APP_ABOUT:
					DialogBox(g_hinst,MAKEINTRESOURCE(IDD_ABOUT),hWnd,(DLGPROC)About);
					break;

				case ID_APP_EXIT:
					PostMessage(hWnd,WM_CLOSE,0,0l);
					break;

				case ID_FILE_NEW:
					if(credit>0)			// disattivare se credit=0??
						credit--;
					playTime=0;
					totShips[0]=3;
					score[0]=0;
					TimerCnt=(TIMER_GRANULARITY);			// forza redraw iniziale!
					bPlayMode=PLAY_STARTING;
					currQuadro=1;
					maxBombeNow=5;
					fuel=800;
					ominiRescued=ominiLost=0;

					loadMobs(PLAY_MOTHERSHIPWAITING);
					velocitaAlieni=10;		//v.sotto

					RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE | RDW_ERASE);
					// continua a venir cancellata DOPO...
					mp=&myMob[0];
					mp->x.whole=20*doubleSize;
					hDC=GetDC(hWnd);
//					SelectObject(hDC,hBrush);
//					PatBlt(hDC,mp->x,mp->y,mp->s.cx,mp->s.cy,PATCOPY);
					MobDrawXY(hDC,mp,mp->x.whole,mp->y.whole);
					ReleaseDC(hWnd,hDC);

updPlay:
					TimerState=0;
					break;

				case ID_FILE_CLOSE:
					currQuadro=0;		// tanto per
					InvalidateRect(hWnd,NULL,TRUE);
					bPlayMode=PLAY_IDLE;
					goto updPlay;
					break;

				case ID_FILE_UPDATE:
					if((bPlayMode>=PLAY_MOTHERSHIPWAITING && bPlayMode<=PLAY_SPACESHIPDOCKING)) {
						bPausedPlayMode=bPlayMode;
						pausedSubPlayMode=subPlayMode;
						bPlayMode=PLAY_PAUSED;
						SetWindowText(hWnd,APPNAME" (in pausa)");
						}
					else {
						bPlayMode=bPausedPlayMode;
						subPlayMode=pausedSubPlayMode;
						SetWindowText(hWnd,APPNAME);
						}
					break;

				case ID_OPZIONI_HEX:
					InvalidateRect(hWnd,NULL,TRUE);
					break;

				case ID_OPZIONI_CREDITO:
					{
					RECT creditRect={ AppXSize/2,AppYSizeR-16*doubleSize,AppXSizeR,AppYSizeR };
					credit++;
					InvalidateRect(hWnd,&creditRect,TRUE);
					}
					break;

				case ID_OPZIONI_DIMENSIONEDOPPIA:
					doubleSize= doubleSize==1 ? 2 : 1;
					InvalidateRect(hWnd,NULL,TRUE);
          MessageBox(GetFocus(),"Riavviare il gioco!",szAppName,MB_OK|MB_ICONEXCLAMATION);
					break;

				case ID_OPZIONI_SUONI:
					bSuoni=!bSuoni;
					break;

				case ID_EDIT_PASTE:
					break;

        case ID_HELP: // Only called in Windows 95
          bGotHelp = WinHelp(hWnd, APPNAME".HLP", HELP_FINDER,(DWORD)0);
          if(!bGotHelp) {
            MessageBox(GetFocus(),"Unable to activate help",
              szAppName,MB_OK|MB_ICONHAND);
					  }
					break;

				case ID_HELP_INDEX: // Not called in Windows 95
          bGotHelp = WinHelp(hWnd, APPNAME".HLP", HELP_CONTENTS,(DWORD)0);
		      if(!bGotHelp) {
            MessageBox(GetFocus(),"Unable to activate help",
              szAppName,MB_OK|MB_ICONHAND);
					  }
					break;

				case ID_HELP_FINDER: // Not called in Windows 95
          if(!WinHelp(hWnd, APPNAME".HLP", HELP_PARTIALKEY,	(DWORD)(LPSTR)"")) {
						MessageBox(GetFocus(),"Unable to activate help",
							szAppName,MB_OK|MB_ICONHAND);
					  }
					break;

				case ID_HELP_USING: // Not called in Windows 95
					if(!WinHelp(hWnd, (LPSTR)NULL, HELP_HELPONHELP, 0)) {
						MessageBox(GetFocus(),"Unable to activate help",
							szAppName, MB_OK|MB_ICONHAND);
					  }
					break;

				default:
					return (DefWindowProc(hWnd, message, wParam, lParam));
					break;
				}
			break;

		case WM_NCRBUTTONUP: // RightClick on windows non-client area...
			if(IS_WIN95 && SendMessage(hWnd, WM_NCHITTEST, 0, lParam) == HTSYSMENU) {
				// The user has clicked the right button on the applications
				// 'System Menu'. Here is where you would alter the default
				// system menu to reflect your application. Notice how the
				// explorer deals with this. For this app, we aren't doing anything
				return DefWindowProc(hWnd, message, wParam, lParam);
			  }
			else {
				// Nothing we are interested in, allow default handling...
				return DefWindowProc(hWnd, message, wParam, lParam);
			  }
      break;

    case WM_RBUTTONDOWN: // RightClick in windows client area...
      pnt.x = LOWORD(lParam);
      pnt.y = HIWORD(lParam);
      ClientToScreen(hWnd, (LPPOINT)&pnt);
      hMenu = GetSubMenu(GetMenu(hWnd),2);
      if(hMenu) {
        TrackPopupMenu(hMenu, 0, pnt.x, pnt.y, 0, hWnd, NULL);
        }
      break;

		case WM_PAINT:
			hDC=BeginPaint(hWnd,&ps);
			hOldPen=SelectObject(hDC,hPen1);
			hOldFont=SelectObject(hDC,hFont2);
			switch(bPlayMode) {
				case PLAY_IDLE:
					SetTextColor(hDC,RGB(255,255,255));
					SetBkColor(hDC,RGB(0,0,0));
					goto plot_credit;
					break;

				case PLAY_MOTHERSHIPWAITING:
				case PLAY_SPACESHIPFALLING:
				case PLAY_SPACESHIPRESCUING:
				case PLAY_SPACESHIPRISING:
				case PLAY_SPACESHIPDOCKING:
				case PLAY_PAUSED:
				case PLAY_DEMO:
				case PLAY_ENDING:
					{
					HBITMAP hImg;
					hImg=LoadBitmap(g_hinst,MAKEINTRESOURCE(IDB_BACKGROUND));
					SelectObject(hCompDC,hImg);
					StretchBlt(hDC,0,0,AppXSizeR,AppYSizeR,hCompDC,0,0,280,320,SRCCOPY);
					hImg=LoadBitmap(g_hinst,MAKEINTRESOURCE(IDB_BASE3));
					SelectObject(hCompDC,hImg);
					for(i=0; i<3; i++) {
						if(basi[0][i]) {
							StretchBlt(hDC,AppXSizeR/6+i*AppXSizeR/3.3,BASI_AREA+20*doubleSize,
								22*doubleSize,10*doubleSize,hCompDC,0,0,22,10,SRCCOPY);
							}
						}
					hImg=LoadBitmap(g_hinst,MAKEINTRESOURCE(IDB_BASE2));
					SelectObject(hCompDC,hImg);
					for(i=0; i<3; i++) {
						if(basi[1][i]) {
							StretchBlt(hDC,AppXSizeR/6+i*AppXSizeR/3.3-8*doubleSize,BASI_AREA+10*doubleSize,
								38*doubleSize,10*doubleSize,hCompDC,0,0,38,10,SRCCOPY);
							}
						}
					hImg=LoadBitmap(g_hinst,MAKEINTRESOURCE(IDB_BASE1));
					SelectObject(hCompDC,hImg);
					for(i=0; i<3; i++) {
						if(basi[2][i]) {
							StretchBlt(hDC,AppXSizeR/6+i*AppXSizeR/3.3-15*doubleSize,BASI_AREA,
								50*doubleSize,10*doubleSize,hCompDC,0,0,50,10,SRCCOPY);
							}
						}
					}

				case PLAY_STARTING:
				case PLAY_NEWLEVEL:
					mp=&myMob[4];
					for(i=0; i<MAX_SHIPS; i++) {		// astronavi rimaste
						SelectObject(hDC,hBrush);
						PatBlt(hDC,30*doubleSize+(mp->s.cx+4)*i,AppYSizeR-13*doubleSize,mp->s.cx,mp->s.cy,PATCOPY);
						if(i<(totShips[0]-1))
							MobDrawXY(hDC,mp,30*doubleSize+(mp->s.cx+4)*i,AppYSizeR-13*doubleSize);
						}
					SetTextColor(hDC,RGB(0,255,0));
					SetBkColor(hDC,RGB(0,0,0));
					TextOut(hDC,28*doubleSize,45*doubleSize,"FUEL",4);
					wsprintf(myBuf,"%03u",fuel);
					SetTextColor(hDC,RGB(255,255,255));
					TextOut(hDC,64*doubleSize,45*doubleSize,myBuf,_tcslen(myBuf));

plot_omini:
					mp=&myOmini[6];
					for(i=0; i<6; i++) {		// omini salvati
						SelectObject(hDC,hBrush);
						PatBlt(hDC,AppXSizeR-55*doubleSize-20*i*doubleSize,45*doubleSize,mp->s.cx,mp->s.cy,PATCOPY);
						if(i<(ominiRescued-ominiLost))
							MobDrawXY(hDC,mp,AppXSizeR-55*doubleSize-20*i*doubleSize,45*doubleSize);
						}

plot_credit:
					hOldPen=SelectObject(hDC,hPen2);
//					MoveToEx(hDC,0,AppYSizeR-(LOWER_AREA+2)*doubleSize,NULL);
//					LineTo(hDC,AppXSizeR,AppYSizeR-(LOWER_AREA+2)*doubleSize);
					SetTextColor(hDC,RGB(255,255,255));
					SetBkColor(hDC,RGB(0,0,0));
					hOldFont=SelectObject(hDC,hFont2);
					SetTextColor(hDC,RGB(0,255,255));
					TextOut(hDC,20,8*doubleSize,"SCORE<1>",8);
					wsprintf(myBuf,"%05u",score[0]);
					SetTextColor(hDC,RGB(255,255,255));
					TextOut(hDC,38,20*doubleSize,myBuf,_tcslen(myBuf));
					SetTextColor(hDC,RGB(255,0,0));
					TextOut(hDC,AppXSizeR/2-20*doubleSize,8*doubleSize,"TAITO" /*"HI-SCORE"*/,5);
					wsprintf(myBuf,"%05u",hiScore);
					SetTextColor(hDC,RGB(255,255,255));
					TextOut(hDC,AppXSizeR/2-20*doubleSize,20*doubleSize,myBuf,strlen(myBuf));
					SetTextColor(hDC,RGB(255,255,0));
					TextOut(hDC,AppXSizeR-72*doubleSize,8*doubleSize,"SCORE<2>",8);
					wsprintf(myBuf,"%05u",score[1]);
					SetTextColor(hDC,RGB(255,255,255));
					TextOut(hDC,AppXSizeR-60*doubleSize,20*doubleSize,myBuf,strlen(myBuf));
					SetTextColor(hDC,RGB(255,0,255));
					TextOut(hDC,AppXSizeR-95*doubleSize,AppYSizeR-15*doubleSize,"CREDIT",6);
					wsprintf(myBuf,"%2u",credit);
					SetTextColor(hDC,RGB(255,255,255));
					TextOut(hDC,AppXSizeR-40*doubleSize,AppYSizeR-15*doubleSize,myBuf,_tcslen(myBuf));

					break;
				}
			SelectObject(hDC,hOldFont);
			SelectObject(hDC,hOldPen);
			EndPaint(hWnd,&ps);
			break;        

		case WM_SIZE:
			GetClientRect(hWnd,&rc);
			MoveWindow(hStatusWnd,0,rc.bottom-16,rc.right,16,1);
			break;        

		case WM_KEYDOWN:
			i=(TCHAR)wParam;
			switch(bPlayMode) {
				case PLAY_MOTHERSHIPWAITING:
					if(i==nauFChar) {
						goto spaceshiptofall;
						}
					break;
				case PLAY_SPACESHIPFALLING:
					if(i==nauLChar) {
						myMob[0].speed.x=-3*doubleSize;
						}
					else if(i==nauRChar) {
						myMob[0].speed.x=+3*doubleSize;
						}
					else if(i==nauFChar) {
						if(fuel>0) {
							RECT fuelRect={ 58*doubleSize,43*doubleSize,98*doubleSize,57*doubleSize };
							myMob[0].speed.y=1*doubleSize;
							myMob[1].bVis=1;
							if(bSuoni)
								PlayResource(MAKEINTRESOURCE(IDR_WAVE_THRUST),FALSE);
							fuel-=10;
							InvalidateRect(hWnd,&fuelRect,TRUE);
							}
						}
					break;
				case PLAY_SPACESHIPRESCUING:
					break;
				case PLAY_SPACESHIPRISING:
					if(i==nauLChar) {
						myMob[0].speed.x=-3*doubleSize;
						}
					else if(i==nauRChar) {
						myMob[0].speed.x=+3*doubleSize;
						}
					else if(i==nauFChar) {
						myMob[0].speed.y=-3*doubleSize;
//							PlayResource(MAKEINTRESOURCE(IDR_WAVE_THRUST),FALSE);
						mp=&myMob[3];
						if(!mp->bVis) {
							mp->bVis=1;
							mp->x.whole=myMob[0].x.whole+(myMob[0].s.cx/2-2*doubleSize);
							mp->y.whole=myMob[0].y.whole-mp->s.cy+0*doubleSize;
							if(bSuoni)
								PlayResource(MAKEINTRESOURCE(IDR_WAVE_MISSILE),TRUE);
							}
						}
					break;
				case PLAY_SPACESHIPDOCKING:
					break;
				}
			break;        
		case WM_KEYUP:
			i=(TCHAR)wParam;
			switch(bPlayMode) {
				case PLAY_MOTHERSHIPWAITING:
					break;
				case PLAY_SPACESHIPFALLING:
					if(i==nauLChar || i==nauRChar) {
						myMob[0].speed.x=0;
						}
					else if(i==nauFChar) {
						myMob[0].speed.y=2*doubleSize;
						hDC=GetDC(hWnd);
						MobErase(hDC,&myMob[1]);
						ReleaseDC(hWnd,hDC);
						myMob[1].bVis=0;
						}
					break;
				case PLAY_SPACESHIPRESCUING:
					break;
				case PLAY_SPACESHIPRISING:
					if(i==nauLChar || i==nauRChar) {
						myMob[0].speed.x=0;
						}
					else if(i==nauFChar) {
						myMob[0].speed.y=-1*doubleSize;
						}
					break;
				case PLAY_SPACESHIPDOCKING:
					break;
				}
			break;        

		case WM_CREATE:
//			bInFront=GetPrivateProfileInt(APPNAME,"SempreInPrimoPiano",0,INIFile);

			srand( (unsigned)time( NULL ) );  

			doubleSize=GetPrivateProfileInt(APPNAME,"DoubleSize",1,INIFile);
			bSuoni=GetPrivateProfileInt(APPNAME,"Suoni",1,INIFile);
			AppXSize=APP_XSIZE*doubleSize;
			AppYSize=APP_YSIZE*doubleSize;

			hFont=CreateFont(8*doubleSize,4*doubleSize,0,0,FW_LIGHT,0,0,0,
				ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,DEFAULT_PITCH | FF_MODERN, (LPSTR)"Courier New");
			hFont2=CreateFont(14*doubleSize,8*doubleSize,0,0,FW_LIGHT,0,0,0,
				ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS, (LPSTR)"Free pixel regular" /*"Arial"*/);
			hFont3=CreateFont(7*doubleSize,3*doubleSize,0,0,FW_LIGHT,0,0,0,
				ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS, (LPSTR)"Free pixel regular" /*"Arial"*/);
			hTitleFont=CreateFont(18*doubleSize,9*doubleSize,0,0,FW_NORMAL,0,0,0,
				ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS, (LPSTR)"Free pixel regular" /*"Arial"*/);


			GetWindowRect(hWnd,&rc);
			rc.right=rc.left+AppXSize;
			rc.bottom=rc.top+AppYSize;
			MoveWindow(hWnd,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,TRUE);

			GetClientRect(hWnd,&rc);
			hStatusWnd = CreateWindow("static","",
				WS_BORDER | SS_LEFT | WS_CHILD,
				0,rc.bottom-16,AppXSize-GetSystemMetrics(SM_CXVSCROLL)-2*GetSystemMetrics(SM_CXSIZEFRAME),16,
				hWnd,1001,g_hinst,NULL);
			ShowWindow(hStatusWnd, SW_SHOW);
			GetClientRect(hWnd,&rc);
			AppXSizeR=rc.right-rc.left;
			AppYSizeR=rc.bottom-rc.top-16;
			SendMessage(hStatusWnd,WM_SETFONT,(WPARAM)hFont,0);
			hPen1=CreatePen(PS_SOLID,1,RGB(255,255,255));
			hPen2=CreatePen(PS_SOLID,1,RGB(0,255,0));
			br.lbStyle=BS_SOLID;
			br.lbColor=0x000000;
			br.lbHatch=0;
			hBrush=CreateBrushIndirect(&br);
			br.lbStyle=BS_SOLID;
			br.lbColor=GetSysColor(COLOR_MENU);
			br.lbHatch=0;
			hBrush2=CreateBrushIndirect(&br);
			loadMobs(PLAY_STARTING);
			hDC=GetDC(hWnd);
			hCompDC=CreateCompatibleDC(hDC);
			ReleaseDC(hWnd,hDC);

			hiScore=GetPrivateProfileInt(APPNAME,"HiScore",5000,INIFile);

			credit=0;

			hTimer=SetTimer(hWnd,1,1000/TIMER_GRANULARITY,NULL /*(TIMERPROC)myTimerProc*/);

#ifdef _DEBUG
			SetWindowText(hStatusWnd,"<debugmode>");
#endif
			break;

		case WM_TIMER:
			TimerCnt++;
			switch(bPlayMode) {
				static BYTE introScreen=0;

				case PLAY_STARTING:
				case PLAY_CHANGEPLAYER:
					subPlayModeTime=timeGetTime()+2000;
					subPlayMode=SUBPLAY_INFO;
					bPlayMode=PLAY_NEWLEVEL;
					break;

				case PLAY_NEWLEVEL:
					switch(subPlayMode) {
						case SUBPLAY_INFO:
							hDC=GetDC(hWnd);
							SetBkColor(hDC,RGB(0,0,0));
							hOldFont=SelectObject(hDC,hFont2);
							SetTextColor(hDC,RGB(255,0,255));
							wsprintf(myBuf,"PLAY PLAYER<%u>",1   );		// finire
							TextOut(hDC,AppXSizeR/4+25*doubleSize,AppYSizeR/2-25*doubleSize,myBuf,_tcslen(myBuf));
							wsprintf(myBuf,"= %u POINTS",50*currQuadro);
							SetTextColor(hDC,RGB(0,255,255));
							TextOut(hDC,AppXSizeR/4+42*doubleSize,AppYSizeR/2+1*doubleSize,myBuf,_tcslen(myBuf));
							MobSetColor(&myOmini[6],1,RGB(0,255,255),0);
							MobDrawXY(hDC,&myOmini[6],AppXSizeR/4+29*doubleSize,AppYSizeR/2+3*doubleSize);
							MobSetColor(&myMob[6],1,RGB(255,255,255),0);
							SelectObject(hDC,hOldFont);
							ReleaseDC(hWnd,hDC);
							subPlayMode=SUBPLAY_WAIT;
							break;
						case SUBPLAY_WAIT:
							if(timeGetTime() > subPlayModeTime) {
								subPlayMode=SUBPLAY_NONE;
								}
							break;
						default:
							hDC=GetDC(hWnd);
							SetTextColor(hDC,RGB(255,255,255));
							SetBkColor(hDC,RGB(0,0,0));
							hOldFont=SelectObject(hDC,hFont2);

							loadMobs(bPlayMode);
							basi[0][0]=basi[0][1]=basi[0][2]=1;			// livello più basso ossia 150 SEMPRE
							// fare casuale, o con sequenze prememorizzate
							basi[1][0]=1; basi[1][1]=1; basi[1][2]=0;
							basi[2][0]=0; basi[2][1]=1; basi[2][2]=0;
							for(i=0; i<6; i++) {			// 
								myOmini[i].bVis=1;
								}
							SelectObject(hDC,hOldFont);
							ReleaseDC(hWnd,hDC);
							fuel=800;
							ominiRescued=ominiLost=0;
							InvalidateRect(hWnd,NULL,TRUE);
							animateOmini(hWnd,5-ominiRescued);
							bPlayMode=PLAY_MOTHERSHIPWAITING;
							subPlayMode=SUBPLAY_SPACESHIPTOFALL;
							break;
						}
					break;

				case PLAY_MOTHERSHIPWAITING:
					switch(subPlayMode) {
						case SUBPLAY_NONE:
							if(!myMob[2].speed.x)
								myMob[2].speed.x=(rand() & 1 ? 2 : -2)*doubleSize;
							animateMobs(hWnd,bPlayMode);
							if(subPlayModeTime < timeGetTime()) {

		spaceshiptofall:
								bPlayMode=PLAY_SPACESHIPFALLING;

		// INSERIRE animazione portello mothership...
								hDC=GetDC(hWnd);
								MobErase(hDC,&myMob[2]);
								SelectObject(hCompDC,myMob[2].hImgAlt);
								MobDraw(hDC,&myMob[2]);
								ReleaseDC(hWnd,hDC);

								loadMobs(bPlayMode);
								myMob[2].speed.x=myMob[2].speed.y=0;
								MobSetColor(&myMob[0],1,RGB(0,255,0),0);
								myMob[0].speed.y=3*doubleSize;
								}
							if(!(TimerCnt % velocitaAsteroidi)) {
								i=animateAsteroidi(hWnd);
								}
							if(!(TimerCnt % 5)) {
								animateOmini(hWnd,5-ominiRescued);
								}
							break;
						case SUBPLAY_SPACESHIPTOFALL:
							subPlayModeTime=timeGetTime()+(16+(rand() & 7))*1000;
							subPlayMode=SUBPLAY_NONE;
							break;
						}

					if(!(TimerCnt % TIMER_GRANULARITY)) {
						playTime++;
						}
					break;

				case PLAY_SPACESHIPFALLING:
				  animateMobs(hWnd,bPlayMode);
					if(!(TimerCnt % (TIMER_GRANULARITY/8))) {	// 100mS?
						if(bSuoni)
							PlayResource(MAKEINTRESOURCE(IDR_WAVE_SHIPFALLING),FALSE);// NON fare se thrust
						}
					if(!(TimerCnt % 5)) {
						animateOmini(hWnd,5-ominiRescued);
						}
					if(!(TimerCnt % velocitaAsteroidi)) {
						i=animateAsteroidi(hWnd);
						if(i) {

spaceshipcrashed:
							totShips[0]--;
							myMob[0].speed.x=myMob[0].speed.y=0;
							myMob[1].bVis=0;
							subPlayMode=SUBPLAY_SPACESHIPBOOM;
							subPlayModeTime=timeGetTime()+3000;		// qua di +!!
							bPlayMode=PLAY_PAUSED;
							}
						}
					switch(subPlayMode) {
						case SUBPLAY_NONE:
							mp=&myMob[0];				// astronave
							j=mp->x.whole+mp->s.cx/2 ;
							if(mp->y.whole > (AppYSizeR-mp->s.cy-70*doubleSize)) {		// aggiungere "gradienti" dei gradini :D ...
								if(j < (20*doubleSize) || j > (AppXSizeR-20*doubleSize)) {		// 
									goto spaceshipcrashed;
									}
								}
							if(mp->y.whole > (BASI_AREA-mp->s.cy-1*doubleSize)) {		// 
								for(i=0; i<3; i++) {
									if(basi[2][i]) {
										if(j > (AppXSizeR/6+i*AppXSizeR/3.3-15*doubleSize) && j < (AppXSizeR/6+i*AppXSizeR/3.3+35*doubleSize)) {		// 
											basi[2][i]=0;
											score[0]+=50;
spaceshiplanded:
											mp->speed.x=mp->speed.y=0;
											bPlayMode=PLAY_SPACESHIPRESCUING;
											myOmini[5-ominiRescued].speed.x=0;
											myOmini[5-ominiRescued].speed.y=3*doubleSize;
											myOmini[5-ominiRescued].mirrorX= ominiRescued >= 3 ? 1 : 0;
											MobSetColor(&myOmini[5-ominiRescued],1,RGB(255,255,255),0);
											}
										}
									}
								}
							if(mp->y.whole > (BASI_AREA-mp->s.cy+(10-1)*doubleSize)) {		// 
								for(i=0; i<3; i++) {
									if(basi[1][i]) {
										if(j > (AppXSizeR/6+i*AppXSizeR/3.3-8*doubleSize) && j < (AppXSizeR/6+i*AppXSizeR/3.3+30*doubleSize)) {		// 
											basi[1][i]=0;
											score[0]+=100;
											goto spaceshiplanded;
											}
										}
									}
								}
							if(mp->y.whole > (BASI_AREA-mp->s.cy+(20-1)*doubleSize)) {		// 
								for(i=0; i<3; i++) {
									if(basi[0][i]) {
										if(j > (AppXSizeR/6+i*AppXSizeR/3.3) && j < (AppXSizeR/6+i*AppXSizeR/3.3+22*doubleSize)) {		// 
											basi[0][i]=0;
											score[0]+=150;
											goto spaceshiplanded;
											}
										}
									}
								}
							if(mp->y.whole > (AppYSizeR-mp->s.cy-30*doubleSize)) {		// 
								goto spaceshipcrashed;
								}
							break;
						case SUBPLAY_SPACESHIPBOOM:
							if(bSuoni)
								PlayResource(MAKEINTRESOURCE(IDR_WAVE_SHIPDESTROYED),TRUE);
							break;
						}
					if(!(TimerCnt % TIMER_GRANULARITY)) {
						playTime++;
						}
					break;

				case PLAY_SPACESHIPRESCUING:
				  animateMobs(hWnd,bPlayMode);
					if(!(TimerCnt % (TIMER_GRANULARITY/5))) {			//200mS~
						if(bSuoni)
							PlayResource(MAKEINTRESOURCE(IDR_WAVE_MANBOARDING),TRUE);
						}
					if(!(TimerCnt % (TIMER_GRANULARITY/10))) {
						i=animateOmini(hWnd,5-ominiRescued);
						switch(i) {
							case 1:
								// l'ultimo è in alto a dx, ed è il primo che va via
								myOmini[5-ominiRescued].speed.x= (ominiRescued >= 3 ? +5 : -5)*doubleSize;
								myOmini[5-ominiRescued].speed.y=0;
								break;
							case 2:
								myOmini[5-ominiRescued].speed.x=0;
								myOmini[5-ominiRescued].speed.y=-3*doubleSize;
								break;
							case 3:
								for(i=0; i<MAX_ASTEROIDI; i++) {							// trasformare asteroidi in alieni... (v.video, anche se in effetti son sempre di meno!
									if(myAsteroidi[i].bVis) {
										hDC=GetDC(hWnd);
										MobErase(hDC,&myAsteroidi[i]);
										ReleaseDC(hWnd,hDC);
										myAsteroidi[i].bVis=0;
										}
									}
								{ RECT myRect={ AppXSizeR/8+4*doubleSize,BASI_AREA,(AppXSizeR*7)/8-4*doubleSize,AppYSizeR-20*doubleSize };
									InvalidateRect(hWnd,&myRect,TRUE);		// aggiorno le Basi, ORA!
								}
								MobErase(hDC,&myOmini[5-ominiRescued]);
								myOmini[5-ominiRescued].bVis=0;
								myMob[1].bVis=1;
								myMob[0].speed.y=-1*doubleSize;
								if(!myMob[2].speed.x)		// riparte mothership
									myMob[2].speed.x=(rand() & 1 ? 2 : -2)*doubleSize;
								bPlayMode=PLAY_SPACESHIPRISING;
								loadMobs(bPlayMode);


								animateOmini(hWnd,5-ominiRescued);
								break;
							}
						}
					switch(subPlayMode) {
						}
					if(!(TimerCnt % TIMER_GRANULARITY)) {
						playTime++;
						}
					break;

				case PLAY_SPACESHIPRISING:
				  animateMobs(hWnd,bPlayMode);
					mp=&myMob[0];				// astronave
					j=mp->x.whole+mp->s.cx/2 ;
					if(mp->y.whole < (SCORE_AREA*doubleSize+myMob[2].s.cy   +26*doubleSize)) {
						myMob[2].speed.x=0;
						hDC=GetDC(hWnd);
						MobErase(hDC,&myMob[2]);
						MobSetImage(&myMob[2],0,IDB_MOTHERSHIP3);
						SelectObject(hCompDC,myMob[2].hImgAlt);
						MobDraw(hDC,&myMob[2]);
						// cambio forma e aprire porta mothership qua!
						ReleaseDC(hWnd,hDC);
						}

					if(mp->y.whole < (SCORE_AREA*doubleSize+myMob[2].s.cy   +4*doubleSize)) {

						if(j > (myMob[2].x.whole+myMob[2].s.cx/3) && j < ((myMob[2].x.whole+myMob[2].s.cx-myMob[2].s.cx/3))) {		// 

spaceshipdocked:
							hDC=GetDC(hWnd);
							MobErase(hDC,&myMob[1]);
							ReleaseDC(hWnd,hDC);
							myMob[0].speed.x=myMob[0].speed.y=0;
							myMob[1].speed.y=0;
							myMob[1].bVis=0;
							bPlayMode=PLAY_SPACESHIPDOCKING;
							subPlayMode=SUBPLAY_NONE;
							if(bSuoni)
								PlayResource(MAKEINTRESOURCE(IDR_WAVE_RESCUEOK),FALSE);
							subPlayModeTime=timeGetTime()+2000;		// 
							}
						else {
							goto spaceshipcrashed2;
							}
						}
					if(mp->y.whole < ((SCORE_AREA  +10)*doubleSize)) {
						goto spaceshipcrashed2;
						}
					if(!(TimerCnt % TIMER_GRANULARITY)) {
						playTime++;

						if(getNumBombe()<maxBombeNow && rand() > 20000) {
							POINT pos;
							if(getSuitableBomb(rand() & 1,&pos)) {		// 50% probabile contro astronave! aumentare coni quadri?
								for(i=10; i<10+maxBombeNow; i++) {			// 
									if(!myMob[i].bVis) {
										myMob[i].x.whole=pos.x+ALIEN_X_SIZE/2-0*doubleSize;
										myMob[i].y.whole=pos.y+ALIEN_Y_SIZE;
										myMob[i].bVis=1;
										myMob[i].speed.y=(3+3*(currQuadro/3))*doubleSize;
										break;
										}
									}
								}
							}

//						if(!totShips[0]) {
//							bPlayMode=PLAY_ENDING;
//							}

						}
					if(!(TimerCnt % velocitaAlieni)) {
						i=animateAlieni(hWnd);
						if(i) {

spaceshipcrashed2:
							myMob[0].speed.x=myMob[0].speed.y=0;
							myMob[1].bVis=0;
							myMob[3].bVis=0;
							if(bSuoni)
								PlayResource(MAKEINTRESOURCE(IDR_WAVE_SHIPDESTROYED),TRUE);
							subPlayMode=SUBPLAY_SPACESHIPBOOM;
							subPlayModeTime=timeGetTime()+3000;		// qua di +!!
							bPlayMode=PLAY_PAUSED;
							totShips[0]--;

//#pragma warning RIMETTERE A POSTO OMINO se schianta! o disperdere cmq
							ominiRescued++;
							ominiLost++;

							}
						}
					switch(subPlayMode) {
						case SUBPLAY_NONE:
							break;
						case SUBPLAY_ALIENBOOM:
							if(timeGetTime() > subPlayModeTime) {
								hDC=GetDC(hWnd);
								MobErase(hDC,&myAlieni[10]);
								ReleaseDC(hWnd,hDC);
								subPlayMode=SUBPLAY_NONE;
								}
							else {
								hDC=GetDC(hWnd);
								MobErase(hDC,&myAlieni[10]);
								SelectObject(hCompDC,rand() & 1 ? myAlieni[10].hImg : myAlieni[10].hImgAlt);
								i=MobGetColor(&myAlieni[subPlayData],-1);
								MobSetColor(&myAlieni[10],0,i,0);
								MobSetColor(&myAlieni[10],1,i,0);
								MobDraw(hDC,&myAlieni[10]);
								ReleaseDC(hWnd,hDC);
								}
							break;
						case SUBPLAY_SPACESHIPBOOM:
							break;
						}
					break;

				case PLAY_SPACESHIPDOCKING:
				  animateMobs(hWnd,bPlayMode);
					switch(subPlayMode) {
						default:
							if(timeGetTime() > subPlayModeTime) {
								ominiRescued++;
								InvalidateRect(hWnd,NULL,TRUE);
								if(ominiRescued==6      /*6*/) {

nuovoQuadro:

									bPlayMode=PLAY_NEWLEVEL;

									// fare conteggio a scalare omini e somma punti
//									ominiRescued=ominiRescued+1;
//									ominiLost=0;

										while((ominiRescued-ominiLost) > 0) {
											RECT scoreRect={ 0,0,AppXSizeR,(SCORE_AREA  +20)*doubleSize };
											score[0] += currQuadro*50;
											if(bSuoni)
												PlayResource(MAKEINTRESOURCE(IDR_WAVE_BEEP),TRUE);
											ominiRescued--;
											InvalidateRect(hWnd,&scoreRect,TRUE);
											Wait(400);
											}

									//bonus 500 punti? sempre??

									currQuadro++;
									currQuadro=min(currQuadro,MAX_QUADRI);		// 

									maxBombeNow=min(10,5+currQuadro/2);



									subPlayModeTime=timeGetTime()+2000;
									subPlayMode=SUBPLAY_INFO;

									// if FINITO! ...
			//incorporato						loadMobs(PLAY_MOTHERSHIPWAITING);
									velocitaAlieni=10+currQuadro*2;			// 
									}
								else {
									bPlayMode=PLAY_MOTHERSHIPWAITING;
									loadMobs(bPlayMode);
									subPlayMode=SUBPLAY_SPACESHIPTOFALL;
									}
								}
							break;
						}
					if(!(TimerCnt % TIMER_GRANULARITY)) {
						playTime++;
						}
					break;

				case PLAY_PAUSED:
					switch(subPlayMode) {
						case SUBPLAY_NONE:
							break;
						case SUBPLAY_ALIENBOOM:
							break;
						case SUBPLAY_SPACESHIPBOOM:
							if(timeGetTime() > subPlayModeTime) {
								hDC=GetDC(hWnd);
								subPlayMode=SUBPLAY_NONE;
								if(totShips[0]>0) {
									InvalidateRect(hWnd,NULL,TRUE);
									animateOmini(hWnd,5-ominiRescued);
									bPlayMode=PLAY_MOTHERSHIPWAITING;
									loadMobs(bPlayMode);
									subPlayMode=SUBPLAY_SPACESHIPTOFALL;
									}
								else
									bPlayMode=PLAY_ENDING;
								ReleaseDC(hWnd,hDC);
								}
							else {
								hDC=GetDC(hWnd);
								MobErase(hDC,&myMob[0]);
								MobSetImage(&myMob[0],IDB_SPACESHIPBOOM2,0);
								SelectObject(hCompDC,timeGetTime() & 2 ? myMob[0].hImg : myMob[0].hImgAlt);
								MobDraw(hDC,&myMob[0]);
								ReleaseDC(hWnd,hDC);
								}
							break;
						}
//					InvalidateRect(hWnd,NULL,TRUE);
					break;

				case PLAY_ENDING:
					myMob[2].speed.x=myMob[2].speed.y=0;		// mothership
					loadMobs(PLAY_ENDING);

					{ const char *s="GAME OVER",*p=s;
					int x,y;
					RECT myRect;

					hDC=GetDC(hWnd);
					SetTextColor(hDC,RGB(255,0,255));
					SetBkColor(hDC,RGB(0,0,0));
					hOldFont=SelectObject(hDC,hTitleFont);
					x=AppXSizeR/3.15;
					y=AppYSizeR/2-10*doubleSize;
					for(i=0; i<9; i++) {
						myBuf[0]=*p++; myBuf[1]=0;
						TextOut(hDC,x,y,myBuf,1);
						x+=10*doubleSize;
						myRect.left=x; myRect.top=y; myRect.right=x+8*doubleSize; myRect.bottom=y+8*doubleSize;
						InvalidateRect(hWnd,&myRect,TRUE);
						Wait(150);
						}
					SelectObject(hDC,hOldFont);
					ReleaseDC(hWnd,hDC);
					}

					if(score[0] > hiScore)
						hiScore=score[0];
					PostMessage(hWnd,WM_COMMAND,ID_FILE_CLOSE,0);
					demoTime=600;
					InvalidateRect(hWnd,NULL,TRUE);
					break;

				case PLAY_IDLE:
					if(!(TimerCnt % (TIMER_GRANULARITY/2))) {
						TimerState++;
						switch(TimerState) {
							HBITMAP hImg;
							int x,y;
							RECT myRect;

							case 0:
//								InvalidateRect(hWnd,NULL,TRUE);
								hDC=GetDC(hWnd);
								hImg=LoadBitmap(g_hinst,MAKEINTRESOURCE(IDB_LUNARRESCUE));
								SelectObject(hCompDC,hImg);
								StretchBlt(hDC,0,0,AppXSizeR,AppYSizeR,hCompDC,0,0,280 /*240*/,320,SRCCOPY);
								ReleaseDC(hWnd,hDC);
								break;
							case 1:
								InvalidateRect(hWnd,NULL,TRUE);
								break;
							case 2:
								hDC=GetDC(hWnd);
								x=AppXSizeR/3.15;
								y=80*doubleSize;
								MobCreate(&myAlieni[10],IDB_ALIENO3,IDB_ALIENO3,ALIEN_X_SIZE*doubleSize,ALIEN_Y_SIZE*doubleSize,FALSE);
								MobSetColor(&myAlieni[10],1,RGB(0,255,0),0);
								MobDrawXY(hDC,&myAlieni[10],x-60*doubleSize,y+4*doubleSize);
								MobCreate(&myAlieni[10],IDB_ALIENO2,IDB_ALIENO2,ALIEN_X_SIZE*doubleSize,ALIEN_Y_SIZE*doubleSize,FALSE);
								MobSetColor(&myAlieni[10],1,RGB(0,255,0),0);
								MobDrawXY(hDC,&myAlieni[10],x-25*doubleSize,y+5*doubleSize);
								y=110*doubleSize;
								MobCreate(&myAlieni[10],IDB_ALIENO1,IDB_ALIENO1,ALIEN_X_SIZE*doubleSize,ALIEN_Y_SIZE*doubleSize,FALSE);
								MobSetColor(&myAlieni[10],1,RGB(0,255,0),0);
								MobDrawXY(hDC,&myAlieni[10],x-59*doubleSize,y+6*doubleSize);
								y=140*doubleSize;
								hImg=LoadBitmap(g_hinst,MAKEINTRESOURCE(IDB_BASE1));
								SelectObject(hCompDC,hImg);
								StretchBlt(hDC,x-65*doubleSize,y+4*doubleSize,50*doubleSize,10*doubleSize,hCompDC,0,0,50,10,SRCCOPY);
								y=170*doubleSize;
								hImg=LoadBitmap(g_hinst,MAKEINTRESOURCE(IDB_BASE2));
								SelectObject(hCompDC,hImg);
								StretchBlt(hDC,x-55*doubleSize,y+4*doubleSize,38*doubleSize,10*doubleSize,hCompDC,0,0,38,10,SRCCOPY);
								y=200*doubleSize;
								hImg=LoadBitmap(g_hinst,MAKEINTRESOURCE(IDB_BASE3));
								SelectObject(hCompDC,hImg);
								StretchBlt(hDC,x-48*doubleSize,y+4*doubleSize,22*doubleSize,10*doubleSize,hCompDC,0,0,22,10,SRCCOPY);
								break;
							case 3:
								hDC=GetDC(hWnd);
								SetTextColor(hDC,RGB(0,255,0));
								SetBkColor(hDC,RGB(0,0,0));
								hOldFont=SelectObject(hDC,hTitleFont);
								x=AppXSizeR/3.15;
								y=80*doubleSize;
								for(i=0; i<4; i++) {
									TextOut(hDC,x,y,".",1);
									x+=18*doubleSize;
									myRect.left=x; myRect.top=y; myRect.right=x+8*doubleSize; myRect.bottom=y+8*doubleSize;
									InvalidateRect(hWnd,&myRect,TRUE);
									Wait(150);
									}
								x+=10*doubleSize;
								TextOut(hDC,x,y,"30 POINTS",9);
								SelectObject(hDC,hOldFont);
								ReleaseDC(hWnd,hDC);
								break;
							case 4:
								hDC=GetDC(hWnd);
								SetTextColor(hDC,RGB(0,255,0));
								SetBkColor(hDC,RGB(0,0,0));
								x=AppXSizeR/3.15;
								y=110*doubleSize;
								hOldFont=SelectObject(hDC,hTitleFont);
								for(i=0; i<4; i++) {
									TextOut(hDC,x,y,".",1);
									myRect.left=x; myRect.top=y; myRect.right=x+8*doubleSize; myRect.bottom=y+8*doubleSize;
									InvalidateRect(hWnd,&myRect,TRUE);
									Wait(150);
									x+=18*doubleSize;
									}
								x+=10*doubleSize;
								TextOut(hDC,x,y,"50 POINTS",9);
								SelectObject(hDC,hOldFont);
								ReleaseDC(hWnd,hDC);
								break;
							case 5:
								hDC=GetDC(hWnd);
								SetTextColor(hDC,RGB(255,0,255));
								SetBkColor(hDC,RGB(0,0,0));
								x=AppXSizeR/3.15;
								y=140*doubleSize;
								hOldFont=SelectObject(hDC,hTitleFont);
								for(i=0; i<4; i++) {
									TextOut(hDC,x,y,".",1);
									myRect.left=x; myRect.top=y; myRect.right=x+8*doubleSize; myRect.bottom=y+8*doubleSize;
									InvalidateRect(hWnd,&myRect,TRUE);
									Wait(150);
									x+=18*doubleSize;
									}
								x+=10*doubleSize;
								TextOut(hDC,x,y,"50 POINTS",9);
								SelectObject(hDC,hOldFont);
								ReleaseDC(hWnd,hDC);
								break;
							case 6:
								hDC=GetDC(hWnd);
								SetTextColor(hDC,RGB(0,255,255));
								SetBkColor(hDC,RGB(0,0,0));
								x=AppXSizeR/3.15;
								y=170*doubleSize;
								hOldFont=SelectObject(hDC,hTitleFont);
								for(i=0; i<4; i++) {
									TextOut(hDC,x,y,".",1);
									myRect.left=x; myRect.top=y; myRect.right=x+8*doubleSize; myRect.bottom=y+8*doubleSize;
									InvalidateRect(hWnd,&myRect,TRUE);
									Wait(150);
									x+=18*doubleSize;
									}
								x-=0*doubleSize;
								TextOut(hDC,x,y,"100 POINTS",10);
								SelectObject(hDC,hOldFont);
								ReleaseDC(hWnd,hDC);
								break;
							case 7:
								hDC=GetDC(hWnd);
								SetTextColor(hDC,RGB(255,255,0));
								SetBkColor(hDC,RGB(0,0,0));
								x=AppXSizeR/3.15;
								y=200*doubleSize;
								hOldFont=SelectObject(hDC,hTitleFont);
								for(i=0; i<4; i++) {
									TextOut(hDC,x,y,".",1);
									myRect.left=x; myRect.top=y; myRect.right=x+8*doubleSize; myRect.bottom=y+8*doubleSize;
									InvalidateRect(hWnd,&myRect,TRUE);
									Wait(150);
									x+=18*doubleSize;
									}
								x-=0*doubleSize;
								TextOut(hDC,x,y,"150 POINTS",10);
								SelectObject(hDC,hOldFont);
								ReleaseDC(hWnd,hDC);
								break;
							case 8:
								hDC=GetDC(hWnd);
								SetTextColor(hDC,RGB(255,0,0));
								SetBkColor(hDC,RGB(0,0,0));
								x=AppXSizeR/2.35;
								y=230*doubleSize;
								hOldFont=SelectObject(hDC,hTitleFont);
								TextOut(hDC,x,y,"1979",4);
								SelectObject(hDC,hOldFont);
								ReleaseDC(hWnd,hDC);
								break;
							case 9:
								hDC=GetDC(hWnd);
								SetTextColor(hDC,RGB(0,255,0));
								SetBkColor(hDC,RGB(0,0,0));
								x=AppXSizeR/6.5;
								y=250*doubleSize;
								hOldFont=SelectObject(hDC,hTitleFont);
								TextOut(hDC,x,y,"(C) TAITO CORPORATION",21);
								SelectObject(hDC,hOldFont);
								ReleaseDC(hWnd,hDC);
								break;
							case 10:
								hDC=GetDC(hWnd);
								SetTextColor(hDC,RGB(0,255,0));
								SetBkColor(hDC,RGB(0,0,0));
								x=AppXSizeR/8.5;
								y=270*doubleSize;
								hOldFont=SelectObject(hDC,hTitleFont);
										GetTextExtentPoint32(hDC,myBuf,strlen(myBuf),&mySize);
								TextOut(hDC,x,y,"2023 DARIO'S AUTOMATION",23);
								SelectObject(hDC,hOldFont);
								ReleaseDC(hWnd,hDC);
								break;
							case 11:
								introScreen++;
								if(introScreen>3) {
									bPlayMode=PLAY_DEMO;
									InvalidateRect(hWnd,NULL,TRUE);
									loadMobs(PLAY_STARTING);
									myMob[0].x.whole=AppXSizeR/2;
									demoTime=600;
									}
								TimerState=-1;
								break;
							}
						animateMobs(hWnd,bPlayMode);		// per stelline!
						}
					break;
				case PLAY_DEMO:
					velocitaAlieni=10;
				  animateMobs(hWnd,bPlayMode);
					if(!(TimerCnt % velocitaAlieni)) {
						animateAlieni(hWnd);
						hDC=GetDC(hWnd);

						ReleaseDC(hWnd,hDC);
						}
					hDC=GetDC(hWnd);
					if(!(TimerCnt % (TIMER_GRANULARITY/2))) {
						mp=&myMob[0];
						SelectObject(hDC,hBrush);
						PatBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx,mp->s.cy,PATCOPY);
						if(rand() > 20000) {
							if(mp->x.whole < (AppXSizeR - 20*doubleSize))
								mp->speed.x=3*doubleSize;
							else
								mp->speed.x=0;
							}
						else if(rand() > 10000) {
							if(mp->x.whole > 14*doubleSize)
								mp->speed.x=-3*doubleSize;
							else
								mp->speed.x=0;
							}
						MobDrawXY(hDC,mp,mp->x.whole,mp->y.whole);
						}
					ReleaseDC(hWnd,hDC);

					demoTime--;
					if(!demoTime) {
						introScreen=0;
						bPlayMode=PLAY_IDLE;
						InvalidateRect(hWnd,NULL,TRUE);
						}
					break;
				}
			break;

		case WM_CLOSE:
			if((bPlayMode>=PLAY_MOTHERSHIPWAITING && bPlayMode<=PLAY_SPACESHIPDOCKING)|| bPlayMode==PLAY_PAUSED || bPlayMode==PLAY_STARTING || bPlayMode==PLAY_ENDING) {
				if(MessageBox(hWnd,"Terminare la partita in corso?",APPNAME,MB_YESNO | MB_DEFBUTTON2)==IDYES)
					DestroyWindow(hWnd);
			  }
			else
				DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			WritePrivateProfileInt(APPNAME,"HiScore",hiScore,INIFile);
			WritePrivateProfileInt(APPNAME,"DoubleSize",doubleSize,INIFile);
			WritePrivateProfileInt(APPNAME,"Suoni",bSuoni,INIFile);
//			WritePrivateProfileInt(APPNAME,"SempreInPrimoPiano",bInFront,INIFile);
			// Tell WinHelp we don't need it any more...
			KillTimer(hWnd,hTimer);
	    WinHelp(hWnd,APPNAME".HLP",HELP_QUIT,(DWORD)0);
			DeleteDC(hCompDC);
			DeleteObject(hBrush);
			DeleteObject(hBrush2);
			DeleteObject(hPen1);
			DeleteObject(hPen2);
			DeleteObject(hFont);
			DeleteObject(hFont2);
			DeleteObject(hTitleFont);
			PostQuitMessage(0);
			break;

   	case WM_INITMENU:
   	  EnableMenuItem((HMENU)wParam,ID_FILE_NEW,(bPlayMode==PLAY_IDLE || bPlayMode==PLAY_DEMO) ? MF_ENABLED : MF_GRAYED);
   	  EnableMenuItem((HMENU)wParam,ID_FILE_UPDATE,((bPlayMode>=PLAY_MOTHERSHIPWAITING && bPlayMode<=PLAY_SPACESHIPDOCKING) || bPlayMode==PLAY_PAUSED) ? MF_ENABLED : MF_GRAYED);
   	  EnableMenuItem((HMENU)wParam,ID_FILE_CLOSE,((bPlayMode>=PLAY_MOTHERSHIPWAITING && bPlayMode<=PLAY_SPACESHIPDOCKING) || bPlayMode==PLAY_PAUSED) ? MF_ENABLED : MF_GRAYED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_DIMENSIONEDOPPIA,doubleSize==2 ? MF_CHECKED : MF_UNCHECKED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_SUONI,bSuoni ? MF_CHECKED : MF_UNCHECKED);
			break;

		case WM_CTLCOLORSTATIC:
			SetTextColor((HDC)wParam,GetSysColor(COLOR_WINDOWTEXT));
			SetBkColor((HDC)wParam,GetSysColor(COLOR_MENU));
			return (long)hBrush2;
			break;

		default:
			return (DefWindowProc(hWnd, message, wParam, lParam));
		}
	return (0);
	}



ATOM MyRegisterClass(CONST WNDCLASS *lpwc) {
	HANDLE  hMod;
	FARPROC proc;
	WNDCLASSEX wcex;

	hMod=GetModuleHandle("USER32");
	if(hMod != NULL) {

#if defined (UNICODE)
		proc = GetProcAddress (hMod, "RegisterClassExW");
#else
		proc = GetProcAddress (hMod, "RegisterClassExA");
#endif

		if(proc != NULL) {
			wcex.style         = lpwc->style;
			wcex.lpfnWndProc   = lpwc->lpfnWndProc;
			wcex.cbClsExtra    = lpwc->cbClsExtra;
			wcex.cbWndExtra    = lpwc->cbWndExtra;
			wcex.hInstance     = lpwc->hInstance;
			wcex.hIcon         = lpwc->hIcon;
			wcex.hCursor       = lpwc->hCursor;
			wcex.hbrBackground = lpwc->hbrBackground;
    	wcex.lpszMenuName  = lpwc->lpszMenuName;
			wcex.lpszClassName = lpwc->lpszClassName;

			// Added elements for Windows 95:
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.hIconSm = LoadIcon(wcex.hInstance, "SMALL");
			
			return (*proc)(&wcex);//return RegisterClassEx(&wcex);
		}
	}
return (RegisterClass(lpwc));
}


BOOL InitApplication(HINSTANCE hInstance) {
  WNDCLASS  wc;
//  HWND      hwnd;

  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = (WNDPROC)WndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hIcon         = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APP32));
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(GetStockObject(BLACK_BRUSH));

        // Since Windows95 has a slightly different recommended
        // format for the 'Help' menu, lets put this in the alternate menu like this:
  if(IS_WIN95) {
		wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
    } else {
	  wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
    }
  wc.lpszClassName = szAppName;

  if(IS_WIN95) {
	  if(!MyRegisterClass(&wc))
			return 0;
    }
	else {
	  if(!RegisterClass(&wc))
	  	return 0;
    }


  }

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	
	g_hinst=hInstance;

	ghWnd = CreateWindow(szAppName, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX /*| WS_MAXIMIZEBOX */ | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, AppXSize,AppYSize,
		NULL, NULL, hInstance, NULL);

	if(!ghWnd) {
		return (FALSE);
	  }

	ShowWindow(ghWnd, nCmdShow);
	UpdateWindow(ghWnd);

	return (TRUE);
  }

//
//  FUNCTION: About(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for "About" dialog box
// 		This version allows greater flexibility over the contents of the 'About' box,
// 		by pulling out values from the 'Version' resource.
//
//  MESSAGES:
//
//	WM_INITDIALOG - initialize dialog box
//	WM_COMMAND    - Input received
//
//
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	static  HFONT hfontDlg;		// Font for dialog text
	static	HFONT hFinePrint;	// Font for 'fine print' in dialog
	DWORD   dwVerInfoSize;		// Size of version information block
	LPSTR   lpVersion;			// String pointer to 'version' text
	DWORD   dwVerHnd=0;			// An 'ignored' parameter, always '0'
	UINT    uVersionLen;
	WORD    wRootLen;
	BOOL    bRetCode;
	int     i;
	char    szFullPath[256];
	char    szResult[256];
	char    szGetName[256];
	DWORD	dwVersion;
	char	szVersion[40];
	DWORD	dwResult;

	switch (message) {
    case WM_INITDIALOG:
//			ShowWindow(hDlg, SW_HIDE);
			hfontDlg = CreateFont(14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				VARIABLE_PITCH | FF_SWISS, "");
			hFinePrint = CreateFont(11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				VARIABLE_PITCH | FF_SWISS, "");
//			CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
			GetModuleFileName(g_hinst, szFullPath, sizeof(szFullPath));

			// Now lets dive in and pull out the version information:
			dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
			if(dwVerInfoSize) {
				LPSTR   lpstrVffInfo;
				HANDLE  hMem;
				hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
				lpstrVffInfo  = GlobalLock(hMem);
				GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
				// The below 'hex' value looks a little confusing, but
				// essentially what it is, is the hexidecimal representation
				// of a couple different values that represent the language
				// and character set that we are wanting string values for.
				// 040904E4 is a very common one, because it means:
				//   US English, Windows MultiLingual characterset
				// Or to pull it all apart:
				// 04------        = SUBLANG_ENGLISH_USA
				// --09----        = LANG_ENGLISH
				// ----04E4 = 1252 = Codepage for Windows:Multilingual
				lstrcpy(szGetName, "\\StringFileInfo\\040904E4\\");	 
				wRootLen = lstrlen(szGetName); // Save this position
			
				// Set the title of the dialog:
				lstrcat (szGetName, "ProductName");
				bRetCode = VerQueryValue((LPVOID)lpstrVffInfo,
					(LPSTR)szGetName,
					(LPVOID)&lpVersion,
					(UINT *)&uVersionLen);
//				lstrcpy(szResult, "About ");
//				lstrcat(szResult, lpVersion);
//				SetWindowText (hDlg, szResult);

				// Walk through the dialog items that we want to replace:
				for(i=DLG_VERFIRST; i <= DLG_VERLAST; i++) {
					GetDlgItemText(hDlg, i, szResult, sizeof(szResult));
					szGetName[wRootLen] = (char)0;
					lstrcat (szGetName, szResult);
					uVersionLen   = 0;
					lpVersion     = NULL;
					bRetCode      =  VerQueryValue((LPVOID)lpstrVffInfo,
						(LPSTR)szGetName,
						(LPVOID)&lpVersion,
						(UINT *)&uVersionLen);

					if(bRetCode && uVersionLen && lpVersion) {
					// Replace dialog item text with version info
						lstrcpy(szResult, lpVersion);
						SetDlgItemText(hDlg, i, szResult);
					  }
					else {
						dwResult = GetLastError();
						wsprintf (szResult, "Error %lu", dwResult);
						SetDlgItemText (hDlg, i, szResult);
					  }
					SendMessage (GetDlgItem (hDlg, i), WM_SETFONT, 
						(UINT)((i==DLG_VERLAST)?hFinePrint:hfontDlg),TRUE);
				  } // for


				GlobalUnlock(hMem);
				GlobalFree(hMem);

			}
		else {
				// No version information available.
			} // if (dwVerInfoSize)

    SendMessage(GetDlgItem (hDlg, IDC_LABEL), WM_SETFONT,
			(WPARAM)hfontDlg,(LPARAM)TRUE);

			// We are  using GetVersion rather then GetVersionEx
			// because earlier versions of Windows NT and Win32s
			// didn't include GetVersionEx:
			dwVersion = GetVersion();

			if(dwVersion < 0x80000000) {
				// Windows NT
				wsprintf (szVersion, "Microsoft Windows NT %u.%u (Build: %u)",
					(DWORD)(LOBYTE(LOWORD(dwVersion))),
					(DWORD)(HIBYTE(LOWORD(dwVersion))),
          (DWORD)(HIWORD(dwVersion)) );
				}
			else
				if(LOBYTE(LOWORD(dwVersion))<4) {
					// Win32s
				wsprintf (szVersion, "Microsoft Win32s %u.%u (Build: %u)",
  				(DWORD)(LOBYTE(LOWORD(dwVersion))),
					(DWORD)(HIBYTE(LOWORD(dwVersion))),
					(DWORD)(HIWORD(dwVersion) & ~0x8000) );
				}
			else {
					// Windows 95
				wsprintf(szVersion,"Microsoft Windows 95 %u.%u",
					(DWORD)(LOBYTE(LOWORD(dwVersion))),
					(DWORD)(HIBYTE(LOWORD(dwVersion))) );
				}

			SetWindowText(GetDlgItem(hDlg, IDC_OSVERSION), szVersion);
//			SetWindowPos(hDlg,NULL,GetSystemMetrics(SM_CXSCREEN)/2,GetSystemMetrics(SM_CYSCREEN)/2,0,0,SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOZORDER);
//			ShowWindow(hDlg, SW_SHOW);
			return (TRUE);

		case WM_COMMAND:
			if(wParam==IDOK || wParam==IDCANCEL) {
  		  EndDialog(hDlg,0);
			  return (TRUE);
			  }
			else if(wParam==3) {
				MessageBox(hDlg,"Se trovate utile questo programma, mandate un contributo!!\nLunt rd. 121 - L205EZ Liverpool (England)\n[Dario Greggio]","ADPM Synthesis sas",MB_OK);
			  return (TRUE);
			  }
			break;
		}

	return FALSE;
	}


BOOL PlayResource(LPSTR lpName,BOOL bStop) { 
  BOOL bRtn; 
  LPSTR lpRes; 
  HANDLE hResInfo, hRes; 

  // Find the WAVE resource.  
  hResInfo = FindResource(g_hinst, lpName, "WAVE"); 
  if(hResInfo == NULL) 
    return FALSE; 

  // Load the WAVE resource. 
  hRes = LoadResource(g_hinst, hResInfo); 
  if(hRes == NULL) 
    return FALSE; 

  // Lock the WAVE resource and play it. 
  lpRes = LockResource(hRes); 
  if(lpRes != NULL) { 
    bRtn = sndPlaySound(lpRes, SND_MEMORY | SND_ASYNC | SND_NODEFAULT | (bStop ? 0 : SND_NOSTOP));
    UnlockResource(hRes); 
		} 
  else 
    bRtn = 0; 
 
  // Free the WAVE resource and return success or failure. 
 
  FreeResource(hRes); 
  return bRtn; 
	}

int WritePrivateProfileInt(char *s, char *s1, int n, char *f) {
//  int i;
  char myBuf[16];
  
  wsprintf(myBuf,"%d",n);
  return WritePrivateProfileString(s,s1,myBuf,f);
  }

int ShowMe(void) {
	int i;
	char buffer[16];

	buffer[0]='A'^ 0x17;
	buffer[1]='D'^ 0x17;
	buffer[2]='P'^ 0x17;
	buffer[3]='M'^ 0x17;
	buffer[4]='-'^ 0x17;
	buffer[5]='G'^ 0x17;
	buffer[6]='.'^ 0x17;
	buffer[7]='D'^ 0x17;
	buffer[8]='a'^ 0x17;
	buffer[9]='r'^ 0x17;
	buffer[10]=' '^ 0x17;
	buffer[11]='2'^ 0x17;
	buffer[12]='0' ^ 0x17;
	buffer[13]=0;
	for(i=0; i<13; i++) buffer[i]^=0x17;
	return MessageBox(GetDesktopWindow(),buffer,APPNAME,MB_OK | MB_ICONEXCLAMATION);
	}


