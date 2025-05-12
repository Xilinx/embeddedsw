/******************************************************************************
* Copyright (c) 2020 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025, Advanced Micro Devices, Inc.  All rights reserved.
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
*       rpo 08/19/2020 Clear the tamper interrupt source
*       am  09/24/2020 Resolved MISRA C violations
*       har 10/12/2020 Addressed security review comments
* 4.5   ma  04/05/2021 Use error mask instead of ID to set an error action
*       bm  05/13/2021 Add common crypto instances
* 4.6   har 07/14/2021 Fixed doxygen warnings
* 4.7   ma  07/08/2022 Added support for secure lockdown
* 5.1   dc  12/27/2022 Added SHA1 instance
* 5.2   yog 08/07/2023 Moved Trng init API to xsecure_plat.c
* 5.4   kpt 06/23/2024 Added XSecure_AddRsaKeyPairGenerationToScheduler
*       kpt 07/17/2024 Remove RSA keypair generation support on QEMU
*       kal 07/24/2024 Code refactoring for versal_2ve_2vm
*       pre 03/02/2025 Modified XSecure_Init for initialization of AES and SHA in server mode
*       sd  04/30/2025 Make XSecure_AesShaInit as non static function and move
*                      XSecure_QueuesAndTaskInit to XSecure_Init function
*       pre  05/10/2025 Added AES and SHA events queuing mechanism under XPLMI_IPI_DEVICE_ID macro
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
#if defined(VERSAL_NET) && !defined(VERSAL_2VE_2VM)
#include "xsecure_plat_kat.h"
#include "xsecure_plat_rsa.h"
#include "xplmi.h"
#endif
#ifdef VERSAL_PLM
#include "xsecure_resourcehandling.h"
#include "xplmi_dma.h"
#endif

/************************** Constant Definitions *****************************/

static XSecure_Sha XSecure_ShaInstance[XSECURE_SHA_NUM_OF_INSTANCES];

/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function initializes AES and SHA hardware instances
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - error code  On failure
 *
 *****************************************************************************/
int XSecure_AesShaInit(void)
{
	int Status = XST_FAILURE;
	XSecure_Sha *XSecureShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE_ID);
#ifndef PLM_SECURE_EXCLUDE
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();

	/** Initializes AES structure */
	Status = XSecure_AesInitialize(XSecureAesInstPtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

	/* Initializes SHA structure for operating the SHA3 engine */
	Status = XSecure_ShaInitialize(XSecureShaInstPtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifdef VERSAL_2VE_2VM
	XSecureShaInstPtr = XSecure_GetSha2Instance(XSECURE_SHA_1_DEVICE_ID);
	Status = XSecure_ShaInitialize(XSecureShaInstPtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

#if (defined(VERSAL_NET) && !defined(VERSAL_2VE_2VM))
	XSecureShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_1_DEVICE_ID);
	Status = XSecure_ShaLookupConfig(XSecureShaInstPtr, XSECURE_SHA_1_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_ShaInitialize(XSecureShaInstPtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function registers the handlers for XilSecure IPI commands
 *
 * @param   PpdiEventParamsPtr is the pointer to partial PDI event parameters
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - error code  On failure
 *
 *****************************************************************************/
int XSecure_Init(XSecure_PartialPdiEventParams *PpdiEventParamsPtr)
{
	int Status = XST_FAILURE;

	Status = XSecure_AesShaInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

#if (defined(PLM_ENABLE_SHA_AES_EVENTS_QUEUING) || defined(VERSAL_NET)\
     && defined(XPLMI_IPI_DEVICE_ID))
	/** AES & SHA IPI event queues and free resource task initialization */
	Status = XSecure_QueuesAndTaskInit(PpdiEventParamsPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

	(void)PpdiEventParamsPtr;

	XSecure_CmdsInit();

#if defined (VERSAL_NET) && !defined(PLM_RSA_EXCLUDE) && !defined(VERSAL_2VE_2VM)
	if ((XPLMI_PLATFORM != PMC_TAP_VERSION_QEMU) &&
		(XPLMI_PLATFORM != PMC_TAP_VERSION_COSIM)) {
        /* Add keypair generation to scheduler for versalnet */
		Status = XSecure_AddRsaKeyPairGenerationToScheduler();
	}
	else {
		Status = XST_SUCCESS;
	}
#else
	Status = XST_SUCCESS;
#endif

END:
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
XSecure_Sha *XSecure_GetShaInstance(u32 DeviceId)
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
