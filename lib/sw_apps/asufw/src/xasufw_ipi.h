/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_ipi.h
 * @addtogroup Overview
 * @{
 *
 * This file contains declarations for xasufw_ipi.c file in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   01/02/24 Initial release
 *       ma   07/08/24 Added IPI_ASU_TRIG macro
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XASUFW_IPI_H
#define XASUFW_IPI_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xparameters.h"
#if defined(XPAR_XIPIPSU_0_BASEADDR)
#include "xipipsu.h"

/************************************ Constant Definitions ***************************************/
/** ASU IPI Base address */
#define IPI_ASU_BASEADDR		XPAR_XIPIPSU_0_BASEADDR
/** ASU IPI Interrupt Status Register */
#define IPI_ASU_TRIG			( ( IPI_ASU_BASEADDR ) + ((u32)0x00000000U) )
/** ASU IPI Interrupt Status Register */
#define IPI_ASU_ISR				( ( IPI_ASU_BASEADDR ) + ((u32)0x00000010U) )
/** ASU IPI Interrupt Mask Register */
#define IPI_ASU_IMR				( ( IPI_ASU_BASEADDR ) + ((u32)0x00000014U) )
/** ASU IPI Interrupt Enable Register */
#define IPI_ASU_IER				( ( IPI_ASU_BASEADDR ) + ((u32)0x00000018U) )
/** ASU IPI Interrupt Disable Register */
#define IPI_ASU_IDR				( ( IPI_ASU_BASEADDR ) + ((u32)0x0000001CU) )

#define XASUFW_MAX_IPI_CHANNELS	8U /**< Maximum IPI channels supported by ASUFW */

#define XASUFW_IPI_PMC_MASK		((u32)0x00000002U) /**< PMC IPI channel mask */
#define XASUFW_IPI_NOBUF_6_MASK	((u32)0x00008000U) /**< IPI6 NoBuf channel mask */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#ifndef SDT
#define XASUFW_IPI_DEVICE_ID    XPAR_XIPIPSU_0_DEVICE_ID
#else
#define XASUFW_IPI_DEVICE_ID    XPAR_XIPIPSU_0_BASEADDR
#endif

/************************************ Function Prototypes ****************************************/
s32 XAsufw_IpiInit(void);
void XAsufw_IpiHandler(void *Data);
s32 XAsufw_SendIpiToPlm(u32 *MsgBufPtr, u32 MsgBufLen);
s32 XAsufw_ReadIpiRespFromPlm(u32 *RespBufPtr, u32 RespBufLen);
void XAsufw_EnableIpiInterrupt(u16 IpiBitMask);

/************************************ Variable Definitions ***************************************/

#endif

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_IPI_H */
/** @} */
