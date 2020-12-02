/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrfdc_mts.c
* @addtogroup rfdc_v8_1
* @{
*
* Contains the multi tile sync related structures, Macros of the XRFdc driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 3.1   jm     01/24/18 Initial release
* 3.2   jm     03/12/18 Fixed DAC latency calculation.
*       jm     03/12/18 Added support for reloading DTC scans.
*       jm     03/12/18 Add option to configure sysref capture after MTS.
* 4.0   sk     04/09/18 Added API to enable/disable the sysref.
*       rk     04/17/18 Adjust calculated latency by sysref period, where doing
*                       so results in closer alignment to the target latency.
* 5.0   sk     08/03/18 Fixed MISRAC warnings.
*       sk     08/03/18 Check for Block0 enable for tiles participating in MTS.
*       sk     08/24/18 Reorganize the code to improve readability and
*                       optimization.
* 6.0   cog    02/17/19 Added XRFdc_GetMTSEnable API.
* 7.0   cog    05/13/19 Formatting changes.
* 7.1   cog    01/20/20 Changes for MTS Gen 1/2 compatibility mode.
* 8.0   cog    02/10/20 Updated addtogroup.
* 8.1   cog    06/24/20 Upversion.
*
* </pre>
*
******************************************************************************/
#ifndef RFDC_MTS_H_
#define RFDC_MTS_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xrfdc.h"

/************************** Constant Definitions *****************************/

#define XRFDC_MTS_RMW(read, mask, data) (((read) & ~(mask)) | ((data) & (mask)))
#define XRFDC_MTS_FIELD(data, mask, shift) (((data) & (mask)) >> (shift))

/**************************** Type Definitions *******************************/

typedef struct {
	u32 RefTile;
	u32 IsPLL;
	int Target[4];
	int Scan_Mode;
	int DTC_Code[4];
	int Num_Windows[4];
	int Max_Gap[4];
	int Min_Gap[4];
	int Max_Overlap[4];
} XRFdc_MTS_DTC_Settings;

typedef struct {
	u32 RefTile;
	u32 Tiles;
	int Target_Latency;
	int Offset[4];
	int Latency[4];
	int Marker_Delay;
	int SysRef_Enable;
	XRFdc_MTS_DTC_Settings DTC_Set_PLL;
	XRFdc_MTS_DTC_Settings DTC_Set_T1;
} XRFdc_MultiConverter_Sync_Config;

typedef struct {
	u32 Count[4];
	u32 Loc[4];
} XRFdc_MTS_Marker;

/***************** Macros (Inline Functions) Definitions *********************/

#define XRFDC_MTS_SYSREF_DISABLE 0U
#define XRFDC_MTS_SYSREF_ENABLE 1U

#define XRFDC_MTS_NUM_DTC 128U
#define XRFDC_MTS_REF_TARGET 64U
#define XRFDC_MTS_MAX_CODE 16U
#define XRFDC_MTS_MIN_GAP_T1 10U
#define XRFDC_MTS_MIN_GAP_PLL 5U
#define XRFDC_MTS_SR_TIMEOUT 4096U
#define XRFDC_MTS_DTC_COUNT 10U
#define XRFDC_MTS_MARKER_COUNT 4U
#define XRFDC_MTS_SCAN_INIT 0U
#define XRFDC_MTS_SCAN_RELOAD 1U
#define XRFDC_MTS_SRCOUNT_TIMEOUT 1000U
#define XRFDC_MTS_DELAY_MAX 31U
#define XRFDC_MTS_CHECK_ALL_FIFOS 0U

#define XRFDC_MTS_SRCAP_T1_EN 0x4000U
#define XRFDC_MTS_SRCAP_T1_RST 0x0800U
#define XRFDC_MTS_SRFLAG_T1 0x4U
#define XRFDC_MTS_SRFLAG_PLL 0x2U
#define XRFDC_MTS_FIFO_DEFAULT 0x0000U
#define XRFDC_MTS_FIFO_ENABLE 0x0003U
#define XRFDC_MTS_FIFO_DISABLE 0x0002U
#define XRFDC_MTS_AMARK_LOC_S 0x10U
#define XRFDC_MTS_AMARK_DONE_S 0x14U
#define XRFDC_MTS_DLY_ALIGNER0 0x28U
#define XRFDC_MTS_DLY_ALIGNER1 0x2CU
#define XRFDC_MTS_DLY_ALIGNER2 0x30U
#define XRFDC_MTS_DLY_ALIGNER3 0x34U
#define XRFDC_MTS_DIR_FIFO_PTR 0x40U
#define XRFDC_MTS_DAC_MARKER_LOC_MASK(X) ((X < XRFDC_GEN3) ? 0x7U : 0xFU)

/* Error Codes */
#define XRFDC_MTS_OK 0U
#define XRFDC_MTS_NOT_SUPPORTED 1U
#define XRFDC_MTS_TIMEOUT 2U
#define XRFDC_MTS_MARKER_RUN 4U
#define XRFDC_MTS_MARKER_MISM 8U
#define XRFDC_MTS_DELAY_OVER 16U
#define XRFDC_MTS_TARGET_LOW 32U
#define XRFDC_MTS_IP_NOT_READY 64U
#define XRFDC_MTS_DTC_INVALID 128U
#define XRFDC_MTS_NOT_ENABLED 512U
#define XRFDC_MTS_SYSREF_GATE_ERROR 2048U
#define XRFDC_MTS_SYSREF_FREQ_NDONE 4096U

/************************** Function Prototypes ******************************/

u32 XRFdc_MultiConverter_Sync(XRFdc *InstancePtr, u32 Type, XRFdc_MultiConverter_Sync_Config *ConfigPtr);
void XRFdc_MultiConverter_Init(XRFdc_MultiConverter_Sync_Config *ConfigPtr, int *PLL_CodesPtr, int *T1_CodesPtr);
u32 XRFdc_MTS_Sysref_Config(XRFdc *InstancePtr, XRFdc_MultiConverter_Sync_Config *DACSyncConfigPtr,
			    XRFdc_MultiConverter_Sync_Config *ADCSyncConfigPtr, u32 SysRefEnable);
u32 XRFdc_GetMTSEnable(XRFdc *InstancePtr, u32 Type, u32 Tile, u32 *EnablePtr);

#ifdef __cplusplus
}
#endif

#endif /* RFDC_MTS_H_ */
/** @} */
