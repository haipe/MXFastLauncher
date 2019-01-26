#pragma once
#include "MXDuiWnd.h"
#include <list>
#include <vector>


struct LauncherInfo
{
    mxtoolkit::TString id;     //guid
    mxtoolkit::TString name;   //名称
    mxtoolkit::TString path;   //路径
    mxtoolkit::TString param;  //参数
    mxtoolkit::TString image;  //缩略图路径
};

typedef std::list<LauncherInfo>         LauncherInfoList;
typedef std::vector<LauncherInfoList>   LauncherInfoListContainer;



class FLMainToast 
    : public mxtoolkit::MXDuiWnd
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
    void LoadConfig();
    void SaveConfig();


protected:
    void OperateHide(bool start);
    void OperateShow(bool start);
    void OperateCheck(bool start);

    void JustShow();

    void OnDropFiles(HDROP hDropInfo);

private:
    DWORD m_maxAphle = 255;
    
    DWORD m_operateHide = 0;
    DWORD m_operateShow = 0;
    DWORD m_checkCtronlUp = 0;

    CDuiRect m_position;

private:
    mxtoolkit::TString m_dataDir;
    mxtoolkit::TString m_logDir;
    mxtoolkit::TString m_cacheDir;
    mxtoolkit::TString m_imgDir;

    LauncherInfoListContainer m_dataMap;
};

