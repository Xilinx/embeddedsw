/******************************************************************************
*
* Copyright (C) 2015 - 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
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
* @file xcanfd.h
* @addtogroup canfd_v2_2
* @{
* @details
*
* The Xilinx CANFD driver.  This driver supports the Xilinx CANFD Controller.
*
* The CANFD Controller supports the following features:
*   - Confirms to the ISO 11898-1, CAN 2.0A and CAN 2.0B standards.
*   - Supports both Standard (11 bit Identifier) and Extended (29 bit
*     Identifier) frames.
*   - Supports Bit Rates up to 8 Mbps.
*   - Transmit message object FIFO with a user configurable depth of up to 64
*     message objects.
*   - Receive message object FIFO with a user configurable depth of up to 64
*     message objects.
*   - Acceptance filtering through a user configurable number of up to 32
*     acceptance filters.
*   - Loop Back mode for diagnostic applications.
*   - Maskable Error and Status Interrupts.
*   - Readable Error Counters.
*   - External PHY chip required.
*   - Backward compatible for Legacy CAN.
*   - Supports reception in Mailbox and Sequential Mode
*
* The device driver supports all the features listed above, if applicable.
*
* <b>Driver Description</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the CANFD. The driver handles transmission and reception of
* CAN frames, as well as configuration of the controller. The driver is simply a
* pass-through mechanism between a protocol stack and the CANFD. A single device
* driver can support multiple CANFDs.
*
* Since the driver is a simple pass-through mechanism between a protocol stack
* and the CANFD, no assembly or disassembly of CANFD frames is done at the
* driver-level. This assumes that the protocol stack passes a correctly
* formatted CANFD frame to the driver for transmission, and that the driver
* does not validate the contents of an incoming frame
*
* <b>Operation Modes</b>
*
* The CANFD controller supports the following modes of operation:
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
*     is loop backed to the rx line and acknowledged. The CAN controller
*     thus receives any message that it transmits. It does not participate in
*     normal bus communication and does not receive any messages that are
*     transmitted by other CAN nodes. This mode is used for diagnostic
*     purposes.
*   - <b>Snoop mode</b>:in Snoop Mode,The CAN controller transmits recessive
*     bit stream on to CAN bus and does not participate in normal bus
*     communication but receives messages that are transmitted by other
*     CAN nodes.Received messages are stored based on receive ID match result.
*     Error counters are disabled and cleared to 0. Reads to error counter
*     register will return zero.
*
* <b>Buffer Alignment</b>
*
* It is important to note that frame buffers passed to the driver must be
* 32-bit aligned.
*
* <b>Receive Address Filtering</b>
*
* The device can be set to accept frames whose Identifiers match any of up to
* 32 filters set in the Acceptance Filter Mask/ID registers.
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
* XCanFd_VmInitialize(), instead of XCanFd_Initialize(), to initialize the device
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
* The XCanFd driver is composed of several source files.This allows the user to
* build and link only those parts of the driver that are necessary.
* <br><br>
*
* <pre>
* Temp Change
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   nsk  06/04/15 First release
* 1.0   nsk  15/05/15 Updated xcanfd.c for correct AFRID and AFRMSK Registers
* 		      Updated xcanfd.c and xcanfd.h to get configurable
*		      TxBuffers in XCanFd_Config Struct and
*		      XCanFd_GetFreeBuffer().
*		      Modified XCANFD_BTR_TS1_MASK in xcanfd_hw.h.
*		      Updated xcanfd.c while sending data when EDL is Zero.
*		      Updated driver tcl file to get configurable TxBuffers.
*		      (CR 861772).
* 1.0	nsk  16/06/15 Updated XCanFd_Recv_Mailbox(), XCanFd_EnterMode()
*		      XCanFd_GetMode() in xcanfd.c and Added new definition
*		      for Register bits in xcanfd_hw.h and updated
*		      XCanFd_IntrHandler() in xcanfd_intr.c as per new RTL.
*		      Changes in RTL, Added new bits to MSR,SR,ISR,IER,ICR
*		      Registers and modified TS2 bits in BTR and F_SJW bits
*		      in F_BTR Registers.
* 1.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototype of XCanFd_CfgInitialize API.
* 1.2   mi   09/22/16 Fixed compilation warnings.
*       ms   01/23/17 Modified xil_printf statement in main function for all
*                     examples to ensure that "Successfully ran" and "Failed"
*                     strings are available in all examples. This is a fix
*                     for CR-965028.
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
*       ms   04/05/17 Added tabspace for return statements in functions
*                     of canfd examples for proper documentation while
*                     generating doxygen.
* 2.0   mj   1/3/18   updated supported_peripherals and version number CR# 991037.
*                     CANFD Driver not pulled by drivers.
* 2.1   ask  09/12/18 Added support for canfd 2.0 spec sequential mode.
*                                        API's added : XCanFd_Recv_Sequential
*                                                                      XCanFd_SeqRecv_logic
*                                                                      XCanFd_Recv_TXEvents_Sequential
*                                                                      XCanFd_GetNofMessages_Stored_TXE_FIFO
*                                                                      XCanFd_GetNofMessages_Stored_Rx_Fifo
*                                                                      above apis added in xcanfd.c .
*
*                                                                      XCanFd_SetRxIntrWatermarkFifo1
*                                                                      XCanFd_SetTxEventIntrWatermark
*                                                                      XCanFd_SetRxFilterPartition
*                                   above apis added in xcanfd_config.c
*
*                                                                      XCANFD_TXEID_OFFSET
*                                                                      XCANFD_TXEDLC_OFFSET
*                                                                      XCANFD_FIFO_1_RXID_OFFSET
*                                                                      XCANFD_FIFO_1_RXDLC_OFFSET
*                                                                      XCANFD_FIFO_1_RXDW_OFFSET
*                                                                      above apis added in xcanfd.h
*			Fixed Message Queuing logic by modifying in functions
*					  	XCanFd_Send_Queue, XCanFd_Addto_Queue, XCanFd_Send,
*					  	and XCanFd_GetFreeBuffer.
*			 Added an static function
*					  	XCanfd_TrrVal_Get_SetBit_Position, added in xcanfd.c
*						XCanFD_Check_TrrVal_Set_Bit, added in xcanfd.h
*			 Modified apis
*						XCanFd_SetBitTiming
*                                               XCanFd_SetFBitTiming in xcanfd.h
*       ask  07/03/18 Fix for Sequential recv CR# 992606,CR# 1004222.
*       nsk  07/11/18 Updated tcl to generate CANFD Frequency macro in
*		      xparameters.h (CR 1005641).
*	ask  08/27/18 Modified RecvSeq function to return XST_NO_DATA when the
*	 	      fifo fill levels are zero.
* 	ask  08/08/18 Fixed Gcc, Cppcheck and doxygen warnings in api's :
*                                        XCanFd_PollQueue_Buffer, XCanFd_AcceptFilterSet,
*                                        XCanFd_Recv_Sequential, XCanFd_SetBitTiming,
*                                        XCanFd_SetBitRateSwitch_EnableNominal.
*                                        Changed value of Canfd Id to 11 bit value to comply
*                                        with standard Can ID.
*	ask  09/21/18 Fixed CanFD hang issue in selftest by correcting the
*                    Configuration regarding the Baud Rate and bit timing
*                    for both Arbitration and Data Phase.
* 2.1   nsk  01/22/19 Pass correct fifo number to XCanFd_SeqRecv_logic() in
*		      xcanfd.c CR# 1018379
* 2.1   nsk  01/22/19 Fixed XCanFd_SetFBaudPrescalar(), which is not setting
*		      prescalar value properly in xcanfd_config.c CR# 1016013
* 2.1	nsk  03/09/19 Updated XCanFd_GetDlc2len(), for CAN frames, to handle
*		      number of data bytes greater than 8. CR# 1022045
* 2.1	nsk  03/09/19 Fix for TrrMask to not to get written when using
		      XCanFd_Addto_Queue(). CR# 1022093
* 2.1	nsk  03/09/19 Added support for PS CANFD, PL CANFD 1.0 and PL CANFD 2.0
*		      CR# 1021963
* 2.2   sn   06/11/19 Corrected below incorrect Mask values for CANFD2.0 in xcanfd_hw.h
*		      XCANFD_MAILBOX_RB_MASK_BASE_OFFSET,XCANFD_WMR_RXFP_MASK
*		      and CONTROL_STATUS_3.
*
* </pre>
*
******************************************************************************/

#ifndef XCANFD_H			/* prevent circular inclusions */
#define XCANFD_H			/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xcanfd_hw.h"

/************************** Constant Definitions *****************************/
/** @name CAN normal Bit rate fields
 *  @{
 */
#if defined (CANFD_v1_0)
#define XCANFD_MAX_SJW_VALUE 0x10
#define XCANFD_MAX_TS1_VALUE 0x40
#define XCANFD_MAX_TS2_VALUE 0x20
#else
#define XCANFD_MAX_SJW_VALUE 0x80
#define XCANFD_MAX_TS1_VALUE 0x100
#define XCANFD_MAX_TS2_VALUE 0x80
#endif

/** @name CAN Fast Bit rate fields
 *  @{
 */
#if defined (CANFD_v1_0)
#define XCANFD_MAX_F_SJW_VALUE 0x03
#define XCANFD_MAX_F_TS1_VALUE 0x0F
#define XCANFD_MAX_F_TS2_VALUE 0x07
#else
#define XCANFD_MAX_F_SJW_VALUE 0x10
#define XCANFD_MAX_F_TS1_VALUE 0x20
#define XCANFD_MAX_F_TS2_VALUE 0x10
#endif
/** @name CAN operation modes
 *  @{
 */
#define XCANFD_MODE_CONFIG	0x00000001 /**< Configuration mode */
#define XCANFD_MODE_NORMAL	0x00000002 /**< Normal mode */
#define XCANFD_MODE_LOOPBACK	0x00000004 /**< Loop Back mode */
#define XCANFD_MODE_SLEEP	0x00000008 /**< Sleep mode */
#define XCANFD_MODE_SNOOP	0x00000010 /**< Snoop mode */
#define XCANFD_MODE_ABR		0x00000020 /**< Auto Bus-Off Recovery */
#define XCANFD_MODE_SBR		0x00000040 /**< Starut Bus-Off Recovery */
#define XCANFD_MODE_PEE		0x00000080 /**< Protocol Exception mode */
#define XCANFD_MODE_DAR		0x0000000A /**< Disable Auto Retransmission mode */
#define XCANFD_MODE_BR		0x0000000B /**< Bus-Off Recovery Mode */
#define XCANFD_RX_FIFO_0	         0 /**< Selection for RX Fifo 0 */
#define XCANFD_RX_FIFO_1	         1 /**< Selection for RX Fifo 1 */
/* @} */

/** @name Callback identifiers used as parameters to XCanFd_SetHandler()
 *  @{
 */
#define XCANFD_HANDLER_SEND   1 /**< Handler type for frame sending interrupt
								*/
#define XCANFD_HANDLER_RECV   2 /**< Handler type for frame reception interrupt
								 */
#define XCANFD_HANDLER_ERROR  3 /**< Handler type for error interrupt */
#define XCANFD_HANDLER_EVENT  4 /**< Handler type for all other interrupts */
/* @} */

/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for a device.
 */
typedef struct {
	u16 DeviceId;		/**< Unique ID  of device */
	UINTPTR BaseAddress;	/**< Register base address */
	u32 Rx_Mode;			/**< 1-Mailbox 0-sequential */
	u32 NumofRxMbBuf;	/**< Number of RxBuffers */
	u32 NumofTxBuf;         /**< Number of TxBuffers */
} XCanFd_Config;

/*****************************************************************************/
/**
* Callback type for frame sending and reception interrupts.
*
* @param 	CallBackRef is a callback reference passed in by the upper layer
*		when setting the callback functions, and passed back to the
*		upper layer when the callback is invoked.
*
******************************************************************************/
typedef void (*XCanFd_SendRecvHandler) (void *CallBackRef);

/******************************************************************************/
/**
* Callback type for error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the upper layer
*		when setting the callback functions, and passed back to the
*		upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
*		value equals 'OR'ing one or more XCANFD_ESR_* values defined in
*		xcan_l.h
******************************************************************************/
typedef void (*XCanFd_ErrorHandler) (void *CallBackRef, u32 ErrorMask);

/*****************************************************************************/
/**
* Callback type for all kinds of interrupts except sending frame interrupt,
* receiving frame interrupt, and error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the upper layer
*		when setting the callback functions, and passed back to the
*		upper layer when the callback is invoked.
* @param	Mask is a bit mask indicating the pending interrupts. Its value
*		equals 'OR'ing one or more XCANFD_IXR_* defined in xcanfd_hw.h
******************************************************************************/
typedef void (*XCanFd_EventHandler) (void *CallBackRef, u32 Mask);

/*****************************************************************************/
/**
 * The XCanFd driver instance data. The user is required to allocate a
 * variable of this type for every CAN device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	XCanFd_Config CanFdConfig;	/**< Device Configuration */
	u32 IsReady;		/**< Device is initialized and ready */
	u32 MultiBuffTrr;	/**< used in multibuffer send case
							to update TRR Register */
	u32 GlobalTrrValue; /**< used in multibuffer send case
							to update TRR Register */
	u32 GlobalTrrMask;  /**< used in multibuffer send case
							to update TRR Register */

	XCanFd_SendRecvHandler SendHandler;
	void *SendRef;

	XCanFd_SendRecvHandler RecvHandler;
	void *RecvRef;

	XCanFd_ErrorHandler ErrorHandler;
	void *ErrorRef;

	XCanFd_EventHandler EventHandler;
	void *EventRef;

}XCanFd;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro checks if the transmission is complete.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	- TRUE if the transmission is done (completed).
*		- FALSE if the transmission is not completed.
*
* @note		C-Style signature:
*		int XCanFd_IsTxDone(XCanFd *InstancePtr);
*
*****************************************************************************/
#define XCanFd_IsTxDone(InstancePtr) \
	((XCanFd_ReadReg(((InstancePtr)->CanFdConfig.BaseAddress), \
			XCANFD_ISR_OFFSET) & XCANFD_IXR_TXOK_MASK) ? TRUE : FALSE)

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
*		u32 XCanFd_CreateIdValue(u32 StandardId, u32 SubRemoteTransReq,
*			u32 IdExtension, u32 ExtendedId,
*			u32 RemoteTransReq);
*
* 		Read the CAN specification for meaning of each parameter.
*
*****************************************************************************/
#define XCanFd_CreateIdValue(StandardId, SubRemoteTransReq, IdExtension, \
		ExtendedId, RemoteTransReq) \
	((((StandardId) << XCANFD_IDR_ID1_SHIFT) & XCANFD_IDR_ID1_MASK) | \
	(((SubRemoteTransReq) << XCANFD_IDR_SRR_SHIFT) & XCANFD_IDR_SRR_MASK) | \
	(((IdExtension) << XCANFD_IDR_IDE_SHIFT) & XCANFD_IDR_IDE_MASK) | \
	(((ExtendedId) << XCANFD_IDR_ID2_SHIFT) & XCANFD_IDR_ID2_MASK) | \
	((RemoteTransReq) & XCANFD_IDR_RTR_MASK))

/****************************************************************************/
/**
*
* This macro calculates value for Data Length Code register given Data
* Length Code value with EDL set to 1(Only Can FD frames).
*
* @param	DataLengCode indicates Data Length Code value.
*
* @return	Value that can be assigned to Data Length Code register.
*
* @note		C-Style signature:
*		u32 XCanFd_Create_CanFD_DlcValue(u32 DataLengCode);
*
* 		Read the CAN specification for meaning of Data Length Code.
*
*****************************************************************************/
#define XCanFd_Create_CanFD_DlcValue(DataLengCode) \
	((((DataLengCode) << XCANFD_DLCR_DLC_SHIFT) & XCANFD_DLCR_DLC_MASK) \
			|(XCANFD_DLCR_EDL_MASK))

/****************************************************************************/
/**
*
* This macro calculates value for Data Length Code register given Data
* Length Code value with EDL set to 1(Only Can FD frames) and Setting the BRS.
*
* @param	DataLengCode indicates Data Length Code value.
*
* @return	Value that can be assigned to Data Length Code register.
*
* @note		C-Style signature:
*		u32 XCanFd_Create_CanFD_Dlc_BrsValue(u32 DataLengCode);
*
*	 	Read the CAN specification for meaning of Data Length Code.
*
*****************************************************************************/
#define XCanFd_Create_CanFD_Dlc_BrsValue(DataLengCode) \
	((((DataLengCode) << XCANFD_DLCR_DLC_SHIFT) & XCANFD_DLCR_DLC_MASK) \
			|(XCANFD_DLCR_EDL_MASK) |(XCANFD_DLCR_BRS_MASK))

/****************************************************************************/
/**
*
* This macro calculates value for Data Length Code register given Data
* Length Code value i.e Only Stand.. Can frames.
*
* @param	DataLengCode indicates Data Length Code value.
*
* @return	Value that can be assigned to Data Length Code register.
*
* @note		C-Style signature:
*		u32 XCanFd_CreateDlcValue(u32 DataLengCode);
*
*	 	Read the CAN specification for meaning of Data Length Code.
*
*****************************************************************************/
#define XCanFd_CreateDlcValue(DataLengCode) \
	((((DataLengCode) << XCANFD_DLCR_DLC_SHIFT) & XCANFD_DLCR_DLC_MASK))

/****************************************************************************/
/**
*
* This macro checks whether Particular Buffer is Transmitted or not
*
* Transmit Ready Request Register gives which Buffer is transmitted
* if you trigger the Buffer1 then
* 			TRR Reg : 0x00000001
* After transmission Core clears the bit
* 			TRR Reg : 0x00000000
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @param	TxBuffer is the buffer where driver has written user data\
*
* @return	- TRUE if the device is busy and NOT ready to accept writes to
*		AFIR and AFMR.
*		- FALSE if the device is ready to accept writes to AFIR and
*		AFMR.
*
* @note		C-Style signature:
*		int XCanFd_IsAcceptFilterBusy(XCanFd *InstancePtr);
*
*****************************************************************************/
#define XCanFd_IsBufferTransmitted(InstancePtr,TxBuffer)	\
		((XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, \
				XCANFD_TRR_OFFSET) & (1 << TxBuffer)) ? FALSE : TRUE)


/****************************************************************************/
/**
* This macro initializes CurrentBuffer[32] to zeros.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	none
*
* @note		makes bufferstatus to zero.
*
*****************************************************************************/
#define MAKE_CURRENTBUFFER_ZERO(InstancePtr) \
	for (BufferNr = 0;BufferNr <= 31; BufferNr++) \
		InstancePtr->FreeBuffStatus[BufferNr] = 0

/*****************************************************************************/
/**
* This macro Returns the TXBUFFER ID Offset
*
* @param	FreeBuffer is the Buffer number to locate the FIFO Index
*
* @note		none
*
*****************************************************************************/
#define XCANFD_TXID_OFFSET(FreeBuffer) \
	(XCANFD_TXFIFO_0_BASE_ID_OFFSET+(FreeTxBuffer*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
* This macro Returns the TXBUFFER DLC Offset
*
* @param	FreeBuffer is the Buffer number to locate the FIFO
*
* @note		none
*
 *****************************************************************************/
#define XCANFD_TXDLC_OFFSET(FreeBuffer) \
	(XCANFD_TXFIFO_0_BASE_DLC_OFFSET+(FreeTxBuffer*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
* This macro Returns the TXBUFFER DW Offset
*
* @param	FreeBuffer is the Buffer number to locate the FIFO
*
* @note		none
*
*****************************************************************************/
#define XCANFD_TXDW_OFFSET(FreeBuffer) \
	(XCANFD_TXFIFO_0_BASE_DW0_OFFSET+(FreeTxBuffer*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
* This macro Returns the TX Event Buffer ID Offset
*
* @param	TXEVENTIndex is the Buffer number to locate the TXE FIFO Index
*
* @note		This API is meant to be used with IP
*			with CanFD 2.0 spec support only.
*
*****************************************************************************/
#define XCANFD_TXEID_OFFSET(TXEVENTIndex) \
	(XCANFD_TXEFIFO_0_BASE_ID_OFFSET+(TXEVENTIndex*XCANFD_TXE_MESSAGE_SIZE))

/*****************************************************************************/
/**
* This macro Returns the TX Event Buffer DLC Offset
*
* @param	TXEVENTIndex is the Buffer number to locate the FIFO
*
* @note		This API is meant to be used with IP
*			with CanFD 2.0 spec support only.
*
 *****************************************************************************/
#define XCANFD_TXEDLC_OFFSET(TXEVENTIndex) \
	(XCANFD_TXEFIFO_0_BASE_DLC_OFFSET+(TXEVENTIndex*XCANFD_TXE_MESSAGE_SIZE))
/*****************************************************************************/
/**
* This macro Returns the RXBUFFER ID Offset
*
* @param	ReadIndex is the Buffer	number to locate the FIFO
*
* @note		none
*
*****************************************************************************/
#define XCANFD_RXID_OFFSET(ReadIndex) \
	(XCANFD_RXFIFO_0_BASE_ID_OFFSET+(ReadIndex*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
* This macro Returns the RXBUFFER DLC Offset
*
* @param	ReadIndex is the Buffer	number to locate the FIFO
*
* @note		none
*
*****************************************************************************/
#define XCANFD_RXDLC_OFFSET(ReadIndex) \
	(XCANFD_RXFIFO_0_BASE_DLC_OFFSET+(ReadIndex*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
* This macro Returns the RXBUFFER DW Offset
*
* @param	ReadIndex is the Buffer	number to locate the FIFO
*
* @note		none
*
*****************************************************************************/
#define XCANFD_RXDW_OFFSET(ReadIndex) \
	(XCANFD_RXFIFO_0_BASE_DW0_OFFSET+(ReadIndex*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
* This macro Returns the RXBUFFER ID Offset for FIFO 1
*
* @param	ReadIndex is the Buffer	number to locate the FIFO
*
* @note		This API is meant to be used with IP
*			with CanFD 2.0 spec support only.
*
*****************************************************************************/
#define XCANFD_FIFO_1_RXID_OFFSET(ReadIndex) \
	(XCANFD_RXFIFO_1_BUFFER_0_BASE_ID_OFFSET+(ReadIndex*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
* This macro Returns the RXBUFFER DLC Offset for FIFO 1
*
* @param	ReadIndex is the Buffer	number to locate the FIFO
*
* @note		This API is meant to be used with IP
*			with CanFD 2.0 spec support only.
*
*****************************************************************************/
#define XCANFD_FIFO_1_RXDLC_OFFSET(ReadIndex) \
	(XCANFD_RXFIFO_1_BUFFER_0_BASE_DLC_OFFSET+(ReadIndex*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
* This macro Returns the RXBUFFER DW Offset for FIFO 1
*
* @param	ReadIndex is the Buffer	number to locate the FIFO
*
* @note		This API is meant to be used with IP
*			with CanFD 2.0 spec support only.
*
*****************************************************************************/
#define XCANFD_FIFO_1_RXDW_OFFSET(ReadIndex) \
	(XCANFD_RXFIFO_1_BUFFER_0_BASE_DW0_OFFSET+(ReadIndex*XCANFD_MAX_FRAME_SIZE))
/*****************************************************************************/
/**
* This macro Returns the RCS Register Offset
*
* @param	NoCtrlStatus is to locate RCS Registers
*
* @note		NoCtrlStatus = 0 -> RCS0
*			       1 -> RCS1
*			       2 -> RCS2
*
*****************************************************************************/
#define XCANFD_RCS_OFFSET(NoCtrlStatus)	\
		(XCANFD_RCS0_OFFSET+(NoCtrlStatus*4))

/*****************************************************************************/
/**
* This macro Returns the AFMR Register Offset
*
* @param	FilterIndex is the Index number of Mask Register
*
* @note		none
*
*****************************************************************************/
#define XCANFD_AFMR_OFFSET(FilterIndex)	\
		(XCANFD_AFMR_BASE_OFFSET+\
		(FilterIndex*8))

/*****************************************************************************/
/**
* This macro Returns the AFIDR Registger Offset
*
* @param	FilterIndex is the Index of Id Register
*
* @note		none
*
*****************************************************************************/
#define XCANFD_AFIDR_OFFSET(FilterIndex)	\
		(XCANFD_AFIDR_BASE_OFFSET+\
		(FilterIndex*8))

/*****************************************************************************/
/**
* This macro Returns the MAILBOX MODE RXMASK Offset
*
* @param	BufferNr is the Buffer	number to locate the FIFO
*
* @note		none
*
*****************************************************************************/
#define XCANFD_MAILBOX_MASK_OFFSET(BufferNr)	\
	(XCANFD_MAILBOX_RB_MASK_BASE_OFFSET+(BufferNr*4))

/*****************************************************************************/
/**
* This macro Returns the MAILBOX MODE ID Offset
*
* @param	BufferNr is the Buffer	number to locate the FIFO
*
* @note		none
*
*****************************************************************************/
#define XCANFD_MAILBOX_ID_OFFSET(BufferNr)	\
	(XCANFD_RXFIFO_0_BASE_ID_OFFSET+(BufferNr*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
* This macro Returns Design mode
* 1- Mailbox
* 0- Sequential
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked
*		on.
*
* @note		none
*
*****************************************************************************/
#define XCANFD_GET_RX_MODE(InstancePtr)\
		InstancePtr->CanFdConfig.Rx_Mode

/*****************************************************************************/
/**
*
* This function resets the CAN device. Calling this function resets the device
* immediately, and any pending transmission or reception is terminated at once.
* Both Object Layer and Transfer Layer are reset. This function does not reset
* the Physical Layer. All registers are reset to the default values, and no
* previous status will be restored. TX FIFO, RX FIFO and TX High Priority
* Buffer are also reset.
*
* When a reset is required due to an internal error, the driver notifies the
* upper layer software of this need through the error status code or interrupts
* The upper layer software is responsible for calling this Reset function and
* then re-configuring the device.
*
* The CAN device will be in Configuration Mode immediately after this function
* returns.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
#define XCanFd_Reset(InstancePtr)	\
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_SRR_OFFSET,\
			XCANFD_SRR_SRST_MASK)

/*****************************************************************************/
/**
*
* This function reads Error Status value from Error Status Register (ESR). Use
* the XCANFD_ESR_* constants defined in xcanfd_hw.h to interpret the returned value.
*
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	The 32-bit value read from Error Status Register.
*
* @note		None.
*
******************************************************************************/
#define XCanFd_GetBusErrorStatus(InstancePtr)	\
	XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_ESR_OFFSET)

/*****************************************************************************/
/**
*
* This function returns Status value from Status Register (SR). Use the
* XCANFD_SR_* constants defined in xcanfd_hw.h to interpret the returned value.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	The 32-bit value read from Status Register.
*
* @note		None.
*
******************************************************************************/
#define XCanFd_GetStatus(InstancePtr)	\
	XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_SR_OFFSET)

/*****************************************************************************/
/**
*
* This function clears Error Status bit(s) previously set in Error
* Status Register (ESR). Use the XCANFD_ESR_* constants defined in xcanfd_hw.h to
* create the value to pass in. If a bit was cleared in Error Status Register
* before this function is called, it will not be touched.
*
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @param	Mask is he 32-bit mask used to clear bits in Error Status
*		Register. Multiple XCANFD_ESR_* values could be 'OR'ed to clear
*		multiple bits
*
* @note		None.
*
******************************************************************************/
#define XCanFd_ClearBusErrorStatus(InstancePtr,Mask)	\
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_ESR_OFFSET, \
			Mask)

/*****************************************************************************/
/**
*
* This function returns  the Tranceive delay comensation Offset.
* This function can call when user sends multiple Buffers using Addto_Queue()
* and XCanFd_Send_Queue().
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	None.
*
*
* @note		None.
*
******************************************************************************/
#define XCanFd_Get_Tranceiver_Delay_CompensationOffset(InstancePtr)	\
	((XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,\
	XCANFD_F_BRPR_OFFSET) & XCANFD_F_BRPR_TDCMASK) >> 8)

/*****************************************************************************/
/**
*
* This function Clears Time Stamp Counter Value.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	none
*
*
* @note		None.
*
******************************************************************************/
#define XCanFd_ClearTImeStamp_Count(InstancePtr)	\
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress, \
			XCANFD_TIMESTAMPR_OFFSET,XCANFD_CTS_MASK)

/*****************************************************************************/
/**
*
* This function returns Time Stamp Counter Value.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	TimeStampCount
*
*
* @note		None.
*
******************************************************************************/
#define XCanFd_GetTImeStamp_Count(InstancePtr)	\
	(XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,\
			XCANFD_TIMESTAMPR_OFFSET) >> 16)

/****************************************************************************/
/**
*
* This routine returns the Rx water Mark threshold Value.
*
* @param	InstancePtr is a pointer to the XCanFd instance.
*
*
* @return	Threshold Value.
*
* @note		none
*
*****************************************************************************/
#define XCanFd_GetRxIntrWatermark(InstancePtr)	\
	(XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, \
			XCANFD_WIR_OFFSET) & XCANFD_WIR_MASK)

/****************************************************************************/
/**
*
* This routine returns enabled interrupt(s). Use the XCANFD_IXR_* constants
* defined in xcanfd_hw.h to interpret the returned value.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	Enabled interrupt(s) in a 32-bit format.
*
* @note		None.
*
*****************************************************************************/
#define XCanFd_InterruptGetEnabled(InstancePtr)	\
	XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_IER_OFFSET)

/****************************************************************************/
/**
*
* This routine returns interrupt status read from Interrupt Status Register.
* Use the XCANFD_IXR_* constants defined in xcanfd_hw.h to interpret the returned
* value.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	The value stored in Interrupt Status Register.
*
* @note		None.
*
*****************************************************************************/
#define XCanFd_InterruptGetStatus(InstancePtr)	\
	XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_ISR_OFFSET)

/****************************************************************************/
/**
*
* This routine returns Number of RCS registers to access
* because in Mail box mode user can configure 48,32,16 Rx Buffers.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	The value stored in Interrupt Status Register.
*
* @note		None.
*
*****************************************************************************/
#define XCanFd_Get_NofRxBuffers(InstancePtr)	\
		((InstancePtr->CanFdConfig.NumofRxMbBuf == 48) ? (3) \
				:((InstancePtr->CanFdConfig.NumofRxMbBuf == 32) ? \
				(2) : (1)))

/****************************************************************************/
/**
*
* This routine returns Number of RxBuffers
* user can Desing RxBuffers as 48,32,16.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	The value stored in Interrupt Status Register.
*
* @note		None.
*
*****************************************************************************/
#define XCanFd_Get_RxBuffers(InstancePtr)	\
		InstancePtr->CanFdConfig.NumofRxMbBuf;

/****************************************************************************/
/**
*
* This routine returns Number with right most bit set
* from the target input value.
*
* @param	Target value.
*
* @return	Number with right most bit set from the target value.
*
* @note		None.
*
*****************************************************************************/
#define XCanFD_Check_TrrVal_Set_Bit(Var)      Var&(-Var)

/****************************************************************************/
/**
*
* This routine returns Number with right most bit set
* from the target input value.
*
* @param	Target value.
*
* @return	Number with right most bit set from the target value.
*
* @note		None.
*
*****************************************************************************/
#define XCanFD_Check_TrrVal_Set_Bit(Var)      Var&(-Var)

/* Functions in xcan.c */
int XCanFd_CfgInitialize(XCanFd *InstancePtr, XCanFd_Config *ConfigPtr,
						UINTPTR EffectiveAddr);
u8 XCanFd_GetMode(XCanFd *InstancePtr);
void XCanFd_EnterMode(XCanFd *InstancePtr, u8 OperationMode);
void XCanFd_GetBusErrorCounter(XCanFd *InstancePtr, u8 *RxErrorCount,
							u8 *TxErrorCount);

int XCanFd_Send(XCanFd *InstancePtr,u32 *FramePtr,u32 *TxBufferNumber);
int XCanFd_Recv(XCanFd *InstancePtr, u32 *FramePtr);
int XCanFd_SendHighPriority(XCanFd *InstancePtr, u32 *FramePtr);
void XCanFd_AcceptFilterEnable(XCanFd *InstancePtr, u32 FilterIndexMask);
void XCanFd_AcceptFilterDisable(XCanFd *InstancePtr, u32 FilterIndexMask);
u32 XCanFd_AcceptFilterGetEnabled(XCanFd *InstancePtr);
int XCanFd_AcceptFilterSet(XCanFd *InstancePtr, u32 FilterIndex,
						u32 MaskValue, u32 IdValue);
void XCanFd_AcceptFilterGet(XCanFd *InstancePtr, u32 FilterIndex,
						u32 *MaskValue, u32 *IdValue);
XCanFd_Config *XCanFd_LookupConfig(u16 DeviceId);
XCanFd_Config *XCanFd_GetConfig(unsigned int InstanceIndex);
int XCanFd_GetDlc2len(u32 Dlc, u32 Edl);
u8 XCanFd_GetLen2Dlc(int len);
u32 XCanFd_GetFreeBuffer(XCanFd *InstancePtr);
int XCanFd_Send_Queue(XCanFd *InstancePtr);
int XCanFd_Addto_Queue(XCanFd *InstancePtr, u32 *FramePtr,u32 *TxBufferNumber);
u32 XCanFd_RxBuff_MailBox_Active(XCanFd *InstancePtr, u32 RxBuffer);
u32 XCanFd_RxBuff_MailBox_DeActive(XCanFd *InstancePtr, u32 RxBuffer);
u32 XCanFd_Set_MailBox_IdMask(XCanFd *InstancePtr, u32 RxBuffer,
						u32 MaskValue, u32 IdValue);
u32 XCanFd_Recv_Sequential(XCanFd *InstancePtr, u32 *FramePtr);
u32 XCanFd_Recv_Mailbox(XCanFd *InstancePtr, u32 *FramePtr);
u32 XCanFd_Recv_TXEvents_Sequential(XCanFd *InstancePtr, u32 *FramePtr);
void XCanFd_PollQueue_Buffer(XCanFd *InstancePtr);
int XCanFd_GetNofMessages_Stored_Rx_Fifo(XCanFd *InstancePtr, u8 fifo_no);
int XCanFd_GetNofMessages_Stored_TXE_FIFO(XCanFd *InstancePtr);
int XCanFd_TxBuffer_Cancel_Request(XCanFd *InstancePtr, u32 BufferNumber);
void XCanFd_Enable_Tranceiver_Delay_Compensation(XCanFd *InstancePtr);
void XCanFd_Set_Tranceiver_Delay_Compensation(XCanFd *InstancePtr, u32 TdcOffset);
void XCanFd_Disable_Tranceiver_Delay_Compensation(XCanFd *InstancePtr);

/* Configuration functions in xcan_config.c */
int XCanFd_SetBaudRatePrescaler(XCanFd *InstancePtr, u8 Prescaler);
u8 XCanFd_GetBaudRatePrescaler(XCanFd *InstancePtr);
u8 XCanFd_GetFBaudRatePrescaler(XCanFd *InstancePtr);
int XCanFd_SetBitTiming(XCanFd *InstancePtr, u8 SyncJumpWidth,
					u8 TimeSegment2, u16 TimeSegment1);
void XCanFd_GetBitTiming(XCanFd *InstancePtr, u8 *SyncJumpWidth,
					u8 *TimeSegment2, u8 *TimeSegment1);
void XCanFd_GetFBitTiming(XCanFd *InstancePtr, u8 *SyncJumpWidth,
					u8 *TimeSegment2, u8 *TimeSegment1);
int XCanFd_SetFBaudRatePrescaler(XCanFd *InstancePtr, u8 Prescaler);
int XCanFd_SetFBitTiming(XCanFd *InstancePtr, u8 SyncJumpWidth,
					u8 TimeSegment2, u8 TimeSegment1);
void XCanFd_SetBitRateSwitch_DisableNominal(XCanFd *InstancePtr);
void XCanFd_SetBitRateSwitch_EnableNominal(XCanFd *InstancePtr);
u32 XCanFd_SetRxIntrWatermark(XCanFd *InstancePtr, s8 Threshold);
u32 XCanFd_SetRxIntrWatermarkFifo1(XCanFd *InstancePtr, s8 Threshold);
u32 XCanFd_SetTxEventIntrWatermark(XCanFd *InstancePtr, u8 Threshold);
u32 XCanFd_SetRxFilterPartition(XCanFd *InstancePtr, u8 FilterPartition);

/* Diagnostic functions in xcan_selftest.c */
int XCanFd_SelfTest(XCanFd *InstancePtr);

/* Functions in xcan_intr.c */
void XCanFd_InterruptEnable(XCanFd *InstancePtr, u32 Mask);
void XCanFd_InterruptDisable(XCanFd *InstancePtr, u32 Mask);
void XCanFd_InterruptClear(XCanFd *InstancePtr, u32 Mask);
void XCanFd_IntrHandler(void *InstancePtr);
int XCanFd_SetHandler(XCanFd *InstancePtr, u32 HandlerType,
				    void *CallBackFunc, void *CallBackRef);
void XCanFd_InterruptEnable_ReadyRqt(XCanFd *InstancePtr, u32 Mask);
void XCanFd_InterruptEnable_CancelRqt(XCanFd *InstancePtr, u32 Mask);
void XCanFd_InterruptDisable_ReadyRqt(XCanFd *InstancePtr, u32 Mask);
void XCanFd_InterruptDisable_CancelRqt(XCanFd *InstancePtr, u32 Mask);
void XCanFd_InterruptEnable_RxBuffFull(XCanFd *InstancePtr, u32 Mask,
						u32 RxBuffNumber);
void XCanFd_InterruptDisable_RxBuffFull(XCanFd *InstancePtr, u32 Mask,
						u32 RxBuffNumber);

/* Functions in xcanfd_sinit.c */
XCanFd_Config *XCanFd_LookupConfig(u16 Deviceid);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
