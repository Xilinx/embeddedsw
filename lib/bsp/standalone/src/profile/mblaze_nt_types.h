/******************************************************************************
* Copyright (c) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
* @file mblaze_nt_types.h
* @brief MicroBlaze newlib-compatible type definitions.
*/




#ifndef _MBLAZE_NT_TYPES_H
#define _MBLAZE_NT_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef char            byte;   /**< Signed 8-bit byte type. */
typedef short           half;   /**< Signed 16-bit half-word type. */
typedef int             word;   /**< Signed 32-bit word type. */
typedef unsigned char   ubyte;  /**< Unsigned 8-bit byte type. */
typedef unsigned short  uhalf;  /**< Unsigned 16-bit half-word type. */
typedef unsigned int    uword;  /**< Unsigned 32-bit word type. */
typedef ubyte           boolean; /**< Boolean type (0=false, non-zero=true). */


#ifdef __cplusplus
}
#endif

#endif
