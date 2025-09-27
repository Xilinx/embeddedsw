/***************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_dice_dme.c
*
* This file contains the implementation of DME challenge signature for versal_net.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.5   tvp  06/05/25 Initial release
*
* </pre>
*
***************************************************************************************************/

/*************************************** Include Files ********************************************/

#include "xplmi_config.h"
#ifdef PLM_OCP
#include "xocp.h"
#include "xocp_plat.h"
#include "xocp_hw.h"
#include "xocp_sha.h"
#include "xocp_keymgmt.h"
#include "xocp_dice_dme.h"
#include "xplmi.h"
#include "xplmi_tamper.h"
#include "xsecure_plat_kat.h"
#include "xsecure_kat.h"

/************************************ Constant Definitions ****************************************/
#define XOCP_XPPU_MAX_APERTURES		(19U) /**< Maximum XPPU apertures */
#define XOCP_XPPU_ENABLED		(0x46E56A7CU) /**< XPPU enabled */
#define XOCP_XPPU_DISABLED		(~XOCP_XPPU_ENABLED) /**< XPPU disabled */
#define XOCP_XPPU_MASTER_ID_0		(17U) /**< XPPU master id 0 */
#define XOCP_XPPU_MASTER_ID_1		(18U) /**< XPPU master id 1 */

/************************************** Type Definitions ******************************************/
static XOcp_DmeXppuCfg XOcp_DmeXppuCfgTable[XOCP_XPPU_MAX_APERTURES] =
{
	{PMC_XPPU_APERPERM_017, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL, 0U, 0U},
	/* PMC Global register space */
	{PMC_XPPU_APERPERM_018, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL, 0U, 0U},
	/* PMC Global register space */
	{PMC_XPPU_APERPERM_019, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL, 0U, 0U},
	/* PMC Global register space */
	{PMC_XPPU_APERPERM_020, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL, 0U, 0U},
	/* PMC Global register space */
	{PMC_XPPU_APERPERM_021, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL, 0U, 0U},
	/* PMC TAP */
	{PMC_XPPU_APERPERM_026, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL, 0U, 0U},
	/* PMC TAP */
	{PMC_XPPU_APERPERM_027, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL, 0U, 0U},
	/* PMC DMA0 */
	{PMC_XPPU_APERPERM_028, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* AES */
	{PMC_XPPU_APERPERM_030, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* ECDSA RSA */
	{PMC_XPPU_APERPERM_032, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* SHA0 */
	{PMC_XPPU_APERPERM_033, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* TRNG */
	{PMC_XPPU_APERPERM_035, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* EFUSE CACHE */
	{PMC_XPPU_APERPERM_037, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* CRP */
	{PMC_XPPU_APERPERM_038, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* PPU1 RAM */
	{PMC_XPPU_APERPERM_386, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* Configure [23:16] bits of Aperture_049 address */
	{PMC_XPPU_DYNAMIC_RECONFIG_APER_ADDR, XOCP_XPPU_DYNAMIC_RECONFIG_APER_SET_VALUE, 0U, 0U},
	/* Configure PPU0 to enable reconfiguration and PPU1 to configure XPPU registers after DME operation */
	{PMC_XPPU_DYNAMIC_RECONFIG_APER_PERM, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL, 0U, 0U},
	/* MASTER ID 00 */
	{PMC_XPPU_MASTER_ID00, XOCP_XPPU_MASTER_ID0_PPU0_CONFIG_VAL, 0U, 0U},
	/* MASTER ID 01 */
	{PMC_XPPU_MASTER_ID01, XOCP_XPPU_MASTER_ID1_PPU1_CONFIG_VAL, 0U, 0U},
};

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Function Prototypes *****************************************/
static int XOcp_DmeStoreXppuDefaultConfig(void);
static int XOcp_DmeRestoreXppuDefaultConfig(void);

/************************************ Variable Definitions ****************************************/

/************************************ Function Definitions ****************************************/

/**************************************************************************************************/
/**
 * @brief	This function generates the response to DME challenge request and configures the
 * 		XPPU for requesting DME service to ROM.
 *
 * @param	NonceAddr holds the address of 32 bytes buffer Nonce, which shall be used to fill
 * 		one of the member of DME structure.
 *
 * @param	DmeStructResAddr is the address to the 224 bytes buffer, which is used to store the
 * 		response to DME challenge request of type XOcp_DmeResponse.
 *
 * @return
 *		- XST_SUCCESS On success
 *		- XST_FAILURE On failure
 *
 **************************************************************************************************/
int XOcp_GenerateDmeResponseImpl(u64 NonceAddr, u64 DmeStructResAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile int XppuStatus = XST_FAILURE;
	volatile u32 RegVal = XOCP_PMC_XPPU_CTRL_DISABLE_VAL;
	volatile u32 RegValtmp = XOCP_PMC_XPPU_CTRL_DISABLE_VAL;
	int ClearStatus = XST_FAILURE;
	XOcp_RegSpace* XOcp_Reg = XOcp_GetRegSpace();
#ifdef PLM_OCP_KEY_MNGMT
	u32 *DevIkPubKey = (u32 *)(UINTPTR)XOcp_Reg->DevIkPubXAddr;
	u8 Sha3Hash[XOCP_SHA3_LEN_IN_BYTES];
#endif
	XOcp_Dme RomDmeInput;
	XOcp_DmeResponse *DmeResponse = XOcp_GetDmeResponse();
	XOcp_Dme *DmePtr = &RomDmeInput;
	XSecure_TrngInstance *TrngInstance = NULL;
	volatile u32 XppuEnabled = XOCP_XPPU_DISABLED;
	volatile u32 XppuEnabledTmp = XOCP_XPPU_DISABLED;
	u32 Index;
	u32 Aper049InitVal = Xil_In32(PMC_XPPU_APERPERM_049);

	/*
	 * Check if XPPU_LOCK is enabled.
	 * Check if Dynamic reconfiguration is enabled by default.
	 */
	if ((Xil_In32(PMC_XPPU_LOCK) != PMC_XPPU_LOCK_DEFVAL) ||
		(Xil_In32(PMC_XPPU_DYNAMIC_RECONFIG_EN) != PMC_XPPU_DYNAMIC_RECONFIG_EN_DEFVAL)) {
		Status = (int)XOCP_ERR_INVALID_XPPU_CONFIGURATION;
		goto RET;
	}

	/* Zeorizing the DME structure */
	Status = Xil_SMemSet((void *)(UINTPTR)DmePtr, sizeof(XOcp_Dme), 0U, sizeof(XOcp_Dme));
	if (Status != XST_SUCCESS) {
		goto RET;
	}

#ifdef PLM_OCP_KEY_MNGMT
	/* Fill the DME structure's DEVICE ID field with hash of DEV IK Public key */
	if (XOcp_IsDevIkReady() != FALSE) {
		if (XPlmi_IsKatRan(XPLMI_SECURE_SHA384_KAT_MASK) != TRUE) {
			XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XOCP_ERR_KAT_FAILED, Status, SStatus,
							XSecure_Sha384Kat);
			if ((Status != XST_SUCCESS) || (SStatus != XST_SUCCESS)) {
				goto RET;
			}
			XPlmi_SetKatMask(XPLMI_SECURE_SHA384_KAT_MASK);
		}
		Status = XOcp_ShaDigest((u8 *)(UINTPTR)DevIkPubKey,
				XOCP_SIZE_OF_ECC_P384_PUBLIC_KEY_BYTES, Sha3Hash);
		if (Status != XST_SUCCESS) {
			goto RET;
		}
		Status = Xil_SMemCpy((void *)(UINTPTR)DmePtr->DeviceID, XOCP_SHA3_LEN_IN_BYTES,
			(const void *)(UINTPTR)Sha3Hash, XOCP_SHA3_LEN_IN_BYTES,
			XOCP_SHA3_LEN_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto RET;
		}
	}
#endif

	/* Fill the DME structure with Nonce */
	Status = XPlmi_MemCpy64((u64)(UINTPTR)DmePtr->Nonce, NonceAddr, XOCP_DME_NONCE_SIZE_BYTES);
	if (Status != XST_SUCCESS) {
		goto RET;
	}

	/* Store the XPPU registers initial configuration */
	XSECURE_TEMPORAL_IMPL(Status, SStatus, XOcp_DmeStoreXppuDefaultConfig);
	if ((Status != XST_SUCCESS) || (SStatus != XST_SUCCESS)) {
		Status |= XOCP_DME_ERR;
		goto RET;
	}

	for (Index = 0U; Index < XOCP_XPPU_MAX_APERTURES; Index++) {
		if (Index == XOCP_XPPU_MASTER_ID_0) {
			if (XOCP_XPPU_MASTER_ID0_PPU0_CONFIG_VAL != Xil_In32(PMC_XPPU_MASTER_ID00)) {
				XOcp_DmeXppuCfgTable[Index].IsModified = TRUE;
			}
		} else if (Index == XOCP_XPPU_MASTER_ID_1) {
			if (XOCP_XPPU_MASTER_ID1_PPU1_CONFIG_VAL != Xil_In32(PMC_XPPU_MASTER_ID01)) {
				XOcp_DmeXppuCfgTable[Index].IsModified = TRUE;
			}
		} else {
			/** - All other apertures always need modification */
			XOcp_DmeXppuCfgTable[Index].IsModified = TRUE;
		}
		/* Configure the XPPU Apertures with configuration */
		Xil_Out32(XOcp_DmeXppuCfgTable[Index].XppuAperAddr,
				XOcp_DmeXppuCfgTable[Index].XppuAperWriteCfgVal);
	}

	/* Enabling Dynamic Reconfiguration */
	Xil_Out32(PMC_XPPU_DYNAMIC_RECONFIG_EN, PMC_XPPU_DYNAMIC_RECONFIG_EN_DEFVAL);

	/* If XPPU is not enabled, enable XPPU */
	RegVal = (Xil_In32(PMC_XPPU_CTRL) & PMC_XPPU_CTRL_ENABLE_MASK);
	RegValtmp = (Xil_In32(PMC_XPPU_CTRL) & PMC_XPPU_CTRL_ENABLE_MASK);
	if ((RegVal != XOCP_PMC_XPPU_CTRL_ENABLE_VAL) || (RegVal != XOCP_PMC_XPPU_CTRL_ENABLE_VAL)) {
		Status = Xil_SecureRMW32(PMC_XPPU_CTRL, PMC_XPPU_CTRL_ENABLE_MASK,
				XOCP_PMC_XPPU_CTRL_ENABLE_VAL);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		XppuEnabled = XOCP_XPPU_ENABLED;
		XppuEnabledTmp = XOCP_XPPU_ENABLED;
	}

	/* XPPU */
	Xil_Out32(PMC_XPPU_APERPERM_049, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL);

	/* Mention the Address and Size of DME structure for ROM service */
	XPlmi_Out32(PMC_GLOBAL_GLOBAL_GEN_STORAGE5, (u32)(UINTPTR)DmePtr);
	XPlmi_Out32(PMC_GLOBAL_GLOBAL_GEN_STORAGE6, sizeof(XOcp_Dme));

	Status = XPlmi_RomISR(XPLMI_DME_CHL_SIGN_GEN);
	if (Status != XST_SUCCESS) {
		Status = (int)XOCP_DME_ROM_ERROR;
		goto END;
	}

	/* Check if any ROM error occurred during DME request */
	Status = (int)Xil_In32(PMC_GLOBAL_PMC_BOOT_ERR);
	if (Status != XST_SUCCESS) {
		Status = (int)XOCP_DME_ROM_ERROR;
		goto END;
	}

	/* Copy the contents to user DME response structure */
	Status = Xil_SChangeEndiannessAndCpy((u8*)(UINTPTR)DmeResponse->DmeSignatureR,
				XOCP_ECC_P384_SIZE_BYTES, (const u8 *)XOcp_Reg->DmeSignRAddr,
				XOCP_ECC_P384_SIZE_BYTES, XOCP_ECC_P384_SIZE_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SChangeEndiannessAndCpy((u8*)(UINTPTR)DmeResponse->DmeSignatureS,
				XOCP_ECC_P384_SIZE_BYTES, (const u8 *)XOcp_Reg->DmeSignSAddr,
				XOCP_ECC_P384_SIZE_BYTES, XOCP_ECC_P384_SIZE_BYTES);
END:
	if ((RegVal == XOCP_PMC_XPPU_CTRL_ENABLE_VAL) &&
		(RegValtmp == XOCP_PMC_XPPU_CTRL_ENABLE_VAL)) {
		XppuStatus = XOcp_DmeRestoreXppuDefaultConfig();
		if (XppuStatus != XST_SUCCESS) {
			if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
				Status = XppuStatus;
			}
			goto RET;
		}
	}
	XSECURE_TEMPORAL_IMPL(Status, SStatus, Xil_SecureOut32, PMC_XPPU_APERPERM_049,
				Aper049InitVal);
	if ((Status != XST_SUCCESS) || (SStatus != XST_SUCCESS)) {
		Status |= XOCP_DME_ERR;
		goto RET;
	}

	XSECURE_TEMPORAL_IMPL(Status, SStatus, Xil_SecureOut32, PMC_XPPU_DYNAMIC_RECONFIG_EN,
				XOCP_XPPU_DYNAMIC_RECONFIG_DISABLE_VAL);
	if ((Status != XST_SUCCESS) || (SStatus != XST_SUCCESS)) {
		Status |= XOCP_DME_ERR;
		goto RET;
	}

	if ((XppuEnabled == XOCP_XPPU_ENABLED) && (XppuEnabledTmp == XOCP_XPPU_ENABLED)) {
		XppuStatus = Xil_SecureRMW32(PMC_XPPU_CTRL, PMC_XPPU_CTRL_ENABLE_MASK,
			XOCP_PMC_XPPU_CTRL_DISABLE_VAL);
		if (XppuStatus != XST_SUCCESS) {
			if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
				Status = XppuStatus | XOCP_DME_ERR;
			}
			goto RET;
		}
	}

	if (Status == XST_SUCCESS) {
		Status = Xil_SMemCpy(&DmeResponse->Dme, sizeof(XOcp_Dme), DmePtr, sizeof(XOcp_Dme),
					sizeof(XOcp_Dme));
		if (Status != XST_SUCCESS) {
			Status = Status | XOCP_DME_ERR;
		}
		Status = XPlmi_MemCpy64(DmeStructResAddr, (u64)(UINTPTR)DmeResponse,
			sizeof(XOcp_DmeResponse));
		if (Status != XST_SUCCESS) {
			Status = Status | XOCP_DME_ERR;
		}
	}
RET:
	if (Status != XST_SUCCESS) {
		ClearStatus = XPlmi_MemSet((u64)(UINTPTR)&RomDmeInput, 0U, sizeof(XOcp_Dme) /
			XOCP_WORD_LEN);
		if (ClearStatus != XST_SUCCESS) {
			Status = Status | XLOADER_SEC_BUF_CLEAR_ERR;
		} else {
			Status = Status | XLOADER_SEC_BUF_CLEAR_SUCCESS;
		}
	}

	/*
	 * ROM uses TRNG for DME service and resets the core after the usage
	 * in this case TRNG state should be set to uninitialized state
	 * so that PLM can re-initialize during runtime requests.
	 */
	TrngInstance = XSecure_GetTrngInstance();
	if (XSecure_TrngIsInitialized(TrngInstance)){
		SStatus = XSecure_Uninstantiate(TrngInstance);
		if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
			if (SStatus != XST_SUCCESS) {
				Status = SStatus | XOCP_DME_ERR;
			}
		}
		XSecure_UpdateTrngCryptoStatus(XSECURE_CLEAR_BIT);
	}

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function stores default XPPU aperture configuration before DME operation.
 *
 * @return
 * 		- XST_SUCCESS On success.
 * 		- XST_FAILURE On failure.
 *
 **************************************************************************************************/
static int XOcp_DmeStoreXppuDefaultConfig(void)
{
	volatile u32 Index;
	int Status = XST_FAILURE;

	for (Index = 0U; Index < XOCP_XPPU_MAX_APERTURES; Index++) {
		XOcp_DmeXppuCfgTable[Index].XppuAperReadCfgVal =
			Xil_In32(XOcp_DmeXppuCfgTable[Index].XppuAperAddr);
	}

	if (Index == XOCP_XPPU_MAX_APERTURES) {
		Status = XST_SUCCESS;
	}

	return Status;

}

/**************************************************************************************************/
/**
 * @brief	This function restores default XPPU aperture configuration after DME operation.
 *
 * @return
 * 		- XST_SUCCESS On success.
 * 		- XST_FAILURE On failure.
 *
 **************************************************************************************************/
static int XOcp_DmeRestoreXppuDefaultConfig(void)
{
	volatile u32 Index;
	int Status = XST_FAILURE;

	/* Restore XPPU registers to their previous state */
	for (Index = 0U; Index < XOCP_XPPU_MAX_APERTURES; Index++) {
		if (
		(XOcp_DmeXppuCfgTable[Index].XppuAperAddr == PMC_XPPU_DYNAMIC_RECONFIG_APER_ADDR) ||
		(XOcp_DmeXppuCfgTable[Index].XppuAperAddr == PMC_XPPU_DYNAMIC_RECONFIG_APER_PERM)) {
			continue;
		}

		if (XOcp_DmeXppuCfgTable[Index].IsModified == TRUE) {
			Status = Xil_SecureOut32(XOcp_DmeXppuCfgTable[Index].XppuAperAddr,
				XOcp_DmeXppuCfgTable[Index].XppuAperReadCfgVal);
			if (Status != XST_SUCCESS) {
				Status = XOCP_DME_ERR;
				goto END;
			}
		}
	}

	if (Index == XOCP_XPPU_MAX_APERTURES) {
		Status = XST_SUCCESS;
	}
END:
	return Status;
}
#endif /* PLM OCP */
