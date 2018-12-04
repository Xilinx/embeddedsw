/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmcfw_main.h
*
* This is the main header file which contains definitions for the PMCFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPMCFW_MAIN_H
#define XPMCFW_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xpmcfw_default.h"
#include "xpmcfw_crx.h"
#include "xpmcfw_iomodule.h"
/************************** Constant Definitions *****************************/
#define XPMCFW_MULTIBOOT_OFFSET_MASK	(0x001FFFFF)
/**************************** Type Definitions *******************************/

/**
 * This stores the handoff Address of the different cpu's
 */
typedef struct {
	u32 CpuSettings;
	u64 HandoffAddr;
} XPmcFw_HandoffParam;

/**
 * This is PMCFW instance pointer. This stores all the information
 * required for PMCFW
 */
typedef struct {
	u32 Version; /**< PMCFW Version */
	XStatus Status; /**< Error code during PMCFW failure */
	u32 BootHdrAttributes; /**< Boot Hdr attributes */
	XilPdi_MetaHdr MetaHdr; /**< Image header */
	u32 PrimaryBootDevice; /**< Primary boot device used for booting  */
	u32 SecondaryBootDevice; /**< Secondary boot device in image header*/
	XPmcFw_DeviceOps DeviceOps; /**< Device operations for bootmodes */
	u32 ResetReason; /**< Reset reason */
	u32 PlCfiPresent; /**< PL data presence */
	u32 PlCleaningDone;
	u32 NoOfHandoffCpus; /**< Number of CPU's PMCFW will handoff to */
	XPmcFw_HandoffParam HandoffParam[10];
	u32 EccStatus; /**< ECC status of TCM and DDR. Indicates whether ECC
				is done or not */
	u32 SlrType;
} XPmcFw;


/***************** Macros (Inline Functions) Definitions *********************/

/* SDK release version */
#define PMCFW_RELEASE_VERSION	"7.0"
#define SDK_RELEASE_YEAR	"2019"
#define SDK_RELEASE_QUARTER	"1"

#define XPMCFW_IMAGE_SEARCH_OFST	(0x8000U) /**< 32KB offset */

/**
 * Boot Modes definition
 */
#define XPMCFW_JTAG_BOOT_MODE		(0x0U)
#define XPMCFW_QSPI24_BOOT_MODE		(0x1U)
#define XPMCFW_QSPI32_BOOT_MODE		(0x2U)
#define XPMCFW_SD0_BOOT_MODE		(0x3U)
#define XPMCFW_SD1_BOOT_MODE		(0x5U)
#define XPMCFW_EMMC_BOOT_MODE		(0x6U)
#define XPMCFW_USB_BOOT_MODE		(0x7U)
#define XPMCFW_OSPI_BOOT_MODE		(0x8U)
#define XPMCFW_SMAP_BOOT_MODE		(0xAU)
#define XPMCFW_SD1_LS_BOOT_MODE		(0xEU)
#define XPMCFW_SBI_JTAG_BOOT_MODE	(0x100U)

/**
 * PMCFW stages definition
 */
#define XPMCFW_STAGE1		(0x1U)
#define XPMCFW_STAGE2		(0x2U)
#define XPMCFW_STAGE3		(0x3U)
#define XPMCFW_STAGE4		(0x4U)
#define XPMCFW_STAGE_ERR	(0x5U)
#define XPMCFW_STAGE_DEFAULT	(0x6U)

/* General defines */
#define MAX_CHUNK_SIZE			(100U*1024U) /* 100KB */
#define XPMCFW_DMA_LEN_ALIGN		(16U)

/* ECC Status */
#define XPMCFW_ECC_R5_0_TCM_INIT	(0x1U)
#define XPMCFW_ECC_R5_1_TCM_INIT	(0x2U)
#define XPMCFW_ECC_R5_L_TCM_INIT	(0x4U)
#define XPMCFW_ECC_DDR_INIT		(0x8U)

/* SLR Types */
#define SSIT_MONOLITIC				(0x7U)
#define SSIT_MASTER_SLR				(0x6U)
#define SSIT_SLAVE_SLR1_TOP_SLR		(0x5U)
#define SSIT_SLAVE_SLR1_NON_TOP_SLR	(0x4U)
#define SSIT_SLAVE_SLR2_TOP_SLR		(0x3U)
#define SSIT_SLAVE_SLR2_NON_TOP_SLR	(0x2U)
#define SSIT_SLAVE_SLR3_TOP_SLR		(0x1U)

/* PMC Aliases */
#define PMC_LOCAL_BASEADDR			(0xF0000000U)
#define PMC_ALIAS0_GLOBAL_BASEADDR	(0x100000000UL)
#define PMC_ALIAS1_GLOBAL_BASEADDR	(0x108000000UL)
#define PMC_ALIAS2_GLOBAL_BASEADDR	(0x110000000UL)
#define PMC_ALIAS3_GLOBAL_BASEADDR	(0x118000000UL)

#define PMC_SBI_STREAM_BUF_BASEADDR	(0xF2100000U)

/* SBI definitions */
#define	XPMCFW_SBI_CTRL_INTERFACE_SMAP			(0x0U)
#define	XPMCFW_SBI_CTRL_INTERFACE_JTAG			(0x4U)
#define XPMCFW_SBI_CTRL_INTERFACE_AXI_SLAVE		(0x8U)
#define XPMCFW_SBI_CTRL_ENABLE					(0x1U)

/* XDMA Registers */
#define XPMCFW_XDMA_SCRATCH_PAD_REG1			(0x92003080U)
#define XPMCFW_SBI_DATA_RECV_READY				(0x1U)

/* SSIT error/interrupt masks */
#define SSIT_INTR_MASK			(0x1U)
#define SSIT_ERR_INTR_MASK		(0x4U)
#define SSIT_INTR_CLEAR			(0x0U)

/************************** Function Prototypes ******************************/

/* Functions defined in xpmcfw_main.c */
XStatus XPmcFw_InitUart(void);
void XPmcFw_PrintPmcFwBanner(void);
void XPmcFw_ErrorLockDown(u32 ErrorStatus);
void XPmcFw_ExceptionInit(void);
void XPmcFw_ExceptionHandler(u32 Status);
void XPmcFw_DumpRegisters();
int npi_init();
/* Functions defined in xpmcfw_initialization.c */
XStatus XPmcFw_Initialize(XPmcFw * PmcFwInstancePtr);
XStatus XPmcFw_BootInitAndValidate(XPmcFw * PmcFwInstancePtr);

/* Functions defined in xpmcfw_partition_load.c */
XStatus XPmcFw_PrtnLoad(XPmcFw * PmcFwInstancePtr, u32 PrtnNum);

/* Functions defined in xpmcfw_handoff.c */
XStatus XPmcFw_Handoff (XPmcFw * PmcFwInstancePtr);

/* Functions defined in xpmcfw_pdi.c */
XStatus XPmcFw_PdiInit(XPmcFw * PmcFwInstancePtr);
XStatus XPmcFw_PdiLoad(XPmcFw * PmcFwInstancePtr);
XStatus XPmcFw_MemCopy(u32 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);
XStatus XPmcFw_MemCopySecure(u32 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);

/* Functions defined in xpmcfw_board.c */
XStatus XPmcFw_BoardInit(void);

/* Functions defined in xpmcfw_sd.c */
XStatus XPmcFw_SdInit(u32 DeviceFlags);
XStatus XPmcFw_SdCopy(u32 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);
XStatus XPmcFw_SdRelease(void );

/* Functions defined in xpmcfw_sbi.c */
XStatus XPmcFw_SbiInit(u32 DeviceFlags);
void XPmcFw_SbiConfig(u32 CtrlInterface);
XStatus XPmcFw_SbiCopy(u32 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);
XStatus XPmcFw_SbiRelease(void );

/* Functions defined in xpmcfw_ospi.c */
XStatus XPmcFw_OspiInit(u32 DeviceFlags);
XStatus XPmcFw_OspiCopy(u32 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);
XStatus XPmcFw_OspiRelease(void );
/************************** Variable Definitions *****************************/


#ifdef __cplusplus
}
#endif

#endif  /* XPMCFW_MAIN_H */
