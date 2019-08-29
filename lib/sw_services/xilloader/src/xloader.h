/******************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
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
* 1.00  kc   07/24/2018 Initial release
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
#include "xplmi_hw.h"
#include "xplmi_status.h"
#include "xplmi_debug.h"
#include "xloader_ospi.h"
#include "xloader_sd.h"
#include "xloader_sbi.h"
#include "xloader_qspi.h"
#include "xloader_ddr.h"
#include "xplmi_dma.h"
#include "xpm_device.h"
#include "xcfupmc.h"
#include "xcframe.h"
#include "xplmi_proc.h"
/************************** Constant Definitions *****************************/
#define XLOADER_SUCCESS		XST_SUCCESS
#define XLoader_Printf		XPlmi_Printf
#define XLOADER_32BIT_MASK	(0xFFFFFFFFU)
#define PMC_LOCAL_BASEADDR	(0xF0000000U)
#define XLOADER_CHUNK_MEMORY		(XPLMI_PMCRAM_BASEADDR)
#define XLOADER_CHUNK_SIZE			(0x10000U) /** 64K */
#define XLOADER_CFI_CHUNK_SIZE		(0x40000U) /** 256K */
#define XLOADER_DMA_LEN_ALIGN           (0x10U)
#define XLOADER_IMAGE_SEARCH_OFFSET	(0x8000U) /** 32K */

#define XLOADER_R5_0_TCMA_BASE_ADDR			(0xFFE00000U)
#define XLOADER_R5_0_TCMB_BASE_ADDR			(0xFFE20000U)
#define XLOADER_R5_1_TCMA_BASE_ADDR			(0xFFE90000U)
#define XLOADER_R5_1_TCMB_BASE_ADDR			(0xFFEB0000U)
#define XLOADER_R5_TCMA_SIZE				(0x10000U)
#define XLOADER_R5_TCMB_SIZE				(0x10000U)
#define XLOADER_CRX_RPU_1_BASE_OFFSET   	(0x100U)

/**
 * TCM address for R5
 */
#define XLOADER_R5_TCMA_LOAD_ADDRESS	(u32)(0x0U)
#define XLOADER_R5_TCMB_LOAD_ADDRESS	(0x20000U)
#define XLOADER_R5_TCM_BANK_LENGTH		(0x10000U)

/*
 * APU related macros
 */
#define XLOADER_FPD_APU_CONFIG_0		0xFD5C0020U

#define XLOADER_FPD_APU_CONFIG_0_AA64N32_MASK_CPU0	0x1U
#define XLOADER_FPD_APU_CONFIG_0_AA64N32_MASK_CPU1	0x2U
#define XLOADER_FPD_APU_CONFIG_0_VINITHI_MASK_CPU0	0x100U
#define XLOADER_FPD_APU_CONFIG_0_VINITHI_MASK_CPU1	0x200U

/*
 * Subsystem related macros
 */
#define XLOADER_MAX_SUBSYSTEMS	10U
#define XLOADER_RUNNING_CPU_SHIFT	(0x8U)

/*
 * PDI type macros
 */
#define XLOADER_PDI_TYPE_FULL			0x1U
#define XLOADER_PDI_TYPE_PARTIAL		0x2U
/*
 * Secondary boot mode related macros
 */
#define XLOADER_PDISRC_FLAGS_MASK		(0xFU)
#define XLOADER_SBD_ADDR_SET_MASK		(0x10U)
#define XLOADER_SBD_ADDR_SHIFT			(0x5U)
/*
 * PDI Loading status
 */
#define XLOADER_PDI_LOAD_COMPLETE		(0x1U)
#define XLOADER_PDI_LOAD_STARTED		(0x0U)

/*
 * SLR Types
 */
#define XLOADER_SSIT_MONOLITIC				(0x7U)
#define XLOADER_SSIT_MASTER_SLR				(0x6U)
#define XLOADER_SSIT_SLAVE_SLR1_TOP_SLR		(0x5U)
#define XLOADER_SSIT_SLAVE_SLR1_NON_TOP_SLR	(0x4U)
#define XLOADER_SSIT_SLAVE_SLR2_TOP_SLR		(0x3U)
#define XLOADER_SSIT_SLAVE_SLR2_NON_TOP_SLR	(0x2U)
#define XLOADER_SSIT_SLAVE_SLR3_TOP_SLR		(0x1U)

/* Boot Modes */
enum XLOADER_PDI_SRC {
	XLOADER_PDI_SRC_JTAG = (0x0U),
	XLOADER_PDI_SRC_QSPI24 = (0x1U),
	XLOADER_PDI_SRC_QSPI32 = (0x2U),
	XLOADER_PDI_SRC_SD0 =    (0x3U),
	XLOADER_PDI_SRC_SD1 =    (0x5U),
	XLOADER_PDI_SRC_EMMC =   (0x6U),
	XLOADER_PDI_SRC_USB =    (0x7U),
	XLOADER_PDI_SRC_OSPI =   (0x8U),
	XLOADER_PDI_SRC_SMAP =   (0xAU),
	XLOADER_PDI_SRC_SD1_LS = (0xEU),
	XLOADER_PDI_SRC_SBI = (0x10U),
	XLOADER_PDI_SRC_PCIE = (0x11U)
};

/* Multiboot register offset mask */
#define XLOADER_MULTIBOOT_OFFSET_MASK    	(0x001FFFFF)

/* Minor Error codes for Major Error code: XLOADER_ERR_IDCODE */
#define XLOADER_ERR_IDCODE			(0x1U) /**< IDCODE mismatch */
#define XLOADER_ERR_EXT_IDCODE		(0x2U) /**< EXTENDED IDCODE mismatch */
#define XLOADER_ERR_EXT_ID_SI		(0x3U) /**< Invalid combination of
										EXTENDED IDCODE - Device */

/**************************** Type Definitions *******************************/

/**
 * This stores the handoff Address of the different cpu's
 */
typedef struct {
        u32 CpuSettings;
        u64 HandoffAddr;
} XLoader_HandoffParam;

typedef struct {
	char *Name; /**< Source name */
	u32 DeviceBaseAddr; /**< Flash device base address */
	int (*Init) (u32 DeviceFlags);
		/**< Function pointer for Device initialization code */
	XStatus (*Copy) (u32 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);
		/**< Function pointer for device copy */
} XLoader_DeviceOps;


/**
 * This is PDI instance pointer. This stores all the information
 * required for PDI
 */
typedef struct {
	u32 PdiType; /**< Indicate PDI Type, full PDI, partial PDI */
	u32 PdiSrc; /**< Source of the PDI - Boot device, DDR */
	u64 PdiAddr; /**< Address where PDI is present in PDI Source */
	u32 PdiId; /**< Indicates the full PDI Id */
	XilPdi_MetaHdr MetaHdr; /**< Metaheader of the PDI */
	XStatus (*DeviceCopy) (u32, u64, u32, u32);
	u32 NoOfHandoffCpus; /**< Number of CPU's loader will handoff to */
    XLoader_HandoffParam HandoffParam[10];
	u32 CurImgId; /**< Current Processing image ID */
	u32 CurPrtnId; /**< Current Processing Partition ID */
	u32 ImageNum; /**< Image number in the PDI */
	u32 PrtnNum; /**< Partition number in the PDI */
	u32 SlrType; /**< SLR Type */
} XilPdi;

/**
 * This stores all the information required for Subsystem
 */
typedef struct {
	u32 SubsystemId; /**< Corresponding subsystem ID */
	u32 ImageId; /**< Corresponding Image ID in the PDI */
	u32 ImageNum; /**< Corresponding Image Number in the PDI */
	u32 PrtnNum; /**< Corresponding Partition Number in the PDI */
} XilSubsysInfo;

/**
 * This is a subsystem instance pointer. This stores all the information
 * required for subsystem along with subsystem count
 */
typedef struct {
	XilSubsysInfo SubsystemLut[XLOADER_MAX_SUBSYSTEMS]; /**< Subsystem lookup table */
	XilPdi *PdiPtr; /**< PDI source for that Subsystem */
	u32 Count; /**< Subsystem count */
} XilSubsystem;

/* Structure to store various attributes required for IDCODEs checks */

typedef struct {
	u32 IdCodeIHT; /**< IdCode as read from IHT */
	u32 ExtIdCodeIHT; /**< Extended IdCode as read from IHT */
	u32 IdCodeRd; /**< IdCode as read from Device */
	u32 ExtIdCodeRd; /**< Extended IdCode as read from Device */
	u32 BypassChkIHT; /**< Flag to bypass checks */
	u32 IsVC1902Es1; /**< Flag to indicate IsVC1902-ES1 device */
	u32 IsExtIdCodeZero; /**< Flag to indicate Extended IdCode is valid */
} XLoader_IdCodeInfo __attribute__ ((aligned(16)));

/***************** Macros (Inline Functions) Definitions *********************/
#define XLoader_GetBootMode()	XPlmi_In32(CRP_BOOT_MODE_USER) & \
				CRP_BOOT_MODE_USER_BOOT_MODE_MASK

#define XLoader_IsJtagSbiMode()	((XPlmi_In32(SLAVE_BOOT_SBI_MODE) & \
				SLAVE_BOOT_SBI_MODE_JTAG_MASK) == \
				    SLAVE_BOOT_SBI_MODE_JTAG_MASK) ? \
					(TRUE) : (FALSE)

/************************** Function Prototypes ******************************/
extern XilPdi SubsystemPdiIns;
#if 0
int XSubSys_CopyPdi(u32 PdiSrc, u64 SrcAddr, u64 DestAddr, u32 PdiLen);
#endif

int XLoader_Init();
int XLoader_PdiInit(XilPdi* PdiPtr, u32 PdiSrc, u64 PdiAddr);
int XLoader_LoadAndStartSubSystemPdi(XilPdi *PdiPtr);
int XLoader_LoadPdi(XilPdi* PdiPtr, u32 PdiSrc, u64 PdiAddr);
int XLoader_LoadImage(XilPdi *PdiPtr, u32 ImageId);
int XLoader_StartImage(XilPdi *PdiPtr);
int XLoader_RestartImage(u32 ImageId);
int XLoader_ReloadImage(u32 ImageId);
XStatus XLoader_IdCodeCheck(XilPdi_ImgHdrTable * ImgHdrTable);
void XLoader_A72Config(u32 CpuId, u32 ExecState, u32 VInitHi);
void XLoader_ClearIntrSbiDataRdy();
void XLoader_CfiErrorHandler(void);
int XLoader_CframeInit();

/* functions defined in xloader_prtn_load.c */
int XLoader_LoadImagePrtns(XilPdi* PdiPtr, u32 ImgNum, u32 PrtnNum);

/** Functions defined in xloader_cmds.c */
void XLoader_CmdsInit(void);

/** Functions defined in xloader_intr.c */
int XLoader_IntrInit();
int XLoader_SbiLoadPdi(void *Data);
void XLoader_SbiRecovery();
#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_H */
