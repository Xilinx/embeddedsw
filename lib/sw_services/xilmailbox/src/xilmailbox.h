/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XILMAILBOX_H
#define XILMAILBOX_H

/*****************************************************************************/
/**
 *
 * @file xilmailbox.h
 * @addtogroup xilmailbox Overview
 * @{
 * @brief Defines the XilMailbox library interface for IPI messaging.
 *
 * @details
 *
 * The XilMailbox library provides the top-level hooks for sending or receiving
 * an inter-processor interrupt (IPI) message using the Zynq® UltraScale+ MPSoC
 * IPI hardware.
 *
 * For a full description of IPI features, see the hardware specification.
 * This library supports the following features:
 *      - Triggering an IPI to a remote agent.
 *      - Sending an IPI message to a remote agent.
 *      - Callbacks for error and receive IPI events.
 *      - Reading an IPI message.
 *
 * <b> Software Initialization </b>
 * - The XMailbox_Initialize() function initializes IPI. This step
 *   initializes a library instance for the given IPI channel.
 * - The XMailbox_Send() function triggers an IPI to a remote agent.
 * - The XMailbox_SendData() function sends an IPI message to a remote agent.
 *   The message type should be either XILMBOX_MSG_TYPE_REQ or
 *   XILMBOX_MSG_TYPE_RESP.
 * - The XMailbox_Recv() function reads an IPI message from a specified source
 *   agent. The message type should be either XILMBOX_MSG_TYPE_REQ or
 *   XILMBOX_MSG_TYPE_RESP.
 * - Use the XMailbox_SetCallBack() function to register callbacks for receive
 *   and error events.
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
 * 1.9   ht   09/12/23	  Added code for versioning of library.
 *	 sd   11/23/23	  Clear the interrupts after calling the user handler.
 * 1.11  ht   11/12/24    Update description of XMAILBOX_MAX_MSG_LEN
 *       ht   02/18/25    Update Max Message length to XIPIPSU_MAX_MSG_LEN to
 *                        accommodate for CRC bytes when IPI CRC is enabled.
 * 1.12  ht   09/02/25    IPI operation relied on the RemoteId stored in XMailbox
 * 			  Instance, which could lead to race conditions and data
 * 			  corruption when handling multiple requests from
 * 			  different IPI channels concurrently.
 * 			  Use RemoteId based APIs to support concurrent IPI
 * 			  channel requests.
 * 1.14  vn   08/26/26    Fix Doxygen warnings.
 * 1.14  vn   09/30/26    Align Doxygen comments with AMD standards
 *
 *</pre>
 *
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xil_util.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xilmailbox_ipips.h"

/************************** Constant Definitions *****************************/
/** @name Message types and maximum message length
 * @{
 */
#define XILMBOX_MSG_TYPE_REQ	(0x00000001U) /**< Message type buffer. */
#define XILMBOX_MSG_TYPE_RESP	(0x00000002U) /**< Response type buffer. */
/** Maximum number of messages (each 4 bytes long). */
#define XMAILBOX_MAX_MSG_LEN	XIPIPSU_MAX_MSG_LEN
/** @} */

/** @name Payload lengths
 * @{
 */
/** Payload length of size 1 byte. */
#define XMAILBOX_PAYLOAD_LEN_1U                (1U)
/** Payload length of size 2 bytes. */
#define XMAILBOX_PAYLOAD_LEN_2U                (2U)
/** Payload length of size 3 bytes. */
#define XMAILBOX_PAYLOAD_LEN_3U                (3U)
/** Payload length of size 4 bytes. */
#define XMAILBOX_PAYLOAD_LEN_4U                (4U)
/** Payload length of size 5 bytes. */
#define XMAILBOX_PAYLOAD_LEN_5U                (5U)
/** Payload length of size 6 bytes. */
#define XMAILBOX_PAYLOAD_LEN_6U                (6U)
/** Payload length of size 7 bytes. */
#define XMAILBOX_PAYLOAD_LEN_7U                (7U)
/** @} */

/** @name Library version info
 * @{
 */
#define XMAILBOX_MAJOR_VERSION	1U /**< Library major version. */
#define XMAILBOX_MINOR_VERSION	9U /**< Library minor version. */
/** @} */

/**************************** Type Definitions *******************************/
/**
 * @brief Defines the callback type for the receive handler.
 *
 * @param[in]   CallBackRefPtr Callback reference registered with
 *              XMailbox_SetCallBack().
 */
typedef void (*XMailbox_RecvHandler) (void *CallBackRefPtr);

/**
 * @brief Defines the callback type for the error handler.
 *
 * @param[in]   CallBackRefPtr Callback reference registered with
 *              XMailbox_SetCallBack().
 * @param[in]   ErrorMask Bit mask indicating the cause of the error.
 */
typedef void (*XMailbox_ErrorHandler) (void *CallBackRefPtr, u32 ErrorMask);

/**
 * @brief Describes the XMailbox shared memory state.
 */
typedef enum {
	XMAILBOX_SHARED_MEM_UNINITIALIZED = 0, /**< Shared memory
						* uninitialized. */
	XMAILBOX_SHARED_MEM_INITIALIZED, /**< Shared memory initialized. */
} XMailbox_IpiSharedMemState;


/**
 * @brief Describes the shared memory segment used for IPI.
 */
typedef struct {
	u64 Address; /**< Address of the shared memory location. */
	u32 Size;    /**< Size of the shared memory location. */
	XMailbox_IpiSharedMemState SharedMemState;	/**< State of shared
							 * memory. */
} XMailbox_IpiSharedMem;

/**
 * @brief Stores the XilMailbox instance data.
 *
 * @attention The XMbox_IPI_Send, XMbox_IPI_SendData, and XMbox_IPI_Recv
 *            function pointers are deprecated in 2026.2 and become obsolete
 *            in the 2027.1 release.
 */
typedef struct XMboxTag {
	u32 (*XMbox_IPI_Send)(struct XMboxTag *InstancePtr, u8 Is_Blocking);
					    /**< Triggers an IPI to a
					     * destination CPU. */
	u32 (*XMbox_IPI_SendData)(struct XMboxTag *InstancePtr, void *BufferPtr,
				  u32 MsgLen, u8 BufferType, u8 Is_Blocking);
					    /**< Sends an IPI message to a
					     * destination CPU. */
	u32 (*XMbox_IPI_Recv)(struct XMboxTag *InstancePtr, void *BufferPtr,
			      u32 MsgLen, u8 BufferType);
					    /**< Reads an IPI message. */
	u32 (*XMbox_IPI_SendById)(struct XMboxTag *InstancePtr, u32 RemoteId, u8 Is_Blocking);
					    /**< Triggers an IPI to a
					     * destination CPU. */
	u32 (*XMbox_IPI_SendDataById)(struct XMboxTag *InstancePtr, void *BufferPtr,
				      u32 MsgLen, u8 BufferType, u32 RemoteId, u8 Is_Blocking);
					    /**< Sends an IPI message to a
					     * destination CPU. */
	u32 (*XMbox_IPI_RecvById)(struct XMboxTag *InstancePtr, void *BufferPtr,
				  u32 MsgLen, u32 SourceId, u8 BufferType);
					    /**< Reads an IPI message. */
	XMailbox_RecvHandler RecvHandler;   /**< Receive handler. */
	XMailbox_ErrorHandler ErrorHandler; /**< Callback for rx IPI event. */
	void *ErrorRefPtr;		    /**< Reference passed to the error
					     * interrupt callback. */
	void *RecvRefPtr;		    /**< Reference passed to the receive
					     * interrupt callback. */
	XMailbox_Agent Agent;		    /**< Agent that stores the IPI
					     * channel information. */
	XMailbox_IpiSharedMem SharedMem;    /**< Shared memory segment. */
} XMailbox;

/**
 * @brief Describes the XMailbox handler types.
 */
typedef enum {
	XMAILBOX_RECV_HANDLER,     /**< Receive handler. */
	XMAILBOX_ERROR_HANDLER,    /**< Error handler. */
} XMailbox_Handler;

/****************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* @brief Returns the version number of the XilMailbox library.
*
* @return 32-bit version number.
*
******************************************************************************/
static __attribute__((always_inline)) INLINE
u32 XMailbox_GetLibVersion(void)
{
	return (XIL_BUILD_VERSION(XMAILBOX_MAJOR_VERSION, XMAILBOX_MINOR_VERSION));
}

/************************** Function Prototypes ******************************/
/** @name Functions for xilmailbox.c
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

/** @} */
#endif /* XILMAILBOX_H */
