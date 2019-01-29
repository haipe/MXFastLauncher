#pragma once
#include "MXDuiWnd.h"
#include <list>
#include <vector>


struct LauncherInfo
{
    mxtoolkit::TString id;     //guid
    mxtoolkit::TString name;   //名称
    mxtoolkit::TString icon;   //缩略图路径
    mxtoolkit::TString path;   //路径
    mxtoolkit::TString param;  //参数
};

typedef std::list<LauncherInfo>         LauncherInfoList;

struct LauncherPageInfo 
{
    mxtoolkit::TString  name;
    LauncherInfoList    launchers;
};

typedef std::vector<LauncherPageInfo>   LauncherPageInfoVector;



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
    virtual HRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* bHandled);

    virtual void Notify(TNotifyUI& msg);

protected:
    void LoadConfig();
    void SaveConfig();

    void LoadRunConfig();
    void SaveRunConfig();

protected:
    void OperateHide(bool start);
    void OperateShow(bool start);
    void OperateCheck(bool start);

    void JustShow();

    void OnDropFiles(HDROP hDropInfo);
    bool AnlysisFile(const TCHAR* path, LauncherInfo& info);
    bool NewLauncher(const LauncherInfo& info);

    bool OnLauncherEvent(void* param);
    void DoLauncher(const TCHAR* name);

private:
    CTileLayoutUI * m_uiLauncherArea = nullptr;
    CEditUI*        m_searchEdit = nullptr;

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

    LauncherPageInfoVector m_launcherData;
};

