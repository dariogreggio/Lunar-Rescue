#include <windows.h>
#include "LunarRescue.h"


extern struct MOB myAlieni[MAX_ALIENI+1];			// 10 + esplosione :D
extern struct MOB myAsteroidi[MAX_ASTEROIDI+1];			// 20 + esplosione :D ridi su sto cazzo :D :D :D 2023
extern struct MOB myOmini[7];			// 6 + 1 per mostrare in alto a dx i rescued
extern struct MOB myMob[20];			// astronave, missile, spaceship, 10 bombe aliene
extern struct MOB myStars[3];
extern struct MOB myMeteorite;
extern int AppXSize,AppYSize,AppXSizeR,AppYSizeR;
extern BYTE doubleSize,bSuoni;
extern BYTE totShips[2];
extern WORD score[2];
extern BYTE maxBombeNow;
extern BYTE player;
extern DWORD subPlayModeTime;
extern DWORD subPlayData;
extern WORD spaceshipScore[];
extern BYTE ominiRescued[2],ominiLost[2];
extern BYTE currQuadro[2];
extern HDC hCompDC;
extern HBRUSH hBrush,hBrush2;
extern HFONT hFont2,hFont3;

extern COLORREF colorPalette[16];

int MobCreate(struct MOB *mp,int img1,int img2,WORD cx,WORD cy,BYTE bSaveImage) {

	if(mp->hImg)
		DeleteObject(mp->hImg);
	if(mp->hImgAlt)
		DeleteObject(mp->hImgAlt);
	mp->hImg=LoadBitmap(g_hinst,MAKEINTRESOURCE(img1));
	if(img2)
		mp->hImgAlt=LoadBitmap(g_hinst,MAKEINTRESOURCE(img2));
	mp->s.cx=cx; mp->s.cy=cy;
	mp->mirrorX=mp->mirrorY=0;
	SetBitmapDimensionEx(mp->hImg,mp->s.cx,mp->s.cy,NULL);		// 
	if(mp->hImgAlt)
		SetBitmapDimensionEx(mp->hImgAlt,mp->s.cx,mp->s.cy,NULL);
	mp->hImgOld=NULL;
	mp->magnify=1;
	mp->bSaveImage=bSaveImage;
	mp->bTransparent=1;
	return 1;
	}

int MobErase(HDC hDC,struct MOB *mp) {

//	mp->bVis=0;		// mah
	if(mp->hImgOld) {
		SelectObject(hCompDC,mp->hImgOld);
		return BitBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx/**doubleSize*/,mp->s.cy/**doubleSize*/,hCompDC,0,0,SRCCOPY);
		}
	else {
		SelectObject(hDC,hBrush);
	/*	if(doubleSize==2)
			return PatBlt(hDC,mp->x,mp->y,mp->s.cx*2,mp->s.cy*2,PATCOPY);
		else*/
		return PatBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx*mp->magnify,mp->s.cy*mp->magnify,PATCOPY);
		}
	}

int MobSaveImage(HDC hDC,struct MOB *mp) {		// la farei private
	HANDLE hMyImg;

	if(!mp->hImgOld)
		mp->hImgOld=CreateCompatibleBitmap(hDC,mp->s.cx*mp->magnify /**doubleSize*/,mp->s.cy*mp->magnify /**doubleSize*/);
	hMyImg=SelectObject(hCompDC,mp->hImgOld);
	BitBlt(hCompDC,0,0,mp->s.cx*mp->magnify,mp->s.cy*mp->magnify,hDC,mp->x.whole,mp->y.whole,SRCCOPY);
	SelectObject(hCompDC,hMyImg);
	}

int MobDraw(HDC hDC,struct MOB *mp) {

	if(mp->bSaveImage)
		MobSaveImage(hDC,mp);

	SelectObject(hCompDC,mp->hImg); 
	return StretchBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx*doubleSize*mp->magnify,mp->s.cy*doubleSize*mp->magnify,
		hCompDC,mp->mirrorX ? mp->s.cx-1 : 0,mp->mirrorY ? mp->s.cy-1 : 0,mp->mirrorX ? -mp->s.cx : mp->s.cx,mp->mirrorY ? -mp->s.cy : mp->s.cy,mp->bTransparent ? SRCPAINT : SRCCOPY);
	}

int MobDrawXY(HDC hDC,struct MOB *mp,WORD x,WORD y) {

	if(mp->bSaveImage)
		MobSaveImage(hDC,mp);

	SelectObject(hCompDC,mp->hImg); 
	return StretchBlt(hDC,x,y,mp->s.cx*doubleSize*mp->magnify,mp->s.cy*doubleSize*mp->magnify,
		hCompDC,mp->mirrorX ? mp->s.cx-1 : 0,mp->mirrorY ? mp->s.cy-1 : 0,mp->mirrorX ? -mp->s.cx : mp->s.cx,mp->mirrorY ? -mp->s.cy : mp->s.cy,mp->bTransparent ? SRCPAINT : SRCCOPY);
	}

int MobDrawImage(HDC hDC,struct MOB *mp,HANDLE img) {

	if(mp->bSaveImage)
		MobSaveImage(hDC,mp);

	SelectObject(hCompDC,img);
	return StretchBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx*doubleSize*mp->magnify,mp->s.cy*doubleSize*mp->magnify,
		hCompDC,mp->mirrorX ? mp->s.cx-1 : 0,mp->mirrorY ? mp->s.cy-1 : 0,mp->mirrorX ? -mp->s.cx : mp->s.cx,mp->mirrorY ? -mp->s.cy : mp->s.cy,mp->bTransparent ? SRCPAINT : SRCCOPY);
	}

int MobDrawImageXY(HDC hDC,struct MOB *mp,HANDLE img,WORD x,WORD y) {

	if(mp->bSaveImage)
		MobSaveImage(hDC,mp);

	SelectObject(hCompDC,img);
	return StretchBlt(hDC,x,y,mp->s.cx*doubleSize*mp->magnify,mp->s.cy*doubleSize*mp->magnify,
		hCompDC,mp->mirrorX ? mp->s.cx-1 : 0,mp->mirrorY ? mp->s.cy-1 : 0,mp->mirrorX ? -mp->s.cx : mp->s.cx,mp->mirrorY ? -mp->s.cy : mp->s.cy,mp->bTransparent ? SRCPAINT : SRCCOPY);
	}

int MobMove(HDC hDC,struct MOB *mp,SIZE s) {

	if(mp->bVis) {
		MobErase(hDC,mp);
		// qua NON uso fract... (2022)
		mp->x.whole += s.cx;
		mp->y.whole += s.cy;
		return MobDraw(hDC,mp);
		}
	return 0;
	}

int MobMoveXY(HDC hDC,struct MOB *mp,SIZE s,WORD x,WORD y) {

	if(mp->bVis) {
		MobErase(hDC,mp);
		// qua NON uso fract... (2022)
		mp->x.whole = x;
		mp->y.whole = y;
		return MobDraw(hDC,mp);
		}
	return 0;
	}

int MobCollision(struct MOB *mp1,struct MOB *mp2) {
	RECT rc;
// forse dovrebbe controllare anche i singoli pixel...
	rc.left=mp2->x.whole;
	rc.right=rc.left+mp2->s.cx*mp2->magnify;
	rc.top=mp2->y.whole;
	rc.bottom=rc.top+mp2->s.cy*mp2->magnify;
	if(MobCollisionRect(mp1,&rc))
		return 1;
	return 0;
	}

int MobCollisionRect(struct MOB *mp,RECT *rc) {
	RECT rc2;

	rc2.left=mp->x.whole;
	rc2.right=rc2.left+mp->s.cx*mp->magnify;
	rc2.top=mp->y.whole;
	rc2.bottom=rc2.top+mp->s.cy*mp->magnify;

  return ! (rc2.left > rc->right || rc2.right < rc->left
        || rc2.top > rc->bottom || rc2.bottom < rc->top);
	}

int MobCollisionPoint(struct MOB *mp,POINT pt) {
	RECT rc2;
	rc2.left=mp->x.whole;
	rc2.right=rc2.left+mp->s.cx*mp->magnify;
	rc2.top=mp->y.whole;
	rc2.bottom=rc2.top+mp->s.cy*mp->magnify;

  return ! (rc2.left > pt.x || rc2.right < pt.x
        || rc2.top > pt.y || rc2.bottom < pt.y);
	}

BYTE MobCollisionColor(struct MOB *mp,DWORD color,BYTE mode) {
	RECT rc2;
	DWORD buf[128*64 /*FARE DINAMICO! mp->s.cx*mp->s.cy*/],c,*p;		// 32bpp
	int x,y;
	int i;

	rc2.left=mp->x.whole;
	rc2.right=rc2.left+mp->s.cx*mp->magnify;
	rc2.top=mp->y.whole;
	rc2.bottom=rc2.top+mp->s.cy*mp->magnify;
	//serve??

	if(!mp->bSaveImage)
		return 0;

	i=GetBitmapBits(mp->hImgOld,sizeof(buf),buf);		// questa restituisce 32bpp, vecchia storia...
	p=&buf;
	if(mode) {			// cerco il colore dato
//		i=GetObject(mp->hImgOld,sizeof(buf),buf);		// solo proprietà bitmap...
		for(y=0; y<mp->s.cy; y++) {
			for(x=0; x<mp->s.cx; x++) {
				c=*p++;
				if(c == color)
					return 1;
				}
			}
		}
	else {			// cerco qualsiasi colore diverso da quello dato
		for(y=0; y<mp->s.cy; y++) {
			for(x=0; x<mp->s.cx; x++) {
				c=*p++;
				if(c != color)
					return 1;
				}
			}
		}
	return 0;
	}

int MobSetColor(struct MOB *mp,BYTE w,COLORREF forecolor,COLORREF backcolor) {
	DWORD buf[128*64 /*FARE DINAMICO! mp->s.cx*mp->s.cy*/],c,*p;		// 32bpp
	int x,y;
	int i;

	i=GetBitmapBits(w ? mp->hImg : mp->hImgAlt,sizeof(buf),buf);		// questa restituisce 32bpp, vecchia storia...
	p=&buf;
//		i=GetObject(mp->hImgOld,sizeof(buf),buf);		// solo proprietà bitmap...
	for(y=0; y<mp->s.cy; y++) {
		for(x=0; x<mp->s.cx; x++) {
			c=*p;
			if(c)		// tutto quel che non è nero! (finire...?)
			//	mp->bTransparent;//e usare ??
				*p=RGB(GetBValue(forecolor),GetGValue(forecolor),GetRValue(forecolor));
			p++;
			}
		}

	return SetBitmapBits(w ? mp->hImg : mp->hImgAlt,sizeof(buf),buf);		// 
	}

COLORREF MobGetColor(struct MOB *mp,DWORD where) {
	DWORD buf[128*64 /*FARE DINAMICO! mp->s.cx*mp->s.cy*/],c,*p;		// 32bpp
	int i,n;

	i=GetBitmapBits(mp->hImg,sizeof(buf),buf);		// questa restituisce 32bpp, vecchia storia...
	p=&buf;
	if(where != 0xffffffff) {
		POINT pt={ LOWORD(where),HIWORD(where) };
		n=pt.y*mp->s.cx+pt.x;
		if(n<sizeof(buf)/sizeof(DWORD))
			p += n;
		}
	else {
		n=sizeof(buf)/sizeof(DWORD);
		while(*p == RGB(0,0,0) && n>=0) {
			p++; n--;
			}
		}

	return RGB(GetBValue(*p),GetGValue(*p),GetRValue(*p));
	}

int MobSetImage(struct MOB *mp,int img1,int img2) {

	if(mp->hImg && img1)
		DeleteObject(mp->hImg);
	if(mp->hImgAlt && img2)
		DeleteObject(mp->hImgAlt);
	if(img1)
		mp->hImg=LoadBitmap(g_hinst,MAKEINTRESOURCE(img1));
	if(img2)
		mp->hImgAlt=LoadBitmap(g_hinst,MAKEINTRESOURCE(img2));
	return 1;
	}




int loadMobs(enum PLAY_STATE playState) {
	int i,j;

	switch(playState) {
		case PLAY_STARTING:
		case PLAY_NEWLEVEL:

			//omini
			for(i=0; i<6; i++) {			// poi vengono attivati/disattivati dinamicamente nel gioco
				MobCreate(&myOmini[i],IDB_OMINO1,IDB_OMINO2,10*doubleSize,10*doubleSize,TRUE);
				if(i<3) {
					myOmini[i].x.whole=(i*10+11)*doubleSize;
					myOmini[i].y.whole=AppYSizeR-(28+31-11*i)*doubleSize;
					}
				else {
					myOmini[i].x.whole=AppXSizeR-(20+(5-i)*10)*doubleSize;
					myOmini[i].y.whole=AppYSizeR-(58-11*(5-i))*doubleSize;
					}
				myOmini[i].speed.y=0; myOmini[i].speed.x=0;
				myOmini[i].bVis=0;
				}
			MobCreate(&myOmini[6],IDB_OMINO2,IDB_OMINO2,10*doubleSize,10*doubleSize,TRUE);
			myOmini[6].x.whole=myOmini[6].y.whole=0;
			myOmini[6].speed.y=myOmini[6].speed.x=0;
			myOmini[6].bVis=0;
 			MobSetColor(&myOmini[6],1,RGB(255,255,255),0);

			goto mothershipwaiting;		// per ora
			break;
		case PLAY_MOTHERSHIPWAITING:
			//astronave madre
			MobCreate(&myMob[2],IDB_MOTHERSHIP1,IDB_MOTHERSHIP2,56*doubleSize,20*doubleSize,FALSE);
			myMob[2].x.whole=(AppXSizeR/2)-(myMob[2].s.cx/2)*doubleSize;
			myMob[2].y.whole=(SCORE_AREA+20)*doubleSize;
			myMob[2].speed.x=0; myMob[2].speed.y=0;
			myMob[2].bVis=1;

			//astronave
			MobCreate(&myMob[0],IDB_SPACESHIP,IDB_SPACESHIPBOOM,16*doubleSize,14*doubleSize,TRUE);
			myMob[0].x.whole=(AppXSizeR/2)+(myMob[2].s.cx/2)*doubleSize;
			myMob[0].y.whole=(SCORE_AREA+20 +2)*doubleSize;
			MobSetColor(&myMob[0],1,RGB(255,255,0),0);
			myMob[0].speed.x=0; myMob[0].speed.y=0;
			myMob[0].bVis=1;
			//razzo
			MobCreate(&myMob[1],IDB_RAZZO1,IDB_RAZZO2,8*doubleSize,10*doubleSize,TRUE);
			myMob[1].x.whole=1;
			myMob[1].y.whole=AppYSizeR-36*doubleSize;
			myMob[1].speed.x=0; myMob[1].speed.y=0;
			myMob[1].bVis=0;

mothershipwaiting:
			for(i=0; i<MAX_ALIENI; i++)
				myAlieni[i].bVis=0;
			for(i=0; i<MAX_ASTEROIDI; i++)
				myAsteroidi[i].bVis=0;
			for(i=10; i<20; i++)		// bombe
				myMob[i].bVis=0;

			for(i=0; i<MAX_ASTEROIDI; i++) {
				j=rand() & 1;
				MobCreate(&myAsteroidi[i],j ? IDB_ASTEROID2 : IDB_ASTEROID1,j ? IDB_ASTEROID2 : IDB_ASTEROID1,/*ASTEROID_X_SIZE*/12*doubleSize,ASTEROID_Y_SIZE*doubleSize,1);
				myAsteroidi[i].x.whole=10*doubleSize+(rand() % (AppXSizeR-20*doubleSize));
				myAsteroidi[i].y.whole=(ALIEN_ASTEROID_AREA+(rand() % MAX_ASTEROIDI)*((ASTEROID_Y_SIZE*4)/5))*doubleSize;
				// (in effetti ce ne possono essere 2+ sulla stessa riga...)
				MobSetColor(&myAsteroidi[i],1,colorPalette[rand() & 15],0);
				myAsteroidi[i].punti=0;
				myAsteroidi[i].speed.x=rand() & 1 ? (currQuadro[player]/2+1)*doubleSize : -(currQuadro[player]/2+1)*doubleSize; myAsteroidi[i].speed.y=0;
				myAsteroidi[i].bVis=((rand() % (MAX_QUADRI/2)) <= currQuadro[player]);
				myAsteroidi[i].mirrorX=rand() & 1;
				// e in effetti pare ci siano sempre "tutti"
				}
			MobCreate(&myAsteroidi[MAX_ASTEROIDI],IDB_ALIENOBOOM,IDB_ALIENOBOOM1,ASTEROID_X_SIZE*doubleSize,ASTEROID_Y_SIZE*doubleSize,FALSE); // mah cambiare, o non serve?
			myAsteroidi[MAX_ASTEROIDI].x.whole=0;
			myAsteroidi[MAX_ASTEROIDI].y.whole=0;
			myAsteroidi[MAX_ASTEROIDI].punti=0;
			myAsteroidi[MAX_ASTEROIDI].speed.x=0; myAlieni[MAX_ASTEROIDI].speed.y=0;
			myAsteroidi[MAX_ASTEROIDI].bVis=0;

			// stelle
			for(i=0; i<3; i++) {			// 
				MobCreate(&myStars[i],IDB_LITTLESTAR,IDB_LITTLESTAR1,4*doubleSize,4*doubleSize,FALSE);
				myStars[i].speed.y=0; myStars[i].speed.x=0;
				myStars[i].bVis=1;
				}
			myStars[0].x.whole=(AppXSizeR/8.1)+(rand() & 7)*doubleSize;
			myStars[0].y.whole=(AppYSizeR/2.1)+(rand() & 7)*doubleSize;
			myStars[1].x.whole=((AppXSizeR*4)/5.1)+(rand() & 7)*doubleSize;
			myStars[1].y.whole=(AppYSizeR/1.9)+(rand() & 7)*doubleSize;
			myStars[2].x.whole=((AppXSizeR*2)/3.1)+(rand() & 7);
			myStars[2].y.whole=((AppYSizeR*3)/4.1)+(rand() & 7);

			break;
		case PLAY_SPACESHIPFALLING:
spaceshipfalling:
			break;
		case PLAY_SPACESHIPRESCUING:
			break;
		case PLAY_SPACESHIPRISING:
			for(i=0; i<MAX_ASTEROIDI; i++)
				myAsteroidi[i].bVis=0;
			for(i=0; i<3; i++) {			// 
				myStars[i].bVis=0;
				}
			for(i=0; i<MAX_ALIENI; i++) {
				j=rand() % 3;
				MobCreate(&myAlieni[i],j ? (j==2 ? IDB_ALIENO3 : IDB_ALIENO2) : IDB_ALIENO1,j ? (j==2 ? IDB_ALIENO3 : IDB_ALIENO2) : IDB_ALIENO1,
					/*ALIEN_X_SIZE*/ (j ? (j==2 ? 21 : 16) : 14)*doubleSize,
					ALIEN_Y_SIZE*doubleSize,TRUE);
				myAlieni[i].x.whole=10*doubleSize+(rand() % (AppXSizeR-20*doubleSize));
				myAlieni[i].y.whole=(ALIEN_ASTEROID_AREA+i*(ALIEN_Y_SIZE))*doubleSize;
				MobSetColor(&myAlieni[i],1,colorPalette[rand() & 15],0);
				myAlieni[i].punti=j ? 30 : 50;
				myAlieni[i].speed.x=rand() & 1 ? (currQuadro[player]/2+2)*doubleSize : -(currQuadro[player]/2+2)*doubleSize; myAlieni[i].speed.y=0;
				myAlieni[i].bVis=1;
				}
			MobCreate(&myAlieni[MAX_ALIENI],IDB_ALIENOBOOM,IDB_ALIENOBOOM1,24 /*ALIEN_X_SIZE*/ *doubleSize,11 /*ALIEN_Y_SIZE*/ *doubleSize,FALSE);
			myAlieni[MAX_ALIENI].x.whole=0;
			myAlieni[MAX_ALIENI].y.whole=0;
			myAlieni[MAX_ALIENI].punti=0;
			myAlieni[MAX_ALIENI].speed.x=0; myAlieni[MAX_ALIENI].speed.y=0;
			myAlieni[MAX_ALIENI].bVis=0;

			MobCreate(&myMeteorite,IDB_METEOR1,IDB_METEOR2,23*doubleSize,19*doubleSize,TRUE);
			break;
		case PLAY_SPACESHIPDOCKING:
			break;
		case PLAY_ENDING:

			// spegnere tutto??

			break;
		}


#pragma warning il MISSILE e singola linea!! non bomba
	MobCreate(&myMob[3],IDB_MISSILE,IDB_MISSILE2,3*doubleSize,10*doubleSize,TRUE);
	myMob[3].x.whole=0;
	myMob[3].y.whole=150;
	myMob[3].speed.y=-4*doubleSize; myMob[3].speed.x=0;

	MobCreate(&myMob[4],IDB_SPACESHIP2,IDB_SPACESHIP2,18*doubleSize,10*doubleSize,FALSE);		// per mostrare area park
	myMob[4].x.whole=0;
	myMob[4].y.whole=0;
	myMob[4].speed.x=0; myMob[4].speed.y=0;

	//bombe
	for(i=10; i<20 /*10+MAX_BOMBE*/; i++) {			// 
#pragma warning usare 2 bombe diverse
		MobCreate(&myMob[i],IDB_BOMBA,IDB_BOMBA2,4*doubleSize,10*doubleSize,TRUE);
		myMob[i].x.whole=0;
		myMob[i].y.whole=0;
		myMob[i].speed.y=0; myMob[i].speed.x=0;
		myMob[i].bVis=0;
		}


	return 1;
	}

int animateMobs(HWND hWnd,enum PLAY_STATE playState) {
	struct MOB *mp;
	register int i,j;
	HDC hDC;
	static BYTE dividerMissile,dividerStar;

	hDC=GetDC(hWnd);

	dividerStar++;
	if(dividerStar>2) {
		for(i=0; i<3; i++) {			// stelle prima, sullo sfondo!
			mp=&myStars[i];
			MobErase(hDC,mp);
			if(mp->bVis) {
				dividerStar=0;

				if(rand() >10000) {
					// potrebbero anche sparire del tutto a tratti
					MobDrawImage(hDC,mp,rand() & 1 ? mp->hImgAlt : mp->hImg);
					}
				}
			}
		}

	mp=&myMob[2];			// mothership
	if(mp->bVis) {
		if(mp->speed.x) {
			MobErase(hDC,mp);
	//		PatBlt(hDC,mp->x,mp->y,mp->s.cx,mp->s.cy,PATCOPY);
	//		SelectObject(hCompDC,mp->hImg);
			mp->x.whole += mp->speed.x;
			if(mp->speed.x>0) {
				if(mp->x.whole > (AppXSizeR-mp->s.cx-4*doubleSize)) {
					mp->speed.x=-mp->speed.x;
					}
				}
			else {
				if(mp->x.whole < (5*doubleSize)) {
					mp->speed.x=-mp->speed.x;
					}
				}
			MobDraw(hDC,mp);
			}
		if(playState==PLAY_MOTHERSHIPWAITING) {
			MobErase(hDC,&myMob[0]);
			myMob[0].x.whole=mp->x.whole+((myMob[2].s.cx-myMob[0].s.cx)/2);
			myMob[0].y.whole=mp->y.whole+(5)*doubleSize;
			MobDraw(hDC,&myMob[0]);
			}
		}

	mp=&myMob[0];				// astronave
	if(mp->bVis) {
		if(mp->speed.x>0) {
			if(mp->x.whole > (AppXSizeR-mp->s.cx-2*doubleSize)) {
				mp->speed.x=0;
				}
			}
		else {
			if(mp->x.whole < (2*doubleSize)) {
				mp->speed.x=0;
				}
			}
		if(mp->speed.x) {
			MobErase(hDC,mp);
			mp->x.whole += mp->speed.x;
			MobDraw(hDC,mp);
			}
/*		if(mp->speed.y>0) {
			if(mp->y.whole > (AppYSizeR-mp->s.cy-4*doubleSize)) {		// finire
				mp->speed.y=0;
				}
			}
		else {
			if(mp->y.whole < ((SCORE_AREA+20 +2)*doubleSize)) {
				mp->speed.y=0;
				}
			}*/
		if(mp->speed.y) {
			MobErase(hDC,mp);
			if(myMob[1].bVis) {		// se no lascia traccia...
				MobErase(hDC,&myMob[1]);
				myMob[1].x.whole=mp->x.whole+((myMob[0].s.cx-myMob[1].s.cx)/2);
				myMob[1].y.whole=mp->y.whole+myMob[0].s.cy;
				if(myMob[0].speed.y>0)			// se scende
					MobDrawImage(hDC,&myMob[1],myMob[0].speed.y<2*doubleSize ? myMob[1].hImg : myMob[1].hImg);		// in effetti gestito con bVis..
				else		// se sale
					MobDrawImage(hDC,&myMob[1],(myMob[0].speed.x != 0 || myMob[0].speed.y<-2*doubleSize) ? myMob[1].hImgAlt : myMob[1].hImg);
				}
			mp->y.whole += mp->speed.y;
			MobDraw(hDC,mp);
			}
		}

	mp=&myMob[3];
	if(mp->bVis) {			// missile
		dividerMissile++;
		MobErase(hDC,mp);
		SelectObject(hCompDC,dividerMissile & 1 ? mp->hImgAlt : mp->hImg);
		mp->y.whole += mp->speed.y;
		if(mp->y.whole < (SCORE_AREA+40 +2)*doubleSize) {		// mothership...
			mp->bVis=0;
			}
		else {

			if(mp->bVis) {		// PRIMA PROVARE case e bombe!
				for(i=10; i<10+maxBombeNow; i++) {			// 
					if(myMob[i].bVis) {
						if(isMissileInArea(&myMob[i])) {
							mp->bVis=0;
							if((rand() % 20) > currQuadro[player]) {		// in certi casi non scoppia :D
								myMob[i].bVis=0;
								MobErase(hDC,&myMob[i]);
	//							subPlayMode=SUBPLAY_BOMBABOOM; // in teoria ci sarebbe l'esplosioncina pure della bomba...
								}
							break;
							}
						}
					}
				}

			if(mp->bVis) {		// se non ha trovato bombe ... provo alieni
				for(i=0; i<MAX_ALIENI; i++) {
					if(myAlieni[i].bVis) {
						if(isMissileInArea(&myAlieni[i])) {
							RECT scoreRect={ 0,0,AppXSize,SCORE_AREA*doubleSize };
							mp->bVis=myAlieni[i].bVis=0;
							MobErase(hDC,&myAlieni[i]);
							if(myAlieni[10].bVis) {		// se doppia esplosione di seguito!
								MobErase(hDC,&myAlieni[10]);
								}
							myAlieni[10].x.whole=myAlieni[i].x.whole;  myAlieni[10].y.whole=myAlieni[i].y.whole-1*doubleSize;
							MobDrawImage(hDC,&myAlieni[10],rand() & 1 ? myAlieni[10].hImg : myAlieni[10].hImgAlt);
//					j=StretchBlt(hDC,myAlieni[10].x.whole,myAlieni[10].y.whole,myAlieni[10].s.cx*doubleSize,myAlieni[10].s.cy*doubleSize,hCompDC,0,0,
//						myAlieni[10].s.cx,myAlieni[10].s.cy,SRCCOPY);
							subPlayMode=SUBPLAY_ALIENBOOM;
							subPlayModeTime=timeGetTime()+500;
							subPlayData=i;
							score[0]+=myAlieni[i].punti;

							if(bSuoni)
								PlayResource(MAKEINTRESOURCE(IDR_WAVE_UFOEXPLODE),TRUE);
							InvalidateRect(hWnd,&scoreRect,TRUE);
							break;
							}
						}
					}
				}

			if(mp->bVis) 
				MobDraw(hDC,mp);

			}

		}


	for(i=10; i<10+maxBombeNow; i++) {			// 
		mp=&myMob[i];
		if(mp->bVis) {
			MobErase(hDC,mp);

			if(isBombaInArea(mp,&myMob[0])) {
				RECT shipAreaRect={ 0,AppYSizeR-LOWER_AREA*doubleSize,AppXSizeR,AppYSizeR};
				myMob[0].speed.x=myMob[0].speed.y=0;
				myMob[1].bVis=0;
				subPlayMode=SUBPLAY_SPACESHIPBOOM;
				subPlayModeTime=timeGetTime()+2500;
				bPlayMode=PLAY_PAUSED;
				if(bSuoni)
					PlayResource(MAKEINTRESOURCE(IDR_WAVE_SHIPDESTROYED),TRUE);
				totShips[0]--;

// unire con di là... spaceshipcrashed2
				// if(RISING) sottinteso
				ominiRescued[player]++;
				ominiLost[player]++;

				InvalidateRect(hWnd,&shipAreaRect,TRUE);
				}


			if(mp->bVis) {
				SelectObject(hCompDC,rand() & 1 ? mp->hImgAlt : mp->hImg);
				mp->y.whole += mp->speed.y;
				if(mp->y.whole > (AppYSizeR-mp->s.cy-30*doubleSize)) {		// 

//				if(mp->y.whole > (AppYSizeR-mp->s.cy-70*doubleSize)) {		
//					if(mp->x.whole < (20*doubleSize) || mp->x.whole > (AppXSizeR-20*doubleSize)) {		// 

					// GESTIRE gradienti/gradini!

/*					if(mp->y.whole > (AppYSizeR-mp->s.cy-61*doubleSize)) {		// finire
						for(i=0; i<3; i++) {
							if(basi[2][i]) {
								if(j > (AppXSizeR/6+i*AppXSizeR/3.3-15*doubleSize) && j < (AppXSizeR/6+i*AppXSizeR/3.3+35*doubleSize)) {		// 
									}
								}
							}
						}
					if(mp->y.whole > (AppYSizeR-mp->s.cy-51*doubleSize)) {		// finire
						for(i=0; i<3; i++) {
							if(basi[1][i]) {
								if(j > (AppXSizeR/6+i*AppXSizeR/3.3-8*doubleSize) && j < (AppXSizeR/6+i*AppXSizeR/3.3+30*doubleSize)) {		// 
									}
								}
							}
						}
					if(mp->y.whole > (AppYSizeR-mp->s.cy-41*doubleSize)) {		// finire
						for(i=0; i<3; i++) {
							if(basi[0][i]) {
								if(j > (AppXSizeR/6+i*AppXSizeR/3.3) && j < (AppXSizeR/6+i*AppXSizeR/3.3+22*doubleSize)) {		// 
									}
								}
							}
						}*/


					mp->bVis=0;
					mp->speed.y=0;
					}
				else {
					MobDraw(hDC,mp);
					}
				}

			}
		}

	mp=&myMeteorite;
	if(mp->bVis) {
		MobErase(hDC,mp);
		mp->x.whole += mp->speed.x;
		mp->y.whole += mp->speed.y;
		if(mp->y.whole > ALIEN_ASTEROID_AREA+(20+(rand() & 8))*9*doubleSize) {
			mp->bVis=0;
			}
		else if(mp->y.whole > ALIEN_ASTEROID_AREA+20*8*doubleSize) {
			// DEVE cambiare dimensioni sprite! per collisione... fare in SetImage o?
			MobSetImage(mp,IDB_METEOR9,0);
			mp->s.cx=mp->s.cy=20;
			}
		else if(mp->y.whole > ALIEN_ASTEROID_AREA+20*7*doubleSize) {
			MobSetImage(mp,IDB_METEOR8,0);
			mp->s.cx=mp->s.cy=18;
			}
		else if(mp->y.whole > ALIEN_ASTEROID_AREA+20*6*doubleSize) {
			MobSetImage(mp,IDB_METEOR7,0);
			mp->s.cx=mp->s.cy=16;
			}
		else if(mp->y.whole > ALIEN_ASTEROID_AREA+20*5*doubleSize) {
			MobSetImage(mp,IDB_METEOR6,0);
			mp->s.cx=mp->s.cy=14;
			}
		else if(mp->y.whole > ALIEN_ASTEROID_AREA+20*4*doubleSize) {
			MobSetImage(mp,IDB_METEOR5,0);
			mp->s.cx=mp->s.cy=12;
			}
		else if(mp->y.whole > ALIEN_ASTEROID_AREA+20*3*doubleSize) {
			MobSetImage(mp,IDB_METEOR4,0);
			mp->s.cx=mp->s.cy=10;
			}
		else if(mp->y.whole > ALIEN_ASTEROID_AREA+20*2*doubleSize) {
			MobSetImage(mp,IDB_METEOR3,0);
			mp->s.cx=mp->s.cy=8;
			}
		else if(mp->y.whole > ALIEN_ASTEROID_AREA+20*doubleSize) {
			MobSetImage(mp,IDB_METEOR2,0);
			mp->s.cx=mp->s.cy=6;
			}
		else if(mp->y.whole > ALIEN_ASTEROID_AREA) {
			MobSetImage(mp,IDB_METEOR1,0);
			mp->s.cx=mp->s.cy=4;
			}
		if(mp->bVis)
			MobDraw(hDC,mp);
		}

	ReleaseDC(hWnd,hDC);

	return 1;
  }

int animateAlieni(HWND hWnd) {
	struct MOB *mp;
	register int i,j=0;
	HDC hDC;

	hDC=GetDC(hWnd);

	for(i=0; i<MAX_ALIENI; i++) {
		mp=&myAlieni[i];
		if(mp->bVis) {
//			PatBlt(hDC,mp->x,mp->y,mp->s.cx,mp->s.cy,PATCOPY);
			MobErase(hDC,mp);
//			mp->x+=mp->speed.x;
			}
		}


	for(i=0; i<10; i++) {
		mp=&myAlieni[i];
		if(mp->bVis) {
			mp->x.whole +=mp->speed.x;
			if(mp->speed.x>0) {
				if(mp->x.whole > (AppXSizeR-mp->s.cx-4*doubleSize)) {
					mp->x.whole = (5*doubleSize);
					}
				}
			else {
				if(mp->x.whole < (5*doubleSize)) {
					mp->x.whole = (AppXSizeR-mp->s.cx-4*doubleSize);
					}
				}
			MobDraw(hDC,mp);

			if(MobCollision(&myMob[0],mp)) {
				j=1;
				}
			}
		}

	ReleaseDC(hWnd,hDC);

	return j;
  }

int animateAsteroidi(HWND hWnd) {
	struct MOB *mp;
	register int i,j=0;
	HDC hDC;

	hDC=GetDC(hWnd);


	for(i=0; i<MAX_ASTEROIDI; i++) {
		mp=&myAsteroidi[i];
		MobErase(hDC,mp);
		if(mp->bVis) {
		//				i=DrawIcon(hDC,xPos,100,hNave1);
			mp->x.whole +=mp->speed.x;
			if(mp->speed.x>0) {
				if(mp->x.whole > (AppXSizeR-mp->s.cx-4*doubleSize)) {
					mp->x.whole = (5*doubleSize);
					}
				}
			else {
				if(mp->x.whole < (5*doubleSize)) {
					mp->x.whole = (AppXSizeR-mp->s.cx-4*doubleSize);
					}
				}
			MobDraw(hDC,mp);

			if(MobCollision(&myMob[0],mp)) {
				j=1;
				}
			}
		}

	ReleaseDC(hWnd,hDC);

	return j;
  }

int animateOmini(HWND hWnd,BYTE w) {
	struct MOB *mp;
	register int i,j=0;
	HDC hDC;
	static BYTE dividerOmini;
	HFONT hOldFont;

//#pragma warning in effetti e solo uno per volta...

	hDC=GetDC(hWnd);

//	for(i=5; i>=0; i--) {			// l'ultimo è in alto a dx, ed è il primo che va via
		mp=&myOmini[w];
		MobErase(hDC,mp);
		if(mp->bVis) {
			mp->y.whole +=mp->speed.y;
			if(mp->speed.y>0) {
				if(mp->y.whole > (AppYSizeR-BOTTOM_AREA*doubleSize)) {		// 
					mp->speed.y=0;
					j=1;
					}
				}
			else if(mp->speed.y<0) {
				if(mp->y.whole < (myMob[0].y.whole+myMob[0].s.cy -2)) {			// 
					mp->speed.y=0;
					j=3;
					}
 				}
			if(mp->speed.x) {
				mp->x.whole +=mp->speed.x;
				if((mp->x.whole  /*+mp->s.cx/2*/ ) > (myMob[0].x.whole) && (mp->x.whole) < (myMob[0].x.whole+myMob[0].s.cx)) {
					mp->x.whole=myMob[0].x.whole+myMob[0].s.cx/4;
					mp->speed.x=0;
					j=2;
					}
				}
			if(!mp->speed.x && !mp->speed.y)
				SelectObject(hCompDC,dividerOmini & 1 ? mp->hImgAlt : mp->hImg);
			else {
				MobSetImage(mp,IDB_OMINO3,IDB_OMINO3);
				MobSetColor(mp,1,RGB(255,255,255),0);
				SelectObject(hCompDC,mp->hImg);
				}

			MobDraw(hDC,mp);
			}


	for(i=w-1; i>=0; i--) {			// l'ultimo è in alto a dx, ed è il primo che va via
		mp=&myOmini[i];
		MobErase(hDC,mp);
		if(mp->bVis) {
			MobDrawImage(hDC,mp,dividerOmini & 1 ? mp->hImgAlt : mp->hImg);
			if(!myOmini[w].speed.x && !myOmini[w].speed.y) {		// se siamo in attesa, lampeggio gli HELP se no no!
				if((rand() % 10) < 4) {
					hOldFont=SelectObject(hDC,hFont3);
					SetTextColor(hDC,RGB(255,255,255));
					SetBkColor(hDC,RGB(0,0,0));
					TextOut(hDC,mp->x.whole-3,mp->y.whole-13,"HELP",4);
					SelectObject(hDC,hOldFont);
					}
				else {
					hOldFont=SelectObject(hDC,hFont3);
					SetTextColor(hDC,RGB(0,0,0));
					SetBkColor(hDC,RGB(0,0,0));
					TextOut(hDC,mp->x.whole-3,mp->y.whole-13,"HELP",4);
					SelectObject(hDC,hOldFont);
					}
				}
			}
		}


	ReleaseDC(hWnd,hDC);

	dividerOmini++;

	return j;
  }

BYTE isMissileInArea(struct MOB *mp) {		// v. MobCollisionPoint
	int x=myMob[3].x.whole+myMob[3].s.cx/2;
	int y=myMob[3].y.whole;			// centro, alto

	if(x>=(mp->x.whole) && x<(mp->x.whole+mp->s.cx) && 
		y>=(mp->y.whole) && y<(mp->y.whole+mp->s.cy)) {
		return 1;
		}
	return 0;
	}

BYTE isBombaInArea(struct MOB *m1,struct MOB *mp) {		// v. MobCollisionPoint
	int x=m1->x.whole+m1->s.cx/2;
	int y=m1->y.whole+m1->s.cy;			// centro, basso

	if(x>=(mp->x.whole) && x<(mp->x.whole+mp->s.cx) && 
		y>=(mp->y.whole) && y<(mp->y.whole+mp->s.cy)) {
		return 1;
		}
	return 0;
	}

int getNumBombe(void) {
	int i,n=0;

	for(i=10; i<10+maxBombeNow; i++) {			// 
		if(myMob[i].bVis)
			n++;
		}

	return n;
	}

int getSuitableBomb(int m,POINT *pos) {		// [0 se random o 1 se mirata a astronave] bah qua no...
	int i,f;
	int t;

	t=0;
	do {
		f=(rand() % MAX_ALIENI);

		if(myAlieni[f].bVis) {
found:
			pos->x=myAlieni[f].x.whole;
			pos->y=myAlieni[f].y.whole;
			return 1;
			}
		t++;
		} while(t<3);

	for(i=0; i<MAX_ALIENI; i++) {			// se non ne ho trovato nessuno dopo 3 tentativi, pesco il primo disponibile!
		if(myAlieni[i].bVis) {
			f=i;
			goto found;
			}
		}

	return 0;
	}



