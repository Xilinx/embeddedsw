/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaielib.h
* @{
*
* This file contains the generic definitions for the AIE drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/27/2018  Initial creation
* 1.1  Naresh  05/23/2018  Added support for bare-metal BSP
* 1.2  Naresh  06/18/2018  Updated code as per standalone driver framework
* 1.3  Naresh  07/11/2018  Updated copyright info
* 1.4  Hyun    10/10/2018  Added the mask write API
* 1.5  Hyun    11/14/2018  Move platform dependent code to xaielib.c
* 1.6  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.7  Hyun    01/08/2019  Add XAieLib_MaskPoll()
* 1.8  Tejus   09/24/2019  Modified and added for aie
* 1.9  Tejus   06/09/2020  Remove NPI apis.
* </pre>
*
******************************************************************************/
#ifndef XAIELIB_H
#define XAIELIB_H

/***************************** Include Files *********************************/
#include <stdint.h>
#include <stdio.h>
/************************** Constant Definitions *****************************/
typedef int8_t			s8;
typedef uint8_t			u8;
typedef uint16_t		u16;
typedef int32_t			s32;
typedef uint32_t		u32;
typedef uint64_t		u64;

/* Don't mix with equivalent baremetal macros, ex, XST_SUCCESS */
#define XAIELIB_SUCCESS			0U
#define XAIELIB_FAILURE			1U
#define XAIELIB_COMPONENT_IS_READY	1U

#define XAIELIB_CMDIO_COMMAND_SETSTACK	0U
#define XAIELIB_CMDIO_COMMAND_LOADSYM	1U

/* Enable cache for memory mapping */
#define XAIELIB_MEM_ATTR_CACHE		0x1U

/************************** Variable Definitions *****************************/

/**************************     Inline Helpers   *****************************/
/************************** Function Prototypes  *****************************/

#endif		/* end of protection macro */
/** @} */
