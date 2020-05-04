/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xdsi_hw.h
 * @addtogroup dsi_v1_2
 * @{
 *
 * Hardware definition file. It defines the register interface.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver Who Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0 ram 02/11/16 First release
 * 1.1 sss 08/17/16 Added 64 bit support
 *     sss 08/26/16 Add "Command queue Vacancy" api support
 *                  Add "Command queue FIFO Full" interrupt support
 * </pre>
 *
 *****************************************************************************/

#ifndef XDSI_HW_H_    /* prevent circular inclusions */
#define XDSI_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/* Register offset definitions. Register accesses are 32-bit.
 */
/** @name Device registers
 *  Register sets of MIPI DSI Tx
 *  @{
 */

#define XDSI_CCR_OFFSET		0x00000000  /**< Core Configuration
					      *  Register Offset */
#define XDSI_PCR_OFFSET		0x00000004  /**< Protocol Configuration
					      *  Register Offset*/
#define XDSI_GIER_OFFSET	0x00000020  /**< Global Interrupt
						Register Offset */
#define XDSI_ISR_OFFSET         0x00000024  /**< Interrupt Status Register */
#define XDSI_IER_OFFSET         0x00000028  /**< Interrupt Enable Register */
#define XDSI_STATUS_OFFSET      0x0000002C  /**< Status Register */
#define XDSI_COMMAND_OFFSET	0x00000030  /**< Packet Entry to command Queue */
#define XDSI_DATA_OFFSET	0x00000034  /**< Packet Data to data Queue */
#define XDSI_TIME1_OFFSET	0x00000050  /**< Time 1 Offset */
#define XDSI_TIME2_OFFSET	0x00000054  /**< Time 2 Offset */
#define XDSI_TIME3_OFFSET	0x00000058  /**< Time 3 Offset */
#define XDSI_TIME4_OFFSET	0x0000005C  /**< Time 4 Offset */
#define XDSI_LTIME_OFFSET	0x00000060  /**< Total Line Timing Offset */
#define XDSI_BLLP_TIME_OFFSET	0x00000064  /**< BLLP Time duration Offset */

/** @name Core configuration register masks and shifts
 *
 * This register is used for the enabling/disabling and resetting
 * the core of DSI Tx Controller
 * @{
 */

#define XDSI_CCR_RESET_CMD_FIFO_MASK	0x00000020	/**< Command FIFO Reset
						  *  bit Mask */
#define XDSI_CCR_RESET_DATA_FIFO_MASK	0x00000010	/**< Data FIFO Reset
						  *  bit Mask */
#define XDSI_CCR_CRREADY_MASK	0x00000004	/**< Controller Ready
						  *  bit Mask */
#define XDSI_CCR_SOFTRESET_MASK	0x00000002	/**< Soft Reset
						  *  core bit Mask*/
#define XDSI_CCR_COREENB_MASK 	0x00000001	/**< Enable/Disable
						  *  core Mask */
#define XDSI_CCR_CORECMDMODE_MASK	0x00000008	/**< Enable/Disable
						  *  Command/video mode Mask */

#define XDSI_CCR_CORECMDMODE_SHIFT      3	/**< Shift for selection of
						  *  command/video mode */
#define XDSI_CCR_CRREADY_SHIFT          2	/**< Shift for
						  *  Controller Ready */
#define XDSI_CCR_SOFTRESET_SHIFT	1	/**< Shift for
						  *  Soft reset */
#define XDSI_CCR_COREENB_SHIFT		0	/**< Shift for
						  *  Core Enable*/
/*@}*/

/** @name Bitmasks and shifts of Protocol control register
 *
 * This register reports the number of lanes configured during core generation
 * and number of lanes actively used.
 * @{
 */
#define XDSI_PCR_EOTPENABLE_MASK	0x00002000  /**< End of Transmission
						      *  Mask bit */
#define XDSI_PCR_PIXELFORMAT_MASK	0x00001F80  /**< Pixel Format Type
						      *  Bit Mask */
#define XDSI_PCR_BLLPMODE_MASK		0x00000040  /**< Blank packet Mode
						      *  Bit Mask */
#define XDSI_PCR_BLLPTYPE_MASK		0x00000020  /**< Blank packet type
						      *  Bit Mask */
#define XDSI_PCR_VIDEOMODE_MASK 	0x00000018  /**< Video mode Type
						      *  Bit Mask */
#define XDSI_PCR_ACTLANES_MASK		0x00000003  /**< Active lanes
						      *  in core */

#define XDSI_PCR_EOTPENABLE_SHIFT	13  /**< Shift for EOTP
						Enable */
#define XDSI_PCR_PIXELFORMAT_SHIFT	7  /**< Shift for pixel
						format */
#define XDSI_PCR_BLLPMODE_SHIFT 	6  /**< Shift for Blank
						packet Type*/
#define XDSI_PCR_BLLPTYPE_SHIFT 	5  /**< Shift for Blank
						packet Type*/
#define XDSI_PCR_VIDEOMODE_SHIFT	3  /**< Shift for
						Max Lanes */
#define XDSI_PCR_ACTLANES_SHIFT 	0  /**< Shift for
						Active Lanes */

/** @name Bitmasks and shift of XDSI_STSTUS_OFFSET
 *
 * This register used to get Command Queue Vacancy
 * @{
 */
#define XDSI_UNDER_PROCESS_MASK		0x00001000	/**< Command Underprocess*/
#define XDSI_INPOGRESS_MASK			0x00000800	/**< Command InProgress*/
#define XDSI_WAIT_FOR_DATA_MASK		0x00000400	/**< Wait for Long packet data*/
#define XDSI_FIFO_EMPTY_MASK			0x00000200	/**< FIFO EMPTY*/
#define XDSI_FIFO_FULL_MASK			0x00000100	/**< FIFO FULL*/
#define XDSI_RDY_FOR_LONG_MASK		0x00000080	/**< Readiness for Long packet*/
#define XDSI_RDY_FOR_SHORT_MASK		0x00000040	/**< Readiness for short packet*/
#define XDSI_CMDQ_MASK			0x0000003F	/**< Command Queue Vacancy*/

#define XDSI_UNDER_PROCESS_SHIFT	12	/**< Command Underprocess*/
#define XDSI_INPOGRESS_SHIFT		11  /**< Command InProgress*/
#define XDSI_WAIT_FOR_DATA_SHIFT	10	/**< Wait for Long packet data*/
#define XDSI_FIFO_EMPTY_SHIFT		9	/**< FIFO EMPTY*/
#define XDSI_FIFO_FULL_SHIFT		8	/**< FIFO FULL*/
#define XDSI_RDY_FOR_LONGPKT_SHIFT	7	/**< Readiness for Long packet*/
#define XDSI_RDY_FOR_SHORTPKT_SHIFT	6	/**< Command Queue Vacancy*/
#define XDSI_CMDQ_SHIFT		0		/**< Shift for Command Queue */

/*@}*/

/** @name Bitmasks and shift of XDSI_TIME1_OFFSET
 *
 * This register used to set timing parameters HSA and BLLP
 * @{
 */
#define XDSI_TIME1_HSA_MASK		0xFFFF0000  /**< Horizontal timing
						      *   parameter HSA mask */
#define XDSI_TIME1_BLLP_BURST_MASK	0x0000FFFF  /**< BLLP Packet size
						      *  Mask  bit*/

#define XDSI_TIME1_HSA_SHIFT    	16  /**< Shift for HSA*/
#define XDSI_TIME1_BLLP_BURST_SHIFT	 0  /**< Shift for BLLP*/

/*@}*/

/** @name Bitmasks and shift of XDSI_TIME2_OFFSET
 *
 * This register used to set timing parameters
 * @{
 */
#define XDSI_TIME2_HACT_MASK	0xFFFF0000  /**< Horizontal timing
						parameter HACT Bit Mask */
#define XDSI_TIME2_VACT_MASK	0x0000FFFF  /**< Vertical timing
						parameter VACT*/
#define XDSI_TIME2_HACT_SHIFT	16	   /**< Shift for HACT*/
#define XDSI_TIME2_VACT_SHIFT	0	   /**< Shift for VACT*/

/*@}*/
/** @name Bitmasks and shift of XDSI_TIME3_OFFSET
 *
 * This register used to set timing parameters
 * @{
 */
#define XDSI_TIME3_HBP_MASK	0xFFFF0000  /**< Horizontal timing
						parameter HBP Bit Mask*/
#define XDSI_TIME3_HFP_MASK	0x0000FFFF  /**< Horizontal timing
						 parameter HFP*/

#define XDSI_TIME3_HBP_SHIFT	16	/**< Shift for HBP*/
#define XDSI_TIME3_HFP_SHIFT	0	/**< Shift for HFP*/
/*@}*/

/** @name Bitmasks and offset of XDSI_TIME4_OFFSET
 *
 * This register used to set Vertical timing parameters
 * @{
 */
#define XDSI_TIME4_VSA_MASK	0x00FF0000   /**< Time 4 Vertical Sync
						  * Active Mask */
#define XDSI_TIME4_VBP_MASK	0x0000FF00  /**< Vertical timing
						parameter1 VBP*/
#define XDSI_TIME4_VFP_MASK	0x000000FF  /**< Vertical timing
						parameter1 VFP*/

#define XDSI_TIME4_VSA_SHIFT	16	/**< Shift for VSA*/
#define XDSI_TIME4_VBP_SHIFT	8	/**< Shift for VBP*/
#define XDSI_TIME4_VFP_SHIFT	0	/**< Shift for VFP*/
/*@}*/

/** @name Bitmasks and offsets of XDSI_GIER_OFFSET register
 *
 * This register contains the global interrupt enable bit.
 * @{
 */
#define XDSI_GIER_GIE_MASK	0x00000001  /**< Global Interrupt Enable bit */
#define XDSI_GIER_GIE_SHIFT	0	   /**< Shift bits for Global Interrupt
					     *  Enable */

#define XDSI_GIER_SET	1	/**< Enable the Global Interrupts */
#define XDSI_GIER_RESET 0	/**< Disable the Global Interrupts */

/*@}*/

/** @name Bitmasks and offsets of XDSI_ISR_OFFSET register
 *
 * This register contains the interrupt status.
 * @{
 */
#define XDSI_ISR_CMDQ_FIFO_FULL_MASK	0x00000004 /**< Command queue vacancy
							full */
#define XDSI_ISR_DATA_ID_ERR_MASK	0x00000002  /**< Unsupport datatype
						      *  Error */
#define XDSI_ISR_PXL_UNDR_RUN_MASK	0x00000001  /**< Pixel under run
						      *  error */
#define XDSI_ISR_ALLINTR_MASK   	0x00000007 /**< All interrupts mask */
#define XDSI_ISR_DATA_ID_ERR_SHIFT	1  /**< Shift for
						Unsupport Data Type */
#define XDSI_ISR_PXL_UNDR_RUN_SHIFT	0  /**< Shift for
						Pixel under run*/

/*@}*/

/** @name Bitmasks and offsets of XDSI_IER_OFFSET register
 *
 * This register contains the interrupt enable masks
 * @{
 */
#define XDSI_IER_CMDQ_FIFO_FULL_MASK	0x00000004 /**< Command queue vacancy
							full */
#define XDSI_IER_DATA_ID_ERR_MASK	0x00000002 /**< Un supported
							data type */
#define XDSI_IER_PXL_UNDR_RUN_MASK	0x00000001 /**< Pixel Under run */
#define XDSI_IER_ALLINTR_MASK   	0x00000007 /**< All interrupts mask */

#define XDSI_IER_DATA_ID_ERR_SHIFT	1   	   /**< Shift for
						     *  Unsupport data type */
#define XDSI_IER_PXL_UNDR_RUN_SHIFT	0   /**< Shift for
						Pixel under run */
/*@}*/

/** @name Bitmasks and offsets of XDSI_COMMAND_OFFSET register
 *
 * This register contains the short packet command TBD as now
 * @{
 */

#define XDSI_SPKTR_DT_MASK	0x0000003F /**< Data Type */
#define XDSI_SPKTR_VC_MASK	0x000000C0 /**< Virtual channel number */
#define XDSI_SPKTR_BYTE1_MASK	0x0000FF00 /**< BYTE1 mask */
#define XDSI_SPKTR_BYTE2_MASK	0x00FF0000 /**< BYTE2 maks */

#define XDSI_SPKTR_DT_SHIFT	0 /**< Shift for DataType */
#define XDSI_SPKTR_VC_SHIFT	6 /**< Shift for VC */
#define XDSI_SPKTR_BYTE1_SHIFT	8 /**< Shift for BYTE1 */
#define XDSI_SPKTR_BYTE2_SHIFT	16 /**< Shift for BYTE2 */

/*@}*/

/** @name Bitmasks and offsets of XDSI_LTIME_OFFSET register
 *
 * This register contains the Total Line Time
 * @{
 */
#define XDSI_LTIME_MASK 	0xFFFFFFFF  /**< Total Line time */

#define XDSI_LTIME_SHIFT	0 	/**< Shift for DataType */
/*@}*/

/** @name Bitmasks and offsets of XDSI_BBLP_SIZE_OFFSET register
 *
 * This register contains the BLLP Time duration
 * @{
 */
#define XDSI_BLLP_TIME_MASK	0xFFFFFFFF  /**< BLLP Size*/

#define XDSI_BLLP_TIME_SHIFT	0 /**< Shift for BLLP */
/*@}*/

#define XDSI_MAX_LANES		4	/**< Max Lanes supported by DSI */

/**************************** Type Definitions *******************************/


/****************************Function Definitions *******************************/

/*****************************************************************************/
/**
* Inline function to read DSI controller register.
*
* @param	BaseAddress is the base address of DSI
* @param	RegOffset is the register offset.
*
* @return	Value of the register.
*
* @note		None
*
******************************************************************************/
static inline u32 XDsi_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
{
	return (Xil_In32(BaseAddress + (u32)RegOffset));
}

/*****************************************************************************/
/**
* Inline Function to write to DSI controller register.
*
* @param	BaseAddress is the base address of DSI
* @param	RegOffset is the register offset.
* @param	Data is the value to be written to the register.
*
* @return	None.
*
* @note		None
*
******************************************************************************/
static inline void XDsi_WriteReg(UINTPTR BaseAddress, u32 RegOffset, u32 Data)
{
	Xil_Out32(BaseAddress + (u32)RegOffset, (u32)Data);
}

#ifdef __cplusplus
}
#endif

#endif
/** @} */
