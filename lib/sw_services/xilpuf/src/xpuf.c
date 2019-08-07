/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
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

/************************** Constant Definitions ****************************/

#define XPUF_STATUS_TIMEOUT			(50000U)
#define XPUF_ONE_MS				(1000U)

/******************************************************************************/
/**
 * @brief
 * This macro waits till Puf Syndrome rdy bit is set.
 *
 * @param   None.
 *
 * @return	XST_SUCCESS - Syndrome word is ready
 *		XST_FAILURE - Timeout occured
 *
 * @note	None.
 *
 ******************************************************************************/

#define XPuf_WaitForPufSynWordRdy()	\
		Xil_WaitForEvent((XPUF_PMC_GLOBAL_BASEADDR + \
				XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET),\
				XPUF_STATUS_SYNDROME_WORD_RDY, \
				XPUF_STATUS_SYNDROME_WORD_RDY, \
				XPUF_STATUS_TIMEOUT)

/******************************************************************************/
/**
 * @brief
 * This macro waits till Puf done bit is set.
 *
 * @param   None.
 *
 * @return	XST_SUCCESS - Puf Operation is done.
 *		XST_FAILURE - Timeout occured
 *
 * @note	None.
 *
 ******************************************************************************/

#define XPuf_WaitForPufDoneStatus()      \
                Xil_WaitForEvent((XPUF_PMC_GLOBAL_BASEADDR + \
                                XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET),\
                                XPUF_STATUS_PUF_DONE, \
                                XPUF_STATUS_PUF_DONE, \
                                XPUF_STATUS_TIMEOUT)

typedef enum {
	XPUF_REGISTRATION_STARTED,
	XPUF_REGISTRATION_COMPLETE
} XPuf_PufRegistrationState;

/************************** Function Prototypes ******************************/

static void XPuf_Capture_PufID(XPuf_Data *PufData);

/************************** Function Definitions *****************************/

/******************************************************************************/
/**
 * @brief
 * This functions provides PUF provisioning data.
 *
 * @param   	PufData - Pointer to PufData structure with Registration mode set
 *
 * @return  	XST_SUCCESS - PUF provisioning completed and data avaiable in
 *				PUF Data variable
 *		XPUF_ERROR_INVALID_PARAM - PufData is NULL
 *		XPUF_ERROR_INVALID_SYNDROME_MODE - Wrong Reg mode
 *		XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT - Timeout occured while
 *						waiting for PUF Syndrome data
 *		XPUF_ERROR_SYNDROME_DATA_OVERFLOW - Syndrom data overflow reported
 *			by PUF controller or more than required data is provided by
 *			PUF controller
 *		XPUF_ERROR_SYNDROME_DATA_UNDERFLOW - Number of syndrom data words
 *					are less than expected number of words
 *		XST_FAILURE - Unexpected event, should not occure in normal scenario
 *
 * @note	None.
 *
 ******************************************************************************/
u32 XPuf_Puf_Registration(XPuf_Data *PufData)
{
	u32 Status = XST_FAILURE;
	u32 MaxSyndromeSizeInWords;
	u32 Idx = 0;
	XPuf_PufRegistrationState RegistrationStatus;

	/* Validate input arguments */
	if (PufData == NULL) {
		Status = XPUF_ERROR_INVALID_PARAM;
		goto END;
	}

	/* Update PUF_CFG0 register */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_CFG0_OFFSET,
			XPUF_CFG0_HASH_SEL);

	/* Update PUF_CFG1 register */
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
	} else {
		Status = XPUF_ERROR_INVALID_SYNDROME_MODE;
                goto END;
        }


	/* Update PUF Shutter value */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_SHUT_OFFSET,
			PufData->ShutterValue);

	 /* Trigger PUF_CMD for PUF Registration */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_CMD_OFFSET,
			XPUF_CMD_PROVISION);

	RegistrationStatus = XPUF_REGISTRATION_STARTED;

	while (RegistrationStatus != XPUF_REGISTRATION_COMPLETE) {

		/* Wait for PUF Syndrome word ready*/
		Status = XPuf_WaitForPufSynWordRdy();
		if (Status != XST_SUCCESS) {
			Status = XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT;
			break;
		}

		PufData->SyndromeData[Idx] = XPuf_ReadReg(
					XPUF_PMC_GLOBAL_BASEADDR,
					XPUF_PMC_GLOBAL_PUF_WORD_OFFSET);

		if (Idx == MaxSyndromeSizeInWords -1) {
			Status  = XPuf_WaitForPufDoneStatus();
			if (Status != XST_SUCCESS) {
				Status = XPUF_ERROR_PUF_DONE_WAIT_TIMEOUT;
				break;
			}
			RegistrationStatus = XPUF_REGISTRATION_COMPLETE;
			/* Capture CHASH & AUX */
			PufData->Chash = XPuf_ReadReg(
					XPUF_PMC_GLOBAL_BASEADDR,
					XPUF_PMC_GLOBAL_PUF_CHASH_OFFSET);
			PufData->Aux = XPuf_ReadReg(
					XPUF_PMC_GLOBAL_BASEADDR,
					XPUF_PMC_GLOBAL_PUF_AUX_OFFSET);

			/* Capture PUF_ID generated */
			XPuf_Capture_PufID(PufData);

			Status = XST_SUCCESS;
		}
		Idx++;
	};

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function regenerate PUF data using helper date stored in eFUSE or in
 * and external memory.
 *
 * @param   	None.
 *
 * @return  	XST_SUCCESS - PUF Regeneration completed and data avaiable in
 *				PUF Data variable
 *		XST_FAILURE - Unexpected event, should not occure in
 *				normal scenario
 *
 * @note	None.
 *
 ******************************************************************************/
u32 XPuf_Puf_Regeneration(XPuf_Data *PufData)
{
	u32 Status = XST_FAILURE;
	u32 PufChash;
	u32 PufAux;
	u32 PufStatus;
	u32 Index;
	u32 Debug = XPUF_DEBUG_GENERAL;

	/* Validate input arguments */
        if (PufData == NULL) {
                Status = XPUF_ERROR_INVALID_PARAM;
                goto END;
        }

	if ((PufData->ReadOption != XPUF_READ_FROM_CACHE) &&
		(PufData->ReadOption != XPUF_READ_FROM_EFUSE) &&
		(PufData->ReadOption != XPUF_READ_FROM_RAM)) {
		Status = XPUF_ERROR_INVALID_PARAM;
                goto END;
        }

	if ((PufData->ReadOption == XPUF_READ_FROM_CACHE) ||
		(PufData->ReadOption == XPUF_READ_FROM_EFUSE)) {

		/* TODO : Call NVM API's to read Chash and Aux */
		Status = XPUF_ERROR_PUF_EFUSE_REGEN_NOT_IMPLEMENTED;
		goto END;

	} else {
		PufChash = PufData->Chash;
		PufAux = PufData->Aux;
	}

	if ((PufChash == 0U) || (PufAux == 0U)) {
		Status = (u32)XPUF_ERROR_CHASH_NOT_PROGRAMMED;
			xPuf_printf(Debug, "PUF regeneration is not allowed"
			", as PUF helper data is not provided/stored\r\n");
		goto END;
	}

	/* Update PUF_CFG0 register */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_CFG0_OFFSET,
			XPUF_CFG0_HASH_SEL);

	/* Update PUF_CFG1 register */
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

	/* Update PUF_SYN Addr in case of on-demand PUF Regeneration*/
	if ((PufData->ReadOption == XPUF_READ_FROM_RAM)) {
		XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
				XPUF_PMC_GLOBAL_PUF_SYN_ADDR_OFFSET,
				PufData->SyndromeAddr);
	}
	/* Update PUF Shutter value */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_SHUT_OFFSET,
			PufData->ShutterValue);

	/* Update PUF Auxiliary data*/
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_AUX_OFFSET,
			PufAux);

	/* Update PUF Chash*/
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_CHASH_OFFSET,
			PufChash);

	/* Trigger PUF_CMD for PUF On-demand Regeneration */
	XPuf_WriteReg(XPUF_PMC_GLOBAL_BASEADDR,
			XPUF_PMC_GLOBAL_PUF_CMD_OFFSET,
			XPUF_CMD_REGENERATION);

	Status  = XPuf_WaitForPufDoneStatus();

	if (Status != XST_SUCCESS) {
		xPuf_printf(Debug,
		"Error: Puf on-demand regeneration failed!! \r\n");
		Status = XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT;
		goto END;
	}

	PufStatus = XPuf_ReadReg(XPUF_PMC_GLOBAL_BASEADDR,
				XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET);
	if (((PufStatus & XPUF_STATUS_KEY_RDY) == XPUF_STATUS_KEY_RDY) &&
		((PufStatus & XPUF_STATUS_ID_RDY) == XPUF_STATUS_ID_RDY)) {

		XPuf_Capture_PufID(PufData);
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function captures PUF_ID generated into XPuf_Data.
 *
 *
 * @param   	None.
 *
 * @return  	None.
 *
 * @note	None.
 *
 ******************************************************************************/

static void XPuf_Capture_PufID(XPuf_Data *PufData)
{
	u32 Index;

	for (Index = 0; Index < XPUF_ID_LENGTH; Index++) {
		PufData->PufID[Index] = XPuf_ReadReg(
				XPUF_PMC_GLOBAL_BASEADDR,
				(XPUF_PMC_GLOBAL_PUF_ID_0_OFFSET +
				(Index * XPUF_WORD_LENGTH)));
	}

}
