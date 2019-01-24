#pragma once

#include <windows.h>
#include <tchar.h>
#include <string>

#ifdef UNICODE
typedef std::wstring TString;

#else
typedef std::string TString;

#endif


namespace commonutil
{


}