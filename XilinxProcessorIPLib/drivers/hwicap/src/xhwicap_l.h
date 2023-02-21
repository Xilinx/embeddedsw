/******************************************************************************
* Copyright (C) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xhwicap_l.h
* @addtogroup hwicap Overview
* @{
*
* This header file contains identifiers and basic driver functions (or
* macros) that can be used to access the device. Other driver functions
* are defined in xhwicap.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 2.00a sv   09/28/07 First release for the FIFO mode
* 3.00a sv   11/28/08 Added Abort bit definition in the Control Register
*		      Removed XHI_WFO_MAX_VACANCY and XHI_RFO_MAX_OCCUPANCY
*		      definitions.
* 3.01a sv   10/21/09 Corrected the IDCODE definitions for some of the
*                     V5 FX parts.
* 4.00a hvm  11/30/09 Added support for V6 and updated with HAL phase 1
*		      modifications
* 5.00a hvm  02/04/10 Added S6 Support
* 5.03a hvm  15/4/11  Updated with V6 CXT device definitions.
* 6.00a hvm  08/01/11 Updated with K7 device Ids.
* 7.00a bss  03/14/12 Added EOS mask and Hang mask CR CR 637538
*		      Added Virtex 7 and Zynq Idcodes - CR 647140, CR 643295
*
* 8.00a bss  06/20/12 Deleted Hang mask definition as per CR 656162
* 9.0   bss  02/20/14 Added Kintex 8 and Virtex72000T device Idcodes.
* 10.0  bss  6/24/14  Removed support for families older than 7 series
* 11.5  Nava 09/30/22 Added new IDCODE's as mentioned in the ug570 Doc.
*
* </pre>
*
*****************************************************************************/
#ifndef XHWICAP_L_H_ /* prevent circular inclusions */
#define XHWICAP_L_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include <xil_types.h>
#include <xil_assert.h>
#include "xil_io.h"
/************************** Constant Definitions ****************************/

/******************************************************************/
/** @name Register Map
 *
 * Register offsets for the XHwIcap device.
 * @{
 */
#define XHI_GIER_OFFSET		0x1C  /**< Device Global Interrupt Enable Reg */
#define XHI_IPISR_OFFSET	0x20  /**< Interrupt Status Register */
#define XHI_IPIER_OFFSET	0x28  /**< Interrupt Enable Register */
#define XHI_WF_OFFSET		0x100 /**< Write FIFO */
#define XHI_RF_OFFSET		0x104 /**< Read FIFO */
#define XHI_SZ_OFFSET		0x108 /**< Size Register */
#define XHI_CR_OFFSET		0x10C /**< Control Register */
#define XHI_SR_OFFSET		0x110 /**< Status Register */
#define XHI_WFV_OFFSET		0x114 /**< Write FIFO Vacancy Register */
#define XHI_RFO_OFFSET		0x118 /**< Read FIFO Occupancy Register */

/* @} */


/** @name Device Global Interrupt Enable Register (GIER) bit definitions
 *
 * @{
 */
#define XHI_GIER_GIE_MASK      0x80000000 /**< Global Interrupt enable Mask */

/* @} */

/** @name HwIcap Device Interrupt Status/Enable Registers
 *
 * <b> Interrupt Status Register (IPISR) </b>
 *
 * This register holds the interrupt status flags for the device. These
 * bits are toggle on write.
 *
 * <b> Interrupt Enable Register (IPIER) </b>
 *
 * This register is used to enable interrupt sources for the device.
 * Writing a '1' to a bit in this register enables the corresponding Interrupt.
 * Writing a '0' to a bit in this register disables the corresponding Interrupt.
 *
 * IPISR/IPIER registers have the same bit definitions and are only defined
 * once.
 * @{
 */
#define XHI_IPIXR_RFULL_MASK	0x00000008 /**< Read FIFO Full */
#define XHI_IPIXR_WEMPTY_MASK	0x00000004 /**< Write FIFO Empty */
#define XHI_IPIXR_RDP_MASK	0x00000002 /**< Read FIFO half full */
#define XHI_IPIXR_WRP_MASK	0x00000001 /**< Write FIFO half full */
#define XHI_IPIXR_ALL_MASK	0x0000000F /**< Mask of all interrupts */

/* @} */

/** @name Control Register (CR)
 *
 * @{
 */
#define XHI_CR_SW_ABORT_MASK	0x00000010 /**< Abort current ICAP Read/Write */
#define XHI_CR_SW_RESET_MASK	0x00000008 /**< SW Reset Mask */
#define XHI_CR_FIFO_CLR_MASK	0x00000004 /**< FIFO Clear Mask */
#define XHI_CR_READ_MASK	0x00000002 /**< Read from ICAP to FIFO */
#define XHI_CR_WRITE_MASK	0x00000001 /**< Write from FIFO to ICAP */

/* @} */


/** @name Status Register (SR)
 *
 * @{
 */
#define XHI_SR_CFGERR_N_MASK	0x00000100 /**< Config Error Mask */
#define XHI_SR_DALIGN_MASK	0x00000080 /**< Data Alignment Mask */
#define XHI_SR_RIP_MASK		0x00000040 /**< Read back Mask */
#define XHI_SR_IN_ABORT_N_MASK	0x00000020 /**< Select Map Abort Mask */
#define XHI_SR_DONE_MASK	0x00000001 /**< Done bit Mask */
#define XHI_SR_EOS_MASK 	0x00000004 /**< EOS bit Mask */

/* @} */

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

#define XHwIcap_In32 Xil_In32

#define XHwIcap_Out32 Xil_Out32

/****************************************************************************/
/**
*
* Read from the specified HwIcap device register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to select the specific register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XHwIcap_ReadReg(u32 BaseAddress, u32 RegOffset);
*
******************************************************************************/
#define XHwIcap_ReadReg(BaseAddress, RegOffset) \
	XHwIcap_In32((BaseAddress) + (RegOffset))

/***************************************************************************/
/**
*
* Write to the specified HwIcap device register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to select the specific register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XHwIcap_WriteReg(u32 BaseAddress, u32 RegOffset,
*					u32 RegisterValue);
******************************************************************************/
#define XHwIcap_WriteReg(BaseAddress, RegOffset, RegisterValue) \
	XHwIcap_Out32((BaseAddress) + (RegOffset), (RegisterValue))

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


#ifdef __cplusplus
}
#endif

#endif         /* end of protection macro */


/** @} */
