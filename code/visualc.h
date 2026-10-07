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
 *                     $Archive:: /Commando/Code/wwlib/visualc.h                              $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 5/16/01 4:11p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once



#ifndef M_E
#define M_E         2.71828182845904523536
#endif
#ifndef M_LOG2E
#define M_LOG2E     1.44269504088896340736
#endif
#ifndef M_LOG10E
#define M_LOG10E    0.434294481903251827651
#endif
#ifndef M_LN2
#define M_LN2       0.693147180559945309417
#endif
#ifndef M_LN10
#define M_LN10      2.30258509299404568402
#endif
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2      1.57079632679489661923
#endif
#ifndef M_PI_4
#define M_PI_4      0.785398163397448309616
#endif
#ifndef M_1_PI
#define M_1_PI      0.318309886183790671538
#endif
#ifndef M_2_PI
#define M_2_PI      0.636619772367581343076
#endif
#ifndef M_1_SQRTPI
#define M_1_SQRTPI  0.564189583547756286948
#endif
#ifndef M_2_SQRTPI
#define M_2_SQRTPI  1.12837916709551257390
#endif
#ifndef M_SQRT2
#define M_SQRT2     1.41421356237309504880
#endif
#ifndef M_SQRT_2
#define M_SQRT_2    0.707106781186547524401
#endif

// Single precision pi, for the float paths that would otherwise round M_PI at every use.
#ifndef M_FPI
#define M_FPI 3.141592654f
#endif

/*
**	Macros to convert between degrees and radians
*/
#ifndef RAD_TO_DEG
#define RAD_TO_DEG(x)	(((double)x)*180.0/M_PI)
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD(x)	(((double)x)*M_PI/180.0)
#endif

#ifndef RAD_TO_DEGF
#define RAD_TO_DEGF(x)	(((float)x)*180.0f/M_PI)
#endif

#ifndef DEG_TO_RADF
#define DEG_TO_RADF(x)	(((float)x)*M_PI/180.0f)
#endif
