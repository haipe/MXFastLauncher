#pragma once
#include "FMWnd.h"

class FLMainToast 
    : public commonutil::FMWnd
    , public DuiLib::INotifyUI
{
public:
    FLMainToast();
    virtual ~FLMainToast();


    HWND Init(HWND owner);
    void Show(bool show);

protected:
    virtual HRESULT OnTimer(WPARAM wParam, LPARAM lParam, BOOL* bHandled);
    virtual LRESULT OnNcHitTest(WPARAM wParam, LPARAM lParam, BOOL* bHandled);
    virtual HRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* bHandled);

    virtual void Notify(TNotifyUI& msg);

protected:
    void OperateHide(bool start);
    void OperateShow(bool start);
    void OperateCheck(bool start);

    void JustShow();

    void OnDropFiles(HDROP hDropInfo);

    DWORD       m_maxAphle = 255;
    
    DWORD m_operateHide = 0;
    DWORD m_operateShow = 0;
    DWORD m_checkCtronlUp = 0;

    CDuiRect    m_position;
};

