/******************************************************************************
* Copyright (C) 2005 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcan_l.h
* @addtogroup can Overview
* @{
*
* This header file contains the identifiers and basic driver functions (or
* macros) that can be used to access the device. Other driver functions
* are defined in xcan.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a xd   04/12/05 First release
* 1.10a mta  05/13/07 Updated to new coding style
* 2.00a ktn  10/22/09 Updated to use the HAL APIs/macros.
*		      The macros have been renamed to remove _m from the name.
* 3.8   ht   12/13/23 Added support for ECC.
*
* </pre>
*
******************************************************************************/

#ifndef XCAN_L_H		/* prevent circular inclusions */
#define XCAN_L_H		/**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register offsets for the CAN. Each register is 32 bits.
 *  @{
 */
#define XCAN_SRR_OFFSET		0x000  /**< Software Reset Register */
#define XCAN_MSR_OFFSET		0x004  /**< Mode Select Register */
#define XCAN_BRPR_OFFSET	0x008  /**< Baud Rate Prescaler Register */
#define XCAN_BTR_OFFSET		0x00C  /**< Bit Timing Register */
#define XCAN_ECR_OFFSET		0x010  /**< Error Counter Register */
#define XCAN_ESR_OFFSET		0x014  /**< Error Status Register */
#define XCAN_SR_OFFSET		0x018  /**< Status Register */

#define XCAN_ISR_OFFSET		0x01C  /**< Interrupt Status Register */
#define XCAN_IER_OFFSET		0x020  /**< Interrupt Enable Register */
#define XCAN_ICR_OFFSET		0x024  /**< Interrupt Clear Register */

#define XCAN_TXFIFO_ID_OFFSET	0x030  /**< TX FIFO ID */
#define XCAN_TXFIFO_DLC_OFFSET	0x034  /**< TX FIFO DLC */
#define XCAN_TXFIFO_DW1_OFFSET	0x038  /**< TX FIFO Data Word 1 */
#define XCAN_TXFIFO_DW2_OFFSET	0x03C  /**< TX FIFO Data Word 2 */

#define XCAN_TXBUF_ID_OFFSET	0x040  /**< TX High Priority Buffer ID */
#define XCAN_TXBUF_DLC_OFFSET	0x044  /**< TX High Priority Buffer DLC */
#define XCAN_TXBUF_DW1_OFFSET	0x048  /**< TX High Priority Buf Data Word 1 */
#define XCAN_TXBUF_DW2_OFFSET	0x04C  /**< TX High Priority Buf Data Word 2 */

#define XCAN_RXFIFO_ID_OFFSET	0x050  /**< RX FIFO ID */
#define XCAN_RXFIFO_DLC_OFFSET	0x054  /**< RX FIFO DLC */
#define XCAN_RXFIFO_DW1_OFFSET	0x058  /**< RX FIFO Data Word 1 */
#define XCAN_RXFIFO_DW2_OFFSET	0x05C  /**< RX FIFO Data Word 2 */

#define XCAN_AFR_OFFSET		0x060  /**< Acceptance Filter Register */
#define XCAN_AFMR1_OFFSET	0x064  /**< Acceptance Filter Mask Register 1 */
#define XCAN_AFIR1_OFFSET	0x068  /**< Acceptance Filter ID Register 1 */
#define XCAN_AFMR2_OFFSET	0x06C  /**< Acceptance Filter Mask Register 2 */
#define XCAN_AFIR2_OFFSET	0x070  /**< Acceptance Filter ID Register 2 */
#define XCAN_AFMR3_OFFSET	0x074  /**< Acceptance Filter Mask Register 3 */
#define XCAN_AFIR3_OFFSET	0x078  /**< Acceptance Filter ID Register 3 */
#define XCAN_AFMR4_OFFSET	0x07C  /**< Acceptance Filter Mask Register 4 */
#define XCAN_AFIR4_OFFSET	0x080  /**< Acceptance Filter ID Register 4 */

#define XCAN_ECC_CFG_OFFSET	0x0C8  /**< ECC Configuration register */
#define XCAN_TXTLFIFO_ECC_OFFSET	0x0CC  /**< TXTL FIFO ECC error counter */
#define XCAN_TXOLFIFO_ECC_OFFSET	0x0D0  /**< TXOL FIFO ECC error counter */
#define XCAN_RXFIFO_ECC_OFFSET	0X0D4  /**< RX FIFO ECC error counter */

/** @} */

/** @name Software Reset Register
 *  @{
 */
#define XCAN_SRR_CEN_MASK	0x00000002  /**< Can Enable Mask */
#define XCAN_SRR_SRST_MASK	0x00000001  /**< Reset Mask */
/** @} */

/** @name Mode Select Register
 *  @{
 */
#define XCAN_MSR_LBACK_MASK	0x00000002  /**< Loop Back Mode Select Mask */
#define XCAN_MSR_SLEEP_MASK	0x00000001  /**< Sleep Mode Select Mask */
/** @} */

/** @name Baud Rate Prescaler register
 *  @{
 */
#define XCAN_BRPR_BRP_MASK	0x000000FF  /**< Baud Rate Prescaler Mask */
/** @} */

/** @name Bit Timing Register
 *  @{
 */
#define XCAN_BTR_SJW_MASK	0x00000180  /**< Sync Jump Width Mask */
#define XCAN_BTR_SJW_SHIFT	7	    /**< Sync Jump Width Shift */
#define XCAN_BTR_TS2_MASK	0x00000070  /**< Time Segment 2 Mask */
#define XCAN_BTR_TS2_SHIFT	4	    /**< Time Segment 2 Shift */
#define XCAN_BTR_TS1_MASK	0x0000000F  /**< Time Segment 1 Mask */
/** @} */

/** @name Error Counter Register
 *  @{
 */
#define XCAN_ECR_REC_MASK	0x0000FF00  /**< Receive Error Counter Mask */
#define XCAN_ECR_REC_SHIFT	8	    /**< Receive Error Counter Shift */
#define XCAN_ECR_TEC_MASK	0x000000FF  /**< Transmit Error Counter Mask */
/** @} */

/** @name Error Status Register
 *  @{
 */
#define XCAN_ESR_ACKER_MASK	0x00000010  /**< ACK Error Mask */
#define XCAN_ESR_BERR_MASK	0x00000008  /**< Bit Error Mask */
#define XCAN_ESR_STER_MASK	0x00000004  /**< Stuff Error Mask */
#define XCAN_ESR_FMER_MASK	0x00000002  /**< Form Error Mask */
#define XCAN_ESR_CRCER_MASK	0x00000001  /**< CRC Error Mask */
/** @} */

/** @name Status Register
 *  @{
 */
#define XCAN_SR_ACFBSY_MASK	0x00000800  /**< Acceptance Filter busy Mask */
#define XCAN_SR_TXFLL_MASK	0x00000400  /**< TX FIFO is full Mask */
#define XCAN_SR_TXBFLL_MASK	0x00000200  /**< TX High Priority Buffer full */
#define XCAN_SR_ESTAT_MASK	0x00000180  /**< Error Status Mask */
#define XCAN_SR_ESTAT_SHIFT	7	    /**< Error Status Shift */
#define XCAN_SR_ERRWRN_MASK	0x00000040  /**< Error Warning Mask */
#define XCAN_SR_BBSY_MASK	0x00000020  /**< Bus Busy Mask */
#define XCAN_SR_BIDLE_MASK	0x00000010  /**< Bus Idle Mask */
#define XCAN_SR_NORMAL_MASK	0x00000008  /**< Normal Mode Mask */
#define XCAN_SR_SLEEP_MASK	0x00000004  /**< Sleep Mode Mask */
#define XCAN_SR_LBACK_MASK	0x00000002  /**< Loop Back Mode Mask */
#define XCAN_SR_CONFIG_MASK	0x00000001  /**< Configuration Mode Mask */
/** @} */

/** @name Interrupt Status/Enable/Clear Register
 *  @{
 */
#define XCAN_IXR_E2BERX_MASK	0x00800000  /**< RX FIFO two bit ECC error */
#define XCAN_IXR_E1BERX_MASK	0x00400000  /**< RX FIFO one bit ECC error */
#define XCAN_IXR_E2BETXOL_MASK	0x00200000  /**< TXOL FIFO two bit ECC error */
#define XCAN_IXR_E1BETXOL_MASK	0x00100000  /**< TXOL FIFO one bit ECC error */
#define XCAN_IXR_E2BETXTL_MASK	0x00080000  /**< TXTL FIFO two bit ECC error */
#define XCAN_IXR_E1BETXTL_MASK	0x00040000  /**< TXTL FIFO one bit ECC error */

#define XCAN_IXR_WKUP_MASK	0x00000800  /**< Wake up Interrupt Mask */
#define XCAN_IXR_SLP_MASK	0x00000400  /**< Sleep Interrupt Mask */
#define XCAN_IXR_BSOFF_MASK	0x00000200  /**< Bus Off Interrupt Mask */
#define XCAN_IXR_ERROR_MASK	0x00000100  /**< Error Interrupt Mask */
#define XCAN_IXR_RXNEMP_MASK	0x00000080  /**< RX FIFO Not Empty Intr Mask */
#define XCAN_IXR_RXOFLW_MASK	0x00000040  /**< RX FIFO Overflow Intr Mask */
#define XCAN_IXR_RXUFLW_MASK	0x00000020  /**< RX FIFO Underflow Intr Mask */
#define XCAN_IXR_RXOK_MASK	0x00000010  /**< New Message Received Intr */
#define XCAN_IXR_TXBFLL_MASK	0x00000008  /**< TX High Priority Buf Full  */
#define XCAN_IXR_TXFLL_MASK	0x00000004  /**< TX FIFO Full Interrupt Mask */
#define XCAN_IXR_TXOK_MASK	0x00000002  /**< TX Successful Interrupt Mask */
#define XCAN_IXR_ARBLST_MASK	0x00000001  /**< Arbitration Lost Intr Mask */

#define XCAN_IXR_ECC_MASK	(XCAN_IXR_E2BERX_MASK   | \
				XCAN_IXR_E1BERX_MASK    | \
				XCAN_IXR_E2BETXOL_MASK   | \
				XCAN_IXR_E1BETXOL_MASK   | \
				XCAN_IXR_E2BETXTL_MASK   | \
				XCAN_IXR_E1BETXTL_MASK)
					/**< Mask for ECC interrupts */


#define XCAN_IXR_ALL		(XCAN_IXR_WKUP_MASK   | \
				XCAN_IXR_SLP_MASK    | \
				XCAN_IXR_BSOFF_MASK  | \
				XCAN_IXR_ERROR_MASK  | \
				XCAN_IXR_RXNEMP_MASK | \
				XCAN_IXR_RXOFLW_MASK | \
				XCAN_IXR_RXUFLW_MASK | \
				XCAN_IXR_RXOK_MASK   | \
				XCAN_IXR_TXBFLL_MASK | \
				XCAN_IXR_TXFLL_MASK  | \
				XCAN_IXR_TXOK_MASK   | \
				XCAN_IXR_ARBLST_MASK)
					/**< Mask for basic interrupts */

/** @} */

/** @name CAN Frame Identifier (TX High Priority Buffer/TX/RX/Acceptance Filter
Mask/Acceptance Filter ID)
 *  @{
 */
#define XCAN_IDR_ID1_MASK	0xFFE00000  /**< Standard Messg Ident Mask */
#define XCAN_IDR_ID1_SHIFT	21	    /**< Standard Messg Ident Shift */
#define XCAN_IDR_SRR_MASK	0x00100000  /**< Substitute Remote TX Req */
#define XCAN_IDR_SRR_SHIFT	20	    /**< Shift Value for SRR */
#define XCAN_IDR_IDE_MASK	0x00080000  /**< Identifier Extension Mask */
#define XCAN_IDR_IDE_SHIFT	19	    /**< Identifier Extension Shift */
#define XCAN_IDR_ID2_MASK	0x0007FFFE  /**< Extended Message Ident Mask */
#define XCAN_IDR_ID2_SHIFT	1	    /**< Extended Message Ident Shift */
#define XCAN_IDR_RTR_MASK	0x00000001  /**< Remote TX Request Mask */
/** @} */

/** @name CAN Frame Data Length Code (TX High Priority Buffer/TX/RX)
 *  @{
 */
#define XCAN_DLCR_DLC_MASK	0xF0000000  /**< Data Length Code Mask */
#define XCAN_DLCR_DLC_SHIFT	28  	    /**< Data Length Code Shift */
/** @} */

/** @name CAN Frame Data Word 1 (TX High Priority Buffer/TX/RX)
 *  @{
 */
#define XCAN_DW1R_DB0_MASK	0xFF000000  /**< Data Byte 0 Mask */
#define XCAN_DW1R_DB0_SHIFT	24	    /**< Data Byte 0 Shift */
#define XCAN_DW1R_DB1_MASK	0x00FF0000  /**< Data Byte 1 Mask */
#define XCAN_DW1R_DB1_SHIFT	16	    /**< Data Byte 1 Shift */
#define XCAN_DW1R_DB2_MASK	0x0000FF00  /**< Data Byte 2 Mask */
#define XCAN_DW1R_DB2_SHIFT	8	    /**< Data Byte 2 Shift */
#define XCAN_DW1R_DB3_MASK	0x000000FF  /**< Data Byte 3 Mask */
/** @} */

/** @name CAN Frame Data Word 2 (TX High Priority Buffer/TX/RX)
 *  @{
 */
#define XCAN_DW2R_DB4_MASK	0xFF000000  /**< Data Byte 4 Mask */
#define XCAN_DW2R_DB4_SHIFT	24	    /**< Data Byte 4 Shift */
#define XCAN_DW2R_DB5_MASK	0x00FF0000  /**< Data Byte 5 Mask */
#define XCAN_DW2R_DB5_SHIFT	16	    /**< Data Byte 5 Shift */
#define XCAN_DW2R_DB6_MASK	0x0000FF00  /**< Data Byte 6 Mask */
#define XCAN_DW2R_DB6_SHIFT	8	    /**< Data Byte 6 Shift */
#define XCAN_DW2R_DB7_MASK	0x000000FF  /**< Data Byte 7 */
/** @} */

/** @name Acceptance Filter Register
 *  @{
 */
#define XCAN_AFR_UAF4_MASK	0x00000008  /**< Use Acceptance Filter No.4 */
#define XCAN_AFR_UAF3_MASK	0x00000004  /**< Use Acceptance Filter No.3 */
#define XCAN_AFR_UAF2_MASK	0x00000002  /**< Use Acceptance Filter No.2 */
#define XCAN_AFR_UAF1_MASK	0x00000001  /**< Use Acceptance Filter No.1 */
#define XCAN_AFR_UAF_ALL_MASK	(XCAN_AFR_UAF4_MASK | XCAN_AFR_UAF3_MASK | \
				 XCAN_AFR_UAF2_MASK | XCAN_AFR_UAF1_MASK)
					   /**< Mask for Acceptance Filters */
/** @} */

/** @name ECC Configuration register
 *  @{
 */
#define XCAN_ECC_CFG_RST_MASK		0x00000007  /**< Reset Mask for ECC configuration register */
#define XCAN_ECC_CFG_REECRX_MASK	0x00000004  /**< Reset RX FIFO ECC error counters */
#define XCAN_ECC_CFG_REECTXOL_MASK	0x00000002  /**< Reset TXOL FIFO ECC error counters */
#define XCAN_ECC_CFG_REECTXTL_MASK	0x00000001  /**< Reset TXTL FIFO ECC error counters */
#define XCAN_ECC_2BIT_SHIFT	16  /**< ECC 2bit error counter shift */
/** @} */

/** @name CAN frame length constants
 *  @{
 */
#define XCAN_MAX_FRAME_SIZE 16	/**< Maximum CAN frame length in bytes */
/** @} */


/** @name Mask for Low 16bits and High 16 bits
 *  @{
 */
#define XCAN_MASK_LOW_16BITS	0x0000FFFF /**< Mask to obtain lower 16bits */
#define XCAN_MASK_HIGH_16BITS	0XFFFF0000 /**< Mask to obtain higher 16bits */
/** @} */
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* This macro reads the given register.
*
* @param	BaseAddress is the base address of the device
* @param	RegOffset is the register offset to be read
*
* @return	The 32-bit value of the register
*
* @note		C-Style signature:
*		u32 XCan_ReadReg(u32 BaseAddress, u32 RegOffset);
*
*****************************************************************************/
#define XCan_ReadReg(BaseAddress, RegOffset) \
	Xil_In32((BaseAddress) + (RegOffset))


/****************************************************************************/
/**
*
* This macro writes the given register.
*
* @param	BaseAddress is the base address of the device
* @param	RegOffset is the register offset to be written
* @param	Data is the 32-bit value to write to the register
*
* @return	None.
*
* @note		C-Style signature:
*		u32 XCan_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data);
*
*****************************************************************************/
#define XCan_WriteReg(BaseAddress, RegOffset, Data) \
	Xil_Out32((BaseAddress) + (RegOffset), (Data))

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */

/** @} */
