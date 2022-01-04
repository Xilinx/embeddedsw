/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf_ipi.h
* @addtogroup xpuf_ipi_apis XilPuf IPI APIs
* @{
*
* @cond xpuf_internal
* This file contains IPI generic APIs for xilpuf library
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt  01/04/22 Initial release
*
* </pre>
* @note
*
* @endcond
******************************************************************************/

#ifndef XPUF_IPI_H
#define XPUF_IPI_H

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
#define XILPUF_MODULE_ID			(12U)

/* 1 for API ID + 5 for API arguments + 1 for reserved + 1 for CRC */
#define PAYLOAD_ARG_CNT			(8U)
/* 1 for status + 3 for values + 3 for reserved + 1 for CRC */
#define RESPONSE_ARG_CNT		(8U)
#define XPUF_IPI_TIMEOUT		(0xFFFFFFFFU)
#define TARGET_IPI_INT_MASK		XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK
#define XPUF_MODULE_ID_SHIFT		(8U)
#define XPUF_PAYLOAD_LEN_SHIFT		(16U)
#define XILPUF_MODULE_ID_MASK		(XILPUF_MODULE_ID << XPUF_MODULE_ID_SHIFT)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

static inline u32 PufHeader(u32 Len, u32 ApiId)
{
	return ((Len << XPUF_PAYLOAD_LEN_SHIFT) |
		XILPUF_MODULE_ID_MASK | (ApiId));
}

/**
 * @}
 * @endcond
 */
/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XPuf_ProcessIpi(u32 Arg0, u32 Arg1, u32 Arg2, u32 Arg3, u32 Arg4,
	u32 Arg5);
int XPuf_ProcessIpiWithPayload0(u32 ApiId);
int XPuf_ProcessIpiWithPayload1(u32 ApiId, u32 Arg1);
int XPuf_ProcessIpiWithPayload2(u32 ApiId, u32 Arg1, u32 Arg2);
int XPuf_ProcessIpiWithPayload3(u32 ApiId, u32 Arg1, u32 Arg2, u32 Arg3);
int XPuf_ProcessIpiWithPayload4(u32 ApiId, u32 Arg1, u32 Arg2, u32 Arg3,
	u32 Arg4);
int XPuf_ProcessIpiWithPayload5(u32 ApiId, u32 Arg1, u32 Arg2, u32 Arg3,
	u32 Arg4, u32 Arg5);
int XPuf_IpiSend(u32 *Payload);
int XPuf_IpiReadBuff32(void);
int XPuf_SetIpi(XIpiPsu* const IpiInst);
int XPuf_InitializeIpi(XIpiPsu* const IpiInstPtr);

#ifdef __cplusplus
}
#endif

#endif  /* XPUF_IPI_H */
