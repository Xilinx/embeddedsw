/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcanfd_hw.h
* @addtogroup canfd_v2_6
* @{
*
* This header file contains the identifiers and basic driver functions (or
* macros) that can be used to access the device. Other driver functions
* are defined in xcanfd.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   nsk  06/04/2015 First release
* 1.0   nsk  15/05/2015 Modified XCANFD_BTR_TS1_MASK
*		 	(CR 861772).
* 1.0	nsk  16/06/2015 Added New definitions for Register
*			bits since RTL has changed.RTL Changes,Added
*		        new bits to MSR,SR,ISR,IER,ICR Registers and modified
*		        TS2 bits in BTR and F_SJW bits in F_BTR Registers.
* 2.1   ask  07/03/18 Added support for canfd 2.0 spec sequential mode.
*       ask  07/03/18 Fix for Sequential recv CR# 992606,CR# 1004222.
* 2.2   sn   06/11/19 Updated Mailbox RX buffer offset for CANFD2.0
*		      Fixed below  incorrect mask values
*		      XCANFD_MAILBOX_RB_MASK_BASE_OFFSET,XCANFD_WMR_RXFP_MASK
*		      and CONTROL_STATUS_3.
*
* </pre>
*
******************************************************************************/

#ifndef XCANFD_HW_H		/* prevent circular inclusions */
#define XCANFD_HW_H		/**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xparameters.h"
/************************** Constant Definitions *****************************/

/** @name Register offsets for the CAN. Each register is 32 bits.
 *  @{
 */
#define XCANFD_SRR_OFFSET	0x000U  /**< Software Reset Register */
#define XCANFD_MSR_OFFSET	0x004U  /**< Mode Select Register */
#define XCANFD_BRPR_OFFSET	0x008U  /**< Baud Rate Prescaler Register */
#define XCANFD_BTR_OFFSET	0x00CU  /**< Bit Timing Register */
#define XCANFD_ECR_OFFSET	0x010U  /**< Error Counter Register */
#define XCANFD_ESR_OFFSET	0x014U  /**< Error Status Register */
#define XCANFD_SR_OFFSET	0x018U  /**< Status Register */

#define XCANFD_ISR_OFFSET	0x01CU  /**< Interrupt Status Register */
#define XCANFD_IER_OFFSET	0x020U  /**< Interrupt Enable Register */
#define XCANFD_ICR_OFFSET	0x024U  /**< Interrupt Clear Register */

#define XCANFD_F_BRPR_OFFSET	0x088U  /**< Data Phase Baud Rate Prescalar
						Register */
#define XCANFD_F_BTR_OFFSET	0x08CU  /**< Data Phase Bit Timing Register */
#define XCANFD_TRR_OFFSET	0x090U  /**< Tx Buffer Ready Request Register */
#define XCANFD_IETRS_OFFSET	0x094U  /**< Tx Buffer Ready Request Served
						Interrupt Enable Register */
#define XCANFD_TCR_OFFSET	0x098U  /**< Tx Buffer Cancel Request Register
					*/
#define XCANFD_IETCS_OFFSET	0x09CU  /**< Tx Buffer Cancel Request Served
						Interrupt Enable Register */
#define XCANFD_RSD0_OFFSET	0x0A0U  /**< Reserved */
#define XCANFD_RSD1_OFFSET	0x0A4U  /**< Reserved */
#define XCANFD_RSD2_OFFSET	0x0A8U  /**< Reserved */
#define XCANFD_RSD3_OFFSET	0x0ACU  /**< Reserved */
/** @} */

/** @name Mail box mode registers
 * @{
 */
#define XCANFD_RCS0_OFFSET	0x0B0U  /**< Rx Buffer Control Status 0 Register
					 */
#define XCANFD_RCS1_OFFSET	0x0B4U  /**< Rx Buffer Control Status 1 Register
					 */
#define XCANFD_RCS2_OFFSET	0x0B8U  /**< Rx Buffer Control Status 2 Register
					 */
#define XCANFD_RCS_HCB_MASK	0xFFFFU /**< Rx Buffer Control Status Register
					 Host Control Bit Mask */
#define XCANFD_RXBFLL1_OFFSET	0x0C0U  /**< Rx Buffer Full Interrupt Enable
					Register */
#define XCANFD_RXBFLL2_OFFSET 	0x0C4U  /**< Rx Buffer Full Interrupt Enable
					Register */
#if defined (CANFD_v1_0)
#define XCANFD_MAILBOX_RB_MASK_BASE_OFFSET	0x1000U  /**< Mailbox RxBuffer
							 Mask Register */
#else
#define XCANFD_MAILBOX_RB_MASK_BASE_OFFSET	0x2F00U  /**< Mailbox RxBuffer
							 Mask Register */
#endif
#define XCANFD_MAILBOX_NXT_RB			4U	/**< Mailbox Next Buffer */
#define XCANFD_MBRXBUF_MASK		0x0000FFFFU	/**< Mailbox Max Rx Buffer mask */
/** @} */

/** @name TxBuffer Element ID Registers
 * Tx Message Buffer Element Start Address - 0x0100   (2304 Bytes)
			     End Address   - 0x09FF
* @{
*/
#define XCANFD_DLCR_TIMESTAMP_MASK 0x0000FFFFU	/**< Dlc Register TimeStamp
							Mask */
#define XCANFD_TXFIFO_0_BASE_ID_OFFSET  0x0100U  /**< Tx Message Buffer Element
							0 ID Register  */
/** @} */

/** @name TxBuffer Element DLC Registers
* @{
*/
#define XCANFD_TXFIFO_0_BASE_DLC_OFFSET 0x0104U  /**< Tx Message Buffer Element
							0 DLC Register  */
/** @} */

/** @name TxBuffer Element DW Registers
* @{
*/
#define XCANFD_TXFIFO_0_BASE_DW0_OFFSET 0x0108U  /**< Tx Message Buffer Element
							0 DW Register  */
/** @} */

/** @name TXEVENT Buffer Element ID Registers
* @{
*/
#define XCANFD_TXEFIFO_0_BASE_ID_OFFSET  0x2000U  /**< Tx Event Message Buffer
							Element 0 ID Register  */
/** @} */

/** @name TXEVENT Buffer Element DLC Registers
* @{
*/
#define XCANFD_TXEFIFO_0_BASE_DLC_OFFSET  0x2004U  /**< Tx Event Message Buffer
                                                  Element 0 DLC Register  */
/** @} */
/** @name Rx Message Buffer Element ID Registers.
 * Start Address - 0x1100   (2304 Bytes)
			     End Address   - 0x19FF
* @{
*/
#if defined (CANFD_v1_0)
#define XCANFD_RXFIFO_0_BASE_ID_OFFSET  0x1100U  /**< Rx Message Buffer Element
							0 ID Register  */
#else
#define XCANFD_RXFIFO_0_BASE_ID_OFFSET  0x2100U  /**< Rx Message Buffer Element
							0 ID Register  */
#endif
/** @} */

/** @name Rx Message Buffer Element DLC Registers.
* @{
*/
#if defined (CANFD_v1_0)
#define XCANFD_RXFIFO_0_BASE_DLC_OFFSET 0x1104U  /**< Rx Message Buffer Element
							0 DLC Register  */
#else
#define XCANFD_RXFIFO_0_BASE_DLC_OFFSET 0x2104U  /**< Rx Message Buffer Element
							0 DLC Register  */
#endif
/** @} */

/** @name Rx Message Buffer Element DW Registers.
* @{
*/
#if defined (CANFD_v1_0)
#define XCANFD_RXFIFO_0_BASE_DW0_OFFSET 0x1108U /**< Rx Message Buffer Element
							0 DW Register  */
#else
#define XCANFD_RXFIFO_0_BASE_DW0_OFFSET 0x2108U /**< Rx Message Buffer Element
							0 DW Register  */
#endif
/** @} */
/** @name Rx Message Buffer Element FIFO 1 ID Registers.
 * Start Address - 0x4100
			     End Address   - 0x52b8
* @{
*/
#define XCANFD_RXFIFO_1_BUFFER_0_BASE_ID_OFFSET  0x4100U  /**< Rx Message Buffer Element
							0 ID Register  */
/** @} */

/** @name Rx Message Buffer Element FIFO 1 DLC Registers.
 * Start Address - 0x4104
			     End Address   - 0x52bc
* @{
*/
#define XCANFD_RXFIFO_1_BUFFER_0_BASE_DLC_OFFSET 0x4104U  /**< Rx Message Buffer Element
							0 DLC Register  */
/** @} */

/** @name Rx Message Buffer Element FIFO 1 SW Registers.
 * Start Address - 0x4108
			     End Address   - 0x52c0
* @{
*/
#define XCANFD_RXFIFO_1_BUFFER_0_BASE_DW0_OFFSET 0x4108U /**< Rx Message Buffer Element
							0 DW Register  */
/** @} */

/** @name Rx Message Buffer Element ID,DLC,DW Sizes.
* @{
*/
#define XCANFD_RXFIFO_NEXTID_OFFSET	72U	/**< Rx Message Buffer Element
						Next ID AT Offset */
#define XCANFD_RXFIFO_NEXTDLC_OFFSET	72U	/**< Rx Message Buffer Element
						 Next DLC AT Offset */
#define XCANFD_RXFIFO_NEXTDW_OFFSET	72U	/**< Rx Message Buffer Element
						 Next DW AT Offset */
/** @} */

/** @name EDL and BRS Masks.
* @{
*/
#define XCANFD_DLCR_EDL_MASK	0x08000000U  /**< EDL Mask in DLC Register */
#define XCANFD_DLCR_BRS_MASK	0x04000000U  /**< BRS Mask in DLC Register */
/** @} */

/** @name Acceptance Filter Mask Registers
 *  @{
 */
#if defined (CANFD_v1_0)
#define XCANFD_AFMR_BASE_OFFSET	0x1A00U  /**<Acceptance Filter Mask Register */
#define XCANFD_AFIDR_BASE_OFFSET 0x1A04U  /**< Acceptance Filter ID Register */
#else
#define XCANFD_AFMR_BASE_OFFSET	0x0A00U  /**<Acceptance Filter Mask Register */
#define XCANFD_AFIDR_BASE_OFFSET 0x0A04U  /**< Acceptance Filter ID Register */
#endif
#define XCANFD_AFMR_NXT_OFFSET	8U	/**< Acceptance Filter Next Offset */

#define XCANFD_AFIDR_NXT_OFFSET	 8U	/**< Acceptance Filter ID Next Offset */
#define XCANFD_AFR_OFFSET	0x0E0U  /**< Acceptance Filter Register */
#define XCANFD_FSR_OFFSET	0x0E8U  /**< Receive FIFO Status Register */
#define XCANFD_NOOF_AFR		32U	/**< Number Of Acceptance FIlter
						Registers */
#define XCANFD_WIR_OFFSET	0x0ECU  /**< Rx FIFO Water Mark Register */
#if defined (CANFD_v1_0)
#define XCANFD_WIR_MASK	0x0000001FU  /**< Rx FIFO Full watermark Mask */
#define XCANFD_WM_FIFO0_THRESHOLD  31/**< Watermark Threshold Value */
#else
#define XCANFD_WIR_MASK	0x0000003FU  /**< Rx FIFO Full watermark Mask */
#define XCANFD_WM_FIFO0_THRESHOLD  63/**< Watermark Threshold Value */
#endif
#define XCANFD_WMR_RXFWM_1_MASK 0x00003F00U /**< RX FIFO 1 Full Watermark
                                               Mask */
#define XCANFD_WMR_RXFWM_1_SHIFT 8U /**< RX FIFO 1 Full Watermark
                                               Mask */
#define XCANFD_WMR_RXFP_MASK 0x001F0000U /**< Receive filter partition
                                               Mask */
#define XCANFD_WMR_RXFP_SHIFT 16U /**< Receive filter partition
                                               Mask */
#define XCANFD_TXEVENT_WIR_OFFSET 0x000000A4U /**< TX FIFO Watermark Offset */
#define XCANFD_TXEVENT_WIR_MASK   0x0FU       /**< TX Event Watermark Mask */
#define XCANFD_TIMESTAMPR_OFFSET	0x0028U	/**< Time Stamp Register */
#define XCANFD_CTS_MASK		0x00000001U /**< Time Stamp Counter Clear */
#define XCANFD_DAR_MASK		0x00000010U/**< Disable AutoRetransmission */
/** @} */

/** @name Software Reset Register
 *  @{
 */
#define XCANFD_SRR_CEN_MASK	0x00000002U  /**< Can Enable Mask */
#define XCANFD_SRR_SRST_MASK	0x00000001U  /**< Reset Mask */
/** @} */

/** @name Mode Select Register
 *  @{
 */
#define XCANFD_MSR_LBACK_MASK	0x00000002U  /**< Loop Back Mode Select Mask */
#define XCANFD_MSR_SLEEP_MASK	0x00000001U  /**< Sleep Mode Select Mask */
#define XCANFD_MSR_BRSD_MASK	0x00000008U  /**< Bit Rate Switch Select Mask */
#define XCANFD_MSR_DAR_MASK	0x00000010U  /**< Disable Auto-Retransmission
						 Select Mask */
#define XCANFD_MSR_SNOOP_MASK	0x00000004U  /**< Snoop Mode Select Mask */
#define XCANFD_MSR_DPEE_MASK	0x00000020U  /**< Protocol Exception Event
						 Mask */
#define XCANFD_MSR_SBR_MASK	0x00000040U  /**< Start Bus-Off Recovery Mask */
#define XCANFD_MSR_ABR_MASK     0x00000080U  /**< Auto Bus-Off Recovery Mask */
#define XCANFD_MSR_CONFIG_MASK	0x000000F8U  /**< Configuration Mode Mask */
/** @} */

/** @name Baud Rate Prescaler register
 *  @{
 */
#define XCANFD_BRPR_BRP_MASK	0x000000FFU  /**< Baud Rate Prescaler Mask */
/** @} */

/** @name Bit Timing Register
 *  @{
 */
#if defined (CANFD_v1_0)
#define XCANFD_BTR_SJW_MASK	0x000F0000U  /**< Sync Jump Width Mask */
#define XCANFD_BTR_TS2_MASK	0x00000F00U  /**< Time Segment 2 Mask */
#define XCANFD_BTR_TS1_MASK	0x0000003FU  /**< Time Segment 1 Mask */
#define XCANFD_F_BRPR_TDCMASK	0x00001F00U	/**< Transceiver Delay
                                        compensation Offset Mask */
#else
#define XCANFD_BTR_SJW_MASK	0x007F0000U  /**< Sync Jump Width Mask */
#define XCANFD_BTR_TS2_MASK	0x00007F00U  /**< Time Segment 2 Mask */
#define XCANFD_BTR_TS1_MASK	0x000000FFU  /**< Time Segment 1 Mask */
#define XCANFD_F_BRPR_TDCMASK	0x00003F00U	/**< Transceiver Delay
						compensation Offset Mask */
#endif
#define XCANFD_BTR_TS2_SHIFT	8U	    /**< Time Segment 2 Shift */
#define XCANFD_BTR_SJW_SHIFT	16U	    /**< Sync Jump Width Shift */
#define XCANFD_F_BRPR_TDC_ENABLE_MASK	0x00010000U	/**< Transceiver Delay
							compensation Enable
							Maskk */
/** @} */

/** @name Fast Bit Timing Register
 *  @{
 */
#if defined (CANFD_v1_0)
#define XCANFD_F_BTR_SJW_MASK	0x00070000U  /**< Sync Jump Width Mask */
#define XCANFD_F_BTR_TS2_MASK	0x00000700U  /**< Time Segment 2 Mask */
#define XCANFD_F_BTR_TS1_MASK	0x0000000FU  /**< Time Segment 1 Mask */
#else
#define XCANFD_F_BTR_SJW_MASK	0x000F0000U  /**< Sync Jump Width Mask */
#define XCANFD_F_BTR_TS2_MASK	0x00000F00U  /**< Time Segment 2 Mask */
#define XCANFD_F_BTR_TS1_MASK	0x0000001FU  /**< Time Segment 1 Mask */
#endif
#define XCANFD_F_BTR_TS2_SHIFT	8U	    /**< Time Segment 2 Shift */
#define XCANFD_F_BTR_SJW_SHIFT	16U	    /**< Sync Jump Width Shift */
/** @} */

/** @name Error Counter Register
 *  @{
 */
#define XCANFD_ECR_REC_MASK	0x0000FF00U  /**< Receive Error Counter Mask */
#define XCANFD_ECR_REC_SHIFT	8U	    /**< Receive Error Counter Shift */
#define XCANFD_ECR_TEC_MASK	0x000000FFU  /**< Transmit Error Counter Mask */
/** @} */

/** @name Error Status Register
 *  @{
 */
#define XCANFD_ESR_ACKER_MASK	0x00000010U  /**< ACK Error Mask */
#define XCANFD_ESR_BERR_MASK	0x00000008U  /**< Bit Error Mask */
#define XCANFD_ESR_STER_MASK	0x00000004U  /**< Stuff Error Mask */
#define XCANFD_ESR_FMER_MASK	0x00000002U  /**< Form Error Mask */
#define XCANFD_ESR_CRCER_MASK	0x00000001U  /**< CRC Error Mask */
#define XCANFD_ESR_F_BERR_MASK	0x00000800U  /**< F_Bit Error Mask */
#define XCANFD_ESR_F_STER_MASK	0x00000400U  /**< F_Stuff Error Mask */
#define XCANFD_ESR_F_FMER_MASK	0x00000200U  /**< F_Form Error Mask */
#define XCANFD_ESR_F_CRCER_MASK	0x00000100U  /**< F_CRC Error Mask */
/** @} */

/** @name Status Register
 *  @{
 */
#define XCANFD_SR_TDCV_MASK	0x007F0000U  /**< Transceiver Dealy compensation
						Mask */
#define XCANFD_SR_SNOOP_MASK	0x00001000U  /**< Snoop Mode Mask */
#define XCANFD_SR_ESTAT_MASK	0x00000180U  /**< Error Status Mask */
#define XCANFD_SR_ESTAT_SHIFT	7U	    /**< Error Status Shift */
#define XCANFD_SR_ERRWRN_MASK	0x00000040U  /**< Error Warning Mask */
#define XCANFD_SR_BBSY_MASK	0x00000020U  /**< Bus Busy Mask */
#define XCANFD_SR_BIDLE_MASK	0x00000010U  /**< Bus Idle Mask */
#define XCANFD_SR_NORMAL_MASK	0x00000008U  /**< Normal Mode Mask */
#define XCANFD_SR_SLEEP_MASK	0x00000004U  /**< Sleep Mode Mask */
#define XCANFD_SR_LBACK_MASK	0x00000002U  /**< Loop Back Mode Mask */
#define XCANFD_SR_CONFIG_MASK	0x00000001U  /**< Configuration Mode Mask */
#define XCANFD_SR_PEE_CONFIG_MASK 0x00000200U /**< Protocol Exception Mode
						  Indicator Mask */
#define XCANFD_SR_BSFR_CONFIG_MASK 0x00000400U /**< Bus-Off recovery Mode
						   Indicator Mask */
#define XCANFD_SR_NISO_MASK	0x00000800U /**< Non-ISO Core Mask */
/** @} */

/** @name Interrupt Status/Enable/Clear Register
 *  @{
 */
#define XCANFD_IXR_RXBOFLW_BI_MASK 0x3F000000U /**< Rx Buffer index for Overflow
						   (Mailbox Mode) */
#define XCANFD_IXR_RXLRM_BI_MASK   0x00FC0000U /**< Rx Buffer index for Last
						   Received Message (Mailbox
						    Mode) */
#define XCANFD_RXLRM_BI_SHIFT	18U	/**< Rx Buffer Index Shift Value */
#define XCANFD_CSB_SHIFT	16U	/**< Core Status Bit Shift Value */
#define XCANFD_IXR_RXMNF_MASK	 0x00020000U /**< Rx Match Not Finished Intr
						Mask */
#define XCANFD_IXR_RXBOFLW_MASK	 0x00010000U /**< Rx Buffer Overflow interrupt
						Mask (Mailbox mode) */
#define XCANFD_IXR_RXRBF_MASK	 0x00008000U /**< Rx Buffer Full Interrupt Mask
						(Mailbox mode) */
#define XCANFD_IXR_TXCRS_MASK	 0x00004000U /**< Tx Cancellation Request Served
						 Interrupt Mask */
#define XCANFD_IXR_TXRRS_MASK	 0x00002000U /**< Tx Buffer Ready Request Served
						 Interrupt Mask */
#define XCANFD_IXR_RXFWMFLL_MASK	 0x00001000U /**< Rx  Watermark Full
							interrupt
							 Mask (FIFO mode) */
#define XCANFD_IXR_WKUP_MASK	0x00000800U  /**< Wake up Interrupt Mask */
#define XCANFD_IXR_SLP_MASK	0x00000400U  /**< Sleep Interrupt Mask */
#define XCANFD_IXR_BSOFF_MASK	0x00000200U  /**< Bus Off Interrupt Mask */
#define XCANFD_IXR_ERROR_MASK	0x00000100U  /**< Error Interrupt Mask */
#define XCANFD_IXR_RXFOFLW_MASK	0x00000040U  /**< RX FIFO Overflow Intr Mask */
#define XCANFD_IXR_RXOK_MASK	0x00000010U  /**< New Message Received Intr */
#define XCANFD_IXR_TXOK_MASK	0x00000002U  /**< TX Successful Interrupt Mask
						*/
#define XCANFD_IXR_ARBLST_MASK	0x00000001U  /**< Arbitration Lost Intr Mask */
#define XCANFD_IXR_PEE_MASK	0x00000004U  /**< Protocol Exception Intr Mask */
#define XCANFD_IXR_BSRD_MASK	0x00000008U  /**< Bus-Off recovery done Intr
						 Mask */
#define XCANFD_IXR_TSCNT_OFLW_MASK	0x00000020U  /**< Timestamp overflow
                          Mask*/
#define XCANFD_IXR_RXFOFLW_1_MASK	0x00008000U /**< RX FIFO 1 Overflow Intr
                                               Mask */
#define XCANFD_IXR_RXFWMFLL_1_MASK	0x00010000U /**< Rx  Watermark Full
							interrupt Mask for FIFO 1 */
#define XCANFD_IXR_TXEOFLW_MASK	    0x40000000U /**< TX Event FIFO Intr Mask */
#define XCANFD_IXR_TXEWMFLL_MASK	0x80000000U /**< TX Event FIFO
                                   Watermark Full Intr Mask */
#if defined(CANFD_v1_0)
#define XCANFD_IXR_ALL		(XCANFD_IXR_PEE_MASK	 	| \
				XCANFD_IXR_BSRD_MASK		| \
				XCANFD_IXR_RXMNF_MASK 		| \
				XCANFD_IXR_RXBOFLW_MASK 	| \
				XCANFD_IXR_RXRBF_MASK 		| \
				XCANFD_IXR_TXCRS_MASK 		| \
				XCANFD_IXR_TXRRS_MASK 		| \
				XCANFD_IXR_RXFWMFLL_MASK 	| \
				XCANFD_IXR_WKUP_MASK	   	| \
				XCANFD_IXR_SLP_MASK		| \
				XCANFD_IXR_BSOFF_MASK  		| \
				XCANFD_IXR_ERROR_MASK  		| \
				XCANFD_IXR_RXFOFLW_MASK 	| \
				XCANFD_IXR_RXOK_MASK   		| \
				XCANFD_IXR_TXOK_MASK   		| \
				XCANFD_IXR_ARBLST_MASK)
#else
#define XCANFD_IXR_ALL		(XCANFD_IXR_PEE_MASK	 	| \
				XCANFD_IXR_BSRD_MASK		| \
				XCANFD_IXR_RXMNF_MASK 		| \
				XCANFD_IXR_RXBOFLW_MASK 	| \
				XCANFD_IXR_RXRBF_MASK 		| \
				XCANFD_IXR_TXCRS_MASK 		| \
				XCANFD_IXR_TXRRS_MASK 		| \
				XCANFD_IXR_RXFWMFLL_MASK 	| \
				XCANFD_IXR_WKUP_MASK	   	| \
				XCANFD_IXR_SLP_MASK		| \
				XCANFD_IXR_BSOFF_MASK  		| \
				XCANFD_IXR_ERROR_MASK  		| \
				XCANFD_IXR_RXFOFLW_MASK 	| \
				XCANFD_IXR_RXOK_MASK   		| \
				XCANFD_IXR_TXOK_MASK   		| \
				XCANFD_IXR_ARBLST_MASK      | \
				XCANFD_IXR_TSCNT_OFLW_MASK  | \
				XCANFD_IXR_RXFOFLW_1_MASK   | \
				XCANFD_IXR_RXFWMFLL_1_MASK  | \
				XCANFD_IXR_TXEOFLW_MASK     | \
				XCANFD_IXR_TXEWMFLL_MASK)
				/**< Mask for Basic interrupts */
#endif
/** @} */

/** @name Transmit Ready request All Mask
 *  @{
 */
#define XCANFD_TRR_MASK 0xFFFFFFFFU /**< Transmit Ready request All Mask */
/** @} */

/** @name CAN Frame Identifier (TX High Priority Buffer/TX/RX/Acceptance Filter
Mask/Acceptance Filter ID)
 *  @{
 */
#define XCANFD_IDR_ID1_MASK	0xFFE00000U  /**< Standard Messg Ident Mask */
#define XCANFD_IDR_ID1_SHIFT	21U	    /**< Standard Messg Ident Shift */
#define XCANFD_IDR_SRR_MASK	0x00100000U  /**< Substitute Remote TX Req */
#define XCANFD_IDR_SRR_SHIFT	20U	     /**< Substitue Remote TX Shift */
#define XCANFD_IDR_IDE_MASK	0x00080000U  /**< Identifier Extension Mask */
#define XCANFD_IDR_IDE_SHIFT	19U	    /**< Identifier Extension Shift */
#define XCANFD_IDR_ID2_MASK	0x0007FFFEU  /**< Extended Message Ident Mask */
#define XCANFD_IDR_ID2_SHIFT	1U	    /**< Extended Message Ident Shift
						*/
#define XCANFD_IDR_RTR_MASK	0x00000001U  /**< Remote TX Request Mask */
/** @} */

/** @name CAN Frame Data Length Code (TX High Priority Buffer/TX/RX)
 *  @{
 */
#define XCANFD_DLCR_DLC_MASK	0xF0000000U  /**< Data Length Code Mask */
#define XCANFD_DLCR_DLC_SHIFT	28U  	    /**< Data Length Code Shift */
#define XCANFD_DLCR_MM_MASK	0x00FF0000U  /**< Message Marker Mask */
#define XCANFD_DLCR_MM_SHIFT	16U  /**< Message Marker Shift */
#define XCANFD_DLCR_EFC_MASK	0x01000000U  /**< Event FIFO Control Mask */
#define XCANFD_DLCR_EFC_SHIFT	24U  /**< Event FIFO Control Shift */
#define XCANFD_DLC1 0x10000000U	/**< Data Length Code 1 */
#define XCANFD_DLC2 0x20000000U	/**< Data Length Code 2 */
#define XCANFD_DLC3 0x30000000U	/**< Data Length Code 3 */
#define XCANFD_DLC4 0x40000000U	/**< Data Length Code 4 */
#define XCANFD_DLC5 0x50000000U	/**< Data Length Code 5 */
#define XCANFD_DLC6 0x60000000U	/**< Data Length Code 6 */
#define XCANFD_DLC7 0x70000000U	/**< Data Length Code 7 */
#define XCANFD_DLC8 0x80000000U	/**< Data Length Code 8 */
#define XCANFD_DLC9 0x90000000U	/**< Data Length Code 9 */
#define XCANFD_DLC10 0xA0000000U	/**< Data Length Code 10 */
#define XCANFD_DLC11 0xB0000000U	/**< Data Length Code 11 */
#define XCANFD_DLC12 0xC0000000U	/**< Data Length Code 12 */
#define XCANFD_DLC13 0xD0000000U	/**< Data Length Code 13 */
#define XCANFD_DLC14 0xE0000000U	/**< Data Length Code 14 */
#define XCANFD_DLC15 0xF0000000U	/**< Data Length Code 15 */
/** @} */

/** @name Acceptance Filter Register
 *  @{
 */
#define XCANFD_AFR_UAF_ALL_MASK	0xFFFFFFFFU /**< Acceptance Filter Register */
/** @} */

/** @name CAN Receive FIFO Status Register
 *  @{
 */
#if defined (CANFD_v1_0)
#define XCANFD_FSR_FL_MASK	0x00001F00U  /**< Fill Level Mask */
#define XCANFD_FSR_RI_MASK	0x0000001FU  /**< Read Index Mask */
#else
#define XCANFD_FSR_FL_MASK	0x00007F00U  /**< Fill Level Mask FIFO 0 */
#define XCANFD_FSR_RI_MASK	0x0000003FU  /**< Read Index Mask FIFO 0 */
#endif
#define XCANFD_FSR_FL_1_MASK	0x7F000000U  /**< Fill Level Mask FIFO 1 */
#define XCANFD_FSR_RI_1_MASK	0x003F0000U  /**< Read Index Mask FIFO 1 */
#define XCANFD_FSR_IRI_1_MASK	0x00800000U  /**< Increment Read Index
                                            Mask FIFO1 */
#define XCANFD_FSR_FL_0_SHIFT	8U   /**< Fill Level Mask FIFO 0 */
#define XCANFD_FSR_FL_1_SHIFT	24U  /**< Fill Level Mask FIFO 1 */
#define XCANFD_FSR_RI_1_SHIFT	16U  /**< Read Index Mask FIFO 1 */
#define XCANFD_FSR_IRI_MASK	0x00000080U  /**< Increment Read Index Mask */
/** @} */
/** @name CAN RX FIFO Watermark Register
 *  @{
 */
 #if defined (CANFD_v1_0)
 #define XCANFD_WMR_RXFWM_MASK 0x0000001FU /**< RX FIFO Full Watermark
                                               Mask */
 #else
 #define XCANFD_WMR_RXFWM_MASK 0x0000003FU /**< RX FIFO 0 Full Watermark
                                               Mask */
 #endif
 #define XCANFD_WMR_RXFWM_1_MASK 0x00003F00U /**< RX FIFO 1 Full Watermark
                                               Mask */

/** @} */
 /** @name TX Event FIFO Registers
 *  @{
 */
 #define XCANFD_TXE_FWM_OFFSET   0x000000A4U /**< TX Event FIFO watermark
                                               Offset */
 #define XCANFD_TXE_FWM_MASK     0x0000001FU /**< TX Event FIFO watermark
                                               Mask */
 #define XCANFD_TXE_FSR_OFFSET   0x000000A0U /**< TX Event FIFO Status
                                               Register Offset */
 #define XCANFD_TXE_RI_MASK      0x0000001FU /**< TX Event FIFO Read
                                               Index Mask */
 #define XCANFD_TXE_IRI_MASK     0x00000080U /**< TX Event FIFO
                                              Increment Read Index Mask */
 #define XCANFD_TXE_FL_MASK      0x00001F00U /**< TX Event FIFO Fill
                                               Level Mask */
 #define XCANFD_TXE_FL_SHIFT    8U /**< TX Event FIFO Fill
                                               Level Shift */
 #define XCANFD_TXE_IRI_SHIFT  7U /**< TX Event FIFO
                                    Increment Read Index SHIFT */

/** @} */
/** @name CAN TxBuffer Ready Request Served Interrupt Enable Register Masks
 *  @{
 */
#define XCANFD_TXBUFFER0_RDY_RQT_MASK 0x00000001U	/**< TxBuffer0 Ready
							Request Mask */
#define XCANFD_TXBUFFER1_RDY_RQT_MASK	0x00000002U	/**< TxBuffer1 Ready
							Request Mask */
#define XCANFD_TXBUFFER2_RDY_RQT_MASK	0x00000004U	/**< TxBuffer2 Ready
							Request Mask */
#define XCANFD_TXBUFFER3_RDY_RQT_MASK	0x00000008U	/**< TxBuffer3 Ready
							Request Mask */
#define XCANFD_TXBUFFER4_RDY_RQT_MASK	0x00000010U	/**< TxBuffer4 Ready
							Request Mask */
#define XCANFD_TXBUFFER5_RDY_RQT_MASK	0x00000020U	/**< TxBuffer5 Ready
							Request Mask */
#define XCANFD_TXBUFFER6_RDY_RQT_MASK	0x00000040U	/**< TxBuffer6 Ready
							Request Mask */
#define XCANFD_TXBUFFER7_RDY_RQT_MASK	0x00000080U	/**< TxBuffer7 Ready
							Request Mask */
#define XCANFD_TXBUFFER8_RDY_RQT_MASK	0x00000100U	/**< TxBuffer8 Ready
							Request Mask */
#define XCANFD_TXBUFFER9_RDY_RQT_MASK	0x00000200U	/**< TxBuffer9 Ready
							Request Mask */
#define XCANFD_TXBUFFER10_RDY_RQT_MASK	0x00000400U	/**< TxBuffer10 Ready
							Request Mask */
#define XCANFD_TXBUFFER11_RDY_RQT_MASK	0x00000800U	/**< TxBuffer11 Ready
							Request Mask */
#define XCANFD_TXBUFFER12_RDY_RQT_MASK	0x00001000U	/**< TxBuffer12 Ready
							Request Mask */
#define XCANFD_TXBUFFER13_RDY_RQT_MASK	0x00002000U	/**< TxBuffer13 Ready
							Request Mask */
#define XCANFD_TXBUFFER14_RDY_RQT_MASK	0x00004000U	/**< TxBuffer14 Ready
							Request Mask */
#define XCANFD_TXBUFFER15_RDY_RQT_MASK	0x00008000U	/**< TxBuffer15 Ready
							Request Mask */
#define XCANFD_TXBUFFER16_RDY_RQT_MASK	0x00010000U	/**< TxBuffer16 Ready
							Request Mask */
#define XCANFD_TXBUFFER17_RDY_RQT_MASK	0x00020000U	/**< TxBuffer17 Ready
							Request Mask */
#define XCANFD_TXBUFFER18_RDY_RQT_MASK	0x00040000U	/**< TxBuffer18 Ready
							Request Mask */
#define XCANFD_TXBUFFER19_RDY_RQT_MASK	0x00080000U	/**< TxBuffer19 Ready
							Request Mask */
#define XCANFD_TXBUFFER20_RDY_RQT_MASK	0x00100000U	/**< TxBuffer20 Ready
							Request Mask */
#define XCANFD_TXBUFFER21_RDY_RQT_MASK	0x00200000U	/**< TxBuffer21 Ready
							Request Mask */
#define XCANFD_TXBUFFER22_RDY_RQT_MASK	0x00400000U	/**< TxBuffer22 Ready
							Request Mask */
#define XCANFD_TXBUFFER23_RDY_RQT_MASK	0x00800000U	/**< TxBuffer23 Ready
							Request Mask */
#define XCANFD_TXBUFFER24_RDY_RQT_MASK	0x01000000U	/**< TxBuffer24 Ready
							Request Mask */
#define XCANFD_TXBUFFER25_RDY_RQT_MASK	0x02000000U	/**< TxBuffer25 Ready
							Request Mask */
#define XCANFD_TXBUFFER26_RDY_RQT_MASK	0x04000000U	/**< TxBuffer26 Ready
							Request Mask */
#define XCANFD_TXBUFFER27_RDY_RQT_MASK	0x08000000U	/**< TxBuffer27 Ready
							Request Mask */
#define XCANFD_TXBUFFER28_RDY_RQT_MASK	0x10000000U	/**< TxBuffer28 Ready
							Request Mask */
#define XCANFD_TXBUFFER29_RDY_RQT_MASK	0x20000000U	/**< TxBuffer29 Ready
							Request Mask */
#define XCANFD_TXBUFFER30_RDY_RQT_MASK	0x40000000U	/**< TxBuffer30 Ready
							Request Mask */
#define XCANFD_TXBUFFER31_RDY_RQT_MASK	0x80000000U	/**< TxBuffer31 Ready
							Request Mask */
#define XCANFD_TXBUFFER_ALL_RDY_RQT_MASK	0xFFFFFFFFU
							/**< TxBuffer Ready
							Request Mask for ALL */
/** @} */

/** @name CAN TxBuffer Cancel Request Served Interrupt Enable Register Masks
 *  @{
 */
#define XCANFD_TXBUFFER0_CANCEL_RQT_MASK 0x00000001U	/**< TxBuffer0 Cancel
								Request Mask */
#define XCANFD_TXBUFFER1_CANCEL_RQT_MASK	0x00000002U	/**< TxBuffer1
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER2_CANCEL_RQT_MASK	0x00000004U	/**< TxBuffer2
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER3_CANCEL_RQT_MASK	0x00000008U	/**< TxBuffer3
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER4_CANCEL_RQT_MASK	0x00000010U	/**< TxBuffer4
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER5_CANCEL_RQT_MASK	0x00000020U	/**< TxBuffer5
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER6_CANCEL_RQT_MASK	0x00000040U	/**< TxBuffer6
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER7_CANCEL_RQT_MASK	0x00000080U	/**< TxBuffer7
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER8_CANCEL_RQT_MASK	0x00000100U	/**< TxBuffer8
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER9_CANCEL_RQT_MASK	0x00000200U	/**< TxBuffer9
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER10_CANCEL_RQT_MASK	0x00000400U	/**< TxBuffer10
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER11_CANCEL_RQT_MASK	0x00000800U	/**< TxBuffer11
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER12_CANCEL_RQT_MASK	0x00001000U	/**< TxBuffer12
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER13_CANCEL_RQT_MASK	0x00002000U	/**< TxBuffer13
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER14_CANCEL_RQT_MASK	0x00004000U	/**< TxBuffer14
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER15_CANCEL_RQT_MASK	0x00008000U	/**< TxBuffer15
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER16_CANCEL_RQT_MASK	0x00010000U	/**< TxBuffer16
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER17_CANCEL_RQT_MASK	0x00020000U	/**< TxBuffer17
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER18_CANCEL_RQT_MASK	0x00040000U	/**< TxBuffer18
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER19_CANCEL_RQT_MASK	0x00080000U	/**< TxBuffer19
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER20_CANCEL_RQT_MASK	0x00100000U	/**< TxBuffer20
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER21_CANCEL_RQT_MASK	0x00200000U	/**< TxBuffer21
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER22_CANCEL_RQT_MASK	0x00400000U	/**< TxBuffer22
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER23_CANCEL_RQT_MASK	0x00800000U	/**< TxBuffer23
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER24_CANCEL_RQT_MASK	0x01000000U	/**< TxBuffer24
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER25_CANCEL_RQT_MASK	0x02000000U	/**< TxBuffer25
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER26_CANCEL_RQT_MASK	0x04000000U	/**< TxBuffer26
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER27_CANCEL_RQT_MASK	0x08000000U	/**< TxBuffer27
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER28_CANCEL_RQT_MASK	0x10000000U	/**< TxBuffer28
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER29_CANCEL_RQT_MASK	0x20000000U	/**< TxBuffer29
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER30_CANCEL_RQT_MASK	0x40000000U	/**< TxBuffer30
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER31_CANCEL_RQT_MASK	0x80000000U	/**< TxBuffer31
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER_CANCEL_RQT_ALL_MASK 0xFFFFFFFFU		/**< TxBuffer
								Cancel Request
								Mask for ALL */
/** @} */

/** @name CAN RxBuffer Full Register
 *  @{
 */
#define XCANFD_RXBUFFER0_FULL_MASK	0x00000001U	/**< RxBuffer0 Full
								Mask */
#define XCANFD_RXBUFFER1_FULL_MASK	0x00000002U	/**< RxBuffer1 Full
								Mask */
#define XCANFD_RXBUFFER2_FULL_MASK	0x00000004U	/**< RxBuffer2 Full
								Mask */
#define XCANFD_RXBUFFER3_FULL_MASK	0x00000008U	/**< RxBuffer3 Full
								Mask */
#define XCANFD_RXBUFFER4_FULL_MASK	0x00000010U	/**< RxBuffer4 Full
								Mask */
#define XCANFD_RXBUFFER5_FULL_MASK	0x00000020U	/**< RxBuffer5 Full
								Mask */
#define XCANFD_RXBUFFER6_FULL_MASK	0x00000040U	/**< RxBuffer6 Full
								Mask */
#define XCANFD_RXBUFFER7_FULL_MASK	0x00000080U	/**< RxBuffer7 Full
								Mask */
#define XCANFD_RXBUFFER8_FULL_MASK	0x00000100U	/**< RxBuffer8 Full
								Mask */
#define XCANFD_RXBUFFER9_FULL_MASK	0x00000200U	/**< RxBuffer9 Full
								Mask */
#define XCANFD_RXBUFFER10_FULL_MASK	0x00000400U	/**< RxBuffer10 Full
								Mask */
#define XCANFD_RXBUFFER11_FULL_MASK	0x00000800U	/**< RxBuffer11 Full
								Mask */
#define XCANFD_RXBUFFER12_FULL_MASK	0x00001000U	/**< RxBuffer12 Full
								Mask */
#define XCANFD_RXBUFFER13_FULL_MASK	0x00002000U	/**< RxBuffer13 Full
								Mask */
#define XCANFD_RXBUFFER14_FULL_MASK	0x00004000U	/**< RxBuffer14 Full
								Mask */
#define XCANFD_RXBUFFER15_FULL_MASK	0x00008000U	/**< RxBuffer15 Full
								Mask */
#define XCANFD_RXBUFFER16_FULL_MASK	0x00010000U	/**< RxBuffer16 Full
								Mask */
#define XCANFD_RXBUFFER17_FULL_MASK	0x00020000U	/**< RxBuffer17 Full
								Mask */
#define XCANFD_RXBUFFER18_FULL_MASK	0x00040000U	/**< RxBuffer18 Full
								Mask */
#define XCANFD_RXBUFFER19_FULL_MASK	0x00080000U	/**< RxBuffer19 Full
								Mask */
#define XCANFD_RXBUFFER20_FULL_MASK	0x00100000U	/**< RxBuffer20 Full
								Mask */
#define XCANFD_RXBUFFER21_FULL_MASK	0x00200000U	/**< RxBuffer21 Full
								Mask */
#define XCANFD_RXBUFFER22_FULL_MASK	0x00400000U	/**< RxBuffer22 Full
								Mask */
#define XCANFD_RXBUFFER23_FULL_MASK	0x00800000U	/**< RxBuffer23 Full
								Mask */
#define XCANFD_RXBUFFER24_FULL_MASK	0x01000000U	/**< RxBuffer24 Full
								Mask */
#define XCANFD_RXBUFFER25_FULL_MASK	0x02000000U	/**< RxBuffer25 Full
								Mask */
#define XCANFD_RXBUFFER26_FULL_MASK	0x04000000U	/**< RxBuffer26 Full
								Mask */
#define XCANFD_RXBUFFER27_FULL_MASK	0x08000000U	/**< RxBuffer27 Full
								Mask */
#define XCANFD_RXBUFFER28_FULL_MASK	0x10000000U	/**< RxBuffer28 Full
								Mask */
#define XCANFD_RXBUFFER29_FULL_MASK	0x20000000U	/**< RxBuffer29 Full
								Mask */
#define XCANFD_RXBUFFER30_FULL_MASK	0x40000000U	/**< RxBuffer30 Full
								Mask */
#define XCANFD_RXBUFFER31_FULL_MASK	0x80000000U	/**< RxBuffer31 Full
								Mask */
#define XCANFD_RXBUFFER32_FULL_MASK	0x00000001U	/**< RxBuffer32 Full
								Mask */
#define XCANFD_RXBUFFER33_FULL_MASK	0x00000002U	/**< RxBuffer33 Full
								Mask */
#define XCANFD_RXBUFFER34_FULL_MASK	0x00000004U	/**< RxBuffer34 Full
								Mask */
#define XCANFD_RXBUFFER35_FULL_MASK	0x00000008U	/**< RxBuffer35 Full
								Mask */
#define XCANFD_RXBUFFER36_FULL_MASK	0x00000010U	/**< RxBuffer36 Full
								Mask */
#define XCANFD_RXBUFFER37_FULL_MASK	0x00000020U	/**< RxBuffer37 Full
								Mask */
#define XCANFD_RXBUFFER38_FULL_MASK	0x00000040U	/**< RxBuffer38 Full
								Mask */
#define XCANFD_RXBUFFER39_FULL_MASK	0x00000080U	/**< RxBuffer39 Full
								Mask */
#define XCANFD_RXBUFFER40_FULL_MASK	0x00000100U	/**< RxBuffer40 Full
								Mask */
#define XCANFD_RXBUFFER41_FULL_MASK	0x00000200U	/**< RxBuffer41 Full
								Mask */
#define XCANFD_RXBUFFER42_FULL_MASK	0x00000400U	/**< RxBuffer42 Full
								Mask */
#define XCANFD_RXBUFFER43_FULL_MASK	0x00000800U	/**< RxBuffer43 Full
								Mask */
#define XCANFD_RXBUFFER44_FULL_MASK	0x00001000U	/**< RxBuffer44 Full
								Mask */
#define XCANFD_RXBUFFER45_FULL_MASK	0x00002000U	/**< RxBuffer45 Full
								Mask */
#define XCANFD_RXBUFFER46_FULL_MASK	0x00004000U	/**< RxBuffer46 Full
								Mask */
#define XCANFD_RXBUFFER47_FULL_MASK	0x00008000U	/**< RxBuffer47 Full
								Mask */
/** @} */

/** @name CAN frame length constants
 *  @{
 */
#define XCANFD_MAX_FRAME_SIZE 72U	/**< Maximum CAN frame length in bytes
					 */
#define XCANFD_TXE_MESSAGE_SIZE 8U	/**< TX Message Size */
#define XCANFD_DW_BYTES	4U		/**< Data Word Bytes */
#define XST_NOBUFFER	33L	/**< All Buffers (32) are filled */
#define XST_BUFFER_ALREADY_FILLED	34L	/**< Given Buffer is Already
						filled */
#define XST_INVALID_DLC		16L		/**< Invalid Dlc code */
#define TRR_POS_MASK            0x1U		/**< TRR Position Mask */
#define MAX_BUFFER_VAL          32U		/**< Max Buffer Value */
#define FAST_MATH_MASK1         0xDB6DB6DBU	/**< Fast Math Mask 1 */
#define FAST_MATH_MASK2         0x49249249U	/**< Fast Math Mask 2 */
#define FAST_MATH_MASK3         0xC71C71C7U	/**< Fast Math Mask 3 */
#define TRR_INIT_VAL            0x00000000U	/**< TRR Initial value */
#define TRR_MASK_INIT_VAL       0xFFFFFFFFU	/**< TRR Mask Initial value */
#define DESIGN_RANGE_1          15U		/**< Design Range 1 */
#define DESIGN_RANGE_2          31U		/**< Design Range 2 */
#define CONTROL_STATUS_1        0U		/**< Control Status 1 */
#define CONTROL_STATUS_2        1U		/**< Control Status 2 */
#define CONTROL_STATUS_3        2U		/**< Congrol Status 3 */
#define EXTRACTION_MASK         63U		/**< Extraction Mask */
#define SHIFT1			1U		/**< Flag for Shift 1 */
#define SHIFT2			2U		/**< Flag for Shift 2 */
#define SHIFT3			3U		/**< Flag for Shift 3 */
#define TDC_MAX_OFFSET		32U		/**< TDC Max Offset */
#define TDC_SHIFT		8U		/**< Shift Value for TDC */
#define MAX_BUFFER_INDEX	32U		/**< Max Buffer Index */
#define MIN_FILTER_INDEX        0U		/**< Minimum Filter Index */
#define MAX_FILTER_INDEX        32U		/**< Maximum Filter Index */
#define EDL_CANFD		1U		/**< Extended Data Length for CANFD */
#define EDL_CAN			0U		/**< Extended Data Length for CAN */
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
*		u32 XCanFd_ReadReg(u32 BaseAddress, u32 RegOffset);
*
*****************************************************************************/
#define XCanFd_ReadReg(BaseAddress, RegOffset) \
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
*		u32 XCanFd_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data);
*
*****************************************************************************/
#define XCanFd_WriteReg(BaseAddress, RegOffset, Data) \
		Xil_Out32((BaseAddress) + (RegOffset), (Data))

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
