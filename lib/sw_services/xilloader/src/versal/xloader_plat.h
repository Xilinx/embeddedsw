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
*       sk   02/22/2023 Added EoPDI SYNC function declaration
*		dd   03/28/2023 Updated doxygen comments
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
#define XLOADER_SECURE_CHUNK_SIZE	(0x8000U) /**< Secure chunk size 32K */

/* Boot Modes */
#define XLOADER_PDI_SRC_JTAG		(0x0U) /**< PDI source JTAG */
#define XLOADER_PDI_SRC_QSPI24		(0x1U) /**< PDI source QSPI24 */
#define XLOADER_PDI_SRC_QSPI32		(0x2U) /**< PDI source QSPI32 */
#define XLOADER_PDI_SRC_SD0		(0x3U) /**< PDI source SD0 */
#define XLOADER_PDI_SRC_EMMC0		(0x4U) /**< PDI source EMMC0 */
#define XLOADER_PDI_SRC_SD1		(0x5U) /**< PDI source SD1 */
#define XLOADER_PDI_SRC_EMMC1		(0x6U) /**< PDI source EMMC1 */
#define XLOADER_PDI_SRC_USB		(0x7U) /**< PDI source USB */
#define XLOADER_PDI_SRC_OSPI		(0x8U) /**< PDI source OSPI */
#define XLOADER_PDI_SRC_SBI		(0x9U) /**< PDI source SBI */
#define XLOADER_PDI_SRC_SMAP		(0xAU) /**< PDI source SMAP */
#define XLOADER_PDI_SRC_PCIE		(0xBU) /**< PDI source PCIE */
#define XLOADER_PDI_SRC_SD1_LS		(0xEU) /**< PDI source SD1-LS */
#define XLOADER_PDI_SRC_DDR		(0xFU) /**< PDI source DDR */
#define XLOADER_PDI_SRC_INVALID		(0xFFU) /**< PDI source invalid */

#define XLOADER_R5_0_TCMA_BASE_ADDR	(0xFFE00000U) /**< R5_0 TCMA base address */
#define XLOADER_R5_1_TCMA_BASE_ADDR	(0xFFE90000U) /**< R5_1 TCMA base address */

/*
 * TCM address for R5
 */
#define XLOADER_R5_TCMA_LOAD_ADDRESS	(0x0U) /**< R5 TCMA load address */
#define XLOADER_R5_TCMB_LOAD_ADDRESS	(0x20000U) /**< R5 TCMB load address */
#define XLOADER_R5_TCM_BANK_LENGTH	(0x10000U) /**< R5 TCM bank length */
#define XLOADER_R5_TCM_TOTAL_LENGTH	(XLOADER_R5_TCM_BANK_LENGTH * 4U) /**< R5 TCM total length */
#define XLOADER_R5_0_TCM_A_BASE_ADDR	(0xFFE00000U) /**< R5_0 TCM_A base address */
#define XLOADER_R5_0_TCM_A_END_ADDR	(0xFFE0FFFFU) /**< R5_0 TCM_A end address */
#define XLOADER_R5_0_TCM_B_BASE_ADDR	(0xFFE20000U) /**< R5_0 TCM_B base address */
#define XLOADER_R5_0_TCM_B_END_ADDR	(0xFFE2FFFFU) /**< R5_0 TCM_B end address */
#define XLOADER_R5_LS_TCM_END_ADDR	(0xFFE3FFFFU) /**< R5_LS TCM end address */
#define XLOADER_R5_1_TCM_A_BASE_ADDR	(0xFFE90000U) /**< R5_1 TCM_A base address */
#define XLOADER_R5_1_TCM_A_END_ADDR	(0xFFE9FFFFU) /**< R5_1 TCM_A end address */
#define XLOADER_R5_1_TCM_B_BASE_ADDR	(0xFFEB0000U) /**< R5_1 TCM_B base address */
#define XLOADER_R5_1_TCM_B_END_ADDR	(0xFFEBFFFFU) /**< R5_1 TCM_B end address */

/*
 * APU related macros
 */
#define XLOADER_FPD_APU_CONFIG_0	(0xFD5C0020U) /**< FPD APU configuration 0 */
#define XLOADER_FPD_APU_CONFIG_0_AA64N32_MASK_CPU0	(0x1U) /**< Aarch32 or Aarch64 CPU0 mask */
#define XLOADER_FPD_APU_CONFIG_0_AA64N32_MASK_CPU1	(0x2U) /**< Aarch32 or Aarch64 CPU1 mask */
#define XLOADER_FPD_APU_CONFIG_0_VINITHI_MASK_CPU0	(0x100U) /**< VINITHI CPU0 mask */
#define XLOADER_FPD_APU_CONFIG_0_VINITHI_MASK_CPU1	(0x200U) /**< VINITHI CPU1 mask */

/* Data measurement flags applicable only for VersalNet */
#define XLOADER_MEASURE_START		(0U) /**< Data measure start */
#define XLOADER_MEASURE_UPDATE		(1U) /**< Data measure update */
#define XLOADER_MEASURE_FINISH		(2U) /**< Data measure finish */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_GET_PDISRC_INFO()	{ \
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
	} /**< PDI source info */

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
 * @param	ImageInfo Pointer to the XLoader_ImageMeasureInfo structure
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static inline int XLoader_DataMeasurement(XLoader_ImageMeasureInfo *ImageInfo)
{
	(void)ImageInfo;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function measures the Secure Config by calculating SHA3 hash.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance.
 * @param	PcrInfo provides the PCR number and Measurement Index
 * 		to be extended.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static inline int XLoader_SecureConfigMeasurement(XLoader_SecureParams* SecurePtr, u32 PcrInfo)
{
	(void)SecurePtr;
	(void)PcrInfo;
	return XST_SUCCESS;
}

/************************** Function Prototypes ******************************/
XLoader_ImageInfoTbl *XLoader_GetImageInfoTbl(void);
void XLoader_SetJtagTapToReset(void);
int XLoader_GetSDPdiSrcNAddr(u32 SecBootMode, XilPdi *PdiPtr, u32 *PdiSrc,
		u64 *PdiAddr);
int XLoader_ProcessElf(XilPdi* PdiPtr, const XilPdi_PrtnHdr * PrtnHdr,
	XLoader_PrtnParams* PrtnParams, XLoader_SecureParams* SecureParams);
int XLoader_UpdateHandoffParam(XilPdi* PdiPtr);
u8 XLoader_SkipMJtagWorkAround(XilPdi *PdiPtr);
int XLoader_ProcessDeferredError(void);
int XLoader_StartImage(XilPdi *PdiPtr);
XilPdi *XLoader_GetPdiInstance(void);
void XLoader_PerformInternalPOR(void);
XLoader_ImageStore* XLoader_GetPdiList(void);
int Xloader_SsitEoPdiSync(XilPdi *PdiPtr);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_PLAT_H */
