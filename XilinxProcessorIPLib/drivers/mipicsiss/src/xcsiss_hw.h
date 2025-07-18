/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsiss_hw.h
* @addtogroup csiss Overview
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
* 1.5 vsa 08/10/20 Add YUV 420 8bits support
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
#define XCSISS_ISR_FR_MASK		XCSI_ISR_FR_MASK	/**< Frame received */
#define XCSISS_ISR_VCXFE_MASK		XCSI_ISR_VCXFE_MASK	/**< VCX frame error */
#define XCSISS_ISR_SKEWCALCHS_MASK	XCSI_ISR_SKEWCALCHS_MASK /**< Skew calc HS */
#define XCSISS_ISR_YUV420_MASK		XCSI_ISR_YUV420_MASK	/**< YUV420 error */
#define XCSISS_ISR_WC_MASK		XCSI_ISR_WC_MASK	/**< Word count error */
#define XCSISS_ISR_ILC_MASK		XCSI_ISR_ILC_MASK	/**< Incorrect lanes */
#define XCSISS_ISR_SPFIFOF_MASK		XCSI_ISR_SPFIFOF_MASK	/**< SP FIFO full */
#define XCSISS_ISR_SPFIFONE_MASK	XCSI_ISR_SPFIFONE_MASK	/**< SP FIFO not empty */
#define XCSISS_ISR_SLBF_MASK		XCSI_ISR_SLBF_MASK	/**< Stream line buf full */
#define XCSISS_ISR_STOP_MASK		XCSI_ISR_STOP_MASK	/**< Stop state detected */
#define XCSISS_ISR_SOTERR_MASK		XCSI_ISR_SOTERR_MASK	/**< SoT error */
#define XCSISS_ISR_SOTSYNCERR_MASK	XCSI_ISR_SOTSYNCERR_MASK /**< SoT sync error */
#define XCSISS_ISR_ECC2BERR_MASK	XCSI_ISR_ECC2BERR_MASK	/**< ECC 2-bit error */
#define XCSISS_ISR_ECC1BERR_MASK	XCSI_ISR_ECC1BERR_MASK	/**< ECC 1-bit error */
#define XCSISS_ISR_CRCERR_MASK		XCSI_ISR_CRCERR_MASK	/**< CRC error */
#define XCSISS_ISR_DATAIDERR_MASK	XCSI_ISR_DATAIDERR_MASK	/**< Data ID error */
#define XCSISS_ISR_VC3FSYNCERR_MASK	XCSI_ISR_VC3FSYNCERR_MASK /**< VC3 frame sync error */
#define XCSISS_ISR_VC3FLVLERR_MASK	XCSI_ISR_VC3FLVLERR_MASK /**< VC3 frame level error */
#define XCSISS_ISR_VC2FSYNCERR_MASK	XCSI_ISR_VC2FSYNCERR_MASK /**< VC2 frame sync error */
#define XCSISS_ISR_VC2FLVLERR_MASK	XCSI_ISR_VC2FLVLERR_MASK /**< VC2 frame level error */
#define XCSISS_ISR_VC1FSYNCERR_MASK	XCSI_ISR_VC1FSYNCERR_MASK /**< VC1 frame sync error */
#define XCSISS_ISR_VC1FLVLERR_MASK	XCSI_ISR_VC1FLVLERR_MASK /**< VC1 frame level error */
#define XCSISS_ISR_VC0FSYNCERR_MASK	XCSI_ISR_VC0FSYNCERR_MASK /**< VC0 frame sync error */
#define XCSISS_ISR_VC0FLVLERR_MASK	XCSI_ISR_VC0FLVLERR_MASK /**< VC0 frame level error */
#define XCSISS_ISR_ALLINTR_MASK		XCSI_ISR_ALLINTR_MASK	/**< All interrupts mask */
/*@}*/

/** @name BitMasks for grouped interrupts
 *
 * All interrupts are grouped into DPHY Level Errors, Protocol Decoding
 * Errors, Packet Level Errors, Normal Errors, Frame Received interrupt and
 * Short Packet related. These are used by application to determine the exact
 * event causing the interrupt
 * @{
 */
#define XCSISS_INTR_PROT_MASK 		XCSI_INTR_PROT_MASK	/**< Protocol error mask */
#define XCSISS_INTR_PKTLVL_MASK		XCSI_INTR_PKTLVL_MASK	/**< Packet level error mask */
#define XCSISS_INTR_DPHY_MASK		XCSI_INTR_DPHY_MASK	/**< D-PHY error mask */
#define XCSISS_INTR_SPKT_MASK 		XCSI_INTR_SPKT_MASK	/**< Short packet mask */
#define XCSISS_INTR_FRAMERCVD_MASK 	XCSI_INTR_FRAMERCVD_MASK /**< Frame received mask */
#define XCSISS_INTR_ERR_MASK		XCSI_INTR_ERR_MASK	/**< Other error mask */
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
