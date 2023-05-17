/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* (c) Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file xsem_ipi_interface.h
*
* @cond xsem_internal
* This file has definitions of commonly used macros and data types needed for
* XilSEM IPI interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date         Changes
* ----  ---  ----------   --------------------------------------------------
* 0.1   gm   08/28/2020   Initial Creation
* 0.2   gm   09/03/2020   Updation of XSem_IpiPlmRespMsg to support
*                         complete response message.
* 0.3   rb   01/25/2021   Added macro for IPI payload 4.
* 0.4   hv   03/11/2021   Doxygen changes
* 0.5   hb   03/15/2021   MISRA fixes, updated IPI macro and formatted code
* 0.6   rb   04/07/2021   Doxygen changes
* 0.7	hv   08/18/2021   Fix Doxygen warnings
* 0.8   hb   01/04/2022   Added macro for IPI payload 2
* 0.9	hv   01/11/2022   Added interface for reading Frame ECC
* 1.0	hv   07/24/2022   Removed unused macro XSem_DataMaskShift
* 1.1   ga   05/16/2023   Updated copyright information and
*                         fixed IPI instance for versal net
* </pre>
* @note
* @endcond
*
******************************************************************************/

#ifndef XSEM_IPI_INTERFACE_H
#define XSEM_IPI_INTERFACE_H
/***************************** Include Files *********************************/
/**
 * @cond xsem_internal
 * @{
 */
#include "xparameters.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xipipsu.h"
#include "xil_printf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Conditional debugging prints */
#ifdef DEBUG_MODE
#define XSem_Dbg(MSG, ...)	xil_printf(MSG, ##__VA_ARGS__);
#else
#define XSem_Dbg(MSG, ...)	{}
#endif

#ifdef VERSAL_NET
#define TARGET_IPI_INT_MASK	(XPAR_XIPIPS_TARGET_PSX_PMC_0_CH0_MASK)
#else
#define TARGET_IPI_INT_MASK	(XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK)
#endif

/* 1 for API ID + 4 for API arguments + 3 for Reserved */
#define PAYLOAD_ARG_CNT		(8U)

/* 1 for status + 2 for values + 5 for Reserved */
#define RESPONSE_ARG_CNT	(8U)

#define SEM_IPI_TIMEOUT		(~0U)

#define SEM_MODULE_ID		(0x03UL)

#define HEADER(Len, ApiId)	((Len << 16U) | (SEM_MODULE_ID << 8U) | ((u32)ApiId))

/* Payload Formation */
#define PACK_PAYLOAD(Payload, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7) \
	Payload[0] = (u32)Arg0;	\
	Payload[1] = (u32)Arg1;	\
	Payload[2] = (u32)Arg2;	\
	Payload[3] = (u32)Arg3;	\
	Payload[4] = (u32)Arg4;	\
	Payload[5] = (u32)Arg5;	\
	Payload[6] = (u32)Arg6;	\
	Payload[7] = (u32)Arg7;


/* For Payload with just API Id */
#define PACK_PAYLOAD1(Payload, ApiId) \
	PACK_PAYLOAD(Payload, HEADER(1UL, ApiId), 0, 0, 0, 0, 0, 0, 0)

/* For Payload with API Id + 1 Args */
#define PACK_PAYLOAD2(Payload, ApiId, Arg1) \
	PACK_PAYLOAD(Payload, HEADER(2UL, ApiId), Arg1, 0, 0, 0, 0, 0, 0)

/* For Payload with API Id + 2 Args */
#define PACK_PAYLOAD3(Payload, ApiId, Arg1, Arg2) \
	PACK_PAYLOAD(Payload, HEADER(3UL, ApiId), Arg1, Arg2, 0, 0, 0, 0, 0)

/* For Payload with API Id + 3 Args */
#define PACK_PAYLOAD4(Payload, ApiId, Arg1, Arg2, Arg3) \
	PACK_PAYLOAD(Payload, HEADER(4UL, ApiId), Arg1, Arg2, Arg3, 0, 0, 0, 0)

/* For Payload with API Id + 4 Args */
#define PACK_PAYLOAD5(Payload, ApiId, Arg1, Arg2, Arg3, Arg4) \
	PACK_PAYLOAD(Payload, HEADER(5UL, ApiId), Arg1, \
	Arg2, Arg3, Arg4, 0, 0, 0)

/* For Payload with API Id + 5 Args */
#define PACK_PAYLOAD6(Payload, ApiId, Arg1, Arg2, Arg3, Arg4, Arg5) \
		PACK_PAYLOAD(Payload, HEADER(6UL, ApiId), Arg1, \
		Arg2, Arg3, Arg4, Arg5, 0, 0)

XStatus XSem_IpiPlmRespMsg(XIpiPsu *IpiInst,u32 RespMsg[RESPONSE_ARG_CNT]);
XStatus XSem_IpiSendReqPlm(XIpiPsu *IpiInst, u32 Payload[PAYLOAD_ARG_CNT]);

#ifdef __cplusplus
}
#endif

/**
 * @}
 * @endcond
 */
#endif /* XSEM_IPI_INTERFACE_H */
