#include "stdafx.h"
#include "FLMainToast.h"
#include "ximage.h"
#include "DualDisplayCtl.h"

#define HIDE_WND_OPERATE_TIMER      100
#define SHOW_WND_OPERATE_TIMER      101

#define CHECK_CTRL_KEY_UP_TIMER  200

#define TIMER_SP 50

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
    if (!FMWnd::Create(
        owner,
        _T("FLMainToast"),
        WS_OVERLAPPED | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN,
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST))
    {
        return 0;
    }

    ::DragAcceptFiles(m_hWnd, TRUE);

    return m_hWnd;
}

void FLMainToast::Show(bool show)
{
    if (show)
    {
        ::SetForegroundWindow(m_hWnd);
        OperateHide(false);
        
        if (m_operateShow)
            return;

        OperateShow(true);
    }

    ShowWindow(show);
}

HRESULT FLMainToast::OnTimer(WPARAM wParam, LPARAM lParam, BOOL* bHandled)
{
    switch (wParam)
    {
    case HIDE_WND_OPERATE_TIMER:
    {
        *bHandled = true;

        POINT pt;
        GetCursorPos(&pt);
        if (PtInRect(&m_position, pt))
        {
            JustShow();
            break;
        }

        int apl = m_pm.GetOpacity();
        apl -= 16;
        if (apl <= 0)
        {
            ShowWindow(false);
            if (apl <= 0)
            {
                OperateHide(false);
            }
        }
        else
        {
            m_pm.SetOpacity(apl);
        }

        return 0;
    }break;
    case SHOW_WND_OPERATE_TIMER:
    {
        *bHandled = true;
        if (!::IsWindowVisible(m_hWnd))
            ShowWindow(true);

        int apl = m_pm.GetOpacity();
        apl += 32;
        if (apl > m_maxAphle)
            apl = m_maxAphle;

        m_pm.SetOpacity(apl);
        if (apl >= m_maxAphle)
        {
            OperateShow(false);
        }

        return 0;
    }break;
    case CHECK_CTRL_KEY_UP_TIMER:
    {
        *bHandled = true;
        if (!::IsWindowVisible(m_hWnd) || !GetAsyncKeyState(VK_CONTROL))
        {
            ShowWindow(false);
            OperateCheck(false);
        }

        return 0;
    }break;
    default:
        break;
    }

    if (*bHandled)
        return 0;

    return FMWnd::OnTimer(wParam, lParam, bHandled);
}

LRESULT FLMainToast::OnNcHitTest(WPARAM wParam, LPARAM lParam, BOOL* bHandled)
{
    if (!GetAsyncKeyState(VK_LBUTTON))
    {
        int i = 0;
    }

    return FMWnd::OnNcHitTest(wParam, lParam, bHandled);
}

HRESULT FLMainToast::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* bHandled)
{
    switch (uMsg)
    {
    case WM_QUERYDRAGICON:
    {
        //return (HCURSOR)m_hIcon;
    }break;
    case WM_KILLFOCUS:
    {
        if (GetAsyncKeyState(VK_CONTROL))
        {
            //检测Control 起来
            OperateCheck(true);
        }
        else
        {
            OperateHide(true);
        }
    }break;
    case WM_MOUSEMOVE:
    {
        if (m_operateHide)
        {
            //停止隐藏，直接显示
            JustShow();
        }
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
        m_maxAphle = 200;
        OperateShow(true);

        FMWnd::InitWndAbilityManager();
    }
    else if (msg.sType == DUI_MSGTYPE_CLICK)
    {
        if (msg.pSender == m_close)
            Close(0);
    }
}

void FLMainToast::OperateHide(bool start)
{
    if (start)
    {
        ::GetWindowRect(m_hWnd, &m_position);

        if (m_operateHide)
            return;

        m_operateHide = ::SetTimer(m_hWnd, HIDE_WND_OPERATE_TIMER, TIMER_SP, NULL);
    }
    else
    {
        if (!m_operateHide)
            return;

        m_operateHide = 0;
        ::KillTimer(m_hWnd, HIDE_WND_OPERATE_TIMER);
    }
}

void FLMainToast::OperateShow(bool start)
{
    if (start)
    {
        if (m_operateShow)
            return;

        m_operateShow = ::SetTimer(m_hWnd, SHOW_WND_OPERATE_TIMER, TIMER_SP, NULL);
    }
    else
    {
        if (!m_operateShow)
            return;

        m_operateShow = 0;
        ::KillTimer(m_hWnd, SHOW_WND_OPERATE_TIMER);
    }
}

void FLMainToast::OperateCheck(bool start)
{
    if (start)
    {
        if (m_checkCtronlUp)
            return;

        m_checkCtronlUp = ::SetTimer(m_hWnd, CHECK_CTRL_KEY_UP_TIMER, TIMER_SP*2, NULL);
    }
    else
    {
        if (!m_checkCtronlUp)
            return;

        m_checkCtronlUp = 0;
        ::KillTimer(m_hWnd, CHECK_CTRL_KEY_UP_TIMER);
    }
}

void FLMainToast::JustShow()
{
    OperateHide(false);
    m_pm.SetOpacity(m_maxAphle);
    ShowWindow(true);

    //::SetFocus(m_hWnd);
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
        
        if (0)
        {
            CxImage  image(CXIMAGE_FORMAT_ICO);
            image.CreateFromHICON(ico, true);
            if (image.IsValid())
                image.Save(L"D:\\lihp\\Desktop\\111.BMP", CXIMAGE_FORMAT_BMP);
        }
    }
    ::DragFinish(hDropInfo);

}
