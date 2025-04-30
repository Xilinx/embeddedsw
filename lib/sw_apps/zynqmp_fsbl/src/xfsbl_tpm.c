/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


/*****************************************************************************/
/**
*
* @file xfsbl_tpm.c
*
* This is the file which contains TPM related code for the FSBL.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bsv  04/01/21 Initial release
* 2.00  sd   04/25/25 Fix incorrect TPM initialization
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xfsbl_hw.h"
#include "xparameters.h"
#ifdef XFSBL_TPM
#include "xfsbl_tpm.h"
#include "xspips.h"
#include "xfsbl_qspi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static u32 XFsbl_TpmStartUp(void);
static u32 XFsbl_TpmSelfTest(void);
static u32 XFsbl_TpmPcrReadIndex(u8 PcrIndex, u8 HashAlg);
static u32 XFsbl_TpmReadID(void);
static u32 XFsbl_TpmGetCap(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes TPM, starts up TPM and performs self test.
*
* @return	XFSBL_SUCCESS if successful, else error code
*
******************************************************************************/
u32 XFsbl_TpmInit(void)
{
	u32 Status = XFSBL_FAILURE;
	u8 Access = 0U;

	Status = XFsbl_SpiInit();
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	/* Set access request to use */
	Status = XFsbl_TpmAccessSet(XFSBL_TPM_ACCESS_REQ_USE);
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	do {
		/* Check for access valid and locality */
		Status = XFsbl_TpmAccessGet(&Access);
		if (Status != XFSBL_SUCCESS) {
			goto END;
		}
	} while (!((Access & XFSBL_TPM_ACCESS_VALID)
			&& (Access & XFSBL_TPM_ACCESS_ACT_LOCAL)));

	Status = XFsbl_TpmReadID();
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	Status = XFsbl_TpmGetCap();
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	Status = XFsbl_TpmSelfTest();
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	Status = XFsbl_TpmStartUp();
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
*
* This function does a startup of TPM.
*
* @return	XFSBL_SUCCESS if successful, else error code
*
******************************************************************************/
static u32 XFsbl_TpmStartUp(void)
{
	int Status = XFSBL_FAILURE;
	u8 TpmRespBuffer[XFSBL_TPM_RESP_MAX_SIZE] = {0U};
	u8 TpmStartup[] = {
		0x80U, 0x01U, /* TPM_ST_NO_SESSIONS */
		0x00U, 0x00U, 0x00U, 0x0CU, /* Command Size */
		0x00U, 0x00U, 0x01U, 0x44U, /* TPM_CC_Startup */
		0x00U, 0x00U /* TPM_SU_CLEAR */
	};

	Status = XFsbl_TpmDataTransfer(TpmStartup, TpmRespBuffer,
		TpmStartup[XFSBL_TPM_DATA_SIZE_INDEX]);

	return Status;
}

/*****************************************************************************/
/**
*
* This function does a self test of TPM.
*
* @return	XFSBL_SUCCESS if successful, else error code
*
******************************************************************************/
static u32 XFsbl_TpmSelfTest(void)
{
	u32 Status = XFSBL_FAILURE;
	u8 TpmRespBuffer[XFSBL_TPM_RESP_MAX_SIZE] = {0U};
	u8 TpmSelfTest[] = {
		0x80U, 0x01U, /* TPM_ST_NO_SESSIONS */
		0x00U, 0x00U, 0x00, 0x0CU, /* Command Size */
		0x00U, 0x00U, 0x01U, 0x43U, /* TPM_CC_Selftest */
		0x00U, 0x00U /* TPM_SU_CLEAR */
	};

	Status = XFsbl_TpmDataTransfer(TpmSelfTest, TpmRespBuffer,
		TpmSelfTest[XFSBL_TPM_DATA_SIZE_INDEX]);

	return Status;
}

/*****************************************************************************/
/**
*
* This function does a get capabilities of TPM.
*
* @return	XFSBL_SUCCESS if successful, else error code
*
******************************************************************************/
static u32 XFsbl_TpmGetCap(void)
{
	u32 Status = XFSBL_FAILURE;
	u8 TpmRespBuffer[XFSBL_TPM_RESP_MAX_SIZE] = {0U};
	u8 TpmGetCap[] = {
		0x80, 0x01, /* TPM_ST_NO_SESSIONS */
		0x00, 0x00, 0x00, 0x16, /* Command Size */
		0x00, 0x00, 0x01, 0x7A, /* TPM_CC_Get Cap */
		0x00, 0x00, 0x00, 0x06, /* TPM_SU_CLEAR */
		0x00, 0x00, 0x01, 0x29, 0x00, 0x00, 0x00, 0x01
	};

	Status = XFsbl_TpmDataTransfer(TpmGetCap, TpmRespBuffer,
		TpmGetCap[XFSBL_TPM_DATA_SIZE_INDEX]);

	return Status;

}

/*****************************************************************************/
/**
*
* This function reads vendor and revision of TPM.
*
* @return	XFSBL_SUCCESS if successful, else error code
*
******************************************************************************/
static u32 XFsbl_TpmReadID(void)
{
	u32 Status = XFSBL_FAILURE;
	u32 VendId;
	u8 RevId;

	Status = XFsbl_TpmTransfer(XFSBL_TPM_DID_VID, NULL, (u8 *)&VendId, XFSBL_TPM_DID_VID_LEN);
		if (Status != XFSBL_SUCCESS) {
			goto END;
	}

	Status = XFsbl_TpmTransfer(XFSBL_TPM_RID, NULL, (u8 *)&RevId, XFSBL_TPM_RID_LEN);
		if (Status != XFSBL_SUCCESS) {
			goto END;
	}

	XFsbl_Printf(DEBUG_INFO, "Vendor ID 0x%x Revision ID 0x%x\r\n", VendId, RevId);

END:
	return Status;
}

/*****************************************************************************/
/**
*
* This function sends ROM digest stored in CSU registers to TPM.
*
* @return	XFSBL_SUCCESS if successful, else error code
*
******************************************************************************/
u32 XFsbl_TpmMeasureRom(void)
{
	u32 Status = XFSBL_FAILURE;
	u32 ShaData[XFSBL_HASH_TYPE_SHA3 / 4U];
	u8 Index = 0U;
	u32 RegVal;
	u8 TpmPcrExtend[XFSBL_TPM_PCR_EXT_CMD_SIZE + XFSBL_HASH_TYPE_SHA3] =
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
	u8 TpmRespBuffer[XFSBL_TPM_RESP_MAX_SIZE] = {0U};

	/* Re-order the data to match LSB first */
	for (RegVal = CSU_ROM_DIGEST_ADDR_11; RegVal >= CSU_ROM_DIGEST_ADDR_0;
		RegVal -= sizeof(u32)) {
		ShaData[Index++] = Xil_In32(RegVal);
	}

	TpmPcrExtend[XFSBL_TPM_DATA_SIZE_INDEX] = XFSBL_TPM_PCR_EXT_CMD_SIZE +
		XFSBL_TPM_DIGEST_SIZE;
	(void)XFsbl_MemCpy(&TpmPcrExtend[XFSBL_TPM_PCR_EXT_CMD_SIZE], (u8 *)ShaData,
		XFSBL_TPM_DIGEST_SIZE);

	XFsbl_PrintArray(DEBUG_INFO, (u8*)ShaData, XFSBL_HASH_TYPE_SHA3, "SHA3 ROM");
	Status = XFsbl_TpmEvent(0, XFSBL_HASH_TYPE_SHA3, (u8 *)ShaData, TpmRespBuffer);
	if (XFSBL_SUCCESS != Status) {
		goto END;
	}

	if ((TpmRespBuffer[6U] != 0U) || (TpmRespBuffer[7U] != 0U) ||
		(TpmRespBuffer[8U] != 0U) || (TpmRespBuffer[9U] != 0U)) {
		Status = XFSBL_FAILURE;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
*
* This function sends SHA3 digest to TPM.
*
* @param	PcrIndex is the PCR register to send
* @param	PartitionHash is pointer to SHA3 digest
*
* @return	XFSBL_SUCCESS if successful, else error code
*
******************************************************************************/
u32 XFsbl_TpmMeasurePartition(u8 PcrIndex, u8* PartitionHash)
{
	u32 Status = XFSBL_FAILURE;
	u8 TpmPcrExtend[XFSBL_TPM_PCR_EXT_CMD_SIZE + XFSBL_TPM_DIGEST_SIZE] =
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
	u8 TpmRespBuffer[XFSBL_TPM_RESP_MAX_SIZE + XFSBL_TPM_TX_HEAD_SIZE] =
		{0U};

	TpmPcrExtend[XFSBL_TPM_DATA_SIZE_INDEX] = XFSBL_TPM_PCR_EXT_CMD_SIZE +
		XFSBL_TPM_DIGEST_SIZE;
	TpmPcrExtend[XFSBL_TPM_PCR_EXTEND_INDEX] = PcrIndex;
	(void)XFsbl_MemCpy((void *)&TpmPcrExtend[XFSBL_TPM_PCR_EXT_CMD_SIZE],
		(void *)PartitionHash, XFSBL_TPM_DIGEST_SIZE);

	XFsbl_PrintArray(DEBUG_INFO, (u8*)PartitionHash, XFSBL_HASH_TYPE_SHA3, "SHA3 PARTITION");
	Status = XFsbl_TpmEvent(PcrIndex, XFSBL_HASH_TYPE_SHA3, (u8 *)PartitionHash, TpmRespBuffer);
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	if ((TpmRespBuffer[6U] != 0U) || (TpmRespBuffer[7U] != 0U) ||
		(TpmRespBuffer[8U] != 0U) || (TpmRespBuffer[9U] != 0U)) {
		Status = XFSBL_FAILURE;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
*
* This function updates TPM PCR registers by SHA digest of data
*
* @param	PcrIndex is the PCR register to send
* @param	Size is size of Event data being internally hashed by TPM
* @param	Data is Event data being internally hashed by TPM
* @param	Response is TPM response buffer
*
* @return	XFSBL_SUCCESS if successful, else error code
*
******************************************************************************/
u32 XFsbl_TpmEvent(u32 PcrIndex, u16 Size, u8 *Data, u8 *Response)
{
	u32 Status = XFSBL_FAILURE;
	u32 Idx;
	u8 TpmPcrEvent[XFSBL_TPM_PCR_EVENT_CMD_SIZE + XFSBL_TPM_REQ_MAX_SIZE] =
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
	/* CmdPtr points to const data TpmPcrEvent */
	u8* CmdPtr = TpmPcrEvent;

	if(Size > XFSBL_TPM_REQ_MAX_SIZE) {
		goto END;
	}

	if (PcrIndex < XFSBL_TPM_MAX_INDEX) {
			XFsbl_Printf(DEBUG_INFO, "Sending PCR_Event to PCR #%d\r\n", PcrIndex);
			TpmPcrEvent[XFSBL_TPM_DATA_SIZE_INDEX] = (u8)(XFSBL_TPM_PCR_EVENT_CMD_SIZE + Size);
			TpmPcrEvent[XFSBL_TPM_PCR_EXTEND_INDEX] = (u8)PcrIndex;
			/* Add the digest to the data structure */
			CmdPtr += XFSBL_TPM_PCR_EVENT_CMD_SIZE;
			for(Idx = 0; Idx < Size ; Idx++) {
				*CmdPtr = Data[Idx];
				CmdPtr++;
			}

			/* Send the command */
			Status = XFsbl_TpmDataTransfer(TpmPcrEvent, Response, XFSBL_TPM_PCR_EVENT_CMD_SIZE + Size);
	}

END:
	return Status;
}

#ifdef DEBUG_INFO
/*****************************************************************************/
/**
*
* This function prints all TPM register values
*
******************************************************************************/
void XFsbl_ReadAllTpmRegisters(void)
{
	u32 Status = XFSBL_FAILURE;
	u8 Idx;

	XFsbl_Printf(DEBUG_INFO, "\n\rRead TPM PCR Registers\n\r");
	for (Idx = 0; Idx < XFSBL_TPM_MAX_INDEX; Idx++){
		XFsbl_Printf(DEBUG_INFO, "PCR%02d : ",Idx);
		Status = XFsbl_TpmPcrReadIndex(Idx, XFSBL_TPM_HASH_ALG_SHA256);
		if (Status != XFSBL_SUCCESS) {
			XFsbl_Printf(DEBUG_INFO, "Failed to read TPM registers\n\r");
		}
		XFsbl_Printf(DEBUG_INFO, "\n\r");
	}
}

/*****************************************************************************/
/**
*
* This function reads TPM register data of passed PCR Index
*
* @param	PcrIndex PCR register index
* @param	HashAlg Hash algo used
*
* @return	XFSBL_SUCCESS if successful, else error code
*
******************************************************************************/
static u32 XFsbl_TpmPcrReadIndex(u8 PcrIndex, u8 HashAlg)
{
	u32 Status = XFSBL_FAILURE;
	u8 ResponseBuf[XFSBL_TPM_RESP_MIN_SIZE] = {0U};
	u8 Idx;
	u8 TpmPcrRead[XFSBL_TPM_PCR_READ_COMMAND_SIZE] =
	{
		0x80U, 0x01U, /* TPM_ST_NO_SESSIONS */
		0x00U, 0x00U, 0x00U, 0x14U, /* Command Size */
		0x00U, 0x00U, 0x01U, 0x7EU, /* TPM_CC_PCR_READ */
		0x00U, 0x00U, 0x00U, 0x01U, /* Count */
		0x00U, 0x0BU, /* Hash SHA256 */
		0x03U, /* size Of PCRs*/
		0x00U, 0x00U, 0x00U /* select PCR */
	};

	if (HashAlg != XFSBL_TPM_HASH_ALG_SHA256) {
		TpmPcrRead[XFSBL_TPM_HASH_ALG_OFFSET] = XFSBL_TPM_HASH_ALG_SHA1;
	}

	TpmPcrRead[XFSBL_TPM_PCR_SELECT_OFFSET + (PcrIndex / XFSBL_TPM_DIVD_VAL)] = (1U << (PcrIndex % XFSBL_TPM_DIVD_VAL));

	Status = XFsbl_TpmDataTransfer( TpmPcrRead, ResponseBuf, TpmPcrRead[XFSBL_TPM_DATA_SIZE_INDEX]);
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	for (Idx = XFSBL_TPM_PCR_DATA_OFFSET; Idx < (XFSBL_TPM_PCR_DATA_OFFSET + XFSBL_TPM_PCR_DIGEST_SIZE); Idx++){
		XFsbl_Printf(DEBUG_INFO, "%02x", ResponseBuf[Idx]);
	}

END:
	return Status;
}

#endif

#endif /* XFSBL_TPM */
