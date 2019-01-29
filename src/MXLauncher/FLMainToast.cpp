#include "stdafx.h"
#include "FLMainToast.h"
#include  <io.h>
#include <fstream>
#include "ximage.h"
#include "DualDisplayCtl.h"
#include <atlconv.h>
#include <DbgHelp.h>
#include "rapidjson\document.h"
#include "rapidjson\writer.h"
#include "rapidjson\filereadstream.h"
#include "rapidjson\istreamwrapper.h"
#include "MXStringConvert.h"
#include "MXPath.h"
#include "MXString.h"
#include <thread>



#define HIDE_WND_OPERATE_TIMER      100
#define SHOW_WND_OPERATE_TIMER      101

#define CHECK_CTRL_KEY_UP_TIMER  200

#define TIMER_SP 50


mxtoolkit::TString GetLocalAppdataPath()
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

BOOL GetShortCutFile(const TCHAR* ShortcutFile, TCHAR* buf, int nSize)
{
    IShellLink *psl = NULL;
    IPersistFile *ppf = NULL;
    WIN32_FIND_DATA fd;

    HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl);
    if (!SUCCEEDED(hres))
        return false;

    hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
    if (SUCCEEDED(hres))
    {
        hres = ppf->Load(ShortcutFile, STGM_READ);
        if (SUCCEEDED(hres))
            hres = psl->GetPath(buf, nSize, &fd, 0);

        ppf->Release();
    }
    psl->Release();

    return SUCCEEDED(hres);
}

HRESULT GetShellThumbnailImage(const WCHAR* pszPath, HBITMAP* pThumbnail)
{
    HRESULT hr = S_FALSE;

    *pThumbnail = NULL;

    LPITEMIDLIST pidlItems = NULL, pidlURL = NULL, pidlWorkDir = NULL;

    mxtoolkit::WString dir, file;
    mxtoolkit::Path::GetFilePathInfo(pszPath, &dir, &file);

    IShellFolder* psfDesktop = NULL;
    IShellFolder* psfWorkDir = NULL;
    IExtractImage* peiURL = NULL;
    do
    {
        hr = SHGetDesktopFolder(&psfDesktop);
        if (FAILED(hr))
            break;

        hr = psfDesktop->ParseDisplayName(NULL, NULL, (LPWSTR)dir.c_str(), NULL, &pidlWorkDir, NULL);
        if (FAILED(hr))
            break;

        hr = psfDesktop->BindToObject(pidlWorkDir, NULL, IID_IShellFolder, (LPVOID*)&psfWorkDir);
        if (FAILED(hr))
            break;

        hr = psfWorkDir->ParseDisplayName(NULL, NULL, (LPWSTR)file.c_str(), NULL, &pidlURL, NULL);
        if (FAILED(hr))
            break;

        // query IExtractImage 
        hr = psfWorkDir->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pidlURL, IID_IExtractImage, NULL, (LPVOID*)&peiURL);
        if (FAILED(hr))
            break;

        // define thumbnail properties 
        SIZE size = { 64, 48 };
        DWORD dwPriority = 0, dwFlags = IEIFLAG_ASPECT;
        WCHAR pszImagePath[MAX_PATH];
        hr = peiURL->GetLocation(pszImagePath, MAX_PATH, &dwPriority, &size, 16, &dwFlags);
        if (FAILED(hr))
            break;

        // generate thumbnail 
        hr = peiURL->Extract(pThumbnail);
    } while (0);

    if (peiURL)peiURL->Release();
    if (psfWorkDir)psfWorkDir->Release();
    if (psfDesktop)psfDesktop->Release();

    // free allocated structures
    if (pidlWorkDir) CoTaskMemFree(pidlWorkDir);
    if (pidlURL) CoTaskMemFree(pidlURL);

    return hr;
}


HICON GetFileIcon(const TCHAR* file)
{
    HICON icon = NULL;

    SHFILEINFO info;
    if (SHGetFileInfo(file, FILE_ATTRIBUTE_NORMAL, &info, sizeof(info), SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_USEFILEATTRIBUTES))
    {
        icon = info.hIcon;
    }

    return icon;
}

HICON GetFolderIcon()
{
    HICON icon = NULL;
    SHFILEINFO info;
    if (SHGetFileInfo(_T("folder"), FILE_ATTRIBUTE_DIRECTORY, &info, sizeof(info), SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_USEFILEATTRIBUTES))
    {
        icon = info.hIcon;
    }

    return icon;
}

FLMainToast::FLMainToast()
    : MXDuiWnd(L"FLMainToast.xml")
{
}

FLMainToast::~FLMainToast()
{
}

HWND FLMainToast::Init(HWND owner)
{
    mxtoolkit::MXDuiWnd::SetNotify(this);
    if (!MXDuiWnd::Create(
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

    LoadConfig();
    LoadRunConfig();

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
        apl -= 32;
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

    return MXDuiWnd::OnTimer(wParam, lParam, bHandled);
}

HRESULT FLMainToast::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* bHandled)
{
    switch (uMsg)
    {
    case WM_KILLFOCUS:
    {
        HWND wnd = (HWND)wParam;
        if(wnd == m_hWnd)
            break;

        if (wnd && wnd != m_hWnd)
        {
            bool bre = false;
            while (true)
            {
                HWND parent = GetParent(wnd);
                if (!parent)
                    break;

                if (parent == m_hWnd)
                {
                    bre = true;
                    break;
                }

                wnd = parent;
            }

            if(bre)
                break;
        }

        OutputDebugStringA("-WM_KILLFOCUS.\n");
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

    return MXDuiWnd::OnMessage(uMsg, wParam, lParam, bHandled);
}

void FLMainToast::Notify(TNotifyUI& msg)
{
    if (msg.sType == DUI_MSGTYPE_WINDOWINIT)
    {
        m_searchEdit = dynamic_cast<CEditUI*>(m_pm.FindControl(L"Dialog_Title_Area_Search"));
        m_uiLauncherArea = dynamic_cast<CTileLayoutUI*>(m_pm.FindControl(L"Launcher_Area"));

        if (!m_position.IsNull())
            ::MoveWindow(m_hWnd, m_position.left, m_position.top, m_position.GetWidth(), m_position.GetHeight(), TRUE);

        m_maxAphle = 200;
        OperateShow(true);

        MXDuiWnd::InitWndAbilityManager();

        for (auto& page : m_launcherData)
        {
            for (auto& launcher : page.launchers)
            {
                NewLauncher(launcher);
            }
        }
    }
    else if (msg.sType == DUI_MSGTYPE_CLICK)
    {
        if (msg.pSender == m_close)
        {
            ::GetWindowRect(m_hWnd, &m_position);

            Close(0);
            SaveConfig();
            SaveRunConfig();
        }
    }
}

void FLMainToast::LoadConfig()
{
    mxtoolkit::TString file = m_dataDir + L"\\launcher.json";
    mxtoolkit::AString filePath;
    mxtoolkit::SCKit::UnicodeToAnsi(file.c_str(), filePath);

    std::ifstream ifs(filePath.c_str());
    mxtoolkit::AString fileData;
    mxtoolkit::AString line;
    while (getline(ifs, line)) 
        fileData.append(line + "\n");
    
    RAPIDJSON_NAMESPACE::Document doc;
    doc.Parse(fileData.c_str());
    if (doc.HasParseError())
        return;

    if (!doc.IsArray())
        return;

    for (int pageIndex = 0; pageIndex < doc.Size(); pageIndex++)
    {
        const RAPIDJSON_NAMESPACE::Value &pageValue = doc[pageIndex];
        if (!pageValue.IsObject())
            continue;

        LauncherPageInfo pageInfo;
        mxtoolkit::SCKit::Utf8ToUnicode(pageValue["Name"].GetString(), pageInfo.name);

        const RAPIDJSON_NAMESPACE::Value& objLauncers = pageValue["Items"];
        if(!objLauncers.IsArray())
            continue;

        for (int launcherIndex = 0; launcherIndex < objLauncers.Size(); launcherIndex++)
        {
            const RAPIDJSON_NAMESPACE::Value &launcherValue = objLauncers[launcherIndex];
            if (!launcherValue.IsObject())
                continue;

            LauncherInfo info;
            if (launcherValue.HasMember("ID"))
                mxtoolkit::SCKit::Utf8ToUnicode(launcherValue["ID"].GetString(), info.id);

            if (launcherValue.HasMember("Name"))
                mxtoolkit::SCKit::Utf8ToUnicode(launcherValue["Name"].GetString(), info.name);

            if (launcherValue.HasMember("Icon"))
                mxtoolkit::SCKit::Utf8ToUnicode(launcherValue["Icon"].GetString(), info.icon);

            if (launcherValue.HasMember("Path"))
                mxtoolkit::SCKit::Utf8ToUnicode(launcherValue["Path"].GetString(), info.path);

            if (launcherValue.HasMember("Param"))
                mxtoolkit::SCKit::Utf8ToUnicode(launcherValue["Param"].GetString(), info.param);

            pageInfo.launchers.emplace_back(info);
        }

        m_launcherData.emplace_back(pageInfo);
    }    
}

void FLMainToast::SaveConfig()
{
    RAPIDJSON_NAMESPACE::Document doc(RAPIDJSON_NAMESPACE::kArrayType);
    RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator = doc.GetAllocator(); //获取分配器

    for (auto& pageInfo : m_launcherData)
    {
        RAPIDJSON_NAMESPACE::Value objPage(RAPIDJSON_NAMESPACE::kObjectType);//创建一个Object类型的元素
        mxtoolkit::AString utf8;
        mxtoolkit::SCKit::UnicodeToUtf8(pageInfo.name.c_str(), utf8);
        objPage.AddMember("Name", RAPIDJSON_NAMESPACE::Value(utf8.c_str(), utf8.length(), allocator), allocator);

        RAPIDJSON_NAMESPACE::Value objLaunchers(RAPIDJSON_NAMESPACE::kArrayType);
        for (auto& launcher : pageInfo.launchers)
        {
            RAPIDJSON_NAMESPACE::Value objLauncher(RAPIDJSON_NAMESPACE::kObjectType);//创建一个Object类型的元素
            mxtoolkit::AString utf8;
            mxtoolkit::SCKit::UnicodeToUtf8(launcher.id.c_str(), utf8);
            objLauncher.AddMember("ID", RAPIDJSON_NAMESPACE::Value(utf8.c_str(), utf8.length(), allocator), allocator);

            mxtoolkit::SCKit::UnicodeToUtf8(launcher.name.c_str(), utf8);
            objLauncher.AddMember("Name", RAPIDJSON_NAMESPACE::Value(utf8.c_str(), utf8.length(), allocator), allocator);

            mxtoolkit::SCKit::UnicodeToUtf8(launcher.icon.c_str(), utf8);
            objLauncher.AddMember("Icon", RAPIDJSON_NAMESPACE::Value(utf8.c_str(), utf8.length(), allocator), allocator);

            mxtoolkit::SCKit::UnicodeToUtf8(launcher.path.c_str(), utf8);
            objLauncher.AddMember("Path", RAPIDJSON_NAMESPACE::Value(utf8.c_str(), utf8.length(), allocator), allocator);

            mxtoolkit::SCKit::UnicodeToUtf8(launcher.param.c_str(), utf8);
            objLauncher.AddMember("Param", RAPIDJSON_NAMESPACE::Value(utf8.c_str(), utf8.length(), allocator), allocator);

            objLaunchers.PushBack(objLauncher, allocator);
        }

        objPage.AddMember("Items", objLaunchers, allocator);
        doc.PushBack(objPage, allocator);
    }
    
    RAPIDJSON_NAMESPACE::StringBuffer buffer;
    RAPIDJSON_NAMESPACE::Writer<RAPIDJSON_NAMESPACE::StringBuffer> writer(buffer);
    doc.Accept(writer);
    
    auto sss = buffer.GetString();
    std::ofstream outfile;
    mxtoolkit::TString file = m_dataDir + L"\\launcher.json";
    mxtoolkit::AString filePath;
    mxtoolkit::SCKit::UnicodeToAnsi(file.c_str(), filePath);

    //写到文件
//     outfile.open(filePath.c_str());
//     if (outfile.fail())
//     {
//         return;
//     }
//     outfile << sss;
//     outfile.close();

    FILE* cfgFile = fopen(filePath.c_str(), "wb");
    if (cfgFile)
    {
        fputs(buffer.GetString(), cfgFile);
        fclose(cfgFile);
    }
}

void FLMainToast::LoadRunConfig()
{
    mxtoolkit::TString file = m_dataDir + L"\\run.json";
    mxtoolkit::AString filePath;
    mxtoolkit::SCKit::UnicodeToAnsi(file.c_str(), filePath);

    std::ifstream ifs(filePath.c_str());
    mxtoolkit::AString fileData;
    mxtoolkit::AString line;
    while (getline(ifs, line))
        fileData.append(line + "\n");

    RAPIDJSON_NAMESPACE::Document doc;
    doc.Parse(fileData.c_str());
    if (doc.HasParseError())
        return;

    m_position.Empty();
    const RAPIDJSON_NAMESPACE::Value &xValue = doc["PosX"];
    if (xValue.IsInt())
        m_position.left = xValue.GetInt();
    const RAPIDJSON_NAMESPACE::Value &yValue = doc["PosY"];
    if (yValue.IsInt())
        m_position.top = yValue.GetInt();

    const RAPIDJSON_NAMESPACE::Value &wValue = doc["WndWidth"];
    if (wValue.IsInt())
        m_position.right = m_position.left + wValue.GetInt();
    const RAPIDJSON_NAMESPACE::Value &hValue = doc["WndHeight"];
    if (hValue.IsInt())
        m_position.bottom = m_position.top + hValue.GetInt();
}

void FLMainToast::SaveRunConfig()
{
    RAPIDJSON_NAMESPACE::Document doc(RAPIDJSON_NAMESPACE::kObjectType);
    RAPIDJSON_NAMESPACE::Document::AllocatorType &allocator = doc.GetAllocator(); //获取分配器

    doc.AddMember("PosX", m_position.left, allocator);
    doc.AddMember("PosY", m_position.top, allocator);
    doc.AddMember("WndWidth", m_position.GetWidth(), allocator);
    doc.AddMember("WndHeight", m_position.GetHeight(), allocator);

    RAPIDJSON_NAMESPACE::StringBuffer buffer;
    RAPIDJSON_NAMESPACE::Writer<RAPIDJSON_NAMESPACE::StringBuffer> writer(buffer);
    doc.Accept(writer);

    auto sss = buffer.GetString();
    std::ofstream outfile;
    mxtoolkit::TString file = m_dataDir + L"\\run.json";
    mxtoolkit::AString filePath;
    mxtoolkit::SCKit::UnicodeToAnsi(file.c_str(), filePath);

    //写到文件
    outfile.open(filePath.c_str());
    if (outfile.fail())
    {
        return;
    }
    outfile << sss;
    outfile.close();
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
    
    if (GetCapture() != m_hWnd)
    {
        ::SetForegroundWindow(m_hWnd);
        //::SetActiveWindow(m_hWnd);
        //::SetCapture(m_hWnd);
        //::SetFocus(m_hWnd);
    }
}

void FLMainToast::OnDropFiles(HDROP hDropInfo)
{
    UINT  uFileCount, u;
    TCHAR cFileName[MAX_PATH];
    uFileCount = ::DragQueryFile(hDropInfo, -1, cFileName, sizeof(cFileName));   //拖拉的文件个数

    for (u = 0; u < uFileCount; u++)
    {
        ::DragQueryFile(hDropInfo, u, cFileName, sizeof(cFileName));

        LauncherInfo info;
        if (AnlysisFile(cFileName, info))
        {
            if(NewLauncher(info))
            {
                if (m_launcherData.empty())
                {
                    m_launcherData.push_back(LauncherPageInfo());
                }

                auto pageIter = m_launcherData.begin();
                pageIter->launchers.push_back(info);
            }
        }
    }

    ::DragFinish(hDropInfo);
}

bool FLMainToast::AnlysisFile(const TCHAR* path, LauncherInfo& info)
{
    //创建一个按钮，并监听双击事件
    mxtoolkit::TString dir, name, ext;
    if (!mxtoolkit::Path::GetFilePathInfo(path, &dir, &name, &ext))
    {
        if (!mxtoolkit::Path::GetFolderPathInfo(path, &dir, &name))
            return false;

        info.name = name;
        info.path = path;
        info.id = mxtoolkit::NewGuidString();
        info.icon = m_imgDir + info.id + _T(".bmp");

        //目录
        HICON ico = GetFolderIcon();
        CxImage ximg(CXIMAGE_FORMAT_ICO);
        ximg.CreateFromHICON(ico, true);
        if (ximg.IsValid())
            ximg.Save(info.icon.c_str(), CXIMAGE_FORMAT_BMP);

        ::DeleteObject(ico);
        return true;
    }

    info.name = name;
    info.path = path;

    if (ext == _T("url"))
    {
        //url文件，就是ini文件 
        //[InternetShortcut]
        //URL = http://hao.169x.cn/
        //IconFile = C : \Windows\SystemApps\Microsoft.MicrosoftEdge_8wekyb3d8bbwe\MicrosoftEdge.exe

        TCHAR tempValue[MAX_PATH] = { 0 };
        ::GetPrivateProfileString(L"InternetShortcut", L"IconFile", L"", tempValue, MAX_PATH, path);
        LauncherInfo infoTemp;
        if (AnlysisFile(tempValue, infoTemp))
        {
            info.id = info.id;
            info.path = path;
            info.icon = info.icon;
            //::GetPrivateProfileString(L"InternetShortcut", L"URL", L"", tempValue, MAX_PATH, path);
        }
    }
    else if (ext == _T("lnk"))
    {
        //获取快捷键路径
        TCHAR realPath[MAX_PATH] = { 0 };
        if (GetShortCutFile(path, realPath, MAX_PATH))
        {
            LauncherInfo infoTemp;
            if (AnlysisFile(realPath, infoTemp))
            {
                info.id = infoTemp.id;
                info.path = realPath;
                info.icon = infoTemp.icon;
            }
        }
    }
    else if (ext == _T("exe"))
    {
        info.id = mxtoolkit::NewGuidString();
        info.icon = m_imgDir + info.id + _T(".bmp");
        int i = 0;
        //获取exe的ico
        HICON ico = NULL;
        while (ico = ::ExtractIcon(CPaintManagerUI::GetInstance(), path, i++))
        {
            CxImage ximg(CXIMAGE_FORMAT_ICO);
            ximg.CreateFromHICON(ico, true);
            if (ximg.IsValid())
            {
                int w = ximg.GetWidth();
                int h = ximg.GetHeight();
                ximg.Save(info.icon.c_str(), CXIMAGE_FORMAT_BMP);
            }

            ::DeleteObject(ico);

        }
    }
    else
    {
        info.id = mxtoolkit::NewGuidString();
        info.icon = m_imgDir + info.id + _T(".bmp");
        //获取文件缩略图
        HBITMAP bm;
        GetShellThumbnailImage(path, &bm);
        if (bm)
        {
            CxImage ximg(CXIMAGE_FORMAT_BMP);
            ximg.CreateFromHBITMAP(bm, 0, true);
            if (ximg.IsValid())
                ximg.Save(info.icon.c_str(), CXIMAGE_FORMAT_BMP);
        }
        else
        {
            //查找系统设置的默认图标
            HICON ico = GetFileIcon(path);
            CxImage ximg(CXIMAGE_FORMAT_ICO);
            ximg.CreateFromHICON(ico, true);
            if (ximg.IsValid())
                ximg.Save(info.icon.c_str(), CXIMAGE_FORMAT_BMP);

            ::DeleteObject(ico);
        }
    }

    return !info.id.empty();
}

bool FLMainToast::NewLauncher(const LauncherInfo& info)
{
    //生成UI
    DuiLib::CButtonUI* btn = new DuiLib::CButtonUI;
    if (!btn)
        return false;

    CDuiString bkImg;
    bkImg.Format(L"file='%s' dest='30,28,66,64'", info.icon.c_str());
    btn->SetBkImage(bkImg);
    btn->SetText(info.name.c_str());
    btn->SetName(info.id.c_str());
    btn->SetTextColor(0xffffff);
    btn->SetTextPadding({ 5,70,5,0 });

    btn->SetAttribute(L"endellipsis", L"true");
    btn->SetAttribute(L"hotbkcolor", L"#FFAEAEAE");

    btn->OnEvent += MakeDelegate(this, &FLMainToast::OnLauncherEvent);
    m_uiLauncherArea->Add(btn);
    
    return true;
}

bool FLMainToast::OnLauncherEvent(void* param)
{
    if (!param)return true;
    TEventUI* event = (TEventUI*)param;

    if (event->Type == UIEVENT_DBLCLICK)
    {
        if (_tcscmp(event->pSender->GetClass(), L"ButtonUI") != 0)
            return true;

        auto name = event->pSender->GetName();

        DoLauncher(name);
    }

    return true;
}

void FLMainToast::DoLauncher(const TCHAR* name)
{
    for (auto& page : m_launcherData)
    {
        for (auto& launcher : page.launchers)
        {
            if (launcher.id.compare(name) == 0)
            {
                std::thread doLauncher([launcher]()
                {
                    ::ShellExecute(NULL, _T("open"), launcher.path.c_str(), launcher.param.c_str(), NULL, SW_SHOWNORMAL);
                });
                doLauncher.detach();
            }
        }
    }
}
