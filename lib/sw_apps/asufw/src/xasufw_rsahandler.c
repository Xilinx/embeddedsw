/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_rsahandler.c
 *
 * This file contains the RSA module commands supported by ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   08/20/24 Initial release
 *       ss   09/26/24 Fixed doxygen comments
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_rsahandler.h"
#include "xrsa.h"
#include "xasufw_modules.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_cmd.h"
#include "xasufw_kat.h"
#include "xasu_def.h"
#include "xasu_rsainfo.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xfih.h"
#include "xsha_hw.h"
#include "xasufw_trnghandler.h"

/************************************ Constant Definitions ***************************************/
#define XRSA_MAX_DB_LEN		(479U)	/**< RSA max data block size in bytes */
#define XRSA_PSS_MAX_MSG_LEN	(550U)	/**< RSA PSS padding max message length */
#define XRSA_PSS_MAX_SALT_LEN	(478U)	/**< RSA PSS padding max salt length */

/************************************** Type Definitions *****************************************/
/** Structure has input and output parameters used for MGF */
typedef struct {
	u8 *Seed;      /**< Input seed on which mask should be generated */
	u8 *Output;    /**< Buffer to store the mask */
	u32 SeedLen;   /**< Seed length */
	u32 OutputLen; /**< Buffer length */
} XAsufw_MgfInput;

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASUFW_RSA_OAEP_OUTPUT_DATA_BLOCK_FIRST_INDEX		(0X00U)
#define XASUFW_RSA_OAEP_OUTPUT_DATA_BLOCK_FIRST_INDEX_VALUE	(0X00U)
#define XASUFW_RSA_OAEP_SECOND_DATA_BLOCK_VALUE			(0X01U)
#define XASUFW_RSA_OAEP_OUTPUT_DATA_BLOCK_HASH_INDEX		(0X01U)
#define XASUFW_RSA_OAEP_OUTPUT_DATA_BLOCK_ONE_SEPERATION_VALUE	(0X01U)
#define XASUFW_RSA_PKCS_MIN_DATA_BLOCK_LENGTH			(0X0BU)
#define XASUFW_RSA_PKCS_DATA_BLOCK_LENGTH_EXCL_RANDOM_STRING	(0X03U)
#define XASUFW_RSA_PKCS_OUTPUT_DATA_BLOCK_FIRST_INDEX		(0X00U)
#define XASUFW_RSA_PKCS_OUTPUT_DATA_BLOCK_SECOND_INDEX		(0X01U)
#define XASUFW_RSA_PKCS_OUTPUT_DATA_BLOCK_RAND_NUM_INDEX	(0X02U)
#define XASUFW_RSA_PKCS_OUTPUT_DATA_BLOCK_FIRST_INDEX_VALUE	(0X00U)
#define XASUFW_RSA_PKCS_OUTPUT_DATA_BLOCK_SECOND_INDEX_VALUE	(0X02U)
#define XASUFW_RSA_PKCS_OUTPUT_DATA_BLOCK_ZERO_SEPERATION_VALUE	(0X00U)
#define XASUFW_RSA_PSS_DATA_BLOCK_FIRST_INDEX			(0X00U)
#define XASUFW_RSA_PSS_DATA_BLOCK_SECOND_INDEX			(0X01U)
#define XASUFW_RSA_PSS_DATA_BLOCK_FIRST_INDEX_VALUE		(0X01U)
#define XASUFW_RSA_PSS_HASH_BLOCK_ZEROIZE_LEN			(0X08U)
#define XASUFW_RSA_PSS_MSB_PADDING_MASK				(0X7FU)
#define XASUFW_RSA_PSS_MSB_PADDING_CHECK_MASK			(0X80U)
#define XASUFW_RSA_PSS_OUTPUT_END_BYTE_VALUE			(0XBCU)

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_RsaKat(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaGetInfo(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaPubEnc(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaPvtDec(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaPvtCrtDec(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaOaepEncDecSha2(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaOaepEncDecSha3(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaPkcsEncDec(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaPssSignGenVerSha2(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaPssSignGenVerSha3(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaPkcsSignGenVerSha2(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaPkcsSignGenVerSha3(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_OaepEncode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, u32 Len,
			u32 KeySize, u64 InputDataAddr, u64 OptionalLabelAddr,
			u32 OptionalLabelSize, u8 *PaddedOutputData, u8 ShaType);
static s32 XAsufw_OaepDecode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, u32 Len,
			u32 KeySize, u8 *PaddedInputData, u64 OptionalLabelAddr,
			u32 OptionalLabelSize, u64 OutputDataAddr, u8 ShaType);
static s32 XAsufw_PkcsEncode(XAsufw_Dma *DmaPtr, u32 Len, u32 KeySize, u64 InputDataAddr,
			u8 *PaddedOutputData);
static s32 XAsufw_PkcsDecode(XAsufw_Dma *DmaPtr, u32 Len, u32 KeySize, u8 *PaddedInputData,
			u64 OutputDataAddr);
static s32 XAsufw_PssEncode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, u32 Len,
			u32 KeySize, u32 SaltLen, u8 *InputData, u8 *PaddedOutputData,
			u8 ShaType);
static s32 XAsufw_PssDecode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, u32 Len,
			u32 KeySize, u32 SaltLen, u8 *PaddedInputData, u64 InputDataAddr,
			u8 ShaType);
static s32 XAsufw_MaskGenFunc(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, u8 ShaType,
				XAsufw_MgfInput *MgfInput);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_RsaModule; /**< ASUFW RSA Module ID and commands array */

/*************************************************************************************************/
/**
 * @brief	This function initializes the RSA module.
 *
 * @return
 * 	- On successful initialization of RSA module, it returns XASUFW_SUCCESS.
 * 	- XASUFW_RSA_MODULE_REGISTRATION_FAILED, if RSA module registration fails.
 *
 *************************************************************************************************/
s32 XAsufw_RsaInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/** Contains the array of ASUFW RSA commands. */
	static const XAsufw_ModuleCmd XAsufw_RsaCmds[] = {
		[XASU_RSA_PUB_ENC_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPubEnc),
		[XASU_RSA_PVT_DEC_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPvtDec),
		[XASU_RSA_PVT_CRT_DEC_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPvtCrtDec),
		[XASU_RSA_OAEP_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaOaepEncDecSha2),
		[XASU_RSA_OAEP_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaOaepEncDecSha3),
		[XASU_RSA_PKCS_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPkcsEncDec),
		[XASU_RSA_PSS_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPssSignGenVerSha2),
		[XASU_RSA_PSS_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPssSignGenVerSha3),
		[XASU_RSA_PKCS_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPkcsSignGenVerSha2),
		[XASU_RSA_PKCS_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPkcsSignGenVerSha3),
		[XASU_RSA_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaKat),
		[XASU_RSA_GET_INFO_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaGetInfo),
	};

	/** Contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_RsaResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_RsaCmds)] = {
		[XASU_RSA_PUB_ENC_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK,
		[XASU_RSA_PVT_DEC_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_PVT_CRT_DEC_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_OAEP_SHA2_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_OAEP_SHA3_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA3_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_PKCS_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_PSS_SHA2_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_PSS_SHA3_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA3_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_PKCS_SHA2_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK|
		XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_PKCS_SHA3_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA3_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK,
		[XASU_RSA_GET_INFO_CMD_ID] = 0U,
	};

	XAsufw_RsaModule.Id = XASU_MODULE_RSA_ID;
	XAsufw_RsaModule.Cmds = XAsufw_RsaCmds;
	XAsufw_RsaModule.ResourcesRequired = XAsufw_RsaResourcesBuf;
	XAsufw_RsaModule.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_RsaCmds);

	/** Register RSA module. */
	Status = XAsufw_ModuleRegister(&XAsufw_RsaModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MODULE_REGISTRATION_FAILED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA encryption operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if public encryption operation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_RSA_PUB_OP_ERROR, if public encryption operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPubEnc(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaClientParams *Cmd = (const XAsu_RsaClientParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;

	/** Check resource availability (DMA and RSA) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto RET;
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);

	/** Perform public exponentiation encryption operation. */
	Status = XRsa_PubExp(AsuDmaPtr, Cmd->Len, Cmd->InputDataAddr, Cmd->OutputDataAddr,
			     Cmd->KeyCompAddr, Cmd->ExpoCompAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PUB_OP_ERROR);
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA decryption operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if private decryption operation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_RSA_PVT_OP_ERROR, if private exponentiation decryption operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPvtDec(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaClientParams *Cmd = (const XAsu_RsaClientParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;

	/** Check resource availability (DMA,RSA and TRNG) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto RET;
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);
	XAsufw_AllocateResource(XASUFW_TRNG, QueueId);

	/** Perform private exponentiation decryption operation. */
	Status = XRsa_PvtExp(AsuDmaPtr, Cmd->Len, Cmd->InputDataAddr, Cmd->OutputDataAddr,
			     Cmd->KeyCompAddr, Cmd->ExpoCompAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PVT_OP_ERROR);
	}

	/** Release resources. */
	if ((XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS) ||
	    (XAsufw_ReleaseResource(XASUFW_TRNG, QueueId) != XASUFW_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA decryption operation command using CRT
 * 		algorithm.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if private decryption operation using CRT is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_RSA_CRT_OP_ERROR, if private CRT decryption operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPvtCrtDec(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaClientParams *Cmd = (const XAsu_RsaClientParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;

	/** Check resource availability (DMA,RSA and TRNG) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto RET;
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);
	XAsufw_AllocateResource(XASUFW_TRNG, QueueId);

	/** Perform private CRT decryption operation. */
	Status = XRsa_CrtOp(AsuDmaPtr, Cmd->Len, Cmd->InputDataAddr, Cmd->OutputDataAddr,
			    Cmd->KeyCompAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_CRT_OP_ERROR);
	}

	/** Release resources. */
	if ((XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS) ||
	    (XAsufw_ReleaseResource(XASUFW_TRNG, QueueId) != XASUFW_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA OAEP padding for the provided message to output a data
 * 		of length equal to keysize.
 *
 * @param	DmaPtr			Pointer to the AsuDma instance.
 * @param	ShaInstancePtr		Pointer to the Sha instance.
 * @param	Len			Length of Input and Output Data in bytes.
 * @param	KeySize			Length of public key in bytes.
 * @param	InputDataAddr		Address of the input data buffer.
 * @param	OptionalLabelAddr	Address of the optional label data buffer.
 * @param	OptionalLabelSize	Length of optional label data in bytes.
 * @param	PaddedOutputData	Pointer of the output data buffer.
 * @param	ShaType			SHA mode selection.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 *		- Also can return termination error codes from 0xB2U to 0xB7U and 0xA8U,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaOaepEncode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, u32 Len,
			u32 KeySize, u64 InputDataAddr, u64 OptionalLabelAddr,
			u32 OptionalLabelSize, u8 *PaddedOutputData, u8 ShaType)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u32 HashLen = 0U;
	u32 Index = 0U;
	u8 *DataBlock = XRsa_GetDataBlockAddr();
	u8 *MaskedDataBlock = DataBlock + XRSA_MAX_DB_LEN;
	u8 *SeedBuffer = MaskedDataBlock + XRSA_MAX_DB_LEN;
	u8 *MaskSeedBuffer = SeedBuffer + XASU_SHAKE_256_MAX_HASH_LEN;
	XAsufw_MgfInput MgfInput;

	/** Validate the input arguments. */
	if ((InputDataAddr == 0U) || (OptionalLabelAddr == 0U) || (PaddedOutputData == NULL) ||
		(DmaPtr == NULL) || (ShaInstancePtr == NULL)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_GetHashLen(ShaType, &HashLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if (Len > (KeySize - (2U * HashLen) - 2U)) {
		Status = XASUFW_RSA_OAEP_INVALID_LEN;
		goto END;
	}

	/** Calculate digest for optional label and arrange data block. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Start(ShaInstancePtr, ShaType);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Update(ShaInstancePtr, DmaPtr, OptionalLabelAddr, OptionalLabelSize, TRUE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Finish(ShaInstancePtr, (u64)(UINTPTR)DataBlock, HashLen,
				XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if ((KeySize - Len - (2U * HashLen) - 2U) != 0U) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemSet(&DataBlock[HashLen], XRSA_MAX_DB_LEN, 0U,
				(KeySize - Len - (2U * HashLen) - 2U));
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ZEROIZE_MEMSET_FAIL;
			goto END;
		}
	}

	DataBlock[KeySize - Len - HashLen - 2U] = XASUFW_RSA_OAEP_SECOND_DATA_BLOCK_VALUE;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, InputDataAddr,
			(u64)(UINTPTR)&DataBlock[KeySize - Len - HashLen - 1U], Len, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_TrngGetRandomNumbers(SeedBuffer, HashLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	MgfInput.Output = MaskedDataBlock;
	MgfInput.OutputLen = (KeySize - HashLen - 1U);
	MgfInput.Seed = SeedBuffer;
	MgfInput.SeedLen = HashLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_MaskGenFunc(DmaPtr, ShaInstancePtr, ShaType, &MgfInput);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MASK_GEN_DATA_BLOCK_ERROR);
		goto END;
	}

	for (Index = 0U; Index < (KeySize - HashLen - 1U); Index++) {
		DataBlock[Index] ^= MaskedDataBlock[Index];
	}

	MgfInput.Output = MaskSeedBuffer;
	MgfInput.OutputLen = HashLen;
	MgfInput.Seed = DataBlock;
	MgfInput.SeedLen = (KeySize - HashLen - 1U);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_MaskGenFunc(DmaPtr, ShaInstancePtr, ShaType, &MgfInput);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MASK_GEN_SEED_BUFFER_ERROR);
		goto END;
	}

	for (Index = 0U; Index < HashLen; Index++) {
		SeedBuffer[Index] ^= MaskSeedBuffer[Index];
	}

	PaddedOutputData[XASUFW_RSA_OAEP_OUTPUT_DATA_BLOCK_FIRST_INDEX] = XASUFW_RSA_OAEP_OUTPUT_DATA_BLOCK_FIRST_INDEX_VALUE;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)SeedBuffer,
			(u64)(UINTPTR)&PaddedOutputData[XASUFW_RSA_OAEP_OUTPUT_DATA_BLOCK_HASH_INDEX], HashLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)DataBlock,
			(u64)(UINTPTR)&PaddedOutputData[HashLen + 1U], (KeySize - HashLen - 1U), 0U);

END:
	/** Zeroize local copy of all the parameters. */
	SStatus = XAsufw_DmaMemSet(DmaPtr, (u32)(UINTPTR)DataBlock, 0U, XRSA_MAX_KEY_SIZE_IN_BYTES *
					XRSA_TOTAL_PARAMS);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA OAEP decode operation for the provided decrypted data
 * 		to extract the original message.
 *
 * @param	DmaPtr			Pointer to the AsuDma instance.
 * @param	ShaInstancePtr		Pointer to the Sha instance.
 * @param	Len			Length of Input and Output Data in bytes.
 * @param	KeySize			Length of public key in bytes.
 * @param	PaddedInputData		Pointer of the input data buffer.
 * @param	OptionalLabelAddr	Address of the optional label data buffer.
 * @param	OptionalLabelSize	Length of optional label data in bytes.
 * @param	OutputDataAddr		Address of the output data buffer.
 * @param	ShaType			SHA mode selection.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 *		- Also can return termination error codes from 0x9CU to 0x9EU and 0xA1U,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaOaepDecode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, u32 Len,
			u32 KeySize, u8 *PaddedInputData, u64 OptionalLabelAddr,
			u32 OptionalLabelSize, u64 OutputDataAddr, u8 ShaType)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u32 HashLen = 0U;
	u32 Index = 0U;
	u32 DataBlockLen = 0U;
	u32 MsgLen = 0U;
	u8 *DataBlock = XRsa_GetDataBlockAddr();
	u8 *MaskedDataBlock = DataBlock + XRSA_MAX_DB_LEN;
	u8 *SeedBuffer = MaskedDataBlock + XRSA_MAX_DB_LEN;
	u8 *MaskedSeedBuffer = SeedBuffer + XASU_SHAKE_256_MAX_HASH_LEN;
	u8 *HashBuffer = MaskedSeedBuffer + XASU_SHAKE_256_MAX_HASH_LEN;
	XAsufw_MgfInput MgfInput;

	/** Validate the input arguments. */
	if ((OutputDataAddr == 0U) || (OptionalLabelAddr == 0U) || (PaddedInputData == NULL) ||
		(DmaPtr == NULL) || (ShaInstancePtr == NULL)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_GetHashLen(ShaType, &HashLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if ((Len != KeySize) || (Len < (2U * HashLen) + 2U)) {
		Status = XASUFW_RSA_OAEP_INVALID_LEN;
		goto END;
	}

	DataBlockLen = (KeySize - HashLen - 1U);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Start(ShaInstancePtr, ShaType);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Update(ShaInstancePtr, DmaPtr, OptionalLabelAddr, OptionalLabelSize, TRUE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Finish(ShaInstancePtr, (u64)(UINTPTR)HashBuffer, HashLen,
				XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if (PaddedInputData[XASUFW_RSA_OAEP_OUTPUT_DATA_BLOCK_FIRST_INDEX]
		== XASUFW_RSA_OAEP_OUTPUT_DATA_BLOCK_FIRST_INDEX_VALUE) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr,
				(u64)(UINTPTR)&PaddedInputData[XASUFW_RSA_OAEP_OUTPUT_DATA_BLOCK_HASH_INDEX],
				(u64)(UINTPTR)MaskedSeedBuffer, HashLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)&PaddedInputData[HashLen + 1U],
				(u64)(UINTPTR)MaskedDataBlock, DataBlockLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	} else {
		Status = XASUFW_RSA_OAEP_OCTET_ONE_CMP_FAIL;
		goto END;
	}

	MgfInput.Output = SeedBuffer;
	MgfInput.OutputLen = HashLen;
	MgfInput.Seed = MaskedDataBlock;
	MgfInput.SeedLen = DataBlockLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_MaskGenFunc(DmaPtr, ShaInstancePtr, ShaType, &MgfInput);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MASK_GEN_SEED_BUFFER_ERROR);
		goto END;
	}
	for (Index = 0U; Index < HashLen; Index++) {
		SeedBuffer[Index] ^= MaskedSeedBuffer[Index];
	}

	MgfInput.Output = DataBlock;
	MgfInput.OutputLen = (KeySize - HashLen - 1U);
	MgfInput.Seed = SeedBuffer;
	MgfInput.SeedLen = HashLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_MaskGenFunc(DmaPtr, ShaInstancePtr, ShaType, &MgfInput);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MASK_GEN_DATA_BLOCK_ERROR);
		goto END;
	}
	for (Index = 0U; Index < (KeySize - HashLen - 1U); Index++) {
		DataBlock[Index] ^= MaskedDataBlock[Index];
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCmp(DataBlock, XRSA_MAX_DB_LEN, HashBuffer, XASU_SHAKE_256_MAX_HASH_LEN,
				HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_OAEP_HASH_CMP_FAIL;
		goto END;
	}

	if (DataBlock[HashLen] == XASUFW_RSA_OAEP_SECOND_DATA_BLOCK_VALUE) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)&DataBlock[HashLen + 1U],
				OutputDataAddr, DataBlockLen, 0U);
	}
	else {
		Index = HashLen;
		while ((DataBlock[Index] != XASUFW_RSA_OAEP_OUTPUT_DATA_BLOCK_ONE_SEPERATION_VALUE) || (Index == DataBlockLen)) {
			Index++;
		}
		if (Index == Len) {
			Status = XASUFW_RSA_OAEP_ONE_SEP_CMP_FAIL;
		} else {
			Index++;
			MsgLen = DataBlockLen - Index;
			if (MsgLen > (KeySize - (2U * HashLen) - 2U)) {
				Status = XASUFW_RSA_OAEP_INVALID_LEN;
			} else {
				ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
				Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)(&(DataBlock[Index])),
						OutputDataAddr, MsgLen, 0U);
			}
		}
	}

END:
	/** Zeroize local copy of all the parameters. */
	SStatus = XAsufw_DmaMemSet(DmaPtr, (u32)(UINTPTR)DataBlock, 0U, XRSA_MAX_KEY_SIZE_IN_BYTES *
					XRSA_TOTAL_PARAMS);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA PKCS encode operation for provided message to output
 * 		a data of length equal to keysize.
 *
 * @param	DmaPtr			Pointer to the AsuDma instance.
 * @param	Len			Length of Input and Output Data in bytes.
 * @param	KeySize			Length of public key in bytes.
 * @param	InputDataAddr		Address of the intput data buffer.
 * @param	PaddedOutputData	Pointer of the output data buffer.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 *		- Also can return termination error codes from 0x9CU to 0x9EU and 0xA1U,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPkcsEncode(XAsufw_Dma *DmaPtr, u32 Len, u32 KeySize, u64 InputDataAddr,
			u8 *PaddedOutputData)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate the input arguments. */
	if ((InputDataAddr == 0U) || (PaddedOutputData == NULL) || (DmaPtr == NULL)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	if (Len > (KeySize - XASUFW_RSA_PKCS_MIN_DATA_BLOCK_LENGTH)) {
		Status = XASUFW_RSA_PKCS_INVALID_LEN;
		goto END;
	}

	PaddedOutputData[XASUFW_RSA_PKCS_OUTPUT_DATA_BLOCK_FIRST_INDEX] = XASUFW_RSA_PKCS_OUTPUT_DATA_BLOCK_FIRST_INDEX_VALUE;
	PaddedOutputData[XASUFW_RSA_PKCS_OUTPUT_DATA_BLOCK_SECOND_INDEX] = XASUFW_RSA_PKCS_OUTPUT_DATA_BLOCK_SECOND_INDEX_VALUE;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_TrngGetRandomNumbers(&PaddedOutputData[XASUFW_RSA_PKCS_OUTPUT_DATA_BLOCK_RAND_NUM_INDEX],
						(KeySize - Len - XASUFW_RSA_PKCS_DATA_BLOCK_LENGTH_EXCL_RANDOM_STRING));
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	PaddedOutputData[KeySize - Len - 1U] = XASUFW_RSA_PKCS_OUTPUT_DATA_BLOCK_ZERO_SEPERATION_VALUE;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, InputDataAddr,
			(u64)(UINTPTR)(&PaddedOutputData[KeySize - Len]), Len, 0U);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA PKCS decode operation for the provided decrypted data
 * 		to extract the original message.
 *
 * @param	DmaPtr			Pointer to the AsuDma instance.
 * @param	Len			Length of Input and Output Data in bytes.
 * @param	KeySize			Length of public key in bytes.
 * @param	PaddedInputData		Pointer of the input data buffer.
 * @param	OutputDataAddr		Address of the output data buffer.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 *		- Also can return termination error codes from 0x9CU to 0x9EU and 0xA1U,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPkcsDecode(XAsufw_Dma *DmaPtr, u32 Len, u32 KeySize, u8 *PaddedInputData,
			u64 OutputDataAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Index = 0U;

	/** Validate the input arguments. */
	if ((PaddedInputData == NULL) || (OutputDataAddr == 0U) || (DmaPtr == NULL)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	if (Len != KeySize) {
		Status = XASUFW_RSA_PKCS_INVALID_LEN;
		goto END;
	}

	if (PaddedInputData[Index] == XASUFW_RSA_PKCS_OUTPUT_DATA_BLOCK_FIRST_INDEX_VALUE) {
		Index++;
		if (PaddedInputData[Index] == XASUFW_RSA_PKCS_OUTPUT_DATA_BLOCK_SECOND_INDEX_VALUE) {
			Index++;
			while ((PaddedInputData[Index] != XASUFW_RSA_PKCS_OUTPUT_DATA_BLOCK_ZERO_SEPERATION_VALUE) || (Index == KeySize)) {
				Index++;
			}
			if (Index == Len) {
				Status = XASUFW_RSA_PKCS_ZERO_SEP_CMP_FAIL;
			} else {
				Index++;
				ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
				Status = XAsufw_DmaXfr(DmaPtr,
						(u64)(UINTPTR)&PaddedInputData[Index],
						OutputDataAddr, Len - Index, 0U);
			}
		} else {
			Status = XASUFW_RSA_PKCS_OCTET_TWO_CMP_FAIL;
		}
	} else {
		Status = XASUFW_RSA_PKCS_OCTET_ONE_CMP_FAIL;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA PSS padding for the provided message to output a data
 * 		of length equal to keysize.
 *
 * @param	DmaPtr			Pointer to the AsuDma instance.
 * @param	ShaInstancePtr		Pointer to the Sha instance.
 * @param	Len			Length of Input and Output Data in bytes.
 * @param	KeySize			Length of public key in bytes.
 * @param	SaltLen			Length of random data to pad in bytes.
 * @param	HashInputData		Pointer of the input data buffer.
 * @param	OutputData		Pointer of the output data buffer.
 * @param	ShaType			SHA mode selection.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 *		- Also can return termination error codes from 0x9CU to 0x9EU and 0xA1U,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPssEncode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, u32 Len,
			u32 KeySize, u32 SaltLen, u8 *HashInputData, u8 *OutputData, u8 ShaType)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u32 HashLen = 0U;
	u32 Index = 0U;
	u8 *DataBlock = XRsa_GetDataBlockAddr();
	u8 *MaskedDataBlock = DataBlock + XRSA_MAX_DB_LEN;
	u8 *HashBlock = MaskedDataBlock + XRSA_MAX_DB_LEN;
	u8 *HashBuffer = HashBlock + XRSA_PSS_MAX_MSG_LEN;
	u8 *InputDataHash = HashBuffer + XASU_SHAKE_256_MAX_HASH_LEN;
	XAsufw_MgfInput MgfInput;

	/** Validate the input arguments. */
	if ((HashInputData == NULL) || (OutputData == NULL) || (DmaPtr == NULL) ||
		(ShaInstancePtr == NULL)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	Status = XAsu_RsaValidateKeySize(KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_GetHashLen(ShaType, &HashLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Start(ShaInstancePtr, ShaType);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Update(ShaInstancePtr, DmaPtr, (u64)(UINTPTR)HashInputData,
				Len, TRUE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Finish(ShaInstancePtr, (u64)(UINTPTR)InputDataHash, HashLen,
				XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(HashBlock, XRSA_PSS_MAX_MSG_LEN, 0U, XASUFW_RSA_PSS_HASH_BLOCK_ZEROIZE_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)InputDataHash,
				(u64)(UINTPTR)&HashBlock[XASUFW_RSA_PSS_HASH_BLOCK_ZEROIZE_LEN],
				HashLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if (SaltLen != 0U) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_TrngGetRandomNumbers(&HashBlock[HashLen + XASUFW_RSA_PSS_HASH_BLOCK_ZEROIZE_LEN],
							SaltLen);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Start(ShaInstancePtr, ShaType);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Update(ShaInstancePtr, DmaPtr, (u64)(UINTPTR)HashBlock,
				HashLen + SaltLen + XASUFW_RSA_PSS_HASH_BLOCK_ZEROIZE_LEN, TRUE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Finish(ShaInstancePtr, (u64)(UINTPTR)HashBuffer, HashLen,
				XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if ((KeySize - SaltLen - HashLen - 2U) != 0U) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemSet(DataBlock, XRSA_MAX_DB_LEN, 0U,
				(KeySize - SaltLen - HashLen - 2U));
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_RSA_ZEROIZE_MEMSET_FAIL;
			goto END;
		}
		DataBlock[KeySize - SaltLen - HashLen - 2U] = XASUFW_RSA_PSS_DATA_BLOCK_FIRST_INDEX_VALUE;
		if (SaltLen != 0U) {
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)&HashBlock[HashLen + XASUFW_RSA_PSS_HASH_BLOCK_ZEROIZE_LEN],
					(u64)(UINTPTR)&DataBlock[KeySize - SaltLen - HashLen - 1U],
					SaltLen, 0U);
		}
	}
	else {
		DataBlock[XASUFW_RSA_PSS_DATA_BLOCK_FIRST_INDEX] = XASUFW_RSA_PSS_DATA_BLOCK_FIRST_INDEX_VALUE;
		if (SaltLen != 0U) {
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)&HashBlock[HashLen + XASUFW_RSA_PSS_HASH_BLOCK_ZEROIZE_LEN],
					(u64)(UINTPTR)&DataBlock[XASUFW_RSA_PSS_DATA_BLOCK_SECOND_INDEX], SaltLen, 0U);
		} else {
			Status = XASUFW_RSA_PSS_NO_SALT_NO_RANDOM_STRING;
		}
	}

	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	MgfInput.Output = MaskedDataBlock;
	MgfInput.OutputLen = (KeySize - HashLen - 1U);
	MgfInput.Seed = HashBuffer;
	MgfInput.SeedLen = HashLen;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_MaskGenFunc(DmaPtr, ShaInstancePtr, ShaType, &MgfInput);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MASK_GEN_DATA_BLOCK_ERROR);
		goto END;
	}
	for (Index = 0U; Index < (KeySize - HashLen - 1U); Index++) {
		DataBlock[Index] ^= MaskedDataBlock[Index];
	}

	DataBlock[XASUFW_RSA_PSS_DATA_BLOCK_FIRST_INDEX] = DataBlock[XASUFW_RSA_PSS_DATA_BLOCK_FIRST_INDEX] & XASUFW_RSA_PSS_MSB_PADDING_MASK;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)DataBlock, (u64)(UINTPTR)OutputData,
				(KeySize - HashLen - 1U), 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)HashBuffer,
				(u64)(UINTPTR)&OutputData[KeySize - HashLen - 1U], HashLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	OutputData[KeySize - 1U] = XASUFW_RSA_PSS_OUTPUT_END_BYTE_VALUE;

END:
	/** Zeroize local copy of all the parameters. */
	SStatus = XAsufw_DmaMemSet(DmaPtr, (u32)(UINTPTR)DataBlock, 0U, XRSA_MAX_KEY_SIZE_IN_BYTES *
					XRSA_TOTAL_PARAMS);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA PSS decode operation and verifies the signature of
 * 		the provided message.
 *
 * @param	DmaPtr			Pointer to the AsuDma instance.
 * @param	ShaInstancePtr		Pointer to the Sha instance.
 * @param	Len			Length of Input and Output Data in bytes.
 * @param	KeySize			Length of public key in bytes.
 * @param	SaltLen			Length of random data to pad in bytes.
 * @param	SignedInputData		Pointer of the input data buffer.
 * @param	InputDataAddr		Address of the input data buffer.
 * @param	ShaType			SHA mode selection.
 *
 * @return
 *		- XASUFW_SUCCESS on success.
 *		- XASUFW_FAILURE on failure.
 *		- XASUFW_RSA_INVALID_PARAM on invalid parameters.
 *		- Also can return termination error codes from 0x9CU to 0x9EU and 0xA1U,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPssDecode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, u32 Len, u32 SignatureLen,
			u32 KeySize, u32 SaltLen, u8* SignedInputData, u64 InputDataAddr,
			u8 ShaType)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	u32 HashLen = 0U;
	u32 Index = 0U;
	u8 *DataBlock = XRsa_GetDataBlockAddr();
	u8 *MaskedDataBlock = DataBlock + XRSA_MAX_DB_LEN;
	u8 *MsgBlock = MaskedDataBlock + XRSA_MAX_DB_LEN;
	u8 *HashBuffer = MsgBlock + XRSA_PSS_MAX_MSG_LEN;
	u8 *MsgHashBuffer = HashBuffer + XASU_SHAKE_256_MAX_HASH_LEN;
	u8 *SaltBuff = MsgHashBuffer + XASU_SHAKE_256_MAX_HASH_LEN;
	u8 *EncodedMsgHashBuffer = SaltBuff + XRSA_PSS_MAX_SALT_LEN;
	XAsufw_MgfInput MgfInput;

	/** Validate the input arguments. */
	if ((InputDataAddr == 0U) || (SignedInputData == NULL) || (DmaPtr == NULL) ||
		(ShaInstancePtr == NULL)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_RsaValidateKeySize(KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	if (SignatureLen != KeySize) {
		Status = XASUFW_RSA_PSS_INVALID_LEN;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_GetHashLen(ShaType, &HashLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if (SignedInputData[KeySize - 1U] == XASUFW_RSA_PSS_OUTPUT_END_BYTE_VALUE) {
		Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)SignedInputData,
					(u64)(UINTPTR)MaskedDataBlock, (KeySize - HashLen -1U), 0U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)&SignedInputData[KeySize - HashLen -1U],
						(u64)(UINTPTR)HashBuffer, HashLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	} else {
		Status = XASUFW_RSA_PSS_RIGHT_MOST_CMP_FAIL;
		goto END;
	}

	if ((MaskedDataBlock[XASUFW_RSA_PSS_DATA_BLOCK_FIRST_INDEX] & XASUFW_RSA_PSS_MSB_PADDING_CHECK_MASK) == 0x00U) {
		MgfInput.Output = DataBlock;
		MgfInput.OutputLen = (KeySize - HashLen - 1U);
		MgfInput.Seed = HashBuffer;
		MgfInput.SeedLen = HashLen;

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_MaskGenFunc(DmaPtr, ShaInstancePtr, ShaType, &MgfInput);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_RSA_MASK_GEN_DATA_BLOCK_ERROR);
			goto END;
		}
		for (Index = 0U; Index < (KeySize - HashLen - 1U); Index++) {
			DataBlock[Index] ^= MaskedDataBlock[Index];
		}
		DataBlock[XASUFW_RSA_PSS_DATA_BLOCK_FIRST_INDEX] = DataBlock[XASUFW_RSA_PSS_DATA_BLOCK_FIRST_INDEX]
									& XASUFW_RSA_PSS_MSB_PADDING_MASK;
	} else {
		Status = XASUFW_RSA_PSS_LEFT_MOST_BIT_CMP_FAIL;
		goto END;
	}

	if ((KeySize - HashLen - SaltLen - 2U) != 0U) {
		for (Index = 0U; Index < (KeySize - HashLen - SaltLen - 2U); Index++) {
			if (DataBlock[Index] != 0U) {
				Status = XASUFW_RSA_PSS_DB_LEFT_MOST_BYTE_CMP_FAIL;
				goto END;
			}
		}
	} else {
		Index = 0U;
	}

	if (DataBlock[Index] != XASUFW_RSA_PSS_DATA_BLOCK_FIRST_INDEX_VALUE) {
		Status = XASUFW_RSA_PSS_DB_BYTE_ONE_CMP_FAIL;
		goto END;
	}

	if (SaltLen != 0U) {
		Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)&DataBlock[Index + 1U],
					(u64)(UINTPTR)SaltBuff, SaltLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Start(ShaInstancePtr, ShaType);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Update(ShaInstancePtr, DmaPtr, InputDataAddr, Len, TRUE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Finish(ShaInstancePtr, (u64)(UINTPTR)MsgHashBuffer, HashLen,
				XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemSet(MsgBlock, XRSA_PSS_MAX_MSG_LEN, 0U, XASUFW_RSA_PSS_HASH_BLOCK_ZEROIZE_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)MsgHashBuffer, (u64)(UINTPTR)&MsgBlock[XASUFW_RSA_PSS_HASH_BLOCK_ZEROIZE_LEN],
				HashLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if (SaltLen != 0U) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)SaltBuff,
					(u64)(UINTPTR)&MsgBlock[HashLen + XASUFW_RSA_PSS_HASH_BLOCK_ZEROIZE_LEN], SaltLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Start(ShaInstancePtr, ShaType);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Update(ShaInstancePtr, DmaPtr, (u64)(UINTPTR)MsgBlock,
				HashLen + SaltLen + XASUFW_RSA_PSS_HASH_BLOCK_ZEROIZE_LEN, TRUE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Finish(ShaInstancePtr, (u64)(UINTPTR)EncodedMsgHashBuffer,
				HashLen, XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCmp(HashBuffer, XASU_SHAKE_256_MAX_HASH_LEN, EncodedMsgHashBuffer,
				XASU_SHAKE_256_MAX_HASH_LEN, HashLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_PSS_HASH_CMP_FAIL;
	}

END:
	/** Zeroize local copy of all the parameters. */
	SStatus = XAsufw_DmaMemSet(DmaPtr, (u32)(UINTPTR)DataBlock, 0U, XRSA_MAX_KEY_SIZE_IN_BYTES *
					XRSA_TOTAL_PARAMS);

	Status = XAsufw_UpdateBufStatus(Status, SStatus);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function takes an input of variable length and
 *		a desired output length as input, and provides fixed output
 *		mask using cryptographic hash function.
 *
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	ShaInstancePtr	Pointer to the Sha instance.
 * @param	ShaType		SHA mode selection.
 * @param       MgfInput	Pointer to all required parameters of MGF.
 *
 * @return
 *		- XASUFW_SUCCESS on modulus data is greater than input data.
 *		- XASUFW_RSA_MOD_DATA_INVALID on modulus data less than input data.
 *		- XASUFW_RSA_MOD_DATA_INPUT_DATA_EQUAL on modulus data equal to input data.
 *
 *************************************************************************************************/
static s32 XAsufw_MaskGenFunc(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, u8 ShaType,
				XAsufw_MgfInput *MgfInput)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Counter = 0U;
	u32 HashLen = 0U;
	u8 Hash[XASU_SHAKE_256_MAX_HASH_LEN];
	u8 Bytes[XASUFW_WORD_LEN_IN_BYTES];
	u32 Size = 0U;
	u32 NoOfIterations = 0U;
	u8 *OutputPtr;

	if (MgfInput == NULL) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	if ((MgfInput->Seed == NULL) || (MgfInput->Output == NULL) || (MgfInput->OutputLen == 0U)) {
		Status = XASUFW_RSA_INVALID_PARAM;
		goto END;
	}

	Status = XSha_GetHashLen(ShaType, &HashLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Size = MgfInput->OutputLen;
	if (Size > HashLen) {
		Size = HashLen;
	}

	OutputPtr = MgfInput->Output;
	NoOfIterations = (MgfInput->OutputLen + HashLen - 1U) / HashLen;
	while (Counter < NoOfIterations) {
		XAsufw_I2Osp(Counter, XASUFW_WORD_LEN_IN_BYTES, Bytes);

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Start(ShaInstancePtr, ShaType);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Update(ShaInstancePtr, DmaPtr, (u64)(UINTPTR)MgfInput->Seed,
					MgfInput->SeedLen, FALSE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Update(ShaInstancePtr, DmaPtr, (u64)(UINTPTR)Bytes,
					XASUFW_WORD_LEN_IN_BYTES, TRUE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Finish(ShaInstancePtr, (u64)(UINTPTR)Hash, HashLen,
					XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCpy(OutputPtr, Size, Hash, Size, Size);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		OutputPtr = &OutputPtr[Size];
		Counter = Counter + 1U;
		if (Counter == (NoOfIterations - 1U)) {
			Size = MgfInput->OutputLen - ((NoOfIterations - 1U) * HashLen);
		}
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA OAEP encryption/decryption operation command
 * 		using SHA2 for hash calculation.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if public encryption operation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_RSA_OAEP_ENCRYPT_ERROR, if encryption operaiton fails.
 * 	- XASUFW_RSA_OAEP_DECRYPT_ERROR, if private exponentiation decryption operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaOaepEncDecSha2(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	s32 SStatus = XASUFW_FAILURE;
	XSha *XAsufw_Sha2 = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	const XAsu_RsaClientOaepPaddingParams *Cmd = (const XAsu_RsaClientOaepPaddingParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;
	u8 IntermediateOutput[XRSA_4096_KEY_SIZE];

	/** Check resource availability (DMA, RSA, SHA2 and TRNG) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto RET;
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);
	XAsufw_AllocateResource(XASUFW_TRNG, QueueId);
	XAsufw_AllocateResource(XASUFW_SHA2, QueueId);

	/** Perform public exponentiation encryption operation. */
	if (Cmd->OperationFlag == XASU_RSA_ENCRYPTION) {
		Status = XAsufw_RsaOaepEncode(AsuDmaPtr, XAsufw_Sha2, Cmd->XAsu_ClientComp.Len,
			Cmd->XAsu_ClientComp.KeySize, Cmd->XAsu_ClientComp.InputDataAddr,
			Cmd->OptionalLabelAddr, Cmd->OptionalLabelSize, IntermediateOutput,
			Cmd->DigestType);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_OAEP_ENCODE_ERROR);
			goto END;
		}

		Status = XRsa_PubExp(AsuDmaPtr, Cmd->XAsu_ClientComp.KeySize,
					(u64)(UINTPTR)IntermediateOutput,
					Cmd->XAsu_ClientComp.OutputDataAddr,
					Cmd->XAsu_ClientComp.KeyCompAddr,
					Cmd->XAsu_ClientComp.ExpoCompAddr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_OAEP_ENCRYPT_ERROR);
		}
	} else if (Cmd->OperationFlag == XASU_RSA_EXPONENTIATION_DECRYPTION) {
		Status = XRsa_PvtExp(AsuDmaPtr, Cmd->XAsu_ClientComp.Len,
				Cmd->XAsu_ClientComp.InputDataAddr,
				(u64)(UINTPTR)IntermediateOutput,
				Cmd->XAsu_ClientComp.KeyCompAddr,
				Cmd->XAsu_ClientComp.ExpoCompAddr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_OAEP_DECODE_ERROR);
			goto END;
		}

		Status = XAsufw_RsaOaepDecode(AsuDmaPtr, XAsufw_Sha2, Cmd->XAsu_ClientComp.Len,
				Cmd->XAsu_ClientComp.KeySize, IntermediateOutput,
				Cmd->OptionalLabelAddr, Cmd->OptionalLabelSize,
				Cmd->XAsu_ClientComp.OutputDataAddr, Cmd->DigestType);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_OAEP_DECODE_ERROR);
			goto END;
		}
	} else {
			Status = XASUFW_RSA_INVALID_OP_FLAG_ERROR;
	}
END:
	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&IntermediateOutput[0U], XRSA_4096_KEY_SIZE, 0U,
			      XRSA_4096_KEY_SIZE);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
RET:
	return Status;
}
/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA OAEP encryption/decryption operation command
 * 		using SHA3 for hash calculation.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if public encryption operation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_RSA_OAEP_ENCRYPT_ERROR, if encryption operaiton fails.
 * 	- XASUFW_RSA_OAEP_DECODE_ERROR, if private exponentiation decryption operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaOaepEncDecSha3(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	s32 SStatus = XASUFW_FAILURE;
	XSha *XAsufw_Sha3 = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	const XAsu_RsaClientOaepPaddingParams *Cmd = (const XAsu_RsaClientOaepPaddingParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;
	u8 PaddedOutput[XRSA_4096_KEY_SIZE];

	/** Check resource availability (DMA, RSA, SHA3 and TRNG) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto RET;
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);
	XAsufw_AllocateResource(XASUFW_TRNG, QueueId);
	XAsufw_AllocateResource(XASUFW_SHA3, QueueId);

	/** Perform public exponentiation encryption operation. */
	if (Cmd->OperationFlag == XASU_RSA_ENCRYPTION) {
		Status = XAsufw_RsaOaepEncode(AsuDmaPtr, XAsufw_Sha3, Cmd->XAsu_ClientComp.Len,
			Cmd->XAsu_ClientComp.KeySize, Cmd->XAsu_ClientComp.InputDataAddr,
			Cmd->OptionalLabelAddr, Cmd->OptionalLabelSize, PaddedOutput,
			Cmd->DigestType);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_OAEP_ENCODE_ERROR);
			goto END;
		}

		Status = XRsa_PubExp(AsuDmaPtr, Cmd->XAsu_ClientComp.KeySize,
				(u64)(UINTPTR)PaddedOutput,
				Cmd->XAsu_ClientComp.OutputDataAddr,
				Cmd->XAsu_ClientComp.KeyCompAddr,
				Cmd->XAsu_ClientComp.ExpoCompAddr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_OAEP_ENCRYPT_ERROR);
		}
	} else if (Cmd->OperationFlag == XASU_RSA_EXPONENTIATION_DECRYPTION) {
		Status = XRsa_PvtExp(AsuDmaPtr, Cmd->XAsu_ClientComp.Len,
				Cmd->XAsu_ClientComp.InputDataAddr,
				(u64)(UINTPTR)PaddedOutput,
				Cmd->XAsu_ClientComp.KeyCompAddr,
				Cmd->XAsu_ClientComp.ExpoCompAddr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_OAEP_DECRYPT_ERROR);
			goto END;
		}
		Status = XAsufw_RsaOaepDecode(AsuDmaPtr, XAsufw_Sha3, Cmd->XAsu_ClientComp.Len,
				Cmd->XAsu_ClientComp.KeySize, PaddedOutput,
				Cmd->OptionalLabelAddr, Cmd->OptionalLabelSize,
				Cmd->XAsu_ClientComp.OutputDataAddr, Cmd->DigestType);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_OAEP_DECODE_ERROR);
		}
	} else {
			Status = XASUFW_RSA_INVALID_OP_FLAG_ERROR;
	}
END:
	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&PaddedOutput[0U], XRSA_4096_KEY_SIZE, 0U,
			      XRSA_4096_KEY_SIZE);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
RET:
	return Status;
}
/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA PKCS encryption/decryption operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if public encryption operation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_RSA_PKCS_ENCODE_ERROR, if encryption operaiton fails.
 * 	- XASUFW_RSA_PKCS_DECODE_ERROR, if private exponentiation decryption operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPkcsEncDec(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	s32 SStatus = XASUFW_FAILURE;
	const XAsu_RsaClientPaddingParams *Cmd = (const XAsu_RsaClientPaddingParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;
	u8 PaddedOutput[XRSA_4096_KEY_SIZE];

	/** Check resource availability (DMA, RSA and TRNG) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto RET;
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);
	XAsufw_AllocateResource(XASUFW_TRNG, QueueId);

	/** Perform public exponentiation encryption operation. */
	if (Cmd->OperationFlag == XASU_RSA_ENCRYPTION) {
		Status = XAsufw_RsaPkcsEncode(AsuDmaPtr, Cmd->XAsu_ClientComp.Len,
						Cmd->XAsu_ClientComp.KeySize,
						Cmd->XAsu_ClientComp.InputDataAddr, PaddedOutput);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PKCS_ENCODE_ERROR);
			goto END;
		}

		Status = XRsa_PubExp(AsuDmaPtr, Cmd->XAsu_ClientComp.KeySize,
					(u64)(UINTPTR)PaddedOutput,
					Cmd->XAsu_ClientComp.OutputDataAddr,
					Cmd->XAsu_ClientComp.KeyCompAddr,
					Cmd->XAsu_ClientComp.ExpoCompAddr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PKCS_ENCRYPT_ERROR);
		}
	} else if (Cmd->OperationFlag == XASU_RSA_EXPONENTIATION_DECRYPTION) {
		Status = XRsa_PvtExp(AsuDmaPtr, Cmd->XAsu_ClientComp.Len,
				Cmd->XAsu_ClientComp.InputDataAddr, (u64)(UINTPTR)PaddedOutput,
				Cmd->XAsu_ClientComp.KeyCompAddr, Cmd->XAsu_ClientComp.ExpoCompAddr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PKCS_DECRYPT_ERROR);
			goto END;
		}

		Status = XAsufw_RsaPkcsDecode(AsuDmaPtr, Cmd->XAsu_ClientComp.Len,
				Cmd->XAsu_ClientComp.KeySize, PaddedOutput,
				Cmd->XAsu_ClientComp.OutputDataAddr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PKCS_DECODE_ERROR);
		}
	} else {
			Status = XASUFW_RSA_INVALID_OP_FLAG_ERROR;
	}
END:
	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&PaddedOutput[0U], XRSA_4096_KEY_SIZE, 0U,
			      XRSA_4096_KEY_SIZE);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA PSS sign generation/verification operation
 * 		command using SHA2 for hash calculation.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if public encryption operation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_RSA_PSS_SIGN_GEN_ERROR, if sign generation operaiton fails.
 * 	- XASUFW_RSA_PSS_SIGN_VER_ERROR, if sign verification operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPssSignGenVerSha2(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	XSha *XAsufw_Sha2 = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	const XAsu_RsaClientPaddingParams *Cmd = (const XAsu_RsaClientPaddingParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;
	u32 HashLen = 0U;
	u8 InputDataHash[XASU_SHAKE_256_MAX_HASH_LEN];
	u8 PaddedOutput[XRSA_4096_KEY_SIZE];

	/** Check resource availability (DMA, RSA, SHA2 and TRNG) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto RET;
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);
	XAsufw_AllocateResource(XASUFW_TRNG, QueueId);
	XAsufw_AllocateResource(XASUFW_SHA2, QueueId);

	if (Cmd->InputDataType != XASU_RSA_HASHED_INPUT_DATA) {
		Status = XSha_GetHashLen(Cmd->DigestType, &HashLen);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Start(XAsufw_Sha2, Cmd->DigestType);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Update(XAsufw_Sha2, AsuDmaPtr, Cmd->XAsu_ClientComp.InputDataAddr,
					Cmd->XAsu_ClientComp.Len, TRUE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Finish(XAsufw_Sha2, (u64)(UINTPTR)InputDataHash,
					HashLen, XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	} else {
		HashLen = Cmd->XAsu_ClientComp.Len;
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(AsuDmaPtr, Cmd->XAsu_ClientComp.InputDataAddr,
					(u64)(UINTPTR)InputDataHash, Cmd->XAsu_ClientComp.Len, 0U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	/** Perform public exponentiation encryption operation. */
	if (Cmd->OperationFlag == XASU_RSA_SIGN_GENERATION) {
		Status = XAsufw_RsaPssEncode(AsuDmaPtr, XAsufw_Sha2, HashLen,
				Cmd->XAsu_ClientComp.KeySize, Cmd->SaltLen, InputDataHash,
				PaddedOutput, Cmd->DigestType);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PSS_ENCODE_ERROR);
			goto END;
		}
		Status = XRsa_PvtExp(AsuDmaPtr, Cmd->XAsu_ClientComp.KeySize,
				(u64)(UINTPTR)PaddedOutput, Cmd->XAsu_ClientComp.OutputDataAddr,
				Cmd->XAsu_ClientComp.KeyCompAddr, Cmd->XAsu_ClientComp.ExpoCompAddr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PSS_SIGN_GEN_ERROR);
		}
	} else if (Cmd->OperationFlag == XASU_RSA_SIGN_VERIFICATION) {
		Status = XRsa_PubExp(AsuDmaPtr, Cmd->SignatureLen,
				Cmd->SignatureDataAddr, (u64)(UINTPTR)PaddedOutput,
				Cmd->XAsu_ClientComp.KeyCompAddr, Cmd->XAsu_ClientComp.ExpoCompAddr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PSS_SIGN_VER_ERROR);
			goto END;
		}

		Status = XAsufw_RsaPssDecode(AsuDmaPtr, XAsufw_Sha2, HashLen, Cmd->SignatureLen,
				Cmd->XAsu_ClientComp.KeySize, Cmd->SaltLen, PaddedOutput,
				(u64)(UINTPTR)InputDataHash, Cmd->DigestType);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PSS_DECODE_ERROR);
			goto END;
		}
	} else {
			Status = XASUFW_RSA_INVALID_OP_FLAG_ERROR;
	}
END:
	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&PaddedOutput[0U], XRSA_4096_KEY_SIZE, 0U,
			      XRSA_4096_KEY_SIZE);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&InputDataHash[0U], XASU_SHAKE_256_MAX_HASH_LEN, 0U,
			      XASU_SHAKE_256_MAX_HASH_LEN);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA PSS sign generation/verification operation
 * 		command using SHA3 for hash calculation.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if public encryption operation is successful.
 * 	- XASUFW_RSA_PSS_SIGN_GEN_ERROR, if sign generation operaiton fails.
 * 	- XASUFW_RSA_PSS_SIGN_VER_ERROR, if sign verification operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPssSignGenVerSha3(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	XSha *XAsufw_Sha3 = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	const XAsu_RsaClientPaddingParams *Cmd = (const XAsu_RsaClientPaddingParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;
	u32 HashLen = 0U;
	u8 InputDataHash[XASU_SHAKE_256_MAX_HASH_LEN];
	u8 PaddedOutput[XRSA_4096_KEY_SIZE];

	/** Check resource availability (DMA, RSA, SHA3 and TRNG) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto RET;
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);
	XAsufw_AllocateResource(XASUFW_TRNG, QueueId);
	XAsufw_AllocateResource(XASUFW_SHA3, QueueId);

	if (Cmd->InputDataType != XASU_RSA_HASHED_INPUT_DATA) {
		Status = XSha_GetHashLen(Cmd->DigestType, &HashLen);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Start(XAsufw_Sha3, Cmd->DigestType);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Update(XAsufw_Sha3, AsuDmaPtr, Cmd->XAsu_ClientComp.InputDataAddr,
					Cmd->XAsu_ClientComp.Len, TRUE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Finish(XAsufw_Sha3, (u64)(UINTPTR)InputDataHash,
					HashLen, XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	} else {
		HashLen = Cmd->XAsu_ClientComp.Len;
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(AsuDmaPtr, Cmd->XAsu_ClientComp.InputDataAddr,
					(u64)(UINTPTR)InputDataHash, Cmd->XAsu_ClientComp.Len, 0U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}
	/** Perform public exponentiation encryption operation. */
	if (Cmd->OperationFlag == XASU_RSA_SIGN_GENERATION) {
		Status = XAsufw_RsaPssEncode(AsuDmaPtr, XAsufw_Sha3, HashLen,
				Cmd->XAsu_ClientComp.KeySize, Cmd->SaltLen, InputDataHash,
				PaddedOutput, Cmd->DigestType);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PSS_ENCODE_ERROR);
			goto END;
		}
		Status = XRsa_PvtExp(AsuDmaPtr, Cmd->XAsu_ClientComp.KeySize,
				(u64)(UINTPTR)PaddedOutput, Cmd->XAsu_ClientComp.OutputDataAddr,
				Cmd->XAsu_ClientComp.KeyCompAddr, Cmd->XAsu_ClientComp.ExpoCompAddr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PSS_SIGN_GEN_ERROR);
		}
	} else if (Cmd->OperationFlag == XASU_RSA_SIGN_VERIFICATION) {
		Status = XRsa_PubExp(AsuDmaPtr, Cmd->SignatureLen,
				Cmd->SignatureDataAddr, (u64)(UINTPTR)PaddedOutput,
				Cmd->XAsu_ClientComp.KeyCompAddr, Cmd->XAsu_ClientComp.ExpoCompAddr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PSS_SIGN_VER_ERROR);
			goto END;
		}

		Status = XAsufw_RsaPssDecode(AsuDmaPtr, XAsufw_Sha3, HashLen, Cmd->SignatureLen,
				Cmd->XAsu_ClientComp.KeySize, Cmd->SaltLen, PaddedOutput,
				(u64)(UINTPTR)InputDataHash, Cmd->DigestType);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PSS_DECODE_ERROR);
			goto END;
		}
	} else {
			Status = XASUFW_RSA_INVALID_OP_FLAG_ERROR;
	}
END:
	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&PaddedOutput[0U], XRSA_4096_KEY_SIZE, 0U,
			      XRSA_4096_KEY_SIZE);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&InputDataHash[0U], XASU_SHAKE_256_MAX_HASH_LEN, 0U,
			      XASU_SHAKE_256_MAX_HASH_LEN);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA PKCS sign generation/verification operation command
 * 		using SHA2 for hash calculation.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if public encryption operation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_RSA_PKCS_SIGN_GEN_ERROR, if sign generation operaiton fails.
 * 	- XASUFW_RSA_PKCS_SIGN_VER_ERROR, if sign verification operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPkcsSignGenVerSha2(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	XSha *XAsufw_Sha2 = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	const XAsu_RsaClientPaddingParams *Cmd = (const XAsu_RsaClientPaddingParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;
	u32 HashLen = 0U;
	u8 InputDataHash[XASU_SHAKE_256_MAX_HASH_LEN];
	u8 PaddedOutput[XRSA_4096_KEY_SIZE];
	u8 DecodedOutput[XASU_SHAKE_256_MAX_HASH_LEN];

	/** Check resource availability (DMA, RSA, SHA2 and TRNG) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto RET;
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);
	XAsufw_AllocateResource(XASUFW_TRNG, QueueId);
	XAsufw_AllocateResource(XASUFW_SHA2, QueueId);

	if (Cmd->InputDataType != XASU_RSA_HASHED_INPUT_DATA) {
		Status = XSha_GetHashLen(Cmd->DigestType, &HashLen);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Start(XAsufw_Sha2, Cmd->DigestType);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Update(XAsufw_Sha2, AsuDmaPtr, Cmd->XAsu_ClientComp.InputDataAddr,
					Cmd->XAsu_ClientComp.Len, TRUE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Finish(XAsufw_Sha2, (u64)(UINTPTR)InputDataHash,
					HashLen, XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	} else {
		HashLen = Cmd->XAsu_ClientComp.Len;
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(AsuDmaPtr, Cmd->XAsu_ClientComp.InputDataAddr,
					(u64)(UINTPTR)InputDataHash, Cmd->XAsu_ClientComp.Len, 0U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}
	/** Perform public exponentiation encryption operation. */
	if (Cmd->OperationFlag == XASU_RSA_SIGN_GENERATION) {
		Status = XAsufw_RsaPkcsEncode(AsuDmaPtr, HashLen, Cmd->XAsu_ClientComp.KeySize,
						(u64)(UINTPTR)InputDataHash, PaddedOutput);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PKCS_ENCODE_ERROR);
			goto END;
		}
		Status = XRsa_PvtExp(AsuDmaPtr, Cmd->XAsu_ClientComp.KeySize,
				(u64)(UINTPTR)PaddedOutput, Cmd->XAsu_ClientComp.OutputDataAddr,
				Cmd->XAsu_ClientComp.KeyCompAddr, Cmd->XAsu_ClientComp.ExpoCompAddr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PKCS_SIGN_GEN_ERROR);
		}
	} else if (Cmd->OperationFlag == XASU_RSA_SIGN_VERIFICATION) {
		Status = XRsa_PubExp(AsuDmaPtr, Cmd->SignatureLen,
				Cmd->SignatureDataAddr, (u64)(UINTPTR)PaddedOutput,
				Cmd->XAsu_ClientComp.KeyCompAddr, Cmd->XAsu_ClientComp.ExpoCompAddr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PKCS_SIGN_VER_ERROR);
		}

		Status = XAsufw_RsaPkcsDecode(AsuDmaPtr, Cmd->SignatureLen,
				Cmd->XAsu_ClientComp.KeySize, PaddedOutput,
				(u64)(UINTPTR)DecodedOutput);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PKCS_DECODE_ERROR);
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCmp(DecodedOutput, XASU_SHAKE_256_MAX_HASH_LEN, InputDataHash,
					XASU_SHAKE_256_MAX_HASH_LEN, HashLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PKCS_SIGN_VER_ERROR);
			goto END;
		}
	} else {
			Status = XASUFW_RSA_INVALID_OP_FLAG_ERROR;
	}
END:
	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&PaddedOutput[0U], XRSA_4096_KEY_SIZE, 0U,
			      XRSA_4096_KEY_SIZE);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&InputDataHash[0U], XASU_SHAKE_256_MAX_HASH_LEN, 0U,
			      XASU_SHAKE_256_MAX_HASH_LEN);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&DecodedOutput[0U], XASU_SHAKE_256_MAX_HASH_LEN, 0U,
			      XASU_SHAKE_256_MAX_HASH_LEN);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA PKCS sign generation/verification operation command
 * 		using SHA3 for hash calculation.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if public encryption operation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_RSA_PUB_OP_ERROR, if public encryption operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPkcsSignGenVerSha3(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 SStatus = XASUFW_FAILURE;
	XSha *XAsufw_Sha3 = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	const XAsu_RsaClientPaddingParams *Cmd = (const XAsu_RsaClientPaddingParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;
	u32 HashLen = 0U;
	u8 InputDataHash[XASU_SHAKE_256_MAX_HASH_LEN];
	u8 PaddedOutput[XRSA_4096_KEY_SIZE];
	u8 DecodedOutput[XASU_SHAKE_256_MAX_HASH_LEN];

	/** Check resource availability (DMA, RSA, SHA3 and TRNG) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto RET;
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);
	XAsufw_AllocateResource(XASUFW_TRNG, QueueId);
	XAsufw_AllocateResource(XASUFW_SHA3, QueueId);

	if (Cmd->InputDataType != XASU_RSA_HASHED_INPUT_DATA) {
		Status = XSha_GetHashLen(Cmd->DigestType, &HashLen);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Start(XAsufw_Sha3, Cmd->DigestType);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Update(XAsufw_Sha3, AsuDmaPtr, Cmd->XAsu_ClientComp.InputDataAddr,
					Cmd->XAsu_ClientComp.Len, TRUE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XSha_Finish(XAsufw_Sha3, (u64)(UINTPTR)InputDataHash,
					HashLen, XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	} else {
		HashLen = Cmd->XAsu_ClientComp.Len;
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(AsuDmaPtr, Cmd->XAsu_ClientComp.InputDataAddr,
					(u64)(UINTPTR)InputDataHash, Cmd->XAsu_ClientComp.Len, 0U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}
	/** Perform public exponentiation encryption operation. */
	if (Cmd->OperationFlag == XASU_RSA_SIGN_GENERATION) {
		Status = XAsufw_RsaPkcsEncode(AsuDmaPtr, HashLen, Cmd->XAsu_ClientComp.KeySize,
						(u64)(UINTPTR)InputDataHash, PaddedOutput);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PKCS_SIGN_GEN_ERROR);
			goto END;
		}
		Status = XRsa_PvtExp(AsuDmaPtr, Cmd->XAsu_ClientComp.KeySize,
				(u64)(UINTPTR)PaddedOutput, Cmd->XAsu_ClientComp.OutputDataAddr,
				Cmd->XAsu_ClientComp.KeyCompAddr, Cmd->XAsu_ClientComp.ExpoCompAddr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PKCS_SIGN_GEN_ERROR);
		}
	} else if (Cmd->OperationFlag == XASU_RSA_SIGN_VERIFICATION) {
		Status = XRsa_PubExp(AsuDmaPtr, Cmd->SignatureLen,
				Cmd->SignatureDataAddr, (u64)(UINTPTR)PaddedOutput,
				Cmd->XAsu_ClientComp.KeyCompAddr, Cmd->XAsu_ClientComp.ExpoCompAddr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PKCS_SIGN_VER_ERROR);
		}

		Status = XAsufw_RsaPkcsDecode(AsuDmaPtr, Cmd->SignatureLen,
				Cmd->XAsu_ClientComp.KeySize, PaddedOutput,
				(u64)(UINTPTR)DecodedOutput);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PKCS_SIGN_VER_ERROR);
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCmp(DecodedOutput, XASU_SHAKE_256_MAX_HASH_LEN, InputDataHash,
					XASU_SHAKE_256_MAX_HASH_LEN, HashLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PKCS_SIGN_VER_ERROR);
			goto END;
		}
	} else {
			Status = XASUFW_RSA_INVALID_OP_FLAG_ERROR;
	}
END:
	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&PaddedOutput[0U], XRSA_4096_KEY_SIZE, 0U,
			      XRSA_4096_KEY_SIZE);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&InputDataHash[0U], XASU_SHAKE_256_MAX_HASH_LEN, 0U,
			      XASU_SHAKE_256_MAX_HASH_LEN);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Zeroize local copy of output value. */
	SStatus = Xil_SMemSet(&DecodedOutput[0U], XASU_SHAKE_256_MAX_HASH_LEN, 0U,
			      XASU_SHAKE_256_MAX_HASH_LEN);
	if (Status == XASUFW_SUCCESS) {
		Status = SStatus;
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs Known Answer Tests (KATs).
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- Returns XASUFW_SUCCESS, if KAT is successful.
 * 	- Error code, returned when XAsufw_RsaEncDecKat API fails.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaKat(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	/** Perform KAT on 2048 bit key size. */

	return XAsufw_RsaEncDecKat(QueueId);
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA Get Info command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- Returns XASUFW_SUCCESS on successful execution of the command.
 * 	- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaGetInfo(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	/* TODO: Implement XAsufw_RsaGetInfo */
	s32 Status = XASUFW_FAILURE;

	return Status;
}
/** @} */
