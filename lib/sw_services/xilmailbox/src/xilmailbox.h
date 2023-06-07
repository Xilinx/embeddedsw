/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilmailbox.h
 * @addtogroup xilmailbox Overview
 * @{
 * @details
 *
 * The XilMailbox library provides the top-level hooks for sending or receiving
 * an inter-processor interrupt (IPI) message using the Zynq® UltraScale+ MPSoC
 * IPI hardware.
 *
 * For a full description of IPI features, please see the hardware spec.
 * This library supports the following features:
 *      - Triggering an IPI to a remote agent.
 *      - Sending an IPI message to a remote agent.
 *      - Callbacks for error and recv IPI events.
 *      - Reading an IPI message.
 *
 * <b> Software Initialization </b>
 * - IPI Initialization using XMailbox_Initalize() function. This step
 *   initializes a library instance for the given IPI channel.
 * - XMailbox_Send() function triggers an IPI to a remote agent.
 * - XMailbox_SendData() function sends an IPI message to a remote agent,
 *   Message type should be either XILMBOX_MSG_TYPE_REQ (OR) XILMBOX_MSG_TYPE_RESP.
 * - XMailbox_Recv() function reads an IPI message from a specified source agent,
 *   Message type should be either XILMBOX_MSG_TYPE_REQ (OR) XILMBOX_MSG_TYPE_RESP.
 * - XMailbox_SetCallBack() using this function user can register call backs
 *   for recv and error events.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   adk  14/02/19    Initial Release
 *       adk  06/03/19    In the mld file updated supported peripheral option
 *			  with A72 and PMC.
 * 1.3   sd   03/03/21    Doxygen Fixes
 * 1.6   sd   28/02/21    Add support for microblaze
 *       kpt  03/16/22    Added shared memory API's for IPI utilization
 * 1.7   sd   10/11/22    Fix a typo
 * 1.8   am   09/03/23    Added payload length macros
 *	 ht   05/30/23	  Added support for system device-tree flow.
 *
 *</pre>
 *
 *@note
 *****************************************************************************/
#ifndef XILMAILBOX_H
#define XILMAILBOX_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xilmailbox_ipips.h"

/************************** Constant Definitions *****************************/
#define XILMBOX_MSG_TYPE_REQ	(0x00000001U) /**< Message type buffer */
#define XILMBOX_MSG_TYPE_RESP	(0x00000002U) /**< Response type buffer */
#define XMAILBOX_MAX_MSG_LEN	8U     /**< Maximum message length */

/**
 * Payload lengths
 */
#define XMAILBOX_PAYLOAD_LEN_1U                (1U) /**< Payload length of size 1byte */
#define XMAILBOX_PAYLOAD_LEN_2U                (2U)	/**< Payload length of size 2byte */
#define XMAILBOX_PAYLOAD_LEN_3U                (3U)	/**< Payload length of size 3byte */
#define XMAILBOX_PAYLOAD_LEN_4U                (4U)	/**< Payload length of size 4byte */
#define XMAILBOX_PAYLOAD_LEN_5U                (5U)	/**< Payload length of size 5byte */
#define XMAILBOX_PAYLOAD_LEN_6U                (6U)	/**< Payload length of size 6byte */
#define XMAILBOX_PAYLOAD_LEN_7U                (7U)	/**< Payload length of size 7byte */

/**************************** Type Definitions *******************************/
typedef void (*XMailbox_RecvHandler) (void *CallBackRefPtr); /**< Receive handler */
typedef void (*XMailbox_ErrorHandler) (void *CallBackRefPtr, u32 ErrorMask); /**< Error handler */

/**
 * This typedef contains XMAILBOX shared memory state.
 */
typedef enum {
	XMAILBOX_SHARED_MEM_UNINITIALIZED = 0, /**< Shared memory uninitialized */
	XMAILBOX_SHARED_MEM_INITIALIZED, /**< Shared memory initialized */
} XMailbox_IpiSharedMemState;


typedef struct {
	u64 Address; /**< Address of the shared memory location */
	u32 Size; /**< Size of the shared memory location */
	XMailbox_IpiSharedMemState SharedMemState; /**< State of shared memory */
} XMailbox_IpiSharedMem;

/**
 * Data structure used to refer XilMailbox
 */
typedef struct XMboxTag {
	u32 (*XMbox_IPI_Send)(struct XMboxTag *InstancePtr, u8 Is_Blocking); /**< Triggers an IPI to a destination CPU */
	u32 (*XMbox_IPI_SendData)(struct XMboxTag *InstancePtr, void *BufferPtr,
				  u32 MsgLen, u8 BufferType, u8 Is_Blocking); /**< Sends an IPI message to a destination CPU */
	u32 (*XMbox_IPI_Recv)(struct XMboxTag *InstancePtr, void *BufferPtr,
			      u32 MsgLen, u8 BufferType); /**< Reads an IPI message */
	XMailbox_RecvHandler RecvHandler;   /**< Receive handler */
	XMailbox_ErrorHandler ErrorHandler; /**< Callback for rx IPI event */
	void *ErrorRefPtr; /**<  To be passed to the error interrupt callback */
	void *RecvRefPtr;  /**< To be passed to the receive interrupt callback */
	XMailbox_Agent Agent; /**< Agent to store IPI channel information */
	XMailbox_IpiSharedMem SharedMem; /**< shared memory segment */
} XMailbox; /**< XilMailbox structure */

/**
 * This typedef contains XMAILBOX Handler Types.
 */
typedef enum {
	XMAILBOX_RECV_HANDLER,     /**< For Recv Handler */
	XMAILBOX_ERROR_HANDLER,    /**< For Error Handler */
} XMailbox_Handler;

/************************** Function Prototypes ******************************/
/**
 * Functions for xilmailbox.c
 * @{
 */
#ifndef SDT
u32 XMailbox_Initialize(XMailbox *InstancePtr, u8 DeviceId);
#else
u32 XMailbox_Initialize(XMailbox *InstancePtr, UINTPTR BaseAddress);
#endif
u32 XMailbox_Send(XMailbox *InstancePtr, u32 RemoteId, u8 Is_Blocking);
u32 XMailbox_SendData(XMailbox *InstancePtr, u32 RemoteId,
		      void *BufferPtr, u32 MsgLen, u8 BufferType, u8 Is_Blocking);
u32 XMailbox_Recv(XMailbox *InstancePtr, u32 SourceId, void *BufferPtr,
		  u32 MsgLen, u8 BufferType);
s32 XMailbox_SetCallBack(XMailbox *InstancePtr, XMailbox_Handler HandlerType,
			 void *CallBackFuncPtr, void *CallBackRefPtr);
u32 XMailbox_SetSharedMem(XMailbox *InstancePtr, u64 Address, u32 Size);
u32 XMailbox_GetSharedMem(XMailbox *InstancePtr, u64 **Address);
int XMailbox_ReleaseSharedMem(XMailbox *InstancePtr);
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XILMAILBOX_H */
