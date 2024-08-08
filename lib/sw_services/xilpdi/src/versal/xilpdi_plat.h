/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file versal/xilpdi_plat.h
*
* This is the header file which contains versal specific definitions
* for the PDI.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       bm   09/13/2022 Reduce maximum number of partitions and images
* 1.01  har  11/17/2022 Added macros for bh_auth attribute in Bootheader
*       ng   11/23/2022 Fixed doxygen file name error
*       sk   01/11/2023 Added macro for Image Store as SBD
* 1.02  kpt  12/04/2023 Added XilPdi_BootHdr
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XILPDI_PLAT_H
#define XILPDI_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
/**
 * @name XilPdi Definitions
 *
 * @{
 */

/**
 * Boot header Attr fields
 */
#define XIH_BH_IMG_ATTRB_BH_AUTH_MASK	(0xC000U)
#define XIH_BH_IMG_ATTRB_BH_AUTH_SHIFT	(14U)
#define XIH_BH_IMG_ATTRB_BH_AUTH_VALUE	(0x3U)

#define XIH_MAX_PRTNS			(20U)
#define XIH_MAX_IMGS			(10U)

/**
 * Secondary Boot Device (SBD) in IHT Attributes
 */
#define XIH_IHT_ATTR_SBD_MASK	        (0xFC0U)
#define XIH_IHT_ATTR_SBD_SHIFT			(0x6U)
#define XIH_IHT_ATTR_SBD_SAME	        (0x0U)
#define XIH_IHT_ATTR_SBD_QSPI32	        (0x1U)
#define XIH_IHT_ATTR_SBD_QSPI24	        (0x2U)
#define XIH_IHT_ATTR_SBD_SD_0			(0x4U)
#define XIH_IHT_ATTR_SBD_SD_1			(0x5U)
#define XIH_IHT_ATTR_SBD_SD_LS          (0x6U)
#define XIH_IHT_ATTR_SBD_EMMC	        (0x7U)
#define XIH_IHT_ATTR_SBD_USB			(0x8U)
#define XIH_IHT_ATTR_SBD_PCIE	        (0xAU)
#define XIH_IHT_ATTR_SBD_OSPI			(0xCU)
#define XIH_IHT_ATTR_SBD_SMAP			(0xDU)
#define XIH_IHT_ATTR_SBD_SD_0_RAW		(0xFU)
#define XIH_IHT_ATTR_SBD_SD_1_RAW		(0x10U)
#define XIH_IHT_ATTR_SBD_SD_LS_RAW		(0x11U)
#define XIH_IHT_ATTR_SBD_EMMC_RAW		(0x12U)
#define XIH_IHT_ATTR_SBD_EMMC_0			(0x13U)
#define XIH_IHT_ATTR_SBD_EMMC_0_RAW		(0x14U)
#define XIH_IHT_ATTR_IMAGE_STORE		(0x15U)

/**
 *  Prtn Attribute fields
 */
#define XIH_PH_ATTRB_HIVEC_MASK			(0x800000U)
#define XIH_PH_ATTRB_ENDIAN_MASK		(0x40000U)
#define XIH_PH_ATTRB_A72_EXEC_ST_MASK	(0x0008U)

/**
 *  Prtn Attribute Values
 */
#define XIH_PH_ATTRB_PRTN_TYPE_RSVD		(0x0000000U)
#define XIH_PH_ATTRB_PRTN_TYPE_ELF		(0x1000000U)
#define XIH_PH_ATTRB_PRTN_TYPE_CDO		(0x2000000U)
#define XIH_PH_ATTRB_PRTN_TYPE_CFI_GSC_UNMASK	(0x7000000U)

#define XIH_PH_ATTRB_PRTN_OWNER_PLM				(0x00000U)
#define XIH_PH_ATTRB_HASH_SHA3					(0x3000U)

#define XIH_PH_ATTRB_DSTN_CPU_NONE				(0x0000U)
#define XIH_PH_ATTRB_DSTN_CPU_A72_0				(0x100U)
#define XIH_PH_ATTRB_DSTN_CPU_A72_1				(0x200U)
#define XIH_PH_ATTRB_DSTN_CPU_R5_0				(0x500U)
#define XIH_PH_ATTRB_DSTN_CPU_R5_1				(0x600U)
#define XIH_PH_ATTRB_DSTN_CPU_R5_L				(0x700U)
#define XIH_PH_ATTRB_DSTN_CPU_PSM				(0x800U)

#define XIH_PH_ATTRB_A72_EXEC_ST_AA64			(0x0000U)

/**
 * Below is the bit mapping of fields in the ATF Handoff parameters
 * with that of Prtn header. The number of bits shifted is
 * is based on the difference between these two offsets
 *
 *                   ATFHandoffParams	PrtnHdr         Shift
 *     Parameter     PrtnFlags     PrtnAttrb   difference
 * ----------------------------------------------------------------------
 *	Exec State            0			         3                  3 right
 *	ENDIAN	              1	                 18                 17 right
 *	SECURE                2                  0                  2 left
 *	EL                    3:4                1:2                2 left
 *	CPU_A72               5:6                8:10
 */
#define XIH_ATTRB_A72_EXEC_ST_SHIFT_DIFF    (3U)
#define XIH_ATTRB_ENDIAN_SHIFT_DIFF         (17U)
#define XIH_ATTRB_TR_SECURE_SHIFT_DIFF      (2U)
#define XIH_ATTRB_TARGET_EL_SHIFT_DIFF      (2U)

#define XIH_ATTRB_EL_MASK			(0x18U)
#define XIH_PRTN_FLAGS_EL_2			(0x10U)
#define XIH_PRTN_FLAGS_EL_3			(0x18U)
#define XIH_PRTN_FLAGS_DSTN_CPU_A72_0		(0x00U)
#define XIH_PRTN_FLAGS_DSTN_CPU_A72_1		(0x20U)

/**
 * Offset to the metaheader offset field present in boot header
 */
#define XIH_BH_META_HDR_OFFSET		(0xC4U)

/**
 * Boot header address in PRAM copied by ROM
 */
#define XIH_BH_PRAM_ADDR			(0xF201E000U)

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
 * Structure to store the partition header details.
 * It contains all the information of partition header in order.
 */
typedef struct {
	u32 EncDataWordLen; /**< Enc word length of partition*/
	u32 UnEncDataWordLen; /**< Unencrypted word length */
	u32 TotalDataWordLen; /**< Total word length including the authentication
							certificate if any*/
	u32 NextPrtnOfst; /**< Addr of the next partition header*/
	u64 DstnExecutionAddr; /**< Execution address */
	u64 DstnLoadAddr; /**< Load address in DDR/TCM */
	u32 DataWordOfst; /**< Data word offset */
	u32 PrtnAttrb; /**< Partition attributes */
	u32 SectionCount; /**< Section count */
	u32 ChecksumWordOfst; /**< Address to checksum when enabled */
	u32 PrtnId; /**< Partition ID */
	u32 AuthCertificateOfst; /**< Address to the authentication certificate
							when enabled */
	u32 PrtnIv[3U]; /**< IV of the partition's SH */
	u32 EncStatus; /**< Encryption Status/Key Selection */
	u32 KekIv[3U]; /**< KEK IV for partition decryption */
	u32 EncRevokeID; /**< Revocation ID of partition for encrypted partition */
	u32 Reserved[9U]; /**< Reserved */
	u32 Checksum; /**< checksum of the partition header */
} XilPdi_PrtnHdr __attribute__ ((aligned(16U)));

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
* @brief	This function will return the value of A72 Execution State field.
*
* @param	PrtnHdr is pointer to the Partition Header
*
* @return	A72 Execution State
*
*****************************************************************************/
static inline u32 XilPdi_GetA72ExecState(const XilPdi_PrtnHdr *PrtnHdr)
{
	return (PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_A72_EXEC_ST_MASK);
}

/************************** Function Prototypes ******************************/

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* XILPDI_PLAT_H */
