/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_init.c
*
* This file contains the initialization functions to be called by PLM. This
* file will only be part of XilSecure when it is compiled with PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   rpo 06/25/2020 Initial release
* 4.3   rpo 06/25/2020 Updated file version to sync with library version
*	rpo 08/19/2020 Clear the tamper interrupt source
*	am  09/24/2020 Resolved MISRA C violations
*       har 10/12/2020 Addressed security review comments
* 4.5   ma  04/05/2021 Use error mask instead of ID to set an error action
*       bm  05/13/2021 Add common crypto instances
* 4.6   har 07/14/2021 Fixed doxygen warnings
* 4.7   ma   07/08/2022 Added support for secure lockdown
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_init.h"
#include "xsecure_cmd.h"

/************************** Constant Definitions *****************************/
/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function registers the handler for tamper interrupt
 *
 * @return
 *	-	XST_SUCCESS - Always returns success
 *
 *****************************************************************************/
int XSecure_Init(void)
{
	int Status = XST_FAILURE;

	XSecure_CmdsInit();
	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides the pointer to the common Sha3 instance
 * which has to be used between PLM and xilsecure server
 *
 * @return
 *	-	Pointer to the XSecure_Sha3 instance
 *
 *****************************************************************************/
XSecure_Sha3 *XSecure_GetSha3Instance(void)
{
	static XSecure_Sha3 Sha3Instance = {0U};

	return &Sha3Instance;
}

/*****************************************************************************/
/**
 * @brief	This function provides the pointer to the common Aes instance
 * which has to be used between PLM and xilsecure server
 *
 * @return
 *	-	Pointer to the XSecure_Aes instance
 *
 *****************************************************************************/
XSecure_Aes *XSecure_GetAesInstance(void)
{
	static XSecure_Aes AesInstance = {0U};

	return &AesInstance;
}

/*****************************************************************************/
/**
 * @brief	This function provides the pointer to the common Rsa instance
 * which has to be used between PLM and xilsecure server
 *
 * @return
 *	-	Pointer to the XSecure_Rsa instance
 *
 *****************************************************************************/
XSecure_Rsa *XSecure_GetRsaInstance(void)
{
	static XSecure_Rsa RsaInstance = {0U};

	return &RsaInstance;
}
