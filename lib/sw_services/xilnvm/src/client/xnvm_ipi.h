/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_ipi.h
* @addtogroup xnvm_ipi_apis XilNvm IPI APIs
* @{
*
* @cond xnvm_internal
* This file contains IPI generic APIs for xilnvm library
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  07/05/21 Initial release
* 1.1   kpt  01/13/21 Added macro XNVM_SHARED_MEM_SIZE
*
* </pre>
* @note
*
* @endcond
******************************************************************************/

#ifndef XNVM_IPI_H
#define XNVM_IPI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xipipsu.h"
#include "xparameters.h"

/************************** Constant Definitions ****************************/

/**@cond xnvm_internal
 * @{
 */
#define XILNVM_MODULE_ID			(11U)

/* 1 for API ID + 5 for API arguments + 1 for reserved + 1 for CRC */
#define PAYLOAD_ARG_CNT			(8U)
/* 1 for status + 3 for values + 3 for reserved + 1 for CRC */
#define RESPONSE_ARG_CNT		(8U)
#define XNVM_IPI_TIMEOUT		(0xFFFFFFFFU)
					/**< IPI timeout */
#define XNVM_TARGET_IPI_INT_MASK	(0x00000002U)
					/**< Target PMC IPI interrupt mask */
#define XNVM_IPI_UNUSED_PARAM		(0U)
#define XNVM_MODULE_ID_SHIFT		(8U)
#define XNVM_PAYLOAD_LEN_SHIFT		(16U)
#define XILNVM_MODULE_ID_MASK		(XILNVM_MODULE_ID << XNVM_MODULE_ID_SHIFT)

/* Max size of shared memory used to store the CDO command */
#define XNVM_SHARED_MEM_SIZE		(256U)

/**************************** Type Definitions *******************************/
typedef enum {
	XNVM_SHARED_MEM_UNINITIALIZED = 0, /**< Shared memory uninitialized */
	XNVM_SHARED_MEM_INITIALIZED, /**< Shared memory initialized */
}XNvm_IpiSharedMemState;

typedef struct {
	u64 Address;
	u32 Size;
	XNvm_IpiSharedMemState SharedMemState;
}XNvm_IpiSharedMem;

/***************** Macros (Inline Functions) Definitions *********************/

static inline u32 Header(u32 Len, u32 ApiId)
{
	return ((Len << XNVM_PAYLOAD_LEN_SHIFT) |
		XILNVM_MODULE_ID_MASK | (ApiId));
}

/**
 * @}
 * @endcond
 */
/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XNvm_ProcessIpi(u32 Arg0, u32 Arg1, u32 Arg2, u32 Arg3, u32 Arg4,
	u32 Arg5);
int XNvm_ProcessIpiWithPayload0(u32 ApiId);
int XNvm_ProcessIpiWithPayload1(u32 ApiId, u32 Arg1);
int XNvm_ProcessIpiWithPayload2(u32 ApiId, u32 Arg1, u32 Arg2);
int XNvm_ProcessIpiWithPayload3(u32 ApiId, u32 Arg1, u32 Arg2, u32 Arg3);
int XNvm_ProcessIpiWithPayload4(u32 ApiId, u32 Arg1, u32 Arg2, u32 Arg3,
	u32 Arg4);
int XNvm_ProcessIpiWithPayload5(u32 ApiId, u32 Arg1, u32 Arg2, u32 Arg3,
	u32 Arg4, u32 Arg5);
int XNvm_IpiSend(u32 *Payload);
int XNvm_IpiReadBuff32(void);
int XNvm_SetIpi(XIpiPsu* const IpiInst);
int XNvm_InitializeIpi(XIpiPsu* const IpiInstPtr);
void XNvm_SetSharedMem(u64 Address, u32 Size);
u32 XNvm_GetSharedMem(u64 **Address);
int XNvm_ReleaseSharedMem(void);

#ifdef __cplusplus
}
#endif

#endif  /* XNVM_IPI_H */
