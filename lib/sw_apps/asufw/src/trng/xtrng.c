/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xtrng.c
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
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xtrng_server_apis TRNG Server APIs
* @{
*/
/*************************************** Include Files *******************************************/
#include "xtrng.h"
#include "xtrng_hw.h"
#include "xasufw_status.h"
#include "xasufw_hw.h"
#include "sleep.h"
#include "xil_util.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/
/** TBD: Need to update to proper timeout value after measurement */
#define XTRNG_RESEED_TIMEOUT			1500000U	/**< Reseed timeout in micro-seconds. */
#define XTRNG_GENERATE_TIMEOUT			1500000U	/**< Generate timeout in micro-seconds. */
#define XTRNG_BLOCK_LEN_IN_BYTES		16U	/**< TRNG block length length in bytes. */
#define XTRNG_MIN_SEEDLIFE				1U	/**< Minimum seed life. */
#define XTRNG_MAX_SEEDLIFE				0x80000U /**< Maximum seed life 2^19. */
#define XTRNG_SEC_STRENGTH_IN_BURSTS	2U	/**< Security strength in 128-bit bursts. */
#define XTRNG_BURST_SIZE_IN_WORDS		4U	/**< Burst size in words. */
#define XTRNG_DF_MIN_LENGTH				2U	/**< Minimum DF input length. */
#define XTRNG_DF_MAX_LENGTH				0x1FU	/**< Maximum DF input length. */
#define XTRNG_AUTOPROC_NRNPS_VALUE		0x3FU /**< Autoproc NRNPS value. */

#define XTRNG_DF_NUM_OF_BYTES_BEFORE_MIN_700CLKS_WAIT	8U
/**< Number of bytes to be written before wait. */

#define XTRNG_ADAPTPROPTESTCUTOFF_MAX_VAL	0x3FFU /**< Maximum adaptproptest cutoff value. */
#define XTRNG_REPCOUNTTESTCUTOFF_MAX_VAL	0x1FFU /**< Maximum repcounttest cutoff value. */
#define XTRNG_RESET_DELAY_US				10U /**< Reset delay. */
#define XTRNG_DF_700CLKS_WAIT				10U /**< Delay after 4bytes. */
#define XTRNG_DF_2CLKS_WAIT					4U /**< Delay after 1byte. */
#define XTRNG_STATUS_QCNT_VAL				4U /**< QCNT value for single burst. */

/************************************** Type Definitions *****************************************/
/**
* @brief This typedef contains configuration information for a TRNG core.
* Each core should have an associated configuration structure.
*/
typedef struct {
	u16 DeviceId; /**< DeviceId is the unique ID of the device */
	u32 BaseAddress; /**< BaseAddress is the physical base address of the device's registers */
	u32 TrngFifoAddr; /**< TRNG FIFO address */
} XTrng_Config;

/** @brief This typedef contains the seed life information. */
typedef struct {
	u32 ElapsedSeedLife; /**< Elapsed seed life */
} XTrng_Status;

/** This typedef is used to update the state of TRNG. */
typedef enum {
	XTRNG_UNINITIALIZED_STATE = 0, /**< Default state */
	XTRNG_INSTANTIATE_STATE, /**< Instantiate state */
	XTRNG_RESEED_STATE, /**< Reseed state */
	XTRNG_GENERATE_STATE, /**< Generate state */
	XTRNG_AUTOPROC_STATE /**< TRNG in autoproc */
} XTrng_State;

/**
* @brief TRNG driver instance structure. A pointer to an instance data structure is passed around
* by functions to refer to a specific driver instance.
*/
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
static s32 XTrng_ReseedInternal(XTrng *InstancePtr, const u8 *Seed, u8 DLen, const u8 *PerStr);
static s32 XTrng_WritePersString(const XTrng *InstancePtr, const u8 *PersString);
static s32 XTrng_WaitForReseed(XTrng *InstancePtr);
static s32 XTrng_CollectRandData(XTrng *InstancePtr, u8 *RandBuf, u32 RandBufSize);
static s32 XTrng_WriteSeed(const XTrng *InstancePtr, const u8 *Seed, u8 DLen);
static inline s32 XTrng_WaitForEvent(UINTPTR Addr, u32 EventMask, u32 Event, u32 Timeout);
static s32 XTrng_Set(const XTrng *InstancePtr);
static s32 XTrng_Reset(const XTrng *InstancePtr);
static s32 XTrng_PrngReset(const XTrng *InstancePtr);
static s32 XTrng_PrngSet(const XTrng *InstancePtr);
static void XTrng_UpdateConf0(const XTrng *InstancePtr, u32 DitVal, u32 RepCountTestCutoff);
static void XTrng_UpdateConf1(const XTrng *InstancePtr, u32 DFLen, u32 AdaptPropTestCutoff);

/************************************ Variable Definitions ***************************************/
/** The configuration table for TRNG devices */
static XTrng_Config XTrng_ConfigTable[XASU_XTRNG_NUM_INSTANCES] = {
	{
		XASU_XTRNG_0_DEVICE_ID,
		XASU_XTRNG_0_S_AXI_BASEADDR,
		XASU_XTRNG_0_FIFO_S_AXI_BASEADDR
	}
};

/** TRNG HW instances */
static XTrng XTrng_Instance[XASU_XTRNG_NUM_INSTANCES];

/*************************************************************************************************/
/**
 * @brief	This function returns TRNG instance pointer of the provided device ID.
 *
 * @param	DeviceId	The device ID of TRNG core.
 *
 * @return
 * 		- It returns pointer to the XTrng corresponding to the Device ID.
 * 		- It returns NULL if the device ID is invalid.
 *
 *************************************************************************************************/
XTrng *XTrng_GetInstance(u16 DeviceId)
{
	XTrng *XTrng_InstancePtr = NULL;

	if (DeviceId >= XASU_XTRNG_NUM_INSTANCES) {
		goto END;
	}

	XTrng_InstancePtr = &XTrng_Instance[DeviceId];
	XTrng_InstancePtr->DeviceId = DeviceId;

END:
	return XTrng_InstancePtr;
}

/*************************************************************************************************/
/**
 * @brief	This function returns a pointer reference of XTrng_Config structure based on the
 * 		device ID.
 *
 * @param	DeviceId	The device ID of TRNG core.
 *
 * @return
 * 		- CfgPtr, a reference to a config record in the configuration table
 * 			corresponding to <i>DeviceId</i>.
 * 		- NULL, if no valid device ID is found.
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
 * @brief	This function initializes the TRNG instance.
 *
 * @param	InstancePtr	Pointer to the TRNG instance.
 *
 * @return
*		- XASUFW_SUCCESS, if initialization is successful.
*		- XASUFW_TRNG_INVALID_PARAM, if InstancePtr or CfgPtr is NULL.
 *
 *************************************************************************************************/
s32 XTrng_CfgInitialize(XTrng *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XTrng_Config *CfgPtr;

	/** Validate input parameters. */
	if (InstancePtr == NULL) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		goto END;
	}

	CfgPtr = XTrng_LookupConfig(InstancePtr->DeviceId);
	if (CfgPtr == NULL) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		goto SET_ERR;
	}

	/** Initialize TRNG instance. */
	InstancePtr->BaseAddress = CfgPtr->BaseAddress;
	InstancePtr->TrngFifoAddr = CfgPtr->TrngFifoAddr;
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	InstancePtr->State = XTRNG_UNINITIALIZED_STATE;
	InstancePtr->ErrorState = XTRNG_STARTUP_TEST;
	Status = XASUFW_SUCCESS;

SET_ERR:
	if ((InstancePtr != NULL) && (Status != XASUFW_SUCCESS)) {
		InstancePtr->ErrorState = XTRNG_ERROR;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function brings TRNG core and PRNG unit out of reset.
 *
 * @param	InstancePtr	Pointer to the TRNG instance.
 *
 * @return
 * 	- XST_SUCCESS, On success
 * 	- XASUFW_FAILURE, On failure
 *
 *************************************************************************************************/
static s32 XTrng_Set(const XTrng *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	Status = XTrng_Reset(InstancePtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	usleep(XTRNG_RESET_DELAY_US);
	Status = Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_RESET_OFFSET,
				 XASU_TRNG_RESET_VAL_MASK, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/* Soft reset PRNG unit */
	Status = XTrng_PrngSet(InstancePtr);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sets TRNG core in to reset state.
 *
 * @param	InstancePtr	Pointer to the TRNG instance.
 *
 * @return
 * 	- XST_SUCCESS, On success
 * 	- XASUFW_FAILURE, On failure
 *
 *************************************************************************************************/
static s32 XTrng_Reset(const XTrng *InstancePtr)
{
	return Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_RESET_OFFSET,
				 XASU_TRNG_RESET_VAL_MASK, XASU_TRNG_RESET_DEFVAL);
}

/*************************************************************************************************/
/**
 * @brief	This function brings PRNG unit out of reset.
 *
 * @param	InstancePtr	Pointer to the TRNG instance.
 *
 * @return
 * 	- XST_SUCCESS, On success
 * 	- XASUFW_FAILURE, On failure
 *
 *************************************************************************************************/
static s32 XTrng_PrngSet(const XTrng *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	Status = XTrng_PrngReset(InstancePtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	usleep(XTRNG_RESET_DELAY_US);
	Status = Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
				 XASU_TRNG_CTRL_PRNGSRST_MASK, 0U);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sets PRNG unit in to reset state.
 *
 * @param   InstancePtr		Pointer to the TRNG Instance.
 *
 * @return
 * 	- XST_SUCCESS, On success
 * 	- XASUFW_FAILURE, On failure
 *
 *************************************************************************************************/
static s32 XTrng_PrngReset(const XTrng *InstancePtr)
{
	return Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
				 XASU_TRNG_CTRL_PRNGSRST_MASK, XASU_TRNG_CTRL_PRNGSRST_MASK);
}

/*************************************************************************************************/
/**
 * @brief	This function writes the DIT value and repetitive count test cutoff value to the
 * 		TRNG CONF0 register.
 *
 * @param	InstancePtr		Pointer to the TRNG Instance.
 * @param	DitVal			DIT value.
 * @param	RepCountTestCutoff	Cutoff value for repetitive count test.
 *
 *************************************************************************************************/
static void XTrng_UpdateConf0(const XTrng *InstancePtr, u32 DitVal, u32 RepCountTestCutoff)
{
	u32 Conf0Val = DitVal & XASU_TRNG_CONF0_DIT_MASK;

	Conf0Val |= ((RepCountTestCutoff << XASU_TRNG_CONF0_REPCOUNTTESTCUTOFF_SHIFT)
				& XASU_TRNG_CONF0_REPCOUNTTESTCUTOFF_MASK);

	XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_TRNG_CONF0_OFFSET, Conf0Val);
}

/*************************************************************************************************/
/**
 * @brief	This function writes DF length and adaptive count test cutoff value to the TRNG
 * 		CONF1 register.
 *
 * @param	InstancePtr		Pointer to the TRNG Instance.
 * @param	DFLen			Input DF length.
 * @param	AdaptPropTestCutoff	Cutoff value for adaptive count test.
 *
 *************************************************************************************************/
static void XTrng_UpdateConf1(const XTrng *InstancePtr, u32 DFLen, u32 AdaptPropTestCutoff)
{
	u32 Conf1Val = DFLen & XASU_TRNG_CONF1_DLEN_MASK;

	Conf1Val |= ((AdaptPropTestCutoff << XASU_TRNG_CONF1_ADAPTPROPTESTCUTOFF_SHIFT)
				& XASU_TRNG_CONF1_ADAPTPROPTESTCUTOFF_MASK);

	XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_TRNG_CONF1_OFFSET, Conf1Val);
}

/*************************************************************************************************/
/**
 * @brief	This function instantiates the TRNG instance with user configure values.
 *
 * @param	InstancePtr	Pointer to the TRNG Instance.
 * @param	Seed		Pointer to the seed input.
 * @param	SeedLength	Seed length in bytes.
 * @param	PersStr		Pointer to the personalization string input.
 * @param	UserCfg		Pointer to the XTrng_UserConfig structure.
 *
 * @return
 * 	- XASUFW_SUCCESS On successful instantation
 * 	- XASUFW_TRNG_INVALID_PARAM If invalid parameter(s) passed to this function.
 * 	- XASUFW_TRNG_INVALID_SEED_VALUE If provide seed is NULL in DRBG mode
 * 	- XASUFW_TRNG_INVALID_STATE If state is not in uninstantiate state
 * 	- XASUFW_TRNG_UNHEALTHY_STATE If TRNG KAT fails
 * 	- XASUFW_TRNG_INVALID_MODE  If invalid mode is passed to this function
 * 	- XASUFW_TRNG_INVALID_DF_LENGTH If invalid DF input length is passed as a function
 * 	- XASUFW_TRNG_INVALID_SEED_LENGTH If provide seed length doesn't match with given
 * 		DF length
 * 	- XASUFW_TRNG_INVALID_SEED_LIFE If invalid seed life is provided
 * 	- XASUFW_TRNG_INVALID_ADAPTPROPTEST_CUTOFF_VALUE If invalid cutoff value is provided
 * 	- XASUFW_TRNG_INVALID_REPCOUNTTEST_CUTOFF_VALUE If invalid repetitive test cutoff
 * 		value is provided
 * 	- XASUFW_TRNG_USER_CFG_COPY_ERROR If error occurred during copy of XTrng_UserConfig
 * 		structure
 * 	- XASUFW_INVALID_BLOCKING_MODE If invalid blocking mode
 * 	- XASUFW_FAILURE On unexpected failure
 *
 *************************************************************************************************/
s32 XTrng_Instantiate(XTrng *InstancePtr, const u8 *Seed, u32 SeedLength, const u8 *PersStr,
	const XTrng_UserConfig *UserCfg)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);

	/** Validate input parameters. */
	if ((UserCfg == NULL) || (InstancePtr == NULL)) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		goto END;
	}

	if ((Seed == NULL) && (UserCfg->Mode == XTRNG_DRBG_MODE)) {
		Status = XASUFW_TRNG_INVALID_SEED_VALUE;
		goto END;
	}

	if ((Seed != NULL) &&
	    ((UserCfg->Mode == XTRNG_HRNG_MODE) || (UserCfg->Mode == XTRNG_PTRNG_MODE))) {
		Status = XASUFW_TRNG_INVALID_SEED_VALUE;
		goto END;
	}

	if (InstancePtr->State != XTRNG_UNINITIALIZED_STATE) {
		Status = XASUFW_TRNG_INVALID_STATE;
		goto END;
	}

	if ((InstancePtr->ErrorState != XTRNG_HEALTHY) &&
	    (InstancePtr->ErrorState != XTRNG_STARTUP_TEST)) {
		Status = XASUFW_TRNG_UNHEALTHY_STATE;
		goto END;
	}

	if ((UserCfg->Mode != XTRNG_DRBG_MODE) &&
	    (UserCfg->Mode != XTRNG_PTRNG_MODE) &&
	    (UserCfg->Mode != XTRNG_HRNG_MODE)) {
		Status = XASUFW_TRNG_INVALID_MODE;
		goto END;
	}

	if ((UserCfg->DFLength  < XTRNG_DF_MIN_LENGTH) || (UserCfg->DFLength > XTRNG_DF_MAX_LENGTH)) {
		Status = XASUFW_TRNG_INVALID_DF_LENGTH;
		goto END;
	}

	if ((UserCfg->Mode == XTRNG_DRBG_MODE) &&
	    (SeedLength != ((UserCfg->DFLength + 1U) * XTRNG_BLOCK_LEN_IN_BYTES))) {
		Status = XASUFW_TRNG_INVALID_SEED_LENGTH;
		goto END;
	}

	if ((UserCfg->SeedLife < XTRNG_MIN_SEEDLIFE) || (UserCfg->SeedLife > XTRNG_MAX_SEEDLIFE)) {
		Status = XASUFW_TRNG_INVALID_SEED_LIFE;
		goto END;
	}

	if ((UserCfg->Mode != XTRNG_DRBG_MODE) && ((UserCfg->AdaptPropTestCutoff < 1U) ||
		(UserCfg->AdaptPropTestCutoff > XTRNG_ADAPTPROPTESTCUTOFF_MAX_VAL))) {
		Status = XASUFW_TRNG_INVALID_ADAPTPROPTEST_CUTOFF_VALUE;
		goto END;
	}

	if ((UserCfg->Mode != XTRNG_DRBG_MODE) && ((UserCfg->RepCountTestCutoff < 1U) ||
		(UserCfg->RepCountTestCutoff > XTRNG_REPCOUNTTESTCUTOFF_MAX_VAL))) {
		Status = XASUFW_TRNG_INVALID_REPCOUNTTEST_CUTOFF_VALUE;
		goto END;
	}

	if ((UserCfg->Mode != XTRNG_PTRNG_MODE) && (UserCfg->IsBlocking != XASU_TRUE) &&
	    (UserCfg->IsBlocking != XASU_FALSE)) {
		Status = XASUFW_INVALID_BLOCKING_MODE;
		goto END;
	}

	Status = Xil_SMemCpy(&InstancePtr->UserCfg, sizeof(XTrng_UserConfig), UserCfg,
			     sizeof(XTrng_UserConfig), sizeof(XTrng_UserConfig));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_TRNG_USER_CFG_COPY_ERROR;
		goto END;
	}

	/** Bring TRNG and PRNG unit core out of reset. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XTrng_Set(InstancePtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if ((UserCfg->Mode == XTRNG_PTRNG_MODE) || (UserCfg->Mode == XTRNG_HRNG_MODE)) {
		/**
		 * Configure DIT value and repetitive count test cutoff value if mode is
		 * PTRNG OR HRNG.
		 */
		XTrng_UpdateConf0(InstancePtr, XASU_TRNG_CONF0_DIT_DEFVAL, UserCfg->RepCountTestCutoff);
		/**
		 * Configure DF length and adaptive count test cutoff value if mode is
		 * PTRNG OR HRNG.
		 */
		XTrng_UpdateConf1(InstancePtr, UserCfg->DFLength, UserCfg->AdaptPropTestCutoff);
	}

	InstancePtr->State = XTRNG_INSTANTIATE_STATE;
	/** Do reseed operation when mode is DRBG/HRNG. */
	if ((UserCfg->Mode == XTRNG_DRBG_MODE) ||
	    (UserCfg->Mode == XTRNG_HRNG_MODE)) {
		XFIH_CALL_GOTO(XTrng_ReseedInternal, XFihVar, Status, END, InstancePtr, Seed,
					InstancePtr->UserCfg.DFLength, PersStr);
	}

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
 * @brief	This function triggers and reseeds the DRBG module.
 *
 * @param	InstancePtr	Pointer to the TRNG Instance.
 * @param	Seed		Pointer to the seed input.
 * @param	DLen		Seed length in TRNG block size.
 *
 * @return
 * 	- XASUFW_SUCCESS On successful reseed.
 * 	- XASUFW_TRNG_INVALID_PARAM If invalid parameter(s) passed to this function.
 * 	- XASUFW_TRNG_INVALID_SEED_VALUE If provide seed is NULL in DRBG/HRNG mode.
 * 	- XASUFW_TRNG_INVALID_MODE  If invalid mode is passed to this function.
 * 	- XASUFW_TRNG_INVALID_DF_LENGTH If invalid DF input length is passed as a function.
 * 	- XASUFW_TRNG_INVALID_STATE If state is not sequenced correctly.
 * 	- XASUFW_TRNG_UNHEALTHY_STATE If TRNG is in failure state, needs an
 * 		uninstantiation or KAT should be run if the error is catastrophic.
 * 	- XASUFW_FAILURE On unexpected failure.
 *
 *************************************************************************************************/
s32 XTrng_Reseed(XTrng *InstancePtr, const u8 *Seed, u8 DLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);

	/** Validate input parameters. */
	if (InstancePtr == NULL) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		goto END;
	}

	if ((InstancePtr->UserCfg.Mode == XTRNG_DRBG_MODE) && (Seed == NULL)) {
		Status = XASUFW_TRNG_INVALID_SEED_VALUE;
		goto END;
	}

	if ((Seed != NULL) && (InstancePtr->UserCfg.Mode == XTRNG_HRNG_MODE)) {
		Status = XASUFW_TRNG_INVALID_SEED_VALUE;
		goto END;
	}

	if ((DLen < XTRNG_DF_MIN_LENGTH) || (DLen > XTRNG_DF_MAX_LENGTH)) {
		Status = XASUFW_TRNG_INVALID_DF_LENGTH;
		goto END;
	}

	if (InstancePtr->UserCfg.Mode == XTRNG_PTRNG_MODE) {
		Status = XASUFW_TRNG_INVALID_MODE;
		goto END;
	}

	if (InstancePtr->State == XTRNG_UNINITIALIZED_STATE) {
		Status = XASUFW_TRNG_INVALID_STATE;
		goto END;
	}

	if ((InstancePtr->ErrorState != XTRNG_HEALTHY) &&
	    (InstancePtr->ErrorState != XTRNG_STARTUP_TEST)) {
		Status = XASUFW_TRNG_UNHEALTHY_STATE;
		goto END;
	}

	/** Wait for reseed operation and check CTF flag. */
	if ((InstancePtr->State == XTRNG_RESEED_STATE) && (InstancePtr->UserCfg.IsBlocking != XASU_TRUE)) {
		XFIH_CALL_GOTO(XTrng_WaitForReseed, XFihVar, Status, END, InstancePtr);
	}

	/** Do reseed operation. */
	XFIH_CALL_GOTO(XTrng_ReseedInternal, XFihVar, Status, END, InstancePtr, Seed, DLen, NULL);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates random data using TRNG and collects it into a buffer.
 *
 * @param	InstancePtr	Pointer to the TRNG Instance.
 * @param	RandBuf		Pointer to the buffer to store the generated random data.
 * @param	RandBufSize	Size of the buffer to store the random data in bytes.
 * @param	PredResistance	Flag that controls Generate level Prediction Resistance.
 *				When enabled, it mandates fresh seed for every Generate operation.
 *
 * @return
 * 	- XST_SUCCESS On successful generate of random number.
 * 	- XASUFW_TRNG_INVALID_PARAM If invalid parameter(s) passed to this function.
 * 	- XASUFW_TRNG_INVALID_STATE If state is not sequenced correctly.
 * 	- XASUFW_TRNG_INVALID_MODE  If invalid mode is passed to this function.
 * 	- XASUFW_INVALID_PREDRES_VALUE If pred resistance value is invalid.
 * 	- XASUFW_TRNG_INVALID_BUF_SIZE If buffer is less that 256 bytes or NULL.
 * 	- XASUFW_TRNG_UNHEALTHY_STATE If TRNG is in failure state, needs an uninstantiation
 * 		or KAT should be run if error is catastrophic.
 * 	- XASUFW_TRNG_RESEED_REQUIRED_ERROR If elapsed seed life exceeds the requested
 * 		seed life in DRBG mode.
 * 	- XASUFW_OSCILLATOR_ENABLE_FAILED If enabling oscilator fails.
 * 	- XASUFW_FAILURE On unexpected failure.
 *
 *************************************************************************************************/
s32 XTrng_Generate(XTrng *InstancePtr, u8 *RandBuf, u32 RandBufSize, u8 PredResistance)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);

	/** Validate input parameters. */
	if ((InstancePtr == NULL) || (RandBuf == NULL)) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		goto END;
	}

	if ((InstancePtr->UserCfg.Mode == XTRNG_PTRNG_MODE) &&
	    (InstancePtr->State != XTRNG_INSTANTIATE_STATE) &&
	    (InstancePtr->State != XTRNG_GENERATE_STATE)) {
		Status = XASUFW_TRNG_INVALID_STATE;
		goto END;
	}

	if ((InstancePtr->UserCfg.Mode != XTRNG_PTRNG_MODE) &&
	    (InstancePtr->State != XTRNG_RESEED_STATE) &&
	    (InstancePtr->State != XTRNG_GENERATE_STATE)) {
		Status = XASUFW_TRNG_INVALID_STATE;
		goto END;
	}

	if ((RandBufSize == 0U) || (RandBufSize > XTRNG_SEC_STRENGTH_IN_BYTES) ||
	    ((RandBufSize % XASUFW_WORD_LEN_IN_BYTES) != 0U)) {
		Status = XASUFW_TRNG_INVALID_BUF_SIZE;
		goto END;
	}

	if ((PredResistance != XASU_TRUE) && (PredResistance != XASU_FALSE)) {
		Status = XASUFW_INVALID_PREDRES_VALUE;
		goto END;
	}

	if ((InstancePtr->UserCfg.Mode == XTRNG_PTRNG_MODE) && (PredResistance == XASU_TRUE)) {
		Status = XASUFW_INVALID_PREDRES_VALUE;
		goto END;
	}

	if ((InstancePtr->ErrorState != XTRNG_HEALTHY) &&
	    (InstancePtr->ErrorState != XTRNG_STARTUP_TEST)) {
		Status = XASUFW_TRNG_UNHEALTHY_STATE;
		goto END;
	}

	if ((InstancePtr->UserCfg.Mode == XTRNG_DRBG_MODE) ||
	    (InstancePtr->UserCfg.Mode == XTRNG_HRNG_MODE)) {
		if (InstancePtr->UserCfg.Mode == XTRNG_DRBG_MODE) {
			if ((PredResistance == XASU_TRUE) &&
			    (InstancePtr->TrngStats.ElapsedSeedLife > 0U)) {
				Status = XASUFW_TRNG_RESEED_REQUIRED_ERROR;
				goto END;
			}
		}
		/** Wait for reseed operation and check CTF flag in DRBG and HRNG modes. */
		if ((InstancePtr->State == XTRNG_RESEED_STATE) && (InstancePtr->UserCfg.IsBlocking != XASU_TRUE)) {
			XFIH_CALL_GOTO(XTrng_WaitForReseed, XFihVar, Status, END, InstancePtr);
		}

		InstancePtr->UserCfg.PredResistance = PredResistance;
	} else if (InstancePtr->UserCfg.Mode == XTRNG_PTRNG_MODE) {
		/** Enable ring oscillators for random seed source in PTRNG mode. */
		XAsufw_RMW(InstancePtr->BaseAddress + XASU_TRNG_OSC_EN_OFFSET, XASU_TRNG_OSC_EN_VAL_MASK,
					XASU_TRNG_OSC_EN_VAL_MASK);

		XAsufw_RMW(InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
					XASU_TRNG_CTRL_TRSSEN_MASK | XASU_TRNG_CTRL_EUMODE_MASK |
					XASU_TRNG_CTRL_PRNGXS_MASK,
					XASU_TRNG_CTRL_TRSSEN_MASK | XASU_TRNG_CTRL_EUMODE_MASK);
	} else {
		Status = XASUFW_TRNG_INVALID_MODE;
		goto END;
	}

	/** Collect random data in a buffer. */
	XFIH_CALL_GOTO(XTrng_CollectRandData, XFihVar, Status, END, InstancePtr, RandBuf, RandBufSize);

	InstancePtr->TrngStats.ElapsedSeedLife++;
	InstancePtr->State = XTRNG_GENERATE_STATE;
	if (InstancePtr->UserCfg.Mode == XTRNG_HRNG_MODE) {
		/** Auto reseed in HRNG mode */
		if ((InstancePtr->TrngStats.ElapsedSeedLife >= InstancePtr->UserCfg.SeedLife) ||
		    (PredResistance == XASU_TRUE)) {
			XFIH_CALL_GOTO(XTrng_Reseed, XFihVar, Status, END, InstancePtr, NULL,
							InstancePtr->UserCfg.DFLength);
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
 * @brief	This function uninstantiates the TRNG instance.
 *
 * @param	InstancePtr		Pointer to the TRNG instance.
 *
 * @return
 * 	- XASUFW_SUCCESS if uninstantiation was successful.
 * 	- XASUFW_TRNG_INVALID_PARAM if invalid instance is passed to function.
 * 	- XASUFW_OSCILLATOR_DISABLE_FAILED If oscillator reseed source disable is failed.
 *
 *************************************************************************************************/
s32 XTrng_Uninstantiate(XTrng *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate input parameters. */
	if (InstancePtr == NULL) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		goto END;
	}

	/** Bring cores in to reset state. */
	Status = XTrng_Reset(InstancePtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Disable ring oscillators as a random seed source. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SecureRMW32(InstancePtr->BaseAddress + XASU_TRNG_OSC_EN_OFFSET,
				 XASU_TRNG_OSC_EN_VAL_MASK, XASU_TRNG_OSC_EN_VAL_DEFVAL);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OSCILLATOR_DISABLE_FAILED;
		goto END;
	}

	InstancePtr->State = XTRNG_UNINITIALIZED_STATE;
	InstancePtr->ErrorState = XTRNG_STARTUP_TEST;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function initializes and configures the TRNG into HRNG/PTRNG mode of
 * 		operation.
 *
 * @param	InstancePtr	Pointer to the TRNG instance.
 * @param	Mode		Mode input as either HRNG or PTRNG.
 *
 * @return
 * 	- XASUFW_SUCCESS upon success.
 * 	- XASUFW_FAILURE On unexpected failure.
 *
 *************************************************************************************************/
s32 XTrng_InitNCfgTrngMode(XTrng *InstancePtr, XTrng_Mode Mode)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XTrng_UserConfig UsrCfg;

	if (InstancePtr->State != XTRNG_UNINITIALIZED_STATE) {
		Status = XTrng_Uninstantiate(InstancePtr);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}
	/** Initiate TRNG. */
	UsrCfg.Mode = Mode;
	UsrCfg.AdaptPropTestCutoff = XTRNG_USER_CFG_ADAPT_TEST_CUTOFF;
	UsrCfg.RepCountTestCutoff = XTRNG_USER_CFG_REP_TEST_CUTOFF;
	UsrCfg.DFLength = XTRNG_USER_CFG_DF_LENGTH;
	UsrCfg.SeedLife = XTRNG_USER_CFG_SEED_LIFE;
	UsrCfg.IsBlocking = XASU_TRUE;

	Status = XTrng_Instantiate(InstancePtr, NULL, 0U, NULL, &UsrCfg);
	if (Status != XASUFW_SUCCESS) {
		(void)XTrng_Uninstantiate(InstancePtr);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function triggers and reseeds the DRBG module.
 *
 * @param	InstancePtr	Pointer to the TRNG Instance.
 * @param	Seed		Pointer to the seed input.
 * @param	DLen		Seed length in multiples of TRNG block size.
 * @param	PerStr		Pointer to the personalization string.
 *
 * @return
 * 	- XASUFW_SUCCESS On successful reseed.
 * 	- XASUFW_OSCILLATOR_ENABLE_FAILED If oscillator reseed source enable is failed.
 * 	- XASUFW_ENABLE_PRNG_FOR_RESEED_FAILED If enabling PRNG for reseed failed.
 * 	- XASUFW_START_RESEED_FAILED If reseed operation failed.
 *
 *************************************************************************************************/
static s32 XTrng_ReseedInternal(XTrng *InstancePtr, const u8 *Seed, u8 DLen, const u8 *PerStr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u32 PersMask = XASU_TRNG_CTRL_PERSODISABLE_MASK;

	/** Configure given DF Len. */
	XTrng_UpdateConf1(InstancePtr, DLen, InstancePtr->UserCfg.AdaptPropTestCutoff);

	if (PerStr != NULL) {
		Status = XTrng_WritePersString(InstancePtr, PerStr);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		PersMask = XASU_TRNG_CTRL_PERSODISABLE_DEFVAL;
	}

	XFIH_CALL_GOTO(Xil_SecureRMW32, XFihVar, Status, END,
				InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
				XASU_TRNG_CTRL_PERSODISABLE_MASK | XASU_TRNG_CTRL_PRNGSTART_MASK, PersMask);

	/** DRBG Mode */
	if (Seed != NULL) {
		/** Enable TST mode and set PRNG mode for reseed operation. */
		XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Xil_SecureRMW32, XASUFW_ENABLE_PRNG_FOR_RESEED_FAILED,
						XFihVar, Status, END, InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
						XASU_TRNG_CTRL_PRNGMODE_MASK | XASU_TRNG_CTRL_TSTMODE_MASK |
						XASU_TRNG_CTRL_TRSSEN_MASK,
						XASU_TRNG_CTRL_TSTMODE_MASK | XASU_TRNG_CTRL_TRSSEN_MASK);

		/** Start reseed operation. */
		XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Xil_SecureRMW32, XASUFW_START_RESEED_FAILED,
						XFihVar, Status, END, InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
						XASU_TRNG_CTRL_PRNGSTART_MASK, XASU_TRNG_CTRL_PRNGSTART_MASK);

		/** For writing seed as an input to DF, PRNG start needs to be set. */
		XFIH_CALL_GOTO(XTrng_WriteSeed, XFihVar, Status, END, InstancePtr, Seed, DLen);
	} else { /** HTRNG Mode. */
		/** Enable ring oscillators for random seed source. */
		XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Xil_SecureRMW32, XASUFW_OSCILLATOR_ENABLE_FAILED,
						XFihVar, Status, END, InstancePtr->BaseAddress + XASU_TRNG_OSC_EN_OFFSET,
						XASU_TRNG_OSC_EN_VAL_MASK, XASU_TRNG_OSC_EN_VAL_MASK);

		/** Enable TRSSEN and set PRNG mode for reseed operation. */
		XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Xil_SecureRMW32, XASUFW_ENABLE_PRNG_FOR_RESEED_FAILED,
						XFihVar, Status, END, InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
						XASU_TRNG_CTRL_PRNGMODE_MASK | XASU_TRNG_CTRL_TRSSEN_MASK |
						XASU_TRNG_CTRL_PRNGXS_MASK, XASU_TRNG_CTRL_TRSSEN_MASK);

		/** Start reseed operation. */
		XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Xil_SecureRMW32, XASUFW_START_RESEED_FAILED,
						XFihVar, Status, END, InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
						XASU_TRNG_CTRL_PRNGSTART_MASK, XASU_TRNG_CTRL_PRNGSTART_MASK);
	}
	/** Wait for reseed operation and check CTF flag. */
	XFIH_CALL_GOTO(XTrng_WaitForReseed, XFihVar, Status, END, InstancePtr);

	InstancePtr->State = XTRNG_RESEED_STATE;
	InstancePtr->TrngStats.ElapsedSeedLife = 0U;

END:
	if ((Status != XASUFW_SUCCESS) && (InstancePtr->ErrorState != XTRNG_CATASTROPHIC)) {
		InstancePtr->ErrorState = XTRNG_ERROR;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function waits for the reseed done bit.
 *
 * @param	InstancePtr	Pointer to the TRNG instance.
 *
 * @return
 * 	- XASUFW_SUCCESS if Random number collection was successful.
 * 	- XASUFW_TRNG_TIMEOUT_ERROR if timeout occurred waiting for done bit.
 * 	- XASUFW_TRNG_CATASTROPHIC_CTF_ERROR if CTF bit asserted in STATUS register.
 *
 *************************************************************************************************/
static s32 XTrng_WaitForReseed(XTrng *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);

	/** Check whether TRNG operation is completed within Timeout or not. */
	XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(XTrng_WaitForEvent, XASUFW_TRNG_TIMEOUT_ERROR, XFihVar, Status,
					   END, (UINTPTR)(InstancePtr->BaseAddress + XASU_TRNG_STATUS_OFFSET),
					   XASU_TRNG_STATUS_DONE_MASK, XASU_TRNG_STATUS_DONE_MASK, (XTRNG_RESEED_TIMEOUT * 2U));

	if (XASUFW_PLATFORM != PMC_TAP_VERSION_PLATFORM_PROTIUM) {
		if ((XAsufw_ReadReg(InstancePtr->BaseAddress + XASU_TRNG_STATUS_OFFSET) &
		     XASU_TRNG_STATUS_CERTF_MASK) == XASU_TRNG_STATUS_CERTF_MASK) {
			InstancePtr->ErrorState = XTRNG_CATASTROPHIC;
			Status = XASUFW_TRNG_CATASTROPHIC_CTF_ERROR;
			goto END;
		}
	}

	XAsufw_RMW(InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
		   XASU_TRNG_CTRL_PRNGSTART_MASK | XASU_TRNG_CTRL_TRSSEN_MASK, 0U);
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function triggers generate operation and collects random data in buffer.
 *
 * @param	InstancePtr	Pointer to the TRNG Instance.
 * @param	RandBuf		Pointer to the buffer to store the random data.
 * @param	RandBufSize	Size of the buffer in bytes.
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
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(SStatus, XASUFW_FAILURE);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 Idx = 0U;
	u8 NumofBursts = 0U;
	u8 BurstIdx = 0U;
	u32 RegVal = 0U;
	u32 Size = RandBufSize / XASUFW_WORD_LEN_IN_BYTES;
	u32 SingleGenModeVal = 0U;

	if (InstancePtr->UserCfg.PredResistance == XASU_TRUE) {
		SingleGenModeVal = XASU_TRNG_CTRL_SINGLEGENMODE_MASK;
	}
	/** Set PRNG mode to generate. */
	XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Xil_SecureRMW32, XASUFW_RANDOM_DATA_FAILED_TO_GENERATE,
					   XFihVar, Status, END, InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
					   XASU_TRNG_CTRL_PRNGMODE_MASK | XASU_TRNG_CTRL_SINGLEGENMODE_MASK |
					   XASU_TRNG_CTRL_PRNGSTART_MASK,
					   XASU_TRNG_CTRL_PRNGMODE_MASK | SingleGenModeVal);

	XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(Xil_SecureRMW32, XASUFW_RANDOM_DATA_FAILED_TO_GENERATE,
					   XFihVar, Status, END, InstancePtr->BaseAddress + XASU_TRNG_CTRL_OFFSET,
					   XASU_TRNG_CTRL_PRNGSTART_MASK, XASU_TRNG_CTRL_PRNGSTART_MASK);

	for (NumofBursts = 0; NumofBursts < XTRNG_SEC_STRENGTH_IN_BURSTS; NumofBursts++) {
		/** Check whether TRNG operation is completed within Timeout or not. */
		XFIH_CALL_GOTO_WITH_SPECIFIC_ERROR(XTrng_WaitForEvent, XASUFW_TRNG_TIMEOUT_ERROR,
						   XFihVar, Status, END,
						   (UINTPTR)(InstancePtr->BaseAddress + XASU_TRNG_STATUS_OFFSET),
						   XASU_TRNG_STATUS_QCNT_MASK,
						   (XTRNG_STATUS_QCNT_VAL << XASU_TRNG_STATUS_QCNT_SHIFT), XTRNG_GENERATE_TIMEOUT);

		if ((XAsufw_ReadReg(InstancePtr->BaseAddress + XASU_TRNG_STATUS_OFFSET) &
		     XASU_TRNG_STATUS_DFT_MASK) == XASU_TRNG_STATUS_DFT_MASK) {
			InstancePtr->ErrorState = XTRNG_CATASTROPHIC;
			Status = XASUFW_TRNG_CATASTROPHIC_DTF_ERROR;
			goto END;
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
 * @brief	This function writes seed to the derivative function.
 *
 * @param	InstancePtr	Pointer to the TRNG Instance.
 * @param	Seed		Pointer to the seed input.
 * @param	DLen		Seed length in multiples of TRNG block size.
 *
 * @return
 * 	- XASUFW_SUCCESS On successful write.
 * 	- XASUFW_FAILURE On failure.
 *
 *************************************************************************************************/
static s32 XTrng_WriteSeed(const XTrng *InstancePtr, const u8 *Seed, u8 DLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
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
			goto END;
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
 * @brief	This function writes personalization string in to the registers.
 *
 * @param	InstancePtr	Pointer to the TRNG Instance.
 * @param	PersString	Pointer to the personalization string.
 *
 * @return
 *          - XASUFW_SUCCESS On successful write.
 *          - XASUFW_FAILURE On failure.
 *
 *************************************************************************************************/
static s32 XTrng_WritePersString(const XTrng *InstancePtr, const u8 *PersString)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
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
 * @brief	This function waits for an event to occur for a specified duration.
 *
 * @param	Addr		Address of register to be checked for event(s) occurrence.
 * @param	EventMask	Mask indicating event(s) to be checked.
 * @param	Event		Specific event(s) value to be checked.
 * @param	Timeout		Max number of microseconds to wait for an event(s).
 *
 * @return
 * 	- XST_SUCCESS - On occurrence of the event(s).
 * 	- XST_FAILURE - upon timeout.
 *
 *************************************************************************************************/
static inline s32 XTrng_WaitForEvent(UINTPTR Addr, u32 EventMask, u32 Event, u32 Timeout)
{
	return (s32)Xil_WaitForEvent(Addr, EventMask, Event, Timeout);
}

/*************************************************************************************************/
/**
 * @brief	This function checks if random numbers are available in TRNG FIFO or not.
 *
 * @param	InstancePtr	Pointer to the TRNG Instance.
 *
 * @return
 * 	- XASUFW_SUCCESS If random numbers are available in TRNG FIFO.
 * 	- XASUFW_TRNG_INVALID_PARAM If invalid parameter(s) passed to this function.
 * 	- XASUFW_FAILURE If random numbers are not available in TRNG FIFO.
 *
 *************************************************************************************************/
s32 XTrng_IsRandomNumAvailable(const XTrng *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 RandomBytes;

	/** Validate input parameters. */
	if (InstancePtr == NULL) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		goto END;
	}

	/** Check whether random number is available or not by reading NRN_AVAIL register. */
	RandomBytes = XAsufw_ReadReg(InstancePtr->BaseAddress + XASU_TRNG_NRN_AVAIL_OFFSET);
	if (RandomBytes >= XTRNG_SEC_STRENGTH_IN_WORDS) {
		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function reads the TRNG FIFO and provides random numbers to the requester.
 *
 * @param	InstancePtr	Pointer to the TRNG Instance.
 * @param	OutputBuf	Pointer to the output buffer holding 256-bit random number.
 * @param	OutputBufSize	Size of the output buffer in bytes.
 *
 * @return
 * 	- XASUFW_SUCCESS On successfully reading the 256-bit random number from TRNG FIFO.
 * 	- XASUFW_TRNG_INVALID_PARAM If invalid parameter(s) passed to this function.
 * 	- XASUFW_TRNG_INVALID_STATE If TRNG is not in expected state.
 * 	- XASUFW_TRNG_INVALID_BUF_SIZE If buffer is less that 256 bits or NULL.
 *
 *************************************************************************************************/
s32 XTrng_ReadTrngFifo(const XTrng *InstancePtr, u32 *OutputBuf, u32 OutputBufSize)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 *Buffer = OutputBuf;
	u32 Idx;

	/** Validate input parameters. */
	if (InstancePtr == NULL) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->State != XTRNG_AUTOPROC_STATE) {
		Status = XASUFW_TRNG_INVALID_STATE;
		goto END;
	}

	if ((OutputBuf == NULL) || (OutputBufSize != XTRNG_SEC_STRENGTH_IN_BYTES)) {
		Status = XASUFW_TRNG_INVALID_BUF_SIZE;
		goto END;
	}

	/** Read FIFO and update in buffer. */
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
 * @brief	This function enables autoproc mode for TRNG.
 *
 * @param	InstancePtr	Pointer to the TRNG Instance.
 *
 * @return
 * 	- XASUFW_SUCCESS On successfully enabling autoproc mode.
 * 	- XASUFW_TRNG_INVALID_PARAM If invalid parameter(s) passed to this function.
 *
 *************************************************************************************************/
s32 XTrng_EnableAutoProcMode(XTrng *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate input parameters. */
	if (InstancePtr == NULL) {
		Status = XASUFW_TRNG_INVALID_PARAM;
		goto END;
	}

	XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_TRNG_NRNPS_OFFSET, XTRNG_AUTOPROC_NRNPS_VALUE);

	/** Enable autoproc mode. */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_TRNG_AUTOPROC_OFFSET,
			XASU_TRNG_AUTOPROC_ENABLE_MASK);

	InstancePtr->State = XTRNG_AUTOPROC_STATE;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/** @} */
