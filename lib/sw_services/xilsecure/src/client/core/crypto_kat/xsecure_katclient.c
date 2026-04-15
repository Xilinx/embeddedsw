/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file client/core/crypto_kat/xsecure_katclient.c
*
* This file contains the implementation of the client interface functions for
* KAT.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt  07/18/22 Initial release
* 5.2   am   03/09/23 Replaced xsecure payload lengths with xmailbox payload lengths
*       am   06/19/23 Added error code for failure cases
* 5.4   yog  04/29/24 Fixed doxygen warnings.
*       pre  08/16/24 Added SSIT support
* 5.7   tbk  03/16/26 Added SMC support
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_kat_client_apis XilSecure KAT Client APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_katclient.h"
#include "xsecure_generic.h"
#include "xsecure_plat_defs.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to perform decrypt KAT on AES engine
 *
 * @param	InstancePtr	Pointer to the client instance
 *
 * @return
 *		 - XST_SUCCESS  When KAT Pass
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesDecryptKat(XSecure_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters
	 *   are invalid
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XSECURE_PACK_PAYLOAD1(Payload, ((InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT)
				| XSECURE_API_KAT),
				XSECURE_API_AES_DECRYPT_KAT);

	/**
	 * - Send request to PLM through generic request API.
	 *   This internally handles SMC or IPI mailbox based on build configuration.
	 */
	Status = XSecure_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to perform KAT on AES DPA
 *		countermeasure KAT
 *
 * @param	InstancePtr	Pointer to the client instance
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XST_INVALID_PARAM  If input parameters are invalid
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesDecryptCmKat(XSecure_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters
	 *   are invalid
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XSECURE_PACK_PAYLOAD1(Payload, ((InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT)
				| XSECURE_API_KAT),
				XSECURE_API_AES_DECRYPT_CM_KAT);

	/**
	 * - Send request to PLM through generic request API.
	 *   This internally handles SMC or IPI mailbox based on build configuration.
	 */
	Status = XSecure_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to perform RSA encrypt KAT
 *
 * @param	InstancePtr	 Pointer to the client instance
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XST_INVALID_PARAM  If input parameters are invalid
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_RsaPublicEncKat(XSecure_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters
	 *   are invalid
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XSECURE_PACK_PAYLOAD1(Payload, ((InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT)
				| XSECURE_API_KAT),
				XSECURE_API_RSA_PUB_ENC_KAT);

	/**
	 * - Send request to PLM through generic request API.
	 *   This internally handles SMC or IPI mailbox based on build configuration.
	 */
	Status = XSecure_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to perform SHA3 KAT
 *
 * @param	InstancePtr	Pointer to the client instance
 *
 * @return
 *		 - XST_SUCCESS  When KAT Pass
 *		 - XST_INVALID_PARAM  If input parameters are invalid
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_Sha3Kat(XSecure_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters
	 *   are invalid
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XSECURE_PACK_PAYLOAD1(Payload, ((InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT)
				| XSECURE_API_KAT),
				XSECURE_API_SHA3_KAT);

	/**
	 * - Send request to PLM through generic request API.
	 *   This internally handles SMC or IPI mailbox based on build configuration.
	 */
	Status = XSecure_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to perform ECC sign verify KAT
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	CurveClass	Type of elliptic curve class(Prime - 0, Binary - 1)
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XST_INVALID_PARAM  If input parameters are invalid
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_EllipticSignVerifyKat(XSecure_ClientInstance *InstancePtr, XSecure_EccCrvClass CurveClass)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters
	 *   are invalid
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XSECURE_PACK_PAYLOAD2(Payload, ((InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT)
				| XSECURE_API_KAT),
				XSECURE_API_ELLIPTIC_SIGN_VERIFY_KAT,
				CurveClass);

	/**
	 * - Send request to PLM through generic request API.
	 *   This internally handles SMC or IPI mailbox based on build configuration.
	 */
	Status = XSecure_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to perform encrypt KAT
 *
 * @param	InstancePtr	Pointer to the client instance
 *
 * @return
 *		 - XST_SUCCESS When KAT Pass
 *		 - XST_INVALID_PARAM  If input parameters are invalid
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_AesEncryptKat(XSecure_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters
	 *   are invalid
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XSECURE_PACK_PAYLOAD1(Payload, ((InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT)
				| XSECURE_API_KAT),
				XSECURE_API_AES_ENCRYPT_KAT);

	/**
	 * - Send request to PLM through generic request API.
	 *   This internally handles SMC or IPI mailbox based on build configuration.
	 */
	Status = XSecure_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to perform RSA private decrypt KAT
 *
 * @param	InstancePtr	Pointer to the client instance
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XST_INVALID_PARAM  If input parameters are invalid
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_RsaPrivateDecKat(XSecure_ClientInstance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters
	 *   are invalid
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XSECURE_PACK_PAYLOAD1(Payload, ((InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT)
				| XSECURE_API_KAT),
				XSECURE_API_RSA_PRIVATE_DEC_KAT);

	/**
	 * - Send request to PLM through generic request API.
	 *   This internally handles SMC or IPI mailbox based on build configuration.
	 */
	Status = XSecure_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to PLM to perform ECC sign generate KAT
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	CurveClass  Type of elliptic curve class(Prime - 0, Binary - 1)
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XST_INVALID_PARAM  If input parameters are invalid
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_EllipticSignGenKat(XSecure_ClientInstance *InstancePtr, XSecure_EccCrvClass CurveClass)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Perform input parameter validation on InstancePtr. Return XST_FAILURE if input parameters
	 *   are invalid
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XSECURE_PACK_PAYLOAD2(Payload, ((InstancePtr->SlrIndex << XSECURE_SLR_INDEX_SHIFT)
				| XSECURE_API_KAT),
				XSECURE_API_ELLIPTIC_SIGN_GEN_KAT,
				CurveClass);

	/**
	 * - Send request to PLM through generic request API.
	 *   This internally handles SMC or IPI mailbox based on build configuration.
	 */
	Status = XSecure_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}
/** @} */
