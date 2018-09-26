/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*******************************************************************************/
/******************************************************************************/
/**
* @file xpciepsu_hw.h
*
* This header file contains identifiers and basic driver functions for the
* XPciePsu device driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 0.1	bs	08/21/2018	First release
* </pre>
*
*******************************************************************************/
#ifndef SRC_XPCIEPSU_HW_H /* prevent circular inclusions */
#define SRC_XPCIEPSU_HW_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/******************************** Include Files *******************************/
#include "xil_types.h"


/**************************** Constant Definitions ****************************/


/** @name Registers
*
* Register offsets for this device. Some of the registers
* are configurable at hardware build time such that may or may not exist
* in the hardware.
* @{
*/

#define PCIEPSU_PCIE_CORE_OFFSET                                               \
	0x000 /**< PCI Express hard                                            \
	       * core configuration                                            \
	       * register offset                                               \
	       */
#define PCIEPSU_VSECC_OFFSET                                                   \
	0x128 /**<                                                             \
	       * VSEC Capability                                               \
	       * Register                                                      \
	       */
#define PCIEPSU_VSECH_OFFSET                                                   \
	0x12C /**<                                                             \
	       * VSEC Header Register                                          \
	       */
#define PCIEPSU_BI_OFFSET                                                      \
	0x130 /**<                                                             \
	       * Bridge Info Register                                          \
	       */
#define PCIEPSU_BSC_OFFSET                                                     \
	0x134 /**<                                                             \
	       * Bridge Status and                                             \
	       * Control Register                                              \
	       */
#define PCIEPSU_ID_OFFSET                                                      \
	0x138 /**<                                                             \
	       * Interrupt Decode                                              \
	       * Register                                                      \
	       */
#define PCIEPSU_IM_OFFSET                                                      \
	0x13C /**<                                                             \
	       * Interrupt Mask                                                \
	       * Register                                                      \
	       */
#define PCIEPSU_BL_OFFSET                                                      \
	0x140 /**<                                                             \
	       * Bus Location Register                                         \
	       */
#define PCIEPSU_PHYSC_OFFSET                                                   \
	0x144 /**<                                                             \
	       * Physical status and                                           \
	       * Control Register                                              \
	       */
#define PCIEPSU_RPSC_OFFSET                                                    \
	0x148 /**<                                                             \
	       * Root Port Status &                                            \
	       * Control Register                                              \
	       */
#define PCIEPSU_RPMSIB_UPPER_OFFSET                                            \
	0x14C /**<                                                             \
	       * Root Port MSI Base 1                                          \
	       * Register Upper 32 bits                                        \
	       * from 64 bit address                                           \
	       * are written                                                   \
	       */
#define PCIEPSU_RPMSIB_LOWER_OFFSET                                            \
	0x150 /**<                                                             \
	       * Root Port MSI Base 2                                          \
	       * Register Lower 32 bits                                        \
	       * from 64 bit address                                           \
	       * are written                                                   \
	       */
#define PCIEPSU_RPEFR_OFFSET                                                   \
	0x154 /**<                                                             \
	       * Root Port Error FIFO                                          \
	       * Read Register                                                 \
	       */
#define PCIEPSU_RPIFR1_OFFSET                                                  \
	0x158 /**<                                                             \
	       * Root Port Interrupt                                           \
	       * FIFO Read1 Register                                           \
	       */
#define PCIEPSU_RPIFR2_OFFSET                                                  \
	0x15C /**<                                                             \
	       * Root Port Interrupt                                           \
	       * FIFO Read2 Register                                           \
	       */


#define PCIEPSU_AXIBAR2PCIBAR_0U_OFFSET                                        \
	0x208 /**<                                                             \
		       * AXIBAR 2 PCIBAR                                       \
		       * translation 0 upper                                   \
		       * 32 bits                                               \
		       */
#define PCIEPSU_AXIBAR2PCIBAR_0L_OFFSET                                        \
	0x20C /**<                                                             \
		       * AXIBAR to PCIBAR                                      \
		       * translation 0 lower                                   \
		       * 32 bits                                               \
		       */
#define PCIEPSU_AXIBAR2PCIBAR_1U_OFFSET                                        \
	0x210 /**<                                                             \
		       * AXIBAR to PCIBAR                                      \
		       * translation 1 upper                                   \
		       * 32 bits                                               \
		       */
#define PCIEPSU_AXIBAR2PCIBAR_1L_OFFSET                                        \
	0x214 /**<                                                             \
		       * AXIBAR to PCIBAR                                      \
		       * translation 1 lower                                   \
		       * 32 bits                                               \
		       */
#define PCIEPSU_AXIBAR2PCIBAR_2U_OFFSET                                        \
	0x218 /**<                                                             \
		       * AXIBAR to PCIBAR                                      \
		       * translation 2 upper                                   \
		       * 32 bits                                               \
		       */
#define PCIEPSU_AXIBAR2PCIBAR_2L_OFFSET                                        \
	0x21C /**<                                                             \
		       * AXIBAR to PCIBAR                                      \
		       * translation 2 lower                                   \
		       * 32 bits                                               \
		       */
#define PCIEPSU_AXIBAR2PCIBAR_3U_OFFSET                                        \
	0x220 /**<                                                             \
		       * AXIBAR to PCIBAR                                      \
		       * translation 3 upper                                   \
		       * 32 bits                                               \
		       */
#define PCIEPSU_AXIBAR2PCIBAR_3L_OFFSET                                        \
	0x224 /**<                                                             \
		       * AXIBAR to PCIBAR                                      \
		       * translation 3 lower                                   \
		       * 32 bits                                               \
		       */
#define PCIEPSU_AXIBAR2PCIBAR_4U_OFFSET                                        \
	0x228 /**<                                                             \
		       * AXIBAR to PCIBAR                                      \
		       * translation 4 upper                                   \
		       * 32 bits                                               \
		       */
#define PCIEPSU_AXIBAR2PCIBAR_4L_OFFSET                                        \
	0x22C /**<                                                             \
		       * AXIBAR to PCIBAR                                      \
		       * translation 4 lower                                   \
		       * 32 bits                                               \
		       */
#define PCIEPSU_AXIBAR2PCIBAR_5U_OFFSET                                        \
	0x230 /**<                                                             \
		       * AXIBAR to PCIBAR                                      \
		       * translation 5 upper                                   \
		       * 32 bits                                               \
		       */
#define PCIEPSU_AXIBAR2PCIBAR_5L_OFFSET                                        \
	0x234 /**<                                                             \
		       * AXIBAR to PCIBAR                                      \
		       * translation 5 lower                                   \
		       * 32 bits                                               \
		       */
/*@}*/

/** @name VSECC Register bitmaps and masks
* @{
*/
#define PCIEPSU_VSECC_ID_MASK 0x0000FFFF /**< Vsec capability Id */
#define PCIEPSU_VSECC_VER_MASK                                                 \
	0x000F0000 /**< Version of capability                                  \
		     *  Structure                                              \
		     */
#define PCIEPSU_VSECC_NEXT_MASK                                                \
	0xFFF00000		   /**< Offset to next                         \
					     *  capability                     \
					     */
#define PCIEPSU_VSECC_VER_SHIFT 16 /**< VSEC Version shift */
#define PCIEPSU_VSECC_NEXT_SHIFT                                               \
	20 /**< Next capability offset                                         \
	     *  shift                                                          \
	     */
/*@}*/

/** @name VSECH Register bitmaps and masks
*
* @{
*/
#define PCIEPSU_VSECH_ID_MASK 	0x0000FFFF  /**< Vsec structure Id */
#define PCIEPSU_VSECH_REV_MASK 	0x000F0000 /**< Vsec header version*/
#define PCIEPSU_VSECH_LEN_MASK                                                 \
	0xFFF00000		   /**< Length of Vsec                         \
				     *  capability structure                   \
				     */
#define PCIEPSU_VSECH_REV_SHIFT 16 /**< Vsec version shift */
#define PCIEPSU_VSECH_LEN_SHIFT 20 /**< Vsec length shift */
				   /*@}*/

/** @name Bridge Info Register bitmaps and masks
*
* @{
*/
#define PCIEPSU_BI_GEN2_MASK                                                   \
	0x00000001 /**< PCIe Gen2 Speed                                        \
		     *  Support Mask                                           \
		     */
#define PCIEPSU_BI_RP_MASK                                                     \
	0x00000002			     /**< PCIe Root Port Support       \
					       */
#define PCIEPSU_UP_CONFIG_CAPABLE 0x00000004 /**< Up Config Capable */

#define PCIEPSU_BI_ECAM_SIZE_MASK 0x00070000 /**< ECAM size */
#define PCIEPSU_BI_RP_SHIFT 1		     /**< PCIe Root Port Shift */
#define PCIEPSU_BI_ECAM_SIZE_SHIFT 16	/**< PCIe ECAM Size Shift */
/*@}*/

/** @name Bridge Status & Control Register bitmaps and masks
*
* @{
*/
#define PCIEPSU_BSC_ECAM_BUSY_MASK 0x00000001 /**< ECAM Busy Status */
#define PCIEPSU_BSC_GI_MASK                                                    \
	0x00000100 /**< Global Interrupt                                       \
		     *  Disable                                                \
		     */
#define PCIEPSU_BSC_RW1C_MASK                                                  \
	0x00010000 /**< RW Permissions to RW1C                                 \
		     *  Registers                                              \
		     */
#define PCIEPSU_BSC_RO_MASK                                                    \
	0x00020000 /**< RW Permissions to RO                                   \
		     *  Registers                                              \
		     */
#define PCIEPSU_BSC_GI_SHIFT                                                   \
	8			  /**< Global Interrupt Disable                \
				    *  Shift                                   \
				    */
#define PCIEPSU_BSC_RW1C_SHIFT 16 /**< RW1C Shift */
#define PCIEPSU_BSC_RO_SHIFT 17   /**< RO as RW Shift */
				  /*@}*/

/** @name Interrupt Decode Register bitmaps and masks
*
* @{
*/
#define PCIEPSU_ID_LINK_DOWN_MASK 0x00000001 /**< Link Down Mask */
#define PCIEPSU_ID_ECRC_ERR_MASK 0x00000002  /**< Rx Packet CRC failed */
#define PCIEPSU_ID_STR_ERR_MASK 0x00000004   /**< Streaming Error Mask */
#define PCIEPSU_ID_HOT_RST_MASK 0x00000008   /**< Hot Reset Mask */
#define PCIEPSU_ID_CFG_COMPL_STATE_MASK                                        \
	0x000000E0			       /**< Cfg Completion             \
						  *  Status Mask               \
						  */
#define PCIEPSU_ID_CFG_TIMEOUT_MASK 0x00000100 /**< Cfg timeout Mask */
#define PCIEPSU_ID_CORRECTABLE_ERR_MASK                                        \
	0x00000200				/**< Correctable Error         \
						   *  Mask                     \
						   */
#define PCIEPSU_ID_NONFATAL_ERR_MASK 0x00000400 /**< Non-Fatal Error Mask */
#define PCIEPSU_ID_FATAL_ERR_MASK 0x00000800    /**< Fatal Error Mask */
#define PCIEPSU_ID_INTX_INTERRUPT 0x00010000    /**< INTX Interrupt */
#define PCIEPSU_ID_MSI_INTERRUPT 0x00020000     /**< MSI Interrupt */
#define PCIEPSU_ID_UNSUPP_CMPL_MASK                                            \
	0x00100000 /**< Slave Unsupported                                      \
		     *  Request Mask                                           \
		     */
#define PCIEPSU_ID_UNEXP_CMPL_MASK                                             \
	0x00200000 /**< Slave Unexpected                                       \
		     *  Completion Mask                                        \
		     */
#define PCIEPSU_ID_CMPL_TIMEOUT_MASK                                           \
	0x00400000 /**< Slave completion                                       \
		     *  Time Mask                                              \
		     */
#define PCIEPSU_ID_SLV_EP_MASK                                                 \
	0x00800000 /**< Slave Error                                            \
			     *  Poison Mask                                    \
			     */
#define PCIEPSU_ID_CMPL_ABT_MASK                                               \
	0x01000000 /**< Slave completion                                       \
		     *  Abort Mask                                             \
		     */
#define PCIEPSU_ID_ILL_BURST_MASK                                              \
	0x02000000 /**< Slave Illegal                                          \
		     *  Burst Mask                                             \
		     */
#define PCIEPSU_ID_DECODE_ERR_MASK                                             \
	0x04000000 /**< Master Decode                                          \
		     *  Error Interrupt Mask                                   \
		     */
#define PCIEPSU_ID_SLAVE_ERR_MASK                                              \
	0x08000000 /**< Master Slave Error                                     \
		     *  Interrupt Mask                                         \
		     */
#define PCIEPSU_ID_MASTER_EP_MASK                                              \
	0x10000000 /**< Master Error Poison                                    \
		     *  Mask                                                   \
		     */
#define PCIEPSU_ID_CLEAR_ALL_MASK                                              \
	0xFFFFFFFF /**< Mask of all                                            \
		     *  Interrupts                                             \
		     */
/*@}*/


/** @name Interrupt Mask Register bitmaps and masks
*
* @{
*/
#define PCIEPSU_IM_ENABLE_ALL_MASK 0xFFFFFFFF /**< Enable All Interrupts */
#define PCIEPSU_IM_DISABLE_ALL_MASK                                            \
	0x00000000 /**< Disable All                                            \
		     *  Interrupts                                             \
		     */
/*@}*/

/** @name Bus Location Register bitmaps and masks
*
* @{
*/
#define PCIEPSU_BL_FUNC_MASK 0x00000007 /**< Requester ID Function Number */
#define PCIEPSU_BL_DEV_MASK 0x000000F8  /**< Requester ID Device Number   */
#define PCIEPSU_BL_BUS_MASK 0x0000FF00  /**< Requester ID Bus Number */
#define PCIEPSU_BL_PORT_MASK 0x00FF0000 /**< Requester ID Port Number */

#define PCIEPSU_BL_DEV_SHIFT                                                   \
	3 /**< Requester ID Device Number                                      \
	    *  Shift Value                                                     \
	    */
#define PCIEPSU_BL_BUS_SHIFT                                                   \
	8 /**< Requester ID Bus Number Shift                                   \
	    *  Value                                                           \
	    */
#define PCIEPSU_BL_PORT_SHIFT                                                  \
	16 /**< Requester ID Bus Number Shift                                  \
	     *  Value                                                          \
	     */
/*@}*/

/** @name PHY Status & Control Register bitmaps and masks
*
* @{
*/
#define PCIEPSU_PHYSC_LINK_RATE_MASK 0x00000001   /**< Link Rate */
#define PCIEPSU_PHYSC_LINK_WIDTH_MASK 0x00000006  /**< Link Width Mask */
#define PCIEPSU_PHYSC_LTSSM_STATE_MASK 0x000001F8 /**< LTSSM State Mask */
#define PCIEPSU_PHYSC_LANE_REV_MASK 0x00000600    /**< Lane Reversal Mask */
#define PCIEPSU_PHYSC_LINK_UP_MASK 0x00000800     /**< Link Up Status Mask */
#define PCIEPSU_PHYSC_DLW_MASK                                                 \
	0x00030000 /**< Directed Link                                          \
		     *  Width to change Mask                                   \
		     */
#define PCIEPSU_PHYSC_DLWS_MASK                                                \
	0x00040000 /**< Directed Link Width                                    \
			     *  Speed to change Mask                           \
			     */
#define PCIEPSU_PHYSC_DLA_MASK                                                 \
	0x00080000 /**< Directed Link Change                                   \
		     *  change to reliability or                               \
		     * Autonomus Mask                                          \
		     */
#define PCIEPSU_PHYSC_DLC_MASK                                                 \
	0x00300000 /**< Directed Link change                                   \
		      * Mask                                                   \
		      */

#define PCIEPSU_PHYSC_LINK_WIDTH_SHIFT 1  /**< Link Status Shift */
#define PCIEPSU_PHYSC_LTSSM_STATE_SHIFT 3 /**< LTSSM State Shift */
#define PCIEPSU_PHYSC_LANE_REV_SHIFT 9    /**< Lane Reversal Shift */
#define PCIEPSU_PHYSC_LINK_UP_SHIFT 11    /**< Link Up Status Shift */
#define PCIEPSU_PHYSC_DLW_SHIFT                                                \
	16 /**< Directed Link Width                                            \
		     *  to change Shift                                        \
		     */
#define PCIEPSU_PHYSC_DLWS_SHIFT                                               \
	18 /**< Directed Link Width                                            \
	     *  Speed to change Shift                                          \
	     */
#define PCIEPSU_PHYSC_DLA_SHIFT                                                \
	19 /**< Directed Link change to                                        \
		     *  reliability or                                         \
		     *  Autonomus Shift                                        \
		     */
#define PCIEPSU_PHYSC_DLC_SHIFT                                                \
	20 /**< Directed Link                                                  \
		     *  change Shift                                           \
		     */
/*@}*/

/** @name Root Port Status/Control Register bitmaps and masks
*
* @{
*/

#define PCIEPSU_RPSC_MASK                                                      \
	0x0FFF0001 /**<                                                        \
		    * Root Port                                                \
		    * Register mask                                            \
		    */
#define PCIEPSU_RPSC_BRIDGE_ENABLE_MASK                                        \
	0x00000001 /**<                                                        \
			    *  Bridge Enable                                   \
			    *  Mask                                            \
			    */

#define PCIEPSU_RPSC_ERR_FIFO_NOT_EMPTY_MASK                                   \
	0x00010000 /**<                                                        \
		    * Root Port Error                                          \
		    * FIFO Not Empty                                           \
		    */

#define PCIEPSU_RPSC_ERR_FIFO_OVERFLOW_MASK                                    \
	0x00020000 /**<                                                        \
		    * Root Port Error                                          \
		    * FIFO Overflow                                            \
		    */

#define PCIEPSU_RPSC_INT_FIFO_NOT_EMPTY_MASK                                   \
	0x00040000 /**<                                                        \
		    * Root Port                                                \
		    * Interrupt FIFO                                           \
		    * Not Empty                                                \
		    */

#define PCIEPSU_RPSC_INT_FIFO_OVERFLOW_MASK                                    \
	0x00080000 /**<                                                        \
		    * Root Port                                                \
		    * Interrupt FIFO                                           \
		    * Overflow                                                 \
		    */

#define PCIEPSU_RPSC_COMP_TIMEOUT_MASK                                         \
	0x0FF00000 /**<                                                        \
		    * Root Port                                                \
		    * Completion                                               \
		    * Timeout                                                  \
		    */

#define PCIEPSU_RPSC_ERR_FIFO_NOT_EMPTY_SHIFT                                  \
	16 /**<                                                                \
	    * Root Port Error FIFO                                             \
	    * Empty Shift                                                      \
	    */
#define PCIEPSU_RPSC_ERR_FIFO_OVERFLOW_SHIFT                                   \
	17 /**<                                                                \
	    * Root Port Error FIFO                                             \
	    * Overflow Shift                                                   \
	    */
#define PCIEPSU_RPSC_INT_FIFO_NOT_EMPTY_SHIFT                                  \
	18 /**<                                                                \
	    * Root Port Interrupt FIFO                                         \
	    * Empty Shift                                                      \
	    */
#define PCIEPSU_RPSC_INT_FIFO_OVERFLOW_SHIFT                                   \
	19 /**<                                                                \
	    * Root Port Interrupt FIFO                                         \
	    * Overflow Shift                                                   \
	    */
#define PCIEPSU_RPSC_COMP_TIMEOUT_SHIFT                                        \
	20 /**<                                                                \
		    * Root Port Completion                                     \
		    * Timeout Shift                                            \
		    */


/** @name Root Port MSI Base Register bitmaps and masks
*
* @{
*/
#define PCIEPSU_RPMSIB_UPPER_MASK                                              \
	0xFFFFFFFF		      /**<                                     \
				       * Upper 32 bits of 64 bit               \
				       * MSI Base Address                      \
				       */
#define PCIEPSU_RPMSIB_UPPER_SHIFT 32 /* Shift of Upper 32 bits */
#define PCIEPSU_RPMSIB_LOWER_MASK                                              \
	0xFFFFF000 /**<                                                        \
		    * Lower 32 bits of 64 bit                                  \
		    * MSI Base Address                                         \
		    */
/*@}*/

/** @name Root Port Error FIFO Read Register bitmaps and masks
*
* @{
*/
#define PCIEPSU_RPEFR_REQ_ID_MASK                                              \
	0x0000FFFF /**<                                                        \
		    * Requester of Error Msg                                   \
		    */

#define PCIEPSU_RPEFR_ERR_TYPE_MASK                                            \
	0x00030000 /**<                                                        \
		    * Type of Error                                            \
		    */

#define PCIEPSU_RPEFR_ERR_VALID_MASK                                           \
	0x00040000 /**<                                                        \
		    * Error Read Succeeded                                     \
		    * Status                                                   \
		    */

#define PCIEPSU_RPEFR_ERR_TYPE_SHIFT 16 /**< Type of Error Shift*/

#define PCIEPSU_RPEFR_ERR_VALID_SHIFT                                          \
	18 /**< Error Read Succeeded Status                                    \
	    * Shift */

/*@}*/

/** @name Root Port Interrupt FIFO Read 1 Register bitmaps and masks
*
* @{
*/
#define PCIEPSU_RPIFR1_REQ_ID_MASK                                             \
	0x0000FFFF				/**<                           \
						 * Requester Id of             \
						 * Interrupt Message           \
						 */
#define PCIEPSU_RPIFR1_MSI_ADDR_MASK 0x07FF0000 /**< MSI Address */
#define PCIEPSU_RPIFR1_INTR_LINE_MASK                                          \
	0x18000000 /**< Intr Line Mask                                         \
		     */
#define PCIEPSU_RPIFR1_INTR_ASSERT_MASK                                        \
	0x20000000 /**<                                                        \
		    * Whether Interrupt                                        \
		    * INTx is asserted                                         \
		    */

#define PCIEPSU_RPIFR1_MSIINTR_VALID_MASK                                      \
	0x40000000 /**<                                                        \
		    * Whether Interrupt                                        \
		    * is MSI or INTx                                           \
		    */

#define PCIEPSU_RPIFR1_INTR_VALID_MASK                                         \
	0x80000000 /**<                                                        \
			    * Interrupt Read                                   \
			    * Succeeded Status                                 \
			    */

#define PCIEPSU_RPIFR1_MSI_ADDR_SHIFT 16 /**< MSI Address Shift */

#define PCIEPSU_RPIFR1_MSIINTR_VALID_SHIFT 30 /**< MSI/INTx Interrupt Shift */

#define PCIEPSU_RPIFR1_INTR_VALID_SHIFT 31 /**< Interrupt Read Valid Shift */

/*@}*/

/** @name Root Port Interrupt FIFO Read 2 Register bitmaps and masks
*
* @{
*/
#define PCIEPSU_RPIFR2_MSG_DATA_MASK                                           \
	0x0000FFFF /**<                                                        \
		    * Pay Load for MSI                                         \
		    * Message                                                  \
		    */
/*@}*/

/* helper mecros */
#define BIT(x) 								(1 << (x))

/* each bus required 1 MB ecam space */
#define GET_MAX_BUS_NO(ecam_sz) 			(((ecam_sz) / (1024 * 1024)) - 1)

#define BITSPERLONG 						64
#define GENMASK(h, l) (((~0ULL) << (l)) & (~0ULL) >> (BITSPERLONG - 1 - (h)))

/* Bridge core config registers */
#define BRCFG_PCIE_RX0 						0x00000000
#define BRCFG_INTERRUPT 					0x00000010
#define BRCFG_PCIE_RX_MSG_FILTER 			0x00000020

/* Egress - Bridge translation registers */
#define E_BREG_CAPABILITIES 				0x00000200
#define E_BREG_CONTROL 						0x00000208
#define E_BREG_BASE_LO 						0x00000210
#define E_BREG_BASE_HI 						0x00000214
#define E_ECAM_CAPABILITIES 				0x00000220
#define E_ECAM_CONTROL 						0x00000228
#define E_ECAM_BASE_LO 						0x00000230
#define E_ECAM_BASE_HI 						0x00000234
#define E_DREG_CTRL 						0x00000288
#define E_DREG_BASE_LO 						0x00000290

#define DREG_DMA_EN 						BIT(0)
#define DREG_DMA_BASE_LO 					0xFD0F0000

/* Ingress - address translations */
#define I_MSII_CAPABILITIES 				0x00000300
#define I_MSII_CONTROL 						0x00000308
#define I_MSII_BASE_LO 						0x00000310
#define I_MSII_BASE_HI 						0x00000314

/* MSI interrupt status mask bits */
#define MSGF_MSI_SR_LO_MASK 			GENMASK(31, 0)
#define MSGF_MSI_SR_HI_MASK 			GENMASK(31, 0)
#define MSII_PRESENT 					BIT(0)
#define MSII_ENABLE 					BIT(0)
#define MSII_STATUS_ENABLE 				BIT(15)

#define I_ISUB_CONTROL 					0x000003E8
#define SET_ISUB_CONTROL 				BIT(0)
/* Rxed msg fifo  - Interrupt status registers */
#define MSGF_MISC_STATUS 				0x00000400
#define MSGF_MISC_MASK 					0x00000404
#define MSGF_LEG_STATUS 				0x00000420
#define MSGF_LEG_MASK 					0x00000424
#define MSGF_MSI_STATUS_LO 				0x00000440
#define MSGF_MSI_STATUS_HI 				0x00000444
#define MSGF_MSI_MASK_LO 				0x00000448
#define MSGF_MSI_MASK_HI 				0x0000044C
/* Root DMA Interrupt register */
#define MSGF_DMA_MASK 					0x00000464

#define MSGF_INTR_EN 					BIT(0)

/* Msg filter mask bits */
#define CFG_ENABLE_PM_MSG_FWD 			BIT(1)
#define CFG_ENABLE_INT_MSG_FWD 			BIT(2)
#define CFG_ENABLE_ERR_MSG_FWD 			BIT(3)
#define CFG_ENABLE_MSG_FILTER_MASK                                             \
	(CFG_ENABLE_PM_MSG_FWD | CFG_ENABLE_INT_MSG_FWD                        \
	 | CFG_ENABLE_ERR_MSG_FWD)

/* Misc interrupt status mask bits */
#define MSGF_MISC_SR_RXMSG_AVAIL 		BIT(0)
#define MSGF_MISC_SR_RXMSG_OVER 		BIT(1)
#define MSGF_MISC_SR_SLAVE_ERR 			BIT(4)
#define MSGF_MISC_SR_MASTER_ERR 		BIT(5)
#define MSGF_MISC_SR_I_ADDR_ERR 		BIT(6)
#define MSGF_MISC_SR_E_ADDR_ERR 		BIT(7)
#define MSGF_MISC_SR_FATAL_AER 			BIT(16)
#define MSGF_MISC_SR_NON_FATAL_AER 		BIT(17)
#define MSGF_MISC_SR_CORR_AER 			BIT(18)
#define MSGF_MISC_SR_UR_DETECT 			BIT(20)
#define MSGF_MISC_SR_NON_FATAL_DEV 		BIT(22)
#define MSGF_MISC_SR_FATAL_DEV 			BIT(23)
#define MSGF_MISC_SR_LINK_DOWN 			BIT(24)
#define MSGF_MSIC_SR_LINK_AUTO_BWIDTH 	BIT(25)
#define MSGF_MSIC_SR_LINK_BWIDTH 		BIT(26)

#define MSGF_MISC_SR_MASKALL                                                   \
	(MSGF_MISC_SR_RXMSG_AVAIL | MSGF_MISC_SR_RXMSG_OVER                    \
	 | MSGF_MISC_SR_SLAVE_ERR | MSGF_MISC_SR_MASTER_ERR                    \
	 | MSGF_MISC_SR_I_ADDR_ERR | MSGF_MISC_SR_E_ADDR_ERR                   \
	 | MSGF_MISC_SR_FATAL_AER | MSGF_MISC_SR_NON_FATAL_AER                 \
	 | MSGF_MISC_SR_CORR_AER | MSGF_MISC_SR_UR_DETECT                      \
	 | MSGF_MISC_SR_NON_FATAL_DEV | MSGF_MISC_SR_FATAL_DEV                 \
	 | MSGF_MISC_SR_LINK_DOWN | MSGF_MSIC_SR_LINK_AUTO_BWIDTH              \
	 | MSGF_MSIC_SR_LINK_BWIDTH)


/* Legacy interrupt status mask bits */
#define MSGF_LEG_SR_INTA 			BIT(0)
#define MSGF_LEG_SR_INTB 			BIT(1)
#define MSGF_LEG_SR_INTC 			BIT(2)
#define MSGF_LEG_SR_INTD 			BIT(3)
#define MSGF_LEG_SR_MASKALL                                                    \
	(MSGF_LEG_SR_INTA | MSGF_LEG_SR_INTB | MSGF_LEG_SR_INTC                \
	 | MSGF_LEG_SR_INTD)

/* Bridge config interrupt mask */
#define BRCFG_INTERRUPT_MASK 		BIT(0)
#define BREG_PRESENT 				BIT(0)
#define BREG_ENABLE 				BIT(0)
#define BREG_ENABLE_FORCE 			BIT(1)

/* E_ECAM status mask bits */
#define E_ECAM_PRESENT 				BIT(0)
#define E_ECAM_CR_ENABLE 			BIT(0)
#define E_ECAM_SIZE_LOC 			GENMASK(20, 16)
#define E_ECAM_SIZE_MIN 			GENMASK(23, 16)
#define E_ECAM_SIZE_SHIFT 			16
#define ECAM_BUS_LOC_SHIFT 			20
#define ECAM_DEV_LOC_SHIFT 			12
#define PSU_ECAM_VALUE_DEFAULT 		12

#define CFG_DMA_REG_BAR 			GENMASK(2, 0)

/* Readin the PS_LINKUP */
#define PS_LINKUP_OFFSET 			0x00000238
#define PCIE_PHY_LINKUP_BIT 		BIT(0)
#define PHY_RDY_LINKUP_BIT 			BIT(1)

/* Parameters for the waiting for link up routine */
#define LINK_WAIT_MAX_RETRIES 		10
#define LINK_WAIT_USLEEP_MIN 		90000
#define LINK_WAIT_USLEEP_MAX 		100000

/* 4Kb Alignment */
#define ALIGN_4KB 				0xFFFFFFFFFFFFF000

#define XPciePsu_IsEcamBusy(InstancePtr)                                       \
	((XPciepsu_ReadReg((InstancePtr)->Config.Ecam, PCIEPSU_BSC_OFFSET)     \
	  & PCIEPSU_BSC_ECAM_BUSY_MASK)                                        \
		 ? TRUE                                                        \
		 : FALSE)


/** @name ECAM Address Register bitmaps and masks
*
* @{
*/
#define PCIEPSU_ECAM_MASK 0x0FFFFFFF     /**< Mask of all valid bits */
#define PCIEPSU_ECAM_BUS_MASK 0x0FF00000 /**< Bus Number Mask */
#define PCIEPSU_ECAM_DEV_MASK 0x000F8000 /**< Device Number Mask */
#define PCIEPSU_ECAM_FUN_MASK 0x00007000 /**< Function Number Mask */
#define PCIEPSU_ECAM_REG_MASK 0x00000FFC /**< Register Number Mask */
#define PCIEPSU_ECAM_BYT_MASK 0x00000003 /**< Byte Address Mask */

#define PCIEPSU_ECAM_BUS_SHIFT 20 /**< Bus Number Shift Value */
#define PCIEPSU_ECAM_DEV_SHIFT 15 /**< Device Number Shift Value */
#define PCIEPSU_ECAM_FUN_SHIFT 12 /**< Function Number Shift Value */
#define PCIEPSU_ECAM_REG_SHIFT 2  /**< Register Number Shift Value */
#define PCIEPSU_ECAM_BYT_SHIFT 0  /**< Byte Offset Shift Value */
/*@}*/

/* Offset used for getting the VSEC register contents */
#define PCIEPSU_VSEC2_OFFSET_WRT_VSEC1 0xD8

/******************** Macros (Inline Functions) Definitions *******************/
#define XPciepsu_ReadReg(BaseAddr, RegOffset) Xil_In32((BaseAddr) + (RegOffset))

#define XPciepsu_WriteReg(BaseAddr, RegOffset, Val)                            \
	Xil_Out32((BaseAddr) + (RegOffset), (Val))

/**************************** Variable Definitions ****************************/

/***************************** Function Prototypes ****************************/
#ifdef __cplusplus
}
#endif

#endif /* SRC_XPCIEPSU_HW_H */

/** @} */
