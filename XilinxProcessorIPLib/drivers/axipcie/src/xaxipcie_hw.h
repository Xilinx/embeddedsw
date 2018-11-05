/******************************************************************************
*
* Copyright (C) 2011 - 2014 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xaxipcie_hw.h
* @addtogroup axipcie_v3_1
* @{
*
* This header file contains identifiers and basic driver functions for the
* XAxiPcie device driver.
*
* @note		None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rkv  03/03/11  Original code.
* 2.00a nm   09/19/11  Root port related changes are done.
*
* </pre>
*
******************************************************************************/
#ifndef XAXIPCIE_HW_H       /* prevent circular inclusions */
#define XAXIPCIE_HW_H       /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#ifndef _ASMLANGUAGE

#include "xil_types.h"
#include "xil_io.h"

#endif /* _ASMLANGUAGE */

/************************** Constant Definitions *****************************/


/** @name Registers
 *
 * Register offsets for this device. Some of the registers
 * are configurable at hardware build time such that may or may not exist
 * in the hardware.
 * @{
 */

#define XAXIPCIE_PCIE_CORE_OFFSET		0x000 /**< PCI Express hard
						       * core configuration
						       * register offset
						       */
#define XAXIPCIE_VSECC_OFFSET			0x128 /**<
						       * VSEC Capability
						       * Register
						       */
#define XAXIPCIE_VSECH_OFFSET			0x12C /**<
						       * VSEC Header Register
						       */
#define XAXIPCIE_BI_OFFSET			0x130 /**<
						       * Bridge Info Register
						       */
#define XAXIPCIE_BSC_OFFSET			0x134 /**<
						       * Bridge Status and
						       * Control Register
						       */
#define XAXIPCIE_ID_OFFSET			0x138 /**<
						       * Interrupt Decode
						       * Register
						       */
#define XAXIPCIE_IM_OFFSET			0x13C /**<
						       * Interrupt Mask
						       * Register
						       */
#define XAXIPCIE_BL_OFFSET			0x140 /**<
						       * Bus Location Register
						       */
#define XAXIPCIE_PHYSC_OFFSET			0x144 /**<
						       * Physical status and
						       * Control Register
						       */
#define XAXIPCIE_RPSC_OFFSET			0x148 /**<
						       * Root Port Status &
						       * Control Register
						       */
#define XAXIPCIE_RPMSIB_UPPER_OFFSET		0x14C /**<
						       * Root Port MSI Base 1
						       * Register Upper 32 bits
						       * from 64 bit address
						       * are written
						       */
#define XAXIPCIE_RPMSIB_LOWER_OFFSET		0x150 /**<
						       * Root Port MSI Base 2
						       * Register Lower 32 bits
						       * from 64 bit address
						       * are written
						       */
#define XAXIPCIE_RPEFR_OFFSET			0x154 /**<
						       * Root Port Error FIFO
						       * Read Register
						       */
#define XAXIPCIE_RPIFR1_OFFSET			0x158 /**<
						       * Root Port Interrupt
						       * FIFO Read1 Register
						       */
#define XAXIPCIE_RPIFR2_OFFSET			0x15C /**<
						       * Root Port Interrupt
						       * FIFO Read2 Register
						       */


#define XAXIPCIE_AXIBAR2PCIBAR_0U_OFFSET	0x208 /**<
						       * AXIBAR 2 PCIBAR
						       * translation 0 upper
						       * 32 bits
						       */
#define XAXIPCIE_AXIBAR2PCIBAR_0L_OFFSET	0x20C /**<
						       * AXIBAR to PCIBAR
						       * translation 0 lower
						       * 32 bits
						       */
#define XAXIPCIE_AXIBAR2PCIBAR_1U_OFFSET	0x210 /**<
						       * AXIBAR to PCIBAR
						       * translation 1 upper
						       * 32 bits
						       */
#define XAXIPCIE_AXIBAR2PCIBAR_1L_OFFSET	0x214 /**<
						       * AXIBAR to PCIBAR
						       * translation 1 lower
						       * 32 bits
						       */
#define XAXIPCIE_AXIBAR2PCIBAR_2U_OFFSET	0x218 /**<
						       * AXIBAR to PCIBAR
						       * translation 2 upper
						       * 32 bits
						       */
#define XAXIPCIE_AXIBAR2PCIBAR_2L_OFFSET	0x21C /**<
						       * AXIBAR to PCIBAR
						       * translation 2 lower
						       * 32 bits
						       */
#define XAXIPCIE_AXIBAR2PCIBAR_3U_OFFSET	0x220 /**<
						       * AXIBAR to PCIBAR
						       * translation 3 upper
						       * 32 bits
						       */
#define XAXIPCIE_AXIBAR2PCIBAR_3L_OFFSET	0x224 /**<
						       * AXIBAR to PCIBAR
						       * translation 3 lower
						       * 32 bits
						       */
#define XAXIPCIE_AXIBAR2PCIBAR_4U_OFFSET	0x228 /**<
						       * AXIBAR to PCIBAR
						       * translation 4 upper
						       * 32 bits
						       */
#define XAXIPCIE_AXIBAR2PCIBAR_4L_OFFSET	0x22C /**<
						       * AXIBAR to PCIBAR
						       * translation 4 lower
						       * 32 bits
						       */
#define XAXIPCIE_AXIBAR2PCIBAR_5U_OFFSET	0x230 /**<
						       * AXIBAR to PCIBAR
						       * translation 5 upper
						       * 32 bits
						       */
#define XAXIPCIE_AXIBAR2PCIBAR_5L_OFFSET	0x234 /**<
						       * AXIBAR to PCIBAR
						       * translation 5 lower
						       * 32 bits
						       */
/*@}*/

/** @name VSECC Register bitmaps and masks
 * @{
 */
#define XAXIPCIE_VSECC_ID_MASK		0x0000FFFF /**< Vsec capability Id */
#define XAXIPCIE_VSECC_VER_MASK		0x000F0000 /**< Version of capability
						     *  Structure
						     */
#define XAXIPCIE_VSECC_NEXT_MASK	0xFFF00000 /**< Offset to next
						     *  capability
						     */
#define XAXIPCIE_VSECC_VER_SHIFT	16 	   /**< VSEC Version shift */
#define XAXIPCIE_VSECC_NEXT_SHIFT	20	   /**< Next capability offset
						     *  shift
						     */
/*@}*/

/** @name VSECH Register bitmaps and masks
 *
 * @{
 */
#define XAXIPCIE_VSECH_ID_MASK		0x0000FFFF /**< Vsec structure Id */
#define XAXIPCIE_VSECH_REV_MASK		0x000F0000 /**< Vsec header version*/
#define XAXIPCIE_VSECH_LEN_MASK		0xFFF00000 /**< Length of Vsec
						     *  capability structure
						     */
#define XAXIPCIE_VSECH_REV_SHIFT	16 	   /**< Vsec version shift */
#define XAXIPCIE_VSECH_LEN_SHIFT	20 	   /**< Vsec length shift */
/*@}*/

/** @name Bridge Info Register bitmaps and masks
 *
 * @{
 */
#define XAXIPCIE_BI_GEN2_MASK		0x00000001 /**< PCIe Gen2 Speed
						     *  Support Mask
						     */
#define XAXIPCIE_BI_RP_MASK		0x00000002 /**< PCIe Root Port Support
						     */
#define XAXIPCIE_UP_CONFIG_CAPABLE	0x00000004 /**< Up Config Capable */

#define XAXIPCIE_BI_ECAM_SIZE_MASK	0x00070000 /**< ECAM size */
#define XAXIPCIE_BI_RP_SHIFT		1 	   /**< PCIe Root Port Shift */
#define XAXIPCIE_BI_ECAM_SIZE_SHIFT	16 	   /**< PCIe ECAM Size Shift */
/*@}*/

/** @name Bridge Status & Control Register bitmaps and masks
 *
 * @{
 */
#define XAXIPCIE_BSC_ECAM_BUSY_MASK	0x00000001 /**< ECAM Busy Status */
#define XAXIPCIE_BSC_GI_MASK		0x00000100 /**< Global Interrupt
						     *  Disable
						     */
#define XAXIPCIE_BSC_RW1C_MASK		0x00010000 /**< RW Permissions to RW1C
						     *  Registers
						     */
#define XAXIPCIE_BSC_RO_MASK		0x00020000 /**< RW Permissions to RO
						     *  Registers
						     */
#define XAXIPCIE_BSC_GI_SHIFT		8 	   /**< Global Interrupt Disable
						     *  Shift
						     */
#define XAXIPCIE_BSC_RW1C_SHIFT		16 	   /**< RW1C Shift */
#define XAXIPCIE_BSC_RO_SHIFT		17 	   /**< RO as RW Shift */
/*@}*/

/** @name Interrupt Decode Register bitmaps and masks
 *
 * @{
 */
#define XAXIPCIE_ID_LINK_DOWN_MASK 	 0x00000001 /**< Link Down Mask */
#define XAXIPCIE_ID_ECRC_ERR_MASK 	 0x00000002 /**< Rx Packet CRC failed */
#define XAXIPCIE_ID_STR_ERR_MASK 	 0x00000004 /**< Streaming Error Mask */
#define XAXIPCIE_ID_HOT_RST_MASK 	 0x00000008 /**< Hot Reset Mask */
#define XAXIPCIE_ID_CFG_COMPL_STATE_MASK 0x000000E0 /**< Cfg Completion
						      *  Status Mask
						      */
#define XAXIPCIE_ID_CFG_TIMEOUT_MASK 	 0x00000100 /**< Cfg timeout Mask */
#define XAXIPCIE_ID_CORRECTABLE_ERR_MASK 0x00000200 /**< Correctable Error
						      *  Mask
						      */
#define XAXIPCIE_ID_NONFATAL_ERR_MASK 	 0x00000400 /**< Non-Fatal Error Mask */
#define XAXIPCIE_ID_FATAL_ERR_MASK 	 0x00000800 /**< Fatal Error Mask */
#define XAXIPCIE_ID_INTX_INTERRUPT	 0x00010000 /**< INTX Interrupt */
#define XAXIPCIE_ID_MSI_INTERRUPT	 0x00020000 /**< MSI Interrupt */
#define XAXIPCIE_ID_UNSUPP_CMPL_MASK 	 0x00100000 /**< Slave Unsupported
						      *  Request Mask
						      */
#define XAXIPCIE_ID_UNEXP_CMPL_MASK 	 0x00200000 /**< Slave Unexpected
						      *  Completion Mask
						      */
#define XAXIPCIE_ID_CMPL_TIMEOUT_MASK 	 0x00400000 /**< Slave completion
						      *  Time Mask
						      */
#define XAXIPCIE_ID_SLV_EP_MASK 	 0x00800000 /**< Slave Error
						      *  Poison Mask
						      */
#define XAXIPCIE_ID_CMPL_ABT_MASK 	 0x01000000 /**< Slave completion
						      *  Abort Mask
						      */
#define XAXIPCIE_ID_ILL_BURST_MASK 	 0x02000000 /**< Slave Illegal
						      *  Burst Mask
						      */
#define XAXIPCIE_ID_DECODE_ERR_MASK 	 0x04000000 /**< Master Decode
						      *  Error Interrupt Mask
						      */
#define XAXIPCIE_ID_SLAVE_ERR_MASK 	 0x08000000 /**< Master Slave Error
						      *  Interrupt Mask
						      */
#define XAXIPCIE_ID_MASTER_EP_MASK 	 0x10000000 /**< Master Error Poison
						      *  Mask
						      */
#define XAXIPCIE_ID_CLEAR_ALL_MASK 	 0xFFFFFFFF /**< Mask of all
						      *  Interrupts
						      */
/*@}*/


/** @name Interrupt Mask Register bitmaps and masks
 *
 * @{
 */
#define XAXIPCIE_IM_ENABLE_ALL_MASK 	0xFFFFFFFF /**< Enable All Interrupts */
#define XAXIPCIE_IM_DISABLE_ALL_MASK	0x00000000 /**< Disable All
						     *  Interrupts
						     */
/*@}*/

/** @name Bus Location Register bitmaps and masks
 *
 * @{
 */
#define XAXIPCIE_BL_FUNC_MASK	0x00000007 /**< Requester ID Function Number */
#define XAXIPCIE_BL_DEV_MASK	0x000000F8 /**< Requester ID Device Number   */
#define XAXIPCIE_BL_BUS_MASK	0x0000FF00 /**< Requester ID Bus Number */
#define XAXIPCIE_BL_PORT_MASK	0x00FF0000 /**< Requester ID Port Number */

#define XAXIPCIE_BL_DEV_SHIFT		3  /**< Requester ID Device Number
					     *  Shift Value
					     */
#define XAXIPCIE_BL_BUS_SHIFT		8  /**< Requester ID Bus Number Shift
					     *  Value
					     */
#define XAXIPCIE_BL_PORT_SHIFT		16 /**< Requester ID Bus Number Shift
					     *  Value
					     */
/*@}*/

/** @name PHY Status & Control Register bitmaps and masks
 *
 * @{
 */
#define XAXIPCIE_PHYSC_LINK_RATE_MASK	0x00000001 /**< Link Rate */
#define XAXIPCIE_PHYSC_LINK_WIDTH_MASK	0x00000006 /**< Link Width Mask */
#define XAXIPCIE_PHYSC_LTSSM_STATE_MASK	0x000001F8 /**< LTSSM State Mask */
#define XAXIPCIE_PHYSC_LANE_REV_MASK	0x00000600 /**< Lane Reversal Mask */
#define XAXIPCIE_PHYSC_LINK_UP_MASK	0x00000800 /**< Link Up Status Mask */
#define XAXIPCIE_PHYSC_DLW_MASK		0x00030000 /**< Directed Link
						     *  Width to change Mask
						     */
#define XAXIPCIE_PHYSC_DLWS_MASK	0x00040000 /**< Directed Link Width
						     *  Speed to change Mask
						     */
#define XAXIPCIE_PHYSC_DLA_MASK		0x00080000 /**< Directed Link Change
						     *  change to reliability or
						     * Autonomus Mask
						     */
#define XAXIPCIE_PHYSC_DLC_MASK		0x00300000 /**< Directed Link change
						      * Mask
						      */

#define XAXIPCIE_PHYSC_LINK_WIDTH_SHIFT		1  /**< Link Status Shift */
#define XAXIPCIE_PHYSC_LTSSM_STATE_SHIFT	3  /**< LTSSM State Shift */
#define XAXIPCIE_PHYSC_LANE_REV_SHIFT		9  /**< Lane Reversal Shift */
#define XAXIPCIE_PHYSC_LINK_UP_SHIFT		11 /**< Link Up Status Shift */
#define XAXIPCIE_PHYSC_DLW_SHIFT		16 /**< Directed Link Width
						     *  to change Shift
						     */
#define XAXIPCIE_PHYSC_DLWS_SHIFT		18  /**< Directed Link Width
						      *  Speed to change Shift
						      */
#define XAXIPCIE_PHYSC_DLA_SHIFT		19  /**< Directed Link change to
						      *  reliability or
						      *  Autonomus Shift
						      */
#define XAXIPCIE_PHYSC_DLC_SHIFT		20  /**< Directed Link
						      *  change Shift
						      */
/*@}*/

/** @name Root Port Status/Control Register bitmaps and masks
 *
 * @{
 */

#define XAXIPCIE_RPSC_MASK			0x0FFF0001 /**<
							    * Root Port
							    * Register mask
							    */
#define XAXIPCIE_RPSC_BRIDGE_ENABLE_MASK	0x00000001 /**<
							    *  Bridge Enable
							    *  Mask
							    */

#define XAXIPCIE_RPSC_ERR_FIFO_NOT_EMPTY_MASK	0x00010000 /**<
							    * Root Port Error
							    * FIFO Not Empty
							    */

#define XAXIPCIE_RPSC_ERR_FIFO_OVERFLOW_MASK	0x00020000 /**<
							    * Root Port Error
							    * FIFO Overflow
							    */

#define XAXIPCIE_RPSC_INT_FIFO_NOT_EMPTY_MASK	0x00040000 /**<
							    * Root Port
							    * Interrupt FIFO
							    * Not Empty
							    */

#define XAXIPCIE_RPSC_INT_FIFO_OVERFLOW_MASK	0x00080000 /**<
							    * Root Port
							    * Interrupt FIFO
							    * Overflow
							    */

#define XAXIPCIE_RPSC_COMP_TIMEOUT_MASK		0x0FF00000 /**<
							    * Root Port
							    * Completion
							    * Timeout
							    */

#define XAXIPCIE_RPSC_ERR_FIFO_NOT_EMPTY_SHIFT	16 /**<
						    * Root Port Error FIFO
						    * Empty Shift
						    */
#define XAXIPCIE_RPSC_ERR_FIFO_OVERFLOW_SHIFT	17 /**<
						    * Root Port Error FIFO
						    * Overflow Shift
						    */
#define XAXIPCIE_RPSC_INT_FIFO_NOT_EMPTY_SHIFT	18 /**<
						    * Root Port Interrupt FIFO
						    * Empty Shift
						    */
#define XAXIPCIE_RPSC_INT_FIFO_OVERFLOW_SHIFT	19 /**<
						    * Root Port Interrupt FIFO
						    * Overflow Shift
						    */
#define XAXIPCIE_RPSC_COMP_TIMEOUT_SHIFT	20 /**<
						    * Root Port Completion
						    * Timeout Shift
						    */


/** @name Root Port MSI Base Register bitmaps and masks
 *
 * @{
 */
#define XAXIPCIE_RPMSIB_UPPER_MASK 	0xFFFFFFFF /**<
						    * Upper 32 bits of 64 bit
						    * MSI Base Address
						    */
#define XAXIPCIE_RPMSIB_UPPER_SHIFT 	32	   /* Shift of Upper 32 bits */
#define XAXIPCIE_RPMSIB_LOWER_MASK 	0xFFFFF000 /**<
						    * Lower 32 bits of 64 bit
						    * MSI Base Address
						    */
/*@}*/

/** @name Root Port Error FIFO Read Register bitmaps and masks
 *
 * @{
 */
#define XAXIPCIE_RPEFR_REQ_ID_MASK 	0x0000FFFF /**<
						    * Requester of Error Msg
						    */

#define XAXIPCIE_RPEFR_ERR_TYPE_MASK 	0x00030000 /**<
						    * Type of Error
						    */

#define XAXIPCIE_RPEFR_ERR_VALID_MASK 	0x00040000 /**<
						    * Error Read Succeeded
						    * Status
						    */

#define XAXIPCIE_RPEFR_ERR_TYPE_SHIFT 	16 /**< Type of Error Shift*/

#define XAXIPCIE_RPEFR_ERR_VALID_SHIFT 	18 /**< Error Read Succeeded Status
					    * Shift */

/*@}*/

/** @name Root Port Interrupt FIFO Read 1 Register bitmaps and masks
 *
 * @{
 */
#define XAXIPCIE_RPIFR1_REQ_ID_MASK 		0x0000FFFF /**<
							    * Requester Id of
							    * Interrupt Message
							    */
#define XAXIPCIE_RPIFR1_MSI_ADDR_MASK 		0x07FF0000 /**< MSI Address */
#define XAXIPCIE_RPIFR1_INTR_LINE_MASK 		0x18000000 /**< Intr Line Mask
							     */
#define XAXIPCIE_RPIFR1_INTR_ASSERT_MASK 	0x20000000 /**<
							    * Whether Interrupt
							    * INTx is asserted
							    */

#define XAXIPCIE_RPIFR1_MSIINTR_VALID_MASK 	0x40000000 /**<
							    * Whether Interrupt
							    * is MSI or INTx
							    */

#define XAXIPCIE_RPIFR1_INTR_VALID_MASK 	0x80000000 /**<
							    * Interrupt Read
							    * Succeeded Status
						 	    */

#define XAXIPCIE_RPIFR1_MSI_ADDR_SHIFT 	16 /**< MSI Address Shift */

#define XAXIPCIE_RPIFR1_MSIINTR_VALID_SHIFT 30 /**< MSI/INTx Interrupt Shift */

#define XAXIPCIE_RPIFR1_INTR_VALID_SHIFT 31 /**< Interrupt Read Valid Shift */

/*@}*/

/** @name Root Port Interrupt FIFO Read 2 Register bitmaps and masks
 *
 * @{
 */
#define XAXIPCIE_RPIFR2_MSG_DATA_MASK 	0x0000FFFF /**<
						    * Pay Load for MSI
						    * Message
						    */
/*@}*/

/** @name ECAM Address Register bitmaps and masks
 *
 * @{
 */
#define XAXIPCIE_ECAM_MASK	0x0FFFFFFF  /**< Mask of all valid bits */
#define XAXIPCIE_ECAM_BUS_MASK	0x0FF00000  /**< Bus Number Mask */
#define XAXIPCIE_ECAM_DEV_MASK	0x000F8000  /**< Device Number Mask */
#define XAXIPCIE_ECAM_FUN_MASK	0x00007000  /**< Function Number Mask */
#define XAXIPCIE_ECAM_REG_MASK	0x00000FFC  /**< Register Number Mask */
#define XAXIPCIE_ECAM_BYT_MASK	0x00000003  /**< Byte Address Mask */

#define XAXIPCIE_ECAM_BUS_SHIFT		20  /**< Bus Number Shift Value */
#define XAXIPCIE_ECAM_DEV_SHIFT		15  /**< Device Number Shift Value */
#define XAXIPCIE_ECAM_FUN_SHIFT		12  /**< Function Number Shift Value */
#define XAXIPCIE_ECAM_REG_SHIFT		2   /**< Register Number Shift Value */
#define XAXIPCIE_ECAM_BYT_SHIFT		0   /**< Byte Offset Shift Value */
/*@}*/

/* Offset used for getting the VSEC register contents */
#define XAXIPCIE_VSEC2_OFFSET_WRT_VSEC1 	0xD8

/****************** Macros (Inline Functions) Definitions ********************/
/*****************************************************************************/
/**
* Macro to read register.
*
* @param	BaseAddress is the base address of the PCIe.
* @param	RegOffset is the register offset.
*
* @return	Value of the register.
*
* @note		C-style signature:
*		u32 XAxiPcie_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XAxiPcie_ReadReg(BaseAddress, RegOffset) \
	Xil_In32((BaseAddress) + (RegOffset))


/*****************************************************************************/
/**
* Macro to write register.
*
* @param 	BaseAddress is the base address of the PCIe.
* @param 	RegOffset is the register offset.
* @param 	Data is the data to write.
*
* @return 	None
*
* @note		C-style signature:
*		void XAxiPcie_WriteReg(u32 BaseAddress, u32 RegOffset,
*								u32 Data)
*
******************************************************************************/
#define XAxiPcie_WriteReg(BaseAddress, RegOffset, Data) \
	Xil_Out32((BaseAddress) + (RegOffset), (Data))

/*************************** Variable Definitions ****************************/

/*************************** Function Prototypes *****************************/
#ifdef __cplusplus
}
#endif

#endif /* XAXIPCIE_HW_H */

/** @} */
