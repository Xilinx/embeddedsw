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
 *       rmv  07/31/25 Added XASUFW_PLM_CMD_ID_ASU_CDI_TX_ID command ID
 *       rmv  07/16/25 Added function prototype of "XAsufw_ReadIpiMsgFromPlm" function
 *	 rmv  07/16/25 Added XASUFW_PLM_CMD_ID_ASU_GET_SUBSYS_ID macro
 *	 rmv  09/12/25 Moved IPI addresses macros to hw related file
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
#define XASUFW_PLM_CMD_ID_ASU_CDI_TX_ID	(2U) /**< Command Id of ASU CDI transfer */
#define XASUFW_PLM_CMD_ID_SUBSYSTEM_HASH_TX_ID	(3U) /**< Command Id of subsystem hash transfer */

#define XOCP_ASU_CDI_TX_ID_CMD_LEN		(0U)	/**< ASU CDI transfer ID command length */
#define XOCP_ASU_CDI_TX_PAYLOAD_RESP_SIZE	(1U)	/**< ASU CDI ID payload response size */
#define XOCP_ASU_CDI_TX_PAYLOAD_SIZE		(3U)	/**< ASU CDI transfer payload size*/

#define XOCP_SUBSYS_HASH_TX_CMD_LEN		(0U)	/**< Get subsystem ID command length */
#define XOCP_SUBSYS_HASH_TX_PAYLOAD_RESP_SIZE	(1U)	/**< Get subsystem hash payload
							response size */
#define XOCP_SUBSYS_HASH_TX_PAYLOAD_SIZE	(3U)	/**< ASU hash transfer Payload size */

#define XASUFW_PLM_ASU_MODULE_ID	(14U) /**< PLM ASU module Id */

#define XASUFW_IPI_MAX_MSG_LEN		(XIPIPSU_MAX_MSG_LEN) /**< Maximum IPI buffer length */

/************************************ Function Prototypes ****************************************/
s32 XAsufw_IpiInit(void);
void XAsufw_IpiHandler(const void *Data);
s32 XAsufw_SendIpiToPlm(const u32 *MsgBufPtr, u32 MsgBufLen);
s32 XAsufw_ReadIpiRespFromPlm(u32 *RespBufPtr, u32 RespBufLen);
s32 XAsufw_ReadIpiMsgFromPlm(u32 *MsgBufPtr, u32 MsgBufLen);
void XAsufw_EnableIpiInterrupt(u16 IpiBitMask);

/************************************ Variable Definitions ***************************************/

#endif

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_IPI_H_ */
/** @} */
