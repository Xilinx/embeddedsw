/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#ifndef PLM_SECURE_EXCLUDE
#include "xloader_auth_enc.h"
#include "xplmi_status.h"
#include "xplmi_scheduler.h"
#include "xloader_plat_secure.h"
#include "xloader_secure.h"
#include "xplmi.h"
#include "xplmi_err.h"
#include "xil_error_node.h"

/************************** Constant Definitions *****************************/
#define XLOADER_EFUSE_OBFUS_KEY		(0xA5C3C5A7U) /* eFuse Obfuscated Key */
#define XLOADER_BBRAM_OBFUS_KEY		(0x3A5C3C57U) /* BBRAM Obfuscated Key */
#define XLOADER_BH_OBFUS_KEY		(0xA35C7CA5U) /*Boot Header Obfuscated Key */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XLoader_CheckDeviceStateChange(void *Arg);
/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function updates KEK red key availability status from
 * boot header.
 *
 * @param	PdiPtr is pointer to the XilPdi instance.
 *
 * @return	None.
 *
 ******************************************************************************/
void XLoader_UpdateKekSrc(XilPdi *PdiPtr)
{
	PdiPtr->KekStatus = 0x0U;

	XPlmi_Printf(DEBUG_INFO, "Identifying KEK's corresponding RED "
			"key availability status\n\r");
	switch(PdiPtr->MetaHdr.BootHdrPtr->EncStatus) {
		case XLOADER_BH_BLK_KEY:
		case XLOADER_BH_OBFUS_KEY:
			PdiPtr->KekStatus = XLOADER_BHDR_RED_KEY;
			break;
		case XLOADER_BBRAM_BLK_KEY:
		case XLOADER_BBRAM_OBFUS_KEY:
			PdiPtr->KekStatus = XLOADER_BBRAM_RED_KEY;
			break;
		case XLOADER_EFUSE_BLK_KEY:
		case XLOADER_EFUSE_OBFUS_KEY:
			PdiPtr->KekStatus = XLOADER_EFUSE_RED_KEY;
			break;
		default:
			/* No KEK is available for PLM */
			break;
	}
	XPlmi_Printf(DEBUG_DETAILED, "KEK red key available after "
			"for PLM %x\n\r", PdiPtr->KekStatus);
}

/*****************************************************************************/
/**
 * @brief	This function provides Obfuscated Aes Key source
 *
 * @param	PdiKeySrc is the Key source given in Pdi
 * @param	KekStatus is the current KekStatus
 * @param	KeySrcPtr is the pointer to the calculated KeySrc
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XLoader_AesObfusKeySelect(u32 PdiKeySrc, u32 KekStatus, void *KeySrcPtr)
{
	int Status = XST_FAILURE;
	XSecure_AesKeySrc *KeySrc = (XSecure_AesKeySrc *)KeySrcPtr;

	switch (PdiKeySrc) {
		case XLOADER_EFUSE_OBFUS_KEY:
			if (((KekStatus) & XLOADER_EFUSE_RED_KEY) == XLOADER_EFUSE_RED_KEY) {
				Status = XST_SUCCESS;
				*KeySrc = XSECURE_AES_EFUSE_RED_KEY;
			}
			break;
		case XLOADER_BBRAM_OBFUS_KEY:
			if (((KekStatus) & XLOADER_BBRAM_RED_KEY) == XLOADER_BBRAM_RED_KEY) {
				*KeySrc = XSECURE_AES_BBRAM_RED_KEY;
				Status = XST_SUCCESS;
			}
			break;
		case XLOADER_BH_OBFUS_KEY:
			if (((KekStatus) & XLOADER_BHDR_RED_KEY) == XLOADER_BHDR_RED_KEY) {
				*KeySrc = XSECURE_AES_BH_RED_KEY;
				Status = XST_SUCCESS;
			}
			break;
		default:
			Status  = XLoader_UpdateMinorErr(XLOADER_SEC_AES_KEK_DEC, Status);
			break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function return FIPS mode status
 *
 * @return
 * 		TRUE  if FIPS mode is enabled
 * 		FALSE if FIPS mode is not enabled
 *
 * @note
 *     Fips_Mode[24:23] is used to control scan clear execution as a part of
 *     secure lockdown
 *
 *****************************************************************************/
u8 XLoader_IsFipsModeEn(void) {
	u8 FipsModeEn = (u8)(XPlmi_In32(XLOADER_EFUSE_CACHE_FIPS) >>
					XLOADER_EFUSE_FIPS_MODE_SHIFT);

	return (FipsModeEn != 0U) ? (u8)TRUE: (u8)FALSE;
}

/*****************************************************************************/
/**
 * @brief	This function updates the KAT status
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance
 * @param	PlmKatMask is the mask of the KAT that is going to run
 *
 * @return	None
 *
 *****************************************************************************/
void XLoader_UpdateKatStatus(XLoader_SecureParams *SecurePtr, u32 PlmKatMask) {
	u8 FipsModeEn = XLoader_IsFipsModeEn();

	if (FipsModeEn == TRUE) {
		if (PlmKatMask != 0U) {
			/* Rerun KAT for every image */
			XLoader_ClearKatStatus(&SecurePtr->PdiPtr->PlmKatStatus, PlmKatMask);
		}
		else {
			SecurePtr->PdiPtr->PlmKatStatus = 0U;
		}
	}
	else {
		if (SecurePtr->PdiPtr->PdiType == XLOADER_PDI_TYPE_PARTIAL) {
			XLoader_UpdatePpdiKatStatus(SecurePtr, PlmKatMask);
		}
	}
}

/******************************************************************************/
/**
* @brief        This function adds periodic checks of the device status
*               change during secure boot.
*
* @return       XST_SUCCESS otherwise error code is returned
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
			(SecureStateSHWRoT = XPLMI_RTCFG_SECURESTATE_SHWROT)) {
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
static int XLoader_CheckDeviceStateChange(void *Arg)
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

#endif /* END OF PLM_SECURE_EXCLUDE */
