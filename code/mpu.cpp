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
 *                     $Archive:: /Commando/Code/wwlib/mpu.cpp                                $*
 *                                                                                             *
 *                      $Author:: Denzil_l                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/23/01 5:07p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Get_CPU_Rate -- Fetch the rate of CPU ticks per second.                                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "mpu.h"

#include <mach/mach_time.h>

#include <cmath>


static double Counter_Hertz(void)
{
	mach_timebase_info_data_t timebase;
	mach_timebase_info(&timebase);
	return(1.0e9 * (double)timebase.denom / (double)timebase.numer);
}


/// <summary>
/// Fetches the rate of the Get_CPU_Clock counter in ticks per second, as two 32 bit halves.
/// </summary>
/// <param name="high">Receives the high half of the rate.</param>
/// <returns>unsigned int; the low half of the rate.</returns>
unsigned int Get_CPU_Rate(unsigned int & high)
{
	unsigned long long const rate = (unsigned long long)llround(Counter_Hertz());
	high = (unsigned int)(rate >> 32);
	return((unsigned int)rate);
}


/// <summary>
/// Fetches the system's monotonic tick counter, which runs at the rate Get_CPU_Rate reports.
/// The value is 64 bits wide; the low half is returned and the high half stored through the
/// reference.
/// </summary>
/// <param name="high">Receives the high half of the counter.</param>
/// <returns>unsigned int; the low half of the counter.</returns>
unsigned int Get_CPU_Clock(unsigned int & high)
{
	unsigned long long const stamp = mach_absolute_time();

	high = (unsigned int)(stamp >> 32);
	return((unsigned int)stamp);
}


/// <summary>
/// Returns the rate of the Get_CPU_Clock counter in megahertz, which converts its ticks to
/// microseconds. macOS does not report the processor's own clock.
/// </summary>
int Get_RDTSC_CPU_Speed(void)
{
	return((int)llround(Counter_Hertz() / 1.0e6));
}
