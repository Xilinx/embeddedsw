/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp.c
*
* This file contains the implementation of the interface functions for DME
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   vns  06/26/22 Initial release
* 1.1   kal  01/05/23 Added PCR Extend and Pcr Logging functions
*       am   01/10/23 Modified function argument type to u64 in
*                     XOcp_GenerateDmeResponse().
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xocp.h"
#include "xocp_keymgmt.h"
#include "xocp_hw.h"
#include "xplmi_hw.h"
#include "xplmi.h"
#include "xplmi_plat.h"
#include "xplmi_dma.h"
#include "xil_util.h"
#include "xsecure_sha384.h"
#include "xplmi_status.h"
#include "xsecure_init.h"
#include "xsecure_trng.h"
#include "xil_error_node.h"
#include "xplmi_err.h"

/************************** Constant Definitions *****************************/
#define XOCP_SHA3_LEN_IN_BYTES		(48U)

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XOcp_DmeXppuConfig(void);
static int XOcp_UpdateHwPcrLog(u8 PcrNum, u64 ExtHashAddr, u32 DataSize);
static int XOcp_CountNumOfOnes(u32 Num);

/************************** Variable Definitions *****************************/
static XOcp_HwPcrLog HwPcrLog = {0U};

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function extends the PCR with provided hash by requesting
 * 		ROM service
 *
 * @param	PcrNum is the variable of enum XOcp_RomHwPcr to select the PCR
 * 		to be extended.
 * @param	ExtHashAddr is the address of the buffer which holds the hash
 *		to extended.
 * @param 	DataSize Data Size to be extended
 *
 * @return
 *		- XST_SUCCESS - If PCR extend is success
 *		- XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_ExtendHwPcr(XOcp_RomHwPcr PcrNum, u64 ExtHashAddr, u32 DataSize)
{
	int Status = XST_FAILURE;
	u32 RegValue;

	if ((PcrNum < XOCP_PCR_2) || (PcrNum > XOCP_PCR_7)) {
		Status = XOCP_PCR_ERR_PCR_SELECT;
		goto END;
	}

	Status = XOcp_MemCopy(ExtHashAddr, XOCP_PMC_GLOBAL_PCR_EXTEND_INPUT_0,
					XOCP_PCR_SIZE_WORDS, XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XPlmi_Out32(XOCP_PMC_GLOBAL_PCR_OP,
			(u32)PcrNum << XOCP_PMC_GLOBAL_PCR_OP_IDX_SHIFT);
	Status = XPlmi_RomISR(XPLMI_PCR_OP);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/* Check PCR extend status */
	RegValue = XPlmi_In32(XOCP_PMC_GLOBAL_PCR_OP_STATUS);
	if ((RegValue & XOCP_PMC_GLOBAL_PCR_OP_STATUS_DONE_MASK) == 0x0U) {
		Status = XOCP_PCR_ERR_NOT_COMPLETED;
	}
	if ((RegValue & XOCP_PMC_GLOBAL_PCR_OP_STATUS_ERROR_MASK) != 0x0U) {
		Status = (int)XOCP_PCR_ERR_OPERATION;
	}

	if (Status == XST_SUCCESS) {
		/* Update HwPcr Log */
		Status = XOcp_UpdateHwPcrLog(PcrNum, ExtHashAddr, DataSize);
		if (Status != XST_SUCCESS) {
			Status = (int)XOCP_PCR_ERR_IN_UPDATE_LOG;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function reads the HWPCR log into the user provided
 * 		buffer.
 *
 * @param	Log 		Pointer to the XOcp_HwPcrEvent
 * @param	NumOfLogEntries	Maximum number of log entries to be read
 *
 * @return
 *		- XST_SUCCESS - If log read is successful
 *		- XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_GetHwPcrLog(u64 LogAddr, u32 NumOfLogEntries)
{
	int Status = XST_FAILURE;
	XOcp_HwPcrLog *Log = (XOcp_HwPcrLog *)(UINTPTR)LogAddr;

	if (HwPcrLog.TailIndex == HwPcrLog.HeadIndex) {
		Status = XST_SUCCESS;
		goto END;
	}
	if (NumOfLogEntries > XOCP_MAX_NUM_OF_HWPCR_EVENTS) {
		Status = XOCP_PCR_ERR_INVALID_LOG_READ_REQUEST;
		goto END;
	}
	if ((HwPcrLog.TailIndex + NumOfLogEntries) > HwPcrLog.HeadIndex) {
		Status = XOCP_PCR_ERR_INVALID_LOG_READ_REQUEST;
		goto END;
       }

	Status = XPlmi_MemCpy64(LogAddr, (u64)(UINTPTR)&HwPcrLog,
		(NumOfLogEntries * sizeof(XOcp_HwPcrEvent)));
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if (HwPcrLog.TailIndex ==
		((sizeof(HwPcrLog.Buffer)/ sizeof(XOcp_HwPcrEvent))) - 1U) {
		HwPcrLog.TailIndex = 0;
	} else {
		HwPcrLog.TailIndex = HwPcrLog.TailIndex + NumOfLogEntries;
	}

	Log->HeadIndex = HwPcrLog.HeadIndex;
	Log->TailIndex = HwPcrLog.TailIndex;
	Log->OverFlowFlag = HwPcrLog.OverFlowFlag;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function gets the PCR value from requested PCRs.
 *
 * @param	PcrNum is the variable of enum XOcp_RomHwPcr to select the
 * 		PCR number
 * @param	PcrBuf is the address of the 48 bytes buffer to store the
 * 		requested PCR contents
 * @param	PcrBufSize is the Size of the PCR buffer provided
 *
 * @return
 *		- XST_SUCCESS - If PCR contents are copied
 *		- XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_GetHwPcr(u32 PcrMask, u64 PcrBuf, u32 PcrBufSize)
{
	int Status = XST_FAILURE;
	int NumOfBitsSetInMask;
	u32 Index = 0U;
	u32 Mask = PcrMask;
	u32 PcrOffset;
	u32 PcrNum;

	NumOfBitsSetInMask = XOcp_CountNumOfOnes(Mask);
	if (PcrBufSize != (NumOfBitsSetInMask * XOCP_PCR_SIZE_BYTES)) {
		Status = XOCP_PCR_ERR_IN_GET_PCR;
		goto END;
	}
	while (Mask != 0x0U) {
		if (Mask & 0x01U) {
			PcrNum = Index;
			PcrOffset = XOCP_PMC_GLOBAL_PCR_0_0 +
					((u32)PcrNum * XOCP_PCR_SIZE_BYTES);
			if (PcrNum > XOCP_PCR_7) {
				Status = (int)XOCP_PCR_ERR_PCR_SELECT;
				goto END;
			}
			Status = XOcp_MemCopy(PcrOffset, PcrBuf,
					XOCP_PCR_SIZE_WORDS,
					XPLMI_PMCDMA_0);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			PcrBuf = PcrBuf + XOCP_PCR_SIZE_BYTES;
		}
		Mask = Mask >> 1U;
		Index++;
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function generates the response to DME challenge request.
 *
 * @param	NonceAddr holds the address of 48 bytes buffer Nonce,
 *		which shall be used to fill one of the member of DME structure
 * @param	DmeStructResAddr is the address to the 224 bytes buffer,
 *		which is used to store the response to DME challenge request of
 *		type XOcp_DmeResponse.
 *
 * @return
 *		- XST_SUCCESS - Upon success and
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
int XOcp_GenerateDmeResponse(u64 NonceAddr, u64 DmeStructResAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	u32 *DevIkPubKey = (u32 *)XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0;
	u8 Sha3Hash[XOCP_SHA3_LEN_IN_BYTES];
	XOcp_DmeResponse *DmeResPtr = (XOcp_DmeResponse *)(UINTPTR)DmeStructResAddr;
	XOcp_Dme Dme;
	XOcp_Dme *DmePtr = &Dme;
	XSecure_TrngInstance *TrngInstance = NULL;

	/* Zeorizing the DME structure */
	Status = Xil_SMemSet((void *)DmePtr,
				sizeof(XOcp_Dme), 0U, sizeof(XOcp_Dme));
	if (Status != XST_SUCCESS) {
		goto RET;
	}

	/* Fill the DME structure's DEVICE ID field with hash of DEV IK Public key */
	if (XOcp_IsDevIkReady() != FALSE) {
		Status = XSecure_Sha384Digest((u8 *)(UINTPTR)DevIkPubKey,
				XOCP_SIZE_OF_ECC_P384_PUBLIC_KEY_BYTES, Sha3Hash);
		if (Status != XST_SUCCESS) {
			goto RET;
		}
		Status = Xil_SMemCpy((void *)DmePtr->DeviceID,
			 XOCP_SHA3_LEN_IN_BYTES,
			(const void *)Sha3Hash,
			 XOCP_SHA3_LEN_IN_BYTES,
			 XOCP_SHA3_LEN_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto RET;
		}
	}

	/* Fill the DME structure with Nonce */
	Status = Xil_SMemCpy((void *)DmePtr->Nonce,
			 XOCP_DME_NONCE_SIZE_BYTES,
			(const void *)(UINTPTR)NonceAddr,
			XOCP_DME_NONCE_SIZE_BYTES,
			XOCP_DME_NONCE_SIZE_BYTES);
	if (Status != XST_SUCCESS) {
		goto RET;
	}


	/* Mention the Address and Size of DME structure for ROM service */
	XPlmi_Out32(PMC_GLOBAL_GLOBAL_GEN_STORAGE5, (u32)DmePtr);
	XPlmi_Out32(PMC_GLOBAL_GLOBAL_GEN_STORAGE6, sizeof(XOcp_Dme));
	XOcp_DmeXppuConfig();

	Status = XPlmi_RomISR(XPLMI_DME_CHL_SIGN_GEN);
	if (Status != XST_SUCCESS) {
		goto RET;
	}
	/* Disabling XPPU */
	Xil_Out32(PMC_XPPU_CTRL, PMC_XPPU_CTRL_DEFVAL);

	/* Check if any ROM error occurred during DME request */
	Status = Xil_In32(PMC_GLOBAL_PMC_BOOT_ERR);
	if (Status != 0x0U) {
		Status = XOCP_DME_ROM_ERROR;
		goto RET;
	}

	/* Copy the contents to user DME response structure */
	Status = Xil_SMemCpy(&DmeResPtr->Dme,
				sizeof(XOcp_Dme),
				&Dme,
				sizeof(XOcp_Dme),
				sizeof(XOcp_Dme));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemCpy(DmeResPtr->DmeSignatureR,
			XOCP_ECC_P384_SIZE_BYTES,
			(const void *)XOCP_PMC_GLOBAL_DME_CHALLENGE_SIGNATURE_R_0,
			XOCP_ECC_P384_SIZE_BYTES,
			XOCP_ECC_P384_SIZE_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemCpy(DmeResPtr->DmeSignatureS,
			XOCP_ECC_P384_SIZE_BYTES,
			(const void *)XOCP_PMC_GLOBAL_DME_CHALLENGE_SIGNATURE_S_0,
			XOCP_ECC_P384_SIZE_BYTES,
			XOCP_ECC_P384_SIZE_BYTES);

END:
	if (Status != XST_SUCCESS) {
		/* Upon any failure whole response structure is cleared */
		ClearStatus = Xil_SMemSet((void *)DmeResPtr,
			sizeof(XOcp_DmeResponse), 0U, sizeof(XOcp_DmeResponse));
		if (ClearStatus != XST_SUCCESS) {
			Status = Status | XLOADER_SEC_BUF_CLEAR_ERR;
		}
		else {
			Status = Status | XLOADER_SEC_BUF_CLEAR_SUCCESS;
		}
		Status = Status | XOCP_DME_ERR;
	}
RET:
	/*
	 * ROM uses TRNG for DME service and resets the core after the usage
	 * in this case TRNG state should be set to uninitialized state
	 * so that PLM can re-initialize during runtime requests.
	 */
	TrngInstance = XSecure_GetTrngInstance();
	if (TrngInstance->State != XSECURE_TRNG_UNINITIALIZED_STATE){
		SStatus = XSecure_TrngUninstantiate(TrngInstance);
		if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
			if(SStatus != XST_SUCCESS) {
				Status = SStatus | XOCP_DME_ERR;
			}
		}
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function configures the XPPU for requesting DME service
 *		to ROM.
 *
 * @return	None.
 ******************************************************************************/
static void XOcp_DmeXppuConfig(void)
{
	Xil_Out32(PMC_XPPU_MASTER_ID00, XOCP_XPPU_MASTER_ID0_CONFIG_VAL);
	Xil_Out32(PMC_XPPU_APERPERM_032, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL);
	Xil_Out32(PMC_XPPU_APERPERM_035, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL);
	Xil_Out32( PMC_XPPU_APERPERM_049, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL);

	Xil_Out32( PMC_XPPU_APERPERM_028, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL);
	Xil_Out32( PMC_XPPU_APERPERM_030, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL);
	Xil_Out32( PMC_XPPU_APERPERM_037, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL);
	Xil_Out32( PMC_XPPU_APERPERM_038, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL);
	Xil_Out32(PMC_XPPU_APERPERM_386, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL);

	Xil_Out32(PMC_XPPU_MASTER_ID01, XOCP_XPPU_MASTER_ID1_CONFIG_VAL);
	Xil_Out32(PMC_XPPU_APERPERM_017, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL);
	Xil_Out32(PMC_XPPU_APERPERM_018, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL);
	Xil_Out32(PMC_XPPU_APERPERM_019, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL);
	Xil_Out32(PMC_XPPU_APERPERM_020, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL);
	Xil_Out32(PMC_XPPU_APERPERM_021, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL);

	Xil_Out32(PMC_XPPU_APERPERM_146, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL);
	Xil_Out32(PMC_XPPU_APERPERM_147, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL);

	Xil_Out32(PMC_XPPU_DYNAMIC_RECONFIG_APER_ADDR,
				XOCP_XPPU_DYNAMIC_RECONFIG_APER_SET_VALUE);
	Xil_Out32(PMC_XPPU_DYNAMIC_RECONFIG_APER_PERM,
		XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL);
	Xil_Out32(PMC_XPPU_DYNAMIC_RECONFIG_EN, PMC_XPPU_DYNAMIC_RECONFIG_EN_DEFVAL);

	Xil_Out32(PMC_XPPU_CTRL, PMC_XPPU_CTRL_ENABLE_MASK);
}

/*****************************************************************************/
/**
 * @brief	This function updates the HWPCR log.
 *
 * @param	PcrNum		PCR register number
 * @param	ExtHashAddr	Address of the hash to be extended
 * @param	DataSize	Size of the Data
 *
 * @return
 *		- XST_SUCCESS - If log update is successful
 *		- XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
static int XOcp_UpdateHwPcrLog(u8 PcrNum, u64 ExtHashAddr, u32 DataSize)
{
	int Status = XST_FAILURE;

	HwPcrLog.Buffer[HwPcrLog.HeadIndex].PcrNo = PcrNum;
	Status = XOcp_MemCopy(ExtHashAddr,
		(u64)(UINTPTR)HwPcrLog.Buffer[HwPcrLog.HeadIndex].Hash,
		DataSize / XOCP_WORD_LEN, XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XOcp_MemCopy((XOCP_PMC_GLOBAL_PCR_0_0 +
		(PcrNum * XOCP_PCR_SIZE_BYTES)),
		(u64)(UINTPTR)HwPcrLog.Buffer[HwPcrLog.HeadIndex].PcrValue,
		XOCP_PCR_SIZE_WORDS, XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if (HwPcrLog.HeadIndex ==
		((sizeof(HwPcrLog.Buffer)/ sizeof(XOcp_HwPcrEvent))) - 1U) {
		HwPcrLog.HeadIndex = 0U;
		HwPcrLog.OverFlowFlag = TRUE;
	} else {
		HwPcrLog.HeadIndex++;
	}

	XPlmi_HandleSwError(XIL_NODETYPE_EVENT_ERROR_SW_ERR,
			XIL_EVENT_ERROR_PCR_LOG_UPDATE);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function counts number of 1's in the given number.
 *
 * @param	Num	Given number to check number of set bits
 *
 * @return	Returns Count number of bits set
 *
 ******************************************************************************/
static int XOcp_CountNumOfOnes(u32 Num)
{
	int Count = 0U;

	while (Num != 0U)
	{
		Num = Num & (Num - 1U);
		Count++;
	}

	return Count;
}
