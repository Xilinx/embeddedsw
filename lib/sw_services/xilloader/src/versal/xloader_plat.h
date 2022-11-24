/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal/xloader_plat.h
* @addtogroup xloader_plat_apis XilLoader Versal specific APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       bm   07/13/2022 Retain critical data structures after In-Place PLM Update
* 1.01  ng   11/11/2022 Fixed doxygen file name error
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

#ifndef XLOADER_PLAT_H
#define XLOADER_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xpm_node.h"
#include "xloader.h"
#include "xloader_auth_enc.h"

/************************** Constant Definitions *****************************/
#define XLOADER_SECURE_CHUNK_SIZE	(0x8000U) /* 32K */

/* Boot Modes */
#define XLOADER_PDI_SRC_JTAG		(0x0U)
#define XLOADER_PDI_SRC_QSPI24		(0x1U)
#define XLOADER_PDI_SRC_QSPI32		(0x2U)
#define XLOADER_PDI_SRC_SD0		(0x3U)
#define XLOADER_PDI_SRC_EMMC0		(0x4U)
#define XLOADER_PDI_SRC_SD1		(0x5U)
#define XLOADER_PDI_SRC_EMMC1		(0x6U)
#define XLOADER_PDI_SRC_USB		(0x7U)
#define XLOADER_PDI_SRC_OSPI		(0x8U)
#define XLOADER_PDI_SRC_SBI		(0x9U)
#define XLOADER_PDI_SRC_SMAP		(0xAU)
#define XLOADER_PDI_SRC_PCIE		(0xBU)
#define XLOADER_PDI_SRC_SD1_LS		(0xEU)
#define XLOADER_PDI_SRC_DDR		(0xFU)
#define XLOADER_PDI_SRC_INVALID		(0xFFU)

#define XLOADER_R5_0_TCMA_BASE_ADDR	(0xFFE00000U)
#define XLOADER_R5_1_TCMA_BASE_ADDR	(0xFFE90000U)

/*
 * TCM address for R5
 */
#define XLOADER_R5_TCMA_LOAD_ADDRESS	(0x0U)
#define XLOADER_R5_TCMB_LOAD_ADDRESS	(0x20000U)
#define XLOADER_R5_TCM_BANK_LENGTH	(0x10000U)
#define XLOADER_R5_TCM_TOTAL_LENGTH	(XLOADER_R5_TCM_BANK_LENGTH * 4U)
#define XLOADER_R5_0_TCM_A_BASE_ADDR	(0xFFE00000U)
#define XLOADER_R5_0_TCM_A_END_ADDR	(0xFFE0FFFFU)
#define XLOADER_R5_0_TCM_B_BASE_ADDR	(0xFFE20000U)
#define XLOADER_R5_0_TCM_B_END_ADDR	(0xFFE2FFFFU)
#define XLOADER_R5_LS_TCM_END_ADDR	(0xFFE3FFFFU)
#define XLOADER_R5_1_TCM_A_BASE_ADDR	(0xFFE90000U)
#define XLOADER_R5_1_TCM_A_END_ADDR	(0xFFE9FFFFU)
#define XLOADER_R5_1_TCM_B_BASE_ADDR	(0xFFEB0000U)
#define XLOADER_R5_1_TCM_B_END_ADDR	(0xFFEBFFFFU)

/*
 * APU related macros
 */
#define XLOADER_FPD_APU_CONFIG_0	(0xFD5C0020U)
#define XLOADER_FPD_APU_CONFIG_0_AA64N32_MASK_CPU0	(0x1U)
#define XLOADER_FPD_APU_CONFIG_0_AA64N32_MASK_CPU1	(0x2U)
#define XLOADER_FPD_APU_CONFIG_0_VINITHI_MASK_CPU0	(0x100U)
#define XLOADER_FPD_APU_CONFIG_0_VINITHI_MASK_CPU1	(0x200U)

/* Data measurement flags applicable only for VersalNet */
#define XLOADER_MEASURE_START		(0U)
#define XLOADER_MEASURE_UPDATE		(1U)
#define XLOADER_MEASURE_FINISH		(2U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_GET_PDISRC_INFO()	{\
		{"SBI", XLOADER_SBI_INDEX}, /* SBI JTAG - 0 */\
		{"QSPI24", XLOADER_QSPI_INDEX}, /* QSPI24 - 1 */\
		{"QSPI32", XLOADER_QSPI_INDEX}, /* QSPI32 - 2 */\
		{"SD0", XLOADER_SD_INDEX}, /* SD0 - 3 */\
		{"EMMC0", XLOADER_SD_INDEX}, /* EMMC0 - 4 */\
		{"SD1", XLOADER_SD_INDEX}, /* SD1 - 5 */\
		{"EMMC1", XLOADER_SD_INDEX}, /* EMMC - 6 */\
		{"USB", XLOADER_USB_INDEX}, /* USB - 7 */\
		{"OSPI", XLOADER_OSPI_INDEX}, /* OSPI - 8 */\
		{"SBI", XLOADER_SBI_INDEX}, /* SBI - 9*/\
		{"SMAP", XLOADER_SBI_INDEX}, /* SMAP - 0xA */\
		{"PCIE", XLOADER_SBI_INDEX}, /* PCIE - 0xB */\
		{"", XLOADER_INVALID_INDEX}, /* Unused - 0xC */\
		{"", XLOADER_INVALID_INDEX}, /* Unused - 0xD */\
		{"SD1_LS", XLOADER_SD_INDEX}, /* SD1_LS - 0xE */\
		{"DDR", XLOADER_DDR_INDEX}, /* DDR - 0xF */\
	}

/*****************************************************************************/
/**
 * @brief	This function is used to check if the given NodeId is
 *		 applicable for DFx
 *
 * @param	NodeId is Image ID
 *
 * @return 	TRUE if applicable, else FALSE
 *
 *****************************************************************************/
static inline u8 XLoader_IsDFxApplicable(u32 NodeId)
{
	return ((NodeId == (u32)XPM_NODESUBCL_DEV_PL) ||
		(NodeId == (u32)XPM_NODESUBCL_DEV_AIE)) ? (u8)TRUE : (u8)FALSE;
}

/*****************************************************************************/
/**
 * @brief	This function is used to check if the given PdiSrc is SD
 *
 * @param	PdiSrc is the source of Pdi
 *
 * @return 	TRUE if SD, else FALSE
 *
 *****************************************************************************/
static inline u8 XLoader_IsPdiSrcSD(u32 PdiSrc)
{
	return ((PdiSrc == XLOADER_PDI_SRC_SD0) || (PdiSrc == XLOADER_PDI_SRC_SD1)
		|| (PdiSrc == XLOADER_PDI_SRC_SD1_LS)) ? (u8)TRUE : (u8)FALSE;
}

/*****************************************************************************/
/**
 * @brief	This function is used to check if the given PdiSrc is SD0
 *
 * @param	PdiSrc is the source of Pdi
 *
 * @return 	TRUE if SD0, else FALSE
 *
 *****************************************************************************/
static inline u8 XLoader_IsPdiSrcSD0(u8 PdiSrc)
{
	return (PdiSrc == XLOADER_PDI_SRC_SD0) ? (u8)TRUE : (u8)FALSE;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the loader with platform specific
 * 		initializations.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static inline int XLoader_PlatInit(void)
{
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function measures the PDI's meta header data by calculating
 *		the hash using SHA3.
 *
 * @param	PdiPtr is the pointer to PDI instance
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static inline int XLoader_HdrMeasurement(XilPdi* PdiPtr)
{
	(void)PdiPtr;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function measures the data by calculating SHA3 hash.
 *
 * @param	DataAddr is the address of the data to be measured.
 * @param	DataSize is the size of the data to be measured.
 * @param	PcrInfo provides the PCR number to be extended.
 * @param	Flags - The hash calcualtion flags
 * 			- XLOADER_MEASURE_START : Sha3 start
 * 			- XLOADER_MEASURE_UPDATE: Sha3 update
 * 			- XLOADER_MEASURE_FINISH: Sha3Finish
 * 			- Any other option will be an error.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static inline int XLoader_DataMeasurement(u64 DataAddr, u32 DataSize,
				u32 PcrInfo, u8 Flags)
{
	(void)DataAddr;
	(void)DataSize;
	(void)PcrInfo;
	(void)Flags;
	return XST_SUCCESS;
}

/************************** Function Prototypes ******************************/
XLoader_ImageInfoTbl *XLoader_GetImageInfoTbl(void);
void XLoader_SetJtagTapToReset(void);
int XLoader_GetSDPdiSrcNAddr(u32 SecBootMode, XilPdi *PdiPtr, u32 *PdiSrc,
		u32 *PdiAddr);
int XLoader_ProcessElf(XilPdi* PdiPtr, const XilPdi_PrtnHdr * PrtnHdr,
	XLoader_PrtnParams* PrtnParams, XLoader_SecureParams* SecureParams);
int XLoader_UpdateHandoffParam(XilPdi* PdiPtr);
u8 XLoader_SkipMJtagWorkAround(XilPdi *PdiPtr);
int XLoader_ProcessDeferredError(void);
int XLoader_StartImage(XilPdi *PdiPtr);
XilPdi *XLoader_GetPdiInstance(void);
void XLoader_PerformInternalPOR(void);
XLoader_ImageStore* XLoader_GetPdiList(void);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_PLAT_H */
