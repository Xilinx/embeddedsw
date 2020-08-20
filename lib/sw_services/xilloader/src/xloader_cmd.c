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
*						subsystem information
*       kc   07/28/2020 PLM mode is set to configuration during PDI load
*       bsv  07/29/2020 Removed hard coding of DDR back up address
*       bm   08/03/2020 Added LoadReadBackPdi Cmd
*       bsv  08/10/2020 Added subsystem restart support from DDR
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       bm   08/19/2020 Added ImageInfo Cmds
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
#ifdef XPLM_SEM
#include "xilsem.h"
#endif

/************************** Constant Definitions *****************************/

/* READBACK Cmd Macros */
#define XLOADER_BUFFER_MAX_SIZE_MASK		(0x7FFFFFFFU)

/* Get Image Info List Macros */
#define XLOADER_NUM_ENTRIES_MASK		(0x0000FFFFU)
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
extern XilPdi* BootPdiPtr;
extern XLoader_DeviceOps DeviceOps[];
static XPlmi_Module XPlmi_Loader;

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/*****************************************************************************/
/**
 * @brief This function checks if a particular Loader Command ID is supported
 * or not. Command ID is the only payload parameter.
 *
 * @param Pointer to the command structure
 *
 * @return Returns XST_SUCCESS
 *
 *****************************************************************************/
static int XLoader_Features(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;

	if (Cmd->Payload[0U] < XPlmi_Loader.CmdCnt) {
		Cmd->Response[1U] = XLOADER_SUCCESS;
	}
	else {
		Cmd->Response[1U] = XLOADER_FAILURE;
	}
	Status = XST_SUCCESS;
	Cmd->Response[0U] = XLOADER_SUCCESS;
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides load DDR copy image execution.
 * Command payload parameters are
 *	* Img ID - of ddr copied image
 *
 * @param	Pointer to the command structure
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_LoadDdrCpyImg(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;
	u32 ImgId = Cmd->Payload[0U];
	XilPdi* PdiPtr = BootPdiPtr;

	PdiPtr->IpiMask = Cmd->IpiMask;
	XPlmi_Printf(DEBUG_INFO, "%s \n\r", __func__);

	XPlmi_SetPlmMode(XPLMI_MODE_CONFIGURATION);

	Status = XLoader_RestartImage(ImgId);
	if (Status != XST_SUCCESS) {
		/* Update the error code */
		XPlmi_ErrMgr(Status);
		goto END;
	}

END:
	XPlmi_SetPlmMode(XPLMI_MODE_OPERATIONAL);
	Cmd->Response[0U] = (u32)Status;
	return Status;
}


/*****************************************************************************/
/**
 * @brief	This function provides PDI execution from DDR.
 *  Command payload parameters are
 *	* PdiSrc - Boot Mode values, DDR, PCIe
 *	* PdiAddr - 64bit PDI address located in the Source
 *
 * @param	Pointer to the command structure
 *
 * @return	Returns the Load PDI command
 *
 *****************************************************************************/
static int XLoader_LoadSubsystemPdi(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;
	PdiSrc_t PdiSrc;
	u64 PdiAddr;
	XilPdi* PdiPtr = &SubsystemPdiIns;

	XPlmi_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	/* Store the command fields in resume data */
	PdiSrc = (PdiSrc_t)(Cmd->Payload[0U]);
	PdiAddr = (u64)Cmd->Payload[1U];
	PdiAddr = ((u64)(Cmd->Payload[2U]) | (PdiAddr << 32U));

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
	Cmd->Response[0U] = (u32)Status;
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides ImageInfo stored in ImageInfoTbl for a
 *  given NodeID.
 *  Command payload parameters are
 *	* Node ID
 *  Command Response parameters are
 * 	* Unique ID
 * 	* Parent Unique ID
 * 	* Function ID
 *
 * @param	Pointer to the command structure
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_GetImageInfo(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;
	XPlmi_ImageInfo *ImageInfo;
	u32 Index;

	if (Cmd->Payload[0U] == XPLMI_INVALID_IMG_ID) {
		Status = XLOADER_ERR_INVALID_IMGID;
		XPlmi_Printf(DEBUG_GENERAL, "Invalid ImgID\n\r");
		goto END;
	}

	ImageInfo = XPlmi_GetImageInfoEntry(Cmd->Payload[0U], &Index);
	if (ImageInfo == NULL) {
		Status = XLOADER_ERR_NO_VALID_IMG_FOUND;
		goto END;
	}
	if ((ImageInfo->ImgID != Cmd->Payload[0U]) ||
		(ImageInfo->ImgID == XPLMI_INVALID_IMG_ID)) {
		Status = XLOADER_ERR_NO_VALID_IMG_FOUND;
		XPlmi_Printf(DEBUG_GENERAL, "No Valid Image Entry Found\n\r");
		goto END;
	}

	Cmd->Response[1U] = ImageInfo->UID;
	Cmd->Response[2U] = ImageInfo->PUID;
	Cmd->Response[3U] = ImageInfo->FuncID;

	Status = XST_SUCCESS;

END:
	Cmd->Response[0U] = (u32)Status;
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function does nothing but provides a command which is
 * used by bootgen to set the Image Header IDs
 *
 *  Command payload parameters are
 *	* Node ID
 *	* Unique ID
 *	* Parent Unique ID
 *	* Function ID
 * @param	Pointer to the command structure
 *
 * @return	XST_SUCCESS
 *
 *****************************************************************************/
static int XLoader_SetImageInfo(XPlmi_Cmd * Cmd)
{
	/* This acts as a placeholder for the implementation done by bootgen */
	Cmd->Response[0U] = (u32)XST_SUCCESS;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function gets ImageInfo Table and copies it into the
 * buffer address passed in the command.
 *
 *  Command payload parameters are
 *	* 64-bit Buffer Address
 *	* Max Size
 *
 *  Command Response parameters are
 * 	* Number of Entries Returned
 *
 * @param	Pointer to the command structure
 *
 * @return	XST_SUCCESS on success, error code on failure
 *
 *****************************************************************************/
static int XLoader_GetImageInfoList(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;
	u64 DestAddr;
	u32 MaxSize;
	u32 NumEntries = 0U;

	DestAddr = (u64)Cmd->Payload[0U];
	DestAddr = (((u64)Cmd->Payload[1U]) | (DestAddr << 32U));
	MaxSize = (u32)(Cmd->Payload[2U] & XLOADER_BUFFER_MAX_SIZE_MASK);
	Status = XPlmi_LoadImageInfoTbl(DestAddr, MaxSize, &NumEntries);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Cmd->Response[1U] = NumEntries & XLOADER_NUM_ENTRIES_MASK;

END:
	Cmd->Response[0U] = (u32)Status;
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides loading of ReadBack PDI overriding
 *  destination address of readback data.
 *  Command payload parameters are
 *	* PdiSrc - Boot Mode values, DDR
 *	* PdiAddr - 64bit PDI address located in the Source
 *	* ReadbackDdrDestAddr - 64bit DDR destination address
 *	* MaxSize - MaxSize of the buffer present at destination address
 *
 * @param	Pointer to the command structure
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_LoadReadBackPdi(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;
	XPlmi_ReadBackProps ReadBack;
	XPlmi_ReadBackProps DefaultReadBack = {
		XPLMI_READBACK_DEF_DST_ADDR, 0U, 0U
	};

	ReadBack.DestAddr = (u64)Cmd->Payload[3U];
	ReadBack.DestAddr = ((u64)(Cmd->Payload[4U]) |
					(ReadBack.DestAddr << 32U));
	ReadBack.MaxSize = Cmd->Payload[5U] & XLOADER_BUFFER_MAX_SIZE_MASK;
	ReadBack.ProcessedLen = 0U;

	XPlmi_SetReadBackProps(&ReadBack);
	Status = XLoader_LoadSubsystemPdi(Cmd);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	XPlmi_GetReadBackPropsValue(&ReadBack);
	XPlmi_SetReadBackProps(&DefaultReadBack);
	Cmd->Response[0U] = (u32)Status;
	Cmd->Response[1U] = ReadBack.ProcessedLen;
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function can act as a placeholder for Unimplemented cmds
 *
 * @param	Pointer to the command structure
 *
 * @return	XST_SUCCESS
 *
 *****************************************************************************/
static int XLoader_UnImplementedCmd(XPlmi_Cmd * Cmd)
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
