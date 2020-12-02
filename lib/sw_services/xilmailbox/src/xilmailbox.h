/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilmailbox.h
 * @addtogroup xilmailbox_v1_1
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
#define XILMBOX_MSG_TYPE_REQ	(0x00000001U)
#define XILMBOX_MSG_TYPE_RESP	(0x00000002U)
#define XMAILBOX_MAX_MSG_LEN	8U

/**************************** Type Definitions *******************************/
typedef void (*XMailbox_RecvHandler) (void *CallBackRefPtr);
typedef void (*XMailbox_ErrorHandler) (void *CallBackRefPtr, u32 ErrorMask);

/**
 * @XMbox_IPI_Send:	    Triggers an IPI to a destination CPU
 * @XMbox_IPI_SendData:     Sends an IPI message to a destination CPU
 * @XMbox_IPI_Recv:         Reads an IPI message
 * @RecvHandler:            Callback for rx IPI event
 * @ErrorHandler:           Callback for error event
 * @ErroRef:                To be passed to the error interrupt callback
 * @RecvRef:                To be passed to the receive interrupt callback.
 * @Agent:                  Used to store IPI Channel information.
 */
typedef struct XMboxTag {
	u32 (*XMbox_IPI_Send)(struct XMboxTag *InstancePtr, u8 Is_Blocking);
	u32 (*XMbox_IPI_SendData)(struct XMboxTag *InstancePtr, void *BufferPtr,
				  u32 MsgLen, u8 BufferType, u8 Is_Blocking);
	u32 (*XMbox_IPI_Recv)(struct XMboxTag *InstancePtr, void *BufferPtr,
			      u32 MsgLen, u8 BufferType);
	XMailbox_RecvHandler RecvHandler;
	XMailbox_ErrorHandler ErrorHandler;
	void *ErrorRefPtr;
	void *RecvRefPtr;
	XMailbox_Agent Agent;
} XMailbox;

/**
 * This typedef contains XMAILBOX Handler Types.
 */
typedef enum {
        XMAILBOX_RECV_HANDLER,     /**< For Recv Handler */
        XMAILBOX_ERROR_HANDLER,    /**< For Error Handler */
} XMailbox_Handler;

/************************** Function Prototypes ******************************/
u32 XMailbox_Initialize(XMailbox *InstancePtr, u8 DeviceId);
u32 XMailbox_Send(XMailbox *InstancePtr, u32 RemoteId, u8 Is_Blocking);
u32 XMailbox_SendData(XMailbox *InstancePtr, u32 RemoteId,
		      void *BufferPtr, u32 MsgLen, u8 BufferType, u8 Is_Blocking);
u32 XMailbox_Recv(XMailbox *InstancePtr, u32 SourceId, void *BufferPtr,
		  u32 MsgLen, u8 BufferType);
s32 XMailbox_SetCallBack(XMailbox *InstancePtr, XMailbox_Handler HandlerType,
			 void *CallBackFuncPtr, void *CallBackRefPtr);

#ifdef __cplusplus
}
#endif

#endif /* XILMAILBOX_H */
