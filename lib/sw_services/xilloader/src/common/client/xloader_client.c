/**************************************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xloader_client.c
 *
 * This file contains the implementation of the client interface functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  dd   01/09/24 Initial release
 *       har  03/05/24 Fixed doxygen warnings
 *       pre  08/22/24 Added XLoader_CfiSelectiveReadback, XLoader_InputSlrIndex functions
 *       pre  10/26/24 Removed XLoader_LoadReadBackPdi API
 *       pre  09/30/25 Updated comments for rtf docs
 * 2.4   tbk  02/10/26 Added SMC call support
 *       gnr  03/18/26 Updated the Payload assignments with XLOADER_PACK_PAYLOAD macros
 *       vm   03/30/26 Added support for OSPI as pdi source to get the optional data
 *
 * </pre>
 *
 *************************************************************************************************/

/**
 * @addtogroup xloader_client_apis XilLoader Client APIs
 * @{
 */

/*************************************** Include Files *******************************************/

#include "xloader_client.h"
#include "xloader_generic.h"

/************************************ Constant Definitions ***************************************/

#define XLOADER_ADDR_HIGH_SHIFT 		(32U) /**< Shift value to get higher 32 bit address */
#define XLOADER_MSB_MASK 				(0x80000000) /**< Mask for MSB bit */

#define XLOADER_SELREADBACK_ROW_START_POS     (23U) /**< Row field start bit position */
#define XLOADER_SELREADBACK_BLKTYPE_START_POS (20U) /**< Block type field start bit position */
#define XLOADER_SELREADBACK_ROW_MASK          (0x7800000U) /**< Mask for row */
#define XLOADER_SLEREADBACK_BLKTYPE_MASK      (0X700000U) /**< Mask for block type */
#define XLOADER_SLEREADBACK_FRAMEADDR_MASK    (0xFFFFFU) /**< Mask for frame address */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to provides PDI execution.
 *
 * @param	InstancePtr 	Pointer to the client instance.
 * @param	PdiSrc			Boot Mode values, DDR, PCIe.
 * @param	PdiAddr			64bit PDI address located in the Source.
 * @param	PlmErrStatus	Used to store the plm error status.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 **************************************************************************************************/
int XLoader_LoadPartialPdi(XLoader_ClientInstance *InstancePtr, XLoader_PdiSrc PdiSrc,
		u64 PdiAddr, u32 *PlmErrStatus)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	/** Performs input parameters validation */
	if (PlmErrStatus == NULL) {
		goto END;
	}

	/** Fill Payload */
	XLOADER_PACK_PAYLOAD3(Payload, (u32)XLOADER_CMD_ID_LOAD_SUBSYSTEM_PDI, PdiSrc,
			(u32)(PdiAddr >> XLOADER_ADDR_HIGH_SHIFT), (u32)(PdiAddr));

	/** Send request using generic API */
	Status = XLoader_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, Response, (u32)RESPONSE_ARG_CNT);
	*PlmErrStatus = Response[0U];

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief This function sends IPI request to Load image.
 *
 * @param	InstancePtr Pointer to the client instance.
 * @param	NodeId		Node id of ddr copied image.
 * @param	FunctionId 	To verify with the FuncID of the image copied to DDR.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 **************************************************************************************************/
int XLoader_LoadImage(XLoader_ClientInstance *InstancePtr, u32 NodeId, u32 FunctionId)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};

	/** Fill Payload */
	XLOADER_PACK_PAYLOAD2(Payload, (u32)XLOADER_CMD_ID_LOAD_DDRCPY_IMG, NodeId, FunctionId);

	/** Send request using generic API */
	Status = XLoader_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to get image information.
 *
 * @param	InstancePtr Pointer to the client instance.
 * @param	NodeId		Node id of ddr copied image.
 * @param	ImageInfo	Pointer to the structure XLoader_ImageInfo which is used to store the
 * 						image information.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 **************************************************************************************************/
int XLoader_GetImageInfo(XLoader_ClientInstance *InstancePtr, u32 NodeId,
		XLoader_ImageInfo *ImageInfo)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	/** Performs input parameters validation */
	if (ImageInfo == NULL) {
		goto END;
	}

	/** Fill Payload */
	XLOADER_PACK_PAYLOAD1(Payload, (u32)XLOADER_CMD_ID_GET_IMAGE_INFO, NodeId);

	/** Send request using generic API */
	Status = XLoader_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, Response, (u32)RESPONSE_ARG_CNT);

	ImageInfo->ImgID = NodeId;
	ImageInfo->UID = Response[0U];
	ImageInfo->PUID = Response[1U];
	ImageInfo->FuncID = Response[2U];

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to gets ImageInfo Table.
 *
 * @param	InstancePtr Pointer to the client instance.
 * @param	Buff_Addr	Buffer address to copy the ImageInfo.
 * @param	Maxsize		Maximum size requires in bytes.
 * @param	NumEntries	To get the number of entries returned.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 **************************************************************************************************/
int XLoader_GetImageInfoList(XLoader_ClientInstance *InstancePtr, u64 Buff_Addr,u32 Maxsize,
		u32 *NumEntries)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};
	u32 Continue = 0;

	/** Performs input parameters validation */
	if (NumEntries == NULL) {
		goto END;
	}

	/**
	 * - Typically be Continue[31 bit] set to 0,
	 * In case the first call indicated that more entries are available then this can be set to 1
	 * present we are not supporting more entries , So returning XST_FAILURE if continue parameter
	 * is 1.
	 */
	Continue = ((Maxsize & XLOADER_MSB_MASK) >> 31);
	if (Continue != 0) {
		goto END;
	}

	/** Fill Payload */
	XLOADER_PACK_PAYLOAD3(Payload, (u32)XLOADER_CMD_ID_GET_IMAGE_INFO_LIST,
			(u32)(Buff_Addr >> XLOADER_ADDR_HIGH_SHIFT), (u32)(Buff_Addr), Maxsize);

	/** Send request using generic API */
	Status = XLoader_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, Response, (u32)RESPONSE_ARG_CNT);
	*NumEntries = Response[0U];

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to Extract meta header.
 *
 * @param	InstancePtr 	Pointer to the client instance.
 * @param	PdiSrcAddr		Address of the pdi.
 * @param	DestBuffAddr	Address to export the pdi.
 * @param	DestBuffSize	size of the destination buffer in bytes.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 **************************************************************************************************/
int XLoader_ExtractMetaheader(XLoader_ClientInstance *InstancePtr, u64 PdiSrcAddr,
		u64 DestBuffAddr, u32 DestBuffSize)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};

	/** Fill Payload */
	XLOADER_PACK_PAYLOAD5(Payload, (u32)XLOADER_CMD_ID_EXTRACT_METAHEADER,
			(u32)(PdiSrcAddr >> XLOADER_ADDR_HIGH_SHIFT), (u32)(PdiSrcAddr),
			(u32)(DestBuffAddr >> XLOADER_ADDR_HIGH_SHIFT),
			(u32)(DestBuffAddr), DestBuffSize);

	/** Send request using generic API */
	Status = XLoader_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to update multiboot.
 *
 *
 * @param	InstancePtr 	Pointer to the client instance.
 * @param	BootMode		Pdi boot source
 * @param	Type			Flash type
 * @param	ImageLocation	Location of Image in the boot device.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 **************************************************************************************************/
int XLoader_UpdateMultiboot(XLoader_ClientInstance *InstancePtr, XLoader_PdiSrc BootMode,
		XLoader_FlashType Type, u32 ImageLocation)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 BootModeType = Type | BootMode << 8;

	/** Fill Payload */
	XLOADER_PACK_PAYLOAD2(Payload, (u32)XLOADER_CMD_ID_UPDATE_MULTIBOOT, BootModeType, ImageLocation);

	/** Send request using generic API */
	Status = XLoader_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to add Image store pdi.
 *
 * @param	InstancePtr Pointer to the client instance.
 * @param	PdiId		Id of the pdi.
 * @param	PdiAddr		Address to store Image store pdi's maintained by PLM.
 * @param	PdiSize		Size of pdi in words.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 **************************************************************************************************/
int XLoader_AddImageStorePdi(XLoader_ClientInstance *InstancePtr, u32 PdiId,
		const u64 PdiAddr, u32 PdiSize)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};

	/** Fill Payload */
	XLOADER_PACK_PAYLOAD4(Payload, (u32)XLOADER_CMD_ID_ADD_IMAGESTORE_PDI, PdiId,
			(u32)(PdiAddr >> XLOADER_ADDR_HIGH_SHIFT), (u32)(PdiAddr), PdiSize);

	/** Send request using generic API */
	Status = XLoader_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to remove Image store pdi.
 *
 * @param	InstancePtr Pointer to the client instance.
 * @param	PdiId		Id of the PDI to be removed from Image Store.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 **************************************************************************************************/
int XLoader_RemoveImageStorePdi(XLoader_ClientInstance *InstancePtr, u32 PdiId)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};

	/** Fill Payload */
	XLOADER_PACK_PAYLOAD1(Payload, (u32)XLOADER_CMD_ID_REMOVE_IMAGESTORE_PDI, PdiId);

	/** Send request using generic API */
	Status = XLoader_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to get ATF Handoff parameters.
 *
 * @param	InstancePtr Pointer to the client instance (Input).
 * @param	BuffAddr	Address to store the handoff parameter information (Input).
 * @param	Size		Maximum number of bytes that can be stored in the buffer (Input).
 * @param	BufferSize	To get the size of the buffer in bytes (Output).
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 **************************************************************************************************/
int XLoader_GetATFHandOffParams(XLoader_ClientInstance *InstancePtr, u64 BuffAddr, u32 Size,
		 u32 *BufferSize)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	/** Performs input parameters validation */
	if (BufferSize == NULL) {
		goto END;
	}

	/** Fill Payload */
	XLOADER_PACK_PAYLOAD3(Payload, (u32)XLOADER_CMD_ID_GET_ATF_HANDOFF_PARAMS,
			(u32)(BuffAddr >> XLOADER_ADDR_HIGH_SHIFT),
			(u32)(BuffAddr), Size);

	/** Send request using generic API */
	Status = XLoader_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, Response, (u32)RESPONSE_ARG_CNT);
	*BufferSize = Response[0U];

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to read selective frames from configuration memory
 *
 * @param	InstancePtr             Pointer to the client instance
 * @param	SelectiveReadbackPtr    Pointer to structure which contains parameters of CFI selective
 *                                  readback command
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 *************************************************************************************************/
int XLoader_CfiSelectiveReadback(XLoader_ClientInstance *InstancePtr,
                                 XLoader_CfiSelReadbackParams *SelectiveReadbackPtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 ApiId = (u32)XLOADER_CFI_SEL_READBACK_ID;

	/** Performs input parameters validation */
	if ((SelectiveReadbackPtr == NULL) || (InstancePtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** Include SLR index in API ID */
	ApiId |= (InstancePtr->SlrIndex << XLOADER_SLR_INDEX_SHIFT);

	/** Fill Payload */
	XLOADER_PACK_PAYLOAD4(Payload, ApiId,
			((SelectiveReadbackPtr->Row << XLOADER_SELREADBACK_ROW_START_POS) & XLOADER_SELREADBACK_ROW_MASK)|
			((SelectiveReadbackPtr->Blocktype << XLOADER_SELREADBACK_BLKTYPE_START_POS) & XLOADER_SLEREADBACK_BLKTYPE_MASK) |
			((SelectiveReadbackPtr->FrameAddr) & XLOADER_SLEREADBACK_FRAMEADDR_MASK),
			SelectiveReadbackPtr->FrameCnt,
			(u32)(SelectiveReadbackPtr->DestAddr >> XLOADER_ADDR_HIGH_SHIFT), (u32)SelectiveReadbackPtr->DestAddr);

	/** Send request using generic API */
	Status = XLoader_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Adds the SLR Index.
 *
 * @param  InstancePtr is a pointer to instance XLoader_ClientInstance
 * @param  SlrIndex - SLR index number
 *
 * @return	- XST_SUCCESS - On valid input SlrIndex.
 *		    - XST_FAILURE - On non valid input SlrIndex
 *
 *************************************************************************************************/
int XLoader_InputSlrIndex(XLoader_ClientInstance *InstancePtr, u32 SlrIndex)
{
	int Status = XST_FAILURE;

	if (SlrIndex <= XLOADER_SLR_INDEX_3) {
		InstancePtr->SlrIndex = SlrIndex;
	    Status = XST_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to get optional data from the PDI available in DDR or
 * 		Image Store.
 *
 * @param	InstancePtr 		Pointer to the client instance.
 * @param	OptionalDataInfo	Pointer to XLoader_OptionalDataInfo structure
 * @param	DestAddr		Address of the output buffer wheren optional data shall be copied
 * @param	DestSize		Size of destination buffer in bytes
 *
 * @return
 *		 - XST_SUCCESS  on success.
 *		 - XST_FAILURE  on failure.
 *
**************************************************************************************************/
int XLoader_GetOptionalData(XLoader_ClientInstance *InstancePtr,
	const XLoader_OptionalDataInfo* OptionalDataInfo, u64 DestAddr, u32 *DestSize)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	/** Performs input parameters validation */
	if ((OptionalDataInfo == NULL) || (DestSize == NULL)) {
		goto END;
	}

	if ((OptionalDataInfo->PdiSrc == XLOADER_PDI_SRC_DDR) || (OptionalDataInfo->PdiSrc == XLOADER_PDI_SRC_OSPI)) {
		XLOADER_PACK_PAYLOAD6(Payload, (u32)XLOADER_CMD_ID_EXTRACT_METAHEADER, (OptionalDataInfo->PdiAddrHigh),
				(OptionalDataInfo->PdiAddrLow),
				(u32)(DestAddr >> XLOADER_ADDR_HIGH_SHIFT), (u32)(DestAddr), *DestSize,
				(OptionalDataInfo->DataId << XLOADER_DATA_ID_SHIFT) | (XLOADER_GET_OPT_DATA_FLAG |
				OptionalDataInfo->PdiSrc));
	}
	else if (OptionalDataInfo->PdiSrc == XLOADER_PDI_SRC_IS){
		XLOADER_PACK_PAYLOAD6(Payload, (u32)XLOADER_CMD_ID_EXTRACT_METAHEADER, OptionalDataInfo->PdiId, 0x0U,
				(u32)(DestAddr >> XLOADER_ADDR_HIGH_SHIFT), (u32)(DestAddr), *DestSize,
				(OptionalDataInfo->DataId << XLOADER_DATA_ID_SHIFT) | (XLOADER_GET_OPT_DATA_FLAG |
				OptionalDataInfo->PdiSrc));
	}
	else {
		Status = XST_FAILURE;
		goto END;
	}

	/** Send request using generic API */
	Status = XLoader_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, Response, (u32)RESPONSE_ARG_CNT);
	*DestSize = Response[0U];

END:
	return Status;
}

/** @} end of xloader_client_apis group */
