/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf_client.c
*
* This file contains the implementation of the client interface functions for
* PUF hardware interface API's.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt  01/04/22 Initial release
*       am   02/28/22 Fixed MISRA C violation rule 10.3
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xpuf_client.h"
#include "xpuf_ipi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function sends IPI request for PUF registration
 *
 * @param	DataAddr	Address of the data structure which includes
 * 				        options to configure PUF
 *
 * @return	- XST_SUCCESS - If the PUF registration is successful
 * 		    - XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XPuf_Registration(const u64 DataAddr)
{
	volatile int Status = XST_FAILURE;

	Status = XPuf_ProcessIpiWithPayload2((u32)XPUF_PUF_REGISTRATION,
			(u32)DataAddr, (u32)(DataAddr >> 32U));
	if (Status != XST_SUCCESS) {
		XPuf_Printf(XPUF_DEBUG_GENERAL, "PUF registration Failed \r\n");
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function sends IPI request for PUF regeneration
 *
 * @param	DataAddr	Address of the data structure which includes
 * 				        options to configure PUF
 *
 * @return	- XST_SUCCESS - If the PUF regeneration is successful
 * 		    - XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XPuf_Regeneration(const u64 DataAddr)
{
	volatile int Status = XST_FAILURE;

	Status = XPuf_ProcessIpiWithPayload2((u32)XPUF_PUF_REGENERATION,
			(u32)DataAddr, (u32)(DataAddr >> 32U));
	if (Status != XST_SUCCESS) {
		XPuf_Printf(XPUF_DEBUG_GENERAL, "PUF regeneration Failed \r\n");
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function sends IPI request for PUF clear ID
 *
 *
 * @return	- XST_SUCCESS - If the PUF clear ID is successful
 * 		    - XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XPuf_ClearPufID(void)
{
	volatile int Status = XST_FAILURE;

	Status = XPuf_ProcessIpiWithPayload0((u32)XPUF_PUF_CLEAR_PUF_ID);
	if (Status != XST_SUCCESS) {
		XPuf_Printf(XPUF_DEBUG_GENERAL, "Clear PUF ID Failed \r\n");
	}

	return Status;
}