/******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsirxss_hw.h
* @addtogroup dsirxss Overview
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
#ifndef XDSIRXSS_HW_H_
#define XDSIRXSS_HW_H_		/**< Prevent circular inclusions
				by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"
#include "xdsi_hw.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/
#define XDsiRxSs_In32	Xil_In32
#define XDsiRxSs_Out32	Xil_Out32

/*****************************************************************************/
/**
*
* This function reads a value from a MIPI DSI Rx Subsystem register.
* A 32 bit read is performed. If the component is implemented in a smaller
* width, only the least significant data is read from the register. The most
* significant data will be read as 0.
*
* @param	BaseAddress is the base address of the XDsiRxSs core instance.
* @param	RegOffset is the register offset of the register (defined at
*		the top of this file).
*
* @return	The 32-bit value of the register.
*
* @note		None
*
******************************************************************************/
static inline u32 XDsiRxSs_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
{
	return	XDsiRxSs_In32(BaseAddress + (u32)RegOffset);
}

/*****************************************************************************/
/**
*
* This function writes a value to a MIPI DSI Rx Subsystem register.
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
static inline void XDsiRxSs_WriteReg(UINTPTR BaseAddress, u32 RegOffset,
								u32 Data)
{
	XDsiRxSs_Out32(BaseAddress + (u32)RegOffset, (u32)Data);
}

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/

/** MIPI DSI Rx Subsystem core DSI system interrupt handlers
 *  User will register specific interrupts handler and enable
 *  interrupts by below defining macros
 */

#define XDSIRXSS_HANDLER_UNSUPPORT_DATATYPE	XDSI_HANDLER_UNSUPPORT_DATATYPE
#define XDSIRXSS_HANDLER_PIXELDATA_UNDERRUN	XDSI_HANDLER_PIXELDATA_UNDERRUN
#define XDSIRXSS_HANDLER_CMDQ_FIFOFULL		XDSI_HANDLER_CMDQ_FIFOFULL
#define XDSIRXSS_HANDLER_OTHERERROR		XDSI_HANDLER_OTHERERROR

#define XDSIRXSS_ISR_DATAIDERR_MASK		XDSI_ISR_DATA_ID_ERR_MASK
#define XDSIRXSS_ISR_PIXELUNDERRUN_MASK		XDSI_ISR_PXL_UNDR_RUN_MASK
#define XDSIRXSS_ISR_CMDQ_FIFO_FULL_MASK	XDSI_ISR_CMDQ_FIFO_FULL_MASK

#define XDSIRXSS_IER_DATAIDERR_MASK		XDSI_IER_DATA_ID_ERR_MASK
#define XDSIRXSS_IER_PIXELUNDERRUN_MASK		XDSI_IER_PXL_UNDR_RUN_MASK
#define XDSIRXSS_IER_CMDQ_FIFO_FULL_MASK	XDSI_IER_CMDQ_FIFO_FULL_MASK
#define XDSIRXSS_IER_ALLINTR_MASK		XDSI_IER_ALLINTR_MASK

#ifdef __cplusplus
}
#endif

#endif
/** @} */
