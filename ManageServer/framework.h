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

// Including SDKDDKVer.h defines the highest available Windows platform.
// If you wish to build your application for a previous Windows platform,
// include WinSDKVer.h and set the _WIN32_WINNT macro to the platform
// you wish to support before including SDKDDKVer.h.
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
#include <cstdarg> // (variadics)
#include <iomanip>
#include <memory>
#include <random>
#include <signal.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <strsafe.h>
#include <thread>
#include <time.h>
#include <tuple>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
