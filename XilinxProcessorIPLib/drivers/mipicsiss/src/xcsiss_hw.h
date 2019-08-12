/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xcsiss_hw.h
* @addtogroup csiss_v1_1
* @{
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx MIPI CSI Rx Subsystem core.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver
* xcsiss.h file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 vsa 07/25/15 Initial release
* 1.1 sss 08/17/16 Added 64 bit support
* 1.2 vsa 03/12/17 Add Word Count corruption interrupt support
* </pre>
*
******************************************************************************/
#ifndef XCSISS_HW_H_
#define XCSISS_HW_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Bitmasks for interrupt callbacks
 *
 * Please refer to CSI driver for details of the bitmasks.
 * The application should use the XCSISS_ISR* masks in the call back functions
 * to decode the exact cause of interrupt and handle it accordingly.
 * @{
 */
#define XCSISS_ISR_FR_MASK		XCSI_ISR_FR_MASK
#define XCSISS_ISR_WC_MASK		XCSI_ISR_WC_MASK
#define XCSISS_ISR_ILC_MASK		XCSI_ISR_ILC_MASK
#define XCSISS_ISR_SPFIFOF_MASK		XCSI_ISR_SPFIFOF_MASK
#define XCSISS_ISR_SPFIFONE_MASK	XCSI_ISR_SPFIFONE_MASK
#define XCSISS_ISR_SLBF_MASK		XCSI_ISR_SLBF_MASK
#define XCSISS_ISR_STOP_MASK		XCSI_ISR_STOP_MASK
#define XCSISS_ISR_SOTERR_MASK		XCSI_ISR_SOTERR_MASK
#define XCSISS_ISR_SOTSYNCERR_MASK	XCSI_ISR_SOTSYNCERR_MASK
#define XCSISS_ISR_ECC2BERR_MASK	XCSI_ISR_ECC2BERR_MASK
#define XCSISS_ISR_ECC1BERR_MASK	XCSI_ISR_ECC1BERR_MASK
#define XCSISS_ISR_CRCERR_MASK		XCSI_ISR_CRCERR_MASK
#define XCSISS_ISR_DATAIDERR_MASK	XCSI_ISR_DATAIDERR_MASK
#define XCSISS_ISR_VC3FSYNCERR_MASK	XCSI_ISR_VC3FSYNCERR_MASK
#define XCSISS_ISR_VC3FLVLERR_MASK	XCSI_ISR_VC3FLVLERR_MASK
#define XCSISS_ISR_VC2FSYNCERR_MASK	XCSI_ISR_VC2FSYNCERR_MASK
#define XCSISS_ISR_VC2FLVLERR_MASK	XCSI_ISR_VC2FLVLERR_MASK
#define XCSISS_ISR_VC1FSYNCERR_MASK	XCSI_ISR_VC1FSYNCERR_MASK
#define XCSISS_ISR_VC1FLVLERR_MASK	XCSI_ISR_VC1FLVLERR_MASK
#define XCSISS_ISR_VC0FSYNCERR_MASK	XCSI_ISR_VC0FSYNCERR_MASK
#define XCSISS_ISR_VC0FLVLERR_MASK	XCSI_ISR_VC0FLVLERR_MASK
#define XCSISS_ISR_ALLINTR_MASK		XCSI_ISR_ALLINTR_MASK
/*@}*/

/** @name BitMasks for grouped interrupts
 *
 * All interrupts are grouped into DPHY Level Errors, Protocol Decoding
 * Errors, Packet Level Errors, Normal Errors, Frame Received interrupt and
 * Short Packet related. These are used by application to determine the exact
 * event causing the interrupt
 * @{
 */
#define XCSISS_INTR_PROT_MASK 		XCSI_INTR_PROT_MASK
#define XCSISS_INTR_PKTLVL_MASK		XCSI_INTR_PKTLVL_MASK
#define XCSISS_INTR_DPHY_MASK		XCSI_INTR_DPHY_MASK
#define XCSISS_INTR_SPKT_MASK 		XCSI_INTR_SPKT_MASK
#define XCSISS_INTR_FRAMERCVD_MASK 	XCSI_INTR_FRAMERCVD_MASK
#define XCSISS_INTR_ERR_MASK		XCSI_INTR_ERR_MASK
/*@}*/

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This function reads a value from a MIPI CSI Rx Subsystem register.
* A 32 bit read is performed. If the component is implemented in a smaller
* width, only the least significant data is read from the register. The most
* significant data will be read as 0.
*
* @param	BaseAddress is the base address of the XCsiSs core instance.
* @param	RegOffset is the register offset of the register (defined at
*		the top of this file).
*
* @return	The 32-bit value of the register.
*
* @note		None.
*
******************************************************************************/
static inline u32 XCsiSs_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
{
	return (Xil_In32(BaseAddress + (u32)RegOffset));
}

/*****************************************************************************/
/**
*
* This function writes a value to a MIPI CSI Rx Subsystem register.
* A 32 bit write is performed. If the component is implemented in a smaller
* width, only the least significant data is written.
*
* @param	BaseAddress is the base address of the XCsiSs core instance.
* @param	RegOffset is the register offset of the register (defined at
*		the top of this file) to be written.
* @param	Data is the 32-bit value to write into the register.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XCsiSs_WriteReg(UINTPTR BaseAddress, u32 RegOffset, u32 Data)
{
	Xil_Out32(BaseAddress + (u32)RegOffset, (u32)Data);
}
/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
