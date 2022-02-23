/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_ipi.h
*
* This file contains IPI generic APIs for xilsecure library
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/23/21 Initial release
* 4.5   kal  03/23/20 Updated file version to sync with library version
*       har  04/14/21 Renamed XSecure_ConfigIpi as XSecure_SetIpi
*                     Added XSecure_InitializeIpi
*       am   05/22/21 Resolved MISRA C violation
* 4.6   har  07/14/21 Fixed doxygen warnings
* 4.7   kpt  01/13/22 Added macro XSECURE_SHARED_MEM_SIZE
*
* </pre>
* @note
*
******************************************************************************/

#ifndef XSECURE_IPI_H
#define XSECURE_IPI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xipipsu.h"
#include "xparameters.h"

/************************** Constant Definitions ****************************/
#define XILSECURE_MODULE_ID			(0x05UL)
				/**< Module ID for xilsecure */

#define HEADER(len, ApiId) ((len << 16U) | (XILSECURE_MODULE_ID << 8U) | ((u32)ApiId))
				/**< Header for XilSecure Commands */

#define PAYLOAD_ARG_CNT			(8U)
	/**< 1 for API ID + 5 for API arguments + 1 for reserved + 1 for CRC */

#define RESPONSE_ARG_CNT		(8U)
	/**< 1 for status + 3 for values + 3 for reserved + 1 for CRC */

#define XSECURE_IPI_TIMEOUT		(~0U)
					/**< IPI timeout */

#define XSECURE_TARGET_IPI_INT_MASK	(0x00000002U)
					/**< Target PMC IPI interrupt mask */

#define XSECURE_IPI_UNUSED_PARAM	(0U)
					/**< Unused param */

/* Maximum size of shared memory used to store the CDO command */
#define XSECURE_SHARED_MEM_SIZE		(128U)
					/**< Shared memory size */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

typedef enum {
	XSECURE_SHARED_MEM_UNINITIALIZED = 0, /**< Shared memory uninitialized */
	XSECURE_SHARED_MEM_INITIALIZED, /**< Shared memory initialized */
}XSecure_IpiSharedMemState;

typedef struct {
	u64 Address; /**< Address of the shared memory location */
	u32 Size; /**< Size od the shared memory location */
	XSecure_IpiSharedMemState SharedMemState; /**< State of shared memory */
}XSecure_IpiSharedMem;

/************************** Function Definitions *****************************/
int XSecure_ProcessIpi(u32 Arg0, u32 Arg1, u32 Arg2, u32 Arg3, u32 Arg4,
	u32 Arg5);
int XSecure_ProcessIpiWithPayload0(u32 ApiId);
int XSecure_ProcessIpiWithPayload1(u32 ApiId, u32 Arg1);
int XSecure_ProcessIpiWithPayload2(u32 ApiId, u32 Arg1, u32 Arg2);
int XSecure_ProcessIpiWithPayload3(u32 ApiId, u32 Arg1, u32 Arg2, u32 Arg3);
int XSecure_ProcessIpiWithPayload4(u32 ApiId, u32 Arg1, u32 Arg2, u32 Arg3,
	u32 Arg4);
int XSecure_ProcessIpiWithPayload5(u32 ApiId, u32 Arg1, u32 Arg2, u32 Arg3,
	u32 Arg4, u32 Arg5);
int XSecure_IpiSend(u32 *Payload);
int XSecure_IpiReadBuff32(void);
int XSecure_SetIpi(XIpiPsu* const IpiInst);
int XSecure_InitializeIpi(XIpiPsu* const IpiInstPtr);
void XSecure_SetSharedMem(u64 Address, u32 Size);
u32 XSecure_GetSharedMem(u64 **Address);
int XSecure_ReleaseSharedMem(void);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_IPI_H */
