/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal_net/xloader_plat.h
* @addtogroup xloader_apis XilLoader versal_net specific APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- --------   -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       bm   07/13/2022 Retain critical data structures after In-Place PLM Update
*       bm   07/18/2022 Shutdown modules gracefully during update
*       dc   07/20/2022 Added support for data measurement.
*       har  08/29/2022 Updated secure chunk size from 16K to 32K
* 1.01  ng   11/11/2022 Fixed doxygen file name error
*       kal  01/05/2023 Added XLoader_SecureConfigMeasurement function
*       sk   02/22/2023 Added EoPDI SYNC dummy stub
*       dd   03/28/2023 Updated doxygen comments
*       sk   04/28/2023 Added define for Image Store as PDI source
*       sk   05/31/2023 Reassign PDI inst DS id for BootPDI info DS
*       sk   06/12/2023 Removed XLoader_GetPdiInstance function declaration
*       sk   07/07/2023 Added function declaration for Config Jtag State
*                       Moved minor error codes to plat headers
*                       Added error codes for Invalid JTAG Config Request
*       kpt  07/10/2023 Added IPI support to read DDR crypto status
*       sk   07/10/2023 Removed TCM Address, Offset defines
*       sk   07/31/2023 Added error code for Image Store feature
*	ro   08/01/2023 Added error codes for DDR initialization
*
* </pre>
*
* @note
*
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
#define XLOADER_PDI_SRC_SDLS_B0		(0x3U) /**< PDI source SDLS_B0 */
#define XLOADER_PDI_SRC_EMMC0		(0x4U) /**< PDI source EMMC0 */
#define XLOADER_PDI_SRC_SD_B1		(0x5U) /**< PDI source SD_B1 */
#define XLOADER_PDI_SRC_EMMC1		(0x6U) /**< PDI source EMMC1 */
#define XLOADER_PDI_SRC_USB		(0x7U) /**< PDI source USB */
#define XLOADER_PDI_SRC_OSPI		(0x8U) /**< PDI source OSPI */
#define XLOADER_PDI_SRC_SBI		(0x9U) /**< PDI source SBI */
#define XLOADER_PDI_SRC_SMAP		(0xAU) /**< PDI source SMAP */
#define XLOADER_PDI_SRC_PCIE		(0xBU) /**< PDI source PCIE */
#define XLOADER_PDI_SRC_SDLS_B1		(0xEU) /**< PDI source SDLS_B1 */
#define XLOADER_PDI_SRC_DDR		(0xFU) /**< PDI source DDR */
#define XLOADER_PDI_SRC_IS		(0x10U) /**< PDI source Image Store */
#define XLOADER_PDI_SRC_INVALID		(0xFFU)  /**< PDI source invalid */

#define XLOADER_APU_CLUSTER0	(0U) /**< APU cluster 0 */
#define XLOADER_APU_CLUSTER1	(1U) /**< APU cluster 1 */
#define XLOADER_APU_CLUSTER2	(2U) /**< APU cluster 2 */
#define XLOADER_APU_CLUSTER3	(3U) /**< APU cluster 3 */

#define XLOADER_APU_CORE0	(0U) /**< APU core 0 */
#define XLOADER_APU_CORE1	(1U) /**< APU core 1 */
#define XLOADER_APU_CORE2	(2U) /**< APU core 2 */
#define XLOADER_APU_CORE3	(3U) /**< APU core 3 */

#define XLOADER_RPU_CLUSTERA	(0U) /**< RPU cluster A */
#define XLOADER_RPU_CLUSTERB	(1U) /**< RPU cluster B */

#define XLOADER_RPU_CORE0	(0U) /**< RPU core 0 */
#define XLOADER_RPU_CORE1	(1U) /**< RPU core 1 */

/* Xilloader Module Data Structure Ids*/
#define XLOADER_IMAGE_INFO_DS_ID		(0x01U) /**< Image information data structure Id */
#define XLOADER_BOOTPDI_INFO_DS_ID       	(0x02U) /**< Boot PDI Info data structure Id */
#define XLOADER_PDI_LIST_DS_ID			(0x03U) /**< PDI list data structure Id */
#define XLOADER_ATF_HANDOFF_PARAMS_DS_ID	(0x04U) /**< ATF handoff parameters data structure Id */
#define XLOADER_IMAGE_INFO_PTR_DS_ID		(0x05U) /**< Image information data structure Id */

#define XLOADER_SHA3_1_DEVICE_ID	(1U) /**< SHA3_1 device Id */
/* Data measurement flags */
#define XLOADER_MEASURE_START		(0U) /**< Data measure start */
#define XLOADER_MEASURE_UPDATE		(1U) /**< Data measure update */
#define XLOADER_MEASURE_FINISH		(2U) /**< Data measure finish */

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
#define XLOADER_ERR_MAX_BASE_ADDR	(0x14U) /**< Error when Max ddr base addr */
#define XLOADER_ERR_HS_TIMEOUT		(0x17U) /**< Handshake process timeout */
#define XLOADER_ERR_I2C_TRANSACTION	(0x18U) /**< I2c transaction error */
#define XLOADER_ERR_I2C_INIT		(0x19U) /**< I2c initialization error */
#define XLOADER_ERR_I2C_BUS_BUSY	(0x20U) /**< I2c bus busy error */

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

#define XLOADER_ERR_INVALID_JTAG_OPERATION	(0x100U) /**< Invalid JTAG/DAP config request */
#define XLOADER_ERR_DDR_DEVICE_ID           (0x101U) /**< Invalid DDR device id */
#define XLOADER_ERR_PCOMPLETE_NOT_DONE      (0x102U) /**< Pcomplete not done for given DDR device id */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_GET_PDISRC_INFO()	{\
		{"SBI", XLOADER_SBI_INDEX}, /* SBI JTAG - 0 */\
		{"QSPI24", XLOADER_QSPI_INDEX}, /* QSPI24 - 1 */\
		{"QSPI32", XLOADER_QSPI_INDEX}, /* QSPI32 - 2 */\
		{"SDLS_B0", XLOADER_SD_INDEX}, /* SDLS_B0 - 3 */\
		{"EMMC0", XLOADER_SD_INDEX}, /* EMMC0 - 4 */\
		{"SD_B1", XLOADER_SD_INDEX}, /* SD_B1 - 5 */\
		{"EMMC1", XLOADER_SD_INDEX}, /* EMMC - 6 */\
		{"USB", XLOADER_USB_INDEX}, /* USB - 7 */\
		{"OSPI", XLOADER_OSPI_INDEX}, /* OSPI - 8 */\
		{"SBI", XLOADER_SBI_INDEX}, /* SBI - 9*/\
		{"SMAP", XLOADER_SBI_INDEX}, /* SMAP - 0xA */\
		{"PCIE", XLOADER_SBI_INDEX}, /* PCIE - 0xB */\
		{"", XLOADER_INVALID_INDEX}, /* Unused - 0xC */\
		{"", XLOADER_INVALID_INDEX}, /* Unused - 0xD */\
		{"SDLS_B1", XLOADER_SD_INDEX}, /* SDLS_B1 - 0xE */\
		{"DDR", XLOADER_DDR_INDEX}, /* DDR - 0xF */\
	}/**< Get PDI source info */

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
	return (NodeId == (u32)XPM_NODESUBCL_DEV_PL) ? (u8)TRUE : (u8)FALSE;
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
	return ((PdiSrc == XLOADER_PDI_SRC_SDLS_B0) || (PdiSrc == XLOADER_PDI_SRC_SD_B1)
		|| (PdiSrc == XLOADER_PDI_SRC_SDLS_B1)) ? (u8)TRUE : (u8)FALSE;
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
	(void)PdiSrc;
	/* Not Applicable for Versal Net */
	return XLoader_IsPdiSrcSD((u32)PdiSrc);
}

/*****************************************************************************/
/**
 * @brief	This function checks if MJTAG workaround partition needs to be
 *              skipped
 *
 * @param	PdiPtr is pointer to PDI instance

 * @return	FALSE
 *
 *****************************************************************************/
static inline u8 XLoader_SkipMJtagWorkAround(XilPdi *PdiPtr)
{
	(void)PdiPtr;
	/* Not Applicable for Versal Net */
	return (u8)FALSE;
}

/*****************************************************************************/
/**
 * @brief	This function check conditions and perform internal POR
 * 		for VP1802 and VP1502 device if required.
 *
 * @return	None.
 *
 *****************************************************************************/
static inline void XLoader_PerformInternalPOR(void)
{
	/* Not applicable for Versal Net */
	return;
}

/*****************************************************************************/
/**
 * @brief	This function is used to run MJTAG solution workaround in which
 * JTAG Tap state will be set to reset.
 *
 *
 * @return	None
 *
 *****************************************************************************/
static inline void XLoader_SetJtagTapToReset(void)
{
	/* Not applicable for Versal Net */
	return;
}

/*****************************************************************************/
/**
 * @brief	This function will sync the PDI load status with master
 *
 * @param	PdiPtr is pointer to PDI instance
 *
 * @return
 * 		-XST_SUCCESS return success
 *
 *****************************************************************************/
static inline int Xloader_SsitEoPdiSync(XilPdi *PdiPtr)
{
	/* Not Applicable for VersalNet */
	(void)PdiPtr;

	return XST_SUCCESS;
}
/************************** Function Prototypes ******************************/
XLoader_ImageInfoTbl *XLoader_GetImageInfoTbl(void);
int XLoader_StartImage(XilPdi *PdiPtr);
int XLoader_GetSDPdiSrcNAddr(u32 SecBootMode, XilPdi *PdiPtr, u32 *PdiSrc,
		u64 *PdiAddr);
int XLoader_UpdateHandoffParam(XilPdi* PdiPtr);
int XLoader_ProcessDeferredError(void);
int XLoader_ProcessElf(XilPdi* PdiPtr, const XilPdi_PrtnHdr * PrtnHdr,
	XLoader_PrtnParams* PrtnParams, XLoader_SecureParams* SecureParams);
XLoader_ImageStore* XLoader_GetPdiList(void);
int XLoader_UpdateHandler(XPlmi_ModuleOp Op);
int XLoader_PlatInit(void);
int XLoader_HdrMeasurement(XilPdi* PdiPtr);
int XLoader_DataMeasurement(XLoader_ImageMeasureInfo *ImageInfo);
int XLoader_SecureConfigMeasurement(XLoader_SecureParams* SecurePtr, u32 PcrInfo, u32 *DigestIndex, u32 OverWrite);
XilBootPdiInfo* XLoader_GetBootPdiInfo(void);
int XLoader_ConfigureJtagState(XPlmi_Cmd *Cmd);
int XLoader_ReadDdrCryptoPerfCounters(XPlmi_Cmd *Cmd);
int XLoader_MbPmcI2cHandshake(XPlmi_Cmd *Cmd);
/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_PLAT_H */
