#pragma once

#include "MXDuiWnd.h"

class FLTaskBarIcon
    : public mxtoolkit::MXDuiWnd
    , public DuiLib::INotifyUI
{
public:
    FLTaskBarIcon();
    virtual ~FLTaskBarIcon();

    HWND Init(HWND owner);
    void Show(bool show);

protected:
    virtual HRESULT OnTimer(WPARAM wParam, LPARAM lParam, BOOL* bHandled);

    virtual void Notify(TNotifyUI& msg);

protected:
    enum
    {
        UPDATE_POSITION_TIMER = 1000,
    };

    void UpdatePosition();


protected:
    CButtonUI*  m_taskBtn = nullptr;
    HWND m_hTaskbar = 0;		//Ѱ��������Shell_TrayWnd�Ĵ��ھ��
    HWND m_hBar = 0;	//Ѱ�Ҷ��������ľ��
    HWND m_hMin = 0;	//Ѱ����С�����ڵľ��

    CDuiRect m_rcBar;
    CDuiRect m_rcMin;
};

