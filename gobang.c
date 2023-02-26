// AI GoBang 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 16

void initGoBang(int *pBoard, int *pChess, int *pStep);
void drawBoard(HDC hdc,HPEN hpen);
void showPos(HWND hwnd, int pos,int step);
void showChess(HDC hdc,HPEN hpen, int *pChess,int step);
int inside(int row,int col);
int checkBoard(HDC hdc, int *pBoard, int *pChess, int step);
int getBestPos(int *pBoard,int *pChess,int step);
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow){
	static TCHAR szAppName[]=TEXT("HelloWin");
	static TCHAR szClassName[]=TEXT("HelloWinClass");
	MSG msg;
	HWND hwnd;
	WNDCLASS wndclass;
	wndclass.lpfnWndProc=WndProc;
	wndclass.style=CS_HREDRAW|CS_VREDRAW;
	wndclass.cbClsExtra=0;
	wndclass.cbWndExtra=0;
	wndclass.hInstance=hInstance;
	wndclass.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hCursor=LoadCursor(NULL,IDC_ARROW);
	wndclass.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName=NULL;
	wndclass.lpszClassName=szClassName;
	if(!RegisterClass(&wndclass)){
		MessageBox(NULL,TEXT("This programrequires Windows NT!"),szAppName,MB_ICONERROR);
		return 0;
	}
	hwnd=CreateWindow(szClassName,TEXT("MyWinChess"),WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,NULL,hInstance,NULL);
	ShowWindow(hwnd,nCmdShow);
	UpdateWindow(hwnd);
	while(GetMessage(&msg,NULL,0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);		
	}
	return msg.wParam;
	}
LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam){
	HINSTANCE hInstance;
	HDC hdc; 
	PAINTSTRUCT ps;
	RECT rect;
	char buf[100];
	int x,y,row,col,pos,dir,len;
	HPEN hOldPen;

	static int *pBoard;
	static int *pChess;
	static int step;
	static int value;

	static HPEN hpen[4];
	static HDC mdc,mdc1;
	static HBITMAP hBitmap,hBitmap1;
	static BITMAP bitmap,bitmap1;
	static BYTE *pClean;
	static HFONT hFont;

	static RECT boardRect, markRect, pathRect;
	static int boardWidth,boardHeight,markWidth,markHeight,pathWidth,pathHeight;

	switch(message){
		case WM_CREATE:
			srand(time(NULL));
			pBoard = (int*)malloc( SIZE * SIZE * sizeof(int));
			pChess = (int*)malloc( SIZE * SIZE * sizeof(int));
			boardRect.left=180;
			boardRect.top=50;
			boardRect.right=690;
			boardRect.bottom=560;
			boardWidth = 510;
			boardHeight = 510;

			markRect.left=700;
			markRect.top=50;
			markRect.right=1300;
			markRect.bottom=560;
			markWidth = 510;
			markHeight = 510;

			hdc = GetDC(hwnd);			

			mdc = CreateCompatibleDC (hdc) ;
			hBitmap = CreateCompatibleBitmap (hdc,510, 510);
			GetObject(hBitmap, sizeof(BITMAP),&bitmap);
			bitmap.bmBits = malloc(bitmap.bmWidthBytes * bitmap.bmHeight);
			memset(bitmap.bmBits,0xFF,bitmap.bmWidthBytes * bitmap.bmHeight);
			SetBitmapBits(hBitmap,bitmap.bmWidthBytes * bitmap.bmHeight,bitmap.bmBits);
			SelectObject(mdc,hBitmap);


			mdc1 = CreateCompatibleDC (hdc) ;
			hBitmap1 = CreateCompatibleBitmap (hdc,boardWidth, boardWidth);
			GetObject(hBitmap1, sizeof(BITMAP),&bitmap1);
			bitmap1.bmBits = malloc(bitmap1.bmWidthBytes * bitmap1.bmHeight);
			memset(bitmap1.bmBits,0xFF,bitmap1.bmWidthBytes * bitmap1.bmHeight);
			SetBitmapBits(hBitmap1,bitmap1.bmWidthBytes * bitmap1.bmHeight,bitmap1.bmBits);
			SelectObject(mdc1,hBitmap1);
			hFont=CreateFont(-8,-4,0,0,0,0,0,0,0,0,0,0,0,0);
			SelectObject(mdc,hFont);
			SelectObject(mdc1,hFont);

			pClean = (BYTE*)malloc(bitmap1.bmWidthBytes * bitmap1.bmHeight);
			memset(pClean,0xFF,bitmap1.bmWidthBytes * bitmap1.bmHeight);

			hpen[0] = CreatePen(PS_SOLID,3,RGB(255,0,0));
			hpen[1] = CreatePen(PS_SOLID,3,RGB(0,0,255));
			hpen[2] = CreatePen(PS_SOLID,1,RGB(0,0,0));
			hpen[3] = CreatePen(PS_DOT,7,RGB(0,255,255));

			ReleaseDC(hwnd,hdc);
			initGoBang(pBoard, pChess, &step);
			return 0;
		case WM_PAINT:
			hdc=BeginPaint(hwnd,&ps);
			BitBlt(hdc,boardRect.left,boardRect.top,boardWidth,boardWidth,mdc,0,0,SRCCOPY);
			BitBlt(hdc,markRect.left,markRect.top,markWidth,markWidth,mdc1,0,0,SRCCOPY);
			if(step){
				sprintf(buf,"side=%d value=%08X",(step-1)%2+1,value);
				TextOut(hdc,10,10+((step-1)%2+1)*20,buf,strlen(buf));
			}
			if(value%16>4){
				// return value in maxCount
				//        |                   |         |    |    |
				//  int :  xxxx xxxx xxxx xxxx|xxxx xxxx|xxxx|xxxx
				//   bit                                       0-3 count
				//   bit                                 4-7 dxdy
				//   bit                       8-15 (x,y) beginning pos
				//   bit    16-31 mark

				pos=(value & 0x0000FF00)>>8;
				row=pos>>4;col=pos&0x0F;
				dir=(value & 0x000000F0)>>4;
				len=(value & 0x0000000F)-1;
				x=row+(dir%4-1)*len;
				y=col+((dir>>2)-1)*len;
				hOldPen = SelectObject(hdc,hpen[3]);
				MoveToEx(hdc,boardRect.left+15+row*30,boardRect.top+15+col*30,NULL);
				LineTo(hdc,boardRect.left+15+x*30,boardRect.top+15+y*30);
				SelectObject(hdc,hOldPen);
				if((step-1)%2==0)
					MessageBox(NULL,"你赢了!","winner",MB_OK);
				else
					MessageBox(NULL,"电脑赢了!","winner",MB_OK);
				PostMessage(hwnd,WM_KEYDOWN,VK_F3,0);
			}
			EndPaint(hwnd,&ps);
			return 0;
		case WM_KEYDOWN:
			switch(wParam){
				case VK_ESCAPE:
					PostMessage(hwnd,WM_DESTROY,0,0);
					break;
				case VK_F3:
					value=0;
					initGoBang(pBoard, pChess, &step);
					memset(bitmap.bmBits,0xFF,bitmap.bmWidthBytes * bitmap.bmHeight);
					memset(bitmap1.bmBits,0xFF,bitmap1.bmWidthBytes * bitmap1.bmHeight);
					SetBitmapBits(hBitmap,bitmap.bmWidthBytes * bitmap.bmHeight,bitmap.bmBits);
					SetBitmapBits(hBitmap1,bitmap1.bmWidthBytes * bitmap1.bmHeight,bitmap1.bmBits);
					drawBoard(mdc,hpen[2]);  // hpen[2]:thinkness=1,color=black
					InvalidateRect(hwnd,NULL,FALSE);
					break;	
			}
			return 0;
		case WM_LBUTTONDOWN:
			if( step%2 == 0 ){  // 人 执黑 先手 落子
				x = LOWORD(lParam);
				y = HIWORD(lParam);
				if(boardRect.left<=x && x<boardRect.right && boardRect.top<y && y<boardRect.bottom){
					row=(x-180)/30;col=(y-50)/30;
					pos=row* SIZE+col;
					showPos(hwnd,pos,step);
					pChess[step] = pos;
					pBoard[pos] = step+1;					
					showChess(mdc,hpen[step%2],pChess,step);//red
					value=checkBoard(mdc1,pBoard,pChess, step);
					step++;	
					if(value%16<5)PostMessage(hwnd,WM_LBUTTONDOWN,0,0);
				}
			}else{ //电脑 执白 后手 落子
				pos = getBestPos(pBoard,pChess, step);
				showPos(hwnd,pos,step);
				pChess[step]=pos;
				pBoard[pos]=step+1;
				showChess(mdc,hpen[1],pChess, step);//blue
				value=checkBoard(mdc1,pBoard,pChess, step);
				step++;
			}
			InvalidateRect(hwnd,NULL,FALSE);
			return 0;		
		case WM_DESTROY:
			free(pBoard);
			free(pChess);
			free(bitmap.bmBits);
			free(bitmap1.bmBits);
			free(pClean);
			DeleteObject(hBitmap);
			DeleteObject(hBitmap1);
			for(int i=0;i<3;i++)DeleteObject(hpen[i]);
			PostQuitMessage(0);
			return 0;	
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
	}
	void showPos(HWND hwnd, int pos,int step){
		char buf[100];
		int row=pos/SIZE;
		int col=pos%SIZE;
		int side=step%2+1;
		sprintf(buf,"row=%d col=%d step=%d side = %d",row,col,step,side);
		SetWindowText(hwnd,buf);		
		}
	void initGoBang(int *pBoard, int *pChess, int *pStep){
		*pStep = 0;
		memset(pChess,0,SIZE*SIZE* sizeof(int));
		memset(pBoard,0,SIZE*SIZE* sizeof(int)); 
		}
	void drawBoard(HDC hdc,HPEN hpen){
		SelectObject(hdc,hpen);
		for(int i=0;i<SIZE;i++){
			MoveToEx(hdc, 15+i*30,15,NULL);
			LineTo(hdc,15+i*30,15+(SIZE-1)*30);
		}
		for(int i=0;i<SIZE;i++){
			MoveToEx(hdc, 15,15+i*30,NULL);
			LineTo(hdc,15+(SIZE-1)*30,15+i*30);
		}		
		}
	void showChess(HDC hdc, HPEN hpen,int *pChess, int step){
		char buf[100];
		SelectObject(hdc,hpen);
		int row=pChess[step] / SIZE;
		int col=pChess[step] % SIZE;
		int x=row*30;int y=col*30;
		Ellipse(hdc,3+x,y+3,x+27,y+27);
		sprintf(buf,"%d",step+1);//pBoard[pos]=step+1
		TextOut(hdc,8+x,10+y,buf,strlen(buf));
		}	
	void dispValue(int *pBoard,int *pChess,int step){
		char buf[100];
		for(int i=0;i<SIZE;i++){
			for(int j=0;j<SIZE;j++){
				
			}
		}
		}
	int checkBoard(HDC hdc, int *pBoard, int *pChess, int step){
		// return value in maxCount
		// bit 0-3 count
		// bit 4-7 dxdy
		// bit 8-15 pos beginning
		// bit 16-31 mark

		char buf[100];
		int dir[8][2]={{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1}};
		int count[8]={0,0,0,0,0,0,0,0};
		int row=pChess[step] / SIZE;
		int col=pChess[step] % SIZE;
		int side=step%2+1;//pBoard[pos]=step+1;
		int i,x,y,dx,dy;
		int maxCount=0,maxdir,mark=0;

		//count the chessmem of this side
		for(int d=0;d<8;d++){
			dx=dir[d][0];dy=dir[d][1];
			x=row+dx;y=col+dy;
			while(inside(x,y) && pBoard[x* SIZE+y] && 2-pBoard[x* SIZE+y]%2==side){
				count[d]++;
				x += dx;y += dy;
			}
		}
		//display the count
		for(i=0;i<8;i++){
			sprintf(buf,"%d ",count[i]);
			TextOut(hdc,50+i*18,50+step%2*30,buf,strlen(buf));
		}
		//get the max count and return
		for(i=0;i<4;i++)
			if(maxCount < count[i]+count[i+4]+1){
				maxCount = count[i]+count[i+4]+1;
				maxdir = i;
			}

		int posL=(row+dir[maxdir+4][0]*count[maxdir+4])*SIZE + col+dir[maxdir+4][1]*count[maxdir+4];
		maxCount=maxCount & 0x0000000F;
		maxCount = maxCount | (dir[maxdir][0]+1)<<4 |(dir[maxdir][1]+1)<<6 | posL << 8;
		return maxCount;
		}
	int inside(int row,int col){
		if(0<=row && row< SIZE && 0<=col && col< SIZE)return 1;
		else return 0;
		}
	int getBestPos(int *pBoard, int *pChess, int step){
		int row,col,pos;
		
		do{
			row=rand()% SIZE,col=rand()% SIZE;
			pos = row*SIZE+col;
		}while(pBoard[pos]!=0);

		return pos;
	}
	int maxMinAlphaBetaCut(int *pBoard, int *pChess, int step,  int depth, int *value,int alpha, int beta) {
		int bestValue, curValue;

	} 


//
/*
typedef struct tagRECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT, *PRECT, *NPRECT, *LPRECT;
BOOL InvalidateRect(
  [in] HWND       hWnd,
  [in] const RECT *lpRect,
  [in] BOOL       bErase
);
BOOL Beep(
  [in] DWORD dwFreq,
  [in] DWORD dwDuration
);
HFONT CreateFontA(
  [in] int    cHeight,
  [in] int    cWidth,
  [in] int    cEscapement,
  [in] int    cOrientation,
  [in] int    cWeight,
  [in] DWORD  bItalic,
  [in] DWORD  bUnderline,
  [in] DWORD  bStrikeOut,
  [in] DWORD  iCharSet,
  [in] DWORD  iOutPrecision,
  [in] DWORD  iClipPrecision,
  [in] DWORD  iQuality,
  [in] DWORD  iPitchAndFamily,
  [in] LPCSTR pszFaceName
);

*/
