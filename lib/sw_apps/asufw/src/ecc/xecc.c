/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
*       am   02/21/2025 Integrated performance measurement macros
*       yog  03/13/2025 Removed CmConfig variable in InstancePtr and used the macro directly.
*       yog  03/21/2025 Added PWCT support
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
#include "xasufw_trnghandler.h"

/************************************ Constant Definitions ***************************************/
#define XECC_CURVES_SUPPORTED		(2U) /**< Curves P-256 and P-384 are supported for ECC engine */
#define XECC_TIMEOUT_MAX		(0x1FFFFU) /**< ECC done timeout */
						/* TBD: need to calculate as part of VNC */
#define XECC_DOUBLE_CURVE_LENGTH_SHIFT	(0x1U) /**< Shift value to double the curve length */
#define XECC_SUPPRESS_SCP_SCP2_MASK	(XECC_CTRL_SUPPRESS_SCP_MASK | \
		XECC_CTRL_SUPPRESS_SCP2_MASK)  /**< Mask for enabling suppress SCP and SCP2 */
#define XECC_CFG_WR_RD_ENDIANNESS_MASK	(XECC_CFG_WR_ENDIANNESS_MASK | \
		XECC_CFG_RD_ENDIANNESS_MASK)  /**< Mask for enabling write and read endianness */
#define XECC_SCP_RANDOM_MSB_BIT_SET_MASK	(0x7FU) /**< Mask for setting MSB bit of SCP random values to 0 */
#define XECC_SCP_RANDOM_1_MSB_BYTE_OFFSET	(0U) /**< MSB byte offset of SCP random 1 value */
#define XECC_RAND_NUM_REG_OFFSET		(48U) /**< Offset to be added to write random
								numbers to the registers */
#define XECC_RAND_NUM_COUNT_GEN_PUB_KEY		(2U) /**< Number of random values required for
								generate public key operation. */
#define XECC_RAND_NUM_COUNT_GEN_SIGN		(4U) /**< Number of random values required for
								generate SIGN operation. */

/************************************** Type Definitions *****************************************/

/**
* @brief This structure contains curve related information.
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
	u32 DeviceId; /**< ECC Device Id */
	u32 BaseAddress; /**< ECC Base address */
	u32 IsReady; /**< ECC component ready state */
	XEcc_CurveInfo *CurveInfo; /**< ECC curve information */
};

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static XEcc_Config *XEcc_LookupConfig(u32 DeviceId);
static inline s32 XEcc_WaitForDone(const XEcc *InstancePtr);
static s32 XEcc_ConfigNStartOperation(const XEcc *InstancePtr, u32 OpCode);
static s32 XEcc_InputValidate(const XEcc *InstancePtr, u32 CurveType);
#ifdef XASU_ECC_CM_ENABLE
static s32 XEcc_GenNdUpdateRandNumToReg(XEcc *InstancePtr, u32 CurveLen, u32 Count);
#endif

/************************************ Variable Definitions ***************************************/

/** ECC configuration table for ECC crypto devices */
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
		XASU_ECC_P256_SIZE_IN_BYTES
	},
	{
		XECC_CURVE_TYPE_NIST_P384,
		XASU_ECC_P384_SIZE_IN_BYTES
	}
};

/** Message to be used for pair wise consistency test. */
static const u8 MsgPwctEcc[XASU_ECC_P384_SIZE_IN_BYTES] = {
	0x2FU, 0xBFU, 0x02U, 0x9EU, 0xE9U, 0xFBU, 0xD6U, 0x11U,
	0xC2U, 0x4DU, 0x81U, 0x4EU, 0x6AU, 0xFFU, 0x26U, 0x77U,
	0xC3U, 0x5AU, 0x83U, 0xBCU, 0xE5U, 0x63U, 0x2CU, 0xE7U,
	0x89U, 0x43U, 0x6CU, 0x68U, 0x82U, 0xCAU, 0x1CU, 0x71U,
	0xF8U, 0x2BU, 0x72U, 0xD3U, 0xA4U, 0xC2U, 0x8EU, 0x10U,
	0xD8U, 0x25U, 0x5DU, 0x21U, 0x33U, 0xD5U, 0xCAU, 0x38U
};

/** Ephemeral Key to be used for pair wise consistency test. */
static const u8 EKeyPwctEcc[XASU_ECC_P384_SIZE_IN_BYTES] = {
	0x36U, 0x77U, 0xFBU, 0xF9U, 0xBBU, 0x2DU, 0x96U, 0xA3U,
	0x1BU, 0x01U, 0x11U, 0x08U, 0x57U, 0x93U, 0x8CU, 0xC4U,
	0x9DU, 0x9AU, 0x30U, 0xA4U, 0xE0U, 0x0EU, 0x9CU, 0xD4U,
	0xB5U, 0x5DU, 0x97U, 0x77U, 0x58U, 0x0CU, 0x84U, 0xC7U,
	0x0CU, 0x67U, 0x48U, 0x94U, 0xE8U, 0x53U, 0xD3U, 0x6BU,
	0xBEU, 0xC6U, 0xC2U, 0x1FU, 0xDCU, 0xFCU, 0x7BU, 0xD1U
};

/*************************************************************************************************/
/**
 * @brief	This function returns an ECC instance pointer of the provided device ID.
 *
 * @param	DeviceId	The device ID of ECC core.
 *
 * @return
 * 		- Pointer to the XEcc_Instance corresponding to the Device ID.
 * 		- NULL, if the device ID is invalid.
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
* @param	InstancePtr	Pointer to the ECC instance.
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

	Status = XASUFW_SUCCESS;
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates the public key using the provided private key for the
 * 		specified elliptic curve using ECC core.
 *
 * @param	InstancePtr	Pointer to the ECC instance.
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	CurveType	ECC curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PrivKeyAddr	Address of the private key buffer, whose length shall be equal to CurveLen.
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
 * 		- XASUFW_RSA_ECC_PWCT_SIGN_GEN_FAIL, if sign generation fails in PWCT.
 *		- XASUFW_RSA_ECC_PWCT_SIGN_VER_FAIL, if sign verification fails in PWCT.
 *		- XASUFW_ECC_SCP_RANDOM_NUM_UPDATE_FAIL, if random values updation fails.
 * 		- Also, this function can return termination error codes from 0x21U to 0x2CU from core,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XEcc_GeneratePublicKey(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
			   u64 PrivKeyAddr, u64 PubKeyAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEccVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XEcc_CurveInfo *CurveInfo = NULL;

	/** Validate input parameters. */
	Status = XEcc_InputValidate(InstancePtr, CurveType);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_INVALID_PARAM);
		goto END;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
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

	/** Release ECC core reset. */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);

	/** Enable endianness for write and read operations. */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_CFG_OFFSET,
			XECC_CFG_WR_RD_ENDIANNESS_MASK);

	/** Copy private key to respective registers using DMA. */
	Status = XAsufw_DmaXfr(DmaPtr, PrivKeyAddr,
			(u64)(UINTPTR)(InstancePtr->BaseAddress + XECC_MEM_GEN_KEY_PVT_KEY_OFFSET),
			CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

#ifdef XASU_ECC_CM_ENABLE
	/**
	 * When SCP is enabled, the public key generation operation requires two random numbers.
	 * Repeat the following steps twice to generate two random numbers.
	 * - Generate a random number with a length equal to the curve length (CurveLen).
	 * - Clear the most significant bit (MSB) of the buffer.
	 * - Copy the buffer contents to the corresponding registers.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcc_GenNdUpdateRandNumToReg(InstancePtr, CurveLen,
				XECC_RAND_NUM_COUNT_GEN_PUB_KEY);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_SCP_RANDOM_NUM_UPDATE_FAIL);
		goto END;
	}
#endif
	/** Update configuration and start the operation. */
	XFIH_CALL_GOTO(XEcc_ConfigNStartOperation, XFihEccVar, Status, END, InstancePtr,
			XECC_CTRL_PUB_KEY_GENERATION_OP_CODE);

	/** Copy public key from registers to destination address using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + XECC_MEM_PUB_KEY_X_OFFSET),
					   PubKeyAddr, CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_READ_DATA_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + XECC_MEM_PUB_KEY_Y_OFFSET),
			       (PubKeyAddr + CurveLen), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_READ_DATA_FAIL;
		goto END;
	}

	/** Validate the public key generated from the private key. */
	XFIH_CALL_GOTO(XEcc_ValidatePublicKey, XFihEccVar, Status, END, InstancePtr, DmaPtr, CurveType,
		CurveLen, PubKeyAddr);

	/** Perform pair wise consistency test using the key pair. */
	XFIH_CALL(XEcc_Pwct, XFihEccVar, Status, InstancePtr, DmaPtr, CurveType, CurveLen,
		PrivKeyAddr, PubKeyAddr);

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
 * @param	InstancePtr	Pointer to the ECC instance.
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
 * 		- XASUFW_ECC_WRITE_DATA_FAIL, if write data to registers through DMA fails.
 * 		- Also, this function can return termination error codes from 0x21U to 0x2CU from core,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XEcc_ValidatePublicKey(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
			   u64 PubKeyAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEccVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XEcc_CurveInfo *CurveInfo = NULL;

	/** Validate input parameters. */
	Status = XEcc_InputValidate(InstancePtr, CurveType);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_INVALID_PARAM);
		goto END;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
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

	/** Release ECC core reset. */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);

	/** Enable endianness for write and read operations. */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_CFG_OFFSET,
			XECC_CFG_WR_RD_ENDIANNESS_MASK);

	/** Copy public key to respective registers using DMA. */
	Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + XECC_MEM_PUB_KEY_X_OFFSET),
			       CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (PubKeyAddr + CurveLen),
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + XECC_MEM_PUB_KEY_Y_OFFSET),
			       CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	/** Update configuration and start the operation. */
	XFIH_CALL(XEcc_ConfigNStartOperation, XFihEccVar, Status, InstancePtr,
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
 * @param	InstancePtr	Pointer to the ECC instance.
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	CurveType	ECC curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PrivKeyAddr	Address of the private key buffer, whose length shall be equal to CurveLen.
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
 * 		- XASUFW_ECC_SCP_RANDOM_NUM_UPDATE_FAIL, if random values updation fails.
 * 		- Also, this function can return termination error codes from 0x21U to 0x2CU from core,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XEcc_GenerateSignature(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
			   u64 PrivKeyAddr, const u8 *EphemeralKeyPtr, u64 HashAddr, u32 HashBufLen,
			   u64 SignAddr)
{
	/**
	 * Capture start time of ECC signature generation operation if performance measurement is
	 * enabled.
	 */
	XASUFW_MEASURE_PERF_START(StartTime, PerfTime);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEccVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XEcc_CurveInfo *CurveInfo = NULL;

	/** Validate input parameters. */
	Status = XEcc_InputValidate(InstancePtr, CurveType);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_INVALID_PARAM);
		goto END;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
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

	/** Release ECC core reset. */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);

	/** Enable endianness for write and read operations. */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_CFG_OFFSET,
			XECC_CFG_WR_RD_ENDIANNESS_MASK);

	/** Copy private key and hash to respective registers using DMA. */
	Status = XAsufw_DmaXfr(DmaPtr, PrivKeyAddr,
			(u64)(UINTPTR)(InstancePtr->BaseAddress + XECC_MEM_GEN_SIGN_PVT_KEY_OFFSET),
			CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, HashAddr,
			(u64)(UINTPTR)(InstancePtr->BaseAddress + XECC_MEM_HASH_OFFSET),
			HashBufLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	/** Copy ephemeral key to respective registers. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy((u8*)(InstancePtr->BaseAddress + XECC_MEM_EPHEMERAL_KEY_OFFSET),
			CurveLen, EphemeralKeyPtr, CurveLen, CurveLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

#ifdef XASU_ECC_CM_ENABLE
	/**
	 * When SCP is enabled, the public key generation operation requires four random numbers.
	 * Repeat the following steps four times to generate four random numbers.
	 * - Generate a random number with a length equal to the curve length (CurveLen).
	 * - Clear the most significant bit (MSB) of the buffer.
	 * - Copy the buffer contents to the corresponding registers.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcc_GenNdUpdateRandNumToReg(InstancePtr, CurveLen,
				XECC_RAND_NUM_COUNT_GEN_SIGN);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_SCP_RANDOM_NUM_UPDATE_FAIL);
		goto END;
	}
#endif

	/** Update configuration and start the operation. */
	XFIH_CALL_GOTO(XEcc_ConfigNStartOperation, XFihEccVar, Status, END, InstancePtr,
			XECC_CTRL_SIGN_GENERATION_OP_CODE);

	/** Copy generated signature from registers to destination address using DMA. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + XECC_MEM_SIGN_R_OFFSET),
			       SignAddr, CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_READ_DATA_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + XECC_MEM_SIGN_S_OFFSET),
			       (SignAddr + CurveLen), CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_READ_DATA_FAIL;
	}

	/**
	 * Measure ECC signature generation operation performance time if performance measurement is
	 * enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(StartTime, PerfTime, __func__);

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
 * 		using the provided ECC public key on ECC core.
 *
 * @param	InstancePtr	Pointer to the ECC instance.
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
 * 		- Also, this function can return termination error codes from 0x21U to 0x2CU from core,
 * 		please refer to xasufw_status.h.
 *
 *************************************************************************************************/
s32 XEcc_VerifySignature(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
			 u64 PubKeyAddr, u64 HashAddr, u32 HashBufLen, u64 SignAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihEccVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XEcc_CurveInfo *CurveInfo = NULL;

	/** Validate input parameters. */
	Status = XEcc_InputValidate(InstancePtr, CurveType);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_INVALID_PARAM);
		goto END;
	}
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
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

	/** Release ECC core reset. */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->BaseAddress, XECC_RESET_OFFSET);

	/** Enable endianness for write and read operations. */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_CFG_OFFSET,
			XECC_CFG_WR_RD_ENDIANNESS_MASK);

	/** Copy signature, hash and public key to respective registers using DMA. */
	Status = XAsufw_DmaXfr(DmaPtr, SignAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + XECC_MEM_SIGN_R_OFFSET),
			       CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, SignAddr + CurveLen,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + XECC_MEM_SIGN_S_OFFSET),
			       CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, HashAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + XECC_MEM_HASH_OFFSET),
			       HashBufLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, PubKeyAddr,
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + XECC_MEM_PUB_KEY_X_OFFSET),
			       CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (PubKeyAddr + CurveLen),
			       (u64)(UINTPTR)(InstancePtr->BaseAddress + XECC_MEM_PUB_KEY_Y_OFFSET),
			       CurveLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_WRITE_DATA_FAIL;
		goto END;
	}

	/** Update configuration and start the operation. */
	XFIH_CALL(XEcc_ConfigNStartOperation, XFihEccVar, Status, InstancePtr,
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
 * @brief	This function performs ECC pair wise consistency test for ECC core
 *
 * @param	InstancePtr	Pointer to the ECC instance.
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	CurveType	ECC Curve type.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	PrivKeyAddr	Address of the private key buffer, whose length shall be equal to
 * 				CurveLen.
 * @param	PubKeyAddr	Address of the public key buffer, whose length shall be equal to
 * 				double of CurveLen as it contains both Qx, Qy components.
 *
 * @return
 *	-	XASUFW_SUCCESS, if signature provided is valid.
 *	-	XASUFW_RSA_ECC_INVALID_PARAM, if any of the input parameter is invalid.
 *	-	XASUFW_RSA_ECC_PWCT_SIGN_GEN_FAIL, if sign generation fails.
 *	-	XASUFW_RSA_ECC_PWCT_SIGN_VER_FAIL, if sign verification fails.
 *
 *************************************************************************************************/
s32 XEcc_Pwct(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
	u64 PrivKeyAddr, u64 PubKeyAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 Signature[XASU_ECC_P384_SIZE_IN_BYTES + XASU_ECC_P384_SIZE_IN_BYTES];

	if ((DmaPtr == NULL) || (PrivKeyAddr == 0U) || (PubKeyAddr == 0U)) {
		Status = XASUFW_RSA_ECC_INVALID_PARAM;
		goto END;
	}

	Status = XEcc_GenerateSignature(InstancePtr, DmaPtr, CurveType, CurveLen, PrivKeyAddr,
			EKeyPwctEcc, (u64)(UINTPTR)MsgPwctEcc, CurveLen,
			(u64)(UINTPTR)Signature);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateBufStatus(Status, XASUFW_RSA_ECC_PWCT_SIGN_GEN_FAIL);
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcc_VerifySignature(InstancePtr, DmaPtr, CurveType, CurveLen, PubKeyAddr,
			(u64)(UINTPTR)MsgPwctEcc, CurveLen,
			(u64)(UINTPTR)Signature);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateBufStatus(Status, XASUFW_RSA_ECC_PWCT_SIGN_VER_FAIL);
	}

END_CLR:
	Status = XAsufw_UpdateBufStatus(Status, Xil_SecureZeroize((u8 *)(UINTPTR)Signature,
					XAsu_DoubleCurveLength(XASU_ECC_P384_SIZE_IN_BYTES)));

END:
	return Status;
}

#ifdef XASU_ECC_CM_ENABLE
/*************************************************************************************************/
/**
 * @brief	This function generates random numbers of length equal to CurveLen using the TRNG
 * 		and updates the generated values to the corresponding registers,
 * 		repeating the process Count times.
 *
 * @param	InstancePtr	Pointer to the ECC instance.
 * @param	CurveLen	Length of the curve in bytes.
 * @param	Count		Number of random numbers to be updated.
 * 				Count = 2: In case of generate public key operation.
 * 				Count = 4: In case of generate signature operation.
 *
 * @return
 * 	- XASUFW_SUCCESS, if generation and updation is successful.
 * 	- XASUFW_ECC_SCP_RANDOM_NUM_GEN_FAIL, if random number generated is failed.
 * 	- XASUFW_ECC_WRITE_DATA_FAIL, if write data to registers is failed.
 * 	- XASUFW_ECC_SCP_RANDOM_NUM_COUNT_FAIL, if buffer clear is failed.
 *
 *************************************************************************************************/
static s32 XEcc_GenNdUpdateRandNumToReg(XEcc *InstancePtr, u32 CurveLen, u32 Count)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 ScpRandom[XASU_ECC_P384_SIZE_IN_BYTES];
	u32 RegOffset = XECC_MEM_SCP_RAND_1_OFFSET;
	u32 Index;

	/** Perform the following steps Count times: */
	for (Index = 0U; Index < Count; Index++) {
		/** - Generate the random number using TRNG of length equal to CurveLen. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_TrngGetRandomNumbers(ScpRandom, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_ECC_SCP_RANDOM_NUM_GEN_FAIL;
			goto END;
		}
		/** - Clear the Most Significant Bit(MSB). */
		ScpRandom[XECC_SCP_RANDOM_1_MSB_BYTE_OFFSET] &= XECC_SCP_RANDOM_MSB_BIT_SET_MASK;

		/** - Copy the buffer contents to the corresponding registers.  */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = Xil_SMemCpy((u8*)(InstancePtr->BaseAddress + RegOffset), CurveLen, ScpRandom,
					CurveLen, CurveLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_ECC_WRITE_DATA_FAIL;
			goto END;
		}
		RegOffset += XECC_RAND_NUM_REG_OFFSET;
	}

	if (Index != Count) {
		Status = XASUFW_ECC_SCP_RANDOM_NUM_COUNT_FAIL;
	}

END:
	/** Clear the local random buffer. */
	Status = XAsufw_UpdateBufStatus(Status, Xil_SecureZeroize((u8 *)(UINTPTR)ScpRandom,
					XASU_ECC_P384_SIZE_IN_BYTES));

	return Status;
}
#endif

/*************************************************************************************************/
/**
 * @brief	This function will wait for ECC core completion.
 *
 * @param	InstancePtr	Pointer to the ECC instance.
 *
 * @return
 * 		- XASUFW_SUCCESS, if wait for done is successful.
 * 		- XASUFW_FAILURE, if ECC DONE interrupt is not generated within given timeout.
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

	/** Clear and disable DONE interrupt. */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_ISR_OFFSET, XECC_ISR_DONE_MASK);

	/* Disable interrupt */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_IDR_OFFSET, XECC_IDR_DONE_MASK);

END:
	return Status;
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
 * @param	InstancePtr	Pointer to the ECC instance.
 * @param	OpCode		ECC operation code to select the operation.
 * 				0 - Signature verification
 * 				1 - public key validation
 * 				2 - public key generation
 * 				3 - signature generation
 *
 * @return
 * 		- XASUFW_SUCCESS, if operation is successful.
 * 		- XASUFW_ECC_WAIT_FOR_DONE_TIMEOUT, if ECC operation DONE interrupt is not generated
 *        within given timeout.
 * 		- XASUFW_ECC_SCP_DISABLE_FAILED, if SCP disable failed.
 * 		- Also, this function can return termination error codes from 0x21U to 0x2CU from core,
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

	/** Configure ECC curve type, operation and countermeasures. */
	CtrlRegValue = XECC_CTRL_CURVE_MASK & (InstancePtr->CurveInfo->CurveType <<
			XECC_CTRL_CURVE_SHIFT);
	CtrlRegValue |= (XECC_CTRL_OPCODE_MASK & (OpCode << XECC_CTRL_OPCODE_SHIFT)) |
			XECC_CTRL_START_MASK;

#ifdef XASU_ECC_CM_ENABLE
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_CTRL_OFFSET, CtrlRegValue);
#else
	CtrlRegValue |= XECC_SUPPRESS_SCP_SCP2_MASK;
	XAsufw_WriteReg(InstancePtr->BaseAddress + XECC_CTRL_OFFSET, CtrlRegValue);
	if ((XAsufw_ReadReg(InstancePtr->BaseAddress + XECC_STATUS_OFFSET) &
	     XECC_STATUS_SCP_ENABLED_MASK) == XECC_STATUS_SCP_ENABLED_MASK) {
		Status = XASUFW_ECC_SCP_DISABLE_FAILED;
		goto END;
	}
#endif

	/** Wait for ECC operation to complete. */
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
 * @param	InstancePtr	Pointer to the ECC instance.
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
