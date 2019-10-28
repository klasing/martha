// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <windowsx.h>
// an easy way to add the common control library
#pragma comment(lib, "comctl32.lib")
#include <CommCtrl.h>
// C RunTime Header Files
#include <algorithm>
#include <iomanip>
#include <malloc.h>
#include <memory.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <strsafe.h>
#include <tchar.h>
#include <time.h>
