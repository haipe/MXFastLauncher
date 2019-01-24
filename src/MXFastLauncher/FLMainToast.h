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
    virtual HRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* bHandled);

    virtual void Notify(TNotifyUI& msg);

protected:
    void OnDropFiles(HDROP hDropInfo);
};

