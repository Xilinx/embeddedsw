/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartns550_options.c
* @addtogroup uartns550_v3_6
* @{
*
* The implementation of the options functions for the XUartNs550 driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b jhl  03/11/02 Repartitioned driver for smaller files.
* 1.00b rpm  04/12/05 Added critical section protection in ReadFcrRegister
* 1.11a sv   03/20/07 Updated to use the new coding guidelines.
* 1.13a sdm  07/10/09 Added receive line interrupt option to OptionsTable[].
* 2.00a sdm  09/22/09 Converted all register accesses to 32 bit access.
* 2.00a ktn  10/20/09 Converted all register accesses to 32 bit access.
*		      Updated to use HAL Processor APIs. _m is removed from the
*		      name of all the macro definitions.
* 3.4   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototype of ReadFcrRegister API.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xuartns550.h"
#include "xuartns550_i.h"
#include "xil_io.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/
/*
 * The following data type maps an option to a register and the bits of the
 * register such that getting and setting the options may be table driven.
 */
typedef struct {
	u16 Option;
	u16 RegisterOffset;
	u8  Mask;
} Mapping;

/*
 * Create the table which contains options which are to be processed to get/set
 * the options. These options are table driven to allow easy maintenance and
 * expansion of the options.
 */
static Mapping OptionsTable[] = {
	{ XUN_OPTION_SET_BREAK, XUN_LCR_OFFSET, XUN_LCR_SET_BREAK },
	{ XUN_OPTION_LOOPBACK, XUN_MCR_OFFSET, XUN_MCR_LOOP },
	{ XUN_OPTION_DATA_INTR, XUN_IER_OFFSET, XUN_IER_RX_DATA },
	{ XUN_OPTION_MODEM_INTR, XUN_IER_OFFSET, XUN_IER_MODEM_STATUS },
	{ XUN_OPTION_FIFOS_ENABLE, XUN_FCR_OFFSET, XUN_FIFO_ENABLE },
	{ XUN_OPTION_RESET_TX_FIFO, XUN_FCR_OFFSET, XUN_FIFO_TX_RESET },
	{ XUN_OPTION_RESET_RX_FIFO, XUN_FCR_OFFSET, XUN_FIFO_RX_RESET },
	{ XUN_OPTION_ASSERT_OUT2, XUN_MCR_OFFSET, XUN_MCR_OUT_2 },
	{ XUN_OPTION_ASSERT_OUT1, XUN_MCR_OFFSET, XUN_MCR_OUT_1 },
	{ XUN_OPTION_ASSERT_RTS, XUN_MCR_OFFSET, XUN_MCR_RTS },
	{ XUN_OPTION_ASSERT_DTR, XUN_MCR_OFFSET, XUN_MCR_DTR },
	{ XUN_OPTION_RXLINE_INTR, XUN_IER_OFFSET, XUN_IER_RX_LINE }
};

/* Create a constants for the number of entries in the table */

#define XUN_NUM_OPTIONS	  (sizeof(OptionsTable) / sizeof(Mapping))

/************************** Function Prototypes *****************************/

static u32 ReadFcrRegister(UINTPTR BaseAddress);

/****************************************************************************/
/**
*
* Gets the options for the specified driver instance. The options are
* implemented as bit masks such that multiple options may be enabled or
* disabled simultaneously.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return 	The current options for the UART. The options are bit masks
*		that are contained in the file xuartns550.h and
*		named XUN_OPTION_*.
*
* @note		None.
*
*****************************************************************************/
u16 XUartNs550_GetOptions(XUartNs550 *InstancePtr)
{
	u16 Options = 0;
	u32 Register;
	u32 Index;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Loop through the options table to map the physical options in the
	 * registers of the UART to the logical options to be returned
	 */
	for (Index = 0; Index < XUN_NUM_OPTIONS; Index++) {
		/*
		 * If the FIFO control register is being read, the make sure to
		 * setup the line control register so it can be read
		 */
		if (OptionsTable[Index].RegisterOffset == XUN_FCR_OFFSET) {
			Register = ReadFcrRegister(InstancePtr->BaseAddress);
		} else {
			Register = XUartNs550_ReadReg(InstancePtr->BaseAddress,
					OptionsTable[Index].RegisterOffset);
		}

		/*
		 * If the bit in the register which correlates to the option
		 * is set, then set the corresponding bit in the options,
		 * ignoring any bits which are zero since the options variable
		 * is initialized to zero
		 */
		if (Register & OptionsTable[Index].Mask) {
			Options |= OptionsTable[Index].Option;
		}
	}

	return Options;
}

/****************************************************************************/
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
* @param	InstancePtr is a pointer to the XUartNs550 instance.
* @param	Options contains the options to be set which are bit masks
*		contained in the file xuartns550.h and named XUN_OPTION_*.
*
* @return
*		- XST_SUCCESS if the options were set successfully.
*		- XST_UART_CONFIG_ERROR if the options could not be set because
*		the hardware does not support FIFOs
*
* @note		None.
*
*****************************************************************************/
int XUartNs550_SetOptions(XUartNs550 *InstancePtr, u16 Options)
{
	u32 Index;
	u32 Register;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Loop through the options table to map the logical options to the
	 * physical options in the registers of the UART
	 */
	for (Index = 0; Index < XUN_NUM_OPTIONS; Index++) {

		/*
		 * If the FIFO control register is being read, this is a
		 * special case that requires special register processing
		 */
		if (OptionsTable[Index].RegisterOffset == XUN_FCR_OFFSET) {
			Register = ReadFcrRegister(InstancePtr->BaseAddress);
		} else {
			/*
			 * Read the register which contains option so that the
			 * register can be changed without destoying any other
			 * bits of the register
			 */
			Register = XUartNs550_ReadReg(InstancePtr->BaseAddress,
					OptionsTable[Index].RegisterOffset);
		}

		/*
		 * If the option is set in the input, then set the
		 * corresponding bit in the specified register, otherwise
		 * clear the bit in the register
		 */
		if (Options & OptionsTable[Index].Option) {
			Register |= OptionsTable[Index].Mask;
		} else {
			Register &= ~OptionsTable[Index].Mask;
		}

		/*
		 * Write the new value to the register to set the option
		 */
		XUartNs550_WriteReg(InstancePtr->BaseAddress,
				 OptionsTable[Index].RegisterOffset, Register);
	}

	/* To be done, add error checks for enabling/resetting FIFOs */

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function gets the receive FIFO trigger level. The receive trigger
* level indicates the number of bytes in the receive FIFO that cause a receive
* data event (interrupt) to be generated.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return	The current receive FIFO trigger level. Constants which
*		define each trigger level are contained in the file
*		xuartns550.h and named XUN_FIFO_TRIGGER_*.
*
* @note		None.
*
*****************************************************************************/
u8 XUartNs550_GetFifoThreshold(XUartNs550 *InstancePtr)
{
	u32 FcrRegister;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the value of the FIFO control register so that the threshold
	 * can be retrieved, this read takes special register processing
	 */
	FcrRegister = ReadFcrRegister(InstancePtr->BaseAddress);

	/*
	 * Return only the trigger level from the register value
	 */
	return (u8)(FcrRegister & XUN_FIFO_RX_TRIGGER);
}

/****************************************************************************/
/**
*
* This functions sets the receive FIFO trigger level. The receive trigger
* level specifies the number of bytes in the receive FIFO that cause a receive
* data event (interrupt) to be generated. The FIFOs must be enabled to set the
* trigger level.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
* @param	TriggerLevel contains the trigger level to set. Constants which
*		define each trigger level are contained in the file xuartns550.h
*		and named XUN_FIFO_TRIGGER_*.
*
* @return
*		- XST_SUCCESS if the trigger level was set
*		- XST_UART_CONFIG_ERROR if the trigger level could not be set,
*		either the hardware does not support the FIFOs or FIFOs
*		are not enabled
*
* @note		None.
*
*****************************************************************************/
int XUartNs550_SetFifoThreshold(XUartNs550 *InstancePtr, u8 TriggerLevel)
{
	u32 FcrRegister;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((TriggerLevel == XUN_FIFO_TRIGGER_14) ||
			(TriggerLevel == XUN_FIFO_TRIGGER_08) ||
			(TriggerLevel == XUN_FIFO_TRIGGER_04) ||
			(TriggerLevel == XUN_FIFO_TRIGGER_01));
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the value of the FIFO control register, this read takes special
	 * register processing
	 */
	FcrRegister = ReadFcrRegister(InstancePtr->BaseAddress);

	/*
	 * If the FIFO control register indicates that FIFOs are disabled, then
	 * either they are just disabled or it has no FIFOs, return an error
	 */
	if ((FcrRegister & XUN_FIFO_ENABLE) == 0) {
		return XST_UART_CONFIG_ERROR;
	}

	/*
	 * Set the receive FIFO trigger level by clearing out the old level in
	 * the FIFO control register and writing in the new level
	 */
	FcrRegister &= ~XUN_FIFO_RX_TRIGGER;
	FcrRegister |= (u32) TriggerLevel;

	/*
	 * Write the new value for the FIFO control register to it such that the
	 * threshold is changed, writing to it is normal unlike reading from it
	 */
	XUartNs550_WriteReg(InstancePtr->BaseAddress,
				XUN_FCR_OFFSET, FcrRegister);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function returns the last errors that have occurred in the specified
* UART. It also clears the errors such that they cannot be retrieved again.
* The errors include parity error, receive overrun error, framing error, and
* break detection.
*
* The last errors is an accumulation of the errors each time an error is
* discovered in the driver. A status is checked for each received byte and
* this status is accumulated in the last errors.
*
* If this function is called after receiving a buffer of data, it will indicate
* any errors that occurred for the bytes of the buffer. It does not indicate
* which bytes contained errors.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return	The last errors that occurred. The errors are bit masks
*		that are contained in the file xuartns550.h and
*		named XUN_ERROR_*.
*
* @note		None.
*
*****************************************************************************/
u8 XUartNs550_GetLastErrors(XUartNs550 *InstancePtr)
{
	u8 Temp = InstancePtr->LastErrors;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Clear the last errors and return the previous value
	 */
	InstancePtr->LastErrors = 0;

	/*
	 * Only return the bits that are reported errors which include
	 * receive overrun, framing, parity and break detection, the last errors
	 * variable holds an accumulation of the line status register bits which
	 * have been set
	 */
	return Temp & XUN_LSR_ERROR_BREAK;
}

/****************************************************************************/
/**
*
* This function gets the modem status from the specified UART. The modem
* status indicates any changes of the modem signals. This function allows
* the modem status to be read in a polled mode. The modem status is updated
* whenever it is read such that reading it twice may not yield the same
* results.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance .
*
* @return 	The modem status which are bit masks that are contained in
*		the file  xuartns550.h and named XUN_MODEM_*.
*
* @note
*
* The bit masks used for the modem status are the exact bits of the modem
* status register with no abstraction.
*
*****************************************************************************/
u8 XUartNs550_GetModemStatus(XUartNs550 *InstancePtr)
{
	u32 ModemStatusRegister;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Read the modem status register to return
	 */
	ModemStatusRegister = XUartNs550_ReadReg(InstancePtr->BaseAddress,
					XUN_MSR_OFFSET);
	return (u8) ModemStatusRegister;
}

/****************************************************************************/
/**
*
* This function determines if the specified UART is sending data. If the
* transmitter register is not empty, it is sending data.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return	A value of TRUE if the UART is sending data, otherwise FALSE.
*
* @note		None.
*
*****************************************************************************/
int XUartNs550_IsSending(XUartNs550 *InstancePtr)
{
	u32 LsrRegister;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Read the line status register to determine if the transmitter is
	 * empty
	 */
	LsrRegister = XUartNs550_GetLineStatusReg(InstancePtr->BaseAddress);

	/*
	 * If the transmitter is not empty then indicate that the UART is still
	 * sending some data
	 */
	return ((LsrRegister & XUN_LSR_TX_EMPTY) == 0);
}

/****************************************************************************/
/**
*
* This functions reads the FIFO control register. It's primary purpose is to
* isolate the special processing for reading this register. It is necessary
* to write to the line control register, then read the FIFO control register,
* and then restore the line control register.
*
* @param	BaseAddress contains the base address of the registers in the
*		device.
*
* @return	The contents of the FIFO control register.
*
* @note		None.
*
*****************************************************************************/
static u32 ReadFcrRegister(UINTPTR BaseAddress)
{
	u32 LcrRegister;
	u32 FcrRegister;
	u32 IerRegister;

	/*
	 * Enter a critical section here by disabling Uart interrupts.  We do
	 * not want to receive an interrupt while we have the FCR latched since
	 * the interrupt handler may want to read the IIR
	 */
	IerRegister = XUartNs550_ReadReg(BaseAddress, XUN_IER_OFFSET);
	XUartNs550_WriteReg(BaseAddress, XUN_IER_OFFSET, 0);

	/*
	 * Get the line control register contents and set the divisor latch
	 * access bit so the FIFO control register can be read, this can't
	 * be done with a true 16550, but is a feature in the Xilinx device
	 */
	LcrRegister = XUartNs550_GetLineControlReg(BaseAddress);
	XUartNs550_SetLineControlReg(BaseAddress, LcrRegister | XUN_LCR_DLAB);

	/*
	 * Read the FIFO control register so it can be returned
	 */
	FcrRegister = XUartNs550_ReadReg(BaseAddress, XUN_FCR_OFFSET);

	/*
	 * Restore the line control register to it's original contents such
	 * that the DLAB bit is no longer set and return the register
	 */
	XUartNs550_SetLineControlReg(BaseAddress, LcrRegister);

	/*
	 * Exit the critical section by restoring the IER
	 */
	XUartNs550_WriteReg(BaseAddress, XUN_IER_OFFSET, IerRegister);

	return FcrRegister;
}
/** @} */
