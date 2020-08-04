/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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
*		am 	 08/04/2020 Resolved MISRA C Violations
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
#include "xil_mem.h"

/************************** Constant Definitions *****************************/
#define XPUF_STATUS_WAIT_TIMEOUT		(50000U)
						/* 50ms */

typedef enum {
	XPUF_REGISTRATION_STARTED,
	XPUF_REGISTRATION_COMPLETE
} XPuf_PufRegistrationState;

/********************Macros (Inline function) Definitions*********************/
/*****************************************************************************/
/**
 * @brief
 * This function waits till Puf Syndrome rdy bit is set.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS - Syndrome word is ready
 *		XST_FAILURE - Timeout occurred
 *
 *****************************************************************************/
static inline int XPuf_WaitForPufSynWordRdy()
{
	return (int)Xil_WaitForEvent((XPUF_PMC_GLOBAL_BASEADDR +
		XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET),
		XPUF_STATUS_SYNDROME_WORD_RDY, XPUF_STATUS_SYNDROME_WORD_RDY,
		XPUF_STATUS_WAIT_TIMEOUT);
}

/*****************************************************************************/
/**
 * @brief
 * This function waits till Puf done bit is set.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS - Puf Operation is done.
 *		XST_FAILURE - Timeout occurred
 *
 *****************************************************************************/
static inline int XPuf_WaitForPufDoneStatus()
{
	return (int)Xil_WaitForEvent((XPUF_PMC_GLOBAL_BASEADDR +
		XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET), XPUF_STATUS_PUF_DONE,
		XPUF_STATUS_PUF_DONE, XPUF_STATUS_WAIT_TIMEOUT);
}

/*****************************************************************************/
/**
 *
 * This function reads the given register.
 *
 * @param	BaseAddress is the Xilinx base address of the eFuse or Bbram
 *		controller.
 * @param	RegOffset is the register offset of the register.
 *
 * @return	The 32-bit value of the register.
 *
 * @note	C-style signature:
 * 		u32 XilPuf_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
 * ***************************************************************************/
static inline u32 XPuf_ReadReg(u32 BaseAddress, u32 RegOffset)
{
	return Xil_In32(BaseAddress + RegOffset);
}

/*****************************************************************************/
/**
 *
 * This function writes the value into the given register.
 *
 * @param	BaseAddress is the Xilinx base address of the eFuse or Bbram
 *		controller.
 * @param	RegOffset is the register offset of the register.
 * @param	Data is the 32-bit value to write to the register.
 *
 * @return	None.
 *
 * @note	C-style signature:
 * 		void XilPuf_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
 *****************************************************************************/
static inline void XPuf_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
{
	Xil_Out32(BaseAddress + RegOffset, Data);
}

/************************** Function Prototypes ******************************/
static void XPuf_CapturePufID(XPuf_Data *PufData);
static int XPuf_ValidateAccessRules(const XPuf_Data *PufData);
void XPuf_GenerateFuseFormat(XPuf_Data *PufData);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief
 * This functions performs PUF registration
 *
 * @param	PufData - Pointer to XPuf_Data structure
 *
 * @return	XST_SUCCESS - PUF registration successful
 *		XPUF_ERROR_INVALID_PARAM - PufData is NULL
 *		XPUF_ERROR_INVALID_SYNDROME_MODE - Incorrect Registration mode
 *		XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT - Timeout occurred while
 *			waiting for PUF Syndrome data
 *		XPUF_ERROR_SYNDROME_DATA_OVERFLOW - Syndrome data overflow
 *			reported by PUF controller or more than required data
 *			is provided by PUF controller
 *		XPUF_ERROR_SYNDROME_DATA_UNDERFLOW - Number of syndrome data
 *			words are less than expected number of words
 *		XST_FAILURE - Unexpected event
 *
 * @note	Helper data will be available in PufData->SyndromeData,
 *		PufData->Chash, PufData->Aux
 *
 *****************************************************************************/
int XPuf_Registration(XPuf_Data *PufData)
{
	int Status = XST_FAILURE;
	u32 MaxSyndromeSizeInWords;
	u32 Idx = 0;
	XPuf_PufRegistrationState RegistrationStatus;

	if (PufData == NULL) {
		Status = XPUF_ERROR_INVALID_PARAM;
		goto END;
	}

	Status =  XPuf_ValidateAccessRules(PufData);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_CFG0_OFFSET,
			XPUF_CFG0_HASH_SEL);

	if (XPUF_SYNDROME_MODE_4K == PufData->RegMode) {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
				XPUF_PMC_GLOBAL_PUF_CFG1_OFFSET,
				XPUF_CFG1_INIT_VAL_4K);
		MaxSyndromeSizeInWords = XPUF_4K_PUF_SYN_LEN_IN_WORDS;
	}
	else if (XPUF_SYNDROME_MODE_12K == PufData->RegMode) {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
				XPUF_PMC_GLOBAL_PUF_CFG1_OFFSET,
				XPUF_CFG1_INIT_VAL_12K);
		MaxSyndromeSizeInWords = XPUF_12K_PUF_SYN_LEN_IN_WORDS;
	}
	else {
		Status = XPUF_ERROR_INVALID_SYNDROME_MODE;
		goto END;
	}

	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_SHUT_OFFSET,
			PufData->ShutterValue);

	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_CMD_OFFSET,
			XPUF_CMD_REGISTRATION);

	RegistrationStatus = XPUF_REGISTRATION_STARTED;

	while (RegistrationStatus != XPUF_REGISTRATION_COMPLETE) {
		Status = XPuf_WaitForPufSynWordRdy(PufData);
		if (Status != XST_SUCCESS) {
			Status = XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT;
			break;
		}

		PufData->SyndromeData[Idx] = XPuf_ReadReg(
					XPUF_PMC_GLOBAL_BASEADDR,
					XPUF_PMC_GLOBAL_PUF_WORD_OFFSET);

		if (Idx == (MaxSyndromeSizeInWords -1U)) {
			Status  = XPuf_WaitForPufDoneStatus(PufData);
			if (Status != XST_SUCCESS) {
				Status = XPUF_ERROR_PUF_DONE_WAIT_TIMEOUT;
				break;
			}
			RegistrationStatus = XPUF_REGISTRATION_COMPLETE;

			PufData->Chash = XPuf_ReadReg(
					XPUF_PMC_GLOBAL_BASEADDR,
					XPUF_PMC_GLOBAL_PUF_CHASH_OFFSET);
			PufData->Aux = XPuf_ReadReg(
					XPUF_PMC_GLOBAL_BASEADDR,
					XPUF_PMC_GLOBAL_PUF_AUX_OFFSET);

			XPuf_CapturePufID(PufData);

			Status = XST_SUCCESS;
		}
		Idx++;
	};
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function regenerate PUF data using helper data stored in eFUSE or in
 * and external memory.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS - PUF Regeneration successful
 *		XPUF_ERROR_INVALID_REGENERATION_TYPE -Selection of invalid
 *			 regeneration type
 *		XPUF_ERROR_CHASH_NOT_PROGRAMMED- Helper data not provided
 *		XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT - Timeout before Status was
 *			 done
 *		XST_FAILURE - Unexpected event
 *
 * @note	None.
 *
 *****************************************************************************/
int XPuf_Regeneration(XPuf_Data *PufData)
{
	int Status = XST_FAILURE;
	u32 PufChash;
	u32 PufAux;
	u32 PufStatus;
	u32 Debug = XPUF_DEBUG_GENERAL;

	if (PufData == NULL) {
		Status = XPUF_ERROR_INVALID_PARAM;
		goto END;
        }

	Status =  XPuf_ValidateAccessRules(PufData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (PufData->ReadOption == XPUF_READ_FROM_RAM) {
		PufChash = PufData->Chash;
		PufAux = PufData->Aux;
		if ((PufChash == 0U) || (PufAux == 0U)) {
			Status = XPUF_ERROR_CHASH_NOT_PROGRAMMED;
			xPuf_printf(Debug, "PUF regeneration is not allowed"
			", as PUF helper data is not provided/stored\r\n");
			goto END;
		}
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_AUX_OFFSET,
			PufAux);
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_CHASH_OFFSET,
			PufChash);
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_SYN_ADDR_OFFSET,
			PufData->SyndromeAddr);
	}
	else if (PufData->ReadOption == XPUF_READ_FROM_EFUSE_CACHE) {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_SYN_ADDR_OFFSET,
			XPUF_EFUSE_SYN_ADD_INIT);
	}
	else {
		Status = XPUF_ERROR_INVALID_READ_HD_INPUT;
		goto END;
	}

	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR, XPUF_PMC_GLOBAL_PUF_CFG0_OFFSET,
			XPUF_CFG0_HASH_SEL);

	if (XPUF_SYNDROME_MODE_4K == PufData->RegMode) {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_CFG1_OFFSET,
			XPUF_CFG1_INIT_VAL_4K);
	}
	else {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_CFG1_OFFSET,
			XPUF_CFG1_INIT_VAL_12K);
	}

	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_SHUT_OFFSET,
			PufData->ShutterValue);

	if(XPUF_REGEN_ON_DEMAND == PufData->PufOperation) {
		 XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_CMD_OFFSET,
			XPUF_CMD_REGEN_ON_DEMAND);
	}
	else if(XPUF_REGEN_ID_ONLY == PufData->PufOperation) {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_CMD_OFFSET,
			XPUF_CMD_REGEN_ID_ONLY);
	}
	else {
		Status = XPUF_ERROR_INVALID_REGENERATION_TYPE;
		goto END;
	}

	Status  = XPuf_WaitForPufDoneStatus(PufData);
	if (Status != XST_SUCCESS) {
		xPuf_printf(Debug,
		"Error: Puf Regeneration failed!! \r\n");
		Status = XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT;
		goto END;
	}

	PufStatus = XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
				XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET);

	if(XPUF_REGEN_ON_DEMAND == PufData->PufOperation) {
		if (((PufStatus & XPUF_STATUS_KEY_RDY) == XPUF_STATUS_KEY_RDY) &&
		((PufStatus & XPUF_STATUS_ID_RDY) == XPUF_STATUS_ID_RDY)) {
			XPuf_CapturePufID(PufData);
		}
	}
	else {
		/* PufData->PufOperation = XPUF_REGEN_ID_ONLY */
		if ((PufStatus & XPUF_STATUS_ID_RDY) == XPUF_STATUS_ID_RDY) {
			XPuf_CapturePufID(PufData);
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function captures PUF ID generated into XPuf_Data.
 *
 *
 * @param	PufData - Pointer to XPuf_Data structure
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
static void XPuf_CapturePufID(XPuf_Data *PufData)
{
	u32 Index;

	for (Index = 0U; Index < XPUF_ID_LEN_IN_WORDS; Index++) {
		PufData->PufID[Index] = XPuf_ReadReg(
			XPUF_PMC_GLOBAL_BASEADDR,
			(XPUF_PMC_GLOBAL_PUF_ID_0_OFFSET +
			(Index * XPUF_WORD_LENGTH)));
	}
}

/*****************************************************************************/
/**
 * @brief
 * This function validates if secure control bits for each PUF operations are
 * set or not.
 *
 *
 * @param	PufData - Pointer to XPuf_Data structure
 *
 * @return	XST_SUCCESS - secure control bits are not set
 * 		XST_FAILURE - secure control bits are set
 *
 *****************************************************************************/
static int XPuf_ValidateAccessRules(const XPuf_Data *PufData)
{
	int Status = XST_FAILURE;
	u32 Operation = PufData->PufOperation;
	u32 Puf_Ecc_Puf_Ctrl_Value = XPuf_ReadReg(XPUF_EFUSE_CACHE_BASEADDR,
					XPUF_PUF_ECC_PUF_CTRL_OFFSET);
	u32 Security_Control_Value = XPuf_ReadReg(XPUF_EFUSE_CACHE_BASEADDR,
					XPUF_EFUSE_CACHE_SECURITY_CONTROL);

	switch (Operation) {

		/* For Registration */
		case XPUF_REGISTRATION:
			if ((Security_Control_Value & XPUF_PUF_DIS) == XPUF_PUF_DIS) {
				Status = XPUF_ERROR_REGISTRATION_INVALID;
			}
			else {
				Status = XST_SUCCESS;
			}
			break;

		/* For Regeneration */
		case XPUF_REGEN_ON_DEMAND:
		case XPUF_REGEN_ID_ONLY:
			if (((Security_Control_Value & XPUF_PUF_DIS) == XPUF_PUF_DIS) &&
				((Puf_Ecc_Puf_Ctrl_Value & XPUF_PUF_REGEN_DIS) == XPUF_PUF_REGEN_DIS)) {
				Status = XPUF_ERROR_REGENERATION_INVALID;
			}
			else if((PufData->ReadOption == XPUF_READ_FROM_EFUSE_CACHE) &&
				((Puf_Ecc_Puf_Ctrl_Value & XPUF_PUF_HD_INVLD) == XPUF_PUF_HD_INVLD)) {
				Status = XPUF_ERROR_REGEN_PUF_HD_INVALID;
			}
			else {
				Status = XST_SUCCESS;
			}
			break;
		default:
			Status = XST_FAILURE;
			break;

	}
	return Status;
}

/*****************************************************************************/
/**
 *
 * Converts the PUF Syndrome data to eFUSE writing format.
 *
 * @param	PufData - Pointer to XPuf_Data structure
 *
 * @return	none
 *
 * @note	Formatted data will be available at PufData->EfuseSynData.
 *
 ******************************************************************************/
void XPuf_GenerateFuseFormat(XPuf_Data *PufData)
{

	u32 SynData[XPUF_4K_PUF_SYN_LEN_IN_WORDS] = {0U};
	u32 SIndex = 0U;
	u32 DIndex = 0U;
	u32 Index;
	u32 SubIndex;

	Xil_MemCpy(SynData, PufData->SyndromeData,
		(u32)(XPUF_4K_PUF_SYN_LEN_IN_WORDS * sizeof(u32)));

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
				(SynData[SIndex+1U] >> 20U);
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
						SynData[SIndex+1U] >> 8U);
			}
			else {
				PufData->EfuseSynData[DIndex] =
				((SynData[SIndex] << 12U) |
						SynData[SIndex+1U] >> 20U);
			}
			SIndex++;
			DIndex++;
		}

		for (SubIndex = 0U; SubIndex < 3U; SubIndex++) {
			if (SubIndex == 2U) {
				PufData->EfuseSynData[DIndex] =
					((SynData[SIndex] << 24U) |
					(SynData[SIndex+1U] &
						XPUF_EFUSE_TRIM_MASK) >> 8U);
				if (DIndex < (XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS - 1U)) {
					PufData->EfuseSynData[DIndex] |=
						(SynData[SIndex+2U] >> 28U);
				}
			}
			else {
				PufData->EfuseSynData[DIndex]=
					((SynData[SIndex] << 24U) |
						SynData[SIndex+1U] >> 8U);
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
						SynData[SIndex+1U] >> 16U);

				}
				else {
					PufData->EfuseSynData[DIndex] =
						((SynData[SIndex] << 4U) |
						SynData[SIndex+1U] >> 28U);
				}
				SIndex++;
				DIndex++;
			}

			for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
				if(SubIndex == 3U) {
					PufData->EfuseSynData[DIndex] =
						(((SynData[SIndex] &
					XPUF_EFUSE_TRIM_MASK) << 16U) |
						SynData[SIndex+1U] >> 4U);

				}
				else {
					PufData->EfuseSynData[DIndex]=
						((SynData[SIndex] << 16U) |
						SynData[SIndex+1U] >> 16U);
				}
				SIndex++;
				DIndex++;
			}

			for (SubIndex = 0U; SubIndex < 3U; SubIndex++) {
				if (SubIndex == 2U) {
					PufData->EfuseSynData[DIndex] =
						((SynData[SIndex] << 28U) |
						(SynData[SIndex+1U] &
						XPUF_EFUSE_TRIM_MASK) >> 4U);
					PufData->EfuseSynData[DIndex] |=
						(SynData[SIndex+2U] >> 24U);
				}
				else {
					PufData->EfuseSynData[DIndex]=
						((SynData[SIndex] << 28U) |
						SynData[SIndex+1U] >> 4U);
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
						SynData[SIndex+1U] >> 12U);

				}
				else {
					PufData->EfuseSynData[DIndex] =
						((SynData[SIndex] << 8U) |
						SynData[SIndex+1U] >> 24U);
				}
				SIndex++;
				DIndex++;
			}

			for (SubIndex = 0U; SubIndex < 3U; SubIndex++) {
				if (SubIndex == 2U) {
					PufData->EfuseSynData[DIndex] =
						((SynData[SIndex] << 20U) |
						(SynData[SIndex+1U] &
					XPUF_EFUSE_TRIM_MASK) >> 12U);

				}
				else {
					PufData->EfuseSynData[DIndex]=
						((SynData[SIndex] << 20U) |
						SynData[SIndex+1U] >> 12U);
				}
				SIndex++;
				DIndex++;
			}
			SIndex++;
		}
	}
	PufData->EfuseSynData[XPUF_LAST_WORD_OFFSET] &=
						XPUF_LAST_WORD_MASK;
}
