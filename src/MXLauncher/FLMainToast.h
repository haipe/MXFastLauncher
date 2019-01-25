#pragma once
#include "FMWnd.h"
#include <list>
#include <map>


struct LauncherInfo
{
    int position;   //顺序
    int page;       //页码
    TString id;     //guid
    TString name;   //名称
    TString path;   //路径
    TString param;  //参数
    TString image;  //缩略图路径
};

typedef std::list<LauncherInfo> LauncherInfoList;
typedef std::map<int, LauncherInfoList> LauncherInfoListMap;



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
    TString m_dataDir;
    TString m_logDir;
    TString m_cacheDir;
    TString m_imgDir;

    LauncherInfoListMap m_dataMap;
};

