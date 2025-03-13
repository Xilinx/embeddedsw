/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtpm.c
*
* This is the file which contains TPM related code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  tri  03/13/25 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
/**
 * @addtogroup xtpm_apis XilTpm APIs
 * @{
 */
/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_TPM
#include "xil_util.h"
#include "xtpm.h"
#include "xplmi_hw.h"
#include "xtpm_hw.h"
#include "xtpm_error.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static u32 XTpm_StartUp(void);
static u32 XTpm_SelfTest(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function initializes TPM, starts up TPM and performs self test.
 *
 * @return
 * 			- XST_SUCCESS if successful, else error code
 *
 ******************************************************************************/
u32 XTpm_Init(void)
{
	int Status = XST_FAILURE;
	u8 Access = 0U;

	Status = XTpm_SpiInit();
	if (Status != XST_SUCCESS) {
		Status = XTPM_ERR_SPIPS_INIT;
		goto END;
	}

	/* Set access request to use */
	Status = XTpm_AccessSet(XTPM_ACCESS_REQ_USE);
	if (Status != XST_SUCCESS) {
		Status = XTPM_ERR_SET_ACCESS;
		goto END;
	}

	do {
		/* Check for access valid and locality */
		Status = XTpm_AccessGet(&Access);
		if (Status != XST_SUCCESS) {
			Status = XTPM_ERR_GET_ACCESS;
			goto END;
		}
	} while (!(((Access & XTPM_ACCESS_VALID) != 0U)
			&& ((Access & XTPM_ACCESS_ACT_LOCAL) !=0U)));

	Status = XTpm_StartUp();
	if (Status != XST_SUCCESS) {
		Status = XTPM_ERR_START_UP;
		goto END;
	}

	Status = XTpm_SelfTest();
	if (Status != XST_SUCCESS) {
		Status = XTPM_ERR_SELF_TEST;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function does a startup of TPM.
 *
 * @return
 * 			- XST_SUCCESS if successful, else error code
 *
 ******************************************************************************/
static u32 XTpm_StartUp(void)
{
	int Status = XST_FAILURE;
	u8 TpmRespBuffer[XTPM_RESP_MAX_SIZE] = {0U};
	u8 TpmStartup[XTPM_START_CMD_SIZE] = {
		0x80U, 0x01U, /* TPM_ST_NO_SESSIONS */
		0x00U, 0x00U, 0x00U, 0x0CU, /* Command Size */
		0x00U, 0x00U, 0x01U, 0x44U, /* TPM_CC_Startup */
		0x00U, 0x00U /* TPM_SU_CLEAR */
	};

	Status = XTpm_DataTransfer(TpmStartup, TpmRespBuffer,
		TpmStartup[XTPM_DATA_SIZE_INDEX]);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function does a self test of TPM.
 *
 * @return
 * 			- XST_SUCCESS if successful, else error code
 *
 ******************************************************************************/
static u32 XTpm_SelfTest(void)
{
	int Status = XST_FAILURE;
	u8 TpmRespBuffer[XTPM_RESP_MAX_SIZE] = {0U};
	u8 TpmSelfTest[XTPM_SELF_TEST_CMD_SIZE] = {
		0x80U, 0x01U, /* TPM_ST_NO_SESSIONS */
		0x00U, 0x00U, 0x00, 0x0CU, /* Command Size */
		0x00U, 0x00U, 0x01U, 0x43U, /* TPM_CC_Selftest */
		0x00U, 0x00U /* TPM_SU_CLEAR */
	};

	Status = XTpm_DataTransfer(TpmSelfTest, TpmRespBuffer,
		TpmSelfTest[XTPM_DATA_SIZE_INDEX]);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends ROM digest stored in PMC registers to TPM.
 *
 * @return
 * 			- XST_SUCCESS if successful, else error code
 *
 ******************************************************************************/
int XTpm_MeasureRom(void)
{
	volatile int Status = XTPM_ERR_MEASURE_ROM;
	u32 ShaData[XTPM_HASH_TYPE_SHA3 / XTPM_DATA_WORD_LENGTH];
	u8 Index = 0U;
	u32 RegVal;
	u8 TpmPcrExtend[XTPM_PCR_EXT_CMD_SIZE + XTPM_DIGEST_SIZE] =
	{
		0x80U, 0x02U, /* TPM_ST_SESSIONS */
		0x00U, 0x00U, 0x00U, 0x00U, /* Command Size */
		0x00U, 0x00U, 0x01U, 0x82U, /* TPM_CC_PCR_Extend */
		0x00U, 0x00U, 0x00U, 0x00U, /* PCR_Index */
		0x00U, 0x00U, /* NULL Password */
		0x00U, 0x09U, /* Authorization Size */
		0x40U, 0x00U, 0x00U, 0x09U, /* Password authorization session */
		0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U, /* TPML_DIGEST_VALUES */
		0x00U, 0x0BU /* SHA2 Hash Algorithm */
	};
	u8 TpmRespBuffer[XTPM_RESP_MAX_SIZE] = {0U};

	/* Re-order the data to match LSB first */
	for (RegVal = PMC_GLOBAL_ROM_VALIDATION_DIGEST_11; RegVal >= PMC_GLOBAL_ROM_VALIDATION_DIGEST_0;
		RegVal -= sizeof(u32)) {
		ShaData[Index] = Xil_In32(RegVal);
		Index++;
	}

	TpmPcrExtend[XTPM_DATA_SIZE_INDEX] = XTPM_PCR_EXT_CMD_SIZE +
		XTPM_DIGEST_SIZE;
	(void)Xil_SMemCpy(&TpmPcrExtend[XTPM_PCR_EXT_CMD_SIZE], XTPM_DIGEST_SIZE, (u8 *)ShaData,
		XTPM_DIGEST_SIZE, XTPM_DIGEST_SIZE);

	Status = (int)XTpm_Event(XTPM_TPM_ROM_PCR_INDEX, XTPM_HASH_TYPE_SHA3, (u8 *)ShaData, TpmRespBuffer);
	if ((TpmRespBuffer[6U] != 0U) || (TpmRespBuffer[7U] != 0U) ||
		(TpmRespBuffer[8U] != 0U) || (TpmRespBuffer[9U] != 0U)) {
		if (Status != XST_SUCCESS) {
			Status = XTPM_ERR_RESP_POLLING;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends PLM FW digest stored in PMC registers to TPM.
 *
 * @return
 * 			- XST_SUCCESS if successful, else error code
 *
 ******************************************************************************/
int XTpm_MeasurePlm(void)
{
	volatile int Status = XTPM_ERR_MEASURE_PLM;
	u32 ShaData[XTPM_HASH_TYPE_SHA3 / XTPM_DATA_WORD_LENGTH];
	u8 Index = 0U;
	u32 RegVal;
	u8 TpmPcrExtend[XTPM_PCR_EXT_CMD_SIZE + XTPM_DIGEST_SIZE] =
	{
		0x80U, 0x02U, /* TPM_ST_SESSIONS */
		0x00U, 0x00U, 0x00U, 0x00U, /* Command Size */
		0x00U, 0x00U, 0x01U, 0x82U, /* TPM_CC_PCR_Extend */
		0x00U, 0x00U, 0x00U, 0x01U, /* PCR_Index */
		0x00U, 0x00U, /* NULL Password */
		0x00U, 0x09U, /* Authorization Size */
		0x40U, 0x00U, 0x00U, 0x09U, /* Password authorization session */
		0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U, /* TPML_DIGEST_VALUES */
		0x00U, 0x0BU /* SHA2 Hash Algorithm */
	};
	u8 TpmRespBuffer[XTPM_RESP_MAX_SIZE] = {0U};

	/* Re-order the data to match LSB first */
	for (RegVal = PMC_GLOBAL_FW_AUTH_HASH_11 ; RegVal >= PMC_GLOBAL_FW_AUTH_HASH_0;
		RegVal -= sizeof(u32)) {
		ShaData[Index] = Xil_In32(RegVal);
		Index++;
	}

	TpmPcrExtend[XTPM_DATA_SIZE_INDEX] = XTPM_PCR_EXT_CMD_SIZE +
		XTPM_DIGEST_SIZE;
	(void)Xil_SMemCpy(&TpmPcrExtend[XTPM_PCR_EXT_CMD_SIZE], XTPM_DIGEST_SIZE, (u8 *)ShaData,
		XTPM_DIGEST_SIZE, XTPM_DIGEST_SIZE);

	Status = XTpm_Event(XTPM_TPM_PLM_PCR_INDEX, XTPM_HASH_TYPE_SHA3, (u8 *)ShaData, TpmRespBuffer);
	if ((TpmRespBuffer[6U] != 0U) || (TpmRespBuffer[7U] != 0U) ||
		(TpmRespBuffer[8U] != 0U) || (TpmRespBuffer[9U] != 0U)) {
		if (Status != XST_SUCCESS) {
			Status = XTPM_ERR_RESP_POLLING;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends SHA3 digest to TPM.
 *
 * @param	PcrIndex is the PCR register to send
 * @param	ImageHash is pointer to SHA3 digest
 *
 * @return
 * 			- XST_SUCCESS if successful, else error code
 *
 ******************************************************************************/
int XTpm_MeasurePartition(u32 PcrIndex, u8* ImageHash)
{
	int Status = XTPM_ERR_MEASURE_PARTITION;
	u8 TpmPcrExtend[XTPM_PCR_EXT_CMD_SIZE + XTPM_DIGEST_SIZE] =
	{
		0x80U, 0x02U, /* TPM_ST_SESSIONS */
		0x00U, 0x00U, 0x00U, 0x00U, /* Command Size */
		0x00U, 0x00U, 0x01U, 0x82U, /* TPM_CC_PCR_Extend */
		0x00U, 0x00U, 0x00U, 0x00U, /* PCR_Index */
		0x00U, 0x00U, /* NULL Password */
		0x00U, 0x09U, /* Authorization Size */
		0x40U, 0x00U, 0x00U, 0x09U, /* Password authorization session */
		0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U, /* TPML_DIGEST_VALUES */
		0x00U, 0x0BU /* SHA2 Hash Algorithm */
	};
	u8 TpmRespBuffer[XTPM_RESP_MAX_SIZE + XTPM_TX_HEAD_SIZE] =
		{0U};
	u8 Index;

	for (Index = 0U; Index < XTPM_HASH_TYPE_SHA3; ++Index) {
		XPlmi_Printf(DEBUG_INFO, "%02x", ImageHash[Index]);
	}
	XPlmi_Printf(DEBUG_INFO, "\n\r");

	TpmPcrExtend[XTPM_DATA_SIZE_INDEX] = XTPM_PCR_EXT_CMD_SIZE +
		XTPM_DIGEST_SIZE;
	TpmPcrExtend[XTPM_PCR_EXTEND_INDEX] = (u8)PcrIndex;
	(void)Xil_SMemCpy((void *)&TpmPcrExtend[XTPM_PCR_EXT_CMD_SIZE], XTPM_DIGEST_SIZE,
		(void *)ImageHash, XTPM_DIGEST_SIZE, XTPM_DIGEST_SIZE);

	Status = (int)XTpm_Event(PcrIndex, XTPM_HASH_TYPE_SHA3, (u8 *)ImageHash, TpmRespBuffer);
	if ((TpmRespBuffer[6U] != 0U) || (TpmRespBuffer[7U] != 0U) ||
		(TpmRespBuffer[8U] != 0U) || (TpmRespBuffer[9U] != 0U)) {
		if (Status != XST_SUCCESS) {
			Status = XTPM_ERR_RESP_POLLING;
		}
	}

	return Status;
}


/*****************************************************************************/
/**
 *
 * @brief	This function to extend PCR with SHA3-384 digest
 *
 * @param	pcr_number is the PCR register to send
 * @param	size is size of SHA digest
 * @param	data is sha digest
 * @param	Response is TPM response buffer
 *
 * @return	XST_SUCCESS if successful, else error code
 *
 ******************************************************************************/
u32 XTpm_Event(u32 PcrIndex, u16 size, u8 *data, u8 *Response)
{
	int Status = XST_FAILURE;
	u32 idx;
	u8 TpmPcrExtend[XTPM_PCR_MAX_EVENT_SIZE + XTPM_PCR_EVENT_CMD_SIZE + XTPM_HASH_TYPE_SHA3] =
	{
		0x80U, 0x02U, /* TPM_ST_SESSIONS */
		0x00U, 0x00U, 0x00U, 0x00U, /* Command Size */
		0x00U, 0x00U, 0x01U, 0x3CU, /* TPM_CC_PCR_EVENT */
		0x00U, 0x00U, 0x00U, 0x00U, /* PCR_Index */
		0x00U, 0x09U, /* Authorization Size */
		0x40U, 0x00U, 0x00U, 0x09U, /* Password authorization session - TPM_RH_PW */
		0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U, /* TPML_DIGEST_VALUES */
	};
	/* cmd_ptr points to const data TpmPcrExtend */
	u8* cmd_ptr = TpmPcrExtend;
	if (PcrIndex < XTPM_MAX_PCR_CNT) {
		TpmPcrExtend[XTPM_DATA_SIZE_INDEX] = (u8)(XTPM_PCR_EVENT_CMD_SIZE + size);
		TpmPcrExtend[XTPM_PCR_EXTEND_INDEX] = (u8)PcrIndex;
		/* Add the digest to the data structure */
		cmd_ptr += XTPM_PCR_EVENT_CMD_SIZE;
		for(idx = 0; idx < size ; idx++) {
			*TpmPcrExtend = data[idx];
			cmd_ptr++;
		}

		Status = XTpm_DataTransfer(TpmPcrExtend, Response, XTPM_PCR_EVENT_CMD_SIZE + size);
		if (Status != XST_SUCCESS) {
			Status = XTPM_ERR_DATA_TRANSFER;
		}
	}

	return Status;
}

#endif	/* PLM_TPM */

/**
 * @}
 * @endcond
 */
