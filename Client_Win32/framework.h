// header.h : include file for standard system include files,
// or project specific include files
//
#pragma once
#define _WIN32_WINNT 0x0601
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem/fstream.hpp>

// // Including SDKDDKVer.h defines the highest available Windows platform.
// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.
#include <SDKDDKVer.h>

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
// Windows Header Files
#include <windows.h>
#include <windowsx.h>
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
// an easy way to add the common control library
#pragma comment(lib, "comctl32.lib")
#include <CommCtrl.h>

// C RunTime Header Files
#include <cstdlib>
#include <functional>
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include <string>
#include <strsafe.h>
#include <tchar.h>

