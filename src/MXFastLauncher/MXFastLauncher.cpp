//MXFastLauncher.cpp: 定义应用程序的入口点。
//

#include "stdafx.h"
#include "MXFastLauncher.h"
#include "FLMainToast.h"


#ifdef _DEBUG
#pragma comment(lib,"DuiLib_d.lib")

#else							 
#pragma comment(lib,"DuiLib.lib")

#endif


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此放置代码。

    // 初始化全局字符串
    //LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    commonutil::FMWnd::s_smalIcon = LoadIcon(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(IDI_SMALL));
    commonutil::FMWnd::s_bigIcon = LoadIcon(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(IDI_MXFASTLAUNCHER));

    //初始化Duilib
    DuiLib::CPaintManagerUI::SetInstance(hInstance);
    DuiLib::CPaintManagerUI::SetResourceDll(hInstance);    
    DuiLib::CPaintManagerUI::SetResourcePath(DuiLib::CPaintManagerUI::GetInstancePath() + _T("\\Skin\\"));

    FLMainToast mainToast;
    if (mainToast.Init(GetDesktopWindow()) == 0)
        return 0;

    mainToast.CenterWindow();
    mainToast.ShowModal();

//     HWND breakWnd = (HWND)1;
//     // 主消息循环: 
//     MSG msg;
//     while (GetMessage(&msg, nullptr, 0, 0))
//     {
//         if (msg.hwnd == breakWnd && msg.message == WM_QUIT)
//             break;
// 
//         TranslateMessage(&msg);
//         DispatchMessage(&msg);
//     }
// 
//     return (int) msg.wParam;
    return 0;
}

