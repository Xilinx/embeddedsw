/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xtrng.c
 * @addtogroup Overview
 * @{
 *
 * This file contains the required functions to operate TRNG core.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   05/20/24 Initial release
 *       ma   07/01/24 Update few TRNG APIs to support DRBG related IPI commands
 *       ma   07/23/24 Updated NRNPS and NRN_AVAIL in case of AUTOPROC mode
 *       ma   07/26/24 Removed XTrng_DisableAutoProcMode API and updated TRNG to support PTRNG mode
 *       yog  08/25/24 Integrated FIH library
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xtrng.h"
#include "xtrng_hw.h"
#include "xasufw_status.h"
#include "xasufw_hw.h"
#include "sleep.h"
#include "xil_util.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/
#define XTRNG_RESEED_TIMEOUT			1500000U	/**< Reseed timeout in micro-seconds */
#define XTRNG_GENERATE_TIMEOUT			1500000U	/**< Generate timeout in micro-seconds */
#define XTRNG_BLOCK_LEN_IN_BYTES		16U	/** < TRNG block length length in bytes */
#define XTRNG_MIN_SEEDLIFE				1U	/**< Minimum seed life */
#define XTRNG_MAX_SEEDLIFE				0x80000 /**< Maximum seed life 2^19 */
#define XTRNG_SEC_STRENGTH_IN_BURSTS	2U	/**< Security strength in 128-bit bursts */
#define XTRNG_BURST_SIZE_IN_WORDS 		4U	/**< Burst size in words */
#define XTRNG_DF_MIN_LENGTH				2U	/**< Minimum DF input length */
#define XTRNG_DF_MAX_LENGTH				0x1FU	/**< Maximum DF input length */
#define XTRNG_AUTOPROC_NRNPS_VALUE		0x3FU /**< Seed life in autoproc mode */

#define XTRNG_DF_NUM_OF_BYTES_BEFORE_MIN_700CLKS_WAIT	8U
/**< Number of bytes to be written before wait */

#define XTRNG_ADAPTPROPTESTCUTOFF_MAX_VAL	0x3FFU /**< maximum adaptprpptest cutoff value */
#define XTRNG_REPCOUNTTESTCUTOFF_MAX_VAL	0x1FFU /**< maximum repcounttest cutoff value */
#define XTRNG_RESET_DELAY_US				10U /** < Reset delay */
#define XTRNG_DF_700CLKS_WAIT				10U /** < delay after 4bytes */
#define XTRNG_DF_2CLKS_WAIT					4U /** < delay after 1byte */
#define XTRNG_STATUS_QCNT_VAL				4U /** < QCNT value for single burst */

/************************************** Type Definitions *****************************************/
/**
* This typedef contains configuration information for a TRNG core.
* Each core should have an associated configuration structure.
*/
typedef struct {
	u16 DeviceId; /**< DeviceId is the unique ID of the device */
	u32 BaseAddress; /**< BaseAddress is the physical base address of the device's registers */
	u32 TrngFifoAddr; /**< TRNG FIFO address */
} XTrng_Config;

typedef struct {
	u32 ElapsedSeedLife; /**< Elapsed seed life */
} XTrng_Status;

typedef enum {
	XTRNG_UNINITIALIZED_STATE = 0, /**< Default state */
	XTRNG_INSTANTIATE_STATE, /**< Instantiate state */
	XTRNG_RESEED_STATE, /**< Reseed state */
	XTRNG_GENERATE_STATE, /**< Generate state */
	XTRNG_AUTOPROC_STATE /**< TRNG in autoproc */
} XTrng_State;

struct _XTrng {
	u16 DeviceId; /**< DeviceId is the unique ID of the device */
	u32 BaseAddress; /**< BaseAddress is the physical base address of the device's registers */
	u32 TrngFifoAddr; /**< TRNG FIFO address */
	u32 IsReady; /**< SHA component ready state */
	XTrng_UserConfig UserCfg; /**< User configurations for TRNG */
	XTrng_Status TrngStats; /**< TRNG core status */
	XTrng_ErrorState ErrorState; /**< TRNG core error status */
	XTrng_State State; /**< TRNG core state */
};

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XTrng_ReseedInternal(XTrng *InstancePtr, u8 *Seed, u8 DLen, u8 *PerStr);
static s32 XTrng_WritePersString(XTrng *InstancePtr, u8 *PersString);
static s32 XTrng_WaitForReseed(XTrng *InstancePtr);
static s32 XTrng_CollectRandData(XTrng *InstancePtr, u8 *RandBuf, u32 RandBufSize);
static s32 XTrng_WriteSeed(XTrng *InstancePtr, u8 *Seed, u8 DLen);
static inline s32 XTrng_WaitForEvent(UINTPTR Addr, u32 EventMask, u32 Event, u32 Timeout);
static s32 XTrng_Set(XTrng *InstancePtr);
static s32 XTrng_Reset(XTrng *InstancePtr);
static s32 XTrng_PrngReset(XTrng *InstancePtr);
static s32 XTrng_PrngSet(XTrng *InstancePtr);
static void XTrng_UpdateConf0(XTrng *InstancePtr, u32 DitVal, u32 RepCountTestCutoff);
static void XTrng_UpdateConf1(XTrng *InstancePtr, u32 DFLen, u32 AdaptPropTestCutoff);

/************************************ Variable Definitions ***************************************/
/* The configuration table for TRNG devices */
static XTrng_Config XTrng_ConfigTable[XASU_XTRNG_NUM_INSTANCES] = {
	{
		XASU_XTRNG_0_DEVICE_ID,
		XASU_XTRNG_0_S_AXI_BASEADDR,
		XASU_XTRNG_0_FIFO_S_AXI_BASEADDR
	}
};

/* TRNG HW instances */
static XTrng XTrng_Instance[XASU_XTRNG_NUM_INSTANCES];

/*************************************************************************************************/
/**
 * @brief   This function returns an instance pointer of the TRNG HW based on Device ID.
 *
 * @param   DeviceId	Unique device ID of the device for the lookup operation.
 *
 * @return
 * 			- It returns pointer to the XTrng instance corresponding to the Device ID.
 *          - It returns NULL if Device ID is invalid.
 *
 *************************************************************************************************/
XTrng *XTrng_GetInstance(u16 DeviceId)
{
	XTrng *XTrng_InstancePtr = NULL;

	if (DeviceId >= XASU_XTRNG_NUM_INSTANCES) {
		XFIH_GOTO(END);
	}

	XTrng_InstancePtr = &XTrng_Instance[DeviceId];
	XTrng_InstancePtr->DeviceId = DeviceId;

END:
	return XTrng_InstancePtr;
}

/*************************************************************************************************/
/**
 * @brief   This function returns a reference to an XTrng_Config structure based on the unique
 * device id. The return value will refer to an entry in the device configuration table defined in
 * the XTrng_ConfigTable array.
 *
 * @param   DeviceId	Unique device ID of the device for the lookup operation.
 *
 * @return
 * 			- It returns CfgPtr which is a reference to a config record in the configuration table
 *            in XTrng_ConfigTable corresponding to the given DeviceId if match is found.
 *          - It returns NULL if no match is found.
 *
 *************************************************************************************************/
static XTrng_Config *XTrng_LookupConfig(u16 DeviceId)
{
	XTrng_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = 0x0U; Index < XASU_XTRNG_NUM_INSTANCES; Index++) {
		if (XTrng_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XTrng_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

/*************************************************************************************************/
/**
 * @brief   This function initializes TRNG core. This function must be called prior to using a TRNG
 * core. Initialization of TRNG includes setting up the instance data and ensuring the hardware is
 * in a quiescent state.
 *
 * @param   InstancePtr		Pointer to the TRNG instance.
 *
 * @return
 * 			- Upon successful initialization of TRNG core, it returns XASUFW_SUCCESS.
 *          - Otherwise, it returns an error code.
 *
 *************************************************************************************************/
s32 XTrng_CfgInitialize(XTrng *InstancePtr)
{
	s32 Status = XASUFW_FAILURE;
	XTrng_Config *CfgPtr = XTrng_LookupConfig(InstancePtr->DeviceId);

	/* Validate input parameters */
	if ((InstancePtr == NULL) || (CfgPtr == NULL)) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/* Initialize SHA configuration */
	InstancePtr->BaseAddress = CfgPtr->BaseAddress;
	InstancePtr->TrngFifoAddr = CfgPtr->TrngFifoAddr;
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	InstancePtr->State = XTRNG_UNINITIALIZED_STATE;
	InstancePtr->ErrorState = XTRNG_STARTUP_TEST;
	Status = XASUFW_SUCCESS;

END:
	if (Status != XASUFW_SUCCESS) {
		InstancePtr->ErrorState = XTRNG_ERROR;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function brings TRNG core and prng unit out of reset.
 *
 *************************************************************************************************/
static s32 XTrng_Set(XTrng *InstancePtr)
{
	s32 Status = XASUFW_FAILURE;

	Status = XTrng_Reset(InstancePtr);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}
	usleep(XTRNG_RESET_DELAY_US);
	Status = Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_RESET_OFFSET,
				 XASU_TRNG_RESET_VAL_MASK, 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}
	/* Soft reset PRNG unit */
	Status = XTrng_PrngSet(InstancePtr);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function sets TRNG core in to reset state.
 *
 *************************************************************************************************/
static s32 XTrng_Reset(XTrng *InstancePtr)
{
	s32 Status = XASUFW_FAILURE;

	Status = Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_RESET_OFFSET,
				 XASU_TRNG_RESET_VAL_MASK, XASU_TRNG_RESET_DEFVAL);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function brings PRNG unit out of reset.
 *
 *************************************************************************************************/
static s32 XTrng_PrngSet(XTrng *InstancePtr)
{
	s32 Status = XASUFW_FAILURE;

	Status = XTrng_PrngReset(InstancePtr);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}
	usleep(XTRNG_RESET_DELAY_US);
	Status = Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
				 XASU_TRNG_CTRL_PRNGSRST_MASK, 0U);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function sets PRNG unit in to reset state.
 *
 * @param   InstancePtr		Pointer to the TRNG Instance.
 *
 *************************************************************************************************/
static s32 XTrng_PrngReset(XTrng *InstancePtr)
{
	s32 Status = XASUFW_FAILURE;

	Status = Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
				 XASU_TRNG_CTRL_PRNGSRST_MASK, XASU_TRNG_CTRL_PRNGSRST_MASK);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function writes the DIT value and repetitive count test cutoff value to the TRNG
 * CONF0 register.
 *
 * @param   InstancePtr			Pointer to the TRNG Instance.
 * @param   DitVal				DIT value.
 * @param	RepCountTestCutoff	Cutoff value for repetitive count test.
 *
 *************************************************************************************************/
static void XTrng_UpdateConf0(XTrng *InstancePtr, u32 DitVal, u32 RepCountTestCutoff)
{
	XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_TRNG_CONF0_OFFSET,
			(DitVal & XASU_TRNG_CONF0_DIT_MASK) |
			((RepCountTestCutoff << XASU_TRNG_CONF0_REPCOUNTTESTCUTOFF_SHIFT)
			 & XASU_TRNG_CONF0_REPCOUNTTESTCUTOFF_MASK));
}

/*************************************************************************************************/
/**
 * @brief   This function writes DF length and adaptive count test cutoff value to the TRNG CONF1
 * register.
 *
 * @param   InstancePtr			Pointer to the TRNG Instance.
 * @param	DFLen				Input DF length.
 * @param   AdaptPropTestCutoff	Cutoff value for adaptive count test.
 *
 *************************************************************************************************/
static void XTrng_UpdateConf1(XTrng *InstancePtr, u32 DFLen, u32 AdaptPropTestCutoff)
{
	XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_TRNG_CONF1_OFFSET,
			(DFLen & XASU_TRNG_CONF1_DLEN_MASK) |
			((AdaptPropTestCutoff << XASU_TRNG_CONF1_ADAPTPROPTESTCUTOFF_SHIFT)
			 & XASU_TRNG_CONF1_ADAPTPROPTESTCUTOFF_MASK));
}

/*************************************************************************************************/
/**
 * @brief   This function instantiates the TRNG instance with user configure values.
 *
 * @param	InstancePtr	Pointer to the TRNG Instance.
 * @param	Seed		Pointer to the seed input.
 * @param	SeedLength	Seed length in bytes.
 * @param	PersStr		Pointer to the personalization string input.
 * @param	UserCfg		Pointer to the XTrng_UserConfig.
 *
 * @return
 *          - XASUFW_SUCCESS On successful instantation
 *          - XASUFW_TRNG_INVALID_PARAM If invalid parameter(s) passed to this function.
 *          - XASUFW_TRNG_INVALID_SEED_VALUE If provide seed is NULL in DRBG mode
 *          - XASUFW_TRNG_INVALID_STATE If state is not in uninstantiate state
 *          - XASUFW_TRNG_UNHEALTHY_STATE If TRNG KAT fails
 *          - XASUFW_TRNG_INVALID_MODE  If invalid mode is passed to this function
 *          - XASUFW_TRNG_INVALID_DF_LENGTH If invalid DF input length is passed as a function
 *          - XASUFW_TRNG_INVALID_SEED_LENGTH If provide seed length doesn't match with given
 *            DF length
 *          - XASUFW_TRNG_INVALID_SEED_LIFE If invalid seed life is provided
 *          - XASUFW_TRNG_INVALID_ADAPTPROPTEST_CUTOFF_VALUE If invalid cutoff value is provided
 *          - XASUFW_TRNG_INVALID_REPCOUNTTEST_CUTOFF_VALUE If invalid repetitive test cutoff
 *            value is provided
 *          - XASUFW_TRNG_USER_CFG_COPY_ERROR If error occurred during copy of XTrng_UserConfig
 *            structure
 *          - XASUFW_TRNG_TIMEOUT_ERROR If timeout occurred waiting for done bit
 *          - XASUFW_TRNG_CATASTROPHIC_CTF_ERROR If CTF bit asserted in STATUS register.
 *          - XASUFW_TRNG_ERROR_WRITE On write failure
 *          - XASUFW_FAILURE On unexpected failure
 *
 *************************************************************************************************/
s32 XTrng_Instantiate(XTrng *InstancePtr, u8 *Seed, u32 SeedLength, u8 *PersStr,
		      XTrng_UserConfig *UserCfg)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);

	if ((UserCfg == NULL) || (InstancePtr == NULL)) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if ((Seed == NULL) && (UserCfg->Mode == XTRNG_DRNG_MODE)) {
		Status = XASUFW_TRNG_INVALID_SEED_VALUE;
		XFIH_GOTO(END);
	}

	if ((Seed != NULL) &&
	    ((UserCfg->Mode == XTRNG_HRNG_MODE) || (UserCfg->Mode == XTRNG_PTRNG_MODE))) {
		Status = XASUFW_TRNG_INVALID_SEED_VALUE;
		XFIH_GOTO(END);
	}

	if (InstancePtr->State != XTRNG_UNINITIALIZED_STATE) {
		Status = XASUFW_TRNG_INVALID_STATE;
		XFIH_GOTO(END);
	}

	if ((InstancePtr->ErrorState != XTRNG_HEALTHY) &&
	    (InstancePtr->ErrorState != XTRNG_STARTUP_TEST)) {
		Status = XASUFW_TRNG_UNHEALTHY_STATE;
		XFIH_GOTO(END);
	}

	if ((UserCfg->Mode != XTRNG_DRNG_MODE) &&
	    (UserCfg->Mode != XTRNG_PTRNG_MODE) &&
	    (UserCfg->Mode != XTRNG_HRNG_MODE)) {
		Status = XASUFW_TRNG_INVALID_MODE;
		XFIH_GOTO(END);
	}

	if ((UserCfg->DFLength  < XTRNG_DF_MIN_LENGTH) || (UserCfg->DFLength > XTRNG_DF_MAX_LENGTH)) {
		Status = XASUFW_TRNG_INVALID_DF_LENGTH;
		XFIH_GOTO(END);
	}

	if ((UserCfg->Mode == XTRNG_DRNG_MODE) &&
	    (SeedLength != ((UserCfg->DFLength + 1U) * XTRNG_BLOCK_LEN_IN_BYTES))) {
		Status = XASUFW_TRNG_INVALID_SEED_LENGTH;
		XFIH_GOTO(END);
	}

	if ((UserCfg->SeedLife < XTRNG_MIN_SEEDLIFE) || (UserCfg->SeedLife > XTRNG_MAX_SEEDLIFE)) {
		Status = XASUFW_TRNG_INVALID_SEED_LIFE;
		XFIH_GOTO(END);
	}

	if ((UserCfg->Mode != XTRNG_DRNG_MODE) && ((UserCfg->AdaptPropTestCutoff < 1U) ||
		(UserCfg->AdaptPropTestCutoff > XTRNG_ADAPTPROPTESTCUTOFF_MAX_VAL))) {
		Status = XASUFW_TRNG_INVALID_ADAPTPROPTEST_CUTOFF_VALUE;
		XFIH_GOTO(END);
	}

	if ((UserCfg->Mode != XTRNG_DRNG_MODE) && ((UserCfg->RepCountTestCutoff < 1U) ||
		(UserCfg->RepCountTestCutoff > XTRNG_REPCOUNTTESTCUTOFF_MAX_VAL))) {
		Status = XASUFW_TRNG_INVALID_REPCOUNTTEST_CUTOFF_VALUE;
		XFIH_GOTO(END);
	}

	if ((UserCfg->Mode != XTRNG_PTRNG_MODE) && (UserCfg->IsBlocking != TRUE) &&
	    (UserCfg->IsBlocking != FALSE)) {
		Status = XASUFW_INVALID_BLOCKING_MODE;
		XFIH_GOTO(END);
	}

	Status = Xil_SMemCpy(&InstancePtr->UserCfg, sizeof(XTrng_UserConfig), UserCfg,
			     sizeof(XTrng_UserConfig), sizeof(XTrng_UserConfig));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_TRNG_USER_CFG_COPY_ERROR;
		XFIH_GOTO(END);
	}

	/* Bring TRNG and PRNG unit core out of reset */
	Status = XTrng_Set(InstancePtr);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	if ((UserCfg->Mode == XTRNG_PTRNG_MODE) || (UserCfg->Mode == XTRNG_HRNG_MODE)) {
		/* Configure DIT value and repetitive count test cutoff value */
		XTrng_UpdateConf0(InstancePtr, XASU_TRNG_CONF0_DIT_DEFVAL, UserCfg->RepCountTestCutoff);
		/* Configure DF length and adaptive count test cutoff value */
		XTrng_UpdateConf1(InstancePtr, UserCfg->DFLength, UserCfg->AdaptPropTestCutoff);
	}

	InstancePtr->State = XTRNG_INSTANTIATE_STATE;
	/* Do reseed operation when mode is DRNG/HRNG */
	if ((UserCfg->Mode == XTRNG_DRNG_MODE) ||
	    (UserCfg->Mode == XTRNG_HRNG_MODE)) {
		Status = XASUFW_FAILURE;
		Status = XTrng_ReseedInternal(InstancePtr, Seed, InstancePtr->UserCfg.DFLength, PersStr);
		if ((Status != XASUFW_SUCCESS) || (InstancePtr->State != XTRNG_RESEED_STATE)) {
			XFIH_GOTO(END);
		}
	}

	Status = XASUFW_SUCCESS;
	InstancePtr->ErrorState = XTRNG_HEALTHY;

END:
	if (InstancePtr != NULL) {
		if ((Status != XST_SUCCESS) && (InstancePtr->ErrorState != XTRNG_CATASTROPHIC)) {
			InstancePtr->ErrorState = XTRNG_ERROR;
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function triggers and reseeds the DRBG module.
 *
 * @param   InstancePtr	Pointer to TRNG Instance.
 * @param	Seed		Pointer to the seed input.
 * @param	DLen		Seed length in TRNG block size.
 *
 * @return
 *          - XASUFW_SUCCESS On successful reseed.
 *          - XASUFW_TRNG_INVALID_PARAM If invalid parameter(s) passed to this function.
 *          - XASUFW_TRNG_INVALID_SEED_VALUE If provide seed is NULL in DRBG/HRNG mode.
 *          - XASUFW_TRNG_INVALID_MODE  If invalid mode is passed to this function.
 *          - XASUFW_TRNG_INVALID_DF_LENGTH If invalid DF input length is passed as a function.
 *          - XASUFW_TRNG_INVALID_STATE If state is not sequenced correctly.
 *          - XASUFW_TRNG_UNHEALTHY_STATE If TRNG is in failure state, needs an
 *            uninstantiation or KAT should be run if the error is catastrophic.
 *          - XASUFW_TRNG_TIMEOUT_ERROR If timeout occurred waiting for done bit.
 *          - XASUFW_TRNG_CATASTROPHIC_CTF_ERROR If CTF bit asserted in STATUS register.
 *          - XASUFW_TRNG_ERROR_WRITE On write failure.
 *          - XASUFW_FAILURE On unexpected failure.
 *
 *************************************************************************************************/
s32 XTrng_Reseed(XTrng *InstancePtr, u8 *Seed, u8 DLen)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);

	if (InstancePtr == NULL) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if ((InstancePtr->UserCfg.Mode == XTRNG_DRNG_MODE) && (Seed == NULL)) {
		Status = XASUFW_TRNG_INVALID_SEED_VALUE;
		XFIH_GOTO(END);
	}

	if ((Seed != NULL) && (InstancePtr->UserCfg.Mode == XTRNG_HRNG_MODE)) {
		Status = XASUFW_TRNG_INVALID_SEED_VALUE;
		XFIH_GOTO(END);
	}

	if ((DLen < XTRNG_DF_MIN_LENGTH) || (DLen > XTRNG_DF_MAX_LENGTH)) {
		Status = XASUFW_TRNG_INVALID_DF_LENGTH;
		XFIH_GOTO(END);
	}

	if (InstancePtr->UserCfg.Mode == XTRNG_PTRNG_MODE) {
		Status = XASUFW_TRNG_INVALID_MODE;
		XFIH_GOTO(END);
	}

	if (InstancePtr->State == XTRNG_UNINITIALIZED_STATE) {
		Status = XASUFW_TRNG_INVALID_STATE;
		XFIH_GOTO(END);
	}

	if ((InstancePtr->ErrorState != XTRNG_HEALTHY) &&
	    (InstancePtr->ErrorState != XTRNG_STARTUP_TEST)) {
		Status = XASUFW_TRNG_UNHEALTHY_STATE;
		XFIH_GOTO(END);
	}

	/* Wait for reseed operation and check CTF flag */
	if ((InstancePtr->State == XTRNG_RESEED_STATE) && (InstancePtr->UserCfg.IsBlocking != TRUE)) {
		Status = XTrng_WaitForReseed(InstancePtr);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
	}

	Status = XTrng_ReseedInternal(InstancePtr, Seed, DLen, NULL);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function generates and collects random data in to a buffer.
 *
 * @param	InstancePtr		Pointer to TRNG Instance.
 * @param	RandBuf			Pointer to buffer in which random data is stored.
 * @param	RandBufSize		Size of the buffer in which random data is stored.
 * @param	PredResistance	Flag that controls Generate level Prediction Resistance.
 * 							When enabled, it mandates fresh seed for every Generate operation.
 *
 * @return
 *          - XST_SUCCESS On successful generate of random number.
 *          - XASUFW_TRNG_INVALID_PARAM If invalid parameter(s) passed to this function.
 *          - XASUFW_TRNG_INVALID_STATE If state is not sequenced correctly.
 *          - XASUFW_TRNG_INVALID_MODE  If invalid mode is passed to this function.
 *          - XASUFW_TRNG_INVALID_BUF_SIZE If buffer is less that 256 bytes or NULL.
 *          - XASUFW_TRNG_UNHEALTHY_STATE If TRNG is in failure state, needs an uninstantiation
 *            or KAT should be run if error is catastrophic.
 *          - XASUFW_TRNG_RESEED_REQUIRED_ERROR If elapsed seed life exceeds the requested
 *            seed life in DRBG mode.
 *          - XASUFW_TRNG_TIMEOUT_ERROR If timeout occurred waiting for QCNT to become 4.
 *          - XASUFW_TRNG_CATASTROPHIC_DTF_ERROR If DTF bit asserted in STATUS register.
 *          - XASUFW_TRNG_ERROR_WRITE On write failure.
 *          - XASUFW_FAILURE On unexpected failure.
 *
 *************************************************************************************************/
s32 XTrng_Generate(XTrng *InstancePtr, u8 *RandBuf, u32 RandBufSize, u8 PredResistance)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);

	if ((InstancePtr == NULL) || (RandBuf == NULL)) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if ((InstancePtr->UserCfg.Mode == XTRNG_PTRNG_MODE) &&
	    (InstancePtr->State != XTRNG_INSTANTIATE_STATE) &&
	    (InstancePtr->State != XTRNG_GENERATE_STATE)) {
		Status = XASUFW_TRNG_INVALID_STATE;
		XFIH_GOTO(END);
	}

	if ((InstancePtr->UserCfg.Mode != XTRNG_PTRNG_MODE) &&
	    (InstancePtr->State != XTRNG_RESEED_STATE) &&
	    (InstancePtr->State != XTRNG_GENERATE_STATE)) {
		Status = XASUFW_TRNG_INVALID_STATE;
		XFIH_GOTO(END);
	}

	if ((RandBufSize == 0U) || (RandBufSize > XTRNG_SEC_STRENGTH_IN_BYTES) ||
	    ((RandBufSize % XASUFW_WORD_LEN_IN_BYTES) != 0U)) {
		Status = XASUFW_TRNG_INVALID_BUF_SIZE;
		XFIH_GOTO(END);
	}

	if ((PredResistance != TRUE) && (PredResistance != FALSE)) {
		Status = XASUFW_INVALID_PREDRES_VALUE;
	}

	if ((InstancePtr->UserCfg.Mode == XTRNG_PTRNG_MODE) && (PredResistance == TRUE)) {
		Status = XASUFW_INVALID_PREDRES_VALUE;
		XFIH_GOTO(END);
	}

	if ((InstancePtr->ErrorState != XTRNG_HEALTHY) &&
	    (InstancePtr->ErrorState != XTRNG_STARTUP_TEST)) {
		Status = XASUFW_TRNG_UNHEALTHY_STATE;
		XFIH_GOTO(END);
	}

	if ((InstancePtr->UserCfg.Mode == XTRNG_DRNG_MODE) ||
	    (InstancePtr->UserCfg.Mode == XTRNG_HRNG_MODE)) {
		if (InstancePtr->UserCfg.Mode == XTRNG_DRNG_MODE) {
			if ((PredResistance == TRUE) &&
			    (InstancePtr->TrngStats.ElapsedSeedLife > 0U)) {
				Status = XASUFW_TRNG_RESEED_REQUIRED_ERROR;
				XFIH_GOTO(END);
			}
		}
		/* Wait for reseed operation and check CTF flag */
		if ((InstancePtr->State == XTRNG_RESEED_STATE) && (InstancePtr->UserCfg.IsBlocking != TRUE)) {
			Status = XTrng_WaitForReseed(InstancePtr);
			if (Status != XASUFW_SUCCESS) {
				XFIH_GOTO(END);
			}
		}

		InstancePtr->UserCfg.PredResistance = PredResistance;
	} else if (InstancePtr->UserCfg.Mode == XTRNG_PTRNG_MODE) {
		/* Enable ring oscillators for random seed source */
		Status = Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_OSC_EN_OFFSET,
					 XASU_TRNG_OSC_EN_VAL_MASK, XASU_TRNG_OSC_EN_VAL_MASK);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_OSCILLATOR_ENABLE_FAILED;
			XFIH_GOTO(END);
		}

		Status = Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
					 XASU_TRNG_CTRL_TRSSEN_MASK | XASU_TRNG_CTRL_EUMODE_MASK | XASU_TRNG_CTRL_PRNGXS_MASK,
					 XASU_TRNG_CTRL_TRSSEN_MASK | XASU_TRNG_CTRL_EUMODE_MASK);
	} else {
		Status = XASUFW_TRNG_INVALID_MODE;
		XFIH_GOTO(END);
	}

	Status = XTrng_CollectRandData(InstancePtr, RandBuf, RandBufSize);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	InstancePtr->TrngStats.ElapsedSeedLife++;
	InstancePtr->State = XTRNG_GENERATE_STATE;
	if (InstancePtr->UserCfg.Mode == XTRNG_HRNG_MODE) {
		/* Auto reseed in HRNG mode */
		if ((InstancePtr->TrngStats.ElapsedSeedLife >= InstancePtr->UserCfg.SeedLife) ||
		    (PredResistance == TRUE)) {
			Status = XTrng_Reseed(InstancePtr, NULL, InstancePtr->UserCfg.DFLength);
		}
	}

END:
	if (InstancePtr != NULL) {
		if ((Status != XASUFW_SUCCESS) && (InstancePtr->ErrorState != XTRNG_CATASTROPHIC)) {
			InstancePtr->ErrorState = XTRNG_ERROR;
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function uninstantiates the TRNG instance.
 *
 * @param   InstancePtr		Pointer to the TRNG instance.
 *
 * @return
 *          - XASUFW_SUCCESS if uninstantiation was successful.
 *          - XASUFW_TRNG_INVALID_PARAM if invalid instance is passed to function.
 *          - XASUFW_OSCILLATOR_DISABLE_FAILED If oscillator reseed source disable is failed.
 *
 *************************************************************************************************/
s32 XTrng_Uninstantiate(XTrng *InstancePtr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);

	if (InstancePtr == NULL) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/* Bring cores in to reset state */
	Status = XTrng_Reset(InstancePtr);
	if (Status != (s32)XASUFW_SUCCESS) {
		goto END;
	}

	/* Disable ring oscillators as a random seed source */
	Status = Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_OSC_EN_OFFSET,
				 XASU_TRNG_OSC_EN_VAL_MASK, XASU_TRNG_OSC_EN_VAL_DEFVAL);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OSCILLATOR_DISABLE_FAILED;
		XFIH_GOTO(END);
	}

	InstancePtr->State = XTRNG_UNINITIALIZED_STATE;
	InstancePtr->ErrorState = XTRNG_STARTUP_TEST;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function initialize and configures the TRNG into HRNG mode of operation.
 *
 * @param   InstancePtr		Pointer to the TRNG instance.
 * @param	Mode			HRNG or PTRNG mode input.
 *
 * @return
 *          - XASUFW_SUCCESS upon success.
 *          - Otherwise, it returns an error code.
 *
 *************************************************************************************************/
s32 XTrng_InitNCfgTrngMode(XTrng *InstancePtr, XTrng_Mode Mode)
{
	s32 Status = XASUFW_FAILURE;
	XTrng_UserConfig UsrCfg;

	if (InstancePtr->State != XTRNG_UNINITIALIZED_STATE) {
		Status = XTrng_Uninstantiate(InstancePtr);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
	}
	/* Initiate TRNG */
	UsrCfg.Mode = Mode;
	UsrCfg.AdaptPropTestCutoff = XTRNG_USER_CFG_ADAPT_TEST_CUTOFF;
	UsrCfg.RepCountTestCutoff = XTRNG_USER_CFG_REP_TEST_CUTOFF;
	UsrCfg.DFLength = XTRNG_USER_CFG_DF_LENGTH;
	UsrCfg.SeedLife = XTRNG_USER_CFG_SEED_LIFE;
	UsrCfg.IsBlocking = TRUE;

	Status = XTrng_Instantiate(InstancePtr, NULL, 0U, NULL, &UsrCfg);
	if (Status != XASUFW_SUCCESS) {
		(void)XTrng_Uninstantiate(InstancePtr);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function triggers and reseeds the DRBG module.
 *
 * @param	InstancePtr	Pointer to TRNG Instance.
 * @param	Seed		Pointer to the seed input.
 * @param	DLen		Seed length in multiples of TRNG block size.
 * @param	PerStr		Pointer to the personalization string.
 *
 * @return
 *          - XASUFW_SUCCESS On successful reseed.
 *          - XASUFW_TRNG_ERROR_WRITE On write failure.
 *          - XASUFW_TRNG_TIMEOUT_ERROR If timeout occurred waiting for done bit.
 *          - XASUFW_TRNG_CATASTROPHIC_CTF_ERROR If CTF bit asserted in STATUS register.
 *          - XASUFW_OSCILLATOR_ENABLE_FAILED If oscillator reseed source enable is failed.
 *          - XASUFW_ENABLE_PRNG_FOR_RESEED_FAILED If enabling PRNG for reseed failed.
 *          - XASUFW_START_RESEED_FAILED If reseed operation failed.
 *
 *************************************************************************************************/
static s32 XTrng_ReseedInternal(XTrng *InstancePtr, u8 *Seed, u8 DLen, u8 *PerStr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	u32 PersMask = XASU_TRNG_CTRL_PERSODISABLE_MASK;

	/* Configure given DF Len */
	XTrng_UpdateConf1(InstancePtr, DLen, InstancePtr->UserCfg.AdaptPropTestCutoff);

	if (PerStr != NULL) {
		Status = XASUFW_FAILURE;
		Status = XTrng_WritePersString(InstancePtr, PerStr);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
		PersMask = XASU_TRNG_CTRL_PERSODISABLE_DEFVAL;
	}

	Status = Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
				 XASU_TRNG_CTRL_PERSODISABLE_MASK | XASU_TRNG_CTRL_PRNGSTART_MASK, PersMask);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* DRNG Mode */
	if (Seed != NULL) {
		/* Enable TST mode and set PRNG mode for reseed operation*/
		Status = Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
					 XASU_TRNG_CTRL_PRNGMODE_MASK | XASU_TRNG_CTRL_TSTMODE_MASK |
					 XASU_TRNG_CTRL_TRSSEN_MASK, XASU_TRNG_CTRL_TSTMODE_MASK | XASU_TRNG_CTRL_TRSSEN_MASK);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_ENABLE_PRNG_FOR_RESEED_FAILED;
			XFIH_GOTO(END);
		}
		/* Start reseed operation */
		Status = Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
					 XASU_TRNG_CTRL_PRNGSTART_MASK, XASU_TRNG_CTRL_PRNGSTART_MASK);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_START_RESEED_FAILED;
			XFIH_GOTO(END);
		}

		/* For writing seed as an input to DF, PRNG start needs to be set */
		Status = XTrng_WriteSeed(InstancePtr, Seed, DLen);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
	} else { /* HTRNG Mode */
		/* Enable ring oscillators for random seed source */
		Status = Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_OSC_EN_OFFSET,
					 XASU_TRNG_OSC_EN_VAL_MASK, XASU_TRNG_OSC_EN_VAL_MASK);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_OSCILLATOR_ENABLE_FAILED;
			XFIH_GOTO(END);
		}

		/* Enable TRSSEN and set PRNG mode for reseed operation */
		Status = Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
					 XASU_TRNG_CTRL_PRNGMODE_MASK | XASU_TRNG_CTRL_TRSSEN_MASK | XASU_TRNG_CTRL_PRNGXS_MASK,
					 XASU_TRNG_CTRL_TRSSEN_MASK);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_ENABLE_PRNG_FOR_RESEED_FAILED;
			XFIH_GOTO(END);
		}

		/* Start reseed operation */
		Status = Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
					 XASU_TRNG_CTRL_PRNGSTART_MASK, XASU_TRNG_CTRL_PRNGSTART_MASK);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_START_RESEED_FAILED;
			XFIH_GOTO(END);
		}
	}
	/* Wait for reseed operation and check CTF flag */
	Status = XTrng_WaitForReseed(InstancePtr);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	InstancePtr->State = XTRNG_RESEED_STATE;
	Status = XASUFW_SUCCESS;
	InstancePtr->TrngStats.ElapsedSeedLife = 0U;

END:
	if ((Status != XASUFW_SUCCESS) && (InstancePtr->ErrorState != XTRNG_CATASTROPHIC)) {
		InstancePtr->ErrorState = XTRNG_ERROR;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function waits for the reseed done bit.
 *
 * @param   InstancePtr		Pointer to the TRNG instance.
 *
 * @return
 *          - XASUFW_SUCCESS if Random number collection was successful.
 *          - XASUFW_TRNG_TIMEOUT_ERROR if timeout occurred waiting for done bit.
 *          - XASUFW_TRNG_CATASTROPHIC_CTF_ERROR if CTF bit asserted in STATUS register.
 *
 *************************************************************************************************/
static s32 XTrng_WaitForReseed(XTrng *InstancePtr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);

	XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(XTrng_WaitForEvent, XASUFW_TRNG_TIMEOUT_ERROR, XFihVar, Status,
					   END, (UINTPTR)(InstancePtr->BaseAddress + XASU_TRNG_STATUS_OFFSET),
					   XASU_TRNG_STATUS_DONE_MASK, XASU_TRNG_STATUS_DONE_MASK, XTRNG_RESEED_TIMEOUT * 2);

	if (XASUFW_PLATFORM != PMC_TAP_VERSION_PLATFORM_PROTIUM) {
		if ((XAsufw_ReadReg(InstancePtr->BaseAddress + XASU_TRNG_STATUS_OFFSET) &
		     XASU_TRNG_STATUS_CERTF_MASK) == XASU_TRNG_STATUS_CERTF_MASK) {
			InstancePtr->ErrorState = XTRNG_CATASTROPHIC;
			Status = XASUFW_TRNG_CATASTROPHIC_CTF_ERROR;
			XFIH_GOTO(END);
		}
	}

	XAsufw_RMW(InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
		   XASU_TRNG_CTRL_PRNGSTART_MASK | XASU_TRNG_CTRL_TRSSEN_MASK, 0U);
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function triggers Generate operation and collects random data in buffer.
 *
 * @param	InstancePtr	Pointer to TRNG Instance.
 * @param	RandBuf		Pointer to the buffer in which random data is stored.
 * @param	RandBufSize	Buffer size in bytes.
 *
 * @return
 *          - XASUFW_SUCCESS if Random number collection was successful.
 *          - XASUFW_TRNG_TIMEOUT_ERROR if timeout occurred waiting for QCNT to become 4.
 *          - XASUFW_TRNG_CATASTROPHIC_DTF_ERROR if DTF bit asserted in STATUS register.
 *          - XASUFW_RANDOM_DATA_FAILED_TO_GENERATE If generate random data failed.
 *
 *************************************************************************************************/
static s32 XTrng_CollectRandData(XTrng *InstancePtr, u8 *RandBuf, u32 RandBufSize)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	s32 SStatus = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 Idx = 0U;
	u8 NumofBursts = 0U;
	u8 BurstIdx = 0U;
	u32 RegVal = 0U;
	u32 Size = RandBufSize / XASUFW_WORD_LEN_IN_BYTES;
	u32 SingleGenModeVal = 0U;

	if (InstancePtr->UserCfg.PredResistance == TRUE) {
		SingleGenModeVal = XASU_TRNG_CTRL_SINGLEGENMODE_MASK;
	}
	/* Set PRNG mode to generate */
	XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Xil_SecureRMW32, XASUFW_RANDOM_DATA_FAILED_TO_GENERATE,
					   XFihVar, Status, END, InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
					   XASU_TRNG_CTRL_PRNGMODE_MASK | XASU_TRNG_CTRL_SINGLEGENMODE_MASK |
					   XASU_TRNG_CTRL_PRNGSTART_MASK,
					   XASU_TRNG_CTRL_PRNGMODE_MASK | SingleGenModeVal);

	XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Xil_SecureRMW32, XASUFW_RANDOM_DATA_FAILED_TO_GENERATE,
					   XFihVar, Status, END, InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
					   XASU_TRNG_CTRL_PRNGSTART_MASK, XASU_TRNG_CTRL_PRNGSTART_MASK);

	for (NumofBursts = 0; NumofBursts < XTRNG_SEC_STRENGTH_IN_BURSTS; NumofBursts++) {
		XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(XTrng_WaitForEvent, XASUFW_TRNG_TIMEOUT_ERROR,
						   XFihVar, Status, END,
						   (UINTPTR)(InstancePtr->BaseAddress + XASU_TRNG_STATUS_OFFSET),
						   XASU_TRNG_STATUS_QCNT_MASK,
						   (XTRNG_STATUS_QCNT_VAL << XASU_TRNG_STATUS_QCNT_SHIFT), XTRNG_GENERATE_TIMEOUT);

		if ((XAsufw_ReadReg(InstancePtr->BaseAddress + XASU_TRNG_STATUS_OFFSET) &
		     XASU_TRNG_STATUS_DFT_MASK) == XASU_TRNG_STATUS_DFT_MASK) {
			InstancePtr->ErrorState = XTRNG_CATASTROPHIC;
			Status = XASUFW_TRNG_CATASTROPHIC_DTF_ERROR;
			XFIH_GOTO(END);
		}
		BurstIdx = NumofBursts * XTRNG_BURST_SIZE_IN_WORDS;
		for (Idx = 0U; Idx < XTRNG_BURST_SIZE_IN_WORDS; Idx++) {
			RegVal = XAsufw_ReadReg(InstancePtr->BaseAddress + XASU_TRNG_CORE_OUTPUT_OFFSET);
			if ((Idx + BurstIdx) < Size) {
				*((u32 *)RandBuf + Idx + BurstIdx) = Xil_EndianSwap32(RegVal);
			}
		}
	}
	if (NumofBursts == XTRNG_SEC_STRENGTH_IN_BURSTS) {
		Status = XASUFW_SUCCESS;
	}

END:
	if (Status != XASUFW_SUCCESS) {
		SStatus = Xil_SMemSet(RandBuf, RandBufSize, 0U, RandBufSize);
		if (SStatus != XASUFW_SUCCESS) {
			Status |= SStatus;
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function writes seed to the derivative function.
 *
 * @param	InstancePtr Pointer to TRNG Instance.
 * @param	Seed	Pointer to the seed input.
 * @param	DLen	Seed length in multiples of TRNG block size.
 *
 * @return
 *          - XASUFW_SUCCESS On successful write.
 *          - XASUFW_FAILURE On failure.
 *
 *************************************************************************************************/
static s32 XTrng_WriteSeed(XTrng *InstancePtr, u8 *Seed, u8 DLen)
{
	s32 Status = XASUFW_FAILURE;
	u32 SeedLen = (DLen + 1U) * XTRNG_BLOCK_LEN_IN_BYTES;
	volatile u32 Idx = 0U;
	u8 Cnt = 0U;
	u32 Bit = 0U;
	u8 SeedConstruct = 0U;

	while (Idx < SeedLen) {
		SeedConstruct = 0U;
		for (Cnt = 0; Cnt < XASUFW_BYTE_LEN_IN_BITS; Cnt++) {
			Bit = (u32)(Seed[Idx] >> (XASUFW_BYTE_LEN_IN_BITS - 1U - Cnt)) & 0x01U;
			XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_TRNG_TEST_OFFSET, Bit);
			SeedConstruct = (u8)((SeedConstruct << 1U) | (u8)Bit);
		}
		if (SeedConstruct != Seed[Idx]) {
			XFIH_GOTO(END);
		}
		usleep(XTRNG_DF_2CLKS_WAIT);
		if ((Idx % XTRNG_DF_NUM_OF_BYTES_BEFORE_MIN_700CLKS_WAIT) != 0U) {
			usleep(XTRNG_DF_700CLKS_WAIT);
		}
		Idx++;
	}
	if (Idx == SeedLen) {
		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function writes personalization string in to the registers.
 *
 * @param	InstancePtr		Pointer to TRNG Instance.
 * @param   PersString		Pointer to personalization string.
 *
 * @return
 *          - XASUFW_SUCCESS On successful write.
 *          - XASUFW_FAILURE On failure.
 *
 *************************************************************************************************/
static s32 XTrng_WritePersString(XTrng *InstancePtr, u8 *PersString)
{
	s32 Status = XASUFW_FAILURE;
	volatile u8 Idx = 0U;
	u8 Cnt = 0U;
	u32 RegVal = 0U;

	for (Idx = 0; Idx < XTRNG_PERS_STRING_LEN_IN_WORDS; Idx++) {
		RegVal = 0U;
		for (Cnt = 0; Cnt < XASUFW_WORD_LEN_IN_BYTES; Cnt++) {
			RegVal = (RegVal << XASUFW_BYTE_LEN_IN_BITS) |
				 PersString[(Idx * XASUFW_WORD_LEN_IN_BYTES) + Cnt];
		}
		XAsufw_WriteReg((InstancePtr->BaseAddress +
				 (XASU_TRNG_PER_STRNG_11_OFFSET - (Idx * XASUFW_WORD_LEN_IN_BYTES))), RegVal);
	}
	if (Idx == XTRNG_PERS_STRING_LEN_IN_WORDS) {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function waits for an event to occur for a specified duration.
 *
 * @param	Addr		Address of register to be checked for event(s) occurrence.
 * @param	EventMask	Mask indicating event(s) to be checked.
 * @param 	Event		Specific event(s) value to be checked.
 * @param	Timeout		Max number of microseconds to wait for an event(s).
 *
 * @return
 *          - XST_SUCCESS - On occurrence of the event(s).
 *          - XST_FAILURE - Event did not occur before counter reaches 0.
 *
 *************************************************************************************************/
static inline s32 XTrng_WaitForEvent(UINTPTR Addr, u32 EventMask, u32 Event, u32 Timeout)
{
	return (s32)Xil_WaitForEvent(Addr, EventMask, Event, Timeout);
}

/*************************************************************************************************/
/**
 * @brief   This function checks if random numbers are available in TRNG FIFO or not.
 *
 * @param   InstancePtr		Pointer to TRNG Instance.
 *
 * @return
 *          - XASUFW_SUCCESS If random numbers are available in TRNG FIFO.
 *          - XASUFW_TRNG_INVALID_PARAM If invalid parameter(s) passed to this function.
 *          - XASUFW_FAILURE If random numbers are not available in TRNG FIFO.
 *
 *************************************************************************************************/
s32 XTrng_IsRandomNumAvailable(XTrng *InstancePtr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	u32 RandomBytes;

	if (InstancePtr == NULL) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	RandomBytes = XAsufw_ReadReg(InstancePtr->BaseAddress + XASU_TRNG_NRN_AVAIL_OFFSET);
	if (RandomBytes >= XTRNG_SEC_STRENGTH_IN_WORDS) {
		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function reads the TRNG FIFO and provides random numbers to the requester.
 *
 * @param   InstancePtr		Pointer to TRNG Instance.
 * @param	OutputBuf		Output buffer holding 256-bit random number.
 * @param	OutputBufSize	Output buffer size.
 *
 * @return
 *          - XASUFW_SUCCESS On successfully reading the 256-bit random number from TRNG FIFO.
 *          - XASUFW_TRNG_INVALID_PARAM If invalid parameter(s) passed to this function.
 *          - XASUFW_TRNG_INVALID_STATE If TRNG is not in expected state.
 *          - XASUFW_TRNG_INVALID_BUF_SIZE If buffer is less that 256 bits or NULL.
 *          - XASUFW_TRNG_FIFO_IS_EMPTY If TRNG FIFO is empty.
 *
 *************************************************************************************************/
s32 XTrng_ReadTrngFifo(XTrng *InstancePtr, u32 *OutputBuf, u32 OutputBufSize)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	u32 *Buffer = OutputBuf;
	u32 Idx;

	if (InstancePtr == NULL) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if (InstancePtr->State != XTRNG_AUTOPROC_STATE) {
		Status = XASUFW_TRNG_INVALID_STATE;
		XFIH_GOTO(END);
	}

	if ((OutputBuf == NULL) || (OutputBufSize != XTRNG_SEC_STRENGTH_IN_BYTES)) {
		Status = XASUFW_TRNG_INVALID_BUF_SIZE;
		XFIH_GOTO(END);
	}

	for (Idx = 0U; Idx < XTRNG_SEC_STRENGTH_IN_WORDS; Idx++) {
		*Buffer = XAsufw_ReadReg(InstancePtr->TrngFifoAddr);
		Buffer++;
	}
	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function enables autoproc mode for TRNG.
 *
 * @param   InstancePtr	Pointer to TRNG Instance.
 *
 * @return
 *          - XASUFW_SUCCESS On successfully enabling autoproc mode.
 *          - XASUFW_TRNG_INVALID_PARAM If invalid parameter(s) passed to this function.
 *
 *************************************************************************************************/
s32 XTrng_EnableAutoProcMode(XTrng *InstancePtr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);

	if (InstancePtr == NULL) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_TRNG_NRNPS_OFFSET, XTRNG_AUTOPROC_NRNPS_VALUE);

	/* Enable autoproc mode */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_TRNG_AUTOPROC_OFFSET,
			XASU_TRNG_AUTOPROC_ENABLE_MASK);

	InstancePtr->State = XTRNG_AUTOPROC_STATE;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/** @} */