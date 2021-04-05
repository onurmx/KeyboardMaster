#include "framework.h"
#include "KeyboardMaster.h"
#include <stdlib.h>
#include <commdlg.h>
#include <vector>
using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name


struct MainWndInf_s {
	HWND hWnd;
	HINSTANCE hInstance;
	int nCmdShow;
};

struct ChildSquare_s {
	HWND hWnd;
	int X;
	int Y;
	int letter;
	int speed;
};


MainWndInf_s           MaindWndInf;
vector<ChildSquare_s>  childsquares;
WCHAR                  w_square_t[]              = L"SQUARECLASS";
int                    screensizeX               = GetSystemMetrics(SM_CXSCREEN);
int                    screensizeY               = GetSystemMetrics(SM_CYSCREEN);
int                    windowsizeX               = screensizeX / 2;
int                    windowsizeY               = screensizeY / 2;
RECT                   clRect;
int                    missedcounter             = 0;
int                    wrongkeycounter           = 0;
bool                   isPausedFromMenu          = false;
bool                   isPausedFromFocus         = false;
HMENU                  hMenu;
HMENU				   hPopupMenu                = CreatePopupMenu();
int                    backgroundSelection       = 3;                                   // 1-STRETCH 2-TILE 3-COLOR(DEFAULT)
CHOOSECOLOR            CHOOSECOLORDIALOG;                                               // common dialog box structure 
static COLORREF        AOFCUSTOMCOLOR[16];                                              // array of custom colors
HBRUSH                 BKGNDHBRUSH               = (HBRUSH)(COLOR_INACTIVECAPTION + 1); // brush handle
HBITMAP                SOURCEBITMAP;
OPENFILENAME           OPENFILEDIALOG            = { 0 };
TCHAR                  FILENAME[260]             = { 0 };


SIZE MeasureSize(HWND hwnd)
{
	//calculates the size of the rectangle
	SIZE size = SIZE();
	RECT rect;
	if (GetClientRect(hwnd, &rect))
	{
		size.cx = rect.right - rect.left;
		size.cy = rect.bottom - rect.top;
	}
	return size;
}

int random(HWND hWnd) {
	return rand() % (MeasureSize(hWnd).cx - 25);
}

void CreateSquares(HWND hWnd, HINSTANCE hInstance, int nCmdShow) {
	ChildSquare_s child;
	child.hWnd = CreateWindow(w_square_t,
							  L"SQUARE",
							  WS_CHILD | WS_VISIBLE,
							  child.X = random(MaindWndInf.hWnd),
							  child.Y = 0,
							  25,
							  25,
							  hWnd,
							  nullptr,
							  hInstance,
							  nullptr);
	child.letter = (rand() % 25) + 97;
	child.speed = (rand() % 5) + 1;
	childsquares.push_back(child);
}

int getIndex(ChildSquare_s wnd)
{
	for (auto itr = childsquares.begin(); itr != childsquares.end(); ++itr)
	{
		if ((*itr).hWnd == wnd.hWnd) {
			return itr - childsquares.begin();
		}
	}
}

int HexToASCII(WPARAM wParam) {
	return wParam + 32;
}

void updateTitle() {
	TCHAR s[256];
	_stprintf_s(s, 256, _T("Keyboard Master: WinAPI_2021, Missed: %d, Wrong keys: %d"), missedcounter, wrongkeycounter);
	SetWindowText(MaindWndInf.hWnd, s);
}


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM				MyRegisterClassS(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    SquareProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	// TODO: Place code here.
	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_KEYBOARDMASTER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	MyRegisterClassS(hInstance);
	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KEYBOARDMASTER));
	MSG msg;
	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	wcex.cbSize            = sizeof(WNDCLASSEX);
	wcex.style             = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc       = WndProc;
	wcex.cbClsExtra        = 0;
	wcex.cbWndExtra        = 0;
	wcex.hInstance         = hInstance;
	wcex.hIcon             = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KEYBOARDMASTER));
	wcex.hCursor           = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground     = NULL;
	wcex.lpszMenuName      = MAKEINTRESOURCEW(IDC_KEYBOARDMASTER);
	wcex.lpszClassName     = szWindowClass;
	wcex.hIconSm           = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return RegisterClassExW(&wcex);
}


ATOM MyRegisterClassS(HINSTANCE hInstance)
{
	WNDCLASSEX squareex;
	squareex.cbSize        = sizeof(WNDCLASSEX);
	squareex.style         = CS_HREDRAW | CS_VREDRAW;
	squareex.lpfnWndProc   = SquareProc;
	squareex.cbClsExtra    = 0;
	squareex.cbWndExtra    = 0;
	squareex.hInstance     = hInstance;
	squareex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KEYBOARDMASTER));
	squareex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	squareex.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	squareex.lpszMenuName  = MAKEINTRESOURCEW(IDI_KEYBOARDMASTER);
	squareex.lpszClassName = w_square_t;
	squareex.hIconSm       = LoadIcon(squareex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return RegisterClassExW(&squareex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	MaindWndInf.hWnd = CreateWindowW(szWindowClass,
									 szTitle,
									 WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
									 screensizeX / 2 - windowsizeX / 2,
									 screensizeY / 2 - windowsizeY / 2,
									 windowsizeX,
									 windowsizeY,
									 nullptr,
									 nullptr,
									 hInstance,
									 nullptr);

	SetWindowPos(MaindWndInf.hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	GetClientRect(MaindWndInf.hWnd, &clRect);

	MaindWndInf.hInstance = hInstance;
	MaindWndInf.nCmdShow = nCmdShow;

	if (!MaindWndInf.hWnd)
	{
		return FALSE;
	}
	ShowWindow(MaindWndInf.hWnd, nCmdShow);
	UpdateWindow(MaindWndInf.hWnd);
	return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case SLCCOLOR:
			// https://docs.microsoft.com/en-us/windows/win32/dlgbox/using-common-dialog-boxes
			// Initialize CHOOSECOLOR 
			ZeroMemory(&CHOOSECOLORDIALOG, sizeof(CHOOSECOLORDIALOG));
			CHOOSECOLORDIALOG.lStructSize  = sizeof(CHOOSECOLORDIALOG);
			CHOOSECOLORDIALOG.hwndOwner    = MaindWndInf.hWnd;
			CHOOSECOLORDIALOG.lpCustColors = (LPDWORD)AOFCUSTOMCOLOR;
			CHOOSECOLORDIALOG.Flags        = CC_FULLOPEN | CC_RGBINIT;
			if (ChooseColor(&CHOOSECOLORDIALOG) == TRUE)
			{
				BKGNDHBRUSH = CreateSolidBrush(CHOOSECOLORDIALOG.rgbResult);
				backgroundSelection = 3;
				SOURCEBITMAP = NULL;
				EnableMenuItem(hPopupMenu, SLCSTRETCH, MF_DISABLED);
				EnableMenuItem(hPopupMenu, SLCTILE, MF_DISABLED);
				CheckMenuItem(hPopupMenu, SLCSTRETCH, MF_UNCHECKED);
				CheckMenuItem(hPopupMenu, SLCTILE, MF_UNCHECKED);
			}
			break;
		case SLCBITMAP:
			// https://iq.direct/blog/57-displaying-open-file-dialog-using-winapi.html
			// Initialize remaining fields of OPENFILENAME structure
			OPENFILEDIALOG.lStructSize     = sizeof(OPENFILEDIALOG);
			OPENFILEDIALOG.hwndOwner       = MaindWndInf.hWnd;
			OPENFILEDIALOG.lpstrFile       = FILENAME;
			OPENFILEDIALOG.nMaxFile        = sizeof(FILENAME);
			OPENFILEDIALOG.lpstrFilter     = _T("BITMAP(*.bmp)\0*.bmp\0");
			OPENFILEDIALOG.nFilterIndex    = 1;
			OPENFILEDIALOG.lpstrFileTitle  = NULL;
			OPENFILEDIALOG.nMaxFileTitle   = 0;
			OPENFILEDIALOG.lpstrInitialDir = NULL;
			OPENFILEDIALOG.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			if (GetOpenFileName(&OPENFILEDIALOG) == TRUE)
			{
				// use ofn.lpstrFile here
				SOURCEBITMAP = (HBITMAP)LoadImage(NULL, OPENFILEDIALOG.lpstrFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
				backgroundSelection = 2;
				EnableMenuItem(hPopupMenu, SLCSTRETCH, MF_ENABLED);
				EnableMenuItem(hPopupMenu, SLCTILE, MF_ENABLED);
				CheckMenuItem(hPopupMenu, SLCSTRETCH, MF_UNCHECKED);
				CheckMenuItem(hPopupMenu, SLCTILE, MF_CHECKED);
			}
			break;
		case SLCTILE:
			if (SOURCEBITMAP != NULL) {
				backgroundSelection = 2;
				CheckMenuItem(hPopupMenu, SLCSTRETCH, MF_UNCHECKED);
				CheckMenuItem(hPopupMenu, SLCTILE, MF_CHECKED);
			}
			break;
		case SLCSTRETCH:
			if (SOURCEBITMAP != NULL) {
				backgroundSelection = 1;
				CheckMenuItem(hPopupMenu, SLCSTRETCH, MF_CHECKED);
				CheckMenuItem(hPopupMenu, SLCTILE, MF_UNCHECKED);
			}
			break;
		case ID_FILE_NEWGAME:
			missedcounter = 0;
			wrongkeycounter = 0;
			for (auto& wind : childsquares) {
				DestroyWindow(wind.hWnd);
			}
			updateTitle();
			childsquares.clear();
			break;
		case ID_FILE_PAUSE:
			isPausedFromMenu = !isPausedFromMenu;
			isPausedFromMenu ? CheckMenuItem(hMenu, ID_FILE_PAUSE, MF_CHECKED) : CheckMenuItem(hMenu, ID_FILE_PAUSE, MF_UNCHECKED);
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_CREATE:
		hMenu = GetMenu(hWnd);

		InsertMenu(hPopupMenu, 1, MF_BYPOSITION | MF_STRING, SLCBITMAP,  _T("Bitmap...\tCtrl+B"));
		InsertMenu(hPopupMenu, 2, MF_BYPOSITION | MF_STRING, SLCTILE,    _T("Tile\tCtrl+T"));
		InsertMenu(hPopupMenu, 3, MF_BYPOSITION | MF_STRING, SLCSTRETCH, _T("Stretch\tCtrl+S"));
		InsertMenu(hPopupMenu, 4, MFT_SEPARATOR, NULL, NULL);
		InsertMenu(hPopupMenu, 5, MF_BYPOSITION | MF_STRING, SLCCOLOR,   _T("Color...\tCtrl+C"));
		EnableMenuItem(hPopupMenu, SLCSTRETCH, MF_DISABLED);
		EnableMenuItem(hPopupMenu, SLCTILE, MF_DISABLED);

		SetTimer(hWnd, 1001, (rand() % 10 + 3) * 100, NULL);
		SetTimer(hWnd, 1002, 1, NULL);
		break;
	case WM_TIMER:
		if (wParam == 1001 && childsquares.size() <= 100 && !isPausedFromMenu && !isPausedFromFocus) {
			CreateSquares(MaindWndInf.hWnd, MaindWndInf.hInstance, MaindWndInf.nCmdShow);
		}
		if (wParam == 1002 && !isPausedFromMenu && !isPausedFromFocus) {
			for (auto& wind : childsquares) {
				InvalidateRect(wind.hWnd, NULL, true);
			}
			InvalidateRect(MaindWndInf.hWnd, NULL, true);
		}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...

		switch (backgroundSelection) {
		case 1:
		{
			//double buffering
			BITMAP bitmap;
			HDC HDCMEM = CreateCompatibleDC(hdc);
			HDC HDCMEM2 = CreateCompatibleDC(HDCMEM);
			HGDIOBJ OLDBITMAP = SelectObject(HDCMEM2, SOURCEBITMAP);
			HBITMAP MEMBITMAP = CreateCompatibleBitmap(hdc, MeasureSize(MaindWndInf.hWnd).cx, MeasureSize(MaindWndInf.hWnd).cy);
			SelectObject(HDCMEM, MEMBITMAP);
			GetObject(SOURCEBITMAP, sizeof(bitmap), &bitmap);
			StretchBlt(HDCMEM, 0, 0, windowsizeX, windowsizeY, HDCMEM2, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
			BitBlt(hdc, 0, 0, MeasureSize(MaindWndInf.hWnd).cx, MeasureSize(MaindWndInf.hWnd).cy, HDCMEM, 0, 0, SRCCOPY);
			SelectObject(HDCMEM, OLDBITMAP);
			DeleteObject(MEMBITMAP);
			DeleteDC(HDCMEM2);
			DeleteDC(HDCMEM);
			break;
		}
		case 2:
		{
			//double buffering
			BITMAP bitmap;
			POINT point;
			HDC HDCMEM = CreateCompatibleDC(hdc);
			HDC HDCMEM2 = CreateCompatibleDC(HDCMEM);
			HGDIOBJ OLDBITMAP = SelectObject(HDCMEM2, SOURCEBITMAP);
			HBITMAP MEMBITMAP = CreateCompatibleBitmap(hdc, MeasureSize(MaindWndInf.hWnd).cx, MeasureSize(MaindWndInf.hWnd).cy);
			SelectObject(HDCMEM, MEMBITMAP);
			GetObject(SOURCEBITMAP, sizeof(bitmap), &bitmap);
			for (int i = 0; i < windowsizeX; i += bitmap.bmWidth) {
				for (int j = 0; j < windowsizeY; j += bitmap.bmHeight) {
					point.x = i;
					point.y = j;
					BitBlt(HDCMEM, i, j, bitmap.bmWidth, bitmap.bmHeight, HDCMEM2, 0, 0, SRCCOPY);
				}
			}
			BitBlt(hdc, 0, 0, MeasureSize(MaindWndInf.hWnd).cx, MeasureSize(MaindWndInf.hWnd).cy, HDCMEM, 0, 0, SRCCOPY);
			SelectObject(HDCMEM, OLDBITMAP);
			DeleteObject(MEMBITMAP);
			DeleteDC(HDCMEM2);
			DeleteDC(HDCMEM);
			break;
		}
		case 3:
		{
			RECT AREA = { 0,0,windowsizeX,windowsizeY };
			FillRect(hdc, &AREA, BKGNDHBRUSH);
			break;
		}
		default:
		{
			break;
		}
		}

		EndPaint(hWnd, &ps);
	}
	break;
	case WM_RBUTTONDOWN:
		POINT point;
		GetCursorPos(&point);
		TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, point.x, point.y, 0, hWnd, NULL);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_SPACE && childsquares.size() > 0) {
			ChildSquare_s tmp_childsquare = childsquares[0];
			for (auto& wind : childsquares) {
				if (wind.Y > tmp_childsquare.Y) {
					tmp_childsquare = wind;
				}
			}
			DestroyWindow(tmp_childsquare.hWnd);
			childsquares.erase(childsquares.begin() + getIndex(tmp_childsquare));
		}
		if (0x41 <= (wParam) && (wParam) <= 0x5A && childsquares.size() > 0) {
			vector<ChildSquare_s> tmp_vector;
			ChildSquare_s tmp_childsquare;
			for (auto& wind : childsquares) {
				if (wind.letter == HexToASCII(wParam))
				{
					tmp_vector.push_back(wind);
					break;
				}
			}
			if (tmp_vector.size() > 0) {
				tmp_childsquare = tmp_vector[0];
				for (auto& wind : tmp_vector) {
					if (wind.Y > tmp_childsquare.Y) {
						tmp_childsquare = wind;
					}
				}
				DestroyWindow(tmp_childsquare.hWnd);
				childsquares.erase(childsquares.begin() + getIndex(tmp_childsquare));
			}
			else {
				wrongkeycounter++;
				updateTitle();
			}
		}
		break;
	case WM_ACTIVATE:
		wParam == WA_INACTIVE ? isPausedFromFocus = true : isPausedFromFocus = false;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


LRESULT CALLBACK SquareProc(HWND hBall, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		SetTimer(hBall, 2001, 1, NULL);
		break;
	}
	case WM_TIMER:
		if (wParam == 2001 && !isPausedFromMenu && !isPausedFromFocus) {
			for (auto& wind : childsquares) {
				if (wind.hWnd == hBall)
				{
					wind.Y += wind.speed;
					MoveWindow(hBall, wind.X, wind.Y, 25, 25, NULL);
					if (wind.Y + 25 > clRect.bottom) {
						DestroyWindow(wind.hWnd);
						childsquares.erase(childsquares.begin() + getIndex(wind));
						missedcounter++;
						updateTitle();
					}
				}
			}
		}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hBall, &ps);
		// TODO: Add any drawing code that uses hdc here...

		TCHAR s[2];
		RECT rc;
		for (auto& wind : childsquares) {
			if (wind.hWnd == hBall)
			{
				s[0] = wind.letter;
			}
		}
		s[1] = '\0';
		GetClientRect(hBall, &rc);
		SetTextColor(hdc, RGB(255, 255, 255));
		SetBkColor(hdc, RGB(0, 0, 0));
		DrawText(hdc, s, (int)_tcslen(s), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		EndPaint(hBall, &ps);
	}
	break;
	case WM_DESTROY:
		DestroyWindow(hBall);
		break;
	default:
		return DefWindowProc(hBall, message, wParam, lParam);
	}
	return 0;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}