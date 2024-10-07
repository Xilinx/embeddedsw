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
*       yog  09/26/2024 Added doxygen groupings and fixed doxygen comments.
*
* </pre>
*
**************************************************************************************************/
/**
* @addtogroup xecc_server_apis ECC Server APIs
* @{
*/

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
#define XECC_CURVES_SUPPORTED		(2U) /**< Curves P-256 and P-384 are supported for ECC engine */
#define XECC_RESET_ASSERT		(1U) /**< ECC reset assert value */
#define XECC_RESET_DEASSERT		(0U) /**< ECC reset deassert value */
#define XECC_TIMEOUT_MAX		(0x1FFFFU) /**< ECC done timeout */
						/* TBD: need to calculate as part of VNC */
#define XECC_DOUBLE_CURVE_LENGTH_SHIFT	(0x1U) /**< Shift value to double the curve length */
#define XECC_SUPPRESS_SCP_SCP2_MASK	(XECC_CTRL_SUPPRESS_SCP_MASK | \
		XECC_CTRL_SUPPRESS_SCP2_MASK)  /**< Mask for enabling suppress SCP and SCP2 */
#define XECC_CFG_WR_RD_ENDIANNESS_MASK	(XECC_CFG_WR_ENDIANNESS_MASK | \
		XECC_CFG_RD_ENDIANNESS_MASK)  /**< Mask for enabling write and read endianness */

/************************************** Type Definitions *****************************************/

/**
* @brief Structure to get curve info
*/
typedef struct {
	u32 CurveType; /**< Type of the curve */
	u32 CurveBytes; /**< Length of the curve in bytes*/
} XEcc_CurveInfo;

/**
* @brief This structure contains configuration information for a ECC core.
* Each core should have an associated configuration structure.
*/
struct _XEcc_Config {
	u32 DeviceId; /**< DeviceId is the unique ID of the device */
	u32 BaseAddress; /**< BaseAddress is the physical base address of the device's registers */
};

/**
* @brief ECC driver instance data structure. A pointer to an instance data
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
 * @brief	This function returns an ECC instance pointer of the provided device ID.
 *
 * @param	DeviceId	The device ID of ECC core.
 *
 * @return
 * 		- It returns pointer to the XEcc_Instance corresponding to the Device ID.
 * 		- It returns NULL if the device ID is invalid.
 *
 *************************************************************************************************/
XEcc *XEcc_GetInstance(u32 DeviceId)
{
	XEcc *XEcc_InstancePtr = NULL;

	if (DeviceId >= XASU_XECC_NUM_INSTANCES) {
		goto END;
	}

	XEcc_InstancePtr = &XEcc_Instance[DeviceId];
	XEcc_InstancePtr->DeviceId = DeviceId;

END:
	return XEcc_InstancePtr;
}

/*************************************************************************************************/
/**
*
* @brief	This function initializes the ECC instance.
*
* @param	InstancePtr	Pointer to the Ecc instance.
*
* @return
*		- XASUFW_SUCCESS, if initialization is successful.
*		- XASUFW_ECC_INVALID_PARAM, if InstancePtr or CfgPtr is NULL.
*
**************************************************************************************************/
s32 XEcc_Initialize(XEcc *InstancePtr)
{
	s32 Status = XASUFW_FAILURE;
	const XEcc_Config *CfgPtr = NULL;

	/** Validate input parameters. */
	if (InstancePtr == NULL) {
		Status = XASUFW_ECC_INVALID_PARAM;
		goto END;
	}
	CfgPtr = XEcc_LookupConfig(InstancePtr->DeviceId);
	if (CfgPtr == NULL) {
		Status = XASUFW_ECC_INVALID_PARAM;
		goto END;
	}

	/** Initialize ECC instance. */
	InstancePtr->BaseAddress = CfgPtr->BaseAddress;
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	InstancePtr->CmConfig = XASUFW_ECC_CM_CONFIG;

	Status = XASUFW_SUCCESS;
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates the public key using the provided private key for the
 * 		specified elliptic curve using ECC core.
 *
 * @param	InstancePtr	Pointer to the Ecc instance.
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	CurveType	ECC curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PrivKeyAddr	Address of the private key buffer, whose length shall be equal to
 * 				CurveLen.
 * @param	PubKeyAddr	Address of the buffer to store the generated public key, whose
 * 				length shall be	equal to double of CurveLen as it contains both
 * 				Qx, Qy components.
 *
 * @return
 * 		- XASUFW_SUCCESS, if public key generated successfully.
 * 		- XASUFW_ECC_INVALID_PARAM, if InstancePtr is NULL or if curve type or curve length
 * 				is invalid.
 * 		- XASUFW_ECC_WRITE_DATA_FAIL, if write data to registers through DMA fails.
 * 		- XASUFW_ECC_READ_DATA_FAIL, if read data from registers through DMA fails.
 * 		- Also can return termination error codes from 0x21U to 0x2CU from core,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XEcc_GeneratePublicKey(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
			   u64 PrivKeyAddr, u64 PubKeyAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XEcc_CurveInfo *CurveInfo = NULL;
	u32 Offset = XECC_MEM_GEN_KEY_PVT_KEY_OFFSET;

	/** Validate input parameters. */
	Status = XEcc_InputValidate(InstancePtr, CurveType);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if ((DmaPtr == NULL) || (PrivKeyAddr == 0U) || (PubKeyAddr == 0U)) {
		Status = XASUFW_ECC_INVALID_PARAM;
		goto END;
	}

	CurveInfo = &XEcc_CurveInfoTable[CurveType];
	InstancePtr->CurveInfo = CurveInfo;

	if (CurveLen != CurveInfo->CurveBytes) {
		Status = XASUFW_ECC_INVALID_PARAM;
		goto END;
	}

	/** Release Reset. */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);

	/** Enable endianness for write and read operations. */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_CFG_OFFSET,
			XECC_CFG_WR_RD_ENDIANNESS_MASK);

	/** Copy private key to respective registers using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, PrivKeyAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	/** Update configuration and start the operation. */
	XFIH_CALL_GOTO(XEcc_ConfigNStartOperation, XFihVar, Status, END, InstancePtr,
			XECC_CTRL_PUB_KEY_GENERATION_OP_CODE);

	/** Copy public key from registers to destination address using DMA. */
	Offset = XECC_MEM_PUB_KEY_X_OFFSET;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset),
					   PubKeyAddr, CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_READ_DATA_FAIL;
		goto END;
	}

	Offset = XECC_MEM_PUB_KEY_Y_OFFSET;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset),
					   (PubKeyAddr + CurveLen), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_READ_DATA_FAIL;
		goto END;
	}

END:
	if (InstancePtr != NULL) {
		/** Set ECC under reset. */
		XAsufw_CryptoCoreSetReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the provided ECDSA public key for the specified elliptic
 * 		curve using ECC core.
 *
 * @param	InstancePtr	Pointer to the Ecc instance.
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	CurveType	ECC curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PubKeyAddr	Address of the public key buffer, whose length shall be equal to
 * 				double of CurveLen as it contains both Qx, Qy components.
 *
 * @return
 * 		- XASUFW_SUCCESS, if public key provided is valid.
 * 		- XASUFW_ECC_INVALID_PARAM, if InstancePtr is NULL or if curve type or curve length
 * 				is invalid.
 * 		- XASUFW_ECC_WRITE_DATA_FAIL - if write data to registers through DMA fails.
 * 		- Also can return termination error codes from 0x21U to 0x2CU from core,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XEcc_ValidatePublicKey(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
			   u64 PubKeyAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XEcc_CurveInfo *CurveInfo = NULL;
	u32 Offset = XECC_MEM_PUB_KEY_X_OFFSET;

	/** Validate input parameters. */
	Status = XEcc_InputValidate(InstancePtr, CurveType);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if ((DmaPtr == NULL) || (PubKeyAddr == 0U)) {
		Status = XASUFW_ECC_INVALID_PARAM;
		goto END;
	}

	CurveInfo = &XEcc_CurveInfoTable[CurveType];
	InstancePtr->CurveInfo = CurveInfo;

	if (CurveLen != CurveInfo->CurveBytes) {
		Status = XASUFW_ECC_INVALID_PARAM;
		goto END;
	}

	/** Release Reset. */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);

	/** Enable endianness for write and read operations. */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_CFG_OFFSET,
			XECC_CFG_WR_RD_ENDIANNESS_MASK);

	/** Copy public key to respective registers using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	Offset = XECC_MEM_PUB_KEY_Y_OFFSET;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (PubKeyAddr + CurveLen),
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	/** Update configuration and start the operation. */
	XFIH_CALL(XEcc_ConfigNStartOperation, XFihVar, Status, InstancePtr,
			XECC_CTRL_PUB_KEY_VALIDATION_OP_CODE);

END:
	if (InstancePtr != NULL) {
		/** Set ECC under reset. */
		XAsufw_CryptoCoreSetReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates an ECDSA signature for the provided hash by using the
 * 		given private key associated with the elliptic curve using ECC core.
 *
 * @param	InstancePtr	Pointer to the Ecc instance.
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	CurveType	ECC curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PrivKeyAddr	Address of the private key buffer, whose length shall be equal to
 * 				CurveLen.
 * @param	EphemeralKeyPtr	Pointer to the ephemeral key buffer, whose length shall be equal to
 * 				CurveLen.
 * @param	HashAddr	Address of the hash on which signature has to be generated.
 * @param	HashBufLen	Length of the hash in bytes.
 * @param	SignAddr	Address of the buffer to store the generated signature, whose
 * 				length shall be	equal to double of CurveLen as it contains both
 * 				r, s components.
 *
 * @return
 * 		- XASUFW_SUCCESS, if signature generation is successful.
 * 		- XASUFW_ECC_INVALID_PARAM, if InstancePtr is NULL or if curve type is invalid or
 * 			CurveLen and HashBufLen are invalid.
 * 		- XASUFW_ECC_WRITE_DATA_FAIL, if write data to registers through DMA fails.
 * 		- XASUFW_ECC_READ_DATA_FAIL,  if read data from registers through DMA fails.
 * 		- Also can return termination error codes from 0x21U to 0x2CU from core,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XEcc_GenerateSignature(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
			   u64 PrivKeyAddr, const u8 *EphemeralKeyPtr, u64 HashAddr, u32 HashBufLen,
			   u64 SignAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XEcc_CurveInfo *CurveInfo = NULL;
	u32 Offset = XECC_MEM_GEN_SIGN_PVT_KEY_OFFSET;

	/** Validate input parameters. */
	Status = XEcc_InputValidate(InstancePtr, CurveType);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if ((DmaPtr == NULL) || (PrivKeyAddr == 0U) || (EphemeralKeyPtr == NULL) ||
			(HashAddr == 0U) || (SignAddr == 0U)) {
		Status = XASUFW_ECC_INVALID_PARAM;
		goto END;
	}

	CurveInfo = &XEcc_CurveInfoTable[CurveType];
	InstancePtr->CurveInfo = CurveInfo;

	if ((CurveLen != CurveInfo->CurveBytes) || (HashBufLen != CurveInfo->CurveBytes)) {
		Status = XASUFW_ECC_INVALID_PARAM;
		goto END;
	}

	/** Release Reset. */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);

	/** Enable endianness for write and read operations. */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_CFG_OFFSET,
			XECC_CFG_WR_RD_ENDIANNESS_MASK);

	/** Copy private key and hash to respective registers using DMA.*/
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, PrivKeyAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	Offset = XECC_MEM_HASH_OFFSET;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, HashAddr,
			(u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), HashBufLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	/** Copy ephemeral key to respective registers. */
	Offset = XECC_MEM_EPHEMERAL_KEY_OFFSET;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy((u8*)(InstancePtr->BaseAddress + Offset), CurveLen,
			EphemeralKeyPtr, CurveLen, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	/** Update configuration and start the operation. */
	XFIH_CALL_GOTO(XEcc_ConfigNStartOperation, XFihVar, Status, END, InstancePtr,
			XECC_CTRL_SIGN_GENERATION_OP_CODE);

	/** Copy generated signature from registers to destination address using DMA. */
	Offset = XECC_MEM_SIGN_R_OFFSET;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset),
					   SignAddr, CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_READ_DATA_FAIL;
		goto END;
	}

	Offset = XECC_MEM_SIGN_S_OFFSET;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset),
					   (SignAddr + CurveLen), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_READ_DATA_FAIL;
		goto END;
	}

END:
	if (InstancePtr != NULL) {
		/** Set ECC under reset. */
		XAsufw_CryptoCoreSetReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function verifies the validity of an ECDSA signature for the provided hash
 * 		using the provided ecc public key on ECC core.
 *
 * @param	InstancePtr	Pointer to the Ecc instance.
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	CurveType	ECC Curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PubKeyAddr	Address of the public key buffer, whose length shall be equal to
 * 				double of CurveLen as it contains both Qx, Qy components.
 * @param	HashAddr	Address of the hash for which signature has to be verified.
 * @param	HashBufLen	Length of the hash in bytes.
 * @param	SignAddr	Address of the signature buffer, whose length shall be equal to
 * 				double of CurveLen as it contains both r, s components.
 *
 * @return
 * 		- XASUFW_SUCCESS, if signature provided is valid.
 * 		- XASUFW_ECC_INVALID_PARAM, if InstancePtr is NULL or if curve type is invalid or
 * 			CurveLen and HashBufLen are invalid.
 * 		- XASUFW_ECC_WRITE_DATA_FAIL, if write data to registers through DMA fails.
 * 		- Also can return termination error codes from 0x21U to 0x2CU from core,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XEcc_VerifySignature(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
			 u64 PubKeyAddr, u64 HashAddr, u32 HashBufLen, u64 SignAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XEcc_CurveInfo *CurveInfo = NULL;
	u32 Offset = XECC_MEM_SIGN_R_OFFSET;

	/** Validate input parameters. */
	Status = XEcc_InputValidate(InstancePtr, CurveType);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if ((DmaPtr == NULL) || (PubKeyAddr == 0U) || (HashAddr == 0U) || (SignAddr == 0U)) {
		Status = XASUFW_ECC_INVALID_PARAM;
		goto END;
	}

	CurveInfo = &XEcc_CurveInfoTable[CurveType];
	InstancePtr->CurveInfo = CurveInfo;

	if ((CurveLen != CurveInfo->CurveBytes) || (HashBufLen != CurveInfo->CurveBytes)) {
		Status = XASUFW_ECC_INVALID_PARAM;
		goto END;
	}

	/** Release Reset. */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);

	/** Enable endianness for write and read operations. */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_CFG_OFFSET,
			XECC_CFG_WR_RD_ENDIANNESS_MASK);

	/** Copy signature, hash and public key to respective registers using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, SignAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	Offset = XECC_MEM_SIGN_S_OFFSET;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, SignAddr + CurveLen,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	Offset = XECC_MEM_HASH_OFFSET;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, HashAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), HashBufLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	Offset = XECC_MEM_PUB_KEY_X_OFFSET;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	Offset = XECC_MEM_PUB_KEY_Y_OFFSET;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (PubKeyAddr + CurveLen),
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + Offset), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	/** Update configuration and start the operation. */
	XFIH_CALL(XEcc_ConfigNStartOperation, XFihVar, Status, InstancePtr,
			XECC_CTRL_SIGN_VERIFICATION_OP_CODE);

END:
	if (InstancePtr != NULL) {
		/** Set ECC under reset. */
		XAsufw_CryptoCoreSetReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function will wait for ECC core completion.
 *
 * @param	InstancePtr	Pointer to the Ecc instance.
 *
 * @return
 * 		- XASUFW_SUCCESS, if wait for done is successful.
 * 		- XASUFW_FAILURE, upon timeout.
 *
 *************************************************************************************************/
static inline s32 XEcc_WaitForDone(const XEcc *InstancePtr)
{
	s32 Status = XASUFW_FAILURE;

	/** Check whether ECC operation is completed within Timeout(10sec) or not. */
	Status = (s32)Xil_WaitForEvent(InstancePtr->BaseAddress + XECC_ISR_OFFSET,
				       XECC_ISR_DONE_MASK, XECC_ISR_DONE_MASK, XECC_TIMEOUT_MAX);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Clear interrupt */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_ISR_OFFSET, XECC_ISR_DONE_MASK);

	/* Disable interrupt */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_IDR_OFFSET, XECC_IDR_DONE_MASK);
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns the control register configuration for the selected
 * 		operation.
 *
 * @param	InstancePtr	Pointer to the Ecc instance.
 * @param	OpCode		ECC operation code to select the operation.
 * 				0 - Signature verification
 * 				1 - public key validation
 * 				2 - public key generation
 * 				3 - signature generation
 *
 * @return
 * 		- Returns the value to be updated to the control register.
 *
 *************************************************************************************************/
static inline u32 XEcc_ConfigureEngine(const XEcc *InstancePtr, u32 OpCode)
{
	u32 CtrlRegValue = 0U;
	u32 CurveType = InstancePtr->CurveInfo->CurveType;

	/** Configure ECC curve type, operation and countermeasures. */
	CtrlRegValue = XECC_CTRL_CURVE_MASK & (CurveType << XECC_CTRL_CURVE_SHIFT);

	CtrlRegValue |= XECC_CTRL_OPCODE_MASK & (OpCode << XECC_CTRL_OPCODE_SHIFT);

	if (InstancePtr->CmConfig == XASUFW_CONFIG_ENABLE) {
		CtrlRegValue |= (XECC_SUPPRESS_SCP_SCP2_MASK);
	}

	return CtrlRegValue;
}

/*************************************************************************************************/
/**
*
* @brief	This function returns a pointer reference of XEcc_Config structure based on the
*		device ID.
*
* @param	DeviceId	The device ID of the ECC core.
*
* @return
* 		- CfgPtr, a reference to a config record in the configuration table
* 			corresponding to <i>DeviceId</i>
* 		- NULL, if no valid device ID is found.
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
 * @brief	This function does the required configuration and starts the operation.
 *
 * @param	InstancePtr	Pointer to the Ecc instance.
 * @param	OpCode		ECC operation code to select the operation.
 * 				0 - Signature verification
 * 				1 - public key validation
 * 				2 - public key generation
 * 				3 - signature generation
 *
 * @return
 * 		- XASUFW_SUCCESS, if operation is successful.
 * 		- XASUFW_ECC_WAIT_FOR_DONE_TIMEOUT, upon time out.
 * 		- Also can return termination error codes from 0x21U to 0x2CU from core,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
static s32 XEcc_ConfigNStartOperation(const XEcc *InstancePtr, u32 OpCode)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 TerminationCode = 0U;
	u32 CtrlRegValue = 0U;

	/** Enable interrupt, update configuration in control register and start the operation. */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_IER_OFFSET, XECC_IER_DONE_MASK);

	CtrlRegValue = (XEcc_ConfigureEngine(InstancePtr, OpCode) |
			XECC_CTRL_START_MASK);
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_CTRL_OFFSET, CtrlRegValue);

	/** Wait for done. */
	Status = XEcc_WaitForDone(InstancePtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WAIT_FOR_DONE_TIMEOUT;
		goto END;
	}

	/**
	 * Check for the status of operation by reading the termination code field in status
	 * register and if there is a failure return the termination code received from core.
	 */
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
 * @brief	This function validates input parameters.
 *
 * @param	InstancePtr	Pointer to the Ecc instance.
 * @param	CurveType	Type of the curve.
 *
 * @return
 * 		- XASUFW_SUCCESS, if inputs are validated successfully.
 * 		- XASUFW_ECC_INVALID_PARAM, if InstancePtr is NULL or if curve type is invalid.
 * 		- XASUFW_ECC_INIT_NOT_DONE, if ECC component is not ready.
 *
 *************************************************************************************************/
static s32 XEcc_InputValidate(const XEcc *InstancePtr, u32 CurveType)
{
	s32 Status = XASUFW_FAILURE;

	if (InstancePtr == NULL) {
		Status = XASUFW_ECC_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->IsReady != XIL_COMPONENT_IS_READY) {
		Status = XASUFW_ECC_INIT_NOT_DONE;
		goto END;
	}

	if ((CurveType != XECC_CURVE_TYPE_NIST_P256) && (CurveType != XECC_CURVE_TYPE_NIST_P384)) {
		Status = XASUFW_ECC_INVALID_PARAM;
		goto END;
	} else {
		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}
/** @} */
