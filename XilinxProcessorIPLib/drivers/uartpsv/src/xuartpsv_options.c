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
* @file xuartpsv_options.c
* @addtogroup uartpsv_v1_0
* @{
*
* The implementation of the options functions for the XUartPsv driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who  Date      Changes
* ---  ---  --------- -----------------------------------------------
* 1.0  sg   09/18/17  First Releasee
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xuartpsv.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
/*
 * The following data type is a map from an option to the offset in the
 * register to which it belongs as well as its bit mask in that register.
 */
typedef struct {
	u16 Option;
	u16 RegisterOffset;
	u32 Mask;
} Mapping;

/*
 * Create the table which contains options which are to be processed to
 * get/set the options. These options are table driven to allow easy
 * maintenance and expansion of the options.
 */

static Mapping OptionsTable[] = {
	{XUARTPSV_OPTION_SET_BREAK, XUARTPSV_UARTCR_OFFSET,
				XUARTPSV_UARTLCR_BRK},
	{XUARTPSV_OPTION_STOP_BREAK, XUARTPSV_UARTCR_OFFSET,
				XUARTPSV_UARTLCR_BRK},
	{XUARTPSV_OPTION_RESET_TMOUT, XUARTPSV_UARTCR_OFFSET,
				XUARTPSV_UARTCR_RTSEN},
	{XUARTPSV_OPTION_RESET_TX, XUARTPSV_UARTCR_OFFSET,
				XUARTPSV_UARTFR_TXFE},
	{XUARTPSV_OPTION_RESET_RX, XUARTPSV_UARTCR_OFFSET,
				XUARTPSV_UARTFR_RXFE},
	{XUARTPSV_OPTION_ASSERT_RTS, XUARTPSV_UARTCR_OFFSET,
				XUARTPSV_MODEMCR_RTS},
	{XUARTPSV_OPTION_ASSERT_DTR, XUARTPSV_UARTCR_OFFSET,
				XUARTPSV_MODEMCR_DTR},
	{XUARTPSV_OPTION_SET_FCM, XUARTPSV_UARTCR_OFFSET,
					XUARTPSV_MODEMCR_FCM}
};

/* Create a constant for the number of entries in the table */

#define XUARTPSV_NUM_OPTIONS	  (sizeof(OptionsTable) / sizeof(Mapping))

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* Gets the options for the specified driver instance. The options are
* implemented as bit masks such that multiple options may be enabled or
* disabled simulataneously.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
*
* @return	The current options for the UART. The optionss are bit masks
* 		that are contained in the file xuartpsv.h and named
*		XUARTPSV_OPTION_*.
*
* @note 	None.
*
******************************************************************************/
u16 XUartPsv_GetOptions(XUartPsv *InstancePtr)
{
	u16 Options = 0U;
	u32 Register;
	u32 Index;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Loop thru the options table to map the physical options in the
	 * registers of the UART to the logical options to be returned
	 */
	for (Index = 0U; Index < XUARTPSV_NUM_OPTIONS; Index++) {
		Register = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
				OptionsTable[Index].RegisterOffset);

		/*
		 * If the bit in the register which correlates to the option
		 * is set, then set the corresponding bit in the options,
		 * ignoring any bits which are zero since the options
		 * variable is initialized to zero
		 */
		if ((Register & OptionsTable[Index].Mask) != (u32)0) {
			Options |= OptionsTable[Index].Option;
		}
	}

	return Options;
}

/*****************************************************************************/
/**
*
* Sets the options for the specified driver instance. The options are
* implemented as bit masks such that multiple options may be enabled or
* disabled simultaneously.
*
* The GetOptions function may be called to retrieve the currently enabled
* options. The result is ORed in the desired new settings to be enabled and
* ANDed with the inverse to clear the settings to be disabled. The resulting
* value is then used as the options for the SetOption function call.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
* @param	Options contains the options to be set which are bit masks
*		contained in the file xuartpsv.h and named XUARTPSV_OPTION_*.
*
* @return	None.
*
* @note 	None.
*
******************************************************************************/
void XUartPsv_SetOptions(XUartPsv *InstancePtr, u16 Options)
{
	u32 Index;
	u32 Register;
	(void) Options;

	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Loop thru the options table to map the logical options to the
	 * physical options in the registers of the UART.
	 */
	for (Index = 0U; Index < XUARTPSV_NUM_OPTIONS; Index++) {

		/*
		 * Read the register which contains option so that the
		 * register can be changed without destoying any other bits
		 * of the register.
		 */
		Register = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
					OptionsTable[Index].RegisterOffset);
#if 0
		TBD
		/*
		 * If the option is set in the input, then set the
		 * corresponding bit in the specified register, otherwise
		 * clear the bit in the register.
		 */
		if ((Options & OptionsTable[Index].Option) != (u16)0) {
			if (OptionsTable[Index].Option ==
				XUARTPSV_OPTION_SET_BREAK)
				Register &= ~XUARTPSV_CR_STOPBRK;
			Register |= OptionsTable[Index].Mask;
		}
		else {
			Register &= ~OptionsTable[Index].Mask;
		}
#endif
		/* Write the new value to the register to set the option */
		XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
				OptionsTable[Index].RegisterOffset, Register);
	}

}

/*****************************************************************************/
/**
*
* This function gets the receive FIFO trigger level. The receive trigger
* level indicates the number of bytes in the FIFO that cause a receive or
* transmit data event (interrupt) to be generated.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
*
* @return	The current receive FIFO trigger level. This is a value
*		from 0-31.
*
* @note 	None.
*
******************************************************************************/
u8 XUartPsv_GetFifoThreshold(XUartPsv *InstancePtr)
{
	u8 RtrigRegister;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the value of the FIFO control register so that the threshold
	 * can be retrieved, this read takes special register processing
	 */
	RtrigRegister = (u8) XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XUARTPSV_UARTIFLS_OFFSET);

	/* Return only the trigger level from the register value */

	RtrigRegister &= (u8)XUARTPSV_UARTIFLS_TXIFLSEL_MASK;

	return RtrigRegister;
}

/*****************************************************************************/
/**
*
* This functions sets the receive FIFO trigger level. The receive or transmit
* trigger level specifies the number of bytes in the receive FIFO that cause
* a receive or transmit data event (interrupt) to be generated.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
* @param	TriggerLevel contains the trigger level to set.
*
* @return	None
*
* @note 	None.
*
******************************************************************************/
void XUartPsv_SetFifoThreshold(XUartPsv *InstancePtr, u8 TriggerLevel)
{
	u32 FifoTrigRegister;

	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(TriggerLevel <= (u8)XUARTPSV_UARTIFLS_TXIFLSEL_MASK);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	TriggerLevel = ((u32)TriggerLevel) &
				(u32)XUARTPSV_UARTIFLS_TXIFLSEL_MASK;

	FifoTrigRegister = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XUARTPSV_UARTIFLS_OFFSET);

	FifoTrigRegister &= (XUARTPSV_UARTIFLS_RXIFLSEL_MASK |
					XUARTPSV_UARTIFLS_RXIFLSEL_MASK);

	FifoTrigRegister |= TriggerLevel << XUARTPSV_UARTIFLS_TXIFLSEL_SHIFT;
	FifoTrigRegister |= TriggerLevel << XUARTPSV_UARTIFLS_RXIFLSEL_SHIFT;

	/*
	 * Write the new value for the FIFO control register to it such that
	 * the threshold is changed
	 */
	XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTIFLS_OFFSET, FifoTrigRegister);

}

/*****************************************************************************/
/**
*
* This function gets the modem status from the specified UART. The modem
* status indicates any changes of the modem signals. This function allows
* the modem status to be read in a polled mode. The modem status is updated
* whenever it is read such that reading it twice may not yield the same
* results.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
*
* @return	The modem status which are bit masks that are contained
* 		in the file xuartpsv.h and named XUARTPSV_UARTRIS_*.
*
* @note
* The bit masks used for the modem status are the exact bits of the modem
* status register with no abstraction.
*
******************************************************************************/
u16 XUartPsv_GetModemStatus(XUartPsv *InstancePtr)
{
	u32 ModemStatusRegister;
	u16 TmpRegister;
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Read the modem status register to return */
	ModemStatusRegister = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
						XUARTPSV_UARTRIS_OFFSET);
	TmpRegister = (u16)ModemStatusRegister;
	return TmpRegister;
}

/*****************************************************************************/
/**
*
* This function determines if the specified UART is sending data.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
*
* @return
*		- TRUE if the UART is sending data
*		- FALSE if UART is not sending data
*
* @note 	None.
*
******************************************************************************/
u32 XUartPsv_IsSending(XUartPsv *InstancePtr)
{
	u32 FlagRegister;
	u32 SendStatus;
	u32 ActiveResult;
	u32 EmptyResult;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the flags register to determine if the transmitter is
	 * active
	 */
	FlagRegister = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XUARTPSV_UARTFR_OFFSET);

	/*
	 * If the transmitter is active, or the TX FIFO is not empty,
	 * then indicate that the UART is still sending some data
	 */
	ActiveResult = FlagRegister & ((u32)XUARTPSV_UARTFR_BUSY);
	EmptyResult = FlagRegister & ((u32)XUARTPSV_UARTFR_TXFE);
	SendStatus = (((u32)XUARTPSV_UARTFR_BUSY) == ActiveResult) ||
		(((u32)XUARTPSV_UARTFR_TXFE) != EmptyResult);

	return SendStatus;
}

/*****************************************************************************/
/**
*
* This function gets the operational mode of the UART. The UART can operate
* in one of four modes: Normal, Local Loopback, Remote Loopback, or automatic
* echo.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
*
* @return	The operational mode is specified by constants defined in
* 		xuartpsv.h. The constants are named XUARTPSV_OPER_MODE_*
*
* @note 	None.
*
******************************************************************************/
u8 XUartPsv_GetOperMode(XUartPsv *InstancePtr)
{
	u32 CtrlRegister;
	u8 OperMode;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Read the Mode register. */
	CtrlRegister =
		XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XUARTPSV_UARTCR_OFFSET);

	CtrlRegister &= (u32)XUARTPSV_UARTCR_MODE_MASK;

	/* Return the constant */
	switch (CtrlRegister) {
	case XUARTPSV_UARTCR_MODE_NORMAL:
		OperMode = XUARTPSV_OPER_MODE_NORMAL;
		break;
	case XUARTPSV_UARTCR_LBE:
		OperMode = XUARTPSV_OPER_MODE_LOCAL_LOOP;
		break;

	default:
		OperMode = (u8)((CtrlRegister &
				(u32)XUARTPSV_UARTCR_MODE_MASK) >>
				XUARTPSV_UARTCR_MODE_SHIFT);
		break;
	}

	return OperMode;
}

/*****************************************************************************/
/**
*
* This function sets the operational mode of the UART. The UART can operate
* in one of four modes: Normal, Local Loopback, Remote Loopback, or automatic
* echo.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
* @param	OperationMode is the mode of the UART.
*
* @return	None.
*
* @note 	None.
*
******************************************************************************/
void XUartPsv_SetOperMode(XUartPsv *InstancePtr, u8 OperationMode)
{
	u32 CtrlRegister;

	/* Assert validates the input arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Read the Mode register. */
	CtrlRegister =
		XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XUARTPSV_UARTCR_OFFSET);

	/* Set the correct value by masking the bits, then ORing the const. */
	CtrlRegister &= (u32)(~XUARTPSV_UARTCR_MODE_MASK);

	switch (OperationMode) {
		case XUARTPSV_OPER_MODE_NORMAL:
			CtrlRegister |= (u32)XUARTPSV_UARTCR_MODE_NORMAL;
			break;
		case XUARTPSV_OPER_MODE_LOCAL_LOOP:
			CtrlRegister |= (u32)XUARTPSV_UARTCR_LBE;
			break;
		default:
			/* Default case made for MISRA-C Compliance. */
			break;
	}

	XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XUARTPSV_UARTCR_OFFSET, CtrlRegister);

}

/*****************************************************************************/
/**
*
* Sets the data format for the device. The data format includes the
* baud rate, number of data bits, number of stop bits, and parity. It is the
* caller's responsibility to ensure that the UART is not sending or receiving
* data when this function is called.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
* @param	FormatPtr is a pointer to a format structure containing the
*		data format to be set.
*
* @return
*		- XST_SUCCESS if the data format was successfully set.
*		- XST_UART_BAUD_ERROR indicates the baud rate could not be
*		  set because of the amount of error with the baud rate and
*		  the input clock frequency.
*		- XST_INVALID_PARAM if one of the parameters was not valid.
*
* @note
* The data types in the format type, data bits and parity, are 32 bit fields
* to prevent a compiler warning.
* The asserts in this function will cause a warning if these fields are
* bytes.
* <br><br>
*
******************************************************************************/
s32 XUartPsv_SetDataFormat(XUartPsv *InstancePtr,
						XUartPsvFormat * FormatPtr)
{
	s32 Status;
	u32 LineCtrlRegister;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(FormatPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Verify the inputs specified are valid */
	if ((FormatPtr->DataBits > ((u32)XUARTPSV_FORMAT_6_BITS)) ||
		(FormatPtr->StopBits > ((u8)XUARTPSV_FORMAT_2_STOP_BIT)) ||
		(FormatPtr->Parity > ((u32)XUARTPSV_FORMAT_NO_PARITY))) {
		Status = XST_INVALID_PARAM;
	} else {

		/*
		 * Try to set the baud rate and if it's not successful then
		 * don't continue altering the data format, this is done
		 * first to avoid the format from being altered when an
		 * error occurs
		 */
		Status = XUartPsv_SetBaudRate(InstancePtr,
					FormatPtr->BaudRate);
		if (Status != (s32)XST_SUCCESS) {
			;
		} else {

			LineCtrlRegister = XUartPsv_ReadReg(InstancePtr->
						Config.BaseAddress,
						XUARTPSV_UARTLCR_OFFSET);

			/*
			 * Set the length of data (8,7,6) by first clearing
			 * out the bits that control it in the register,
			 * then set the length in the register
			 */
			LineCtrlRegister &= (u32)
					(~XUARTPSV_UARTLCR_WLEN_MASK);
			LineCtrlRegister |= (FormatPtr->DataBits <<
						XUARTPSV_UARTLCR_WLEN_SHIFT);

			/*
			 * Set the number of stop bits in the mode register by
			 * first clearing out the bits that control it in the
			 * register, then set the number of stop bits in the
			 * register.
			 */
			LineCtrlRegister &= (u32)(~XUARTPSV_UARTLCR_STP_MASK);
			LineCtrlRegister |= (((u32)FormatPtr->StopBits) <<
						XUARTPSV_UARTLCR_STP_SHIFT);

			/*
			 * Set the parity by first clearing out the bits that
			 * control it in the register, then set the bits in
			 * the register, the default is no parity after
			 * clearing the register bits
			 */
			LineCtrlRegister &= (u32)
					(~XUARTPSV_UARTLCR_PARITY_MASK);
			LineCtrlRegister |= (FormatPtr->Parity <<
						XUARTPSV_UARTLCR_PARITY_SHIFT);

			/* Update the Line control register */
			XUartPsv_WriteReg(InstancePtr->Config.BaseAddress,
					XUARTPSV_UARTLCR_OFFSET,
					LineCtrlRegister);

			Status = XST_SUCCESS;
		}
	}
	return Status;
}

/*****************************************************************************/
/**
*
* Gets the data format for the specified UART. The data format includes the
* baud rate, number of data bits, number of stop bits, and parity.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
* @param	FormatPtr is a pointer to a format structure that will
*		contain the data format after this call completes.
*
* @return	None.
*
* @note 	None.
*
*
******************************************************************************/
void XUartPsv_GetDataFormat(XUartPsv *InstancePtr,
			XUartPsvFormat * FormatPtr)
{
	u32 LineCtrlRegister;


	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FormatPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Get the baud rate from the instance, this is not retrieved from
	 * the hardware because it is only kept as a divisor such that it
	 * is more difficult to get back to the baud rate
	 */
	FormatPtr->BaudRate = InstancePtr->BaudRate;

	LineCtrlRegister = XUartPsv_ReadReg(InstancePtr->Config.BaseAddress,
					XUARTPSV_UARTLCR_OFFSET);

	/* Get the length of data (8,7,6,5) */
	FormatPtr->DataBits =
		((LineCtrlRegister & (u32)XUARTPSV_UARTLCR_WLEN_MASK) >>
				XUARTPSV_UARTLCR_WLEN_SHIFT);

	/* Get the number of stop bits */
	FormatPtr->StopBits =
		(u8)((LineCtrlRegister & (u32)XUARTPSV_UARTLCR_STP_MASK) >>
				XUARTPSV_UARTLCR_STP_SHIFT);

	/* Determine what parity is */
	FormatPtr->Parity =
		(u32)((LineCtrlRegister & (u32)XUARTPSV_UARTLCR_PARITY_MASK)
				>> XUARTPSV_UARTLCR_PARITY_SHIFT);
}
/** @} */
