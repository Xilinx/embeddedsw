/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XCANFD_H
#define XCANFD_H

/*****************************************************************************/
/**
*
* @file xcanfd.h
* @addtogroup canfd Overview
* @{
* @brief Defines the XCanFd driver API for the Xilinx CANFD controller.
*
* @details
*
* The Xilinx CANFD driver. This driver supports the Xilinx CANFD Controller.
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
* The device driver enables higher layer software (for example, an application)
* to communicate to the CANFD. The driver handles transmission and reception of
* CAN frames, as well as configuration of the controller. The driver is simply a
* pass-through mechanism between a protocol stack and the CANFD. A single device
* driver can support multiple CANFDs.
*
* Because the driver is a simple pass-through mechanism between a protocol stack
* and the CANFD, it does no assembly or disassembly of CANFD frames. The driver
* assumes that the protocol stack passes a correctly formatted CANFD frame to it
* for transmission. The driver does not validate the contents of an incoming
* frame.
*
* <b>Operation Modes</b>
*
* The CANFD controller supports the following modes of operation:
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
*     or the TX High Priority Buffer while in Sleep Mode. The CAN Controller
*     remains in Sleep Mode. Interrupts are generated when the CAN controller
*     enters Sleep mode or Wakes up from Sleep mode.
*   - <b>Loop Back Mode</b>: In Loop Back mode, the CAN controller transmits a
*     recessive bit stream on to the CAN Bus. Any message that is transmitted
*     is loop backed to the rx line and acknowledged. The CAN controller
*     thus receives any message that it transmits. It does not participate in
*     normal bus communication and does not receive any messages that are
*     transmitted by other CAN nodes. This mode is used for diagnostic
*     purposes.
*   - <b>Snoop mode</b>: In Snoop Mode, the CAN controller transmits recessive
*     bit stream on to CAN bus. It does not participate in normal bus
*     communication, but it receives messages that are transmitted by other CAN
*     nodes. Received messages are stored based on receive ID match result.
*     Error counters are disabled and cleared to 0. Reads to error counter
*     register returns zero.
*
* <b>Buffer Alignment</b>
*
* Frame buffers passed to the driver must be 32-bit aligned.
*
* <b>Receive Address Filtering</b>
*
* The device can be set to accept frames whose Identifiers match any of up to
* 32 filters set in the Acceptance Filter Mask/ID registers.
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
* XCanFd_VmInitialize(), instead of XCanFd_Initialize(), to initialize the
* device at first.
*
* <b>Threads</b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
exclusion
* must be satisfied by the layer above this driver.
*
* <b>Device Reset</b>
*
* Bus Off interrupt that can occur in the device requires a device reset. The
* application is responsible for resetting the device and re-configuring it
* based on its needs (the driver does not save the current configuration). When
* integrating into an RTOS, these reset and re-configure obligations are taken
* care of by the OS adapter software if it exists for that RTOS.
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
* is recommended that users leave asserts on during development.
*
* <b>Building the driver</b>
*
* The XCanFd driver is composed of several source files. This allows an
* application to build and link only those parts of the driver that are
* necessary.
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
* 2.3	sne  21/11/19 Used correct macro to access RX FIFO1 buffer.
* 2.3	sne  11/18/19 Fix for missing RX can packets on CANFD2.0.
* 2.3   sne  11/29/19 Fix for missing TX canfd packet while sending multiple packets
*                     by using multi buffer in loopback mode, CR# 1048366.
* 2.3	sne  12/18/19 Added Protocol Exception Event and BusOff event support
*		      CR#1048363 & CR#1046723.
* 2.3	sne  03/06/20 Fixed sending extra frames in XCanFd_Send_Queue API.
* 2.3	sne  03/09/20 Initialize IsPl of config structure.
* 2.4   sne  08/28/20 Modify Makefile to support parallel make execution.
* 2.5	sne  11/23/20 Fixed MISRAC violations.
* 2.7	sne  04/26/22 Corrected Return value of XCanFd_GetFreeBuffer().
* 2.8	ht   06/19/23 Added support for system device-tree flow.
* 2.8	gm   06/22/23 Add XCanFd_stop to support release node.
* 2.8	ht   07/18/23 Fixed GCC warnings.
* 2.12  vlt  11/05/25 Add 64-bit Addressing support.
* 2.12  vlt  12/30/25 Update Doxygen comments to include SDT flow details.
*       vlt  01/27/26 Fixed codespell issues
*       ht   02/26/26 Fix TX buffer transmitted check
* 2.13  vn   09/08/26 Align Doxygen comments with AMD standards
* 2.13  vn   09/30/26 Update Doxygen comments to AMD standards
* </pre>
*
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplatform_info.h"
#include "xstatus.h"
#include "xcanfd_hw.h"

/************************** Constant Definitions *****************************/
/** @name CAN normal Bit rate fields
 *  @{
 */
#if defined (CANFD_v1_0)
#define XCANFD_MAX_SJW_VALUE 0x10  /**< Synchronization Jump Width. */
#define XCANFD_MAX_TS1_VALUE 0x40  /**< Time Segment 1. */
#define XCANFD_MAX_TS2_VALUE 0x20  /**< Time Segment 2. */
#else
#define XCANFD_MAX_SJW_VALUE 0x80  /**< Synchronization Jump Width. */
#define XCANFD_MAX_TS1_VALUE 0x100 /**< Time Segment 1. */
#define XCANFD_MAX_TS2_VALUE 0x80  /**< Time Segment 2. */
#endif
/** @} */
/** @name CAN Fast Bit rate fields
 *  @{
 */
#if defined (CANFD_v1_0)
/** Synchronization Jump Width for Data Phase. */
#define XCANFD_MAX_F_SJW_VALUE 0x03
#define XCANFD_MAX_F_TS1_VALUE 0x0F /**< Time Segment 1 for Data Phase. */
#define XCANFD_MAX_F_TS2_VALUE 0x07 /**< Time Segment 2 for Data Phase. */
#else
/** Synchronization Jump Width for Data Phase. */
#define XCANFD_MAX_F_SJW_VALUE 0x10
#define XCANFD_MAX_F_TS1_VALUE 0x20 /**< Time Segment 1 for Data Phase. */
#define XCANFD_MAX_F_TS2_VALUE 0x10 /**< Time Segment 2 for Data Phase. */
#endif
/** @} */
/** @name CAN operation modes
 *  @{
 */
#define XCANFD_MODE_CONFIG	0x00000001 /**< Configuration mode. */
#define XCANFD_MODE_NORMAL	0x00000002 /**< Normal mode. */
#define XCANFD_MODE_LOOPBACK	0x00000004 /**< Loop Back mode. */
#define XCANFD_MODE_SLEEP	0x00000008 /**< Sleep mode. */
#define XCANFD_MODE_SNOOP	0x00000010 /**< Snoop mode. */
/** Auto Bus-Off Recovery. */
#define XCANFD_MODE_ABR		0x00000020
/** Start Bus-Off Recovery. */
#define XCANFD_MODE_SBR		0x00000040
/** Protocol Exception mode. */
#define XCANFD_MODE_PEE		0x00000080
/** Disable Auto Retransmission mode. */
#define XCANFD_MODE_DAR		0x0000000A
#define XCANFD_MODE_BR		0x0000000B /**< Bus-Off Recovery Mode. */
#define XCANFD_RX_FIFO_0	         0 /**< Selection for RX Fifo 0. */
#define XCANFD_RX_FIFO_1	         1 /**< Selection for RX Fifo 1. */
/** @} */

/** @name Callback identifiers used as parameters to XCanFd_SetHandler()
 *  @{
 */
/** Handler type for frame sending interrupt. */
#define XCANFD_HANDLER_SEND   1
/** Handler type for frame reception interrupt. */
#define XCANFD_HANDLER_RECV   2
#define XCANFD_HANDLER_ERROR  3 /**< Handler type for error interrupt. */
#define XCANFD_HANDLER_EVENT  4 /**< Handler type for all other interrupts. */
/** @} */

/**************************** Type Definitions *******************************/

/**
 * @brief Contains configuration information for a device.
 */
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< Unique ID of device. */
#else
	char *Name;		/**< Name of the device. */
#endif
	UINTPTR BaseAddress;	/**< Register base address. */
	u32 Rx_Mode;		/**< 1-Mailbox 0-sequential. */
	u32 NumofRxMbBuf;	/**< Number of RxBuffers. */
	u32 NumofTxBuf;		/**< Number of TxBuffers. */
#ifndef SDT
	u32 IsPl;		/**< IsPl, 1= AXI CANFD instance,0= CANFD
				 * instance. */
#else
	u16 IntrId;		/**< Bits[11:0] Interrupt-id Bits[15:12] trigger
				 * type and level flags. */
	UINTPTR IntrParent;	/**< Bit[0] Interrupt parent type Bit[64/32:1]
				 * Parent base address. */
#endif
} XCanFd_Config;

/************************** Variable Definitions *****************************/
/**
 * @brief Holds the configuration entry for each CAN FD instance.
 */
extern XCanFd_Config XCanFd_ConfigTable[];

/*****************************************************************************/
/**
* @brief Defines the callback type for frame sending and reception interrupts.
*
* @param[in]    CallBackRef Callback reference passed in by the upper layer
*		when setting the callback functions, and passed back to the
*		upper layer when the callback is invoked.
******************************************************************************/
typedef void (*XCanFd_SendRecvHandler) (void *CallBackRef);

/******************************************************************************/
/**
* @brief Defines the callback type for the error interrupt.
*
* @param[in]    CallBackRef Callback reference passed in by the upper layer
*		when setting the callback functions, and passed back to the
*		upper layer when the callback is invoked.
* @param[in]    ErrorMask Bit mask indicating the cause of the error. Its
*               value equals the bitwise OR of one or more XCANFD_ESR_* values
*               defined in xcan_l.h.
******************************************************************************/
typedef void (*XCanFd_ErrorHandler) (void *CallBackRef, u32 ErrorMask);

/*****************************************************************************/
/**
* @brief Defines the callback type for all other interrupts.
*
* Other interrupts are all interrupts except the frame send, frame receive, and
* error interrupts.
*
* @param[in]    CallBackRef Callback reference passed in by the upper layer
*		when setting the callback functions, and passed back to the
*		upper layer when the callback is invoked.
* @param[in]    Mask Bit mask indicating the pending interrupts. Its value
*               equals the bitwise OR of one or more XCANFD_IXR_* defined in
*               xcanfd_hw.h.
******************************************************************************/
typedef void (*XCanFd_EventHandler) (void *CallBackRef, u32 Mask);

/*****************************************************************************/
/**
 * @brief Holds the XCanFd driver instance data.
 *
 * Allocate a variable of this type for every CAN device in the system. A
 * pointer to a variable of this type is then passed to the driver API
 * functions.
 */
typedef struct {
	XCanFd_Config CanFdConfig;	/**< Device Configuration. */
	u32 IsReady;			/**< Device is initialized and ready. */
	u32 MultiBuffTrr;		/**< Used in multibuffer send case to
					 * update TRR Register. */
	u32 GlobalTrrValue;		/**< Used in multibuffer send case to
					 * update TRR Register. */
	u32 GlobalTrrMask;		/**< Used in multibuffer send case to
					 * update TRR Register. */

	XCanFd_SendRecvHandler SendHandler;
					/**< Callback for TXOK interrupts. */
	void *SendRef;			/**< Reference passed to the TXOK
					 * interrupt callback. */

	XCanFd_SendRecvHandler RecvHandler;
					/**< Callback for RXOK interrupt. */
	void *RecvRef;			/**< Reference passed to the RXOK
					 * interrupt callback. */

	XCanFd_ErrorHandler ErrorHandler;
					/**< Callback for ERROR interrupt. */
	void *ErrorRef;			/**< Reference passed to the ERROR
					 * interrupt callback. */

	XCanFd_EventHandler EventHandler;
					/**< Callback for RXOFLW/RXUFLW/TXBFLL/
					 * TXFLL/Wakeup/Sleep/Bus off/ARBLST
					 * interrupts. */
	void *EventRef;			/**< Reference passed to the
					 * EventHandler callback. */

} XCanFd;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* @brief Checks if the transmission is complete.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @return	- TRUE if the transmission is done (completed).
*		- FALSE if the transmission is not completed.
*
* @note		C-Style signature:
*		int XCanFd_IsTxDone(XCanFd *InstancePtr);
*****************************************************************************/
#define XCanFd_IsTxDone(InstancePtr) \
	((XCanFd_ReadReg(((InstancePtr)->CanFdConfig.BaseAddress), \
			 XCANFD_ISR_OFFSET) & XCANFD_IXR_TXOK_MASK) ? TRUE : FALSE)

/****************************************************************************/
/**
*
* @brief Calculates CAN message identifier value given identifier field
* values.
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
*		u32 XCanFd_CreateIdValue(u32 StandardId, u32 SubRemoteTransReq,
*			u32 IdExtension, u32 ExtendedId,
*			u32 RemoteTransReq);
*               <br><br>
* 		Read the CAN specification for meaning of each parameter.
*****************************************************************************/
#define XCanFd_CreateIdValue(StandardId, SubRemoteTransReq, IdExtension, \
			     ExtendedId, RemoteTransReq) \
((((StandardId) << (u32)XCANFD_IDR_ID1_SHIFT) & (u32)XCANFD_IDR_ID1_MASK) | \
 (((SubRemoteTransReq) << (u32)XCANFD_IDR_SRR_SHIFT) & (u32)XCANFD_IDR_SRR_MASK) | \
 (((IdExtension) << (u32)XCANFD_IDR_IDE_SHIFT) & (u32)XCANFD_IDR_IDE_MASK) | \
 (((ExtendedId) << (u32)XCANFD_IDR_ID2_SHIFT) & (u32)XCANFD_IDR_ID2_MASK) | \
 ((RemoteTransReq) & (u32)XCANFD_IDR_RTR_MASK))

/****************************************************************************/
/**
*
* @brief Calculates the Data Length Code register value for CAN FD frames.
*
* Calculates the value from the given Data Length Code value, with EDL set to 1
* (Only Can FD frames).
*
* @param[in]    DataLengCode Indicates Data Length Code value.
*
* @return	Value that can be assigned to Data Length Code register.
*
* @note		C-Style signature:
*		u32 XCanFd_Create_CanFD_DlcValue(u32 DataLengCode);
*               <br><br>
* 		Read the CAN specification for meaning of Data Length Code.
*****************************************************************************/
#define XCanFd_Create_CanFD_DlcValue(DataLengCode) \
	((((DataLengCode) << XCANFD_DLCR_DLC_SHIFT) & XCANFD_DLCR_DLC_MASK) \
	 |(XCANFD_DLCR_EDL_MASK))

/****************************************************************************/
/**
*
* @brief Calculates the Data Length Code register value with BRS set.
*
* Calculates the value from the given Data Length Code value, with EDL set to 1
* (Only Can FD frames) and the BRS bit set.
*
* @param[in]    DataLengCode Indicates Data Length Code value.
*
* @return	Value that can be assigned to Data Length Code register.
*
* @note		C-Style signature:
*		u32 XCanFd_Create_CanFD_Dlc_BrsValue(u32 DataLengCode);
*               <br><br>
*	 	Read the CAN specification for meaning of Data Length Code.
*****************************************************************************/
#define XCanFd_Create_CanFD_Dlc_BrsValue(DataLengCode) \
	((((DataLengCode) << (u32)XCANFD_DLCR_DLC_SHIFT) & (u32)XCANFD_DLCR_DLC_MASK) \
	 |(XCANFD_DLCR_EDL_MASK) |(XCANFD_DLCR_BRS_MASK))

/****************************************************************************/
/**
*
* @brief Calculates the Data Length Code register value for standard frames.
*
* Calculates the value from the given Data Length Code value (Only Standard Can
* frames).
*
* @param[in]    DataLengCode Indicates Data Length Code value.
*
* @return	Value that can be assigned to Data Length Code register.
*
* @note		C-Style signature:
*		u32 XCanFd_CreateDlcValue(u32 DataLengCode);
*               <br><br>
*	 	Read the CAN specification for meaning of Data Length Code.
*****************************************************************************/
#define XCanFd_CreateDlcValue(DataLengCode) \
	((((DataLengCode) << XCANFD_DLCR_DLC_SHIFT) & XCANFD_DLCR_DLC_MASK))

/****************************************************************************/
/**
*
* @brief Checks whether Particular Buffer is Transmitted or not.
*
* Transmit Ready Request Register gives which Buffer is transmitted.
* For example, if you trigger the Buffer1, then:
* 			TRR Reg : 0x00000001
* After transmission, Core clears the bit:
* 			TRR Reg : 0x00000000
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]    TxBuffer Buffer where driver has written user data.
*
* @return
*               - TRUE if the TRR bit of the buffer is cleared (buffer
*                 transmitted).
*               - FALSE if the TRR bit of the buffer is still set.
*
* @note         C-style signature: int XCanFd_IsBufferTransmitted(XCanFd
*               *InstancePtr, u32 TxBuffer)
*
*****************************************************************************/
#define XCanFd_IsBufferTransmitted(InstancePtr,TxBuffer)	\
	(((XCanFd_ReadReg((InstancePtr)->CanFdConfig.BaseAddress, \
			  XCANFD_TRR_OFFSET) & ((u32)1 << (TxBuffer))) \
	  != (u32)0U) ? FALSE : TRUE)


/****************************************************************************/
/**
*
* @brief Initializes CurrentBuffer[32] to zeros.
*
* @param[out]   InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @note         C-style signature:
*               void MAKE_CURRENTBUFFER_ZERO(XCanFd *InstancePtr)
*               Makes bufferstatus zero.
*
*****************************************************************************/
#define MAKE_CURRENTBUFFER_ZERO(InstancePtr) \
	for (BufferNr = 0;BufferNr <= 31; BufferNr++) \
		InstancePtr->FreeBuffStatus[BufferNr] = 0

/*****************************************************************************/
/**
*
* @brief Returns the TXBUFFER ID Offset.
*
* @param[in]    FreeBuffer Buffer number to locate the FIFO Index.
*
* @return       TXBUFFER ID offset.
*
* @note         C-style signature:
*               u32 XCANFD_TXID_OFFSET(u32 FreeBuffer)
*
*****************************************************************************/
#define XCANFD_TXID_OFFSET(FreeBuffer) \
	(XCANFD_TXFIFO_0_BASE_ID_OFFSET+((UINTPTR)FreeTxBuffer*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
*
* @brief Returns the TXBUFFER DLC Offset.
*
* @param[in]    FreeBuffer Buffer number to locate the FIFO.
*
* @return       TXBUFFER DLC offset.
*
* @note         C-style signature:
*               u32 XCANFD_TXDLC_OFFSET(u32 FreeBuffer)
*
 *****************************************************************************/
#define XCANFD_TXDLC_OFFSET(FreeBuffer) \
	(XCANFD_TXFIFO_0_BASE_DLC_OFFSET+((UINTPTR)FreeTxBuffer*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
*
* @brief Returns the TXBUFFER DW Offset.
*
* @param[in]    FreeBuffer Buffer number to locate the FIFO.
*
* @return       TXBUFFER DW offset.
*
* @note         C-style signature:
*               u32 XCANFD_TXDW_OFFSET(u32 FreeBuffer)
*
*****************************************************************************/
#define XCANFD_TXDW_OFFSET(FreeBuffer) \
	(XCANFD_TXFIFO_0_BASE_DW0_OFFSET+((UINTPTR)FreeTxBuffer*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
*
* @brief Returns the TX Event Buffer ID Offset.
*
* @param[in]    TXEVENTIndex Buffer number to locate the TXE FIFO Index.
*
* @return       TX Event Buffer ID offset.
*
* @note         C-style signature: u32 XCANFD_TXEID_OFFSET(u32 TXEVENTIndex)
*               This API is meant to be used with IP with CanFD 2.0 spec
*               support only.
*
*****************************************************************************/
#define XCANFD_TXEID_OFFSET(TXEVENTIndex) \
	(XCANFD_TXEFIFO_0_BASE_ID_OFFSET+((UINTPTR)TXEVENTIndex*XCANFD_TXE_MESSAGE_SIZE))

/*****************************************************************************/
/**
*
* @brief Returns the TX Event Buffer DLC Offset.
*
* @param[in]    TXEVENTIndex Buffer number to locate the FIFO.
*
* @return       TX Event Buffer DLC offset.
*
* @note         C-style signature: u32 XCANFD_TXEDLC_OFFSET(u32 TXEVENTIndex)
*               This API is meant to be used with IP with CanFD 2.0 spec
*               support only.
*
 *****************************************************************************/
#define XCANFD_TXEDLC_OFFSET(TXEVENTIndex) \
	(XCANFD_TXEFIFO_0_BASE_DLC_OFFSET+((UINTPTR)TXEVENTIndex*XCANFD_TXE_MESSAGE_SIZE))
/*****************************************************************************/
/**
*
* @brief Returns the RXBUFFER ID Offset.
*
* @param[in]    ReadIndex Buffer number to locate the FIFO.
*
* @return       RXBUFFER ID offset.
*
* @note         C-style signature:
*               u32 XCANFD_RXID_OFFSET(u32 ReadIndex)
*
*****************************************************************************/
#define XCANFD_RXID_OFFSET(ReadIndex) \
	(XCANFD_RXFIFO_0_BASE_ID_OFFSET+((UINTPTR)ReadIndex*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
*
* @brief Returns the RXBUFFER DLC Offset.
*
* @param[in]    ReadIndex Buffer number to locate the FIFO.
*
* @return       RXBUFFER DLC offset.
*
* @note         C-style signature:
*               u32 XCANFD_RXDLC_OFFSET(u32 ReadIndex)
*
*****************************************************************************/
#define XCANFD_RXDLC_OFFSET(ReadIndex) \
	(XCANFD_RXFIFO_0_BASE_DLC_OFFSET+((UINTPTR)ReadIndex*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
*
* @brief Returns the RXBUFFER DW Offset.
*
* @param[in]    ReadIndex Buffer number to locate the FIFO.
*
* @return       RXBUFFER DW offset.
*
* @note         C-style signature:
*               u32 XCANFD_RXDW_OFFSET(u32 ReadIndex)
*
*****************************************************************************/
#define XCANFD_RXDW_OFFSET(ReadIndex) \
	(XCANFD_RXFIFO_0_BASE_DW0_OFFSET+((UINTPTR)ReadIndex*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
*
* @brief Returns the RXBUFFER ID Offset for FIFO 1.
*
* @param[in]    ReadIndex Buffer number to locate the FIFO.
*
* @return       RXBUFFER ID offset for FIFO 1.
*
* @note         C-style signature: u32 XCANFD_FIFO_1_RXID_OFFSET(u32 ReadIndex)
*               This API is meant to be used with IP with CanFD 2.0 spec
*               support only.
*
*****************************************************************************/
#define XCANFD_FIFO_1_RXID_OFFSET(ReadIndex) \
	(XCANFD_RXFIFO_1_BUFFER_0_BASE_ID_OFFSET+((UINTPTR)ReadIndex*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
*
* @brief Returns the RXBUFFER DLC Offset for FIFO 1.
*
* @param[in]    ReadIndex Buffer number to locate the FIFO.
*
* @return       RXBUFFER DLC offset for FIFO 1.
*
* @note         C-style signature: u32 XCANFD_FIFO_1_RXDLC_OFFSET(u32 ReadIndex)
*               This API is meant to be used with IP with CanFD 2.0 spec
*               support only.
*
*****************************************************************************/
#define XCANFD_FIFO_1_RXDLC_OFFSET(ReadIndex) \
	(XCANFD_RXFIFO_1_BUFFER_0_BASE_DLC_OFFSET+((UINTPTR)ReadIndex*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
*
* @brief Returns the RXBUFFER DW Offset for FIFO 1.
*
* @param[in]    ReadIndex Buffer number to locate the FIFO.
*
* @return       RXBUFFER DW offset for FIFO 1.
*
* @note         C-style signature: u32 XCANFD_FIFO_1_RXDW_OFFSET(u32 ReadIndex)
*               This API is meant to be used with IP with CanFD 2.0 spec
*               support only.
*
*****************************************************************************/
#define XCANFD_FIFO_1_RXDW_OFFSET(ReadIndex) \
	(XCANFD_RXFIFO_1_BUFFER_0_BASE_DW0_OFFSET+((UINTPTR)ReadIndex*XCANFD_MAX_FRAME_SIZE))
/*****************************************************************************/
/**
*
* @brief Returns the RCS Register Offset.
*
* @param[in]    NoCtrlStatus To locate RCS Registers.
*
* @return       RCS register offset.
*
* @note         C-style signature:
*               u32 XCANFD_RCS_OFFSET(u32 NoCtrlStatus)
*               NoCtrlStatus = 0 -> RCS0, 1 -> RCS1, 2 -> RCS2.
*
*****************************************************************************/
#define XCANFD_RCS_OFFSET(NoCtrlStatus)	\
	(XCANFD_RCS0_OFFSET+((UINTPTR)NoCtrlStatus*4U))

/*****************************************************************************/
/**
*
* @brief Returns the AFMR Register Offset.
*
* @param[in]    FilterIndex Index number of Mask Register.
*
* @return       AFMR register offset.
*
* @note         C-style signature:
*               u32 XCANFD_AFMR_OFFSET(u32 FilterIndex)
*
*****************************************************************************/
#define XCANFD_AFMR_OFFSET(FilterIndex)	\
	(XCANFD_AFMR_BASE_OFFSET+\
	 ((UINTPTR)FilterIndex*8U))

/*****************************************************************************/
/**
*
* @brief Returns the AFIDR Register Offset.
*
* @param[in]    FilterIndex Index of Id Register.
*
* @return       AFIDR register offset.
*
* @note         C-style signature:
*               u32 XCANFD_AFIDR_OFFSET(u32 FilterIndex)
*
*****************************************************************************/
#define XCANFD_AFIDR_OFFSET(FilterIndex)	\
	(XCANFD_AFIDR_BASE_OFFSET+\
	 ((UINTPTR)FilterIndex*8U))

/*****************************************************************************/
/**
*
* @brief Returns the MAILBOX MODE RXMASK Offset.
*
* @param[in]    BufferNr Buffer number to locate the FIFO.
*
* @return       MAILBOX MODE RXMASK offset.
*
* @note         C-style signature:
*               u32 XCANFD_MAILBOX_MASK_OFFSET(u32 BufferNr)
*
*****************************************************************************/
#define XCANFD_MAILBOX_MASK_OFFSET(BufferNr)	\
	(XCANFD_MAILBOX_RB_MASK_BASE_OFFSET+((UINTPTR)BufferNr*4U))

/*****************************************************************************/
/**
*
* @brief Returns the MAILBOX MODE ID Offset.
*
* @param[in]    BufferNr Buffer number to locate the FIFO.
*
* @return       MAILBOX MODE ID offset.
*
* @note         C-style signature:
*               u32 XCANFD_MAILBOX_ID_OFFSET(u32 BufferNr)
*
*****************************************************************************/
#define XCANFD_MAILBOX_ID_OFFSET(BufferNr)	\
	(XCANFD_RXFIFO_0_BASE_ID_OFFSET+((UINTPTR)BufferNr*XCANFD_MAX_FRAME_SIZE))

/*****************************************************************************/
/**
*
* @brief Returns Design mode.
*
* 1- Mailbox
* 0- Sequential
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @return       Design mode: 1 for Mailbox, 0 for Sequential.
*
* @note         C-style signature:
*               u32 XCANFD_GET_RX_MODE(XCanFd *InstancePtr)
*
*****************************************************************************/
#define XCANFD_GET_RX_MODE(InstancePtr)\
	((InstancePtr)->CanFdConfig.Rx_Mode)

/*****************************************************************************/
/**
*
* @brief Resets the CAN device.
*
* Calling this function resets the device immediately, and any pending
* transmission or reception is terminated at once. Both Object Layer and
* Transfer Layer are reset. This function does not reset the Physical Layer.
* All registers are reset to the default values, and no previous status is
* restored. TX FIFO, RX FIFO and TX High Priority Buffer are also reset.
*
* When an internal error requires a reset, the driver notifies the upper layer
* software through the error status code or interrupts. The upper layer software
* is responsible for calling this Reset function and then re-configuring the
* device.
*
* The CAN device is in Configuration Mode immediately after this function
* returns.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @note         C-style signature:
*               void XCanFd_Reset(XCanFd *InstancePtr)
*
*****************************************************************************/
#define XCanFd_Reset(InstancePtr)	\
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_SRR_OFFSET,\
			XCANFD_SRR_SRST_MASK)

/*****************************************************************************/
/**
*
* @brief Reads Error Status value from Error Status Register (ESR).
*
* Use the XCANFD_ESR_* constants defined in xcanfd_hw.h to interpret the
* returned value.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @return	The 32-bit value read from Error Status Register.
*
* @note         C-style signature:
*               u32 XCanFd_GetBusErrorStatus(XCanFd *InstancePtr)
*
*****************************************************************************/
#define XCanFd_GetBusErrorStatus(InstancePtr)	\
	XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_ESR_OFFSET)

/*****************************************************************************/
/**
*
* @brief Returns Status value from Status Register (SR).
*
* Use the XCANFD_SR_* constants defined in xcanfd_hw.h to interpret the
* returned value.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @return	The 32-bit value read from Status Register.
*
* @note         C-style signature:
*               u32 XCanFd_GetStatus(XCanFd *InstancePtr)
*
*****************************************************************************/
#define XCanFd_GetStatus(InstancePtr)	\
	XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_SR_OFFSET)

/*****************************************************************************/
/**
*
* @brief Clears one or more previously set bits in the Error Status
* Register (ESR).
*
* Use the XCANFD_ESR_* constants defined in xcanfd_hw.h to create the value to
* pass in. If a bit was cleared in Error Status Register before this function is
* called, it is not touched.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]    Mask 32-bit mask used to clear bits in Error Status Register.
*               Combine multiple XCANFD_ESR_* values with a bitwise OR to clear
*               multiple bits.
*
* @note         C-style signature:
*               void XCanFd_ClearBusErrorStatus(XCanFd *InstancePtr, u32 Mask)
*
*****************************************************************************/
#define XCanFd_ClearBusErrorStatus(InstancePtr,Mask)	\
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_ESR_OFFSET, \
			Mask)

/*****************************************************************************/
/**
*
* @brief Returns the transceiver delay compensation offset.
*
* This function can be called when user sends multiple Buffers using
* Addto_Queue() and XCanFd_Send_Queue().
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @return       Transceiver delay compensation offset.
*
* @note         C-style signature:
*               u32 XCanFd_Get_Tranceiver_Delay_CompensationOffset(
*               XCanFd *InstancePtr)
*
*****************************************************************************/
#define XCanFd_Get_Tranceiver_Delay_CompensationOffset(InstancePtr)	\
	((XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,\
			 XCANFD_F_BRPR_OFFSET) & XCANFD_F_BRPR_TDCMASK) >> 8)

/*****************************************************************************/
/**
*
* @brief Clears Time Stamp Counter Value.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @note         C-style signature:
*               void XCanFd_ClearTImeStamp_Count(XCanFd *InstancePtr)
*
*****************************************************************************/
#define XCanFd_ClearTImeStamp_Count(InstancePtr)	\
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress, \
			XCANFD_TIMESTAMPR_OFFSET,XCANFD_CTS_MASK)

/*****************************************************************************/
/**
*
* @brief Returns Time Stamp Counter Value.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @return       TimeStampCount.
*
* @note         C-style signature:
*               u32 XCanFd_GetTImeStamp_Count(XCanFd *InstancePtr)
*
*****************************************************************************/
#define XCanFd_GetTImeStamp_Count(InstancePtr)	\
	(XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,\
			XCANFD_TIMESTAMPR_OFFSET) >> 16)

/****************************************************************************/
/**
*
* @brief Returns the Rx water Mark threshold Value.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance.
*
* @return	Threshold Value.
*
* @note         C-style signature:
*               u32 XCanFd_GetRxIntrWatermark(XCanFd *InstancePtr)
*
*****************************************************************************/
#define XCanFd_GetRxIntrWatermark(InstancePtr)	\
	(XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, \
			XCANFD_WIR_OFFSET) & XCANFD_WIR_MASK)

/****************************************************************************/
/**
*
* @brief Returns the enabled interrupts.
*
* Use the XCANFD_IXR_* constants defined in xcanfd_hw.h to interpret the
* returned value.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @return       Enabled interrupts in a 32-bit format.
*
* @note         C-style signature:
*               u32 XCanFd_InterruptGetEnabled(XCanFd *InstancePtr)
*
*****************************************************************************/
#define XCanFd_InterruptGetEnabled(InstancePtr)	\
	XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_IER_OFFSET)

/****************************************************************************/
/**
*
* @brief Returns interrupt status read from Interrupt Status Register.
*
* Use the XCANFD_IXR_* constants defined in xcanfd_hw.h to interpret the
* returned value.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @return	The value stored in Interrupt Status Register.
*
* @note         C-style signature:
*               u32 XCanFd_InterruptGetStatus(XCanFd *InstancePtr)
*
*****************************************************************************/
#define XCanFd_InterruptGetStatus(InstancePtr)	\
	XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_ISR_OFFSET)

/****************************************************************************/
/**
*
* @brief Returns the number of RCS registers to access.
*
* The number varies because in Mail box mode user can configure 48,32,16 Rx
* Buffers.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @return       Number of RCS registers: 3 for 48, 2 for 32, and 1 for 16 Rx
*               Buffers.
*
* @note         C-style signature:
*               u32 XCanFd_Get_NofRxBuffers(XCanFd *InstancePtr)
*
*****************************************************************************/
#define XCanFd_Get_NofRxBuffers(InstancePtr)	\
	((InstancePtr->CanFdConfig.NumofRxMbBuf == 48) ? (3) \
	 :((InstancePtr->CanFdConfig.NumofRxMbBuf == 32) ? \
	   (2) : (1)))

/****************************************************************************/
/**
*
* @brief Returns Number of RxBuffers.
*
* User can Design RxBuffers as 48,32,16.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @return       Number of RxBuffers.
*
* @note         C-style signature:
*               u32 XCanFd_Get_RxBuffers(XCanFd *InstancePtr)
*
*****************************************************************************/
#define XCanFd_Get_RxBuffers(InstancePtr)	\
	InstancePtr->CanFdConfig.NumofRxMbBuf;

/****************************************************************************/
/**
*
* @brief Returns Number with right most bit set from the target input value.
*
* @param[in]    Var Target value.
*
* @return	Number with right most bit set from the target value.
*
* @note         C-style signature:
*               u32 XCanFD_Check_TrrVal_Set_Bit(u32 Var)
*
*****************************************************************************/
#define XCanFD_Check_TrrVal_Set_Bit(Var)      ((Var)&(~(Var) + (u32)1))


/* Functions in xcan.c */
int XCanFd_CfgInitialize(XCanFd *InstancePtr, XCanFd_Config *ConfigPtr,
			 UINTPTR EffectiveAddr);
int XCanFd_stop(XCanFd *InstancePtr);
u8 XCanFd_GetMode(XCanFd *InstancePtr);
void XCanFd_EnterMode(XCanFd *InstancePtr, u8 OperationMode);
void XCanFd_GetBusErrorCounter(XCanFd *InstancePtr, u8 *RxErrorCount,
			       u8 *TxErrorCount);

int XCanFd_Send(XCanFd *InstancePtr, u32 *FramePtr, u32 *TxBufferNumber);
void XCanFd_AcceptFilterEnable(XCanFd *InstancePtr, u32 FilterIndexMask);
void XCanFd_AcceptFilterDisable(XCanFd *InstancePtr, u32 FilterIndexMask);
u32 XCanFd_AcceptFilterGetEnabled(XCanFd *InstancePtr);
int XCanFd_AcceptFilterSet(XCanFd *InstancePtr, u32 FilterIndex,
			   u32 MaskValue, u32 IdValue);
void XCanFd_AcceptFilterGet(XCanFd *InstancePtr, u32 FilterIndex,
			    u32 *MaskValue, u32 *IdValue);
#ifndef SDT
XCanFd_Config *XCanFd_LookupConfig(u16 DeviceId);
#else
XCanFd_Config *XCanFd_LookupConfig(UINTPTR BaseAddress);
#endif
#ifndef SDT
XCanFd_Config *XCanFd_GetConfig(unsigned int InstanceIndex);
#endif
int XCanFd_GetDlc2len(u32 Dlc, u32 Edl);
u8 XCanFd_GetLen2Dlc(int len);
u32 XCanFd_GetFreeBuffer(XCanFd *InstancePtr);
int XCanFd_Send_Queue(XCanFd *InstancePtr);
int XCanFd_Addto_Queue(XCanFd *InstancePtr, u32 *FramePtr, u32 *TxBufferNumber);
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
void XCanFd_Pee_BusOff_Handler(XCanFd *InstancePtr);

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


#ifdef __cplusplus
}
#endif

/** @} */
#endif /* end of protection macro */
