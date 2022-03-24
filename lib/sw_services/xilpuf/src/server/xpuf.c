/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
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

/************************** Constant Definitions *****************************/
#define XPUF_STATUS_WAIT_TIMEOUT		(1000000U)
				/**< Recommended software timeout is 1 second */
#define XPUF_SHUT_GLB_VAR_FLTR_ENABLED_SHIFT	(31)
		/**< Shift for Global Variation Filter bit in shutter value */
/********************Macros (Inline function) Definitions*********************/
#define XPuf_Printf(DebugType, ...)	\
	if ((DebugType) == (1U)) {xil_printf (__VA_ARGS__);}
		/**< For prints in XilPuf library */

/*****************************************************************************/
/**
 * @brief	This function waits till Puf Syndrome ready bit is set.
 *
 * @return	XST_SUCCESS	Syndrome word is ready.
 *		XST_FAILURE	Timeout occurred.
 *
 *****************************************************************************/
static inline int XPuf_WaitForPufSynWordRdy(void)
{
	return (int)Xil_WaitForEvent((XPUF_PMC_GLOBAL_BASEADDR +
		XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET),
		XPUF_STATUS_SYNDROME_WORD_RDY, XPUF_STATUS_SYNDROME_WORD_RDY,
		XPUF_STATUS_WAIT_TIMEOUT);
}

/*****************************************************************************/
/**
 * @brief	This function waits till Puf done bit is set.
 *
 * @return	XST_SUCCESS	Puf Operation is done.
 *		XST_FAILURE	Timeout occurred.
 *
 *****************************************************************************/
static inline int XPuf_WaitForPufDoneStatus(void)
{
	return (int)Xil_WaitForEvent((XPUF_PMC_GLOBAL_BASEADDR +
		XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET), XPUF_STATUS_PUF_DONE,
		XPUF_STATUS_PUF_DONE, XPUF_STATUS_WAIT_TIMEOUT);
}

/*****************************************************************************/
/**
 *
 * @brief	This function reads the value from the given register.
 *
 * @param	BaseAddress is the base address of the module which consists
 *			the register
 * @param	RegOffset is the register offset of the register.
 *
 * @return	The 32-bit value of the register.
 *
 * ***************************************************************************/
static inline u32 XPuf_ReadReg(u32 BaseAddress, u32 RegOffset)
{
	return Xil_In32((UINTPTR)(BaseAddress + RegOffset));
}

/*****************************************************************************/
/**
 *
 * @brief	This function writes the value into the given register.
 *
 * @param	BaseAddress is the base address of the module which consists
 *			the register
 * @param	RegOffset is the register offset of the register.
 * @param	Data is the 32-bit value to write to the register.
 *
 * @return	None.
 *
 *****************************************************************************/
static inline void XPuf_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
{
	Xil_Out32((UINTPTR)(BaseAddress + RegOffset), Data);
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
 * @brief	This function checks if the IRO frequency at time of PUF
 *		operation matches IRO frequency at boot i.e. 320 MHz.
 *
 * @return	XST_SUCCESS if IRO frequency is 320 MHz
 *		XPUF_IRO_FREQ_MISMATCH - IRO frequency is not 320 MHz
 *
 *****************************************************************************/
static inline int XPuf_CheckIroFreq(void)
{
	int Status = XPUF_IRO_FREQ_MISMATCH;

	if (XPuf_ReadReg(XPUF_EFUSE_CTRL_BASEADDR,
		XPUF_ANLG_OSC_SW_1LP_OFFSET) != 0U) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/************************** Function Prototypes ******************************/
static void XPuf_CapturePufID(XPuf_Data *PufData);
static int XPuf_ValidateAccessRules(const XPuf_Data *PufData);
static int XPuf_UpdateHelperData(const XPuf_Data *PufData);
static int XPuf_StartRegeneration(XPuf_Data *PufData);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This functions performs PUF registration
 *
 * @param	PufData - Pointer to XPuf_Data structure which includes options
 *                        to configure PUF
 *
 * @return  XST_SUCCESS - PUF registration successful
 *          XPUF_ERROR_INVALID_PARAM - PufData is NULL
 *          XPUF_ERROR_INVALID_SYNDROME_MODE      - Incorrect Registration mode
 *          XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT - Timeout occurred while
 *                                                  waiting for PUF Syndrome data
 *          XPUF_ERROR_PUF_DONE_WAIT_TIMEOUT      - Timeout occurred while
 *                                                  waiting for PUF done bit
 *          XPUF_IRO_FREQ_MISMATCH                - Mismatch in IRO frequency
 *                                                  at the time of PUF registration
 *          XST_FAILURE - Unexpected event
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
	volatile u32 MaxSyndromeSizeInWords;
	u32 Idx = 0U;

	if (PufData == NULL) {
		Status = XPUF_ERROR_INVALID_PARAM;
		goto END;
	}

	/**
	 * Error code shall be returned if MSB of PUF shutter value does not
	 * match with Global Variation Filter option
	 */
	if (((PufData->ShutterValue >> XPUF_SHUT_GLB_VAR_FLTR_ENABLED_SHIFT) ^
		PufData->GlobalVarFilter) == TRUE) {
		Status = XPUF_SHUTTER_GVF_MISMATCH;
		goto END;
	}

	Status = XPuf_CheckIroFreq();
	if(Status != XST_SUCCESS) {
		goto END;
	}

	Status =  XPuf_ValidateAccessRules(PufData);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	XPuf_CfgGlobalVariationFilter(PufData->GlobalVarFilter);

	if (XPUF_SYNDROME_MODE_4K == PufData->RegMode) {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CFG1_OFFSET,
			XPUF_CFG1_INIT_VAL_4K);
		MaxSyndromeSizeInWords = XPUF_4K_PUF_SYN_LEN_IN_WORDS;
	}
	else if (XPUF_SYNDROME_MODE_12K == PufData->RegMode) {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CFG1_OFFSET,
			XPUF_CFG1_INIT_VAL_12K);
		MaxSyndromeSizeInWords = XPUF_12K_PUF_SYN_LEN_IN_WORDS;
	}
	else {
		Status = XPUF_ERROR_INVALID_SYNDROME_MODE;
		goto END;
	}

	/**
	 * Update shutter value for PUF registration
	 */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_SHUT_OFFSET,
		PufData->ShutterValue);

	/**
	 * Trigger PUF registration
	 */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CMD_OFFSET,
		XPUF_CMD_REGISTRATION);

	/**
	 * Recheck MaxSyndromeSizeInWords before use to avoid overflow
	 */
	if (XPUF_SYNDROME_MODE_4K == PufData->RegMode) {
		MaxSyndromeSizeInWords = XPUF_4K_PUF_SYN_LEN_IN_WORDS;
	}
	else {
		MaxSyndromeSizeInWords = XPUF_12K_PUF_SYN_LEN_IN_WORDS;
	}

	Status = XST_FAILURE;

	/**
	 * PUF helper data includes Syndrome data, CHash and Auxillary data.
	 * Capturing Syndrome data word by word
	 */
	for (Idx = 0; Idx < MaxSyndromeSizeInWords; Idx++) {
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
	 * read CHash, Auxillary data and PUF ID
	 */
	if (Idx == MaxSyndromeSizeInWords) {
		Status = XST_FAILURE;
		Status  = XPuf_WaitForPufDoneStatus();
		if (Status != XST_SUCCESS) {
			Status = XPUF_ERROR_PUF_DONE_WAIT_TIMEOUT;
			goto END;
		}
		PufData->Chash = XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_CHASH_OFFSET);
		PufData->Aux = XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_AUX_OFFSET);
		XPuf_CapturePufID(PufData);
	}
	else {
		Status = XPUF_ERROR_SYN_DATA_ERROR;
		goto END;
	}

END:
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
 * @return      XST_SUCCESS - PUF Regeneration successful
 *              XPUF_ERROR_INVALID_PARAM - PufData is NULL
 *              XPUF_ERROR_INVALID_REGENERATION_TYPE - Selection of invalid
 *                                                     regeneration type
 *              XPUF_ERROR_CHASH_NOT_PROGRAMMED      - Helper data not provided
 *              XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT   - Timeout occurred while
 *                                                     waiting for PUF done bit
 *              XPUF_ERROR_PUF_DONE_KEY_ID_NT_RDY    - Key ready bit and ID ready
 *                                                     bit is not set
 *              XPUF_ERROR_PUF_DONE_ID_NT_RDY        - Id ready bit is not set
 *              XPUF_IRO_FREQ_MISMATCH               - Mismatch in IRO frequency
 *                                                     at the time of PUF regeneration
 *              XST_FAILURE - Unexpected event
 *
 * @note	PUF is only supported when using a nominal VCC_PMC of 0.70V or
 *		IRO frequency of 320 MHz
 *
 *****************************************************************************/
int XPuf_Regeneration(XPuf_Data *PufData)
{
	volatile int Status = XST_FAILURE;
	u32 GlobalCntrlVal;
	u32 Reset = FALSE;

	if (PufData == NULL) {
		Status = XPUF_ERROR_INVALID_PARAM;
		goto END;
	}

	if ((PufData->PufOperation != XPUF_REGEN_ON_DEMAND) &&
		(PufData->PufOperation != XPUF_REGEN_ID_ONLY)) {
		Status = XPUF_ERROR_INVALID_PARAM;
		goto END;
	}

	/**
	 * Error code shall be returned if MSB of PUF shutter value does not
	 * match with Global Variation Filter option
	 */
	if (((PufData->ShutterValue >> XPUF_SHUT_GLB_VAR_FLTR_ENABLED_SHIFT) ^
		PufData->GlobalVarFilter) == TRUE) {
		Status = XPUF_SHUTTER_GVF_MISMATCH;
		goto END;
	}

	Status = XPuf_CheckIroFreq();
	if(Status != XST_SUCCESS) {
		goto END;
	}

	Status =  XPuf_ValidateAccessRules(PufData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;

	Status = XPuf_UpdateHelperData(PufData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XPuf_CfgGlobalVariationFilter(PufData->GlobalVarFilter);

	if (XPUF_SYNDROME_MODE_4K == PufData->RegMode) {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CFG1_OFFSET,
			XPUF_CFG1_INIT_VAL_4K);
	}
	else {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CFG1_OFFSET,
			XPUF_CFG1_INIT_VAL_12K);
	}

	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_SHUT_OFFSET,
		PufData->ShutterValue);

	GlobalCntrlVal = XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
                XPUF_PMC_GLOBAL_GLOBAL_CNTRL_OFFSET);

	if ((GlobalCntrlVal & XPUF_SLVERR_ENABLE_MASK) == XPUF_SLVERR_ENABLE_MASK) {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_GLOBAL_CNTRL_OFFSET,
			GlobalCntrlVal & ~(XPUF_SLVERR_ENABLE_MASK));
		Reset = TRUE;
	}

	Status = XST_FAILURE;

	Status = XPuf_StartRegeneration(PufData);

	if (Reset == TRUE) {
		GlobalCntrlVal = XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_GLOBAL_CNTRL_OFFSET);
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_GLOBAL_CNTRL_OFFSET,
			GlobalCntrlVal | XPUF_SLVERR_ENABLE_MASK);
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function clears PUF ID
 *
 * @return	XST_SUCCESS - PUF ID is successfully cleared
 *		XPUF_ERROR_PUF_ID_ZERO_TIMEOUT - Timeout occurred for clearing
 *                                               PUF ID
 *		XST_FAILURE - Unexpected event
 *
 *****************************************************************************/
int XPuf_ClearPufID(void)
{
	int Status = XST_FAILURE;
	int WaitStatus = XST_FAILURE;

	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CLEAR_OFFSET,
		XPUF_CLEAR_ID);

	WaitStatus = (int)Xil_WaitForEvent((XPUF_PMC_GLOBAL_BASEADDR +
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
	 * Reads PUF ID from PUF_ID_0 to PUF_ID_7 registers
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
 * @return	XST_SUCCESS - secure control bits are not set
 *              XPUF_ERROR_REGISTRATION_INVALID  - PUF registration is not allowed
 *              XPUF_ERROR_REGENERATION_INVALID  - PUF regeneration is not allowed
 *              XPUF_ERROR_REGEN_PUF_HD_INVALID  - PUF HD in eFUSE id invalidated
 *              XPUF_ERROR_INVALID_SYNDROME_MODE - If regeneration from eFUSE cache
 *                                                 is selected with 12K syndrome mode
 *              XPUF_ERROR_INVALID_PUF_OPERATION - In case of invalid PUF operation
 *              XST_FAILURE - Unexpected event
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

		case XPUF_REGISTRATION:
			if ((SecurityCtrlVal & XPUF_PUF_DIS) == XPUF_PUF_DIS) {
				Status = XPUF_ERROR_REGISTRATION_INVALID;
			}
			else {
				Status = XST_SUCCESS;
			}
			break;

		case XPUF_REGEN_ON_DEMAND:
		case XPUF_REGEN_ID_ONLY:
			if (((SecurityCtrlVal & XPUF_PUF_DIS) == XPUF_PUF_DIS) ||
				((PufEccCtrlValue & XPUF_PUF_REGEN_DIS) == XPUF_PUF_REGEN_DIS)) {
				Status = XPUF_ERROR_REGENERATION_INVALID;
			}
			else if((PufData->ReadOption == XPUF_READ_FROM_EFUSE_CACHE) &&
				((PufEccCtrlValue & XPUF_PUF_HD_INVLD) == XPUF_PUF_HD_INVLD)) {
				Status = XPUF_ERROR_REGEN_PUF_HD_INVALID;
			}
			else if((PufData->ReadOption == XPUF_READ_FROM_EFUSE_CACHE) &&
				(PufData->RegMode == XPUF_SYNDROME_MODE_12K)) {
				Status = XPUF_ERROR_INVALID_SYNDROME_MODE;
			}
			else {
				Status = XST_SUCCESS;
			}
			break;
		default:
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
 * @return      XST_SUCCESS - On valid Read option
 *              XPUF_ERROR_INVALID_READ_HD_INPUT - On invalid Read option
 *              XST_FAILURE - On unexpected event
 *
 *****************************************************************************/
static int XPuf_UpdateHelperData(const XPuf_Data *PufData)
{
	int Status = XST_FAILURE;
	u32 PufChash;
	u32 PufAux;

	if (PufData->ReadOption == XPUF_READ_FROM_RAM) {
		PufChash = PufData->Chash;
		PufAux = PufData->Aux;
		if ((PufChash == 0U) || (PufAux == 0U)) {
			Status = XPUF_ERROR_CHASH_NOT_PROGRAMMED;
			XPuf_Printf(XPUF_DEBUG_GENERAL,
				"PUF regeneration is not allowed,"
				"as PUF helper data is not provided/stored\r\n");
			goto END;
		}
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_AUX_OFFSET,
			PufAux);
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_CHASH_OFFSET, PufChash);
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_SYN_ADDR_OFFSET, PufData->SyndromeAddr);
		Status = XST_SUCCESS;
	}
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
 * @return      XST_SUCCESS - On Successful Regeneration
 *              XPUF_ERROR_INVALID_REGENERATION_TYPE - On Selection of invalid
 *                                                     regeneration type
 *              XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT   - Timeout occurred while
 *                                                     waiting for PUF done bit
 *              XPUF_ERROR_PUF_DONE_KEY_ID_NT_RDY    - Key ready bit and ID ready
 *                                                     bit is not set
 *              XPUF_ERROR_PUF_DONE_ID_NT_RDY        - Id ready bit is not set
 *
 *****************************************************************************/
static int XPuf_StartRegeneration(XPuf_Data *PufData)
{
	volatile int Status = XST_FAILURE;
	u32 PufStatus;

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

	Status  = XPuf_WaitForPufDoneStatus();
	if (Status != XST_SUCCESS) {
		XPuf_Printf(XPUF_DEBUG_GENERAL,
			"Error: Puf Regeneration failed!! \r\n");
		Status = XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT;
		goto END;
	}

	PufStatus = XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET);

	Status = XST_FAILURE;

	if ((PufStatus & XPUF_STATUS_ID_RDY) == XPUF_STATUS_ID_RDY) {
		if ((XPUF_REGEN_ON_DEMAND == PufData->PufOperation) &&
			((PufStatus & XPUF_STATUS_KEY_RDY) !=
				XPUF_STATUS_KEY_RDY)) {
				Status = XPUF_ERROR_PUF_DONE_KEY_NT_RDY;
				goto END;
		}
		XPuf_CapturePufID(PufData);
		Status = XST_SUCCESS;
	}
	else {
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
 * @return	XST_SUCCESS - Syndrome data is successfully trimmed
 *		XPUF_ERROR_INVALID_PARAM - PufData instance pointer is NULL
 * 		XST_FAILURE - Unexpected event
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

	if (PufData == NULL) {
		Status = XPUF_ERROR_INVALID_PARAM;
		goto END;
	}

	Status = Xil_SMemCpy(SynData, XPUF_4K_PUF_SYN_LEN_IN_BYTES,
		PufData->SyndromeData, XPUF_4K_PUF_SYN_LEN_IN_BYTES,
		XPUF_4K_PUF_SYN_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Trimming logic for PUF Syndrome Data:
	 * ------------------------------------
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
	PufData->EfuseSynData[XPUF_LAST_WORD_OFFSET] &=
						XPUF_LAST_WORD_MASK;
	Status = XST_SUCCESS;

END:
	return Status;
}
