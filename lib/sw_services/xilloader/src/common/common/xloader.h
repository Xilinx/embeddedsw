/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
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
*       ana  10/19/2020 Added doxygen comments
* 1.03  bm   12/15/2020 Added XLOADER_SD_MAX_BOOT_FILES_LIMIT macro
*       ma   01/12/2021 Added macro for invalid boot mode
*                       Added macro for invalid SLR Type
*                       Added function for PMC State clear
*       bm   03/16/2021 Added Image Upgrade support
*       bm   05/10/2021 Added additional macro for PDI version
* 1.04  bm   07/16/2021 Updated XLoader_PdiInit prototype
*       bsv  07/19/2021 Disable UART prints when invalid header is encountered
*                       in slave boot modes
*       bsv  08/02/2021 Updated function return type as part of code clean up
*       bm   08/09/2021 Removed obsolete XLoader_PMCStateClear API prototype
*       bm   08/24/2021 Added Extract Metaheader support
*       bm   08/26/2021 Added XLOADER_PDI_TYPE_PARTIAL_METAHEADER macro
*       bsv  08/31/2021 Code clean up
*       ma   09/01/2021 Use SSIT defines from XilPlmi
*       kpt  09/06/2021 Added macro XLOADER_TOTAL_CHUNK_SIZE
*       bsv  10/26/2021 Code clean up
*       bsv  03/17/2022 Add support for A72 elfs to run from TCM
* 1.05  ma   06/21/2022 Add support for Get Handoff Parameters IPI command
*       sk   06/24/2022 Removed bit field from XLoader_ImageInfoTbl to avoid
*                       compiler or portability issues
*       bm   07/06/2022 Refactor versal and versal_net code
*       kpt  07/05/22 Added PpdiKatStatus for partial pdi KAT
*       bm   07/13/2022 Added compatibility check for In-Place PLM Update
*       bm   07/13/2022 Retain critical data structures after In-Place PLM Update
*       ma   07/27/2022 Added support for CFrame data clear check which is
*                        required during PL secure lockdown
*       kal  01/05/2023 Added XLOADER_PCR_INVALID_VALUE macro
* 1.06  sk   01/11/2023 Updated XLoader_ImageStore structure to support
*                       image store feature
*       bm   02/22/2023 Fix XLOADER_MAX_HANDOFF_CPUS value for versal Net
*       sk   03/17/2023 Renamed member Kekstatus to DecKeySrc in xilpdi structure
*       sk   04/28/2023 Added function to retrieve PDI Address from Image Store
*                       based on PDI ID
*       sk   05/18/2023 Deprecate copy to memory feature,removed SubsystemPdiIns,
*                       Added BootPdiInfo Structure,Added XLoader_GetPdiInstance
*                       function declaration
*       sk   07/06/2023 Added prototype for Enable and Disable Jtag functions
*                       Moved minor error codes to plat headers
*       sk   08/18/2023 Renamed ValidHeader member to DiscardUartLogs in XilPdi
*       dd   09/11/2023 MISRA-C violation Rule 17.8 fixed
* 2.00  kpt  02/21/2024 Update prototype for XLoader_EnableJtag and XLoader_DisableJtag
*       mss  03/06/2024 Added ClearAtfHandoff member in XilPdi structure and
*                       ClearATFHandoffParams function prototype
*       har  03/12/2024 Fixed doxygen warnings
*       kal  06/04/2024 Added ImagePrtnId to XilPdi structure
*       mss  04/05/2024 Added XLOADER_DEVICE_COPY_OPTIMIZATION_MASK macro
*
* </pre>
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
#include "xloader_defs.h"

/************************** Constant Definitions *****************************/
#define XLOADER_SUCCESS		(u32)XST_SUCCESS	/**< Alias for XST_SUCCESS */
#define XLOADER_FAILURE		(u32)XST_FAILURE	/**< Alias for XST_FAILURE */
#define XLoader_Printf		XPlmi_Printf		/**< Alias for XPlmi_Printf */
#define XLOADER_DDR_TEMP_BUFFER_ADDRESS	(0x50000000U)	/**< Fixed DFU temporary address in DDR*/
#define XLOADER_CHUNK_SIZE		(0x10000U) 	/**< 64K */
#define XLOADER_TOTAL_CHUNK_SIZE    (XLOADER_CHUNK_SIZE + 0x100U)	/**< Total chunk size */
#define XLOADER_DMA_LEN_ALIGN		(0x10U)		/**< 16 bytes alignment*/
#define XLOADER_DMA_LEN_ALIGN_MASK	(0xFU)		/**< Mask for 16 bytes alignment */
#define XLOADER_IMAGE_SEARCH_OFFSET	(0x8000U)	/**< 32K */
#define XLOADER_DEVICE_COPY_OPTIMIZATION_MASK	(0x30U) /** Mask to extract copy optimization bits of RTCA SecureCtrl registers */

/**
 * @name  Subsystem related macros
 * @{
 */
/**< Subsystem related macros */
#ifdef VERSAL_NET
/* Limit to the maximum number of partitions possible */
#define XLOADER_MAX_HANDOFF_CPUS	(XIH_MAX_PRTNS)
#else
#define XLOADER_MAX_HANDOFF_CPUS	(10U)
#endif
/** @} */

/**
 * @name  Macros for different PDI types
 * @{
 */
/**< Different PDI types */
#define XLOADER_PDI_TYPE_FULL		(0x1U)
#define XLOADER_PDI_TYPE_PARTIAL	(0x2U)
#define XLOADER_PDI_TYPE_RESTORE	(0x3U)
#define XLOADER_PDI_TYPE_FULL_METAHEADER 	(0x4U)
#define XLOADER_PDI_TYPE_PARTIAL_METAHEADER 	(0x5U)
#define XLOADER_PDI_TYPE_IPU	(0x6U)
/** @} */

/**
 * @name  SD boot
 * @{
 */
/**< SD boot related macros */
#define XLOADER_PDISRC_FLAGS_MASK	(0xFU)
#define XLOADER_PDISRC_FLAGS_SHIFT	(0x4U)
/** @} */

/**
 * @name  Status of PDI Load
 * @{
 */
/**< Status of PDI load */
#define XLOADER_PDI_LOAD_COMPLETE	(0x1U)
#define XLOADER_PDI_LOAD_STARTED	(0x0U)
/** @} */

/**
 * @name  SLR Types
 * @{
 */
/**< SLR Types */
#define XLOADER_SSIT_MONOLITIC		XPLMI_SSIT_MONOLITIC
#define XLOADER_SSIT_MASTER_SLR		XPLMI_SSIT_MASTER_SLR
#define XLOADER_SSIT_INVALID_SLR	XPLMI_SSIT_INVALID_SLR
/** @} */

/**
 * @name  Flash size
 * @{
 */
/**< Flash size */
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
/** @} */

/**
 * @name PDI version
 * @{
 */
/**< PDI version */
#define XLOADER_PDI_VERSION_1			(0x01030000U)
#define XLOADER_PDI_VERSION_2			(0x00020000U)
#define XLOADER_PDI_VERSION_4			(0x00040000U)
/** @} */

#define XLOADER_INVALID_IMG_ID			(0x0U)	/**< Invalid Img ID */

#define XLOADER_INVALID_UID			(0x0U)	/**< Invalid UID */

#define XLOADER_IMG_INDEX_NOT_FOUND		(0xFFFFFFFFU)	/**< Image Index Not found */
#define XLOADER_IMG_STORE_INVALID_ADDR		(0xFFFFFFFFFFFFFFFFUL)	/**< Invalid address in Image Store */
#define XLOADER_IMG_STORE_INVALID_SIZE		(0x0U)	/**< Invalid size in Image Store */

/**
 * @name PDI Src Index
 * @{
 */
/**< PDI Src index */
#define XLOADER_SBI_INDEX		(0U)
#define XLOADER_QSPI_INDEX		(1U)
#define XLOADER_SD_INDEX		(2U)
#define XLOADER_SD_RAW_INDEX		(3U)
#define XLOADER_DDR_INDEX		(4U)
#define XLOADER_OSPI_INDEX		(5U)
#define XLOADER_USB_INDEX		(6U)
#define XLOADER_INVALID_INDEX		(7U)
/** @} */

#define PdiSrc_t			u32	/**< Alias for u32 */

typedef struct {
	const char* Name;	/**< Name of PDI source */
	u8 Index;		/**< Index of PDI source */
} PdiSrcMap;

#define XLOADER_MULTIBOOT_OFFSET_MASK		(0x001FFFFFU)	/**< Mask for multiboot offset */

/**
 * @name SD Raw boot
 * @{
 */
/**< SD Raw boot */
#define XLOADER_SD_RAWBOOT_MASK			(0xF0000000U)
#define XLOADER_SD_RAWBOOT_VAL			(0x70000000U)
#define XLOADER_EMMC_BP1_RAW_VAL		(0x10000000U)
#define XLOADER_EMMC_BP2_RAW_VAL		(0x20000000U)
/** @} */

#define XLOADER_SD_MAX_BOOT_FILES_LIMIT		(8192U)
			/**< Maximum limit of boot files in SD filesystem */

#define XLOADER_AUTH_JTAG_INT_STATUS_POLL_INTERVAL	(1000U)
			/**< Polling interval for checking Authenticated JTAG interrupt status*/
#define XLOADER_SHA3_LEN				(48U)	/**< SHA3 hash length in bytes */

#define XLOADER_MAX_PDI_LIST		(32U)	/**< Maximum number of PDIs */

#if defined(XLOADER_SD_0) || defined(XLOADER_SD_1)
#define XLOADER_SD_ADDR_MASK		(0xFFFFU)
#define XLOADER_SD_ADDR_SHIFT		(0x4U)
#endif

/**************************** Type Definitions *******************************/
/*
 * This stores the handoff Address of the different cpu's
 */
typedef struct {
	u32 CpuSettings;	/**< CPU settings */
	u64 HandoffAddr;	/**< Handoff address for CPU */
} XLoader_HandoffParam;

typedef struct {
	int (*Init) (u32 DeviceFlags); /**< Function pointer for Device
				initialization code */
	int (*Copy) (u64 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);
		/**< Function pointer for device copy */
	int (*Release) (void);	/**< Function pointer for device release */
} XLoader_DeviceOps;

/*
 * This is PDI instance pointer. This stores all the information
 * required for PDI
 */
typedef struct {
	u8 PdiType; /**< Indicates PDI Type, full PDI, partial PDI */
	u8 DiscardUartLogs; /**< Used to control uart logs */
	u8 PdiIndex; /**< Index in DeviceOps array */
	u8 SlrType; /**< SLR Type */
	u32 PdiSrc; /**< PDI source */
	XilPdi_MetaHdr MetaHdr; /**< Metaheader of the PDI */
	XLoader_HandoffParam HandoffParam[XLOADER_MAX_HANDOFF_CPUS];	/**< Handoff Param for each CPU */
	u32 IpiMask; /**< Info about which master has sent the request*/
	u32 ClearAtfHandoff; /**< Flag to clear ATF Handoff params before loading
					    APU Subsystem Partition */
	u8 NoOfHandoffCpus; /**< Number of CPU's loader will handoff to */
	u8 ImageNum; /**< Image number in the PDI */
	u8 PrtnNum; /**< Partition number in the PDI */
	u8 DelayHandoff; /**< Delay handoff is enabled if set */
	u8 DelayLoad; /**< Delay Load is enabled if set */
#ifndef PLM_SECURE_EXCLUDE
	u32 PlmKatStatus; /**< PLM Known Answer Test Status */
	u32 DecKeySrc; /**< Decryption Key Source */
	u32 PpdiKatStatus; /**< PPDI Known Answer Test Status */
#endif
	u32 DigestIndex; 	/**< Digest index for data measurement */
	u32 ImagePrtnId; /**< Partition number in the Image */
} XilPdi;

/*
* This BootPDI info struct is to store required config
* for later use.
*/
typedef struct {
	u32 PdiSrc; /**< Pdi Source */
	u32 MetaHdrOfst; /**< Offset to the start of meta header */
#ifndef PLM_SECURE_EXCLUDE
	u32 PlmKatStatus; /**< PLM Known Answer Test Status */
	u32 DecKeySrc; /**< Decryption Key Source */
#endif
}XilBootPdiInfo;

/* Structure to store various parameters for Device Copy */
typedef struct {
	u64 SrcAddr;	/**< Source Address */
	u64 DestAddr;	/**< Desrination Address */
	u32 Len;	/**< Number of bytes to be copied */
	u32 Flags;	/**< Flags indicate mode of copying */
} XLoader_DeviceCopy;

/* Structure to store various parameters for processing partitions */
typedef struct {
	XLoader_DeviceCopy DeviceCopy; /**< Device Copy instance */
	u32 DstnCpu;	/**< Destination Cpu */
} XLoader_PrtnParams;

typedef struct {
	u32 Count;	/**< Number of images */
	u8 IsBufferFull;	/**< Flag to indicate if buffer is full */
} XLoader_ImageInfoTbl;

typedef struct {
	u32 PdiId;	/**< PDI ID */
	u64 PdiAddr;	/**< Address of PDI */
} XLoader_PdiInfo;

typedef struct {
	u64 PdiImgStrAddr; /**< Image Store address */
	u32 PdiImgStrSize;/**< Image Store Memory Size */
	XLoader_PdiInfo ImgList[XLOADER_MAX_PDI_LIST + 1];	/**< List of Images in Image Store */
	u8 Count;	/**< Number of images in Image Store */
} XLoader_ImageStore;

typedef struct {
	u64 DataAddr;	/**< Address of data to be extended to PCR */
	u32 DataSize;	/**< Size of data to be extended to PCR */
	u32 PcrInfo;	/**< PCR Info which included PCR number and PCR measurement index */
	u32 *DigestIndex;	/**< Digest Index in the SW PCR log */
	u32 Flags;	/**< Flags to indicate state of hash calculation - start, update, finish */
	u32 SubsystemID;	/**< Subsystem ID */
	u32 OverWrite;	/**< Flag to indicate if digest can be overwritten at the measurement index */
} XLoader_ImageMeasureInfo;

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
 * @brief	This function reads the boot mode register and returns the
 * 			boot source
 *
 * @return	Boot Source
 *
 *****************************************************************************/
static inline PdiSrc_t XLoader_GetBootMode(void)
{
	u32 BootMode;

	BootMode = (XPlmi_In32(CRP_BOOT_MODE_USER) &
				CRP_BOOT_MODE_USER_BOOT_MODE_MASK);

	return (PdiSrc_t)BootMode;
}


/************************** Function Prototypes ******************************/
int XLoader_Init(void);
XilPdi *XLoader_GetPdiInstance(void);
/**
 * @}
 * @endcond
 */
int XLoader_LoadPdi(XilPdi* PdiPtr, PdiSrc_t PdiSrc, u64 PdiAddr);
/**
 * @{
 * @cond xloader_internal
 */
int XLoader_RestartImage(u32 ImageId, u32 *FuncID);
void XLoader_CframeErrorHandler(u32 ImageId);
int XLoader_CframeInit(void);
int XLoader_CframeDataClearCheck(XPlmi_Cmd *Cmd);
void XLoader_SetATFHandoffParameters(const XilPdi_PrtnHdr *PrtnHdr);
XLoader_ImageInfo* XLoader_GetImageInfoEntry(u32 ImgID);
int XLoader_LoadImageInfoTbl(u64 DestAddr, u32 MaxSize, u32 *NumEntries);
int XLoader_PdiInit(XilPdi* PdiPtr, PdiSrc_t PdiSource, u64 PdiAddr);
int XLoader_ReadImageStoreCfg(void);
int XLoader_IsPdiAddrLookup(u32 PdiId, u64 *PdiAddr);
int XLoader_ClearATFHandoffParams(XilPdi* PdiPtr);
int XLoader_LoadImage(XilPdi *PdiPtr);
int XLoader_GetImageAndPrtnInfo(XilPdi *PdiPtr, u32 ImageId);

/* Functions defined in xloader_prtn_load.c */
int XLoader_LoadImagePrtns(XilPdi* PdiPtr);
int XLoader_PrtnCopy(const XilPdi* PdiPtr, const XLoader_DeviceCopy* DeviceCopy,
	void* SecureParamsPtr);

/* Functions defined in xloader_cmds.c */
void XLoader_CmdsInit(void);

/* Functions defined in xloader_intr.c */
int XLoader_IntrInit(void);
void XLoader_ClearIntrSbiDataRdy(void);
XilPdi_ATFHandoffParams *XLoader_GetATFHandoffParamsAddr(void);
int XLoader_IdCodeCheck(const XilPdi_ImgHdrTbl * ImgHdrTbl);
void Xloader_SaveBootPdiInfo(XilPdi *BootPdiPtr);
int XLoader_EnableJtag(u32 CfgState);
int XLoader_DisableJtag(void);
int XLoader_InitPdiInstanceForExtractMHAndOptData(XPlmi_Cmd* Cmd, XilPdi* PdiPtr, u64 SrcAddr, u64 DestAddr, u32 DestSize);

/************************** Variable Definitions *****************************/
#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_H */

/**
 * @}
 * @endcond
 */

/** @} */
