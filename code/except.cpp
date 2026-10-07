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
 *                     $Archive:: /Commando/Code/wwlib/Except.cpp                             $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/07/02 12:28p                                              $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "except.h"

// macOS writes its own crash report for a faulting process, so the game installs no handler
// of its own and the test hooks the -EXCEPTIONTEST option drives do nothing.

#include <cstdio>
#include <cstdlib>
#include <cxxabi.h>
#include <dlfcn.h>

void Install_Exception_Handler(void)
{
}


void Exception_Register_Log_File(char const *)
{
}


bool Describe_Code_Address(void const * address, char * buffer, unsigned size)
{
	if (buffer == NULL || size == 0) {
		return(false);
	}
	buffer[0] = '\0';

	// dladdr names exported functions only; a static function reports the nearest one before it.
	Dl_info info;
	if (dladdr(address, &info) == 0 || info.dli_sname == NULL) {
		return(false);
	}

	int status = 0;
	char * demangled = abi::__cxa_demangle(info.dli_sname, NULL, NULL, &status);
	snprintf(buffer, size, "%s+0x%lx", status == 0 && demangled != NULL ? demangled : info.dli_sname,
		(unsigned long)((uintptr_t)address - (uintptr_t)info.dli_saddr));
	free(demangled);
	return(true);
}


void Exception_Set_Test_Mode(char const *)
{
}


void Exception_Run_Immediate_Test(void)
{
}


void Exception_Run_Post_Window_Test(void)
{
}


void Exception_Wndproc_Test_Fault(void)
{
}

