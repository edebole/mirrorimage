/*
	File:		mirrorimage.cpp

	Contains:	Full code

	Written by:	Raul Sobon

	Copyright:	© 2004 Raul Sobon, all rights reserved.

	PLATFORMS:	Windows 95, Windows NT, Win32s

	Change History (most recent first):









*/
#include "stdafx.h"
#include "stdio.h"
#include <stdlib.h>
#include <string>
#include "mirrorimage.h"
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// ----------- our working variables for mirroring
int		inProcess				= 0;
int		windowNumber			= 0;		// 0 or 1.
HWND	hWndSourceWindow		= NULL,
		hWndSourceWindow1		= NULL,
		hWndSourceWindow2		= NULL,
		hDlgMirror				= NULL;
BITMAP	hbmTemp;
HBITMAP TempBitMap;
int		src_w					= 640,
		src_h					= 480;
char	sourceWindowName1[256];
char	sourceWindowName2[256];


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	int argc = __argc;
	char ** argv = __argv;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MIRRORIMAGE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	int i = 0;
	while( i<argc )
	{
		if( strstr( argv[i], "-window1" ) )
		{
			sscanf( argv[i+1], "%s", &sourceWindowName1 );
		} else
		if( strstr( argv[i], "-window2" ) )
		{
			sscanf( argv[i+1], "%s", &sourceWindowName2 );
		}
		i++;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_MIRRORIMAGE);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_MIRRORIMAGE);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_MIRRORIMAGE;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 300, 230, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


void CopyMirrored( HDC hdcDest, HDC hdcSrc, RECT drc )
{
	int border = GetSystemMetrics( SM_CXBORDER )+8;

	StretchBlt( hdcDest,	
				0,0, 
				(drc.right-drc.left),	(drc.bottom-drc.top),
				hdcSrc,		
				src_w-border,0, 
				-src_w,	src_h, SRCCOPY );
}


#define	MYWAIT_EVENT		1000

/*
int GetWindowText(
  HWND hWnd,        // handle to window or control
  LPTSTR lpString,  // text buffer
  int nMaxCount     // maximum number of characters to copy
);
*/

#define	AddPopupItem( id, string )		SendMessage( GetDlgItem(hDlg, id), CB_ADDSTRING, 0, (LPARAM) string )
#define SetPopupNum( id, val )			SendMessage( GetDlgItem(hDlg, id), CB_SETCURSEL , val , 0 )
#define GetPopupNum( id )				SendMessage( GetDlgItem(hDlg, id), CB_GETCURSEL , 0, 0 )
#define GetPopupTot( id )				SendMessage( GetDlgItem(hDlg, id), CB_GETCOUNT , 0, 0 )
#define GetPopupText( id, n, ptr )		SendMessage( GetDlgItem(hDlg, id), CB_GETLBTEXT , n, (LPARAM)ptr )


int SelectComboText( HWND hDlg, long id, const char *szString )
{
	long count = GetPopupTot( id );
	while( count > 0 )
	{
		char lString[255];
		GetPopupText( id, 255, lString );
		if ( strstr( lString, szString ) )
		{
			SetPopupNum( id, count );
			return count;
		}
		count--;
	}
}


HWND FillComboWithWindowNames( HWND hDlg, long popupID )
{
	char szString[256];
	
	// get search string
	//GetDlgItemText( hDlg, IDC_WINDOWNAME, szString , sizeof(szString) );

	// find exact window name
	hWndSourceWindow = FindWindow( NULL, szString );

	if ( !hWndSourceWindow )
	{
		char windowString[256];

		// start from desktop
		hWndSourceWindow = GetDesktopWindow();

		// go to first child of desktop
		hWndSourceWindow = GetWindow( hWndSourceWindow, GW_CHILD );

		HWND hComboDlg = GetDlgItem(hDlg, popupID);
		SendMessage(hComboDlg, CB_RESETCONTENT, 0, 0); 

		int i = 0;
		// loop through this level until we find our window.
		while( hWndSourceWindow && i < 127)
		{
			GetWindowText( hWndSourceWindow, windowString, 255 );

			WINDOWINFO pwi;
			GetWindowInfo( hWndSourceWindow, &pwi );

			if( windowString[0] && windowString[0] != '&' && pwi.dwStyle & WS_SYSMENU )
			{
				SendMessage( hComboDlg, CB_INSERTSTRING, -1, (LPARAM) windowString );
				i++;
			}
			hWndSourceWindow = GetNextWindow( hWndSourceWindow, GW_HWNDNEXT );
		}
		SetPopupNum( popupID, 0 );
	}
	return hWndSourceWindow;
}



HWND FindSourceWindow( HWND hDlg, char *szString  )
{

	// if we have a search string.... proceed
	if( szString[0] )
	{
		char windowString[256];

		// start from desktop
		hWndSourceWindow = GetDesktopWindow();
		// go to first child of desktop
		hWndSourceWindow = GetWindow( hWndSourceWindow, GW_CHILD );
		// loop through this level until we find our window.
		while( hWndSourceWindow )
		{
			GetWindowText( hWndSourceWindow, windowString, 255 );
			if( strstr( windowString, szString ) )
				return hWndSourceWindow;
			hWndSourceWindow = GetNextWindow( hWndSourceWindow, GW_HWNDNEXT );
		}
	}
	return NULL;
}


int StartMirrorProcess( HWND hDlg )
{
	if( hWndSourceWindow )
	{
		HDC hdc;
		PAINTSTRUCT ps;
		RECT	rcSrc;

		hdc = BeginPaint(hDlg, &ps);
		hdc = GetDC (hDlg);

		GetWindowRect( hWndSourceWindow, &rcSrc );

		int xfluf = (2*GetSystemMetrics( SM_CXSIZEFRAME )) + GetSystemMetrics( SM_CXVSCROLL );
		int yfluf = GetSystemMetrics( SM_CYSIZEFRAME ) + GetSystemMetrics( SM_CYMENUSIZE );
		src_w = (rcSrc.right - rcSrc.left) - xfluf + 4;
		src_h = (rcSrc.bottom - rcSrc.top) - yfluf + 8 ;

		ReleaseDC( hDlg, hdc );
		EndPaint(hDlg, &ps);
		inProcess = TRUE;
	}
	return inProcess;
}

void StopMirrorProcess( HWND hWnd )
{
	inProcess = FALSE;
	DeleteObject (TempBitMap);
	KillTimer (hWnd, MYWAIT_EVENT);
	EndDialog( hDlgMirror, TRUE);
}


// copy source window in vertical slices backwards into a temp buffer.
// from the temp buffer, copy the whole region to the final output stretched.
int DoMirrorProcess( HWND hDlg )
{
    PAINTSTRUCT ps;
	HDC		hdc, hdcTemp, hdcSrc;
	RECT	rcSrc, rcDest;

	GetWindowRect( hWndSourceWindow, &rcSrc );
	GetWindowRect( hDlg, &rcDest );

	if( (rcSrc.right - rcSrc.left) > 10 &&
		(rcDest.right - rcDest.left) > 10 )
	{
		hdc = BeginPaint(hDlg, &ps);

		// prepare source
		hdc = GetDC (hDlg);					// mirror window
		hdcSrc = GetDC (hWndSourceWindow);	// source window

		SetStretchBltMode( hdc, HALFTONE );

		CopyMirrored( hdc, hdcSrc, rcDest );

		DeleteDC (hdcSrc);

		ReleaseDC( hDlg, hdc );
		EndPaint( hDlg, &ps);
	}
	return 1;
}


LONG APIENTRY MirrorProc(HWND hWnd, UINT msg, DWORD dwParam, LONG lParam)
{
	short	wParam = (short)dwParam;

    switch (msg) {
        case WM_PAINT:
            break;

		case WM_CLOSE:
			StopMirrorProcess( hWnd );
			EndDialog(hWnd, TRUE);
			return (TRUE);
			break;

		case WM_DESTROY:
			break;

        case WM_INITDIALOG:
			{
			hDlgMirror = hWnd;
			std::string wstr = "Mirror image of ";
			wstr += sourceWindowName1;
			SetWindowText( hWnd, wstr.c_str() );

			StartMirrorProcess( hWnd );

			RECT rc;
			GetWindowRect( hWnd, &rc );
			MoveWindow( hWnd, rc.left, rc.top, rc.left+src_w/2, rc.top+src_h/2, TRUE );

			SetTimer (hWnd, MYWAIT_EVENT, 40, NULL);
			}
			return FALSE;
			break;
		case WM_LBUTTONDOWN:
			if ( hWndSourceWindow2 && hWndSourceWindow2 != hWndSourceWindow1) {
				switch( windowNumber )
				{
					case 0:	windowNumber = 1;	hWndSourceWindow = hWndSourceWindow2; break;
					case 1: windowNumber = 0;	hWndSourceWindow = hWndSourceWindow1; break;
				}
			}
			break;

		case WM_TIMER:
			switch (wParam){
				case MYWAIT_EVENT:
					DoMirrorProcess( hWnd );
					break;
			}
			break;
	}
	return FALSE;
}



LONG APIENTRY MainDialogProc(HWND hWnd, UINT msg, DWORD dwParam, LONG lParam)
{
	short	wParam = (short)dwParam;
	PAINTSTRUCT ps;
	HDC hdc;
	HWND hDlg = hWnd;

    switch (msg) {
        case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			EndPaint(hWnd, &ps);
            break;

		case WM_CLOSE:
			EndDialog(hWnd, TRUE);
			return (TRUE);
			break;

		case WM_DESTROY:
			break;

        case WM_INITDIALOG:
			{
			FillComboWithWindowNames( hWnd, IDC_COMBO1 );
			FillComboWithWindowNames( hWnd, IDC_COMBO2 );

			if ( sourceWindowName1[0] )
				SelectComboText( hWnd, IDC_COMBO1, sourceWindowName1 );

			if ( sourceWindowName2[0] )
				SelectComboText( hWnd, IDC_COMBO2, sourceWindowName2 );

			//SetDlgItemText( hWnd, IDC_WINDOWNAME, "Type window's title here" );
			return TRUE;
			}
			break;

		case WM_TIMER:
			switch (wParam){
				case MYWAIT_EVENT:
					//DoMirrorProcess( hWnd );
					break;
			}
			break;

	case WM_COMMAND: 
            switch (dwParam) {
				case IDC_START:
					char szString[256];
					HWND hDlg = hWnd;
					// get search string
					GetDlgItemText( hDlg, IDC_WINDOWNAME, szString , sizeof(szString) );
					if( szString[0] )
					{
						SelectComboText( hDlg, IDC_COMBO1, szString );
						hWndSourceWindow1 = FindSourceWindow( hWnd, szString );
						strcpy( sourceWindowName1, szString );
					} else
					{
						int selA = GetPopupNum( IDC_COMBO1 );
						GetPopupText( IDC_COMBO1, selA, szString );
						if( szString[0] ){
							hWndSourceWindow1 = FindSourceWindow( hWnd, szString );
							strcpy( sourceWindowName1, szString );
						}

						int selB = GetPopupNum( IDC_COMBO2 );
						GetPopupText( IDC_COMBO2, selB, szString );
						if( szString[0] ){
							hWndSourceWindow2 = FindSourceWindow( hWnd, szString );
							strcpy( sourceWindowName2, szString );
						}

						hWndSourceWindow = hWndSourceWindow1;
						
						SetForegroundWindow( hWndSourceWindow );

					}

					if( hWndSourceWindow )
					{
						DialogBox( hInst, (LPCTSTR)IDD_MIRROR, hWnd, (DLGPROC)MirrorProc);
						FillComboWithWindowNames( hWnd, IDC_COMBO1 );
						FillComboWithWindowNames( hWnd, IDC_COMBO2 );
					} else {
						MessageBox( GetFocus(), "Cannot find source window", "Error", MB_OK|MB_ICONQUESTION );
					}
					break;
			}
	}
	return FALSE;
}



//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_CREATE:
		{
		HWND hDlg = CreateDialog(hInst, (LPCSTR)MAKEINTRESOURCE(IDD_MIRRORIMAGE_DIALOG), hWnd, (DLGPROC) MainDialogProc);
		}
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return FALSE;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
