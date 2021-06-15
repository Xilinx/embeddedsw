/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xfsbl_main.h
*
* This is the main header file which contains definitions for the FSBL.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   10/21/13 Initial release
* 2.0   vb   03/24/17 Added macros for LOVEC/HIVEC and USB boot mode,
*                     Made compliance to MISRAC 2012 guidelines
* 3.00  bsv  04/28/21 Added support to ensure authenticated images boot as
*                     non-secure when RSA_EN is not programmed
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XFSBL_MAIN_H
#define XFSBL_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xfsbl_image_header.h"
#include "xfsbl_misc_drivers.h"
#include "xfsbl_hw.h"
#include "xplatform_info.h"
#include "xtime_l.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**
 * This stores the handoff Address of the different cpu's
 */
typedef struct {
	u32 CpuSettings;
	u64 HandoffAddress;
} XFsblPs_HandoffValues;

#if defined XFSBL_PERF
/**
 * This stores the timer values for measuring FSBL execution time.
 */
typedef struct {
	XTime  tFsblStart;
} XFsblPs_Perf;
#endif /* XFSBL_PERF */

/**
 * This is FSBL instance pointer. This stores all the information
 * required for FSBL
 */
typedef struct {
	u32 Version; /**< FSBL Version */
	u32 PresentStage; /**< Stage */
	u32 ProcessorID; /**< One of R5-0, R5-LS, A53-0 */
	u32 A53ExecState; /**< One of A53 64-bit, A53 32-bit */
	u32 BootHdrAttributes; /**< Boot Header attributes */
	u32 AuthEnabled;  /**< Check if RSA_EN is programmed or
				Boot Header authentication is enabled */
	u32 ImageOffsetAddress; /**< Flash offset address */
	XFsblPs_ImageHeader ImageHeader; /** Image header */
	u32 ErrorCode; /**< Error code during FSBL failure */
	u32 PrimaryBootDevice; /**< Primary boot device used for booting  */
	u32 SecondaryBootDevice; /**< Secondary boot device in image header*/
	XFsblPs_DeviceOps DeviceOps; /**< Device operations for bootmodes */
	u32 HandoffCpuNo; /**< Number of CPU's FSBL will handoff to */
	u32 ResetReason; /**< Reset reason */
	u32 TcmEccInitStatus; /**< Bits 0, 1 indicate TCM ECC Init status */
	XFsblPs_HandoffValues HandoffValues[10];
		/**< Handoff address for different CPU's  */
#if defined XFSBL_PERF
	XFsblPs_Perf PerfTime;
#endif
} XFsblPs;


/***************** Macros (Inline Functions) Definitions *********************/

/* SDK release version */
#define SDK_RELEASE_YEAR	2021
#define SDK_RELEASE_QUARTER	2

#define XFSBL_RUNNING			(0xFFFFU)
#define XFSBL_COMPLETED			(0x0U)
#define XFSBL_IMAGE_SEARCH_OFFSET	(0x8000U) /**< 32KB offset */

/**
 * Fsbl exit definitions
 */
#define XFSBL_NO_HANDOFFEXIT	(0x0U)
#define XFSBL_HANDOFFEXIT	(0x1U)
#define XFSBL_HANDOFFEXIT_32	(0x2U)

/**
 * Boot Modes definition
 */
#define XFSBL_JTAG_BOOT_MODE		(0x0U)
#define XFSBL_QSPI24_BOOT_MODE		(0x1U)
#define XFSBL_QSPI32_BOOT_MODE		(0x2U)
#define XFSBL_SD0_BOOT_MODE	        (0x3U)
#define XFSBL_NAND_BOOT_MODE		(0x4U)
#define XFSBL_SD1_BOOT_MODE	        (0x5U)
#define XFSBL_EMMC_BOOT_MODE		(0x6U)
#define XFSBL_USB_BOOT_MODE			(0x7U)
#define XFSBL_SD1_LS_BOOT_MODE	    (0xEU)

/**
 * FSBL stages definition
 */
#define XFSBL_STAGE1		(0x1U)
#define XFSBL_STAGE2		(0x2U)
#define XFSBL_STAGE3		(0x3U)
#define XFSBL_STAGE4		(0x4U)
#define XFSBL_STAGE_ERR		(0x5U)
#define XFSBL_STAGE_DEFAULT	(0x6U)

/* A53 MMU definitions */

#define BLOCK_SIZE_A53_64 0x200000U
#define BLOCK_SIZE_A53_32 0x100000U

#define NUM_BLOCKS_A53_64 0x400U
#define NUM_BLOCKS_A53_32 0x800U

#define BLOCK_SIZE_A53_64_HIGH 0x40000000U
#define NUM_BLOCKS_A53_64_HIGH (((XFSBL_PS_HI_DDR_END_ADDRESS - \
		XFSBL_PS_HI_DDR_START_ADDRESS) + 1U) / BLOCK_SIZE_A53_64_HIGH)

#define ATTRIB_MEMORY_A53_64 0x705U
#define ATTRIB_MEMORY_A53_32 0x15DE6U
#define ATTRIB_RESERVED_A53  0x0U

/* Pattern to be filled for DDR ECC Initialization */
#define XFSBL_ECC_INIT_VAL_WORD 0xDEADBEEFU

#define XFSBL_R50_TCM_ECC_INIT_STATUS 0x00000001U
#define XFSBL_R51_TCM_ECC_INIT_STATUS 0x00000002U

/* R5 vectors value*/
#define XFSBL_R5_LOVEC_VALUE 	0xEAFEFFFEU
#define XFSBL_R5_HIVEC_VALUE    0xEAFF3FFEU

/*
 * FSBL processor reporting to PMU
 */
#define XFSBL_RUNNING_ON_A53			(0x1U)
#define XFSBL_RUNNING_ON_R5_0			(0x2U)
#define XFSBL_RUNNING_ON_R5_L			(0x3U)

#define XFSBL_STATE_PROC_SHIFT			(0x1U)

#define XFSBL_STATE_PROC_INFO_MASK		(0x3U << XFSBL_STATE_PROC_SHIFT)
#define XFSBL_FSBL_ENCRYPTED_MASK		(0x8U)


/************************** Function Prototypes ******************************/
/**
 * Functions defined in xfsbl_main.c
 */
void XFsbl_PrintFsblBanner(void );
void XFsbl_ErrorLockDown(u32 ErrorStatus);

#if defined(XFSBL_PERF)
void XFsbl_MeasurePerfTime(XTime tCur);
#endif

/**
 * Functions defined in xfsbl_initialization.c
 */
u32 XFsbl_Initialize(XFsblPs * FsblInstancePtr);
u32 XFsbl_BootDeviceInitAndValidate(XFsblPs * FsblInstancePtr);
u32 XFsbl_TcmEccInit(XFsblPs * FsblInstancePtr, u32 CpuId);
void XFsbl_MarkDdrAsReserved(u8 Cond);

/**
 * Functions defined in xfsbl_partition_load.c
 */
u32 XFsbl_PartitionLoad(XFsblPs * FsblInstancePtr, u32 PartitionNum);
u32 XFsbl_PowerUpMemory(u32 MemoryType);
/**
 * Functions defined in xfsbl_handoff.c
 */
u32 XFsbl_Handoff (const XFsblPs * FsblInstancePtr, u32 PartitionNum, u32 EarlyHandoff);
void XFsbl_HandoffExit(u64 HandoffAddress, u32 Flags);
u32 XFsbl_CheckEarlyHandoff(XFsblPs * FsblInstancePtr, u32 PartitionNum);
/************************** Variable Definitions *****************************/


#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_MAIN_H */
