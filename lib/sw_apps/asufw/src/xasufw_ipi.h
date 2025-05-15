/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_ipi.h
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
 *       am   02/24/25 Added macros related to PLM ASU IPI module
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_IPI_H_
#define XASUFW_IPI_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xparameters.h"
#if defined(XPAR_XIPIPSU_0_BASEADDR)
#include "xipipsu.h"

/************************************ Constant Definitions ***************************************/
#define IPI_ASU_BASEADDR	XPAR_XIPIPSU_0_BASEADDR /**< ASU IPI Base address */
#define IPI_ASU_TRIG		(IPI_ASU_BASEADDR + 0x00000000U) /**< ASU IPI Interrupt Status
									Register */
#define IPI_ASU_ISR		(IPI_ASU_BASEADDR + 0x00000010U) /**< ASU IPI Interrupt Status
									Register */
#define IPI_ASU_IMR		(IPI_ASU_BASEADDR + 0x00000014U) /**< ASU IPI Interrupt Mask
									Register */
#define IPI_ASU_IER		(IPI_ASU_BASEADDR + 0x00000018U) /**< ASU IPI Interrupt Enable
									Register */
#define IPI_ASU_IDR		(IPI_ASU_BASEADDR + 0x0000001CU) /**< ASU IPI Interrupt Disable
									Register */

#define XASUFW_IPI_PMC_MASK	(0x00000002U) /**< PMC IPI channel mask */
#define XASUFW_IPI_NOBUF_6_MASK	(0x00008000U) /**< IPI6 NoBuf channel mask */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#ifndef SDT
#define XASUFW_IPI_DEVICE_ID    XPAR_XIPIPSU_0_DEVICE_ID /**< ASUFW IPI device Id for
								versionless flow */
#else
#define XASUFW_IPI_DEVICE_ID    XPAR_XIPIPSU_0_BASEADDR /**< ASUFW IPI device Id for SDT flow */
#endif

#define XASUFW_PLM_IPI_HDR_LEN_SHIFT		(16U)	/**< Shift value of IPI message length */
#define XASUFW_PLM_IPI_HDR_MODULE_ID_SHIFT	(8U)	/**< Shift value of IPI Module ID */

#define XASUFW_PLM_IPI_HEADER(Length, ApiId, ModuleId)	\
		(((u32)(Length) << XASUFW_PLM_IPI_HDR_LEN_SHIFT) | \
		((u32)(ModuleId) << XASUFW_PLM_IPI_HDR_MODULE_ID_SHIFT) | ((u32)(ApiId)))
		/**< Header for PLM IPI commands */

#define XASUFW_PLM_ASU_KEY_TX_API_ID	(1U) /**< PLM ASU key transfer API Id */

#define XASUFW_PLM_ASU_MODULE_ID	(14U) /**< PLM ASU module Id */

/************************************ Function Prototypes ****************************************/
s32 XAsufw_IpiInit(void);
void XAsufw_IpiHandler(const void *Data);
s32 XAsufw_SendIpiToPlm(const u32 *MsgBufPtr, u32 MsgBufLen);
s32 XAsufw_ReadIpiRespFromPlm(u32 *RespBufPtr, u32 RespBufLen);
void XAsufw_EnableIpiInterrupt(u16 IpiBitMask);

/************************************ Variable Definitions ***************************************/

#endif

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_IPI_H_ */
/** @} */
