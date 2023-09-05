/******************************************************************************
* Copyright (c) 2023 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtrngpsx.c
*
* @addtogroup Overview
* @{
* This file contains the required functions to operate TRNG core
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt  01/03/23 Initial release
*       kpt  05/18/23 Fix passing invalid DF length in HRNG mode
* 1.1   kpt  08/29/23 Add volatile keyword to avoid compiler optimization
*       ng   09/04/23 Added SDT support
*
* </pre>
*
* @endcond
******************************************************************************/

/***************************** Include Files *********************************/
#include "xtrngpsx.h"
#include "xtrngpsx_hw.h"
#include "sleep.h"
#include "xil_util.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/
#define XTRNGPSX_RESEED_TIMEOUT			1500000U /**< Reseed timeout in micro-seconds */
#define XTRNGPSX_GENERATE_TIMEOUT		1500000U /**< Generate timeout in micro-seconds */
#define XTRNGPSX_WORD_LEN_IN_BYTES		4U	     /**< Word length in bytes */
#define XTRNGPSX_BYTE_LEN_IN_BITS		8U	     /**< Byte length in bits */
#define XTRNGPSX_BLOCK_LEN_IN_BYTES		16U	     /**< TRNG block length length in bytes */
#define XTRNGPSX_MIN_SEEDLIFE			1U	     /**< Minimum seed life */
#define XTRNGPSX_MAX_SEEDLIFE			0x80000  /**< Maximum seed life 2^19 */
#define XTRNGPSX_SEC_STRENGTH_IN_BURSTS	2U	     /**< Security strength in 128-bit bursts */
#define XTRNGPSX_BURST_SIZE_IN_WORDS	4U	     /**< Burst size in words */
#define XTRNGPSX_DF_MIN_LENGTH			2U	     /**< Minimum DF input length */
#define XTRNGPSX_DF_MAX_LENGTH			0x1FU	 /**< Maximum DF input length */

#define XTRNGPSX_DF_NUM_OF_BYTES_BEFORE_MIN_700CLKS_WAIT	8U
												 /**< Number of bytes to be written before wait */

#define XTRNGPSX_ADAPTPROPTESTCUTOFF_MAX_VAL	0x3FFU /**< maximum adaptprpptest cutoff value */
#define XTRNGPSX_REPCOUNTTESTCUTOFF_MAX_VAL		0x1FFU /**< maximum repcounttest cutoff value */
#define XTRNGPSX_RESET_DELAY_US					10U    /** < Reset delay */
#define XTRNGPSX_DF_700CLKS_WAIT				10U    /** < delay after 4bytes */
#define XTRNGPSX_DF_2CLKS_WAIT					2U     /** < delay after 1byte */
#define XTRNGPSX_STATUS_QCNT_VAL				4U     /** < QCNT value for single burst */

#define XTRNGPSX_TEMPORAL_IMPL 					XSECURE_TEMPORAL_IMPL
#define XTRNGPSX_TEMPORAL_CHECK 				XSECURE_TEMPORAL_CHECK

/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/
static int XTrngpsx_ReseedInternal(XTrngpsx_Instance *InstancePtr, const u8 *Seed, u8 DLen,
	const u8 *PerStr);
static int XTrngpsx_WritePersString(XTrngpsx_Instance *InstancePtr, const u8 *PersString);
static int XTrngpsx_WaitForReseed(XTrngpsx_Instance *InstancePtr);
static int XTrngpsx_CollectRandData(XTrngpsx_Instance *InstancePtr, u8 *RandBuf, u32 RandBufSize);
static int XTrngpsx_WriteSeed(XTrngpsx_Instance *InstancePtr, const u8 *Seed, u8 DLen);
static inline int XTrngpsx_WaitForEvent(UINTPTR Addr, u32 EventMask, u32 Event,
		u32 Timeout);
static inline void XTrngpsx_WriteReg(UINTPTR Address, u32 RegValue);
static inline u32 XTrngpsx_ReadReg(UINTPTR RegAddress);
static inline int XTrngpsx_UtilRMW32(UINTPTR RegAddress, u32 Mask, u32 Value);
static int XTrngpsx_Set(XTrngpsx_Instance *InstancePtr);
static int XTrngpsx_Reset(XTrngpsx_Instance *InstancePtr);
static int XTrngpsx_PrngReset(XTrngpsx_Instance *InstancePtr);
static int XTrngpsx_PrngSet(XTrngpsx_Instance *InstancePtr);
static int XTrngpsx_CfgDfLen(XTrngpsx_Instance *InstancePtr, u8 DfLen);
static int XTrngpsx_CfgAdaptPropTestCutoff(XTrngpsx_Instance *InstancePtr, u16 AdaptPropTestCutoff);
static int XTrngpsx_CfgRepCountTestCutoff(XTrngpsx_Instance *InstancePtr, u16 RepCountTestCutoff);
static int XTrngpsx_CfgDIT(XTrngpsx_Instance *InstancePtr, u8 DITValue);

/**************************************************************************************************/
/**
 * @brief
 * Write to the register.
 *
 * @param	Address is the address of the register to be written.
 * @param	RegValue is the value to be written to the register.
 *
 **************************************************************************************************/
static inline void XTrngpsx_WriteReg(UINTPTR Address, u32 RegValue)
{
	Xil_Out32(Address, RegValue);
}

/*************************************************************************************************/
/**
 * @brief
 * Read from the register.
 *
 * @param	RegAddress Address of the register.
 *
 * @return	The value read from the register.
 *
 **************************************************************************************************/
static inline u32 XTrngpsx_ReadReg(UINTPTR RegAddress)
{
	return Xil_In32(RegAddress);
}

/*************************************************************************************************/
/**
 * @brief
 * This function reads, modifies and writes in to the register
 *
 * @param	RegAddress Address of the register.
 * @param	Mask Indicates the bits to be modified
 * @param	Value Contains 32 bit Value to be written at the specified address
 *
 * @return
 * 		-XST_SUCCESS On successful write
 * 		-XST_FAILURE On failure
 *
 **************************************************************************************************/
static inline int XTrngpsx_UtilRMW32(UINTPTR RegAddress, u32 Mask, u32 Value) {
	int Status = XST_FAILURE;

	Status = Xil_SecureRMW32(RegAddress, Mask, Value);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function brings TRNG core and prng unit out of reset
 *
 * @param	InstancePtr Pointer to XTrngpsx_Instance.
 *
 * @return
 * 		-XST_SUCCESS On success
 * 		-XST_FAILURE On failure
 *
 **************************************************************************************************/
static int XTrngpsx_Set(XTrngpsx_Instance *InstancePtr) {
	int Status = XST_FAILURE;

	Status = XTrngpsx_Reset(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	usleep(XTRNGPSX_RESET_DELAY_US);
	Status = XTrngpsx_UtilRMW32((InstancePtr->Config.BaseAddress + TRNG_RESET), TRNG_RESET_VAL_MASK, 0U);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/* Soft reset PRNG unit */
	Status = XTrngpsx_PrngSet(InstancePtr);
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function sets TRNG core in to reset state
 *
 * @param	InstancePtr Pointer to XTrngpsx_Instance.
 *
 * @return
 * 		-XST_SUCCESS On success
 * 		-XST_FAILURE On failure
 *
 **************************************************************************************************/
static int XTrngpsx_Reset(XTrngpsx_Instance *InstancePtr) {
	int Status = XST_FAILURE;

	Status = XTrngpsx_UtilRMW32((InstancePtr->Config.BaseAddress + TRNG_RESET), TRNG_RESET_VAL_MASK,
		TRNG_RESET_DEFVAL);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function brings PRNG unit out of reset
 *
 * @param	InstancePtr Pointer to XTrngpsx_Instance.
 *
 * @return
 * 		-XST_SUCCESS On success
 * 		-XST_FAILURE On failure
 *
 **************************************************************************************************/
static int XTrngpsx_PrngSet(XTrngpsx_Instance *InstancePtr) {
	int Status = XST_FAILURE;

	Status = XTrngpsx_PrngReset(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	usleep(XTRNGPSX_RESET_DELAY_US);
	Status = XTrngpsx_UtilRMW32((InstancePtr->Config.BaseAddress + TRNG_RESET), TRNG_CTRL_PRNGSRST_MASK, 0U);
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function sets PRNG unit in to reset state
 *
 * @param	InstancePtr Pointer to XTrngpsx_Instance.
 *
 * @return
 * 		-XST_SUCCESS On success
 * 		-XST_FAILURE On failure
 *
 **************************************************************************************************/
static int XTrngpsx_PrngReset(XTrngpsx_Instance *InstancePtr) {
	int Status = XST_FAILURE;

	Status = XTrngpsx_UtilRMW32((InstancePtr->Config.BaseAddress + TRNG_RESET), TRNG_CTRL_PRNGSRST_MASK,
		TRNG_CTRL_PRNGSRST_MASK);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function configures the DF input length.
 *
 * @param	InstancePtr Pointer to XTrngpsx_Instance.
 * @param	DfLen input DF length.
 *
 * @return
 * 		-XST_SUCCESS On successful write
 * 		-XST_FAILURE On failure
 *
 **************************************************************************************************/
static int XTrngpsx_CfgDfLen(XTrngpsx_Instance *InstancePtr, u8 DfLen) {
	int Status = XST_FAILURE;

	Status = XTrngpsx_UtilRMW32((InstancePtr->Config.BaseAddress + TRNG_CTRL_3), TRNG_CTRL_3_DLEN_MASK,
		(DfLen << TRNG_CTRL_3_DLEN_SHIFT));

	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function writes the cutoff value in to the register.
 *
 * @param	InstancePtr Pointer to XTrngpsx_Instance.
 * @param	AdaptPropTestCutoff cutoff value for adaptive count test.
 *
 * @return
 * 		-XST_SUCCESS On successful write
 * 		-XST_FAILURE On failure
 *
 **************************************************************************************************/
static int XTrngpsx_CfgAdaptPropTestCutoff(XTrngpsx_Instance *InstancePtr, u16 AdaptPropTestCutoff) {
	int Status = XST_FAILURE;

	Status = XTrngpsx_UtilRMW32((InstancePtr->Config.BaseAddress + TRNG_CTRL_3), TRNG_CTRL_3_ADAPTPROPTESTCUTOFF_MASK,
		(AdaptPropTestCutoff << TRNG_CTRL_3_ADAPTPROPTESTCUTOFF_SHIFT));

	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function writes the cutoff value in to the register.
 *
 * @param	InstancePtr Pointer to XTrngpsx_Instance.
 * @param	RepCountTestCutoff cutoff value for repetitive count test.
 *
 * @return
 * 		-XST_SUCCESS On successful write
 * 		-XST_FAILURE On failure
 *
 **************************************************************************************************/
static int XTrngpsx_CfgRepCountTestCutoff(XTrngpsx_Instance *InstancePtr, u16 RepCountTestCutoff) {
	int Status = XST_FAILURE;

	Status = XTrngpsx_UtilRMW32((InstancePtr->Config.BaseAddress + TRNG_CTRL_2), TRNG_CTRL_2_REPCOUNTTESTCUTOFF_MASK,
		(RepCountTestCutoff << TRNG_CTRL_2_REPCOUNTTESTCUTOFF_SHIFT));

	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function writes the DIT value in to the register.
 *
 * @param	InstancePtr Pointer to XTrngpsx_Instance.
 * @param   DITValue    Digitization interval time.
 *
 * @return
 * 		-XST_SUCCESS On successful write
 * 		-XST_FAILURE On failure
 *
 **************************************************************************************************/
static int XTrngpsx_CfgDIT(XTrngpsx_Instance *InstancePtr, u8 DITValue) {
	int Status = XST_FAILURE;

	Status = XTrngpsx_UtilRMW32((InstancePtr->Config.BaseAddress + TRNG_CTRL_2), TRNG_CTRL_2_DIT_MASK,
		(DITValue << TRNG_CTRL_2_DIT_SHIFT));

	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function initializes the TRNGPSV instance/driver. This function must be called prior to
 * using the driver. Initialization includes only setting up the device id and base address.
 * All the remaining parameters of the Instance will be initialized in Instantiate function.
 *
 * @param	InstancePtr is a pointer to the XTrngpsx instance.
 * @param	CfgPtr points to the configuration structure associated with the TRNGPSX driver.
 * @param	EffectiveAddr is the base address of the device. If address translation is being
 * 		used, then this parameter must reflect the virtual base address. Otherwise, the
 * 		physical address should be used.
 *
 * @return
 *		- XST_SUCCESS if initialization was successful.
 *		- XTRNGPSX_INVALID_PARAM if invalid parameter passed to this function.
 *
 **************************************************************************************************/
int XTrngpsx_CfgInitialize(XTrngpsx_Instance *InstancePtr, const XTrngpsx_Config *CfgPtr,
		UINTPTR EffectiveAddr)
{
	volatile int Status = XST_FAILURE;

	/* Validate arguments. */
	if (InstancePtr == NULL) {
		Status = XTRNGPSX_INVALID_PARAM;
		goto END;
	}

	if ((CfgPtr == NULL) || (EffectiveAddr == (UINTPTR)0U)) {
		Status = XTRNGPSX_INVALID_PARAM;
		goto SET_ERR;
	}

	/* Populate Config parameters */
	#ifndef SDT
	InstancePtr->Config.DeviceId = CfgPtr->DeviceId;
	#else
	InstancePtr->Config.Name = CfgPtr->Name;
	#endif
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	InstancePtr->State = XTRNGPSX_UNINITIALIZED_STATE;
	Status = XST_SUCCESS;

SET_ERR:
	if (Status != XST_SUCCESS) {
		InstancePtr->ErrorState = XTRNGPSX_ERROR;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function instantiates the TRNG instance with user configure values
 *
 * @param	InstancePtr Pointer to the XTrngpsx_Instance.
 * @param	Seed Pointer to the seed input
 * @param	SeedLength Seed length in bytes
 * @param	PersStr Pointer to the personalization string input
 * @param	UserCfg Pointer to the XTrngpsx_UserConfig
 *
 * @return
 * 		- XST_SUCCESS On successful instantation
 * 		- XTRNGPSX_INVALID_PARAM If invalid parameter(s) passed to this function.
 * 		- XTRNGPSX_INVALID_SEED_VALUE If provide seed is NULL in DRBG mode
 * 		- XTRNGPSX_INVALID_STATE If state is not in uninstantiate state
 * 		- XTRNGPSX_UNHEALTHY_STATE If TRNG KAT fails
 * 		- XTRNGPSX_INVALID_MODE  If invalid mode is passed to this function
 * 		- XTRNGPSX_INVALID_DF_LENGTH If invalid DF input length is passed as a function
 * 		- XTRNGPSX_INVALID_SEED_LENGTH If provide seed length doesn't match with given DF length
 * 		- XTRNGPSX_INVALID_SEED_LIFE If invalid seed life is provided
 * 		- XTRNGPSX_INVALID_ADAPTPROPTEST_CUTOFF_VALUE If invalid cutoff value is provided
 * 		- XTRNGPSX_INVALID_REPCOUNTTEST_CUTOFF_VALUE If invalid repetitive test cutoff value is provided
 * 		- XTRNGPSX_USER_CFG_COPY_ERROR If error occurred during copy of XTrngpsx_UserConfig structure
 * 		- XTRNGPSX_TIMEOUT_ERROR If timeout occurred waiting for done bit
 * 		- XTRNGPSX_CATASTROPHIC_CTF_ERROR If CTF bit asserted in STATUS register.
 * 		- XTRNGPSX_ERROR_WRITE On write failure
 * 		- XST_FAILURE On unexpected failure
 *
 **************************************************************************************************/
int XTrngpsx_Instantiate(XTrngpsx_Instance *InstancePtr, const u8 *Seed, u32 SeedLength, const u8 *PersStr,
			const XTrngpsx_UserConfig *UserCfg) {
	volatile int Status = XST_FAILURE;

	if ((UserCfg == NULL) || (InstancePtr == NULL)) {
		Status = XTRNGPSX_INVALID_PARAM;
		goto END;
	}

	if ((Seed == NULL) && (UserCfg->Mode == XTRNGPSX_DRNG_MODE)) {
		Status = XTRNGPSX_INVALID_SEED_VALUE;
		goto END;
	}

	if ((Seed != NULL) && (UserCfg->Mode == XTRNGPSX_HRNG_MODE)) {
		Status = XTRNGPSX_INVALID_SEED_VALUE;
		goto END;
	}

	if (InstancePtr->State != XTRNGPSX_UNINITIALIZED_STATE) {
		Status = XTRNGPSX_INVALID_STATE;
		goto END;
	}

	if ((UserCfg->Mode != XTRNGPSX_DRNG_MODE) && (UserCfg->Mode != XTRNGPSX_PTRNG_MODE) &&
		(UserCfg->Mode != XTRNGPSX_HRNG_MODE)) {
		Status = XTRNGPSX_INVALID_MODE;
		goto END;
	}

	if ((UserCfg->DFLength  < XTRNGPSX_DF_MIN_LENGTH) ||
		(UserCfg->DFLength > XTRNGPSX_DF_MAX_LENGTH)) {
		Status = XTRNGPSX_INVALID_DF_LENGTH;
		goto END;
	}

	if ((UserCfg->Mode == XTRNGPSX_DRNG_MODE) &&
		(SeedLength != ((UserCfg->DFLength + 1U) * XTRNGPSX_BLOCK_LEN_IN_BYTES))) {
		Status = XTRNGPSX_INVALID_SEED_LENGTH;
		goto END;
	}

	if ((UserCfg->SeedLife < XTRNGPSX_MIN_SEEDLIFE) ||
		(UserCfg->SeedLife > XTRNGPSX_MAX_SEEDLIFE)) {
		Status = XTRNGPSX_INVALID_SEED_LIFE;
		goto END;
	}

	if ((UserCfg->Mode != XTRNGPSX_DRNG_MODE) && ((UserCfg->AdaptPropTestCutoff < 1U) ||
		(UserCfg->AdaptPropTestCutoff > XTRNGPSX_ADAPTPROPTESTCUTOFF_MAX_VAL))) {
		Status = XTRNGPSX_INVALID_ADAPTPROPTEST_CUTOFF_VALUE;
		goto END;
	}

	if ((UserCfg->Mode != XTRNGPSX_DRNG_MODE) && ((UserCfg->RepCountTestCutoff < 1U) ||
		(UserCfg->RepCountTestCutoff > XTRNGPSX_REPCOUNTTESTCUTOFF_MAX_VAL))) {
		Status = XTRNGPSX_INVALID_REPCOUNTTEST_CUTOFF_VALUE;
		goto END;
	}

	Status = Xil_SMemCpy(&InstancePtr->UserCfg, sizeof(XTrngpsx_UserConfig), UserCfg,
			sizeof(XTrngpsx_UserConfig), sizeof(XTrngpsx_UserConfig));
	if (Status != XST_SUCCESS) {
		Status = XTRNGPSX_USER_CFG_COPY_ERROR;
		goto END;
	}

	/* Bring TRNG and PRNG unit core out of reset */
	Status = XTrngpsx_Set(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((UserCfg->Mode == XTRNGPSX_PTRNG_MODE) ||
		(UserCfg->Mode == XTRNGPSX_HRNG_MODE)) {
		/* Configure cutoff values */
		Status = XTrngpsx_CfgAdaptPropTestCutoff(InstancePtr, UserCfg->AdaptPropTestCutoff);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XTrngpsx_CfgRepCountTestCutoff(InstancePtr, UserCfg->RepCountTestCutoff);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/* Configure default DIT value */
		Status = XTrngpsx_CfgDIT(InstancePtr, TRNG_CTRL_2_DIT_DEFVAL);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	InstancePtr->State = XTRNGPSX_INSTANTIATE_STATE;

	/* Do reseed operation when mode is DRNG/HRNG */
	if ((UserCfg->Mode == XTRNGPSX_DRNG_MODE) ||
		(UserCfg->Mode == XTRNGPSX_HRNG_MODE)) {
		Status = XST_FAILURE;
		Status = XTrngpsx_ReseedInternal(InstancePtr, Seed, InstancePtr->UserCfg.DFLength,
				PersStr);
		if ((Status != XST_SUCCESS) || (InstancePtr->State != XTRNGPSX_RESEED_STATE)) {
			goto END;
		}
	}

	Status = XST_SUCCESS;
	InstancePtr->ErrorState = XTRNGPSX_HEALTHY;

END:
	if (InstancePtr != NULL) {
		if ((Status != XST_SUCCESS) &&
		(InstancePtr->ErrorState != XTRNGPSX_CATASTROPHIC)) {
			InstancePtr->ErrorState = XTRNGPSX_ERROR;
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function triggers and reseeds the DRBG module
 *
 * @param	InstancePtr Pointer to XTrngpsx_Instance.
 * @param	Seed Pointer to the seed input
 * @param	DLen Seed length in TRNG block size
 *
 * @return
 * 		- XST_SUCCESS On successful reseed
 * 		- XTRNGPSX_INVALID_PARAM If invalid parameter(s) passed to this function.
 * 		- XTRNGPSX_INVALID_SEED_VALUE If provide seed is NULL in DRBG/HRNG mode
 * 		- XTRNGPSX_INVALID_MODE  If invalid mode is passed to this function
 * 		- XTRNGPSX_INVALID_DF_LENGTH If invalid DF input length is passed as a function
 * 		- XTRNGPSX_INVALID_STATE If state is not sequenced correctly
 * 		- XTRNGPSX_UNHEALTHY_STATE If TRNG is in failure state, needs an uninstantiation
 * 			or KAT should be run if the error is catastrophic
 *		- XTRNGPSX_TIMEOUT_ERROR If timeout occurred waiting for done bit
 *		- XTRNGPSX_CATASTROPHIC_CTF_ERROR If CTF bit asserted in STATUS register.
 *		- XTRNGPSX_ERROR_WRITE On write failure
 * 		- XST_FAILURE On unexpected failure
 *
 **************************************************************************************************/
int XTrngpsx_Reseed(XTrngpsx_Instance *InstancePtr, const u8 *Seed, u8 DLen) {
	volatile int Status = XST_FAILURE;

	if (InstancePtr == NULL) {
		Status = XTRNGPSX_INVALID_PARAM;
		goto END;
	}

	if ((InstancePtr->UserCfg.Mode == XTRNGPSX_DRNG_MODE) && (Seed == NULL)) {
		Status = XTRNGPSX_INVALID_SEED_VALUE;
		goto END;
	}

	if ((Seed != NULL) && (InstancePtr->UserCfg.Mode == XTRNGPSX_HRNG_MODE)) {
		Status = XTRNGPSX_INVALID_SEED_VALUE;
		goto END;
	}

	if ((DLen < XTRNGPSX_DF_MIN_LENGTH) || (DLen > XTRNGPSX_DF_MAX_LENGTH)) {
		Status = XTRNGPSX_INVALID_DF_LENGTH;
		goto END;
	}

	if (InstancePtr->UserCfg.Mode == XTRNGPSX_PTRNG_MODE) {
		Status = XTRNGPSX_INVALID_MODE;
		goto END;
	}

	if (InstancePtr->State == XTRNGPSX_UNINITIALIZED_STATE) {
		Status = XTRNGPSX_INVALID_STATE;
		goto END;
	}

	if ((InstancePtr->ErrorState != XTRNGPSX_HEALTHY) &&
		(InstancePtr->ErrorState != XTRNGPSX_STARTUP_TEST)) {
		Status = XTRNGPSX_UNHEALTHY_STATE;
		goto END;
	}

	XTRNGPSX_TEMPORAL_CHECK(END, Status, XTrngpsx_ReseedInternal, InstancePtr,
		Seed, DLen, NULL);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function generates and collects random data in to a buffer.
 *
 * @param	InstancePtr Pointer to XTrngpsx_Instance
 * @param	RandBuf Pointer to buffer in which random data is stored.
 * @param	RandBufSize Size of the buffer in which random data is stored.
 * @param	PredResistance is the flag that controls Generate level Prediction Resistance.
 * 			When enabled, it mandates fresh seed for every Generate operation.
 *
 * @return
 * 		- XST_SUCCESS On successful generate of random number
 * 		- XTRNGPSX_INVALID_PARAM If invalid parameter(s) passed to this function.
 * 		- XTRNGPSX_INVALID_STATE If state is not sequenced correctly
 * 		- XTRNGPSX_INVALID_MODE  If invalid mode is passed to this function
 * 		- XTRNGPSX_INVALID_BUF_SIZE If buffer is less that 256 bytes or NULL
 *      - XTRNG_PSX_INVALID_PREDRES_VALUE If invalid predication resistance value is passed to this function
 * 		- XTRNGPSX_UNHEALTHY_STATE If TRNG is in failure state, needs an uninstantiation
 * 			or KAT should be run if error is catastrophic
 * 		- XTRNGPSX_RESEED_REQUIRED_ERROR If elapsed seed life exceeds the requested seed life
 * 			in DRBG mode
 *		- XTRNGPSX_TIMEOUT_ERROR If timeout occurred waiting for QCNT to become 4.
 *		- XTRNGPSX_CATASTROPHIC_DTF_ERROR If DTF bit asserted in STATUS register.
 *		- XTRNGPSX_ERROR_WRITE On write failure
 * 		- XST_FAILURE On unexpected failure
 *
 **************************************************************************************************/
int XTrngpsx_Generate(XTrngpsx_Instance *InstancePtr, u8 *RandBuf, u32 RandBufSize, u8 PredResistance) {
	volatile int Status = XST_FAILURE;

	if ((InstancePtr == NULL) || (RandBuf == NULL)) {
		Status = XTRNGPSX_INVALID_PARAM;
		goto END;
	}

	if ((InstancePtr->UserCfg.Mode == XTRNGPSX_PTRNG_MODE) &&
		(InstancePtr->State != XTRNGPSX_INSTANTIATE_STATE) &&
		(InstancePtr->State != XTRNGPSX_GENERATE_STATE)) {
		Status = XTRNGPSX_INVALID_STATE;
		goto END;
	}

	if ((InstancePtr->UserCfg.Mode != XTRNGPSX_PTRNG_MODE) &&
		(InstancePtr->State != XTRNGPSX_RESEED_STATE) &&
		(InstancePtr->State != XTRNGPSX_GENERATE_STATE)) {
		Status = XTRNGPSX_INVALID_STATE;
		goto END;
	}

	if ((RandBufSize == 0U) || (RandBufSize > XTRNGPSX_SEC_STRENGTH_IN_BYTES) ||
		((RandBufSize % XTRNGPSX_WORD_LEN_IN_BYTES) != 0U)) {
		Status = XTRNGPSX_INVALID_BUF_SIZE;
		goto END;
	}

	if ((PredResistance != TRUE) && (PredResistance != FALSE)) {
		Status = XTRNG_PSX_INVALID_PREDRES_VALUE;
	}

	if ((InstancePtr->UserCfg.Mode != XTRNGPSX_PTRNG_MODE) && (PredResistance == TRUE)) {
		Status = XTRNG_PSX_INVALID_PREDRES_VALUE;
	}

	if ((InstancePtr->ErrorState != XTRNGPSX_HEALTHY) &&
		(InstancePtr->ErrorState != XTRNGPSX_STARTUP_TEST)) {
		Status = XTRNGPSX_UNHEALTHY_STATE;
		goto END;
	}

	if ((InstancePtr->UserCfg.Mode == XTRNGPSX_DRNG_MODE) ||
		(InstancePtr->UserCfg.Mode == XTRNGPSX_HRNG_MODE)) {
		if (InstancePtr->UserCfg.Mode == XTRNGPSX_HRNG_MODE) {
			/* Auto reseed in HRNG mode */
			if ((InstancePtr->Stats.ElapsedSeedLife >= InstancePtr->UserCfg.SeedLife) ||
				(PredResistance == TRUE)) {
				XTRNGPSX_TEMPORAL_CHECK(END, Status, XTrngpsx_Reseed, InstancePtr,
						NULL, InstancePtr->UserCfg.DFLength);
			}
		}
		else {
			if ((PredResistance == TRUE) &&
				(InstancePtr->Stats.ElapsedSeedLife > 0U)) {
				Status = XTRNGPSX_RESEED_REQUIRED_ERROR;
				goto END;
			}
		}
		InstancePtr->UserCfg.PredResistance = PredResistance;
	}
	else if (InstancePtr->UserCfg.Mode == XTRNGPSX_PTRNG_MODE) {
		/* Enable ring oscillators for random seed source */
		XTRNGPSX_TEMPORAL_CHECK(END, Status, Xil_SecureRMW32, (InstancePtr->Config.BaseAddress + TRNG_OSC_EN),
			TRNG_OSC_EN_VAL_MASK, TRNG_OSC_EN_VAL_MASK);

		XTRNGPSX_TEMPORAL_CHECK(END, Status, Xil_SecureRMW32, (InstancePtr->Config.BaseAddress + TRNG_CTRL),
			TRNG_CTRL_TRSSEN_MASK | TRNG_CTRL_EUMODE_MASK |
			TRNG_CTRL_PRNGXS_MASK, TRNG_CTRL_TRSSEN_MASK |
			TRNG_CTRL_EUMODE_MASK);
	}
	else {
		Status = XTRNGPSX_INVALID_MODE;
		goto END;
	}

	/* Trigger generate and collect random data */
	XTRNGPSX_TEMPORAL_CHECK(END, Status, XTrngpsx_CollectRandData, InstancePtr, RandBuf,
			RandBufSize);

	/* For every generate elapsed sseed life should be incremented */
	InstancePtr->Stats.ElapsedSeedLife++;
	InstancePtr->State = XTRNGPSX_GENERATE_STATE;
	Status = XST_SUCCESS;

END:
	if (InstancePtr != NULL) {
		if ((Status != XST_SUCCESS) &&
		(InstancePtr->ErrorState != XTRNGPSX_CATASTROPHIC)) {
			InstancePtr->ErrorState = XTRNGPSX_ERROR;
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function uninstantiates the TRNG instance.
 *
 * @param	InstancePtr Pointer to XTrngpsx_Instance.
 *
 * @return
 * 		- XST_SUCCESS if uninstantiation was successful.
 * 		- XTRNGPSX_INVALID_PARAM if invalid instance is passed to function
 *		- XTRNGPSX_ERROR_MEMSET_UNINSTANTIATE_ERROR if memset was not successful
 * 		- XST_FAILURE On unexpected failure
 *
 **************************************************************************************************/
int XTrngpsx_Uninstantiate(XTrngpsx_Instance *InstancePtr) {
	int Status = XST_FAILURE;
	XTrngpsx_ErrorState ErrorState;

	if (InstancePtr == NULL) {
		Status = XTRNGPSX_INVALID_PARAM;
		goto END;
	}

	/* Bring cores in to reset state */
	Status = XTrngpsx_Reset(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XTrngpsx_PrngReset(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Disable ring oscillators as a random seed source */
	Status = Xil_SecureRMW32((InstancePtr->Config.BaseAddress + TRNG_OSC_EN), TRNG_OSC_EN_VAL_MASK,
			TRNG_OSC_EN_VAL_DEFVAL);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	ErrorState = InstancePtr->ErrorState;
	InstancePtr->State = XTRNGPSX_UNINITIALIZED_STATE;

	/* Retain the error state */
	InstancePtr->ErrorState = ErrorState;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function triggers and reseeds the DRBG module
 *
 * @param	InstancePtr Pointer to XTrngpsx_Instance.
 * @param	Seed Pointer to the seed input
 * @param	DLen Seed length in multiples of TRNG block size
 * @param	PerStr Pointer to the personalization string
 *
 * @return
 * 		- XST_SUCCESS On successful reseed
 * 		- XTRNGPSX_ERROR_WRITE On write failure
 *		- XTRNGPSX_TIMEOUT_ERROR If timeout occurred waiting for done bit
 *		- XTRNGPSX_CATASTROPHIC_CTF_ERROR If CTF bit asserted in STATUS register.
 *
 **************************************************************************************************/
static int XTrngpsx_ReseedInternal(XTrngpsx_Instance *InstancePtr, const u8 *Seed, u8 DLen,
		const u8 *PerStr) {
	volatile int Status = XST_FAILURE;
	u32 PersMask = TRNG_CTRL_PERSODISABLE_MASK;

	/* Configure DF Len */
	Status = XTrngpsx_CfgDfLen(InstancePtr, DLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (PerStr != NULL) {
		Status = XST_FAILURE;
		Status = XTrngpsx_WritePersString(InstancePtr, PerStr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		PersMask = TRNG_CTRL_PERSODISABLE_DEFVAL;
	}

	XTRNGPSX_TEMPORAL_CHECK(END, Status, Xil_SecureRMW32, (InstancePtr->Config.BaseAddress + TRNG_CTRL),
		TRNG_CTRL_PERSODISABLE_MASK | TRNG_CTRL_PRNGSTART_MASK, PersMask);

	/* DRNG Mode */
	if (Seed != NULL) {
		/* Enable TST mode and set PRNG mode for reseed operation*/
		XTRNGPSX_TEMPORAL_CHECK(END, Status, Xil_SecureRMW32, (InstancePtr->Config.BaseAddress + TRNG_CTRL),
			TRNG_CTRL_PRNGMODE_MASK | TRNG_CTRL_TSTMODE_MASK |
			TRNG_CTRL_TRSSEN_MASK, TRNG_CTRL_TSTMODE_MASK |
			TRNG_CTRL_TRSSEN_MASK);

		/* Start reseed operation */
		XTRNGPSX_TEMPORAL_CHECK(END, Status, Xil_SecureRMW32, (InstancePtr->Config.BaseAddress + TRNG_CTRL),
			TRNG_CTRL_PRNGSTART_MASK, TRNG_CTRL_PRNGSTART_MASK);

		/* For writing seed as an input to DF, PRNG start needs to be set */
		XTRNGPSX_TEMPORAL_CHECK(END,Status, XTrngpsx_WriteSeed, InstancePtr, Seed,
			DLen);
	}
	else { /* HTRNG Mode */
		/* Enable ring oscillators for random seed source */
		XTRNGPSX_TEMPORAL_CHECK(END, Status, Xil_SecureRMW32, (InstancePtr->Config.BaseAddress + TRNG_OSC_EN),
			TRNG_OSC_EN_VAL_MASK, TRNG_OSC_EN_VAL_MASK);

		/* Enable TRSSEN and set PRNG mode for reseed operation */
		XTRNGPSX_TEMPORAL_CHECK(END, Status, Xil_SecureRMW32, (InstancePtr->Config.BaseAddress + TRNG_CTRL),
			TRNG_CTRL_PRNGMODE_MASK | TRNG_CTRL_TRSSEN_MASK |
			TRNG_CTRL_PRNGXS_MASK, TRNG_CTRL_TRSSEN_MASK);

		/* Start reseed operation */
		XTRNGPSX_TEMPORAL_CHECK(END, Status, Xil_SecureRMW32, (InstancePtr->Config.BaseAddress + TRNG_CTRL),
			TRNG_CTRL_PRNGSTART_MASK, TRNG_CTRL_PRNGSTART_MASK);
	}

	/* Wait for reseed operation and check CTF flag */
	XTRNGPSX_TEMPORAL_CHECK(END, Status, XTrngpsx_WaitForReseed, InstancePtr);

	InstancePtr->State = XTRNGPSX_RESEED_STATE;
	Status = XST_SUCCESS;
	InstancePtr->Stats.ElapsedSeedLife = 0U;

END:
	if ((Status != XST_SUCCESS) && (InstancePtr->ErrorState != XTRNGPSX_CATASTROPHIC)) {
		InstancePtr->ErrorState = XTRNGPSX_ERROR;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function waits for the reseed done bit.
 *
 * @param	InstancePtr Pointer to XTrngpsx_Instance.
 *
 * @return
 *		- XST_SUCCESS if Random number collection was successful.
 *		- XTRNGPSX_TIMEOUT_ERROR if timeout occurred waiting for done bit
 *		- XTRNGPSX_CATASTROPHIC_CTF_ERROR if CTF bit asserted in STATUS register.
 *
 **************************************************************************************************/
static int XTrngpsx_WaitForReseed(XTrngpsx_Instance *InstancePtr) {
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;

	XTRNGPSX_TEMPORAL_IMPL(Status, StatusTmp, XTrngpsx_WaitForEvent,
		   (UINTPTR)(InstancePtr->Config.BaseAddress + TRNG_STATUS),
			TRNG_STATUS_DONE_MASK, TRNG_STATUS_DONE_MASK,
			XTRNGPSX_RESEED_TIMEOUT);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XTRNGPSX_TIMEOUT_ERROR;
		goto END;
	}
	if ((Xil_In32(InstancePtr->Config.BaseAddress + TRNG_STATUS) & TRNG_STATUS_CERTF_MASK) ==
			TRNG_STATUS_CERTF_MASK) {
		InstancePtr->ErrorState = XTRNGPSX_CATASTROPHIC;
		Status = XTRNGPSX_CATASTROPHIC_CTF_ERROR;
		goto END;
	}

	Status = XTrngpsx_UtilRMW32((InstancePtr->Config.BaseAddress + TRNG_CTRL), TRNG_CTRL_PRNGSTART_MASK |
			TRNG_CTRL_TRSSEN_MASK, 0U);
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function triggers Generate operation and collects random data in buffer.
 *
 * @param	InstancePtr Pointer to XTrngpsx_Instance.
 * @param	RandBuf Pointer to the buffer in which random data is stored.
 * @param	RandBufSize Buffer size in bytes.
 *
 * @return
 *		- XST_SUCCESS if Random number collection was successful.
 *		- XTRNGPSX_TIMEOUT_ERROR if timeout occurred waiting for QCNT to become 4.
 *		- XTRNGPSX_CATASTROPHIC_DTF_ERROR if DTF bit asserted in STATUS register.
 *
 **************************************************************************************************/
static int XTrngpsx_CollectRandData(XTrngpsx_Instance *InstancePtr, u8 *RandBuf, u32 RandBufSize) {
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	u8 Idx = 0U;
	volatile u8 NumofBursts = 0U;
	u8 BurstIdx = 0U;
	u32 RegVal = 0U;
	u32 Size = RandBufSize / XTRNGPSX_WORD_LEN_IN_BYTES;
	u32 SingleGenModeVal = 0U;

	if (InstancePtr->UserCfg.PredResistance == TRUE) {
		SingleGenModeVal = TRNG_CTRL_SINGLEGENMODE_MASK;
	}

	/* Set PRNG mode to generate */
	XTRNGPSX_TEMPORAL_IMPL(Status, StatusTmp, Xil_SecureRMW32, (InstancePtr->Config.BaseAddress + TRNG_CTRL),
		TRNG_CTRL_PRNGMODE_MASK | TRNG_CTRL_SINGLEGENMODE_MASK | TRNG_CTRL_PRNGSTART_MASK,
		TRNG_CTRL_PRNGMODE_MASK | SingleGenModeVal);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		goto END;
	}

	XTRNGPSX_TEMPORAL_IMPL(Status, StatusTmp, Xil_SecureRMW32, (InstancePtr->Config.BaseAddress + TRNG_CTRL),
		TRNG_CTRL_PRNGSTART_MASK, TRNG_CTRL_PRNGSTART_MASK);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		goto END;
	}

	for (NumofBursts = 0; NumofBursts < XTRNGPSX_SEC_STRENGTH_IN_BURSTS; NumofBursts++) {
		XTRNGPSX_TEMPORAL_IMPL(Status, StatusTmp, XTrngpsx_WaitForEvent, (UINTPTR)(InstancePtr->Config.BaseAddress + TRNG_STATUS),
			TRNG_STATUS_QCNT_MASK,
			(XTRNGPSX_STATUS_QCNT_VAL << TRNG_STATUS_QCNT_SHIFT),
			XTRNGPSX_GENERATE_TIMEOUT);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = XTRNGPSX_TIMEOUT_ERROR;
			goto END;
		}
		if ((Xil_In32(InstancePtr->Config.BaseAddress + TRNG_STATUS) & TRNG_STATUS_DTF_MASK) ==
			TRNG_STATUS_DTF_MASK) {
			InstancePtr->ErrorState = XTRNGPSX_CATASTROPHIC;
			Status = XTRNGPSX_CATASTROPHIC_DTF_ERROR;
			goto END;
		}
		BurstIdx = NumofBursts * XTRNGPSX_BURST_SIZE_IN_WORDS;
		for (Idx = 0U; Idx < XTRNGPSX_BURST_SIZE_IN_WORDS; Idx++) {
			RegVal = XTrngpsx_ReadReg((InstancePtr->Config.BaseAddress + TRNG_CORE_OUTPUT));
			if ((Idx + BurstIdx) < Size) {
				*((u32*)RandBuf + Idx + BurstIdx) =
						Xil_EndianSwap32(RegVal);
			}
		}
	}
	if (NumofBursts == XTRNGPSX_SEC_STRENGTH_IN_BURSTS) {
		Status = XST_SUCCESS;
	}

END:
	if (Status != XST_SUCCESS) {
		SStatus = Xil_SMemSet(RandBuf, RandBufSize, 0U, RandBufSize);
		if (SStatus != XST_SUCCESS) {
			Status |= SStatus;
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function writes seed to the derivative function.
 *
 * @param	InstancePtr Pointer to XTrngpsx_Instance.
 * @param	Seed Pointer to the seed input.
 * @param	DLen Seed length in multiples of TRNG block size
 *
 * @return
 * 		-XST_SUCCESS On successful write
 * 		-XST_FAILURE On failure
 *
 **************************************************************************************************/
static int XTrngpsx_WriteSeed(XTrngpsx_Instance *InstancePtr, const u8 *Seed, u8 DLen) {
	volatile int Status = XST_FAILURE;
	u32 SeedLen = (DLen + 1U) * XTRNGPSX_BLOCK_LEN_IN_BYTES;
	volatile u32 Idx = 0U;
	u8 Cnt = 0U;
	u32 Bit = 0U;
	u8 SeedConstruct = 0U;

	while (Idx < SeedLen) {
		SeedConstruct = 0U;
		for (Cnt = 0; Cnt < XTRNGPSX_BYTE_LEN_IN_BITS; Cnt++) {
			Bit = (u32)(Seed[Idx] >> (XTRNGPSX_BYTE_LEN_IN_BITS - 1U - Cnt)) & 0x01U;
			XTrngpsx_WriteReg((InstancePtr->Config.BaseAddress + TRNG_CTRL_4), Bit);
			SeedConstruct = (u8)((SeedConstruct << 1U) | (u8)Bit);
		}
		if (SeedConstruct != Seed[Idx]) {
			goto END;
		}
		usleep(XTRNGPSX_DF_2CLKS_WAIT);
		if ((Idx % XTRNGPSX_DF_NUM_OF_BYTES_BEFORE_MIN_700CLKS_WAIT) == 0U) {
			usleep(XTRNGPSX_DF_700CLKS_WAIT);
		}
		Idx++;
	}
	if (Idx == SeedLen) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function writes personalization string in to the registers.
 *
 * @param	InstancePtr Pointer to XTrngpsx_Instance.
 * @param	PersString Pointer to personalization string.
 *
 * @return
 *		-XST_SUCCESS On successful write
 *		-XST_FAILURE On failure
 *
 **************************************************************************************************/
static int XTrngpsx_WritePersString(XTrngpsx_Instance *InstancePtr, const u8 *PersString) {
	int Status = XST_FAILURE;
	volatile u8 Idx = 0U;
	u8 Cnt = 0U;
	u32 RegVal=0U;

	for (Idx = 0; Idx < XTRNGPSX_PERS_STRING_LEN_IN_WORDS; Idx++){
		RegVal = 0U;
		for (Cnt = 0; Cnt < XTRNGPSX_WORD_LEN_IN_BYTES; Cnt++) {
			RegVal = (RegVal << XTRNGPSX_BYTE_LEN_IN_BITS) |
				PersString[(Idx * XTRNGPSX_WORD_LEN_IN_BYTES) + Cnt];
		}
		XTrngpsx_WriteReg((InstancePtr->Config.BaseAddress + (TRNG_PER_STRNG_11 - (Idx * XTRNGPSX_WORD_LEN_IN_BYTES))),
				RegVal);
	}
	if (Idx == XTRNGPSX_PERS_STRING_LEN_IN_WORDS) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/**************************************************************************************************/
/**
 * @brief
 * Waits for an event to occur for a specified duration.
 *
 * @param	BaseAddress Address of register to be checked for event(s) occurrence.
 * @param	EventMask Mask indicating event(s) to be checked.
 * @param 	Event Specific event(s) value to be checked.
 * @param	Timeout Max number of microseconds to wait for an event(s).
 *
 * @return
 *          XST_SUCCESS - On occurrence of the event(s).
 *          XST_FAILURE - Event did not occur before counter reaches 0.
 *
 **************************************************************************************************/
static inline int XTrngpsx_WaitForEvent(UINTPTR Addr, u32 EventMask, u32 Event,
		u32 Timeout)
{
	return (int)Xil_WaitForEvent(Addr, EventMask, Event, Timeout);
}
