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
*       pre  08/23/25 Did enhancements needed
*       pre  09/23/25 Fixed misrac violations
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
#define XTPM_BYTE6 (6U) /**< Byte 6 */
#define XTPM_BYTE7 (7U) /**< Byte 7 */
#define XTPM_BYTE8 (8U) /**< Byte 8 */
#define XTPM_BYTE9 (9U) /**< Byte 9 */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XTPM_PCR_EVENT_CMD_SIZE 		(29U) /**< PCR event command size */

/************************** Function Prototypes ******************************/
static u32 XTpm_StartUp(void);
static u32 XTpm_SelfTest(void);
static u32 XTpm_GetCap(void);

/************************** Variable Definitions *****************************/
static u8 TpmRespBuffer[XTPM_RESP_MAX_SIZE + XTPM_TX_HEAD_SIZE] = {0U};

/*****************************************************************************/
/**
 * @brief	This function initializes TPM, starts up TPM, performs self-test and
 * 			gets capability of the device.
 *
 * @return
 * 			- XST_SUCCESS if successful
 *			- Error code on failure
 *
 ******************************************************************************/
u32 XTpm_Init(void)
{
	u32 Status = XST_FAILURE;
	u8 Access = 0U;

	/** - Initialize SPI. Return XTPM_ERR_SPIPS_INIT error if it fails. */
	Status = XTpm_SpiInit();
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_SPIPS_INIT;
		goto END;
	}

	/** - Set access request to use. Return XTPM_ERR_SET_ACCESS error if it fails. */
	Status = XTpm_AccessSet(XTPM_ACCESS_REQ_USE);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_SET_ACCESS;
		goto END;
	}

	do {
		/**
		 * - Check for access validity and locality.
		 * Return XTPM_ERR_GET_ACCESS error in case of any failure.
		 */
		Status = XTpm_AccessGet(&Access);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XTPM_ERR_GET_ACCESS;
			goto END;
		}
	} while (!(((Access & XTPM_ACCESS_VALID) != 0U)
			&& ((Access & XTPM_ACCESS_ACT_LOCAL) !=0U)));

	/** - Startup of TPM. Return XTPM_ERR_START_UP error in case of any failure. */
	Status = XTpm_StartUp();
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_START_UP;
		goto END;
	}

	/** - TPM self-test. Return XTPM_ERR_SELF_TEST error in case of any failure. */
	Status = XTpm_SelfTest();
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_SELF_TEST;
		goto END;
	}

	/** - Get capability of TPM. Return XST_SUCCESS on success. Otherwise, return error code. */
	Status = XTpm_GetCap();

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function does startup of TPM.
 *
 * @return
 * 			- XST_SUCCESS if successful
 * 			- Error code on failure
 *
 ******************************************************************************/
static u32 XTpm_StartUp(void)
{
	u32 Status = (u32)XST_FAILURE;
	/* TPM startup command formation */
	const u8 TpmStartup[XTPM_START_CMD_SIZE] = {
		0x80U, 0x01U, /* TPM_ST_NO_SESSIONS */
		0x00U, 0x00U, 0x00U, 0x0CU, /* Command Size */
		0x00U, 0x00U, 0x01U, 0x44U, /* TPM_CC_Startup */
		0x00U, 0x00U /* TPM_SU_CLEAR */
	};

	/**
	 * - Send startup command to TPM and get response.
	 * Return XST_SUCCESS on success. Otherwise, return error code.
	 */
	Status = XTpm_DataTransfer(TpmStartup, TpmRespBuffer,
		TpmStartup[XTPM_DATA_SIZE_INDEX]);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function does a self-test of TPM.
 *
 * @return
 * 			- XST_SUCCESS if successful
 * 			- Error code on failure
 *
 ******************************************************************************/
static u32 XTpm_SelfTest(void)
{
	u32 Status = (u32)XST_FAILURE;
	/* TPM self-test command formation */
	const u8 TpmSelfTest[XTPM_SELF_TEST_CMD_SIZE] = {
		0x80U, 0x01U, /* TPM_ST_NO_SESSIONS */
		0x00U, 0x00U, 0x00, 0x0CU, /* Command Size */
		0x00U, 0x00U, 0x01U, 0x43U, /* TPM_CC_Selftest */
		0x00U, 0x00U /* TPM_SU_CLEAR */
	};

	/**
	 * - Send self-test command to TPM and get response.
	 * Return XST_SUCCESS on success. Otherwise, return error code.
	 */
	Status = XTpm_DataTransfer(TpmSelfTest, TpmRespBuffer,
		TpmSelfTest[XTPM_DATA_SIZE_INDEX]);

	return Status;
}


/*****************************************************************************/
/**
*
* This function does a get capabilities of TPM.
*
* @return
*			- XST_SUCCESS if successful
*			- Error code on failure
*
******************************************************************************/
static u32 XTpm_GetCap(void)
{
	u32 Status = (u32)XST_FAILURE;
	/* TPM Get capability command formation */
	const u8 TpmGetCap[] = {
		0x80, 0x01, /* TPM_ST_NO_SESSIONS */
		0x00, 0x00, 0x00, 0x16, /* Command Size */
		0x00, 0x00, 0x01, 0x7A, /* TPM_CC_Get Cap */
		0x00, 0x00, 0x00, 0x06, /* TPM_SU_CLEAR */
		0x00, 0x00, 0x01, 0x29, 0x00, 0x00, 0x00, 0x01 /* Property and property count */
	};

	/** - Send get capability command to TPM and get response. */
	Status = XTpm_DataTransfer(TpmGetCap, TpmRespBuffer, TpmGetCap[XTPM_DATA_SIZE_INDEX]);
	/** - If the command is not successful, return error code */
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/** - Verify response. Return XST_SUCCESS on success response reception. Otherwise, return XTPM_ERR_RESP_POLLING error. */
	if ((TpmRespBuffer[XTPM_BYTE6] != 0U) || (TpmRespBuffer[XTPM_BYTE7] != 0U) ||
		(TpmRespBuffer[XTPM_BYTE8] != 0U) || (TpmRespBuffer[XTPM_BYTE9] != 0U)) {
		Status = XTPM_ERR_RESP_POLLING;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends ROM digest stored in PMC registers to TPM.
 *
 * @return
 * 			- XST_SUCCESS if successful
 * 			- Error code on failure
 *
 ******************************************************************************/
int XTpm_MeasureRom(void)
{
	volatile int Status = (int)XTPM_ERR_MEASURE_ROM;
	u32 ShaData[XTPM_HASH_TYPE_SHA3 / XTPM_DATA_WORD_LENGTH];
	u8 Index = 0U;
	u32 RegVal;

	/** - Re-order the data in PMC_GLOBAL_ROM_VALIDATION_DIGEST_x registers to match LSB first */
	for (RegVal = PMC_GLOBAL_ROM_VALIDATION_DIGEST_11; RegVal >= PMC_GLOBAL_ROM_VALIDATION_DIGEST_0;
		RegVal -= sizeof(u32)) {
		ShaData[Index] = Xil_In32(RegVal);
		Index++;
	}

	/**
	 * - Extend data to PCR using PCR_EVENT command and get response.
	 * If the command is not successful, return error code.
	 */
	Status = (int)XTpm_Event(XTPM_TPM_ROM_PCR_INDEX, XTPM_HASH_TYPE_SHA3, (const u8 *)ShaData, TpmRespBuffer);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Verify response. Return XST_SUCCESS on success response reception.
	 * Otherwise, return XTPM_ERR_RESP_POLLING error.
	 */
	if ((TpmRespBuffer[XTPM_BYTE6] != 0U) || (TpmRespBuffer[XTPM_BYTE7] != 0U) ||
		(TpmRespBuffer[XTPM_BYTE8] != 0U) || (TpmRespBuffer[XTPM_BYTE9] != 0U)) {
		Status = (int)XTPM_ERR_RESP_POLLING;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends PLM FW digest stored in PMC registers to TPM.
 *
 * @return
 * 			- XST_SUCCESS if successful
 * 			- Error code on failure
 *
 ******************************************************************************/
int XTpm_MeasurePlm(void)
{
	volatile int Status = (int)XTPM_ERR_MEASURE_PLM;
	u32 ShaData[XTPM_HASH_TYPE_SHA3 / XTPM_DATA_WORD_LENGTH];
	u8 Index = 0U;
	u32 RegVal;

	/** - Re-order the data in PMC_GLOBAL_FW_AUTH_HASH_x registers to match LSB first */
	for (RegVal = PMC_GLOBAL_FW_AUTH_HASH_11 ; RegVal >= PMC_GLOBAL_FW_AUTH_HASH_0;
		RegVal -= sizeof(u32)) {
		ShaData[Index] = Xil_In32(RegVal);
		Index++;
	}

	/**
	 * - Extend data to PCR using PCR_EVENT command and get response.
	 * If the command is not successful, return error code.
	 */
	Status = (int)XTpm_Event(XTPM_TPM_PLM_PCR_INDEX, XTPM_HASH_TYPE_SHA3, (const u8 *)ShaData, TpmRespBuffer);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Verify response. Return XST_SUCCESS on success response reception.
	 * Otherwise, return XTPM_ERR_RESP_POLLING error
	 */
	if ((TpmRespBuffer[XTPM_BYTE6] != 0U) || (TpmRespBuffer[XTPM_BYTE7] != 0U) ||
		(TpmRespBuffer[XTPM_BYTE8] != 0U) || (TpmRespBuffer[XTPM_BYTE9] != 0U)) {
		Status = (int)XTPM_ERR_RESP_POLLING;
	}

END:
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
 * 			- XST_SUCCESS if successful
 * 			- Error code on failure.
 *
 ******************************************************************************/
int XTpm_MeasurePartition(u32 PcrIndex, const u8* ImageHash)
{
	int Status = (int)XTPM_ERR_MEASURE_PARTITION;

	/**
	 * - Extend data to PCR using PCR_EVENT command and get response.
	 * If the command is not successful, return error code.
	 */
	Status = (int)XTpm_Event(PcrIndex, XTPM_HASH_TYPE_SHA3, ImageHash, TpmRespBuffer);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Verify response. Return XST_SUCCESS on success response reception.
	 * Otherwise, return XTPM_ERR_RESP_POLLING error
	 */
	if ((TpmRespBuffer[XTPM_BYTE6] != 0U) || (TpmRespBuffer[XTPM_BYTE7] != 0U) ||
		(TpmRespBuffer[XTPM_BYTE8] != 0U) || (TpmRespBuffer[XTPM_BYTE9] != 0U)) {
		Status = (int)XTPM_ERR_RESP_POLLING;
	}

END:
	return Status;
}


/*****************************************************************************/
/**
 *
 * @brief	This function extends PCR with SHA3-384 digest
 *
 * @param	pcr_number is the PCR register to send
 * @param	size is size of SHA digest
 * @param	data is sha digest
 * @param	Response is TPM response buffer
 *
 * @return
 * 			- XST_SUCCESS if successful
 * 			- Error code on failure
 *
 ******************************************************************************/
u32 XTpm_Event(u32 PcrIndex, u16 size, const u8 *data, u8 *Response)
{
	u32 Status = (u32)XST_FAILURE;
	u32 idx;
	/* TPM PCR event command formation */
	u8 TpmPcrEvent[XTPM_PCR_MAX_EVENT_SIZE + XTPM_PCR_EVENT_CMD_SIZE] =
	{
		0x80U, 0x02U, /* TPM_ST_SESSIONS */
		0x00U, 0x00U, 0x00U, 0x00U, /* Command Size */
		0x00U, 0x00U, 0x01U, 0x3CU, /* TPM_CC_PCR_EVENT */
		0x00U, 0x00U, 0x00U, 0x00U, /* PCR_Index */
		0x00U, 0x00U, /* NULL Password */
		0x00U, 0x09U, /* Authorization Size */
		0x40U, 0x00U, 0x00U, 0x09U, /* Password authorization session - TPM_RH_PW */
		0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x30U /* TPML_DIGEST_VALUES */
	};
	/* cmd_ptr points to const data TpmPcrEvent */
	u8* cmd_ptr = TpmPcrEvent;

	/** - Validate size of input data. Return XTPM_REQ_MAX_SIZE error if it exceeds 1024 bytes */
	if(size > XTPM_REQ_MAX_SIZE) {
		goto END;
	}

	/* Validate PCR Index */
	if (PcrIndex < XTPM_MAX_PCR_CNT) {
		XPlmi_Printf(DEBUG_INFO, "Sending PCR_Event to PCR #%d\r\n", PcrIndex);
		TpmPcrEvent[XTPM_DATA_SIZE_INDEX] = (u8)(XTPM_PCR_EVENT_CMD_SIZE + size);
		TpmPcrEvent[XTPM_PCR_EXTEND_INDEX] = (u8)PcrIndex;
		/* Add the digest to the data structure */
		for(idx = 0; idx < size ; idx++) {
			cmd_ptr[XTPM_PCR_EVENT_CMD_SIZE + idx] = data[idx];
		}

		/**
		 * - Send PCR_EVENT command to TPM and get response.
		 * If the command is not successful, return error code.
		 */
		Status = XTpm_DataTransfer((const u8 *)TpmPcrEvent, Response, XTPM_PCR_EVENT_CMD_SIZE + size);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XTPM_ERR_DATA_TRANSFER;
		}
	}

END:
	return Status;
}

#endif	/* PLM_TPM */

/**
 * @}
 * @endcond
 */
