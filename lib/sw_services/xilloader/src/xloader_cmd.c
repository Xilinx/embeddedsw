/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_cmd.c
*
* This file contains the xloader commands implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/12/2019 Initial release
* 1.01  kc   04/09/2019 Added support to load partial Pdi
* 1.02  har  08/28/2019 Fixed MISRA C violations
* 1.03  bsv  02/27/2020 Added support for delay handoff
*       bsv  03/09/2020 Added CDO features command for xilloader
*       bsv  04/09/2020 Code clean up Xilloader
* 1.04  kc   06/12/2020 Added IPI mask to PDI CDO commands to get
*                       subsystem information
*       kc   07/28/2020 PLM mode is set to configuration during PDI load
*       bsv  07/29/2020 Removed hard coding of DDR back up address
*       bm   08/03/2020 Added LoadReadBackPdi Cmd
*       bsv  08/10/2020 Added subsystem restart support from DDR
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       bm   08/19/2020 Added ImageInfo Cmds
*       bm   09/21/2020 Modified ImageInfo related API calls
*       bm   09/24/2020 Added FuncID parameter in LoadDdrCpyImg
*       bsv  10/13/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xloader.h"
#include "xplmi_cmd.h"
#include "xplmi_generic.h"
#include "xplmi_modules.h"
#include "xplmi_wdt.h"
#include "xplmi_event_logging.h"

/************************** Constant Definitions *****************************/

/* READBACK Cmd Macros */
#define XLOADER_BUFFER_MAX_SIZE_MASK		(0x7FFFFFFFU)

/* Get Image Info List Macros */
#define XLOADER_NUM_ENTRIES_MASK		(0x0000FFFFU)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
static XPlmi_Module XPlmi_Loader;

#define XLOADER_CMD_FEATURES_CMD_ID_INDEX	(0U)
#define XLOADER_CMD_DDR_CPY_IMGID_INDEX		(0U)
#define XLOADER_CMD_DDR_CPY_FUNCID_INDEX	(1U)
#define XLOADER_CMD_LOAD_PDI_PDISRC_INDEX	(0U)
#define XLOADER_CMD_LOAD_PDI_PDIADDR_HIGH_INDEX		(1U)
#define XLOADER_CMD_LOAD_PDI_PDIADDR_LOW_INDEX		(2U)
#define XLOADER_CMD_GET_IMG_INFO_IMGID_INDEX		(0U)
#define XLOADER_CMD_GET_IMG_INFO_LIST_DESTADDR_HIGH_INDEX	(0U)
#define XLOADER_CMD_GET_IMG_INFO_LIST_DESTADDR_LOW_INDEX	(1U)
#define XLOADER_CMD_GET_IMG_INFO_LIST_MAXLEN_INDEX	(2U)
#define XLOADER_CMD_READBACK_PDIADDR_HIGH_INDEX		(3U)
#define XLOADER_CMD_READBACK_PDIADDR_LOW_INDEX		(4U)
#define XLOADER_CMD_READBACK_MAXLEN_INDEX		(5U)
#define XLOADER_RESP_CMD_EXEC_STATUS_INDEX	(0U)
#define XLOADER_RESP_CMD_FEATURES_CMD_SUPPORTED	(1U)
#define XLOADER_RESP_CMD_READBACK_PROCESSED_LEN_INDEX	(1U)
#define XLOADER_RESP_CMD_GET_IMG_INFO_UID_INDEX		(1U)
#define XLOADER_RESP_CMD_GET_IMG_INFO_PID_INDEX		(2U)
#define XLOADER_RESP_CMD_GET_IMG_INFO_FUNCID_INDEX		(3U)
#define XLOADER_RESP_CMD_GET_IMG_INFO_LIST_NUM_ENTRIES_INDEX	(1U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/*****************************************************************************/
/**
 * @brief This function checks if a particular Loader Command ID is supported
 * or not. Command ID is the only payload parameter.
 *
 * @param Cmd is pointer to the command structure
 *
 * @return Returns XST_SUCCESS
 *
 *****************************************************************************/
static int XLoader_Features(XPlmi_Cmd *Cmd)
{
	if (Cmd->Payload[XLOADER_CMD_FEATURES_CMD_ID_INDEX] <
		XPlmi_Loader.CmdCnt) {
		Cmd->Response[XLOADER_RESP_CMD_FEATURES_CMD_SUPPORTED] = XLOADER_SUCCESS;
	}
	else {
		Cmd->Response[XLOADER_RESP_CMD_FEATURES_CMD_SUPPORTED] = XLOADER_FAILURE;
	}
	Cmd->Response[XLOADER_RESP_CMD_EXEC_STATUS_INDEX] = XLOADER_SUCCESS;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function provides load DDR copy image execution.
 * Command payload parameters are
 *	* Img ID - of ddr copied image
 *	* Func ID - to verify with the FuncID of the image copied to DDR
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_LoadDdrCpyImg(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 ImgId = Cmd->Payload[XLOADER_CMD_DDR_CPY_IMGID_INDEX];
	u32 *FuncID = (Cmd->Len > XLOADER_CMD_DDR_CPY_FUNCID_INDEX) ?
		(&Cmd->Payload[XLOADER_CMD_DDR_CPY_FUNCID_INDEX]) : NULL;
	XilPdi* PdiPtr = BootPdiPtr;

	PdiPtr->IpiMask = Cmd->IpiMask;
	XPlmi_Printf(DEBUG_INFO, "%s \n\r", __func__);

	XPlmi_SetPlmMode(XPLMI_MODE_CONFIGURATION);

	Status = XLoader_RestartImage(ImgId, FuncID);
	if (Status != XST_SUCCESS) {
		/* Update the error code */
		XPlmi_ErrMgr(Status);
		goto END;
	}

END:
	XPlmi_SetPlmMode(XPLMI_MODE_OPERATIONAL);
	Cmd->Response[XLOADER_RESP_CMD_EXEC_STATUS_INDEX] = (u32)Status;
	return Status;
}


/*****************************************************************************/
/**
 * @brief	This function provides PDI execution from DDR.
 *  Command payload parameters are
 *	- PdiSrc - Boot Mode values, DDR, PCIe
 *	- PdiAddr - 64bit PDI address located in the Source
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_LoadSubsystemPdi(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	PdiSrc_t PdiSrc;
	u64 PdiAddr;
	XilPdi* PdiPtr = &SubsystemPdiIns;

	XPlmi_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	/* Store the command fields in resume data */
	PdiSrc = (PdiSrc_t)(Cmd->Payload[XLOADER_CMD_LOAD_PDI_PDISRC_INDEX]);
	PdiAddr = (u64)Cmd->Payload[XLOADER_CMD_LOAD_PDI_PDIADDR_HIGH_INDEX];
	PdiAddr = ((u64)(Cmd->Payload[XLOADER_CMD_LOAD_PDI_PDIADDR_LOW_INDEX]) |
			(PdiAddr << 32U));

	XPlmi_Printf(DEBUG_INFO, "Subsystem PDI Load: Started\n\r");

	PdiPtr->PdiType = XLOADER_PDI_TYPE_PARTIAL;
	PdiPtr->IpiMask = Cmd->IpiMask;
	Status = XLoader_LoadPdi(PdiPtr, PdiSrc, PdiAddr);
	if (Status != XST_SUCCESS) {
		/* Update the error code */
		XPlmi_ErrMgr(Status);
		goto END;
	}

	XPlmi_Printf(DEBUG_GENERAL, "Subsystem PDI Load: Done\n\r");

END:
	Cmd->Response[XLOADER_RESP_CMD_EXEC_STATUS_INDEX] = (u32)Status;
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides ImageInfo stored in ImageInfoTbl for a
 *  given NodeID.
 *  Command payload parameters are
 *	- Node ID
 *  Command Response parameters are
 * 	- Unique ID
 * 	- Parent Unique ID
 * 	- Function ID
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_GetImageInfo(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	XLoader_ImageInfo *ImageInfo;

	if (Cmd->Payload[XLOADER_CMD_GET_IMG_INFO_IMGID_INDEX] ==
		XLOADER_INVALID_IMG_ID) {
		Status = XLOADER_ERR_INVALID_IMGID;
		XPlmi_Printf(DEBUG_GENERAL, "Invalid ImgID\n\r");
		goto END;
	}

	ImageInfo = XLoader_GetImageInfoEntry(
		Cmd->Payload[XLOADER_CMD_GET_IMG_INFO_IMGID_INDEX]);
	if (ImageInfo == NULL) {
		Status = XLOADER_ERR_NO_VALID_IMG_FOUND;
		goto END;
	}
	if ((ImageInfo->ImgID != Cmd->Payload[XLOADER_CMD_GET_IMG_INFO_IMGID_INDEX])
		|| (ImageInfo->ImgID == XLOADER_INVALID_IMG_ID)) {
		Status = XLOADER_ERR_NO_VALID_IMG_FOUND;
		XPlmi_Printf(DEBUG_GENERAL, "No Valid Image Entry Found\n\r");
		goto END;
	}

	Cmd->Response[XLOADER_RESP_CMD_GET_IMG_INFO_UID_INDEX] = ImageInfo->UID;
	Cmd->Response[XLOADER_RESP_CMD_GET_IMG_INFO_PID_INDEX] = ImageInfo->PUID;
	Cmd->Response[XLOADER_RESP_CMD_GET_IMG_INFO_FUNCID_INDEX] =
		ImageInfo->FuncID;

	Status = XST_SUCCESS;

END:
	Cmd->Response[XLOADER_RESP_CMD_EXEC_STATUS_INDEX] = (u32)Status;
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function does nothing but provides a command which is
 * used by bootgen to set the Image Header IDs
 *
 *  Command payload parameters are
 *	- Node ID
 *	- Unique ID
 *	- Parent Unique ID
 *	- Function ID
 * @param	Cmd is pointer to the command structure
 *
 * @return	XST_SUCCESS
 *
 *****************************************************************************/
static int XLoader_SetImageInfo(XPlmi_Cmd *Cmd)
{
	/* This acts as a placeholder for the implementation done by bootgen */
	Cmd->Response[XLOADER_RESP_CMD_EXEC_STATUS_INDEX] = (u32)XST_SUCCESS;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function gets ImageInfo Table and copies it into the
 * buffer address passed in the command.
 *
 *  Command payload parameters are
 *	- 64-bit Buffer Address
 *	- Max Size
 *
 *  Command Response parameters are
 * 	- Number of Entries Returned
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	XST_SUCCESS on success, error code on failure
 *
 *****************************************************************************/
static int XLoader_GetImageInfoList(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u64 DestAddr;
	u32 MaxSize;
	u32 NumEntries = 0U;

	DestAddr =
		(u64)Cmd->Payload[XLOADER_CMD_GET_IMG_INFO_LIST_DESTADDR_HIGH_INDEX];
	DestAddr =
		(((u64)Cmd->Payload[XLOADER_CMD_GET_IMG_INFO_LIST_DESTADDR_LOW_INDEX]) |
		(DestAddr << 32U));
	MaxSize = (u32)(Cmd->Payload[XLOADER_CMD_GET_IMG_INFO_LIST_MAXLEN_INDEX] &
			XLOADER_BUFFER_MAX_SIZE_MASK);
	Status = XLoader_LoadImageInfoTbl(DestAddr, MaxSize, &NumEntries);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Cmd->Response[XLOADER_RESP_CMD_GET_IMG_INFO_LIST_NUM_ENTRIES_INDEX] =
		NumEntries & XLOADER_NUM_ENTRIES_MASK;

END:
	Cmd->Response[XLOADER_RESP_CMD_EXEC_STATUS_INDEX] = (u32)Status;
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides loading of ReadBack PDI overriding
 *  destination address of readback data.
 *  Command payload parameters are
 *	- PdiSrc - Boot Mode values, DDR
 *	- PdiAddr - 64bit PDI address located in the Source
 *	- ReadbackDdrDestAddr - 64bit DDR destination address
 *	- MaxSize - MaxSize of the buffer present at destination address
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_LoadReadBackPdi(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	XPlmi_ReadBackProps ReadBack;
	XPlmi_ReadBackProps DefaultReadBack = {
		XPLMI_READBACK_DEF_DST_ADDR, 0U, 0U
	};

	ReadBack.DestAddr = (u64)Cmd->Payload[XLOADER_CMD_READBACK_PDIADDR_HIGH_INDEX];
	ReadBack.DestAddr = ((u64)(Cmd->Payload[XLOADER_CMD_READBACK_PDIADDR_LOW_INDEX]) |
		(ReadBack.DestAddr << 32U));
	ReadBack.MaxSize = Cmd->Payload[XLOADER_CMD_READBACK_MAXLEN_INDEX] &
		XLOADER_BUFFER_MAX_SIZE_MASK;
	ReadBack.ProcessedLen = 0U;

	Status = XPlmi_SetReadBackProps(&ReadBack);
	if (Status != XST_SUCCESS) {
		Cmd->Response[XLOADER_RESP_CMD_EXEC_STATUS_INDEX] = (u32)Status;
		goto END;
	}
	Status = XLoader_LoadSubsystemPdi(Cmd);
	if (Status != XST_SUCCESS) {
		goto END1;
	}

END1:
	Cmd->Response[XLOADER_RESP_CMD_EXEC_STATUS_INDEX] = (u32)Status;
	Status = XPlmi_GetReadBackPropsValue(&ReadBack);
	if (Status != XST_SUCCESS) {
		if (Cmd->Response[XLOADER_RESP_CMD_EXEC_STATUS_INDEX] == (u32)XST_SUCCESS) {
			Cmd->Response[XLOADER_RESP_CMD_EXEC_STATUS_INDEX] = (u32)Status;
		}
		goto END;
	}
	Cmd->Response[XLOADER_RESP_CMD_READBACK_PROCESSED_LEN_INDEX] =
		ReadBack.ProcessedLen;
END:
	Status = XPlmi_SetReadBackProps(&DefaultReadBack);
	if (Cmd->Response[XLOADER_RESP_CMD_EXEC_STATUS_INDEX] == (u32)XST_SUCCESS) {
		Cmd->Response[XLOADER_RESP_CMD_EXEC_STATUS_INDEX] = (u32)Status;
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function can act as a placeholder for Unimplemented cmds
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	XST_SUCCESS
 *
 *****************************************************************************/
static int XLoader_UnImplementedCmd(XPlmi_Cmd *Cmd)
{
	/* For MISRA C */
	(void)Cmd;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	Contains the array of PLM loader commands
 *
 *****************************************************************************/
static XPlmi_ModuleCmd XLoader_Cmds[] =
{
	XPLMI_MODULE_COMMAND(XLoader_Features),
	XPLMI_MODULE_COMMAND(XLoader_LoadSubsystemPdi),
	XPLMI_MODULE_COMMAND(XLoader_LoadDdrCpyImg),
	XPLMI_MODULE_COMMAND(XLoader_GetImageInfo),
	XPLMI_MODULE_COMMAND(XLoader_SetImageInfo),
	XPLMI_MODULE_COMMAND(XLoader_GetImageInfoList),
	XPLMI_MODULE_COMMAND(XLoader_UnImplementedCmd),
	XPLMI_MODULE_COMMAND(XLoader_LoadReadBackPdi)
};

/*****************************************************************************/
/**
 * @brief	Contains the module ID and loader commands array
 *
 *****************************************************************************/
static XPlmi_Module XPlmi_Loader =
{
	XPLMI_MODULE_LOADER_ID,
	XLoader_Cmds,
	XPLMI_ARRAY_SIZE(XLoader_Cmds),
};

/*****************************************************************************/
/**
 * @brief	This function registers the PLM Loader commands to the PLMI.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void XLoader_CmdsInit(void)
{
	XPlmi_ModuleRegister(&XPlmi_Loader);
}
