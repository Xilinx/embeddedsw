/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
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
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xocp.h"
#include "xocp_hw.h"
#include "xplmi_hw.h"
#include "xplmi.h"
#include "xplmi_plat.h"
#include "xplmi_dma.h"
#include "xil_util.h"
#include "xsecure_sha.h"
#include "xplmi_status.h"
#include "xsecure_init.h"
#include "xsecure_trng.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XOcp_DmeXppuConfig(void);

/************************** Variable Definitions *****************************/

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
 *
 * @return
 *		- XST_SUCCESS - If PCR extend is success
 *		- XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_ExtendPcr(XOcp_RomHwPcr PcrNum, u64 ExtHashAddr)
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

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function gets the PCR value from requested PCR.
 *
 * @param	PcrNum is the variable of enum XOcp_RomHwPcr to select the
 * 		PCR number
 * @param	PcrBuf is the address of the 48 bytes buffer to store the
 * 		requested PCR contents
 *
 * @return
 *		- XST_SUCCESS - If PCR contents are copied
 *		- XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_GetPcr(XOcp_RomHwPcr PcrNum, u64 PcrBuf)
{
	int Status = XST_FAILURE;
	u32 PcrOffset = XOCP_PMC_GLOBAL_PCR_0_0 +
			((u32)PcrNum * XOCP_PCR_SIZE_BYTES);

	if (PcrNum > XOCP_PCR_7) {
		Status = (int)XOCP_PCR_ERR_PCR_SELECT;
	}
	else {
		Status = XOcp_MemCopy(PcrOffset, PcrBuf, XOCP_PCR_SIZE_WORDS,
				XPLMI_PMCDMA_0);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function generates the response to DME challenge request.
 *
 * @param	NonceAddr holds the address of 48 bytes buffer Nonce,
 *		which shall be used to fill one of the member of DME structure
 * @param	DmeResPtr is the pointer to the 224 bytes buffer,
 *		which is used to store the response to DME challenge request of
 *		type XOcp_DmeResponse.
 *
 * @return
 *		- XST_SUCCESS - Upon success and
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
int XOcp_GenerateDmeResponse(u32 NonceAddr, XOcp_DmeResponse *DmeResPtr)
{
	int Status = XST_FAILURE;
	int SStatus = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	u32 *DevIkPubKey = (u32 *)XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0;
	XSecure_Sha3Hash Sha3Hash;
	XSecure_Sha3 Sha3Instance = {0U};
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(0U);
	XOcp_Dme *DmePtr = &DmeResPtr->Dme;
	XSecure_TrngInstance *TrngInstance = NULL;

	/* Fill the DME structure with DEVICE ID */
	Status = XSecure_Sha3Initialize(&Sha3Instance, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_Sha3Digest(&Sha3Instance, (UINTPTR)DevIkPubKey,
			XOCP_SIZE_OF_ECC_P384_PUBLIC_KEY_BYTES, &Sha3Hash);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = Xil_SMemCpy((void *)DmePtr->DeviceID,
			 XSECURE_HASH_SIZE_IN_BYTES,
			(const void *)Sha3Hash.Hash,
			 XSECURE_HASH_SIZE_IN_BYTES,
			 XSECURE_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Fill the DME structure with Nonce */
	Status = Xil_SMemCpy((void *)DmePtr->Nonce,
			 XOCP_DME_NONCE_SIZE_BYTES,
			(const void *)(UINTPTR)NonceAddr,
			XOCP_DME_NONCE_SIZE_BYTES,
			XOCP_DME_NONCE_SIZE_BYTES);

	/* Mention the Address and Size of DME structure for ROM service */
	XPlmi_Out32(PMC_GLOBAL_GLOBAL_GEN_STORAGE5, (u32)DmePtr);
	XPlmi_Out32(PMC_GLOBAL_GLOBAL_GEN_STORAGE6, sizeof(XOcp_Dme));
	XOcp_DmeXppuConfig();

	Status = XPlmi_RomISR(XPLMI_DME_CHL_SIGN_GEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/* Disabling XPPU */
	Xil_Out32(PMC_XPPU_CTRL, PMC_XPPU_CTRL_DEFVAL);

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
	/*
	 * ROM uses TRNG for DME service and resets the core after the usage
	 * in this case TRNG state should be set to uninitialized state
	 * so that PLM can re-initialize during runtime requests.
	 */
	TrngInstance = XSecure_GetTrngInstance();
	if (TrngInstance->State != XSECURE_TRNG_UNINITIALIZED_STATE){
		SStatus = XSecure_TrngUninstantiate(TrngInstance);
		if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
			Status = SStatus | XOCP_DME_ERR;
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
