/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_trng.c
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
* 5.0   kpt  05/05/22 Initial release
*       dc   07/13/22 Modified XSECURE_TRNG_DF_MIN_LENGTH to 2
*       kpt  08/03/22 Added volatile keyword to avoid compiler optimization of loop redundancy checks
*       dc   09/04/22 Add an API to set HRNG mode
*
* </pre>
*
* @endcond
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_trng.h"
#include "sleep.h"
#include "xil_util.h"
#include "xstatus.h"
#include "xsecure_trng_hw.h"
#include "xsecure_error.h"
#include "xsecure_plat_kat.h"
#include "xsecure_plat.h"

/************************** Constant Definitions *****************************/
#define XSECURE_TRNG_RESEED_TIMEOUT		1500000U	/**< Reseed timeout in micro-seconds */
#define XSECURE_TRNG_GENERATE_TIMEOUT		1500000U	/**< Generate timeout in micro-seconds */
#define XSECURE_TRNG_WORD_LEN_IN_BYTES		4U	/**< Word length in bytes */
#define XSECURE_TRNG_BYTE_LEN_IN_BITS		8U	/** < Byte length in bits */
#define XSECURE_TRNG_BLOCK_LEN_IN_BYTES		16U	/** < TRNG block length length in bytes */
#define XSECURE_TRNG_MIN_SEEDLIFE		1U	/**< Minimum seed life */
#define XSECURE_TRNG_MAX_SEEDLIFE		0x80000 /**< Maximum seed life 2^19 */
#define XSECURE_TRNG_SEC_STRENGTH_IN_BURSTS	2U	/**< Security strength in 128-bit bursts */
#define XSECURE_TRNG_BURST_SIZE_IN_WORDS	4U	/**< Burst size in words */
#define XSECURE_TRNG_DF_MIN_LENGTH		2U	/**< Minimum DF input length */
#define XSECURE_TRNG_DF_MAX_LENGTH		0x1FU	/**< Maximum DF input length */

#define XSECURE_TRNG_DF_NUM_OF_BYTES_BEFORE_MIN_700CLKS_WAIT	8U	/**< Number of bytes to be written before wait */

#define XSECURE_TRNG_ADAPTPROPTESTCUTOFF_MAX_VAL	0x3FFU /**< maximum adaptprpptest cutoff value */
#define XSECURE_TRNG_REPCOUNTTESTCUTOFF_MAX_VAL		0x1FFU /**< maximum repcounttest cutoff value */
#define XSECURE_TRNG_RESET_DELAY_US			10U /** < Reset delay */
#define XSECURE_TRNG_DF_700CLKS_WAIT			10U /** < delay after 4bytes */
#define XSECURE_TRNG_DF_2CLKS_WAIT			4U /** < delay after 1byte */
#define XSECURE_TRNG_STATUS_QCNT_VAL			4U /** < QCNT value for single burst */

/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/
static int XSecure_TrngReseedInternal(XSecure_TrngInstance *InstancePtr, const u8 *Seed, u8 DLen,
	const u8 *PerStr);
static int XSecure_TrngWritePersString(const u8 *PersString);
static int XSecure_TrngWaitForReseed(XSecure_TrngInstance *InstancePtr);
static int XSecure_TrngTriggerGenerate(XSecure_TrngInstance *InstancePtr, u8 *RandBuf, u32 RandBufSize);
static int XSecure_TrngWriteSeed(const u8 *Seed, u8 DLen);
static inline int XSecure_TrngWaitForEvent(UINTPTR Addr, u32 EventMask, u32 Event,
		u32 Timeout);
static inline void XSecure_TrngWriteReg(UINTPTR Address, u32 RegValue);
static inline u32 XSecure_TrngReadReg(UINTPTR RegAddress);
static inline void XSecure_TrngUtilRMW32(UINTPTR RegAddress, u32 Mask, u32 Value);
static void XSecure_TrngSet(void);
static void XSecure_TrngReset(void);
static void XSecure_TrngPrngReset(void);
static void XSecure_TrngPrngSet(void);
static void XSecure_TrngCfgDfLen(u8 DfLen);
static void XSecure_TrngCfgAdaptPropTestCutoff(u16 AdaptPropTestCutoff);
static void XSecure_TrngCfgRepCountTestCutoff(u16 RepCountTestCutoff);
static void XSecure_TrngCfgDIT(u8 DITValue);

/**************************************************************************************************/
/**
 * @brief
 * Write to the register.
 *
 * @param	Address is the address of the register to be written.
 * @param	RegValue is the value to be written to the register.
 *
 **************************************************************************************************/
static inline void XSecure_TrngWriteReg(UINTPTR Address, u32 RegValue)
{
	Xil_Out32((UINTPTR)Address, RegValue);
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
static inline u32 XSecure_TrngReadReg(UINTPTR RegAddress)
{
	return Xil_In32((UINTPTR)RegAddress);
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
 **************************************************************************************************/
static inline void XSecure_TrngUtilRMW32(UINTPTR RegAddress, u32 Mask, u32 Value) {
	Xil_UtilRMW32((u32)RegAddress, Mask, Value);
}

/*************************************************************************************************/
/**
 * @brief
 * This function brings TRNG core and prng unit out of reset
 *
 **************************************************************************************************/
static void XSecure_TrngSet(void) {
	XSecure_TrngReset();
	usleep(XSECURE_TRNG_RESET_DELAY_US);
	XSecure_TrngUtilRMW32(XSECURE_TRNG_RESET, XSECURE_TRNG_RESET_VAL_MASK, 0U);
	/* Soft reset PRNG unit */
	XSecure_TrngPrngSet();
	/* Update crypto indicator */
	XSecure_UpdateTrngCryptoStatus(XSECURE_SET_BIT);
}

/*************************************************************************************************/
/**
 * @brief
 * This function sets TRNG core in to reset state
 *
 **************************************************************************************************/
static void XSecure_TrngReset(void) {
	XSecure_TrngUtilRMW32(XSECURE_TRNG_RESET, XSECURE_TRNG_RESET_VAL_MASK,
		XSECURE_TRNG_RESET_DEFVAL);
	/* Update crypto indicator */
	XSecure_UpdateTrngCryptoStatus(XSECURE_CLEAR_BIT);
}

/*************************************************************************************************/
/**
 * @brief
 * This function brings PRNG unit out of reset
 *
 **************************************************************************************************/
static void XSecure_TrngPrngSet(void) {
	XSecure_TrngPrngReset();
	usleep(XSECURE_TRNG_RESET_DELAY_US);
	XSecure_TrngUtilRMW32(XSECURE_TRNG_RESET, XSECURE_TRNG_CTRL_PRNGSRST_MASK, 0U);
}

/*************************************************************************************************/
/**
 * @brief
 * This function sets PRNG unit in to reset state
 *
 **************************************************************************************************/
static void XSecure_TrngPrngReset(void) {
	XSecure_TrngUtilRMW32(XSECURE_TRNG_RESET, XSECURE_TRNG_CTRL_PRNGSRST_MASK,
		XSECURE_TRNG_CTRL_PRNGSRST_MASK);
}

/*************************************************************************************************/
/**
 * @brief
 * This function configures the DF input length.
 *
 * @param	DfLen input DF length.
 *
 **************************************************************************************************/
static void XSecure_TrngCfgDfLen(u8 DfLen) {
	XSecure_TrngUtilRMW32(XSECURE_TRNG_CTRL_3, XSECURE_TRNG_CTRL_3_DLEN_MASK,
		(DfLen << XSECURE_TRNG_CTRL_3_DLEN_SHIFT));
}

/*************************************************************************************************/
/**
 * @brief
 * This function writes the cutoff value in to the register.
 *
 * @param	AdaptPropTestCutoff cutoff value for adaptive count test.
 *
 **************************************************************************************************/
static void XSecure_TrngCfgAdaptPropTestCutoff(u16 AdaptPropTestCutoff) {
	XSecure_TrngUtilRMW32(XSECURE_TRNG_CTRL_3, XSECURE_TRNG_CTRL_3_ADAPTPROPTESTCUTOFF_MASK,
		(AdaptPropTestCutoff << XSECURE_TRNG_CTRL_3_ADAPTPROPTESTCUTOFF_SHIFT));
}

/*************************************************************************************************/
/**
 * @brief
 * This function writes the cutoff value in to the register.
 *
 * @param	RepCountTestCutoff cutoff value for repetitive count test.
 *
 **************************************************************************************************/
static void XSecure_TrngCfgRepCountTestCutoff(u16 RepCountTestCutoff) {
	XSecure_TrngUtilRMW32(XSECURE_TRNG_CTRL_2, XSECURE_TRNG_CTRL_2_REPCOUNTTESTCUTOFF_MASK,
		(RepCountTestCutoff << XSECURE_TRNG_CTRL_2_REPCOUNTTESTCUTOFF_SHIFT));
}

/*************************************************************************************************/
/**
 * @brief
 * This function writes the DIT value in to the register.
 *
 **************************************************************************************************/
static void XSecure_TrngCfgDIT(u8 DITValue) {
	XSecure_TrngUtilRMW32(XSECURE_TRNG_CTRL_2, XSECURE_TRNG_CTRL_2_DIT_MASK,
		(DITValue << XSECURE_TRNG_CTRL_2_DIT_SHIFT));
}

/*************************************************************************************************/
/**
 * @brief
 * This function instantiates the TRNG instance with user configure values
 *
 * @param	InstancePtr Pointer to the XSecure_TrngInstance.
 * @param	Seed Pointer to the seed input
 * @param	SeedLength Seed length in bytes
 * @param	PersStr Pointer to the personalization string input
 * @param	UserCfg Pointer to the XSecure_TrngUserConfig
 *
 * @return
 * 		- XST_SUCCESS On successful instantation
 * 		- XSECURE_TRNG_INVALID_PARAM If invalid parameter(s) passed to this function.
 * 		- XSECURE_TRNG_INVALID_SEED_VALUE If provide seed is NULL in DRBG mode
 * 		- XSECURE_TRNG_INVALID_STATE If state is not in uninstantiate state
 * 		- XSECURE_TRNG_UNHEALTHY_STATE If TRNG KAT fails
 * 		- XSECURE_TRNG_INVALID_MODE  If invalid mode is passed to this function
 * 		- XSECURE_TRNG_INVALID_DF_LENGTH If invalid DF input length is passed as a function
 * 		- XSECURE_TRNG_INVALID_SEED_LENGTH If provide seed length doesn't match with given DF length
 * 		- XSECURE_TRNG_INVALID_SEED_LIFE If invalid seed life is provided
 * 		- XSECURE_TRNG_INVALID_ADAPTPROPTEST_CUTOFF_VALUE If invalid cutoff value is provided
 * 		- XSECURE_TRNG_INVALID_REPCOUNTTEST_CUTOFF_VALUE If invalid repetitive test cutoff value is provided
 * 		- XSECURE_TRNG_USER_CFG_COPY_ERROR If error occurred during copy of XSecure_TrngUserConfig structure
 * 		- XSECURE_TRNG_TIMEOUT_ERROR If timeout occurred waiting for done bit
 * 		- XSECURE_TRNG_CATASTROPHIC_CTF_ERROR If CTF bit asserted in STATUS register.
 * 		- XSECURE_TRNG_ERROR_WRITE On write failure
 * 		- XST_FAILURE On unexpected failure
 *
 **************************************************************************************************/
int XSecure_TrngInstantiate(XSecure_TrngInstance *InstancePtr, const u8 *Seed, u32 SeedLength, const u8 *PersStr,
			const XSecure_TrngUserConfig *UserCfg) {
	volatile int Status = XST_FAILURE;

	if ((UserCfg == NULL) || (InstancePtr == NULL)) {
		Status = XSECURE_TRNG_INVALID_PARAM;
		goto END;
	}

	if ((Seed == NULL) && (UserCfg->Mode == XSECURE_TRNG_DRNG_MODE)) {
		Status = XSECURE_TRNG_INVALID_SEED_VALUE;
		goto END;
	}

	if ((Seed != NULL) && (UserCfg->Mode == XSECURE_TRNG_HRNG_MODE)) {
		Status = XSECURE_TRNG_INVALID_SEED_VALUE;
		goto END;
	}

	if (InstancePtr->State != XSECURE_TRNG_UNINITIALIZED_STATE) {
		Status = XSECURE_TRNG_INVALID_STATE;
		goto END;
	}

	if ((InstancePtr->ErrorState != XSECURE_TRNG_HEALTHY) &&
		(InstancePtr->ErrorState != XSECURE_TRNG_STARTUP_TEST)) {
		Status = XSECURE_TRNG_UNHEALTHY_STATE;
		goto END;
	}

	if ((UserCfg->Mode != XSECURE_TRNG_DRNG_MODE) && (UserCfg->Mode != XSECURE_TRNG_PTRNG_MODE) &&
		(UserCfg->Mode != XSECURE_TRNG_HRNG_MODE)) {
		Status = XSECURE_TRNG_INVALID_MODE;
		goto END;
	}

	if ((UserCfg->DFLength  < XSECURE_TRNG_DF_MIN_LENGTH) ||
		(UserCfg->DFLength > XSECURE_TRNG_DF_MAX_LENGTH)) {
		Status = XSECURE_TRNG_INVALID_DF_LENGTH;
		goto END;
	}

	if ((UserCfg->Mode == XSECURE_TRNG_DRNG_MODE) &&
		(SeedLength != ((UserCfg->DFLength + 1U) * XSECURE_TRNG_BLOCK_LEN_IN_BYTES))) {
		Status = XSECURE_TRNG_INVALID_SEED_LENGTH;
		goto END;
	}

	if ((UserCfg->SeedLife < XSECURE_TRNG_MIN_SEEDLIFE) ||
		(UserCfg->SeedLife > XSECURE_TRNG_MAX_SEEDLIFE)) {
		Status = XSECURE_TRNG_INVALID_SEED_LIFE;
		goto END;
	}

	if ((UserCfg->Mode != XSECURE_TRNG_DRNG_MODE) && ((UserCfg->AdaptPropTestCutoff < 1U) ||
		(UserCfg->AdaptPropTestCutoff > XSECURE_TRNG_ADAPTPROPTESTCUTOFF_MAX_VAL))) {
		Status = XSECURE_TRNG_INVALID_ADAPTPROPTEST_CUTOFF_VALUE;
		goto END;
	}

	if ((UserCfg->Mode != XSECURE_TRNG_DRNG_MODE) && ((UserCfg->RepCountTestCutoff < 1U) ||
		(UserCfg->RepCountTestCutoff > XSECURE_TRNG_REPCOUNTTESTCUTOFF_MAX_VAL))) {
		Status = XSECURE_TRNG_INVALID_REPCOUNTTEST_CUTOFF_VALUE;
		goto END;
	}

	Status = Xil_SMemCpy(&InstancePtr->UserCfg, sizeof(XSecure_TrngUserConfig), UserCfg,
			sizeof(XSecure_TrngUserConfig), sizeof(XSecure_TrngUserConfig));
	if (Status != XST_SUCCESS) {
		Status = XSECURE_TRNG_USER_CFG_COPY_ERROR;
		goto END;
	}

	/* Bring TRNG and PRNG unit core out of reset */
	XSecure_TrngSet();

	if ((UserCfg->Mode == XSECURE_TRNG_PTRNG_MODE) ||
		(UserCfg->Mode == XSECURE_TRNG_HRNG_MODE)) {
		/* Configure cutoff values */
		XSecure_TrngCfgAdaptPropTestCutoff(UserCfg->AdaptPropTestCutoff);
		XSecure_TrngCfgRepCountTestCutoff(UserCfg->RepCountTestCutoff);
		/* Configure default DIT value */
		XSecure_TrngCfgDIT(XSECURE_TRNG_CTRL_2_DIT_DEFVAL);
	}

	InstancePtr->State = XSECURE_TRNG_INSTANTIATE_STATE;

	/* Do reseed operation when mode is DRNG/HRNG */
	if ((UserCfg->Mode == XSECURE_TRNG_DRNG_MODE) ||
		(UserCfg->Mode == XSECURE_TRNG_HRNG_MODE)) {
		Status = XST_FAILURE;
		Status = XSecure_TrngReseedInternal(InstancePtr, Seed, InstancePtr->UserCfg.DFLength,
				PersStr);
		if ((Status != XST_SUCCESS) || (InstancePtr->State != XSECURE_TRNG_RESEED_STATE)) {
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	if (InstancePtr != NULL) {
		if ((Status != XST_SUCCESS) &&
		(InstancePtr->ErrorState != XSECURE_TRNG_CATASTROPHIC)) {
			InstancePtr->ErrorState = XSECURE_TRNG_ERROR;
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function triggers and reseeds the DRBG module
 *
 * @param	InstancePtr Pointer to XSecure_TrngInstance.
 * @param	Seed Pointer to the seed input
 * @param	DLen Seed length in TRNG block size
 *
 * @return
 * 		- XST_SUCCESS On successful reseed
 * 		- XSECURE_TRNG_INVALID_PARAM If invalid parameter(s) passed to this function.
 * 		- XSECURE_TRNG_INVALID_SEED_VALUE If provide seed is NULL in DRBG/HRNG mode
 * 		- XSECURE_TRNG_INVALID_MODE  If invalid mode is passed to this function
 * 		- XSECURE_TRNG_INVALID_DF_LENGTH If invalid DF input length is passed as a function
 * 		- XSECURE_TRNG_INVALID_STATE If state is not sequenced correctly
 * 		- XSECURE_TRNG_UNHEALTHY_STATE If TRNG is in failure state, needs an uninstantiation
 * 			or KAT should be run if the error is catastrophic
 *		- XSECURE_TRNG_TIMEOUT_ERROR If timeout occurred waiting for done bit
 *		- XSECURE_TRNG_CATASTROPHIC_CTF_ERROR If CTF bit asserted in STATUS register.
 *		- XSECURE_TRNG_ERROR_WRITE On write failure
 * 		- XST_FAILURE On unexpected failure
 *
 **************************************************************************************************/
int XSecure_TrngReseed(XSecure_TrngInstance *InstancePtr, const u8 *Seed, u8 DLen) {
	volatile int Status = XST_FAILURE;

	if (InstancePtr == NULL) {
		Status = XSECURE_TRNG_INVALID_PARAM;
		goto END;
	}

	if ((InstancePtr->UserCfg.Mode == XSECURE_TRNG_DRNG_MODE) && (Seed == NULL)) {
		Status = XSECURE_TRNG_INVALID_SEED_VALUE;
		goto END;
	}

	if ((Seed != NULL) && (InstancePtr->UserCfg.Mode == XSECURE_TRNG_HRNG_MODE)) {
		Status = XSECURE_TRNG_INVALID_SEED_VALUE;
		goto END;
	}

	if ((DLen < XSECURE_TRNG_DF_MIN_LENGTH) || (DLen > XSECURE_TRNG_DF_MAX_LENGTH)) {
		Status = XSECURE_TRNG_INVALID_DF_LENGTH;
		goto END;
	}

	if (InstancePtr->UserCfg.Mode == XSECURE_TRNG_PTRNG_MODE) {
		Status = XSECURE_TRNG_INVALID_MODE;
		goto END;
	}

	if (InstancePtr->State == XSECURE_TRNG_UNINITIALIZED_STATE) {
		Status = XSECURE_TRNG_INVALID_STATE;
		goto END;
	}

	if ((InstancePtr->ErrorState != XSECURE_TRNG_HEALTHY) &&
		(InstancePtr->ErrorState != XSECURE_TRNG_STARTUP_TEST)) {
		Status = XSECURE_TRNG_UNHEALTHY_STATE;
		goto END;
	}

	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_TrngReseedInternal, InstancePtr,
		Seed, DLen, NULL);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function generates and collects random data in to a buffer.
 *
 * @param	InstancePtr Pointer to XSecure_TrngInstance
 * @param	RandBuf Pointer to buffer in which random data is stored.
 * @param	RandBufSize Size of the buffer in which random data is stored.
 *
 * @return
 * 		- XST_SUCCESS On successful generate of random number
 * 		- XSECURE_TRNG_INVALID_PARAM If invalid parameter(s) passed to this function.
 * 		- XSECURE_TRNG_INVALID_STATE If state is not sequenced correctly
 * 		- XSECURE_TRNG_INVALID_MODE  If invalid mode is passed to this function
 * 		- XSECURE_TRNG_INVALID_BUF_SIZE If buffer is less that 256 bytes or NULL
 * 		- XSECURE_TRNG_UNHEALTHY_STATE If TRNG is in failure state, needs an uninstantiation
 * 			or KAT should be run if error is catastrophic
 * 		- XSECURE_TRNG_RESEED_REQUIRED_ERROR If elapsed seed life exceeds the requested seed life
 * 			in DRBG mode
 *		- XSECURE_TRNG_TIMEOUT_ERROR If timeout occurred waiting for QCNT to become 4.
 *		- XSECURE_TRNG_CATASTROPHIC_DTF_ERROR If DTF bit asserted in STATUS register.
 *		- XSECURE_TRNG_ERROR_WRITE On write failure
 * 		- XST_FAILURE On unexpected failure
 *
 **************************************************************************************************/
int XSecure_TrngGenerate(XSecure_TrngInstance *InstancePtr, u8 *RandBuf, u32 RandBufSize) {
	volatile int Status = XST_FAILURE;

	if ((InstancePtr == NULL) || (RandBuf == NULL)) {
		Status = XSECURE_TRNG_INVALID_PARAM;
		goto END;
	}

	if ((InstancePtr->UserCfg.Mode == XSECURE_TRNG_PTRNG_MODE) &&
		(InstancePtr->State != XSECURE_TRNG_INSTANTIATE_STATE) &&
		(InstancePtr->State != XSECURE_TRNG_GENERATE_STATE)) {
		Status = XSECURE_TRNG_INVALID_STATE;
		goto END;
	}

	if ((InstancePtr->UserCfg.Mode != XSECURE_TRNG_PTRNG_MODE) &&
		(InstancePtr->State != XSECURE_TRNG_RESEED_STATE) &&
		(InstancePtr->State != XSECURE_TRNG_GENERATE_STATE)) {
		Status = XSECURE_TRNG_INVALID_STATE;
		goto END;
	}

	if ((RandBufSize == 0U) || (RandBufSize > XSECURE_TRNG_SEC_STRENGTH_IN_BYTES) ||
		((RandBufSize % XSECURE_TRNG_WORD_LEN_IN_BYTES) != 0U)) {
		Status = XSECURE_TRNG_INVALID_BUF_SIZE;
		goto END;
	}

	if ((InstancePtr->ErrorState != XSECURE_TRNG_HEALTHY) &&
		(InstancePtr->ErrorState != XSECURE_TRNG_STARTUP_TEST)) {
		Status = XSECURE_TRNG_UNHEALTHY_STATE;
		goto END;
	}

	if ((InstancePtr->UserCfg.Mode == XSECURE_TRNG_DRNG_MODE) ||
		(InstancePtr->UserCfg.Mode == XSECURE_TRNG_HRNG_MODE)) {
		if (InstancePtr->TrngStats.ElapsedSeedLife >= InstancePtr->UserCfg.SeedLife) {
			/* Auto reseed in HRNG mode */
			if (InstancePtr->UserCfg.Mode == XSECURE_TRNG_HRNG_MODE) {
				XSECURE_TEMPORAL_CHECK(END, Status, XSecure_TrngReseed, InstancePtr,
						NULL, 0U);
			}
			else {
				Status = XSECURE_TRNG_RESEED_REQUIRED_ERROR;
				goto END;
			}
		}
	}
	else if (InstancePtr->UserCfg.Mode == XSECURE_TRNG_PTRNG_MODE) {
		/* Enable ring oscillators for random seed source */
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SecureRMW32, XSECURE_TRNG_OSC_EN,
			XSECURE_TRNG_OSC_EN_VAL_MASK, XSECURE_TRNG_OSC_EN_VAL_MASK);

		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SecureRMW32, XSECURE_TRNG_CTRL,
			XSECURE_TRNG_CTRL_TRSSEN_MASK | XSECURE_TRNG_CTRL_EUMODE_MASK |
			XSECURE_TRNG_CTRL_PRNGXS_MASK, XSECURE_TRNG_CTRL_TRSSEN_MASK |
			XSECURE_TRNG_CTRL_EUMODE_MASK);
	}
	else {
		Status = XSECURE_TRNG_INVALID_MODE;
		goto END;
	}

	/* Trigger generate and collect random data */
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_TrngTriggerGenerate, InstancePtr, RandBuf,
			RandBufSize);

	InstancePtr->TrngStats.ElapsedSeedLife++;
	InstancePtr->State = XSECURE_TRNG_GENERATE_STATE;
	Status = XST_SUCCESS;

END:
	if (InstancePtr != NULL) {
		if ((Status != XST_SUCCESS) &&
		(InstancePtr->ErrorState != XSECURE_TRNG_CATASTROPHIC)) {
			InstancePtr->ErrorState = XSECURE_TRNG_ERROR;
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function uninstantiates the TRNG instance.
 *
 * @param	InstancePtr Pointer to XSecure_TrngInstance.
 *
 * @return
 * 		- XST_SUCCESS if uninstantiation was successful.
 * 		- XSECURE_TRNG_INVALID_PARAM if invalid instance is passed to function
 *		- XSECURE_TRNG_ERROR_MEMSET_UNINSTANTIATE_ERROR if memset was not successful
 * 		- XST_FAILURE On unexpected failure
 *
 **************************************************************************************************/
int XSecure_TrngUninstantiate(XSecure_TrngInstance *InstancePtr) {
	int Status = XST_FAILURE;
	XSecure_TrngErrorState ErrorState;

	if (InstancePtr == NULL) {
		Status = XSECURE_TRNG_INVALID_PARAM;
		goto END;
	}

	/* Bring cores in to reset state */
	XSecure_TrngReset();
	XSecure_TrngPrngReset();

	/* Disable ring oscillators as a random seed source */
	Status = Xil_SecureRMW32(XSECURE_TRNG_OSC_EN, XSECURE_TRNG_OSC_EN_VAL_MASK,
			XSECURE_TRNG_OSC_EN_VAL_DEFVAL);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	ErrorState = InstancePtr->ErrorState;
	Status = Xil_SMemSet(InstancePtr, sizeof(XSecure_TrngInstance), 0U, sizeof(XSecure_TrngInstance));
	if (Status != XST_SUCCESS) {
		Status = XSECURE_TRNG_MEMSET_UNINSTANTIATE_ERROR;
		goto END;
	}

	InstancePtr->State = XSECURE_TRNG_UNINITIALIZED_STATE;

	/* Retain the error state */
	InstancePtr->ErrorState = ErrorState;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides the pointer to the common trng instance
 *
 * @return	Pointer to the XSecure_TrngInstance instance
 *
 *****************************************************************************/
XSecure_TrngInstance *XSecure_GetTrngInstance(void)
{
	static XSecure_TrngInstance TrngInstance = {0U};

	return &TrngInstance;
}

/*****************************************************************************/
/**
 * @brief	This function initialize and configures the TRNG into HRNG mode of operation.
 *
 * @return
 *			- XST_SUCCESS upon success.
 *			- Error code on failure.
 *
 *****************************************************************************/
int XSecure_TrngInitNCfgHrngMode (void)
{
	int Status = XST_FAILURE;
	XSecure_TrngUserConfig UsrCfg;
	XSecure_TrngInstance *TrngInstance = XSecure_GetTrngInstance();

	if (TrngInstance->State != XSECURE_TRNG_UNINITIALIZED_STATE) {
		Status = XSecure_TrngUninstantiate(TrngInstance);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	/* Initiate TRNG */
	UsrCfg.Mode = XSECURE_TRNG_HRNG_MODE;
	UsrCfg.AdaptPropTestCutoff = XSECURE_TRNG_USER_CFG_ADAPT_TEST_CUTOFF;
	UsrCfg.RepCountTestCutoff = XSECURE_TRNG_USER_CFG_REP_TEST_CUTOFF;
	UsrCfg.DFLength = XSECURE_TRNG_USER_CFG_DF_LENGTH;
	UsrCfg.SeedLife = XSECURE_TRNG_USER_CFG_SEED_LIFE;
	Status = XSecure_TrngInstantiate(TrngInstance, NULL, 0U, NULL, &UsrCfg);
	if (Status != XST_SUCCESS) {
		(void)XSecure_TrngUninstantiate(TrngInstance);
	}

END:
	return Status;
}
/*************************************************************************************************/
/**
 * @brief
 * This function triggers and reseeds the DRBG module
 *
 * @param	InstancePtr Pointer to XSecure_TrngInstance.
 * @param	Seed Pointer to the seed input
 * @param	DLen Seed length in multiples of TRNG block size
 * @param	PerStr Pointer to the personalization string
 *
 * @return
 * 		- XST_SUCCESS On successful reseed
 * 		- XSECURE_TRNG_ERROR_WRITE On write failure
 *		- XSECURE_TRNG_TIMEOUT_ERROR If timeout occurred waiting for done bit
 *		- XSECURE_TRNG_CATASTROPHIC_CTF_ERROR If CTF bit asserted in STATUS register.
 *
 **************************************************************************************************/
static int XSecure_TrngReseedInternal(XSecure_TrngInstance *InstancePtr, const u8 *Seed, u8 DLen,
		const u8 *PerStr) {
	volatile int Status = XST_FAILURE;
	u32 PersMask = XSECURE_TRNG_CTRL_PERSODISABLE_MASK;

	/* Configure DF Len */
	XSecure_TrngCfgDfLen(DLen);

	if (PerStr != NULL) {
		Status = XST_FAILURE;
		Status = XSecure_TrngWritePersString(PerStr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		PersMask = XSECURE_TRNG_CTRL_PERSODISABLE_DEFVAL;
	}

	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SecureRMW32, XSECURE_TRNG_CTRL,
		XSECURE_TRNG_CTRL_PERSODISABLE_MASK | XSECURE_TRNG_CTRL_PRNGSTART_MASK, PersMask);

	/* DRNG Mode */
	if (Seed != NULL) {
		/* Enable TST mode and set PRNG mode for reseed operation*/
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SecureRMW32, XSECURE_TRNG_CTRL,
			XSECURE_TRNG_CTRL_PRNGMODE_MASK | XSECURE_TRNG_CTRL_TSTMODE_MASK |
			XSECURE_TRNG_CTRL_TRSSEN_MASK, XSECURE_TRNG_CTRL_TSTMODE_MASK |
			XSECURE_TRNG_CTRL_TRSSEN_MASK);

		/* Start reseed operation */
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SecureRMW32, XSECURE_TRNG_CTRL,
			XSECURE_TRNG_CTRL_PRNGSTART_MASK, XSECURE_TRNG_CTRL_PRNGSTART_MASK);

		/* For writing seed as an input to DF, PRNG start needs to be set */
		XSECURE_TEMPORAL_CHECK(END,Status, XSecure_TrngWriteSeed, Seed,
			DLen);
	}
	else { /* HTRNG Mode */
		/* Enable ring oscillators for random seed source */
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SecureRMW32, XSECURE_TRNG_OSC_EN,
			XSECURE_TRNG_OSC_EN_VAL_MASK, XSECURE_TRNG_OSC_EN_VAL_MASK);

		/* Enable TRSSEN and set PRNG mode for reseed operation */
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SecureRMW32, XSECURE_TRNG_CTRL,
			XSECURE_TRNG_CTRL_PRNGMODE_MASK | XSECURE_TRNG_CTRL_TRSSEN_MASK |
			XSECURE_TRNG_CTRL_PRNGXS_MASK, XSECURE_TRNG_CTRL_TRSSEN_MASK);

		/* Start reseed operation */
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SecureRMW32, XSECURE_TRNG_CTRL,
			XSECURE_TRNG_CTRL_PRNGSTART_MASK, XSECURE_TRNG_CTRL_PRNGSTART_MASK);
	}

	/* Wait for reseed operation and check CTF flag */
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_TrngWaitForReseed, InstancePtr);

	InstancePtr->State = XSECURE_TRNG_RESEED_STATE;
	Status = XST_SUCCESS;
	InstancePtr->TrngStats.ElapsedSeedLife = 0U;

END:
	if ((Status != XST_SUCCESS) && (InstancePtr->ErrorState != XSECURE_TRNG_CATASTROPHIC)) {
		InstancePtr->ErrorState = XSECURE_TRNG_ERROR;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function waits for the reseed done bit.
 *
 * @param	InstancePtr Pointer to XSecure_TrngInstance.
 *
 * @return
 *		- XST_SUCCESS if Random number collection was successful.
 *		- XSECURE_TRNG_TIMEOUT_ERROR if timeout occurred waiting for done bit
 *		- XSECURE_TRNG_CATASTROPHIC_CTF_ERROR if CTF bit asserted in STATUS register.
 *
 **************************************************************************************************/
static int XSecure_TrngWaitForReseed(XSecure_TrngInstance *InstancePtr) {
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;

	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XSecure_TrngWaitForEvent, (UINTPTR)XSECURE_TRNG_STATUS,
			XSECURE_TRNG_STATUS_DONE_MASK, XSECURE_TRNG_STATUS_DONE_MASK,
			XSECURE_TRNG_RESEED_TIMEOUT);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XSECURE_TRNG_TIMEOUT_ERROR;
		goto END;
	}
	if ((Xil_In32(XSECURE_TRNG_STATUS) & XSECURE_TRNG_STATUS_CERTF_MASK) ==
			XSECURE_TRNG_STATUS_CERTF_MASK) {
		InstancePtr->ErrorState = XSECURE_TRNG_CATASTROPHIC;
		Status = XSECURE_TRNG_CATASTROPHIC_CTF_ERROR;
	}

END:
	XSecure_TrngUtilRMW32(XSECURE_TRNG_CTRL, XSECURE_TRNG_CTRL_PRNGSTART_MASK |
			XSECURE_TRNG_CTRL_TRSSEN_MASK, 0U);
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function triggers Generate operation and collects random data in buffer.
 *
 * @param	InstancePtr Pointer to XSecure_TrngInstance.
 * @param	RandBuf Pointer to the buffer in which random data is stored.
 * @param	RandBufSize Buffer size in bytes.
 *
 * @return
 *		- XST_SUCCESS if Random number collection was successful.
 *		- XSECURE_TRNG_TIMEOUT_ERROR if timeout occurred waiting for QCNT to become 4.
 *		- XSECURE_TRNG_CATASTROPHIC_DTF_ERROR if DTF bit asserted in STATUS register.
 *
 **************************************************************************************************/
static int XSecure_TrngTriggerGenerate(XSecure_TrngInstance *InstancePtr, u8 *RandBuf, u32 RandBufSize) {
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	u8 Idx = 0U;
	volatile u8 NumofBursts = 0U;
	u8 BurstIdx = 0U;
	u32 RegVal = 0U;
	u32 Size = RandBufSize / XSECURE_TRNG_WORD_LEN_IN_BYTES;

	/* Set PRNG mode to generate */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, Xil_SecureRMW32, XSECURE_TRNG_CTRL,
		XSECURE_TRNG_CTRL_PRNGMODE_MASK, XSECURE_TRNG_CTRL_PRNGMODE_MASK);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		goto END;
	}

	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, Xil_SecureRMW32, XSECURE_TRNG_CTRL,
		XSECURE_TRNG_CTRL_PRNGSTART_MASK, XSECURE_TRNG_CTRL_PRNGSTART_MASK);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		goto END;
	}

	for (NumofBursts = 0; NumofBursts < XSECURE_TRNG_SEC_STRENGTH_IN_BURSTS; NumofBursts++) {
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XSecure_TrngWaitForEvent, (UINTPTR)XSECURE_TRNG_STATUS,
			XSECURE_TRNG_STATUS_QCNT_MASK,
			(XSECURE_TRNG_STATUS_QCNT_VAL << XSECURE_TRNG_STATUS_QCNT_SHIFT),
			XSECURE_TRNG_GENERATE_TIMEOUT);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			Status = XSECURE_TRNG_TIMEOUT_ERROR;
			goto END;
		}
		if ((Xil_In32(XSECURE_TRNG_STATUS) & XSECURE_TRNG_STATUS_DTF_MASK) ==
			XSECURE_TRNG_STATUS_DTF_MASK) {
			InstancePtr->ErrorState = XSECURE_TRNG_CATASTROPHIC;
			Status = XSECURE_TRNG_CATASTROPHIC_DTF_ERROR;
			goto END;
		}
		BurstIdx = NumofBursts * XSECURE_TRNG_BURST_SIZE_IN_WORDS;
		for (Idx = 0U; Idx < XSECURE_TRNG_BURST_SIZE_IN_WORDS; Idx++) {
			RegVal = XSecure_TrngReadReg(XSECURE_TRNG_CORE_OUTPUT);
			if ((Idx + BurstIdx) < Size) {
				*((u32*)RandBuf + Idx + BurstIdx) =
						Xil_EndianSwap32(RegVal);
			}
		}
	}
	if (NumofBursts == XSECURE_TRNG_SEC_STRENGTH_IN_BURSTS) {
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
 * @param	Seed Pointer to the seed input.
 * @param	DLen Seed length in multiples of TRNG block size
 *
 * @return
 * 		-XST_SUCCESS On successful write
 * 		-XST_FAILURE On failure
 *
 **************************************************************************************************/
static int XSecure_TrngWriteSeed(const u8 *Seed, u8 DLen) {
	int Status = XST_FAILURE;
	u32 SeedLen = (DLen + 1U) * XSECURE_TRNG_BLOCK_LEN_IN_BYTES;
	volatile u32 Idx = 0U;
	u8 Cnt = 0U;
	u32 Bit = 0U;
	u8 SeedConstruct = 0U;

	while (Idx < SeedLen) {
		SeedConstruct = 0U;
		for (Cnt = 0; Cnt < XSECURE_TRNG_BYTE_LEN_IN_BITS; Cnt++) {
			Bit = (u32)(Seed[Idx] >> (XSECURE_TRNG_BYTE_LEN_IN_BITS - 1U - Cnt)) & 0x01U;
			XSecure_TrngWriteReg(XSECURE_TRNG_CTRL_4, Bit);
			SeedConstruct = (u8)((SeedConstruct << 1U) | (u8)Bit);
		}
		if (SeedConstruct != Seed[Idx]) {
			goto END;
		}
		usleep(XSECURE_TRNG_DF_2CLKS_WAIT);
		if ((Idx % XSECURE_TRNG_DF_NUM_OF_BYTES_BEFORE_MIN_700CLKS_WAIT) != 0U) {
			usleep(XSECURE_TRNG_DF_700CLKS_WAIT);
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
 * @param	PersString Pointer to personalization string.
 *
 * @return
 *		-XST_SUCCESS On successful write
 *		-XST_FAILURE On failure
 *
 **************************************************************************************************/
static int XSecure_TrngWritePersString(const u8 *PersString) {
	int Status = XST_FAILURE;
	volatile u8 Idx = 0U;
	u8 Cnt = 0U;
	u32 RegVal=0U;

	for (Idx = 0; Idx < XSECURE_TRNG_PERS_STRING_LEN_IN_WORDS; Idx++){
		RegVal = 0U;
		for (Cnt = 0; Cnt < XSECURE_TRNG_WORD_LEN_IN_BYTES; Cnt++) {
			RegVal = (RegVal << XSECURE_TRNG_BYTE_LEN_IN_BITS) |
				PersString[(Idx * XSECURE_TRNG_WORD_LEN_IN_BYTES) + Cnt];
		}
		XSecure_TrngWriteReg((XSECURE_TRNG_PER_STRNG_11 - (Idx * XSECURE_TRNG_WORD_LEN_IN_BYTES)),
				RegVal);
	}
	if (Idx == XSECURE_TRNG_PERS_STRING_LEN_IN_WORDS) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/**************************************************************************************************/
/**
 * @brief
 * Waits for an event to occur for a specified duration.
 *
 * @param	BaseAddr Address of register to be checked for event(s) occurrence.
 * @param	EventMask Mask indicating event(s) to be checked.
 * @param 	Event Specific event(s) value to be checked.
 * @param	Timeout Max number of microseconds to wait for an event(s).
 *
 * @return
 *          XST_SUCCESS - On occurrence of the event(s).
 *          XST_FAILURE - Event did not occur before counter reaches 0.
 *
 **************************************************************************************************/
static inline int XSecure_TrngWaitForEvent(UINTPTR Addr, u32 EventMask, u32 Event,
		u32 Timeout)
{
	return (int)Xil_WaitForEvent(Addr, EventMask, Event, Timeout);
}
