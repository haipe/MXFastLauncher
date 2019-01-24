#include "stdafx.h"
#include "FLMainToast.h"
#include "ximage.h"


FLMainToast::FLMainToast()
    : FMWnd(L"FLMainToast.xml")
{
}


FLMainToast::~FLMainToast()
{
}

HWND FLMainToast::Init(HWND owner)
{
    commonutil::FMWnd::SetNotify(this);
    if (FMWnd::Create( owner, _T("FLMainToast"), WS_VISIBLE | WS_OVERLAPPED | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN, 0, { 0, 0, 0, 0 }) == NULL)
    {
        return 0;
    }

    ::DragAcceptFiles(m_hWnd, TRUE);
    // 对话框程序可在其【属性】-【行为】-【Accept Files】置为【True】，而不用调用此行。反之则可，两者可选其一嘛~~~

    return m_hWnd;
}

void FLMainToast::Show(bool show)
{
    if (show)
        CenterWindow();

    ShowWindow(show);
}

HRESULT FLMainToast::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* bHandled)
{
    switch (uMsg)
    {
    case WM_QUERYDRAGICON:
    {
        //return (HCURSOR)m_hIcon;
    }break;
    case WM_DROPFILES:
    {
        OnDropFiles((HDROP)wParam);
    }break;
    }

    return FMWnd::OnMessage(uMsg, wParam, lParam, bHandled);
}

void FLMainToast::Notify(TNotifyUI& msg)
{
    if (msg.sType == DUI_MSGTYPE_WINDOWINIT)
    {
        FMWnd::InitWndAbilityManager();
    }
    else if (msg.sType == DUI_MSGTYPE_CLICK)
    {
        if (msg.pSender == m_close)
            Close(0);
    }
}

void FLMainToast::OnDropFiles(HDROP hDropInfo)
{
    UINT  uFileCount, u;
    TCHAR cFileName[MAX_PATH];
    uFileCount = ::DragQueryFile(hDropInfo, -1, cFileName, sizeof(cFileName));   //拖拉的文件个数

    for (u = 0; u < uFileCount; u++)
    {
        ::DragQueryFile(hDropInfo, u, cFileName, sizeof(cFileName));

        //创建一个按钮，并监听双击事件

        HICON ico = ::ExtractIcon(CPaintManagerUI::GetInstance(), cFileName, 0);

        CxImage  image(CXIMAGE_FORMAT_ICO);
        image.CreateFromHICON(ico, true);
        if (image.IsValid())
        image.Save(L"D:\\lihp\\Desktop\\111.BMP", CXIMAGE_FORMAT_BMP);
    }
    ::DragFinish(hDropInfo);

}
