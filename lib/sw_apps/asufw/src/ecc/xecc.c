/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xecc.c
* This file contains implementation of the interface functions for ECC engine.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- ---------- -----------------------------------------------------------------------------
* 1.0   yog  06/19/2024 Initial release
*       yog  08/19/2024 Received Dma instance from handler
*       yog  08/25/2024 Integrated FIH library
*
* </pre>
*
**************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xecc.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xecc_hw.h"
#include "xasufw_config.h"
#include "xil_io.h"
#include "xil_util.h"
#include "xasu_eccinfo.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/
#define XECC_CURVES_SUPPORTED		2U /**< Curves P-256 and P-384 are supported for ECC engine */
#define XECC_RESET_ASSERT		(1U) /**< ECC reset assert value */
#define XECC_RESET_DEASSERT		(0U) /**< ECC reset deassert value */
#define XECC_TIMEOUT_MAX		(0x1FFFFU) /**< ECC done timeout \
TBD: need to calculate as part of VNC */
#define XECC_SIGN_VERIFICATION_OP_CODE	(0x0U) /**< OpCode for signature verification */
#define XECC_PUB_KEY_VALIDATION_OP_CODE	(0x1U) /**< OpCode for public key validation */
#define XECC_PUB_KEY_GENERATION_OP_CODE	(0x2U) /**< Opcode for Public Key Generation */
#define XECC_SIGN_GENERATION_OP_CODE	(0x3U) /**< OpCode for signature generation */
#define XECC_DOUBLE_CURVE_LENGTH_SHIFT	(0x1U) /**< Shift value to double the curve length */

/************************************** Type Definitions *****************************************/

/**
* Structure to get curve info
*/
typedef struct {
	u32 CurveType; /**< Type of the curve */
	u32 CurveBytes; /**< Length of the curve in bytes*/
} XEcc_CurveInfo;

/**
* This structure contains configuration information for a ECC core.
* Each core should have an associated configuration structure.
*/
struct _XEcc_Config {
	u32 DeviceId; /**< DeviceId is the unique ID of the device */
	u32 BaseAddress; /**< BaseAddress is the physical base address of the device's registers */
};

/**
* ECC driver instance data structure. A pointer to an instance data
* structure is passed around by functions to refer to a specific driver
* instance.
*/
struct _XEcc {
	u32 DeviceId; /**< Ecc Device Id */
	u32 BaseAddress; /**< Ecc Base address */
	u32 IsReady; /**< ECC component ready state */
	XEcc_CurveInfo *CurveInfo; /**< To get ECC curve information */
	u32 CmConfig; /**< Counter measure configuration */
};

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static XEcc_Config *XEcc_LookupConfig(u32 DeviceId);
static inline s32 XEcc_WaitForDone(const XEcc *InstancePtr);
static inline u32 XEcc_ConfigureEngine(const XEcc *InstancePtr, u32 OpCode);
static s32 XEcc_ConfigNStartOperation(const XEcc *InstancePtr, u32 OpCode);
static s32 XEcc_InputValidate(const XEcc *InstancePtr, u32 CurveType);

/************************************ Variable Definitions ***************************************/

/** ECC configuration table for devices */
static XEcc_Config XEcc_ConfigTable[XASU_XECC_NUM_INSTANCES] = {
	{
		XASU_XECC_0_DEVICE_ID,
		XASU_XECC_0_BASEADDR
	}
};

static XEcc XEcc_Instance[XASU_XECC_NUM_INSTANCES]; /**< ASUFW ECC HW instances */

static XEcc_CurveInfo XEcc_CurveInfoTable[XECC_CURVES_SUPPORTED] = {
	{
		XECC_CURVE_TYPE_NIST_P256,
		XECC_P256_SIZE_IN_BYTES
	},
	{
		XECC_CURVE_TYPE_NIST_P384,
		XECC_P384_SIZE_IN_BYTES
	}
};

/*************************************************************************************************/
/**
 * @brief	This function returns an instance pointer of the ECC HW based on Device ID.
 *
 * @param	DeviceId	Unique device ID of the device for the lookup operation.
 *
 * @return
 * 		- It returns pointer to the XEcc_Instance corresponding to the Device ID.
 * 		- It returns NULL if Device ID is invalid.
 *
 *************************************************************************************************/
XEcc *XEcc_GetInstance(u32 DeviceId)
{
	XEcc *XEcc_InstancePtr = NULL;

	if (DeviceId >= XASU_XECC_NUM_INSTANCES) {
		XFIH_GOTO(END);
	}

	XEcc_InstancePtr = &XEcc_Instance[DeviceId];
	XEcc_InstancePtr->DeviceId = DeviceId;

END:
	return XEcc_InstancePtr;
}

/*************************************************************************************************/
/**
*
* @brief	This function initializes ECC core. This function must be called prior using a
*		ECC core.
*
* @param	InstancePtr	Pointer to the XEcc instance.
*
* @return
*		- Upon successful initialization, returns XASUFW_SUCCESS.
*		- XASUFW_ECC_INVALID_PARAM, if InstancePtr or CfgPtr is NULL
*
**************************************************************************************************/
s32 XEcc_Initialize(XEcc *InstancePtr)
{
	s32 Status = XASUFW_FAILURE;
	XEcc_Config *CfgPtr = NULL;

	if (InstancePtr == NULL) {
		Status = XASUFW_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}
	CfgPtr = XEcc_LookupConfig(InstancePtr->DeviceId);
	if (CfgPtr == NULL) {
		Status = XASUFW_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	InstancePtr->BaseAddress = CfgPtr->BaseAddress;
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	InstancePtr->CmConfig = XASUFW_ECC_CM_CONFIG;

	Status = XASUFW_SUCCESS;
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates public key for a given curve type using	private key.
 *
 * @param	InstancePtr	Pointer to the ECC instance.
 * @param	DmaPtr		Pointer to DMA instance.
 * @param	CurveType	ECC curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PrivKeyAddr	Address of the private key used to generate public key.
 *				Length of the private key shall be CurveLen.
 * @param	PubKeyAddr	Address to store the generated Public Key.
 *				Length of the public key shall be double of CurveLen since
 *				public key has both Qx, Qy components.
 *
 * @return
 * 		- Upon successful public key generation, it returns XASUFW_SUCCESS.
 * 		- XASUFW_ECC_INVALID_PARAM, if InstancePtr is NULL or if curve type or curve length
 * 				is invalid
 * 		- XASUFW_ECC_INIT_NOT_DONE, if ECC component is not ready
 * 		- XASUFW_ECC_WRITE_DATA_FAIL - if write data to registers through DMA fails
 * 		- XASUFW_ECC_READ_DATA_FAIL - if read data from registers through DMA fails
 *
 *************************************************************************************************/
s32 XEcc_GeneratePublicKey(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
			   u64 PrivKeyAddr, u64 PubKeyAddr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XEcc_CurveInfo *CurveInfo = NULL;
	u32 Offset = XECC_MEM_GEN_KEY_PVT_KEY_OFFSET;

	Status = XEcc_InputValidate(InstancePtr, CurveType);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	CurveInfo = &XEcc_CurveInfoTable[CurveType];
	InstancePtr->CurveInfo = CurveInfo;

	if (CurveLen != CurveInfo->CurveBytes) {
		Status = XASUFW_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/* Release Reset */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);

	/* Enable endianness for write and read operations */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_CFG_OFFSET,
			XECC_CFG_WR_RD_ENDIANNESS_MASK);

	/* Write private key to specified registers */
	Status = XAsufw_DmaXfr(DmaPtr, PrivKeyAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END);
	}

	/* Updates configuration and starts the operation */
	Status = XEcc_ConfigNStartOperation(InstancePtr, XECC_PUB_KEY_GENERATION_OP_CODE);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* Read and store the generated Public Key */
	Offset = XECC_MEM_PUB_KEY_X_OFFSET;
	XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(XAsufw_DmaXfr, XASUFW_ECC_READ_DATA_FAIL, XFihVar,
					   Status, END, DmaPtr, (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset),
					   PubKeyAddr, CurveLen, 0U);

	Offset = XECC_MEM_PUB_KEY_Y_OFFSET;
	XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(XAsufw_DmaXfr, XASUFW_ECC_READ_DATA_FAIL, XFihVar,
					   Status, END, DmaPtr, (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset),
					   (PubKeyAddr + CurveLen), CurveLen, 0U);

END:
	if (InstancePtr != NULL) {
		/* Set ECC under reset */
		XAsufw_CryptoCoreSetReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates public key for a given curve type.
 *
 * @param	InstancePtr	Pointer to the ECC instance.
 * @param	DmaPtr		Pointer to DMA instance.
 * @param	CurveType	ECC curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PubKeyAddr	Address of the public key to validate.
 *				Length of the public key shall be double of CurveLen since
 *				public key has both Qx, Qy components.
 *
 * @return
 * 		- Upon successful public key validation, it returns XASUFW_SUCCESS.
 * 		- XASUFW_ECC_INVALID_PARAM, if InstancePtr is NULL or if curve type or curve length
 * 				is invalid.
 * 		- XASUFW_ECC_INIT_NOT_DONE, if ECC component is not ready.
 * 		- XASUFW_ECC_WRITE_DATA_FAIL - if write data to registers through DMA fails.
 *
 *************************************************************************************************/
s32 XEcc_ValidatePublicKey(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
			   u64 PubKeyAddr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	XEcc_CurveInfo *CurveInfo = NULL;
	u32 Offset = XECC_MEM_PUB_KEY_X_OFFSET;

	Status = XEcc_InputValidate(InstancePtr, CurveType);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	CurveInfo = &XEcc_CurveInfoTable[CurveType];
	InstancePtr->CurveInfo = CurveInfo;

	if (CurveLen != CurveInfo->CurveBytes) {
		Status = XASUFW_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/* Release Reset */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);

	/* Enable endianness for write and read operations */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_CFG_OFFSET,
			XECC_CFG_WR_RD_ENDIANNESS_MASK);

	/* Write public key to specified registers */
	Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END);
	}
	Offset = XECC_MEM_PUB_KEY_Y_OFFSET;
	Status = XAsufw_DmaXfr(DmaPtr, (PubKeyAddr + CurveLen),
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END);
	}
	/* Updates configuration and starts the operation */
	Status = XEcc_ConfigNStartOperation(InstancePtr, XECC_PUB_KEY_VALIDATION_OP_CODE);

END:
	if (InstancePtr != NULL) {
		/* Set ECC under reset */
		XAsufw_CryptoCoreSetReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates signature for a given curve type, hash and private key.
 *
 * @param	InstancePtr	Pointer to the ECC instance.
 * @param	DmaPtr		Pointer to DMA instance.
 * @param	CurveType	ECC curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PrivKeyAddr	Address of the private key.
 *				Length of the private key shall be CurveLen.
 * @param	EphemeralKeyPtr	Pointer to ephemeral key.
 *				Length of the ephemeral key shall be CurveLen.
 * @param	HashAddr	Address of hash to which signature is to be generated.
 * @param	HashBufLen	Length of the hash in bytes.
 * @param	SignAddr	Address to store the generated signature.
 *				Length of the signature shall be double of CurveLen since sign
 *				has both r, s components.
 *
 * @return
 * 		- Upon successful Signature generation, it returns XASUFW_SUCCESS.
 * 		- XASUFW_ECC_INVALID_PARAM, if InstancePtr is NULL or if curve type is invalid or
 * 			CurveLen and HashBufLen are invalid.
 * 		- XASUFW_ECC_INIT_NOT_DONE, if ECC component is not ready.
 * 		- XASUFW_ECC_EPHEMERAL_KEY_GEN_FAIL, if ephemeral key generation fails.
 * 		- XASUFW_ECC_WRITE_DATA_FAIL - if write data to registers through DMA fails.
 * 		- XASUFW_ECC_READ_DATA_FAIL - if read data from registers through DMA fails.
 *
 *************************************************************************************************/
s32 XEcc_GenerateSignature(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
			   u64 PrivKeyAddr, const u8 *EphemeralKeyPtr, u64 HashAddr, u32 HashBufLen,
			   u64 SignAddr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XEcc_CurveInfo *CurveInfo = NULL;
	u32 Offset = XECC_MEM_GEN_SIGN_PVT_KEY_OFFSET;

	Status = XEcc_InputValidate(InstancePtr, CurveType);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	CurveInfo = &XEcc_CurveInfoTable[CurveType];
	InstancePtr->CurveInfo = CurveInfo;

	if ((CurveLen != CurveInfo->CurveBytes) || (HashBufLen != CurveInfo->CurveBytes)) {
		Status = XASUFW_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/* Release Reset */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);

	/* Enable endianness for write and read operations */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_CFG_OFFSET,
			XECC_CFG_WR_RD_ENDIANNESS_MASK);

	/* Write private key to specified registers */
	Status = XAsufw_DmaXfr(DmaPtr, PrivKeyAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END);
	}

	/* Write Ephemeral key to specified registers */
	Offset = XECC_MEM_EPHEMERAL_KEY_OFFSET;
	Status = Xil_SMemCpy((u8 *)(InstancePtr->BaseAddress + Offset), CurveLen,
			     EphemeralKeyPtr, CurveLen, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END);
	}

	/* Write Hash to specified registers */
	Offset = XECC_MEM_HASH_OFFSET;
	Status = XAsufw_DmaXfr(DmaPtr, HashAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), HashBufLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END);
	}

	/* Updates configuration and starts the operation */
	Status = XEcc_ConfigNStartOperation(InstancePtr, XECC_SIGN_GENERATION_OP_CODE);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* Read and store the generated signature */
	Offset = XECC_MEM_SIGN_R_OFFSET;
	XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(XAsufw_DmaXfr, XASUFW_ECC_READ_DATA_FAIL, XFihVar,
					   Status, END, DmaPtr, (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset),
					   SignAddr, CurveLen, 0U);

	Offset = XECC_MEM_SIGN_S_OFFSET;
	XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(XAsufw_DmaXfr, XASUFW_ECC_READ_DATA_FAIL, XFihVar,
					   Status, END, DmaPtr, (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset),
					   (SignAddr + CurveLen), CurveLen, 0U);

END:
	if (InstancePtr != NULL) {
		/* Set ECC under reset */
		XAsufw_CryptoCoreSetReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function verifies the signature for a given curve type.
 *
 * @param	InstancePtr	Pointer to the ECC instance.
 * @param	DmaPtr		Pointer to DMA instance.
 * @param	CurveType	ECC Curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PubKeyAddr	Address of the public key.
 *				Length of the public key shall be double of CurveLen since
 *				public key has both Qx, Qy components.
 * @param	HashAddr	Address of hash.
 * @param	HashBufLen	Length of hash in bytes.
 * @param	SignAddr	Address of the signature to be verified.
 *				Length of the signature shall be double of CurveLen since sign
 *				has both r, s components.
 *
 * @return
 * 		- Upon successful Signature Validation, it returns XASUFW_SUCCESS.
 * 		- XASUFW_ECC_INVALID_PARAM, if InstancePtr is NULL or if curve type is invalid or
 * 			CurveLen and HashBufLen are invalid.
 * 		- XASUFW_ECC_INIT_NOT_DONE, if ECC component is not ready.
 * 		- XASUFW_ECC_WRITE_DATA_FAIL - if write data to registers through DMA fails.
 *
 *************************************************************************************************/
s32 XEcc_VerifySignature(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
			 u64 PubKeyAddr, u64 HashAddr, u32 HashBufLen, u64 SignAddr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	XEcc_CurveInfo *CurveInfo = NULL;
	u32 Offset = XECC_MEM_SIGN_R_OFFSET;

	Status = XEcc_InputValidate(InstancePtr, CurveType);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	CurveInfo = &XEcc_CurveInfoTable[CurveType];
	InstancePtr->CurveInfo = CurveInfo;

	if ((CurveLen != CurveInfo->CurveBytes) || (HashBufLen != CurveInfo->CurveBytes)) {
		Status = XASUFW_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/* Release Reset */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);

	/* Enable endianness for write and read operations */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_CFG_OFFSET,
			XECC_CFG_WR_RD_ENDIANNESS_MASK);

	/* Write Signature to specified registers */
	Status = XAsufw_DmaXfr(DmaPtr, SignAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END);
	}
	Offset = XECC_MEM_SIGN_S_OFFSET;
	Status = XAsufw_DmaXfr(DmaPtr, SignAddr + CurveLen,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END);
	}

	/* Write Hash to specified registers */
	Offset = XECC_MEM_HASH_OFFSET;
	Status = XAsufw_DmaXfr(DmaPtr, HashAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), HashBufLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END);
	}

	/* Write Public key to specified registers */
	Offset = XECC_MEM_PUB_KEY_X_OFFSET;
	Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END);
	}
	Offset = XECC_MEM_PUB_KEY_Y_OFFSET;
	Status = XAsufw_DmaXfr(DmaPtr, (PubKeyAddr + CurveLen),
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		XFIH_GOTO(END);
	}

	/* Updates configuration and starts the operation */
	Status = XEcc_ConfigNStartOperation(InstancePtr, XECC_SIGN_VERIFICATION_OP_CODE);

END:
	if (InstancePtr != NULL) {
		/* Set ECC under reset */
		XAsufw_CryptoCoreSetReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function check whether specified operation is completed or not.
 *
 * @param	InstancePtr	Pointer to the Ecc instance.
 *
 * @return
 * 		- XASUFW_SUCCESS, if operation is successful
 * 		- XASUFW_FAILURE, if operation fails
 *
 *************************************************************************************************/
static inline s32 XEcc_WaitForDone(const XEcc *InstancePtr)
{
	s32 Status = XASUFW_FAILURE;

	/* Check for ECC operation is completed with in Timeout(10sec) or not */
	Status = (s32)Xil_WaitForEvent(InstancePtr->BaseAddress + XECC_ISR_OFFSET,
				       XECC_ISR_DONE_MASK, XECC_ISR_DONE_MASK, XECC_TIMEOUT_MAX);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* Clear interrupt */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_ISR_OFFSET, XECC_ISR_DONE_MASK);

	/* Disable interrupt */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_IDR_OFFSET, XECC_IDR_DONE_MASK);
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is to update the control register for the respective operation.
 *
 * @param	InstancePtr	Pointer to the Ecc instance.
 * @param	CurveType	Type of the curve.
 * @param	OpCode		ECC operation code.
 *
 * @return
 * 		- Returns the value to be updated to the control register.
 *
 *************************************************************************************************/
static inline u32 XEcc_ConfigureEngine(const XEcc *InstancePtr, u32 OpCode)
{
	u32 CtrlRegValue = 0U;
	u32 CurveType = InstancePtr->CurveInfo->CurveType;

	/* Configure ECC curve type */
	CtrlRegValue = XECC_CTRL_CURVE_MASK & (CurveType << XECC_CTRL_CURVE_SHIFT);

	/* Configure ECC operation */
	CtrlRegValue |= XECC_CTRL_OPCODE_MASK & (OpCode << XECC_CTRL_OPCODE_SHIFT);

	/* Configure ECC countermeasure */
	if (InstancePtr->CmConfig == XASUFW_CONFIG_ENABLE) {
		CtrlRegValue |= (XECC_SUPPRESS_SCP_MASK);
	}

	return CtrlRegValue;
}

/*************************************************************************************************/
/**
*
* @brief	XEcc_LookupConfig returns a reference to an XEcc_Config structure based on the
*		unique device id.
*
* @param	DeviceId	Unique device ID of the device for the lookup operation.
*
* @return
*		- It returns CfgPtr is a reference to a config record in the configuration table
*		  corresponding to <i>DeviceId</i>
* 		- It returns NULL if no match is found.
*
**************************************************************************************************/
static XEcc_Config *XEcc_LookupConfig(u32 DeviceId)
{
	XEcc_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = 0x0U; Index < XASU_XECC_NUM_INSTANCES; Index++) {
		if (XEcc_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XEcc_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

/*************************************************************************************************/
/**
 * @brief	This function updates the configuration and starts the operation
 *
 * @param	InstancePtr	Pointer to the Ecc instance.
 * @param	CurveType	Pointer to curve information
 * @param	OpCode		Memory offset to be used for getting the output
 *
 * @return
 * 		- Upon successful operation, returns XASUFW_SUCCESS.
 *		- Otherwise, returns XASUFW_FAILURE.
 *
 *************************************************************************************************/
static s32 XEcc_ConfigNStartOperation(const XEcc *InstancePtr, u32 OpCode)
{
	s32 Status = XASUFW_FAILURE;
	u32 TerminationCode = 0U;
	u32 CtrlRegValue = 0U;

	/* Enable interrupt */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_IER_OFFSET, XECC_IER_DONE_MASK);

	/* Update config and Set start */
	CtrlRegValue = (XEcc_ConfigureEngine(InstancePtr, OpCode) |
			XECC_CTRL_START_MASK);
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_CTRL_OFFSET, CtrlRegValue);

	/* Wait for done interrupt */
	Status = XEcc_WaitForDone(InstancePtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WAIT_FOR_DONE_TIMEOUT;
		XFIH_GOTO(END);
	}

	/* Check for the status of operation by reading the error status*/
	TerminationCode = XAsufw_ReadReg(InstancePtr->BaseAddress + XECC_STATUS_OFFSET);
	TerminationCode &= XECC_STATUS_TERMINATION_CODE_MASK;
	if (TerminationCode != 0U) {
		Status = (s32)(TerminationCode | XASUFW_ECC_TERMINATION_CODE_MASK);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates input parameters
 *
 * @param	InstancePtr	Pointer to the Ecc instance.
 * @param	CurveType	Type of curve
 *
 * @return
 * 		- Upon successful validation, returns XASUFW_SUCCESS.
 *		- XASUFW_ECC_INVALID_PARAM, if InstancePtr is NULL or if curve type is invalid
 *		- XASUFW_ECC_INIT_NOT_DONE, if ECC component is not ready
 *
 *************************************************************************************************/
static s32 XEcc_InputValidate(const XEcc *InstancePtr, u32 CurveType)
{
	s32 Status = XASUFW_FAILURE;

	if (InstancePtr == NULL) {
		Status = XASUFW_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if (InstancePtr->IsReady != XIL_COMPONENT_IS_READY) {
		Status = XASUFW_ECC_INIT_NOT_DONE;
		XFIH_GOTO(END);
	}

	if ((CurveType != XECC_CURVE_TYPE_NIST_P256) && (CurveType != XECC_CURVE_TYPE_NIST_P384)) {
		Status = XASUFW_ECC_INVALID_PARAM;
		XFIH_GOTO(END);
	} else {
		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}
