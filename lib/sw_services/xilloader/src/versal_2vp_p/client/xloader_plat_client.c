/***************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
 *
 * @file versal_2vp_p/client/xloader_plat_client.c
 *
 * This file contains the implementation of the client interface functions
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 2.5   tvp  08/03/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

/**
 * @addtogroup xloader_client_apis XilLoader Client APIs
 * @{
 */

/**************************************** Include Files *******************************************/
#include "xloader_plat_client.h"
#include "xloader_defs.h"

/*************************************** Constant Definitions *************************************/

/***************************************** Type Definitions ***************************************/

/*************************************** Function Prototypes **************************************/

/*************************************** Variable Definitions *************************************/

/**************************************************************************************************/
/**
 * @brief	This function sends IPI request to configure jtag status.
 *
 * @param	InstancePtr	Pointer to the client instance.
 * @param	Flag		To enable/disable jtag.
 *
 * @return
 *		 - XST_SUCCESS on success.
 *		 - XST_FAILURE on failure.
 *
 **************************************************************************************************/
int XLoader_ConfigureJtagState(XLoader_ClientInstance *InstancePtr, u32 Flag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Perform input parameters validation. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI Payload */
	XLOADER_PACK_PAYLOAD1(Payload, (u32)XLOADER_CMD_ID_CONFIG_JTAG_STATE, Flag);

	/**
	 * - Send an IPI request to the PLM by using the XLoader_ConfigureJtagState CDO command
	 *   Wait for IPI response from PLM with a timeout.
	 * - If the timeout exceeds then error is returned otherwise it returns the status of the
	 *   IPI response.
	 */
	Status = XLoader_ProcessMailbox(InstancePtr, Payload, PAYLOAD_ARG_CNT);

END:
	return Status;
}
/** @} end of xloader_client_apis group */
