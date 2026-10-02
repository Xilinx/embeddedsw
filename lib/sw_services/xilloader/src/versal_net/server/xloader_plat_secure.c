/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_plat_secure.c
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
* 2.4   abh  10/09/2025 Fixed MISRA-C violations
* 2.5   rama 09/21/2026 Removed XLoader_RsaKat and XLoader_RsaPssSignVeirfyKat,
*                       consolidated into xloader_kat.c
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xloader_server_apis XilLoader Server APIs
 * @{
 */

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
#ifndef PLM_SECURE_EXCLUDE
#include "xsecure_init.h"
#include "xsecure_error.h"
#include "xsecure_mgf.h"

/************************** Constant Definitions *****************************/
#define XLOADER_EFUSE_OBFUS_KEY		(0xA5C3C5A7U) /**< eFuse obfuscated key */
#define XLOADER_BBRAM_OBFUS_KEY		(0x3A5C3C57U) /**< BBRAM obfuscated key */
#define XLOADER_BH_OBFUS_KEY		(0xA35C7CA5U) /**< Boot header obfuscated key */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

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
			Status  = XLoader_UpdateMinorErr(XLOADER_SEC_GLITCH_DETECTED_ERROR, (int)0);
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
int XLoader_MaskGenFunc(XSecure_Sha3 *Sha3InstancePtr,
	u8 * Out, u32 OutLen, u8 *Input)
{
	int Status = XST_FAILURE;
	XSecure_MgfInput MgfInput;

	MgfInput.Seed = Input;
	MgfInput.SeedLen = XSECURE_SHA_384_HASH_SIZE_IN_BYTES;
	MgfInput.Output = Out;
	MgfInput.OutputLen = OutLen;
	Status = XSecure_MaskGenFunc(XSECURE_SHA3_384, Sha3InstancePtr, &MgfInput);

	return Status;
}
#endif


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


/** @} end of xloader_server_apis group */
