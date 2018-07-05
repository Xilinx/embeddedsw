/******************************************************************************
*
* Copyright (C) 2015 - 18 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xfsbl_image_header.h
*
* This is the image header file which contains definitions for the image header.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   10/21/13 Initial release
* 2.0   bv   12/05/16 Made compliance to MISRAC 2012 guidelines
*       vns  01/20/17 Added XIH_PH_ATTRB_VEC_LOCATION_MASK,
*                     XIH_PH_ATTRB_DEST_CPU_PMU
*                     XIH_PH_ATTRB_VEC_LOCATION_SHIFT masks and APIs
*                     XFsbl_GetVectorLocation(), XFsbl_GetBlockSize()
*                     Added Offsets of image header's Partition header,
*                     AC, and SPKID and PPK select masks.
*       bv   03/17/17 Modified XFsbl_ValidatePartitionHeader API to have
*                     parameter for ResetType
* 3.0   vns  01/03/18 In structure XFsblPs_PartitionHeader 8 bits from
*                     reserved bits are used for storing last 8 bits of IV
*                     IV from boot header should be added with thes 8 bits.
*       vns  03/07/18 Added BHDR attribute mask for PUF and macros for
*                     boot header size.
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XFSBL_IH_H
#define XFSBL_IH_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xfsbl_misc_drivers.h"

/************************** Constant Definitions *****************************/
#define XIH_MIN_PARTITIONS			(1U)
#define XIH_MAX_PARTITIONS			(32U)
#define XIH_RESERVED_OLD_IH			(0x380U)
#define XIH_PARTITION_WORD_LENGTH		(0x4U)

/**
 * Boot header field offset
 */
#define XIH_BH_IH_OFFSET		(0x3CU)
#define XIH_BH_TOTAL_PFW_LENGTH_OFFSET		(0x38U)
#define XIH_BH_IMAGE_ATTRB_OFFSET	(0x44U)
#define XIH_BH_IH_TABLE_OFFSET		(0x98U)
#define XIH_BH_PH_TABLE_OFFSET		(0x9CU)

#define XIH_BH_IMAGE_ATTRB_RSA_MASK	(0xC000U)
#define XIH_BH_IMAGE_ATTRB_PUF_BH_MASK	(0x00C0U)
#define XIH_BH_IMAGE_ATTRB_SHA2_MASK	(0x3000U)
#define XIH_BH_IV_OFFSET       		(0xA0U)
#define XIH_BH_IV_LENGTH   			(0x10U)

#define XIH_BH_MIN_SIZE	(0x000008B8U)
#define XIH_BH_MAX_SIZE	(XIH_BH_MIN_SIZE + \
					(0x00000182U * 4))
/**
 * Defines for length of the headers
 */
#define XIH_FIELD_LEN				(4U)
#define XIH_PFW_LEN_FIELD_LEN		(4U)
#define XIH_IHT_LEN				(64U)
#define XIH_PH_LEN				(64U)

/**
 * Image header table field offsets
 */
#define XIH_IHT_VERSION_OFFSET				(0x0U)
#define XIH_IHT_NO_OF_PARTITONS_OFFSET			(0x4U)
#define XIH_IHT_PH_ADDR_OFFSET	        		(0x8U)
#define XIH_IHT_AC_OFFSET				(0x10U)
#define XIH_IHT_PPD_OFFSET			        (0x14U)
#define XIH_IHT_CHECKSUM_OFFSET				(0x3CU)

/**
 * Partition Header Fields
 */
#define XIH_PH_ENC_DATAWORD_LENGTH		(0x0U)
#define XIH_PH_UNENC_DATAWORD_LENGTH		(0x4U)
#define XIH_PH_TOTAL_DATAWORD_LENGTH			(0x8U)
#define XIH_PH_NEXT_PARTITION_OFFSET			(0xCU)
#define XIH_PH_DEST_EXECUTION_ADDRESS	        (0x10U)
#define XIH_PH_DEST_LOAD_ADDRESS			(0x18U)
#define XIH_PH_DATA_WORD_OFFSET				(0x20U)
#define XIH_PH_ATTRB_OFFSET				(0x24U)
#define XIH_PH_SECTION_COUNT				(0x28U)
#define XIH_PH_CHECKSUM_WORD_OFFSET			(0x2CU)
#define XIH_PH_IMAGEHEADER_OFFSET			(0x30U)
#define XIH_PH_AUTHCERTIFICATE_OFFSET			(0x34U)
#define XIH_PH_CHECKSUM					(0x3CU)

/**
 * Partition Present Devices(PPD) in IHT
 */
#define XIH_IHT_PPD_SAME	        (0x0U)
#define XIH_IHT_PPD_QSPI32	        (0x1U)
#define XIH_IHT_PPD_QSPI24	        (0x2U)
#define XIH_IHT_PPD_NAND	        (0x3U)
#define XIH_IHT_PPD_SD_0			(0x4U)
#define XIH_IHT_PPD_SD_1			(0x5U)
#define XIH_IHT_PPD_SD_LS			(0x6U)
#define XIH_IHT_PPD_MMC		        (0x7U)
#define XIH_IHT_PPD_USB	        	(0x8U)
#define XIH_IHT_PPD_ETHERNET		(0x9U)
#define XIH_IHT_PPD_PCIE	        (0xAU)
#define XIH_IHT_PPD_SATA	        (0xBU)

/**
 * Partition Attribute fields
 */
#define XIH_PH_ATTRB_VEC_LOCATION_MASK		(0x800000U)
#define XIH_PH_ATTR_BLOCK_SIZE_MASK		(0x700000U)
#define XIH_PH_ATTRB_ENDIAN_MASK		(0x40000U)
#define XIH_PH_ATTRB_PART_OWNER_MASK		(0x30000U)
#define XIH_PH_ATTRB_RSA_SIGNATURE_MASK		(0x8000U)
#define XIH_PH_ATTRB_CHECKSUM_MASK		(0x7000U)
#define XIH_PH_ATTRB_DEST_CPU_MASK		(0x0F00U)
#define XIH_PH_ATTRB_ENCRYPTION_MASK		(0x0080U)
#define XIH_PH_ATTRB_DEST_DEVICE_MASK		(0x0070U)
#define XIH_PH_ATTRB_A53_EXEC_ST_MASK		(0x0008U)
#define XIH_PH_ATTRB_TARGET_EL_MASK		(0x0006U)
#define XIH_PH_ATTRB_TR_SECURE_MASK		(0x0001U)

/**
 * Partition Attribute Values
 */
#define XIH_PH_ATTRB_PART_OWNER_FSBL		(0x00000U)
#define XIH_PH_ATTRB_RSA_SIGNATURE		(0x8000U)
#define XIH_PH_ATTRB_NOCHECKSUM			(0x0000U)
#define XIH_PH_ATTRB_CHECKSUM_MD5		(0x1000U)
#define XIH_PH_ATTRB_HASH_SHA2			(0x2000U)
#define XIH_PH_ATTRB_HASH_SHA3			(0x3000U)

#define XIH_PH_ATTRB_DEST_CPU_NONE	(0x0000U)
#define XIH_PH_ATTRB_DEST_CPU_A53_0	(u32)(0x100U)
#define XIH_PH_ATTRB_DEST_CPU_A53_1	(u32)(0x200U)
#define XIH_PH_ATTRB_DEST_CPU_A53_2	(u32)(0x300U)
#define XIH_PH_ATTRB_DEST_CPU_A53_3	(u32)(0x400U)
#define XIH_PH_ATTRB_DEST_CPU_R5_0	(u32)(0x500U)
#define XIH_PH_ATTRB_DEST_CPU_R5_1	(u32)(0x600U)
#define XIH_PH_ATTRB_DEST_CPU_R5_L	(u32)(0x700U)
#define XIH_PH_ATTRB_DEST_CPU_PMU	(u32)(0x800U)

#define XIH_PH_ATTRB_ENCRYPTION		(u32)(0x80U)


#define XIH_PH_ATTRB_DEST_DEVICE_NONE	(u32)(0x0000U)
#define XIH_PH_ATTRB_DEST_DEVICE_PS	(u32)(0x0010U)
#define XIH_PH_ATTRB_DEST_DEVICE_PL	(u32)(0x0020U)
#define XIH_PH_ATTRB_DEST_DEVICE_PMU	(u32)(0x0030U)

#define XIH_PH_ATTRB_A53_EXEC_ST_AA32	(u32)(0x0008U)
#define XIH_PH_ATTRB_A53_EXEC_ST_AA64	(u32)(0x0000U)

#define XIH_AC_ATTRB_PPK_SELECT_MASK	(u32)(0x30000U)
#define XIH_AC_SPKID_OFFSET		(u32)(0x04U)

#define XIH_INVALID_EXEC_ST	(u32)(0xFFFFU)
/**
 * Below is the bit mapping of fields in the ATF Handoff parameters
 * with that of Partition header. The number of bits shifted is
 * is based on the difference between these two offsets
 *
 *                   ATFHandoffParams	PartitionHeader         Shift
 *     Parameter     PartitionFlags     PartitionAttributes   difference
 * ----------------------------------------------------------------------
 *	Exec State            0			         3                  3 right
 *	ENDIAN	              1	                 18                 17 right
 *	SECURE                2                  0                  2 left
 *	EL                    3:4                1:2                2 left
 *	CPU_A53               5:6                8:10
 */
#define XIH_ATTRB_VECTOR_LOCATION_SHIFT		(23U)
#define XIH_ATTRB_BLOCK_SIZE_SHIFT			(20U)
#define XIH_ATTRB_A53_EXEC_ST_SHIFT_DIFF    (3U)
#define XIH_ATTRB_ENDIAN_SHIFT_DIFF         (17U)
#define XIH_ATTRB_TR_SECURE_SHIFT_DIFF      (2U)
#define XIH_ATTRB_TARGET_EL_SHIFT_DIFF      (2U)


#define XIH_PART_FLAGS_DEST_CPU_A53_MASK   	(0x60U)
#define XIH_PART_FLAGS_DEST_CPU_A53_0   	(u32)(0x00U)
#define XIH_PART_FLAGS_DEST_CPU_A53_1   	(u32)(0x20U)
#define XIH_PART_FLAGS_DEST_CPU_A53_2   	(u32)(0x40U)
#define XIH_PART_FLAGS_DEST_CPU_A53_3   	(u32)(0x60U)

/* Number of entries possible in ATF: 4 cores * 2 (secure, nonsecure) */
#define XFSBL_MAX_ENTRIES_FOR_ATF	8U

/* BLOCK SIZE multiplier */
#define XFSBL_MUL_MEGABYTES			(1024U * 1024U)

/* Size of Image Header */
#define XFSBL_SIZE_IMAGE_HDR		(0x1080)

/**************************** Type Definitions *******************************/

/**
 * Structure to store the image header table details.
 * It contains all the information of image header table in order.
 */
typedef struct {
	u32 Version; /**< bootgen version used  */
	u32 NoOfPartitions; /**< No of partition present  */
	u32 PartitionHeaderAddress; /**< Address to start of partition header*/
	u32 Reserved_0xC; /**< Reserved */
	u32 AuthCertificateOffset; /** Authentication certificate address */
	u32 PartitionPresentDevice;
		/**< Partition present device for secondary boot modes*/
	u32 Reserved[9]; /**< Reserved */
	u32 Checksum; /**< Checksum of the image header table */
} XFsblPs_ImageHeaderTable;

/**
 * Structure to store the partition header details.
 * It contains all the information of partition header in order.
 */
typedef struct {
	u32 EncryptedDataWordLength; /**< Encrypted word length of partition*/
	u32 UnEncryptedDataWordLength; /**< unencrypted word length */
	u32 TotalDataWordLength;
		/**< Total word length including the authentication
			certificate if any*/
	u32 NextPartitionOffset; /**< Address of the next partition header*/
	u64 DestinationExecutionAddress; /**< Execution address */
	u64 DestinationLoadAddress; /**< Load address in DDR/TCM */
	u32 DataWordOffset; /**< */
	u32 PartitionAttributes; /**< partition attributes */
	u32 SectionCount; /**< section count */
	u32 ChecksumWordOffset; /**< address to checksum when enabled */
	u32 ImageHeaderOffset; /**< address to image header */
	u32 AuthCertificateOffset;
		/**< address to the authentication certificate when enabled */
	u32 Iv; /**< 8 bits are to be added to IV and remaining are reserved */
	u32 Checksum; /**< checksum of the partition header */
} XFsblPs_PartitionHeader;

/**
 * Structure of the image header which contains
 * information of image header table and
 * partition headers.
 */
typedef struct {
	XFsblPs_ImageHeaderTable ImageHeaderTable;
		/**< Image header table structure */
	XFsblPs_PartitionHeader PartitionHeader[XIH_MAX_PARTITIONS];
		/**< Partition header */
} XFsblPs_ImageHeader;

/* Structure corresponding to each partition entry */
typedef struct {
	u64 EntryPoint;
	/**< Entry point */
	u64 PartitionFlags;
	/**< Attributes of partition */
} XFsblPs_PartitionEntry;

/* Structure for handoff parameters to ARM Trusted Firmware (ATF) */
typedef struct {
	char8 MagicValue[4];
		/**< 32 bit magic string */
	u32 NumEntries;
		/**< Number of Entries */
	XFsblPs_PartitionEntry Entry[XFSBL_MAX_ENTRIES_FOR_ATF];
	/**< Structure corresponding to each entry */
} XFsblPs_ATFHandoffParams;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
u32 XFsbl_GetPartitionOwner(const XFsblPs_PartitionHeader * PartitionHeader);
u32 XFsbl_IsRsaSignaturePresent(const XFsblPs_PartitionHeader * PartitionHeader);
u32 XFsbl_GetChecksumType(XFsblPs_PartitionHeader * PartitionHeader);
u32 XFsbl_GetDestinationCpu(const XFsblPs_PartitionHeader * PartitionHeader);
u32 XFsbl_IsEncrypted(const XFsblPs_PartitionHeader * PartitionHeader);
u32 XFsbl_GetDestinationDevice(const XFsblPs_PartitionHeader * PartitionHeader);
u32 XFsbl_GetA53ExecState(const XFsblPs_PartitionHeader * PartitionHeader);
u32 XFsbl_GetVectorLocation(const XFsblPs_PartitionHeader * PartitionHeader);
u32 XFsbl_GetBlockSize(const XFsblPs_PartitionHeader * PartitionHeader);

u32 XFsbl_ValidateChecksum(u32 Buffer[], u32 Length);
u32 XFsbl_ReadImageHeader(XFsblPs_ImageHeader * ImageHeader,
                  const XFsblPs_DeviceOps * DeviceOps, u32 FlashImageOffsetAddress,
				u32 RunningCpu, u32 ImageHeaderAddress);
u32 XFsbl_ValidateImageHeader(const XFsblPs_ImageHeaderTable * ImageHeaderTable);
u32 XFsbl_ValidatePartitionHeader(XFsblPs_PartitionHeader * PartitionHeader,
			u32 RunningCpu, u32 ResetType);

#ifdef __cplusplus
}
#endif

#endif /* XFSBL_IH_H */
