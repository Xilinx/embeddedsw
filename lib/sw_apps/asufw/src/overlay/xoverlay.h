/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xoverlay.h
*
* This file contains declarations for xoverlay.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   rmv  06/11/26 Initial release
*       kk   08/25/26 Added SLH-DSA overlay algorithm ID
*       vm   09/15/26 Added the key vault overlay algorithm identifier
*
* </pre>
*
*************************************************************************************************/
/**
* @addtogroup xoverlay_server_apis Overlay Manager APIs
* @{
*/
#ifndef XOVERLAY_H_
#define XOVERLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasufw_dma.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#else
#include "xparameters.h"
#endif

/************************************ Constant Definitions ***************************************/
#define XOVERLAY_MAGIC			(0x594C564FU)	/**< Overlay image sentinel */
#define XOVERLAY_HEADER_VERSION		(1U)		/**< Overlay header format version */

#define XOVERLAY_ALGO_INVALID		(0U)		/**< Invalid overlay algorithm ID */
#define XOVERLAY_ALGO_RSA_ECC		(1U)		/**< Combined RSA and ECC overlay algorithm ID */
#define XOVERLAY_ALGO_LMS		(2U)		/**< LMS overlay algorithm ID */
#define XOVERLAY_ALGO_MLDSA		(3U)		/**< ML-DSA overlay algorithm ID */
#define XOVERLAY_ALGO_MLKEM		(4U)		/**< ML-KEM overlay algorithm ID */
#define XOVERLAY_ALGO_SLHDSA		(5U)		/**< SLH-DSA overlay algorithm ID */
#define XOVERLAY_ALGO_KEYVAULT		(6U)		/**< Key vault overlay algorithm ID */

/************************************** Type Definitions *****************************************/
/**
 * This structure defines the header of an overlay image.
 */
typedef struct {
	u32 Magic;		/**< Image sentinel */
	u32 AlgoId;		/**< Algorithm identifier */
	u32 Version;		/**< Header format version */
	u32 ImgSize;		/**< Total image size in bytes */
	u32 TxtSize;		/**< Size in bytes of the IRAM (text) portion of the image */
	u32 DataSize;		/**< Size in bytes of the data portion of the image */
} XOverlay_Header;

/**
 * This structure defines the catalog entry of an overlay image.
 */
typedef struct {
	u32 AlgoId;				/**< Algorithm identifier */
	const void *ImgLma;			/**< Load address (LMA) of the image in DDR */
	const void *DataLma;			/**< Load address (LMA) of the data in DDR */
	u32 ImgSize;				/**< Total image size in bytes */
	u32 TxtSize;				/**< Size in bytes of the text portion */
	u32 DataSize;				/**< Size in bytes of the data portion */
	s32 (*OnLoad)(void);			/**< Optional hook run after a fresh load to restore
						overlaid instance state */
} XOverlay_CatalogEntry;

/************************************ Function Prototypes ****************************************/
s32 XOverlay_Activate(u32 AlgoId, XAsufw_Dma *DmaPtr);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XOVERLAY_H_ */
/** @} */
