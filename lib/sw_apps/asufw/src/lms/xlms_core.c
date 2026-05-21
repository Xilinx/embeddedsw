/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xlms_core.c
*
* This module provides the ASU firmware implementation of LMS/HSS signature verification.
* It owns the following responsibilities:
*	- Managing shared scratch buffers that are reused across LMS layers to reduce stack usage.
*	- Providing the XLms_SignatureVerification primitive that authenticates a single LMS level
*	  (OTS verification, Merkle traversal, and root comparison).
*	- Stitching multiple LMS levels together inside the HSS helpers (XLms_HssInit/XLms_HssFinish)
*	  so that higher-level public keys are authenticated before the final payload is checked.
*	- Maintaining context such as the currently authenticated public key, q/I/C pre-digest
*	  fields and signature progress so that HSS init/hash/finish calls can be sequenced with
*	  minimal data copies.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ss   01/07/26  Initial release
* 1.1   ss   03/21/26  Added non-blocking DMA support for LMS/HSS signature verification
*
* </pre>
* @note
*
**************************************************************************************************/
/**
* @addtogroup xlms_server_apis LMS Server APIs
* @{
*/
/***************************** Include Files *****************************************************/
#include "xil_io.h"
#include "xil_util.h"
#include "xsha.h"
#include "xlms_core.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasu_lms_common.h"
#include "xasufw_perf.h"

#ifdef XASU_LMS_ENABLE
/************************** Constant Definitions *************************************************/
#define XLMS_LOCAL_SIGNATURE_BUFF_SIZE (XLMS_OTS_MAX_SIGN_LEN + XLMS_Q_FIELD_SIZE + \
				XLMS_TYPE_SIZE + (XLMS_M_BYTE_FIELD_SIZE * XLMS_TREE_HEIGHT_25))
				/**< Size of local signature buffer used during LMS signature verification */
#define XLMS_HSS_LOCAL_SIGNATURE_BUFF_SIZE (XLMS_OTS_MAX_SIGN_LEN + XLMS_Q_FIELD_SIZE + \
			XLMS_TYPE_SIZE + (XLMS_M_BYTE_FIELD_SIZE * XLMS_TREE_HEIGHT_25) + \
			XLMS_PUB_KEY_TOTAL_SIZE + XLMS_HSS_LEVELS_FIELD_SIZE + \
			XLMS_OTS_MAX_SIGN_LEN + XLMS_Q_FIELD_SIZE + \
			XLMS_TYPE_SIZE + (XLMS_M_BYTE_FIELD_SIZE * XLMS_TREE_HEIGHT_25))
			/**< Size of local signature buffer used during HSS signature verification */

#define XLMS_DMA_STATE_IDLE			(0U) /**< No non-blocking DMA pending */
#define XLMS_DMA_STATE_PENDING			(1U) /**< Non-blocking DMA transfer in progress */

#define XLMS_SIGN_VERIFY_STATE_VALIDATE_INPUTS	(1U) /**< ValidateInputs DMA pending (DST) */
#define XLMS_SIGN_VERIFY_STATE_OTS_PROCESS	(2U) /**< OtsSignatureProcess DMA pending (SRC) */

#define XLMS_OTS_PROCESS_STATE_HASH_MESSAGE	(1U) /**< HashMessage DMA pending (SRC) */
#define XLMS_OTS_PROCESS_STATE_FINAL_DIGEST	(2U) /**< Final XSha_Digest DMA pending (SRC) */

#define XLMS_HSS_INIT_STATE_SIG_COPY		(1U) /**< Signature DMA copy in progress (DST) */
#define XLMS_HSS_INIT_STATE_SIGN_VERIFY		(2U) /**< Inner SignatureVerification DMA pending */

#define XLMS_HSS_FINISH_STATE_SIG_COPY		(1U) /**< Signature DMA copy in progress (DST) */
#define XLMS_HSS_FINISH_STATE_SIGN_VERIFY	(2U) /**< Inner SignatureVerification DMA pending */

/**************************** Type Definitions ***************************************************/

/**
 * @brief Context structure for LMS signature verification
 *
 * Holds validated parameters extracted during input validation phase.
 */
typedef struct {
	XLms_Type PubKeyLmsType;	/**< Public key LMS type */
	XLms_OtsType PubKeyLmsOtsType;	/**< Public key LMS OTS type */
	const XLms_Param* LmsPubKeyParam;	/**< Pointer to public key LMS parameters */
	const XLms_OtsParam* LmsOtsSignParam;	/**< Pointer to LMS OTS signature parameters */
	const XLms_Param* LmsSignParam;	/**< Pointer to LMS signature parameters */
} XLms_SignVerifyCtx;

/***************** Macros (Inline Functions) Definitions *****************************************/

/************************** Function Prototypes **************************************************/
static s32 XLms_OtsSignatureInit(u8* LmsOtsSignatureBuff, u32 LmsOtsSignatureLen,
				XLms_OtsType LmsOtsExpectedType);
static s32 XLms_OtsSignatureProcess(XSha *ShaInstPtr, XAsufw_Dma *DmaPtr,
	const XAsu_LmsHssSignVerifyParams *LmsSignVerifyParams, u8* OtsPubKeyCandidate);
static s32 XLms_ValidateInputs(XAsufw_Dma *DmaPtr,
		const XAsu_LmsHssSignVerifyParams *LmsSignVerifyParams, XLms_SignVerifyCtx *CtxPtr);
static s32 XLms_ComputeMerkleRoot(XSha *ShaInstPtr, XAsufw_Dma *DmaPtr,
	const XLms_SignVerifyCtx *CtxPtr, const u8 *Path, u32 StartNodeNum);

/************************** Variable Definitions *************************************************/
static volatile XLms_PublicKey AuthenticatedKey; /**< Latest authenticated HSS/LMS public key */
static XLms_DataDigestFixedFields DigestPrefixFields; /**< Cached I/q/C/D_MESG fields used when
							hashing messages */
static volatile u32 SignatureLengthConsumed = 0U; /**< Tracks bytes of HSS signature already
							processed during init */
static XLms_DataDigest DigestChecksum; /**< Holds message digest plus Winternitz checksum for OTS
						processing */
static u32 CurrentLmsQ = 0U; /**< Current LMS leaf index extracted from signature */
static u8* LmsOtsSignatureBuff_g = NULL; /**< Static pointer to OTS signature buffer */
static const XLms_OtsParam* LmsOtsSignParam_g = NULL; /**< Static pointer to OTS parameters */
static u32 HashOutputN_g = XLMS_N_FIELD_SIZE; /**< Runtime hash output bytes (n/m) for current operation */
static u8 HssCallerShaMode_g = 0U; /**< Caller-specified SHA mode from HssInit, used by HssFinish */
static u8 HssCallerShaType_g = 0U; /**< Caller-specified SHA type from HssInit, used by HssFinish */
static u32 OtsProcessDmaState = XLMS_DMA_STATE_IDLE; /**< State variable to track non-blocking
							DMA progress in XLms_OtsSignatureProcess */
/** Shared buffers for LMS signature verification */
static u8 LmsLocalPubKeyBuff[XLMS_PUB_KEY_TOTAL_SIZE]; /**< Local copy of public key */
static u8 LmsLocalSignatureBuff[XLMS_LOCAL_SIGNATURE_BUFF_SIZE]; /**< Contains current LMS/HSS
									signature */
static u8 LmsTmpBuff[XLMS_M_BYTE_FIELD_SIZE]; /**< Scratch buffer used for intermediate hashes
							(leaf/path computations) */
static XLms_PubKeyTmp LmsPubKeyTmpBuff; /**< Structured view over leaf/internal node hash input
						layout */

/************************** Function Definitions *************************************************/

/*************************************************************************************************/
/**
* @brief	This function performs LMS level signature verification
*
* @param	ShaInstPtr	Pointer to SHA instance
* @param	DmaPtr		Pointer to DMA instance
* @param	LmsSignVerifyParams	Pointer to LMS signature verification parameters
*
* @return
*		- XASUFW_SUCCESS - LMS signature verified successfully
*		- XASUFW_LMS_INVALID_PARAM - Invalid pointers, message lengths, or addresses provided
*		- XASUFW_DMA_COPY_FAIL - DMA transfer for public key, signature, or message failed
*		- XASUFW_MEM_COPY_FAIL - Local buffer copy failed integrity checks
*		- XASUFW_LMS_PUB_KEY_UNSUPPORTED_TYPE_1_ERROR - LMS public key type is unsupported
*		- XASUFW_LMS_SIGN_EXPECTED_PUB_KEY_LEN_2_ERROR - Public key length mismatches
								expected size
*		- XASUFW_LMS_SIGN_LEN_1_ERROR - Signature too short to contain LMS header
*		- XASUFW_LMS_SIGN_LEN_2_ERROR - Signature lacks the full LMS OTS section
*		- XASUFW_LMS_OTS_PUB_KEY_LMS_OTS_SIGN_TYPE_MISMATCH_ERROR - LMS OTS types mismatch
								between signature and public key
*		- XASUFW_LMS_SIGN_UNSUPPORTED_OTS_TYPE_1_ERROR - LMS OTS signature type unsupported
*		- XASUFW_LMS_PUB_KEY_LMS_SIGN_TYPE_MISMATCH_ERROR - LMS signature type mismatches
								public key type
*		- XASUFW_LMS_SIGN_UNSUPPORTED_TYPE_1_ERROR - LMS signature type unsupported
*		- XASUFW_LMS_SIGN_INVALID_NODE_NUMBER_ERROR - Signature references a node outside
								the tree range
*		- XASUFW_LMS_SIGN_LEN_3_ERROR - Signature length mismatch after accounting for
						authentication path
*		- XASUFW_LMS_SIGN_OTS_OP_ERROR - LMS OTS initialization or processing failed
*		- XASUFW_LMS_SIGN_VERIF_SHA_DIGEST_LEAF_FAILED_ERROR - SHA digest failure while
								processing a leaf node
*		- XASUFW_LMS_PUB_KEY_AUTHENTICATION_FAILED_ERROR - Calculated root does not match
								the public key
*		- XASUFW_LMS_PUB_KEY_AUTHENTICATION_GLITCH_ERROR - Root comparison glitch detected
*		- XASUFW_CMD_IN_PROGRESS - Non-blocking DMA transfer started for large signature or message
*
**************************************************************************************************/
s32 XLms_SignatureVerification(XSha *ShaInstPtr, XAsufw_Dma *DmaPtr,
	const XAsu_LmsHssSignVerifyParams *LmsSignVerifyParams)
{
	/**
	 * Capture the start time of the LMS signature verification operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_LMS_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	static XLms_SignVerifyCtx Ctx;
	const u8* Path = NULL;
	u32 CurrentNodeNum = 0U;
	static const u8 XLMS_D_LEAF[XLMS_D_FIELD_SIZE] = {XLMS_D_LEAF_BYTE, XLMS_D_LEAF_BYTE};
	static u8 ExpectedPublicKey[XLMS_OTS_PUB_KEY_K_FIELD_SIZE];
	XAsu_ShaOperationCmd ShaParamsInput;
	/** Static state variable to track non-blocking DMA progress across function calls */
	static u32 SignVerifyDmaState = XLMS_DMA_STATE_IDLE;
	u32 ResumeState = SignVerifyDmaState;

	/** Validate input pointers. */
	if ((NULL == ShaInstPtr) || (NULL == DmaPtr)) {
		Status = XASUFW_LMS_INVALID_PARAM;
		goto END;
	}

	/** Validate LMS parameters on initial call. */
	if (SignVerifyDmaState == XLMS_DMA_STATE_IDLE) {
		Status = XAsu_LmsValidateParams(LmsSignVerifyParams);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_LMS_INVALID_PARAM;
			goto END;
		}
	}

	/**
	 * Handle non-blocking DMA resume and initial call paths via switch.
	 * VALIDATE_INPUTS: Resume ValidateInputs after DMA, then continue to OTS init + process.
	 * OTS_PROCESS: Resume OtsSignatureProcess after DMA, skip ValidateInputs and OTS init.
	 * IDLE: Full path - ValidateInputs, OTS init, OTS process.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	switch (SignVerifyDmaState) {
	case XLMS_DMA_STATE_IDLE:
	case XLMS_SIGN_VERIFY_STATE_VALIDATE_INPUTS:
		/**
		 * Step 1: Validate all inputs and copy public key/signature to local buffers.
		 * This populates the context structure with validated LMS and OTS parameters.
		 * XLms_ValidateInputs may return CMD_IN_PROGRESS for large signature DMA.
		 */
		/* Resume after ValidateInputs DMA completion - call again to complete */
		if (SignVerifyDmaState == XLMS_SIGN_VERIFY_STATE_VALIDATE_INPUTS) {
			SignVerifyDmaState = XLMS_DMA_STATE_IDLE;
		}
		Status = XLms_ValidateInputs(DmaPtr, LmsSignVerifyParams, &Ctx);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			/* DMA still in progress (shouldn't happen on resume, but handle it) */
			SignVerifyDmaState = XLMS_SIGN_VERIFY_STATE_VALIDATE_INPUTS;
		}
		break;

	case XLMS_SIGN_VERIFY_STATE_OTS_PROCESS:
		/* Resume after OtsSignatureProcess DMA completion - call again to complete */
		SignVerifyDmaState = XLMS_DMA_STATE_IDLE;
		Status = XLms_OtsSignatureProcess(ShaInstPtr, DmaPtr, LmsSignVerifyParams,
						(u8* const)ExpectedPublicKey);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			/* DMA still in progress, continue waiting */
			SignVerifyDmaState = XLMS_SIGN_VERIFY_STATE_OTS_PROCESS;
		}
		break;

	default:
		Status = XASUFW_LMS_INVALID_PARAM;
		goto END;
	}

	/**
	 * Check ValidateInputs result (applies to both IDLE and VALIDATE_INPUTS resume paths).
	 * Skip when resuming from OTS_PROCESS since ValidateInputs was already completed.
	 */
	if ((Status == XASUFW_SUCCESS) &&
		(ResumeState != XLMS_SIGN_VERIFY_STATE_OTS_PROCESS)) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		/**
		 * Verify SHA algorithm consistency: LMS and OTS types must use the same hash,
		 * and the caller-selected SHA mode must match.
		 */
		if ((Ctx.LmsPubKeyParam->HashAlgId != Ctx.LmsOtsSignParam->HashAlgId) ||
			(LmsSignVerifyParams->ShaMode != Ctx.LmsOtsSignParam->HashAlgId) ||
			(Ctx.LmsPubKeyParam->HashOutputBytes != Ctx.LmsOtsSignParam->HashOutputBytes)) {
			Status = XASUFW_LMS_SIGN_VERIFY_BH_AND_TYPE_SHA_ALGO_MISMATCH_L0_ERROR;
			goto END;
		}

		/**
		 * Step 2: Initialize OTS signature verification.
		 * Validates OTS type in signature matches public key and looks up parameters.
		 */
		Status = XLms_OtsSignatureInit(&LmsLocalSignatureBuff[XLMS_Q_FIELD_SIZE],
						Ctx.LmsOtsSignParam->SignLen, Ctx.PubKeyLmsOtsType);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_LMS_SIGN_OTS_OP_ERROR);
			goto END;
		}

		/**
		 * Step 3: Process OTS signature to compute candidate public key.
		 * This involves computing message digest and performing hash chain operations
		 * on each signature element to recover the expected OTS public key.
		 * XLms_OtsSignatureProcess may return CMD_IN_PROGRESS for:
		 * - Large messages when PreHashedMsg == FALSE (via XLms_HashMessage)
		 * - Large p values (e.g., w=1, p=265) when computing final OTS hash (via XSha_Digest)
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XLms_OtsSignatureProcess(ShaInstPtr, DmaPtr, LmsSignVerifyParams,
						(u8* const)ExpectedPublicKey);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			/* Non-blocking DMA in progress, set state for resume on next call */
			SignVerifyDmaState = XLMS_SIGN_VERIFY_STATE_OTS_PROCESS;
		}
	} else if ((Status != XASUFW_SUCCESS) &&
		(Status != XASUFW_CMD_IN_PROGRESS) &&
		(ResumeState != XLMS_SIGN_VERIFY_STATE_OTS_PROCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_LMS_SIGN_INPUT_VALIDATION_FAILED);
		goto END;
	} else {
		/* Do nothing */
	}

	if ((Status != XASUFW_SUCCESS) && (Status != XASUFW_CMD_IN_PROGRESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_LMS_SIGN_OTS_OP_ERROR);
		goto END;
	}

	if (Status == XASUFW_SUCCESS) {
		/**
		 * Step 4: Extract authentication path from signature.
		 * The auth path contains h sibling hashes needed for Merkle tree traversal.
		 */
		Path = (const u8*)&LmsLocalSignatureBuff[(Ctx.LmsOtsSignParam->SignLen + XLMS_Q_FIELD_SIZE
							+ XLMS_TYPE_SIZE)];

		/**
		 * Step 5: Copy unique key identifier 'I' from public key to temp buffer.
		 * 'I' is included in every hash computation during tree traversal.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCpy((void*)LmsPubKeyTmpBuff.Fields.MerkleTreeId, XLMS_I_FIELD_SIZE,
			(void*)&LmsLocalPubKeyBuff[XLMS_PUB_KEY_I_OFFSET], XLMS_I_FIELD_SIZE,
			XLMS_I_FIELD_SIZE);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_COPY_FAIL;
			goto END;
		}

		/**
		 * Step 6: Compute starting leaf node number.
		 * Leaf nodes are numbered from 2^h to (2^(h+1) - 1) in the tree.
		 * node_num = 2^h + q, where q is the leaf index from signature.
		 */
		CurrentNodeNum = (((u32)XASUFW_VALUE_ONE << (u32)Ctx.LmsSignParam->TreeHeight) + CurrentLmsQ);

		/**
		 * Step 7: Prepare buffer for leaf node hash computation.
		 * Leaf hash = H(I || node_num || D_LEAF || OTS_PubKey_candidate)
		 */
		LmsPubKeyTmpBuff.Fields.half_node_number = Xil_Htonl(CurrentNodeNum);
		LmsPubKeyTmpBuff.Fields.D[XASUFW_BUFFER_INDEX_ZERO] = XLMS_D_LEAF[XASUFW_BUFFER_INDEX_ZERO];
		LmsPubKeyTmpBuff.Fields.D[XASUFW_BUFFER_INDEX_ONE] = XLMS_D_LEAF[XASUFW_BUFFER_INDEX_ONE];

		/** Copy computed OTS public key candidate (Kc) to hash input buffer */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCpy((void*)&LmsPubKeyTmpBuff.Fields.Tmp[0U], XLMS_OTS_PUB_KEY_K_FIELD_SIZE,
			(void*)&ExpectedPublicKey[0U], Ctx.LmsOtsSignParam->HashOutputBytes,
			Ctx.LmsOtsSignParam->HashOutputBytes);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_COPY_FAIL;
			goto END;
		}

		/**
		 * Step 8: Compute leaf node hash.
		 * Result is stored in LmsTmpBuff and will be combined with auth path siblings.
		 */
		ShaParamsInput.DataAddr = (u64)(UINTPTR)&LmsPubKeyTmpBuff.Buff[0U];
		ShaParamsInput.HashAddr = (u64)(UINTPTR)LmsTmpBuff;
		ShaParamsInput.DataSize = (XLMS_I_FIELD_SIZE + XLMS_MERKLE_TREE_NODE_NUMBER_SIZE +
						XLMS_D_LEAF_FIELD_SIZE + Ctx.LmsOtsSignParam->HashOutputBytes);
		ShaParamsInput.HashBufSize = XLMS_M_BYTE_FIELD_SIZE;
		ShaParamsInput.ShaMode = (u8)Ctx.LmsOtsSignParam->HashAlgId;

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Digest(ShaInstPtr, DmaPtr, &ShaParamsInput);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_LMS_SIGN_VERIF_SHA_DIGEST_LEAF_FAILED_ERROR;
			goto END;
		}

		/**
		 * Step 9: Traverse Merkle tree from leaf to root and verify.
		 * Combines computed hashes with auth path siblings at each level,
		 * then compares final root hash with public key's T[1] field.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XLms_ComputeMerkleRoot(ShaInstPtr, DmaPtr, &Ctx, Path, CurrentNodeNum);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
						XASUFW_LMS_PUB_KEY_AUTHENTICATION_FAILED_ERROR);
			goto END;
		}

		/* Additional status if signature verification is successful without any glitch */
		ReturnStatus = XASUFW_LMS_SIGNATURE_VERIFIED;
	}

	/**
	 * Measure and print the performance time for the LMS signature verification operation,
	 * if performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_LMS_ID);

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function Initiates LMS Signature verification process.
* 		Completes all upper level Merkle trees verification and prepares
* 		for data's Digest calculation
*
* @param	ShaInstPtr	Pointer to SHA instance
* @param	DmaPtr		Pointer to DMA instance
* @param	HssParamsPtr	Pointer to HSS signature verification parameters
*
* @return
*		- XASUFW_SUCCESS - HSS context prepared successfully
*		- XASUFW_LMS_INVALID_PARAM - Input pointers, lengths, or addresses are invalid
*		- XASUFW_DMA_COPY_FAIL - DMA transfer of public key or signature failed
*		- XASUFW_ZEROIZE_MEMSET_FAIL - Secure zeroization of cached keys failed
*		- XASUFW_LMS_HSS_PUB_KEY_INVALID_LEN_2_ERROR - HSS public key length mismatch
*		- XASUFW_LMS_HSS_SIGN_LEVEL_UNSUPPORTED_ERROR - Unsupported number of HSS levels
								requested
*		- XASUFW_LMS_HSS_SIGN_PUB_KEY_LEVEL_MISMATCH_ERROR - Level count mismatch between
									signature and public key
*		- XASUFW_LMS_HSS_L0_PUB_KEY_LMS_TYPE_UNSUPPORTED_ERROR - LMS type in lower level
									public key unsupported
*		- XASUFW_LMS_HSS_L0_PUB_KEY_LMS_OTS_TYPE_UNSUPPORTED_ERROR - LMS OTS type in lower
									level public key unsupported
*		- XASUFW_LMS_SIGN_VERIFY_BH_AND_TYPE_SHA_ALGO_MISMATCH_L0_ERROR - Digest algorithm
								mismatch across BH/signature selections
*		- XASUFW_LMS_HSS_L0_SIGN_INVALID_LEN_2_ERROR - HSS level-0 signature does not contain
								required data
*		- XASUFW_LMS_HSS_L0_PUB_KEY_AUTH_FAILED_ERROR - Lower level public key authentication
								failed
*		- XASUFW_MEM_COPY_FAIL - Local buffer copies failed integrity checks
*		- XASUFW_LMS_PUB_OP_FAILED_ERROR - Authenticated key copyback verification failed
*		- XASUFW_LMS_PUB_OP_FAILED_1_ERROR - Authenticated key size verification failed
*		- XASUFW_LMS_HSS_L1_PUB_KEY_LMS_TYPE_1_UNSUPPORTED_ERROR - Upper level LMS type
							unsupported when preparing digest context
*		- XASUFW_LMS_HSS_L1_PUB_KEY_LMS_OTS_TYPE_UNSUPPORTED_ERROR - Upper level LMS OTS type
							unsupported when preparing digest context
*		- XASUFW_CMD_IN_PROGRESS - Non-blocking DMA transfer started for large signature
**************************************************************************************************/
s32 XLms_HssInit(XSha *ShaInstPtr, XAsufw_Dma *DmaPtr,
		const XAsu_LmsHssSignVerifyParams *HssParamsPtr)
{
	/**
	 * Capture the start time of the HSS/LMS initialization operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_LMS_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XLms_Type PublicKeyLmsType = XLMS_NOT_SUPPORTED;
	XLms_OtsType PublicKeyLmsOtsType = XLMS_OTS_NOT_SUPPORTED;
	static const u8* TmpPublicKeyPtr = NULL;
	const XLms_Param* PubKeyLmsParam = NULL;
	const XLms_OtsParam* PubKeyLmsOtsParam = NULL;
	static u32 TotalLevelsInHssPublicKey = XASUFW_ALLFS;
	static const u8* TmpSignaturePtr = NULL;
	static u32 CurrentLevelSignLen = 0U;
	static u32 CurrentPubKeyActualSize = 0U;
	static u32 Index = 0U;
	static u32 TotalLevelsInSignature = 0U;
	static XAsu_LmsHssSignVerifyParams LmsSignVerifyParams;
	static u8 HssLocalSignBuff[XLMS_HSS_LOCAL_SIGNATURE_BUFF_SIZE];
	static u8 HssLocalPubKeyBuff[XLMS_HSS_PUBLIC_KEY_TOTAL_SIZE];
	/** Static state variable to track non-blocking DMA progress */
	static u32 HssInitDmaState = XLMS_DMA_STATE_IDLE;
	u32 ResumeState = HssInitDmaState;
	const u8 *SignBuff = NULL;
	const u8 *PublicKey = NULL;

	/** Validate input pointers. */
	if ((NULL == ShaInstPtr) || (NULL == DmaPtr)) {
		Status = XASUFW_LMS_INVALID_PARAM;
		goto END;
	}

	/** Validate LMS parameters and addresses/lengths on initial call. */
	if (HssInitDmaState == XLMS_DMA_STATE_IDLE) {
		Status = XAsu_LmsValidateParams(HssParamsPtr);
		if (Status != XST_SUCCESS) {
			Status = XASUFW_LMS_INVALID_PARAM;
			goto END;
		}
		if (HssParamsPtr->SignatureLen > XLMS_HSS_LOCAL_SIGNATURE_BUFF_SIZE) {
			Status = XASUFW_LMS_INVALID_PARAM;
			goto END;
		}
	}

	/**
	 * Handle non-blocking DMA resume and initial call paths via switch.
	 * SIG_COPY: resume after signature DMA transfer, continue to signature processing.
	 * SIGN_VERIFY: resume after inner SignatureVerification DMA, advance loop and continue.
	 * IDLE: Full path - validate, DMA copy, then process signatures.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	switch (HssInitDmaState) {
	case XLMS_HSS_INIT_STATE_SIG_COPY:
		HssInitDmaState = XLMS_DMA_STATE_IDLE;
		Status = XASUFW_SUCCESS;
		Index = 0U;
		break;

	case XLMS_HSS_INIT_STATE_SIGN_VERIFY:
		/* Resume after inner SignatureVerification DMA completion */
		HssInitDmaState = XLMS_DMA_STATE_IDLE;
		Status = XLms_SignatureVerification(ShaInstPtr, DmaPtr, &LmsSignVerifyParams);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			/* Still more DMA operations pending in inner verification */
			HssInitDmaState = XLMS_HSS_INIT_STATE_SIGN_VERIFY;
		} else if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
						XASUFW_LMS_HSS_L0_PUB_KEY_AUTH_FAILED_ERROR);
			goto END;
		} else {
			/** Skip sig[i] + pub[i+1] to process next level in the HSS chain */
			TmpPublicKeyPtr = (TmpSignaturePtr + CurrentLevelSignLen);
			TmpSignaturePtr += (CurrentLevelSignLen + CurrentPubKeyActualSize);
			SignatureLengthConsumed += (CurrentLevelSignLen + CurrentPubKeyActualSize);
			Index++;
		}
		break;

	case XLMS_DMA_STATE_IDLE:
		CurrentLmsQ = XASUFW_ALLFS;
		SignatureLengthConsumed = 0U;

		/** Validate that the public key buffer is long enough to extract L and LMS type */
		if ((XLMS_HSS_LEVELS_FIELD_SIZE + XLMS_TYPE_SIZE) > HssParamsPtr->LmsHssKeyObj.PubKeyLen) {
			Status = XASUFW_LMS_INVALID_PARAM;
			goto END;
		}

		/** Copy public key provided by user to local buffer since 64-bit address space is not
			accessible to ASU processor */
		Status = XAsufw_DmaXfr(DmaPtr, HssParamsPtr->LmsHssKeyObj.PubKeyAddr,
			(u64)(UINTPTR)HssLocalPubKeyBuff, HssParamsPtr->LmsHssKeyObj.PubKeyLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}
		PublicKey = HssLocalPubKeyBuff;

		/**
		 * RFC 8554 Section 6: HSS public key format is L || pub[0]
		 * Extract L (number of levels) from first 4 bytes - convert from BE to LE.
		 */
		TotalLevelsInHssPublicKey = XAsufw_SwapBytes(&PublicKey[XLMS_HSS_PUBLIC_KEY_LEVEL_FIELD_OFFSET],
					XLMS_HSS_LEVELS_FIELD_SIZE);

		TmpPublicKeyPtr = (const u8*)&PublicKey[XLMS_HSS_PUBLIC_KEY_LMS_FIELD_OFFSET];

		/** Extract LMS type from pub[0] to determine dynamic public key size */
		PublicKeyLmsType = (XLms_Type)XAsufw_SwapBytes(
				&TmpPublicKeyPtr[XLMS_PUB_KEY_TYPE_OFFSET], XLMS_TYPE_SIZE);
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XLms_LookupParamSet(PublicKeyLmsType, &PubKeyLmsParam);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_LMS_HSS_PUB_KEY_INVALID_LEN_2_ERROR;
			goto END;
		}
		CurrentPubKeyActualSize = XLMS_PUB_KEY_FIXED_FIELD_SIZE + PubKeyLmsParam->HashOutputBytes;

		/** excluding levels field, length should match the Length of LMS public key of that size */
		if ((XLMS_HSS_LEVELS_FIELD_SIZE + CurrentPubKeyActualSize) != HssParamsPtr->LmsHssKeyObj.PubKeyLen) {
			Status = XASUFW_LMS_HSS_PUB_KEY_INVALID_LEN_2_ERROR;
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SecureZeroize((u8 *)(UINTPTR)&AuthenticatedKey.Buff[0U], sizeof(XLms_PublicKey));
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_ZEROIZE_MEMSET_FAIL;
			goto END;
		}

		/* Signature processing - sequence. */

		/* Length should have at least levels mentioned */
		if (XLMS_HSS_LEVELS_FIELD_SIZE > HssParamsPtr->SignatureLen) {
			Status = XASUFW_LMS_INVALID_PARAM;
			goto END;
		}

		/**
		 * DMA copy signature to local buffer since 64-bit addresses may not be directly accessible.
		 * For large signatures (> XASUFW_DMA_BLOCKING_SIZE), use non-blocking DMA transfer to avoid
		 * blocking the processor during HSS signature transfers which can exceed 4KB.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		if (HssParamsPtr->SignatureLen > XASUFW_DMA_BLOCKING_SIZE) {
			/* Use non-blocking DMA for large signature transfers */
			Status = XAsufw_StartDmaXfr(DmaPtr, HssParamsPtr->SignatureAddr,
						(u64)(UINTPTR)HssLocalSignBuff,
						HssParamsPtr->SignatureLen, 0U);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_DMA_COPY_FAIL;
				goto END;
			}
			/* Set state to indicate DMA in progress */
			HssInitDmaState = XLMS_HSS_INIT_STATE_SIG_COPY;
			Status = XASUFW_CMD_IN_PROGRESS;
		} else {
			/* Use blocking DMA for small signature transfers */
			Status = XAsufw_DmaXfr(DmaPtr, HssParamsPtr->SignatureAddr,
						(u64)(UINTPTR)HssLocalSignBuff,
						HssParamsPtr->SignatureLen, 0U);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_DMA_COPY_FAIL;
				goto END;
			}
			Index = 0U;
		}
		break;

	default:
		Status = XASUFW_LMS_INVALID_PARAM;
		goto END;
	}

	/**
	 * Signature processing setup - skipped when resuming from SIGN_VERIFY state
	 * since the for loop variables are already populated.
	 * Also skipped when CMD_IN_PROGRESS (DMA pending from SIG_COPY or SIGN_VERIFY).
	 */
	if ((Status == XASUFW_SUCCESS) &&
		(ResumeState != XLMS_HSS_INIT_STATE_SIGN_VERIFY)) {
		SignBuff = HssLocalSignBuff;

		/* Initialize pointer to local signature buffer for sequential extraction of HSS multi-level
			signature components */
		TmpSignaturePtr = (const u8*)SignBuff;

		/**
		 * RFC 8554 Section 6.2: HSS signature format is u32str(Nspk) || signed_pub_key[0..Nspk-1] || sig[Nspk].
		 * Nspk = L - 1 (number of signed public keys). Extract and add 1 to get total levels.
		 */
		TotalLevelsInSignature = (XAsufw_SwapBytes(&SignBuff[XLMS_HSS_SIGN_LIST_LEVEL_FIELD_OFFSET],
				XLMS_HSS_LEVELS_FIELD_SIZE) + XASUFW_VALUE_ONE);

		/* Increment pointer to skip Nspk field (4 bytes) and track consumed bytes */
		TmpSignaturePtr += XLMS_HSS_LEVELS_FIELD_SIZE;
		SignatureLengthConsumed += XLMS_HSS_LEVELS_FIELD_SIZE;

		/** RFC 8554 Section 9: L must be in range [1, 8]. */
		if ((TotalLevelsInSignature < XLMS_HSS_MIN_LEVELS_SUPPORTED)
			|| (TotalLevelsInSignature > XLMS_HSS_MAX_LEVELS_SUPPORTED)) {
			Status = XASUFW_LMS_HSS_SIGN_LEVEL_UNSUPPORTED_ERROR;
			goto END;
		}

		/* Verify signature L matches public key L - RFC 8554 Section 6.3 Step 1 */
		if (TotalLevelsInSignature != TotalLevelsInHssPublicKey) {
			Status = XASUFW_LMS_HSS_SIGN_PUB_KEY_LEVEL_MISMATCH_ERROR;
			goto END;
		}
	}

	/**
	 * Now we have to parse through 0 to (Levels - 1) and verify the public keys
	 * using signatures at each level, but ASUFW only supports 2 levels, so need to run the
	 * verification only once
	 */
	/**
	 * Signature structure
	 * u32str(Nspk) || signed_pub_key[0] || ... || signed_pub_key[Nspk-1] || sig[Nspk]
	 * u32str(Nspk) || sig[0] || pub[1] || ... || sig[Nspk-1] || pub[Nspk] || sig[Nspk]
	 *
	 * Public key of lower level is used as a message to be authenticated, and signature of that
	 * level to back calculate the public key at that level. if this is happening for top most
	 * tree then value will be HSS public key, in turn all the levels above
	 * [levels - 1] are verified, so when the actual message is verified using lowest tree's
	 * leaf node, as the public key for that tree (lowest) is already verified, message is
	 * authenticated.
	 *
	 * ASUFW Supports two levels trees, level-0 & 1 namely.
	 */

	for (; (Status == XASUFW_SUCCESS) && (Index < (TotalLevelsInSignature - 1U)); Index++) {
		/**
		 * RFC 8554 Section 6.3 Step 2: Extract LMS type and OTS type from current level's
		 * public key. Types are stored in BE - convert to LE for parameter lookup.
		 */
		PublicKeyLmsType = (XLms_Type)XAsufw_SwapBytes(&TmpPublicKeyPtr[XLMS_PUB_KEY_TYPE_OFFSET],
								XLMS_TYPE_SIZE);
		PublicKeyLmsOtsType = (XLms_OtsType)XAsufw_SwapBytes(
				&TmpPublicKeyPtr[XLMS_PUB_KEY_OTS_TYPE_OFFSET],
				XLMS_OTS_TYPE_SIZE);

		/**
		 * Parse HSS/Current level's public key provided, determine the expected signature
		 * length of sig[0]
		 */

		/**Fetch the type of public key, if not a valid/supported type return error. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XLms_LookupParamSet(PublicKeyLmsType, &PubKeyLmsParam);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_LMS_HSS_L0_PUB_KEY_LMS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}

		/** Fetch the type of public key, if not a valid/supported Type return error */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XLms_OtsLookupParamSet(PublicKeyLmsOtsType, &PubKeyLmsOtsParam);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_LMS_HSS_L0_PUB_KEY_LMS_OTS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		/**
		 * Before we proceed to LMS OTS, we also need to check if SHA algo selected matches
		 * the type selected in signature.
		 */
		if ((PubKeyLmsParam->HashAlgId != PubKeyLmsOtsParam->HashAlgId) ||
			(HssParamsPtr->ShaMode != PubKeyLmsOtsParam->HashAlgId) ||
			(PubKeyLmsParam->HashOutputBytes != PubKeyLmsOtsParam->HashOutputBytes)) {
			Status = XASUFW_LMS_SIGN_VERIFY_BH_AND_TYPE_SHA_ALGO_MISMATCH_L0_ERROR;
			goto END;
		}

		CurrentPubKeyActualSize = XLMS_PUB_KEY_FIXED_FIELD_SIZE + PubKeyLmsParam->HashOutputBytes;

		CurrentLevelSignLen = (PubKeyLmsOtsParam->SignLen + XLMS_Q_FIELD_SIZE + XLMS_TYPE_SIZE +
					PubKeyLmsParam->SignatureLenBytes);

		/**
		 * Signature Length should be equal or more than required,
		 * should have at least sig[0] + pub[1] Length.
		 */
		if (HssParamsPtr->SignatureLen < (XLMS_HSS_LEVELS_FIELD_SIZE + CurrentLevelSignLen +
				CurrentPubKeyActualSize + SignatureLengthConsumed)) {
			Status = XASUFW_LMS_HSS_L0_SIGN_INVALID_LEN_2_ERROR;
			goto END;
		}

		LmsSignVerifyParams.MsgAddr = (u64)(UINTPTR)&TmpSignaturePtr[CurrentLevelSignLen];
		LmsSignVerifyParams.MsgLen = CurrentPubKeyActualSize;
		LmsSignVerifyParams.PreHashedMsg = XASU_LMS_MSG_NOT_PREHASHED;
		LmsSignVerifyParams.SignatureAddr = (u64)(UINTPTR)TmpSignaturePtr;
		LmsSignVerifyParams.SignatureLen = CurrentLevelSignLen;
		LmsSignVerifyParams.LmsHssKeyObj.PubKeyAddr = (u64)(UINTPTR)TmpPublicKeyPtr;
		LmsSignVerifyParams.LmsHssKeyObj.PubKeyLen = CurrentPubKeyActualSize;
		LmsSignVerifyParams.ShaType = HssParamsPtr->ShaType;
		LmsSignVerifyParams.ShaMode = HssParamsPtr->ShaMode;

		/**
		 * Now that we have checked if signature has sig[0] & pub[1] contents verify pub[1]
		 * as data to be authenticated, with sig[0], output of operation should match with
		 * HSS public key.
		 */
		Status = XLms_SignatureVerification(ShaInstPtr, DmaPtr, &LmsSignVerifyParams);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			/* Inner SignatureVerification needs non-blocking DMA */
			HssInitDmaState = XLMS_HSS_INIT_STATE_SIGN_VERIFY;
			break;
		}
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
						XASUFW_LMS_HSS_L0_PUB_KEY_AUTH_FAILED_ERROR);
			goto END;
		}

		/**
		 * At this point the signature has a valid Length signature
		 * for a valid Length public key for a lower level tree,
		 * which is all verified.
		 */

		/** Skip sig[i] + pub[i+1] to process next level in the HSS chain */
		TmpPublicKeyPtr = (TmpSignaturePtr + CurrentLevelSignLen);
		TmpSignaturePtr += (CurrentLevelSignLen + CurrentPubKeyActualSize);
		SignatureLengthConsumed += (CurrentLevelSignLen + CurrentPubKeyActualSize);
	}

	if (Status == XASUFW_SUCCESS) {
		/**
		 * Security measure: Copy authenticated pub[Nspk] with byte-by-byte verification.
		 * If loop index is glitched by fault injection, same location is re-copied;
		 * if copy fails, immediate read-back comparison detects the fault.
		 */

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		/** Copy all bytes of pub[Nspk] (LMS public key) for use in final message verification */
		for (Index = 0U; Index < CurrentPubKeyActualSize; Index++) {
			AuthenticatedKey.Buff[Index] = TmpPublicKeyPtr[Index];
			if (TmpPublicKeyPtr[Index] != AuthenticatedKey.Buff[Index]) {
				Status = XASUFW_LMS_PUB_OP_FAILED_ERROR;
				goto END;
			}
		}

		/* Glitch detection: verify all iterations completed without being skipped */
		if (CurrentPubKeyActualSize != Index) {
			Status = XASUFW_LMS_PUB_OP_FAILED_1_ERROR;
			goto END;
		}

		/**
		 * RFC 8554 Section 4.6: Extract 'q' (leaf index) from sig[Nspk] for message digest prefix.
		 * q identifies which OTS key in the lowest-level tree signed the message.
		 */
		CurrentLmsQ = XAsufw_SwapBytes(&TmpSignaturePtr[XLMS_SIGNATURE_Q_FIELD_OFFSET],
						XLMS_Q_FIELD_SIZE);
		DigestPrefixFields.Fields.q = Xil_Htonl(CurrentLmsQ);

		/**
		 * RFC 8554 Section 4.6 Step 2: Extract 'I' (16-byte tree identifier) from authenticated
		 * public key for use in message digest computation: Q = H(I || q || D_MESG || C || message).
		 */
		Status = Xil_SMemCpy((void*)DigestPrefixFields.Fields.DigestTreeId, XLMS_I_FIELD_SIZE,
			(const void*)AuthenticatedKey.Fields.PubKeyTreeId, XLMS_I_FIELD_SIZE, XLMS_I_FIELD_SIZE);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_COPY_FAIL;
			goto END;
		}

		/** Extract LMS type and LMS-OTS type from pub[Nspk] for final signature verification step */
		PublicKeyLmsType = (XLms_Type)XAsufw_SwapBytes((const u8*)&AuthenticatedKey.Buff[XLMS_PUB_KEY_TYPE_OFFSET],
								XLMS_TYPE_SIZE);
		PublicKeyLmsOtsType = (XLms_OtsType)XAsufw_SwapBytes((const u8*)&AuthenticatedKey.Buff[XLMS_PUB_KEY_OTS_TYPE_OFFSET],
								XLMS_OTS_TYPE_SIZE);

		/** Lookup LMS parameters (tree height h, hash output m) for final verification stage */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XLms_LookupParamSet(PublicKeyLmsType, &PubKeyLmsParam);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_LMS_HSS_L1_PUB_KEY_LMS_TYPE_1_UNSUPPORTED_ERROR;
			goto END;
		}

		/** Lookup OTS parameters (Winternitz w, chain count p) for final verification stage */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XLms_OtsLookupParamSet(PublicKeyLmsOtsType, &PubKeyLmsOtsParam);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_LMS_HSS_L1_PUB_KEY_LMS_OTS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}

		/**
		 * RFC 8554 Section 4.6: Extract 'C' (n-byte randomizer) from sig[Nspk]'s OTS signature.
		 * C provides randomization for the message digest to prevent chosen-message attacks.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCpy((void*)DigestPrefixFields.Fields.C, XLMS_C_FIELD_SIZE,
			(const void*)&TmpSignaturePtr[XLMS_SIGNATURE_OTS_FIELD_OFFSET + XLMS_OTS_SIGN_C_FIELD_OFFSET],
			PubKeyLmsOtsParam->HashOutputBytes, PubKeyLmsOtsParam->HashOutputBytes);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_COPY_FAIL;
			goto END;
		}

		HashOutputN_g = PubKeyLmsOtsParam->HashOutputBytes;
		HssCallerShaMode_g = HssParamsPtr->ShaMode;
		HssCallerShaType_g = HssParamsPtr->ShaType;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function calculates the Digest of data to authenticate to initiate the process
*		of LMS verification
*
* @param	ShaInstPtr	Pointer to the SHA instance.
* @param	DmaPtr		Pointer to the DMA instance.
* @param	DataAddr	Address of the data to be authenticated
* @param	DataLen		Length of data to be authenticated
* @param	Mode		Mode of the SHA operation
*
* @return
*		- XASUFW_SUCCESS - Digest generation succeeded
*		- XASUFW_SHA_START_FAILED - SHA engine start operation failed
*		- XASUFW_SHA_UPDATE_FAIL - SHA engine update operations failed
*		- XASUFW_CMD_IN_PROGRESS - Non-blocking DMA in progress for large message
**************************************************************************************************/
s32 XLms_HashMessage(XSha *ShaInstPtr, XAsufw_Dma *DmaPtr, u64 DataAddr, u32 DataLen, u32 Mode)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	static const u8 XLMS_D_MESG[XLMS_D_FIELD_SIZE] = { XLMS_D_MESG_BYTE, XLMS_D_MESG_BYTE };
	/** Static state variable to track non-blocking DMA progress */
	static u32 HashMessageDmaState = XLMS_DMA_STATE_IDLE;

	/** Validate input pointers and address. */
	if ((NULL == ShaInstPtr) || (NULL == DmaPtr)) {
		Status = XASUFW_LMS_INVALID_PARAM;
		goto END;
	}

	/** Validate data address on initial call when message is non-empty. */
	if ((HashMessageDmaState == XLMS_DMA_STATE_IDLE) && (DataLen > 0U) && (0U == DataAddr)) {
		Status = XASUFW_LMS_INVALID_PARAM;
		goto END;
	}

	switch (HashMessageDmaState) {
	case XLMS_DMA_STATE_PENDING:
		/* Resume after non-blocking DMA completion - skip to finish stage */
		HashMessageDmaState = XLMS_DMA_STATE_IDLE;
		break;

	case XLMS_DMA_STATE_IDLE:
		/**
		 * RFC 8554 Section 4.4: Set domain separator D_MESG (0x8181) to distinguish message
		 * hashing from other hash computations.
		 */
		DigestPrefixFields.Fields.D_MESG[XASUFW_BUFFER_INDEX_ZERO] = XLMS_D_MESG[XASUFW_BUFFER_INDEX_ZERO];
		DigestPrefixFields.Fields.D_MESG[XASUFW_BUFFER_INDEX_ONE] = XLMS_D_MESG[XASUFW_BUFFER_INDEX_ONE];

		/* Initialize SHA engine for streaming hash computation of: H(I || q || D_MESG || C || message) */
		Status = XSha_Start(ShaInstPtr, Mode);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_SHA_START_FAILED;
			goto END;
		}

		/**
		 * Handle empty message case: hash only the prefix fields.
		 * Q = H(I || u32str(q) || u16str(D_MESG) || C)
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		if (0U == DataLen) {
			Status = XSha_Update(ShaInstPtr, DmaPtr, (u64)(UINTPTR)DigestPrefixFields.Buff,
				(XLMS_I_FIELD_SIZE + XLMS_Q_FIELD_SIZE + XLMS_D_MESG_FIELD_SIZE + HashOutputN_g),
				(u32)XASU_TRUE);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_SHA_UPDATE_FAIL;
				goto END;
			}
		} else {
			/**
			 * Handle non-empty message: stream prefix fields then message data.
			 * RFC 8554 Section 4.6 Step 2: Q = H(I || u32str(q) || u16str(D_MESG) || C || message)
			 */
			/* Feed prefix (I[16] || q[4] || D_MESG[2] || C[n]) into hash engine */
			Status = XSha_Update(ShaInstPtr, DmaPtr, (u64)(UINTPTR)DigestPrefixFields.Buff,
				(XLMS_I_FIELD_SIZE + XLMS_Q_FIELD_SIZE + XLMS_D_MESG_FIELD_SIZE + HashOutputN_g),
				(u32)XASU_FALSE);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_SHA_UPDATE_FAIL;
				goto END;
			}
			/**
			 * Feed message data (via DMA from 64-bit address) and signal final block.
			 * For large messages (> XASUFW_DMA_BLOCKING_SIZE), XSha_Update returns
			 * CMD_IN_PROGRESS to enable non-blocking operation.
			 */
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XSha_Update(ShaInstPtr, DmaPtr, DataAddr, DataLen, (u32)XASU_TRUE);
			if (Status == XASUFW_CMD_IN_PROGRESS) {
				HashMessageDmaState = XLMS_DMA_STATE_PENDING;
			} else if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_SHA_UPDATE_FAIL;
				goto END;
			} else {
				/* Do nothing */
			}
		}
		break;

	default:
		Status = XASUFW_LMS_INVALID_PARAM;
		goto END;
	}

	if (Status != XASUFW_CMD_IN_PROGRESS) {
		/* Finalize hash and store 32-byte digest Q in DigestChecksum buffer for OTS processing */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Finish(ShaInstPtr, DmaPtr, (u64)(UINTPTR)DigestChecksum.Buff, XLMS_DIGEST_SIZE,
					XASU_FALSE);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function Completes LMS Signature verification process.Data should have been
*		pre-processed before calling this function, by calling XLms_HssInit & XLms_HashMessage
*		in sequence
*
* @param	ShaInstPtr	Pointer to the SHA instance.
* @param	DmaPtr		Pointer to the DMA instance.
* @param	SignatureAddr	Address of the Signature buffer
* @param	SignatureLen	Length of signature buffer
*
* @return
*		- XASUFW_SUCCESS - Remaining LMS verification completed successfully
*		- XASUFW_LMS_HSS_L1_PUB_KEY_LMS_TYPE_2_UNSUPPORTED_ERROR - Authenticated key LMS
						type unsupported for final checks
*		- XASUFW_LMS_HSS_L1_PUB_KEY_LMS_OTS_TYPE_UNSUPPORTED_ERROR - Authenticated key LMS
						OTS type unsupported for final checks
*		- XASUFW_LMS_HSS_OTS_SIGN_INVALID_LEN_1_ERROR - Final signature fragment too small
*		- XASUFW_LMS_HSS_L1_SIGN_INVALID_LEN_2_ERROR - Final signature fragment length
								mismatch
*		- XASUFW_DMA_COPY_FAIL - DMA transfer of the remaining signature failed
*		- XASUFW_LMS_HSS_L0_PUB_KEY_AUTH_FAILED_ERROR - Final LMS verification failed to
								authenticate public key
*		- XASUFW_CMD_IN_PROGRESS - Non-blocking DMA transfer started for large signature
**************************************************************************************************/
s32 XLms_HssFinish(XSha *ShaInstPtr, XAsufw_Dma *DmaPtr, u64 SignatureAddr, u32 SignatureLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XLms_Param* PskPubKeyLmsParam = NULL;
	const XLms_OtsParam* PskPubKeyLmsOtsParam = NULL;
	XLms_Type PublicKeyLmsType = XLMS_NOT_SUPPORTED;
	XLms_OtsType PublicKeyLmsOtsType = XLMS_OTS_NOT_SUPPORTED;
	static u32 SignNpskLen_s = 0U;
	volatile u32 Sign1Len = (SignatureLen - SignatureLengthConsumed);
	static XAsu_LmsHssSignVerifyParams LmsSignVerifyParams;
	/** Static state variable to track non-blocking DMA progress */
	static u32 HssFinishDmaState = XLMS_DMA_STATE_IDLE;
	u32 ResumeState = HssFinishDmaState;

	/** Validate input pointers. */
	if ((NULL == ShaInstPtr) || (NULL == DmaPtr)) {
		Status = XASUFW_LMS_INVALID_PARAM;
		goto END;
	}

	/** Validate address and length on initial call. */
	if (HssFinishDmaState == XLMS_DMA_STATE_IDLE) {
		if ((0U == SignatureAddr) || (0U == SignatureLen)) {
			Status = XASUFW_LMS_INVALID_PARAM;
			goto END;
		}
	}

	/**
	 * Check if we're resuming after non-blocking DMA completion.
	 * SIG_COPY: resume after signature DMA transfer.
	 * SIGN_VERIFY: resume after inner SignatureVerification DMA.
	 */
	switch (HssFinishDmaState) {
	case XLMS_HSS_FINISH_STATE_SIG_COPY:
	case XLMS_HSS_FINISH_STATE_SIGN_VERIFY:
		HssFinishDmaState = XLMS_DMA_STATE_IDLE;
		break;

	case XLMS_DMA_STATE_IDLE:
		/**
		 * Extract LMS type from the authenticated public key (stored during HssInit).
		 * This key represents the lowest level tree in the HSS hierarchy.
		 */
		PublicKeyLmsType = (XLms_Type)XAsufw_SwapBytes(
			(const u8*)&AuthenticatedKey.Buff[XLMS_PUB_KEY_TYPE_OFFSET], XLMS_TYPE_SIZE);
		PublicKeyLmsOtsType = (XLms_OtsType)XAsufw_SwapBytes(
			(const u8*)&AuthenticatedKey.Buff[XLMS_PUB_KEY_OTS_TYPE_OFFSET],
			XLMS_OTS_TYPE_SIZE);

		/* Lookup LMS parameters for the authenticated key's tree height and hash size */
		Status = XLms_LookupParamSet(PublicKeyLmsType, &PskPubKeyLmsParam);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_LMS_HSS_L1_PUB_KEY_LMS_TYPE_2_UNSUPPORTED_ERROR;
			goto END;
		}

		/* Lookup OTS parameters for signature verification */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XLms_OtsLookupParamSet(PublicKeyLmsOtsType, &PskPubKeyLmsOtsParam);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_LMS_HSS_L1_PUB_KEY_LMS_OTS_TYPE_UNSUPPORTED_ERROR;
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		/* Remaining signature must have at least 4 bytes for minimal validation */
		if (XLMS_OTS_TYPE_SIZE > Sign1Len) {
			Status = XASUFW_LMS_HSS_OTS_SIGN_INVALID_LEN_1_ERROR;
			goto END;
		}

		/**
		 * Calculate expected length of sig[Npsk] (final LMS signature).
		 * sig[Npsk] = OTS signature length + q field (4 bytes) + LMS type (4 bytes) +
		 *             authentication path (m * h bytes, where m is hash output size, h is tree height)
		 */
		SignNpskLen_s = (PskPubKeyLmsOtsParam->SignLen + XLMS_Q_FIELD_SIZE + XLMS_TYPE_SIZE +
				PskPubKeyLmsParam->SignatureLenBytes);

		/* Verify remaining signature length exactly matches expected sig[Npsk] length */
		if (Sign1Len != SignNpskLen_s) {
			Status = XASUFW_LMS_HSS_L1_SIGN_INVALID_LEN_2_ERROR;
			goto END;
		}

		/**
		 * DMA copy the final signature portion (sig[Npsk]) to local buffer.
		 * This signature is used to verify the pre-hashed message digest against
		 * the authenticated lowest-level public key.
		 * For large signatures (> XASUFW_DMA_BLOCKING_SIZE), use non-blocking DMA transfer.
		 */
		if (SignNpskLen_s > XASUFW_DMA_BLOCKING_SIZE) {
			Status = XAsufw_StartDmaXfr(DmaPtr, SignatureAddr + SignatureLengthConsumed,
				(u64)(UINTPTR)LmsLocalSignatureBuff, SignNpskLen_s, 0U);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_DMA_COPY_FAIL;
				goto END;
			}
			HssFinishDmaState = XLMS_HSS_FINISH_STATE_SIG_COPY;
			Status = XASUFW_CMD_IN_PROGRESS;
		} else {
			Status = XAsufw_DmaXfr(DmaPtr, SignatureAddr + SignatureLengthConsumed,
				(u64)(UINTPTR)LmsLocalSignatureBuff, SignNpskLen_s, 0U);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_DMA_COPY_FAIL;
				goto END;
			}
		}
		break;

	default:
		Status = XASUFW_LMS_INVALID_PARAM;
		goto END;
	}

	/**
	 * Setup verification parameters for the final LMS signature.
	 * PreHashedMsg = TRUE indicates the message digest was already computed
	 * by XLms_HashMessage() and is stored in DigestChecksum.
	 * Skip when resuming from SIGN_VERIFY state since params are already set.
	 */
	if (Status != XASUFW_CMD_IN_PROGRESS) {
		if (ResumeState != XLMS_HSS_FINISH_STATE_SIGN_VERIFY) {
			LmsSignVerifyParams.MsgAddr = (u64)(UINTPTR)DigestChecksum.Buff;
			LmsSignVerifyParams.MsgLen = XLMS_DIGEST_SIZE;
			LmsSignVerifyParams.PreHashedMsg = XASU_LMS_MSG_PREHASHED;
			LmsSignVerifyParams.SignatureAddr = (u64)(UINTPTR)LmsLocalSignatureBuff;
			LmsSignVerifyParams.SignatureLen = SignNpskLen_s;
			LmsSignVerifyParams.LmsHssKeyObj.PubKeyAddr = (u64)(UINTPTR)AuthenticatedKey.Buff;
			LmsSignVerifyParams.LmsHssKeyObj.PubKeyLen = (XLMS_PUB_KEY_FIXED_FIELD_SIZE + HashOutputN_g);
			LmsSignVerifyParams.ShaType = HssCallerShaType_g;
			LmsSignVerifyParams.ShaMode = HssCallerShaMode_g;
		}

		/**
		 * Perform final LMS signature verification.
		 * This authenticates the pre-hashed message using the lowest-level
		 * public key that was authenticated during XLms_HssInit.
		 * XLms_SignatureVerification may return CMD_IN_PROGRESS for large p values
		 * (e.g., w=1, p=265) when the final OTS digest buffer exceeds 4KB.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XLms_SignatureVerification(ShaInstPtr, DmaPtr, &LmsSignVerifyParams);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			HssFinishDmaState = XLMS_HSS_FINISH_STATE_SIGN_VERIFY;
		}
	}

	/**
	 * Measure and print the performance time for the HSS/LMS verification operation, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_LMS_ID);

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Returns the DMA channel that the caller should wait on after receiving
*		XASUFW_CMD_IN_PROGRESS from any LMS/HSS core function.
*
*		- XASUDMA_DST_CHANNEL when the pending transfer is a DMA copy
*		  (signature/public key copy in ValidateInputs or HssInit/HssFinish).
*		- XASUDMA_SRC_CHANNEL when the pending transfer is a SHA engine feed
*		  (HashMessage or XSha_Digest inside OtsSignatureProcess).
*
* @return	XASUDMA_SRC_CHANNEL or XASUDMA_DST_CHANNEL.
*
**************************************************************************************************/
XAsuDma_Channel XLms_GetPendingDmaChannel(void)
{
	XAsuDma_Channel Channel = XASUDMA_SRC_CHANNEL;
	/*
	 * OtsProcessDmaState != XLMS_DMA_STATE_IDLE means OtsSignatureProcess is waiting on
	 * SHA (SRC). HashMessageDmaState is local to HashMessage, but when it's set, the
	 * OtsProcessDmaState is also set to XLMS_OTS_PROCESS_STATE_HASH_MESSAGE by the caller,
	 * so checking OtsProcessDmaState covers both.
	 *
	 * SignVerifyDmaState == XLMS_SIGN_VERIFY_STATE_OTS_PROCESS means OTS path (SRC),
	 *   checked via OtsProcessDmaState.
	 * SignVerifyDmaState == XLMS_SIGN_VERIFY_STATE_VALIDATE_INPUTS means ValidateInputs
	 *   path (DST).
	 * HssInitDmaState == XLMS_HSS_INIT_STATE_SIG_COPY means HssInit signature copy (DST).
	 * HssInitDmaState == XLMS_HSS_INIT_STATE_SIGN_VERIFY means inner SignatureVerification
	 *   DMA pending (channel varies, checked via OtsProcessDmaState).
	 * HssFinishDmaState == XLMS_HSS_FINISH_STATE_SIG_COPY means HssFinish signature copy (DST).
	 * HssFinishDmaState == XLMS_HSS_FINISH_STATE_SIGN_VERIFY means inner SignatureVerification
	 *   DMA pending in HssFinish (channel varies, checked via OtsProcessDmaState).
	 *
	 * All SRC-channel paths flow through OtsProcessDmaState being non-idle.
	 */

	if (OtsProcessDmaState == XLMS_DMA_STATE_IDLE) {
		Channel = XASUDMA_DST_CHANNEL;
	}

	return Channel;
}

/*************************************************************************************************/
/**
* @brief	This function validates and initializes LMS OTS signature parameters.
*		It validates the signature buffer, checks the OTS type matches expected,
*		and looks up the corresponding parameter set.
*
* @param	LmsOtsSignatureBuff	Pointer to OTS signature buffer
* @param	LmsOtsSignatureLen	Length of OTS signature buffer
* @param	LmsOtsExpectedType	Expected OTS signature type from public key
*
* @return
*		- XASUFW_SUCCESS - Validation completed successfully
*		- XASUFW_LMS_INVALID_PARAM - Signature buffer pointer or length is invalid
*		- XASUFW_LMS_OTS_PUB_KEY_SIGN_TYPE_MISMATCH_ERROR - Extracted OTS type differs
									from expected
*		- XASUFW_LMS_OTS_SIGN_UNSUPPORTED_TYPE_ERROR - OTS parameter lookup failed or
								unsupported type selected
**************************************************************************************************/
static s32 XLms_OtsSignatureInit(u8* LmsOtsSignatureBuff, u32 LmsOtsSignatureLen,
				XLms_OtsType LmsOtsExpectedType)
{
	s32 Status = XASUFW_FAILURE;
	XLms_OtsType LmsOtsType = XLMS_OTS_NOT_SUPPORTED;

	/** Ensure buffer is large enough to contain at least the OTS type field */
	if (XLMS_OTS_TYPE_SIZE > LmsOtsSignatureLen) {
		Status = XASUFW_LMS_INVALID_PARAM;
		goto END;
	}

	/** Validate signature buffer pointer is not NULL */
	if (NULL == LmsOtsSignatureBuff) {
		Status = XASUFW_LMS_INVALID_PARAM;
		goto END;
	}

	/** Extract OTS type from signature buffer (Big Endian to Little Endian conversion) */
	LmsOtsType = (XLms_OtsType)XAsufw_SwapBytes(
		(const u8*)&LmsOtsSignatureBuff[XLMS_OTS_SIGN_TYPE_FIELD_OFFSET],
		XLMS_OTS_TYPE_SIZE);

	/** Verify extracted OTS type matches expected type from public key */
	if (LmsOtsType != LmsOtsExpectedType) {
		Status = XASUFW_LMS_OTS_PUB_KEY_SIGN_TYPE_MISMATCH_ERROR;
		goto END;
	}

	/** Lookup OTS parameters (w, p, n) based on the extracted type */
	Status = XLms_OtsLookupParamSet(LmsOtsType, &LmsOtsSignParam_g);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_LMS_OTS_SIGN_UNSUPPORTED_TYPE_ERROR;
		goto END;
	}

	/** Verify signature length matches expected length for this OTS type */
	if (LmsOtsSignParam_g->SignLen != LmsOtsSignatureLen) {
		Status = XASUFW_LMS_INVALID_PARAM;
		goto END;
	}

	/** Store signature buffer and runtime hash size for process function */
	HashOutputN_g = LmsOtsSignParam_g->HashOutputBytes;
	LmsOtsSignatureBuff_g = LmsOtsSignatureBuff;

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function processes an LMS OTS signature to compute the candidate OTS public key
*		(spec notation Kc). It computes a message digest (if not pre-hashed), calculates the
*		Winternitz checksum, and walks the hash chains for each signature element to recover
*		the corresponding public key value.
*
* @param	ShaInstPtr	Pointer to the SHA instance.
* @param	DmaPtr		Pointer to the DMA instance.
* @param	LmsSignVerifyParams	Pointer to LMS signature verification parameters
* @param	OtsPubKeyCandidate	Buffer where the computed OTS public key candidate (Kc)
*					needs to be copied, assumed that has enough place
*
* @return
*		- XASUFW_SUCCESS - Signature verification completed without errors
*		- XASUFW_LMS_INVALID_PARAM - Invalid input pointers, addresses, or lengths
*		- XASUFW_DMA_COPY_FAIL - DMA transfers for public key, signature, or message failed
*		- XASUFW_MEM_COPY_FAIL - Local buffer copies failed integrity checks
*		- XASUFW_LMS_PUB_KEY_UNSUPPORTED_TYPE_1_ERROR - Public key LMS type unsupported
*		- XASUFW_LMS_SIGN_EXPECTED_PUB_KEY_LEN_2_ERROR - Public key length mismatched with
*								expected size
*		- XASUFW_LMS_SIGN_LEN_1_ERROR - Signature too short to contain LMS OTS header
*		- XASUFW_LMS_SIGN_LEN_2_ERROR - Signature shorter than required LMS OTS portion
*		- XASUFW_LMS_SIGN_LEN_3_ERROR - Signature length mismatch once auth path included
*		- XASUFW_LMS_OTS_PUB_KEY_LMS_OTS_SIGN_TYPE_MISMATCH_ERROR - LMS OTS types mismatch
*								between signature and public key
*		- XASUFW_LMS_SIGN_UNSUPPORTED_OTS_TYPE_1_ERROR - Unsupported LMS OTS signature type
*		- XASUFW_LMS_PUB_KEY_LMS_SIGN_TYPE_MISMATCH_ERROR - LMS signature type mismatches
*								public key type
*		- XASUFW_LMS_SIGN_UNSUPPORTED_TYPE_1_ERROR - Unsupported LMS signature type encountered
*		- XASUFW_LMS_SIGN_INVALID_NODE_NUMBER_ERROR - Node number outside the tree range
*		- XASUFW_LMS_SIGN_OTS_OP_ERROR - OTS initialization or processing failed
*		- XASUFW_LMS_SIGN_VERIF_SHA_DIGEST_LEAF_FAILED_ERROR - SHA digest failure while
*									processing leaf node
*		- XASUFW_LMS_PUB_KEY_AUTHENTICATION_FAILED_ERROR - Calculated root does not match
*								public key
*		- XASUFW_LMS_PUB_KEY_AUTHENTICATION_GLITCH_ERROR - Root comparison glitch detected
*
**************************************************************************************************/
static s32 XLms_OtsSignatureProcess(XSha *ShaInstPtr, XAsufw_Dma *DmaPtr,
	const XAsu_LmsHssSignVerifyParams *LmsSignVerifyParams, u8* OtsPubKeyCandidate)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Checksum = 0U;
	u32 DigitVal = 0U;
	u32 DigitIndex = 0U;
	u32 ChainIter = 0U;
	u32 SignToOtsBuffIndex = 0U;
	u32 IntToOutLoopBuffIndex = 0U;
	u32 n = XLMS_N_FIELD_SIZE;
	u32 ResumeState = OtsProcessDmaState;
	static XLms_OtsSignToPubKeyHash LmsOtsSignVerifBuff __attribute__((aligned(16)));
	static XLms_OtsHashPerDigit TmpHashPerDigitBuff;
	static const u8 XLMS_D_PBLC[XLMS_D_FIELD_SIZE] = { XLMS_D_PBLC_BYTE, XLMS_D_PBLC_BYTE };
	XAsu_ShaOperationCmd ShaParamsInput;

	/**
	 * Computed root to be copied here,
	 * assumed to have enough room, created and checked by caller,
	 * also should be cleared by caller.
	 */
	if (NULL == OtsPubKeyCandidate) {
		Status = XASUFW_LMS_INVALID_PARAM;
		goto END;
	}

	/** Verify that Init was called */
	if ((NULL == LmsOtsSignatureBuff_g) || (NULL == LmsOtsSignParam_g)) {
		Status = XASUFW_LMS_INVALID_PARAM;
		goto END;
	}

	n = LmsOtsSignParam_g->HashOutputBytes;

	/**
	 * Check if we're resuming after non-blocking DMA completion.
	 * State 1: Resume at HashMessage, State 2: Resume at final XSha_Digest
	 */
	switch (OtsProcessDmaState) {
	case XLMS_OTS_PROCESS_STATE_HASH_MESSAGE:
		OtsProcessDmaState = XLMS_DMA_STATE_IDLE;
		/* Need to call HashMessage again to complete - it has internal state to resume */
		Status = XLms_HashMessage(ShaInstPtr, DmaPtr, LmsSignVerifyParams->MsgAddr,
					LmsSignVerifyParams->MsgLen, LmsOtsSignParam_g->HashAlgId);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
						XASUFW_LMS_OTS_DIGEST_CHECKSUM_OP_FAILED_ERROR);
			goto END;
		}
		break;

	case XLMS_OTS_PROCESS_STATE_FINAL_DIGEST:
		OtsProcessDmaState = XLMS_DMA_STATE_IDLE;
		/* Need to call XSha_Digest again to complete - it has internal state to resume */
		break;

	case XLMS_DMA_STATE_IDLE:
		/**
		 * Message processing - sequence-2
		 */
		/**
		 * Now the message's digest needs to be calculated and checksum needs to be appended
		 * Q = H(I || u32str(q) || u16str(D_MESG) || C || message)
		 * a = (Q || checksum(Q))
		 */

		/**
		 * If message is pre hashed, copy the pre-computed digest from MsgAddr into
		 * DigestChecksum buffer. Otherwise, calculate the digest.
		 * Q = H(I || u32str(q) || u16str(D_MESG) || C || message)
		 * a = (Q || checksum(Q))
		 */
		if (XASU_FALSE == LmsSignVerifyParams->PreHashedMsg) {
			/** Extracting C, Big Endian to Big Endian */
			Status = Xil_SMemCpy((void*)DigestPrefixFields.Fields.C, XLMS_C_FIELD_SIZE,
				(void*)&LmsOtsSignatureBuff_g[XLMS_OTS_SIGN_C_FIELD_OFFSET],
				n, n);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_MEM_COPY_FAIL;
				goto END;
			}

			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XLms_HashMessage(ShaInstPtr, DmaPtr, LmsSignVerifyParams->MsgAddr,
						LmsSignVerifyParams->MsgLen, LmsOtsSignParam_g->HashAlgId);
			if (Status == XASUFW_CMD_IN_PROGRESS) {
				/* Non-blocking DMA in progress for large message */
				OtsProcessDmaState = XLMS_OTS_PROCESS_STATE_HASH_MESSAGE;
			} else if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
							XASUFW_LMS_OTS_DIGEST_CHECKSUM_OP_FAILED_ERROR);
				goto END;
			} else {
				/* Do nothing */
			}
		} else {
			/**
			 * Copy pre-hashed digest from MsgAddr into DigestChecksum buffer.
			 * Skip DMA if source is already DigestChecksum (internal HSS path).
			 */
			if (LmsSignVerifyParams->MsgAddr != (u64)(UINTPTR)DigestChecksum.Buff) {
				Status = XAsufw_DmaXfr(DmaPtr, LmsSignVerifyParams->MsgAddr,
						(u64)(UINTPTR)DigestChecksum.Buff, n, 0U);
				if (Status != XASUFW_SUCCESS) {
					Status = XASUFW_DMA_COPY_FAIL;
					goto END;
				}
			}
		}
		break;

	default:
		Status = XASUFW_LMS_INVALID_PARAM;
		goto END;
	}

	/**
	 * Skip checksum computation and chain processing when resuming from FINAL_DIGEST state,
	 * as these were already completed before the non-blocking DMA was started.
	 */
	if ((Status != XASUFW_CMD_IN_PROGRESS) &&
		(ResumeState != XLMS_OTS_PROCESS_STATE_FINAL_DIGEST)) {
		/**
		 * RFC 8554 Section 4.4: Compute checksum over message digest.
		 * Checksum = sum(2^w - 1 - coef(Q, i, w)) for i=0..u-1, then left-shift by ls bits.
		 * This ensures signature elements have enough "headroom" for verification.
		 */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XLms_OtsComputeChecksum(DigestChecksum.Fields.Digest, n,
				(u32)LmsOtsSignParam_g->w, (u32)LmsOtsSignParam_g->ls, (u32* const)&Checksum);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_LMS_DIGEST_CHECKSUM_FAILED_ERROR);
			goto END;
		}

		/**
		 * Convert 16-bit checksum from (little-endian) to (big-endian) as required by RFC 8554 Section 3.1.
		 * Checksum is appended after digest Q to form the coef extraction buffer.
		 */
		DigestChecksum.Buff[n + 1U] = (u8)(Checksum & XASUFW_TWO_BYTE_MASK);
		DigestChecksum.Buff[n] = (u8)((Checksum >> XASUFW_ONE_BYTE_SHIFT_VALUE)
									& XASUFW_TWO_BYTE_MASK);

		/**
		 * Signature processing - sequence-2
		 */

		/**
		 * All params are extracted from public key, now proceed to parse signature
		 * 4b.3
		 * Q = H(I || u32str(q) || u16str(D_MESG) || C || message)
		 * a = (Q || checksum(Q))
		 *  for ( i = 0; i < p; i = i + 1 ) {
		 *    a = coef(Q || Cksm(Q), i, w)
		 *    tmp = y[i]
		 *    for ( j = a; j < 2^w - 1; j = j + 1 ) {
		 *      tmp = H(I || u32str(q) || u16str(i) || u8str(j) || tmp)
		 *    }
		 *    z[i] = tmp
		 *  }
		 *  OtsPubKeyCandidate = H(I || u32str(q) || u16str(D_PBLC) ||
		 *                                z[0] || z[1] || ... || z[p-1])
		 */

		/**
		 * Pre-populate hash input buffer fields that remain constant across all p iterations.
		 * RFC 8554 Section 4.6 Step 4b.3: Hash input format is H(I || u32str(q) || u16str(i) || u8str(j) || tmp)
		 * I and q are fixed for this signature; i and j vary per-iteration.
		 */

		/* Copy 16-byte tree identifier 'I' */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCpy((void*)&TmpHashPerDigitBuff.Fields.TreeId[0U], XLMS_I_FIELD_SIZE,
			(void*)DigestPrefixFields.Fields.DigestTreeId, XLMS_I_FIELD_SIZE, XLMS_I_FIELD_SIZE);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_COPY_FAIL;
			goto END;
		}

		/* Copy 4-byte leaf index 'q' - convert from LE to BE */
		TmpHashPerDigitBuff.Fields.LeafIndex = Xil_Htonl(CurrentLmsQ);

		/**
		 * Pre-populate final OTS public key hash buffer for RFC 8554 Section 4.6 Step 4b.3:
		 * Kc = H(I || u32str(q) || u16str(D_PBLC) || z[0] || z[1] || ... || z[p-1])
		 * The z[i] values are computed in the main loop and appended to this buffer.
		 */

		/* Copy 16-byte tree identifier 'I' to final hash buffer */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCpy((void*)&LmsOtsSignVerifBuff.Buff[XLMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_I_OFFSET],
			XLMS_I_FIELD_SIZE, (void*)DigestPrefixFields.Fields.DigestTreeId, XLMS_I_FIELD_SIZE,
			XLMS_I_FIELD_SIZE);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_COPY_FAIL;
			goto END;
		}

		/* Copy 4-byte leaf index 'q' - convert from LE to BE */
		LmsOtsSignVerifBuff.Fields.q = Xil_Htonl(CurrentLmsQ);

		/* Set D_PBLC domain separator (0x8080) indicating public key hash computation */
		LmsOtsSignVerifBuff.Fields.D_PBLC[XASUFW_BUFFER_INDEX_ZERO] = XLMS_D_PBLC[XASUFW_BUFFER_INDEX_ZERO];
		LmsOtsSignVerifBuff.Fields.D_PBLC[XASUFW_BUFFER_INDEX_ONE] = XLMS_D_PBLC[XASUFW_BUFFER_INDEX_ONE];

		IntToOutLoopBuffIndex = 0U;
		SignToOtsBuffIndex = (XLMS_OTS_TYPE_SIZE + n);

		ShaParamsInput.DataAddr = (u64)(UINTPTR)&TmpHashPerDigitBuff.Buff[XLMS_OTS_SIGN_VERIF_TMP_BUFF_I_OFFSET];
		ShaParamsInput.HashAddr = (u64)(UINTPTR)&TmpHashPerDigitBuff.Fields.y[0U];
		ShaParamsInput.DataSize = ((XLMS_OTS_SIGN_VERIF_TMP_BUFF_Y_OFFSET - XLMS_OTS_SIGN_VERIF_TMP_BUFF_I_OFFSET) + n);
		ShaParamsInput.HashBufSize = XLMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE;
		ShaParamsInput.ShaMode = (u8)LmsOtsSignParam_g->HashAlgId;

		/**
		 * RFC 8554 Section 4.6 Step 4b.3: Process p Winternitz chain elements.
		 * For each digit position i (0 to p-1), extract w-bit digit 'a' from (Q || Cksm(Q)),
		 * then compute z[i] by hashing y[i] exactly (2^w - 1 - a) times to reach public key element.
		 */
		for (DigitIndex = 0U ; DigitIndex < (u16)LmsOtsSignParam_g->p ; DigitIndex++) {

			/* Extract w-bit digit 'a' at position i using coef() function per RFC 8554 Section 3.1.3 */
			DigitVal = XLms_OtsCoeff(DigestChecksum.Buff, DigitIndex, (u32)LmsOtsSignParam_g->w);

			/* Set digit position 'i' */
			TmpHashPerDigitBuff.Fields.DigitPosition = (u16)(DigitIndex);
			TmpHashPerDigitBuff.Fields.DigitPosition = Xil_Htons(TmpHashPerDigitBuff.Fields.DigitPosition);

			/* Copy signature element y[i] (n bytes) as initial chain value; j is set per iteration */
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = Xil_SMemCpy((void*)&TmpHashPerDigitBuff.Fields.y[0U],
				XLMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE,
				(void*)&LmsOtsSignatureBuff_g[SignToOtsBuffIndex],
				n,
				n);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_MEM_COPY_FAIL;
				goto END;
			}

			/**
			 * Forward the hash chain from signature element y[i] to public key element z[i].
			 * The number of iterations depends on the digit value extracted from the message digest.
			 * For Winternitz parameter w, each chain element is hashed (2^w - 1 - a) times,
			 * where 'a' is the digit value. This allows signature verification to complete
			 * the chain from y[i] to z[i] and compare with the public key.
			 */
			for (ChainIter = DigitVal; ChainIter < (LmsOtsSignParam_g->NoOfInvSign); ChainIter++) {
				/* Copy iteration index 'j' to hash input buffer */
				TmpHashPerDigitBuff.Fields.j = (u8)(ChainIter);

				/** Compute H(I || u32str(q) || u16str(i) || u8str(j) || tmp) */
				ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
				Status = XSha_Digest(ShaInstPtr, DmaPtr , &ShaParamsInput);
				if (Status != XASUFW_SUCCESS) {
					Status = XAsufw_UpdateErrorStatus(Status, XASUFW_LMS_OTS_SIGN_SHA_DIGEST_FAILED_ERROR);
					goto END;
				}
			}

			/**
			 * Copy to overall buffer, later all values concatenated used to calculate LMS OTS
			 * public key.
			 */
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = Xil_SMemCpy((void*)&LmsOtsSignVerifBuff.Fields.z[IntToOutLoopBuffIndex],
				XLMS_OTS_SIGN_VERIF_TMP_BUFF_Y_SIZE,
				(void*)&TmpHashPerDigitBuff.Buff[XLMS_OTS_SIGN_VERIF_TMP_BUFF_Y_OFFSET],
				n,
				n);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_MEM_COPY_FAIL;
				goto END;
			}

			IntToOutLoopBuffIndex += n;
			SignToOtsBuffIndex += n;
		}
	}

	/**
	 * RFC 8554 Section 4.6 Step 4b.3: Compute OTS public key candidate (Kc).
	 * All p hash chains are complete, now hash the concatenation:
	 * Kc = H(I || u32str(q) || u16str(D_PBLC) || z[0] || z[1] || ... || z[p-1])
	 * Total input size = 16 (I) + 4 (q) + 2 (D_PBLC) + p*n (z values)
	 * For large p values (e.g., w=1, p=265), this can exceed 4KB threshold.
	 */
	if (Status != XASUFW_CMD_IN_PROGRESS) {
		/* Set up parameters and call XSha_Digest (it has internal state to complete on resume) */
		ShaParamsInput.DataAddr = (u64)(UINTPTR)LmsOtsSignVerifBuff.Buff;
		ShaParamsInput.HashAddr = (u64)(UINTPTR)OtsPubKeyCandidate;
		ShaParamsInput.DataSize = (XLMS_OTS_SIGN_VERIF_CHAIN_TMP_BUFF_Z_OFFSET + ((u32)LmsOtsSignParam_g->p * n));
		ShaParamsInput.HashBufSize = XLMS_N_FIELD_SIZE;
		ShaParamsInput.ShaMode = (u8)LmsOtsSignParam_g->HashAlgId;

		/** Compute final OTS public key candidate hash - result used in leaf node calculation */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Digest(ShaInstPtr, DmaPtr, &ShaParamsInput);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			OtsProcessDmaState = XLMS_OTS_PROCESS_STATE_FINAL_DIGEST;
		}
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function validates inputs and performs DMA copies for LMS signature verification.
*		It validates parameters, copies public key/signature/message to local buffers,
*		and extracts/validates types.
*
* @param	DmaPtr			Pointer to DMA instance
* @param	LmsSignVerifyParams	Pointer to LMS signature verification parameters
* @param	CtxPtr			Pointer to verification context to populate
*
* @return
*		- XASUFW_SUCCESS - Validation completed successfully
*		- XASUFW_LMS_INVALID_PARAM - Invalid pointers, lengths, or addresses
*		- XASUFW_DMA_COPY_FAIL - DMA transfer for public key, signature, or message failed
*		- XASUFW_MEM_COPY_FAIL - Local buffer copy failed
*		- XASUFW_LMS_PUB_KEY_UNSUPPORTED_TYPE_1_ERROR - LMS public key type unsupported
*		- XASUFW_LMS_SIGN_EXPECTED_PUB_KEY_LEN_2_ERROR - Public key length mismatch
*		- XASUFW_LMS_SIGN_LEN_1_ERROR - Signature too short for LMS header
*		- XASUFW_LMS_SIGN_LEN_2_ERROR - Signature lacks full LMS OTS section
*		- XASUFW_LMS_SIGN_LEN_3_ERROR - Signature length mismatch after auth path
*		- XASUFW_LMS_OTS_PUB_KEY_LMS_OTS_SIGN_TYPE_MISMATCH_ERROR - OTS type mismatch
*		- XASUFW_LMS_SIGN_UNSUPPORTED_OTS_TYPE_1_ERROR - OTS signature type unsupported
*		- XASUFW_LMS_PUB_KEY_LMS_SIGN_TYPE_MISMATCH_ERROR - LMS type mismatch
*		- XASUFW_LMS_SIGN_UNSUPPORTED_TYPE_1_ERROR - LMS signature type unsupported
*		- XASUFW_LMS_SIGN_INVALID_NODE_NUMBER_ERROR - Node number outside tree range
*		- XASUFW_CMD_IN_PROGRESS - Non-blocking DMA in progress for large signature
**************************************************************************************************/
static s32 XLms_ValidateInputs(XAsufw_Dma *DmaPtr,
		const XAsu_LmsHssSignVerifyParams *LmsSignVerifyParams, XLms_SignVerifyCtx *CtxPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XLms_Type SignLmsType = XLMS_NOT_SUPPORTED;
	XLms_OtsType SignLmsOtsType = XLMS_OTS_NOT_SUPPORTED;
	static u32 ValidateInputsDmaState = XLMS_DMA_STATE_IDLE;

	switch (ValidateInputsDmaState) {
	case XLMS_DMA_STATE_PENDING:
		/* Resume after non-blocking DMA completion - skip to post-DMA processing */
		ValidateInputsDmaState = XLMS_DMA_STATE_IDLE;
		break;

	case XLMS_DMA_STATE_IDLE:
		if (LmsSignVerifyParams->SignatureLen > XLMS_LOCAL_SIGNATURE_BUFF_SIZE) {
			Status = XASUFW_LMS_INVALID_PARAM;
			goto END;
		}

		/** Public key must contain at least LMS type (4 bytes) + OTS type (4 bytes) */
		if ((XLMS_TYPE_SIZE + XLMS_OTS_TYPE_SIZE) > LmsSignVerifyParams->LmsHssKeyObj.PubKeyLen) {
			Status = XASUFW_LMS_INVALID_PARAM;
			goto END;
		}

	/**
	 * Skip public key DMA if source is already the local buffer (PreHashedMsg case from HssFinish).
	 * Otherwise, copy public key provided by user to local buffer since 64-bit address space
	 * is not accessible to ASU processor.
	 */
	if (LmsSignVerifyParams->LmsHssKeyObj.PubKeyAddr != (u64)(UINTPTR)AuthenticatedKey.Buff) {
		Status = XAsufw_DmaXfr(DmaPtr, LmsSignVerifyParams->LmsHssKeyObj.PubKeyAddr,
			(u64)(UINTPTR)LmsLocalPubKeyBuff, LmsSignVerifyParams->LmsHssKeyObj.PubKeyLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}
	} else {
		/**
		 * Source is AuthenticatedKey.Buff, copy to LmsLocalPubKeyBuff.
		 * This is a local-to-local copy, use memcpy for efficiency.
		 */
		Status = Xil_SMemCpy((void*)LmsLocalPubKeyBuff, XLMS_PUB_KEY_TOTAL_SIZE,
				(const void *)(UINTPTR)&AuthenticatedKey.Buff,
				LmsSignVerifyParams->LmsHssKeyObj.PubKeyLen,
				LmsSignVerifyParams->LmsHssKeyObj.PubKeyLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_COPY_FAIL;
			goto END;
		}
	}

	/**
	 * RFC 8554 Section 5.4: LMS public key format is u32str(type) || u32str(otstype) || I || T[1].
	 * Extract type (4 bytes) to determine tree parameters.
	 */
	CtxPtr->PubKeyLmsType = (XLms_Type)XAsufw_SwapBytes(
			(const u8*)&LmsLocalPubKeyBuff[XLMS_PUB_KEY_TYPE_OFFSET], XLMS_TYPE_SIZE);

	/** Extract OTS type (4 bytes) which defines Winternitz parameter w and chain configuration */
	CtxPtr->PubKeyLmsOtsType = (XLms_OtsType)XAsufw_SwapBytes(
			(const u8*)&LmsLocalPubKeyBuff[XLMS_PUB_KEY_OTS_TYPE_OFFSET], XLMS_OTS_TYPE_SIZE);

	/** Lookup LMS parameters (tree height, hash output size, etc.) based on type */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XLms_LookupParamSet(CtxPtr->PubKeyLmsType, &CtxPtr->LmsPubKeyParam);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_LMS_PUB_KEY_UNSUPPORTED_TYPE_1_ERROR;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Verify public key length matches: fixed fields + hash output size (m bytes) */
	if ((XLMS_PUB_KEY_FIXED_FIELD_SIZE + CtxPtr->LmsPubKeyParam->HashOutputBytes) !=
		LmsSignVerifyParams->LmsHssKeyObj.PubKeyLen) {
		Status = XASUFW_LMS_SIGN_EXPECTED_PUB_KEY_LEN_2_ERROR;
		goto END;
	}

	/** Signature must contain at least q field (4 bytes) + OTS type (4 bytes) */
	if ((XLMS_Q_FIELD_SIZE + XLMS_OTS_TYPE_SIZE) > LmsSignVerifyParams->SignatureLen) {
		Status = XASUFW_LMS_SIGN_LEN_1_ERROR;
		goto END;
	}

	/**
	 * Skip signature DMA if source is already the local buffer (PreHashedMsg case from HssFinish).
	 * Otherwise, DMA copy signature to local buffer.
	 * For large signatures from external memory (> XASUFW_DMA_BLOCKING_SIZE), use non-blocking DMA.
	 */
	if (LmsSignVerifyParams->SignatureAddr != (u64)(UINTPTR)LmsLocalSignatureBuff) {
		if (LmsSignVerifyParams->SignatureLen > XASUFW_DMA_BLOCKING_SIZE) {
			Status = XAsufw_StartDmaXfr(DmaPtr, LmsSignVerifyParams->SignatureAddr,
				(u64)(UINTPTR)LmsLocalSignatureBuff, LmsSignVerifyParams->SignatureLen, 0U);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_DMA_COPY_FAIL;
				goto END;
			}
			ValidateInputsDmaState = XLMS_DMA_STATE_PENDING;
			Status = XASUFW_CMD_IN_PROGRESS;
		} else {
			Status = XAsufw_DmaXfr(DmaPtr, LmsSignVerifyParams->SignatureAddr,
				(u64)(UINTPTR)LmsLocalSignatureBuff, LmsSignVerifyParams->SignatureLen, 0U);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_DMA_COPY_FAIL;
				goto END;
			}
		}
	}
	/* When SignatureAddr == LmsLocalSignatureBuff, signature is already in place (from HssFinish) */
		break;

	default:
		Status = XASUFW_LMS_INVALID_PARAM;
		goto END;
	}

	if (Status != XASUFW_CMD_IN_PROGRESS) {
		/**
		 * For non-pre-hashed messages, extract q and I for digest prefix.
		 * q = leaf node index in the Merkle tree
		 * I = unique identifier for this LMS key pair
		 */
		if (XASU_FALSE == LmsSignVerifyParams->PreHashedMsg) {
			/** Extract q from signature (Big Endian to Little Endian) */
			CurrentLmsQ = XAsufw_SwapBytes(
					&LmsLocalSignatureBuff[XLMS_SIGNATURE_Q_FIELD_OFFSET], XLMS_Q_FIELD_SIZE);
			/* Convert q back to BE for digest calculation */
			DigestPrefixFields.Fields.q = Xil_Htonl(CurrentLmsQ);

				/** Copy I from public key to digest prefix buffer */
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = Xil_SMemCpy((void*)DigestPrefixFields.Fields.DigestTreeId, XLMS_I_FIELD_SIZE,
					(void*)&LmsLocalPubKeyBuff[XLMS_PUB_KEY_I_OFFSET], XLMS_I_FIELD_SIZE,
					XLMS_I_FIELD_SIZE);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_MEM_COPY_FAIL;
				goto END;
			}
		}

		/**
		 * RFC 8554 Section 5.4.2: LMS signature structure is u32str(q) || lmots_signature || u32str(type) || path[0..h-1].
		 * Extract OTS type from within the lmots_signature portion (after OTS type field offset).
		 */
		SignLmsOtsType = (XLms_OtsType)XAsufw_SwapBytes(
				&LmsLocalSignatureBuff[XLMS_SIGNATURE_OTS_FIELD_OFFSET + XLMS_OTS_SIGN_TYPE_FIELD_OFFSET],
				XLMS_OTS_TYPE_SIZE);

			/** RFC 8554 Section 5.4.2 Step 1: Signature OTS type must match public key's expected OTS type */
		if (CtxPtr->PubKeyLmsOtsType != SignLmsOtsType) {
			Status = XASUFW_LMS_OTS_PUB_KEY_LMS_OTS_SIGN_TYPE_MISMATCH_ERROR;
			goto END;
		}

		/** Lookup OTS parameters (Winternitz parameter w, chain count p.) */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XLms_OtsLookupParamSet(SignLmsOtsType,
				&CtxPtr->LmsOtsSignParam);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_LMS_SIGN_UNSUPPORTED_OTS_TYPE_1_ERROR;
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		/** Verify signature contains complete OTS signature portion */
		if ((CtxPtr->LmsOtsSignParam->SignLen + XLMS_Q_FIELD_SIZE + XLMS_TYPE_SIZE) >
			LmsSignVerifyParams->SignatureLen) {
			Status = XASUFW_LMS_SIGN_LEN_2_ERROR;
			goto END;
		}

		/**
		 * RFC 8554 Section 5.4.2: LMS type appears after the complete OTS signature.
		 * Offset = OTS signature length + q field (4 bytes). Type identifies tree height h.
		 */
		SignLmsType = (XLms_Type)XAsufw_SwapBytes(
				(const u8*)&LmsLocalSignatureBuff[(CtxPtr->LmsOtsSignParam->SignLen +
				XLMS_Q_FIELD_SIZE)], XLMS_TYPE_SIZE);

		/** RFC 8554 Section 5.4.2 Step 1: Signature LMS type must match public key type */
		if (CtxPtr->PubKeyLmsType != SignLmsType) {
			Status = XASUFW_LMS_PUB_KEY_LMS_SIGN_TYPE_MISMATCH_ERROR;
			goto END;
		}

		/** Lookup LMS signature parameters (tree height for auth path depth) */
		Status = XLms_LookupParamSet(SignLmsType, &CtxPtr->LmsSignParam);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_LMS_SIGN_UNSUPPORTED_TYPE_1_ERROR;
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		/**
		 * Validate node number q is within valid tree range.
		 * For tree height h, valid leaf indices are 0 to (2^h - 1).
		 */
		if ((((u32)XASUFW_VALUE_ONE) << (u32)(CtxPtr->LmsSignParam->TreeHeight)) <= CurrentLmsQ) {
			Status = XASUFW_LMS_SIGN_INVALID_NODE_NUMBER_ERROR;
			goto END;
		}

		/**
		 * Validate total signature length matches expected:
		 * OTS signature + q field (4B) + LMS type (4B) + auth path (m*h bytes)
		 */
		if (LmsSignVerifyParams->SignatureLen != (CtxPtr->LmsOtsSignParam->SignLen + XLMS_Q_FIELD_SIZE +
			XLMS_TYPE_SIZE + CtxPtr->LmsSignParam->SignatureLenBytes)) {
			Status = XASUFW_LMS_SIGN_LEN_3_ERROR;
			XFIH_GOTO(END);
		}

		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function computes the Merkle tree root by traversing the authentication path.
*
* @param	ShaInstPtr	Pointer to SHA instance
* @param	DmaPtr		Pointer to DMA instance
* @param	CtxPtr		Pointer to verification context
* @param	Path		Pointer to authentication path in signature
* @param	StartNodeNum	Starting node number (leaf node)
*
* @return
*		- XASUFW_SUCCESS - Merkle root computed and verified successfully
*		- XASUFW_MEM_COPY_FAIL - Buffer copy failed
*		- XASUFW_LMS_PUB_KEY_AUTHENTICATION_FAILED_ERROR - Root does not match public key
*		- XASUFW_LMS_PUB_KEY_AUTHENTICATION_GLITCH_ERROR - Root comparison glitch detected
**************************************************************************************************/
static s32 XLms_ComputeMerkleRoot(XSha *ShaInstPtr, XAsufw_Dma *DmaPtr,
	const XLms_SignVerifyCtx *CtxPtr, const u8 *Path, u32 StartNodeNum)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 CurrentNodeNum = StartNodeNum;
	volatile u32 PathIndex = 0U;
	volatile u32 Index = 0U;
	u32 m = XLMS_M_BYTE_FIELD_SIZE;
	static const u8 XLMS_D_INTR[XLMS_D_FIELD_SIZE] = {XLMS_D_INTR_BYTE, XLMS_D_INTR_BYTE};
	XAsu_ShaOperationCmd ShaParamsInput;

	m = CtxPtr->LmsPubKeyParam->HashOutputBytes;

	/* Configure SHA parameters */
	ShaParamsInput.HashAddr = (u64)(UINTPTR)LmsTmpBuff;
	ShaParamsInput.HashBufSize = XLMS_M_BYTE_FIELD_SIZE;
	ShaParamsInput.ShaMode = (u8)CtxPtr->LmsOtsSignParam->HashAlgId;

	/**
	 * RFC 8554 Section 5.4.2 Step 6b: Set domain separator D_INTR (0x8383) to distinguish
	 * internal node hashing from leaf and public key hashing operations.
	 */
	LmsPubKeyTmpBuff.Fields.D[XASUFW_BUFFER_INDEX_ZERO] = XLMS_D_INTR[XASUFW_BUFFER_INDEX_ZERO];
	LmsPubKeyTmpBuff.Fields.D[XASUFW_BUFFER_INDEX_ONE] = XLMS_D_INTR[XASUFW_BUFFER_INDEX_ONE];

	ShaParamsInput.DataAddr = (u64)(UINTPTR)LmsPubKeyTmpBuff.Buff;
	ShaParamsInput.DataSize = (XLMS_I_FIELD_SIZE + XLMS_MERKLE_TREE_NODE_NUMBER_SIZE +
					XLMS_D_INTR_FIELD_SIZE + (2U * m));

	/**
	 * Traverse the Merkle tree from leaf to root.
	 * At each level, combine the current node value with its sibling from the
	 * authentication path to compute the parent node hash.
	 * Odd nodes are right children (sibling on left), even nodes are left children
	 * (sibling on right).
	 */
	while (CurrentNodeNum > XLMS_ROOT_NODE_NUM) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		/* RFC 8554 Section 5.3.3: Compute parent node as u32str(node_num/2) */
		LmsPubKeyTmpBuff.Fields.half_node_number = Xil_Htonl((CurrentNodeNum) / XASUFW_EVEN_MODULUS);

		if (XASUFW_IS_ODD(CurrentNodeNum)) {
			/**
			 * Odd node: current node is right child (index 2r+1), sibling (path[i]) is left child (index 2r).
			 * RFC 8554 Section 5.4.2 Step 6b: H(I || u32str(node_num/2) || D_INTR || path[i] || tmp)
			 */
			Status = Xil_SMemCpy((void*)LmsPubKeyTmpBuff.Fields.Tmp,
				XLMS_M_BYTE_FIELD_SIZE, (const void*)&Path[PathIndex], m,
				m);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_MEM_COPY_FAIL;
				goto END;
			}
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = Xil_SMemCpy(
				(void*)&LmsPubKeyTmpBuff.Fields.Tmp[m],
				XLMS_M_BYTE_FIELD_SIZE, (void*)LmsTmpBuff, m,
				m);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_MEM_COPY_FAIL;
				goto END;
			}
		}
		else {
			/**
			 * Even node: current node is left child (index 2r), sibling (path[i]) is right child (index 2r+1).
			 * RFC 8554 Section 5.4.2 Step 6b: H(I || u32str(node_num/2) || D_INTR || tmp || path[i])
			 */
			Status = Xil_SMemCpy((void*)LmsPubKeyTmpBuff.Fields.Tmp,
				XLMS_M_BYTE_FIELD_SIZE, (void*)LmsTmpBuff, m,
				m);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_MEM_COPY_FAIL;
				goto END;
			}
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = Xil_SMemCpy(
				(void*)&LmsPubKeyTmpBuff.Fields.Tmp[m],
				XLMS_M_BYTE_FIELD_SIZE, (const void*)&Path[PathIndex], m,
				m);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_MEM_COPY_FAIL;
				goto END;
			}
		}

		/* RFC 8554 Section 5.4.2 Step 6b: Compute H(I || r || D_INTR || left_child || right_child) */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Digest(ShaInstPtr, DmaPtr, &ShaParamsInput);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA_DIGEST_CALC_FAILED);
			goto END;
		}

		/* Move up one level: parent becomes current node, advance to next sibling in auth path */
		CurrentNodeNum = CurrentNodeNum / XASUFW_EVEN_MODULUS;
		PathIndex += m;
	}

	/**
	 * Compare computed Merkle root (in LmsTmpBuff) with expected root from public key.
	 * The T[1] field in the public key contains the expected Merkle tree root.
	 * Use constant-time comparison to prevent timing side-channel attacks.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCmp_CT(LmsTmpBuff, XLMS_PUB_KEY_T_FIELD_SIZE,
			&LmsLocalPubKeyBuff[XLMS_PUB_KEY_T_OFFSET],
			m, m);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_LMS_PUB_KEY_AUTHENTICATION_FAILED_ERROR;
		goto END;
	}

END:
	return Status;
}
#endif /* XASU_LMS_ENABLE */
/** @} */
