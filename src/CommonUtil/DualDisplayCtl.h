#pragma once

#include <string>
#include <vector>

#include "ddraw.h"
#include "CommonUtil.h"

#define MAX_VIRTUALSCREEN_WIDTH         3000   //虚拟屏幕间隔

namespace commonutil
{
    typedef std::vector<DISPLAY_DEVICE>		DisplayDevList;


    class DisplayInfo
    {
    public:
        DisplayInfo() {}
        ~DisplayInfo()
        {
            m_vecDisplayInfo.clear();
        }

    public:
        void AddMonitorInfo(const MONITORINFO& info);

        int GetDisplayCount()
        {
            return m_vecDisplayInfo.size();
        }

        void ModifyDisplayInfo(int nIndex, const MONITORINFO& info);

        BOOL GetDualDisplayRect(int nIndex, MONITORINFO* info);

        BOOL GetDualDisplayRect(const POINT& pt, MONITORINFO* info);

        void Clear();
    private:
        std::vector<MONITORINFO>    m_vecDisplayInfo;
    };

    class DualDisplayCtl
    {
    public:
        DualDisplayCtl();
        virtual ~DualDisplayCtl();

    public:
        static BOOL	SetDualMode(BOOL bDualMode);
        static BOOL SetDualModeEx(BOOL bDualMode);//add by ron 2013-06-27
        static BOOL	IsSupportDualMode();
        
        static void	Release();
        static BOOL	Init();

        static void EnumDisplay(DisplayInfo* info);
        static void EnumDisplayDevice();

        //add by ron2013-06-20
        static DWORD GetRealDevCount() { return m_dwRealDevCount; }
        static BOOL SetDualDisplay(BOOL bDualMode, int nIndex, POINT ptDisplay);
        static BOOL	GetDualDisplayRect(LPRECT lpRect);
        static BOOL	GetDualDisplayRect(POINT ptPos, LPRECT lpRect);
        static BOOL GetDualDisplayRect(int nIndex, LPRECT lpRect);
        static BOOL GetDualDisplayRect(HWND hWnd, LPRECT lpRect);
        //end

        static void OnDeviceChange();
        static void ModifyDisplayInfo(int nIndex, const MONITORINFO& info);
        static BOOL IsPrimaryMonitor(HMONITOR hMonitor);
        static BOOL IsRepeatMonitor(HMONITOR hMonitor, int nIndex);
        static void EmptyDisplayInfo(int nIndex);
        static bool GetPrimaryMonitorID(std::string* sMonitorID);
        static bool GetPrimaryMonitorID(std::wstring* sMonitorID);
        static int GetOldScreenCount();
        static bool IsPrimaryMonitorChanged();

        static DisplayInfo*	GetDiaplayInfo() { return &m_DisplayInfo; }
        static int			GetOldScreeenCount() { return m_nOldScreenCount; }

    protected:
        static DisplayInfo     m_DisplayInfo;

        static int              m_nOldScreenCount;

    protected:
        static DisplayDevList	m_lsDisplayDev;
        static DWORD			m_dwRealDevCount;
        static std::vector<POINT>   m_vecDisplaySrcPos;

    };

}