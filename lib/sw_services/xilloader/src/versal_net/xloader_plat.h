/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_plat.h
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
#define XLOADER_SECURE_CHUNK_SIZE	(0x8000U) /* 32K */

/* Boot Modes */
#define XLOADER_PDI_SRC_JTAG		(0x0U)
#define XLOADER_PDI_SRC_QSPI24		(0x1U)
#define XLOADER_PDI_SRC_QSPI32		(0x2U)
#define XLOADER_PDI_SRC_SDLS_B0		(0x3U)
#define XLOADER_PDI_SRC_EMMC0		(0x4U)
#define XLOADER_PDI_SRC_SD_B1		(0x5U)
#define XLOADER_PDI_SRC_EMMC1		(0x6U)
#define XLOADER_PDI_SRC_USB		(0x7U)
#define XLOADER_PDI_SRC_OSPI		(0x8U)
#define XLOADER_PDI_SRC_SBI		(0x9U)
#define XLOADER_PDI_SRC_SMAP		(0xAU)
#define XLOADER_PDI_SRC_PCIE		(0xBU)
#define XLOADER_PDI_SRC_SDLS_B1		(0xEU)
#define XLOADER_PDI_SRC_DDR		(0xFU)
#define XLOADER_PDI_SRC_INVALID		(0xFFU)

#define XLOADER_R52_0A_TCMA_BASE_ADDR 	(0xEBA00000U)
#define XLOADER_R52_1A_TCMA_BASE_ADDR 	(0xEBA40000U)
#define XLOADER_R52_0B_TCMA_BASE_ADDR 	(0xEBA80000U)
#define XLOADER_R52_1B_TCMA_BASE_ADDR 	(0xEBAC0000U)
#define XLOADER_R52_TCM_CLUSTER_OFFSET	(0x00080000U)

/*
 * TCM address for R52
 */
#define XLOADER_R52_TCMA_LOAD_ADDRESS	(0x0U)
#define XLOADER_R52_TCM_TOTAL_LENGTH	(0x30000U)

#define XLOADER_APU_CLUSTER0	(0U)
#define XLOADER_APU_CLUSTER1	(1U)
#define XLOADER_APU_CLUSTER2	(2U)
#define XLOADER_APU_CLUSTER3	(3U)

#define XLOADER_APU_CORE0	(0U)
#define XLOADER_APU_CORE1	(1U)
#define XLOADER_APU_CORE2	(2U)
#define XLOADER_APU_CORE3	(3U)

#define XLOADER_RPU_CLUSTERA	(0U)
#define XLOADER_RPU_CLUSTERB	(1U)

#define XLOADER_RPU_CORE0	(0U)
#define XLOADER_RPU_CORE1	(1U)

/* Xilloader Module Data Structure Ids*/
#define XLOADER_IMAGE_INFO_DS_ID		(0x01U)
#define XLOADER_PDI_INST_DS_ID			(0x02U)
#define XLOADER_PDI_LIST_DS_ID			(0x03U)
#define XLOADER_ATF_HANDOFF_PARAMS_DS_ID	(0x04U)

#define XLOADER_SHA1_DEVICE_ID		(1U)
/* Data measurement flags */
#define XLOADER_MEASURE_START		(0U)
#define XLOADER_MEASURE_UPDATE		(1U)
#define XLOADER_MEASURE_FINISH		(2U)

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
	}

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

/************************** Function Prototypes ******************************/
XLoader_ImageInfoTbl *XLoader_GetImageInfoTbl(void);
int XLoader_StartImage(XilPdi *PdiPtr);
int XLoader_GetSDPdiSrcNAddr(u32 SecBootMode, XilPdi *PdiPtr, u32 *PdiSrc,
		u32 *PdiAddr);
int XLoader_UpdateHandoffParam(XilPdi* PdiPtr);
int XLoader_ProcessDeferredError(void);
int XLoader_ProcessElf(XilPdi* PdiPtr, const XilPdi_PrtnHdr * PrtnHdr,
	XLoader_PrtnParams* PrtnParams, XLoader_SecureParams* SecureParams);
XilPdi *XLoader_GetPdiInstance(void);
XLoader_ImageStore* XLoader_GetPdiList(void);
int XLoader_UpdateHandler(XPlmi_ModuleOp Op);
int XLoader_PlatInit(void);
int XLoader_HdrMeasurement(XilPdi* PdiPtr);
int XLoader_DataMeasurement(u64 DataAddr, u32 DataSize, u32 PcrInfo, u8 Flags);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_PLAT_H */
