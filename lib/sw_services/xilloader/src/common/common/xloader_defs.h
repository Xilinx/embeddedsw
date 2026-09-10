/**************************************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xloader_defs.h
 *
 * This file contains the xilloader API IDs
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  dd   01/09/24 Initial release
 *       har  03/05/24 Fixed doxygen warnings
 *       pre  08/22/24 Added XLOADER_CFI_SEL_READBACK_ID
 *       pre  10/26/24 Made XLOADER_CMD_ID_LOAD_READBACK_PDI command ID as reserved
 * 2.4   gnr  02/06/26 Removed conditional inclusion for XLOADER_CMD_ID_DATA_AUTH
 * 2.5   ng   07/09/26 Added XLOADER_SET_DDR_ADDR_FOR_RESTART_IMAGE_ID command id
 *       ng   08/22/26 Added macros for glitch detector configuration
 *       ng   07/09/26 Added XLOADER_SET_DDR_ADDR_FOR_RESTART_IMAGE_ID command id
 * 2.5   pre  08/12/26 Added XLOADER_CMD_ID_CALIBDATA_RESTORE command id
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XLOADER_DEFS_H
#define XLOADER_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

#include "xil_printf.h"
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/* Loader command Ids */
#define XLOADER_CMD_ID_FEATURES					(0U) /**< command id for features */
#define XLOADER_CMD_ID_LOAD_SUBSYSTEM_PDI       (1U) /**< command id for load subsystem pdi */
#define XLOADER_CMD_ID_LOAD_DDRCPY_IMG          (2U) /**< command id for DDR copy image */
#define XLOADER_CMD_ID_GET_IMAGE_INFO           (3U) /**< command id for get Image info */
#define XLOADER_CMD_ID_SET_IMAGE_INFO           (4U) /**< command id for set Image info */
#define XLOADER_CMD_ID_GET_IMAGE_INFO_LIST      (5U) /**< command id for get Image info list */
#define XLOADER_CMD_ID_EXTRACT_METAHEADER       (6U) /**< command id for Extract meta header */
#define XLOADER_CMD_ID_RESERVED        			(7U) /**< Reserved command id */
#define XLOADER_CMD_ID_UPDATE_MULTIBOOT         (8U) /**< command id for update Multiboot */
#define XLOADER_CMD_ID_ADD_IMAGESTORE_PDI       (9U) /**< command id for add Imagestore pdi */
#define XLOADER_CMD_ID_REMOVE_IMAGESTORE_PDI    (10U) /**< command id for remove Imagestore pdi */
#define XLOADER_CMD_ID_GET_ATF_HANDOFF_PARAMS   (11U)
                                                /**< command id for get ATF handoff parameters */
#define XLOADER_CMD_ID_CFRAME_DATA_CLEAR_CHECK 	(12U)
                                                /**< command id for Cframe data clear check */
#define XLOADER_CMD_ID_WRITE_IMAGESTORE_PDI   	(13U) /**< command id for write Imagestore pdi */
#define XLOADER_CMD_ID_CONFIG_JTAG_STATE  	    (14U) /**< command id for config jtag state */
#define XLOADER_CMD_ID_READ_DDR_CRYPTO_COUNTERS (15U)
                                                /**< command id for read DDR crypto  counters */
#define XLOADER_CMD_ID_I2C_HANDSHAKE            (16U) /**< command id for I2C handshake */
#define XLOADER_CMD_ID_DATA_AUTH		        (17U) /**< API ID for Data authentication */
#define XLOADER_CFI_SEL_READBACK_ID             (18U) /**< command id for CFI selective readback */
#define XLOADER_SET_DDR_ADDR_FOR_RESTART_IMAGE_ID (19U) /**< command id for setting DDR address to
							 * restart image from DDR */
#define XLOADER_CMD_ID_CONFIG_GLITCH_DETECTOR (20U) /**< command id for glitch detector config */
#define XLOADER_CMD_ID_CALIBDATA_RESTORE	(21U) /**< command id for DDR calibration
							 * data restore */

#define XLOADER_HEADER_LEN_1			(1U) /**< Header length 1 */
#define XLOADER_HEADER_LEN_2			(2U) /**< Header length 2 */
#define XLOADER_HEADER_LEN_3			(3U) /**< Header length 3 */
#define XLOADER_HEADER_LEN_4			(4U) /**< Header length 4 */
#define XLOADER_HEADER_LEN_5			(5U) /**< Header length 5 */
#define XLOADER_HEADER_LEN_6			(6U) /**< Header length 6 */

/**
 * @name Payload arguments for Extract Metaheader command
 * @{
 */
/**< Indices for payload arguments in Extract Metaheader command */
#define XLOADER_CMD_EXTRACT_METAHDR_PDIADDR_HIGH_INDEX	(0U)
#define XLOADER_CMD_EXTRACT_METAHDR_PDI_ID_INDEX	(0U)
#define XLOADER_CMD_EXTRACT_METAHDR_PDIADDR_LOW_INDEX	(1U)
#define XLOADER_CMD_EXTRACT_METAHDR_DESTADDR_HIGH_INDEX	(2U)
#define XLOADER_CMD_EXTRACT_METAHDR_DESTADDR_LOW_INDEX	(3U)
#define XLOADER_CMD_EXTRACT_METAHDR_DEST_SIZE_INDEX	(4U)
#define XLOADER_CMD_EXTRACT_METAHDR_DATAID_PDISRC_INDEX	(5U)
/** @} */

#define XLOADER_ADDR_HIGH_SHIFT 		(32U) /**< Shift value to get higher 32 bit address */
#define XLOADER_GET_OPT_DATA_FLAG		(0x80U)
	/**< Flag to indicate that Extract Metaheader request is to extract optional data only */
#define XLOADER_DATA_ID_SHIFT			(16U)
	/**< Shift to get data ID from the payload of Extract metaheader command */

/***************** Macros (Inline Functions) Definitions *********************/
/**
 * @def XLOADER_DEBUG
 * @brief Enable debug prints in XLoader when set to 1.
 */
 #define XLOADER_DEBUG	(0U)

 /**
 * @def XLOADER_DEBUG_GENERAL
 * @brief General flag to enable debug prints in XLoader when set to 1.
 */
#if (XLOADER_DEBUG)
#define XLOADER_DEBUG_GENERAL (1U)
#else
#define XLOADER_DEBUG_GENERAL (0U)
#endif
#define XLoader_Client_Printf(DebugType, ...)	\
	if ((DebugType) == 1U) {xil_printf (__VA_ARGS__);}	/**< Print debug messages */
/************************************** Type Definitions *****************************************/
#if defined(VERSAL_2VE_2VM)
/**
 * Glitch detector type values for XLOADER_CMD_ID_CONFIG_GLITCH_DETECTOR.
 * Each payload word packs: [7:0] type, [15:8] depth, [23:16] width, [31:24] ref_voltage.
 */
typedef enum {
	XLOADER_GD_TYPE_PMC_GD0 = 1U, /**< PMC glitch detector 0 */
	XLOADER_GD_TYPE_PMC_GD1 = 2U, /**< PMC glitch detector 1 */
	XLOADER_GD_TYPE_ASU_GD = 3U, /**< ASU glitch detector */
	XLOADER_GD_TYPE_LPD_GD0 = 4U, /**< LPD glitch detector 0 */
	XLOADER_GD_TYPE_LPD_GD1 = 5U, /**< LPD glitch detector 1 */
} XLoader_GdType; /**< Glitch detector types */

#define XLOADER_GD_TYPE_SHIFT			(0U) /**< GD type in payload bits [7:0] */
#define XLOADER_GD_DEPTH_SHIFT			(8U) /**< Depth in payload bits [15:8] */
#define XLOADER_GD_WIDTH_SHIFT			(16U) /**< Width in payload bits [23:16] */
#define XLOADER_GD_REFVOL_SHIFT			(24U) /**< Ref voltage in payload bits [31:24] */
#define XLOADER_GD_FIELD_MASK			(0xFFU) /**< Mask for each packed u8 field */

#define XLOADER_GD_CFG_MIN_LEN			(1U) /**< Minimum payload word count */
#define XLOADER_GD_CFG_MAX_LEN			(5U) /**< Max Len: PMC + ASU + LPD */

#define XLOADER_GD_USER_REG_VAL			(1U) /**< Use register values vs eFUSE */
#define XLOADER_GD_NUM0				(0U) /**< Glitch detector 0 instance */
#define XLOADER_GD_NUM1				(1U) /**< Glitch detector 1 instance */

/*
 * Maximum values of the calibration fields. These are field widths, not
 * detector properties: the PMC, ASU and LPD detectors all take a 5-bit depth,
 * a 4-bit width and a 4-bit reference voltage, so all three configure paths
 * share these bounds. Should a future device diverge, split them deliberately.
 */
#define XLOADER_GD_DEPTH_MAX			(0x1FU) /**< Depth field is 5 bits */
#define XLOADER_GD_WIDTH_MAX			(0x0FU) /**< Width field is 4 bits */
#define XLOADER_GD_REFVOL_MAX			(0x0FU) /**< Ref voltage field is 4 bits */

/*
 * PMC GD_FUSE_CTRL packing.
 * Common to all platforms since the PMC GD_FUSE_CTRL registers are present
 * on every Versal variant.
 */
#define XLOADER_GD_REFVOL_3LSBITS_MASK		(0x07U)
#define XLOADER_GD_DEPTH_3LSBITS_MASK		(0x07U)
#define XLOADER_GD_REFVOL_MSBIT_MASK		(0x08U)
#define XLOADER_GD_DEPTH_2MSBITS_MASK		(0x18U)
#define XLOADER_GD_REFVOL_MSBIT_SHIFT		(16U)
#define XLOADER_GD_REFVOL_3LSBITS_SHIFT		(12U)
#define XLOADER_GD_DEPTH_2MSBITS_SHIFT		(1U)
#define XLOADER_GD_DEPTH_2MSBITS_RSHIFT		(3U)
#define XLOADER_GD_DEPTH_3LSBITS_SHIFT		(16U)
#define XLOADER_GD_WIDTH_BITS_SHIFT		(8U)
#define XLOADER_GD_WIDTH_BITS_MASK		(0x0FU)

/** Bitmask covering all GD_FUSE_CTRL register fields for PMC GD0/GD1. */
#define XLOADER_GD_FUSE_CTRL_RMW_MASK		( \
	XLOADER_GD_USER_REG_VAL | \
	(((u32)XLOADER_GD_DEPTH_2MSBITS_MASK) << XLOADER_GD_DEPTH_2MSBITS_SHIFT) | \
	(((u32)XLOADER_GD_WIDTH_BITS_MASK) << XLOADER_GD_WIDTH_BITS_SHIFT) | \
	(((u32)XLOADER_GD_REFVOL_3LSBITS_MASK) << XLOADER_GD_REFVOL_3LSBITS_SHIFT) | \
	(((u32)XLOADER_GD_REFVOL_MSBIT_MASK) << XLOADER_GD_REFVOL_MSBIT_SHIFT) | \
	(((u32)XLOADER_GD_DEPTH_3LSBITS_MASK) << XLOADER_GD_DEPTH_3LSBITS_SHIFT))

#endif

/**
 * Structure to hold Image information
 */
typedef struct {
	u32 ImgID; /**< Image ID */
	u32 UID; /**< Unique ID */
	u32 PUID; /**< Parent UID */
	u32 FuncID; /**< Function ID */
} XLoader_ImageInfo; /**< Image information */

/**
 * Structure to hold optional data information
 */
typedef struct {
	u32 PdiSrc;		/**< Source where PDI is present - DDR or Image Store*/
	u32 PdiAddrLow;		/**< Lower address of PDI when present in DDR */
	u32 PdiAddrHigh;	/**< Higher address of PDI when present in DDR */
	u32 DataId;		/**< Data ID of the requested optional data */
	u32 PdiId;		/**< PDI ID of the PDI in Image Store */
} XLoader_OptionalDataInfo;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_DEFS_H */