/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
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
*	kpt  07/31/2023 Removed dead code in XLoader_CheckSecureStateAuth
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
* @return       XST_SUCCESS is returned.
*
******************************************************************************/
int XLoader_CheckDeviceStateChange(void *Arg)
{
	volatile u32 JtagStatus = XPlmi_In32(XLOADER_PMC_TAP_JTAG_STATUS_0) &
					XLOADER_PMC_TAP_JTAG_STATUS_DAP_STATUS_MASK;
	static u8 JtagStateChange = XLOADER_JTAG_SEC_GATE_CLOSE;
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

	return XST_SUCCESS;
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
	static u32 RsaPssSign[XSECURE_RSA_4096_SIZE_WORDS] = {
		0x1C1F3652, 0xAAB712F5, 0xF5D5A1B6, 0xD6D39EC2,
		0xACF624DE, 0x8CB3D909, 0x51A3DA87, 0x003DCCF7,
		0xD445CC9F, 0x672497D0, 0x84616AC0, 0xADFE38AC,
		0x98E98F65, 0x10659C55, 0x1778B86A, 0x492D8CE0,
		0x68A100D0, 0x5144E2D8, 0x74F1674E, 0xD62027C5,
		0x19E993D6, 0x225BD393, 0x6C42221F, 0xC5F56CC0,
		0x4B4305F8, 0x804663CD, 0x4F0B9892, 0xA1F35E6E,
		0xED6E220D, 0x3AF33B06, 0x546ACD6D, 0xA6539CD4,
		0xE9E0FD57, 0xC4DF082F, 0x36736B21, 0xD46CAD29,
		0x4C5A6879, 0x5573A133, 0xB4B5EEA5, 0xD1AC0B26,
		0x95F4E35E, 0xD5DCAE46, 0xD7D71D56, 0x8A39FE66,
		0x6880EF70, 0x898CD627, 0x9628E054, 0xBC92A962,
		0xA9DF5743, 0xA73D7852, 0x183E5DBF, 0xB112B303,
		0xCF3354B8, 0x0B4444E9, 0x5220F35B, 0x66D1DBFF,
		0x81392B6B, 0xCD3BE0C0, 0xFC1F50D7, 0x07E31875,
		0xD56AB661, 0xE9F8A43A, 0x43244057, 0x31653AEE,
		0x749E7B6D, 0xE48515EB, 0x7C8D03D6, 0xE058C64B,
		0x7D4F39F5, 0xAA066515, 0x1F07C07E, 0xB730B64B,
		0x1DEEE24F, 0xDA246FEB, 0xA440703E, 0xF92F3A42,
		0xF5388030, 0x652C417C, 0x7965EA9C, 0x857996D7,
		0x3F9A10CA, 0xB387EEFA, 0x7772E23E, 0x970AE759,
		0x7F8E6B29, 0xFAE06126, 0xDB9289FD, 0x90FECB65,
		0x68D11AE1, 0x2DAA0840, 0x99AA8B86, 0x55E7F81D,
		0x43AB26C7, 0xA46A5B8F, 0xF638E03D, 0x0A6D701F,
		0xE0D21011, 0x724CEE13, 0xE0955002, 0xA2CAE99B,
		0x9EF5873C, 0x4257DB7A, 0x9B461544, 0xC327DAAA,
		0x2B5579D5, 0x24D67939, 0xB7F033DD, 0x517EB9C2,
		0x4FFBEA9F, 0x308B04C2, 0x40284170, 0x0E420824,
		0x31A6F605, 0x721C6608, 0x0623E604, 0xE1D94C3C,
		0xE29002F9, 0xE4347CD0, 0xAC1287B8, 0x40718B65,
		0xC4185EE6, 0xB1C6094C, 0xE3210914, 0xF05A2CD6,
		0x25179E50, 0xD7C3054F, 0x24F2F7F9, 0x2FE6F393
	};

	Status = XSecure_RsaInitialize(RsaInstance, (u8 *)RsaModulus,
		(u8 *)RsaModExt, (u8 *)&RsaPubExp);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_RsaPssSignVerify, PmcDmaPtr,
			(u8*)MsgHash, RsaInstance, (u8*)RsaPssSign);
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
	MgfInput.SeedLen = XLOADER_SHA3_LEN;
	MgfInput.Output = Out;
	MgfInput.OutputLen = OutLen;
	Status = XSecure_MaskGenFunc(XSECURE_SHA3_384, Sha3InstancePtr, &MgfInput);

	return Status;
}
#endif

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
	volatile u8 IsSignedImg = XIH_BH_IMG_ATTRB_SIGNED_IMG_VALUE;
	volatile u8 IsSignedImgTmp = XIH_BH_IMG_ATTRB_SIGNED_IMG_VALUE;

	IsSignedImg = (u8)((XPlmi_In32(XIH_BH_PRAM_ADDR + XIH_BH_IMG_ATTRB_OFFSET) &
		XIH_BH_IMG_ATTRB_SIGNED_IMG_MASK) >> XIH_BH_IMG_ATTRB_SIGNED_IMG_SHIFT);
	IsSignedImgTmp = (u8)((XPlmi_In32(XIH_BH_PRAM_ADDR + XIH_BH_IMG_ATTRB_OFFSET) &
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
