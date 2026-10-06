// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2026 OpenTS contributors
// See LICENSE.md for applicable additional terms and warranty disclaimers.

#include "dbgprint.h"
#include "except.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>


void DebugString(char const * message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	std::vfprintf(stderr, message, arguments);
	va_end(arguments);
}


void DebugStringNoPrefix(char const * message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	std::vfprintf(stderr, message, arguments);
	va_end(arguments);
}


void Fatal(char const * message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	std::vfprintf(stderr, message, arguments);
	va_end(arguments);
	std::fputc('\n', stderr);
	std::abort();
}
