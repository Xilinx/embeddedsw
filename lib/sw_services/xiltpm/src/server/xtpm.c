/******************************************************************************
* Copyright (C) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.2   pre  01/16/25 Updated comments for RTF documentation
*       pre  03/12/26 Added validation of PCR number in partition measurement
*       pre  03/16/26 Added PCR reading support in TPM
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_TPM
#include "xil_util.h"
#include "xtpm.h"
#include "xplmi_hw.h"
#include "xtpm_hw.h"
#include "xtpm_error.h"
#include "xplmi_status.h"
#include "xtpm_cmd.h"

/************************** Constant Definitions *****************************/
#define XTPM_PCR_23 (23U) /**< Final PCR index of TPM */
#define XTPM_BYTE_SIZE_IN_BITS (8U) /**< Number of bits in a byte */
#define XTPM_SET (1U) /**< Value to set a bit */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XTPM_PCR_EVENT_CMD_SIZE 		(29U) /**< PCR event command size in bytes */
#define XTPM_PCR_READ_CMD_SIZE          (20U) /**< PCR read command size in bytes */
#define XTPM_HASH_ALGO_INDEX			(15U) /**< Index for hash algorithm in PCR read command */
#define XTPM_PCR_READ_INDEX				(17U) /**< Index for PCR selection in PCR read command */

/**
 * @addtogroup xtpm_apis XilTPM APIs
 * @{
 */

/************************** Function Prototypes ******************************/
static u32 XTpm_GetCap(void);

/************************** Variable Definitions *****************************/
static u8 TpmRespBuffer[XTPM_RESP_MAX_SIZE + XTPM_TX_HEAD_SIZE] = {0U};
/* TPM PCR event command formation */
static u8 TpmPcrEvent[XTPM_PCR_MAX_EVENT_SIZE + XTPM_PCR_EVENT_CMD_SIZE] =
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

	/** - Initializes SPI. Returns XTPM_ERR_SPIPS_INIT error if it fails. */
	Status = XTpm_InterfaceInit();
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_SPIPS_INIT;
		goto END;
	}

	/** - Sets access request to use. Returns XTPM_ERR_SET_ACCESS error if it fails. */
	Status = XTpm_AccessSet(XTPM_ACCESS_REQ_USE);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_SET_ACCESS;
		goto END;
	}

	do {
		/**
		 * - Checks for access validity and locality.
		 * Returns XTPM_ERR_GET_ACCESS error in case of any failure.
		 */
		Status = XTpm_AccessGet(&Access);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XTPM_ERR_GET_ACCESS;
			goto END;
		}
	} while (!(((Access & XTPM_ACCESS_VALID) != 0U)
			&& ((Access & XTPM_ACCESS_ACT_LOCAL) !=0U)));

	/** - Startup of TPM. Returns XTPM_ERR_START_UP error in case of any failure. */
	Status = XTpm_StartUp();
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_START_UP;
		goto END;
	}

	/** - TPM self-test. Returns XTPM_ERR_SELF_TEST error in case of any failure. */
	Status = XTpm_SelfTest();
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_SELF_TEST;
		goto END;
	}

	/** - Gets capability of TPM. Returns XST_SUCCESS on success. Otherwise, returns error code. */
	Status = XTpm_GetCap();

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the TPM module and commands.
 *
 *
 * @return
 * 			- XST_SUCCESS if successful
 *			- Error code on failure
 *
 ******************************************************************************/
u32 XTpm_ModuleInit(void)
{
	u32 Status = XST_FAILURE;

	/** - Initializes TPM. Returns XST_SUCCESS on success. Otherwise, returns error code. */
	Status = XTpm_Init();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/** - Initializes TPM commands. Returns XST_SUCCESS on success. Otherwise, returns error code. */
	XTpm_CmdsInit();

END:
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPlmi_UpdateStatus(XPLMI_ERR_TPM_INIT, (int)Status);
	}
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
u32 XTpm_StartUp(void)
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
	 * - Sends startup command to TPM and gets response.
	 * Returns XST_SUCCESS on success. Otherwise, returns error code.
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
u32 XTpm_SelfTest(void)
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
	 * - Sends self-test command to TPM and gets response.
	 * Returns XST_SUCCESS on success. Otherwise, returns error code.
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
	const u8 TpmGetCap[XTPM_GET_CAP_CMD_SIZE] = {
		0x80, 0x01, /* TPM_ST_NO_SESSIONS */
		0x00, 0x00, 0x00, 0x16, /* Command Size */
		0x00, 0x00, 0x01, 0x7A, /* TPM_CC_Get Cap */
		0x00, 0x00, 0x00, 0x06, /* TPM_SU_CLEAR */
		0x00, 0x00, 0x01, 0x29, 0x00, 0x00, 0x00, 0x01 /* Property and property count */
	};

	/** - Sends get capability command to TPM and gets response. */
	Status = XTpm_DataTransfer(TpmGetCap, TpmRespBuffer, TpmGetCap[XTPM_DATA_SIZE_INDEX]);
	/** - If the command is not successful, returns error code */
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/** - Verifies response. Returns XST_SUCCESS on success response reception. Otherwise, returns XTPM_ERR_RESP_POLLING error. */
	if ((TpmRespBuffer[XTPM_INDEX_6] != 0U) || (TpmRespBuffer[XTPM_INDEX_7] != 0U) ||
		(TpmRespBuffer[XTPM_INDEX_8] != 0U) || (TpmRespBuffer[XTPM_INDEX_9] != 0U)) {
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
	 * - Extends data to PCR using PCR_EVENT command and gets response.
	 * If the command is not successful, returns error code.
	 */
	Status = (int)XTpm_Event(XTPM_TPM_ROM_PCR_INDEX, XTPM_HASH_TYPE_SHA3, (const u8 *)ShaData, TpmRespBuffer);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Verifies response. Returns XST_SUCCESS on success response reception.
	 * Otherwise, returns XTPM_ERR_RESP_POLLING error.
	 */
	if ((TpmRespBuffer[XTPM_INDEX_6] != 0U) || (TpmRespBuffer[XTPM_INDEX_7] != 0U) ||
		(TpmRespBuffer[XTPM_INDEX_8] != 0U) || (TpmRespBuffer[XTPM_INDEX_9] != 0U)) {
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
	 * - Extends data to PCR using PCR_EVENT command and gets response.
	 * If the command is not successful, returns error code.
	 */
	Status = (int)XTpm_Event(XTPM_TPM_PLM_PCR_INDEX, XTPM_HASH_TYPE_SHA3, (const u8 *)ShaData, TpmRespBuffer);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Verifies response. Returns XST_SUCCESS on success response reception.
	 * Otherwise, returns XTPM_ERR_RESP_POLLING error.
	 */
	if ((TpmRespBuffer[XTPM_INDEX_6] != 0U) || (TpmRespBuffer[XTPM_INDEX_7] != 0U) ||
		(TpmRespBuffer[XTPM_INDEX_8] != 0U) || (TpmRespBuffer[XTPM_INDEX_9] != 0U)) {
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

	/** Validate input parameters. PCR0 and PCR1 are reserved for ROM and PLM measurements */
	if (PcrIndex < XTPM_PCR_2 || PcrIndex > XTPM_PCR_23) {
		Status = (int)XTPM_ERR_PCR_INDEX_INVALID;
		goto END;
	}

	/**
	 * - Extends data to PCR using PCR_EVENT command and gets response.
	 * If the command is not successful, returns error code.
	 */
	Status = (int)XTpm_Event(PcrIndex, XTPM_HASH_TYPE_SHA3, ImageHash, TpmRespBuffer);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Verifies response. Returns XST_SUCCESS on success response reception.
	 * Otherwise, returns XTPM_ERR_RESP_POLLING error.
	 */
	if ((TpmRespBuffer[XTPM_INDEX_6] != 0U) || (TpmRespBuffer[XTPM_INDEX_7] != 0U) ||
		(TpmRespBuffer[XTPM_INDEX_8] != 0U) || (TpmRespBuffer[XTPM_INDEX_9] != 0U)) {
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
 * @param	PcrIndex is the number of the PCR register to which extension is to be done
 * @param	size is size of SHA digest in bytes
 * @param	data is pointer to buffer which contains SHA digest
 * @param	Response is pointer to TPM response buffer in which response from TPM gets stored
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
	/* cmd_ptr points to const data TpmPcrEvent */
	u8* cmd_ptr = TpmPcrEvent;

	/** - Validates size of input data. Returns XTPM_REQ_MAX_SIZE error if it exceeds 1024 bytes */
	if(size > XTPM_REQ_MAX_SIZE) {
		goto END;
	}

	/* Validate PCR Index */
	if (PcrIndex < XTPM_MAX_PCR_CNT) {
		XPlmi_Printf(DEBUG_INFO, "Sending PCR_Event to PCR #%d\r\n", PcrIndex);
		TpmPcrEvent[XTPM_DATA_SIZE_INDEX] = (u8)(XTPM_PCR_EVENT_CMD_SIZE + size);
		TpmPcrEvent[XTPM_PCR_EXTEND_INDEX] = (u8)PcrIndex;
		 /* Update command size in command buffer */
		TpmPcrEvent[XTPM_PCR_EVENT_SIZE_INDEX] = (u8)size;
		/* Add the digest to the data structure */
		for(idx = 0; idx < size ; idx++) {
			cmd_ptr[XTPM_PCR_EVENT_CMD_SIZE + idx] = data[idx];
		}

		/**
		 * - Sends PCR_EVENT command to TPM and gets response.
		 * If the command is not successful, returns error code.
		 */
		Status = XTpm_DataTransfer((const u8 *)TpmPcrEvent, Response, XTPM_PCR_EVENT_CMD_SIZE + size);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XTPM_ERR_DATA_TRANSFER;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function reads PCR value for the specified PCR index and hash algorithm using PCR_READ command.
 *
 * @param	PcrIndex is the number of the PCR register to be read
 * @param	HashAlgo is the hash algorithm identifier for which PCR value needs to be read
 * @param	Response is pointer to TPM response buffer in which response from TPM gets stored
 *
 * @return
 * 			- XST_SUCCESS if successful
 * 			- Error code on failure
 *
 ******************************************************************************/
u32 XTpm_PcrRead(u32 PcrIndex, u8 HashAlgo, u8 *Response)
{
	u32 Status = (u32)XST_FAILURE;
	/* TPM PCR read command formation */
	u8 TpmPcrRead[XTPM_PCR_READ_CMD_SIZE] = {
	0x80, 0x01, /* TPM_ST_NO_SESSIONS */
	0x00, 0x00, 0x00, 0x14, /* Command Size */
	0x00, 0x00, 0x01, 0x7E, /* TPM_CC_PCR_Read */
	0x00, 0x00, 0x00, 0x01, /* PCR Count */
	0x00, 0x00, /* Hash algorithm */
	0x03, /* Size PCRs */
	0x00, 0x00, 0x00 /* PCR Select */
	};

	if (PcrIndex > XTPM_PCR_23) {
		Status = (int)XTPM_ERR_PCR_INDEX_INVALID;
		goto END;
	}

	if ((HashAlgo != XTPM_HASH_TYPE_SHA1) && (HashAlgo != XTPM_HASH_TYPE_SHA256)) {
		Status = (int)XTPM_ERR_HASH_ALGO_INVALID;
		goto END;
	}

	TpmPcrRead[XTPM_HASH_ALGO_INDEX] = HashAlgo;
	TpmPcrRead[XTPM_PCR_READ_INDEX + (PcrIndex / XTPM_BYTE_SIZE_IN_BITS)] =
			(XTPM_SET << (PcrIndex % XTPM_BYTE_SIZE_IN_BITS));

	Status = XTpm_DataTransfer((const u8 *)TpmPcrRead, Response, XTPM_PCR_READ_CMD_SIZE);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_DATA_TRANSFER;
	}

END:
	return Status;
}

/** @} End of xtpm_apis group */

#endif	/* PLM_TPM */
