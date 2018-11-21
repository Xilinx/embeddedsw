/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* @file xuartsbsa_hw.h
* @addtogroup uartsbsa_v1_0
* @{
*
* This header file contains the hardware interface of an XUartSbsa device.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who  Date      Changes
* ---  ---  --------- -----------------------------------------------
* 1.0  sg   09/18/17  First Release
*
* </pre>
*
******************************************************************************/
#ifndef XUARTSBSA_HW_H		/* prevent circular inclusions */
#define XUARTSBSA_HW_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register Map
 *
 * Register offsets for the UART.
 * @{
 */
#define XUARTSBSA_UARTDR_OFFSET 	0x0000U	/**< Data */
#define XUARTSBSA_UARTRSR_OFFSET	0x0004U
									/**< Receive Status Register/Error Clear */
#define XUARTSBSA_UARTFR_OFFSET 	0x0018U	/**< Flag Register */
#define XUARTSBSA_UARTILPR_OFFSET	0x0020U	/**< IrDA Low-Power Counter */
#define XUARTSBSA_UARTIBRD_OFFSET	0x0024U	/**< Integer Baud Rate */
#define XUARTSBSA_UARTFBRD_OFFSET	0x0028U	/**< Fractional Baud Rate */
#define XUARTSBSA_UARTLCR_OFFSET	0x002CU	/**< Line Control */
#define XUARTSBSA_UARTCR_OFFSET 	0x0030U	/**< Control */
#define XUARTSBSA_UARTIFLS_OFFSET	0x0034U	/**< Interrupt FIFO Level Select */
#define XUARTSBSA_UARTIMSC_OFFSET	0x0038U	/**< Interrupt Mask Set/Clear */
#define XUARTSBSA_UARTRIS_OFFSET	0x003CU	/**< Raw interrupt Status */
#define XUARTSBSA_UARTMIS_OFFSET	0x0040U	/**< Mask interrupt Status */
#define XUARTSBSA_UARTICR_OFFSET	0x0044U	/**< Interrupt Clear */
#define XUARTSBSA_UARTDMACR_OFFSET	0x0048U	/**< DMA Control */
/* @} */


/** @name Flag Register
 *
 * The Flag register (UARTFR)
 *
 * Flag Register Bit Definition
 * @{
 */
#define XUARTSBSA_UARTFR_RI 	0x00000100U	/**< Ring indicator */
#define XUARTSBSA_UARTFR_TXFE	0x00000080U	/**< Transmit FIFO empty */
#define XUARTSBSA_UARTFR_RXFF	0x00000040U	/**< Receive FIFO full */
#define XUARTSBSA_UARTFR_TXFF	0x00000020U	/**< Transmit FIFO full */
#define XUARTSBSA_UARTFR_RXFE	0x00000010U	/**< Receive FIFO empty */
#define XUARTSBSA_UARTFR_BUSY	0x00000008U	/**< UART Busy */
#define XUARTSBSA_UARTFR_DCD 	0x00000004U	/**< Data carrier detect */
#define XUARTSBSA_UARTFR_DSR	0x00000002U	/**<  Data set ready */
#define XUARTSBSA_UARTFR_CTS 	0x00000001U	/**< Clear to send */
/* @}*/

/** @name Line Control Register
 *
 * The Line Control register (UARTLCR) controls the functions of the
*  device.
 *
 * Line Control Register Bit Definition
 * @{
 */
#define XUARTSBSA_UARTLCR_SPS	0x00000080U	/**< Stick parity select */
#define XUARTSBSA_UARTLCR_FEN	0x00000010U	/**< Enable FIFOs */
#define XUARTSBSA_UARTLCR_STP2	0x00000008U	/**< Two stop bits selected */
#define XUARTSBSA_UARTLCR_EPS	0x00000004U	/**< Even parity select. */
#define XUARTSBSA_UARTLCR_PEN	0x00000002U	/**< Parity enable */
#define XUARTSBSA_UARTLCR_BRK	0x00000001U	/**< Send break */

#define XUARTSBSA_UARTLCR_WLEN_MASK 	0x00000060U	/**< Word length mask */
#define XUARTSBSA_UARTLCR_WLEN_SHIFT	0x00000005U	/**< Word length shift */
#define XUARTSBSA_UARTLCR_WLEN_5_BIT	0x00000000U	/**< 5 bits data */
#define XUARTSBSA_UARTLCR_WLEN_6_BIT	0x00000020U	/**< 6 bits data */
#define XUARTSBSA_UARTLCR_WLEN_7_BIT	0x00000040U	/**< 7 bits data */
#define XUARTSBSA_UARTLCR_WLEN_8_BIT	0x00000060U	/**< 8 bits data */
#define XUARTSBSA_UARTLCR_STP_1_BIT 	0x00000000U
										/**< One stop bits selected */
#define XUARTSBSA_UARTLCR_STP_MASK  	0x00000008U	/**< Stop bits mask */
#define XUARTSBSA_UARTLCR_STP_SHIFT  	0x00000003U	/**< Stop bits shift */
#define XUARTSBSA_UARTLCR_PARITY_EVEN	0x00000004U	/**< Even parity mode */
#define XUARTSBSA_UARTLCR_PARITY_MASK	0x00000002U	/**< Parity mask */
#define XUARTSBSA_UARTLCR_PARITY_SHIFT	0x00000001U	/**< Parity shift */
#define XUARTSBSA_UARTLCR_PARITY_NONE	0x00000000U	/**< No parity mode */
#define XUARTSBSA_UARTLCR_PARITY_ODD	0x00000000U	/**< Odd parity mode */

/* @}*/


/** @name Control Register
 *
 * The Control register (UARTCR) controls the major functions of the device.
 *
 * Control Register Bit Definition
 * @{
 */
#define XUARTSBSA_UARTCR_CTSEN	0x00008000U
								/**< CTS hardware flow control enable */
#define XUARTSBSA_UARTCR_RTSEN	0x00004000U
								/**< RTS hardware flow control enable */
#define XUARTSBSA_UARTCR_OUT2	0x00002000U
								/**< UART Out2 modem status output */
#define XUARTSBSA_UARTCR_OUT1	0x00001000U
								/**< UART Out1 modem status output */
#define XUARTSBSA_UARTCR_RTS	0x00000800U	/**< Request to send */
#define XUARTSBSA_UARTCR_DTR	0x00000400U	/**< Data transmit ready */
#define XUARTSBSA_UARTCR_RXE	0x00000200U	/**< Receive enable */
#define XUARTSBSA_UARTCR_TXE	0x00000100U	/**< Transmit enable */
#define XUARTSBSA_UARTCR_LBE	0x00000080U	/**< Loop back enable */
#define XUARTSBSA_UARTCR_SIRLP	0x00000004U	/**< SIR low-power IrDA mode */
#define XUARTSBSA_UARTCR_SIREN  	0x00000002U	/**< SIR enable */
#define XUARTSBSA_UARTCR_UARTEN 	0x00000001U	/**< UART enable */
#define XUARTSBSA_UARTCR_MODE_MASK	0x00000080U	/**< Mode mask */
#define XUARTSBSA_UARTCR_MODE_SHIFT	0x00000007U	/**< Mode shift */
#define XUARTSBSA_UARTCR_MODE_NORMAL	0x00000000U	/**< Normal Mode */
/* @}*/

/** @name Interrupt FIFO Level Select Register
 *
 * The UARTIFLS Register is the interrupt FIFO level select register.
 * You can use this register to define the FIFO level that triggers
 * the assertion of UARTTXINTR and UARTRXINTR.
 *
 * Interrupt FIFO Level Select Register Register Bit Definition
 * @{
 */
#define XUARTSBSA_UARTIFLS_RXIFLSEL_MASK	0x00000038U
							/**< Receive interrupt FIFO level select mask */
#define XUARTSBSA_UARTIFLS_RXIFLSEL_SHIFT	0x00000003U
							/**< Transmit interrupt FIFO level select shift */
#define XUARTSBSA_UARTIFLS_TXIFLSEL_MASK 	0x00000007U
							/**< Receive interrupt FIFO level select mask */
#define XUARTSBSA_UARTIFLS_TXIFLSEL_SHIFT	0x00000000U
							/**< Transmit interrupt FIFO level select shift */
#define XUARTSBSA_UARTIFLS_RXIFLSEL_1_8 	0x00000000U
							/**< Receive FIFO becomes . 1/8 full */
#define XUARTSBSA_UARTIFLS_RXIFLSEL_1_4 	0x00000008U
							/**< Receive FIFO becomes . 1/4 full */
#define XUARTSBSA_UARTIFLS_RXIFLSEL_1_2 	0x00000010U
							/**< Receive FIFO becomes * . 1/2 full */
#define XUARTSBSA_UARTIFLS_RXIFLSEL_3_4 	0x00000018U
							/**< Receive FIFO becomes * . 3/4 full */
#define XUARTSBSA_UARTIFLS_RXIFLSEL_7_8 	0x00000020U
							/**< Receive FIFO becomes * . 7/8 full */
#define XUARTSBSA_UARTIFLS_TXIFLSEL_1_8 	0x00000000U
							/**< Transmit FIFO becomes * . 1/8 full */
#define XUARTSBSA_UARTIFLS_TXIFLSEL_1_4 	0x00000001U
							/**< Transmit FIFO becomes * . 1/4 full */
#define XUARTSBSA_UARTIFLS_TXIFLSEL_1_2 	0x00000002U
							/**< Transmit FIFO becomes * . 1/2 full */
#define XUARTSBSA_UARTIFLS_TXIFLSEL_3_4 	0x00000003U
							/**< Transmit FIFO becomes * . 3/4 full */
#define XUARTSBSA_UARTIFLS_TXIFLSEL_7_8 	0x00000004U
							/**< Transmit FIFO becomes * . 7/8 full */
/* @}*/

/** @name Interrupt Mask Set/Clear Register
 *
 * The UARTIMSC Register is the interrupt mask set/clear register.
 * It is a read/write register.
 * On a read this register returns the current value of the mask
 * on the relevant interrupt. On a write of 1 to the particular bit,
 * it sets the corresponding mask of that interrupt.
 * A write of 0 clears the corresponding mask.
 *
 * Interrupt Mask Set/Clear Register Bit Definition
 * @{
 */
#define XUARTSBSA_UARTIMSC_OEIM	0x00000400U	/**< Overrun Error Interrupt */
#define XUARTSBSA_UARTIMSC_BEIM	0x00000200U	/**< Break Error Interrupt */
#define XUARTSBSA_UARTIMSC_PEIM	0x00000100U	/**< Parity Error Interrupt */
#define XUARTSBSA_UARTIMSC_FEIM	0x00000080U	/**< Framing Error Interrupt */
#define XUARTSBSA_UARTIMSC_RTIM	0x00000040U	/**< Receive Timeout Interrupt */
#define XUARTSBSA_UARTIMSC_TXIM	0x00000020U	/**< Transmit Interrupt */
#define XUARTSBSA_UARTIMSC_RXIM	0x00000010U	/**< Receive Interrupt */
#define XUARTSBSA_UARTIMSC_DSRMIM	0x00000008U
										/**< nUARTDSR modem interrupt */
#define XUARTSBSA_UARTIMSC_DCDMIM	0x00000004U
										/**< nUARTDCD modem interrupt */
#define XUARTSBSA_UARTIMSC_CTSMIM	0x00000002U
										/**< nUARTCTS modem interrupt */
#define XUARTSBSA_UARTIMSC_RIMIM	0x00000001U
										/**< nUARTRI modem interrupt */
#define XUARTSBSA_UARTIMSC_MASK 	0x000007FFU	/**< Valid bit mask */
/* @} */

/** @name Raw Interrupt Status Register
 *
 * The raw interrupt status register (UARTRIS) is a read-only register.
 * This register returns the current raw status value, prior to masking,
 * of the corresponding interrupt. A write has no effect.
 *
 * Raw Interrupt Status Register Bit Definition
 * @{
 */
#define XUARTSBSA_UARTRIS_OERIS	0x00000400U	/**< Overrun Error Interrupt */
#define XUARTSBSA_UARTRIS_BERIS	0x00000200U	/**< Break Error Interrupt */
#define XUARTSBSA_UARTRIS_PERIS	0x00000100U	/**< Parity Error Interrupt */
#define XUARTSBSA_UARTRIS_FERIS	0x00000080U	/**< Framing Error Interrupt */
#define XUARTSBSA_UARTRIS_RTRIS	0x00000040U	/**< Receive Timeout Interrupt */
#define XUARTSBSA_UARTRIS_TXRIS	0x00000020U	/**< Transmit Interrupt */
#define XUARTSBSA_UARTRIS_RXRIS	0x00000010U	/**< Receive Interrupt */
#define XUARTSBSA_UARTRIS_DSRRMIS	0x00000008U
									/**< nUARTDSR modem interrupt */
#define XUARTSBSA_UARTRIS_DCDRMIS	0x00000004U
									/**< nUARTDCD modem interrupt */
#define XUARTSBSA_UARTRIS_CTSRMIS	0x00000002U
									/**< nUARTCTS modem interrupt */
#define XUARTSBSA_UARTRIS_RIRMIS	0x00000001U
									/**< nUARTRI modem interrupt */
/* @} */


/** @name Masked Interrupt Status Register
 *
 * The UARTMIS Register is the masked interrupt status register.
 * It is a read-only register.
 * This register returns the current masked status value of the
 * corresponding interrupt. A write has no effect.
 *
 * Masked Interrupt Status Register Bit Definition
 * @{
 */
#define XUARTSBSA_UARTMIS_OEMIS	0x00000400U	/**< Overrun Error Interrupt */
#define XUARTSBSA_UARTMIS_BEMIS	0x00000200U	/**< Break Error Interrupt */
#define XUARTSBSA_UARTMIS_PEMIS	0x00000100U	/**< Parity Error Interrupt */
#define XUARTSBSA_UARTMIS_FEMIS	0x00000080U	/**< Framing Error Interrupt */
#define XUARTSBSA_UARTMIS_RTMIS	0x00000040U	/**< Receive Timeout Interrupt */
#define XUARTSBSA_UARTMIS_TXMIS	0x00000020U	/**< Transmit Interrupt */
#define XUARTSBSA_UARTMIS_RXMIS	0x00000010U	/**< Receive Interrupt */
#define XUARTSBSA_UARTMIS_DSRMMIS	0x00000008U
									/**< nUARTDSR modem interrupt */
#define XUARTSBSA_UARTMIS_DCDMMIS	0x00000004U
									/**< nUARTDCD modem interrupt */
#define XUARTSBSA_UARTMIS_CTSMMIS	0x00000002U
									/**< nUARTCTS modem interrupt */
#define XUARTSBSA_UARTMIS_RIRMMIS	0x00000001U
									/**< nUARTRI modem interrupt */
/* @} */



/** @name Interrupt Clear Register
 *
 * The UARTICR Register is the interrupt clear register and is write-only.
 * On a write of 1, the corresponding interrupt is cleared.
 * A write of 0 has no effect.
 *
 * Interrupt Clear Register Bit Definition
 * @{
 */
#define XUARTSBSA_UARTICR_OEIC  	0x00000400U	/**< Overrun Error Interrupt */
#define XUARTSBSA_UARTICR_BEIC  	0x00000200U	/**< Break Error Interrupt */
#define XUARTSBSA_UARTICR_PEIC  	0x00000100U	/**< Parity Error Interrupt */
#define XUARTSBSA_UARTICR_FEIC  	0x00000080U	/**< Framing Error Interrupt */
#define XUARTSBSA_UARTICR_RTIC  	0x00000040U
									/**< Receive Timeout Interrupt */
#define XUARTSBSA_UARTICR_TXIC  	0x00000020U	/**< Transmit Interrupt */
#define XUARTSBSA_UARTICR_RXIC  	0x00000010U	/**< Receive Interrupt */
#define XUARTSBSA_UARTICR_DSRMIC	0x00000008U
									/**< nUARTDSR modem interrupt */
#define XUARTSBSA_UARTICR_DCDMIC	0x00000004U
									/**< nUARTDCD modem interrupt */
#define XUARTSBSA_UARTICR_CTSMIC	0x00000002U
									/**< nUARTCTS modem interrupt */
#define XUARTSBSA_UARTICR_RIMIC 	0x00000001U
									/**< nUARTRI modem interrupt */
/* @} */

/** @name DMA Control Register
 *
 * The UARTDMACR Register is the DMA control register.
 * It is a read/write register. All the bits are cleared to 0 on reset.
 *
 * DMA Control Register Bit Definition
 * @{
 */
#define XUARTSBSA_UARTDMACR_DMAONERR	0x00000004U	/**< DMA on error */
#define XUARTSBSA_UARTDMACR_TXDMAE	0x00000002U 	/**< Transmit DMA enable */
#define XUARTSBSA_UARTDMACR_RXDMAE	0x00000001U	/**< Receive DMA enable */
/* @} */

/** @name Integer Baud Rate Register
 *
 * The UARTIBRD Register is the integer part of the baud rate divisor value
 * @{
 */
#define XUARTSBSA_UARTIBRD_BAUD_DIVINT_MASK			0x0000FFFFU
												/**< 16 UARTIBRD bit mask */
#define XUARTSBSA_UARTIBRD_BAUD_DIVINT_RESET_VAL	0x00000000U
												/**< Reset value */
/* @} */


/** @name Fractional Baud Rate Register
 *
 * The UARTFBRD Register is the fractional part of the baud rate divisor
 *  value
 *
 * Baud rate divisor BAUDDIV = (FUARTCLK/(16xBaud rate))
 * where FUARTCLK is the UART reference clock frequency
 * The BAUDDIV is comprised of the integer value (BAUD DIVINT) and the
 * fractional value (BAUD DIVFRAC)
 * @{
 */
#define XUARTSBSA_UARTFBRD_BAUD_DIVFRAC_MASK		0x0000003FU
												/**< 6 UARTFBRD bit mask */
#define XUARTSBSA_UARTFBRD_BAUD_DIVFRAC_RESET_VAL	0x00000000U
												/**< Reset value */
/* @} */


/** @name Receiver Timeout Register
 *
 * Use the receiver timeout register (RTR) to detect an idle condition on
 * the receiver data line.
 *
 * @{
 */
#define XUARTSBSA_RXTOUT_DISABLE	0x00000000U  /**< Disable time out */
#define XUARTSBSA_RXTOUT_MASK		0x000000FFU  /**< Valid bits mask */

/** @name Receiver FIFO Trigger Level Register
 *
 * Use the Receiver FIFO Trigger Level Register (RTRIG) to set the value at
 * which the RX FIFO triggers an interrupt event.
 * @{
 */

#define XUARTSBSA_RXWM_DISABLE  	0x00000000U
									/**< Disable RX trigger interrupt */
#define XUARTSBSA_RXWM_MASK 		0x0000003FU	/**< Valid bits mask */
#define XUARTSBSA_RXWM_RESET_VAL	0x00000020U	/**< Reset value */
/* @} */

/** @name Transmit FIFO Trigger Level Register
 *
 * Use the Transmit FIFO Trigger Level Register (TTRIG) to set the value at
 * which the TX FIFO triggers an interrupt event.
 * @{
 */

#define XUARTSBSA_TXWM_MASK 		0x0000003FU 	/**< Valid bits mask */
#define XUARTSBSA_TXWM_RESET_VAL	0x00000020U 	/**< Reset value */
/* @} */

/** @name Modem Control Register
 *
 * This register (MODEMCR) controls the interface with the modem or data set,
 * or a peripheral device emulating a modem.
 *
 * @{
 */
#define XUARTSBSA_MODEMCR_FCM	0x00000010U	/**< Flow control mode */
#define XUARTSBSA_MODEMCR_RTS	0x00000002U	/**< Request to send */
#define XUARTSBSA_MODEMCR_DTR	0x00000001U	/**< Data terminal ready */
/* @} */

/** @name Modem Status Register
 *
 * This register (MODEMSR) indicates the current state of the control lines
 * from a modem, or another peripheral device, to the CPU. In addition, four
 * bits of the modem status register provide change information. These bits
 * are set to a logic 1 whenever a control input from the modem changes state.
 *
 * Note: Whenever the DCTS, DDSR, TERI, or DDCD bit is set to logic 1, a
 * modem status interrupt is generated and this is reflected in the modem
 * status register.
 *
 * @{
 */
#define XUARTSBSA_MODEMSR_FCMS	0x00000100U  /**< Flow control mode (FCMS) */
#define XUARTSBSA_MODEMSR_DCD	0x00000080U  /**< Complement of DCD input */
#define XUARTSBSA_MODEMSR_RI	0x00000040U  /**< Complement of RI input */
#define XUARTSBSA_MODEMSR_DSR	0x00000020U  /**< Complement of DSR input */
#define XUARTSBSA_MODEMSR_CTS	0x00000010U  /**< Complement of CTS input */
#define XUARTSBSA_MODEMSR_DDCD	0x00000008U  /**< Delta DCD indicator */
#define XUARTSBSA_MODEMSR_TERI	0x00000004U
								/**< Trailing Edge Ring Indicator */
#define XUARTSBSA_MODEMSR_DDSR	0x00000002U  /**< Change of DSR */
#define XUARTSBSA_MODEMSR_DCTS	0x00000001U  /**< Change of CTS */
/* @} */

/** @name Flow Delay Register
 *
 * Operation of the flow delay register (FLOWDEL) is very similar to the
 * receive FIFO trigger register. An internal trigger signal activates when
 * the FIFO is filled to the level set by this register. This trigger will not
 * cause an interrupt, although it can be read through the channel status
 * register. In hardware flow control mode, RTS is deactivated when the trigger
 * becomes active. RTS only resets when the FIFO level is four less than the
 * level of the flow delay trigger and the flow delay trigger is not activated.
 * A value less than 4 disables the flow delay.
 * @{
 */
#define XUARTSBSA_FLOWDEL_MASK	XUARTSBSA_RXWM_MASK	/**< Valid bit mask */
/* @} */

/** @name Receiver FIFO Byte Status Register
 *
 * The Receiver FIFO Status register is used to have a continuous
 * monitoring of the raw unmasked byte status information. The register
 * contains frame, parity and break status information for the top
 * four bytes in the RX FIFO.
 *
 * Receiver FIFO Byte Status Register Bit Definition
 * @{
 */
#define XUARTSBSA_RXBS_BYTE3_BRKE	0x00000800U /**< Byte3 Break Error */
#define XUARTSBSA_RXBS_BYTE3_FRME	0x00000400U /**< Byte3 Frame Error */
#define XUARTSBSA_RXBS_BYTE3_PARE	0x00000200U /**< Byte3 Parity Error */
#define XUARTSBSA_RXBS_BYTE2_BRKE	0x00000100U /**< Byte2 Break Error */
#define XUARTSBSA_RXBS_BYTE2_FRME	0x00000080U /**< Byte2 Frame Error */
#define XUARTSBSA_RXBS_BYTE2_PARE	0x00000040U /**< Byte2 Parity Error */
#define XUARTSBSA_RXBS_BYTE1_BRKE	0x00000020U /**< Byte1 Break Error */
#define XUARTSBSA_RXBS_BYTE1_FRME	0x00000010U /**< Byte1 Frame Error */
#define XUARTSBSA_RXBS_BYTE1_PARE	0x00000008U /**< Byte1 Parity Error */
#define XUARTSBSA_RXBS_BYTE0_BRKE	0x00000004U /**< Byte0 Break Error */
#define XUARTSBSA_RXBS_BYTE0_FRME	0x00000002U /**< Byte0 Frame Error */
#define XUARTSBSA_RXBS_BYTE0_PARE	0x00000001U /**< Byte0 Parity Error */
#define XUARTSBSA_RXBS_MASK 		0x0000000FU
									/**< 3 bit RX byte status mask */
/* @} */


/*
 * Defines for backwards compatabilty, will be removed
 * in the next version of the driver
 */
#define XUARTSBSA_MEDEMSR_DCDX  XUARTSBSA_MODEMSR_DDCD
#define XUARTSBSA_MEDEMSR_RIX   XUARTSBSA_MODEMSR_TERI
#define XUARTSBSA_MEDEMSR_DSRX  XUARTSBSA_MODEMSR_DDSR
#define XUARTSBSA_MEDEMSR_CTSX  XUARTSBSA_MODEMSR_DCTS

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* Read a UART register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the base address of the
*			device.
*
* @return	The value read from the register.
*
* @note 	C-Style signature:
*			u32 XUartSbsa_ReadReg(u32 BaseAddress, int RegOffset)
*
******************************************************************************/
#define XUartSbsa_ReadReg(BaseAddress, RegOffset) \
			Xil_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
* Write a UART register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the base address of the
*			device.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note 	C-Style signature:
*			void XUartSbsa_WriteReg(u32 BaseAddress, int RegOffset,
*			u16 RegisterValue)
*
******************************************************************************/
#define XUartSbsa_WriteReg(BaseAddress, RegOffset, RegisterValue) \
			Xil_Out32((BaseAddress) + (u32)(RegOffset), \
			(u32)(RegisterValue))

/*****************************************************************************/
/**
* Determine if there is receive data in the receiver and/or FIFO.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	TRUE if there is receive data, FALSE otherwise.
*
* @note 	C-Style signature:
*			u32 XUartSbsa_IsReceiveData(u32 BaseAddress)
*
******************************************************************************/
#define XUartSbsa_IsReceiveData(BaseAddress) \
			!((Xil_In32((BaseAddress) + \
			XUARTSBSA_UARTFR_OFFSET) & \
			(u32)XUARTSBSA_UARTFR_RXFE) == \
			(u32)XUARTSBSA_UARTFR_RXFE)

/*****************************************************************************/
/**
* Determine if a byte of data can be sent with the transmitter.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	TRUE if the TX FIFO is full, FALSE if a byte can be put in
*			the FIFO.
*
* @note 	C-Style signature:
*			u32 XUartSbsa_IsTransmitFull(u32 BaseAddress)
*
******************************************************************************/
#define XUartSbsa_IsTransmitFull(BaseAddress) \
			((Xil_In32((BaseAddress) + XUARTSBSA_UARTFR_OFFSET) & \
			(u32)XUARTSBSA_UARTFR_TXFF) == \
			(u32)XUARTSBSA_UARTFR_TXFF)

/*****************************************************************************/
/**
* Determine if a byte of data can be sent with the transmitter.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	TRUE if the TX is busy, FALSE if a byte can be put in the
*			FIFO.
*
* @note 	C-Style signature:
*			u32 XUartSbsa_IsTransmitFull(u32 BaseAddress)
*
******************************************************************************/
#define XUartSbsa_IsTransmitbusy(BaseAddress) \
			((Xil_In32((BaseAddress) + XUARTSBSA_UARTFR_OFFSET) & \
			(u32)XUARTSBSA_UARTFR_BUSY) == \
			(u32)XUARTSBSA_UARTFR_BUSY)

/************************** Function Prototypes ******************************/

void XUartSbsa_SendByte(u32 BaseAddress, u8 Data);

u8 XUartSbsa_RecvByte(u32 BaseAddress);

void XUartSbsa_ResetHw(u32 BaseAddress);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
