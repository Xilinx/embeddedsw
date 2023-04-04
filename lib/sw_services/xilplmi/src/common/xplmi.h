/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
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
*       td   10/19/2020 MISRA C Fixes
* 1.04  nsk  12/14/2020 Modified xplmi_event_logging.c to use Canonical
*                       names.
* 1.05  ma   03/04/2021 Added IPI security defines
*       har  03/17/2021 Added Secure State register for authentication and
*                       encryption
*       ma   03/24/2021 Added RTCA Debug Log Address define
*       bm   03/24/2021 Added RTCA defines for Error Status registers
*       har  03/31/2021 Added RTCA defines for PDI ID
*       bm   05/05/2021 Added USR_ACCESS defines for PLD0 image
*       ma   05/21/2021 Added RTCA define for Secure Boot State
* 1.06  bsv  07/16/2021 Fix doxygen warnings
*       bsv  07/24/2021 Clear RTC area at the beginning of PLM
*       bsv  08/02/2021 Code clean up to reduce elf size
*       ma   08/06/2021 Added RTCA define for storing PMC_FW_ERR register value
*       bm   08/09/2021 Cleared PMC CDO buffer by default after processing
*       bm   08/12/2021 Added support to configure uart during run-time
*       ma   08/23/2021 Do not clear Debug Log RTCA memory
*       ma   08/30/2021 Added defines related to SSIT
*       gm   09/17/2021 Added RunTime Configuration register for MJTAG
*                       workaround
*       tnt  11/11/2021 Added RTCA defines for MIO Flush routine
*       tnt  12/17/2021 Added RTCA define for PL_POR HDIO workaround
* 1.07  ma   05/10/2022 Added PLM to PLM communication feature
*       bm   07/06/2022 Refactor versal and versal_net code
*       kpt  07/05/2022 Added XPLMI_RTCFG_SECURE_CTRL_ADDR
*       ma   07/08/2022 Added support for secure lockdown
*       kpt  07/19/2022 Added APIs and macros related to KAT
* 1.08  ng   12/08/2022 Updated SDK release version
*       sk   01/13/2023 Added defines for Image Store
* 1.09  skd  02/10/2023 SDK release version updated
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
#include "xplmi_plat.h"

/************************** Constant Definitions *****************************/
/* SDK release version */
#define SDK_RELEASE_YEAR	"2023" /**< Specifies the SDK release year */
#define SDK_RELEASE_QUARTER	"2"    /**< Specifies the SDK release quarter */

/*
 * Device Copy flag related macros
 */
#define XPLMI_DEVICE_COPY_STATE_MASK		(0x7U << 5U) /**< Device copy state
														   mask flag */
#define XPLMI_DEVICE_COPY_STATE_BLK			(0x0U << 5U) /**< Flag set to block
														   device copy */
#define XPLMI_DEVICE_COPY_STATE_INITIATE	(0x1U << 5U) /**< Flag set after
														   device copy initiates */
#define XPLMI_DEVICE_COPY_STATE_WAIT_DONE	(0x2U << 5U) /**< Flag set after
														   device copy done */

#define XPLMI_CHUNK_SIZE	(0x10000U) /**< PMCRAM chunk size */

#define XPLMI_CMD_SECURE		(0x0U) /**< IPI command secure flag */
#define XPLMI_CMD_NON_SECURE	(0x1U) /**< IPI command non-secure flag */
#define XPLMI_PMC_VERSION_MASK		(0xFU) /**< Used to calculate PMC Version */
#define XPLMI_PMC_VERSION_SHIFT		(0x4U) /**< Used to calculate PMC Version */

/**************************** Type Definitions *******************************/
#define UART_INITIALIZED	((u32)(1U << 0U)) /**< Flag indicates UART is initialized */
#define LPD_INITIALIZED		((u32)(1U << 1U)) /**< Flag indicates LPD is initialized */
#define LPD_WDT_INITIALIZED	((u32)(1U << 2U)) /**< Flag indicates LPD_WDT is initialized */
#define UART_PRINT_ENABLED	((u32)(1U << 3U)) /**< Flag indicates UART prints are enabled */
#define UART_PRINT_INITIALIZED	(UART_INITIALIZED | UART_PRINT_ENABLED)
					/**< Flag indicates UART is initialized
						and prints are enabled */
/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
 * @brief	This function checks if LPD is initialized or not
 *
 * @return
 *			- TRUE if Lpd initialized or
 *			- FALSE if Lpd not initialized
 *
 *****************************************************************************/
static inline u8 XPlmi_IsLpdInitialized(void)
{
	u32 *LpdInitializedPtr = XPlmi_GetLpdInitialized();

	return (((*LpdInitializedPtr & LPD_INITIALIZED) == LPD_INITIALIZED) ?
			(u8)TRUE : (u8)FALSE);
}
/*****************************************************************************/
/**
 * @brief	This function checks if UART is initialized or not
 *
 * @return
 *			- TRUE if UART initialized or
 *			- FALSE if UART not initialized
 *
 *****************************************************************************/
static inline u8 XPlmi_IsUartInitialized(void)
{
	u32 *LpdInitializedPtr = XPlmi_GetLpdInitialized();

	return (((*LpdInitializedPtr & UART_INITIALIZED) == UART_INITIALIZED) ?
			(u8)TRUE : (u8)FALSE);
}
/*****************************************************************************/
/**
 * @brief	This function checks if UART print is initialized or not
 *
 * @return
 *			- TRUE if UART Print initialized or
 *			- FALSE if UART Print not initialized
 *
 *****************************************************************************/
static inline u8 XPlmi_IsUartPrintInitialized(void)
{
	u32 *LpdInitializedPtr = XPlmi_GetLpdInitialized();

	return (((*LpdInitializedPtr & UART_PRINT_INITIALIZED) ==
			UART_PRINT_INITIALIZED) ?  (u8)TRUE : (u8)FALSE);
}

/**@cond xplmi_internal
 * @{
 */

/*
 * PLM RunTime Configuration Registers related defines
 */
/* Offsets of PLM Runtime Configuration Registers */
#define XPLMI_RTCFG_RTCA_ADDR			(XPLMI_RTCFG_BASEADDR + 0x0U)
#define XPLMI_RTCFG_VERSION_ADDR		(XPLMI_RTCFG_BASEADDR + 0x4U)
#define XPLMI_RTCFG_SIZE_ADDR			(XPLMI_RTCFG_BASEADDR + 0x8U)
#define XPLMI_RTCFG_DBG_LOG_BUF_ADDR	(XPLMI_RTCFG_BASEADDR + 0x10U)
#define XPLMI_RTCFG_SECURE_CTRL_ADDR		(XPLMI_RTCFG_BASEADDR + 0x28U)
#define XPLMI_RTCFG_IMGINFOTBL_ADDRLOW_ADDR	(XPLMI_RTCFG_BASEADDR + 0x40U)
#define XPLMI_RTCFG_IMGINFOTBL_ADDRHIGH_ADDR	(XPLMI_RTCFG_BASEADDR + 0x44U)
#define XPLMI_RTCFG_IMGINFOTBL_LEN_ADDR		(XPLMI_RTCFG_BASEADDR + 0x48U)
#define XPLMI_RTCFG_SECURESTATE_AHWROT_ADDR	(XPLMI_RTCFG_BASEADDR + 0x14CU)
#define XPLMI_RTCFG_SECURESTATE_SHWROT_ADDR	(XPLMI_RTCFG_BASEADDR + 0x150U)
#define XPLMI_RTCFG_PDI_ID_ADDR			(XPLMI_RTCFG_BASEADDR + 0x164U)
#define XPLMI_RTCFG_USR_ACCESS_ADDR		(XPLMI_RTCFG_BASEADDR + 0x168U)
#define XPLMI_RTCFG_SECURE_STATE_ADDR	(XPLMI_RTCFG_BASEADDR + 0x16CU)
#define XPLMI_RTCFG_PMC_FW_ERR_VAL_ADDR	(XPLMI_RTCFG_BASEADDR + 0x184U)
#define XPLMI_RTCFG_TAMPER_RESP			(XPLMI_RTCFG_BASEADDR + 0x18CU)

#define XPLMI_RTCFG_PLM_KAT_ADDR 		(XPLMI_RTCFG_SECURE_STATE_PLM_ADDR)

#define XPLMI_RTCFG_DBG_LOG_BUF_OFFSET	(0x10U)
#define XPLMI_RTCFG_LOG_UART_OFFSET		(0x24U)

/* Masks of PLM RunTime Configuration Registers */
#define XPLMI_RTCFG_IMGINFOTBL_NUM_ENTRIES_MASK	(0x0000FFFFU)
#define XPLMI_RTCFG_IMGINFOTBL_CHANGE_CTR_MASK	(0xFFFF0000U)
/* Mask for Tamper Response Secure lockdown 0 and 1 */
#define XPLMI_RTCFG_TAMPER_RESP_SLD_0_1_MASK	(0xCU)
/* Mask for Tamper response Secure lockdown 1 */
#define XPLMI_RTCFG_TAMPER_RESP_SLD_1_MASK		(0x8U)

/* Shifts of PLM RunTime Configuration Registers */
#define XPLMI_RTCFG_IMGINFOTBL_CHANGE_CTR_SHIFT	(0x10U)

/* Default Values of PLM RunTime Configuration Registers */
#define XPLMI_RTCFG_VER				(0x1U)
#define XPLMI_RTCFG_SIZE			(0x400U)
#define XPLMI_RTCFG_IMGINFOTBL_ADDR_HIGH	(0x0U)
#define XPLMI_RTCFG_IMGINFOTBL_LEN		(0x0U)
#define XPLMI_RTCFG_IDENTIFICATION		(0x41435452U)
#define XPLMI_RTCFG_SECURESTATE_AHWROT		(0xA5A5A5A5U)
#define XPLMI_RTCFG_SECURESTATE_SHWROT		(0x96969696U)
#define XPLMI_RTCFG_PDI_ID			(0x0U)

/* Values of Secure State Register */
#define XPLMI_RTCFG_SECURESTATE_EMUL_AHWROT	(0x5A5A5A5AU)
#define XPLMI_RTCFG_SECURESTATE_EMUL_SHWROT	(0x69696969U)
#define XPLMI_RTCFG_SECURESTATE_NONSECURE	(0xD2D2D2D2U)

/* Mask and shifts for KAT */
#define XPLMI_EFUSE_CACHE_CRYPTO_KAT_EN_MASK  (0X00008000U)
#define XPLMI_EFUSE_CACHE_CRYPTO_KAT_EN_SHIFT (15U)

/* Default value for Image Store */
#define XPLMI_RTCFG_IMG_STORE_ADDRESS_HIGH_INVALID   (0xFFFFFFFFU)
#define XPLMI_RTCFG_IMG_STORE_ADDRESS_LOW_INVALID    (0xFFFFFFFFU)
#define XPLMI_RTCFG_IMG_STORE_SIZE_INVALID 	     (0x0U)

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

/* Maximum length of PMC CDO */
#define XPLMI_PMC_CDO_MAX_LEN		(0x14000U)
#define XPLMI_PMC_CDO_MAX_WORD_LEN	(XPLMI_PMC_CDO_MAX_LEN / XPLMI_WORD_LEN)

/************************** Function Prototypes ******************************/
int XPlmi_Init(void);
void XPlmi_LpdInit(void);
void XPlmi_ResetLpdInitialized(void);
void XPlmi_PrintPlmBanner(void);
int XPlmi_RunTimeConfigInit(void);
void XPlmi_SetLpdInitialized(u32 Flag);
void XPlmi_UnSetLpdInitialized(u32 Flag);
void XPlmi_UpdateKatStatus(u32 PlmKatStatus);
u32 XPlmi_GetKatStatus(void);
void XPlmi_SetKatMask(u32 PlmKatMask);
void XPlmi_ClearKatMask(u32 PlmKatMask);
u8 XPlmi_IsCryptoKatEn(void);

/************************** Variable Definitions *****************************/

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_H */
