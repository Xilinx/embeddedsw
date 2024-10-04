/******************************************************************************
* Copyright (c) 2020 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024, Advanced Micro Devices, Inc.  All rights reserved.
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
* 4.7   ma  07/08/2022 Added support for secure lockdown
* 5.1   dc  12/27/2022 Added SHA1 instance
* 5.2   yog 08/07/2023 Moved Trng init API to xsecure_plat.c
* 5.4   kpt 06/23/2024 Added XSecure_AddRsaKeyPairGenerationToScheduler
*       kpt 07/17/2024 Remove RSA keypair generation support on QEMU
* 	kal 07/24/2024 Code refactoring for versal_aiepg2
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_generic_server_apis XilSecure Generic Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_init.h"
#include "xsecure_cmd.h"
#if defined(VERSAL_NET) && !defined(VERSAL_AIEPG2)
#include "xsecure_plat_kat.h"
#include "xsecure_plat_rsa.h"
#include "xplmi.h"
#endif

/************************** Constant Definitions *****************************/

static XSecure_Sha XSecure_ShaInstance[XSECURE_SHA_NUM_OF_INSTANCES];

/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static XSecure_Sha *XSecure_GetShaInstance(u32 DeviceId);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function registers the handlers for XilSecure IPI commands
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XST_FAILURE  On failure
 *
 *****************************************************************************/
int XSecure_Init(void)
{
	int Status = XST_FAILURE;

	XSecure_CmdsInit();

#if defined (VERSAL_NET) && !defined(PLM_RSA_EXCLUDE) && !defined(VERSAL_AIEPG2)
	if (XPLMI_PLATFORM != PMC_TAP_VERSION_QEMU) {
		/* Add keypair generation to scheduler for versalnet */
		Status = XSecure_AddRsaKeyPairGenerationToScheduler();
	}
	else {
		Status = XST_SUCCESS;
	}
#else
	Status = XST_SUCCESS;
#endif

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides the pointer to the sha3 instance
 *
 * @return
 *		 - Pointer to the XSecure_Sha instance
 *
 *****************************************************************************/
XSecure_Sha *XSecure_GetSha3Instance(u32 DeviceId)
{
	return XSecure_GetShaInstance(DeviceId);
}

/*****************************************************************************/
/**
 * @brief	This function provides the pointer to the sha2 instance
 *
 * @return
 *		 - Pointer to the XSecure_Sha instance
 *
 *****************************************************************************/
XSecure_Sha *XSecure_GetSha2Instance(u32 DeviceId)
{
	return XSecure_GetShaInstance(DeviceId);
}

/*****************************************************************************/
/**
 * @brief	This function provides the pointer to the common Aes instance
 * 		which has to be used between PLM and xilsecure server
 *
 * @return
 *		 - Pointer to the XSecure_Aes instance
 *
 *****************************************************************************/
XSecure_Aes *XSecure_GetAesInstance(void)
{
	static XSecure_Aes AesInstance = {0U};

	return &AesInstance;
}

#ifndef PLM_RSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function provides the pointer to the common Rsa instance
 * 		which has to be used between PLM and xilsecure server
 *
 * @return
 *		 - Pointer to the XSecure_Rsa instance
 *
 *****************************************************************************/
XSecure_Rsa *XSecure_GetRsaInstance(void)
{
	static XSecure_Rsa RsaInstance = {0U};

	return &RsaInstance;
}
#endif

/*****************************************************************************/
/**
 * @brief	This function provides the pointer to the sha instance
 * 		based on the DevieId
 *
 * @return
 *		 - Pointer to the XSecure_Sha instance
 *
 *****************************************************************************/
static XSecure_Sha *XSecure_GetShaInstance(u32 DeviceId)
{
	XSecure_Sha *XSecure_ShaInstPtr = NULL;

	if (DeviceId >= XSECURE_SHA_NUM_OF_INSTANCES) {
		goto END;
	}

	XSecure_ShaInstPtr = &XSecure_ShaInstance[DeviceId];
	XSecure_ShaInstPtr->DeviceId = DeviceId;

END:
	return XSecure_ShaInstPtr;
}
/** @} */
