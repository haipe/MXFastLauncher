
#pragma once
#include <tchar.h>
#include "UIlib.h"
#include "CommonUtil.h"

namespace commonutil
{
    using namespace DuiLib;

    class FMWnd
        : public DuiLib::CWindowWnd
    {
    public:
        FMWnd(const TCHAR* xml);
        virtual ~FMWnd();

    public:
        static void ShowInTask(HWND wnd, bool show);
        static bool CheckWindowStyle(HWND wnd, DWORD style);

        static HICON s_smalIcon;
        static HICON s_bigIcon;

    public:
        void SetNotify(DuiLib::INotifyUI* notify);
        void SetBuilderCallback(DuiLib::IDialogBuilderCallback* callback);

        HWND Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, const RECT rc, HMENU hMenu = NULL);  //NOLINT()
        HWND Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int cx = CW_USEDEFAULT, int cy = CW_USEDEFAULT, HMENU hMenu = NULL);//NOLINT()

        DuiLib::CPaintManagerUI* GetPaintManager() { return &m_pm; }

        void ShowShadow(bool bShow);
        void CenterWindow(int width = 0, int height = 0);

        void Minimize();	//最小化
        void Maxmize();		//最大化
        void Restore();		//还原
        
        void SetWindowText(LPCTSTR text);

    public:

        //静态窗口
        virtual void PopWindow(DWORD addFlag = WS_POPUPWINDOW | WS_CAPTION | WS_DLGFRAME, DWORD removeFlag = WS_CHILD);

        //模态窗口
        virtual DWORD ModalWindow(DWORD addFlag = WS_OVERLAPPEDWINDOW, DWORD removeFlag = WS_CHILD);

        virtual void TileWindow(DWORD addFlag = WS_CHILD, DWORD removeFlag = WS_POPUPWINDOW | WS_CAPTION | WS_DLGFRAME);
        
        //初始化能力
        virtual void InitWndAbilityManager();

    public:
        //--------------------------------------------------------------------------------------------
        virtual UINT GetClassStyle() const { return (CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_SAVEBITS); };
        virtual LPCTSTR GetWindowClassName() const { return _T("FMWnd"); };
        virtual HRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) final; //使用OnMessage 处理其他消息

    protected:
        //----------------------------------------------------------------------------------------------		
        virtual LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* bHandled) { return 0; };

        virtual LRESULT OnTimer(WPARAM wParam, LPARAM lParam, BOOL* bHandled);

        virtual LRESULT OnCreate(WPARAM wParam, LPARAM lParam, BOOL* bHandled);

        virtual LRESULT OnClose(WPARAM wParam, LPARAM lParam, BOOL* bHandled);

        virtual LRESULT OnDestroy(WPARAM wParam, LPARAM lParam, BOOL* bHandled);

        virtual LRESULT OnNcActivate(WPARAM wParam, LPARAM lParam, BOOL* bHandled);

        virtual LRESULT OnNcCalcSize(WPARAM wParam, LPARAM lParam, BOOL* bHandled);

        virtual LRESULT OnNcPaint(WPARAM wParam, LPARAM lParam, BOOL* bHandled);

        virtual LRESULT OnNcHitTest(WPARAM wParam, LPARAM lParam, BOOL* bHandled);

        virtual LRESULT OnNCLButtonDBClick(WPARAM wParam, LPARAM lParam, BOOL* bHandled);

        virtual LRESULT OnSize(WPARAM wParam, LPARAM lParam, BOOL* bHandled);

        virtual LRESULT OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam, BOOL* bHandled);

        virtual LRESULT OnSysCommand(WPARAM wParam, LPARAM lParam, BOOL* bHandled);

        virtual LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam, BOOL* bHandled);
                
    protected:
        TString	        			    m_xmlFile;
        DuiLib::CPaintManagerUI			m_pm;

        DuiLib::IDialogBuilderCallback* m_duiBuilderCallback = nullptr;
        DuiLib::INotifyUI*				m_duiNotify = nullptr;
        
        CControlUI*						m_logo = nullptr;				//logo
        CControlUI*                     m_logoName = nullptr;           //Dialog_Title_Area_Logo_Name
        CButtonUI*						m_min = nullptr;				//最小化
        CButtonUI*						m_max = nullptr;				//最大化
        CButtonUI*						m_restore = nullptr;			//还原
        CButtonUI*						m_close = nullptr;				//关闭
        CControlUI*						m_titleArea = nullptr;			//标题栏区域
        CControlUI*						m_bodyArea = nullptr;			//主界面body区域

        bool							m_uiCompleted = false;			//界面UI  Notify Inited

    };
}