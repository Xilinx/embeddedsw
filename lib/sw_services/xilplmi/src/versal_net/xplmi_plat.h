/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file versal_net/xplmi_plat.h
*
* This file contains declarations for versal_net specific APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       ma   07/08/2022 Add ScatterWrite and ScatterWrite2 commands to versal
*       dc   07/12/2022 Added API XPlmi_RomISR() API
*       kpt  07/21/2022 Added APIs and macros for KAT
*       bm   07/22/2022 Update EAM logic for In-Place PLM Update
*       bm   07/22/2022 Retain critical data structures after In-Place PLM Update
*       bm   07/22/2022 Shutdown modules gracefully during update
*       ma   07/27/2022 Added XPlmi_SsitEventsInit function
*       bm   09/14/2022 Move ScatterWrite commands from common to versal_net
* 1.01  bm   11/07/2022 Clear SSS Cfg Error in SSSCfgSbiDma for Versal Net
*       ng   11/11/2022 Fixed doxygen file name error
*       kpt  01/04/2023 Added APIs and macros related to FIPS
*       bm   01/04/2023 Notify Other SLRs about Secure Lockdown
*       sk   01/11/2023 Added Config Space for Image Store in RTCA
*       bm   01/18/2023 Fix CFI readback logic with correct keyhole size
*       bm   03/11/2023 Modify XPlmi_PreInit prototype
*       dd   03/28/2023 Updated doxygen comments
* 1.04  bm   04/28/2023 Add XPlmi_GetRomIroFreq prototype
*       dd   05/24/2023 Updated doxygen comments
*       bm   06/23/2023 Added error codes for ipi access filtering
*       bm   07/06/2023 Added XPlmi_RunProc prototype
*                       Refactored Proc logic to more generic logic
*                       Added list commands prototypes
*       sk   07/28/2023 Added ptototype for XPlmi_IsPlmUpdateDoneTmp
*       bm   09/04/2023 Added support to use DDR region for backup of PLM data
*                       structures during In-Place PLM Update
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_PLAT_H
#define XPLMI_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xplmi_dma.h"
#include "xplmi_event_logging.h"
#include "xplmi_update.h"
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/

#define XPLMI_PLM_BANNER	"Xilinx Versal Net Platform Loader and Manager\n\r" /**< PLM banner */

/* PLM RunTime Configuration Area Base Address */
#define XPLMI_RTCFG_BASEADDR			(0xF2014000U) /**< Runtime configuration base address */

#define XPLMI_RTCFG_PMC_ERR1_STATUS_ADDR			(XPLMI_RTCFG_BASEADDR + 0x154U) /**< PMC error 1 status address */
#define XPLMI_RTCFG_PSM_ERR1_STATUS_ADDR			(XPLMI_RTCFG_BASEADDR + 0x15CU) /**< PSM error 1 status address */
#define XPLMI_RTCFG_PMC_ERR3_STATUS_ADDR			(XPLMI_RTCFG_BASEADDR + 0x190U) /**< PMC error 3 status address */
#define XPLMI_RTCFG_PSM_ERR3_STATUS_ADDR			(XPLMI_RTCFG_BASEADDR + 0x1A0U) /**< PSM error 3 status address */
#define XPLMI_RTCFG_SECURE_STATE_PLM_ADDR			(XPLMI_RTCFG_BASEADDR + 0x280U) /**< Secure state PLM address */
#define XPLMI_RTCFG_PLM_CRYPTO_STATUS_ADDR			(XPLMI_RTCFG_BASEADDR + 0x284U) /**< PLM crypto status address */
#define XPLMI_RTCFG_IMG_STORE_ADDRESS_HIGH			(XPLMI_RTCFG_BASEADDR + 0x288U) /**< Image store address high */
#define XPLMI_RTCFG_IMG_STORE_ADDRESS_LOW			(XPLMI_RTCFG_BASEADDR + 0x28CU) /**< Image store address low */
#define XPLMI_RTCFG_IMG_STORE_SIZE				(XPLMI_RTCFG_BASEADDR + 0x290U) /**< Image store size */
#define XPLMI_RTCFG_SECURE_DDR_KAT_ADDR				(XPLMI_RTCFG_BASEADDR + 0x294U) /**< Secure DDR KAT address */
#define XPLMI_RTCFG_SECURE_HNIC_CPM5N_PCIDE_KAT_ADDR		(XPLMI_RTCFG_BASEADDR + 0x298U) /**< Secure HNIC CPM5N PCIDE KAT address */
#define XPLMI_RTCFG_SECURE_PKI_KAT_ADDR_0			(XPLMI_RTCFG_BASEADDR + 0x29CU) /**< Secure PKI KAT address 0 */
#define XPLMI_RTCFG_SECURE_PKI_KAT_ADDR_1			(XPLMI_RTCFG_BASEADDR + 0x2A0U) /**< Secure PKI KAT address 1 */
#define XPLMI_RTCFG_SECURE_PKI_KAT_ADDR_2			(XPLMI_RTCFG_BASEADDR + 0x2A4U) /**< Secure PKI KAT address 2 */
#define XPLMI_RTCFG_PLM_RSVD_DDR_ADDR				(XPLMI_RTCFG_BASEADDR + 0x2A8U) /**< Baseadress of DDR region reserved for PLM */
#define XPLMI_RTCFG_PLM_RSVD_DDR_SIZE				(XPLMI_RTCFG_BASEADDR + 0x2ACU) /**< Size of DDR region reserved for PLM */

#define XPLMI_INVALID_PLM_RSVD_DDR_ADDR				(0x0U)
#define XPLMI_INVALID_PLM_RSVD_DDR_SIZE				(0U)

#define XPLMI_ROM_SERVICE_TIMEOUT			(1000000U) /**< ROM service timeout */

#define XPLMI_PMC_IRO_FREQ_320_MHZ	(320000000U) /**< PMC IRO frequency 320Mhz */

/**************************** Type Definitions *******************************/
/* Minor Error Codes */
enum {
	XPLMI_ERR_CURRENT_UART_INVALID = 0x2, /**< 0x2 - Error when current uart
						selected has invalid base address */
	XPLMI_ERR_INVALID_UART_SELECT, /**< 0x3 - Error when invalid uart select
						argument is passed */
	XPLMI_ERR_INVALID_UART_ENABLE, /**< 0x4 - Error when invalid uart enable
						argument is passed */
	XPLMI_ERR_NO_UART_PRESENT, /**< 0x5 - Error when no uart is present to
						configure in run-time */
	XPLMI_ERR_SET_IPI_MODULE_MAX, /**< 0x6 - Error if the module id is greater than max module count */
	XPLMI_ERR_SET_IPI_PERM_BUFF_NOT_REGISTERED, /**< 0x7 - Error if the permission buffer in which the access
						      permissions has to be stored is not registered or defined */
	XPLMI_ERR_SET_IPI_INVALID_API_ID_UPPER, /**< 0x8 - Error if the Api Id Upper limit is greater than
						maximum api count */
	XPLMI_ERR_SET_IPI_INVALID_API_ID_LOWER, /**< 0x9 - Error if the Api Id Lower limit is greater than
						Api Id upper limit */
	XPLMI_ERR_SET_IPI_ACCESS_PERM_NOT_SET, /**< 0xA - Error if the Ipi Access permission is not set
						 properly */
	XPLMI_ERR_VALIDATE_IPI_NO_SECURE_ACCESS, /**< 0xB - Error if the Api Id received during IPI request only
						   supports non-secure request */
	XPLMI_ERR_VALIDATE_IPI_INVALID_ID, /**< 0xC - Error if the module id or ipi source index is not
						valid during ipi validation */
	XPLMI_ERR_VALIDATE_IPI_MODULE_NOT_REGISTERED, /**< 0xD - Error if the module associated with the
						command's module id is not registered. This is checked
						during ipi validation */
	XPLMI_ERR_VALIDATE_IPI_INVALID_API_ID, /**< 0xE - Error if the Api Id received is greater than
						 the maximum supported commands in the module. This is checked
						 during ipi validation */
	XPLMI_ERR_VALIDATE_IPI_NO_IPI_ACCESS, /**< 0xF - Error if the Api Id received during IPI request doesn't
						have IPI access. */
	XPLMI_ERR_VALIDATE_IPI_NO_NONSECURE_ACCESS, /**< 0x10 - Error if the Api Id received during IPI request only
						   supports secure request */
	XPLMI_ERR_BUFFER_MEM_NOT_AVAILABLE, /**< 0x11 - Error if Buffer memory is not available for storing */

	/* Platform specific error codes start at 0x200 */
	XPLMI_ERR_INVALID_RSVD_DDR_REGION_UPDATE = 0x200, /**< 0x200 - Invalid/No DDR region reserved for PLM.
							    This is checked before the update */
	XPLMI_ERR_INVALID_RSVD_DDR_REGION_RESTORE, /**< 0x201 - Invalid/No DDR region reserved for
							     PLM during restore operation after the update */
	XPLMI_ERR_INSUFFICIENT_PLM_RSVD_DDR_REGION, /**< 0x202 - Insufficient DDR region reserved for PLM */
};

typedef struct {
	u8 Mode; /**< Operation information mode */
} XPlmi_ModuleOp;

typedef struct {
	u32 RomKatMask; /**< ROM Kat mask*/
	u32 PlmKatMask; /**< PLM Kat mask*/
	u32 DDRKatMask; /**< DDR Kat mask*/
	u32 HnicCpm5NPcideKatMask; /**< HNIC CPM5N PCIDE Kat mask*/
	u32 PKI0KatMask; /**< PKI0 Kat mask*/
	u32 PKI1KatMask; /**< PKI1 Kat mask*/
	u32 PKI2KatMask; /**< PKI2 Kat mask*/
}XPlmi_FipsKatMask;

typedef int (*XPlmi_UpdateHandler_t)(XPlmi_ModuleOp Op);

/* ROM interrupt services */
typedef enum {
	XPLMI_DME_CHL_SIGN_GEN = 0U,	/**< DME channel signature generation */
	XPLMI_PCR_OP,			/**< PCR extenstion */
	XPLMI_SHA2_HASH_GEN,		/**< SHA2 hash calculation */
	XPLMI_PLM_UPDT_REQ,		/**< In place PLM update */
	XPLMI_INVALID_INT		/**< Invalid interrupt */
} XPlmi_RomIntr;
/***************** Macros (Inline Functions) Definitions *********************/
/* PLMI GENERIC MODULE Data Structures IDs */
#define XPLMI_WDT_DS_ID			(0x01U) /**< WDT data structure Id */
#define XPLMI_TRACELOG_DS_ID		(0x02U) /**< Trace log data structure Id */
#define XPLMI_LPDINITIALIZED_DS_ID	(0x03U) /**< LPD intialized data structure Id */
#define XPLMI_UPDATE_IPIMASK_DS_ID	(0x04U) /**< Update IPI mask data structure Id */
#define XPLMI_UART_BASEADDR_DS_ID	(0x05U) /**< UART base address data structure Id */
#define XPLMI_ERROR_TABLE_DS_ID		(0x06U) /**< Error table data structure Id */
#define XPLMI_IS_PSMCR_CHANGED_DS_ID	(0x07U) /**< PSMCR status check data structure Id */
#define XPLMI_NUM_ERROUTS_DS_ID		(0x08U) /**< Number of error outs data structure Id */
#define XPLMI_BOARD_PARAMS_DS_ID	(0x09U) /**< Board parameters data structure Id */

/*
 * SLR Types
 */
#define XPLMI_SSIT_MONOLITIC		(0x7U) /**< SSIT monolitic */
#define XPLMI_SSIT_MASTER_SLR		(0x6U) /**< SSIT master SLR */
#define XPLMI_SSIT_SLAVE0_SLR_TOP	(0x5U) /**< Slave0 SLR Top */
#define XPLMI_SSIT_SLAVE0_SLR_NTOP	(0x4U) /**< Slave0 SLR NTop */
#define XPLMI_SSIT_SLAVE1_SLR_TOP	(0x3U) /**< Slave1 SLR Top */
#define XPLMI_SSIT_SLAVE1_SLR_NTOP	(0x2U) /**< Slave1 SLR NTop */
#define XPLMI_SSIT_SLAVE2_SLR_TOP	(0x1U) /**< Slave2 SLR Top */
#define XPLMI_SSIT_INVALID_SLR		(0x0U) /**< Invalid SLR */

/* GIC related Macros */
#define XPLMI_GICP_SOURCE_COUNT		(0x8U) /**< GICP source count */
#define XPLMI_GICP_INDEX_SHIFT		(16U) /**< GICP index shift */
#define XPLMI_GICPX_INDEX_SHIFT		(24U) /**< GICPX index shift */
#define XPLMI_GICPX_LEN			(0x14U) /**< GICPX length */

/*
 * PMC GIC interrupts
 */
#define XPLMI_PMC_GIC_IRQ_GICP0		(0U) /**< GICP0 Interrupt */
#define XPLMI_PMC_GIC_IRQ_GICP1		(1U) /**< GICP1 Interrupt */
#define XPLMI_PMC_GIC_IRQ_GICP2		(2U) /**< GICP2 Interrupt */
#define XPLMI_PMC_GIC_IRQ_GICP3		(3U) /**< GICP3 Interrupt */
#define XPLMI_PMC_GIC_IRQ_GICP4		(4U) /**< GICP4 Interrupt */
#define XPLMI_PMC_GIC_IRQ_GICP5		(5U) /**< GICP5 Interrupt */
#define XPLMI_PMC_GIC_IRQ_GICP6		(6U) /**< GICP6 Interrupt */

/*
 * PMC GICP0 interrupts
 */
#define XPLMI_GICP0_SRC20	(20U) /**< GPIO Interrupt */
#define XPLMI_GICP0_SRC21	(21U) /**< I2C_0 Interrupt */
#define XPLMI_GICP0_SRC22	(22U) /**< I2C_1 Interrupt */
#define XPLMI_GICP0_SRC23	(23U) /**< SPI_0 Interrupt */
#define XPLMI_GICP0_SRC24	(24U) /**< SPI_1 Interrupt */
#define XPLMI_GICP0_SRC25	(25U) /**< UART_0 Interrupt */
#define XPLMI_GICP0_SRC26	(26U) /**< UART_1 Interrupt */
#define XPLMI_GICP0_SRC27	(27U) /**< CAN_0 Interrupt */
#define XPLMI_GICP0_SRC28	(28U) /**< CAN_1 Interrupt */
#define XPLMI_GICP0_SRC29	(29U) /**< USB_0 Interrupt */
#define XPLMI_GICP0_SRC30	(30U) /**< USB_0 Interrupt */
#define XPLMI_GICP0_SRC31	(31U) /**< USB_0 Interrupt */

/*
 * PMC GICP1 interrupts
 */
#define XPLMI_GICP1_SRC0	(0U) /**< USB_0 Interrupt */
#define XPLMI_GICP1_SRC1	(1U) /**< USB_0 Interrupt */
#define XPLMI_GICP1_SRC2	(2U) /**< USB_1 Interrupt */
#define XPLMI_GICP1_SRC3	(3U) /**< USB_1 Interrupt */
#define XPLMI_GICP1_SRC4	(4U) /**< USB_1 Interrupt */
#define XPLMI_GICP1_SRC5	(5U) /**< USB_1 Interrupt */
#define XPLMI_GICP1_SRC6	(6U) /**< USB_1 Interrupt */
#define XPLMI_GICP1_SRC7	(7U) /**< GEM_0 Interrupt */
#define XPLMI_GICP1_SRC8	(8U) /**< GEM_0 Interrupt */
#define XPLMI_GICP1_SRC9	(9U) /**< GEM_1 Interrupt */
#define XPLMI_GICP1_SRC10	(10U) /**< GEM_1 Interrupt */
#define XPLMI_GICP1_SRC11	(11U) /**< TTC_0 Interrupt */
#define XPLMI_GICP1_SRC12	(12U) /**< TTC_0 Interrupt */
#define XPLMI_GICP1_SRC13	(13U) /**< TTC_0 Interrupt */
#define XPLMI_GICP1_SRC14	(14U) /**< TTC_1 Interrupt */
#define XPLMI_GICP1_SRC15	(15U) /**< TTC_1 Interrupt */
#define XPLMI_GICP1_SRC16	(16U) /**< TTC_1 Interrupt */
#define XPLMI_GICP1_SRC17	(17U) /**< TTC_2 Interrupt */
#define XPLMI_GICP1_SRC18	(18U) /**< TTC_2 Interrupt */
#define XPLMI_GICP1_SRC19	(19U) /**< TTC_2 Interrupt */
#define XPLMI_GICP1_SRC20	(20U) /**< TTC_3 Interrupt */
#define XPLMI_GICP1_SRC21	(21U) /**< TTC_3 Interrupt */
#define XPLMI_GICP1_SRC22	(22U) /**< TTC_3 Interrupt */

/*
 * PMC GICP2 interrupts
 */
#define XPLMI_GICP2_SRC8	(8U) /**< ADMA_0 Interrupt */
#define XPLMI_GICP2_SRC9	(9U) /**< ADMA_1 Interrupt */
#define XPLMI_GICP2_SRC10	(10U) /**< ADMA_2 Interrupt */
#define XPLMI_GICP2_SRC11	(11U) /**< ADMA_3 Interrupt */
#define XPLMI_GICP2_SRC12	(12U) /**< ADMA_4 Interrupt */
#define XPLMI_GICP2_SRC13	(13U) /**< ADMA_5 Interrupt */
#define XPLMI_GICP2_SRC14	(14U) /**< ADMA_6 Interrupt */
#define XPLMI_GICP2_SRC15	(15U) /**< ADMA_7 Interrupt */

/*
 * PMC GICP3 interrupts
 */
#define XPLMI_GICP3_SRC2	(2U) /**< USB_0 Interrupt */
#define XPLMI_GICP3_SRC3	(3U) /**< USB_1 Interrupt */

/*
 * PMC GICP4 interrupts
 */
#define XPLMI_GICP5_SRC22	(22U) /**< OSPI Interrupt */
#define XPLMI_GICP5_SRC23	(23U) /**< QSPI Interrupt */
#define XPLMI_GICP5_SRC24	(24U) /**< SD_0 Interrupt */
#define XPLMI_GICP5_SRC25	(25U) /**< SD_0 Interrupt */
#define XPLMI_GICP5_SRC26	(26U) /**< SD_1 Interrupt */
#define XPLMI_GICP5_SRC27	(27U) /**< SD_1 Interrupt */

/*
 * PMC GICP6 interrupts
 */
#define XPLMI_GICP6_SRC1	(1U) /**< SBI Interrupt */

#define XPLMI_SBI_GICP_INDEX	(XPLMI_PMC_GIC_IRQ_GICP6) /**< SBI GICP index */
#define XPLMI_SBI_GICPX_INDEX	(XPLMI_GICP6_SRC1) /**< SBI GICPX index */

#define XPLMI_IPI_INTR_ID	(0x1CU) /**< IPI interrupt Id */
#define XPLMI_IPI_INDEX_SHIFT	(24U) /**< IPI shift index */

/* PPU1 HW Interrupts */
#define XPLMI_HW_INT_GIC_IRQ	(0U) /**< GIC hardware interrupt */

#define XPLMI_HW_SW_INTR_MASK	(0xFF00U) /**< Hardware / software interrupt mask */
#define XPLMI_HW_SW_INTR_SHIFT	(0x8U) /**< Shift hardware / software interrupt */

/* Defines related to module commands */
#define XPLMI_PLM_GENERIC_PLMUPDATE		(0x20U) /**< Generic PLM update */

/* Module Operations */
#define XPLMI_MODULE_NO_OPERATION		(0U) /**< No operation */
#define XPLMI_MODULE_SHUTDOWN_INITIATE		(1U) /**< Shutdown initiate */
#define XPLMI_MODULE_SHUTDOWN_COMPLETE		(2U) /**< Shutdown complete */
#define XPLMI_MODULE_SHUTDOWN_ABORT		(3U) /**< Shutdown abort */

/* Module Handler States */
#define XPLMI_MODULE_NORMAL_STATE		(0U) /**< Normal state */
#define XPLMI_MODULE_SHUTDOWN_INITIATED_STATE	(1U) /**< Shutdown initiated state */
#define XPLMI_MODULE_SHUTDOWN_COMPLETED_STATE	(2U) /**< Shutdown completed state */

#define XPlmi_SsitSyncMaster	NULL /**< SSIT sync master */
#define XPlmi_SsitSyncSlaves	NULL /**< SSIT sync slaves */
#define XPlmi_SsitWaitSlaves	NULL /**< SSIT wait slaves */

/*
 * RTCA area KAT masks
 */
#define XPLMI_SECURE_SHA3_KAT_MASK 					(0x00000010U) /**< SHA3 Instance0 KAT mask */
#define XPLMI_SECURE_RSA_KAT_MASK 					(0x00000020U) /**< RSA KAT mask */
#define XPLMI_SECURE_ECC_SIGN_VERIFY_SHA3_384_KAT_MASK			(0x00000040U) /**< ECC sign verify SHA3_384 KAT mask */
#define XPLMI_SECURE_AES_DEC_KAT_MASK 					(0x00000080U) /**< AES decrypt KAT mask */
#define XPLMI_SECURE_AES_CMKAT_MASK					(0x00000100U) /**< AES CMKAT mask */
#define XPLMI_SECURE_TRNG_KAT_MASK					(0x00001000U) /**< TRNG KAT mask */
#define XPLMI_SECURE_SHA384_KAT_MASK					(0x00002000U) /**< SHA384 KAT mask */
#define XPLMI_SECURE_AES_ENC_KAT_MASK					(0x00004000U) /**< AES encrypt KAT mask */
#define XPLMI_SECURE_HMAC_KAT_MASK 					(0x00010000U) /**< HMAC KAT mask */
#define XPLMI_SECURE_RSA_PRIVATE_DEC_KAT_MASK	 			(0x00020000U) /**< RSA private decrypt KAT mask */
#define XPLMI_SECURE_ECC_SIGN_GEN_SHA3_384_KAT_MASK			(0x00040000U) /**< ECC sign generation SHA3_384 KAT mask */
#define XPLMI_SECURE_ECC_PWCT_KAT_MASK					(0x00080000U) /**< PWCT KAT mask */
#define XPLMI_SECURE_ECC_DEVIK_PWCT_KAT_MASK				(0x00100000U) /**< DEVIK PWCT KAT mask */
#define XPLMI_SECURE_ECC_DEVAK_PWCT_KAT_MASK				(0x00200000U) /**< DEVAK PWCT KAT mask */
#define XPLMI_SECURE_SHA3_1_KAT_MASK					(0x00400000U) /**< SHA3 Instance1 KAT mask */
#define XPLMI_SECURE_FIPS_STATE_MASK					(0xC0000000U) /**< FIPS state mask */

#define XPLMI_ROM_KAT_MASK		(XPLMI_SECURE_SHA3_KAT_MASK | XPLMI_SECURE_RSA_KAT_MASK |\
					XPLMI_SECURE_ECC_SIGN_VERIFY_SHA3_384_KAT_MASK | XPLMI_SECURE_AES_DEC_KAT_MASK | \
					XPLMI_SECURE_AES_CMKAT_MASK | XPLMI_SECURE_TRNG_KAT_MASK | \
					XPLMI_SECURE_AES_ENC_KAT_MASK | XPLMI_SECURE_HMAC_KAT_MASK)  /**< ROM KAT mask */

#define XPLMI_KAT_MASK			(XPLMI_ROM_KAT_MASK | XPLMI_SECURE_RSA_PRIVATE_DEC_KAT_MASK |\
					XPLMI_SECURE_ECC_SIGN_GEN_SHA3_384_KAT_MASK | \
					XPLMI_SECURE_ECC_PWCT_KAT_MASK | XPLMI_SECURE_ECC_DEVIK_PWCT_KAT_MASK | \
					XPLMI_SECURE_ECC_DEVAK_PWCT_KAT_MASK | XPLMI_SECURE_SHA384_KAT_MASK | \
					XPLMI_SECURE_SHA3_1_KAT_MASK) /**< KAT mask */


#define XPLMI_DDR_0_KAT_MASK			(0x0000000FU) /**< DDR 0 KAT mask */
#define XPLMI_DDR_1_KAT_MASK			(0x000000F0U) /**< DDR 1 KAT mask */
#define XPLMI_DDR_2_KAT_MASK			(0x00000F00U) /**< DDR 2 KAT mask */
#define XPLMI_DDR_3_KAT_MASK			(0x0000F000U) /**< DDR 3 KAT mask */
#define XPLMI_DDR_4_KAT_MASK			(0x000F0000U) /**< DDR 4 KAT mask */
#define XPLMI_DDR_5_KAT_MASK			(0x00F00000U) /**< DDR 5 KAT mask */
#define XPLMI_DDR_6_KAT_MASK			(0x0F000000U) /**< DDR 6 KAT mask */
#define XPLMI_DDR_7_KAT_MASK			(0xF0000000U) /**< DDR 7 KAT mask */
#define XPLMI_HNIC_KAT_MASK			(0x000000FFU) /**< HNIC KAT mask */
#define XPLMI_CPM5N_KAT_MASK			(0x0000FF00U) /**< CPM5N KAT mask */
#define XPLMI_PCIDE_KAT_MASK			(0x00030000U) /**< PCIDE KAT mask */
#define XPLMI_HNIC_CPM5N_PCIDE_KAT_MASK		(XPLMI_HNIC_KAT_MASK | XPLMI_CPM5N_KAT_MASK | XPLMI_PCIDE_KAT_MASK) /**< HNIC CPM5N PCIDE KAT mask */
#define XPLMI_PKI_KAT_MASK			(0x01FFFFFFU) /**< PKI KAT mask */

/*
 * RTCA area crypto bit masks
 */
#define XPLMI_SECURE_AES_MASK			(0x00200000U) /**< AES mask */
#define XPLMI_SECURE_RSA_MASK			(0x00400000U) /**< RSA mask */
#define XPLMI_SECURE_ECDSA_MASK			(0x00800000U) /**< ECDSA mask */
#define XPLMI_SECURE_SHA3_384_MASK		(0x01000000U) /**< SHA3_384 mask */
#define XPLMI_SECURE_TRNG_MASK			(0x02000000U) /**< TRNG mask */
#define XPLMI_SECURE_HNIC_AES_MASK		(0x04000000U) /**< HNIC AES mask */
#define XPLMI_SECURE_CPM5N_AES_MASK		(0x08000000U) /**< CPM5N AES mask */
#define XPLMI_SECURE_PCIDE_AES_MASK		(0x10000000U) /**< PCIDE AES mask */
#define XPLMI_SECURE_PKI_RSA_MASK		(0x20000000U) /**< PKI RSA mask */
#define XPLMI_SECURE_PKI_ECC_MASK		(0x40000000U) /**< PKI ECC mask */
#define XPLMI_SECURE_PKI_SHA2_MASK		(0x80000000U) /**< PKI SHA2 mask */
#define XPLMI_SECURE_PKI_CRYPTO_MASK	(XPLMI_SECURE_PKI_RSA_MASK | XPLMI_SECURE_PKI_ECC_MASK | \
					XPLMI_SECURE_PKI_SHA2_MASK) /**< PKI crypto mask */

#define XPLMI_PLM_CRYPTO_MASK           (XPLMI_SECURE_AES_MASK | XPLMI_SECURE_RSA_MASK | XPLMI_SECURE_ECDSA_MASK | \
					XPLMI_SECURE_SHA3_384_MASK | XPLMI_SECURE_TRNG_MASK) /**< PLM crypto mask */

#define GET_RTCFG_PMC_ERR_ADDR(Index)	(Index > 1U) ? \
			(XPLMI_RTCFG_PMC_ERR3_STATUS_ADDR) : \
			(XPLMI_RTCFG_PMC_ERR1_STATUS_ADDR + (Index * 4U)) /**< Runtime configuration PMC error address */
#define GET_RTCFG_PSM_ERR_ADDR(Index)		(Index > 1U) ? \
			(XPLMI_RTCFG_PSM_ERR3_STATUS_ADDR + ((Index - 2U) * 4U)) : \
			(XPLMI_RTCFG_PSM_ERR1_STATUS_ADDR + (Index * 4U)) /**< Runtime configuration PSM error address */

/*****************************************************************************/
/**
 * @brief	This function provides the Slr Type
 *
 * @return 	SlrType
 *
 *****************************************************************************/
static inline u8 XPlmi_GetSlrType(void)
{
	return XPLMI_SSIT_MONOLITIC;
}

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
/****************************************************************************/
/**
* @brief    This function enables the SSIT interrupts
*
* @return   None
*
****************************************************************************/
static inline void XPlmi_EnableSsitErrors(void)
{
	/* Not Applicable for versalnet */
	return;
}

/****************************************************************************/
/**
 * * @brief    This function initializes SSIT events
 * *
 * * @return   XST_SUCCESS
 * *
 * ****************************************************************************/
static inline int XPlmi_SsitEventsInit(void)
{
	/* Not Applicable for versalnet */
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 * @brief	This function Disables CFRAME Isolation
 *
 * @return	None
 *
 *****************************************************************************/
static inline void XPlmi_DisableCFrameIso(void)
{
	/* Not Applicable for Versal Net */
	return;
}

/*****************************************************************************/
/**
 * @brief	This function provides Iomodule instance pointer
 *
 * @return	Pointer to Iomodule instance
 *
 *****************************************************************************/
static inline void XPlmi_EnableIpiIntr(void)
{
	/* Not Applicable for Versal Net */
	return;
}

/*****************************************************************************/
/**
 * @brief	This function clears IPI interrupt
 *
 * @return	None
 *
 *****************************************************************************/
static inline void XPlmi_ClearIpiIntr(void)
{
	/* Not Applicable for Versal Net */
	return;
}

/*****************************************************************************/
/**
 * @brief	This function is not applicable for Versal Net
 *
 * @return	None
 *
 *****************************************************************************/
static inline void XPlmi_NotifySldSlaveSlrs(void)
{
	/* Not Applicable for Versal Net */
	return;
}

/*****************************************************************************/
/**
 * @brief	This function is not applicable for Versal Net
 *
 * @return	None
 *
 *****************************************************************************/
static inline void XPlmi_InterSlrSldHandshake(void)
{
	/* Not Applicable for Versal Net */
	return;
}

/************************** Function Prototypes ******************************/
u32 *XPlmi_GetLpdInitialized(void);
int XPlmi_PreInit(void);
void XPlmi_RtcaPlatInit(void);
u8 XPlmi_IsPlmUpdateDone(void);
u8 XPlmi_IsPlmUpdateDoneTmp(void);
u8 XPlmi_IsPlmUpdateInProgress(void);
void XPlmi_SssMask(u32 InputSrc, u32 OutputSrc);
XPlmi_CircularBuffer *XPlmi_GetTraceLogInst(void);
void XPlmi_GetReadbackSrcDest(u32 SlrType, u64 *SrcAddr, u64 *DestAddrRead);
int XPlmi_GenericHandler(XPlmi_ModuleOp Op);
void XPlmi_GicAddTask(u32 PlmIntrId);
int XPlmi_RegisterNEnableIpi(void);
void XPlmi_EnableIomoduleIntr(void);
int XPlmi_SetPmcIroFreq(void);
int XPlmi_GetPitResetValues(u32 *Pit1ResetValue, u32 *Pit2ResetValue);
int XPlmi_VerifyAddrRange(u64 StartAddr, u64 EndAddr);
XInterruptHandler *XPlmi_GetTopLevelIntrTbl(void);
u8 XPlmi_GetTopLevelIntrTblSize(void);
XPlmi_WaitForDmaDone_t XPlmi_GetPlmiWaitForDone(u64 DestAddr);
void XPlmi_PrintRomVersion(void);
u32 XPlmi_GetGicIntrId(u32 GicPVal, u32 GicPxVal);
u32 XPlmi_GetIpiIntrId(u32 BufferIndex);
u32 *XPlmi_GetUartBaseAddr(void);
u8 XPlmi_IsFipsModeEn(void);
u32 XPlmi_GetRomKatStatus(void);
void XPlmi_GetBootKatStatus(volatile u32 *PlmKatStatus);
void XPlmi_IpiIntrHandler(void *CallbackRef);
void XPlmi_ClearSSSCfgErr(void);
XPlmi_FipsKatMask* XPlmi_GetFipsKatMaskInstance(void);
int XPlmi_CheckAndUpdateFipsState(void);
void XPlmi_UpdateCryptoStatus(u32 Mask, u32 Val);
u32 XPlmi_GetCryptoStatus(u32 Mask);
u8 XPlmi_IsKatRan(u32 PlmKatMask);
u32 XPlmi_GetRomIroFreq(void);

/* Functions defined in xplmi_plat_cmd.c */
int XPlmi_InPlacePlmUpdate(XPlmi_Cmd *Cmd);
int XPlmi_PsmSequence(XPlmi_Cmd *Cmd);
int XPlmi_ScatterWrite(XPlmi_Cmd *Cmd);
int XPlmi_ScatterWrite2(XPlmi_Cmd *Cmd);
int XPlmi_SetFipsKatMask(XPlmi_Cmd *Cmd);
int XPlmi_RunProc(XPlmi_Cmd *Cmd);
void XPlmi_SetAddrBufferList(void);
int XPlmi_ListSet(XPlmi_Cmd *Cmd);
int XPlmi_ListWrite(XPlmi_Cmd *Cmd);
int XPlmi_ListMaskWrite(XPlmi_Cmd *Cmd);
int XPlmi_ListMaskPoll(XPlmi_Cmd *Cmd);

int XPlmi_RomISR(XPlmi_RomIntr RomServiceReq);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_PLAT_H */
