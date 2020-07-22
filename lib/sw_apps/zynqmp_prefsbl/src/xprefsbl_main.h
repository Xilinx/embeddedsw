/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xprefsbl_main.h
*
* This is the main header file which contains definitions for the Pre-FSBL.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who             Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  Ana			  07/02/20    First release
*
* </pre>
*
******************************************************************************/

#ifndef XPREFSBL_MAIN_H
#define XPREFSBL_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplatform_info.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xprefsbl_config.h"
#include "xprefsbl_debug.h"
#include "xprefsbl_i2c.h"
#include "xprefsbl_error.h"
#ifdef XPREFSBL_UPDATE_A_B_MECHANISM
#include "xprefsbl_qspi.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#ifdef XPREFSBL_UPDATE_A_B_MECHANISM
#define XPREFSBL_PERS_REGISTER_BASE_ADDRESS				(0x100000U)
#define XPREFSBL_PERS_REGISTER_BACKUP_ADDRESS			(0x200000U)
#define XPREFSBL_SIZE_4K								(4096U)
#define XPREFSBL_IMAGE_A								(0U)
#define XPREFSBL_IMAGE_B								(1U)
#define XPREFSBL_IMAGE_A_BOOTABLE 						(0x0U)
#define XPREFSBL_IMAGE_B_BOOTABLE 						(0x1U)
#define	XPREFSBL_REQUESTED_BOOT_IMAGE					(0x2U)
#define XPREFSBL_LAST_BOOTED_IMAGE 						(0x3U)
#define XPREFSBL_IMAGE_A_OFFSET 						(4U)
#define XPREFSBL_IMAGE_B_OFFSET 						(8U)
#define XPREFSBL_RECOVERY_IMAGE_OFFSET 					(12U)
#define XPREFSBL_SIZE_32KB								(32768U)
#endif

#define XPREFSBL_MAX_SIZE								(64U)
#define XPREFSBL_CRL_APB_RESET_CTRL 					(0xFF5E0218U)
#define XPREFSBL_CSU_APB_RESET_VAL  					(0x10U)
#define XPREFSBL_CSU_MULTI_BOOT							(0xFFCA0010U)
#define COUNTS_PER_USECOND  			(COUNTS_PER_SECOND / 1000000)
#define XPREFSBL_ERROR_STATUS_REGISTER_OFFSET			(0xFFD80060U)

#define XPreFsbl_In32(Addr)                	Xil_In32(Addr)
#define XPreFsbl_Out32(Addr, Data)		Xil_Out32(Addr, Data)

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
int XPreFsbl_GetBoardName(void);
void XPrefsbl_Softreset(void);
void XPrefsbl_UpdateError(int Error);
void XPrefsbl_UpdateMultiBootValue(u32 Offset);
int XPrefsbl_UartConfiguration(void);
#ifndef XPREFSBL_UPDATE_A_B_MECHANISM
int XPrefsbl_UpdateMultiBootRegister(u8 *ReadBuffer);
#else
int XPrefsbl_UpdateABMultiBootValue(void);
#endif

#ifdef __cplusplus
}
#endif

#endif