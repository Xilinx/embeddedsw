/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_mldsa.c
*
* This file contains the implementation of the interface functions for ML-DSA driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.7   tvp  02/19/26 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_mldsa_server_apis XilSecure MLDSA Server APIs
* @{
*/
/*************************************** Include Files ********************************************/
#include "xsecure_mldsa_hw.h"
#include "xsecure_mldsa.h"
#include "xsecure_error.h"
#include "xsecure_defs.h"
#include "xsecure_utils.h"

/************************************ Constant Definitions ****************************************/

/************************************ Variable Definitions ****************************************/

/************************************ Function Prototypes *****************************************/
static void XSecure_MldsaMsgTransferToCore(u64 DataAddr, u32 DataLen);

/************************************ Function Definitions ****************************************/

/**************************************************************************************************/
/**
 * @brief	This function transfers message/data to the ML-DSA hardware core in streaming mode.
 *
 * @param	DataAddr	64-bit address of the message/data to be transferred.
 * @param	DataLen		Length of the message/data in bytes.
 *
 * @return	None
 *
 **************************************************************************************************/
static void XSecure_MldsaMsgTransferToCore(u64 DataAddr, u32 DataLen)
{
	u32 Index;
	u32 LastWordVal = XSECURE_ZERO;
	u32 WordLen;
	u32 RemLen;
	u32 StrobeValue;

	/** - Transfer message/data to hardware registers */
	WordLen = DataLen / XSECURE_WORD_SIZE;
	RemLen = DataLen % XSECURE_WORD_SIZE;

	/** - Stream full words to hardware */
	StrobeValue = (u32)XSECURE_MLDSA_MSG_STROBE_DATA_MASK;
	XSecure_Out32((XSECURE_MLDSA_BASEADDR + XSECURE_MLDSA_MSG_STROBE_OFFSET), StrobeValue);
	for (Index = 0U; Index < WordLen; Index++) {
		XSecure_Out32((XSECURE_MLDSA_BASEADDR + XSECURE_MLDSA_MSG_0_OFFSET),
			       XSecure_In64(DataAddr + (u64)(Index * XSECURE_WORD_SIZE)));
	}

	/* Handle last partial word if present */
	if (RemLen > XSECURE_ZERO) {
		/**
		 * - If last message word is not 32-bit aligned, set strobe according to valid
		 *   bytes in the last message word
		 */
		LastWordVal = XSECURE_ZERO;
		for (Index = 0U; Index < RemLen; Index++) {
			LastWordVal |= ((u32)XSecure_InByte64(DataAddr + Index + DataLen - RemLen)
					<< (Index * XSECURE_BYTE_IN_BITS));
		}
		switch (RemLen) {
		case XSECURE_VALUE_THREE:
			StrobeValue = XSECURE_MLDSA_3_BYTE_STROBE_MASK;
			break;
		case XSECURE_VALUE_TWO:
			StrobeValue = XSECURE_MLDSA_2_BYTE_STROBE_MASK;
			break;
		case XSECURE_VALUE_ONE:
			StrobeValue = XSECURE_MLDSA_1_BYTE_STROBE_MASK;
			break;
		default:
			StrobeValue = XSECURE_MLDSA_MSG_STROBE_DATA_MASK;
			break;
		}
	} else {
		/** - If last message word is 32-bit aligned, indicate end with zero strobe */
		LastWordVal = XSECURE_ZERO;
		StrobeValue = XSECURE_MLDSA_0_BYTE_STROBE_MASK;
	}

	/** - Stream last partial message word to hardware */
	XSecure_Out32((XSECURE_MLDSA_BASEADDR + XSECURE_MLDSA_MSG_STROBE_OFFSET), StrobeValue);
	XSecure_Out32((XSECURE_MLDSA_BASEADDR + XSECURE_MLDSA_MSG_0_OFFSET), LastWordVal);
}

/**************************************************************************************************/
/**
 * @brief	This function verifies MLDSA signature for the given data using the provided public
 *		key and context.
 *
 * @param	MldsaParams	Pointer to MLDSA signature verification parameters instance.
 *
 * @return
 *		- XST_SUCCESS On successful verification.
 *		- Error code On failure.
 *
 **************************************************************************************************/
int XSecure_MldsaSignVerify(const XSecure_MldsaSignVerifyParams *MldsaParams)
{
	volatile int Status = XST_FAILURE;
	volatile int ClrStatus = XST_FAILURE;
	volatile u32 Index;
	u32 CtValue = XSECURE_ZERO;

	/* Validate MLDSA instance pointer */
	if (MldsaParams == NULL) {
		Status = XSECURE_MLDSA_INVALID_PARAM_ERROR;
		goto END;
	}

	if ((MldsaParams->PubKeyLen != XSECURE_MLDSA_PK_LEN) ||
	    (MldsaParams->SignLen != XSECURE_MLDSA_SIGN_LEN)) {
		Status = XSECURE_MLDSA_INVALID_PARAM_ERROR;
		goto END;
	}

	/* Validate ctx length */
	if (MldsaParams->ContextLen > (u32)XSECURE_MLDSA_MAX_CTX_LEN) {
		Status = XSECURE_MLDSA_INVALID_PARAM_ERROR;
		goto END;
	}

	/** Check that first 64 bytes of signature are not all zeros (CT validation) */
	for (Index = 0U; Index < (u32)XSECURE_MLDSA_CT_LEN; Index += XSECURE_WORD_SIZE) {
		CtValue |= XSecure_In64(MldsaParams->SignAddr + Index);
	}

	if ((CtValue == XSECURE_ZERO) || (Index != (u32)XSECURE_MLDSA_CT_LEN)) {
		Status = XSECURE_MLDSA_SIGN_CT_ZERO_ERROR;
		goto END;
	}

	XSecure_Printf(XSECURE_DEBUG_GENERAL, "MLDSA signature verification - CT Check Passed\r\n");

	/** - Release MLDSA from reset */
	XSecure_ReleaseReset(XSECURE_MLDSA_BASEADDR, XSECURE_MLDSA_RESET_OFFSET);

	/** - Wait for the core to be ready for processing inputs */
	Status = (int)Xil_WaitForEvent((XSECURE_MLDSA_BASEADDR + XSECURE_MLDSA_STATUS_OFFSET),
				  XSECURE_MLDSA_STATUS_READY_MASK, XSECURE_MLDSA_STATUS_READY_MASK,
				  XSECURE_MLDSA_TIMEOUT_MAX);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_MLDSA_CORE_NOT_READY_ERROR;
		goto END;
	}

	if (MldsaParams->ContextLen > XSECURE_ZERO) {
		/* Write context length to context config register */
		XSecure_Out32((XSECURE_MLDSA_BASEADDR + XSECURE_MLDSA_CTX_CONFIG_OFFSET),
			      MldsaParams->ContextLen);

		/** - Transfer context to hardware registers */
		Xil_MemCpy64((u64)(UINTPTR)(XSECURE_MLDSA_BASEADDR +
						XSECURE_MLDSA_CTX_0_OFFSET),
				 MldsaParams->ContextAddr, MldsaParams->ContextLen);
	}

	/** - Transfer public key to hardware registers */
	XSecure_Printf(XSECURE_DEBUG_GENERAL, "MLDSA signature verification - Writing PK\r\n");
	Xil_MemCpy64((u64)(UINTPTR)(XSECURE_MLDSA_BASEADDR + XSECURE_MLDSA_PK_0_OFFSET),
			 MldsaParams->PubKeyAddr, XSECURE_MLDSA_PK_LEN);

	/** - Transfer signature to hardware registers */
	XSecure_Printf(XSECURE_DEBUG_GENERAL, "MLDSA signature verification - "
					      "Writing Signature\r\n");
	/* Add 1 to the length to ensure word alignment for the signature transfer */
	Xil_MemCpy64((u64)(UINTPTR)(XSECURE_MLDSA_BASEADDR + XSECURE_MLDSA_SIGN_0_OFFSET),
			 MldsaParams->SignAddr, XSECURE_MLDSA_SIGN_LEN + XSECURE_VALUE_ONE);

	/** - Configure ML-DSA for signature verification with streaming message mode */
	XSecure_Printf(XSECURE_DEBUG_GENERAL,
		       "MLDSA signature verification - Initiating CMD for "
		       "signature verification with streaming message mode\r\n");
	XSecure_Out32((XSECURE_MLDSA_BASEADDR + XSECURE_MLDSA_CTRL_OFFSET),
		  (XSECURE_MLDSA_CTRL_SIGN_VERIFY_CMD | XSECURE_MLDSA_CTRL_STREAM_MSG_MASK));

	/** - Wait for the core to be ready to receive the message in streaming mode */
	Status = XST_FAILURE;
	Status = (int)Xil_WaitForEvent((XSECURE_MLDSA_BASEADDR + XSECURE_MLDSA_STATUS_OFFSET),
				  XSECURE_MLDSA_STATUS_MSG_STREAM_READY_MASK,
				  XSECURE_MLDSA_STATUS_MSG_STREAM_READY_MASK,
				  XSECURE_MLDSA_TIMEOUT_MAX);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_MLDSA_MSG_STREAM_ERROR;
		goto END;
	}

	/** - Transfer message/data to hardware registers */
	XSecure_MldsaMsgTransferToCore(MldsaParams->DataAddr, MldsaParams->DataLen);

	/** - Wait for signature verification operation to complete */
	Status = XST_FAILURE;
	Status = (int)Xil_WaitForEvent((XSECURE_MLDSA_BASEADDR + XSECURE_MLDSA_STATUS_OFFSET),
				  XSECURE_MLDSA_STATUS_VALID_MASK, XSECURE_MLDSA_STATUS_VALID_MASK,
				  XSECURE_MLDSA_TIMEOUT_MAX);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_MLDSA_SIGN_TIMEOUT_ERROR;
		goto END;
	}

	Status = XSECURE_MLDSA_SIGNATURE_MISMATCH;

	/** - Compare generated result with expected signature's CT component */
	for (Index = 0U; Index < (u32)(XSECURE_MLDSA_CT_LEN); Index += XSECURE_WORD_SIZE) {
		if (XSecure_In64(XSECURE_MLDSA_BASEADDR + XSECURE_MLDSA_VERIF_RESULT_0_OFFSET +
					Index) != XSecure_In64(MldsaParams->SignAddr + Index)) {
			Status = XSECURE_MLDSA_SIGNATURE_MISMATCH;
			goto END;
		}
	}

	if (Index != (u32)(XSECURE_MLDSA_CT_LEN)) {
		Status = XSECURE_ERR_GLITCH_DETECTED;
		goto END;
	}

	Status = XST_SUCCESS;

	XSecure_Printf(XSECURE_DEBUG_GENERAL, "MLDSA signature verification success\r\n");

END:
	/**
	 * - Clear ML-DSA control registers (using ZEROIZE command) to remove sensitive data from
	 *   hardware for security
	 */
	XSecure_Out32((XSECURE_MLDSA_BASEADDR + XSECURE_MLDSA_CTRL_OFFSET),
		  XSECURE_MLDSA_CTRL_ZEROIZE_MASK);

	/** - Wait for zeroize operation to complete */
	ClrStatus = (int)Xil_WaitForEvent((XSECURE_MLDSA_BASEADDR + XSECURE_MLDSA_STATUS_OFFSET),
				     XSECURE_MLDSA_STATUS_READY_MASK,
				     XSECURE_MLDSA_STATUS_READY_MASK, XSECURE_MLDSA_TIMEOUT_MAX);
	if (Status == XST_SUCCESS) {
		Status = ClrStatus;
	}

	XSecure_ReleaseReset(XSECURE_MLDSA_BASEADDR, XSECURE_MLDSA_RESET_OFFSET);

	return Status;
}
/** @} */
