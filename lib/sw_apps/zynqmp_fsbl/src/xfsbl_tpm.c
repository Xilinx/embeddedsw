/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
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

	Status = XFsbl_TpmStartUp();
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	Status = XFsbl_TpmSelfTest();
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

	Status = XFsbl_TpmDataTransfer(TpmPcrExtend, TpmRespBuffer,
		TpmPcrExtend[XFSBL_TPM_DATA_SIZE_INDEX]);

	if ((TpmRespBuffer[6U] != 0U) || (TpmRespBuffer[7U] != 0U) ||
		(TpmRespBuffer[8U] != 0U) || (TpmRespBuffer[9U] != 0U))
	{
		if (Status == XFSBL_SUCCESS)
		{
			Status = XFSBL_FAILURE;
		}
	}

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
	u8 Index;

	for (Index = 0U; Index < XFSBL_HASH_TYPE_SHA3; ++Index) {
		XFsbl_Printf(DEBUG_INFO, "%02x", PartitionHash[Index]);
	}
	XFsbl_Printf(DEBUG_INFO, "\n\r");


	TpmPcrExtend[XFSBL_TPM_DATA_SIZE_INDEX] = XFSBL_TPM_PCR_EXT_CMD_SIZE +
		XFSBL_TPM_DIGEST_SIZE;
	TpmPcrExtend[XFSBL_TPM_PCR_EXTEND_INDEX] = PcrIndex;
	(void)XFsbl_MemCpy((void *)&TpmPcrExtend[XFSBL_TPM_PCR_EXT_CMD_SIZE],
		(void *)PartitionHash, XFSBL_TPM_DIGEST_SIZE);

	Status = XFsbl_TpmDataTransfer(TpmPcrExtend, TpmRespBuffer,
		TpmPcrExtend[XFSBL_TPM_DATA_SIZE_INDEX]);
	if (Status != XST_SUCCESS)
	{
		goto END;
	}

	if ((TpmRespBuffer[6U] != 0U) || (TpmRespBuffer[7U] != 0U) ||
		(TpmRespBuffer[8U] != 0U) || (TpmRespBuffer[9U] != 0U))
	{
		if (Status == XFSBL_SUCCESS)
		{
			Status = XFSBL_FAILURE;
		}
	}

END:
	return Status;
}
#endif /* XFSBL_TPM */