/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xbir_qspimap.h
*
* This is the System Board QSPI image memory map definition.
*
******************************************************************************/
#ifndef XBIR_QSPI_MAP_H
#define XBIR_QSPI_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
/* Boot image component sizes */
#define XBIR_QSPI_MAX_IMGSEL_SIZE			(512U * 1024U)		/* 512KB */
#define XBIR_QSPI_MAX_BOOT_IMG_INFO_SIZE	(128U * 1024U)		/* 128KB */
#define XBIR_QSPI_MAX_BOOT_IMG_SIZE		(13U * 1024U * 1024U)	/* 13MB */
#define XBIR_QSPI_MAX_RECVRY_IMG_SIZE		(2U * 1024U * 1024U)	/* 2MB */
#define XBIR_QSPI_MM_FREE_AREA_1_SIZE		(768U * 1024U)		/* 768KB */
#define XBIR_QSPI_MM_FREE_AREA_2_SIZE		(512U * 1024U)		/* 512KB */
#define XBIR_QSPI_MM_FREE_AREA_3_SIZE		(1024U * 1024U)		/* 1MB */

/* Boot image component start addresses */
#define	XBIR_QSPI_MM_IMGSEL_ADDR		(0x00000000U)
#define	XBIR_QSPI_MM_IMGSEL_RECVRY_ADDR 		\
	(XBIR_QSPI_MM_IMGSEL_ADDR + XBIR_QSPI_MAX_IMGSEL_SIZE) /* 0x80000 */
#define	XBIR_QSPI_MM_BOOT_IMG_INFO_ADDR 	\
	(XBIR_QSPI_MM_IMGSEL_RECVRY_ADDR + XBIR_QSPI_MAX_IMGSEL_SIZE) /* 0x100000 */
#define	XBIR_QSPI_MM_BOOT_IMG_INFO_BKP_ADDR 	\
	(XBIR_QSPI_MM_BOOT_IMG_INFO_ADDR + XBIR_QSPI_MAX_BOOT_IMG_INFO_SIZE) /* 0x120000 */
#define	XBIR_QSPI_MM_FREE_AREA_1_ADDR 		\
	(XBIR_QSPI_MM_BOOT_IMG_INFO_BKP_ADDR + XBIR_QSPI_MAX_BOOT_IMG_INFO_SIZE) /* 0x140000 */
#define	XBIR_QSPI_MM_IMG_A_ADDR 		\
	(XBIR_QSPI_MM_FREE_AREA_1_ADDR + XBIR_QSPI_MM_FREE_AREA_1_SIZE) /* 0x200000 */
#define	XBIR_QSPI_MM_IMG_A_CATCH_IMGSEL_ADDR 	\
	(XBIR_QSPI_MM_IMG_A_ADDR + XBIR_QSPI_MAX_BOOT_IMG_SIZE) /* 0xF00000 */
#define	XBIR_QSPI_MM_FREE_AREA_2_ADDR 		\
	(XBIR_QSPI_MM_IMG_A_CATCH_IMGSEL_ADDR + XBIR_QSPI_MAX_IMGSEL_SIZE) /* 0xF80000 */
#define	XBIR_QSPI_MM_IMG_B_ADDR 		\
	(XBIR_QSPI_MM_FREE_AREA_2_ADDR + XBIR_QSPI_MM_FREE_AREA_2_SIZE) /* 0x1000000 */
#define	XBIR_QSPI_MM_IMG_B_CATCH_IMGSEL_ADDR 	\
	(XBIR_QSPI_MM_IMG_B_ADDR + XBIR_QSPI_MAX_BOOT_IMG_SIZE) /* 0x1D00000 */
#define	XBIR_QSPI_MM_FREE_AREA_3_ADDR 		\
	(XBIR_QSPI_MM_IMG_B_CATCH_IMGSEL_ADDR + XBIR_QSPI_MAX_IMGSEL_SIZE) /* 0x1D80000 */
#define XBIR_QSPI_MM_RECOVERY_IMG_ADDR 		\
	(XBIR_QSPI_MM_FREE_AREA_3_ADDR + XBIR_QSPI_MM_FREE_AREA_3_SIZE) /* 0x1E80000 */
#define XBIR_QSPI_MM_RECOVERY_IMG_BKP_ADDR 	\
	(XBIR_QSPI_MM_RECOVERY_IMG_ADDR + XBIR_QSPI_MAX_RECVRY_IMG_SIZE) /* 0x2080000 */
#define XBIR_QSPI_MM_FREE_USR_AREA_ADDR 	\
	(XBIR_QSPI_MM_RECOVERY_IMG_BKP_ADDR + XBIR_QSPI_MAX_RECVRY_IMG_SIZE) /* 0x2280000 */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif  /* XBIR_QSPI_MAP_H */
