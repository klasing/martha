// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

// // Including SDKDDKVer.h defines the highest available Windows platform.
// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.
#include <SDKDDKVer.h>

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
// Windows Header Files
#include <windows.h>
#include <windowsx.h>
// an easy way to add the common control library
#pragma comment(lib, "comctl32.lib")
#include <CommCtrl.h>

// C RunTime Header Files
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include <string>
#include <tchar.h>
