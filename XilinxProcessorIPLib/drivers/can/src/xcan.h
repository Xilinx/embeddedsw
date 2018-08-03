/******************************************************************************
*
* Copyright (C) 2005 - 2018 Xilinx, Inc.  All rights reserved.
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
* @file xcan.h
* @addtogroup can_v3_3
* @{
* @details
*
* The Xilinx CAN driver.  This driver supports the Xilinx CAN Controller.
*
* The CAN Controller supports the following features:
*   - Confirms to the ISO 11898-1, CAN 2.0A and CAN 2.0B standards.
*   - Supports both Standard (11 bit Identifier) and Extended (29 bit
*     Identifier) frames.
*   - Supports Bit Rates up to 1 Mbps.
*   - Transmit message object FIFO with a user configurable depth of up to 64
*     message objects.
*   - Transmit prioritization through one TX High Priority Buffer.
*   - Receive message object FIFO with a user configurable depth of up to 64
*     message objects.
*   - Acceptance filtering through a user configurable number of up to 4
*     acceptance filters.
*   - Loop Back mode for diagnostic applications.
*   - Maskable Error and Status Interrupts.
*   - Readable Error Counters.
*   - External PHY chip required.
*
* The device driver supports all the features listed above, if applicable.
*
* <b>Driver Description</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the CAN. The driver handles transmission and reception of
* CAN frames, as well as configuration of the controller. The driver is simply a
* pass-through mechanism between a protocol stack and the CAN. A single device
* driver can support multiple CANs.
*
* Since the driver is a simple pass-through mechanism between a protocol stack
* and the CAN, no assembly or disassembly of CAN frames is done at the
* driver-level. This assumes that the protocol stack passes a correctly
* formatted CAN frame to the driver for transmission, and that the driver
* does not validate the contents of an incoming frame
*
* <b>Operation Modes</b>
*
* The CAN controller supports the following modes of operation:
*   - <b>Configuration Mode</b>: In this mode the CAN timing parameters and
*     Baud Rate Pre-scalar parameters can be changed. In this mode the CAN
*     controller loses synchronization with the CAN bus and drives a
*     constant recessive bit on the bus line. The Error Counter Register are
*     reset. The CAN controller does not receive or transmit any messages even
*     if there are pending transmit requests from the TX FIFO or the TX High
*     Priority Buffer. the Storage FIFOs and the CAN configuration registers
*     are still accessible.
*   - <b>Normal Mode</b>:In Normal Mode the CAN controller participates in bus
*     communication, by transmitting and receiving messages.
*   - <b>Sleep Mode</b>: In Sleep Mode the CAN Controller does not transmit any
*     messages. However, if any other node transmits a message, then the CAN
*     Controller receives the transmitted message and exits from Sleep Mode.
*     If there are new transmission requests from either the TX FIFO or the
*     TX High Priority Buffer when the CAN Controller is in Sleep Mode, these
*     requests are not serviced, and the CAN Controller continues to remain in
*     Sleep Mode. Interrupts are generated when the CAN controller enters Sleep
*     mode or Wakes up from Sleep mode.
*   - <b>Loop Back Mode</b>: In Loop Back mode, the CAN controller transmits a
*     recessive bit stream on to the CAN Bus. Any message that is transmitted
*     is loop backed to the ‘rx’ line and acknowledged. The CAN controller
*     thus receives any message that it transmits. It does not participate in
*     normal bus communication and does not receive any messages that are
*     transmitted by other CAN nodes. This mode is used for diagnostic
*     purposes.
*
* <b>Buffer Alignment</b>
*
* It is important to note that frame buffers passed to the driver must be
* 32-bit aligned.
*
* <b>Receive Address Filtering</b>
*
* The device can be set to accept frames whose Identifiers match any of up to
* 4 filters set in the Acceptance Filter Mask/ID registers.
*
* The incoming Identifier is masked with the bits in the Acceptance Filter Mask
* Register. This value is compared with the result of masking the bits in the
* Acceptance Filter ID Register with the Acceptance Filter Mask Register. If
* both these values are equal, the message will be stored in the RX FIFO.
*
* Acceptance Filtering is performed by each of the defined acceptance filters.
* If the incoming identifier passes through any acceptance filter then the
* frame is stored in the RX FIFO.
*
* <b>PHY Communication</b>
*
* This driver does not provide any mechanism for directly programming PHY.
*
* <b>Interrupts</b>
*
* The driver has no dependencies on the interrupt controller. The driver
* provides an interrupt handler. User of this driver needs to provide
* callback functions. An interrupt handler example is available with
* the driver.
*
* <b>Virtual Memory</b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space and invoking function
* XCan_VmInitialize(), instead of XCan_Initialize(), to initialize the device
* at first.
*
* <b>Threads</b>
*
* This driver is not thread safe.  Any needs for threads or thread mutual
exclusion
* must be satisfied by the layer above this driver.
*
* <b>Device Reset</b>
*
* Bus Off interrupt that can occur in the device requires a device reset. The
* user is responsible for resetting the device and re-configuring it
* based on its needs (the driver does not save the current configuration). When
* integrating into an RTOS, these reset and re-configure obligations are
* taken care of by the OS adapter software if it exists for that RTOS.
*
* <b>Device Configuration</b>
*
* The device can be configured in various ways during the FPGA implementation
* process.  Configuration parameters are stored in the xcan_g.c files.
* A table is defined where each entry contains configuration information
* for a CAN device.  This information includes such things as the base address
* of the memory-mapped device, and the number of acceptance filters.
*
* <b>Asserts</b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b>Building the driver</b>
*
* The XCan driver is composed of several source files. This allows the user to
* build and link only those parts of the driver that are necessary.
* <br><br>
*
* <pre>
* Temp Change
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a xd   04/12/05 First release
* 1.10a mta  05/13/07 Updated to new coding style
* 1.11a sdm  08/22/08 Removed support for static interrupt handlers from the MDD
*		      file
* 2.00a ktn  10/22/09 Updated driver to use the HAL APIs/macros.
*		      The macros have been renamed to remove _m from the name in
*		      all the driver files.
* 3.0   adk  19/12/13 Updated as per the New Tcl API's
* 3.1	adk  20/10/15 Update the driver tcl to check for valid IP parameters.
*		      CR#910450.
* 3.2   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototype of XCan_VmInitialize API.
*       ms   01/23/17 Added xil_printf statement in main function for all
*                     examples to ensure that "Successfully ran" and "Failed"
*                     strings are available in all examples. This is a fix
*                     for CR-965028.
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
* 3.3   ask  08/01/18 Fixed Cppcheck and GCC warnings in can driver
* </pre>
*
******************************************************************************/

#ifndef XCAN_H			/* prevent circular inclusions */
#define XCAN_H			/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xcan_l.h"

/************************** Constant Definitions *****************************/

/** @name CAN operation modes
 *  @{
 */
#define XCAN_MODE_CONFIG	0x00000001 /**< Configuration mode */
#define XCAN_MODE_NORMAL	0x00000002 /**< Normal mode */
#define XCAN_MODE_LOOPBACK	0x00000004 /**< Loop Back mode */
#define XCAN_MODE_SLEEP		0x00000008 /**< Sleep mode */
/* @} */

/** @name Callback identifiers used as parameters to XCan_SetHandler()
 *  @{
 */
#define XCAN_HANDLER_SEND   1 /**< Handler type for frame sending interrupt */
#define XCAN_HANDLER_RECV   2 /**< Handler type for frame reception interrupt */
#define XCAN_HANDLER_ERROR  3 /**< Handler type for error interrupt */
#define XCAN_HANDLER_EVENT  4 /**< Handler type for all other interrupts */
/* @} */

/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for a device.
 */
typedef struct {
	u16 DeviceId;		/**< Unique ID  of device */
	UINTPTR BaseAddress;	/**< Register base address */
	u8 NumOfAcceptFilters;	/**< Number of Acceptance Filters */
} XCan_Config;

/******************************************************************************/
/**
* Callback type for frame sending and reception interrupts.
*
* @param 	CallBackRef is a callback reference passed in by the upper layer
*		when setting the callback functions, and passed back to the
*		upper layer when the callback is invoked.
******************************************************************************/
typedef void (*XCan_SendRecvHandler) (void *CallBackRef);

/******************************************************************************/
/**
* Callback type for error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the upper layer
*		when setting the callback functions, and passed back to the
*		upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
*		value equals 'OR'ing one or more XCAN_ESR_* values defined in
*		xcan_l.h
******************************************************************************/
typedef void (*XCan_ErrorHandler) (void *CallBackRef, u32 ErrorMask);

/******************************************************************************/
/**
* Callback type for all kinds of interrupts except sending frame interrupt,
* receiving frame interrupt, and error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the upper layer
*		when setting the callback functions, and passed back to the
*		upper layer when the callback is invoked.
* @param	Mask is a bit mask indicating the pending interrupts. Its value
*		equals 'OR'ing one or more XCAN_IXR_* defined in xcan_l.h
******************************************************************************/
typedef void (*XCan_EventHandler) (void *CallBackRef, u32 Mask);


/**
 * The XCan driver instance data. The user is required to allocate a
 * variable of this type for every CAN device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	UINTPTR BaseAddress;	/**< Device Base address */
	u32 IsReady;		/**< Device is initialized and ready */
	u8 NumOfAcceptFilters;  /**< Number of Acceptance Filters */

	/** Callback for TXOK interrupt */
	XCan_SendRecvHandler SendHandler;

	/** This will be passed to the TXOK interrupt callback */
	void *SendRef;

	/** Callback for RXOK/RXNEMP interrupts */
	XCan_SendRecvHandler RecvHandler;

	/** This will be passed to the RXOK/RXNEMP interrupt callback */
	void *RecvRef;

	/** Callback for ERROR interrupt */
	XCan_ErrorHandler ErrorHandler;

	/** This will be passed to the ERROR interrupt callback */
	void *ErrorRef;

	/** Callback for RXOFLW/RXUFLW/TXBFLL/TXFLL/Wakeup/Sleep/Bus off/ARBLST
	 *  interrupts
	 */
	XCan_EventHandler EventHandler;

	/** This will be passed to the EventHandler callback */
	void *EventRef;

} XCan;


/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* This macro checks if the transmission is complete.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
*
* @return
*		- TRUE if the transmission is done (completed).
*		- FALSE if the transmission is not completed.
*
* @note		C-Style signature:
*		int XCan_IsTxDone(XCan *InstancePtr);
*
*****************************************************************************/
#define XCan_IsTxDone(InstancePtr) \
	((XCan_ReadReg(((InstancePtr)->BaseAddress), XCAN_ISR_OFFSET) & \
		XCAN_IXR_TXOK_MASK) ? TRUE : FALSE)


/****************************************************************************/
/**
*
* This macro checks if the transmission FIFO is full.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
*
* @return
*		- TRUE if TX FIFO is full.
*		- FALSE if TX FIFO is not full.
*
* @note		C-Style signature:
*		int XCan_IsTxFifoFull(XCan *InstancePtr);
*
*****************************************************************************/
#define XCan_IsTxFifoFull(InstancePtr) \
	((XCan_ReadReg(((InstancePtr)->BaseAddress), XCAN_SR_OFFSET) & \
		XCAN_SR_TXFLL_MASK) ? TRUE : FALSE)


/****************************************************************************/
/**
*
* This macro checks if the Transmission High Priority Buffer is full.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
*
* @return
*		- TRUE if TX High Priority Buffer is full.
*		- FALSE if TX High Priority Buffer is not full.
*
* @note		C-Style signature:
*		int XCan_IsHighPriorityBufFull(XCan *InstancePtr);
*
*****************************************************************************/
#define XCan_IsHighPriorityBufFull(InstancePtr) \
	((XCan_ReadReg(((InstancePtr)->BaseAddress), XCAN_SR_OFFSET) & \
	XCAN_SR_TXBFLL_MASK) ? TRUE : FALSE)


/****************************************************************************/
/**
*
* This macro checks if the receive FIFO is empty.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
*
* @return
*		- TRUE if RX FIFO is empty.
*		- FALSE if RX FIFO is not empty.
*
* @note		C-Style signature:
*		int XCan_IsRxEmpty(XCan *InstancePtr);
*
*****************************************************************************/
#define XCan_IsRxEmpty(InstancePtr) \
	((XCan_ReadReg(((InstancePtr)->BaseAddress), XCAN_ISR_OFFSET) & \
		XCAN_IXR_RXNEMP_MASK) ? FALSE : TRUE)


/****************************************************************************/
/**
*
* This macro checks if the CAN device is ready for the driver to change
* Acceptance Filter Identifier Registers (AFIR) and Acceptance Filter Mask
* Registers (AFMR).
*
* AFIR and AFMR for a filter are changeable only after the filter is disabled
* and this routine returns FALSE.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
*
* @return
*		- TRUE if the device is busy and NOT ready to accept writes to
*		AFIR and AFMR.
*		- FALSE if the device is ready to accept writes to AFIR and
*		AFMR.
*
* @note		C-Style signature:
*		int XCan_IsAcceptFilterBusy(XCan *InstancePtr);
*
*****************************************************************************/
#define XCan_IsAcceptFilterBusy(InstancePtr) \
		((XCan_ReadReg(((InstancePtr)->BaseAddress), XCAN_SR_OFFSET) & \
		XCAN_SR_ACFBSY_MASK) ? TRUE : FALSE)


/****************************************************************************/
/**
*
* This macro calculates CAN message identifier value given identifier field
* values.
*
* @param	StandardId contains Standard Message ID value.
* @param	SubRemoteTransReq contains Substitute Remote Transmission
*		Request value.
* @param	IdExtension contains Identifier Extension value.
* @param	ExtendedId contains Extended Message ID value.
* @param	RemoteTransReq contains Remote Transmission Request value.
*
* @return	Message Identifier value.
*
* @note		C-Style signature:
*		u32 XCan_CreateIdValue(u32 StandardId, u32 SubRemoteTransReq,
*					u32 IdExtension, u32 ExtendedId,
*					u32 RemoteTransReq);
*
* 		Read the CAN specification for meaning of each parameter.
*
*****************************************************************************/
#define XCan_CreateIdValue(StandardId, SubRemoteTransReq, IdExtension, \
		ExtendedId, RemoteTransReq) \
	((((StandardId) << XCAN_IDR_ID1_SHIFT) & XCAN_IDR_ID1_MASK) | \
	(((SubRemoteTransReq) << XCAN_IDR_SRR_SHIFT) & XCAN_IDR_SRR_MASK) | \
	(((IdExtension) << XCAN_IDR_IDE_SHIFT) & XCAN_IDR_IDE_MASK) | \
	(((ExtendedId) << XCAN_IDR_ID2_SHIFT) & XCAN_IDR_ID2_MASK) | \
	((RemoteTransReq) & XCAN_IDR_RTR_MASK))

/****************************************************************************/
/**
*
* This macro calculates value for Data Length Code register given Data
* Length Code value.
*
* @param	DataLengCode indicates Data Length Code value.
*
* @return	Value that can be assigned to Data Length Code register.
*
* @note		C-Style signature:
*		u32 XCan_CreateDlcValue(u32 DataLengCode);
*
* 		Read the CAN specification for meaning of Data Length Code.
*
*****************************************************************************/
#define XCan_CreateDlcValue(DataLengCode) \
	(((DataLengCode) << XCAN_DLCR_DLC_SHIFT) & XCAN_DLCR_DLC_MASK)

/************************** Function Prototypes ******************************/

/*
 * Functions in xcan.c
 */
int XCan_Initialize(XCan *InstancePtr, u16 DeviceId);
int XCan_VmInitialize(XCan *InstancePtr, u16 DeviceId, UINTPTR VirtAddr);
void XCan_Reset(XCan *InstancePtr);
u8 XCan_GetMode(XCan *InstancePtr);
void XCan_EnterMode(XCan *InstancePtr, u8 OperationMode);
u32 XCan_GetStatus(XCan *InstancePtr);
void XCan_GetBusErrorCounter(XCan *InstancePtr, u8 *RxErrorCount,
			     u8 *TxErrorCount);
u32 XCan_GetBusErrorStatus(XCan *InstancePtr);
void XCan_ClearBusErrorStatus(XCan *InstancePtr, u32 Mask);
int XCan_Send(XCan *InstancePtr, u32 *FramePtr);
int XCan_Recv(XCan *InstancePtr, u32 *FramePtr);
int XCan_SendHighPriority(XCan *InstancePtr, u32 *FramePtr);
void XCan_AcceptFilterEnable(XCan *InstancePtr, u32 FilterIndexes);
void XCan_AcceptFilterDisable(XCan *InstancePtr, u32 FilterIndexes);
u32 XCan_AcceptFilterGetEnabled(XCan *InstancePtr);
int XCan_AcceptFilterSet(XCan *InstancePtr, u32 FilterIndex,
			 u32 MaskValue, u32 IdValue);
void XCan_AcceptFilterGet(XCan *InstancePtr, u32 FilterIndex,
			  u32 *MaskValue, u32 *IdValue);
XCan_Config *XCan_LookupConfig(u16 DeviceId);
XCan_Config *XCan_GetConfig(unsigned int InstanceIndex);

/*
 * Configuration functions in xcan_config.c
 */
int XCan_SetBaudRatePrescaler(XCan *InstancePtr, u8 Prescaler);
u8 XCan_GetBaudRatePrescaler(XCan *InstancePtr);
int XCan_SetBitTiming(XCan *InstancePtr, u8 SyncJumpWidth,
		      u8 TimeSegment2, u8 TimeSegment1);
void XCan_GetBitTiming(XCan *InstancePtr, u8 *SyncJumpWidth,
		       u8 *TimeSegment2, u8 *TimeSegment1);

/*
 * Diagnostic functions in xcan_selftest.c
 */
int XCan_SelfTest(XCan *InstancePtr);

/*
 * Functions in xcan_intr.c
 */
void XCan_InterruptEnable(XCan *InstancePtr, u32 Mask);
void XCan_InterruptDisable(XCan *InstancePtr, u32 Mask);
u32 XCan_InterruptGetEnabled(XCan *InstancePtr);
u32 XCan_InterruptGetStatus(XCan *InstancePtr);
void XCan_InterruptClear(XCan *InstancePtr, u32 Mask);
void XCan_IntrHandler(void *InstancePtr);
int XCan_SetHandler(XCan *InstancePtr, u32 HandlerType,
		    void *CallBackFunc, void *CallBackRef);
#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */

/** @} */
