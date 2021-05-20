/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsi2txss_hw.h
* @addtogroup csi2txss_v1_5
* @{
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx MIPI CSI2 Tx Subsystem core.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver
* xcsi2tx.h file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who  Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 sss 07/14/16 Initial release
* 1.2 vsa 02/28/18 Add Frame End Generation feature
* </pre>
*
******************************************************************************/
#ifndef XCSI2TXSS_HW_H_
#define XCSI2TXSS_HW_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/
#define XCSI2TXSS_ISR_ALLINTR_MASK	XCSI2TX_ISR_ALLINTR_MASK
#define XCSI2TXSS_LCSTAT_VC0_IER_MASK	XCSITX_LCSTAT_VC0_IER_MASK
#define XCSI2TXSS_LCSTAT_VC1_IER_MASK	XCSITX_LCSTAT_VC1_IER_MASK
#define XCSI2TXSS_LCSTAT_VC2_IER_MASK	XCSITX_LCSTAT_VC2_IER_MASK
#define XCSI2TXSS_LCSTAT_VC3_IER_MASK	XCSITX_LCSTAT_VC3_IER_MASK

/*****************************************************************************/
/**
*
* This function reads a value from a MIPI CSI2 Tx Subsystem register.
* A 32 bit read is performed. If the component is implemented in a smaller
* width, only the least significant data is read from the register. The most
* significant data will be read as 0.
*
* @param	BaseAddress is the base address of the XCsi2Tx core instance.
* @param	RegOffset is the register offset of the register (defined at
*		the top of this file).
*
* @return	The 32-bit value of the register.
*
* @note		None.
*
******************************************************************************/
static inline u32 XCsi2TxSs_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
{
	return Xil_In32(BaseAddress + RegOffset);
}

/*****************************************************************************/
/**
*
* This function writes a value to a MIPI CSI2 Tx Subsystem register.
* A 32 bit write is performed. If the component is implemented in a smaller
* width, only the least significant data is written.
*
* @param	BaseAddress is the base address of the XCsi2Tx core instance.
* @param	RegOffset is the register offset of the register (defined at
*		the top of this file) to be written.
* @param	Data is the 32-bit value to write into the register.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XCsi2TxSs_WriteReg(UINTPTR BaseAddress, u32 RegOffset,
								u32 Data)
{
	Xil_Out32(BaseAddress + RegOffset, Data);
}
/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
