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
*
commands.
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xloader.h"
#include "xplmi_cmd.h"
#include "xplmi_modules.h"
#ifdef XPLM_SEM
#include "xilsem.h"
#endif

/************************** Constant Definitions *****************************/

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
	Cmd->Response[0U] = Status;
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
	u32 ImgId;
	XilPdi* PdiPtr = BootPdiPtr;

	PdiPtr->ImageNum = 1U;
	PdiPtr->PrtnNum = 1U;
	PdiPtr->IpiMask = Cmd->IpiMask;
	XPlmi_Printf(DEBUG_INFO, "%s \n\r", __func__);

#if defined(XPLM_SEM) && defined(XSEM_CFRSCAN_EN)
	/* Stop the SEM scan before Image load */
	Status = XSem_CfrStopScan();
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_SEM_STOP_SCAN, Status);
		goto END;
	}
#endif

	/*
	 * Store the command fields in resume data
	 */
	PdiPtr->PdiType = XLOADER_PDI_TYPE_RESTORE;
	PdiPtr->PdiSrc = XLOADER_PDI_SRC_DDR;
	PdiPtr->DeviceCopy = DeviceOps[XLOADER_PDI_SRC_DDR].Copy;
	PdiPtr->MetaHdr.DeviceCopy = PdiPtr->DeviceCopy;
	ImgId = Cmd->Payload[0U];
	PdiPtr->PdiAddr = PdiPtr->MetaHdr.FlashOfstAddr =
		XLOADER_DDR_COPYIMAGE_BASEADDR;
	PdiPtr->CopyToMem = FALSE;
	PdiPtr->DelayHandoff = FALSE;

	Status = XLoader_LoadImage(PdiPtr, ImgId);
	if (Status != XST_SUCCESS) {
		/* Update the error code */
		XPlmi_ErrMgr(Status);
		goto END;
	}
	Status = XLoader_StartImage(PdiPtr);
	if (Status != XST_SUCCESS) {
		/* Update the error code */
		XPlmi_ErrMgr(Status);
		goto END;
	}

END:
#if defined(XPLM_SEM) && defined(XSEM_CFRSCAN_EN)
	/* Restart the SEM SCAN */
	Status = XSem_CfrInit();
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_SEM_CFR_INIT, Status);
		goto END1;
	}
END1:
#endif
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
	u32 PdiSrc;
	u64 PdiAddr;
	XilPdi* PdiPtr = &SubsystemPdiIns;

	XPlmi_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	/* Store the command fields in resume data */
	PdiSrc = Cmd->Payload[0U];
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
 * @brief	Contains the array of PLM loader commands
 *
 *****************************************************************************/
static XPlmi_ModuleCmd XLoader_Cmds[] =
{
	XPLMI_MODULE_COMMAND(XLoader_Features),
	XPLMI_MODULE_COMMAND(XLoader_LoadSubsystemPdi),
	XPLMI_MODULE_COMMAND(XLoader_LoadDdrCpyImg)
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
