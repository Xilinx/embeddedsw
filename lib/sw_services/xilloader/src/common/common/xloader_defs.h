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
#define XLOADER_CMD_ID_LOAD_READBACK_PDI        (7U) /**< command id for load readback pdi */
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

#define XLOADER_HEADER_LEN_1			(1U) /**< Header length 1 */
#define XLOADER_HEADER_LEN_2			(2U) /**< Header length 2 */
#define XLOADER_HEADER_LEN_3			(3U) /**< Header length 3 */
#define XLOADER_HEADER_LEN_4			(4U) /**< Header length 4 */
#define XLOADER_HEADER_LEN_5			(5U) /**< Header length 5 */
#define XLOADER_HEADER_LEN_6			(6U) /**< Header length 6 */

/************************************** Type Definitions *****************************************/

typedef struct {
	u32 ImgID; /**< Image ID */
	u32 UID; /**< Unique ID */
	u32 PUID; /**< Parent UID */
	u32 FuncID; /**< Function ID */
} XLoader_ImageInfo; /**< Image information */

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_DEFS_H */