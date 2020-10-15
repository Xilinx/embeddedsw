/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi.h
*
* This file contains declarations PLMI module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/07/2019 Initial release
* 1.01  ma   08/01/2019 Added LPD init code
* 1.02  kc   02/19/2020 Moved code to support PLM banner from PLM app
*       bsv  04/04/2020 Code clean up
* 1.03  bsv  07/07/2020 Made functions used in single transaltion unit as
*						static
*       kc   07/28/2020 Added WDT MACRO to indicate WDT initialized
*       skd  07/29/2020 Added device copy macros
*       bm   09/08/2020 Added RunTime Configuration related registers
*       bsv  09/30/2020 Added XPLMI_CHUNK_SIZE macro
*       bm   10/14/2020 Code clean up
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_H
#define XPLMI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xplmi_generic.h"

/************************** Constant Definitions *****************************/
/* SDK release version */
#define SDK_RELEASE_YEAR	"2020"
#define SDK_RELEASE_QUARTER	"2"

/*
 * Device Copy flag related macros
 */
#define XPLMI_DEVICE_COPY_STATE_MASK		(0x7U << 5U)
#define XPLMI_DEVICE_COPY_STATE_BLK			(0x0U << 5U)
#define XPLMI_DEVICE_COPY_STATE_INITIATE	(0x1U << 5U)
#define XPLMI_DEVICE_COPY_STATE_WAIT_DONE	(0x2U << 5U)
/*
 * PMCRAM CHUNK SIZE
 */
#define XPLMI_CHUNK_SIZE	(0x10000U)

/**************************** Type Definitions *******************************/
#define UART_INITIALIZED	((u8)(1U << 0U))
#define LPD_INITIALIZED		((u8)(1U << 1U))
#define LPD_WDT_INITIALIZED	((u8)(1U << 2U))

/***************** Macros (Inline Functions) Definitions *********************/

/*
 * PLM RunTime Configuration Registers related defines
 */
/* PLM RunTime Configuration Area Base Address */
#define XPLMI_RTCFG_BASEADDR			(0xF2014000U)

/* Offsets of PLM Runtime Configuration Registers */
#define XPLMI_RTCFG_RTCA_ADDR			(XPLMI_RTCFG_BASEADDR + 0x0U)
#define XPLMI_RTCFG_VERSION_ADDR		(XPLMI_RTCFG_BASEADDR + 0x4U)
#define XPLMI_RTCFG_SIZE_ADDR			(XPLMI_RTCFG_BASEADDR + 0x8U)
#define XPLMI_RTCFG_IMGINFOTBL_ADDRLOW_ADDR	(XPLMI_RTCFG_BASEADDR + 0x40U)
#define XPLMI_RTCFG_IMGINFOTBL_ADDRHIGH_ADDR	(XPLMI_RTCFG_BASEADDR + 0x44U)
#define XPLMI_RTCFG_IMGINFOTBL_LEN_ADDR		(XPLMI_RTCFG_BASEADDR + 0x48U)

/* Masks of PLM RunTime Configuration Registers */
#define XPLMI_RTCFG_IMGINFOTBL_NUM_ENTRIES_MASK	(0x0000FFFFU)
#define XPLMI_RTCFG_IMGINFOTBL_CHANGE_CTR_MASK	(0xFFFF0000U)

/* Shifts of PLM RunTime Configuration Registers */
#define XPLMI_RTCFG_IMGINFOTBL_CHANGE_CTR_SHIFT	(0x10U)
/* Default Values of PLM RunTime Configuration Registers */
#define XPLMI_RTCFG_VER				(0x1U)
#define XPLMI_RTCFG_SIZE			(0x400U)
#define XPLMI_RTCFG_IMGINFOTBL_ADDR_HIGH	(0x0U)
#define XPLMI_RTCFG_IMGINFOTBL_LEN		(0x0U)
#define XPLMI_RTCFG_IDENTIFICATION		(0x41435452U)

/*
 * Using FW_IS_PRESENT to indicate Boot PDI loading is completed
 */
#define XPlmi_SetBootPdiDone()	XPlmi_UtilRMW(PMC_GLOBAL_GLOBAL_CNTRL, \
					PMC_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK, \
					PMC_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK)

#define XPlmi_IsLoadBootPdiDone() (((XPlmi_In32(PMC_GLOBAL_GLOBAL_CNTRL) & \
				PMC_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK) == \
				PMC_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK) ? \
					(TRUE) : (FALSE))

/************************** Function Prototypes ******************************/
int XPlmi_Init(void);
void XPlmi_LpdInit(void);
void XPlmi_ResetLpdInitialized(void);
void XPlmi_RunTimeConfigInit(void);
void XPlm_PrintPlmBanner(void);

/************************** Variable Definitions *****************************/
extern u8 LpdInitialized;

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_H */
