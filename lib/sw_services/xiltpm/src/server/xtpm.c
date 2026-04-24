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
*       pre  03/21/26 Implemented GetPcrLog feature to fetch PCR event log from TPM
*       pre  04/22/26 Addition of access retry mechanism in case of TPM being busy
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
#define XTPM_BYTE_SIZE_IN_BITS (8U) /**< Number of bits in a byte */
#define XTPM_SET (1U) /**< Value to set a bit */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XTPM_PCR_EVENT_CMD_SIZE 		(29U) /**< PCR event command size in bytes */
#define XTPM_PCR_READ_CMD_SIZE          (20U) /**< PCR read command size in bytes */
#define XTPM_HASH_ALGO_INDEX			(15U) /**< Index for hash algorithm in PCR read command */
#define XTPM_PCR_READ_INDEX				(17U) /**< Index for PCR selection in PCR read command */
#define XTPM_ACCESS_RETRY_COUNT			(5U)  /**< Number of retries for TPM access */
#ifdef VERSAL_2VE_2VM
#define XTPM_PCR_LOG_LCVERSION		(1U)   /**< PCR log LC version */
#define XTPM_PCR_LOG_VERSION		(1U)   /**< PCR log version */
#define XTPM_PCR_LOG_DS_ID			(1U)	/**< PCR log data structure ID */
#endif

/**
 * @addtogroup xtpm_apis XilTPM APIs
 * @{
 */
/*****************************************************************************/
/**
 * @brief	This function updates the index of PCR log
 *
 * @param	PcrIndex pointer to the index to be incremented
 * @param  	Val      Increment value
 *
 ******************************************************************************/
static INLINE void XTpm_UpdatePcrIndex(u32 *PcrIndex, u32 Val)
{
	*PcrIndex += Val;
	if (*PcrIndex >= XTPM_MAX_NUM_OF_PCR_EVENTS) {
		*PcrIndex = 0U;
	}
}

/************************** Function Prototypes ******************************/
static u32 XTpm_GetCap(void);
static XTpm_PcrLog_t *XTpm_GetPcrLogInstance(void);
static u32 XTpm_UpdatePcrLog(u32 PcrNum, u64 ExtHashAddr, u32 DataSize);

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
	u32 Status = (u32)XST_FAILURE;
	u32 Count = 0U;
	u8 Access = 0U;

	/** - Initializes SPI. Returns XTPM_ERR_SPIPS_INIT error if it fails. */
	Status = XilTpm_InterfaceInit();
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
		if (Count > XTPM_ACCESS_RETRY_COUNT) {
			Status = (u32)XTPM_ERR_ACCESS_TIMEOUT;
			goto END;
		}
		/**
		 * - Checks for access validity and locality.
		 * Returns XTPM_ERR_GET_ACCESS error in case of any failure.
		 */
		Status = XTpm_AccessGet(&Access);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XTPM_ERR_GET_ACCESS;
			goto END;
		}
		Count++;
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
	u32 Status = (u32)XST_FAILURE;

	/** - Initializes TPM. Returns XST_SUCCESS on success. Otherwise, returns error code. */
	Status = XTpm_Init();
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/** - Initializes TPM commands. */
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
		Status = (u32)XTPM_ERR_RESP_POLLING;
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
	volatile int Status = XTPM_ERR_MEASURE_ROM;
	u32 ShaData[XTPM_SHA3_HASH_LEN_IN_BYTES / XTPM_DATA_WORD_LENGTH];
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
	Status = (int)XTpm_Event(XTPM_TPM_ROM_PCR_INDEX, XTPM_SHA3_HASH_LEN_IN_BYTES, (const u8 *)ShaData);

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
	u32 ShaData[XTPM_SHA3_HASH_LEN_IN_BYTES / XTPM_DATA_WORD_LENGTH];
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
	Status = (int)XTpm_Event(XTPM_TPM_PLM_PCR_INDEX, XTPM_SHA3_HASH_LEN_IN_BYTES, (const u8 *)ShaData);

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

	/** Validate input parameters. PCR0 and PCR1 are reserved for ROM and PLM measurements, PCR17 to PCR22 are not extendable */
	if ((PcrIndex < XTPM_PCR_2) || (PcrIndex > XTPM_PCR_23) || ((PcrIndex > XTPM_PCR_16) && (PcrIndex < XTPM_PCR_23))) {
		Status = (int)XTPM_ERR_PCR_INDEX_INVALID;
		goto END;
	}

	/**
	 * - Extends data to PCR using PCR_EVENT command and gets response.
	 * If the command is not successful, returns error code.
	 */
	Status = (int)XTpm_Event(PcrIndex, XTPM_SHA3_HASH_LEN_IN_BYTES, ImageHash);

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
 *
 * @return
 * 			- XST_SUCCESS if successful
 * 			- Error code on failure
 *
 ******************************************************************************/
u32 XTpm_Event(u32 PcrIndex, u16 size, const u8 *data)
{
	u32 Status = (u32)XST_FAILURE;
	u32 idx;
	/* cmd_ptr points to const data TpmPcrEvent */
	u8* cmd_ptr = TpmPcrEvent;

	/**
	 * - Validates size of input data. Returns XTPM_ERR_DATA_SIZE_INVALID error if data size
	 * is not equal to 48bytes
	 */
	if (size != XTPM_SHA3_HASH_LEN_IN_BYTES) {
		Status = (u32)XTPM_ERR_DATA_SIZE_INVALID;
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
		Status = XTpm_DataTransfer((const u8 *)TpmPcrEvent, TpmRespBuffer, XTPM_PCR_EVENT_CMD_SIZE + size);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XTPM_ERR_DATA_TRANSFER;
			goto END;
		}

		/**
		 * - Verifies response. Returns XST_SUCCESS on success response reception.
		 * Otherwise, returns XTPM_ERR_RESP_POLLING error.
		 */
		if ((TpmRespBuffer[XTPM_INDEX_6] != 0U) || (TpmRespBuffer[XTPM_INDEX_7] != 0U) ||
			(TpmRespBuffer[XTPM_INDEX_8] != 0U) || (TpmRespBuffer[XTPM_INDEX_9] != 0U)) {
			Status = (u32)XTPM_ERR_RESP_POLLING;
			goto END;
		}

		Status = XTpm_UpdatePcrLog(PcrIndex, (u64)(UINTPTR)&data[0], size);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XTPM_ERR_PCR_LOG_UPDATE;
		}
	}
	else {
		Status = (u32)XTPM_ERR_PCR_INDEX_INVALID;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function reads PCR value for the specified PCR index and hash algorithm using
 *          PCR_READ command.
 *
 * @param	PcrIndex is the number of the PCR register to be read
 * @param	HashAlgo is the hash algorithm identifier for which PCR value needs to be read
 * @param	ResponseBufAddr is the address of the response buffer in which response from TPM gets
 *                          stored
 *
 * @return
 * 			- XST_SUCCESS if successful
 * 			- Error code on failure
 *
 *************************************************************************************************/
u32 XTpm_PcrRead(u32 PcrIndex, u8 HashAlgo, u64 ResponseBufAddr)
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
		Status = (u32)XTPM_ERR_PCR_INDEX_INVALID;
		goto END;
	}

	if ((HashAlgo != (u8)XTPM_HASH_TYPE_SHA1) && (HashAlgo != (u8)XTPM_HASH_TYPE_SHA256)) {
		Status = (u32)XTPM_ERR_HASH_ALGO_INVALID;
		goto END;
	}

	TpmPcrRead[XTPM_HASH_ALGO_INDEX] = HashAlgo;
	TpmPcrRead[XTPM_PCR_READ_INDEX + (PcrIndex / XTPM_BYTE_SIZE_IN_BITS)] =
			(XTPM_SET << (PcrIndex % XTPM_BYTE_SIZE_IN_BITS));

	Status = XTpm_DataTransfer((const u8 *)TpmPcrRead, TpmRespBuffer, XTPM_PCR_READ_CMD_SIZE);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_DATA_TRANSFER;
		goto END;
	}

	if ((TpmRespBuffer[XTPM_INDEX_6] != 0U) || (TpmRespBuffer[XTPM_INDEX_7] != 0U) ||
		(TpmRespBuffer[XTPM_INDEX_8] != 0U) || (TpmRespBuffer[XTPM_INDEX_9] != 0U)) {
		Status = (u32)XTPM_ERR_RESP_POLLING;
		goto END;
	}

	/** -
	 * Copy the PCR read response from local buffer to the response buffer
	 * address provided by the client
	 */
	Status = (u32)XPlmi_MemCpy64(ResponseBufAddr,
			(u64)(UINTPTR)&TpmRespBuffer[XTPM_PCR_VALUE_START_INDEX],
			((u32)TpmRespBuffer[XTPM_INDEX_5] - (u32)XTPM_PCR_VALUE_START_INDEX));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function provides the pointer to the XTpm_PcrLog
 *        instance which has to be used across the file to store the
 *        PCR log.
 *
 * @return
 *		Pointer to the XTpm_PcrLog instance
 *
 ******************************************************************************/
static XTpm_PcrLog_t *XTpm_GetPcrLogInstance(void)
{
	static XTpm_PcrLog_t PcrLog __attribute__ ((aligned(4U))) = {0U};

#ifdef VERSAL_2VE_2VM
	EXPORT_TPM_DS(PcrLog, XTPM_PCR_LOG_DS_ID,
		XTPM_PCR_LOG_VERSION, XTPM_PCR_LOG_LCVERSION,
		sizeof(PcrLog), (u32)(UINTPTR)&PcrLog);
#endif

	return &PcrLog;
}

/*************************************************************************************************/
/**
 * @brief	This function updates the PCR log.
 *
 * @param	PcrNum		PCR register number
 * @param	ExtHashAddr	Address of the hash to be extended
 * @param	DataSize	Size of the Data
 *
 * @return
 *		- XST_SUCCESS - If log update is successful
 *		- XST_FAILURE - Upon failure
 *
 *************************************************************************************************/
static u32 XTpm_UpdatePcrLog(u32 PcrNum, u64 ExtHashAddr, u32 DataSize)
{
	volatile u32 Status = (u32)XST_FAILURE;
	XTpm_PcrLog_t *PcrLog = XTpm_GetPcrLogInstance();

	/** - PCR number validation */
	if ((PcrNum > XTPM_PCR_23) || (ExtHashAddr == 0U) ||
		(DataSize != (u32)XTPM_SHA3_HASH_LEN_IN_BYTES)) {
		goto END;
	}

	/** If number of PCR events is greater than XTPM_MAX_NUM_OF_PCR_EVENTS
	 * update overflow count as true and decrement number of PCR events for
	 * new update and update the tail index.
	 */
	if (PcrLog->LogInfo.RemainingPcrEvents >= XTPM_MAX_NUM_OF_PCR_EVENTS) {
		PcrLog->LogInfo.OverflowCntSinceLastRd++;
		if (PcrLog->HeadIndex == PcrLog->TailIndex) {
			XTpm_UpdatePcrIndex(&PcrLog->TailIndex, 1U);
		}
		PcrLog->LogInfo.RemainingPcrEvents--;
	}

	/** - Update PCR log with extended hash */
	PcrLog->Buffer[PcrLog->HeadIndex].PcrNo = (u8)PcrNum;
	Status = (u32)XPlmi_MemCpy64((u64)(UINTPTR)PcrLog->Buffer[PcrLog->HeadIndex].Hash,
							ExtHashAddr, DataSize);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/** - Update PCR log with new PCR value */
	Status = (u32)XTpm_PcrRead(PcrNum, XTPM_HASH_TYPE_SHA256,
				(u64)(UINTPTR)PcrLog->Buffer[PcrLog->HeadIndex].PcrValue);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/** - Update PCR log info */
	PcrLog->LogInfo.RemainingPcrEvents++;
	PcrLog->LogInfo.TotalPcrLogEvents++;
	XTpm_UpdatePcrIndex(&PcrLog->HeadIndex, 1U);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function reads the PCR log and info into the user provided
 * 		buffer.
 *
 * @param	PcrEventsAddr  Pointer to the buffer which contains PCR log events read from TPM
 * @param	PcrLogInfoAddr Pointer to the buffer which contains PCR log info like number of events
 *                         read, remaining events and overflow count
 * @param	NumOfLogEntries	 Maximum number of log entries to be read
 *
 * @return
 *		- XST_SUCCESS - If log read is successful
 *		- error code on failure
 *************************************************************************************************/
u32 XTpm_GetPcrLog(u64 PcrEventsAddr, u64 PcrLogInfoAddr, u32 NumOfLogEntries)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ReqPcrLogEntries = NumOfLogEntries;
	u32 RemPcrLogEvents = 0U;
	u32 TotalRdPcrLogEvents = 0U;
	u64 PcrEventsAddrTmp = PcrEventsAddr;
	XTpm_PcrLog_t *PcrLog = XTpm_GetPcrLogInstance();

	/** Validate input parameters. */
	if (ReqPcrLogEntries > XTPM_MAX_NUM_OF_PCR_EVENTS) {
		Status = (u32)XTPM_PCR_ERR_INVALID_LOG_READ_REQUEST;
		goto END;
	}

	if ((PcrLog->LogInfo.RemainingPcrEvents == 0U) || (ReqPcrLogEntries == 0U)) {
		Status = (u32)XST_SUCCESS;
		goto END1;
	}

	if (ReqPcrLogEntries > PcrLog->LogInfo.RemainingPcrEvents) {
		ReqPcrLogEntries = PcrLog->LogInfo.RemainingPcrEvents;
	}

	TotalRdPcrLogEvents = ReqPcrLogEntries;
	/**
	 * From current TailIndex if number of entries are more than XTPM_MAX_NUM_OF_PCR_EVENTS
	 * then copy log entries from TailIndex to XTPM_MAX_NUM_OF_PCR_EVENTS and update
	 * log entries and TailIndex.
	 */
	if ((PcrLog->TailIndex + ReqPcrLogEntries) >= XTPM_MAX_NUM_OF_PCR_EVENTS) {
		RemPcrLogEvents = XTPM_MAX_NUM_OF_PCR_EVENTS - PcrLog->TailIndex;
		Status = (u32)XPlmi_MemCpy64(PcrEventsAddrTmp,
					(u64)(UINTPTR)&PcrLog->Buffer[PcrLog->TailIndex],
					(RemPcrLogEvents * sizeof(XTpm_PcrEvent_t)));
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		/** Update PCR events and log entries to handle remaining entries */
		PcrLog->LogInfo.RemainingPcrEvents -= RemPcrLogEvents;
		ReqPcrLogEntries -= RemPcrLogEvents;
		PcrEventsAddrTmp += (u64)RemPcrLogEvents * sizeof(XTpm_PcrEvent_t);
		PcrLog->TailIndex = 0U;
    }

	/** Copy PCR log to the user buffer. */
	if (ReqPcrLogEntries != 0U) {
		Status = (u32)XPlmi_MemCpy64(PcrEventsAddrTmp,
					(u64)(UINTPTR)&PcrLog->Buffer[PcrLog->TailIndex],
					(ReqPcrLogEntries * sizeof(XTpm_PcrEvent_t)));
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		PcrLog->LogInfo.RemainingPcrEvents -= ReqPcrLogEntries;
		XTpm_UpdatePcrIndex(&PcrLog->TailIndex, ReqPcrLogEntries);
	}
END1:
	/** Update current PCR log status */
	PcrLog->LogInfo.PcrEventsRead = TotalRdPcrLogEvents;
	/** Copy PCR log info to the user buffer. */
	Status = (u32)XPlmi_MemCpy64(PcrLogInfoAddr, (u64)(UINTPTR)&PcrLog->LogInfo,
				sizeof(XTpm_PcrLogInfo_t));
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	PcrLog->LogInfo.OverflowCntSinceLastRd = 0U;
END:
	return Status;
}

/** @} End of xtpm_apis group */

#endif	/* PLM_TPM */
