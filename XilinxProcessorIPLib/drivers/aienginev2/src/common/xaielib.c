/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaielib.c
* @{
*
* This file contains the low level layer interface of the AIE driver with the
* definitions for the memory write and read operations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/23/2018  Initial creation
* 1.1  Naresh  05/23/2018  Added bare-metal BSP support
* 1.2  Naresh  06/18/2018  Updated code as per standalone driver framework
* 1.3  Naresh  07/11/2018  Updated copyright info
* 1.4  Hyun    10/10/2018  Added the mask write API
* 1.5  Hyun    10/11/2018  Don't include the xaieio header for sim build
* 1.6  Hyun    10/16/2018  Added the baremetal compile switch everywhere
*                          it's needed
* 1.7  Hyun    11/14/2018  Move platform dependent code to xaielib.c
* 1.8  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.9  Hyun    01/08/2019  Implement 128bit IO operations for baremetal
* 2.0  Hyun    01/08/2019  Add XAieLib_MaskPoll()
* 2.1  Hyun    04/05/2019  NPI support for simulation
* 2.2  Nishad  05/16/2019  Fix deallocation of pointer not on heap MISRA-c
* 				mandatory violation
* 2.3  Tejus   09/24/2019  Modified and added for aie
* 2.4  Tejus   06/09/2020  Remove NPI apis.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaielib.h"

#ifdef __AIESIM__ /* AIE simulator */

#include "xaiesim.h"

#elif defined __AIEBAREMTL__ /* Bare-metal application */

#include "xil_types.h"
#include "xil_io.h"
#include "xil_assert.h"
#include "xil_printf.h"
#include "xil_cache.h"
#include "xstatus.h"
#include "sleep.h"

#include <stdlib.h>

#else /* Non-baremetal application, ex Linux */

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#endif

/***************************** Macro Definitions *****************************/
/************************** Variable Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This provides to sleep in micro seconds.
*
* @param	Usec: Micro seconds to sleep
*
* @return	0 for success, and -1 for error..
*
* @note		None.
*
*******************************************************************************/
int XAieLib_usleep(u64 Usec)
{
#ifdef __AIESIM__
	return XAieSim_usleep(Usec);
#endif
}

/*****************************************************************************/
/**
*
* This API re-routes to platform print function
*
* @param	format strings
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieLib_IntPrint(const char *Format, ...)
{
#ifdef __AIESIM__
	/*
	 * If XAieSim_print() is used, the driver should be built with
	 * XAIE_DEBUG. Use print directly instead.
	 */
	printf(Format);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 32bit data from the specified address.
*
* @param	Addr: Address to read from.
*
* @return	32-bit read value.
*
* @note		None.
*
*******************************************************************************/
u32 XAieLib_Read32(u64 Addr)
{
#ifdef __AIESIM__
	return(XAieSim_Read32(Addr));
#endif
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 32bit data to the specified address.
*
* @param	Addr: Address to write to.
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieLib_Write32(u64 Addr, u32 Data)
{
#ifdef __AIESIM__
	XAieSim_Write32(Addr, Data);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write a masked 32bit data to
* the specified address.
*
* @param	Addr: Address to write to.
* @param	Mask: Mask to be applied to Data.
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieLib_MaskWrite32(u64 Addr, u32 Mask, u32 Data)
{
#ifdef __AIESIM__
	XAieSim_MaskWrite32(Addr, Mask, Data);
#endif
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 128bit data to the specified address.
*
* @param	Addr: Address to write to.
* @param	Data: Pointer to the 128-bit data buffer.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieLib_WriteCmd(u8 Command, u8 ColId, u8 RowId, u32 CmdWd0,
						u32 CmdWd1, u8 *CmdStr)
{
#ifdef __AIESIM__
	XAieSim_WriteCmd(Command, ColId, RowId, CmdWd0, CmdWd1, CmdStr);
#endif
}

/*****************************************************************************/
/**
*
* This is the IO function to poll until the value at the address to be given
* masked value.
*
* @param	Addr: Address to write to.
* @param	Mask: Mask to be applied to read data.
* @param	Value: The expected value
* @param	TimeOutUs: Minimum timeout in usec.
*
* @return	XAIELIB_SUCCESS on success, otherwise XAIELIB_FAILURE
*
* @note		None.
*
*******************************************************************************/
u32 XAieLib_MaskPoll(u64 Addr, u32 Mask, u32 Value, u32 TimeOutUs)
{
	u32 Ret = XAIELIB_FAILURE;

#ifdef __AIESIM__
	if (XAieSim_MaskPoll(Addr, Mask, Value, TimeOutUs) == XAIESIM_SUCCESS) {
		Ret = XAIELIB_SUCCESS;
	}
#endif
	return Ret;
}

/** @} */
