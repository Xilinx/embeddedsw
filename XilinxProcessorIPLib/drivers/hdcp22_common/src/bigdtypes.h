/* $Id: bigdtypes.h $ */

/***** BEGIN LICENSE BLOCK *****
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2001-15 David Ireland, D.I. Management Services Pty Limited
 * <http://www.di-mgt.com.au/bigdigits.html>. All rights reserved.
 *
 ***** END LICENSE BLOCK *****/
/*
 * Last updated:
 * $Date: 2015-10-22 10:23:00 $
 * $Revision: 2.5.0 $
 * $Author: dai $
 */

#ifndef BIGDTYPES_H_
#define BIGDTYPES_H_ 1

#include <stddef.h>
#include "xil_types.h"

/*
The following PP instructions assume that all Linux systems have a C99-conforming
<stdint.h>; that other Unix systems have the uint32_t definitions in <sys/types.h>;
and that MS et al don't have them at all. This version assumes that a long is 32 bits.
Adjust if necessary to suit your system.
You can override by defining HAVE_C99INCLUDES or HAVE_SYS_TYPES.
*/

#ifndef EXACT_INTS_DEFINED_
#define EXACT_INTS_DEFINED_ 1
#ifndef HAVE_C99INCLUDES
	#if (__STDC_VERSION >= 199901L) || defined(linux) || defined(__linux__) || defined(__APPLE__)
	#define HAVE_C99INCLUDES
	#endif
#endif
#ifndef HAVE_SYS_TYPES
	#if defined(unix) || defined(__unix__)
	#define HAVE_SYS_TYPES
	#endif
#endif
#ifdef HAVE_C99INCLUDES
	#include <stdint.h>
#elif defined(HAVE_SYS_TYPES)
	#include <sys/types.h>
#else
	#define uint32_t unsigned int
	#define uint16_t unsigned short
	#define uint8_t unsigned char
#endif	/* HAVE_C99INCLUDES */
#endif	/* EXACT_INTS_DEFINED_ */

/* Macros for format specifiers
-- change to "u", "x" and "X" if necessary */
#ifdef HAVE_C99INCLUDES
	#include <inttypes.h>
#else
	#define PRIu32 "lu"
	#define PRIx32 "lx"
	#define PRIX32 "lX"
#endif
/* We define our own */
#define PRIuBIGD PRIu32
#define PRIxBIGD PRIx32
#define PRIXBIGD PRIX32

#endif /* BIGDTYPES_H_ */
