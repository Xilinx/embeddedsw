/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiomodule_l.h
* @addtogroup iomodule_v2_13
* @{
*
* This header file contains identifiers and low-level driver functions (or
* macros) that can be used to access the device.  The user should refer to the
* hardware device specification for more details of the device operation.
*
*
* Note that users of the driver interface given in this file can register
* an interrupt handler dynamically (at run-time) using the
* XIntc_RegisterHandler() function.
* User of the driver interface given in xiomodule.h should still use
* XIntc_Connect(), as always.
* Also see the discussion of the interrupt vector tables in xiomodule.h.
*
* There are currently two interrupt handlers specified in this interface.
*
* - XIOModule_LowLevelInterruptHandler() is a handler without any arguments
*   that is used in cases where there is a single interrupt controller device
*   in the system and the handler cannot be passed an argument. This function
*   is provided mostly for backward compatibility.
*
* - XIOModule_DeviceInterruptHandler() is a handler that takes a device ID
*   as an argument, indicating which interrupt controller device in the system
*   is causing the interrupt - thereby supporting multiple interrupt
*   controllers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------
* 1.00a sa   07/15/11 First release
* 1.01a sa   04/10/12 Updated with fast interrupt
* 1.02a sa   07/25/12 Updated with GPI interrupt support
* 2.7   sa   11/09/18 Updated macros to support 64 bit addresses
* 2.11  mus  05/07/21 Fixed warnings reported by doxygen tool. It fixes
*                      CR#1088640.
* 2.12	sk   06/08/21 Fix coverity warnings.
* </pre>
*
******************************************************************************/
/**
 *@cond nocomments
 */
#ifndef XIOMODULE_L_H		/* prevent circular inclusions */
#define XIOMODULE_L_H		/* by using protection macros */
/**
 *@endcond
 */
#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xiomodule_io.h"

#include "xio.h"

/************************** Constant Definitions *****************************/

/**
 * Defines the number of timer counters within a single hardware device. This
 * number is not currently parameterized in the hardware but may be in the
 * future.
 */
/**
 *@cond nocomments
 */
#define XTC_DEVICE_TIMER_COUNT		4

/**
 * Each timer counter consumes 16 bytes of address space.
 */
#define XTC_TIMER_COUNTER_OFFSET	16
#define XTC_TIMER_COUNTER_SHIFT		4

/**
 * Define the offsets from the base address for all the registers of the
 * IO module, some registers may be optional in the hardware device.
 */
#define XUL_RX_OFFSET	      0x00000000  /**< UART Receive Register     - R */
#define XUL_TX_OFFSET	      0x00000004  /**< UART Transmit Register    - W */
#define XUL_STATUS_REG_OFFSET 0x00000008  /**< UART Status Register      - R */
#define XUL_BAUDRATE_OFFSET   0x0000004C  /**< UART Baud Rate Register   - W */

#define XIN_IMR_OFFSET	      0x0000000C  /**< Intr Mode Register        - W */

#define XGO_OUT_OFFSET	      0x00000010  /**< General Purpose Output    - W */

#define XGI_IN_OFFSET	      0x00000020  /**< General Purpose Input     - R */

#define XIN_ISR_OFFSET	      0x00000030  /**< Intr Status Register      - R */
#define XIN_IPR_OFFSET	      0x00000034  /**< Intr Pending Register     - R */
#define XIN_IER_OFFSET	      0x00000038  /**< Intr Enable Register      - W */
#define XIN_IAR_OFFSET	      0x0000003C  /**< Intr Acknowledge Register - W */

#define XTC_TLR_OFFSET	      0x00000040  /**< Timer Load register       - W */
#define XTC_TCR_OFFSET	      0x00000044  /**< Timer counter register    - R */
#define XTC_TCSR_OFFSET	      0x00000048  /**< Timer Control register    - W */
#define XIN_IVAR_OFFSET	      0x00000080  /**< Intr Vector Address Register,
					       Interrupt 0 offset, present
					       only for Fast Interrupt   - W */
#define XIN_IVEAR_OFFSET      0x00000100  /**< Intr Extended Vector Address
					       Register  - W*/

/**
 * UART status register bit position masks
 */
#define XUL_SR_PARITY_ERROR		0x80
#define XUL_SR_FRAMING_ERROR		0x40
#define XUL_SR_OVERRUN_ERROR		0x20
#define XUL_SR_INTR_ENABLED		0x10 /**< UART Interrupt enabled     */
#define XUL_SR_TX_FIFO_FULL		0x08 /**< UART Transmit FIFO full    */
#define XUL_SR_RX_FIFO_VALID_DATA	0x01 /**< UART Data Register valid   */

/**
 * UART stop bits are fixed at 1. Baud, parity, and data bits are fixed on a
 * per instance basis.
 */
#define XUL_STOP_BITS			1

/**
 *  UART Parity definitions.
 */
#define XUL_PARITY_NONE			0
#define XUL_PARITY_ODD			1
#define XUL_PARITY_EVEN			2

/**
 * Defines the number of GPI and GPO within a single hardware device. This
 * number is not currently parameterized in the hardware but may be in the
 * future.
 * @{
 */
#define XGPI_DEVICE_COUNT		4
#define XGPO_DEVICE_COUNT		4

/**
 * The following constants describe the offset of each GPI and GPO channel's
 * data from the base address.
 */
#define XGPI_CHAN_OFFSET  0x00004
#define XGPI_DATA_OFFSET  0x00020

#define XGPO_CHAN_OFFSET  0x00004
#define XGPO_DATA_OFFSET  0x00010

/**
 * Interrupt register bit position masks.
 */
#define XIN_IOMODULE_GPI_4_INTERRUPT_INTR	14
#define XIN_IOMODULE_GPI_3_INTERRUPT_INTR	13
#define XIN_IOMODULE_GPI_2_INTERRUPT_INTR	12
#define XIN_IOMODULE_GPI_1_INTERRUPT_INTR	11
#define XIN_IOMODULE_FIT_4_INTERRUPT_INTR	10
#define XIN_IOMODULE_FIT_3_INTERRUPT_INTR	9
#define XIN_IOMODULE_FIT_2_INTERRUPT_INTR	8
#define XIN_IOMODULE_FIT_1_INTERRUPT_INTR	7
#define XIN_IOMODULE_PIT_4_INTERRUPT_INTR	6
#define XIN_IOMODULE_PIT_3_INTERRUPT_INTR	5
#define XIN_IOMODULE_PIT_2_INTERRUPT_INTR	4
#define XIN_IOMODULE_PIT_1_INTERRUPT_INTR	3
#define XIN_IOMODULE_UART_RX_INTERRUPT_INTR	2
#define XIN_IOMODULE_UART_TX_INTERRUPT_INTR	1
#define XIN_IOMODULE_UART_ERROR_INTERRUPT_INTR	0

#define XIN_IOMODULE_EXTERNAL_INTERRUPT_INTR	16
/* @} */

/**
 * @name Control Status Register Bit Definitions
 * Control Status Register bit masks
 * Used to configure the timer counter device.
 * @{
 */
#define XTC_CSR_ENABLE_TMR_MASK		0x00000001 /**< Enables the timer */
#define XTC_CSR_AUTO_RELOAD_MASK	0x00000002 /**< In compare mode,
							configures the timer
							reload  from the Load
							Register. The default
							mode causes the timer
							counter to hold when it
							rolls under. */
/* @} */
/**
 *@endcond
 */

/**************************** Type Definitions *******************************/

/* The following data type defines each entry in an interrupt vector table.
 * The callback reference is the base address of the interrupting device
 * for the driver interface given in this file and an instance pointer for the
 * driver interface given in xintc.h file.
 */
typedef struct {
	XInterruptHandler Handler;
	void *CallBackRef;
} XIOModule_VectorTableEntry;

typedef void (*XFastInterruptHandler) (void);

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* Enable specific interrupt(s) in the interrupt controller.
*
* @param	BaseAddress is the base address of the device
* @param	EnableMask is the 32-bit value to write to the enable register.
*		Each bit of the mask corresponds to an interrupt input signal
*		that is connected to the interrupt controller (INT0 = LSB).
*		Only the bits which are set in the mask will enable interrupts.
*
* @return	None.
*
* @note		C-style signature:
*		void XIOModule_EnableIntr(UINTPTR BaseAddress,
*                                         u32 EnableMask);
*
*****************************************************************************/
#define XIOModule_EnableIntr(BaseAddress, EnableMask) \
	XIomodule_Out32((BaseAddress) + XIN_IER_OFFSET, (EnableMask))

/****************************************************************************/
/**
*
* Disable specific interrupt(s) in the interrupt controller.
*
* @param	BaseAddress is the base address of the device
* @param	DisableMask is the 32-bit value to write to enable register.
*		Each bit of the mask corresponds to an interrupt input signal
*		that is connected to the interrupt controller (INT0 = LSB).
*		Only bits which are set in the mask will disable interrupts.
*
* @return	None.
*
* @note		C-style signature:
*		void XIOModule_DisableIntr(UINTPTR BaseAddress,
*                                          u32 DisableMask);
*
*****************************************************************************/
#define XIOModule_DisableIntr(BaseAddress, DisableMask) \
	XIomodule_Out32((BaseAddress) + XIN_IER_OFFSET, ~(DisableMask))

/****************************************************************************/
/**
*
* Acknowledge specific interrupt(s) in the interrupt controller.
*
* @param	BaseAddress is the base address of the device
* @param	AckMask is the 32-bit value to write to the acknowledge
*		register. Each bit of the mask corresponds to an interrupt
*               input signal that is connected to the interrupt controller
*               (INT0 =	LSB).  Only the bits which are set in the mask will
*               acknowledge interrupts.
*
* @return	None.
*
* @note		C-style signature:
*		void XIOModule_AckIntr(UINTPTR BaseAddress, u32 AckMask);
*
*****************************************************************************/
#define XIOModule_AckIntr(BaseAddress, AckMask) \
	XIomodule_Out32((BaseAddress) + XIN_IAR_OFFSET, (AckMask))

/****************************************************************************/
/**
*
* Get the interrupt status from the interrupt controller which indicates
* which interrupts are active and enabled.
*
* @param	BaseAddress is the base address of the device
*
* @return	The 32-bit contents of the interrupt status register. Each bit
*		corresponds to an interrupt input signal that is connected to
*		the interrupt controller (INT0 = LSB). Bits which are set
*		indicate an active interrupt which is also enabled.
*
* @note		C-style signature:
*		u32 XIOModule_GetIntrStatus(UINTPTR BaseAddress);
*
*****************************************************************************/
#define XIOModule_GetIntrStatus(BaseAddress) \
		(XIomodule_In32((BaseAddress) + XIN_IPR_OFFSET))


/****************************************************************************/
/**
*
* Get the contents of the UART status register. Use the XUL_SR_* constants
* defined above to interpret the bit-mask returned.
*
* @param	BaseAddress is the  base address of the device
*
* @return	A 32-bit value representing the contents of the status
*               register.
*
* @note		C-style Signature:
*		u32 XIOModule_GetStatusReg(UINTPTR BaseAddress);
*
*****************************************************************************/
#define XIOModule_GetStatusReg(BaseAddress) \
		XIomodule_In32((BaseAddress) + XUL_STATUS_REG_OFFSET)

/****************************************************************************/
/**
*
* Check to see if the UART receiver has data.
*
* @param	BaseAddress is the  base address of the device
*
* @return	TRUE if the receiver is empty, FALSE if there is data present.
*
* @note		C-style Signature:
*		int XIOModule_IsReceiveEmpty(UINTPTR BaseAddress);
*
*****************************************************************************/
#define XIOModule_IsReceiveEmpty(BaseAddress) \
  ((XIOModule_GetStatusReg((BaseAddress)) & XUL_SR_RX_FIFO_VALID_DATA) != \
	XUL_SR_RX_FIFO_VALID_DATA)


/****************************************************************************/
/**
*
* Check to see if the transmitter is full.
*
* @param	BaseAddress is the  base address of the device
*
* @return	TRUE if the transmitter is full, FALSE otherwise.
*
* @note		C-style Signature:
* 		int XIOModule_IsTransmitFull(UINTPTR BaseAddress);
*
*****************************************************************************/
#define XIOModule_IsTransmitFull(BaseAddress) \
	((XIOModule_GetStatusReg((BaseAddress)) & XUL_SR_TX_FIFO_FULL) == \
	  XUL_SR_TX_FIFO_FULL)


/****************************************************************************/
/**
*
* Write a value to a GPO register. A 32 bit write is performed. If the
* GPO component is implemented in a smaller width, only the least
* significant data is written.
*
* @param	BaseAddress is the base address of the GPO device.
* @param	RegOffset is the register offset from the base to write to.
* @param	Data is the data written to the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XIOModule_WriteReg(UINTPTR BaseAddress,
*                                       unsigned RegOffset, u32 Data)
*
****************************************************************************/
#define XIOModule_WriteReg(BaseAddress, RegOffset, Data) \
	XIomodule_Out32((BaseAddress) + (RegOffset), (u32)(Data))

/****************************************************************************/
/**
*
* Read a value from a GPI register. A 32 bit read is performed. If the
* GPI component is implemented in a smaller width, only the least
* significant data is read from the register. The most significant data
* will be read as 0.
*
* @param	BaseAddress is the base address of the GPI device.
* @param	RegOffset is the register offset from the base to read from.
*
* @return	Data read from the register.
*
* @note		C-style signature:
*		u32 XIOModule_ReadReg(UINTPTR BaseAddress, unsigned RegOffset)
*
******************************************************************************/
#define XIOModule_ReadReg(BaseAddress, RegOffset) \
	XIomodule_In32((BaseAddress) + (RegOffset))


/************************** Function Prototypes ******************************/

/*
 * UART standard in and standard out handlers, to be connected to generic
 * I/O handling code.
 */
void XIOModule_SendByte(UINTPTR BaseAddress, u8 Data);
u8 XIOModule_RecvByte(UINTPTR BaseAddress);


/*
 * Interrupt controller handlers, to be connected to processor exception
 * handling code.
 */
void XIOModule_LowLevelInterruptHandler(void);
void XIOModule_DeviceInterruptHandler(void *DeviceId);

/* Various configuration functions */
void XIOModule_SetIntrSvcOption(UINTPTR BaseAddress, s32 Option);

void XIOModule_RegisterHandler(UINTPTR BaseAddress,
			       s32 InterruptId,
			       XInterruptHandler Handler,
			       void *CallBackRef);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
