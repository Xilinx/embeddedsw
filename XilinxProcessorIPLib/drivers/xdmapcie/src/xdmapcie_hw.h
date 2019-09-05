/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/******************************************************************************/
/**
*
* @file xdmapcie_hw.h
*
* This header file contains identifiers and basic driver functions for the
* XDmaPcie device driver.
*
* @note		None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0	tk	01/30/2019	First release
* </pre>
*
******************************************************************************/
#ifndef XDMAPCIE_HW_H       /* prevent circular inclusions */
#define XDMAPCIE_HW_H       /* by using protection macros */

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

#define XDMAPCIE_PCIE_CORE_OFFSET		0x000 /**< PCI Express hard
						       * core configuration
						       * register offset
						       */

#ifdef versal
#define XDMAPCIE_VSECC_OFFSET			0xE00 /**<
						       * VSEC Capability
						       * Register
						       */
#define XDMAPCIE_VSECH_OFFSET			0xE04 /**<
						       * VSEC Header Register
						       */
#define XDMAPCIE_BI_OFFSET			0xE08 /**<
						       * Bridge Info Register
						       */
#define XDMAPCIE_BSC_OFFSET			0xE0C /**<
						       * Bridge Status and
						       * Control Register
						       */
#define XDMAPCIE_ID_OFFSET			0xE10 /**<
						       * Interrupt Decode
						       * Register
						       */
#define XDMAPCIE_IM_OFFSET			0xE14 /**<
						       * Interrupt Mask
						       * Register
						       */
#define XDMAPCIE_BL_OFFSET			0xE18 /**<
						       * Bus Location Register
						       */
#define XDMAPCIE_PHYSC_OFFSET			0xE1C /**<
						       * Physical status and
						       * Control Register
						       */
#define XDMAPCIE_RPSC_OFFSET			0xE20 /**<
						       * Root Port Status &
						       * Control Register
						       */
#define XDMAPCIE_RPMSIB_UPPER_OFFSET		0xE24 /**<
						       * Root Port MSI Base 1
						       * Register Upper 32 bits
						       * from 64 bit address
						       * are written
						       */
#define XDMAPCIE_RPMSIB_LOWER_OFFSET		0xE28 /**<
						       * Root Port MSI Base 2
						       * Register Lower 32 bits
						       * from 64 bit address
						       * are written
						       */
#define XDMAPCIE_RPEFR_OFFSET			0xE2C /**<
						       * Root Port Error FIFO
						       * Read Register
						       */
#define XDMAPCIE_RPIFR1_OFFSET			0xE30 /**<
						       * Root Port Interrupt
						       * FIFO Read1 Register
						       */
#define XDMAPCIE_RPIFR2_OFFSET			0xE34 /**<
						       * Root Port Interrupt
						       * FIFO Read2 Register
						       */


#define XDMAPCIE_AXIBAR2PCIBAR_0U_OFFSET	0xEE0 /**<
						       * AXIBAR 2 PCIBAR
						       * translation 0 upper
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_0L_OFFSET	0xEE4 /**<
						       * AXIBAR to PCIBAR
						       * translation 0 lower
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_1U_OFFSET	0xEE8 /**<
						       * AXIBAR to PCIBAR
						       * translation 1 upper
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_1L_OFFSET	0xEEC /**<
						       * AXIBAR to PCIBAR
						       * translation 1 lower
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_2U_OFFSET	0xEF0 /**<
						       * AXIBAR to PCIBAR
						       * translation 2 upper
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_2L_OFFSET	0xEF4 /**<
						       * AXIBAR to PCIBAR
						       * translation 2 lower
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_3U_OFFSET	0xEF8 /**<
						       * AXIBAR to PCIBAR
						       * translation 3 upper
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_3L_OFFSET	0xEFC /**<
						       * AXIBAR to PCIBAR
						       * translation 3 lower
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_4U_OFFSET	0xF00 /**<
						       * AXIBAR to PCIBAR
						       * translation 4 upper
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_4L_OFFSET	0xF04 /**<
						       * AXIBAR to PCIBAR
						       * translation 4 lower
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_5U_OFFSET	0xF08 /**<
						       * AXIBAR to PCIBAR
						       * translation 5 upper
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_5L_OFFSET	0xF0C /**<
						       * AXIBAR to PCIBAR
						       * translation 5 lower
						       * 32 bits
						       */
#else
#define XDMAPCIE_VSECC_OFFSET			0x128 /**<
						       * VSEC Capability
						       * Register
						       */
#define XDMAPCIE_VSECH_OFFSET			0x12C /**<
						       * VSEC Header Register
						       */
#define XDMAPCIE_BI_OFFSET			0x130 /**<
						       * Bridge Info Register
						       */
#define XDMAPCIE_BSC_OFFSET			0x134 /**<
						       * Bridge Status and
						       * Control Register
						       */
#define XDMAPCIE_ID_OFFSET			0x138 /**<
						       * Interrupt Decode
						       * Register
						       */
#define XDMAPCIE_IM_OFFSET			0x13C /**<
						       * Interrupt Mask
						       * Register
						       */
#define XDMAPCIE_BL_OFFSET			0x140 /**<
						       * Bus Location Register
						       */
#define XDMAPCIE_PHYSC_OFFSET			0x144 /**<
						       * Physical status and
						       * Control Register
						       */
#define XDMAPCIE_RPSC_OFFSET			0x148 /**<
						       * Root Port Status &
						       * Control Register
						       */
#define XDMAPCIE_RPMSIB_UPPER_OFFSET		0x14C /**<
						       * Root Port MSI Base 1
						       * Register Upper 32 bits
						       * from 64 bit address
						       * are written
						       */
#define XDMAPCIE_RPMSIB_LOWER_OFFSET		0x150 /**<
						       * Root Port MSI Base 2
						       * Register Lower 32 bits
						       * from 64 bit address
						       * are written
						       */
#define XDMAPCIE_RPEFR_OFFSET			0x154 /**<
						       * Root Port Error FIFO
						       * Read Register
						       */
#define XDMAPCIE_RPIFR1_OFFSET			0x158 /**<
						       * Root Port Interrupt
						       * FIFO Read1 Register
						       */
#define XDMAPCIE_RPIFR2_OFFSET			0x15C /**<
						       * Root Port Interrupt
						       * FIFO Read2 Register
						       */


#define XDMAPCIE_AXIBAR2PCIBAR_0U_OFFSET	0x208 /**<
						       * AXIBAR 2 PCIBAR
						       * translation 0 upper
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_0L_OFFSET	0x20C /**<
						       * AXIBAR to PCIBAR
						       * translation 0 lower
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_1U_OFFSET	0x210 /**<
						       * AXIBAR to PCIBAR
						       * translation 1 upper
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_1L_OFFSET	0x214 /**<
						       * AXIBAR to PCIBAR
						       * translation 1 lower
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_2U_OFFSET	0x218 /**<
						       * AXIBAR to PCIBAR
						       * translation 2 upper
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_2L_OFFSET	0x21C /**<
						       * AXIBAR to PCIBAR
						       * translation 2 lower
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_3U_OFFSET	0x220 /**<
						       * AXIBAR to PCIBAR
						       * translation 3 upper
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_3L_OFFSET	0x224 /**<
						       * AXIBAR to PCIBAR
						       * translation 3 lower
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_4U_OFFSET	0x228 /**<
						       * AXIBAR to PCIBAR
						       * translation 4 upper
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_4L_OFFSET	0x22C /**<
						       * AXIBAR to PCIBAR
						       * translation 4 lower
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_5U_OFFSET	0x230 /**<
						       * AXIBAR to PCIBAR
						       * translation 5 upper
						       * 32 bits
						       */
#define XDMAPCIE_AXIBAR2PCIBAR_5L_OFFSET	0x234 /**<
						       * AXIBAR to PCIBAR
						       * translation 5 lower
						       * 32 bits
						       */
#endif /* versal */

/*@}*/

/** @name VSECC Register bitmaps and masks
 * @{
 */
#define XDMAPCIE_VSECC_ID_MASK		0x0000FFFF /**< Vsec capability Id */
#define XDMAPCIE_VSECC_VER_MASK		0x000F0000 /**< Version of capability
						     *  Structure
						     */
#define XDMAPCIE_VSECC_NEXT_MASK	0xFFF00000 /**< Offset to next
						     *  capability
						     */
#define XDMAPCIE_VSECC_VER_SHIFT	16 	   /**< VSEC Version shift */
#define XDMAPCIE_VSECC_NEXT_SHIFT	20	   /**< Next capability offset
						     *  shift
						     */
/*@}*/

/** @name VSECH Register bitmaps and masks
 *
 * @{
 */
#define XDMAPCIE_VSECH_ID_MASK		0x0000FFFF /**< Vsec structure Id */
#define XDMAPCIE_VSECH_REV_MASK		0x000F0000 /**< Vsec header version*/
#define XDMAPCIE_VSECH_LEN_MASK		0xFFF00000 /**< Length of Vsec
						     *  capability structure
						     */
#define XDMAPCIE_VSECH_REV_SHIFT	16 	   /**< Vsec version shift */
#define XDMAPCIE_VSECH_LEN_SHIFT	20 	   /**< Vsec length shift */
/*@}*/

/** @name Bridge Info Register bitmaps and masks
 *
 * @{
 */
#define XDMAPCIE_BI_GEN2_MASK		0x00000001 /**< PCIe Gen2 Speed
						     *  Support Mask
						     */
#define XDMAPCIE_BI_RP_MASK		0x00000002 /**< PCIe Root Port Support
						     */
#define XDMAPCIE_UP_CONFIG_CAPABLE	0x00000004 /**< Up Config Capable */

#define XDMAPCIE_BI_ECAM_SIZE_MASK	0x00070000 /**< ECAM size */
#define XDMAPCIE_BI_RP_SHIFT		1 	   /**< PCIe Root Port Shift */
#define XDMAPCIE_BI_ECAM_SIZE_SHIFT	16 	   /**< PCIe ECAM Size Shift */
/*@}*/

/** @name Bridge Status & Control Register bitmaps and masks
 *
 * @{
 */
#define XDMAPCIE_BSC_ECAM_BUSY_MASK	0x00000001 /**< ECAM Busy Status */
#define XDMAPCIE_BSC_GI_MASK		0x00000100 /**< Global Interrupt
						     *  Disable
						     */
#define XDMAPCIE_BSC_RW1C_MASK		0x00010000 /**< RW Permissions to RW1C
						     *  Registers
						     */
#define XDMAPCIE_BSC_RO_MASK		0x00020000 /**< RW Permissions to RO
						     *  Registers
						     */
#define XDMAPCIE_BSC_GI_SHIFT		8 	   /**< Global Interrupt Disable
						     *  Shift
						     */
#define XDMAPCIE_BSC_RW1C_SHIFT		16 	   /**< RW1C Shift */
#define XDMAPCIE_BSC_RO_SHIFT		17 	   /**< RO as RW Shift */
/*@}*/

/** @name Interrupt Decode Register bitmaps and masks
 *
 * @{
 */
#define XDMAPCIE_ID_LINK_DOWN_MASK 	 0x00000001 /**< Link Down Mask */
#define XDMAPCIE_ID_ECRC_ERR_MASK 	 0x00000002 /**< Rx Packet CRC failed */
#define XDMAPCIE_ID_STR_ERR_MASK 	 0x00000004 /**< Streaming Error Mask */
#define XDMAPCIE_ID_HOT_RST_MASK 	 0x00000008 /**< Hot Reset Mask */
#define XDMAPCIE_ID_CFG_COMPL_STATE_MASK 0x000000E0 /**< Cfg Completion
						      *  Status Mask
						      */
#define XDMAPCIE_ID_CFG_TIMEOUT_MASK 	 0x00000100 /**< Cfg timeout Mask */
#define XDMAPCIE_ID_CORRECTABLE_ERR_MASK 0x00000200 /**< Correctable Error
						      *  Mask
						      */
#define XDMAPCIE_ID_NONFATAL_ERR_MASK 	 0x00000400 /**< Non-Fatal Error Mask */
#define XDMAPCIE_ID_FATAL_ERR_MASK 	 0x00000800 /**< Fatal Error Mask */
#define XDMAPCIE_ID_INTX_INTERRUPT	 0x00010000 /**< INTX Interrupt */
#define XDMAPCIE_ID_MSI_INTERRUPT	 0x00020000 /**< MSI Interrupt */
#define XDMAPCIE_ID_UNSUPP_CMPL_MASK 	 0x00100000 /**< Slave Unsupported
						      *  Request Mask
						      */
#define XDMAPCIE_ID_UNEXP_CMPL_MASK 	 0x00200000 /**< Slave Unexpected
						      *  Completion Mask
						      */
#define XDMAPCIE_ID_CMPL_TIMEOUT_MASK 	 0x00400000 /**< Slave completion
						      *  Time Mask
						      */
#define XDMAPCIE_ID_SLV_EP_MASK 	 0x00800000 /**< Slave Error
						      *  Poison Mask
						      */
#define XDMAPCIE_ID_CMPL_ABT_MASK 	 0x01000000 /**< Slave completion
						      *  Abort Mask
						      */
#define XDMAPCIE_ID_ILL_BURST_MASK 	 0x02000000 /**< Slave Illegal
						      *  Burst Mask
						      */
#define XDMAPCIE_ID_DECODE_ERR_MASK 	 0x04000000 /**< Master Decode
						      *  Error Interrupt Mask
						      */
#define XDMAPCIE_ID_SLAVE_ERR_MASK 	 0x08000000 /**< Master Slave Error
						      *  Interrupt Mask
						      */
#define XDMAPCIE_ID_MASTER_EP_MASK 	 0x10000000 /**< Master Error Poison
						      *  Mask
						      */
#define XDMAPCIE_ID_CLEAR_ALL_MASK 	 0xFFFFFFFF /**< Mask of all
						      *  Interrupts
						      */
/*@}*/


/** @name Interrupt Mask Register bitmaps and masks
 *
 * @{
 */
#define XDMAPCIE_IM_ENABLE_ALL_MASK 	0xFFFFFFFF /**< Enable All Interrupts */
#define XDMAPCIE_IM_DISABLE_ALL_MASK	0x00000000 /**< Disable All
						     *  Interrupts
						     */
/*@}*/

/** @name Bus Location Register bitmaps and masks
 *
 * @{
 */
#define XDMAPCIE_BL_FUNC_MASK	0x00000007 /**< Requester ID Function Number */
#define XDMAPCIE_BL_DEV_MASK	0x000000F8 /**< Requester ID Device Number   */
#define XDMAPCIE_BL_BUS_MASK	0x0000FF00 /**< Requester ID Bus Number */
#define XDMAPCIE_BL_PORT_MASK	0x00FF0000 /**< Requester ID Port Number */

#define XDMAPCIE_BL_DEV_SHIFT		3  /**< Requester ID Device Number
					     *  Shift Value
					     */
#define XDMAPCIE_BL_BUS_SHIFT		8  /**< Requester ID Bus Number Shift
					     *  Value
					     */
#define XDMAPCIE_BL_PORT_SHIFT		16 /**< Requester ID Bus Number Shift
					     *  Value
					     */
/*@}*/

/** @name PHY Status & Control Register bitmaps and masks
 *
 * @{
 */
#define XDMAPCIE_PHYSC_LINK_RATE_MASK	0x00000001 /**< Link Rate */
#define XDMAPCIE_PHYSC_LINK_WIDTH_MASK	0x00000006 /**< Link Width Mask */
#define XDMAPCIE_PHYSC_LTSSM_STATE_MASK	0x000001F8 /**< LTSSM State Mask */
#define XDMAPCIE_PHYSC_LANE_REV_MASK	0x00000600 /**< Lane Reversal Mask */
#define XDMAPCIE_PHYSC_LINK_UP_MASK	0x00000800 /**< Link Up Status Mask */
#define XDMAPCIE_PHYSC_DLW_MASK		0x00030000 /**< Directed Link
						     *  Width to change Mask
						     */
#define XDMAPCIE_PHYSC_DLWS_MASK	0x00040000 /**< Directed Link Width
						     *  Speed to change Mask
						     */
#define XDMAPCIE_PHYSC_DLA_MASK		0x00080000 /**< Directed Link Change
						     *  change to reliability or
						     * Autonomus Mask
						     */
#define XDMAPCIE_PHYSC_DLC_MASK		0x00300000 /**< Directed Link change
						      * Mask
						      */

#define XDMAPCIE_PHYSC_LINK_WIDTH_SHIFT		1  /**< Link Status Shift */
#define XDMAPCIE_PHYSC_LTSSM_STATE_SHIFT	3  /**< LTSSM State Shift */
#define XDMAPCIE_PHYSC_LANE_REV_SHIFT		9  /**< Lane Reversal Shift */
#define XDMAPCIE_PHYSC_LINK_UP_SHIFT		11 /**< Link Up Status Shift */
#define XDMAPCIE_PHYSC_DLW_SHIFT		16 /**< Directed Link Width
						     *  to change Shift
						     */
#define XDMAPCIE_PHYSC_DLWS_SHIFT		18  /**< Directed Link Width
						      *  Speed to change Shift
						      */
#define XDMAPCIE_PHYSC_DLA_SHIFT		19  /**< Directed Link change to
						      *  reliability or
						      *  Autonomus Shift
						      */
#define XDMAPCIE_PHYSC_DLC_SHIFT		20  /**< Directed Link
						      *  change Shift
						      */
/*@}*/

/** @name Root Port Status/Control Register bitmaps and masks
 *
 * @{
 */

#define XDMAPCIE_RPSC_MASK			0x0FFF0001 /**<
							    * Root Port
							    * Register mask
							    */
#define XDMAPCIE_RPSC_BRIDGE_ENABLE_MASK	0x00000001 /**<
							    *  Bridge Enable
							    *  Mask
							    */

#define XDMAPCIE_RPSC_ERR_FIFO_NOT_EMPTY_MASK	0x00010000 /**<
							    * Root Port Error
							    * FIFO Not Empty
							    */

#define XDMAPCIE_RPSC_ERR_FIFO_OVERFLOW_MASK	0x00020000 /**<
							    * Root Port Error
							    * FIFO Overflow
							    */

#define XDMAPCIE_RPSC_INT_FIFO_NOT_EMPTY_MASK	0x00040000 /**<
							    * Root Port
							    * Interrupt FIFO
							    * Not Empty
							    */

#define XDMAPCIE_RPSC_INT_FIFO_OVERFLOW_MASK	0x00080000 /**<
							    * Root Port
							    * Interrupt FIFO
							    * Overflow
							    */

#define XDMAPCIE_RPSC_COMP_TIMEOUT_MASK		0x0FF00000 /**<
							    * Root Port
							    * Completion
							    * Timeout
							    */

#define XDMAPCIE_RPSC_ERR_FIFO_NOT_EMPTY_SHIFT	16 /**<
						    * Root Port Error FIFO
						    * Empty Shift
						    */
#define XDMAPCIE_RPSC_ERR_FIFO_OVERFLOW_SHIFT	17 /**<
						    * Root Port Error FIFO
						    * Overflow Shift
						    */
#define XDMAPCIE_RPSC_INT_FIFO_NOT_EMPTY_SHIFT	18 /**<
						    * Root Port Interrupt FIFO
						    * Empty Shift
						    */
#define XDMAPCIE_RPSC_INT_FIFO_OVERFLOW_SHIFT	19 /**<
						    * Root Port Interrupt FIFO
						    * Overflow Shift
						    */
#define XDMAPCIE_RPSC_COMP_TIMEOUT_SHIFT	20 /**<
						    * Root Port Completion
						    * Timeout Shift
						    */


/** @name Root Port MSI Base Register bitmaps and masks
 *
 * @{
 */
#define XDMAPCIE_RPMSIB_UPPER_MASK 	0xFFFFFFFF /**<
						    * Upper 32 bits of 64 bit
						    * MSI Base Address
						    */
#define XDMAPCIE_RPMSIB_UPPER_SHIFT 	32	   /* Shift of Upper 32 bits */
#define XDMAPCIE_RPMSIB_LOWER_MASK 	0xFFFFF000 /**<
						    * Lower 32 bits of 64 bit
						    * MSI Base Address
						    */
/*@}*/

/** @name Root Port Error FIFO Read Register bitmaps and masks
 *
 * @{
 */
#define XDMAPCIE_RPEFR_REQ_ID_MASK 	0x0000FFFF /**<
						    * Requester of Error Msg
						    */

#define XDMAPCIE_RPEFR_ERR_TYPE_MASK 	0x00030000 /**<
						    * Type of Error
						    */

#define XDMAPCIE_RPEFR_ERR_VALID_MASK 	0x00040000 /**<
						    * Error Read Succeeded
						    * Status
						    */

#define XDMAPCIE_RPEFR_ERR_TYPE_SHIFT 	16 /**< Type of Error Shift*/

#define XDMAPCIE_RPEFR_ERR_VALID_SHIFT 	18 /**< Error Read Succeeded Status
					    * Shift */

/*@}*/

/** @name Root Port Interrupt FIFO Read 1 Register bitmaps and masks
 *
 * @{
 */
#define XDMAPCIE_RPIFR1_REQ_ID_MASK 		0x0000FFFF /**<
							    * Requester Id of
							    * Interrupt Message
							    */
#define XDMAPCIE_RPIFR1_MSI_ADDR_MASK 		0x07FF0000 /**< MSI Address */
#define XDMAPCIE_RPIFR1_INTR_LINE_MASK 		0x18000000 /**< Intr Line Mask
							     */
#define XDMAPCIE_RPIFR1_INTR_ASSERT_MASK 	0x20000000 /**<
							    * Whether Interrupt
							    * INTx is asserted
							    */

#define XDMAPCIE_RPIFR1_MSIINTR_VALID_MASK 	0x40000000 /**<
							    * Whether Interrupt
							    * is MSI or INTx
							    */

#define XDMAPCIE_RPIFR1_INTR_VALID_MASK 	0x80000000 /**<
							    * Interrupt Read
							    * Succeeded Status
							    */

#define XDMAPCIE_RPIFR1_MSI_ADDR_SHIFT 	16 /**< MSI Address Shift */

#define XDMAPCIE_RPIFR1_MSIINTR_VALID_SHIFT 30 /**< MSI/INTx Interrupt Shift */

#define XDMAPCIE_RPIFR1_INTR_VALID_SHIFT 31 /**< Interrupt Read Valid Shift */

/*@}*/

/** @name Root Port Interrupt FIFO Read 2 Register bitmaps and masks
 *
 * @{
 */
#define XDMAPCIE_RPIFR2_MSG_DATA_MASK 	0x0000FFFF /**<
						    * Pay Load for MSI
						    * Message
						    */
/*@}*/

/** @name ECAM Address Register bitmaps and masks
 *
 * @{
 */
#define XDMAPCIE_ECAM_MASK	0x0FFFFFFF  /**< Mask of all valid bits */
#define XDMAPCIE_ECAM_BUS_MASK	0x0FF00000  /**< Bus Number Mask */
#define XDMAPCIE_ECAM_DEV_MASK	0x000F8000  /**< Device Number Mask */
#define XDMAPCIE_ECAM_FUN_MASK	0x00007000  /**< Function Number Mask */
#define XDMAPCIE_ECAM_REG_MASK	0x00000FFC  /**< Register Number Mask */
#define XDMAPCIE_ECAM_BYT_MASK	0x00000003  /**< Byte Address Mask */

#define XDMAPCIE_ECAM_BUS_SHIFT		20  /**< Bus Number Shift Value */
#define XDMAPCIE_ECAM_DEV_SHIFT		15  /**< Device Number Shift Value */
#define XDMAPCIE_ECAM_FUN_SHIFT		12  /**< Function Number Shift Value */
#define XDMAPCIE_ECAM_REG_SHIFT		2   /**< Register Number Shift Value */
#define XDMAPCIE_ECAM_BYT_SHIFT		0   /**< Byte Offset Shift Value */
/*@}*/

/* Offset used for getting the VSEC register contents */
#define XDMAPCIE_VSEC2_OFFSET_WRT_VSEC1 	0xD8

#ifdef versal
/* Number of buses */
#define XDMAPCIE_NUM_BUSES	16
#endif

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
*		u32 XDmaPcie_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XDmaPcie_ReadReg(BaseAddress, RegOffset) \
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
*		void XDmaPcie_WriteReg(u32 BaseAddress, u32 RegOffset,
*								u32 Data)
*
******************************************************************************/
#define XDmaPcie_WriteReg(BaseAddress, RegOffset, Data) \
	Xil_Out32((BaseAddress) + (RegOffset), (Data))



/*************************** Variable Definitions ****************************/

/*************************** Function Prototypes *****************************/
#ifdef __cplusplus
}
#endif

#endif /* XDMAPCIE_HW_H */
