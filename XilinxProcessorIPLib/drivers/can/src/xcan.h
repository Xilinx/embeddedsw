/******************************************************************************
* Copyright (C) 2005 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XCAN_H
#define XCAN_H

/*****************************************************************************/
/**
*
* @file xcan.h
* @addtogroup can Overview
* @{
* @brief Declares the driver API for the Xilinx CAN controller.
*
* @details
*
* The Xilinx CAN driver. This driver supports the Xilinx CAN Controller.
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
* The device driver enables higher layer software (for example, an application)
* to communicate to the CAN. The driver handles transmission and reception of
* CAN frames, as well as configuration of the controller. The driver is simply a
* pass-through mechanism between a protocol stack and the CAN. A single device
* driver can support multiple CANs.
*
* Because the driver is a simple pass-through mechanism between a protocol stack
* and the CAN, it does no assembly or disassembly of CAN frames. The driver
* assumes that the protocol stack passes a correctly formatted CAN frame to it
* for transmission. The driver does not validate the contents of an incoming
* frame.
*
* <b>Operation Modes</b>
*
* The CAN controller supports the following modes of operation:
*   - <b>Configuration Mode</b>: In this mode the CAN timing parameters and
*     Baud Rate Pre-scalar parameters can be changed. In this mode the CAN
*     controller loses synchronization with the CAN bus and drives a constant
*     recessive bit on the bus line. The Error Counter Register are reset. The
*     CAN controller does not receive or transmit messages, even with pending
*     transmit requests from the TX FIFO or the TX High Priority Buffer. The
*     Storage FIFOs and the CAN configuration
*     registers are still accessible.
*   - <b>Normal Mode</b>:In Normal Mode the CAN controller participates in bus
*     communication, by transmitting and receiving messages.
*   - <b>Sleep Mode</b>: In Sleep Mode the CAN Controller does not transmit any
*     messages. However, if any other node transmits a message, then the CAN
*     Controller receives the transmitted message and exits from Sleep Mode. The
*     CAN Controller does not service new transmission requests from the TX FIFO
*     or the TX High Priority Buffer in Sleep Mode. The controller remains in
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
* Frame buffers passed to the driver must be 32-bit aligned.
*
* <b>Receive Address Filtering</b>
*
* The device can be set to accept frames whose Identifiers match any of up to
* 4 filters set in the Acceptance Filter Mask/ID registers.
*
* The incoming Identifier is masked with the bits in the Acceptance Filter Mask
* Register. This value is compared with the result of masking the bits in the
* Acceptance Filter ID Register with the Acceptance Filter Mask Register. If
* both these values are equal, the message is stored in the RX FIFO.
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
* provides an interrupt handler. The application must provide callback
* functions. An interrupt handler example is available with
* the driver.
*
* <b>Virtual Memory</b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space. The RTOS then invokes
* XCan_VmInitialize(), instead of XCan_Initialize(), to initialize the device at
* first.
*
* <b>Threads</b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b>Device Reset</b>
*
* Bus Off interrupt that can occur in the device requires a device reset. The
* application is responsible for resetting the device and re-configuring it
* based on its needs (the driver does not save the current configuration). When
* integrating into an RTOS, these reset and re-configure obligations are
* taken care of by the OS adapter software if it exists for that RTOS.
*
* <b>Device Configuration</b>
*
* The device can be configured in various ways during the FPGA implementation
* process. Configuration parameters are stored in the xcan_g.c files.
* A table is defined where each entry contains configuration information
* for a CAN device. This information includes such things as the base address
* of the memory-mapped device, and the number of acceptance filters.
*
* <b>Asserts</b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that you leave asserts on during development.
*
* <b>Building the driver</b>
*
* The XCan driver is composed of several source files. This allows you to
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
* 3.5	sne  08/28/20 Modify Makefile to support parallel make execution.
* 3.7	ht   07/04/23 Added support for system device-tree flow.
* 3.8   ht   12/13/23 Added support for ECC.
* 3.12  vlt  12/30/25 Update Doxygen comments to include SDT flow details.
* 3.13  vn   09/08/26 Align Doxygen comments with AMD standards
* 3.13  vn   09/30/26 Update Doxygen comments to AMD standards
* </pre>
*
******************************************************************************/

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
#define XCAN_MODE_CONFIG	0x00000001 /**< Configuration mode. */
#define XCAN_MODE_NORMAL	0x00000002 /**< Normal mode. */
#define XCAN_MODE_LOOPBACK	0x00000004 /**< Loop Back mode. */
#define XCAN_MODE_SLEEP		0x00000008 /**< Sleep mode. */
/** @} */

/** @name Callback identifiers used as parameters to XCan_SetHandler()
 *  @{
 */
#define XCAN_HANDLER_SEND   1 /**< Handler type for frame sending interrupt. */
/** Handler type for frame reception interrupt. */
#define XCAN_HANDLER_RECV   2
#define XCAN_HANDLER_ERROR  3 /**< Handler type for error interrupt. */
#define XCAN_HANDLER_EVENT  4 /**< Handler type for all other interrupts. */
/** @} */

/**************************** Type Definitions *******************************/

/**
 * @brief CAN device configuration structure.
 *
 * Holds configuration information for a device.
 */
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< Unique ID of device. */
#else
	char *Name;		/**< Unique name of the device. */
#endif
	UINTPTR BaseAddress;	/**< Register base address. */
	u8 NumOfAcceptFilters;	/**< Number of Acceptance Filters. */
#ifdef SDT
	u16 IntrId;		/**< Bits[11:0] Interrupt-id Bits[15:12] trigger
				 * type and level flags. */
	UINTPTR IntrParent;	/**< Bit[0] Interrupt parent type Bit[64/32:1]
				 * Parent base address. */
#endif
	u8 EnableECC;		/**< ECC is enabled or disabled. */
} XCan_Config;

/******************************************************************************/
/**
*
* @brief Callback type for frame sending and reception interrupts.
*
* @param[in]    CallBackRef Callback reference passed in by the upper layer
*		when setting the callback functions, and passed back to the
*		upper layer when the callback is invoked.
*
******************************************************************************/
typedef void (*XCan_SendRecvHandler) (void *CallBackRef);

/******************************************************************************/
/**
*
* @brief Callback type for error interrupt.
*
* @param[in]    CallBackRef Callback reference passed in by the upper layer
*		when setting the callback functions, and passed back to the
*		upper layer when the callback is invoked.
* @param[in]    ErrorMask Bit mask indicating the cause of the error. Its
*               value equals the bitwise OR of one or more XCAN_ESR_*
*               values defined in xcan_l.h.
*
******************************************************************************/
typedef void (*XCan_ErrorHandler) (void *CallBackRef, u32 ErrorMask);

/******************************************************************************/
/**
*
* @brief Callback type for all non-send, non-receive, non-error interrupts.
*
* Callback type for all kinds of interrupts except sending frame interrupt,
* receiving frame interrupt, and error interrupt.
*
* @param[in]    CallBackRef Callback reference passed in by the upper layer
*		when setting the callback functions, and passed back to the
*		upper layer when the callback is invoked.
* @param[in]    Mask Bit mask indicating the pending interrupts. Its value
*               equals the bitwise OR of one or more XCAN_IXR_* defined in
*               xcan_l.h.
*
******************************************************************************/
typedef void (*XCan_EventHandler) (void *CallBackRef, u32 Mask);

/**
 * @brief CAN driver instance data structure.
 *
 * The XCan driver instance data. The application must allocate a
 * variable of this type for every CAN device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	UINTPTR BaseAddress;		/**< Device Base address. */
	u32 IsReady;			/**< Device is initialized and ready. */
	u8 NumOfAcceptFilters;		/**< Number of Acceptance Filters. */

	u8 EnableECC;			/**< ECC is enabled or disabled. */

	XCan_SendRecvHandler SendHandler;
					/**< Callback for TXOK interrupt. */

	void *SendRef;			/**< Reference passed to the TXOK
					 * interrupt callback. */

	XCan_SendRecvHandler RecvHandler;
					/**< Callback for RXOK/RXNEMP
					 * interrupts. */

	void *RecvRef;			/**< Reference passed to the RXOK/RXNEMP
					 * interrupt callback. */

	XCan_ErrorHandler ErrorHandler;	/**< Callback for ERROR interrupt. */

	void *ErrorRef;			/**< Reference passed to the ERROR
					 * interrupt callback. */

	XCan_EventHandler EventHandler;	/**< Callback for RXOFLW/RXUFLW/TXBFLL/
					 * TXFLL/Wakeup/Sleep/Bus off/ARBLST
					 * interrupts. */

	void *EventRef;			/**< Reference passed to the
					 * EventHandler callback. */

	u64 ecc_2bit_txtlfifo_cnt;	/**< TXTLFIFO 2bit ECC count. */

	u64 ecc_1bit_txtlfifo_cnt;	/**< TXTLFIFO 1bit ECC count. */

	u64 ecc_2bit_txolfifo_cnt;	/**< TXOLFIFO 2bit ECC count. */

	u64 ecc_1bit_txolfifo_cnt;	/**< TXOLFIFO 1bit ECC count. */

	u64 ecc_2bit_rxfifo_cnt;	/**< RXFIFO 2bit ECC count. */

	u64 ecc_1bit_rxfifo_cnt;	/**< RXFIFO 1bit ECC count. */
} XCan;

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* @brief Checks if the transmission is complete.
*
* @param[in]    InstancePtr Pointer to the XCan instance to be worked on.
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
* @brief Checks if the transmission FIFO is full.
*
* @param[in]    InstancePtr Pointer to the XCan instance to be worked on.
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
* @brief Checks if the Transmission High Priority Buffer is full.
*
* @param[in]    InstancePtr Pointer to the XCan instance to be worked on.
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
* @brief Checks if the receive FIFO is empty.
*
* @param[in]    InstancePtr Pointer to the XCan instance to be worked on.
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
* @brief Checks if the CAN device is ready for acceptance filter changes.
*
* Checks if the CAN device is ready for the driver to change
* Acceptance Filter Identifier Registers (AFIR) and Acceptance Filter Mask
* Registers (AFMR).
*
* AFIR and AFMR for a filter are changeable only after the filter is disabled
* and this routine returns FALSE.
*
* @param[in]    InstancePtr Pointer to the XCan instance to be worked on.
*
* @return
*		- TRUE if the device is busy and NOT ready to accept writes to
*                 AFIR and AFMR.
*		- FALSE if the device is ready to accept writes to AFIR and
*                 AFMR.
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
* @brief Calculates CAN message identifier value given identifier field values.
*
* @param[in]    StandardId Contains Standard Message ID value.
* @param[in]    SubRemoteTransReq Contains Substitute Remote Transmission
*		Request value.
* @param[in]    IdExtension Contains Identifier Extension value.
* @param[in]    ExtendedId Contains Extended Message ID value.
* @param[in]    RemoteTransReq Contains Remote Transmission Request value.
*
* @return	Message Identifier value.
*
* @note		C-Style signature:
*		u32 XCan_CreateIdValue(u32 StandardId, u32 SubRemoteTransReq,
*					u32 IdExtension, u32 ExtendedId,
*					u32 RemoteTransReq);
*               <br><br>
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
* @brief Calculates value for Data Length Code register.
*
* @param[in]    DataLengCode Indicates Data Length Code value.
*
* @return	Value that can be assigned to Data Length Code register.
*
* @note		C-Style signature:
*		u32 XCan_CreateDlcValue(u32 DataLengCode);
*               <br><br>
* 		Read the CAN specification for meaning of Data Length Code.
*
*****************************************************************************/
#define XCan_CreateDlcValue(DataLengCode) \
	(((DataLengCode) << XCAN_DLCR_DLC_SHIFT) & XCAN_DLCR_DLC_MASK)

/************************** Function Prototypes ******************************/

/*
 * Functions in xcan.c
 */
#ifndef SDT
int XCan_Initialize(XCan *InstancePtr, u16 DeviceId);
int XCan_VmInitialize(XCan *InstancePtr, u16 DeviceId, UINTPTR VirtAddr);
#else
int XCan_Initialize(XCan *InstancePtr, UINTPTR BaseAddress);
int XCan_VmInitialize(XCan *InstancePtr, UINTPTR BaseAddress, UINTPTR VirtAddr);
#endif
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
#ifndef SDT
XCan_Config *XCan_LookupConfig(u16 DeviceId);
#else
XCan_Config *XCan_LookupConfig(UINTPTR BaseAddress);
#endif
XCan_Config *XCan_GetConfig(unsigned int InstanceIndex);

void XCan_ResetECC(XCan *InstancePtr);
void XCan_GetECCCount(XCan *InstancePtr);

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
