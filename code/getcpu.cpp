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

/* $Header: /CounterStrike/GETCPU.CPP 1     3/03/97 10:24a Joe_bostic $*/
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : GETCPU                                                       *
 *                                                                                             *
 *                    File Name : GETCPU.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Steve Tall                                                   *
 *                                                                                             *
 *                   Start Date : 6/26/96                                                      *
 *                                                                                             *
 *                  Last Update : June 26th 1996 [ST]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Overview:                                                                                   *
 *   Example of interface to assembly language code to find CPU type                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 * Functions:                                                                                  *
 *   Get_CPU_Type -- interface to ASM detection code                                           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "getcpu.h"

#include <cstdio>
#include <cstring>
#include <sys/sysctl.h>


/***********************************************************************************************
 * Get_CPU_Type -- Find out what kind of CPU we are running on                                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    int   - reference to cpu type                                                     *
 *           char* - ptr to buffer to receive chip vendor info                                 *
 *           int   - length of above buffer                                                    *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    6/26/96 10:15AM ST : Created                                                             *
 *=============================================================================================*/

extern "C" {

char CPUType = 0;

// The processor's marketing name, such as "Apple M2", or "Not available".
char VendorID[64] = "Not available";

}


/// <summary>
/// Records the processor family in CPUType and its name in VendorID. Apple silicon has no x86
/// family, so it reports the P6 family that every x86 processor the game supported reports; the
/// timing paths that test the family take their modern-hardware branch.
/// </summary>
void CPU_Id(void)
{
	size_t size = sizeof(VendorID);
	if (sysctlbyname("machdep.cpu.brand_string", VendorID, &size, NULL, 0) != 0) {
		strncpy(VendorID, "Not available", sizeof(VendorID));
	}
	CPUType = 6;
}


void Get_CPU_Type(int & cpu_type, char * vendor_id, int vendor_id_length)
{
	CPU_Id();

	cpu_type = (int)CPUType;

	if (vendor_id != NULL) {
		strncpy(vendor_id, VendorID, vendor_id_length);
	}
}
