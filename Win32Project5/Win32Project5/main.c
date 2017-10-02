#include "resource.h"
#include <Windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define		szWidth	480//定义窗口宽度
#define		szHeight	320//定义窗口高度

static HWND  hDlgModeless ;
static TCHAR szAppName[] = TEXT ("ImaCom") ;
static HWND  hDlgModeless ;
static TCHAR *succ = TEXT(" success！");
static TCHAR *fail = TEXT(" fialed！");


LRESULT     CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM) ;
BOOL         CALLBACK AboutDlgProc (HWND, UINT, WPARAM, LPARAM) ;
BOOL        PopFileUzipDlg (HWND, PTSTR, PTSTR) ;
BOOL        PopFileComDlg(HWND, PTSTR, PTSTR) ;
void init();
void OkMessage (HWND hwnd, TCHAR * szMessage, TCHAR * szTitleName);

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
    MSG	msg ;
    HWND	hwnd ;
    WNDCLASS  wndclass ;
    HBITMAP Bmp;
    HBRUSH hBrush;

    Bmp=(HBITMAP)LoadImage(NULL, L"cat.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    //说明 NULL 表示不使用内部资源文件。外部图片。 IMAGE_BITMAP表示导入的是BMP图片。LR_LOADFROMFILe表示导入外部文件

    hBrush=CreatePatternBrush(Bmp);
    wndclass.style                                       = CS_HREDRAW | CS_VREDRAW ;
    wndclass.lpfnWndProc                                 = WndProc ;
    wndclass.cbClsExtra                                 = 0 ;
    wndclass.cbWndExtra                                  = 0 ;
    wndclass.hInstance                                   = hInstance ;
    wndclass.hIcon                                       = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_ICON1)) ;
    wndclass.hCursor                                     = LoadCursor (NULL, IDC_ARROW) ;
    wndclass.hbrBackground                       = hBrush ;
    wndclass.lpszMenuName                        = szAppName ;
    wndclass.lpszClassName                       = szAppName ;

    if (!RegisterClass (&wndclass))
    {
        MessageBox (  NULL, TEXT ("This program requires Windows NT!"),
                      szAppName, MB_ICONERROR) ;
        return 0 ;
    }
    hwnd = CreateWindow (szAppName, TEXT("compressin softwre"),
                         WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT,
                         szWidth, szHeight,
                         NULL, NULL, hInstance, szCmdLine) ;
    ShowWindow (hwnd, iCmdShow) ;
    UpdateWindow (hwnd) ;
    while (GetMessage (&msg, NULL, 0, 0))
    {
        TranslateMessage (&msg) ;
        DispatchMessage (&msg) ;
    }
    return msg.message ;
}



void OkMessage (HWND hwnd, TCHAR * szMessage, TCHAR * szTitleName)

{
    TCHAR szBuffer[64 + MAX_PATH] ;
    wsprintf (szBuffer, szMessage,  szTitleName) ;
    MessageBox (hwnd, szBuffer, szAppName, MB_OK) ;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam,LPARAM lParam)
{
    static HINSTANCE hInstance;
    static TCHAR     szFileName[MAX_PATH], szTitleName[MAX_PATH] ;

    switch (message)
    {
    case WM_CREATE:
        hInstance = ((LPCREATESTRUCT) lParam)->hInstance ;
        return 0;
    case WM_COMMAND:
        switch (LOWORD (wParam))
        {
        case   ID_FILE_COMPRESSION:
            init();
            OkMessage(hwnd, TEXT("compressioin%s") ,(PopFileComDlg (hwnd, szFileName, szTitleName))?succ : fail);
            return 0 ;
        case   ID_FILE_UNLOAD:
            init();
            OkMessage(hwnd, TEXT("unload%s") ,(PopFileUzipDlg (hwnd, szFileName, szTitleName))?succ : fail);
            return 0 ;
        case   ID_EXIT:
            SendMessage (hwnd, WM_CLOSE, 0, 0) ;
            return 0 ;
        case  ID_ABOUT:
            DialogBox (hInstance, TEXT ("AboutBox"), hwnd, AboutDlgProc) ;
            return 0 ;
        }

        break;
    case   WM_DESTROY:
        PostQuitMessage (0) ;
        return 0 ;
    }

    return DefWindowProc (hwnd, message, wParam, lParam) ;
}

BOOL CALLBACK AboutDlgProc (HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case   WM_INITDIALOG:
        return TRUE ;
    case   WM_COMMAND:
        switch (LOWORD (wParam))
        {
        case IDOK:
            EndDialog (hDlg, 0) ;
            return TRUE ;
        }
        break ;
    }
    return FALSE ;
}
