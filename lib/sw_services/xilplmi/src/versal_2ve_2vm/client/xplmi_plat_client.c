/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xplmi_plat_client.c
 *
 * This file contains the implementation of the client interface functions
*
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  vm   03/16/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

/**
 * @addtogroup xilplmi_client_apis XilPlmi Client APIs
 * @{
 */

/*************************************** Include Files *******************************************/
#include "xplmi_plat_client.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static int XPlmi_InPlaceAsuUpdate(XPlmi_ClientInstance *InstancePtr, const u32 Flag, u32 PdiValue);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to perform in-place ASU update
 *
 * @param	InstancePtr	Pointer to XPlmi_ClientInstance
 * @param	Flag		Update flag (0 = use Image Store, 1 = use DDR)
 * @param	PdiValue	PDI ID (Image Store) or DDR address
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XST_FAILURE on failure.
 *
 *************************************************************************************************/
static int XPlmi_InPlaceAsuUpdate(XPlmi_ClientInstance *InstancePtr, const u32 Flag, u32 PdiValue)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};

	/** Prepare payload for In-Place ASU update command */
	XPLMI_PACK_PAYLOAD2(Payload, (u32)XPLMI_INPLACE_ASU_UPDATE_CMD_ID, Flag, PdiValue);

	/** Send an IPI request to the PLM for In-Place ASU update */
	Status = XPlmi_ProcessMailbox(InstancePtr, Payload, PAYLOAD_ARG_CNT);


	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to perform in-place ASU update from DDR
 *
 * @param	Ptr	Pointer to XPlmi_ClientInstance
 * @param	Flag	Update flag (should be 1 for DDR source)
 * @param	DDRAddr	Address of update PDI in DDR
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XST_FAILURE on failure.
 *
 *************************************************************************************************/
int XPlmi_InPlaceAsuUpdate_DDR(XPlmi_ClientInstance *Ptr, const u32 Flag, u32 DDRAddr)
{
	int Status = XST_FAILURE;
	Status = XPlmi_InPlaceAsuUpdate(Ptr, Flag, DDRAddr);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to perform in-place ASU update from Image Store
 *
 * @param	Ptr	Pointer to XPlmi_ClientInstance
 * @param	Flag	Update flag (should be 0 for Image Store source)
 * @param	PdiId	Image store PDI ID
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XST_FAILURE on failure.
 *
 *************************************************************************************************/
int XPlmi_InPlaceAsuUpdate_ImageStore(XPlmi_ClientInstance *Ptr, const u32 Flag, u32 PdiId)
{
	int Status = XST_FAILURE;
	Status = XPlmi_InPlaceAsuUpdate(Ptr, Flag, PdiId);

	return Status;
}

/** @} End of xilplmi_client_apis group */
