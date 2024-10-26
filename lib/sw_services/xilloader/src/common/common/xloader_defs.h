/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
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
#ifdef VERSAL_NET
#define XLOADER_CMD_ID_DATA_AUTH		        (17U) /**< API ID for Data authentication */
#endif
#define XLOADER_CFI_SEL_READBACK_ID             (18U) /**< command id for CFI selective readback */

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

/************************************** Type Definitions *****************************************/

typedef struct {
	u32 ImgID; /**< Image ID */
	u32 UID; /**< Unique ID */
	u32 PUID; /**< Parent UID */
	u32 FuncID; /**< Function ID */
} XLoader_ImageInfo; /**< Image information */

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