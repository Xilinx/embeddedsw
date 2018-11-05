/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* @file xcanfd_hw.h
* @addtogroup canfd_v1_2
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
*
* </pre>
*
******************************************************************************/

#ifndef XCANFD_HW_H		/* prevent circular inclusions */
#define XCANFD_HW_H		/* by using protection macros */

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
#define XCANFD_SRR_OFFSET		0x000  /**< Software Reset Register */
#define XCANFD_MSR_OFFSET		0x004  /**< Mode Select Register */
#define XCANFD_BRPR_OFFSET	0x008  /**< Baud Rate Prescaler Register */
#define XCANFD_BTR_OFFSET	0x00C  /**< Bit Timing Register */
#define XCANFD_ECR_OFFSET	0x010  /**< Error Counter Register */
#define XCANFD_ESR_OFFSET	0x014  /**< Error Status Register */
#define XCANFD_SR_OFFSET	0x018  /**< Status Register */

#define XCANFD_ISR_OFFSET	0x01C  /**< Interrupt Status Register */
#define XCANFD_IER_OFFSET	0x020  /**< Interrupt Enable Register */
#define XCANFD_ICR_OFFSET	0x024  /**< Interrupt Clear Register */

#define XCANFD_F_BRPR_OFFSET	0x088  /**< Data Phase Buad Rate Prescalar
						Register */
#define XCANFD_F_BTR_OFFSET	0x08C  /**< Data Phase Bit Timing Register */
#define XCANFD_TRR_OFFSET	0x090  /**< Tx Buffer Ready Request Register */
#define XCANFD_IETRS_OFFSET	0x094  /**< Tx Buffer Ready Request Served
						Interrupt Enable Register */
#define XCANFD_TCR_OFFSET	0x098  /**< Tx Buffer Cancel Request Register
					*/
#define XCANFD_IETCS_OFFSET	0x09C  /**< Tx Buffer Cancel Request Served
						Interrupt Enable Register */
#define XCANFD_RSD0_OFFSET	0x0A0  /**< Reserved */
#define XCANFD_RSD1_OFFSET	0x0A4  /**< Reserved */
#define XCANFD_RSD2_OFFSET	0x0A8  /**< Reserved */
#define XCANFD_RSD3_OFFSET	0x0AC  /**< Reserved */
/* @} */

/** @name Mail box mode registers
 * @{
 */
#define XCANFD_RCS0_OFFSET	0x0B0  /**< Rx Buffer Control Status 0 Register
					 */
#define XCANFD_RCS1_OFFSET	0x0B4  /**< Rx Buffer Control Status 1 Register
					 */
#define XCANFD_RCS2_OFFSET	0x0B8  /**< Rx Buffer Control Status 2 Register
					 */
#define XCANFD_RCS_HCB_MASK	0xFFFF /**< Rx Buffer Control Status Register
					 Host Control Bit Mask */
#define XCANFD_RXBFLL1_OFFSET	0x0C0  /**< Rx Buffer Full Interrupt Enable
					Register */
#define XCANFD_RXBFLL2_OFFSET 	0x0C4  /**< Rx Buffer Full Interrupt Enable
					Register */
#define XCANFD_MAILBOX_RB_MASK_BASE_OFFSET	0x1000  /**< Mailbox RxBuffer
							 Mask Register */
#define XCANFD_MAILBOX_NXT_RB			4
/* @} */

/** @name TxBuffer Element ID Registers
 * Tx Message Buffer Element Start Address - 0x0100   (2304 Bytes)
			     End Address   - 0x09FF
* @{
*/
#define XCANFD_DLCR_TIMESTAMP_MASK 0x0000FFFF	/**< Dlc Register TimeStamp
							Mask */
#define XCANFD_TXFIFO_0_BASE_ID_OFFSET  0x0100  /**< Tx Message Buffer Element
							0 ID Register  */
#define XCANFD_TXFIFO_1_BASE_ID_OFFSET  0x0148  /**< Tx Message Buffer Element
							1 ID Register  */
#define XCANFD_TXFIFO_2_BASE_ID_OFFSET  0x0190  /**< Tx Message Buffer Element
							2 ID Register  */
#define XCANFD_TXFIFO_3_BASE_ID_OFFSET  0x01d8  /**< Tx Message Buffer Element
							3 ID Register  */
#define XCANFD_TXFIFO_4_BASE_ID_OFFSET  0x0220  /**< Tx Message Buffer Element
							4 ID Register  */
#define XCANFD_TXFIFO_5_BASE_ID_OFFSET  0x0268  /**< Tx Message Buffer Element
							5 ID Register  */
#define XCANFD_TXFIFO_6_BASE_ID_OFFSET  0x02b0  /**< Tx Message Buffer Element
							6 ID Register  */
#define XCANFD_TXFIFO_7_BASE_ID_OFFSET  0x02f8  /**< Tx Message Buffer Element
							7 ID Register  */
#define XCANFD_TXFIFO_8_BASE_ID_OFFSET  0x0340  /**< Tx Message Buffer Element
							8 ID Register  */
#define XCANFD_TXFIFO_9_BASE_ID_OFFSET  0x0388  /**< Tx Message Buffer Element
							9 ID Register  */
#define XCANFD_TXFIFO_10_BASE_ID_OFFSET 0x03d0  /**< Tx Message Buffer Element
							10 ID Register  */
#define XCANFD_TXFIFO_11_BASE_ID_OFFSET 0x0418  /**< Tx Message Buffer Element
							11 ID Register  */
#define XCANFD_TXFIFO_12_BASE_ID_OFFSET 0x0460  /**< Tx Message Buffer Element
							12 ID Register  */
#define XCANFD_TXFIFO_13_BASE_ID_OFFSET 0x04a8  /**< Tx Message Buffer Element
							13 ID Register  */
#define XCANFD_TXFIFO_14_BASE_ID_OFFSET 0x04f0  /**< Tx Message Buffer Element
							14 ID Register  */
#define XCANFD_TXFIFO_15_BASE_ID_OFFSET 0x0538  /**< Tx Message Buffer Element
							15 ID Register  */
#define XCANFD_TXFIFO_16_BASE_ID_OFFSET 0x0580  /**< Tx Message Buffer Element
							16 ID Register  */
#define XCANFD_TXFIFO_17_BASE_ID_OFFSET 0x05c8  /**< Tx Message Buffer Element
							17 ID Register  */
#define XCANFD_TXFIFO_18_BASE_ID_OFFSET 0x0610  /**< Tx Message Buffer Element
							18 ID Register  */
#define XCANFD_TXFIFO_19_BASE_ID_OFFSET 0x0658  /**< Tx Message Buffer Element
							19 ID Register  */
#define XCANFD_TXFIFO_20_BASE_ID_OFFSET 0x06a0  /**< Tx Message Buffer Element
							20 ID Register  */
#define XCANFD_TXFIFO_21_BASE_ID_OFFSET 0x06e8  /**< Tx Message Buffer Element
							21 ID Register  */
#define XCANFD_TXFIFO_22_BASE_ID_OFFSET 0x0730  /**< Tx Message Buffer Element
							22 ID Register  */
#define XCANFD_TXFIFO_23_BASE_ID_OFFSET 0x0778  /**< Tx Message Buffer Element
							23 ID Register  */
#define XCANFD_TXFIFO_24_BASE_ID_OFFSET 0x07c0  /**< Tx Message Buffer Element
							24 ID Register  */
#define XCANFD_TXFIFO_25_BASE_ID_OFFSET 0x0808  /**< Tx Message Buffer Element
							25 ID Register  */
#define XCANFD_TXFIFO_26_BASE_ID_OFFSET 0x0850  /**< Tx Message Buffer Element
							26 ID Register  */
#define XCANFD_TXFIFO_27_BASE_ID_OFFSET 0x0898  /**< Tx Message Buffer Element
							27 ID Register  */
#define XCANFD_TXFIFO_28_BASE_ID_OFFSET 0x08e0  /**< Tx Message Buffer Element
							28 ID Register  */
#define XCANFD_TXFIFO_29_BASE_ID_OFFSET 0x0928  /**< Tx Message Buffer Element
							29 ID Register  */
#define XCANFD_TXFIFO_30_BASE_ID_OFFSET 0x0970  /**< Tx Message Buffer Element
							30 ID Register  */
#define XCANFD_TXFIFO_31_BASE_ID_OFFSET 0x09b8  /**< Tx Message Buffer Element
							31 ID Register  */
/* @} */

/** @name TxBuffer Element DLC Registers
* @{
*/
#define XCANFD_TXFIFO_0_BASE_DLC_OFFSET 0x0104  /**< Tx Message Buffer Element
							0 DLC Register  */
#define XCANFD_TXFIFO_1_BASE_DLC_OFFSET 0x014c  /**< Tx Message Buffer Element
							1 DLC Register  */
#define XCANFD_TXFIFO_2_BASE_DLC_OFFSET 0x0194  /**< Tx Message Buffer Element
							2 DLC Register  */
#define XCANFD_TXFIFO_3_BASE_DLC_OFFSET 0x01dc  /**< Tx Message Buffer Element
							3 DLC Register  */
#define XCANFD_TXFIFO_4_BASE_DLC_OFFSET 0x0224  /**< Tx Message Buffer Element
							4 DLC Register  */
#define XCANFD_TXFIFO_5_BASE_DLC_OFFSET 0x026c  /**< Tx Message Buffer Element
							5 DLC Register  */
#define XCANFD_TXFIFO_6_BASE_DLC_OFFSET 0x02b4  /**< Tx Message Buffer Element
							6 DLC Register  */
#define XCANFD_TXFIFO_7_BASE_DLC_OFFSET 0x02fc  /**< Tx Message Buffer Element
							7 DLC Register  */
#define XCANFD_TXFIFO_8_BASE_DLC_OFFSET 0x0344  /**< Tx Message Buffer Element
							8 DLC Register  */
#define XCANFD_TXFIFO_9_BASE_DLC_OFFSET 0x038c  /**< Tx Message Buffer Element
							9 DLC Register  */
#define XCANFD_TXFIFO_10_BASE_DLC_OFFSET	0x03d4  /**< Tx Message Buffer
								Element10 DLC
								Register  */
#define XCANFD_TXFIFO_11_BASE_DLC_OFFSET        0x041c  /**< Tx Message Buffer
								Element11 DLC
								Register  */
#define XCANFD_TXFIFO_12_BASE_DLC_OFFSET        0x0464  /**< Tx Message Buffer
								Element12 DLC
								Register  */
#define XCANFD_TXFIFO_13_BASE_DLC_OFFSET        0x04ac  /**< Tx Message Buffer
								Element13 DLC
								Register  */
#define XCANFD_TXFIFO_14_BASE_DLC_OFFSET        0x04f4  /**< Tx Message Buffer
								Element14 DLC
								Register  */
#define XCANFD_TXFIFO_15_BASE_DLC_OFFSET        0x053c  /**< Tx Message Buffer
								Element15 DLC
								Register  */
#define XCANFD_TXFIFO_16_BASE_DLC_OFFSET        0x0584  /**< Tx Message Buffer
								Element16 DLC
								Register  */
#define XCANFD_TXFIFO_17_BASE_DLC_OFFSET        0x05cc  /**< Tx Message Buffer
								Element17 DLC
								Register  */
#define XCANFD_TXFIFO_18_BASE_DLC_OFFSET        0x0614  /**< Tx Message Buffer
								Element18 DLC
								Register  */
#define XCANFD_TXFIFO_19_BASE_DLC_OFFSET        0x065c  /**< Tx Message Buffer
								Element19 DLC
								Register  */
#define XCANFD_TXFIFO_20_BASE_DLC_OFFSET        0x06a4  /**< Tx Message Buffer
								Element20 DLC
								Register  */
#define XCANFD_TXFIFO_21_BASE_DLC_OFFSET        0x06ec  /**< Tx Message Buffer
								Element21 DLC
								Register  */
#define XCANFD_TXFIFO_22_BASE_DLC_OFFSET        0x0734  /**< Tx Message Buffer
								Element22 DLC
								Register  */
#define XCANFD_TXFIFO_23_BASE_DLC_OFFSET        0x077c  /**< Tx Message Buffer
								Element23 DLC
								Register  */
#define XCANFD_TXFIFO_24_BASE_DLC_OFFSET        0x07c4  /**< Tx Message Buffer
								Element24 DLC
								Register  */
#define XCANFD_TXFIFO_25_BASE_DLC_OFFSET        0x080c  /**< Tx Message Buffer
								Element25 DLC
								Register  */
#define XCANFD_TXFIFO_26_BASE_DLC_OFFSET        0x0854  /**< Tx Message Buffer
								Element26 DLC
								Register  */
#define XCANFD_TXFIFO_27_BASE_DLC_OFFSET        0x089c  /**< Tx Message Buffer
								Element27 DLC
								Register  */
#define XCANFD_TXFIFO_28_BASE_DLC_OFFSET        0x08e4  /**< Tx Message Buffer
								Element28 DLC
								Register  */
#define XCANFD_TXFIFO_29_BASE_DLC_OFFSET        0x092c  /**< Tx Message Buffer
								Element29 DLC
								Register  */
#define XCANFD_TXFIFO_30_BASE_DLC_OFFSET        0x0974  /**< Tx Message Buffer
								Element30 DLC
								Register  */
#define XCANFD_TXFIFO_31_BASE_DLC_OFFSET        0x09bc  /**< Tx Message Buffer
								Element31 DLC
								Register  */
/* @} */

/** @name TxBuffer Element DW Registers
* @{
*/
#define XCANFD_TXFIFO_0_BASE_DW0_OFFSET 0x0108  /**< Tx Message Buffer Element
							0 DW Register  */
#define XCANFD_TXFIFO_1_BASE_DW0_OFFSET 0x0150  /**< Tx Message Buffer Element
							1 DW Register  */
#define XCANFD_TXFIFO_2_BASE_DW0_OFFSET 0x0198  /**< Tx Message Buffer Element
							2 DW Register  */
#define XCANFD_TXFIFO_3_BASE_DW0_OFFSET 0x01e0  /**< Tx Message Buffer Element
							3 DW Register  */
#define XCANFD_TXFIFO_4_BASE_DW0_OFFSET 0x0228  /**< Tx Message Buffer Element
							4 DW Register  */
#define XCANFD_TXFIFO_5_BASE_DW0_OFFSET 0x0270  /**< Tx Message Buffer Element
							5 DW Register  */
#define XCANFD_TXFIFO_6_BASE_DW0_OFFSET 0x02b8  /**< Tx Message Buffer Element
							6 DW Register  */
#define XCANFD_TXFIFO_7_BASE_DW0_OFFSET 0x0300  /**< Tx Message Buffer Element
							7 DW Register  */
#define XCANFD_TXFIFO_8_BASE_DW0_OFFSET 0x0348  /**< Tx Message Buffer Element
							8 DW Register  */
#define XCANFD_TXFIFO_9_BASE_DW0_OFFSET 0x0390  /**< Tx Message Buffer Element
							9 DW Register  */
#define XCANFD_TXFIFO_10_BASE_DW0_OFFSET	0x03d8  /**< Tx Message Buffer
							 Element 10 DW Register
							 */
#define XCANFD_TXFIFO_11_BASE_DW0_OFFSET	0x0420  /**< Tx Message Buffer
							 Element 11 DW Register
							 */
#define XCANFD_TXFIFO_12_BASE_DW0_OFFSET	0x0468  /**< Tx Message Buffer
							 Element 12 DW Register
							*/
#define XCANFD_TXFIFO_13_BASE_DW0_OFFSET	0x04b0  /**< Tx Message Buffer
							 Element 13 DW Register
							 */
#define XCANFD_TXFIFO_14_BASE_DW0_OFFSET	0x04f8  /**< Tx Message Buffer
							 Element 14 DW Register
							 */
#define XCANFD_TXFIFO_15_BASE_DW0_OFFSET	0x0540  /**< Tx Message Buffer
							 Element 15 DW Register
							 */
#define XCANFD_TXFIFO_16_BASE_DW0_OFFSET	0x0588  /**< Tx Message Buffer
							 Element 16 DW Register
							 */
#define XCANFD_TXFIFO_17_BASE_DW0_OFFSET	0x05d0  /**< Tx Message Buffer
							 Element 17 DW Register
							 */
#define XCANFD_TXFIFO_18_BASE_DW0_OFFSET	0x0618  /**< Tx Message Buffer
							 Element 18 DW Register
							 */
#define XCANFD_TXFIFO_19_BASE_DW0_OFFSET	0x0660  /**< Tx Message Buffer
							 Element 19 DW Register
							 */
#define XCANFD_TXFIFO_20_BASE_DW0_OFFSET	0x06a8  /**< Tx Message Buffer
							 Element 20 DW Register
							 */
#define XCANFD_TXFIFO_21_BASE_DW0_OFFSET	0x06f0  /**< Tx Message Buffer
							 Element 21 DW Register
							 */
#define XCANFD_TXFIFO_22_BASE_DW0_OFFSET	0x0738  /**< Tx Message Buffer
							 Element 22 DW Register
							 */
#define XCANFD_TXFIFO_23_BASE_DW0_OFFSET	0x0780  /**< Tx Message Buffer
							 Element 23 DW Register
							 */
#define XCANFD_TXFIFO_24_BASE_DW0_OFFSET	0x07c8  /**< Tx Message Buffer
							 Element 24 DW Register
							 */
#define XCANFD_TXFIFO_25_BASE_DW0_OFFSET	0x0810  /**< Tx Message Buffer
							 Element 25 DW Register
							 */
#define XCANFD_TXFIFO_26_BASE_DW0_OFFSET	0x0858  /**< Tx Message Buffer
							 Element 26 DW Register
							 */
#define XCANFD_TXFIFO_27_BASE_DW0_OFFSET	0x08a0  /**< Tx Message Buffer
							 Element 27 DW Register
							 */
#define XCANFD_TXFIFO_28_BASE_DW0_OFFSET	0x08e8  /**< Tx Message Buffer
							 Element 28 DW Register
							 */
#define XCANFD_TXFIFO_29_BASE_DW0_OFFSET	0x0930  /**< Tx Message Buffer
							 Element 29 DW Register
							 */
#define XCANFD_TXFIFO_30_BASE_DW0_OFFSET	0x0978  /**< Tx Message Buffer
							 Element 30 DW Register
							 */
#define XCANFD_TXFIFO_31_BASE_DW0_OFFSET	0x09c0  /**< Tx Message Buffer
							 Element 31 DW Register
							 */
/* @} */

/** @name Rx Message Buffer Element ID Registers.
 * Start Address - 0x1100   (2304 Bytes)
			     End Address   - 0x19FF
* @{
*/
#define XCANFD_RXFIFO_0_BASE_ID_OFFSET  0x1100  /**< Rx Message Buffer Element
							0 ID Register  */
#define XCANFD_RXFIFO_1_BASE_ID_OFFSET  0x1148  /**< Rx Message Buffer Element
							1 ID Register  */
#define XCANFD_RXFIFO_2_BASE_ID_OFFSET  0x1190  /**< Rx Message Buffer Element
							2 ID Register  */
#define XCANFD_RXFIFO_3_BASE_ID_OFFSET  0x11d8  /**< Rx Message Buffer Element
							3 ID Register  */
#define XCANFD_RXFIFO_4_BASE_ID_OFFSET  0x1220  /**< Rx Message Buffer Element
							4 ID Register  */
#define XCANFD_RXFIFO_5_BASE_ID_OFFSET  0x1268  /**< Rx Message Buffer Element
							5 ID Register  */
#define XCANFD_RXFIFO_6_BASE_ID_OFFSET  0x12b0  /**< Rx Message Buffer Element
							6 ID Register  */
#define XCANFD_RXFIFO_7_BASE_ID_OFFSET  0x12f8  /**< Rx Message Buffer Element
							7 ID Register  */
#define XCANFD_RXFIFO_8_BASE_ID_OFFSET  0x1340  /**< Rx Message Buffer Element
							8 ID Register  */
#define XCANFD_RXFIFO_9_BASE_ID_OFFSET  0x1388  /**< Rx Message Buffer Element
							9 ID Register  */
#define XCANFD_RXFIFO_10_BASE_ID_OFFSET 0x13d0  /**< Rx Message Buffer Element
							10 ID Register  */
#define XCANFD_RXFIFO_11_BASE_ID_OFFSET 0x1418  /**< Rx Message Buffer Element
							11 ID Register  */
#define XCANFD_RXFIFO_12_BASE_ID_OFFSET 0x1460  /**< Rx Message Buffer Element
							12 ID Register  */
#define XCANFD_RXFIFO_13_BASE_ID_OFFSET 0x14a8  /**< Rx Message Buffer Element
							13 ID Register  */
#define XCANFD_RXFIFO_14_BASE_ID_OFFSET 0x14f0  /**< Rx Message Buffer Element
							14 ID Register  */
#define XCANFD_RXFIFO_15_BASE_ID_OFFSET 0x1538  /**< Rx Message Buffer Element
							15 ID Register  */
#define XCANFD_RXFIFO_16_BASE_ID_OFFSET 0x1580  /**< Rx Message Buffer Element
							16 ID Register  */
#define XCANFD_RXFIFO_17_BASE_ID_OFFSET 0x15c8  /**< Rx Message Buffer Element
							17 ID Register  */
#define XCANFD_RXFIFO_18_BASE_ID_OFFSET 0x1610  /**< Rx Message Buffer Element
							18 ID Register  */
#define XCANFD_RXFIFO_19_BASE_ID_OFFSET 0x1658  /**< Rx Message Buffer Element
							19 ID Register  */
#define XCANFD_RXFIFO_20_BASE_ID_OFFSET 0x16a0  /**< Rx Message Buffer Element
							20 ID Register  */
#define XCANFD_RXFIFO_21_BASE_ID_OFFSET 0x16e8  /**< Rx Message Buffer Element
							21 ID Register  */
#define XCANFD_RXFIFO_22_BASE_ID_OFFSET 0x1730  /**< Rx Message Buffer Element
							22 ID Register  */
#define XCANFD_RXFIFO_23_BASE_ID_OFFSET 0x1778  /**< Rx Message Buffer Element
							23 ID Register  */
#define XCANFD_RXFIFO_24_BASE_ID_OFFSET 0x17c0  /**< Rx Message Buffer Element
							24 ID Register  */
#define XCANFD_RXFIFO_25_BASE_ID_OFFSET 0x1808  /**< Rx Message Buffer Element
							25 ID Register  */
#define XCANFD_RXFIFO_26_BASE_ID_OFFSET 0x1850  /**< Rx Message Buffer Element
							26 ID Register  */
#define XCANFD_RXFIFO_27_BASE_ID_OFFSET 0x1898  /**< Rx Message Buffer Element
							27 ID Register  */
#define XCANFD_RXFIFO_28_BASE_ID_OFFSET 0x18e0  /**< Rx Message Buffer Element
							28 ID Register  */
#define XCANFD_RXFIFO_29_BASE_ID_OFFSET 0x1928  /**< Rx Message Buffer Element
							29 ID Register  */
#define XCANFD_RXFIFO_30_BASE_ID_OFFSET 0x1970  /**< Rx Message Buffer Element
							30 ID Register  */
#define XCANFD_RXFIFO_31_BASE_ID_OFFSET 0x19b8  /**< Rx Message Buffer Element
							31 ID Register  */
/* @} */

/** @name Rx Message Buffer Element DLC Registers.
* @{
*/
#define XCANFD_RXFIFO_0_BASE_DLC_OFFSET 0x1104  /**< Rx Message Buffer Element
							0 DLC Register  */
#define XCANFD_RXFIFO_1_BASE_DLC_OFFSET 0x114c  /**< Rx Message Buffer Element
							1 DLC Register  */
#define XCANFD_RXFIFO_2_BASE_DLC_OFFSET 0x1194  /**< Rx Message Buffer Element
							2 DLC Register  */
#define XCANFD_RXFIFO_3_BASE_DLC_OFFSET 0x11dc  /**< Rx Message Buffer Element
							3 DLC Register  */
#define XCANFD_RXFIFO_4_BASE_DLC_OFFSET 0x1224  /**< Rx Message Buffer Element
							4 DLC Register  */
#define XCANFD_RXFIFO_5_BASE_DLC_OFFSET 0x126c  /**< Rx Message Buffer Element
							5 DLC Register  */
#define XCANFD_RXFIFO_6_BASE_DLC_OFFSET 0x12b4  /**< Rx Message Buffer Element
							6 DLC Register  */
#define XCANFD_RXFIFO_7_BASE_DLC_OFFSET 0x12fc  /**< Rx Message Buffer Element
							7 DLC Register  */
#define XCANFD_RXFIFO_8_BASE_DLC_OFFSET 0x1344  /**< Rx Message Buffer Element
							8 DLC Register  */
#define XCANFD_RXFIFO_9_BASE_DLC_OFFSET 0x138c  /**< Rx Message Buffer Element
							9 DLC Register  */
#define XCANFD_RXFIFO_10_BASE_DLC_OFFSET	0x13d4  /**< Rx Message Buffer
							Element 10 DLC Register
							*/
#define XCANFD_RXFIFO_11_BASE_DLC_OFFSET	0x141c  /**< Rx Message Buffer
							Element 11 DLC Register
							*/
#define XCANFD_RXFIFO_12_BASE_DLC_OFFSET	0x1464  /**< Rx Message Buffer
							Element 12 DLC Register
							*/
#define XCANFD_RXFIFO_13_BASE_DLC_OFFSET	0x14ac  /**< Rx Message Buffer
							Element 13 DLC Register
							*/
#define XCANFD_RXFIFO_14_BASE_DLC_OFFSET	0x14f4  /**< Rx Message Buffer
							Element 14 DLC Register
							*/
#define XCANFD_RXFIFO_15_BASE_DLC_OFFSET	0x153c  /**< Rx Message Buffer
							Element 15 DLC Register
							*/
#define XCANFD_RXFIFO_16_BASE_DLC_OFFSET	0x1584  /**< Rx Message Buffer
							Element 16 DLC Register
							*/
#define XCANFD_RXFIFO_17_BASE_DLC_OFFSET	0x15cc  /**< Rx Message Buffer
							Element 17 DLC Register
							*/
#define XCANFD_RXFIFO_18_BASE_DLC_OFFSET	0x1614  /**< Rx Message Buffer
							Element 18 DLC Register
							*/
#define XCANFD_RXFIFO_19_BASE_DLC_OFFSET	0x165c  /**< Rx Message Buffer
							Element 19 DLC Register
							*/
#define XCANFD_RXFIFO_20_BASE_DLC_OFFSET	0x16a4  /**< Rx Message Buffer
							Element 20 DLC Register
							  */
#define XCANFD_RXFIFO_21_BASE_DLC_OFFSET	0x16ec  /**< Rx Message Buffer
							Element 21 DLC Register
							*/
#define XCANFD_RXFIFO_22_BASE_DLC_OFFSET	0x1734  /**< Rx Message Buffer
							Element 22 DLC Register
							*/
#define XCANFD_RXFIFO_23_BASE_DLC_OFFSET	0x177c  /**< Rx Message Buffer
							Element 23 DLC Register
							*/
#define XCANFD_RXFIFO_24_BASE_DLC_OFFSET	0x17c4  /**< Rx Message Buffer
							Element 24 DLC Register
							*/
#define XCANFD_RXFIFO_25_BASE_DLC_OFFSET	0x180c  /**< Rx Message Buffer
							Element 25 DLC Register
							*/
#define XCANFD_RXFIFO_26_BASE_DLC_OFFSET	0x1854  /**< Rx Message Buffer
							Element 26 DLC Register
							*/
#define XCANFD_RXFIFO_27_BASE_DLC_OFFSET	0x189c  /**< Rx Message Buffer
							Element 27 DLC Register
							*/
#define XCANFD_RXFIFO_28_BASE_DLC_OFFSET	0x18e4  /**< Rx Message Buffer
							Element 28 DLC Register
							*/
#define XCANFD_RXFIFO_29_BASE_DLC_OFFSET	0x192c  /**< Rx Message Buffer
							Element 29 DLC Register
							*/
#define XCANFD_RXFIFO_30_BASE_DLC_OFFSET	0x1974  /**< Rx Message Buffer
							Element 30 DLC Register
							*/
#define XCANFD_RXFIFO_31_BASE_DLC_OFFSET	0x19bc  /**< Rx Message Buffer
							Element 31 DLC Register
							*/
/* @} */

/** @name Rx Message Buffer Element DW Registers.
* @{
*/
#define XCANFD_RXFIFO_0_BASE_DW0_OFFSET 0x1108 /**< Rx Message Buffer Element
							0 DW Register  */
#define XCANFD_RXFIFO_1_BASE_DW0_OFFSET 0x1150 /**< Rx Message Buffer Element
							1 DW Register  */
#define XCANFD_RXFIFO_2_BASE_DW0_OFFSET 0x1198 /**< Rx Message Buffer Element
							2 DW Register  */
#define XCANFD_RXFIFO_3_BASE_DW0_OFFSET 0x11e0 /**< Rx Message Buffer Element
							3 DW Register  */
#define XCANFD_RXFIFO_4_BASE_DW0_OFFSET 0x1228 /**< Rx Message Buffer Element
							4 DW Register  */
#define XCANFD_RXFIFO_5_BASE_DW0_OFFSET 0x1270 /**< Rx Message Buffer Element
							5 DW Register  */
#define XCANFD_RXFIFO_6_BASE_DW0_OFFSET 0x12b8 /**< Rx Message Buffer Element
							6 DW Register  */
#define XCANFD_RXFIFO_7_BASE_DW0_OFFSET 0x1300 /**< Rx Message Buffer Element
							7 DW Register  */
#define XCANFD_RXFIFO_8_BASE_DW0_OFFSET 0x1348 /**< Rx Message Buffer Element
							8 DW Register  */
#define XCANFD_RXFIFO_9_BASE_DW0_OFFSET 0x1390 /**< Rx Message Buffer Element
							9 DW Register  */
#define XCANFD_RXFIFO_10_BASE_DW0_OFFSET	0x13d8 /**< Rx Message Buffer
							Element 10 DW Register
							*/
#define XCANFD_RXFIFO_11_BASE_DW0_OFFSET	0x1420 /**< Rx Message Buffer
							Element 11 DW Register
							*/
#define XCANFD_RXFIFO_12_BASE_DW0_OFFSET	0x1468 /**< Rx Message Buffer
							Element 12 DW Register
							*/
#define XCANFD_RXFIFO_13_BASE_DW0_OFFSET	0x14b0 /**< Rx Message Buffer
							Element 13 DW Register
							*/
#define XCANFD_RXFIFO_14_BASE_DW0_OFFSET	0x14f8 /**< Rx Message Buffer
							Element 14 DW Register
							*/
#define XCANFD_RXFIFO_15_BASE_DW0_OFFSET	0x1540 /**< Rx Message Buffer
							Element 15 DW Register
							*/
#define XCANFD_RXFIFO_16_BASE_DW0_OFFSET	0x1588 /**< Rx Message Buffer
							Element 16 DW Register
							*/
#define XCANFD_RXFIFO_17_BASE_DW0_OFFSET	0x15d0 /**< Rx Message Buffer
							Element 17 DW Register
							*/
#define XCANFD_RXFIFO_18_BASE_DW0_OFFSET	0x1618 /**< Rx Message Buffer
							Element 18 DW Register
							*/
#define XCANFD_RXFIFO_19_BASE_DW0_OFFSET	0x1660 /**< Rx Message Buffer
							Element 19 DW Register
							*/
#define XCANFD_RXFIFO_20_BASE_DW0_OFFSET	0x16a8 /**< Rx Message Buffer
							Element 20 DW Register
							*/
#define XCANFD_RXFIFO_21_BASE_DW0_OFFSET	0x16f0 /**< Rx Message Buffer
							Element 21 DW Register
							*/
#define XCANFD_RXFIFO_22_BASE_DW0_OFFSET	0x1738 /**< Rx Message Buffer
							Element 22 DW Register
							*/
#define XCANFD_RXFIFO_23_BASE_DW0_OFFSET	0x1780 /**< Rx Message Buffer
							Element 23 DW Register
							*/
#define XCANFD_RXFIFO_24_BASE_DW0_OFFSET	0x17c8 /**< Rx Message Buffer
							Element 24 DW Register
							*/
#define XCANFD_RXFIFO_25_BASE_DW0_OFFSET	0x1810 /**< Rx Message Buffer
							Element 25 DW Register
							*/
#define XCANFD_RXFIFO_26_BASE_DW0_OFFSET	0x1858 /**< Rx Message Buffer
							Element 26 DW Register
							*/
#define XCANFD_RXFIFO_27_BASE_DW0_OFFSET	0x18a0 /**< Rx Message Buffer
							Element 27 DW Register
							*/
#define XCANFD_RXFIFO_28_BASE_DW0_OFFSET	0x18e8 /**< Rx Message Buffer
							Element 28 DW Register
							*/
#define XCANFD_RXFIFO_29_BASE_DW0_OFFSET	0x1930 /**< Rx Message Buffer
							Element 29 DW Register
							*/
#define XCANFD_RXFIFO_30_BASE_DW0_OFFSET	0x1978 /**< Rx Message Buffer
							Element 30 DW Register
							*/
#define XCANFD_RXFIFO_31_BASE_DW0_OFFSET	0x19c0 /**< Rx Message Buffer
							Element 31 DW Register
							*/
/* @} */

/** @name Rx Message Buffer Element ID,DLC,DW Sizes.
* @{
*/
#define XCANFD_RXFIFO_NEXTID_OFFSET	72	/**< Rx Message Buffer Element
						Next ID AT Offset */
#define XCANFD_RXFIFO_NEXTDLC_OFFSET	72	/**< Rx Message Buffer Element
						 Next DLC AT Offset */
#define XCANFD_RXFIFO_NEXTDW_OFFSET	72	/**< Rx Message Buffer Element
						 Next DW AT Offset */
/* @} */

/** @name EDL and BRS Masks.
* @{
*/
#define XCANFD_DLCR_EDL_MASK	0x08000000  /**< EDL Mask in DLC Register */
#define XCANFD_DLCR_BRS_MASK	0x04000000  /**< BRS Mask in DLC Register */
/* @} */

/** @name Acceptance Filter Mask Registers
 *  @{
 */
#define XCANFD_AFMR_BASE_OFFSET	0x1A00  /**<Acceptance Filter Mask Register */
#define XCANFD_AFMR_NXT_OFFSET	8
#define XCANFD_AFIDR_BASE_OFFSET 0x1A04  /**< Acceptance Filter ID Register */
#define XCANFD_AFIDR_NXT_OFFSET	 8
#define XCANFD_AFR_OFFSET	0x0E0  /**< Acceptance Filter Register */
#define XCANFD_FSR_OFFSET	0x0E8  /**< Receive FIFO Status Register */
#define XCANFD_NOOF_AFR		32	/**< Number Of Acceptance FIlter
						Registers */
#define XCANFD_WIR_OFFSET	0x0EC  /**< Rx FIFO Water Mark Register */
#define XCANFD_WIR_MASK	0x0000001F  /**< Rx FIFO Full watermark Mask */
#define XCANFD_TIMESTAMPR_OFFSET	0x0028	/**< Time Stamp Register */
#define XCANFD_CTS_MASK		0x00000001 /**< Time Stamp Counter Clear */
#define XCANFD_DAR_MASK		0x00000010/**< Disable AutoRetransmission */
/* @} */

/** @name Software Reset Register
 *  @{
 */
#define XCANFD_SRR_CEN_MASK	0x00000002  /**< Can Enable Mask */
#define XCANFD_SRR_SRST_MASK	0x00000001  /**< Reset Mask */
/* @} */

/** @name Mode Select Register
 *  @{
 */
#define XCANFD_MSR_LBACK_MASK	0x00000002  /**< Loop Back Mode Select Mask */
#define XCANFD_MSR_SLEEP_MASK	0x00000001  /**< Sleep Mode Select Mask */
#define XCANFD_MSR_BRSD_MASK	0x00000008  /**< Bit Rate Switch Select Mask */
#define XCANFD_MSR_DAR_MASK	0x00000010  /**< Disable Auto-Retransmission
						 Select Mask */
#define XCANFD_MSR_SNOOP_MASK	0x00000004  /**< Snoop Mode Select Mask */
#define XCANFD_MSR_DPEE_MASK	0x00000020  /**< Protocol Exception Event
						 Mask */
#define XCANFD_MSR_SBR_MASK	0x00000040  /**< Start Bus-Off Recovery Mask */
#define XCANFD_MSR_ABR_MASK     0x00000080  /**< Auto Bus-Off Recovery Mask */
#define XCANFD_MSR_CONFIG_MASK	0x000000F8  /**< Configuration Mode Mask */
/* @} */

/** @name Baud Rate Prescaler register
 *  @{
 */
#define XCANFD_BRPR_BRP_MASK	0x000000FF  /**< Baud Rate Prescaler Mask */
/* @} */

/** @name Bit Timing Register
 *  @{
 */
#define XCANFD_BTR_SJW_MASK	0x000F0000  /**< Sync Jump Width Mask */
#define XCANFD_BTR_SJW_SHIFT	16	    /**< Sync Jump Width Shift */
#define XCANFD_BTR_TS2_MASK	0x00000F00  /**< Time Segment 2 Mask */
#define XCANFD_BTR_TS2_SHIFT	8	    /**< Time Segment 2 Shift */
#define XCANFD_BTR_TS1_MASK	0x0000003F  /**< Time Segment 1 Mask */
#define XCANFD_F_BRPR_TDCMASK	0x00001F00	/**< Tranceiver Delay
						compensation Offset Mask */
#define XCANFD_F_BRPR_TDC_ENABLE_MASK	0x00010000	/**< Tranceiver Delay
							compensation Enable
							Maskk */
/* @} */

/** @name Fast Bit Timing Register
 *  @{
 */
#define XCANFD_F_BTR_SJW_MASK	0x00070000  /**< Sync Jump Width Mask */
#define XCANFD_F_BTR_SJW_SHIFT	16	    /**< Sync Jump Width Shift */
#define XCANFD_F_BTR_TS2_MASK	0x00000700  /**< Time Segment 2 Mask */
#define XCANFD_F_BTR_TS2_SHIFT	8	    /**< Time Segment 2 Shift */
#define XCANFD_F_BTR_TS1_MASK	0x0000000F  /**< Time Segment 1 Mask */
/* @} */

/** @name Error Counter Register
 *  @{
 */
#define XCANFD_ECR_REC_MASK	0x0000FF00  /**< Receive Error Counter Mask */
#define XCANFD_ECR_REC_SHIFT	8	    /**< Receive Error Counter Shift */
#define XCANFD_ECR_TEC_MASK	0x000000FF  /**< Transmit Error Counter Mask */
/* @} */

/** @name Error Status Register
 *  @{
 */
#define XCANFD_ESR_ACKER_MASK	0x00000010  /**< ACK Error Mask */
#define XCANFD_ESR_BERR_MASK	0x00000008  /**< Bit Error Mask */
#define XCANFD_ESR_STER_MASK	0x00000004  /**< Stuff Error Mask */
#define XCANFD_ESR_FMER_MASK	0x00000002  /**< Form Error Mask */
#define XCANFD_ESR_CRCER_MASK	0x00000001  /**< CRC Error Mask */
#define XCANFD_ESR_F_BERR_MASK	0x00000800  /**< F_Bit Error Mask */
#define XCANFD_ESR_F_STER_MASK	0x00000400  /**< F_Stuff Error Mask */
#define XCANFD_ESR_F_FMER_MASK	0x00000200  /**< F_Form Error Mask */
#define XCANFD_ESR_F_CRCER_MASK	0x00000100  /**< F_CRC Error Mask */
/* @} */

/** @name Status Register
 *  @{
 */
#define XCANFD_SR_TDCV_MASK	0x007F0000  /**< Tranceiver Dealy compensation
						Mask */
#define XCANFD_SR_SNOOP_MASK	0x00001000  /**< Snoop Mode Mask */
#define XCANFD_SR_ESTAT_MASK	0x00000180  /**< Error Status Mask */
#define XCANFD_SR_ESTAT_SHIFT	7	    /**< Error Status Shift */
#define XCANFD_SR_ERRWRN_MASK	0x00000040  /**< Error Warning Mask */
#define XCANFD_SR_BBSY_MASK	0x00000020  /**< Bus Busy Mask */
#define XCANFD_SR_BIDLE_MASK	0x00000010  /**< Bus Idle Mask */
#define XCANFD_SR_NORMAL_MASK	0x00000008  /**< Normal Mode Mask */
#define XCANFD_SR_SLEEP_MASK	0x00000004  /**< Sleep Mode Mask */
#define XCANFD_SR_LBACK_MASK	0x00000002  /**< Loop Back Mode Mask */
#define XCANFD_SR_CONFIG_MASK	0x00000001  /**< Configuration Mode Mask */
#define XCANFD_SR_PEE_CONFIG_MASK 0x00000200 /**< Protocol Exception Mode
						  Indicator Mask */
#define XCANFD_SR_BSFR_CONFIG_MASK 0x00000400 /**< Bus-Off recovery Mode
						   Indicator Mask */
#define XCANFD_SR_NISO_MASK	0x00000800 /**< Non-ISO Core Mask */
/* @} */

/** @name Interrupt Status/Enable/Clear Register
 *  @{
 */
#define XCANFD_IXR_RXBOFLW_BI_MASK 0x3F000000 /**< Rx Buffer index for Overflow
						   (Mailbox Mode) */
#define XCANFD_IXR_RXLRM_BI_MASK   0x00FC0000 /**< Rx Buffer index for Last
						   Received Message (Mailbox
						    Mode) */
#define XCANFD_RXLRM_BI_SHIFT	18	/**< Rx Buffer Index Shift Value */
#define XCANFD_CSB_SHIFT	16	/**< Core Status Bit Shift Value */
#define XCANFD_IXR_RXMNF_MASK	 0x00020000 /**< Rx Match Not Finished Intr
						Mask */
#define XCANFD_IXR_RXBOFLW_MASK	 0x00010000 /**< Rx Buffer Overflow interrupt
						Mask (Mailbox mode) */
#define XCANFD_IXR_RXRBF_MASK	 0x00008000 /**< Rx Buffer Full Interrupt Mask
						(Mailbox mode) */
#define XCANFD_IXR_TXCRS_MASK	 0x00004000 /**< Tx Cancellation Request Served
						 Interrupt Mask */
#define XCANFD_IXR_TXRRS_MASK	 0x00002000 /**< Tx Buffer Ready Request Served
						 Interrupt Mask */
#define XCANFD_IXR_RXFWMFLL_MASK	 0x00001000 /**< Rx  Watermark Full
							interrupt
							 Mask (FIFO mode) */
#define XCANFD_IXR_WKUP_MASK	0x00000800  /**< Wake up Interrupt Mask */
#define XCANFD_IXR_SLP_MASK	0x00000400  /**< Sleep Interrupt Mask */
#define XCANFD_IXR_BSOFF_MASK	0x00000200  /**< Bus Off Interrupt Mask */
#define XCANFD_IXR_ERROR_MASK	0x00000100  /**< Error Interrupt Mask */
#define XCANFD_IXR_RXFOFLW_MASK	0x00000040  /**< RX FIFO Overflow Intr Mask */
#define XCANFD_IXR_RXOK_MASK	0x00000010  /**< New Message Received Intr */
#define XCANFD_IXR_TXOK_MASK	0x00000002  /**< TX Successful Interrupt Mask
						*/
#define XCANFD_IXR_ARBLST_MASK	0x00000001  /**< Arbitration Lost Intr Mask */
#define XCANFD_IXR_PEE_MASK	0x00000004  /**< Protocol Exception Intr Mask */
#define XCANFD_IXR_BSRD_MASK	0x00000008  /**< Bus-Off recovery done Intr
						 Mask */
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
/* @} */

/** @name Transmit Ready request All Mask
 *  @{
 */
#define XCANFD_TRR_MASK 0xFFFFFFFF
/* @} */

/** @name CAN Frame Identifier (TX High Priority Buffer/TX/RX/Acceptance Filter
Mask/Acceptance Filter ID)
 *  @{
 */
#define XCANFD_IDR_ID1_MASK	0xFFE00000  /**< Standard Messg Ident Mask */
#define XCANFD_IDR_ID1_SHIFT	21	    /**< Standard Messg Ident Shift */
#define XCANFD_IDR_SRR_MASK	0x00100000  /**< Substitute Remote TX Req */
#define XCANFD_IDR_SRR_SHIFT	20
#define XCANFD_IDR_IDE_MASK	0x00080000  /**< Identifier Extension Mask */
#define XCANFD_IDR_IDE_SHIFT	19	    /**< Identifier Extension Shift */
#define XCANFD_IDR_ID2_MASK	0x0007FFFE  /**< Extended Message Ident Mask */
#define XCANFD_IDR_ID2_SHIFT	1	    /**< Extended Message Ident Shift
						*/
#define XCANFD_IDR_RTR_MASK	0x00000001  /**< Remote TX Request Mask */
/* @} */

/** @name CAN Frame Data Length Code (TX High Priority Buffer/TX/RX)
 *  @{
 */
#define XCANFD_DLCR_DLC_MASK	0xF0000000  /**< Data Length Code Mask */
#define XCANFD_DLCR_DLC_SHIFT	28  	    /**< Data Length Code Shift */
#define XCANFD_DLC1 0x10000000	/**< Data Length Code 1 */
#define XCANFD_DLC2 0x20000000	/**< Data Length Code 2 */
#define XCANFD_DLC3 0x30000000	/**< Data Length Code 3 */
#define XCANFD_DLC4 0x40000000	/**< Data Length Code 4 */
#define XCANFD_DLC5 0x50000000	/**< Data Length Code 5 */
#define XCANFD_DLC6 0x60000000	/**< Data Length Code 6 */
#define XCANFD_DLC7 0x70000000	/**< Data Length Code 7 */
#define XCANFD_DLC8 0x80000000	/**< Data Length Code 8 */
#define XCANFD_DLC9 0x90000000	/**< Data Length Code 9 */
#define XCANFD_DLC10 0xA0000000	/**< Data Length Code 10 */
#define XCANFD_DLC11 0xB0000000	/**< Data Length Code 11 */
#define XCANFD_DLC12 0xC0000000	/**< Data Length Code 12 */
#define XCANFD_DLC13 0xD0000000	/**< Data Length Code 13 */
#define XCANFD_DLC14 0xE0000000	/**< Data Length Code 14 */
#define XCANFD_DLC15 0xF0000000	/**< Data Length Code 15 */
/* @} */

/** @name Acceptance Filter Register
 *  @{
 */
#define XCANFD_AFR_UAF_ALL_MASK	0xFFFFFFFF
/* @} */

/** @name CAN Recieve FIFO Status Register
 *  @{
 */
#define XCANFD_FSR_FL_MASK	0x00003F00  /**< Fill Level Mask */
#define XCANFD_FSR_RI_MASK	0x0000001F  /**< Read Index Mask */
#define XCANFD_FSR_IRI_MASK	0x00000080  /**< Increment Read Index Mask */
/* @} */

/** @name CAN TxBuffer Ready Request Served Interrupt Enable Register Masks
 *  @{
 */
#define XCANFD_TXBUFFER0_RDY_RQT_MASK 0x00000001	/**< TxBuffer0 Ready
							Request Mask */
#define XCANFD_TXBUFFER1_RDY_RQT_MASK	0x00000002	/**< TxBuffer1 Ready
							Request Mask */
#define XCANFD_TXBUFFER2_RDY_RQT_MASK	0x00000004	/**< TxBuffer2 Ready
							Request Mask */
#define XCANFD_TXBUFFER3_RDY_RQT_MASK	0x00000008	/**< TxBuffer3 Ready
							Request Mask */
#define XCANFD_TXBUFFER4_RDY_RQT_MASK	0x00000010	/**< TxBuffer4 Ready
							Request Mask */
#define XCANFD_TXBUFFER5_RDY_RQT_MASK	0x00000020	/**< TxBuffer5 Ready
							Request Mask */
#define XCANFD_TXBUFFER6_RDY_RQT_MASK	0x00000040	/**< TxBuffer6 Ready
							Request Mask */
#define XCANFD_TXBUFFER7_RDY_RQT_MASK	0x00000080	/**< TxBuffer7 Ready
							Request Mask */
#define XCANFD_TXBUFFER8_RDY_RQT_MASK	0x00000100	/**< TxBuffer8 Ready
							Request Mask */
#define XCANFD_TXBUFFER9_RDY_RQT_MASK	0x00000200	/**< TxBuffer9 Ready
							Request Mask */
#define XCANFD_TXBUFFER10_RDY_RQT_MASK	0x00000400	/**< TxBuffer10 Ready
							Request Mask */
#define XCANFD_TXBUFFER11_RDY_RQT_MASK	0x00000800	/**< TxBuffer11 Ready
							Request Mask */
#define XCANFD_TXBUFFER12_RDY_RQT_MASK	0x00001000	/**< TxBuffer12 Ready
							Request Mask */
#define XCANFD_TXBUFFER13_RDY_RQT_MASK	0x00002000	/**< TxBuffer13 Ready
							Request Mask */
#define XCANFD_TXBUFFER14_RDY_RQT_MASK	0x00004000	/**< TxBuffer14 Ready
							Request Mask */
#define XCANFD_TXBUFFER15_RDY_RQT_MASK	0x00008000	/**< TxBuffer15 Ready
							Request Mask */
#define XCANFD_TXBUFFER16_RDY_RQT_MASK	0x00010000	/**< TxBuffer16 Ready
							Request Mask */
#define XCANFD_TXBUFFER17_RDY_RQT_MASK	0x00020000	/**< TxBuffer17 Ready
							Request Mask */
#define XCANFD_TXBUFFER18_RDY_RQT_MASK	0x00040000	/**< TxBuffer18 Ready
							Request Mask */
#define XCANFD_TXBUFFER19_RDY_RQT_MASK	0x00080000	/**< TxBuffer19 Ready
							Request Mask */
#define XCANFD_TXBUFFER20_RDY_RQT_MASK	0x00100000	/**< TxBuffer20 Ready
							Request Mask */
#define XCANFD_TXBUFFER21_RDY_RQT_MASK	0x00200000	/**< TxBuffer21 Ready
							Request Mask */
#define XCANFD_TXBUFFER22_RDY_RQT_MASK	0x00400000	/**< TxBuffer22 Ready
							Request Mask */
#define XCANFD_TXBUFFER23_RDY_RQT_MASK	0x00800000	/**< TxBuffer23 Ready
							Request Mask */
#define XCANFD_TXBUFFER24_RDY_RQT_MASK	0x01000000	/**< TxBuffer24 Ready
							Request Mask */
#define XCANFD_TXBUFFER25_RDY_RQT_MASK	0x02000000	/**< TxBuffer25 Ready
							Request Mask */
#define XCANFD_TXBUFFER26_RDY_RQT_MASK	0x04000000	/**< TxBuffer26 Ready
							Request Mask */
#define XCANFD_TXBUFFER27_RDY_RQT_MASK	0x08000000	/**< TxBuffer27 Ready
							Request Mask */
#define XCANFD_TXBUFFER28_RDY_RQT_MASK	0x10000000	/**< TxBuffer28 Ready
							Request Mask */
#define XCANFD_TXBUFFER29_RDY_RQT_MASK	0x20000000	/**< TxBuffer29 Ready
							Request Mask */
#define XCANFD_TXBUFFER30_RDY_RQT_MASK	0x40000000	/**< TxBuffer30 Ready
							Request Mask */
#define XCANFD_TXBUFFER31_RDY_RQT_MASK	0x80000000	/**< TxBuffer31 Ready
							Request Mask */
#define XCANFD_TXBUFFER_ALL_RDY_RQT_MASK	0xFFFFFFFF
/* @} */

/** @name CAN TxBuffer Cancel Request Served Interrupt Enable Register Masks
 *  @{
 */
#define XCANFD_TXBUFFER0_CANCEL_RQT_MASK 0x00000001	/**< TxBuffer0 Cancel
								Request Mask */
#define XCANFD_TXBUFFER1_CANCEL_RQT_MASK	0x00000002	/**< TxBuffer1
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER2_CANCEL_RQT_MASK	0x00000004	/**< TxBuffer2
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER3_CANCEL_RQT_MASK	0x00000008	/**< TxBuffer3
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER4_CANCEL_RQT_MASK	0x00000010	/**< TxBuffer4
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER5_CANCEL_RQT_MASK	0x00000020	/**< TxBuffer5
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER6_CANCEL_RQT_MASK	0x00000040	/**< TxBuffer6
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER7_CANCEL_RQT_MASK	0x00000080	/**< TxBuffer7
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER8_CANCEL_RQT_MASK	0x00000100	/**< TxBuffer8
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER9_CANCEL_RQT_MASK	0x00000200	/**< TxBuffer9
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER10_CANCEL_RQT_MASK	0x00000400	/**< TxBuffer10
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER11_CANCEL_RQT_MASK	0x00000800	/**< TxBuffer11
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER12_CANCEL_RQT_MASK	0x00001000	/**< TxBuffer12
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER13_CANCEL_RQT_MASK	0x00002000	/**< TxBuffer13
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER14_CANCEL_RQT_MASK	0x00004000	/**< TxBuffer14
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER15_CANCEL_RQT_MASK	0x00008000	/**< TxBuffer15
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER16_CANCEL_RQT_MASK	0x00010000	/**< TxBuffer16
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER17_CANCEL_RQT_MASK	0x00020000	/**< TxBuffer17
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER18_CANCEL_RQT_MASK	0x00040000	/**< TxBuffer18
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER19_CANCEL_RQT_MASK	0x00080000	/**< TxBuffer19
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER20_CANCEL_RQT_MASK	0x00100000	/**< TxBuffer20
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER21_CANCEL_RQT_MASK	0x00200000	/**< TxBuffer21
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER22_CANCEL_RQT_MASK	0x00400000	/**< TxBuffer22
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER23_CANCEL_RQT_MASK	0x00800000	/**< TxBuffer23
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER24_CANCEL_RQT_MASK	0x01000000	/**< TxBuffer24
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER25_CANCEL_RQT_MASK	0x02000000	/**< TxBuffer25
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER26_CANCEL_RQT_MASK	0x04000000	/**< TxBuffer26
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER27_CANCEL_RQT_MASK	0x08000000	/**< TxBuffer27
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER28_CANCEL_RQT_MASK	0x10000000	/**< TxBuffer28
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER29_CANCEL_RQT_MASK	0x20000000	/**< TxBuffer29
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER30_CANCEL_RQT_MASK	0x40000000	/**< TxBuffer30
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER31_CANCEL_RQT_MASK	0x80000000	/**< TxBuffer31
								Cancel 	Request
								Mask */
#define XCANFD_TXBUFFER_CANCEL_RQT_ALL_MASK 0xFFFFFFFF
/* @} */

/** @name CAN RxBuffer Full Register
 *  @{
 */
#define XCANFD_RXBUFFER0_FULL_MASK	0x00000001	/**< RxBuffer0 Full
								Mask */
#define XCANFD_RXBUFFER1_FULL_MASK	0x00000002	/**< RxBuffer1 Full
								Mask */
#define XCANFD_RXBUFFER2_FULL_MASK	0x00000004	/**< RxBuffer2 Full
								Mask */
#define XCANFD_RXBUFFER3_FULL_MASK	0x00000008	/**< RxBuffer3 Full
								Mask */
#define XCANFD_RXBUFFER4_FULL_MASK	0x00000010	/**< RxBuffer4 Full
								Mask */
#define XCANFD_RXBUFFER5_FULL_MASK	0x00000020	/**< RxBuffer5 Full
								Mask */
#define XCANFD_RXBUFFER6_FULL_MASK	0x00000040	/**< RxBuffer6 Full
								Mask */
#define XCANFD_RXBUFFER7_FULL_MASK	0x00000080	/**< RxBuffer7 Full
								Mask */
#define XCANFD_RXBUFFER8_FULL_MASK	0x00000100	/**< RxBuffer8 Full
								Mask */
#define XCANFD_RXBUFFER9_FULL_MASK	0x00000200	/**< RxBuffer9 Full
								Mask */
#define XCANFD_RXBUFFER10_FULL_MASK	0x00000400	/**< RxBuffer10 Full
								Mask */
#define XCANFD_RXBUFFER11_FULL_MASK	0x00000800	/**< RxBuffer11 Full
								Mask */
#define XCANFD_RXBUFFER12_FULL_MASK	0x00001000	/**< RxBuffer12 Full
								Mask */
#define XCANFD_RXBUFFER13_FULL_MASK	0x00002000	/**< RxBuffer13 Full
								Mask */
#define XCANFD_RXBUFFER14_FULL_MASK	0x00004000	/**< RxBuffer14 Full
								Mask */
#define XCANFD_RXBUFFER15_FULL_MASK	0x00008000	/**< RxBuffer15 Full
								Mask */
#define XCANFD_RXBUFFER16_FULL_MASK	0x00010000	/**< RxBuffer16 Full
								Mask */
#define XCANFD_RXBUFFER17_FULL_MASK	0x00020000	/**< RxBuffer17 Full
								Mask */
#define XCANFD_RXBUFFER18_FULL_MASK	0x00040000	/**< RxBuffer18 Full
								Mask */
#define XCANFD_RXBUFFER19_FULL_MASK	0x00080000	/**< RxBuffer19 Full
								Mask */
#define XCANFD_RXBUFFER20_FULL_MASK	0x00100000	/**< RxBuffer20 Full
								Mask */
#define XCANFD_RXBUFFER21_FULL_MASK	0x00200000	/**< RxBuffer21 Full
								Mask */
#define XCANFD_RXBUFFER22_FULL_MASK	0x00400000	/**< RxBuffer22 Full
								Mask */
#define XCANFD_RXBUFFER23_FULL_MASK	0x00800000	/**< RxBuffer23 Full
								Mask */
#define XCANFD_RXBUFFER24_FULL_MASK	0x01000000	/**< RxBuffer24 Full
								Mask */
#define XCANFD_RXBUFFER25_FULL_MASK	0x02000000	/**< RxBuffer25 Full
								Mask */
#define XCANFD_RXBUFFER26_FULL_MASK	0x04000000	/**< RxBuffer26 Full
								Mask */
#define XCANFD_RXBUFFER27_FULL_MASK	0x08000000	/**< RxBuffer27 Full
								Mask */
#define XCANFD_RXBUFFER28_FULL_MASK	0x10000000	/**< RxBuffer28 Full
								Mask */
#define XCANFD_RXBUFFER29_FULL_MASK	0x20000000	/**< RxBuffer29 Full
								Mask */
#define XCANFD_RXBUFFER30_FULL_MASK	0x40000000	/**< RxBuffer30 Full
								Mask */
#define XCANFD_RXBUFFER31_FULL_MASK	0x80000000	/**< RxBuffer31 Full
								Mask */
#define XCANFD_RXBUFFER32_FULL_MASK	0x00000001	/**< RxBuffer32 Full
								Mask */
#define XCANFD_RXBUFFER33_FULL_MASK	0x00000002	/**< RxBuffer33 Full
								Mask */
#define XCANFD_RXBUFFER34_FULL_MASK	0x00000004	/**< RxBuffer34 Full
								Mask */
#define XCANFD_RXBUFFER35_FULL_MASK	0x00000008	/**< RxBuffer35 Full
								Mask */
#define XCANFD_RXBUFFER36_FULL_MASK	0x00000010	/**< RxBuffer36 Full
								Mask */
#define XCANFD_RXBUFFER37_FULL_MASK	0x00000020	/**< RxBuffer37 Full
								Mask */
#define XCANFD_RXBUFFER38_FULL_MASK	0x00000040	/**< RxBuffer38 Full
								Mask */
#define XCANFD_RXBUFFER39_FULL_MASK	0x00000080	/**< RxBuffer39 Full
								Mask */
#define XCANFD_RXBUFFER40_FULL_MASK	0x00000100	/**< RxBuffer40 Full
								Mask */
#define XCANFD_RXBUFFER41_FULL_MASK	0x00000200	/**< RxBuffer41 Full
								Mask */
#define XCANFD_RXBUFFER42_FULL_MASK	0x00000400	/**< RxBuffer42 Full
								Mask */
#define XCANFD_RXBUFFER43_FULL_MASK	0x00000800	/**< RxBuffer43 Full
								Mask */
#define XCANFD_RXBUFFER44_FULL_MASK	0x00001000	/**< RxBuffer44 Full
								Mask */
#define XCANFD_RXBUFFER45_FULL_MASK	0x00002000	/**< RxBuffer45 Full
								Mask */
#define XCANFD_RXBUFFER46_FULL_MASK	0x00004000	/**< RxBuffer46 Full
								Mask */
#define XCANFD_RXBUFFER47_FULL_MASK	0x00008000	/**< RxBuffer47 Full
								Mask */
/* @} */

/** @name CAN frame length constants
 *  @{
 */
#define XCANFD_MAX_FRAME_SIZE 72	/**< Maximum CAN frame length in bytes
					 */
#define XCANFD_DW_BYTES	4		/**< Data Word Bytes */
#define XST_NOBUFFER	33L	/**< All Buffers (32) are filled */
#define XST_BUFFER_ALREADY_FILLED	34L	/**< Given Buffer is Already
						filled */
#define XST_INVALID_DLC	16L	/**< Invalid Dlc code */
/* @} */

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
