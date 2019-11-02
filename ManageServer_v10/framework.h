// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once
#define _WIN32_WINNT 0x0601
#include <boost/algorithm/string.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/system/error_code.hpp>
#include <boost/timer/timer.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>

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
#include <thread>
#include <time.h>
#include <tuple>
#include <vector>
