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
* @file xuartpsv.h
* @addtogroup uartpsv_v1_0
* @{
* @details
*
* This driver supports the following features:
*
* - Dynamic data format (baud rate, data bits, stop bits, parity)
* - Polled mode
* - Interrupt driven mode
* - Transmit and receive FIFOs (32 byte FIFO depth)
* - Access to the external modem control lines
*
* <b>Initialization & Configuration</b>
*
* The XUartPsv_Config structure is used by the driver to configure itself.
* Fields inside this structure are properties of XUartPsv based on its
* hardware build.
*
* To support multiple runtime loading and initialization strategies employed
* by various operating systems, the driver instance can be initialized in
* the following way:
*
*   - XUartPsv_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr) - Uses a
*	 configuration structure provided by the caller. If running in a
*	 system with address translation, the parameter EffectiveAddr should
*	 be the virtual address.
*
* <b>Baud Rate</b>
*
* The UART has an internal baud rate generator, which furnishes the baud rate
* clock for both the receiver and the transmitter. Ther input clock frequency
* can be either the master clock or the master clock divided by 8, configured
* through the mode register.
*
* Accompanied with the baud rate divider register, the baud rate is determined
* by:
* <pre>
*	baud_rate = input_clock / (bgen * (bdiv + 1)
* </pre>
* where bgen is the value of the baud rate generator, and bdiv is the value of
* baud rate divider.
*
* <b>Interrupts</b>
*
* The FIFOs are not flushed when the driver is initialized, but a function is
* provided to allow the user to reset the FIFOs if desired.
*
* The driver defaults to no interrupts at initialization such that interrupts
* must be enabled if desired. An interrupt is generated for one of the
* following conditions.
*
* - A change in the modem signals
* - Data in the receive FIFO for a configuable time without receiver activity
* - A parity error
* - A framing error
* - An overrun error
* - Transmit FIFO is full
* - Transmit FIFO is empty
* - Receive FIFO is full
* - Receive FIFO is empty
* - Data in the receive FIFO equal to the receive threshold
*
* The application can control which interrupts are enabled using the
* XUartPsv_SetInterruptMask() function.
*
* In order to use interrupts, it is necessary for the user to connect the
* driver interrupt handler, XUartPsv_InterruptHandler(), to the interrupt
* system of the application. A separate handler should be provided by the
* application to communicate with the interrupt system, and conduct
* application specific interrupt handling. An application registers its own
* handler through the XUartPsv_SetHandler() function.
*
* <b>Data Transfer</b>
*
* The functions, XUartPsv_Send() and XUartPsv_Recv(), are provided in the
* driver to allow data to be sent and received. They can be used in either
* polled or interrupt mode.
*
* @note
*
* The default configuration for the UART after initialization is:
*
* - 9,600 bps or XPAR_DFT_BAUDRATE if defined
* - 8 data bits
* - 1 stop bit
* - no parity
* - FIFO's are enabled with a receive threshold of 8 bytes
* - The RX timeout is enabled with a timeout of 1 (4 char times)
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

#ifndef XUARTPSV_H		/* prevent circular inclusions */
#define XUARTPSV_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xuartpsv_hw.h"
#include "xplatform_info.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants indicate the max and min baud rates and these
 * numbers are based only on the testing that has been done. The hardware
 * is capable of other baud rates.
 */
#define XUARTPSV_MAX_RATE  	921600U
#define XUARTPSV_MIN_RATE  	110U

#define XUARTPSV_DFT_BAUDRATE	115200U 	/* Default baud rate */

/** @name Configuration options
 * @{
 */
/**
 * These constants specify the options that may be set or retrieved
 * with the driver, each is a unique bit mask such that multiple options
 * may be specified. These constants indicate the available options
 * in active state.
 *
 */

#define XUARTPSV_OPTION_SET_BREAK	0x0080U	/**< Starts break transmission */
#define XUARTPSV_OPTION_STOP_BREAK	0x0040U	/**< Stops break transmission */
#define XUARTPSV_OPTION_RESET_TMOUT	0x0020U
										/**< Reset the receive timeout */
#define XUARTPSV_OPTION_RESET_TX	0x0010U	/**< Reset the transmitter */
#define XUARTPSV_OPTION_RESET_RX	0x0008U	/**< Reset the receiver */
#define XUARTPSV_OPTION_ASSERT_RTS	0x0004U	/**< Assert the RTS bit */
#define XUARTPSV_OPTION_ASSERT_DTR	0x0002U	/**< Assert the DTR bit */
#define XUARTPSV_OPTION_SET_FCM	0x0001U	/**< Turn on flow control mode */
/*@}*/


/** @name Channel Operational Mode
 *
 * The UART can operate in one of four modes: Normal, Local Loopback, Remote
 * Loopback, or automatic echo.
 *
 * @{
 */

#define XUARTPSV_OPER_MODE_NORMAL  	(u8)0x00U/**< Normal Mode */
#define XUARTPSV_OPER_MODE_LOCAL_LOOP	(u8)0x02U/**< Local Loop back Mode */

/* @} */

/** @name Data format values
 *
 * These constants specify the data format that the driver supports.
 * The data format includes the number of data bits, the number of stop
 * bits and parity.
 *
 * @{
 */
#define XUARTPSV_FORMAT_8_BITS 	0U	/**< 8 data bits */
#define XUARTPSV_FORMAT_7_BITS 	2U	/**< 7 data bits */
#define XUARTPSV_FORMAT_6_BITS 	3U	/**< 6 data bits */

#define XUARTPSV_FORMAT_NO_PARITY  	4U	/**< No parity */
#define XUARTPSV_FORMAT_MARK_PARITY	3U	/**< Mark parity */
#define XUARTPSV_FORMAT_SPACE_PARITY	2U	/**< parity */
#define XUARTPSV_FORMAT_ODD_PARITY 	1U	/**< Odd parity */
#define XUARTPSV_FORMAT_EVEN_PARITY	0U	/**< Even parity */

#define XUARTPSV_FORMAT_2_STOP_BIT 	2U	/**< 2 stop bits */
#define XUARTPSV_FORMAT_1_5_STOP_BIT	1U	/**< 1.5 stop bits */
#define XUARTPSV_FORMAT_1_STOP_BIT 	0U	/**< 1 stop bit */
/*@}*/

/** @name Callback events
 *
 * These constants specify the handler events that an application can handle
 * using its specific handler function. Note that these constants are not bit
 * mask, so only one event can be passed to an application at a time.
 *
 * @{
 */
#define XUARTPSV_EVENT_RECV_DATA	1U /**< Data receiving done */
#define XUARTPSV_EVENT_RECV_TOUT	2U /**< A receive timeout occurred */
#define XUARTPSV_EVENT_SENT_DATA	3U /**< Data transmission done */
#define XUARTPSV_EVENT_RECV_ERROR	4U /**< A receive error detected */
#define XUARTPSV_EVENT_MODEM   	5U /**< Modem status changed */
#define XUARTPSV_EVENT_PARE_FRAME_BRKE 6U
					/**< A receive parity, frame,break error detected */
#define XUARTPSV_EVENT_RECV_ORERR	7U /**< A receive overrun error detected */
/*@}*/

/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u16 DeviceId;				/**< Unique ID  of device */
	u32 BaseAddress;			/**< Base address of device (IPIF) */
	u32 InputClockHz;			/**< Input clock frequency */
	s32 ModemPinsConnected; 	/**< Specifies whether modem pins are
								  *  connected to MIO or FMIO */
} XUartPsv_Config;

/**
 * Keep track of state information about a data buffer in the
 * interrupt mode.
 */
typedef struct {
	u8 *NextBytePtr;
	u32 RequestedBytes;
	u32 RemainingBytes;
} XUartPsvBuffer;

/**
 * Keep track of data format setting of a device.
 */
typedef struct {
	u32 BaudRate;			/**< In bps, ie 1200 */
	u32 DataBits;			/**< Number of data bits */
	u32 Parity; 			/**< Parity */
	u8 StopBits;			/**< Number of stop bits */
} XUartPsvFormat;

/*****************************************************************************/
/**
 * This data type defines a handler that an application defines to communicate
 * with interrupt system to retrieve state information about an application.
 *
 * @param	CallBackRef is a callback reference passed in by the upper layer
 *		when setting the handler, and is passed back to the upper layer
 *		when the handler is called. It is used to find the device driver
 *		instance.
 * @param	Event contains one of the event constants indicating events that
 *		have occurred.
 * @param	EventData contains the number of bytes sent or received at the
 *		time of the call for send and receive events and contains the
 *		modem status for modem events.
 *
 *****************************************************************************/
typedef void (*XUartPsv_Handler) (void *CallBackRef, u32 Event,u32 EventData);

/**
 * The XUartPsv driver instance data structure. A pointer to an instance data
 * structure is passed around by functions to refer to a specific driver
 * instance.
 */
typedef struct {
	XUartPsv_Config Config; 	/**< Configuration data structure */
	u32 InputClockHz;			/**< Input clock frequency */
	u32 IsReady;				/**< Device is initialized and ready */
	u32 BaudRate;				/**< Current baud rate */

	XUartPsvBuffer SendBuffer;
	XUartPsvBuffer ReceiveBuffer;

	XUartPsv_Handler Handler;
	void *CallBackRef;			/**< Callback reference for event handler */
} XUartPsv;


/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* Get the UART Channel Status Register.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
*
* @return	The value read from the register.
*
* @note 	C-Style signature:
*			u16 XUartPsv_GetChannelStatus(XUartPsv *InstancePtr)
*
******************************************************************************/
#define XUartPsv_GetChannelStatus(InstancePtr) \
			Xil_In32(((InstancePtr)->Config.BaseAddress) + \
			(u32)XUARTPSV_UARTFR_OFFSET)

/*****************************************************************************/
/**
* Get the UART Mode Control Register.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
*
* @return	The value read from the register.
*
* @note 	C-Style signature:
*			u32 XUartPsv_GetControl(XUartPsv *InstancePtr)
*
******************************************************************************/
#define XUartPsv_GetModeControl(InstancePtr) \
			Xil_In32(((InstancePtr)->Config.BaseAddress) + \
			(u32)XUARTPSV_UARTCR_OFFSET)

/*****************************************************************************/
/**
* Set the UART Mode Control Register.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note 	C-Style signature:
*			void XUartPsv_SetModeControl(XUartPsv *InstancePtr,
*			u16 RegisterValue)
*
******************************************************************************/
#define XUartPsv_SetModeControl(InstancePtr, RegisterValue) \
			Xil_Out32(((InstancePtr)->Config.BaseAddress) + \
			(u32)XUARTPSV_UARTCR_OFFSET, (u32)(RegisterValue))

/*****************************************************************************/
/**
* Enable the transmitter and receiver of the UART.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
*
* @return	None.
*
* @note 	C-Style signature:
*			void XUartPsv_EnableUart(XUartPsv *InstancePtr)
*
******************************************************************************/
#define XUartPsv_EnableUart(InstancePtr) \
			Xil_Out32(((InstancePtr)->Config.BaseAddress + \
			(u32)XUARTPSV_UARTCR_OFFSET), \
			(Xil_In32((InstancePtr)->Config. \
			BaseAddress + (u32)XUARTPSV_UARTCR_OFFSET) | \
			(u32)XUARTPSV_UARTCR_UARTEN))

/*****************************************************************************/
/**
* Disable the transmitter and receiver of the UART.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
*
* @return	None.
*
* @note 	C-Style signature:
*			void XUartPsv_DisableUart(XUartPsv *InstancePtr)
*
******************************************************************************/
#define XUartPsv_DisableUart(InstancePtr) \
			Xil_Out32(((InstancePtr)->Config.BaseAddress + \
			(u32)XUARTPSV_UARTCR_OFFSET), \
			(Xil_In32((InstancePtr)->Config. \
			BaseAddress + (u32)XUARTPSV_UARTCR_OFFSET) & \
			(u32)(~XUARTPSV_UARTCR_UARTEN)))

/*****************************************************************************/
/**
* Determine if the transmitter FIFO is empty.
*
* @param	InstancePtr is a pointer to the XUartPsv instance.
*
* @return
*		- TRUE if a byte can be sent
*		- FALSE if the Transmitter Fifo is not empty
*
* @note 	C-Style signature:
*			u32 XUartPsv_IsTransmitEmpty(XUartPsv InstancePtr)
*
******************************************************************************/
#define XUartPsv_IsTransmitEmpty(InstancePtr) \
		((Xil_In32(((InstancePtr)->Config.BaseAddress) + \
		(u32)XUARTPSV_UARTFR_OFFSET) & (u32)XUARTPSV_UARTFR_TXFE) \
		== (u32)XUARTPSV_UARTFR_TXFE)

/************************** Function Prototypes ******************************/

/* Static lookup function implemented in xuartpsv_sinit.c */
XUartPsv_Config *XUartPsv_LookupConfig(u16 DeviceId);

/* Interface functions implemented in xuartpsv.c */
s32 XUartPsv_CfgInitialize(XUartPsv *InstancePtr,
			XUartPsv_Config * Config, u32 EffectiveAddr);

u32 XUartPsv_Send(XUartPsv *InstancePtr,u8 *BufferPtr,
			u32 NumBytes);

u32 XUartPsv_Recv(XUartPsv *InstancePtr,u8 *BufferPtr,
			u32 NumBytes);

s32 XUartPsv_SetBaudRate(XUartPsv *InstancePtr, u32 BaudRate);

/* Options functions in xuartpsv_options.c */
void XUartPsv_SetOptions(XUartPsv *InstancePtr, u16 Options);

u16 XUartPsv_GetOptions(XUartPsv *InstancePtr);

void XUartPsv_SetFifoThreshold(XUartPsv *InstancePtr, u8 TriggerLevel);

u8 XUartPsv_GetFifoThreshold(XUartPsv *InstancePtr);

u16 XUartPsv_GetModemStatus(XUartPsv *InstancePtr);

u32 XUartPsv_IsSending(XUartPsv *InstancePtr);

u8 XUartPsv_GetOperMode(XUartPsv *InstancePtr);

void XUartPsv_SetOperMode(XUartPsv *InstancePtr, u8 OperationMode);

u8 XUartPsv_GetFlowDelay(XUartPsv *InstancePtr);

void XUartPsv_SetFlowDelay(XUartPsv *InstancePtr, u8 FlowDelayValue);

u8 XUartPsv_GetRecvTimeout(XUartPsv *InstancePtr);

void XUartPsv_SetRecvTimeout(XUartPsv *InstancePtr, u8 RecvTimeout);

s32 XUartPsv_SetDataFormat(XUartPsv *InstancePtr,
			XUartPsvFormat * FormatPtr);

void XUartPsv_GetDataFormat(XUartPsv *InstancePtr,
			XUartPsvFormat * FormatPtr);

/* interrupt functions in xuartpsv_intr.c */
u32 XUartPsv_GetInterruptMask(XUartPsv *InstancePtr);

void XUartPsv_SetInterruptMask(XUartPsv *InstancePtr, u32 Mask);

void XUartPsv_InterruptHandler(XUartPsv *InstancePtr);

void XUartPsv_SetHandler(XUartPsv *InstancePtr,
			XUartPsv_Handler FuncPtr, void *CallBackRef);

/* self-test functions in xuartpsv_selftest.c */
s32 XUartPsv_SelfTest(XUartPsv *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
