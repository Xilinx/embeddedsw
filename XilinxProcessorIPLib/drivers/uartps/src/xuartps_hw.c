/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartps_hw.c
*
* @brief Implements the XUartPs low-level hardware interface functions.
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- ------ -------- ----------------------------------------------
* 1.00	drg/jz 01/12/10 First Release
* 1.05a hk     08/22/13 Added reset function
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* 4.00  sd     02/02/24 Added wait for transmission done function
* 3.18  vlt    01/27/26 Fixed codespell issues
* 3.19  vn     08/26/26 Fix Doxygen warnings
* 3.19  vn   09/10/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup uartps Overview
 * @{
 */

/***************************** Include Files ********************************/
#include "xuartps_hw.h"

/************************** Constant Definitions ****************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* @brief Sends one byte using the device in polled mode.
*
* Blocks until the data has been put into the TX FIFO register.
*
* @param[in]   BaseAddress Base address of the device.
* @param[in]   Data Byte to send.
*
******************************************************************************/
void XUartPs_SendByte(u32 BaseAddress, u8 Data)
{
	/* Wait until there is space in TX FIFO */
	while (XUartPs_IsTransmitFull(BaseAddress)) {
		;
	}

	/* Write the byte into the TX FIFO */
	XUartPs_WriteReg(BaseAddress, XUARTPS_FIFO_OFFSET, (u32)Data);
}

/*****************************************************************************/
/**
*
* @brief Receives a byte from the device in polled mode.
*
* Blocks until a byte has been received.
*
* @param[in]   BaseAddress Base address of the device.
*
* @return      The data byte received.
*
******************************************************************************/
u8 XUartPs_RecvByte(u32 BaseAddress)
{
	u32 RecievedByte;
	/* Wait until there is data */
	while (!XUartPs_IsReceiveData(BaseAddress)) {
		;
	}
	RecievedByte = XUartPs_ReadReg(BaseAddress, XUARTPS_FIFO_OFFSET);
	/* Return the byte received */
	return (u8)RecievedByte;
}

/*****************************************************************************/
/**
*
* @brief Resets the UART.
*
* @param[in]   BaseAddress Base address of the device.
*
******************************************************************************/
void XUartPs_ResetHw(u32 BaseAddress)
{

	/* Disable interrupts */
	XUartPs_WriteReg(BaseAddress, XUARTPS_IDR_OFFSET, XUARTPS_IXR_MASK);

	/* Disable receive and transmit */
	XUartPs_WriteReg(BaseAddress, XUARTPS_CR_OFFSET,
				((u32)XUARTPS_CR_RX_DIS | (u32)XUARTPS_CR_TX_DIS));

	/*
	 * Software reset of receive and transmit
	 * This clears the FIFO.
	 */
	XUartPs_WriteReg(BaseAddress, XUARTPS_CR_OFFSET,
				((u32)XUARTPS_CR_TXRST | (u32)XUARTPS_CR_RXRST));

	/* Clear status flags - SW reset won't clear sticky flags. */
	XUartPs_WriteReg(BaseAddress, XUARTPS_ISR_OFFSET, XUARTPS_IXR_MASK);

	/*
	 * Mode register reset value : All zeroes
	 * Normal mode, even parity, 1 stop bit
	 */
	XUartPs_WriteReg(BaseAddress, XUARTPS_MR_OFFSET,
				XUARTPS_MR_CHMODE_NORM);

	/* Rx and TX trigger register reset values */
	XUartPs_WriteReg(BaseAddress, XUARTPS_RXWM_OFFSET,
				XUARTPS_RXWM_RESET_VAL);
	XUartPs_WriteReg(BaseAddress, XUARTPS_TXWM_OFFSET,
				XUARTPS_TXWM_RESET_VAL);

	/* Rx timeout disabled by default */
	XUartPs_WriteReg(BaseAddress, XUARTPS_RXTOUT_OFFSET,
				XUARTPS_RXTOUT_DISABLE);

	/* Baud rate generator and divider reset values */
	XUartPs_WriteReg(BaseAddress, XUARTPS_BAUDGEN_OFFSET,
				XUARTPS_BAUDGEN_RESET_VAL);
	XUartPs_WriteReg(BaseAddress, XUARTPS_BAUDDIV_OFFSET,
				XUARTPS_BAUDDIV_RESET_VAL);

	/*
	 * Control register reset value -
	 * RX and TX are disable by default
	 */
	XUartPs_WriteReg(BaseAddress, XUARTPS_CR_OFFSET,
				((u32)XUARTPS_CR_RX_DIS | (u32)XUARTPS_CR_TX_DIS |
						(u32)XUARTPS_CR_STOPBRK));

}

/*****************************************************************************/
/**
*
* @brief Waits for transmission to complete.
*
* @param[in]   BaseAddress Base address of the device.
*
******************************************************************************/
void XUartPs_WaitTransmitDone(u32 BaseAddress)
{
	/* Wait until Transmitter FIFO is empty */
	while (!XUartPs_IsTransmitFifoEmpty(BaseAddress)) {
		;
	}
	/* Wait until Transmitter state machine is In-Active */
	while (XUartPs_IsTransmitActive(BaseAddress)) {
		;
	}
}


#ifdef SDT
#ifdef XPAR_STDIN_IS_UARTPS
/*****************************************************************************/
/**
*
* @brief Sends a character to the standard output UART.
*
* @param[in]   c Character to send.
*
******************************************************************************/
void outbyte(char c) {
         XUartPs_SendByte(STDOUT_BASEADDRESS, c);
}

/*****************************************************************************/
/**
*
* @brief Receives a character from the standard input UART.
*
* @return      Character received.
*
******************************************************************************/
char inbyte(void) {
         return XUartPs_RecvByte(STDIN_BASEADDRESS);
}
#endif
#endif
/** @} */
