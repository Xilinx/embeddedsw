/******************************************************************************
******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
/*****************************************************************************/
/**
 * @file xdsi2rx_hw.h
 * @addtogroup dsi2rx Overview
 * @{
 *
 * Hardware definition file. It defines the register interface.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who 	 Date     Changes
 * ----- ----  --------  ---------------------------------------
 * 1.0   Kunal  18/04/24  First release
 * </pre>
 *
 *****************************************************************************/

#ifndef XDSI2RX_HW_H_    /* prevent circular inclusions */
#define XDSI2RX_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/* Register offset definitions. Register accesses are 32-bit.
 */
/** @name Device registers
 *  Register sets of MIPI DSI Rx
 *  @{
 */

#define XDSI2RX_CCR_OFFSET	0x00000000  /**< Core Configuration
					      *  Register Offset */
#define XDSI2RX_PCR_OFFSET	0x00000004  /**< Protocol Configuration
					      *  Register Offset*/
#define XDSI2RX_VER_OFFSET	0x00000008  /**< Version Register */
#define XDSI2RX_GIER_OFFSET	0x00000020  /**< Global Interrupt
						Register Offset */
#define XDSI2RX_ISR_OFFSET      0x00000024  /**< Interrupt Status Register */
#define XDSI2RX_IER_OFFSET      0x00000028  /**< Interrupt Enable Register */
#define XDSI2RX_GSPR_OFFSET     0x00000030  /**< Generic Short Packet Register */
#define XDSI2RX_DCR_OFFSET	0x00000034  /**< Debug Counter register */

/** @name Core configuration register masks and shifts
 *
/* This register is used for the enabling/disabling and resetting
 * the core of DSI Rx Controller
 * @{
 */

#define XDSI2RX_CCR_INIT_DONE_MASK	0x00000008	/**< Init Done
						  *      bit mask */
#define XDSI2RX_CCR_CRREADY_MASK	0x00000004	/**< Controller Ready
						  *  bit Mask */
#define XDSI2RX_CCR_SOFTRESET_MASK	0x00000002	/**< Soft Reset
						  *  core bit Mask*/
#define XDSI2RX_CCR_COREENB_MASK 	0x00000001	/**< Enable/Disable
						  *  core Mask */

#define XDSI2RX_CCR_INIT_DONE_SHIFT     3	/**< Shift for Init Done */
#define XDSI2RX_CCR_CRREADY_SHIFT       2	/**< Shift for
						  *  Controller Ready */
#define XDSI2RX_CCR_SOFTRESET_SHIFT	1	/**< Shift for
						  *  Soft reset */
#define XDSI2RX_CCR_COREENB_SHIFT	0	/**< Shift for
						  *  Core Enable*/
/*@}*/

/** @name Bitmasks and shifts of Protocol control register
 * for DSI RX IP.
 * This register reports the number of lanes configured during core generation
 * and number of lanes actively used.
 * @{
 */
#define XDSI2RX_PCR_MAXLANES_MASK	0x000C0000  /**< MAX LANES
						      *  Mask */
#define XDSI2RX_PCR_ERRDATA_TYPE_MASK	0x0003F000  /**< ERROR DATA TYPE
						      *  Mask */
#define XDSI2RX_PCR_PIXELFORMAT_MASK	0x000007E0  /**< Pixel Format
						      *  Mask */
#define XDSI2RX_PCR_PIXELMODE_MASK 	0x0000001C  /**< Video mode Type
						      *  Bit Mask */

#define XDSI2RX_PCR_MAXLANES_SHIFT	18  /**< Shift MAX LANES */
#define XDSI2RX_PCR_ERRDATA_TYPE_SHIFT 	12  /**< Shift for ERROR DATA
						Type*/
#define XDSI2RX_PCR_PIXELFORMAT_SHIFT 	5  /**< Shift for PIXEL
						FORMAT Type*/
#define XDSI2RX_PCR_PIXELMODE_SHIFT	2  /**< Shift for
						Pixel mode 1, 2, 4 PPC */

/*@}*/

/** @name Bitmasks and offsets of XDSI2RX_GIER_OFFSET register
 *
 * This register contains the global interrupt enable bit.
 * @{
 */
#define XDSI2RX_GIER_GIE_MASK	0x00000001  /**< Global Interrupt Enable bit */
#define XDSI2RX_GIER_GIE_SHIFT	0	   /**< Shift bits for Global Interrupt
					     *  Enable */

#define XDSI2RX_GIER_ENABLE   1	/**< Enable the Global Interrupts */
#define XDSI2RX_GIER_DISABLE 0	/**< Disable the Global Interrupts */
/*@}*/

/** @name Bitmasks and offsets of XDSI2RX_ISR_OFFSET register
 *
 * This register contains the interrupt status.
 * @{
 */

#define XDSI2RX_ISR_FRAME_START_MASK		 0x00200000 /**< FRAME START detected */
#define XDSI2RX_ISR_GSP_FIFO_FULL_ERR_MASK	 0x00040000 /**< Generic short packet
							FIFO FULL error */
#define XDSI2RX_ISR_GSP_FIFO_NE_ERR_MASK	 0x00020000 /**< Generic short packet
							not empty error */
#define XDSI2RX_ISR_STREAM_ASYNC_FIFO_FULL_ERR_MASK  0x00010000 /**< STREAM FIFO
							full error */
#define XDSI2RX_ISR_LM_ASYNC_FIFO_FULL_ERR_MASK	 0x00004000 /**< LM
						async FIFO full error */
#define XDSI2RX_ISR_STOP_STATE_MASK		 0x00001000 /**< STOP
							STATE Error */
#define XDSI2RX_ISR_SOT_ERR_LANE4_MASK   	 0x00000800 /**< SOT
							error lane4 */
#define XDSI2RX_ISR_SOT_SYNC_ERR_LANE4_MASK   	 0x00000400 /**< SOT sync
							error lane4 */
#define XDSI2RX_ISR_SOT_ERR_LANE3_MASK   	 0x00000200 /**< SOT error
							lane3 */
#define XDSI2RX_ISR_SOT_SYNC_ERR_LANE3_MASK   	 0x00000100 /**< SOT
							sync error lane3 */
#define XDSI2RX_ISR_SOT_ERR_LANE2_MASK   	 0x00000080 /**< SOT error
							lane2 */
#define XDSI2RX_ISR_SOT_SYNC_ERR_LANE2_MASK   	 0x00000040 /**< SOT sync
							error lane2 */
#define XDSI2RX_ISR_SOT_ERR_LANE1_MASK   	 0x00000020 /**< SOT error
							lane1 */
#define XDSI2RX_ISR_SOT_SYNC_ERR_LANE1_MASK   	 0x00000010 /**< SOT sync
							error lane1 */
#define XDSI2RX_ISR_ECC2_BIT_MASK   		 0x00000008 /**< SHIFT for
							ECC2 bit error */
#define XDSI2RX_ISR_ECC1_BIT_MASK   		 0x00000004 /**< SHIFT for
							ECC1 bit error */
#define XDSI2RX_ISR_CRC_ERR_MASK		 0x00000002  /**< Shift for
							CRC Error */
#define XDSI2RX_ISR_UN_DATA_TYPE_MASK		 0x00000001 /**< Shift for
							Unsupported Data Type*/

#define XDSI2RX_ISR_ALLINTR_MASK		 0x00275FFF /* ALL intr Mask*/

#define XDSI2RX_ISR_FRAME_START_SHIFT		 21 /**< FRAME START detected */
#define XDSI2RX_ISR_GSP_FIFO_FULL_ERR_SHIFT	 18 /**< Generic short packet
							FIFO FULL error */
#define XDSI2RX_ISR_GSP_FIFO_NE_ERR_SHIFT	 17 /**< Generic short packet
							not empty error */
#define XDSI2RX_ISR_STREAM_ASYNC_FIFO_FULL_ERR_SHIFT 16 /**< STREAM FIFO full error */
#define XDSI2RX_ISR_LM_ASYNC_FIFO_FULL_ERR_SHIFT 14 /**< LM FIFO full error */
#define XDSI2RX_ISR_STOP_STATE_SHIFT		 12 /**< STOP STATE Error */
#define XDSI2RX_ISR_SOT_ERR_LANE4_SHIFT   	 11 /**< SOT error lane4 */
#define XDSI2RX_ISR_SOT_SYNC_ERR_LANE4_SHIFT   	 10 /**< SOT sync error lane4 */
#define XDSI2RX_ISR_SOT_ERR_LANE3_SHIFT   	 9 /**< SOT error lane3 */
#define XDSI2RX_ISR_SOT_SYNC_ERR_LANE3_SHIFT   	 8 /**< SOT sync error lane3 */
#define XDSI2RX_ISR_SOT_ERR_LANE2_SHIFT   	 7 /**< SOT error lane2 */
#define XDSI2RX_ISR_SOT_SYNC_ERR_LANE2_SHIFT   	 6 /**< SOT sync error lane2 */
#define XDSI2RX_ISR_SOT_ERR_LANE1_SHIFT   	 5 /**< SOT error lane1 */
#define XDSI2RX_ISR_SOT_SYNC_ERR_LANE1_SHIFT   	 4 /**< SOT sync error lane1 */
#define XDSI2RX_ISR_ECC2_BIT_SHIFT   		 3 /**< SHIFT for ECC2 bit error */
#define XDSI2RX_ISR_ECC1_BIT_SHIFT   		 2 /**< SHIFT for ECC1 bit error */
#define XDSI2RX_ISR_CRC_ERR_SHIFT		 1  /**< Shift for
							CRC Error */
#define XDSI2RX_ISR_UN_DATA_TYPE_SHIFT		 0  /**< Shift for
							Unsupported Data Type*/
/*@}*/

/** @name Bitmasks and offsets of XDSI2RX_IER_OFFSET register
 *
 * This register contains the interrupt enable masks
 * @{
 */

#define XDSI2RX_IER_FRAME_START_MASK		 0x00200000 /**< FRAME START detected */
#define XDSI2RX_IER_GSP_FIFO_FULL_ERR_MASK	 0x00040000 /**< Generic short packet
							FIFO FULL error */
#define XDSI2RX_IER_GSP_FIFO_NE_ERR_MASK	 0x00020000 /**< Generic short packet
							not empty error */
#define XDSI2RX_IER_STREAM_ASYNC_FIFO_FULL_ERR_MASK  0x00010000 /**< STREAM FIFO
							full error */
#define XDSI2RX_IER_LM_ASYNC_FIFO_FULL_ERR_MASK	 0x00004000 /**< LM
						async FIFO full error */
#define XDSI2RX_IER_STOP_STATE_MASK		 0x00001000 /**< STOP
							STATE Error */
#define XDSI2RX_IER_SOT_ERR_LANE4_MASK   	 0x00000800 /**< SOT
							error lane4 */
#define XDSI2RX_IER_SOT_SYNC_ERR_LANE4_MASK   	 0x00000400 /**< SOT sync
							error lane4 */
#define XDSI2RX_IER_SOT_ERR_LANE3_MASK   	 0x00000200 /**< SOT error
							lane3 */
#define XDSI2RX_IER_SOT_SYNC_ERR_LANE3_MASK   	 0x00000100 /**< SOT
							sync error lane3 */
#define XDSI2RX_IER_SOT_ERR_LANE2_MASK   	 0x00000080 /**< SOT error
							lane2 */
#define XDSI2RX_IER_SOT_SYNC_ERR_LANE2_MASK   	 0x00000040 /**< SOT sync
							error lane2 */
#define XDSI2RX_IER_SOT_ERR_LANE1_MASK   	 0x00000020 /**< SOT error
							lane1 */
#define XDSI2RX_IER_SOT_SYNC_ERR_LANE1_MASK   	 0x00000010 /**< SOT sync
							error lane1 */
#define XDSI2RX_IER_ECC2_BIT_MASK   		 0x00000008 /**< SHIFT for
							ECC2 bit error */
#define XDSI2RX_IER_ECC1_BIT_MASK   		 0x00000004 /**< SHIFT for
							ECC1 bit error */
#define XDSI2RX_IER_CRC_ERR_MASK		 0x00000002  /**< Shift for
							CRC Error */
#define XDSI2RX_IER_UN_DATA_TYPE_MASK		 0x00000001 /**< Shift for
							Unsupported Data Type*/

#define XDSI2RX_IER_ALLINTR_MASK		 0x00275FFF /* ALL intr Mask*/

#define XDSI2RX_IER_FRAME_START_SHIFT		 21 /**< FRAME START detected */
#define XDSI2RX_IER_GSP_FIFO_FULL_ERR_SHIFT	 18 /**< Generic short packet
							FIFO FULL error */
#define XDSI2RX_IER_GSP_FIFO_NE_ERR_SHIFT	 17 /**< Generic short packet
							not empty error */
#define XDSI2RX_IER_STREAM_ASYNC_FIFO_FULL_ERR_SHIFT 16 /**< STREAM FIFO full error */
#define XDSI2RX_IER_LM_ASYNC_FIFO_FULL_ERR_SHIFT 14 /**< LM FIFO full error */
#define XDSI2RX_IER_STOP_STATE_SHIFT		 12 /**< STOP STATE Error */
#define XDSI2RX_IER_SOT_ERR_LANE4_SHIFT   	 11 /**< SOT error lane4 */
#define XDSI2RX_IER_SOT_SYNC_ERR_LANE4_SHIFT   	 10 /**< SOT sync error lane4 */
#define XDSI2RX_IER_SOT_ERR_LANE3_SHIFT   	 9 /**< SOT error lane3 */
#define XDSI2RX_IER_SOT_SYNC_ERR_LANE3_SHIFT   	 8 /**< SOT sync error lane3 */
#define XDSI2RX_IER_SOT_ERR_LANE2_SHIFT   	 7 /**< SOT error lane2 */
#define XDSI2RX_IER_SOT_SYNC_ERR_LANE2_SHIFT   	 6 /**< SOT sync error lane2 */
#define XDSI2RX_IER_SOT_ERR_LANE1_SHIFT   	 5 /**< SOT error lane1 */
#define XDSI2RX_IER_SOT_SYNC_ERR_LANE1_SHIFT   	 4 /**< SOT sync error lane1 */
#define XDSI2RX_IER_ECC2_BIT_SHIFT   		 3 /**< SHIFT for ECC2 bit error */
#define XDSI2RX_IER_ECC1_BIT_SHIFT   		 2 /**< SHIFT for ECC1 bit error */
#define XDSI2RX_IER_CRC_ERR_SHIFT		 1  /**< Shift for
							CRC Error */
#define XDSI2RX_IER_UN_DATA_TYPE_SHIFT		 0  /**< Shift for
							Unsupported Data Type*/
/*@}*/


/** @name Bitmasks and offsets of XDSI2RX_GSPR_OFFSET register
 *
 * This register contains the generic short packet command .
 *
 * @{
 */

#define XDSI2RX_GSPR_DATA_MASK	0x00FFFF00 /**< Mask for Data */
#define XDSI2RX_GSPR_VC_MASK	0x000000C0 /**< Mask for VC */
#define XDSI2RX_GSPR_DATA_TYPE_MASK 0x0000003F /**< Mask for Data Type */

#define XDSI2RX_GSPR_DATA_SHIFT	8 /**< Shift for Data */
#define XDSI2RX_GSPR_VC_SHIFT	6 /**< Shift for VC */
#define XDSI2RX_GSPR_DATA_TYPE_SHIFT 0 /**< Shift for DATA Type */

/*@}*/

/** @name Bitmasks and offsets of XDSI2RX_DCR_OFFSET register
 *
 * This register contains the generic short packet command .
 *
 * @{
 */

#define XDSI2RX_DCR_FRM_RCVD_MASK 0xFFFF0000 /**< mask for frames RCVD */
#define XDSI2RX_DCR_HLINE_MASK	0x0000FFFF /**< mask for H LINES */

#define XDSI2RX_DCR_FRM_RCVD_SHIFT 16 /**< Shift no of frames RCVD */
#define XDSI2RX_DCR_HLINE_SHIFT	 0 /**< Shift for horizontal lines */

/*@}*/

#define XDSI2RX_MAX_LANES	4	/**< Max Lanes supported by DSI2RX */

/**************************** Type Definitions *******************************/


/****************************Function Definitions *******************************/

/*****************************************************************************/
/**
* Inline function to read DSI2RX controller register.
*
* @param	BaseAddress is the base address of DSI
* @param	RegOffset is the register offset.
*
* @return	Value of the register.
*
* @note		None
*
******************************************************************************/
static inline u32 XDsi2Rx_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
{
	return (Xil_In32(BaseAddress + (u32)RegOffset));
}

/*****************************************************************************/
/**
* Inline Function to write to DSI2RX controller register.
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
static inline void XDsi2Rx_WriteReg(UINTPTR BaseAddress, u32 RegOffset, u32 Data)
{
	Xil_Out32(BaseAddress + (u32)RegOffset, (u32)Data);
}

#ifdef __cplusplus
}
#endif

#endif
/** @} */
