/******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsi2rxss_hw.h
* @addtogroup dsi2rxss Overview
* @{
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the AMD MIPI DSI Rx Subsystem core.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver
* xdsirxss.h file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date    Changes
* --- --- ------- -------------------------------------------------------
* 1.0 Kunal 12/02/24 Initial Release for MIPI DSI RX subsystem
* </pre>
*
******************************************************************************/
#ifndef XDSI2RXSS_HW_H_
#define XDSI2RXSS_HW_H_		/**< Prevent circular inclusions
				by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"
#include "xdsi2rx_hw.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/
#define XDsi2RxSs_In32	Xil_In32
#define XDsi2RxSs_Out32	Xil_Out32

/*****************************************************************************/
/**
*
* This function reads a value from a MIPI DSI2 Rx Subsystem register.
* A 32 bit read is performed. If the component is implemented in a smaller
* width, only the least significant data is read from the register. The most
* significant data will be read as 0.
*
* @param	BaseAddress is the base address of the XDsi2RxSs core instance.
* @param	RegOffset is the register offset of the register (defined at
*		the top of this file).
*
* @return	The 32-bit value of the register.
*
* @note		None
*
******************************************************************************/
static inline u32 XDsi2RxSs_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
{
	return	XDsi2RxSs_In32(BaseAddress + (u32)RegOffset);
}

/*****************************************************************************/
/**
*
* This function writes a value to a MIPI DSI2 Rx Subsystem register.
* A 32 bit write is performed. If the component is implemented in a smaller
* width, only the least significant data is written.
*
* @param	BaseAddress is the base address of the XDsiRxSs core instance.
* @param	RegOffset is the register offset of the register (defined at
*			the top of this file) to be written.
* @param	Data is the 32-bit value to write into the register.
*
* @return	None.
*
* @note		None
*
******************************************************************************/
static inline void XDsi2RxSs_WriteReg(UINTPTR BaseAddress, u32 RegOffset,
								u32 Data)
{
	XDsi2RxSs_Out32(BaseAddress + (u32)RegOffset, (u32)Data);
}

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif
/** @} */
