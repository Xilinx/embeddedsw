/******************************************************************************
* Copyright (c) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xilpdi.h
*
* This is the header file which contains definitions for the PDI.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   12/21/2017 Initial release
* 1.01  bsv  04/08/2019 Added support for secondary boot device parameters
*       bsv  07/30/2019 Renamed XilPdi_ReadAndValidateImgHdrTbl to
							XilPdi_ReadImgHdrTbl
*       rm   08/28/2019 Added APIs for retrieving delay load and delay handoff
*						params
* 1.02  bsv  11/29/2019 Added support for smap bus width word in partial pdis
*       vnsl 02/26/2020 Added support to read DPA CM Enable field in meta headers
*       vnsl 03/01/2020 Added support to read PufHeader from Meta Headers and
*						partition headers
*       vnsl 04/12/2020 Added support to read BootHdr Auth Enable field in
*						boot header
* 1.03  skd  07/14/2020 Function pointer DeviceCopy prototype changed
* 1.04  bsv  07/29/2020 Added UID, parent ID, function ID and copy to memory
*                       address
*       kpt  07/30/2020 Added maximum limit for number of images
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       bsv  10/13/2020 Code clean up
*       kpt  10/19/2020 Added support to validate checksum of image headers and
*                       partition headers
* 1.05  td   11/23/2020 Coverity Warning Fixes
*       ma   01/08/2021 Changed maximum number of entries possible for ATF
*       har  02/01/2021 Added API to get PLM encryption key source
*       bm   02/12/2021 Updated logic to use BootHdr directly from PMC RAM
*       har  03/17/2021 Removed XilPdi_IsBhdrAuthEnable
*       ma   03/24/2021 Redirect XilPdi prints to XilLoader
*       ma   03/24/2021 Change ImgName to u8 to print max characters of
*                       PDI image names
*       har  03/31/2021 Added PdiId in XilPdi_ImgHdrTbl structure
* 1.06  td   07/08/2021 Fix doxygen warnings
*       bsv  08/16/2021 Code clean up
*       bm   08/24/2021 Added Extract Metaheader support
*       kpt  09/18/2021 Fixed SW-BP-REDUNDANCY in
*                       XilPdi_IsDpaCmEnable
* 1.07  kpt  02/01/2022 Updated XilPdi_ReadBootHdr prototype
* 1.08  bsv  07/06/2022 Added API to read OptionaData from Metaheader
*       bm   07/06/2022 Refactor versal and versal_net code
*       bsv  07/08/2022 Code changes related to Optional data in IHT
*       bm   07/13/2022 Added compatibility check for In-Place PLM Update
*       bm   09/13/2022 Reduce maximum number of partitions and images
* 1.09  har  11/17/2022 Removed macros for bh_auth attribute in Bootheader
*       ng   11/23/2022 Added macros to replace magic numbers in
*                       XilPdi_ValidateChecksum
*       kal  01/05/2023 Added PcrInfo attribute in XilPdi_ImgHdr
*       sk   02/22/2023 Added Bit MASK for EoPDI SYNC logic
*	dd   03/16/2023 Misra-C violation Rule 17.8 fixed
*       sk   05/18/2023 Deprecate copy to memory feature
*       am   07/03/2023 Added macros related to IHT OP data
*       dd   08/11/2023 Updated doxygen comments
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XILPDI_H
#define XILPDI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xstatus.h"
#include "xil_printf.h"
#include "xil_io.h"
#include "xilpdi_plat.h"

/************************** Constant Definitions *****************************/
/**
 * @name XilPdi Definitions
 *
 * @{
 */
#define XIH_MIN_PRTNS			(1U)
#define XIH_MIN_IMGS			(1U)

/**
 * Boot header identification string
 */
#define XIH_BH_IMAGE_IDENT		(0x584C4E58U) /**< XLNX pattern */
#define XIH_BH_IMAGE_IDENT_OFFSET	(0x14U)

/**
 * Offset to the metaheader offset field present in boot header
 */
#define XIH_BH_META_HDR_OFFSET		(0xC4U)

/**
 * Boot header address in PRAM copied by ROM
 */
#define XIH_BH_PRAM_ADDR		(0xF201E000U)

/**
 * Boot header Key source field
 */
#define XIH_BH_AES_KEYSRC_OFFSET	(0x08U)
#define XIH_BH_IMG_ATTRB_OFFSET		(0x24U)

/**
 * Boot header PUF fields
 */
#define XIH_BH_PUF_HD_OFFSET		(0x918U)
#define XIH_BH_PUF_CHASH_OFFSET		(0xF18U)
#define XIH_BH_PUF_AUX_OFFSET		(0xF1CU)

/**
 * SMAP bus width macros
 */
#define SMAP_BUS_WIDTH_LENGTH		(16U)
#define SMAP_BUS_WIDTH_WORD_LEN		(4U)
#define SMAP_BUS_WIDTH_8_WORD1				(0xDD000000U)
#define SMAP_BUS_WIDTH_16_WORD1				(0x00DD0000U)
#define SMAP_BUS_WIDTH_32_WORD1				(0x000000DDU)

/**
 *  Defines for length of the headers
 */
#define XIH_IHT_LEN			(128U)
#define XIH_IH_LEN			(64U)
#define XIH_PH_LEN			(128U)
#define XIH_PRTN_WORD_LEN		(0x4U)
#define XIH_PRTN_WORD_LEN_SHIFT		(0x2U)

/**
 * IHT attributes
 */
#define XIH_IHT_ATTR_PUFHD_MASK		(0xC000U)
#define XIH_IHT_ATTR_PUFHD_SHIFT		(14U)
#define XIH_IHT_ATTR_DPA_CM_MASK		(0x3000U)
#define XIH_IHT_ATTR_DPA_CM_SHIFT		(12U)
#define XIH_IHT_ATTR_BYPS_MASK				(0x1U) /**< IDCODE checks bypass */
#define XIH_IHT_ATTR_BYPS_ID_CODE_MASK  (0x30000U) /**< Check [17:16] bits to bypass ID Code*/
#define XIH_IHT_ATTR_EOPDI_SYNC_SHIFT		(18U)
#define XIH_IHT_ATTR_EOPDI_SYNC_MASK		XPLMI_BIT(XIH_IHT_ATTR_EOPDI_SYNC_SHIFT)
#define XIH_IHT_EXT_IDCODE_MASK			(0x3FU)

/*
 * IHT Identification string
 */
#define XIH_IHT_VERSION_OFFSET			(0x0U)
#define XIH_IHT_IDENT_STRING_OFFSET		(0x28U)
#define XIH_IHT_PPDI_IDENT_VAL			(0x50504449U)
#define XIH_IHT_FPDI_IDENT_VAL			(0x46504449U)

/**
 *  Prtn Attribute fields
 */
#define XIH_PH_ATTRB_DPA_CM_EN_MASK		(0x18000000U)
#define XIH_PH_ATTRB_DPA_CM_EN_SHIFT		(27U)
#define XIH_PH_ATTRB_PRTN_TYPE_MASK		(0x7000000U)
#define XIH_PH_ATTRB_PRTN_OWNER_MASK	(0x30000U)
#define XIH_PH_ATTRB_PUFHD_MASK			(0xC000U)
#define XIH_PH_ATTRB_CHECKSUM_MASK		(0x3000U)
#define XIH_PH_ATTRB_DSTN_CPU_MASK		(0x0F00U)
#define XIH_PH_ATTRB_TARGET_EL_MASK		(0x0006U)
#define XIH_PH_ATTRB_TZ_SECURE_MASK		(0x0001U)
#define XIH_PH_ATTRB_PUFHD_SHIFT		(14U)

/**
 *  Prtn Attribute Values
 */
#define XIH_PH_ATTRB_PRTN_TYPE_RSVD		(0x0000000U)
#define XIH_PH_ATTRB_PRTN_TYPE_ELF		(0x1000000U)
#define XIH_PH_ATTRB_PRTN_TYPE_CDO		(0x2000000U)
#define XIH_PH_ATTRB_PRTN_TYPE_CFI_GSC_UNMASK	(0x7000000U)

#define XIH_PH_ATTRB_PRTN_OWNER_PLM				(0x00000U)
#define XIH_PH_ATTRB_HASH_SHA3					(0x3000U)

/**
 * Number of entries possible in ATF:
 * 				2 cores * 3 (EL2 non-secure, EL1 secure and EL1 non-secure)
 */
#define XILPDI_MAX_ENTRIES_FOR_ATF	(6U)

/**
 * Errors during XilPdi processing
 */
#define XILPDI_ERR_IHT_CHECKSUM		(0x1)
#define XILPDI_ERR_NO_OF_PRTNS		(0x2)
#define XILPDI_ERR_ZERO_LENGTH		(0x4)
#define XILPDI_ERR_TOTAL_LENGTH		(0x5)
#define XILPDI_ERR_PRTN_TYPE		(0x6)
#define XILPDI_ERR_NO_OF_IMGS		(0x7)
#define XILPDI_ERR_IH_CHECKSUM		(0x40)
#define XILPDI_ERR_PH_CHECKSUM		(0x80)
#define XILPDI_ERR_OPTIONAL_DATA_CHECKSUM_FAILED	(0x70)
#define XILPDI_ERR_NO_VALID_OPTIONAL_DATA	(0x71)
#define XILPDI_ERR_INVALID_DIGEST_TABLE_SIZE	(0x72)

/**
 * Image Header Attributes
 */
#define XILPDI_IH_ATTRIB_DELAY_LOAD_SHIFT		(0x7U)
#define XILPDI_IH_ATTRIB_DELAY_LOAD_MASK		(0X00000080U)
#define XILPDI_IH_ATTRIB_DELAY_HANDOFF_SHIFT	(0x8U)
#define XILPDI_IH_ATTRIB_DELAY_HANDOFF_MASK		(0X00000100U)

/**
 * Array size for image name
 */
#define XILPDI_IMG_NAME_ARRAY_SIZE				(16U)

/**
 * Minimun buffer length for checksum
 */
#define XILPDI_CHECKSUM_MIN_BUF_LEN				(0X2U)

/**
 * Invert checksum
 */
#define XILPDI_INVERT_CHECKSUM					(0xFFFFFFFFU)

/**
 * Partition hash data Id
 */
#define XILPDI_PARTITION_HASH_DATA_ID	(3U)

/**
 * Common address of storing partition hashes for both versal and versal_net
 */
#define XIH_PMC_RAM_IHT_OP_DATA_STORE_ADDR	(0xF201D200U)

/**
 * Optional data attributes
 */
#define XILPDI_OPTIONAL_DATA_WORD_LEN	(4U)
#define XILPDI_OPTIONAL_DATA_DOUBLE_WORD_LEN	(2U * XILPDI_OPTIONAL_DATA_WORD_LEN)
#define XIH_OPT_DATA_HDR_ID_MASK	(0xFFFFU)
#define XIH_OPT_DATA_HDR_LEN_MASK	(0xFFFF0000U)
#define XIH_OPT_DATA_LEN_SHIFT	(16U)
#define XIPLDI_SHA3_HASH_SIZE_IN_BYTES (48U)

/**************************** Type Definitions *******************************/

/**
 * Structure to store the Boot Header PMC FW fields
 */
typedef struct {
	u32 MetaHdrOfst; /**< Offset to the start of meta header */
	u32 FwRsvd[24U]; /**< FW Reserved fields */
} XilPdi_BootHdrFwRsvd;

/**
 * Structure to store the boot header table details.
 * It contains all the information of boot header table in order.
 */
typedef struct {
	u32 WidthDetection; /**< Width Detection 0xAA995566 */
	u32 ImgIden;  /**< Image Identification */
	u32 EncStatus;  /**< Encryption Status */
	u32 DpiSrcOfst;  /**< Source Offset of PMC FW in DPI */
	u32 DpiStartOfst;  /**< PMC FW start offset in RAM */
	u32 DataPrtnLen;  /**< Data Partition Length */
	u32 TotalDataPrtnLen;  /**< Total Data Partition length */
	u32 PlmLen;  /**< PLM Length */
	u32 TotalPlmLen;  /**< Total PLM length */
	u32 ImgAttrb;  /**< Image Attributes */
	u32 Kek[8U];  /**< Encrypted Key */
	u32 KekIv[3U];  /**< Key Iv */
	u32 SecureHdrIv[3U];  /**< Secure Header IV */
	u32 PufShutterVal; /**< PUF Shutter Value */
	u32 RomRsvd[20U]; /**< ROM Reserved */
	XilPdi_BootHdrFwRsvd BootHdrFwRsvd; /**< FW reserved fields */
} XilPdi_BootHdr __attribute__ ((aligned(16U)));

/**
 * Structure to store the image header table details.
 * It contains all the information of image header table in order.
 */
typedef struct {
	u32 Version; /**< PDI version used  */
	u32 NoOfImgs; /**< No of images present  */
	u32 ImgHdrAddr; /**< Address to start of 1st Image header*/
	u32 NoOfPrtns; /**< No of partitions present  */
	u32 PrtnHdrAddr; /**< Address to start of 1st partition header*/
	u32 SBDAddr; /**< Secondary Boot device address */
	u32 Idcode; /**< Device ID Code */
	u32 Attr; /**< Attributes */
	u32 PdiId; /**< PDI ID */
	u32 Rsrvd[3U]; /**< Reserved for future use */
	u32 TotalHdrLen; /**< Total size of Meta header AC + encryption overload */
	u32 IvMetaHdr[3U]; /**< Iv for decrypting SH of meta header */
	u32 EncKeySrc; /**< Encryption key source for decrypting SH of headers */
	u32 ExtIdCode;  /**< Extended ID Code */
	u32 AcOffset; /**< AC offset of Meta header */
	u32 KekIv[3U]; /**< Kek IV for meta header decryption */
	u32 OptionalDataLen; /**< Len in words of OptionalData */
	u32 Rsvd[8U]; /**< Reserved */
	u32 Checksum; /**< Checksum of the image header table */
} XilPdi_ImgHdrTbl __attribute__ ((aligned(16U)));

/**
 * Structure to store the Image header details.
 * It contains all the information of Image header in order.
 */
typedef struct {
	u32 FirstPrtnHdr; /**< First partition header in the image */
	u32 NoOfPrtns; /**< Number of partitions in the image */
	u32 EncRevokeID; /**< Revocation ID of meta header */
	u32 ImgAttr; /**< Image Attributes */
	u8 ImgName[XILPDI_IMG_NAME_ARRAY_SIZE]; /**< Image Name */
	u32 ImgID; /**< Image ID */
	u32 UID; /**< Unique ID */
	u32 PUID; /**< Parent UID */
	u32 FuncID; /**< Function ID */
	u64 Reserved; /**< Reserved */
	u32 PcrInfo;/**< PCR information only applicable for Versal Net */
	u32 Checksum; /**< Checksum of the image header */
} XilPdi_ImgHdr __attribute__ ((aligned(16U)));

/**
 * Structure of the image header which contains
 * information of image header table and
 * partition headers.
 */
typedef struct {
	const XilPdi_BootHdr *BootHdrPtr; /**< Boot Header Pointer */
	XilPdi_ImgHdrTbl ImgHdrTbl; /**< Img header table structure */
	XilPdi_ImgHdr ImgHdr[XIH_MAX_IMGS]; /**< Image header */
	XilPdi_PrtnHdr PrtnHdr[XIH_MAX_PRTNS]; /**< Prtn header */
	u64 FlashOfstAddr; /**< Start of DPI start address in Flash */
	u32 MetaHdrOfst; /**< Offset to the start of meta header */
	int (*DeviceCopy) (u64 SrcAddr, u64 DestAddress, u32 Length,
			u32 Flags); /**< Function pointer for device copy */
	u32 DigestTableSize; /**< Digest table size in bytes */
} XilPdi_MetaHdr __attribute__ ((aligned(16U)));

/**
 * Structure corresponding to each partition entry
 */
typedef struct {
	u64 EntryPoint; /**< Entry point */
	u64 PrtnFlags; /**< Attributes of partition */
} XilPdi_PrtnEntry;

/**
 * Structure for handoff parameters to ARM Trusted Firmware (ATF)
 */
typedef struct {
	char MagicValue[4U]; /**< 32 bit magic string */
	u32 NumEntries; /**< Number of Entries */
	XilPdi_PrtnEntry Entry[XILPDI_MAX_ENTRIES_FOR_ATF]; /**< Structure
							corresponding to each entry */
} XilPdi_ATFHandoffParams __attribute__ ((aligned(16U)));

/**
 * Partition hash entry information
 */
typedef struct {
	u32 PrtnNum; /**< Partition Number */
	u8 PrtnHash[XIPLDI_SHA3_HASH_SIZE_IN_BYTES]; /**< Partition hash */
}XilPdi_PrtnHashInfo __attribute__ ((aligned(16U)));

/***************** Macros (Inline Functions) Definitions *********************/
#ifdef XILPDI_DEBUG
#define XilPdi_Printf(...)	xil_printf(__VA_ARGS__)
#else
#define XilPdi_Printf(...)
#endif

#define XILPDI_PMCRAM_IHT_COPY_ADDR	(0xF2004120U) /* IHT Optional data cannot exceed 16 KB */
#define XILPDI_PMCRAM_IHT_DATA_ADDR	(XILPDI_PMCRAM_IHT_COPY_ADDR + XIH_IHT_LEN)
#define XILPDI_WORD_LEN_SHIFT		(2U)

/****************************************************************************/
/**
* @brief	This function will return the value of Partition Owner field.
*
* @param	PrtnHdr is pointer to the Partition Header
*
* @return	Partition Owner
*
*****************************************************************************/
static inline u32 XilPdi_GetPrtnOwner(const XilPdi_PrtnHdr *PrtnHdr)
{
	return (PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_PRTN_OWNER_MASK);
}

/****************************************************************************/
/**
* @brief	This function will return the value of Checksum Type field.
*
* @param	PrtnHdr is pointer to the Partition Header
*
* @return	Checksum Type
*
*****************************************************************************/
static inline u32 XilPdi_GetChecksumType(const XilPdi_PrtnHdr *PrtnHdr)
{
	return (PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_CHECKSUM_MASK);
}

/****************************************************************************/
/**
* @brief	This function will return the value of Destination Cpu field.
*
* @param	PrtnHdr is pointer to the Partition Header
*
* @return	Destination Cpu
*
*****************************************************************************/
static inline u32 XilPdi_GetDstnCpu(const XilPdi_PrtnHdr *PrtnHdr)
{
	return (PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_DSTN_CPU_MASK);
}

/****************************************************************************/
/**
* @brief	This function will return the value of Partition Type field.
*
* @param	PrtnHdr is pointer to the Partition Header
*
* @return	Partition Type
*
*****************************************************************************/
static inline u32 XilPdi_GetPrtnType(const XilPdi_PrtnHdr *PrtnHdr)
{
	return (PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_PRTN_TYPE_MASK);
}

/****************************************************************************/
/**
* @brief	This function will return the value of HIVEC field.
*
* @param	PrtnHdr is pointer to the Partition Header
*
* @return	HIVEC value
*
*****************************************************************************/
static inline u32 XilPdi_GetVecLocation(const XilPdi_PrtnHdr *PrtnHdr)
{
	return (PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_HIVEC_MASK);
}

/****************************************************************************/
/**
* @brief	This function will return the value of Puf Header field.
*
* @param	PrtnHdr is pointer to the Partition Header
*
* @return	PUF header
*
*****************************************************************************/
static inline u32 XilPdi_GetPufHdPh(const XilPdi_PrtnHdr *PrtnHdr)
{
	return (PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_PUFHD_MASK);
}

/****************************************************************************/
/**
* @brief	This function checks if DpaCm is enabled or not.
*
* @param	PrtnHdr is pointer to the Partition Header
*
* @return	TRUE / FALSE
*
*****************************************************************************/
static inline u8 XilPdi_IsDpaCmEnable(const XilPdi_PrtnHdr *PrtnHdr)
{
	return ((u8)((PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_DPA_CM_EN_MASK) >>
		XIH_PH_ATTRB_DPA_CM_EN_SHIFT));
}

/****************************************************************************/
/**
* @brief	This function checks if DpaCm is enabled or not in Metaheader.
*
* @param	IHdrTbl is pointer to the Image Header Table
*
* @return	TRUE / FALSE
*
*****************************************************************************/
static inline u8 XilPdi_IsDpaCmEnableMetaHdr(const XilPdi_ImgHdrTbl *IHdrTbl)
{
	return ((u8)((IHdrTbl->Attr & XIH_IHT_ATTR_DPA_CM_MASK) >>
		XIH_IHT_ATTR_DPA_CM_SHIFT));
}

/****************************************************************************/
/**
* @brief	This function gets PUF header value from Image Header Table.
*
* @param	IHdrTbl is pointer to the Image Header Table
*
* @return	PUF Header Value
*
*****************************************************************************/
static inline u32 XilPdi_GetPufHdMetaHdr(const XilPdi_ImgHdrTbl *IHdrTbl)
{
	return (IHdrTbl->Attr & XIH_IHT_ATTR_PUFHD_MASK);
}
/****************************************************************************/
/**
* @brief	This function gets Delay Load value from Image Header Table.
*
* @param	ImgHdr is pointer to the Image Header Table
*
* @return	Delay Load Value
*
*****************************************************************************/
static inline u32 XilPdi_GetDelayLoad(const XilPdi_ImgHdr *ImgHdr)
{
	return (ImgHdr->ImgAttr & XILPDI_IH_ATTRIB_DELAY_LOAD_MASK);
}

/****************************************************************************/
/**
* @brief	This function gets Delay Handoff value from Image Header Table.
*
* @param	ImgHdr is pointer to the Image Header Table
*
* @return	Delay Handoff Value
*
*****************************************************************************/
static inline u32 XilPdi_GetDelayHandoff(const XilPdi_ImgHdr *ImgHdr)
{
	return (ImgHdr->ImgAttr & XILPDI_IH_ATTRIB_DELAY_HANDOFF_MASK);
}

/****************************************************************************/
/**
* @brief	This function will return the Secondary boot device.
*
* @param	ImgHdrTbl is pointer to the Image Header Table
*
* @return 	Secondary Boot device
*
*****************************************************************************/
static inline u32 XilPdi_GetSBD(const XilPdi_ImgHdrTbl *ImgHdrTbl)
{
	return (ImgHdrTbl->Attr & XIH_IHT_ATTR_SBD_MASK);
}

/*****************************************************************************/
/**
 * @brief	This function checks if authentication is enabled or not.
 *
 * @param	ImgHdrTblPtr is pointer to PDI Image Header Table
 *
 * @return	TRUE if authentication is enabled and false otherwise
 *
 *****************************************************************************/
static inline u8 XilPdi_IsAuthEnabled(const XilPdi_ImgHdrTbl *ImgHdrTblPtr)
{
	volatile u8 IsAuth = (u8)TRUE;
	volatile u8 IsAuthTemp = (u8)TRUE;
	IsAuth = (ImgHdrTblPtr->AcOffset != 0x0U) ? \
		(TRUE) : (FALSE);
	IsAuthTemp = IsAuth;
	return (IsAuth | IsAuthTemp);
}

/*****************************************************************************/
/**
 * @brief	This function checks if encryption is enabled or not.
 *
 * @param	ImgHdrTblPtr is pointer to PDI Image Header Table
 *
 * @return	TRUE if encryption is enabled and false otherwise
 *
 *****************************************************************************/
static inline u8 XilPdi_IsEncEnabled(const XilPdi_ImgHdrTbl *ImgHdrTblPtr)
{
	volatile u8 IsEnc = (u8)TRUE;
	volatile u8 IsEncTemp = (u8)TRUE;
	IsEnc = (ImgHdrTblPtr->EncKeySrc != 0x0U) ? \
		(TRUE) : (FALSE);
	IsEncTemp = IsEnc;
	return (IsEnc | IsEncTemp);
}

/*****************************************************************************/
/**
 * @brief	This function returns the PLM encryption key source in
 *		Bootheader.
 *
 * @return	Encryption Key source
 *
 *****************************************************************************/
static inline u32 XilPdi_GetPlmKeySrc(void)
{
	return Xil_In32(XIH_BH_PRAM_ADDR + XIH_BH_AES_KEYSRC_OFFSET);
}

/************************** Function Prototypes ******************************/
int XilPdi_ValidatePrtnHdr(const XilPdi_PrtnHdr *PrtnHdr);
int XilPdi_ValidateImgHdrTbl(const XilPdi_ImgHdrTbl *ImgHdrTbl);
void XilPdi_ReadBootHdr(const XilPdi_BootHdr **BootHdrPtr);
int XilPdi_ReadImgHdrTbl(XilPdi_MetaHdr *MetaHdrPtr);
int XilPdi_VerifyImgHdrs(const XilPdi_MetaHdr * MetaHdrPtr);
int XilPdi_VerifyPrtnHdrs(const XilPdi_MetaHdr * MetaHdrPtr);
int XilPdi_ReadImgHdrs(const XilPdi_MetaHdr * MetaHdrPtr);
int XilPdi_ReadPrtnHdrs(const XilPdi_MetaHdr * MetaHdrPtr);
int XilPdi_ReadIhtAndOptionalData(XilPdi_MetaHdr * MetaHdrPtr);
int XilPdi_ValidateChecksum(const void *Buffer, u32 Length);
XilPdi_PrtnHashInfo* XilPdi_IsPrtnHashPresent(u32 PrtnNum, u32 HashTableSize);
int XilPdi_StoreDigestTable(XilPdi_MetaHdr * MetaHdrPtr);

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* XILPDI_H */
