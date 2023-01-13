/******************************************************************************
* Copyright (c) 2020-2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_main.h
*
* This is the main header file which contains definitions for the ImgSel
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  Ana  07/02/20 First release
*
* </pre>
*
******************************************************************************/

#ifndef XIS_MAIN_H
#define XIS_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplatform_info.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xis_config.h"
#include "xis_debug.h"
#include "xis_i2c.h"
#include "xis_error.h"
#if defined(XIS_UPDATE_A_B_MECHANISM)
#include "xis_qspi.h"
#if defined(XPAR_XGPIOPS_NUM_INSTANCES)
#include "xis_gpio.h"
#endif
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#ifdef XIS_UPDATE_A_B_MECHANISM
#define XIS_PERS_REGISTER_BASE_ADDRESS				(0x100000U)
#define XIS_PERS_REGISTER_BACKUP_ADDRESS			(0x120000U)
#define XIS_SIZE_4K									(4096U)
#define XIS_IMAGE_A									(0U)
#define XIS_IMAGE_B									(1U)
#define XIS_IDENTIFICATION_STRING					(0x4D554241)
#define XIS_IDENTIFICATION_STRING_OFFSET			(0x0U)
#define XIS_VERSION_OFFSET							(0x4U)
#define XIS_LENGTH_OF_REGISTERS						(0x4U)
#define XIS_LENGTH_OFFSET							(0x8U)
#define XIS_CHECKSUM_OFFSET							(0xCU)
#define XIS_LAST_BOOTED_IMAGE 						(0x10U)
#define	XIS_REQUESTED_BOOT_IMAGE					(0x11U)
#define XIS_IMAGE_B_BOOTABLE 						(0x12U)
#define XIS_IMAGE_A_BOOTABLE 						(0x13U)
#define XIS_IMAGE_A_OFFSET 							(0x14U)
#define XIS_IMAGE_B_OFFSET 							(0x18U)
#define XIS_RECOVERY_IMAGE_OFFSET 					(0x1CU)
#define XIS_RECOVERY_ADDRESS						(0x01E00000U)
#define XIS_SIZE_32KB								(32768U)
#endif

#define XIS_MAX_SIZE								(64U)
#define XIS_CRL_APB_RESET_CTRL 						(0xFF5E0218U)
#define XIS_CSU_APB_RESET_VAL  						(0x10U)
#define XIS_CSU_MULTI_BOOT							(0xFFCA0010U)
#define COUNTS_PER_USECOND  					(COUNTS_PER_SECOND / 1000000)
#define XIS_ERROR_STATUS_REGISTER_OFFSET			(0xFFD80060U)

#define XIs_In32(Addr)                	Xil_In32(Addr)
#define XIs_Out32(Addr, Data)			Xil_Out32(Addr, Data)

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
void XIs_Softreset(void);
void XIs_UpdateError(int Error);
void XIs_UpdateMultiBootValue(u32 Offset);
int XIs_UartConfiguration(void);
#ifndef XIS_UPDATE_A_B_MECHANISM
int XIs_ImageSelBoardParam(void);
#else
int XIs_UpdateABMultiBootValue(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
