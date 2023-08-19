//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
#define STRICT

#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

#ifndef WIN32
#include <shell.h>
#endif

#include "afxres.h"
#include "resource.h"

extern HINSTANCE g_hinst;

union POS_FRACT {
	struct {
		unsigned int fract:2;
		unsigned int whole:14;
		};
	WORD pos;
	};
struct MOB {
	HANDLE hImg,hImgAlt;
	union POS_FRACT x,y;
	SIZE s;
	BYTE bVis;
	BYTE mirrorX,mirrorY;
	BYTE magnify;
	POINT speed;
	unsigned int punti;
	int res1;          // usato per computare speed
	HANDLE hImgOld;
	BYTE bSaveImage;		// evita flicker se non serve...
	BYTE bTransparent;		// 
  };


#define TIMER_GRANULARITY 20
#define ALIEN_X_SIZE 17		// una via di mezzo...
#define ALIEN_Y_SIZE 14
#define ASTEROID_X_SIZE 10		// 
#define ASTEROID_Y_SIZE 10
#define SCORE_AREA 40
#define ALIEN_ASTEROID_AREA 90
#define LOWER_AREA 14
#define BASI_AREA (AppYSizeR-57*doubleSize)
#define BOTTOM_AREA 37
#define MAX_SHIPS 3
#define MAX_BOMBE 5			// diciamo max 5
#define MAX_QUADRI 10  //boh
#define MAX_ASTEROIDI 20 
#define MAX_ALIENI 10 

enum PLAY_STATE {
	PLAY_IDLE=0,
	PLAY_PAUSED=-1,
	PLAY_STARTING=1,
	PLAY_CHANGEPLAYER=2,
	PLAY_NEWLEVEL=3,
	PLAY_MOTHERSHIPWAITING,
	PLAY_SPACESHIPFALLING,
	PLAY_SPACESHIPRESCUING,
	PLAY_SPACESHIPRISING,
	PLAY_SPACESHIPDOCKING,
	PLAY_ENDING,
	PLAY_DEMO=99
	};
enum SUB_PLAY_STATE {
	SUBPLAY_NONE=0,
	SUBPLAY_INIT,
	SUBPLAY_ALIENBOOM,
	SUBPLAY_SPACESHIPTOFALL,
	SUBPLAY_SPACESHIPBOOM,
	SUBPLAY_INFO,
	SUBPLAY_WAIT
	};
extern enum PLAY_STATE bPlayMode;
extern enum SUB_PLAY_STATE subPlayMode;

ATOM MyRegisterClass(CONST WNDCLASS*);
BOOL InitApplication(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About(HWND, UINT, WPARAM, LPARAM);
BOOL CenterWindow (HWND, HWND);
int WritePrivateProfileInt(char *,char *,int,char *);
BOOL PlayResource(LPSTR lpName,BOOL bStop);

int animateMobs(HWND,enum PLAY_STATE);
int animateAlieni(HWND);
int animateAsteroidi(HWND);
int animateOmini(HWND,BYTE);
int loadMobs(enum PLAY_STATE);

int MobCreate(struct MOB *mp,int img1,int img2,WORD cx,WORD cy,BYTE bSaveImage);
int MobSetImage(struct MOB *mp,int img1,int img2);
int MobErase(HDC,struct MOB *);
int MobDraw(HDC,struct MOB *);
int MobDrawXY(HDC,struct MOB *,WORD,WORD);
int MobDrawImage(HDC,struct MOB *,HANDLE);
int MobDrawImageXY(HDC,struct MOB *,HANDLE,WORD,WORD);
int MobMove(HDC,struct MOB *,SIZE);
int MobMoveXY(HDC,struct MOB *,SIZE,WORD,WORD);
int MobCollision(struct MOB *,struct MOB *);
int MobCollisionRect(struct MOB *,RECT *);
int MobCollisionPoint(struct MOB *,POINT);
BYTE MobCollisionColor(struct MOB *,DWORD,BYTE);
int MobSetColor(struct MOB *,BYTE w,COLORREF forecolor,COLORREF backcolor);
COLORREF MobGetColor(struct MOB *mp,DWORD where);

BYTE isMissileInArea(struct MOB *);
BYTE isBombaInArea(struct MOB *,struct MOB *);
int getNumBombe(void);
int getSuitableBomb(int, POINT *);
