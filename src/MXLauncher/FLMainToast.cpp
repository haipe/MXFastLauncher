#include "stdafx.h"
#include "FLMainToast.h"
#include "ximage.h"
#include "DualDisplayCtl.h"
#include <atlconv.h>
#include <DbgHelp.h>

#define HIDE_WND_OPERATE_TIMER      100
#define SHOW_WND_OPERATE_TIMER      101

#define CHECK_CTRL_KEY_UP_TIMER  200

#define TIMER_SP 50


TString GetLocalAppdataPath()
{
    TCHAR szDefaultDir[MAX_PATH] = { 0 };
    TCHAR szDocument[MAX_PATH] = { 0 };

    LPITEMIDLIST pidl = NULL;
    SHGetSpecialFolderLocation(NULL, CSIDL_LOCAL_APPDATA, &pidl);
    if (pidl && SHGetPathFromIDList(pidl, szDocument))
    {
        GetShortPathName(szDocument, szDefaultDir, MAX_PATH);
    }
    return szDefaultDir;
}

#pragma comment(lib, "Dbghelp.lib")
void CreateDir(const TCHAR* path)
{
    USES_CONVERSION;
    MakeSureDirectoryPathExists(T2A(path));
}

BOOL GetShortCutFile(WCHAR* ShortcutFile, WCHAR* buf, int nSize)
{
    HRESULT           hres;
    IShellLink        *psl;
    IPersistFile      *ppf;
    WIN32_FIND_DATA   fd;


    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl);
    if (!SUCCEEDED(hres))
        return   false;

    hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
    if (SUCCEEDED(hres))
    {
        //wchar_t wsz[MAX_PATH];   //buffer   for   Unicode   string
        //MultiByteToWideChar(CP_ACP,0,ShortcutFile,-1,wsz,MAX_PATH);   
        //hres = ppf->Load(wsz,STGM_READ);
        hres = ppf->Load(ShortcutFile, STGM_READ);
        if (SUCCEEDED(hres))
            hres = psl->GetPath(buf, nSize, &fd, 0);
        ppf->Release();
    }
    psl->Release();

    return SUCCEEDED(hres);
}

HRESULT OpenExeLinkFile(TCHAR *linkPath )
{
    TCHAR szLinkPath[MAX_PATH] = { 0 };
    lstrcpy(szLinkPath, linkPath);

    TCHAR extName[MAX_PATH] = { 0 };
    _wsplitpath(szLinkPath, NULL, NULL, NULL, extName);
    if (lstrcmp(extName, L".lnk") == 0)
    {
        if (GetShortCutFile(szLinkPath, linkPath, MAX_PATH))
        {
        }
    }

    return S_OK;
}


HRESULT GetShellThumbnailImage(LPCWSTR pszPath, HBITMAP* pThumbnail)
{
    HRESULT hr = S_FALSE;

    *pThumbnail = NULL;

    LPITEMIDLIST pidlItems = NULL, pidlURL = NULL, pidlWorkDir = NULL;
    WCHAR szBasePath[MAX_PATH], szFileName[MAX_PATH];
    WCHAR* p;
    wcscpy(szBasePath, pszPath);
    p = wcsrchr(szBasePath, L'\\');
    if (p) *(p + 1) = L'\0';
    wcscpy(szFileName, pszPath + (p - szBasePath) + 1);

    IShellFolder* psfDesktop = NULL;
    IShellFolder* psfWorkDir = NULL;
    IExtractImage* peiURL = NULL;
    while (TRUE)
    {
        hr = SHGetDesktopFolder(&psfDesktop);
        if (FAILED(hr)) break;

        hr = psfDesktop->ParseDisplayName(NULL, NULL, szBasePath, NULL, &pidlWorkDir, NULL);
        if (FAILED(hr)) break;
        hr = psfDesktop->BindToObject(pidlWorkDir, NULL, IID_IShellFolder, (LPVOID*)&psfWorkDir);
        if (FAILED(hr)) break;

        hr = psfWorkDir->ParseDisplayName(NULL, NULL, szFileName, NULL, &pidlURL, NULL);
        if (FAILED(hr)) break;

        // query IExtractImage 
        hr = psfWorkDir->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pidlURL, IID_IExtractImage, NULL, (LPVOID*)&peiURL);
        if (FAILED(hr)) break;

        // define thumbnail properties 
        SIZE size = { 64, 48 };
        DWORD dwPriority = 0, dwFlags = IEIFLAG_ASPECT;
        WCHAR pszImagePath[MAX_PATH];
        hr = peiURL->GetLocation(pszImagePath, MAX_PATH, &dwPriority, &size, 16, &dwFlags);
        if (FAILED(hr)) break;

        // generate thumbnail 
        hr = peiURL->Extract(pThumbnail);
        if (FAILED(hr)) break;

        break;
    }

    if (peiURL)peiURL->Release();
    if (psfWorkDir)psfWorkDir->Release();
    if (psfDesktop)psfDesktop->Release();

    // free allocated structures
    if (pidlWorkDir) CoTaskMemFree(pidlWorkDir);
    if (pidlURL) CoTaskMemFree(pidlURL);
    return hr;
}

const wchar_t* NewGUID()
{
    static wchar_t buf[64] = { 0 };
    GUID guid;
    if (S_OK == ::CoCreateGuid(&guid))
    {
        swprintf(buf, sizeof(buf)
            , L"{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}"
            , guid.Data1
            , guid.Data2
            , guid.Data3
            , guid.Data4[0], guid.Data4[1]
            , guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
            , guid.Data4[6], guid.Data4[7]
        );
    }
    return (const wchar_t*)buf;
}

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

    m_dataDir = GetLocalAppdataPath();
    m_dataDir += L"\\MXLauncher\\";

    m_logDir = m_dataDir + L"log\\";
    CreateDir(m_logDir.c_str());
    m_cacheDir = m_dataDir + L"cache\\";
    CreateDir(m_cacheDir.c_str());
    m_imgDir = m_dataDir + L"img\\";
    CreateDir(m_imgDir.c_str());

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

        if (0)
        {
            //url文件，就是ini文件 
            //[InternetShortcut]
            //URL = http://hao.169x.cn/
            //IconFile = C : \Windows\SystemApps\Microsoft.MicrosoftEdge_8wekyb3d8bbwe\MicrosoftEdge.exe
        }

        if (0)
        {
            //获取快捷键路径
            OpenExeLinkFile(cFileName);
        }
        if (0)
        {
            //获取文件缩略图
            HBITMAP bm;
            GetShellThumbnailImage(cFileName, &bm);
            if (bm)
            {
                CxImage img(CXIMAGE_FORMAT_BMP);
                img.CreateFromHBITMAP(bm, 0, true);
                if (img.IsValid())
                    img.Save(L"D:\\lihp\\Desktop\\222.BMP", CXIMAGE_FORMAT_BMP);
            }
        }
        
        if (0)
        {
            //获取exe的ico
            HICON ico = ::ExtractIcon(CPaintManagerUI::GetInstance(), cFileName, 0);

            CxImage  image(CXIMAGE_FORMAT_ICO);
            image.CreateFromHICON(ico, true);
            if (image.IsValid())
                image.Save(L"D:\\lihp\\Desktop\\111.BMP", CXIMAGE_FORMAT_BMP);
        }
    }
    ::DragFinish(hDropInfo);

}
