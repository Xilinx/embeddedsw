/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal_net/xloader_plat_secure.c
*
* This file contains the versal_net specific secure code related to PDI image
* loading.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- --------   -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       kpt  07/05/2022 Added support to update KAT status
*       dc   07/12/2022 Added Device state change support
*       kpt  07/05/2022 Added XLoader_RsaPssSignVeirfyKati
* 1.01  har  11/17/2022 Added XLoader_CheckSecureStateAuth
*       ng   11/23/2022 Fixed doxygen file name error
*       sk   03/10/2023 Added redundancy for AES Key selection
*       sk   03/17/2023 Renamed Kekstatus to DecKeySrc in xilpdi structure
*		dd	 03/28/2023 Updated doxygen comments
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
*       sk   06/12/2023 Renamed XLoader_UpdateKekSrc to XLoader_GetKekSrc
* 1.9   kpt  07/12/2023 Added mask generation function
*       dd   08/11/2023 Updated doxygen comments
	kpt  07/31/2023 Removed dead code in XLoader_CheckSecureStateAuth
* 2.1   ng   02/01/2024 u8 variables optimization
*       kpt  02/08/2024 Added support to update secure state when DAP state is changed
*       kpt  03/15/2024 Updated RSA KAT to use 2048-bit key
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#include "xplmi_status.h"
#include "xplmi_scheduler.h"
#include "xloader_plat_secure.h"
#include "xloader_plat.h"
#include "xloader_secure.h"
#include "xplmi.h"
#include "xplmi_err.h"
#include "xilpdi_plat.h"
#include "xil_error_node.h"
#include "xplmi_status.h"
#ifndef PLM_SECURE_EXCLUDE
#include "xsecure_init.h"
#include "xsecure_error.h"

/************************** Constant Definitions *****************************/
#define XLOADER_EFUSE_OBFUS_KEY		(0xA5C3C5A7U) /**< eFuse obfuscated key */
#define XLOADER_BBRAM_OBFUS_KEY		(0x3A5C3C57U) /**< BBRAM obfuscated key */
#define XLOADER_BH_OBFUS_KEY		(0xA35C7CA5U) /**< Boot header obfuscated key */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifndef PLM_RSA_EXCLUDE
static int XLoader_RsaPssSignVeirfyKat(XPmcDma *PmcDmaPtr);
#endif

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function updates KEK red key availability status from
 * boot header.
 *
 * @return
 *			 - Decryption Key source status.
 *
 ******************************************************************************/
u32 XLoader_GetKekSrc(void)
{
	volatile u32 DecKeySrc = 0x0U;
	const XilPdi_BootHdr *BootHdrPtr = (XilPdi_BootHdr *)(UINTPTR)XIH_BH_PRAM_ADDR;

	XPlmi_Printf(DEBUG_INFO, "Identifying KEK's corresponding RED "
			"key availability status\n\r");
	switch(BootHdrPtr->EncStatus) {
		case XLOADER_BH_BLK_KEY:
		case XLOADER_BH_OBFUS_KEY:
			DecKeySrc = XLOADER_BHDR_RED_KEY;
			break;
		case XLOADER_BBRAM_BLK_KEY:
		case XLOADER_BBRAM_OBFUS_KEY:
			DecKeySrc = XLOADER_BBRAM_RED_KEY;
			break;
		case XLOADER_EFUSE_BLK_KEY:
		case XLOADER_EFUSE_OBFUS_KEY:
			DecKeySrc = XLOADER_EFUSE_RED_KEY;
			break;
		default:
			/* No KEK is available for PLM */
			break;
	}
	XPlmi_Printf(DEBUG_DETAILED, "KEK red key available after "
			"for PLM %x\n\r", DecKeySrc);
	return DecKeySrc;
}

/*****************************************************************************/
/**
 * @brief	This function provides Obfuscated Aes Key source
 *
 * @param	PdiKeySrc is the Key source given in Pdi
 * @param	DecKeyMask is the current DecKeyMask
 * @param	KeySrcPtr is the pointer to the calculated KeySrc
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XLoader_AesObfusKeySelect(u32 PdiKeySrc, u32 DecKeyMask, void *KeySrcPtr)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesKeySrc *KeySrc = (XSecure_AesKeySrc *)KeySrcPtr;
	volatile u32 PdiKeySrcTmp;

	*KeySrc = XSECURE_AES_INVALID_KEY;

	switch (PdiKeySrc) {
		case XLOADER_EFUSE_OBFUS_KEY:
			PdiKeySrcTmp = XLOADER_EFUSE_OBFUS_KEY;
			if (((DecKeyMask) & XLOADER_EFUSE_RED_KEY) == XLOADER_EFUSE_RED_KEY) {
				Status = XST_SUCCESS;
				*KeySrc = XSECURE_AES_EFUSE_RED_KEY;
			}
			break;
		case XLOADER_BBRAM_OBFUS_KEY:
			PdiKeySrcTmp = XLOADER_BBRAM_OBFUS_KEY;
			if (((DecKeyMask) & XLOADER_BBRAM_RED_KEY) == XLOADER_BBRAM_RED_KEY) {
				*KeySrc = XSECURE_AES_BBRAM_RED_KEY;
				Status = XST_SUCCESS;
			}
			break;
		case XLOADER_BH_OBFUS_KEY:
			PdiKeySrcTmp = XLOADER_BH_OBFUS_KEY;
			if (((DecKeyMask) & XLOADER_BHDR_RED_KEY) == XLOADER_BHDR_RED_KEY) {
				*KeySrc = XSECURE_AES_BH_RED_KEY;
				Status = XST_SUCCESS;
			}
			break;
		default:
			Status  = XLoader_UpdateMinorErr(XLOADER_SEC_AES_KEK_DEC, Status);
			break;
	}

	if (Status == XST_SUCCESS) {
		if ((PdiKeySrc != PdiKeySrcTmp) || (*KeySrc == XSECURE_AES_INVALID_KEY)) {
			Status  = XLoader_UpdateMinorErr(XLOADER_SEC_GLITCH_DETECTED_ERROR, 0x0U);
		}
	}
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function adds periodic checks of the device status
 * change during secure boot.
 *
 * @return	XST_SUCCESS on success.
 * @return	XLOADER_ERR_ADD_TASK_SCHEDULER if failed to add task to scheduler.
 *
 ******************************************************************************/
int XLoader_AddDeviceStateChangeToScheduler(void)
{
	volatile int Status = XST_FAILURE;
	volatile u32 JtagDis = 0x0U;
	volatile u32 JtagDisTmp = 0x0U;
	u32 SecureStateAHWRoT = XLoader_GetAHWRoT(NULL);
	u32 SecureStateSHWRoT = XLoader_GetSHWRoT(NULL);

	JtagDis = XPlmi_In32(XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
			XLOADER_EFUSE_CACHE_JTAG_DIS_MASK;
	JtagDisTmp = XPlmi_In32(XLOADER_EFUSE_CACHE_SECURITY_CONTROL_OFFSET) &
			XLOADER_EFUSE_CACHE_JTAG_DIS_MASK;

	if ((JtagDis != XLOADER_EFUSE_CACHE_JTAG_DIS_MASK) &&
		(JtagDisTmp != XLOADER_EFUSE_CACHE_JTAG_DIS_MASK)) {
		/*
		 * Checking HWROT enabled/disabled, for secure boot
		 * DAP device state change will be identified.
		 */
		if ((SecureStateAHWRoT == XPLMI_RTCFG_SECURESTATE_AHWROT) ||
			(SecureStateSHWRoT == XPLMI_RTCFG_SECURESTATE_SHWROT)) {
			Status = XST_FAILURE;
			Status = XPlmi_SchedulerAddTask(XPLMI_MODULE_LOADER_ID,
					XLoader_CheckDeviceStateChange, NULL,
					XLOADER_DEVICE_STATE_POLL_INTERVAL,
					XPLM_TASK_PRIORITY_0, NULL, XPLMI_PERIODIC_TASK);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_ADD_TASK_SCHEDULER, 0);
			}
			else {
				XPlmi_Printf(DEBUG_INFO,
					"Device state change task added successfully\r\n");
			}
		}
		else {
			/*
			 * In non secure boot DAP is always open state
			*/
			Status = XST_SUCCESS;
		}
	}
	else {
		/*
		 * The task should not be added to the scheduler if JTAG
		 * disable efuse bit is set.
		 * Thus forcing the Status to be XST_SUCCESS.
		 */
		Status = XST_SUCCESS;
	}

	return Status;
}

#ifndef PLM_RSA_EXCLUDE
/*****************************************************************************/
/**
* @brief    This function runs the KAT for RSA
*
* @param    PmcDmaPtr - Pointer to DMA instance
*
* @return   XST_SUCCESS on success and error code on failure
*
******************************************************************************/
int XLoader_RsaKat(XPmcDma *PmcDmaPtr) {
	int Status = XST_FAILURE;

	Status = XLoader_RsaPssSignVeirfyKat(PmcDmaPtr);

	return Status;
}
#endif

/******************************************************************************/
/**
* @brief        This function checks the JTAG device state change.
* 		When Secure gate is opened from close state this API raises an error,
*		if the gate is closed and re-opened again this function recognizes the
*		state change and performs the action configured.
*
* @param	Arg is of pointer of void type.
*
* @return   XST_SUCCESS on success and error code on failure
*
******************************************************************************/
int XLoader_CheckDeviceStateChange(void *Arg)
{
	int Status = XST_FAILURE;
	volatile u32 JtagStatus = XPlmi_In32(XLOADER_PMC_TAP_JTAG_STATUS_0) &
					XLOADER_PMC_TAP_JTAG_STATUS_DAP_STATUS_MASK;
	static u8 JtagStateChange = XLOADER_JTAG_SEC_GATE_CLOSE;
	static u8 PrevJtagStateChange = XLOADER_JTAG_SEC_GATE_CLOSE;
	(void)Arg;

	if (JtagStatus == XLOADER_PMC_TAP_JTAG_STATUS_DAP_STATUS_MASK) {
		if (JtagStateChange == XLOADER_JTAG_SEC_GATE_CLOSE) {
			JtagStateChange = XLOADER_JTAG_SEC_GATE_OPEN;
			XPlmi_HandleSwError(XIL_NODETYPE_EVENT_ERROR_SW_ERR,
				XIL_EVENT_ERROR_MASK_DEV_STATE_CHANGE);
		}
	}
	else {
		/*
		 * When DAP is closed resetting the variable to CLOSE state,
		 * to identify the next JTAG GATE OPEN state and report the error.
		 */
		JtagStateChange = XLOADER_JTAG_SEC_GATE_CLOSE;
	}

	/* Update secure state when DAP state gets changed */
	if (PrevJtagStateChange != JtagStateChange) {
		Status = XLoader_CheckAndUpdateSecureState();
		if (Status != XST_SUCCESS) {
			goto END;
		}
		PrevJtagStateChange = JtagStateChange;
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

#ifndef PLM_RSA_EXCLUDE
/*****************************************************************************/
/**
* @brief    This function runs the KAT for RSA PSS verification
*
* @param    PmcDmaPtr - Pointer to DMA instance
*
* @return   XST_SUCCESS on success and error code on failure
*
******************************************************************************/
static int XLoader_RsaPssSignVeirfyKat(XPmcDma *PmcDmaPtr) {
	XSecure_Rsa *RsaInstance = XSecure_GetRsaInstance();
	u32 *RsaModulus = XSecure_GetKatRsaModulus();
	u32 *RsaModExt = XSecure_GetKatRsaModExt();
	u32 RsaPubExp = XSECURE_KAT_RSA_PUB_EXP;
	u8 *MsgHash = XSecure_GetKatSha3ExpHash();
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	static u32 RsaPssSign[XSECURE_RSA_2048_SIZE_WORDS] = {
		0xD5B0D757U, 0xB402678AU, 0xD846E353U, 0xE615E8BDU,
		0x08000C3EU, 0x0D2BE166U, 0x8ADCA321U, 0xC0653B5BU,
		0xBC9CF06AU, 0x17232DA6U, 0x08124662U, 0xC58F1095U,
		0x9D3ABD1AU, 0x3EB117CBU, 0xE1B702AAU, 0x11267C5FU,
		0x74AEE03EU, 0x20AE3E4AU, 0x584CB9D0U, 0x201B42D7U,
		0xEE21C763U, 0x2E93CF7FU, 0x874C4496U, 0x94A3782EU,
		0x94B084C8U, 0x411698A1U, 0xE64E2B78U, 0x906CC80EU,
		0x8B76DD72U, 0x25F1E9C0U, 0xEDA7E18BU, 0x277C7905U,
		0xAD014078U, 0x0F4C0DB8U, 0x6A01665DU, 0x80BEE426U,
		0xF295C6A2U, 0xF3EFB25DU, 0x7255C0BAU, 0x41D3927BU,
		0x939251B3U, 0x49C59DD2U, 0x616F0890U, 0x230B31C0U,
		0x10A0EC9BU, 0x661EC873U, 0x132DD504U, 0xBDADB525U,
		0x4D7387D8U, 0x8C4E967AU, 0x681936A0U, 0xB4CE964FU,
		0xE632A6C2U, 0xD57C6294U, 0xAD3C1BD8U, 0xCE326E0DU,
		0x27CE58D3U, 0xE975377DU, 0x1F3007D2U, 0x27F80607U,
		0xF7E9821AU, 0x9A61ECCFU, 0xB1EA8EDCU, 0x989E39B0U
	};

	Status = XSecure_RsaInitialize(RsaInstance, (u8 *)RsaModulus,
		(u8 *)RsaModExt, (u8 *)&RsaPubExp);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_RsaPssSignVerify, PmcDmaPtr,
			(u8*)MsgHash, RsaInstance, (u8*)RsaPssSign, XSECURE_RSA_2048_KEY_SIZE);
	if (Status != XST_SUCCESS || StatusTmp != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_KAT_PSS_SIGN_VER_ERROR;
		XPlmi_Printf(DEBUG_GENERAL,"RSA Pss sign verification KAT failed with status:%02x",
			Status);
	}
END:
	SStatus = Xil_SMemSet(RsaInstance, sizeof(XSecure_Rsa), 0U, sizeof(XSecure_Rsa));
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}

	return Status;
}
#endif
/*****************************************************************************/
/**
 * @brief	This function converts a non-negative integer to an octet string of a
 * 			specified length.
 *
 * @param	Integer is the variable in which input should be provided.
 * @param	Size holds the required size.
 * @param	Convert is a pointer in which output will be updated.
 *
 * @return
 * 			- None
 *
 ******************************************************************************/
static inline void XLoader_I2Osp(u32 Integer, u32 Size, u8 *Convert)
{
	if (Integer < XLOADER_I2OSP_INT_LIMIT) {
		Convert[Size - 1U] = (u8)Integer;
	}
}

/*****************************************************************************/
/**
 * @brief	Mask generation function with SHA3.
 *
 * @param	Sha3InstancePtr is pointer to the XSecure_Sha3 instance.
 * @param	Out is pointer in which output of this function will be stored.
 * @param	OutLen specifies the required length.
 * @param	Input is pointer which holds the input data for	which mask should
 * 			be calculated which should be 48 bytes length.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- Errorcode on failure.
 *
 ******************************************************************************/
int XLoader_MaskGenFunc(XSecure_Sha *ShaInstancePtr,
	u8 * Out, u32 OutLen, u8 *Input)
{
	int Status = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	u32 Counter = 0U;
	u32 HashLen = XLOADER_SHA3_LEN;
	u8 HashStore[XLOADER_SHA3_LEN];
	u8 Convert[XIH_PRTN_WORD_LEN] = {0U};
	u32 Size = XLOADER_SHA3_LEN;
	u8 *OutTmp;

	if ((ShaInstancePtr == NULL) || (Out == NULL) ||
		(Input == NULL)) {
		goto END;
	}

	if (OutLen == 0U) {
		goto END;
	}

	OutTmp = Out;
	while (Counter <= (OutLen / HashLen)) {
		XLoader_I2Osp(Counter, XIH_PRTN_WORD_LEN, Convert);

		Status = XSecure_ShaStart(ShaInstancePtr, XSECURE_SHA3_384);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_ShaUpdate(ShaInstancePtr, (UINTPTR)Input, HashLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_ShaLastUpdate(ShaInstancePtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_ShaUpdate(ShaInstancePtr, (UINTPTR)Convert,
					XIH_PRTN_WORD_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_ShaFinish(ShaInstancePtr, (u64)(UINTPTR)&HashStore, sizeof(HashStore));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (Counter == (OutLen / HashLen)) {
			/*
			 * Only 463 bytes are required but the chunklen is 48 bytes.
			 * The extra bytes are discarded by the modulus operation below.
			 */
			 Size = (OutLen % HashLen);
		}
		Status = Xil_SMemCpy(OutTmp, Size, HashStore, Size, Size);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		OutTmp = &OutTmp[XLOADER_SHA3_LEN];
		Counter = Counter + 1U;
	}

END:
	ClearStatus = XPlmi_MemSetBytes(Convert, sizeof(Convert), 0U,
                        sizeof(Convert));
	ClearStatus |= XPlmi_MemSetBytes(&HashStore, XLOADER_SHA3_LEN, 0U,
                        XLOADER_SHA3_LEN);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}

	return Status;
}


/*****************************************************************************/
/**
* @brief	This function verifies whether the additional PPK is valid.
*
* @param	PpkHash is pointer to the PPK hash.
*
* @return	XST_FAILURE always return failue, as this api is not applicable for
* versal_net
*
******************************************************************************/
int XLoader_IsAdditionalPpkValid(const u8 *PpkHash) {
	(void)PpkHash;

	/* Not applicable for Versal Net */
	return XST_FAILURE;
}

/*****************************************************************************/
/**
* @brief	This function checks for the additional PPK select and returns the
* PPK invalid mask and PPK efuse cache start offset if PPK is valid.
*
* @param	PpkSelect	PPK selection of eFUSE.
* @param	InvalidMask Pointer to the PPK invalid mask
* @param	PpkOffset   Pointer to the efuse cache PPK start offset
*
* @return	XST_FAILURE always return failue, as this api is not applicable for
* versal_net
*
******************************************************************************/
int XLoader_AdditionalPpkSelect(XLoader_PpkSel PpkSelect, u32 *InvalidMask, u32 *PpkOffset)
{
	(void)PpkSelect;
	(void)InvalidMask;
	(void)PpkOffset;

	/* Not applicable for Versal Net */
	return XST_FAILURE;
}

/*****************************************************************************/
/**
* @brief	This function updates the configuration limiter count if
*		Configuration limiter feature is enabled in case of secure boot.
*		In case of eny error, secure lockdown is triggered.
*
* @param	UpdateFlag - Indicates id the counter should be incremented or decremeted
*
* @return	XST_SUCCESS on success.
*		Error code in case of failure
*
******************************************************************************/
int XLoader_UpdateCfgLimitCount(u32 UpdateFlag)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u32 SecureStateAHWRoT = XLoader_GetAHWRoT(NULL);
	u32 SecureStateSHWRoT = XLoader_GetSHWRoT(NULL);
	u32 ReadReg;
	int SHwRotStatus;
	int AHwRotStatus;
	u32 ReadCfgLimiterReg = XPlmi_In32(XLOADER_BBRAM_8_ADDRESS);
	u32 MaxConfigsCnt = ReadCfgLimiterReg & XLOADER_BBRAM_CL_COUNTER_MASK;
	u32 ClFeatureEn = ReadCfgLimiterReg & XLOADER_BBRAM_CL_FEATURE_EN_MASK;
	u32 CLMode;

	if ((UpdateFlag != XLOADER_BBRAM_CL_INCREMENT_COUNT) && (UpdateFlag != XLOADER_BBRAM_CL_DECREMENT_COUNT)) {
		goto END;
	}

	if (ClFeatureEn == XLOADER_BBRAM_CL_FEATURE_ENABLE) {
		ReadReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_SHWROT_ADDR);
		SHwRotStatus = XLoader_CheckSecureState(ReadReg, SecureStateSHWRoT,
			XPLMI_RTCFG_SECURESTATE_SHWROT);

		ReadReg = XPlmi_In32(XPLMI_RTCFG_SECURESTATE_AHWROT_ADDR);
		AHwRotStatus = XLoader_CheckSecureState(ReadReg, SecureStateAHWRoT,
			XPLMI_RTCFG_SECURESTATE_AHWROT);

		if ((AHwRotStatus == XST_SUCCESS) || (SHwRotStatus == XST_SUCCESS)) {
			if (UpdateFlag == XLOADER_BBRAM_CL_INCREMENT_COUNT){
				MaxConfigsCnt++;
			}
			else {
				if (MaxConfigsCnt == 0U) {
					Status = XPlmi_UpdateStatus((XPlmiStatus_t)XLOADER_ERR_CONFIG_LIMIT_EXCEEDED,
						Status);
					XPlmi_UtilRMW(PMC_GLOBAL_PMC_FW_ERR, PMC_GLOBAL_PMC_FW_ERR_DATA_MASK,
						(u32)Status);
					XPlmi_TriggerSLDOnHaltBoot(XPLMI_TRIGGER_TAMPER_TASK);
					goto END;
				}
				MaxConfigsCnt--;
			}
			XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_UPDATE_CONFIG_LIMITER_CNT_FAILED,
				Status, StatusTmp, Xil_SecureRMW32, XLOADER_BBRAM_8_ADDRESS,
				XLOADER_BBRAM_CL_COUNTER_MASK, MaxConfigsCnt);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				goto END;
			}
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;

}

#endif /* END OF PLM_SECURE_EXCLUDE */

/*****************************************************************************/
/**
* @brief	This function checks Secure State for Authentication
*
* @param	AHWRoT - Buffer to store Secure state for authentication
*
* @return	XST_SUCCESS on success.
* @return	XLOADER_ERR_GLITCH_DETECTED if glitch is detected.
*
******************************************************************************/
int XLoader_CheckSecureStateAuth(volatile u32* AHWRoT)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile u32 IsSignedImg = XIH_BH_IMG_ATTRB_SIGNED_IMG_VALUE;
	volatile u32 IsSignedImgTmp = XIH_BH_IMG_ATTRB_SIGNED_IMG_VALUE;

	IsSignedImg = ((XPlmi_In32(XIH_BH_PRAM_ADDR + XIH_BH_IMG_ATTRB_OFFSET) &
		XIH_BH_IMG_ATTRB_SIGNED_IMG_MASK) >> XIH_BH_IMG_ATTRB_SIGNED_IMG_SHIFT);
	IsSignedImgTmp = ((XPlmi_In32(XIH_BH_PRAM_ADDR + XIH_BH_IMG_ATTRB_OFFSET) &
		XIH_BH_IMG_ATTRB_SIGNED_IMG_MASK) >> XIH_BH_IMG_ATTRB_SIGNED_IMG_SHIFT);

	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_CheckNonZeroPpk);
	if ((Status == XST_SUCCESS) || (StatusTmp == XST_SUCCESS)) {
		/**
		 * If PPK hash is programmed in eFUSEs, then Secure State of boot is A-HWRoT
		 */
		*AHWRoT = XPLMI_RTCFG_SECURESTATE_AHWROT;
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "State of Boot(Authentication):"
			" Asymmetric HWRoT\r\n");
	}
	else {
		if ((IsSignedImg == XIH_BH_IMG_ATTRB_SIGNED_IMG_VALUE) ||
			(IsSignedImgTmp == XIH_BH_IMG_ATTRB_SIGNED_IMG_VALUE)) {
			/**
			* If PPK hash is not programmed in eFUSEs and PLM is authenticated then Secure State of boot is
			* emulated A-HWRoT
			*/
			*AHWRoT = XPLMI_RTCFG_SECURESTATE_EMUL_AHWROT;
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "State of Boot(Authentication):"
			" Emulated Asymmetric HWRoT\r\n");
		}
		else {
			*AHWRoT = XPLMI_RTCFG_SECURESTATE_NONSECURE;
		}
		Status = XST_SUCCESS;
	}

	return Status;
}
