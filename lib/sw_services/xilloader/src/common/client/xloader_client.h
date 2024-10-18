/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xloader_client.h
 *
 * This file Contains the client function prototypes, defines and macros.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  dd   01/09/24 Initial release
 *       har  03/05/24 Fixed doxygen warnings
 *       pre  08/22/24 Added XLoader_CfiSelectiveReadback, XLoader_InputSlrIndex functions
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XLOADER_CLIENT_H
#define XLOADER_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

#include "xil_types.h"
#include "xloader_mailbox.h"
#include "xloader_defs.h"

/************************************ Constant Definitions ***************************************/
/**< shift constant to place slr id*/
#define XLOADER_SLR_INDEX_SHIFT (6U)

/**< SlrIndexs constants*/
#define XLOADER_SLR_INDEX_0 (0U) /**< SLR Index 0 */
#define XLOADER_SLR_INDEX_1 (1U) /**< SLR Index 1 */
#define XLOADER_SLR_INDEX_2 (2U) /**< SLR Index 2 */
#define XLOADER_SLR_INDEX_3 (3U) /**< SLR Index 3 */

/************************************** Type Definitions *****************************************/

typedef enum {
		XLOADER_PDI_JTAG = 0, /**< 0U PDI source JTAG */
		XLOADER_PDI_QSPI24, /**< 1U PDI source QSPI24 */
		XLOADER_PDI_QSPI32, /**< 2U PDI source QSPI32 */
		XLOADER_PDI_SD0, /**< 3U PDI source SD0 */
		XLOADER_PDI_EMMC0, /**< 4U PDI source EMMC0 */
		XLOADER_PDI_SD1, /**< 5U PDI source SD1 */
		XLOADER_PDI_EMMC1, /**< 6U PDI source EMMC1 */
		XLOADER_PDI_USB, /**< 7U PDI source USB */
		XLOADER_PDI_OSPI, /**< 8U PDI source OSPI */
		XLOADER_PDI_SBI, /**< 9U PDI source SBI */
		XLOADER_PDI_SMAP = 0xAU,/**< 0xAU PDI source SMAP */
		XLOADER_PDI_PCIE, /**< 0xBU PDI source PCIE */
		XLOADER_PDI_SD1_LS = 0XEU, /**< 0xEU PDI source SD1-LS */
		XLOADER_PDI_DDR = 0xFU, /**< 0xFU PDI source DDR */
		XLOADER_PDI_IS = 0x10U, /**< 0x10U PDI source Image Store */
		XLOADER_PDI_INVALID = 0xFFU, /**< 0xFFU PDI source invalid */
} XLoader_PdiSrc; /**< PDI sources */

typedef enum {
		XLOADER_FLASH_RAW = 0, /**< 0 */
		XLOADER_FLASH_FS, /**< 1 */
		XLOADER_FLASH_RAW_BP1, /**< 2 */
		XLOADER_FLASH_RAW_BP2, /**< 3 */
} XLoader_FlashType; /**< Flash types */

typedef struct {
        u64 DestAddr; /**< Destination address */
        u32 Row; /**< Row number */
        u32 Blocktype; /**< Block type */
        u32 FrameAddr; /**< Frame address */
        u32 FrameCnt; /**< Frame count */
} XLoader_CfiSelReadbackParams;

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XLOADER_PDI_SRC_DDR			(0xF)	/**< Source of PDI is DDR */
#define XLOADER_PDI_SRC_IS			(0x10)	/**< Source of PDI is Image Store */

/************************************ Function Prototypes ****************************************/

int XLoader_LoadPartialPdi(XLoader_ClientInstance *InstancePtr, XLoader_PdiSrc PdiSrc,
		u64 PdiAddr, u32 *PlmErrStatus);
int XLoader_LoadImage(XLoader_ClientInstance *InstancePtr, u32 NodeId, u32 FunctionId);
int XLoader_GetImageInfo(XLoader_ClientInstance *InstancePtr, u32 NodeId,
		XLoader_ImageInfo *ImageInfo);
int XLoader_GetImageInfoList(XLoader_ClientInstance *InstancePtr, u64 Buff_Addr,u32 Maxsize,
		u32 *NumEntries);
int XLoader_ExtractMetaheader(XLoader_ClientInstance *InstancePtr, u64 PdiSrcAddr,
		u64 DestBuffAddr, u32 DestBuffSize);
int XLoader_LoadReadBackPdi(XLoader_ClientInstance *InstancePtr, XLoader_PdiSrc PdiSrc, u64 PdiAddr,
		u64 ReadbackDdrDestAddr, u32 Maxsize, u32 *Sizecopied);
int XLoader_UpdateMultiboot(XLoader_ClientInstance *InstancePtr, XLoader_PdiSrc BootMode,
		XLoader_FlashType Type, u32 ImageLocation);
int XLoader_AddImageStorePdi(XLoader_ClientInstance *InstancePtr, u32 PdiId,
		const u64 PdiAddr, u32 PdiSize);
int XLoader_RemoveImageStorePdi(XLoader_ClientInstance *InstancePtr, u32 PdiId);
int XLoader_GetATFHandOffParams(XLoader_ClientInstance * InstancePtr, u64 BuffAddr, u32 Size,
		 u32 *BufferSize);
int XLoader_CfiSelectiveReadback(XLoader_ClientInstance *InstancePtr,
                                 XLoader_CfiSelReadbackParams *SelectiveReadbackPtr);
int XLoader_InputSlrIndex(XLoader_ClientInstance *InstancePtr, u32 SlrIndex);
int XLoader_GetOptionalData(XLoader_ClientInstance *InstancePtr,
		const XLoader_OptionalDataInfo* OptionalDataInfo, u64 DestAddr, u32 *DestSize);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_CLIENT_H */