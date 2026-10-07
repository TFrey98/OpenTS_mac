/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/win.h                                  $*
 *                                                                                             *
 *                      $Author:: Ian_l                                                       $*
 *                                                                                             *
 *                     $Modtime:: 10/16/01 2:42p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

// The engine was written against the Win32 API. These are the macOS definitions of the
// Win32 names it still uses. Integer widths match Win32, not the LP64 macOS ABI, because
// asset, save, and network layouts are built from these types.

#include <cstddef>
#include <cstdint>
#include <ctime>
#include <unistd.h>

#include "vkey.h"

typedef int32_t BOOL;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;
typedef int INT;
typedef unsigned int UINT;
typedef char CHAR;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef intptr_t LRESULT;
typedef DWORD COLORREF;
typedef char * LPSTR;
typedef char const * LPCSTR;
typedef char const * LPCTSTR;
typedef void * LPVOID;

// Opaque handles. Distinct struct types keep a window from being passed as a device context.
typedef void * HANDLE;
typedef struct HWND__ * HWND;
typedef struct HINSTANCE__ * HINSTANCE;
typedef struct HDC__ * HDC;
typedef struct HBITMAP__ * HBITMAP;
typedef struct HFONT__ * HFONT;
typedef void * HGDIOBJ;
typedef void * HKL;

typedef struct { DWORD dwLowDateTime; DWORD dwHighDateTime; } FILETIME;
typedef struct { WORD wYear; WORD wMonth; WORD wDayOfWeek; WORD wDay; WORD wHour; WORD wMinute; WORD wSecond; WORD wMilliseconds; } SYSTEMTIME;
typedef struct { LONG left; LONG top; LONG right; LONG bottom; } RECT;
typedef struct { LONG x; LONG y; } POINT;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)

#define WINAPI
#define CALLBACK
#define __stdcall
#define __cdecl
#define __forceinline inline __attribute__((always_inline))

#define LOWORD(l) ((WORD)(((uintptr_t)(l)) & 0xFFFF))
#define HIWORD(l) ((WORD)((((uintptr_t)(l)) >> 16) & 0xFFFF))
#define RGB(r, g, b) ((COLORREF)(((BYTE)(r)) | (((WORD)(BYTE)(g)) << 8) | (((DWORD)(BYTE)(b)) << 16)))
#define GetRValue(rgb) ((BYTE)(rgb))
#define GetGValue(rgb) ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb) ((BYTE)((rgb) >> 16))

#define _ReturnAddress() __builtin_return_address(0)
#define __debugbreak() __builtin_debugtrap()
#define _Printf_format_string_

// Dialog results, numbered as Win32 numbers them.
#define IDOK 1
#define IDCANCEL 2

// Milliseconds on a monotonic clock. Like the Win32 counter, the value wraps after 49.7 days,
// and callers compare times by unsigned difference.
inline DWORD timeGetTime(void)
{
	return (DWORD)(clock_gettime_nsec_np(CLOCK_UPTIME_RAW) / 1000000);
}

inline DWORD GetTickCount(void)
{
	return timeGetTime();
}

inline ULONGLONG GetTickCount64(void)
{
	return clock_gettime_nsec_np(CLOCK_UPTIME_RAW) / 1000000;
}

// The macOS scheduler needs no timer-resolution request, so these always succeed.
inline UINT timeBeginPeriod(UINT) { return 0; }
inline UINT timeEndPeriod(UINT) { return 0; }

inline void Sleep(DWORD milliseconds)
{
	usleep((useconds_t)milliseconds * 1000);
}

// FILETIME counts 100-nanosecond intervals since 1601-01-01 UTC. Save files store it.
inline ULONGLONG File_Time_Ticks(FILETIME const & time)
{
	return ((ULONGLONG)time.dwHighDateTime << 32) | time.dwLowDateTime;
}

inline FILETIME File_Time_From_Unix(int64_t seconds, int64_t nanoseconds)
{
	ULONGLONG const UNIX_EPOCH_TICKS = 116444736000000000ULL;
	ULONGLONG ticks = UNIX_EPOCH_TICKS + (ULONGLONG)seconds * 10000000ULL + (ULONGLONG)(nanoseconds / 100);
	return FILETIME{(DWORD)ticks, (DWORD)(ticks >> 32)};
}

inline int64_t Unix_Time_From_File_Time(FILETIME const & time)
{
	return ((int64_t)File_Time_Ticks(time) - 116444736000000000LL) / 10000000LL;
}

inline void GetSystemTimeAsFileTime(FILETIME * time)
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	*time = File_Time_From_Unix(now.tv_sec, now.tv_nsec);
}

inline SYSTEMTIME System_Time_From_Unix(struct timespec const & now, bool local)
{
	struct tm parts;
	if (local) {
		localtime_r(&now.tv_sec, &parts);
	} else {
		gmtime_r(&now.tv_sec, &parts);
	}
	return SYSTEMTIME{(WORD)(parts.tm_year + 1900), (WORD)(parts.tm_mon + 1), (WORD)parts.tm_wday, (WORD)parts.tm_mday,
		(WORD)parts.tm_hour, (WORD)parts.tm_min, (WORD)parts.tm_sec, (WORD)(now.tv_nsec / 1000000)};
}

inline void GetSystemTime(SYSTEMTIME * time)
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	*time = System_Time_From_Unix(now, false);
}

inline void GetLocalTime(SYSTEMTIME * time)
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	*time = System_Time_From_Unix(now, true);
}

// The system's minimum pointer travel, in pixels, before a press becomes a drag. Win32's
// default is 4 in each direction.
#define SM_CXDRAG 68
#define SM_CYDRAG 69

inline int GetSystemMetrics(int index)
{
	return (index == SM_CXDRAG || index == SM_CYDRAG) ? 4 : 0;
}

inline LONG CompareFileTime(FILETIME const * a, FILETIME const * b)
{
	ULONGLONG x = File_Time_Ticks(*a);
	ULONGLONG y = File_Time_Ticks(*b);
	return x < y ? -1 : (x > y ? 1 : 0);
}

extern HINSTANCE	ProgramInstance;
extern HWND			MainWindow;
extern bool			GameInFocus;
