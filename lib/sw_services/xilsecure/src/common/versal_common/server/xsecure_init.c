/******************************************************************************
* Copyright (c) 2020 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023, Advanced Micro Devices, Inc.  All rights reserved.
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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_init.h"
#include "xsecure_cmd.h"
#ifdef VERSAL_NET
#include "xsecure_plat_kat.h"
#include "xplmi.h"
#endif

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
 * @brief       This function provides the pointer to the common Sha3_1 instance
 * which has to be used between PLM and xilsecure server
 *
 * @return
 *      -       Pointer to the XSecure_Sha3 instance for Versal Net
 *      -	NULL for non versal Net platforms
 *
 * @note	This is applicable only for Versal Net
 *
 ******************************************************************************/
XSecure_Sha3 *XSecure_GetSha3Instance1(void)
{
	XSecure_Sha3 *Sha3Ptr = NULL;
#if defined (VERSAL_NET)
	static XSecure_Sha3 Sha3Instance = {0U};

	Sha3Ptr = &Sha3Instance;
#endif

        return Sha3Ptr;
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

#ifndef PLM_RSA_EXCLUDE
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
#endif

/*****************************************************************************/
/**
 * @brief	This function initializes the trng in HRNG mode if it is not initialized
 *          and it is applicable only for VersalNet
 *
 * @return
 *		- XST_SUCCESS On Successful initialization
 *      - XST_FAILURE On Failure
 *
 *****************************************************************************/
int XSecure_TrngInit(void)
{
	int Status = XST_FAILURE;
#if defined(VERSAL_NET)
	XSecure_TrngInstance *TrngInstance = XSecure_GetTrngInstance();

	if ((XPlmi_IsKatRan(XPLMI_SECURE_TRNG_KAT_MASK) != TRUE) ||
		(TrngInstance->ErrorState != XSECURE_TRNG_HEALTHY)) {
		Status = XSecure_TrngPreOperationalSelfTests(TrngInstance);
		if (Status != XST_SUCCESS) {
			XPlmi_ClearKatMask(XPLMI_SECURE_TRNG_KAT_MASK);
			goto END;
		}
		else {
			XPlmi_SetKatMask(XPLMI_SECURE_TRNG_KAT_MASK);
		}
	}
	if ((TrngInstance->UserCfg.Mode != XSECURE_TRNG_HRNG_MODE) ||
		(TrngInstance->State == XSECURE_TRNG_UNINITIALIZED_STATE)) {
		Status = XSecure_TrngInitNCfgHrngMode();
	}
	Status = XST_SUCCESS;
END:
#else
	Status = XST_SUCCESS;
#endif

	return Status;
}
