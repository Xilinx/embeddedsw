/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file versal_net/xilpdi_plat.h
*
* This is the header file which contains versalnet specific definitions
* for the PDI.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       bm   07/13/2022 Added compatibility check for In-Place PLM Update
*       bm   09/13/2022 Added macros for maximum number of partitions and images
* 1.01  har  11/17/2022 Added macros for Signed Image attribute in Bootheader
*       ng   11/23/2022 Fixed doxygen file name error
*       sk   01/11/2023 Added macro for Image Store as SBD
* 1.02  ng   04/27/2023 Added support for cluster flags in ATF handoff params
*       sk   07/09/2023 Added defines for TCM Boot Enable Bit, Mask
*       am   07/10/2023 Moved IHT OP data macros to common xilpdi.h file
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
#define XIH_BH_IMG_ATTRB_SIGNED_IMG_MASK	(0xC0000U)
#define XIH_BH_IMG_ATTRB_SIGNED_IMG_SHIFT	(18U)
#define XIH_BH_IMG_ATTRB_SIGNED_IMG_VALUE	(0x3U)

#define XIH_MAX_PRTNS			(32U)
#define XIH_MAX_IMGS			(32U)

/* Secondary Boot Device (SBD) in IHT Attributes */
#define XIH_IHT_ATTR_SBD_MASK	        (0xFC0U)
#define XIH_IHT_ATTR_SBD_SHIFT			(0x6U)
#define XIH_IHT_ATTR_SBD_SAME	        (0x0U)
#define XIH_IHT_ATTR_SBD_QSPI32	        (0x1U)
#define XIH_IHT_ATTR_SBD_QSPI24	        (0x2U)
#define XIH_IHT_ATTR_SBD_SDLS_B0	(0x4U)
#define XIH_IHT_ATTR_SBD_SD_B1			(0x5U)
#define XIH_IHT_ATTR_SBD_SDLS_B1          (0x6U)
#define XIH_IHT_ATTR_SBD_EMMC	        (0x7U)
#define XIH_IHT_ATTR_SBD_USB			(0x8U)
#define XIH_IHT_ATTR_SBD_PCIE	        (0xAU)
#define XIH_IHT_ATTR_SBD_OSPI			(0xCU)
#define XIH_IHT_ATTR_SBD_SMAP			(0xDU)
#define XIH_IHT_ATTR_SBD_SDLS_B0_RAW		(0xFU)
#define XIH_IHT_ATTR_SBD_SD_B1_RAW		(0x10U)
#define XIH_IHT_ATTR_SBD_SDLS_B1_RAW		(0x11U)
#define XIH_IHT_ATTR_SBD_EMMC_RAW		(0x12U)
#define XIH_IHT_ATTR_SBD_EMMC_0			(0x13U)
#define XIH_IHT_ATTR_SBD_EMMC_0_RAW		(0x14U)
#define XIH_IHT_ATTR_IMAGE_STORE		(0x15U)

/**
 *  Prtn Attribute fields
 */
#define XIH_PH_ATTRB_HIVEC_MASK			(0x800000U)
#define XIH_PH_ATTRB_ENDIAN_MASK		(0x40000U)
#define XIH_PH_ATTRB_DSTN_CLUSTER_MASK		(0x00C0U)
#define XIH_PH_ATTRB_CLUSTER_LOCKSTEP_MASK	(0x0030U)
#define XIH_PH_ATTRB_A78_EXEC_ST_MASK		(0x0008U)
#define XIH_PH_ATTRB_DSTN_CLUSTER_SHIFT		(6U)
#define XIH_PH_ATTRB_TCM_BOOT_MASK		(0x180000U)

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
#define XIH_PH_ATTRB_DSTN_CLUSTER_0				(0x000U)
#define XIH_PH_ATTRB_DSTN_CLUSTER_1				(0x040U)
#define XIH_PH_ATTRB_DSTN_CLUSTER_2				(0x080U)
#define XIH_PH_ATTRB_DSTN_CLUSTER_3				(0x0C0U)
#define XIH_ATTRB_DSTN_CLUSTER_0				(0x000U)
#define XIH_ATTRB_DSTN_CLUSTER_1				(0x001U)
#define XIH_ATTRB_DSTN_CLUSTER_2				(0x002U)
#define XIH_ATTRB_DSTN_CLUSTER_3				(0x003U)
#define XIH_PH_ATTRB_CLUSTER_LOCKSTEP_ENABLED			(0x030U)
#define XIH_PH_ATTRB_CLUSTER_LOCKSTEP_DISABLED			(0x000U)
#define XIH_PH_ATTRB_DSTN_CPU_A78_0				(0x100U)
#define XIH_PH_ATTRB_DSTN_CPU_A78_1				(0x200U)
#define XIH_PH_ATTRB_DSTN_CPU_A78_2				(0x300U)
#define XIH_PH_ATTRB_DSTN_CPU_A78_3				(0x400U)
#define XIH_PH_ATTRB_DSTN_CPU_R52_0				(0x500U)
#define XIH_PH_ATTRB_DSTN_CPU_R52_1				(0x600U)
#define XIH_PH_ATTRB_DSTN_CPU_PSM				(0x800U)
#define XIH_PH_ATTRB_TCM_BOOT_ENABLED				(0x180000U)

/**
 * Below is the bit mapping of fields in the ATF Handoff parameters
 * with that of Prtn header. The number of bits shifted is
 * is based on the difference between these two offsets
 *
 *                   ATFHandoffParams     PrtnHdr              Shift
 *  Parameter        PrtnFlags            PrtnAttrb            difference
 * ----------------------------------------------------------------------
 *  Exec State            0                  3                  3 right
 *  ENDIAN                1                  18                 17 right
 *  SECURE                2                  0                  2 left
 *  EL                    3:4                1:2                2 left
 *  CPU_A78               5:6                8:11
 *  Cluster#              11:12              6:7                5 left
 */
#define XIH_ATTRB_A78_EXEC_ST_SHIFT_DIFF    (3U)
#define XIH_ATTRB_ENDIAN_SHIFT_DIFF         (17U)
#define XIH_ATTRB_TR_SECURE_SHIFT_DIFF      (2U)
#define XIH_ATTRB_TARGET_EL_SHIFT_DIFF      (2U)
#define XIH_PRTN_FLAGS_DSTN_CLUSTER_SHIFT_DIFF   (0x5U)

#define XIH_ATTRB_EL_MASK			(0x18U)
#define XIH_PRTN_FLAGS_EL_2			(0x10U)
#define XIH_PRTN_FLAGS_EL_3			(0x18U)

#define XIH_PRTN_FLAGS_DSTN_CPU_A78_0		(0x00U)
#define XIH_PRTN_FLAGS_DSTN_CPU_A78_1		(0x20U)
#define XIH_PRTN_FLAGS_DSTN_CPU_A78_2		(0x40U)
#define XIH_PRTN_FLAGS_DSTN_CPU_A78_3		(0x60U)

/* Optional Data related defines */
#define XIH_OPTIONAL_DATA_LEN_OFFSET	(0x58U)
#define XIH_OPT_DATA_NON_DATA_LEN	(8U)

#define XIH_OPT_DATA_STRUCT_INFO_ID	(2U)

/**************************** Type Definitions *******************************/

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
* @brief	This function will return the value of Destination Cluster field.
*
* @param	PrtnHdr is pointer to the Partition Header
*
* @return	Destination Cluster
*
*****************************************************************************/
static inline u32 XilPdi_GetDstnCluster(const XilPdi_PrtnHdr *PrtnHdr)
{
	return (PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_DSTN_CLUSTER_MASK);
}

/****************************************************************************/
/**
* @brief	This function will return the value of Cluster lockstep field.
*
* @param	PrtnHdr is pointer to the Partition Header
*
* @return	Cluster Lockstep Value
*
*****************************************************************************/
static inline u32 XilPdi_GetClusterLockstep(const XilPdi_PrtnHdr *PrtnHdr)
{
	return (PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_CLUSTER_LOCKSTEP_MASK);
}

/****************************************************************************/
/**
* @brief	This function will return the value of A78 Execution State field.
*
* @param	PrtnHdr is pointer to the Partition Header
*
* @return	A78 Execution State
*
*****************************************************************************/
static inline u32 XilPdi_GetA78ExecState(const XilPdi_PrtnHdr *PrtnHdr)
{
	return (PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_A78_EXEC_ST_MASK);
}

/************************** Function Prototypes ******************************/

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* XILPDI_PLAT_H */
