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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
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
/************************** Constant Definitions *****************************/
#define XLOADER_SUCCESS		XST_SUCCESS
#define XLoader_Printf		XPlmi_Printf
#define XLOADER_32BIT_MASK	(0xFFFFFFFFU)
#define PMC_LOCAL_BASEADDR	(0xF0000000U)
#define XLOADER_CHUNK_MEMORY		(XPLMI_PMCRAM_BASEADDR)
#define XLOADER_CHUNK_SIZE			(0x10000U) /** 64K */
#define XLOADER_CFI_CHUNK_SIZE		(0x40000U) /** 256K */
#define XLOADER_DMA_LEN_ALIGN           (0x10U)

#define XLOADER_CRX_RPU_1_BASE_OFFSET	0x100U

#define RPU_RPU_0_CFG					0xFF9A0100U
#define RPU_RPU_0_CFG_NCPUHALT_MASK		0X00000001U

#define XLOADER_R5_0_TCMA_BASE_ADDR		0xFFE00000U
#define XLOADER_R5_1_TCMA_BASE_ADDR		0xFFE90000U

/**
 * TCM address for R5
 */
#define XLOADER_R5_TCM_START_ADDRESS	(u32)(0x0U)
#define XLOADER_R5_BTCM_START_ADDRESS	(0x20000U)
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
 * PDI Loading status
 */
#define XLOADER_PDI_LOAD_COMPLETE		(0x1U)
#define XLOADER_PDI_LOAD_STARTED		(0x0U)

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
	XLOADER_PDI_SRC_SBI_JTAG = (0x10U)
};

/**************************** Type Definitions *******************************/

/**
 * This stores the handoff Address of the different cpu's
 */
typedef struct {
        u32 CpuSettings;
        u64 HandoffAddr;
} XLoader_HandoffParam;

typedef struct {
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
    u32 CpusRunning; /** CPUs that are already running */
	u32 EccStatus;
	u32 CurImgId; /**< Current Processing image ID */
	u32 CurPrtnId; /**< Current Processing Partition ID */
	u32 ImageNum; /**< Image number in the PDI */
	u32 PrtnNum; /**< Partition number in the PDI */
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

/***************** Macros (Inline Functions) Definitions *********************/
#define XLoader_GetBootMode()	XPlmi_In32(CRP_BOOT_MODE_USER) & \
				CRP_BOOT_MODE_USER_BOOT_MODE_MASK

#define XLoader_IsJtagSbiMode()	((XPlmi_In32(SLAVE_BOOT_SBI_MODE) & \
				SLAVE_BOOT_SBI_MODE_JTAG_MASK) == \
				    SLAVE_BOOT_SBI_MODE_JTAG_MASK) ? \
					(TRUE) : (FALSE)

/************************** Function Prototypes ******************************/
#if 0
int XSubSys_Init(u32 * CdoBuf);
int XSubSys_LoadPdi(XilPdi* PdiPtr, u32 PdiSrc, u64 PdiAddr);
int XSubSys_CopyPdi(u32 PdiSrc, u64 SrcAddr, u64 DestAddr, u32 PdiLen);
int XSubSys_ReStart(u32 SubsysHd);
int XLoader_StartImage(XilPdi *PdiPtr, u32 ImageId);
#endif

int XLoader_Init();
int XLoader_PdiInit(XilPdi* PdiPtr, u32 PdiSrc, u64 PdiAddr);
int XLoader_LoadAndStartSubSystemPdi(XilPdi *PdiPtr);
int XLoader_LoadImage(XilPdi *PdiPtr, u32 ImageId);
int XLoader_StartImage(XilPdi *PdiPtr);
int XLoader_RestartImage(u32 ImageId);
int XLoader_ReloadImage(u32 ImageId);
void XLoader_A72Config(u32 CpuId, u32 ExecState, u32 VInitHi);

/* functions defined in xloader_prtn_load.c */
int XLoader_LoadImagePrtns(XilPdi* PdiPtr, u32 ImgNum, u32 PrtnNum);

/** Functions defined in xloader_cmds.c */
void XLoader_CmdsInit(void);
#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_H */
