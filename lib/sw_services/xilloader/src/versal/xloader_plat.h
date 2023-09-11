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
*       dd   03/28/2023 Updated doxygen comments
*       sk   04/28/2023 Added define for Image Store as PDI source
*       sk   05/31/2023 Added Declaration for Get BootPDI info function
*       sk   06/12/2023 Removed XLoader_GetPdiInstance function declaration
*       sk   07/07/2023 Added define for Config Jtag State
*                       Moved minor error codes to plat headers
*       sk   07/31/2023 Added error code for Image Store feature
*       dd   08/11/2023 Updated doxygen comments
*       dd   09/11/2023 MISRA-C violation Directive 4.5 fixed
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
#define XLOADER_PDI_SRC_IS		(0x10U) /**< PDI source Image Store */
#define XLOADER_PDI_SRC_DDR		(0xFU) /**< PDI source DDR */
#define XLOADER_PDI_SRC_INVALID		(0xFFU) /**< PDI source invalid */

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

#define XLoader_ConfigureJtagState	NULL /**< Configure JTAG State */
#define XLoader_MbPmcI2cHandshake (NULL) /**< DDRMB - PMC I2C Handshake */
/* Minor Error Codes */
#define XLOADER_ERR_INVALID_IMGID		(0x2U) /**< Invalid ImgID passed in Command */
#define XLOADER_ERR_NO_VALID_IMG_FOUND		(0x3U) /**< No Valid Image Found in the Image Info Table */
#define XLOADER_ERR_IMAGE_INFO_TBL_FULL		(0x4U) /**< Image Info Table is Full */
#define XLOADER_ERR_PARENT_QUERY_RELATION_CHECK	(0x5U) /**< Error on Parent Query while checking for Child Relation */
#define XLOADER_ERR_MEMSET_BOOT_HDR_FW_RSVD	(0x6U) /**< Error during memset on XilPdi_BootHdrFwRsvd */
#define XLOADER_ERR_MEMSET_PDIPTR		(0x7U) /**< Error during memset on PdiPtr */
#define XLOADER_ERR_MEMSET_QSPI_PSU_INST	(0x8U) /**< Error during memset on QspiPsuInstance */
#define XLOADER_ERR_MEMSET_SD_BOOT_FILE		(0x9U) /**< Error during memset on SD BootFile */
#define XLOADER_ERR_MEMSET_SD_INSTANCE		(0xAU) /**< Error during memset on SdInstance */
#define XLOADER_ERR_MEMSET_USB_INSTANCE		(0xBU) /**< Error during memset on UsbInstance */
#define XLOADER_ERR_MEMSET_USB_PRIVATE_DATA	(0xCU) /**< Error during memset on UsbPrivateData */
#define XLOADER_ERR_MEMSET_DFU_OBJ		(0xDU) /**< Error during memset on DfuObj */
#define XLOADER_ERR_INVALID_METAHDR_BUFF_SIZE	(0xEU) /**< Error when buffer size given by user
							is less than the metaheader length */
#define XLOADER_ERR_INVALID_PDI_INPUT	(0xFU) /**< Error when PDI given is not a full PDI
							or partial PDI */
#define XLOADER_ERR_INVALID_DEST_IMGINFOTBL_SIZE	(0x10U) /**< Error when the destination
							buffer provided to store image info
							table is less than the current length
							of image info table */
#define XLOADER_ERR_INVALID_METAHEADER_SRC_ADDR		(0x11U) /**< Error when invalid source address
							is passed as a input to extract metaheader
							command */
#define XLOADER_ERR_INVALID_METAHEADER_DEST_ADDR	(0x12U) /**< Error when invalid destination address
							is passed as a input to extract metaheader
							command */
#define XLOADER_ERR_INVALID_METAHEADER_OFFSET	(0x13U) /**< Error when the metaheader offset provided
							in full PDI is not present in DDR */
/* Minor Error codes for Major Error code: XLOADER_ERR_GEN_IDCODE */
#define XLOADER_ERR_IDCODE		(0x14U) /**< IDCODE mismatch */
#define XLOADER_ERR_EXT_IDCODE		(0x15U) /**< EXTENDED IDCODE mismatch */
#define XLOADER_ERR_EXT_ID_SI		(0x16U) /**< Invalid combination of
						* EXTENDED IDCODE - Device
						*/
#define XLOADER_ERR_PDI_LIST_EMPTY		(0x17U) /**< Error when PdiList is empty*/
#define XLOADER_ERR_PDI_ADDR_NOT_FOUND		(0x18U) /**< Error when the PdiAddr that is being tried
							to remove does not exist in the PdiList */
#define XLOADER_ERR_PDI_IMG_STORE_CFG_NOT_SET	(0x19U) /**< Image Store configuration is not enabled/error */
#define XLOADER_ERR_PDI_IMG_STORE_FULL		(0x1AU) /**< Error when PdiList is full and user
							is trying to add a new Pdi */

/* Platform specific Minor Error Codes start from 0x100 */

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
 * @param	DigestIndex Digest index in PCR log, applicable to SW PCR only
 * @param       OverWrite TRUE or FALSE to overwrite the extended digest or not
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static inline int XLoader_SecureConfigMeasurement(XLoader_SecureParams* SecurePtr,
	u32 PcrInfo, u32 *DigestIndex, u32 OverWrite)
{
	(void)SecurePtr;
	(void)PcrInfo;
	(void)DigestIndex;
	(void)OverWrite;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function reads DDR crypto performance counters of given device id
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS on success.
 *          - XST_FAILURE on Failure.
 *
 *****************************************************************************/

static inline int XLoader_ReadDdrCryptoPerfCounters(XPlmi_Cmd *Cmd)
{
	(void)Cmd;
	/* Not Applicable for Versal */
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
void XLoader_PerformInternalPOR(void);
XLoader_ImageStore* XLoader_GetPdiList(void);
int Xloader_SsitEoPdiSync(XilPdi *PdiPtr);
XilBootPdiInfo* XLoader_GetBootPdiInfo(void);


/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_PLAT_H */
