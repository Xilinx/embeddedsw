/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_sharedmem.h
 *
 * This header file defines the structures that are used for shared memory
 * communication between the client and the server. The structures include
 * definitions for message formats, shared memory control blocks, and
 * any other relevant data structures needed for inter-process communication
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  06/04/24 Initial release
 *       ma   07/08/24 Add task based approach at queue level
 *       ma   07/23/24 Update XASU_RESPONSE_STATUS_INDEX with 0
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/
#ifndef XASU_SHAREDMEM_H
#define XASU_SHAREDMEM_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/
#define XASU_MAX_BUFFERS					(8U) /**< Maximum request and response buffers */
#define XASU_CHANNEL_RESERVED_MEM			(1460U) /**< Reserved memory in channel */

#define XASU_COMMAND_IS_PRESENT				(0x1U) /**< Command is written by client */
#define XASU_COMMAND_IN_PROGRESS			(0x2U) /**< Command is in progress by ASUFW */
#define XASU_COMMAND_WAITING_FOR_RESOURCE	(0x3U) /**< Command is waiting for required resources */
#define XASU_COMMAND_EXECUTION_COMPLETE		(0x4U) /**< Command execution is complete by ASUFW */
/* Response buffers status */
#define XASU_RESPONSE_IS_PRESENT			(0x1U) /**< Response is written by ASUFW */

#define XASU_RESPONSE_STATUS_INDEX			(0U) /**< Response status index in response buffer */

#define XASU_COMMAND_ID_MASK				(0x0000003FU) /**< Mask for command ID in header */
#define XASU_UNIQUE_REQ_ID_MASK				(0x00000FC0U) /**< Mask for command unique reqest ID */
#define XASU_UNIQUE_REQ_ID_SHIFT			(6U) /**< Shift value for unique request ID */
#define XASU_MODULE_ID_MASK					(0x0003F000U) /**< Mask for module ID */
#define XASU_MODULE_ID_SHIFT				(12U) /**< Shift value for module ID */
#define XASU_COMMAND_LENGTH_MASK			(0x00FC0000U) /**< Mask for command length in header */
#define XASU_COMMAND_LENGTH_SHIFT			(18U) /**< Shift for command length */

#define XASU_COMMAND_ARGS					(18U) /**< Command/response arguments */

/************************************** Type Definitions *****************************************/
/** This structure is the request buffer */
typedef struct {
	u32 Header; /**< Command header */
	u32 Arg[XASU_COMMAND_ARGS]; /**< Command arguments */
	u32 CheckSum;
} XAsu_ReqBuf;

/** This structure is the response buffer */
typedef struct {
	u32 Header; /**< Command header */
	u32 Arg[XASU_COMMAND_ARGS]; /**< Response arguments */
	u32 CheckSum;
} XAsu_RespBuf;

/** This structure is the channel's queue buffer */
typedef struct {
	u8 ReqBufStatus; /**< Request buffer status */
	u8 RespBufStatus; /**< Response buffer status */
	u16 Reserved; /**< Reserved */
	XAsu_ReqBuf ReqBuf; /**< Request buffer */
	XAsu_RespBuf RespBuf; /**< Response buffer */
} XAsu_ChannelQueueBuf;

/** This structure is the channel's queue which is of 8 buffers */
typedef struct {
	u32 IsCmdPresent; /**< Cmd present status of the queue */
	XAsu_ChannelQueueBuf ChannelQueueBufs[XASU_MAX_BUFFERS]; /**< Channel's queue of 8 buffers */
} XAsu_ChannelQueue;

/** This struture is the channel's memory */
typedef struct {
	u32 Version; /**< Structure version */
	u8 Reserved[XASU_CHANNEL_RESERVED_MEM]; /**< Reserved memory */
	XAsu_ChannelQueue P0ChannelQueue; /**< P0 channel queue */
	XAsu_ChannelQueue P1ChannelQueue; /**< P1 channel queue */
} XAsu_ChannelMemory;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASU_SHAREDMEM_H */
/** @} */
