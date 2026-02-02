/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xocp_dme.c
* @addtogroup xilocp_dme_apis XilOcp DME APIs
* @{
*
* This file contains the xilocp DME implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.7   rmv  01/30/26 Refactor OCP library
*
* </pre>
*
**************************************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_OCP
#include "xil_types.h"
#include "xocp_common.h"
#include "xocp_dme.h"
#include "xocp_dice_dme.h"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/**< DME challenge available */
static u32 IsDmeChlAvail = FALSE;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function returns pointer to XOcp_DmeResponse
 *
 * @return
 *		- DmeRespone Pointer to XOcp_DmeResponse
 *
 ******************************************************************************/
XOcp_DmeResponse* XOcp_GetDmeResponse(void)
{
	static XOcp_DmeResponse DmeResponse = {0U};

	return &DmeResponse;
}

/*****************************************************************************/
/**
 * @brief	This is wrapper function to generate DME response.
 *
 * @param	NonceAddr holds the address of 32 bytes buffer Nonce,
 *		which shall be used to fill one of the member of DME structure.
 * @param	DmeStructResAddr is the address to the 224 bytes buffer,
 *		which is used to store the response to DME challenge request of
 *		type XOcp_DmeResponse.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- Error code in case of failure.
 *
 ******************************************************************************/
int XOcp_GenerateDmeResponse(u64 NonceAddr, u64 DmeStructResAddr)
{
	int Status = XST_FAILURE;

	Status = XOcp_GenerateDmeResponseImpl(NonceAddr, DmeStructResAddr);
	if (Status == XST_SUCCESS) {
		IsDmeChlAvail = TRUE;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function returns the status of Dme challenge response
 *
 ******************************************************************************/
u32 XOcp_IsDmeChlAvail(void)
{
	return IsDmeChlAvail;
}

#endif /* PLM OCP */
/** @} */
