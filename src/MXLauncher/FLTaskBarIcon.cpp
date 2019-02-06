#include "stdafx.h"
#include "FLTaskBarIcon.h"



FLTaskBarIcon::FLTaskBarIcon()
    : MXDuiWnd(_T("FLTaskBarIcon.xml"))
{
}

FLTaskBarIcon::~FLTaskBarIcon()
{
    ::KillTimer(m_hWnd, UPDATE_POSITION_TIMER);

    m_rcMin.right += 40;
    ::MoveWindow(m_hMin, m_rcMin.left, m_rcMin.top, m_rcMin.GetWidth(), m_rcMin.GetHeight(), TRUE);
}

HWND FLTaskBarIcon::Init(HWND owner)
{
    mxtoolkit::MXDuiWnd::SetNotify(this);
    if (!MXDuiWnd::Create(owner, _T("FLTaskBarIcon"), WS_CHILD | WS_CLIPCHILDREN, 0))
    {
        return 0;
    }

    m_hTaskbar = ::FindWindow(L"Shell_TrayWnd", NULL);		//寻找类名是Shell_TrayWnd的窗口句柄
    m_hBar = ::FindWindowEx(m_hTaskbar, 0, L"ReBarWindow32", NULL);	//寻找二级容器的句柄
    m_hMin = ::FindWindowEx(m_hBar, 0, L"MSTaskSwWClass", NULL);	//寻找最小化窗口的句柄

    ::SetParent(m_hWnd, m_hBar);

    UpdatePosition();

    ::DragAcceptFiles(m_hWnd, TRUE);

    return m_hWnd;
}

void FLTaskBarIcon::Show(bool show)
{
    ShowWindow(show);
}

HRESULT FLTaskBarIcon::OnTimer(WPARAM wParam, LPARAM lParam, BOOL* bHandled)
{
    if (wParam == UPDATE_POSITION_TIMER)
    {
        UpdatePosition();
        *bHandled = true;
        return 0;
    }
    return MXDuiWnd::OnTimer(wParam, lParam, bHandled);
}

void FLTaskBarIcon::Notify(TNotifyUI& msg)
{
    if (msg.sType == DUI_MSGTYPE_WINDOWINIT)
    {
        m_taskBtn = static_cast<CButtonUI*>(m_pm.FindControl(L"TaskBarButton"));
        MXDuiWnd::InitWndAbilityManager();

        ::SetTimer(m_hWnd, UPDATE_POSITION_TIMER, 100, NULL);
    }
    else if (msg.sType == DUI_MSGTYPE_CLICK)
    {
        if (msg.pSender == m_taskBtn)
        {
            HANDLE hEvent = ::CreateEvent(NULL, TRUE, FALSE, _T("MX.Launcher.Run.Event"));

            if (hEvent)
            {
                DWORD lsEr = GetLastError();
                if (lsEr != 0)
                {
                    //已经存在的，不是创建的
                    ::SetEvent(hEvent);
                }
            }
        }
    }
}

void FLTaskBarIcon::UpdatePosition()
{
    CDuiRect rcBar, rcMin;
    ::GetWindowRect(m_hBar, &rcBar);	//获得二级容器的区域
    ::GetClientRect(m_hMin, &rcMin);	//获得最小化窗口的区域
    if ((m_rcBar.GetWidth() == rcBar.GetWidth() && m_rcBar.GetHeight() == rcBar.GetHeight()) &&
        (m_rcMin.GetWidth() == rcMin.GetWidth() && m_rcMin.GetHeight() == rcMin.GetHeight()))
        return;
    
    m_rcBar = rcBar;
    m_rcMin = rcMin;

    static const int btnSize = 32;
    //40 40
    CDuiRect dlgRc = m_rcMin;
    //水平模式
    if (m_rcBar.GetWidth() > m_rcBar.GetHeight())
    {
        //放在左边的
//         dlgRc.right = dlgRc.left + btnSize;
//         dlgRc.top = 0;
//         dlgRc.bottom = btnSize;
//         dlgRc.Offset(2, (m_rcBar.GetHeight() - btnSize) / 2);
// 
//         m_rcMin.left = dlgRc.right;

        //放在右边
        dlgRc.left = dlgRc.right - btnSize;
        dlgRc.top = 0;
        dlgRc.bottom = btnSize;
        dlgRc.Offset(0, (m_rcBar.GetHeight() - btnSize) / 2);

        m_rcMin.right = dlgRc.left;
    }
    else
    {
//         //放在上边的
//         dlgRc.bottom = dlgRc.top + btnSize;
//         dlgRc.left = 0;
//         dlgRc.right = btnSize;
//         dlgRc.Offset((m_rcBar.GetWidth() - btnSize) / 2, 2);
// 
//         m_rcMin.top = dlgRc.bottom;
    
        //放在下边
        dlgRc.top = dlgRc.bottom - btnSize;
        dlgRc.left = 0;
        dlgRc.right = btnSize;
        dlgRc.Offset((m_rcBar.GetWidth() - btnSize) / 2, 0);

        m_rcMin.bottom = dlgRc.top;
    }

    ::MoveWindow(m_hWnd, dlgRc.left, dlgRc.top, dlgRc.GetWidth(), dlgRc.GetHeight(), TRUE);
    ::MoveWindow(m_hMin, m_rcMin.left, m_rcMin.top, m_rcMin.GetWidth(), m_rcMin.GetHeight(), TRUE);
}
