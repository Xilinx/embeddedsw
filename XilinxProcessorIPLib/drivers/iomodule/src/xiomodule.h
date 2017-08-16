/******************************************************************************
*
* Copyright (C) 2011 - 2018 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xiomodule.h
* @addtogroup iomodule_v2_7
* @{
* @details
*
* The Xilinx IO Module driver component. This component supports the
* IO Module. The IO Module provides:
* <ol>
* <li>Universal Asynchronous Receiver Transmitter (UART)</li>
* <li>Fixed Interval Timer (FIT)</li>
* <li>Programmable Interval Timer (PIT)</li>
* <li>General Purpose Output (GPO)</li>
* <li>General Purpose Input (GPI)</li>
* <li>Interrupt controller (INTC)</li>
* <li>IO bus (IO)</li>
* </ol>
*
* <b>Universal Asynchronous Receiver Transmitter (UART):</b>
* This UART is a minimal hardware implementation with minimal features. Most
* of the features, including parity, and number of data bits are only
* configurable when the hardware device is built, rather than at run time by
* software. The UART has an internal baud rate generator that is clocked at a
* specified input clock frequency. The baud rate can either be fixed, or
* programmable. Not all programmed baud rates can be generated from some clock
* frequencies. The requested baud rate is checked using the provided clock for
* the system, and checked against the acceptable error range. An error may be
* returned from some functions indicating the baud rate was in error because
* it could not be generated.
*
* The device has support for interrupts: a transmit interrupt, a receive
* interrupt, and an error interrupt. The interrupts are individually
* configurable when the hardware device is built.
*
* The driver defaults to no interrupts at initialization such that interrupts
* must be enabled if desired. An interrupt is generated when the transmitter
* transitions from having data to being empty, when any data is contained in
* the receive register, or when an error occurs in received data.
*
* In order to use interrupts, it's necessary for the user to connect the driver
* interrupt handler, XIOModule_InterruptHandler, to the interrupt system of the
* application.  This function does not save and restore the processor context
* such that the user must provide it.  Send and receive handlers may be set for
* the driver such that the handlers are called when transmit and receive
* interrupts occur.  The handlers are called from interrupt context and are
* designed to allow application specific processing to be performed.
*
* The functions, XIOModule_Send and XIOModule_Recv, are provided in the driver
* to allow data to be sent and received. They are designed to be used in
* polled or interrupt modes.
*
* The driver provides a status for each received byte indicating any parity
* frame or overrun error. The driver provides statistics which allow visibility
* into these errors.
*
* <b>Fixed Interval Timer (FIT):</b>
* The fixed interval timer supports the following features:
*   - Interrupt driven mode
*
* The timer has a hardware programmed time period, which can be configured to
* cause a periodic interrupt. The driver only provides the ability to handle
* such interrupts.
*
* <b>Programmable Interval Timer (PIT):</b>
* The programmable interval timer supports the following features:
*   - Polled mode
*   - Interrupt driven mode
*   - Enabling and disabling
*   - Automatic reload
*
* The timer operates in compare mode. The timer always counts down.
*
* Compare mode is typically used for creating a single time period or multiple
* repeating time periods in the auto reload mode, such as a periodic interrupt.
* When started, the timer loads an initial value, referred to as the load
* value, into the timer and starts counting down. The timer expires when it
* rolls under. An external Output signal may be configured such that a pulse
* is generated with this signal when it rolls under.
*
* The timer can be configured to cause an interrupt when the count reaches
* zero. An external output is also toggled when the count reaches zero.
*
* <b>Interrupts</b>
*
* It is the responsibility of the application to connect the interrupt
* handler of the timer to the interrupt source. The interrupt handler function,
* XIOModule_InterruptHandler, is visible such that the user can connect it to
* the interrupt source. Note that this interrupt handler does not provide
* interrupt context save and restore processing, the user must perform this
* processing.
*
* The driver services interrupts and passes timeouts to the upper layer
* software through callback functions. The upper layer software must register
* its callback functions during initialization. The driver requires callback
* functions for timers.
*
* <b>General Purpose Output (GPO):</b>
* The GPO has support for up to 32 I/O discrete outputs for each channel (128
* bits total).
*
* <b>IO Bus (IO):</b>
* The IO Bus provides a generic mechanism to extend the IO Module
* functionality by providing a memory mapped IO area. Reading and writing of
* byte, halfword and word data is provided.
*
* <b>General Purpose Input (GPI):</b>
* The GPI has support for up to 32 I/O discrete outputs for each channel (128
* bits total). An interrupt can be generated when any bit in a GPI changes.
*
* <b>Interrupt controller (INTC):</b>
* The interrupt controller driver uses the idea of priority for the various
* handlers. Priority is an integer within the range of 0 and 31 inclusive with
* 0 being the highest priority interrupt source.
*
* The Xilinx interrupt controller supports the following features:
*
*   - specific individual interrupt enabling/disabling
*   - specific individual interrupt acknowledging
*   - attaching specific callback function to handle interrupt source
*   - master enable/disable
*   - single callback per interrupt or all pending interrupts handled for
*     each interrupt of the processor
*
* The acknowledgement of the interrupt within the interrupt controller is
* selectable, either prior to the device's handler being called or after
* the handler is called. This is necessary to support interrupt signal inputs
* which are either edge or level signals.  Edge driven interrupt signals
* require that the interrupt is acknowledged prior to the interrupt being
* serviced in order to prevent the loss of interrupts which are occurring
* extremely close together.  A level driven interrupt input signal requires
* the interrupt to acknowledged after servicing the interrupt to ensure that
* the interrupt only generates a single interrupt condition.
*
* Details about connecting the interrupt handler of the driver are contained
* in the source file specific to interrupt processing, xiomodule_intr.c.
*
* This driver is intended to be RTOS and processor independent.  It works with
* physical addresses only.  Any needs for dynamic memory management, threads
* or thread mutual exclusion, virtual memory, or cache control must be
* satisfied by the layer above this driver.
*
* <b>Interrupt Vector Tables</b>
*
* The interrupt vector table for each interrupt controller device is declared
* statically in xiomodule_g.c within the configuration data for each instance.
* The device ID of the interrupt controller device is used by the driver as a
* direct index into the configuration data table - to retrieve the vector table
* for an instance of the interrupt controller. The user should populate the
* vector table with handlers and callbacks at run-time using the
* XIOModule_Connect() and XIOModule_Disconnect() functions.
*
* Each vector table entry corresponds to a device that can generate an
* interrupt. Each entry contains an interrupt handler function and an argument
* to be passed to the handler when an interrupt occurs.  The tools default this
* argument to the base address of the interrupting device.  Note that the
* device driver interrupt handlers given in this file do not take a base
* address as an argument, but instead take a pointer to the driver instance.
* This means that although the table is created statically, the user must still
* use XIOModule_Connect() when the interrupt handler takes an argument other
* than the base address. This is only to say that the existence of the static
* vector tables should not mislead the user into thinking they no longer need
* to register/connect interrupt handlers with this driver.
*
* With fast interrupts enabled, the XIOModule_ConnectFastHandler() function
* should be used instead of XIOModule_Connect(). Note that the function pointer
* parameter Handler, must be a declared with the fast_interrupt attribute:
*
*   void Handler() __attribute__((fast_interrupt));
*
* The routine defined by XIOModule_Connect() can be used by setting normal
* interrupt mode, using XIOModule_SetNormalIntrMode().
*
* @note
*
* This API utilizes 32 bit I/O to the registers. With less than 32 bits, the
* unused bits from registers are read as zero and written as don't cares.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a sa   07/15/11 First release
* 1.01a sa   04/10/12 Updated with fast interrupt
* 1.02a sa   07/25/12 Updated with GPI interrupts and programmable baudrate
* 2.0   adk  19/12/13 Updated as per the New Tcl API's
* 2.1   bss  05/02/14 Added PitMask in XIOModule_Config.(CR #794167)
* 2.2	nsk  07/21/15 Updated iomodule_define_vector_table by removing
*		      absoluted hsi commands like xget_handle.CR#865544.
*		      modified generate proc to get canonical definitions
*		      in xparameters.h
* 2.2   nsk  08/06/15 Updated XIOModule_Uart_InterruptHandler function
*		      in xiomodule_uart_intr.c file to read Status register
*		      instead of reading Interrupt Pending register.
*		      (CR #862715).
* 2.2	nsk  08/07/15 Updated iomodule_define_vector_table in iomodule.tcl
*		      to handle External vector interrupts.(CR #871572)
* 2.2	nsk  08/19/15 Modified iomodule_define_vector_table in iomodule.tcl
*		      to handle,if iomodule doesn't have interrupts enabled
*		      (CR #876507).
* 2.2	nsk  09/02/15 Modified iomodule_define_vector_table in iomodule.tcl
*		      when no external interrupts are used.(CR #878782)
* 2.3   nsk  11/05/15 Updated xdefine_canonical_xpars in iomodule.tcl to
*                     Generate canonical definitions, whose canonical
*                     name is not the same as hardware instance name.
*                     (CR #876604)
* 2.4   ms   01/23/17 Added xil_printf statement in main function for all
*                     examples to ensure that "Successfully ran" and "Failed"
*                     strings are available in all examples. This is a fix
*                     for CR-965028.
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
* 2.5   ms   04/18/17 Modified tcl file to add suffix U for all macros
*                     definitions of iomodule in xparameters.h
*       ms   08/07/17 Fixed compilation warnings in xiomodule.c,
*                     xiomodule_l.c, xiomodule_uart.c
* 2.7   sa   11/09/18  Updated XIOModule_Config to support 64 bit base
*                      addresses.
* </pre>
*
******************************************************************************/

#ifndef IOMODULE_H			/* prevent circular inclusions */
#define IOMODULE_H			/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xiomodule_l.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**
 * @name Configuration options
 * These options are used in XIOModule_SetOptions() to configure the
 * device.
 * @{
 */

/**
 * Used to configure the Programmable Interval Timer.
 * <pre>
 * XTC_INT_MODE_OPTION		Dummy compatibility option. Enable interrupt
 *                              output.
 * XTC_AUTO_RELOAD_OPTION	In compare mode, configures the timer to reload
 *                              from the load value. The default mode causes
 *                              the timer to hold after it rolls under.
 * </pre>
 */
#define XTC_INT_MODE_OPTION		0
#define XTC_AUTO_RELOAD_OPTION		0x00000002UL

/**
 * Used to configure the Interrupt Controller.
 * <pre>
 * XIN_SVC_SGL_ISR_OPTION	Service the highest priority pending interrupt
 *				and then return.
 * XIN_SVC_ALL_ISRS_OPTION	Service all of the pending interrupts and then
 *				return.
 * </pre>
 */
#define XIN_SVC_SGL_ISR_OPTION  1UL
#define XIN_SVC_ALL_ISRS_OPTION 2UL
/*@}*/


/**
 * @name Hardware configuration mnemonics
 * These mnemonics are used when accessing hardware configuration parameters.
 * @{
 */

/**
 * Mnemonics for the Programmable Interval Timer hardware configuration.
 * <pre>
 * XTC_PRESCALER_*	Define the prescaler configured in hardware.
 * </pre>
 */
#define XTC_PRESCALER_NONE		0
#define XTC_PRESCALER_FIT1		1
#define XTC_PRESCALER_FIT2		2
#define XTC_PRESCALER_FIT3		3
#define XTC_PRESCALER_FIT4		4
#define XTC_PRESCALER_PIT1		5
#define XTC_PRESCALER_PIT2		6
#define XTC_PRESCALER_PIT3		7
#define XTC_PRESCALER_PIT4		8
#define XTC_PRESCALER_EXTERNAL		9
/*@}*/

#define XIOMODULE_STANDARD_VECTOR_ADDRESS_WIDTH	32U

/**************************** Type Definitions *******************************/

/**
 * Callback function.  The first argument is a callback reference passed in by
 * the upper layer when setting the callback functions, and passed back to the
 * upper layer when the callback is invoked.
 * The second argument is the ByteCount which is the number of bytes that
 * actually moved from/to the buffer provided in the _Send/_Receive call.
 */
typedef void (*XIOModule_Handler)(void *CallBackRef,
                                  unsigned int ByteCount);

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u16 DeviceId;			     /**< Unique ID  of device       */
	UINTPTR BaseAddress;		     /**< Unique identifier          */
	UINTPTR IoBaseAddress;		     /**< IO Bus Base Address        */
	u32 FastIntr;			     /**< Fast Interrupt enabled     */
	UINTPTR BaseVector;		     /**< Relocatable base vector    */
	u8 VectorAddrWidth;		     /**< Width of vector address */
	u32 AckBeforeService;		     /**< Ack location per interrupt */
	u32 Options;			     /**< Device options             */
	u32 InputClockHz;		     /**< Input clock frequency (Hz) */
	u32 BaudRate;			     /**< Current baud rate          */
	u8  PitUsed[XTC_DEVICE_TIMER_COUNT]; /**< PIT is used                */
	u8  PitSize[XTC_DEVICE_TIMER_COUNT]; /**< PIT timer counter size     */
	u32 PitMask[XTC_DEVICE_TIMER_COUNT]; /**< PIT timer counter size     */
	u8  PitPrescaler[XTC_DEVICE_TIMER_COUNT]; /**< PIT prescaler         */
	u8  PitReadable[XTC_DEVICE_TIMER_COUNT];  /**< PIT readable          */
	u32 GpoInit[XGPO_DEVICE_COUNT];	     /**< GPO initial value          */
	XIOModule_VectorTableEntry
		HandlerTable[XPAR_IOMODULE_INTC_MAX_INTR_SIZE];
			      /**< Static vector table of interrupt handlers */
} XIOModule_Config;

/**
 * Statistics for the UART
 */
typedef struct {
	u32 TransmitInterrupts;	   /**< Number of transmit interrupts    */
	u32 ReceiveInterrupts;	   /**< Number of receive interrupts     */
	u32 CharactersTransmitted; /**< Number of characters transmitted */
	u32 CharactersReceived;	   /**< Number of characters received    */
	u32 ReceiveOverrunErrors;  /**< Number of receive overruns       */
	u32 ReceiveParityErrors;   /**< Number of receive parity errors  */
	u32 ReceiveFramingErrors;  /**< Number of receive framing errors */
} XIOModule_Uart_Stats;

/**
 * The following data type is used to manage the buffers that are handled
 * when sending and receiving UART data in the interrupt mode. It is intended
 * for internal use only.
 */
typedef struct {
	u8 *NextBytePtr;
	unsigned int RequestedBytes;
	unsigned int RemainingBytes;
} XIOModule_Buffer;

/**
 * Signature for the timer callback function.
 *
 * @param	CallBackRef is a callback reference passed in by the upper
 *              layer when setting the callback functions, and passed back to
 *              the upper layer when the callback is invoked. Its type is
 *		unimportant to the driver, so it is a void pointer.
 * @param 	TimerNumber is the number of the timer within the device. The
 *              device typically contains from one to four timers. The timer
 *              number is a zero based number with a range of 0 to
 *              (XTC_DEVICE_TIMER_COUNT - 1).
 */
typedef void (*XIOModule_Timer_Handler) (void *CallBackRef, u8 TimerNumber);

/**
 * Programmable Interval Timer statistics
 */
typedef struct {
	u32 Interrupts;	 /**< Number of timer interrupts that have occurred */
} XIOModule_Timer_Stats;

/**
 * The XIOModule driver instance data. The user is required to allocate a
 * variable of this type for every iomodule device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	UINTPTR BaseAddress;		 /**< Base address of registers      */
	u32 IsReady;			 /**< Device initialized and ready   */
	u32 IsStarted;			 /**< Device has been started        */
	XIOModule_Config *CfgPtr;	 /**< Pointer to inst config entry   */

	XIOModule_Uart_Stats Uart_Stats; /**< UART Statistics                */
	XIOModule_Buffer SendBuffer;     /**< UART Send buffer               */
	XIOModule_Buffer ReceiveBuffer;  /**< UART Receive buffer            */
	XIOModule_Handler RecvHandler;   /**< UART Receive Handler           */
	void *RecvCallBackRef;		 /**< Callback ref for recv handler  */
	XIOModule_Handler SendHandler;   /**< UART Send Handler              */
	void *SendCallBackRef;		 /**< Callback ref for send handler  */
	u32 CurrentUBRR;		 /**< UART Baud Rate Register value  */

	u32 UnhandledInterrupts;	 /**< Iomodule Interrupt Statistics  */
	u32 CurrentIER;			 /**< Interrupt Enable Register value*/
	u32 CurrentIMR;			 /**< Interrupt Mode Register value  */

	XIOModule_Timer_Stats Timer_Stats[XTC_DEVICE_TIMER_COUNT];
					 /**< Timer statistics               */
	u32 CurrentTLR[XTC_DEVICE_TIMER_COUNT];
					 /**< Timer Load Register values     */
	u8 CurrentTCSR[XTC_DEVICE_TIMER_COUNT];
					 /**< Timer Control Register values  */
	XIOModule_Timer_Handler Handler; /**< Timer Callback function        */
	void *CallBackRef;		 /**< Timer Callback handler ref     */

	u32 GpoValue[XGPO_DEVICE_COUNT]; /**< GPO current value              */

	UINTPTR IoBaseAddress;		 /**< Base address of IO Bus         */
} XIOModule;

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/*
 * Required functions in xiomodule.c
 */
int XIOModule_Initialize(XIOModule * InstancePtr, u16 DeviceId);
int XIOModule_Timer_Initialize(XIOModule * InstancePtr, u16 DeviceId);

int XIOModule_Start(XIOModule * InstancePtr);
void XIOModule_Stop(XIOModule * InstancePtr);

int XIOModule_Connect(XIOModule * InstancePtr, u8 Id,
                      XInterruptHandler Handler, void *CallBackRef);
void XIOModule_Disconnect(XIOModule * InstancePtr, u8 Id);

void XIOModule_Enable(XIOModule * InstancePtr, u8 Id);
void XIOModule_Disable(XIOModule * InstancePtr, u8 Id);

void XIOModule_Acknowledge(XIOModule * InstancePtr, u8 Id);

XIOModule_Config *XIOModule_LookupConfig(u16 DeviceId);

int XIOModule_ConnectFastHandler(XIOModule *InstancePtr, u8 Id,
				 XFastInterruptHandler Handler);

void XIOModule_SetNormalIntrMode(XIOModule *InstancePtr, u8 Id);


/*
 * API Basic functions for GPI and GPO implemented in xiomodule.c
 */
u32 XIOModule_DiscreteRead(XIOModule *InstancePtr, unsigned Channel);
void XIOModule_DiscreteWrite(XIOModule *InstancePtr,
			     unsigned Channel,
			     u32 Mask);

/*
 * API Functions for GPI and GPO implemented in xiomodule_extra.c
 */
void XIOModule_DiscreteSet(XIOModule *InstancePtr, unsigned Channel, u32 Mask);
void XIOModule_DiscreteClear(XIOModule *InstancePtr,
			     unsigned Channel,
			     u32 Mask);


/*
 * Required functions, in file xiomodule_uart.c
 */
int XIOModule_CfgInitialize(XIOModule *InstancePtr,
                            XIOModule_Config *Config,
                            u32 EffectiveAddr);

void XIOModule_ResetFifos(XIOModule *InstancePtr); /* Dummy */

unsigned int XIOModule_Send(XIOModule *InstancePtr, u8 *DataBufferPtr,
                            unsigned int NumBytes);
unsigned int XIOModule_Recv(XIOModule *InstancePtr, u8 *DataBufferPtr,
                            unsigned int NumBytes);

int XIOModule_IsSending(XIOModule *InstancePtr);

int XIOModule_SetBaudRate(XIOModule *InstancePtr, u32 BaudRate);

/*
 * Functions for statistics, in file xiomodule_stats.c
 */
void XIOModule_GetStats(XIOModule *InstancePtr,
                        XIOModule_Uart_Stats *StatsPtr);
void XIOModule_ClearStats(XIOModule *InstancePtr);

/*
 * Interrupt functions in xiomodule_intr.c
 */
void XIOModule_VoidInterruptHandler();
void XIOModule_InterruptHandler(XIOModule * InstancePtr);

void XIOModule_Uart_EnableInterrupt(XIOModule *InstancePtr);
void XIOModule_Uart_DisableInterrupt(XIOModule *InstancePtr);

void XIOModule_SetRecvHandler(XIOModule *InstancePtr, XIOModule_Handler FuncPtr,
				void *CallBackRef);
void XIOModule_SetSendHandler(XIOModule *InstancePtr, XIOModule_Handler FuncPtr,
				void *CallBackRef);

void XIOModule_Uart_InterruptHandler(XIOModule *InstancePtr);

/*
 * Options functions in xiomodule_options.c
 */
int XIOModule_SetOptions(XIOModule * InstancePtr, u32 Options);
u32 XIOModule_GetOptions(XIOModule * InstancePtr);

/*
 * Self-test functions in xiomodule_selftest.c
 */
int XIOModule_SelfTest(XIOModule * InstancePtr);



/*
 * Required functions, in file xiomodule.c
 */
void XIOModule_Timer_Start(XIOModule * InstancePtr, u8 TimerNumber);
void XIOModule_Timer_Stop(XIOModule * InstancePtr, u8 TimerNumber);
u32 XIOModule_GetValue(XIOModule * InstancePtr, u8 TimerNumber);
void XIOModule_SetResetValue(XIOModule * InstancePtr, u8 TimerNumber,
			     u32 ResetValue);
u32 XIOModule_GetCaptureValue(XIOModule * InstancePtr, u8 TimerNumber);
int XIOModule_IsExpired(XIOModule * InstancePtr, u8 TimerNumber);
void XIOModule_Reset(XIOModule * InstancePtr, u8 TimerNumber);

/*
 * Functions for options, in file xiomodule_options.c
 */
void XIOModule_Timer_SetOptions(XIOModule * InstancePtr, u8 TimerNumber,
                                u32 Options);
u32 XIOModule_Timer_GetOptions(XIOModule * InstancePtr, u8 TimerNumber);

/*
 * Functions for statistics, in file xiomodule_stats.c
 */
void XIOModule_Timer_GetStats(XIOModule * InstancePtr,
			      XIOModule_Timer_Stats * StatsPtr);
void XIOModule_Timer_ClearStats(XIOModule * InstancePtr);

/*
 * Functions for self-test, in file xiomodule_selftest.c
 */
int XIOModule_Timer_SelfTest(XIOModule * InstancePtr, u8 IOModuleNumber);

/*
 * Functions for interrupts, in file xiomodule_intr.c
 */
void XIOModule_SetHandler(XIOModule * InstancePtr,
			  XIOModule_Timer_Handler FuncPtr,
			  void *CallBackRef);
void XIOModule_Timer_InterruptHandler(void *InstancePtr);


/*
 * Basic functions for IO Bus read and write implemented in xiomodule.c
 */
u32 XIOModule_IoReadWord(XIOModule *InstancePtr, u32 ByteOffset);
u16 XIOModule_IoReadHalfword(XIOModule *InstancePtr, u32 ByteOffset);
u8  XIOModule_IoReadByte(XIOModule *InstancePtr, u32 ByteOffset);

void XIOModule_IoWriteWord(XIOModule *InstancePtr, u32 ByteOffset, u32 Data);
void XIOModule_IoWriteHalfword(XIOModule *InstancePtr, u32 ByteOffset, u16 Data);
void XIOModule_IoWriteByte(XIOModule *InstancePtr, u32 ByteOffset, u8 Data);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
