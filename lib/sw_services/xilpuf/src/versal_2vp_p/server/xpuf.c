/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf.c
*
* This file contains PUF hardware interface API definitions for VERSAL_2VP_P.
* It implements the new PUF IP support for the VERSAL_2VP_P platform.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   sd  04/13/2026 Initial release
*
* </pre>
*
******************************************************************************/
/**
 * @addtogroup xpuf_server_apis XilPuf Server APIs
 * @{
 */
/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xpuf.h"
#include "xpuf_hw.h"
#include "xil_util.h"
#include "xpuf_plat.h"

/************************** Constant Definitions *****************************/
#define XPUF_STATUS_WAIT_TIMEOUT		(1000000U)
				/**< Recommended software timeout is 1 second */

/********************Macros (Inline function) Definitions*********************/
/*****************************************************************************/
/**
 * @brief	This function waits till Puf Syndrome ready bit is set.
 *
 * @return
 *		- XST_SUCCESS if Syndrome word is ready.
 *		- XST_FAILURE if timeout occurred.
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
 * @brief	This function waits until PUF done bit is set.
 *
 * @return
 *		- XST_SUCCESS on successful PUF Operation.
 *		- XST_FAILURE if timeout occurred.
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
 *
 * @brief	This function reads the IRO frequency value from register
 *
 * @return
 *		- 8-bit value of the IRO frequency register.
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
 * @brief       This function writes the IRO frequency value into register
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
 * @brief	This functions performs PUF enrollment (registration for VERSAL_2VP_P)
 *
 * @param	PufData Pointer to XPuf_Data structure which includes options
 *		to configure PUF.
 *
 * @return
 *		- XST_SUCCESS  PUF registration successful
 *		- XPUF_ERROR_INVALID_PARAM  PufData is NULL
 *		- XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT  Timeout occurred while
 *		waiting for PUF Syndrome data
 *		- XPUF_ERROR_PUF_DONE_WAIT_TIMEOUT  Timeout occurred while
 *		waiting for PUF done bit at the time of PUF registration
 *		- XPUF_IRO_FREQ_WRITE_MISMATCH  Mismatch in writing or reading
 *		IRO frequency
 *		- XST_FAILURE  Unexpected event
 *
 * @note	Helper data will be available in PufData->SyndromeData,
 *		PufData->Chash.
 *		PUF is only supported when using a nominal VCC_PMC of 0.70V and
 *		IRO frequency of 320 MHz.
 *		For VERSAL_2VP_P, no AUX data and no Global Variation Filter.
 *
 *****************************************************************************/
int XPuf_Registration(XPuf_Data *PufData)
{
	volatile int Status = XST_FAILURE;
	volatile int IroRestoreStatus = XST_FAILURE;
	u32 Idx = 0U;
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
	 * When registering the PUF, the PMC internal ring oscillator (IRO) frequency must be set
	 * to 320 MHz.
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
	 * Update shutter value for PUF enrollment.
	 */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_SHUT_OFFSET,
		PufData->ShutterValue);

	/**
	 * Update PUF Ring Oscillator Swap setting.
	 */
	XPuf_SetRoSwap(PufData);

	/**
	 * Trigger PUF enrollment command.
	 */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CMD_OFFSET,
		XPUF_CMD_ENROLLMENT);

	Status = XST_FAILURE;

	/**
	 * PUF helper data for VERSAL_2VP_P includes Syndrome data and CHash only (no AUX).
	 * Capturing Syndrome data word by word.
	 */
	for (Idx = 0U; Idx < XPUF_SYN_LEN_IN_WORDS; Idx++) {
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
	 * read CHash and PUF ID.
	 * For VERSAL_2VP_P, no AUX data is read.
	 */
	Status = XST_FAILURE;
	Status  = XPuf_WaitForPufDoneStatus();
	if (Status != XST_SUCCESS) {
		Status = XPUF_ERROR_PUF_DONE_WAIT_TIMEOUT;
		goto END;
	}
	PufData->Chash = XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
		XPUF_PMC_GLOBAL_PUF_CHASH_OFFSET);
	XPuf_CapturePufID(PufData);


END:
	/**
	 * If IRO frequency is updated to 320MHz then set frequency back to 400MHz.
	 */
	if (IroFreqUpdated == TRUE) {
		IroRestoreStatus = XPuf_ChangeIroFreq(XPUF_IRO_FREQ_400MHZ, &IroFreqUpdated);
		if (Status == XST_SUCCESS) {
			Status = IroRestoreStatus;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function regenerates PUF data using helper data stored in eFUSE
 *		or external memory.
 *		For VERSAL_2VP_P, no AUX data or Global Variation Filter.
 *
 * @param	PufData Pointer to XPuf_Data structure which includes options
 *		to configure PUF.
 *
 * @return
 *		- XST_SUCCESS on successful PUF Regeneration
 *		- XPUF_ERROR_INVALID_PARAM if PufData is NULL
 *		- XPUF_ERROR_INVALID_REGENERATION_TYPE if selected regeneration type
 *		is invalid
 *		- XPUF_ERROR_CHASH_NOT_PROGRAMMED  Helper data not provided
 *		- XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT  Timeout occurred while
 *		waiting for PUF done bit
 *		- XPUF_ERROR_PUF_DONE_KEY_NT_RDY  Key ready bit and ID ready
 *		bit is not set
 *		- XPUF_ERROR_PUF_DONE_ID_NT_RDY  Id ready bit is not set
 *		- XPUF_IRO_FREQ_WRITE_MISMATCH  Mismatch in writing or reading
 *		IRO frequency at the time of PUF regeneration
 *		- XST_FAILURE  Unexpected event
 *
 *****************************************************************************/
int XPuf_Regeneration(XPuf_Data *PufData)
{
	volatile int Status = XST_FAILURE;
	volatile int IroRestoreStatus = XST_FAILURE;
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
	 * Check if requested PUF operation is valid.
	 */
	if ((PufData->PufOperation != XPUF_REGEN_ON_DEMAND) &&
		(PufData->PufOperation != XPUF_REGEN_ID_ONLY)) {
		Status = XPUF_ERROR_INVALID_PARAM;
		goto END;
	}

	/**
	 * When regenerating the PUF, the PMC internal ring oscillator (IRO) frequency must be set
	 * to 320 MHz.
	 */
	if (XPuf_IsIroFreqChangeReqd() == XPUF_IROFREQ_CHANGE_REQD) {
		Status = XPuf_ChangeIroFreq(XPUF_IRO_FREQ_320MHZ, &IroFreqUpdated);
		if (Status != XST_SUCCESS) {
			XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
			goto END;
		}
	}

	/**
	 * Validate the access rules for PUF regeneration.
	 */
	Status =  XPuf_ValidateAccessRules(PufData);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Status = XST_FAILURE;

	/**
	 * Update the helper data for PUF regeneration.
	 * For VERSAL_2VP_P, no AUX data is used.
	 */
	Status = XPuf_UpdateHelperData(PufData);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	/**
	 * Update Shutter value in PUF_SHUT register.
	 */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_SHUT_OFFSET,
		PufData->ShutterValue);

	GlobalCntrlVal = XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
                XPUF_PMC_GLOBAL_GLOBAL_CNTRL_OFFSET);

	/**
	 * Disable SLVERR for PMC_GLOBAL during regeneration.
	 */
	if ((GlobalCntrlVal & XPUF_SLVERR_ENABLE_MASK) == XPUF_SLVERR_ENABLE_MASK) {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_GLOBAL_CNTRL_OFFSET,
			GlobalCntrlVal & ~(XPUF_SLVERR_ENABLE_MASK));
		Reset = TRUE;
	}

	/**
	 * Update PUF Ring Oscillator Swap setting.
	 */
	XPuf_SetRoSwap(PufData);

	Status = XST_FAILURE;

	/**
	 * Trigger PUF regeneration.
	 */
	Status = XPuf_StartRegeneration(PufData);

	/**
	 * Re-enable SLVERR if it was disabled.
	 */
	if (Reset == TRUE) {
		GlobalCntrlVal = XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_GLOBAL_CNTRL_OFFSET);
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_GLOBAL_CNTRL_OFFSET,
			GlobalCntrlVal | XPUF_SLVERR_ENABLE_MASK);
	}

END:
	if (IroFreqUpdated == TRUE) {
		IroRestoreStatus = XPuf_ChangeIroFreq(XPUF_IRO_FREQ_400MHZ, &IroFreqUpdated);
		if (Status == XST_SUCCESS) {
			Status = IroRestoreStatus;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function clears PUF ID
 *
 * @return
 * 		 - XST_SUCCESS if PUF ID is cleared successfully
 * 		 - XPUF_ERROR_PUF_ID_ZERO_TIMEOUT if time out while clearing PUF ID
 *
 *****************************************************************************/
int XPuf_ClearPufID(void)
{
	int Status = XST_FAILURE;
	int WaitStatus = XST_FAILURE;

	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CLEAR_OFFSET,
		XPUF_CLEAR_ID);

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
 * @param	PufData Pointer to XPuf_Data structure which includes options
 *		to configure PUF.
 *
 *****************************************************************************/
static void XPuf_CapturePufID(XPuf_Data *PufData)
{
	u32 Index;

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
 * @param	PufData Pointer to XPuf_Data structure which includes options
 *		to configure PUF.
 *
 * @return
 * 		- XST_SUCCESS  secure control bits are not set
 * 		- XPUF_ERROR_REGISTRATION_INVALID  PUF registration is not allowed
 * 		- XPUF_ERROR_REGENERATION_INVALID  PUF regeneration is not allowed
 * 		- XPUF_ERROR_REGEN_PUF_HD_INVALID  PUF HD in eFUSE id invalidated
 * 		- XPUF_ERROR_INVALID_PUF_OPERATION  In case of invalid PUF operation
 * 		- XST_FAILURE  Unexpected event
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
			else if (XPuf_IsRegistrationDisabled() != FALSE) {
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
 * @brief       This function updates the helper data for PUF regeneration.
 *              For VERSAL_2VP_P, only CHash is required (no AUX data).
 *
 * @param	PufData Pointer to XPuf_Data structure.
 *
 * @return
 *		 - XST_SUCCESS  On valid Read option
 *		 - XPUF_ERROR_INVALID_READ_HD_INPUT  On invalid Read option
 *		 - XPUF_ERROR_CHASH_NOT_PROGRAMMED if PUF helper data is not provided
 *
 *****************************************************************************/
static int XPuf_UpdateHelperData(const XPuf_Data *PufData)
{
	int Status = XST_FAILURE;
	u32 PufChash;

	if (PufData->ReadOption == XPUF_READ_FROM_RAM) {
		PufChash = PufData->Chash;
		if (PufChash == 0U) {
			Status = XPUF_ERROR_CHASH_NOT_PROGRAMMED;
			XPuf_Printf(XPUF_DEBUG_GENERAL,
				"PUF regeneration is not allowed,"
				"as PUF helper data is not provided/stored\r\n");
			goto END;
		}
		/**
		 * Write CHASH data (PUF Helper Data) in PUF_CHASH register.
		 * No AUX data for VERSAL_2VP_P.
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
 * @brief       This function triggers PUF Regeneration.
 *
 * @param	PufData Pointer to XPuf_Data structure.
 *
 * @return
 * 		- XST_SUCCESS  On Successful Regeneration
 * 		- XPUF_ERROR_INVALID_REGENERATION_TYPE  On invalid regeneration type
 * 		- XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT  Timeout occurred
 * 		- XPUF_ERROR_PUF_DONE_KEY_NT_RDY  Key ready bit not set
 * 		- XPUF_ERROR_PUF_DONE_ID_NT_RDY  ID ready bit is not set
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
 * @brief	This function sets the IRO frequency.
 *
 * @param	IroFreq IRO frequency to be set (0 for 320 MHz and 1 for 400 MHz)
 * @param	IroFreqUpdated Flag to indicate whether IRO frequency is updated.
 *
 * @return
 *		- XST_SUCCESS if IRO is at required frequency or updated
 *		- XPUF_IRO_FREQ_WRITE_MISMATCH if mismatch in reading or writing
 *
 *****************************************************************************/
static int XPuf_ChangeIroFreq(u32 IroFreq, u8 *IroFreqUpdated)
{
	int Status = XST_FAILURE;
	u8 ReadIroFreq;

	*IroFreqUpdated = FALSE;
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
/** @} */
