/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.0   kpt  08/21/2024 Initial release
*
* </pre>
*
******************************************************************************/
/**
 * @addtogroup xpuf_apis XilPuf APIs
 * @{
 */
/***************************** Include Files *********************************/
#include "sleep.h"
#include "xpuf.h"
#include "xpuf_hw.h"
#include "xil_util.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/
#define XPUF_STATUS_WAIT_TIMEOUT		(1000000U)
				/**< Recommended software timeout is 1 second */
#define XPUF_AUX_MASK_VALUE                     (0x0FFFFFF0U)
				/**< Mask value for AUX*/

#define XPUF_RESET_VAL					(1U)
				/**< PUF reset value */

#define XPUF_SET_VAL					(0U)
				/**< PUF set value  */

#define XPUF_PUF_AUX_ENABLE				(0xFU)
				/**< PUF aux enable */

#define XPUF_PUF_IC_MASK				(1U << 31U)
				/**< PUF IC mask */

#define XPUF_PMC_GLOBAL_PUF_KEY_CAPTURE	(1U << 2U)
				/**< PUF key capture mask */

#define XPUF_PMC_GLOBAL_PUF_ID_CAPTURE		(1 << 1U)
				/**< PUF id capture mask */

#define XPUF_KEY_GEN_ITERATIONS 6U /** PUF key generation iterations */

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
	return (int)Xil_WaitForEvent((UINTPTR)(XPUF_BASEADDR +
		XPUF_PUF_STATUS_OFFSET),
		XPUF_STATUS_SYNDROME_WORD_RDY, XPUF_STATUS_SYNDROME_WORD_RDY,
		XPUF_STATUS_WAIT_TIMEOUT);
}

/*****************************************************************************/
/**
 * @brief	This function waits till Puf Syndrome ready bit is set.
 *
 * @return
 *		- XST_SUCCESS if Syndrome word is ready.
 *		- XST_FAILURE if timeout occurred.
 *
 *****************************************************************************/
static inline int XPuf_WaitForPufSegmentRdy(void)
{
	return (int)Xil_WaitForEvent((UINTPTR)(XPUF_BASEADDR +
		XPUF_PUF_STATUS_OFFSET),
		XPUF_STATUS_SEG_RDY, XPUF_STATUS_SEG_RDY,
		XPUF_STATUS_WAIT_TIMEOUT);
}

/*****************************************************************************/
/**
 * @brief	This function waits till Puf done bit is set.
 *
 * @return
 *		- XST_SUCCESS on successful Puf Operation.
 *		- XST_FAILURE if timeout occurred.
 *
 *****************************************************************************/
static inline int XPuf_WaitForKeyRdyStatus(void)
{
	return (int)Xil_WaitForEvent((UINTPTR)(XPUF_BASEADDR +
		XPUF_PUF_STATUS_OFFSET), XPUF_STATUS_KEY_RDY,
		XPUF_STATUS_KEY_RDY, XPUF_STATUS_WAIT_TIMEOUT);
}

/*****************************************************************************/
/**
 * @brief       This function configures the Global Variation Filter option provided
 *              by user and updates Puf Cfg0 register
 *
 * @param       GlobalVarFilter User configuration to enable/disable
 *		Global Variation Filter in PUF.
 *
 *****************************************************************************/
static inline void XPuf_CfgGlobalVariationFilter(const u8 GlobalVarFilter)
{
	if (GlobalVarFilter == TRUE) {
		XPuf_WriteReg(XPUF_BASEADDR, XPUF_PUF_CFG0_OFFSET,
			(XPUF_CFG0_HASH_SEL | XPUF_CFG0_GLOBAL_FILTER_ENABLE));
	}
	else {
		XPuf_WriteReg(XPUF_BASEADDR, XPUF_PUF_CFG0_OFFSET,
			XPUF_CFG0_HASH_SEL);
	}
}

/*****************************************************************************/
/**
 * @brief       This function clears PUF data
 *
 * @return
 *		- XST_SUCCESS on successful Puf Operation.
 *		- XST_FAILURE if timeout occurred.
 *
 *****************************************************************************/
static inline int XPuf_ClearPuf(void)
{
	int Status = XST_FAILURE;

	Xil_Out32(XPUF_BASEADDR + XPUF_PUF_CMD_OFFSET, XPUF_CMD_STOP);

	Status = (int)Xil_WaitForEvent(XPUF_BASEADDR + XPUF_PUF_STATUS_OFFSET, XPUF_STATUS_ZZ_MASK,
					XPUF_STATUS_ZZ_MASK, XPUF_STATUS_WAIT_TIMEOUT);


	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function reset the PUF module
 *
 * @return
 *		- XST_SUCCESS on successful Puf Operation.
 *		- XST_FAILURE if timeout occurred.
 *
 *****************************************************************************/
static inline int XPuf_ResetPuf(void)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;

	Status = XPuf_ClearPuf();
	SStatus = Xil_SecureOut32(XPUF_PMC_GLOBAL_BASEADDR + XPUF_PMC_GLOBAL_PUF_RST_OFFSET, XPUF_RESET_VAL);

	return (Status | SStatus);
}

/*****************************************************************************/
/**
 * @brief       This function releases reset of the PUF module
 *
 * @return
 *		- XST_SUCCESS on successful Puf Operation.
 *		- XST_FAILURE if timeout occurred.
 *
 *****************************************************************************/
static inline int XPuf_SetPuf(void)
{
	int Status = XST_FAILURE;

	Status = Xil_SecureOut32(XPUF_PMC_GLOBAL_BASEADDR + XPUF_PMC_GLOBAL_PUF_RST_OFFSET, XPUF_RESET_VAL);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	usleep(1U);

	Status =  Xil_SecureOut32(XPUF_PMC_GLOBAL_BASEADDR + XPUF_PMC_GLOBAL_PUF_RST_OFFSET, XPUF_SET_VAL);

END:
	return Status;
}

/************************** Function Prototypes ******************************/
static void XPuf_CapturePufID(XPuf_Data *PufData);
static int XPuf_ValidateAccessRules(const XPuf_Data *PufData);
static int XPuf_UpdateHelperData(const XPuf_Data *PufData);
static int XPuf_StartRegeneration(XPuf_Data *PufData);
static int XPuf_GeneratePufKey(XPuf_Data *PufData);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This functions configures the PUF
 *
 * @param	PufData Pointer to XPuf_Data structure which includes options
 *		to configure PUF.
 *
 * @return
 *		- XST_SUCCESS  PUF registration successful
 *		- XPUF_ERROR_INVALID_PARAM  PufData is NULL
 *
 *
 *****************************************************************************/
static int XPuf_Cfg(const XPuf_Data *PufData)
{
	volatile int Status = XST_FAILURE;

	/**
	 * Perform input parameters validation,
	 * return XPUF_ERROR_INVALID_PARAM if input parameters are invalid.
	 */
	if (PufData == NULL) {
		Status = XPUF_ERROR_INVALID_PARAM;
		goto END;
	}

	Status = XPuf_SetPuf();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Issue a command stop */
	Xil_Out32(XPUF_BASEADDR, XPUF_CMD_STOP);

	/**
	 * Configures the Global Variation Filter option provided by user and
	 * updates PUF Configuration0 register.
	 */
	XPuf_CfgGlobalVariationFilter(PufData->GlobalVarFilter);

	/**
	 * Update PUF Configuration1 register as 4k Registration mode.
	 */
	XPuf_WriteReg(XPUF_BASEADDR, XPUF_PUF_CFG1_OFFSET,
		XPUF_CFG1_INIT_VAL_4K);

	/**
	 * Update Shutter value in PUF_SHUT register.
	 */
	XPuf_WriteReg(XPUF_BASEADDR, XPUF_PUF_SHUT_OFFSET,
		PufData->ShutterValue);


	/** Update PUF Ring Oscillator Swap setting. */
	Xil_Out32(XPUF_PMC_GLOBAL_BASEADDR + XPUF_PMC_GLOBAL_PUF_RO_SWAP_OFFSET,
		PufData->RoSwapVal);

	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This functions performs PUF registration
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
 *		- XST_FAILURE  Unexpected event
 *
 * @note	Helper data will be available in PufData->SyndromeData,
 *		PufData->Chash, PufData->Aux
 *
 *****************************************************************************/
int XPuf_Registration(XPuf_Data *PufData)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	u32 Idx = 0U;

	Status = XPuf_Cfg(PufData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Issue PUF command */
	Xil_Out32(XPUF_BASEADDR + XPUF_PUF_CMD_OFFSET, XPUF_CMD_REGISTRATION);

	Status = XST_FAILURE;

	/**
	 * PUF helper data includes Syndrome data, CHash and Auxiliary data.
	 * Capturing Syndrome data word by word.
	 */
	while (Idx < XPUF_4K_PUF_SYN_LEN_IN_WORDS) {
		Status = XPuf_WaitForPufSynWordRdy();
		if (Status != XST_SUCCESS) {
			Status = XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT;
			goto END;
		}
		PufData->SyndromeData[Idx] = XPuf_ReadReg(XPUF_BASEADDR,
			XPUF_PUF_WORD_OFFSET);
		Idx++;
	}

	/**
	 * Once complete Syndrome data is captured and PUF operation is done,
	 * read CHash, Auxiliary data and PUF ID.
	 */
	if (Idx == XPUF_4K_PUF_SYN_LEN_IN_WORDS) {
		Status = XST_FAILURE;
		Status  = XPuf_WaitForKeyRdyStatus();
		if (Status != XST_SUCCESS) {
			Status = XPUF_ERROR_PUF_DONE_WAIT_TIMEOUT;
			goto END;
		}
		PufData->Chash = XPuf_ReadReg(XPUF_BASEADDR,
			XPUF_PUF_CHASH_OFFSET);
		PufData->Aux = (XPuf_ReadReg(XPUF_BASEADDR,
			XPUF_PUF_STATUS_OFFSET) & XPUF_AUX_MASK_VALUE);
		XPuf_CapturePufID(PufData);
	}
	else {
		Status = XPUF_ERROR_SYN_DATA_ERROR;
	}

END:
	SStatus = XPuf_ResetPuf();
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function regenerates PUF data using helper data stored in eFUSE
 *		or external memory
 *
 * @param	PufData Pointer to XPuf_Data structure which includes options
 *		to configure PUF.
 *
 * @return
 *		- XST_SUCCESS on successful PUF Regeneration
 *		- XPUF_ERROR_INVALID_PARAM if PufData is NULL
 *		- XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT  Timeout occurred while
 *		waiting for PUF done bit
 *		- XPUF_ERROR_PUF_DONE_KEY_NT_RDY  Key ready bit and ID ready
 *		bit is not set
 *		- XPUF_ERROR_PUF_DONE_ID_NT_RDY  Id ready bit is not set
 *
 *****************************************************************************/
int XPuf_Regeneration(XPuf_Data *PufData)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;

	Status = XPuf_Cfg(PufData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	/**
	 * Run PUF regeneration through iterative convergence
	 */
	Status = XPuf_GeneratePufKey(PufData);

END:
	SStatus = XPuf_ResetPuf();
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function clears PUF ID
 *
 * @return
 * 		 - XST_SUCCESS if PUF ID is cleared successfully
 * 		 - XPUF_ERROR_PUF_ID_ZERO_TIMEOUT if timedout while clearing PUF ID
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
	WaitStatus = (int)Xil_WaitForEvent((UINTPTR)(XPUF_BASEADDR +
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

/******************************************************************************/
/**
 * @brief	This function will generate the Puf key form Aux data, Chash and
 * 		syndrome data
 *
 * @param	PufAuxData - AUX Data
 * @param	VarPufChash - CHASH Data
 * @param	SyndromeData - Pointer to the syndrome data
 *
 * @return
 *	-	@ref XST_FAILURE - Upon Failure
 *	-	@ref XROM_S7_PUF_REGEN_WR_TO_ERROR
 *	-	@ref XROM_S7_PUF_REGEN_KR_TO_ERROR
 *	-	@ref XROM_S7_PUF_REGEN_KEY_NOT_CONVERGED_ERROR
 *	-	@ref XST_SUCCESS - If PUF Key generated successfully
 ******************************************************************************/
static int XPuf_GeneratePufKey(XPuf_Data *PufData) {
	u32 Index;
	u32 SyndromeIndex = 0U;
	volatile u32 VarPufStatus = 0U;
	volatile u32 Status = (u32)XST_FAILURE;
	u32 *SynData;
	u32 SynDataTmp;
	u32 SynDataSize;

	SynDataSize = XPUF_4K_PUF_TOT_SYN_LEN_IN_WORDS;
	SynData = (u32*)(UINTPTR)PufData->SyndromeAddr;

	for (Index = 0U;Index < XPUF_KEY_GEN_ITERATIONS;Index++) {
		SyndromeIndex = 0U;
		/*
		 * Start the PUF in Regeneration mode
		 */
		Xil_Out32(XPUF_BASEADDR + XPUF_PUF_WORD_OFFSET, (PufData->Aux << XPUF_AUX_SHIFT_VALUE)| XPUF_PUF_AUX_ENABLE);
		Xil_Out32(XPUF_BASEADDR + XPUF_PUF_CHASH_OFFSET, PufData->Chash);
		Xil_Out32(XPUF_BASEADDR + XPUF_PUF_CMD_OFFSET, XPUF_CMD_REGEN_ON_DEMAND);
		VarPufStatus = Xil_In32(XPUF_BASEADDR + XPUF_PUF_STATUS_OFFSET);
		/*
		 * Check for Key ready
		 */
		/* Indicates that the key is ready. Clears on read or CMD = STOP */
		while ((VarPufStatus & XPUF_STATUS_KEY_RDY)
					!= XPUF_STATUS_KEY_RDY) {

			/* Indicates that the syndrome word is ready. Clears on read or CMD = STOP */
			Status = XPuf_WaitForPufSynWordRdy();
			if (Status !=XST_SUCCESS) {
				Status = XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT;
				goto END;
			}
			/**
			* Read the Debug date and discard
			*/
			VarPufStatus = Xil_In32(XPUF_BASEADDR + XPUF_PUF_STATUS_OFFSET);
			if ((VarPufStatus & XPUF_STATUS_SEG_RDY) == XPUF_STATUS_SEG_RDY) {
				/**
				 * Read the Debug date and discard
				*/
				(void) Xil_In32(XPUF_BASEADDR + XPUF_PUF_DBG_OFFSET);
			}

			if (SyndromeIndex < SynDataSize) {
				SynDataTmp = SynData[SyndromeIndex];
				Xil_Out32(XPUF_BASEADDR + XPUF_PUF_WORD_OFFSET, SynDataTmp);
				SyndromeIndex++;
			}

			usleep(1U);
		}
		if ((VarPufStatus & XPUF_PUF_IC_MASK) == XPUF_PUF_IC_MASK) {
			break;
		}

		if (Index != (XPUF_KEY_GEN_ITERATIONS - 1U)) {
			Xil_Out32(XPUF_BASEADDR + XPUF_PUF_CMD_OFFSET, XPUF_CMD_PAUSE);
		}
	}

	/*
	 * Read the last Debug 2 Word
	 */
	(void) Xil_In32(XPUF_BASEADDR + XPUF_PUF_DBG_OFFSET);

	/*
	 * 	Request to capture Key & ID.
	 */
	Status = XPUF_ERROR_KEY_NOT_CONVERGED;
	if ((VarPufStatus & XPUF_PUF_IC_MASK) == XPUF_PUF_IC_MASK) {
		XPuf_CapturePufID(PufData);
		Xil_Out32(XPUF_PMC_GLOBAL_BASEADDR + XPUF_PMC_GLOBAL_PMC_PUF_CAPTURE_OFFSET,
			(XPUF_PMC_GLOBAL_PUF_KEY_CAPTURE |
			XPUF_PMC_GLOBAL_PUF_ID_CAPTURE));
		Status = XST_SUCCESS;
	}

END:
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
 *
 * @brief	Converts the PUF Syndrome data to eFUSE writing format
 *
 *
 * @param	PufData Pointer to XPuf_Data structure which includes options
 *		to configure PUF.
 *
 * @return
 *		- XST_SUCCESS  Syndrome data is successfully trimmed
 *		- XPUF_ERROR_INVALID_PARAM  PufData instance pointer is NULL
 *
 * @note
 *		Formatted data will be available at PufData->TrimmedSynData
 *
 ******************************************************************************/
int XPuf_TrimPufData(XPuf_Data *PufData)
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
	 *
	 * Space allocated in eFUSE for syndrome data = 4060bits
	 *
	 * eFUSE02 - 2032bits
	 *
	 * eFUSE03 - 2028bits
	 *
	 * PUF Helper data generated for 4K Mode through registration
	 * is 140 Words = 140*32 = 4480 bits.
	 * Remove lower 12 bits of every fourth word of syndrome data.
	 *
	 * After removing these bits remaining syndrome data will be
	 * exactly 4060bits which will fit into eFUSE.
	 *
	 *
	 * Illustration:
	 * -----
	 * 454D025B
	 *
	 * CDCB36FC
	 *
	 * EE1FE4C5
	 *
	 * 3FE53F74 --> F74 has to removed & next word upper 12 bits have to be shifted here
	 *
	 * 3A0AE7F8
	 *
	 * 2373F03A
	 *
	 * C83188AF
	 *
	 * 3A5EB687--> 687 has to be removed
	 *
	 * B83E4A1D
	 *
	 * D53B5C50
	 *
	 * FA8B33D9
	 *
	 * 07EEFF43 --> F43 has to be removed
	 *
	 * CD01973F
	 *
	 * ........
	 *
	 * ........
	 *
	 * ........
	 */

	for (Index = 0U; Index < 5U; Index++) {
		for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
			if (SubIndex == 3U) {
				PufData->TrimmedSynData[DIndex] =
				(SynData[SIndex] & XPUF_EFUSE_TRIM_MASK) |
				(SynData[SIndex + 1U] >> 20U);
			}
			else {
				PufData->TrimmedSynData[DIndex] =
							SynData[SIndex];
			}
			SIndex++;
			DIndex++;
		}

		for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
			if (SubIndex == 3U) {
				PufData->TrimmedSynData[DIndex] =
					(((SynData[SIndex] &
					XPUF_EFUSE_TRIM_MASK) << 12U) |
						(SynData[SIndex + 1U] >> 8U));
			}
			else {
				PufData->TrimmedSynData[DIndex] =
				((SynData[SIndex] << 12U) |
						(SynData[SIndex + 1U] >> 20U));
			}
			SIndex++;
			DIndex++;
		}

		for (SubIndex = 0U; SubIndex < 3U; SubIndex++) {
			if (SubIndex == 2U) {
				PufData->TrimmedSynData[DIndex] =
					((SynData[SIndex] << 24U) |
					((SynData[SIndex + 1U] &
						XPUF_EFUSE_TRIM_MASK) >> 8U));
				if (DIndex < (XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS - 1U)) {
					PufData->TrimmedSynData[DIndex] |=
						(SynData[SIndex + 2U] >> 28U);
				}
			}
			else {
				PufData->TrimmedSynData[DIndex]=
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
					PufData->TrimmedSynData[DIndex] =
						(((SynData[SIndex] &
					XPUF_EFUSE_TRIM_MASK) << 4U) |
						(SynData[SIndex + 1U] >> 16U));

				}
				else {
					PufData->TrimmedSynData[DIndex] =
						((SynData[SIndex] << 4U) |
						(SynData[SIndex + 1U] >> 28U));
				}
				SIndex++;
				DIndex++;
			}

			for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
				if(SubIndex == 3U) {
					PufData->TrimmedSynData[DIndex] =
						(((SynData[SIndex] &
					XPUF_EFUSE_TRIM_MASK) << 16U) |
						(SynData[SIndex + 1U] >> 4U));

				}
				else {
					PufData->TrimmedSynData[DIndex]=
						((SynData[SIndex] << 16U) |
						(SynData[SIndex + 1U] >> 16U));
				}
				SIndex++;
				DIndex++;
			}

			for (SubIndex = 0U; SubIndex < 3U; SubIndex++) {
				if (SubIndex == 2U) {
					PufData->TrimmedSynData[DIndex] =
						((SynData[SIndex] << 28U) |
						((SynData[SIndex + 1U] &
						XPUF_EFUSE_TRIM_MASK) >> 4U));
					PufData->TrimmedSynData[DIndex] |=
						(SynData[SIndex + 2U] >> 24U);
				}
				else {
					PufData->TrimmedSynData[DIndex]=
						((SynData[SIndex] << 28U) |
						(SynData[SIndex + 1U] >> 4U));
				}
				SIndex++;
				DIndex++;
			}
			SIndex++;

			for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
				if (SubIndex == 3U) {
					PufData->TrimmedSynData[DIndex] =
						(((SynData[SIndex] &
						XPUF_EFUSE_TRIM_MASK) << 8U) |
						(SynData[SIndex + 1U] >> 12U));

				}
				else {
					PufData->TrimmedSynData[DIndex] =
						((SynData[SIndex] << 8U) |
						(SynData[SIndex + 1U] >> 24U));
				}
				SIndex++;
				DIndex++;
			}

			for (SubIndex = 0U; SubIndex < 3U; SubIndex++) {
				if (SubIndex == 2U) {
					PufData->TrimmedSynData[DIndex] =
						((SynData[SIndex] << 20U) |
						((SynData[SIndex + 1U] &
					XPUF_EFUSE_TRIM_MASK) >> 12U));

				}
				else {
					PufData->TrimmedSynData[DIndex]=
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
	 * Use the above mentioned logic to trim the data and copy the trimmed data in TrimmedSynData array in the instance pointer.
	 * and return XST_SUCCESS.
	 */
	PufData->TrimmedSynData[XPUF_LAST_WORD_OFFSET] &=
						XPUF_LAST_WORD_MASK;
	Status = XST_SUCCESS;

END:
	return Status;
}
