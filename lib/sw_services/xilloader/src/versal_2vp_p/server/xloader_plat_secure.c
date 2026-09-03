/***************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xloader_plat_secure.c
*
* This file contains the versal_2vp specific secure code related to PDI image
* loading.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 2.3   sd  10/13/25 Initial release
* 2.4   tvp 03/24/26 Add support for obfuscated key
* 2.5   dc  09/01/26 Enabled DAP device state change scheduler task and used
*                    DAP_SECURITY gate state for device-state detection
*
* </pre>
*
***************************************************************************************************/

/**
 * @addtogroup xloader_server_apis XilLoader Server APIs
 * @{
 */

/*************************************** Include Files ********************************************/
#include "xplmi_config.h"
#include "xplmi_status.h"
#include "xplmi_scheduler.h"
#include "xloader_plat_secure.h"
#include "xloader_plat.h"
#include "xloader_secure.h"
#include "xilpdi.h"
#include "xplmi.h"
#include "xplmi_err.h"
#include "xil_error_node.h"
#include "xsecure_init.h"

/************************************ Constant Definitions ****************************************/
#define XLOADER_EFUSE_OBFUS_KEY		(0xA5C3C5A7U)	/**< eFuse obfuscated key */
#define XLOADER_BBRAM_OBFUS_KEY		(0x3A5C3C57U)	/**< BBRAM obfuscated key */
#define XLOADER_BH_OBFUS_KEY		(0xA35C7CA5U)	/**< Boot header obfuscated key */
#define XLOADER_INVALID_OBFUS_KEY	(0xFFFFFFFFU)	/**< Invalid obfuscated key */

/************************************** Type Definitions ******************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Function Prototypes *****************************************/

#ifndef PLM_SECURE_EXCLUDE
#ifndef PLM_RSA_EXCLUDE
static inline void XLoader_I2Osp(u32 Integer, u32 Size, u8 *Convert);
#endif

/************************************ Variable Definitions ****************************************/

/**************************************************************************************************/
/**
 * @brief	This function updates KEK red key availability status from boot header.
 *
 * @return
 *		- Decryption Key source status.
 *
 **************************************************************************************************/
u32 XLoader_GetKekSrc(void)
{
	volatile u32 DecKeySrc = 0x0U;
	const XilPdi_BootHdr *BootHdrPtr = (XilPdi_BootHdr *)(UINTPTR)XIH_BH_PRAM_ADDR;

	XPlmi_Printf(DEBUG_INFO, "Identifying KEK's corresponding RED key availability status\n\r");
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
	XPlmi_Printf(DEBUG_DETAILED, "KEK red key available after for PLM %x\n\r", DecKeySrc);

	return DecKeySrc;
}

/**************************************************************************************************/
/**
 * @brief	This function provides Obfuscated Aes Key source.
 *
 * @param	PdiKeySrc is the Key source given in Pdi.
 * @param	DecKeyMask is the current DecKeyMask.
 * @param	KeySrcPtr is the pointer to the calculated KeySrc.
 *
 * @return
 *		- XST_SUCCESS on success and error code on failure.
 *
 **************************************************************************************************/
int XLoader_AesObfusKeySelect(u32 PdiKeySrc, u32 DecKeyMask, void *KeySrcPtr)
{
	volatile int Status = XST_FAILURE;
	XSecure_AesKeySrc *KeySrc = (XSecure_AesKeySrc *)KeySrcPtr;
	volatile u32 PdiKeySrcTmp = XLOADER_INVALID_OBFUS_KEY;

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

/**************************************************************************************************/
/**
 * @brief	This function adds periodic checks of the device status
 *		change during secure boot.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XLOADER_ERR_ADD_TASK_SCHEDULER if failed to add task to scheduler.
 *
 **************************************************************************************************/
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
				Status = XPlmi_UpdateStatus(XLOADER_ERR_ADD_TASK_SCHEDULER, XIL_SIGNED_ZERO);
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

/**************************************************************************************************/
/**
 * @brief	This function checks the DAP security-gate state change.
 *		When Secure gate is opened from close state this API raises an error,
 *		if the gate is closed and re-opened again this function recognizes the
 *		state change and performs the action configured.
 *		Versal_2VP_P has no JTAG_STATUS_0; open is DAP_SECURITY == 0xFFFFFFFF, the
 *		same value XLoader_EnableJtag writes (including PLM_AUTH_JTAG unlock).
 *		On HWRoT boot the first CLOSE-to-OPEN edge raises SW error 0x8E
 *		(XIL_EVENT_ERROR_MASK_DEV_STATE_CHANGE). Default EAM action is
 *		XPLMI_EM_ACTION_NONE; EM_SET_ACTION can escalate it.
 *
 * @param	Arg is of pointer of void type.
 *
 * @return
 *		- XST_SUCCESS on success and error code on failure
 *
 **************************************************************************************************/
int XLoader_CheckDeviceStateChange(void *Arg)
{
	volatile int Status = XST_FAILURE;
	volatile u32 DapSecurity = XPlmi_In32(XLOADER_PMC_TAP_DAP_SECURITY_OFFSET);
	volatile u32 DapSecurityTmp = XPlmi_In32(XLOADER_PMC_TAP_DAP_SECURITY_OFFSET);
	static u8 JtagStateChange = XLOADER_JTAG_SEC_GATE_CLOSE;
	static u8 PrevJtagStateChange = XLOADER_JTAG_SEC_GATE_CLOSE;
	(void)Arg;

	if ((DapSecurity == XLOADER_DAP_SECURITY_GATE_DISABLE_MASK) &&
		(DapSecurityTmp == XLOADER_DAP_SECURITY_GATE_DISABLE_MASK)) {
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
/**************************************************************************************************/
/**
 * @brief	This function runs the KAT for RSA
 *
 * @return
 * 		- XST_SUCCESS on success and error code on failure
 *
 **************************************************************************************************/
int XLoader_RsaKat(void) {
	int Status = XST_FAILURE;

	Status = XSecure_RsaPublicEncryptKat();

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function converts a non-negative integer to an octet string of a specified
 * 		length.
 *
 * @param	Integer is the variable in which input should be provided.
 * @param	Size holds the required size.
 * @param	Convert is a pointer in which output will be updated.
 *
 **************************************************************************************************/
static inline void XLoader_I2Osp(u32 Integer, u32 Size, u8 *Convert)
{
	if (Integer < XLOADER_I2OSP_INT_LIMIT) {
		Convert[Size - 1U] = (u8)Integer;
	}
}

/**************************************************************************************************/
/**
 * @brief	Mask generation function with SHA3.
 *
 * @param	Sha3InstancePtr is pointer to the XSecure_Sha3 instance.
 * @param	Out is pointer in which output of this function will be stored.
 * @param	OutLen specifies the required length.
 * @param	Input is pointer which holds the input data for	which mask should be calculated
 * 		which should be 48 bytes length.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XLOADER_SEC_BUF_CLEAR_ERR if failed to clear buffer.
 *
 **************************************************************************************************/
int XLoader_MaskGenFunc(XSecure_Sha3 *Sha3InstancePtr, u8 * Out, u32 OutLen, u8 *Input)
{
	int Status = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	u32 Counter = 0U;
	u32 HashLen = XSECURE_SHA_384_HASH_SIZE_IN_BYTES;
	XSecure_Sha384Hash HashStore;
	u8 Convert[XIH_PRTN_WORD_LEN] = {0U};
	u32 Size = XSECURE_SHA_384_HASH_SIZE_IN_BYTES;
	u8 *OutTmp;

	if ((Sha3InstancePtr == NULL) || (Out == NULL) || (Input == NULL)) {
		goto END;
	}

	if (OutLen == 0U) {
		goto END;
	}

	OutTmp = Out;
	while (Counter <= (OutLen / HashLen)) {
		XLoader_I2Osp(Counter, XIH_PRTN_WORD_LEN, Convert);

		Status = XSecure_ShaStart(Sha3InstancePtr, XSECURE_SHA3_384);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_ShaUpdate(Sha3InstancePtr, (UINTPTR)Input, HashLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_ShaLastUpdate(Sha3InstancePtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_ShaUpdate(Sha3InstancePtr, (UINTPTR)Convert,
					XIH_PRTN_WORD_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_ShaFinish(Sha3InstancePtr, (u64)(UINTPTR)&HashStore, sizeof(HashStore));
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
		Status = Xil_SMemCpy(OutTmp, Size, HashStore.Hash, Size, Size);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		OutTmp = &OutTmp[XSECURE_SHA_384_HASH_SIZE_IN_BYTES];
		Counter = Counter + 1U;
	}

END:
	ClearStatus = XPlmi_MemSetBytes(Convert, sizeof(Convert), 0U, sizeof(Convert));
	ClearStatus |= XPlmi_MemSetBytes(&HashStore, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, 0U, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}
#endif


#endif/* END OF PLM_SECURE_EXCLUDE */
/**************************************************************************************************/
/**
* @brief	This function checks Secure State for Authentication.
*
* @param	AHWRoT	Buffer to store Secure state for authentication.
*
* @return
* 		- XST_SUCCESS on success.
* 		- XLOADER_ERR_GLITCH_DETECTED if glitch is detected.
*
***************************************************************************************************/
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
		/** - If PPK hash is programmed in eFUSEs, then Secure State of boot is A-HWRoT. */
		*AHWRoT = XPLMI_RTCFG_SECURESTATE_AHWROT;
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "State of Boot(Authentication):"
			" Asymmetric HWRoT\r\n");
	} else {
		if ((IsSignedImg == XIH_BH_IMG_ATTRB_SIGNED_IMG_VALUE) ||
			(IsSignedImgTmp == XIH_BH_IMG_ATTRB_SIGNED_IMG_VALUE)) {
			/**
			 * - If PPK hash is not programmed in eFUSEs and PLM is authenticated then
			 *   Secure State of boot is emulated A-HWRoT.
			 */
			*AHWRoT = XPLMI_RTCFG_SECURESTATE_EMUL_AHWROT;
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "State of Boot(Authentication):"
			" Emulated Asymmetric HWRoT\r\n");
		} else {
			*AHWRoT = XPLMI_RTCFG_SECURESTATE_NONSECURE;
		}
		Status = XST_SUCCESS;
	}

	return Status;
}


/** @} end of xloader_server_apis group */
