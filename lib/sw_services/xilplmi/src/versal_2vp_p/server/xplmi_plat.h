/***************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/


/**************************************************************************************************/
/**
*
* @file versal_2vp_p/server/xplmi_plat.h
*
* This file contains declarations of versal_2vp specific APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 2.3   sd  10/13/25 Initial release
*
* </pre>
*
* @note
*
***************************************************************************************************/

#ifndef XPLMI_PLAT_H
#define XPLMI_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xplmi_hw.h"
#include "xplmi_dma.h"
#include "xplmi_event_logging.h"
#include "xplmi_cmd.h"
#ifdef SDT
#include "xiltimer.h"
#endif
#include "xiomodule.h"
#include "xplmi_update.h"

/************************************ Constant Definitions ****************************************/
#define XPLMI_PLM_BANNER		"AMD Versal 2vp_p Platform Loader and Manager \n\r"
						/**< PLM banner */
#define XPLMI_PLM_PLAT_RC_VERSION	0U  /**< PLM Plat RC Version */

#define XPLMI_PLM_USER_DEFINED_VERSION	XPAR_PLM_VERSION_USER_DEFINED   /**< PLM User Defined
									     Version */

/* Maximum procs supported */
#define XPLMI_MAX_PSM_BUFFERS		(10U)  /**< Maximum PSM Buffers */
#define XPLMI_MAX_PMC_BUFFERS		(5U)   /**< Maximum PMC Buffers */

#define XPLMI_RTCFG_BASEADDR		(0xF2014000U) /**< Runtime configuration base address */

/* Offsets of PLM Runtime Configuration Registers */
#define XPLMI_RTCFG_PMC_ERR1_STATUS_ADDR	(XPLMI_RTCFG_BASEADDR + 0x154U)
							/**< PMC error 1 status address */
#define XPLMI_RTCFG_PSM_ERR1_STATUS_ADDR	(XPLMI_RTCFG_BASEADDR + 0x15CU)
							/**< PSM error 1 status address */
#define XPLMI_RTCFG_SECURE_STATE_PLM_ADDR	(XPLMI_RTCFG_BASEADDR + 0x16CU)
							/**< Secure state PLM address */

#define XPLMI_RTCFG_PLM_MJTAG_WA		(XPLMI_RTCFG_BASEADDR + 0x188U)
							/**< PLM MJTAG word aligner */
#define XPLMI_RTCFG_MIO_WA_BANK_500_ADDR	(XPLMI_RTCFG_BASEADDR + 0x270U)
							/**< Bank 500 address */
#define XPLMI_RTCFG_MIO_WA_BANK_501_ADDR	(XPLMI_RTCFG_BASEADDR + 0x274U)
							/**< Bank 501 address */
#define XPLMI_RTCFG_MIO_WA_BANK_502_ADDR	(XPLMI_RTCFG_BASEADDR + 0x278U)
							/**< Bank 502 address */
#define XPLMI_MIO_FLUSH_ALL_PINS		0x3FFFFFFU	/**< Flush all pins */
#define XPLMI_RTCFG_RST_PL_POR_WA		(XPLMI_RTCFG_BASEADDR + 0x27CU)
							/**< Reset PL_POR word aligner */
#define XPLMI_RTCFG_HBM_TEMP_CONFIG_AND_MAX	(XPLMI_RTCFG_BASEADDR + 0x280U)
							/**< HBM temporary configuration */
#define XPLMI_RTCFG_HBM_TEMP_VAL		(XPLMI_RTCFG_BASEADDR + 0x284U)
							/**< HBM temporary value */
#define XPLMI_RTCFG_IMG_STORE_ADDRESS_HIGH	(XPLMI_RTCFG_BASEADDR + 0x288U)
							/**< Image store address high */
#define XPLMI_RTCFG_IMG_STORE_ADDRESS_LOW	(XPLMI_RTCFG_BASEADDR + 0x28CU)
							/**< Image store address low */
#define XPLMI_RTCFG_IMG_STORE_SIZE		(XPLMI_RTCFG_BASEADDR + 0x290U)
							/**< Image store size */
#define XPLMI_RTCFG_SSIT_TEMP_PROPAGATION	(XPLMI_RTCFG_BASEADDR + 0x298U)
							/**< SSIT temperature prop config */
#define XPLMI_RTCFG_PLM_RSVD_DDR_ADDR		(XPLMI_RTCFG_BASEADDR + 0x2A8U)
							/**< Baseadress of DDR region reserved
							  for PLM */
#define XPLMI_RTCFG_PLM_RSVD_DDR_SIZE		(XPLMI_RTCFG_BASEADDR + 0x2ACU)
							/**< Size of DDR region reserved for PLM */
#define XPLMI_RTCFG_DDRMC_CALIB_CHECK_SKIP_ADDR	(XPLMI_RTCFG_BASEADDR + 0x300U)
							/**< Skip DDRMC Calib Check */
#ifdef PLM_ENABLE_SECURE_PLM_TO_PLM_COMM
#define XPLMI_RTCFG_SSLR1_IV_ADDR		(XPLMI_RTCFG_BASEADDR + 0x324U)
							/**< Base address of IV of SSLR1 in master
							     for secure comm */
#define XPLMI_RTCFG_SSLR2_IV_ADDR		(XPLMI_RTCFG_BASEADDR + 0x334U)
							/**< Base address of IV of SSLR2 in master
							     for secure comm */
#define XPLMI_RTCFG_SSLR3_IV_ADDR		(XPLMI_RTCFG_BASEADDR + 0x344U)
							/**< Base address of IV of SSLR3 in master
							     for secure comm */
#define XPLMI_SLR_CURRENTIV_ADDR		XPLMI_RTCFG_SSLR1_IV_ADDR
							/**< Base address of current IV in slave for
							     secure comm */
#define XPLMI_SLR_NEWIV_ADDR			XPLMI_RTCFG_SSLR2_IV_ADDR
							/**< Base address of new IV in slave for
							     secure comm */
#endif

/* Masks of PLM RunTime Configuration Registers */
#define XPLMI_RTCFG_PLM_MJTAG_WA_IS_ENABLED_MASK	(0x00000001U) /**< MJTAG word aligner enable
									   status check mask */
#define XPLMI_RTCFG_PLM_MJTAG_WA_STATUS_MASK		(0x00000002U) /**< MJTAG word aligner status
									   mask */

/* Shifts of PLM RunTime Configuration Registers */
#define XPLMI_RTCFG_PLM_MJTAG_WA_STATUS_SHIFT		(0x00000001U) /**< MJTAG word aligner status
									   shift */

#define XPLMI_RTCFG_INPLACE_UPDATE_IPI_MASK		(XPLMI_RTCFG_BASEADDR + 0x2B4U)
								/**< RTCA Register to save IPI Mask */
#define XPLMI_RTCFG_INPLACE_UPDATE_IPI_RESP_BUFF	(XPLMI_RTCFG_BASEADDR + 0x2B8U)
								/**< RTCA Register to save Response Buffer Address */
#define XPLMI_RTCFG_INPLACE_UPDATE_ERR_IPOR_TIMEOUT	(XPLMI_RTCFG_BASEADDR + 0x2BCU)
								/**< RTCA Register to store timeout in milli seconds before IPOR */
/* GIC related Macros */

#define XPLMI_GICP_SOURCE_COUNT		(0x5U) /**< GICP source count */
#define XPLMI_GICP_INDEX_SHIFT		(8U) /**< GICP index shift */
#define XPLMI_GICPX_INDEX_SHIFT		(16U) /**< GICPX index shift */
#define XPLMI_GICPX_LEN			(0x14U) /**< GICPX length */

/*
 * PMC GIC interrupts
 */
#define XPLMI_PMC_GIC_IRQ_GICP0		(0U) /**< GICP0 Interrupt */
#define XPLMI_PMC_GIC_IRQ_GICP1		(1U) /**< GICP1 Interrupt */
#define XPLMI_PMC_GIC_IRQ_GICP2		(2U) /**< GICP2 Interrupt */
#define XPLMI_PMC_GIC_IRQ_GICP3		(3U) /**< GICP3 Interrupt */
#define XPLMI_PMC_GIC_IRQ_GICP4		(4U) /**< GICP4 Interrupt */

/*
 * PMC GICP0 interrupts
 */
#define XPLMI_GICP0_SRC13	(13U) /**< GPIO Interrupt */
#define XPLMI_GICP0_SRC14	(14U) /**< I2C_0 Interrupt */
#define XPLMI_GICP0_SRC15	(15U) /**< I2C_1 Interrupt */
#define XPLMI_GICP0_SRC16	(16U) /**< SPI_0 Interrupt */
#define XPLMI_GICP0_SRC17	(17U) /**< SPI_1 Interrupt */
#define XPLMI_GICP0_SRC18	(18U) /**< UART_0 Interrupt */
#define XPLMI_GICP0_SRC19	(19U) /**< UART_1 Interrupt */
#define XPLMI_GICP0_SRC20	(20U) /**< CAN_0 Interrupt */
#define XPLMI_GICP0_SRC21	(21U) /**< CAN_1 Interrupt */
#define XPLMI_GICP0_SRC22	(22U) /**< USB_0 Interrupt */
#define XPLMI_GICP0_SRC23	(23U) /**< USB_0 Interrupt */
#define XPLMI_GICP0_SRC24	(24U) /**< USB_0 Interrupt */
#define XPLMI_GICP0_SRC25	(25U) /**< USB_0 Interrupt */
#define XPLMI_GICP0_SRC26	(26U) /**< USB_0 Interrupt */
#define XPLMI_GICP0_SRC27	(27U) /**< IPI Interrupt */

/*
 * PMC GICP1 interrupts
 */
#define XPLMI_GICP1_SRC5	(5U) /**< TTC_0 Interrupt */
#define XPLMI_GICP1_SRC6	(6U) /**< TTC_0 Interrupt */
#define XPLMI_GICP1_SRC7	(7U) /**< TTC_0 Interrupt */
#define XPLMI_GICP1_SRC8	(8U) /**< TTC_1 Interrupt */
#define XPLMI_GICP1_SRC9	(9U) /**< TTC_1 Interrupt */
#define XPLMI_GICP1_SRC10	(10U) /**< TTC_1 Interrupt */
#define XPLMI_GICP1_SRC11	(11U) /**< TTC_2 Interrupt */
#define XPLMI_GICP1_SRC12	(12U) /**< TTC_2 Interrupt */
#define XPLMI_GICP1_SRC13	(13U) /**< TTC_2 Interrupt */
#define XPLMI_GICP1_SRC14	(14U) /**< TTC_3 Interrupt */
#define XPLMI_GICP1_SRC15	(15U) /**< TTC_3 Interrupt */
#define XPLMI_GICP1_SRC16	(16U) /**< TTC_3 Interrupt */
#define XPLMI_GICP1_SRC24	(24U) /**< GEM_0 Interrupt */
#define XPLMI_GICP1_SRC25	(25U) /**< GEM_0 Interrupt */
#define XPLMI_GICP1_SRC26	(26U) /**< GEM_1 Interrupt */
#define XPLMI_GICP1_SRC27	(27U) /**< GEM_1 Interrupt */
#define XPLMI_GICP1_SRC28	(28U) /**< ADMA_0 Interrupt */
#define XPLMI_GICP1_SRC29	(29U) /**< ADMA_1 Interrupt */
#define XPLMI_GICP1_SRC30	(30U) /**< ADMA_2 Interrupt */
#define XPLMI_GICP1_SRC31	(31U) /**< ADMA_3 Interrupt */

/*
 * PMC GICP2 interrupts
 */
#define XPLMI_GICP2_SRC0	(0U) /**< ADMA_4 Interrupt */
#define XPLMI_GICP2_SRC1	(1U) /**< ADMA_5 Interrupt */
#define XPLMI_GICP2_SRC2	(2U) /**< ADMA_6 Interrupt */
#define XPLMI_GICP2_SRC3	(3U) /**< ADMA_7 Interrupt */
#define XPLMI_GICP2_SRC10	(10U) /**< USB_0 Interrupt */

/*
 * PMC GICP3 interrupts
 */
#define XPLMI_GICP3_SRC30	(30U) /**< SD_0 Interrupt */
#define XPLMI_GICP3_SRC31	(31U) /**< SD_0 Interrupt */
#define XPLMI_GICP3_SRC26	(26U) /**< PMC GPIO Interrupt */

/*
 * PMC GICP4 interrupts
 */
#define XPLMI_GICP4_SRC0	(0U) /**< SD_1 Interrupt */
#define XPLMI_GICP4_SRC1	(1U) /**< SD_1 Interrupt */
#define XPLMI_GICP4_SRC8	(8U) /**< SBI interrupt */
#define XPLMI_GICP4_SRC14	(14U) /**< RTC interrupt */

#define XPLMI_IPI_INTR_ID	(0x1B0000U) /**< IPI Interrupt Id */
#define XPLMI_IPI_INDEX_SHIFT	(24U) /**< IPI shift index */
#define XPLMI_GIC_IPI_INTR_ID	(0x1B0010U) /**< GIC IPI Interrupt Id */

#define XPLMI_SBI_GICP_INDEX	(XPLMI_PMC_GIC_IRQ_GICP4) /**< SBI GICP index */
#define XPLMI_SBI_GICPX_INDEX	(XPLMI_GICP4_SRC8) /**< SBI GICPX index */

/*
 * DDRMC Defines
 */
#define MAX_DEV_DDRMC				(4U)	/**< Maximum device for Double Data Rate
							     Memory Controller */
#define XPLMI_DDRMC_UB0_BASE_ADDR		(0xF6110000U) /**< DDRMC_UB0 base address */
#define XPLMI_DDRMC_UB1_BASE_ADDR		(0xF6280000U) /**< DDRMC_UB1 base address */
#define XPLMI_DDRMC_UB2_BASE_ADDR		(0xF63F0000U) /**< DDRMC_UB2 base address */
#define XPLMI_DDRMC_UB3_BASE_ADDR		(0xF6560000U) /**< DDRMC_UB3 base address */
#define XPLMI_DDRMC_UB_PCSR_LOCK_OFFSET		(0x0CU) /**< Offset of PCSR register */
#define XPLMI_DDRMC_UB_PMC2UB_GPI1_OFFSET	(0x244U) /**< Offset of PMC2UBInfo register */

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
#define XPLMI_SECURE_ECC_SIGN_GEN_SHA3_384_KAT_MASK			(0x00008000U) /**< ECC sign generation SHA3_384 KAT mask */
#define XPLMI_SECURE_HMAC_KAT_MASK 					(0x00010000U) /**< HMAC KAT mask */
#define XPLMI_SHA2_256_KAT_MASK                                  	(0x00080000U) /**< SHA2 256 KAT mask */
#define XPLMI_SHAKE_256_KAT_MASK                                 	(0x00100000U) /**< SHAKE KAT mask */
#define XPLMI_HSS_SHA2_256_KAT_MASK                              	(0x00200000U) /**< LMS-HSS SHA2 256 KAT mask */
#define XPLMI_HSS_SHAKE_256_KAT_MASK                             	(0x00400000U) /**< LMS-HSS SHAKE 256 KAT mask */
#define XPLMI_LMS_SHA2_256_KAT_MASK					(0x00800000U) /**< LMS SHA2 256 KAT mask */
#define XPLMI_LMS_SHAKE_256_KAT_MASK					(0x01000000U) /**< LMS SHAKE 256 KAT mask */

#define XPLMI_SECURE_RSA_PRIVATE_DEC_KAT_MASK	 			(0x000C0000U) /**< RSA private decrypt KAT mask */
#define XPLMI_SECURE_ECC_PWCT_KAT_MASK					(0x00080000U) /**< PWCT KAT mask */
#define XPLMI_SECURE_ECC_DEVIK_PWCT_KAT_MASK				(0x00100000U) /**< DEVIK PWCT KAT mask */
#define XPLMI_SECURE_ECC_DEVAK_PWCT_KAT_MASK				(0x00200000U) /**< DEVAK PWCT KAT mask */
#define XPLMI_SECURE_FIPS_STATE_MASK					(0xC0000000U) /**< FIPS state mask */

#define XPLMI_ROM_KAT_MASK		(XPLMI_SECURE_SHA3_KAT_MASK | XPLMI_SECURE_RSA_KAT_MASK |\
					XPLMI_SECURE_ECC_SIGN_VERIFY_SHA3_384_KAT_MASK | XPLMI_SECURE_AES_DEC_KAT_MASK | \
					XPLMI_SECURE_AES_CMKAT_MASK | XPLMI_SECURE_TRNG_KAT_MASK | \
					XPLMI_SECURE_SHA384_KAT_MASK | XPLMI_SECURE_ECC_SIGN_GEN_SHA3_384_KAT_MASK | \
					XPLMI_SHA2_256_KAT_MASK | \
					XPLMI_SHAKE_256_KAT_MASK | XPLMI_HSS_SHA2_256_KAT_MASK | \
					XPLMI_HSS_SHAKE_256_KAT_MASK | XPLMI_LMS_SHA2_256_KAT_MASK | \
					XPLMI_LMS_SHAKE_256_KAT_MASK | \
					XPLMI_SECURE_AES_ENC_KAT_MASK | XPLMI_SECURE_HMAC_KAT_MASK)  /**< ROM KAT mask */

#define XPLMI_KAT_MASK			(XPLMI_ROM_KAT_MASK | XPLMI_SECURE_RSA_PRIVATE_DEC_KAT_MASK |\
					XPLMI_SECURE_FIPS_STATE_MASK) /**< KAT mask */

#define XPLMI_SSSCFG_SHA_MASK		(0x000F0000U) /* SSS config SHA mask */
#define XPLMI_SSSCFG_AES_MASK		(0x0000F000U) /* SSS config AES mask */
#define XPLMI_SSS_SHA_DMA0		(0x000C0000U) /* SSS SHA DMA0 mask */
#define XPLMI_SSS_SHA_DMA1		(0x00070000U) /* SSS SHA DMA1 mask */
#define XPLMI_SSS_AES_DMA0		(0x0000E000U) /* SSS AES DMA0 mask */
#define XPLMI_SSS_AES_DMA1		(0x00005000U) /* SSS AES DMA1 mask */

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

#define XPLMI_RTCFG_DDRMC_CALIB_CHECK_SKIP_ADDR			(XPLMI_RTCFG_BASEADDR + 0x300U) /**< Skip DDRMC Calib Check */
#define XPLMI_INVALID_RESP_BUFF_ADDR				(0xFFFFFFFFU)  /**< Invalid Response Buffer Address */
#define XPLMI_INVALID_IPI_MASK					(0x0U)  /**< Invalid IPI Mask */
#define XPLMI_INVALID_PLM_RSVD_DDR_ADDR				(0x0U)	/**< Invalid reserved DDR address */
#define XPLMI_INVALID_PLM_RSVD_DDR_SIZE				(0U)	/**< Invalid reserved DDR size */
#define XPLMI_REG_OFFSET_BYTE_4					(4U)  /**< Register Offset by 4 bytes */



/************************************** Type Definitions ******************************************/
/* Minor Error Codes */
enum {
	/* Do not move this error, as it is used to represent XST_FAILURE */
	/** 0x1 - XPLMI_ERR_FAILURE */
	XPLMI_ERR_FAILURE = 0x1,

	/** 0x2 - Error when current uart selected has invalid base address */
	XPLMI_ERR_CURRENT_UART_INVALID,

	/** 0x3 - Error when invalid uart select argument is passed */
	XPLMI_ERR_INVALID_UART_SELECT,

	/** 0x4 - Error when invalid uart enable argument is passed */
	XPLMI_ERR_INVALID_UART_ENABLE,

	/** 0x5 - Error when no uart is present to configure in run-time */
	XPLMI_ERR_NO_UART_PRESENT,

	/** 0x6 - Error if the permission buffer in which the access permissions has to be stored is
	 * not registered or defined */
	XPLMI_ERR_SET_IPI_PERM_BUFF_NOT_REGISTERED,

	/** 0x7 - Error if the Api Id Upper limit is greater than maximum api count */
	XPLMI_ERR_SET_IPI_INVALID_API_ID_UPPER,

	/** 0x8 - Error if the Api Id Lower limit is greater than Api Id upper limit */
	XPLMI_ERR_SET_IPI_INVALID_API_ID_LOWER,

	/** 0x9 - Error if the Ipi Access permission is not set properly */
	XPLMI_ERR_SET_IPI_ACCESS_PERM_NOT_SET,

	/** 0xA - Error if the Api Id received during IPI request only supports non-secure request
	 * */
	XPLMI_ERR_VALIDATE_IPI_NO_SECURE_ACCESS,

	/** 0xB - Error if the module id or ipi source index is not valid during ipi validation */
	XPLMI_ERR_VALIDATE_IPI_INVALID_ID,

	/** 0xC - Error if the module associated with the command's module id is not registered.
	 * This is checked during ipi validation */
	XPLMI_ERR_VALIDATE_IPI_MODULE_NOT_REGISTERED,

	/** 0xD - Error if the Api Id received is greater than the maximum supported commands in the
	 * module. This is checked during ipi validation */
	XPLMI_ERR_VALIDATE_IPI_INVALID_API_ID,

	/** 0xE - Error if the Api Id received during IPI request doesn't have IPI access. */
	XPLMI_ERR_VALIDATE_IPI_NO_IPI_ACCESS,

	/* Do not move this error, as it is used to represent XST_INVALID_PARAM */
	/** 0xF - XPLMI_ERR_INVALID_PARAM */
	XPLMI_ERR_INVALID_PARAM,

	/** 0x10 - Error if the Api Id received during IPI request only supports secure request */
	XPLMI_ERR_VALIDATE_IPI_NO_NONSECURE_ACCESS,

	/** 0x11 - Error if Buffer memory is not available for storing */
	XPLMI_ERR_BUFFER_MEM_NOT_AVAILABLE,

	/** 0x12 - Error when End Address stack is empty */
	XPLMI_ERR_END_ADDR_STACK_EMPTY,

	/** 0x13 - Error when End Address stack doesn't have a valid top address */
	XPLMI_ERR_INVALID_STACK_TOP,

	/** 0x14 - Error when max limit of nested begin is reached */
	XPLMI_ERR_MAX_NESTED_BEGIN,

	/** 0x15 - Error when the stack top is not valid while pushing into it */
	XPLMI_ERR_INVALID_STACK_TOP_DURING_PUSH,

	/** 0x16 - Error when End address stack is full */
	XPLMI_ERR_END_ADDR_STACK_FULL,

	/** 0x17 - Error when End command doesn't have a valid begin */
	XPLMI_ERR_END_CMD_HAS_NO_BEGIN,

	/** 0x18 - Error when invalid ProcID is received. */
	XPLMI_ERR_PROCID_NOT_VALID,

	/** 0x19 - The Proc command cannot be stored or executed because the LPD is not initialized.
	 * */
	XPLMI_ERR_PROC_LPD_NOT_INITIALIZED,

	/** 0x1A - Maximum supported proc commands received */
	XPLMI_MAX_PROC_COMMANDS_RECEIVED,

	/** 0x1B - Received proc does not fit in proc memory */
	XPLMI_UNSUPPORTED_PROC_LENGTH,

	/** 0x1C - Error when invalid log level is received in Logging command. */
	XPLMI_ERR_INVALID_LOG_LEVEL,

	/** 0x1D - Error when invalid log buffer address is received in Logging command. */
	XPLMI_ERR_INVALID_LOG_BUF_ADDR,

	/** 0x1E - Error when invalid log buffer length is received in Logging command. */
	XPLMI_ERR_INVALID_LOG_BUF_LEN,

	/** 0x1F - Error if readback buffer overflows. */
	XPLMI_ERR_READBACK_BUFFER_OVERFLOW,

	/* Do not move this error, as it is used to represent XST_GLITCH_DETECTED */
	/** 0x20 - XPLMI_ERR_GLITCH_DETECTED. */
	XPLMI_ERR_GLITCH_DETECTED,

	/** 0x21 - Error on unsupported CDO command. */
	XPLMI_ERR_CMD_NOT_SUPPORTED,

	/** 0x22 - Error on invalid Periodicity parameter. */
	XPLMI_ERR_WDT_PERIODICITY,

	/** 0x23 - Error on invalid Node ID parameter. */
	XPLMI_ERR_WDT_NODE_ID,

	/** 0x24 - LPD MIO is used for WDT but LPD is not initialized. */
	XPLMI_ERR_WDT_LPD_NOT_INITIALIZED,

	/** 0x25 - Invalid tamper response received for TamperTrigger IPI call */
	XPLMI_INVALID_TAMPER_RESPONSE,

	/** 0x26 - Error when invalid log buffer type is received in Logging command. */
	XPLMI_ERR_INVALID_LOG_BUF_TYPE,

	/**< 0x27 - Error when registering the address range handler */
	XPLMI_ERR_REGISTER_VERIFY_ADDR_RANGE_HANDLER,

	/** 0x28 - Error when invalid address range is detected. */
	XPLMI_ERR_INVALID_ADDR_RANGE,

	/* Platform specific error codes start at 0x200 */
	/** 0x200 - Error received from SSIT Slave SLR */
	XPLMI_ERR_FROM_SSIT_SLAVE = 0x200,

	/** 0x201 - Error when SSIT master times out waiting for slaves sync point */
	XPLMI_ERR_SSIT_SLAVE_SYNC,

	/**<
	 * 0x202 - Minor error if DMA initialization returns NULL during secure plm to plm
	 * communication
	 */
	XPLMI_SSIT_INAVLID_DMA,

	/**<
	 * 0x203 - Minor error if AES initialization fails during secure plm to plm communication
	 */
	XPLMI_SSIT_AES_INIT_FAIL,

	/**<
	 * 0x204 - Minor error if encryption/decryption initialization fails during secure plm to
	 * plm communication
	 */
	XPLMI_SSIT_ENCDEC_INIT_FAIL,

	/**< 0x205 - Minor error if AAD updation fails during encryption/decryption operation */
	XPLMI_SSIT_AAD_UPDATE_FAIL,

	/**< 0x206 - Minor error if data updation fails during encryption/decryption operation */
	XPLMI_SSIT_DATA_UPDATE_FAIL,

	/**< 0x207 - Minor error if tag generation fails during encryption/decryption operation */
	XPLMI_SSIT_TAG_GENERATION_FAIL,

	/** 0x200 - Invalid/No DDR region reserved for PLM. This is checked before the update */
	XPLMI_ERR_INVALID_RSVD_DDR_REGION_UPDATE = 0x200,

	/** 0x201 - Invalid/No DDR region reserved for PLM during restore operation after the update */
	XPLMI_ERR_INVALID_RSVD_DDR_REGION_RESTORE,

	/** 0x202 - Insufficient DDR region reserved for PLM */
	XPLMI_ERR_INSUFFICIENT_PLM_RSVD_DDR_REGION,


	/** 0x203 - Error while In-Place Update from Image Store */
	XPLMI_ERR_INPLACE_UPDATE_FROM_IMAGE_STORE,

	/** 0x204 - Error while In-Place Update with Invalid Payload Len */
	XPLMI_ERR_INPLACE_UPDATE_INVALID_PAYLOAD_LEN,

	/** 0x205 - Error when invalid Optional Data Length is used in the PDI used for In-Place PLM Update */
	XPLMI_ERR_INPLACE_INVALID_OPTIONAL_DATA_LEN,

	/** 0x206 - Error when a task cannot be executed as the update is in progress.*/
	XPLMI_ERR_INPLACE_UPDATE_IN_PROGRESS,

	/** 0x207 - Error in shutdown initiate of modules during InPlace PLM Update */
	XPLMI_ERR_INPLACE_UPDATE_SHUTDOWN_INIT,

	/** 0x208 - Error when PLM Update is disabled in ROM_RSV efuse */
	XPLMI_ERR_INPLACE_UPDATE_DISABLED,

	/** 0x209 - Error when PLM Update task is not found */
	XPLMI_ERR_INPLACE_UPDATE_TASK_NOT_FOUND,

	/** 0x20A - Error when PMC tries to use WDT but not enabled in design. */
	XPLMI_ERR_PMC_WDT_NOT_ENABLED,

	/** 0x20B - Error when PMC WDT driver initialization fails. */
	XPLMI_ERR_PMC_WDT_DRV_INIT,

	/** 0x20C - Invalid payload length received for the command */
	XPLMI_ERR_INVALID_PAYLOAD_LEN,

	/** 0x20D - ROM error during In Place Update */
	XPLMI_ERR_INPLACE_UPDATE_ROM_ERROR,

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
	XPLMI_PCR_OP,			/**< PCR extension */
	XPLMI_SHA2_HASH_GEN,		/**< SHA2 hash calculation */
	XPLMI_PLM_UPDT_REQ,		/**< In place PLM update */
	XPLMI_INVALID_INT		/**< Invalid interrupt */
} XPlmi_RomIntr;
/***************** Macros (Inline Functions) Definitions *********************/
/* PLMI GENERIC MODULE Data Structures IDs */
#define XPLMI_WDT_DS_ID			(0x01U) /**< WDT data structure Id */
#define XPLMI_TRACELOG_DS_ID		(0x02U) /**< Trace log data structure Id */
#define XPLMI_LPDINITIALIZED_DS_ID	(0x03U) /**< LPD initialized data structure Id */
#define XPLMI_RESERVED_DS_ID		(0x04U) /**< Update RESERVED DS ID */
#define XPLMI_UART_BASEADDR_DS_ID	(0x05U) /**< UART base address data structure Id */
#define XPLMI_ERROR_TABLE_DS_ID		(0x06U) /**< Error table data structure Id */
#define XPLMI_IS_PSMCR_CHANGED_DS_ID	(0x07U) /**< PSMCR status check data structure Id */
#define XPLMI_NUM_ERROUTS_DS_ID		(0x08U) /**< Number of error outs data structure Id */
#define XPLMI_BOARD_PARAMS_DS_ID	(0x09U) /**< Board parameters data structure Id */
#define XPLMI_PPU_BUFFER_DS_ID		(0x0AU) /**< PSM Buffers data structure Id */
#define XPLMI_PMC_BUFFER_DS_ID		(0x0BU) /**< PMC Buffers data structure Id */
#define XPLMI_UPDATE_PDIADDR_DS_ID	(0x10U) /**< Update IPI mask data structure Id */
#define XPLMI_XIOMODULE_DS_ID		(0x11U) /**< IOModule data structure Id */
#define XPLMI_PPU_BUFFER_LIST_DS_ID	(0x12U) /**< PSM Buffers data structure Id */
#define XPLMI_PMC_BUFFER_LIST_DS_ID	(0x13U) /**< PMC Buffers data structure Id */


/*************************** Macros (Inline Functions) Definitions ********************************/
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

#define XPlmi_InPlacePlmUpdate		NULL /**< InPlace PLM update */
#define XPlmi_PsmSequence		NULL /**< PSM sequence */
#define XPlmi_ScatterWrite		NULL /**< Scatter write */
#define XPlmi_ScatterWrite2		NULL /**< Scatter write2 */
#define XPlmi_SetFipsKatMask		NULL /**< Set Fip Kat mask */
#define XPlmi_RunProc			NULL /**< Run Proc */
#define XPlmi_ListSet			NULL /**< List Set */
#define XPlmi_ListWrite			NULL /**< List Write */
#define XPlmi_ListMaskWrite		NULL /**< List Mask Write */
#define XPlmi_ListMaskPoll		NULL /**< List Mask Poll */

#define GET_RTCFG_PMC_ERR_ADDR(Index)	(XPLMI_RTCFG_PMC_ERR1_STATUS_ADDR + (Index * 4U))
						/**< Runtime configuration PMC error address */
#define GET_RTCFG_PSM_ERR_ADDR(Index)	(XPLMI_RTCFG_PSM_ERR1_STATUS_ADDR + (Index * 4U))
						/**< Runtime configuration PSM error address */

#define XPLMI_ROM_SERVICE_TIMEOUT			(1000000U) /**< ROM service timeout */
#define XPLMI_MILLI_SEC_TIME_MULTIPLIER 		(0x10000U) /**< factor for ~1msec for 320MHz to 400MHz range */
#define XPLMI_PMC_IRO_FREQ_320_MHZ	(320000000U) /**< PMC IRO frequency 320Mhz */

#define XPLMI_STATUS_GLITCH_DETECT(Status) /**< Glitch check on Status (excluded for Versal_2vp). */

/**< Function pointer type for address range validation handler */
typedef XStatus (*XPlmi_IsAddrRangeValid_t)(u32 SubsystemId, u64 Addr, u64 Size);

/**< Global address range validation handler */
extern XPlmi_IsAddrRangeValid_t XPlmi_IsAddrRangeValid;

/******************************************************************************/
/**
 * This macro calls the registered address validation handler (XPm_IsMemAddressValid)
 * to ensure memory access is within valid bounds for security-critical functions.
 * This feature is conditionally compiled based on PLM_ENABLE_ADDR_RANGE_VALIDATION.
 *
 * @param   SubsysId         Subsystem ID
 * @param   StartAddr        Start address of the range to verify
 * @param   Len              Length of the address range to verify
 * @param   StatusVar        Status variable to be updated
 * @param   ErrorCode        Error code to set on failure
 * @param   Label            Label to jump to on error
 *
 * @return  None
 *
 ******************************************************************************/
#ifdef PLM_ENABLE_ADDR_RANGE_VALIDATION
#define XPLMI_VERIFY_ADDR_RANGE(SubsysId, StartAddr, Len, StatusVar, ErrorCode, Label) \
	{ \
		StatusVar = XPlmi_IsAddrRangeValid(SubsysId, StartAddr, Len); \
		if (StatusVar != XST_SUCCESS) { \
			StatusVar = ErrorCode; \
			goto Label; \
		} \
	}
#else
#define XPLMI_VERIFY_ADDR_RANGE(SubsysId, StartAddr, Len, StatusVar, ErrorCode, Label) \
	do { \
		/* Address range validation is disabled for Versal by default */ \
		(void)(SubsysId); \
		(void)(StartAddr); \
		(void)(Len); \
		(void)(StatusVar); \
		(void)(ErrorCode); \
	} while(0)
#endif

/**************************************************************************************************/
/**
 * @brief	This function provides the Slr Type.
 *
 * @return
 * 		- SlrType.
 *
 **************************************************************************************************/
static inline u8 XPlmi_GetSlrType(void)
{
	u8 SlrType = (u8)(XPlmi_In32(PMC_TAP_SLR_TYPE) & PMC_TAP_SLR_TYPE_VAL_MASK);

	return SlrType;
}

/**************************************************************************************************/
/**
 * @brief	This function is not applicable for versal_2vp.
 *
 * @return
 * 		- None.
 *
 **************************************************************************************************/
static inline void XPlmi_IpiIntrHandler(void *CallbackRef)
{
	(void)CallbackRef;
	/* Not Applicable for Versal_2vp */
	return;
}

/**************************************************************************************************/
/**
 * @brief	This function is not applicable for versal_2vp.
 *
 * @return
 * 		- None.
 *
 *****************************************************************************/
static inline void XPlmi_ClearSSSCfgErr(void)
{
	/* Not Applicable for Versal_2vp */
	return;
}

/**************************************************************************************************/
/**
 * @brief	This function is not applicable for versal_2vp.
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 **************************************************************************************************/
static inline int XPlmi_CheckAndUpdateFipsState(void)
{
	/* Not Applicable for Versal_2vp */
	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
 * @brief	This function provides the Iro Frequency used in ROM.
 *
 * @return
 * 		- XPLMI_PMC_IRO_FREQ_320_MHZ
 *
 **************************************************************************************************/
static inline u32 XPlmi_GetRomIroFreq(void)
{
	return XPLMI_PMC_IRO_FREQ_320_MHZ;
}

/**************************************************************************************************/
/**
 * @brief	This is not applicable for Versal_2vp.
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 **************************************************************************************************/
static inline int XPlm_PostPlmUpdate(void)
{
	/* Not Applicable for Versal_2vp */
	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
 * @brief	This is not applicable for Versal_2vp.
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 **************************************************************************************************/
static inline int XPlmi_SsitCfgSecComm(XPlmi_Cmd *Cmd)
{
	(void)Cmd;
	/* Not Applicable for Versal_2vp */
	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
 * @brief	This is not applicable for Versal_2vp.
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 **************************************************************************************************/
static inline int XPlmi_GetSsitSecCommStatus(XPlmi_Cmd *Cmd)
{
	(void)Cmd;
	/* Not Applicable for Versal_2vp */
	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
 * @brief	This is not applicable for Versal_2vp.
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 **************************************************************************************************/
static inline int XPlmi_SsitSyncMaster(XPlmi_Cmd *Cmd)
{
	(void)Cmd;
	/* Not Applicable for Versal_2vp */
	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
 * @brief	This is not applicable for Versal_2vp.
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 **************************************************************************************************/
static inline int XPlmi_SsitSyncSlaves(XPlmi_Cmd *Cmd)
{
	(void)Cmd;
	/* Not Applicable for Versal_2vp */
	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
 * @brief	This is not applicable for Versal_2vp.
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 **************************************************************************************************/
static inline int XPlmi_SsitWaitSlaves(XPlmi_Cmd *Cmd)
{
	(void)Cmd;
	/* Not Applicable for Versal_2vp */
	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
 * @brief	This is not applicable for Versal_2vp.
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 **************************************************************************************************/
static inline int XPlmi_SendIpiCmdToSlaveSlr(u32 * Payload, u32 * RespBuf)
{
	(void)Payload;
	(void)RespBuf;
	/* Not Applicable for Versal_2vp */
	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
 * @brief	This is not applicable for Versal_2vp.
 *
 * @return
 * 		- None.
 *
 **************************************************************************************************/
static inline void XPlmi_InterSlrSldHandshake(void)
{
	/* Not Applicable for Versal_2vp */
	return;
}

/**************************************************************************************************/
/**
 * @brief	This is not applicable for Versal_2vp.
 *
 * @return
 * 		- None.
 *
 **************************************************************************************************/
static inline void XPlmi_NotifySldSlaveSlrs(void)
{
	/* Not Applicable for Versal_2vp */
	return;
}

/**************************************************************************************************/
/**
 * @brief	This is not applicable for Versal_2vp.
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 **************************************************************************************************/
static inline u8 XPlmi_GetSlrIndex(void)
{
	/* Not Applicable for Versal_2vp */
	return (u8)XST_SUCCESS;
}

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
/**************************************************************************************************/
/**
 * @brief	This is not applicable for Versal_2vp.
 *
 * @return
 * 		- None.
 *
 **************************************************************************************************/
static inline void XPlmi_DetectSlaveSlrTamper(void)
{
	(void)PmcErr1Status;
	/* Not Applicable for Versal_2vp */
	return;
}

/**************************************************************************************************/
/**
 * @brief	This is not applicable for Versal_2vp.
 *
 * @return
 * 		- None.
 *
 **************************************************************************************************/
static inline void XPlmi_RegisterSsitErrHandlers(u32 Id)
{
	(void)Id;
	/* Not Applicable for Versal_2vp */
	return;
}

/**************************************************************************************************/
/**
 * @brief	This is not applicable for Versal_2vp.
 *
 * @return
 * 		- None.
 *
 **************************************************************************************************/
static inline void XPlmi_HandleSsitErr2(u32 ErrorNodeId, u32 RegMask)
{
	(void)ErrorNodeId;
	(void)RegMask;
	/* Not Applicable for Versal_2vp */
	return;
}

/**************************************************************************************************/
/**
 * @brief	This is not applicable for Versal_2vp.
 *
 * @return
 * 		- None.
 *
 **************************************************************************************************/
static inline void XPlmi_EnableSsitErrors(void)
{
	/* Not Applicable for Versal_2vp */
	return;
}

/**************************************************************************************************/
/**
 * @brief	This is not applicable for Versal_2vp.
 *
 * @return
 * 		- None.
 *
 **************************************************************************************************/
static inline void XPlmi_CheckSlaveErrors(void)
{
	/* Not Applicable for Versal_2vp */
	return;
}
#endif

/*****************************************************************************/
/**
 * @brief	This function sets ACK's the IPI request
 *
 * @param	PlmErr value of Plm error
 * @param	RomErr value of ROM error
 *
 * @return	None
 *
 *****************************************************************************/
static inline __attribute__((always_inline)) void XPlmi_AckIpi(u32 PlmErr, u32 RomErr)
{
	if ((XPlmi_In32(XPLMI_RTCFG_INPLACE_UPDATE_IPI_RESP_BUFF) == XPLMI_INVALID_RESP_BUFF_ADDR) ||
		(XPlmi_In32(XPlmi_In32(XPLMI_RTCFG_INPLACE_UPDATE_IPI_MASK) == XPLMI_INVALID_IPI_MASK)))
	{
		return;
	}
	XPlmi_Out32(XPlmi_In32(XPLMI_RTCFG_INPLACE_UPDATE_IPI_RESP_BUFF), PlmErr);
	XPlmi_Out32(XPlmi_In32(XPLMI_RTCFG_INPLACE_UPDATE_IPI_RESP_BUFF) + XPLMI_REG_OFFSET_BYTE_4, RomErr);
	XPlmi_Out32(XPLMI_IPI_PMC_ISR_ADDR, XPlmi_In32(XPLMI_RTCFG_INPLACE_UPDATE_IPI_MASK));
}

/*****************************************************************************/
/**
 * @brief	This function waits for specified timeout before doing IPOR
 *
 *
 * @return	None
 *
 *****************************************************************************/
static inline __attribute__((always_inline)) void XPlmi_IpuErrIporWait(void)
{
	volatile u32 TimeOut = XPlmi_In32(XPLMI_RTCFG_INPLACE_UPDATE_ERR_IPOR_TIMEOUT);
	volatile u32 TimeMsMultiplier = XPLMI_MILLI_SEC_TIME_MULTIPLIER; /* factor for ~1msec for 320MHz to 400MHz range */
	u32 WdtCntrlStatusReg = XPlmi_In32(XPLMI_PMC_GWDT_CNTRL_STATUS_REG);

	WdtCntrlStatusReg &=(~(u32)XPLMI_PMC_WDT_GWCSR_GWEN_MASK);
	XPlmi_Out32(XPLMI_PMC_GWDT_CNTRL_STATUS_REG, WdtCntrlStatusReg);
	/* Timeout multiple of milli sec */
	while ((TimeOut--) > 0U) {
		while(TimeMsMultiplier-- > 0U);
		TimeMsMultiplier = XPLMI_MILLI_SEC_TIME_MULTIPLIER;
	}
}



/************************************ Function Prototypes *****************************************/
u32 *XPlmi_GetLpdInitialized(void);
void XPlmi_RtcaPlatInit(void);
void XPlmi_PrintRomVersion(void);
int XPlmi_PreInit(void);
XPlmi_WaitForDmaDone_t XPlmi_GetPlmiWaitForDone(u64 DestAddr);
XPlmi_CircularBuffer *XPlmi_GetTraceLogInst(void);
void XPlmi_GetReadbackSrcDest(u32 SlrType, u64 *SrcAddr, u64 *DestAddrRead);
void XPlmi_GicAddTask(u32 PlmIntrId);
XInterruptHandler *XPlmi_GetTopLevelIntrTbl(void);
u8 XPlmi_GetTopLevelIntrTblSize(void);
int XPlmi_RegisterNEnableIpi(void);
void XPlmi_EnableIomoduleIntr(void);
int XPlmi_SetPmcIroFreq(void);
int XPlmi_GetPitResetValues(u32 *Pit1ResetValue, u32 *Pit2ResetValue);
u32 XPlmi_GetGicIntrId(u32 GicPVal, u32 GicPxVal);
u32 XPlmi_GetIpiIntrId(u32 BufferIndex);
void XPlmi_EnableIpiIntr(void);
void XPlmi_ClearIpiIntr(void);
void XPlmi_DisableCFrameIso(void);
u32 *XPlmi_GetUartBaseAddr(void);
void XPlmi_GetBootKatStatus(volatile u32 *PlmKatStatus);
void XPlmi_SetXRamAvailable(void);
void XPlmi_SssMask(u32 DmaSrc);
XIOModule *XPlmi_GetIOModuleInst(void);
u8 XPlmi_IsKatRan(u32 PlmKatMask);
u8 XPlmi_IsPlmUpdateDone(void);
u8 XPlmi_IsPlmUpdateDoneTmp(void);
u8 XPlmi_IsPlmUpdateInProgress(void);

/************************************ Variable Definitions ****************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_PLAT_H */
