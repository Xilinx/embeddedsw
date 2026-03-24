/**************************************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xplmi_plat_cmd_client.c
 *
 * This file contains the implementation of the client interface functions
*
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  dd   01/09/24 Initial release
 *       am   04/04/24 Fixed doxygen warnings
 * 2.4   gnr  03/18/26 Updated the Payload assignments with XPLMI_PACK_PAYLOAD macros
 *
 * </pre>
 *
 *************************************************************************************************/

/**
 * @addtogroup xilplmi_client_apis XilPlmi Client APIs
 * @{
 */

/*************************************** Include Files *******************************************/

#include "xplmi_plat_cmd_client.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

static int XPlmi_InPlacePlmUpdate(XPlmi_ClientInstance *InstancePtr,const u32 Flag, u32 PdiValue);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to perform in-place PLM update
 *
 * @param	InstancePtr	Pointer to XPlmi_ClientInstance
 * @param	Flag		To enable / disable jtag
 * @param	PdiValue	To select the pdi src
 *
 *  @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 *************************************************************************************************/
static int XPlmi_InPlacePlmUpdate(XPlmi_ClientInstance *InstancePtr,const u32 Flag, u32 PdiValue)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

    /**
	 * - Performs input parameters validation. Return error code if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI Payload */
	XPLMI_PACK_PAYLOAD2(Payload, (u32)XPLMI_INPLACE_PLM_UPDATE_CMD_ID, Flag, PdiValue);

	/**
	 * - Send an IPI request to the PLM by using the XPlmi_InPlacePlmUpdate CDO command
	 * Wait for IPI response from PLM with a timeout.
	 * - If the timeout exceeds then error is returned otherwise it returns the status of the IPI
	 * response.
	 */
	Status = XPlmi_ProcessMailbox(InstancePtr, Payload, PAYLOAD_ARG_CNT);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to perform in-place PLM update from DDR
 *
 * @param	Ptr	Pointer to XPlmi_ClientInstance
 * @param	Flag	To enable / disable jtag
 * @param	DDRAddr	Address of DDR
 *
 *  @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 *************************************************************************************************/
int XPlmi_InPlacePlmUpdate_DDR(XPlmi_ClientInstance *Ptr,const u32 Flag, u32 DDRAddr)
{
	int Status = XST_FAILURE;
	Status = XPlmi_InPlacePlmUpdate(Ptr, Flag, DDRAddr);
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to perform in-place PLM update from Image Store
 *
 * @param	Ptr	Pointer to XPlmi_ClientInstance
 * @param	Flag	To enable / disable jtag
 * @param	PdiId	Image store pdi id
 *
 *  @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 *************************************************************************************************/
int XPlmi_InPlacePlmUpdate_ImageStore(XPlmi_ClientInstance *Ptr,const u32 Flag, u32 PdiId)
{
	int Status = XST_FAILURE;
	Status = XPlmi_InPlacePlmUpdate(Ptr, Flag, PdiId);
	return Status;
}

/** @} End of xilplmi_client_apis group */
