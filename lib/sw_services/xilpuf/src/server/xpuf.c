/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf.c
*
* This file contains PUF hardware interface API definitions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   kal  08/01/2019 Initial release
*       har  09/24/2019 Fixed MISRA-C violations
* 1.1   har  01/27/2020 Added support for on-demand regeneration from efuse cache,
*                       ID only regeneration and XPuf_Validate_Access_Rules
* 1.2   har  07/03/2020 Renamed XPUF_ID_LENGTH macro as XPUF_ID_LEN_IN_WORDS
*       am   08/04/2020 Resolved MISRA C Violations
*       am   08/19/2020 Resolved MISRA C violations.
*       har  09/30/2020 Updated XPUF_STATUS_WAIT_TIMEOUT as per the recommended
*                       software timeout
*       am   10/10/2020 Resolved MISRA C violations
*       har  10/17/2020 Added checks for mismatch between MSB of PUF shutter
*                       value and Global Variation Filter option
* 1.3   am   11/23/2020 Resolved MISRA C violation Rule 10.6
*       har  01/06/2021 Added API to clear PUF ID
*       am   01/14/2021 Resolved HIS_CCM Code complexity violations
*       har  02/03/2021 Improved input validation check in XPuf_Regeneration
*       har  02/12/2021 Replaced while loop in PUF registration with for loop
*       har  02/12/2021 Added a redundancy check for MaxSyndromeSizeInWords to
*                       avoid potential buffer overflow
*       har  03/08/2021 Added checks for IRO frequency
*       har  05/03/2021 Restructured code in XPuf_StartRegeneration to remove
*                       repeated code
*       am   05/18/2021 Resolved MISRA C violations
* 1.4   har  07/09/2021 Fixed Doxygen warnings
*       har  08/12/2021 Added comment related to IRO frequency
* 1.5   kpt  12/02/2021 Replaced standard library utility functions with
*                       xilinx maintained functions
*       har  01/20/2022 Removed inclusion of xil_mem.h
*       am   02/18/2022 Fixed COMF code complexity violations
*       har  03/21/2022 Disabled SLV_ERR for PUF on demand regeneration
*       kpt  03/23/2022 Added code to change IRO frequency to 320MHZ when IRO frequency
*                       is 400MHZ to make PUF work at nominal voltage
* 2.0   har  06/09/2022 Added support for Versal_net
*                       Removed support for 12K mode
*            08/02/2022 Modified if check for XPuf_ChangeIroFreq to avoid returning XST_SUCCESS
*                       incase of glitch attack
* 2.1   skg  10/29/2022 Added In Body comments for APIs
*       am   02/13/2023 Fixed MISRA C violations
*       am   02/17/2023 Fixed HIS_COMF violation
*       vss  02/21/2023 Fixed PUF aux shift issue
* 2.2	kpt  08/03/2023 Fix passing efuse cache value and changed XPuf_IsRegistrationEnabled to
*                       XPuf_IsRegistrationDisabled
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "sleep.h"
#include "xpuf.h"
#include "xpuf_hw.h"
#include "xil_util.h"
#include "xil_io.h"
#include "xil_printf.h"
#include "xpuf_plat.h"

/************************** Constant Definitions *****************************/
#define XPUF_STATUS_WAIT_TIMEOUT		(1000000U)
				/**< Recommended software timeout is 1 second */
#define XPUF_AUX_MASK_VALUE                     (0x0FFFFFF0U)
				/**< Mask value for AUX*/
#define XPUF_AUX_SHIFT_VALUE 			(4U)
				/**< No of bits aux has to shift*/

/********************Macros (Inline function) Definitions*********************/
/*****************************************************************************/
/**
 * @brief	This function waits till Puf Syndrome ready bit is set.
 *
 * @return
 *			 - XST_SUCCESS  Syndrome word is ready.
 *			 - XST_FAILURE  Timeout occurred.
 *
 *****************************************************************************/
static inline int XPuf_WaitForPufSynWordRdy(void)
{
	return (int)Xil_WaitForEvent((UINTPTR)(XPUF_PMC_GLOBAL_BASEADDR +
		XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET),
		XPUF_STATUS_SYNDROME_WORD_RDY, XPUF_STATUS_SYNDROME_WORD_RDY,
		XPUF_STATUS_WAIT_TIMEOUT);
}

/*****************************************************************************/
/**
 * @brief	This function waits till Puf done bit is set.
 *
 * @return
 *			 - XST_SUCCESS  Puf Operation is done.
 *			 - XST_FAILURE  Timeout occurred.
 *
 *****************************************************************************/
static inline int XPuf_WaitForPufDoneStatus(void)
{
	return (int)Xil_WaitForEvent((UINTPTR)(XPUF_PMC_GLOBAL_BASEADDR +
		XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET), XPUF_STATUS_PUF_DONE,
		XPUF_STATUS_PUF_DONE, XPUF_STATUS_WAIT_TIMEOUT);
}

/*****************************************************************************/
/**
 * @brief       This function configures the Global Variation Filter option provided
 *              by user and updates Puf Cfg0 register
 *
 * @param       GlobalVarFilter - User configuration to enable/disable
 *                                Global Variation Filter in PUF
 *
 *****************************************************************************/
static inline void XPuf_CfgGlobalVariationFilter(const u8 GlobalVarFilter)
{
	if (GlobalVarFilter == TRUE) {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CFG0_OFFSET,
			(XPUF_CFG0_HASH_SEL | XPUF_CFG0_GLOBAL_FILTER_ENABLE));
	}
	else {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CFG0_OFFSET,
			XPUF_CFG0_HASH_SEL);
	}
}

/*****************************************************************************/
/**
 *
 * @brief	This function reads the IRO frequency value from register
 *
 * @return	The 32-bit value of the IRO frequency register
 *
 *****************************************************************************/
static inline u8 XPuf_ReadIroFreq(void)
{
	return (u8)(XPuf_ReadReg(XPUF_EFUSE_CTRL_BASEADDR,
		XPUF_ANLG_OSC_SW_1LP_OFFSET) & XPUF_IRO_TRIM_FUSE_SEL_BIT);
}

/*****************************************************************************/
/**
 *
 * @brief       This function writes the IRO frequency value in to register
 *
 * @param       IroFreq IRO frequency to be set.
 *
 *****************************************************************************/
static inline void XPuf_WriteIroFreq(u32 IroFreq)
{
	XPuf_WriteReg(XPUF_EFUSE_CTRL_BASEADDR, XPUF_EFUSE_CTRL_WR_LOCK_OFFSET,
		XPUF_EFUSE_CTRL_WR_UNLOCK_VAL);

	Xil_UtilRMW32((XPUF_EFUSE_CTRL_BASEADDR + XPUF_ANLG_OSC_SW_1LP_OFFSET),
			XPUF_IRO_TRIM_FUSE_SEL_BIT, IroFreq);

	XPuf_WriteReg(XPUF_EFUSE_CTRL_BASEADDR, XPUF_EFUSE_CTRL_WR_LOCK_OFFSET,
		XPUF_EFUSE_CTRL_WR_LOCK_VAL);
}

/************************** Function Prototypes ******************************/
static void XPuf_CapturePufID(XPuf_Data *PufData);
static int XPuf_ValidateAccessRules(const XPuf_Data *PufData);
static int XPuf_UpdateHelperData(const XPuf_Data *PufData);
static int XPuf_StartRegeneration(XPuf_Data *PufData);
static int XPuf_ChangeIroFreq(u32 IroFreq, u8 *IroFreqUpdated);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This functions performs PUF registration
 *
 * @param	PufData - Pointer to XPuf_Data structure which includes options
 *                        to configure PUF
 *
 * @return
 *			 - XST_SUCCESS  PUF registration successful
.*			 - XPUF_ERROR_INVALID_PARAM  PufData is NULL
.*			 - XPUF_ERROR_INVALID_SYNDROME_MODE  Incorrect Registration mode
.*			 - XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT  Timeout occurred while
 *                                                  waiting for PUF Syndrome data
.*			 - XPUF_ERROR_PUF_DONE_WAIT_TIMEOUT  Timeout occurred while
 *                                                  waiting for PUF done bit
 *                                                  at the time of PUF registration
.*			 - XPUF_IRO_FREQ_WRITE_MISMATCH  Mismatch in writing or reading
 *                                                  IRO frequency
.*			 - XST_FAILURE  Unexpected event
 *
 * @note	Helper data will be available in PufData->SyndromeData,
 *		PufData->Chash, PufData->Aux
 *		PUF is only supported when using a nominal VCC_PMC of 0.70V or
 *		IRO frequency of 320 MHz
 *
 *****************************************************************************/
int XPuf_Registration(XPuf_Data *PufData)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u32 Idx = 0U;
	u8 IroFreqUpdated = FALSE;

	/**
	 * perform input parameters validation,
	 * return XPUF_ERROR_INVALID_PARAM if input parameters are invalid.
	 */
	if (PufData == NULL) {
		Status = XPUF_ERROR_INVALID_PARAM;
		goto END;
	}

	/**
	 * Check that MSB of PUF shutter value is in sync with Global variation filter option.
	 * In case of mismatch, return XPUF_SHUTTER_GVF_MISMATCH.
	 */
	Status = XPuf_CheckGlobalVariationFilter(PufData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * When registering the PUF, the PMC internal ring oscillator (IRO) frequency must be set
	 * to 320 MHz. When the Versal ACAP boots, it always uses the default frequency of
	 * 320 MHz for -LP devices and 400 MHZ for -MP,-HP devices. If the IRO frequency at boot
	 * does not match the IRO frequency during registration, there is a potential of reduced
	 * stability which can impact the PUFs ability to regenerate properly.
	 */
	if (XPuf_IsIroFreqChangeReqd() == XPUF_IROFREQ_CHANGE_REQD) {
		Status = XPuf_ChangeIroFreq(XPUF_IRO_FREQ_320MHZ, &IroFreqUpdated);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/**
	 * Validate if secure control bits for each PUF operations are
	 * set or not.
	 */
	Status =  XPuf_ValidateAccessRules(PufData);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Check GlobalVariationFilter user option. If TRUE then set GLBL_FILTER and HASH_SEL bits
	 * else, set only HASH_SEL bit in in PUF_CFG0 register.
	 */
	XPuf_CfgGlobalVariationFilter(PufData->GlobalVarFilter);

	/**
	 * Update PUF Configuration1 register as 4k Registration mode.
	 */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CFG1_OFFSET,
		XPUF_CFG1_INIT_VAL_4K);

	/**
	 * Update shutter value for PUF registration.
	 */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_SHUT_OFFSET,
		PufData->ShutterValue);

	/**
	 * Update PUF Ring Oscillator Swap setting for Versal Net device,
	 * do nothing in case of Versal.
	 */
	XPuf_SetRoSwap(PufData);

	/**
	 * Trigger PUF registration.
	 */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CMD_OFFSET,
		XPUF_CMD_REGISTRATION);

	Status = XST_FAILURE;

	/**
	 * PUF helper data includes Syndrome data, CHash and Auxillary data.
	 * Capturing Syndrome data word by word.
	 */
	for (Idx = 0; Idx < XPUF_4K_PUF_SYN_LEN_IN_WORDS; Idx++) {
		Status = XPuf_WaitForPufSynWordRdy();
		if (Status != XST_SUCCESS) {
			Status = XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT;
			goto END;
		}
		PufData->SyndromeData[Idx] = XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_WORD_OFFSET);
	}

	/**
	 * Once complete Syndrome data is captured and PUF operation is done,
	 * read CHash, Auxillary data and PUF ID.
	 */
	if (Idx == XPUF_4K_PUF_SYN_LEN_IN_WORDS) {
		Status = XST_FAILURE;
		Status  = XPuf_WaitForPufDoneStatus();
		if (Status != XST_SUCCESS) {
			Status = XPUF_ERROR_PUF_DONE_WAIT_TIMEOUT;
			goto END;
		}
		PufData->Chash = XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_CHASH_OFFSET);
		PufData->Aux = (XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_AUX_OFFSET) & XPUF_AUX_MASK_VALUE) >> XPUF_AUX_SHIFT_VALUE;
		XPuf_CapturePufID(PufData);
	}
	else {
		Status = XPUF_ERROR_SYN_DATA_ERROR;
		goto END;
	}

END:
	/**
	 * If IRO frequency is updated to 320MHZ then set frequency back to 400MHZ and
	 * if the frequency is not set then return XPUF_IRO_FREQ_WRITE_MISMATCH.
	 * else, return Status.
	 */
	if (IroFreqUpdated == TRUE) {
		StatusTmp = XPuf_ChangeIroFreq(XPUF_IRO_FREQ_400MHZ, &IroFreqUpdated);
		if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
			Status = StatusTmp;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function regenerate PUF data using helper data stored in eFUSE
 *              or external memory
 *
 * @param       PufData - Pointer to XPuf_Data structure which includes options
 *                        to configure PUF
 *
 * @return
.*			 - XST_SUCCESS  PUF Regeneration successful
.*			 - XPUF_ERROR_INVALID_PARAM  PufData is NULL
.*			 - XPUF_ERROR_INVALID_REGENERATION_TYPE  Selection of invalid
 *                                                     regeneration type
.*			 - XPUF_ERROR_CHASH_NOT_PROGRAMMED  Helper data not provided
.*			 - XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT  Timeout occurred while
 *                                                     waiting for PUF done bit
.*			 - XPUF_ERROR_PUF_DONE_KEY_ID_NT_RDY  Key ready bit and ID ready
 *                                                     bit is not set
.*			 - XPUF_ERROR_PUF_DONE_ID_NT_RDY  Id ready bit is not set
.*			 - XPUF_IRO_FREQ_WRITE_MISMATCH  Mismatch in writing or reading
 *                                                     IRO frequency
 *                                                     at the time of PUF regeneration
.*			 - XST_FAILURE  Unexpected event
 *
 * @note	PUF is only supported when using a nominal VCC_PMC of 0.70V or
 *		IRO frequency of 320 MHz
 *
 *****************************************************************************/
int XPuf_Regeneration(XPuf_Data *PufData)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u32 GlobalCntrlVal;
	u32 Reset = FALSE;
	u8 IroFreqUpdated = FALSE;

	/**
	 * Perform input parameters validation,
	 * return XPUF_ERROR_INVALID_PARAM if input parameters are invalid.
	 */
	if (PufData == NULL) {
		Status = XPUF_ERROR_INVALID_PARAM;
		goto END;
	}

	/**
	 * Check if requested PUF operation is not PUF on demand regeneration or ID only regeneration.
	 * If yes, return XPUF_ERROR_INVALID_PARAM.
	 */
	if ((PufData->PufOperation != XPUF_REGEN_ON_DEMAND) &&
		(PufData->PufOperation != XPUF_REGEN_ID_ONLY)) {
		Status = XPUF_ERROR_INVALID_PARAM;
		goto END;
	}

	/**
	 * Check that MSB of PUF shutter value is in sync with Global variation filter option.
	 * In case of mismatch, return XPUF_SHUTTER_GVF_MISMATCH.
	 */
	Status = XPuf_CheckGlobalVariationFilter(PufData);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	/**
	 * When registering the PUF, the PMC internal ring oscillator (IRO) frequency must be set
	 * to 320 MHz. When the Versal ACAP boots, it always uses the default frequency of
	 * 320 MHz for -LP devices and 400 MHZ for -MP,-HP devices. If the IRO frequency at boot
	 * does not match the IRO frequency during registration, there is a potential of reduced
	 * stability which can impact the PUFs ability to regenerate properly.
	 */
	if (XPuf_IsIroFreqChangeReqd() == XPUF_IROFREQ_CHANGE_REQD) {
		Status = XPuf_ChangeIroFreq(XPUF_IRO_FREQ_320MHZ, &IroFreqUpdated);
		if (Status != XST_SUCCESS) {
			XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
			goto END;
		}
	}

	/**
	 * Validate the access rules for PUF regeneration. If PUF_DIS or PUF_REGEN_DIS eFuse bits are set, then return XPUF_ERROR_REGENERATION_INVALID.
	 * If read from eFuse cache is selected and PUF_HD_INVLD bit is set then return XPUF_ERROR_REGEN_PUF_HD_INVALID.
	 * If read from eFuse cache and 12K syndrome mode is selected, then return XPUF_ERROR_INVALID_SYNDROME_MODE.
	 */
	Status =  XPuf_ValidateAccessRules(PufData);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Status = XST_FAILURE;

	/**
	 * Update the helper data for PUF regeneration according to the
	 * location of helper data provided by the user.
	 */
	Status = XPuf_UpdateHelperData(PufData);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	/**
	 * Configures the Global Variation Filter option provided by user and
	 * updates PUF Configuration0 register.
	 */
	XPuf_CfgGlobalVariationFilter(PufData->GlobalVarFilter);

	/**
	 * Update PUF Configuration1 register as 4k Registration mode.
	 */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CFG1_OFFSET,
		XPUF_CFG1_INIT_VAL_4K);

	/**
	 * Update Shutter value in PUF_SHUT register.
	 */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_SHUT_OFFSET,
		PufData->ShutterValue);

	GlobalCntrlVal = XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
                XPUF_PMC_GLOBAL_GLOBAL_CNTRL_OFFSET);

	/**
	 * Checks if SLVERR is enabled for PMC_GLOBAL, then
	 * disable the SLVERR for PMC_GLOBAL before regeneration starts and
	 * enables it again once PUF regeneration is done.
	 */
	if ((GlobalCntrlVal & XPUF_SLVERR_ENABLE_MASK) == XPUF_SLVERR_ENABLE_MASK) {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_GLOBAL_CNTRL_OFFSET,
			GlobalCntrlVal & ~(XPUF_SLVERR_ENABLE_MASK));
		Reset = TRUE;
	}

	/**
	 * Update PUF Ring Oscillator Swap setting for Versal Net device,
	 * do nothing in case of Versal.
	 */
	XPuf_SetRoSwap(PufData);

	Status = XST_FAILURE;

	/**
	 * If On demand regeneration is selected by user then trigger PUF_CMD for PUF On Demand regeneration.
	 * If ID only regeneration is selected then trigger PUF_CMD for PUF ID only regeneration.
	 * If invalid input, return XPUF_ERROR_INVALID_REGENERATION_TYPE.
	 */
	Status = XPuf_StartRegeneration(PufData);

	/**
	 * Enabling the SLVERR after regeneration, if SLVERR is enabled previously in PMC_GLOBAL.
	 */
	if (Reset == TRUE) {
		GlobalCntrlVal = XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_GLOBAL_CNTRL_OFFSET);
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_GLOBAL_CNTRL_OFFSET,
			GlobalCntrlVal | XPUF_SLVERR_ENABLE_MASK);
	}

END:
	/**
	 * If IRO frequency is updated to 320MHZ then set frequency back to 400MHZ and if the frequency
	 * is not set then return XPUF_IRO_FREQ_WRITE_MISMATCH else,
	 * Return status.
	 */
	if (IroFreqUpdated == TRUE) {
		StatusTmp = XPuf_ChangeIroFreq(XPUF_IRO_FREQ_400MHZ, &IroFreqUpdated);
		if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
			Status = StatusTmp;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function clears PUF ID
 *
 * @return
.*			 - XST_SUCCESS  PUF ID is successfully cleared
.*			 - XPUF_ERROR_PUF_ID_ZERO_TIMEOUT  Timeout occurred for clearing
 *                                               PUF ID
.*			 - XST_FAILURE  Unexpected event
 *
 *****************************************************************************/
int XPuf_ClearPufID(void)
{
	int Status = XST_FAILURE;
	int WaitStatus = XST_FAILURE;

	/**
	 * Set least significant bit in the PUF_CLEAR register.
	 */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CLEAR_OFFSET,
		XPUF_CLEAR_ID);

	/**
	 * The API waits for ID_ZERO bit to be set in PUF Status register.
	 * If id zero bit is not set within 1 second then returns XPUF_ERROR_PUF_ID_ZERO_TIMEOUT
	 * else returns XST_SUCCESS.
	 */
	WaitStatus = (int)Xil_WaitForEvent((UINTPTR)(XPUF_PMC_GLOBAL_BASEADDR +
		XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET), XPUF_STATUS_ID_ZERO,
		XPUF_STATUS_ID_ZERO, XPUF_STATUS_WAIT_TIMEOUT);
	if (WaitStatus != XST_SUCCESS) {
		Status = XPUF_ERROR_PUF_ID_ZERO_TIMEOUT;
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function captures PUF ID generated into XPuf_Data.
 *
 * @param	PufData - Pointer to XPuf_Data structure which includes options
 *                        to configure PUF
 *
 *****************************************************************************/
static void XPuf_CapturePufID(XPuf_Data *PufData)
{
	u32 Index;

	/**
	 * Reads PUF ID from PUF_ID_0 to PUF_ID_7 registers.
	 */
	for (Index = 0U; Index < XPUF_ID_LEN_IN_WORDS; Index++) {
		PufData->PufID[Index] = XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
			(XPUF_PMC_GLOBAL_PUF_ID_0_OFFSET + (Index * XPUF_WORD_LENGTH)));
	}
}

/*****************************************************************************/
/**
 * @brief	This function validates if secure control bits for each PUF
 *              operations are set or not
 *
 * @param	PufData - Pointer to XPuf_Data structure which includes options
 *                        to configure PUF
 *
 * @return
 *			 - XST_SUCCESS  secure control bits are not set
.*			 - XPUF_ERROR_REGISTRATION_INVALID  PUF registration is not allowed
.*			 - XPUF_ERROR_REGENERATION_INVALID  PUF regeneration is not allowed
 *			 - XPUF_ERROR_REGEN_PUF_HD_INVALID  PUF HD in eFUSE id invalidated
 *			 - XPUF_ERROR_INVALID_SYNDROME_MODE  If regeneration from eFUSE cache
 *                                                 is selected with 12K syndrome mode
 *			 - XPUF_ERROR_INVALID_PUF_OPERATION  In case of invalid PUF operation
 *			 - XST_FAILURE  Unexpected event
 *
 *****************************************************************************/
static int XPuf_ValidateAccessRules(const XPuf_Data *PufData)
{
	int Status = XST_FAILURE;
	u32 Operation = PufData->PufOperation;
	u32 PufEccCtrlValue = XPuf_ReadReg(XPUF_EFUSE_CACHE_BASEADDR,
					XPUF_PUF_ECC_PUF_CTRL_OFFSET);
	u32 SecurityCtrlVal = XPuf_ReadReg(XPUF_EFUSE_CACHE_BASEADDR,
					XPUF_EFUSE_CACHE_SECURITY_CONTROL);

	switch (Operation) {
		/**
		 * If PUF operation is Registration.
		 */
		case XPUF_REGISTRATION:
			if ((SecurityCtrlVal & XPUF_PUF_DIS) == XPUF_PUF_DIS) {
				/**
				 * Return XPUF_ERROR_REGISTRATION_INVALID as error code,
				 * if PUF is disabled in security control register.
				 */
				Status = XPUF_ERROR_REGISTRATION_INVALID;
			}
			else if (XPuf_IsRegistrationDisabled() != FALSE) {
				/**
				 * Return XPUF_ERROR_REGISTRATION_INVALID as error code,
				 * if PUF registration is disabled for Versal Net device.
				 */
				Status = XPUF_ERROR_REGISTRATION_INVALID;
			}
			else {
				Status = XST_SUCCESS;
			}
			break;
		/**
		 * If PUF operation is Regeneration.
		 */
		case XPUF_REGEN_ON_DEMAND:
		case XPUF_REGEN_ID_ONLY:
			if (((SecurityCtrlVal & XPUF_PUF_DIS) == XPUF_PUF_DIS) ||
				((PufEccCtrlValue & XPUF_PUF_REGEN_DIS) == XPUF_PUF_REGEN_DIS)) {
				/**
				 * Return XPUF_ERROR_REGENERATION_INVALID as error code,
				 * if either PUF or regeneration is disabled in
				 * security control and PUF ECC PUF CTRL register.
				 */
				Status = XPUF_ERROR_REGENERATION_INVALID;
			}
			else if((PufData->ReadOption == XPUF_READ_FROM_EFUSE_CACHE) &&
				((PufEccCtrlValue & XPUF_PUF_HD_INVLD) == XPUF_PUF_HD_INVLD)) {
				/**
				 * Return XPUF_ERROR_REGEN_PUF_HD_INVALID as error code,
				 * if user opts Readoption from eFuse cache and
				 * PUF helper data is invalid(when fuse is blown).
				 */
				Status = XPUF_ERROR_REGEN_PUF_HD_INVALID;
			}
			else {
				Status = XST_SUCCESS;
			}
			break;
		default:
			/**
			 * Return XPUF_ERROR_INVALID_PUF_OPERATION as
			 * error code, if PUF operation is invalid.
			 */
			Status = XPUF_ERROR_INVALID_PUF_OPERATION;
			break;

	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function updates the helper data for PUF regeneration
 *              according to the location of helper data provided by the user
 *
 * @param       PufData - Pointer to XPuf_Data structure which includes options
 *                        to configure PUF
 *
 * @return
 *			 - XST_SUCCESS  On valid Read option
 *			 - XPUF_ERROR_INVALID_READ_HD_INPUT  On invalid Read option
 *			 - XST_FAILURE  On unexpected event
 *
 *****************************************************************************/
static int XPuf_UpdateHelperData(const XPuf_Data *PufData)
{
	int Status = XST_FAILURE;
	u32 PufChash;
	u32 PufAux;

	/**
	 * If Readoption is from RAM.
	 */
	if (PufData->ReadOption == XPUF_READ_FROM_RAM) {
		PufChash = PufData->Chash;
		PufAux = PufData->Aux;
		if ((PufChash == 0U) || (PufAux == 0U)) {
			/**
			 * Return XPUF_ERROR_CHASH_NOT_PROGRAMMED as error code,
			 * if either PUF CHASH or PUF AUX are zero.
			 */
			Status = XPUF_ERROR_CHASH_NOT_PROGRAMMED;
			XPuf_Printf(XPUF_DEBUG_GENERAL,
				"PUF regeneration is not allowed,"
				"as PUF helper data is not provided/stored\r\n");
			goto END;
		}
		/**
		 * Write auxiliary data(PUF Helper Data) in PUF_AUX register.
		 */
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_AUX_OFFSET,
			PufAux);
		/**
		 * Write CHASH data(PUF Helper Data) in PUF_CHASH register.
		 */
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_CHASH_OFFSET, PufChash);
		/**
		 * Write PUF Syndrome Data Address in PUF_SYN_ADDR register.
		 */
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_SYN_ADDR_OFFSET, PufData->SyndromeAddr);
		Status = XST_SUCCESS;
	}
	/**
	 * If Readoption is from eFUSE cache, write PUF Syndrome Data Address in PUF_SYN_ADDR register
	 * else, return XPUF_ERROR_INVALID_READ_HD_INPUT.
	 */
	else if (PufData->ReadOption == XPUF_READ_FROM_EFUSE_CACHE) {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_SYN_ADDR_OFFSET, XPUF_EFUSE_SYN_ADD_INIT);
		Status = XST_SUCCESS;
	}
	else {
		Status = XPUF_ERROR_INVALID_READ_HD_INPUT;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function triggers PUF Regeneration by configuring type of
 *              regeneration provided by the user. It regenerates PUF Key and ID
 *              depending on the type of regeneration using the helper data
 *
 * @param       PufData - Pointer to XPuf_Data structure which includes options
 *                        to configure PUF
 *
 * @return
 *			 - XST_SUCCESS  On Successful Regeneration
 *			 - XPUF_ERROR_INVALID_REGENERATION_TYPE  On Selection of invalid
 *                                                     regeneration type
 *			 - XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT  Timeout occurred while
 *                                                     waiting for PUF done bit
 *			 - XPUF_ERROR_PUF_DONE_KEY_ID_NT_RDY  Key ready bit and ID ready
 *                                                     bit is not set
 *			 - XPUF_ERROR_PUF_DONE_ID_NT_RDY  Id ready bit is not set
 *
 *****************************************************************************/
static int XPuf_StartRegeneration(XPuf_Data *PufData)
{
	volatile int Status = XST_FAILURE;
	u32 PufStatus;

	/**
	 * Check the requested PUF operation (on-demand regeneration or ID only regeneration) and update the PUF_CMD register,
         * If the requested PUF operation is invalid then return XPPUF_ERROR_REGENERATION_TYPE as error code.
	 */
	if(XPUF_REGEN_ID_ONLY == PufData->PufOperation) {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CMD_OFFSET,
			XPUF_CMD_REGEN_ID_ONLY);
	}
	else if(XPUF_REGEN_ON_DEMAND == PufData->PufOperation) {
		 XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CMD_OFFSET,
			XPUF_CMD_REGEN_ON_DEMAND);
	}
	else {
		Status = XPUF_ERROR_INVALID_REGENERATION_TYPE;
		goto END;
	}

	/**
	 * Wait for PUF done bit in the PUF_STATUS register to be set.
	 */
	Status  = XPuf_WaitForPufDoneStatus();
	if (Status != XST_SUCCESS) {
		/**
		 * Return XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT as
		 * error code, if the PUF done bit is not set within 1 second
		 * in PUF_STATUS register.
		 */
		XPuf_Printf(XPUF_DEBUG_GENERAL,
			"Error: Puf Regeneration failed!! \r\n");
		Status = XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT;
		goto END;
	}

	/**
	 * Read content of PUF_STATUS register to PufStatus variable.
	 */
	PufStatus = XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET);

	Status = XST_FAILURE;

	/**
	 * Check if PUF ID is ready.
	 */
	if ((PufStatus & XPUF_STATUS_ID_RDY) == XPUF_STATUS_ID_RDY) {
		if ((XPUF_REGEN_ON_DEMAND == PufData->PufOperation) &&
			((PufStatus & XPUF_STATUS_KEY_RDY) !=
				XPUF_STATUS_KEY_RDY)) {
				/**
				 * Return XPUF_ERROR_PUF_DONE_KEY_NT_RDY as error
				 * code, if key is not ready.
				 */
				Status = XPUF_ERROR_PUF_DONE_KEY_NT_RDY;
				goto END;
		}
		/**
		 * Capture PUF ID generated into XPuf_Data.
		 */
		XPuf_CapturePufID(PufData);
		Status = XST_SUCCESS;
	}
	else {
		/**
		 * Return XPUF_ERROR_PUF_DONE_ID_NT_RDY as error code, if PUF ID
		 * is not ready.
		 */
		Status = XPUF_ERROR_PUF_DONE_ID_NT_RDY;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	Converts the PUF Syndrome data to eFUSE writing format
 *
 * @param	PufData - Pointer to XPuf_Data structure which includes options
 *                        to configure PUF
 *
 * @return
 *			 - XST_SUCCESS  Syndrome data is successfully trimmed
 *			 - XPUF_ERROR_INVALID_PARAM  PufData instance pointer is NULL
 *			 - XST_FAILURE  Unexpected event
 *
 * @note	Formatted data will be available at PufData->EfuseSynData
 *
 ******************************************************************************/
int XPuf_GenerateFuseFormat(XPuf_Data *PufData)
{
	int Status = XST_FAILURE;
	u32 SynData[XPUF_4K_PUF_SYN_LEN_IN_WORDS] = {0U};
	u32 SIndex = 0U;
	u32 DIndex = 0U;
	u32 Index;
	u32 SubIndex;

	/**
	 *  Check if PufData instance pointer is NULL. If NULL, return XPUF_ERROR_INVALID_PARAM.
	 */
	if (PufData == NULL) {
		Status = XPUF_ERROR_INVALID_PARAM;
		goto END;
	}

	/**
	 *  Copy syndrome data from instance pointer to a local variable.
	 */
	Status = Xil_SMemCpy(SynData, XPUF_4K_PUF_SYN_LEN_IN_BYTES,
		PufData->SyndromeData, XPUF_4K_PUF_SYN_LEN_IN_BYTES,
		XPUF_4K_PUF_SYN_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Trimming logic for PUF Syndrome Data:
	 *    ------------------------------------
	 * Space allocated in eFUSE for syndrome data = 4060bits
	 * eFUSE02 - 2032bits
	 * eFUSE03 - 2028bits
	 *
	 * PUF Helper data generated for 4K Mode through registration
	 * is 140 Words = 140*32 = 4480 bits.
	 * Remove lower 12 bits of every fourth word of syndrome data.
	 *
	 *  After removing these bits remaining syndrome data will be
	 *  exactly 4060bits which will fit into eFUSE.
	 *
	 *
	 *	Illustration:
	 *
	 *	Input
	 *	-----
	 * 454D025B
	 * CDCB36FC
	 * EE1FE4C5
	 * 3FE53F74 --> F74 has to removed &
	 * 3A0AE7F8	next word upper 12 bits have to be shifted here
	 * 2373F03A
	 * C83188AF
	 * 3A5EB687--> 687 has to be removed
	 * B83E4A1D
	 * D53B5C50
	 * FA8B33D9
	 * 07EEFF43 --> F43 has to be removed
	 * CD01973F
	 * ........
	 * ........
	 * ........
	 */

	for (Index = 0U; Index < 5U; Index++) {
		for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
			if (SubIndex == 3U) {
				PufData->EfuseSynData[DIndex] =
				(SynData[SIndex] & XPUF_EFUSE_TRIM_MASK) |
				(SynData[SIndex + 1U] >> 20U);
			}
			else {
				PufData->EfuseSynData[DIndex] =
							SynData[SIndex];
			}
			SIndex++;
			DIndex++;
		}

		for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
			if (SubIndex == 3U) {
				PufData->EfuseSynData[DIndex] =
					(((SynData[SIndex] &
					XPUF_EFUSE_TRIM_MASK) << 12U) |
						(SynData[SIndex + 1U] >> 8U));
			}
			else {
				PufData->EfuseSynData[DIndex] =
				((SynData[SIndex] << 12U) |
						(SynData[SIndex + 1U] >> 20U));
			}
			SIndex++;
			DIndex++;
		}

		for (SubIndex = 0U; SubIndex < 3U; SubIndex++) {
			if (SubIndex == 2U) {
				PufData->EfuseSynData[DIndex] =
					((SynData[SIndex] << 24U) |
					((SynData[SIndex + 1U] &
						XPUF_EFUSE_TRIM_MASK) >> 8U));
				if (DIndex < (XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS - 1U)) {
					PufData->EfuseSynData[DIndex] |=
						(SynData[SIndex + 2U] >> 28U);
				}
			}
			else {
				PufData->EfuseSynData[DIndex]=
					((SynData[SIndex] << 24U) |
						(SynData[SIndex + 1U] >> 8U));
			}
			SIndex++;
			DIndex++;
		}
		SIndex++;

		if (Index != 4U) {
			for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
				if (SubIndex == 3U) {
					PufData->EfuseSynData[DIndex] =
						(((SynData[SIndex] &
					XPUF_EFUSE_TRIM_MASK) << 4U) |
						(SynData[SIndex + 1U] >> 16U));

				}
				else {
					PufData->EfuseSynData[DIndex] =
						((SynData[SIndex] << 4U) |
						(SynData[SIndex + 1U] >> 28U));
				}
				SIndex++;
				DIndex++;
			}

			for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
				if(SubIndex == 3U) {
					PufData->EfuseSynData[DIndex] =
						(((SynData[SIndex] &
					XPUF_EFUSE_TRIM_MASK) << 16U) |
						(SynData[SIndex + 1U] >> 4U));

				}
				else {
					PufData->EfuseSynData[DIndex]=
						((SynData[SIndex] << 16U) |
						(SynData[SIndex + 1U] >> 16U));
				}
				SIndex++;
				DIndex++;
			}

			for (SubIndex = 0U; SubIndex < 3U; SubIndex++) {
				if (SubIndex == 2U) {
					PufData->EfuseSynData[DIndex] =
						((SynData[SIndex] << 28U) |
						((SynData[SIndex + 1U] &
						XPUF_EFUSE_TRIM_MASK) >> 4U));
					PufData->EfuseSynData[DIndex] |=
						(SynData[SIndex + 2U] >> 24U);
				}
				else {
					PufData->EfuseSynData[DIndex]=
						((SynData[SIndex] << 28U) |
						(SynData[SIndex + 1U] >> 4U));
				}
				SIndex++;
				DIndex++;
			}
			SIndex++;

			for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
				if (SubIndex == 3U) {
					PufData->EfuseSynData[DIndex] =
						(((SynData[SIndex] &
						XPUF_EFUSE_TRIM_MASK) << 8U) |
						(SynData[SIndex + 1U] >> 12U));

				}
				else {
					PufData->EfuseSynData[DIndex] =
						((SynData[SIndex] << 8U) |
						(SynData[SIndex + 1U] >> 24U));
				}
				SIndex++;
				DIndex++;
			}

			for (SubIndex = 0U; SubIndex < 3U; SubIndex++) {
				if (SubIndex == 2U) {
					PufData->EfuseSynData[DIndex] =
						((SynData[SIndex] << 20U) |
						((SynData[SIndex + 1U] &
					XPUF_EFUSE_TRIM_MASK) >> 12U));

				}
				else {
					PufData->EfuseSynData[DIndex]=
						((SynData[SIndex] << 20U) |
						(SynData[SIndex + 1U] >> 12U));
				}
				SIndex++;
				DIndex++;
			}
			SIndex++;
		}
	}
	/**
	 * Use the above mentioned logic to trim the data and copy the trimmed data in EfuseSynData array in the instance pointer.
	 * Return XST_SUCCESS.
	 */
	PufData->EfuseSynData[XPUF_LAST_WORD_OFFSET] &=
						XPUF_LAST_WORD_MASK;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sets the IRO frequency.IRO frequency can be set
 *		to 320 MHZ or 400 MHZ
 *
 * @param	IroFreq - IRO frequency to be set.
 * @param	IroFreqUpdated -  Flag to indicate whether IRO frequency is updated.
 *
 *
 * @return
 *			 - XST_SUCCESS  if IRO is at required frequency or updated to required
 *			      frequency
 *			 - XPUF_IRO_FREQ_WRITE_MISMATCH  Mismatch in either reading or writing
 *					       IRO frequency
 *
 ******************************************************************************/
static int XPuf_ChangeIroFreq(u32 IroFreq, u8 *IroFreqUpdated)
{
	int Status = XST_FAILURE;
	u8 ReadIroFreq;

	*IroFreqUpdated = FALSE;
	/**
	 * Read IRO frequeny, If IRO frequency is different from required frequency,
	 * then set the IRO frequency to required frequency(either 320 MHZ or 400 MHZ).
	 */
	ReadIroFreq = XPuf_ReadIroFreq();
	if (ReadIroFreq != IroFreq) {
		XPuf_WriteIroFreq(IroFreq);
		ReadIroFreq = XPuf_ReadIroFreq();
		if (ReadIroFreq != IroFreq) {
			Status = XPUF_IRO_FREQ_WRITE_MISMATCH;
			goto END;
		}
		*IroFreqUpdated = TRUE;
	}
	Status = XST_SUCCESS;
END:
	return Status;
}
