/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader.h
*
* This file contains declarations for image store functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/25/2018 Initial release
*       tp   04/05/2019 Added API to reload a particular image
*       kc   04/09/2019 Added support for PCIe secondary boot mode and
*						partial PDI load
*       bsv  06/11/2019 Added TCM power up code to Xilloader to fix issue in
*						R5-1 split mode functionality
*       bsv  06/17/2019 Added support for CFI and CFU error handling
*       bsv  06/26/2019 Added secondary boot support
*       kc   07/16/2019 Added code to print execution time
*       rv   07/29/2019 Added code to request boot source before subsystem restart
*       vnsl 07/30/2019 Added APIs to load secure headers
*       scs  08/29/2019 Added API to validate extended ID Code
*       bsv  08/30/2019 Added fallback and multiboot support in PLM
*       kc   09/05/2019 Added code to use PMCDMA0 and PMCDMA1 in parallel
*       kc   09/13/2019 SBI reset is removed for SMAP boot mode to ensure smap
*						bus width value remains unchanged
* 1.01  bsv  10/31/2019 Added USB secondary boot mode support
*       kc   12/02/2019 Added performance time stamps
*       ma   12/12/2019 Added support for passing hand off parameters to ATF
*       bsv  12/30/2019 Added SMAP secondary boot mode support
*       ma   02/03/2020 Change XPlmi_MeasurePerfTime to retrieve Performance
*                       time and print
*       bsv  02/12/2020 Added support for SD/eMMC raw boot mode
*       bsv  02/23/2020 Added multi partition support for SD/eMMC FS boot modes
*       bsv  02/25/2020 Added macros to handle u32 return values from drivers
*       vnsl 02/26/2020 Added boot header reading call during partial PDI
*       kc   02/27/2020 Added SEM support for partial reconfiguration
*       bsv  02/28/2020 Added support for delay handoff
*       har  02/28/2020 Removed code to return error codes for security related
*                       errors
*       vnsl 03/01/2020 Added PUF KEK decrypt support
*       bsv  03/14/2020 Added eMMC0 FS and raw boot mode support
*       bsv  04/09/2020 Code clean up
* 1.02	ana  06/04/2020 Updated PlmkatStatus and Kekstatus variables from
*						initial boot pdi to partial pdi structure variables
*       bsv  06/22/2020 Cfi error handler should only be called for PL image
*       bsv  07/01/2020 Added DevRelease to DevOps
*       skd  07/14/2020 Function pointers Copy and DeviceCopy prototypes changed
*       kal  07/20/2020 Added macro XLOADER_SECURE_CHUNK_SIZE for security
*       bsv  07/29/2020 Added CopyToMem address and delay load fields in XilPdi
*                       structure
*       skd  07/29/2020 Removed device copy macros
*       bsv  08/06/2020 Code clean up
*       bsv  08/10/2020 Added subsystem restart support from DDR
*       har  08/11/2020 Added XLOADER_AUTH_JTAG_INT_STATUS_POLL_INTERVAL
*       kal  08/12/2020 Added param ImageId for XLoader_CframeErrorHandler
*                       to identify Full PL partition and perform PL house
*                       cleaning.
*       bsv  08/17/2020 Added redundancy in XLoader_IsAuthEnabled
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       bm   08/19/2020 Added minor error codes
*       skd  08/21/2020 Added flash size macros
*       bm   09/21/2020 Added ImageInfo related code and added compatibility
*                       check required for DFx
*       bm   09/24/2020 Added FuncID argument to RestartImage and ReloadImage
*       bsv  10/13/2020 Code clean up
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XLOADER_H
#define XLOADER_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xilpdi.h"
#include "xplmi_status.h"
#include "xplmi_debug.h"
#include "xplmi_dma.h"
#include "xcfupmc.h"

/************************** Constant Definitions *****************************/
#define XLOADER_SUCCESS		(u32)XST_SUCCESS
#define XLOADER_FAILURE		(u32)XST_FAILURE
#define XLoader_Printf		XPlmi_Printf
#define XLOADER_DDR_TEMP_BUFFER_ADDRESS	(0x50000000U)
#define XLOADER_CHUNK_SIZE		(0x10000U) /* 64K */
#define XLOADER_SECURE_CHUNK_SIZE	(0x8000U) /* 32K */
#define XLOADER_DMA_LEN_ALIGN           (0x10U)
#define XLOADER_IMAGE_SEARCH_OFFSET	(0x8000U) /* 32K */

#define XLOADER_R5_0_TCMA_BASE_ADDR	(0xFFE00000U)
#define XLOADER_R5_1_TCMA_BASE_ADDR	(0xFFE90000U)

/*
 * TCM address for R5
 */
#define XLOADER_R5_TCMA_LOAD_ADDRESS	(0x0U)
#define XLOADER_R5_TCMB_LOAD_ADDRESS	(0x20000U)
#define XLOADER_R5_TCM_BANK_LENGTH	(0x10000U)
#define XLOADER_R5_TCM_TOTAL_LENGTH	(XLOADER_R5_TCM_BANK_LENGTH * 4U)

/*
 * APU related macros
 */
#define XLOADER_FPD_APU_CONFIG_0	(0xFD5C0020U)
#define XLOADER_FPD_APU_CONFIG_0_AA64N32_MASK_CPU0	(0x1U)
#define XLOADER_FPD_APU_CONFIG_0_AA64N32_MASK_CPU1	(0x2U)
#define XLOADER_FPD_APU_CONFIG_0_VINITHI_MASK_CPU0	(0x100U)
#define XLOADER_FPD_APU_CONFIG_0_VINITHI_MASK_CPU1	(0x200U)

/*
 * Subsystem related macros
 */
#define XLOADER_MAX_HANDOFF_CPUS	(10U)

/*
 * PDI type macros
 */
#define XLOADER_PDI_TYPE_FULL		(0x1U)
#define XLOADER_PDI_TYPE_PARTIAL	(0x2U)
#define XLOADER_PDI_TYPE_RESTORE	(0x3U)

/*
 * Secondary boot mode related macros
 */
#define XLOADER_PDISRC_FLAGS_MASK	(0xFFU)

/*
 * PDI Loading status
 */
#define XLOADER_PDI_LOAD_COMPLETE	(0x1U)
#define XLOADER_PDI_LOAD_STARTED	(0x0U)

/*
 * SLR Types
 */
#define XLOADER_SSIT_MONOLITIC		(0x7U)
#define XLOADER_SSIT_MASTER_SLR		(0x6U)

/*
 * Flash Size macros
 */
#if defined(XLOADER_QSPI) || defined(XLOADER_OSPI)
#define XLOADER_FLASH_SIZE_ID_64M		(0x17U)
#define XLOADER_FLASH_SIZE_ID_128M		(0x18U)
#define XLOADER_FLASH_SIZE_ID_256M		(0x19U)
#define XLOADER_FLASH_SIZE_ID_512M		(0x20U)
#define XLOADER_FLASH_SIZE_ID_1G		(0x21U)
#define XLOADER_FLASH_SIZE_ID_2G		(0x22U)
#define XLOADER_FLASH_SIZE_64M          (0x0800000U)
#define XLOADER_FLASH_SIZE_128M         (0x1000000U)
#define XLOADER_FLASH_SIZE_256M         (0x2000000U)
#define XLOADER_FLASH_SIZE_512M         (0x4000000U)
#define XLOADER_FLASH_SIZE_1G           (0x8000000U)
#define XLOADER_FLASH_SIZE_2G           (0x10000000U)
#endif

/*
 * PDI Version macros
 */
#define XLOADER_PDI_VERSION_1			(0x01030000U)
#define XLOADER_PDI_VERSION_2			(0x00020000U)

/* Invalid Img ID */
#define XLOADER_INVALID_IMG_ID			(0x0U)

/* Invalid UID */
#define XLOADER_INVALID_UID			(0x0U)

/* Macro for Image Index Not found */
#define XLOADER_IMG_INDEX_NOT_FOUND		(0xFFFFFFFFU)


/* Boot Modes */
typedef enum {
	XLOADER_PDI_SRC_JTAG = (0x0),
	XLOADER_PDI_SRC_QSPI24 = (0x1),
	XLOADER_PDI_SRC_QSPI32 = (0x2),
	XLOADER_PDI_SRC_SD0 = (0x3),
	XLOADER_PDI_SRC_SD1 = (0x5),
	XLOADER_PDI_SRC_EMMC = (0x6),
	XLOADER_PDI_SRC_USB = (0x7),
	XLOADER_PDI_SRC_OSPI = (0x8),
	XLOADER_PDI_SRC_SMAP = (0xA),
	XLOADER_PDI_SRC_SD1_LS = (0xE),
	XLOADER_PDI_SRC_DDR = (0xF),
	XLOADER_PDI_SRC_SBI = (0x10),
	XLOADER_PDI_SRC_PCIE = (0x11),
	XLOADER_PDI_SRC_SD0_RAW = (0x12),
	XLOADER_PDI_SRC_SD1_RAW = (0x13),
	XLOADER_PDI_SRC_EMMC_RAW = (0x14),
	XLOADER_PDI_SRC_SD1_LS_RAW = (0x15),
	XLOADER_PDI_SRC_EMMC_RAW_BP1 = (0x16),
	XLOADER_PDI_SRC_EMMC_RAW_BP2 = (0x17),
	XLOADER_PDI_SRC_EMMC0 = (0x18),
	XLOADER_PDI_SRC_EMMC0_RAW = (0x19),
} PdiSrc_t;

/* Minor Error Codes */
enum {
	XLOADER_ERR_INVALID_IMGID = 0x2, /**< 0x2 - Invalid ImgID passed
						in Command */
	XLOADER_ERR_NO_VALID_IMG_FOUND,	 /**< 0x3 - No Valid Image Found
						in the Image Info Table */
	XLOADER_ERR_IMAGE_INFO_TBL_FULL, /**< 0x4 - Image Info Table is Full */
	XLOADER_ERR_PARENT_QUERY_RELATION_CHECK, /**< 0x5 - Error on Parent Query while checking
							for Child Relation */
	XLOADER_ERR_MEMSET_BOOT_HDR_FW_RSVD, /**< 0x6 - Error during memset on
							XilPdi_BootHdrFwRsvd */
	XLOADER_ERR_MEMSET_PDIPTR, /**< 0x7 - Error during memset on
							PdiPtr */
	XLOADER_ERR_MEMSET_QSPI_PSU_INST, /**< 0x8 - Error during memset on
							QspiPsuInstance */
	XLOADER_ERR_MEMSET_SD_BOOT_FILE, /**< 0x9 - Error during memset on
							SD BootFile */
	XLOADER_ERR_MEMSET_SD_INSTANCE, /**< 0xA - Error during memset on
							SdInstance */
	XLOADER_ERR_MEMSET_USB_INSTANCE, /**< 0xB - Error during memset on
							UsbInstance */
	XLOADER_ERR_MEMSET_USB_PRIVATE_DATA, /**< 0xC - Error during memset on
							UsbPrivateData */
	XLOADER_ERR_MEMSET_DFU_OBJ, /**< 0xD - Error during memset on
							DfuObj */
};

/* Multiboot register offset mask */
#define XLOADER_MULTIBOOT_OFFSET_MASK		(0x001FFFFFU)

/* SD RAW BOOT related macros */
#define XLOADER_SD_RAWBOOT_MASK			(0xF0000000U)
#define XLOADER_SD_RAWBOOT_VAL			(0x70000000U)
#define XLOADER_EMMC_BP1_RAW_VAL		(0x10000000U)
#define XLOADER_EMMC_BP2_RAW_VAL		(0x20000000U)

/* Minor Error codes for Major Error code: XLOADER_ERR_GEN_IDCODE */
#define XLOADER_ERR_IDCODE		(0x1) /* IDCODE mismatch */
#define XLOADER_ERR_EXT_IDCODE		(0x2) /* EXTENDED IDCODE mismatch */
#define XLOADER_ERR_EXT_ID_SI		(0x3) /* Invalid combination of
						* EXTENDED IDCODE - Device
						*/

#define XLOADER_AUTH_JTAG_INT_STATUS_POLL_INTERVAL	(1000U)
/**************************** Type Definitions *******************************/
/*
 * This stores the handoff Address of the different cpu's
 */
typedef struct {
	u32 CpuSettings;
	u64 HandoffAddr;
} XLoader_HandoffParam;

typedef struct {
	char *Name; /**< Source name */
	u32 DeviceBaseAddr; /**< Flash device base address */
	int (*Init) (u32 DeviceFlags); /**< Function pointer for Device
				initialization code */
	/**< Function pointer for device copy */
	int (*Copy) (u64 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);
	/**< Function pointer for device release */
	int (*Release) (void);
} XLoader_DeviceOps;

/*
 * This is PDI instance pointer. This stores all the information
 * required for PDI
 */
typedef struct {
	u8 PdiType; /**< Indicate PDI Type, full PDI, partial PDI */
	PdiSrc_t PdiSrc; /**< Source of the PDI - Boot device, DDR */
	u64 PdiAddr; /**< Address where PDI is present in PDI Source */
	u32 PdiId; /**< Indicates the full PDI Id */
	XilPdi_MetaHdr MetaHdr; /**< Metaheader of the PDI */
	int (*DeviceCopy) (u64 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);
	u32 NoOfHandoffCpus; /**< Number of CPU's loader will handoff to */
	XLoader_HandoffParam HandoffParam[XLOADER_MAX_HANDOFF_CPUS];
	u32 CurImgId; /**< Current Processing image ID */
	u32 CurPrtnId; /**< Current Processing Partition ID */
	u32 IpiMask; /**< Info about which master has sent the request*/
	u32 ImageNum; /**< Image number in the PDI */
	u32 PrtnNum; /**< Partition number in the PDI */
	u32 SlrType; /**< SLR Type */
	u32 CopyToMem; /**< Copy to Memory is enabled if set */
	u32 DelayHandoff; /**< Delay handoff is enabled if set */
	u32 PlmKatStatus; /**< PLM Known Answer Test Status */
	u32 KekStatus; /**< KEK status flag */
	u32 DelayLoad; /**< Delay Load is enabled if set */
	u64 CopyToMemAddr; /**< Address to which image is copied */
} XilPdi;

/* Structure to store various attributes required for IDCODEs checks */
typedef struct {
	u32 IdCodeIHT; /**< IdCode as read from IHT */
	u32 ExtIdCodeIHT; /**< Extended IdCode as read from IHT */
	u32 IdCodeRd; /**< IdCode as read from Device */
	u32 ExtIdCodeRd; /**< Extended IdCode as read from Device */
	u8 BypassChkIHT; /**< Flag to bypass checks */
	u8 IsVC1902Es1; /**< Flag to indicate IsVC1902-ES1 device */
	u8 IsExtIdCodeZero; /**< Flag to indicate Extended IdCode is valid */
} XLoader_IdCodeInfo __attribute__ ((aligned(16U)));

/* Structure to store various parameters for Device Copy */
typedef struct {
	u64 SrcAddr;	/**< Source Address */
	u64 DestAddr;	/**< Desrination Address */
	u32 Len;	/**< Number of bytes to be copied */
	u32 Flags;	/**< Flags indicate mode of copying */
	u8 IsDoubleBuffering;	/**< Indicates if parallel DMA is allowed or not */
} XLoader_DeviceCopy;

/* Structure to store various parameters for processing partitions */
typedef struct {
	XLoader_DeviceCopy DeviceCopy; /**< Device Copy instance */
	u32 DstnCpu;	/** < Destination Cpu */
} XLoader_PrtnParams;

typedef struct {
	u32 ImgID; /**< Image ID */
	u32 UID; /**< Unique ID */
	u32 PUID; /**< Parent UID */
	u32 FuncID; /**< Function ID */
} XLoader_ImageInfo;

typedef struct {
	XLoader_ImageInfo *TblPtr;
	u32 Count;
	u8 IsBufferFull;
} XLoader_ImageInfoTbl;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XLoader_Init(void);
int XLoader_LoadPdi(XilPdi* PdiPtr, PdiSrc_t PdiSrc, u64 PdiAddr);
int XLoader_RestartImage(u32 ImageId, u32 *FuncID);
int XLoader_CframeErrorHandler(u32 ImageId);
int XLoader_CframeInit(void);
void XLoader_SetATFHandoffParameters(const XilPdi_PrtnHdr *PrtnHdr);
int XLoader_StoreImageInfo(XLoader_ImageInfo *ImageInfo);
XLoader_ImageInfo* XLoader_GetImageInfoEntry(u32 ImgID);
int XLoader_LoadImageInfoTbl(u64 DestAddr, u32 MaxSize, u32 *NumEntries);

/* Functions defined in xloader_prtn_load.c */
int XLoader_LoadImagePrtns(XilPdi* PdiPtr);
int XLoader_UpdateHandoffParam(XilPdi* PdiPtr);

/* Functions defined in xloader_cmds.c */
void XLoader_CmdsInit(void);

/* Functions defined in xloader_intr.c */
int XLoader_IntrInit(void);
void XLoader_SbiRecovery(void);
void XLoader_ClearIntrSbiDataRdy(void);

/************************** Variable Definitions *****************************/
extern XilPdi* BootPdiPtr;
extern XilPdi SubsystemPdiIns;

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_H */
