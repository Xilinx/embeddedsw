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
* @file xpmcfw_hw.h
*
* This is the header file which contains definitions for the hardware
* registers.
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

#ifndef XPMCFW_HW_H
#define XPMCFW_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xpmcfw_config.h"
#include "xparameters.h"

#ifdef XPMCFW_HW50
#include "hw50/crf.h"
#include "hw50/crl.h"
#include "hw50/crp.h"
#include "hw50/fpd_apu.h"
#include "hw50/rpu.h"
#include "hw50/psm_global_reg.h"
#include "hw50/pmc_global.h"
#include "hw50/pmc_analog.h"
#include "hw50/pmc_dma0.h"
#include "hw50/pmc_dma1.h"
#include "hw50/pmc_tap.h"
#include "hw50/slave_boot.h"
#include "hw50/efuse_cache.h"
#include "hw50/cfu_apb.h"
#endif

#ifdef XPMCFW_HW60
#include "hw60/crf.h"
#include "hw60/crl.h"
#include "hw60/crp.h"
#include "hw60/fpd_apu.h"
#include "hw60/rpu.h"
#include "hw60/psm_global_reg.h"
#include "hw60/pmc_global.h"
#include "hw60/pmc_analog.h"
#include "hw60/pmc_dma0.h"
#include "hw60/pmc_dma1.h"
#include "hw60/pmc_tap.h"
#include "hw60/slave_boot.h"
#include "hw60/efuse_cache.h"
#include "hw60/cfu_apb.h"
#include "hw60/lpd_iou_slcr.h"
#include "hw60/cpm_slcr.h"
#endif

#ifdef XPMCFW_HW70
#include "hw70/crf.h"
#include "hw70/crl.h"
#include "hw70/crp.h"
#include "hw70/fpd_apu.h"
#include "hw70/rpu.h"
#include "hw70/psm_global_reg.h"
#include "hw70/pmc_global.h"
#include "hw70/pmc_analog.h"
#include "hw70/pmc_dma0.h"
#include "hw70/pmc_dma1.h"
#include "hw70/pmc_tap.h"
#include "hw70/slave_boot.h"
#include "hw70/efuse_cache.h"
#include "hw70/cfu_apb.h"
#include "hw70/lpd_iou_slcr.h"
#include "hw70/cpm_slcr.h"
#endif

#ifdef XPMCFW_HW80
#include "hw80/crf.h"
#include "hw80/crl.h"
#include "hw80/crp.h"
#include "hw80/fpd_apu.h"
#include "hw80/rpu.h"
#include "hw80/psm_global_reg.h"
#include "hw80/pmc_global.h"
#include "hw80/pmc_analog.h"
#include "hw80/pmc_dma0.h"
#include "hw80/pmc_dma1.h"
#include "hw80/pmc_tap.h"
#include "hw80/slave_boot.h"
#include "hw80/efuse_cache.h"
#include "hw80/cfu_apb.h"
#include "hw80/lpd_iou_slcr.h"
#include "hw80/cpm_slcr.h"
#endif

#ifdef XPMCFW_HW90
#include "hw90/crf.h"
#include "hw90/crl.h"
#include "hw90/crp.h"
#include "hw90/fpd_apu.h"
#include "hw90/rpu.h"
#include "hw90/psm_global_reg.h"
#include "hw90/pmc_global.h"
#include "hw90/pmc_analog.h"
#include "hw90/pmc_dma0.h"
#include "hw90/pmc_dma1.h"
#include "hw90/pmc_tap.h"
#include "hw90/slave_boot.h"
#include "hw90/efuse_cache.h"
#include "hw90/cfu_apb.h"
#include "hw90/lpd_iou_slcr.h"
#include "hw90/cpm_slcr.h"
#endif

#ifdef XPMCFW_HW100
#include "hw100/crf.h"
#include "hw100/crl.h"
#include "hw100/crp.h"
#include "hw100/fpd_apu.h"
#include "hw100/rpu.h"
#include "hw100/psm_global_reg.h"
#include "hw100/pmc_global.h"
#include "hw100/pmc_analog.h"
#include "hw100/pmc_dma0.h"
#include "hw100/pmc_dma1.h"
#include "hw100/pmc_tap.h"
#include "hw100/slave_boot.h"
#include "hw100/efuse_cache.h"
#include "hw100/cfu_apb.h"
#include "hw100/lpd_iou_slcr.h"
#include "hw100/cpm_slcr.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/


/**
 * defines for the PMCFW peripherals present
 */

/**
 * Definition for QSPI to be included
 */
#if (!defined(PMCFW_QSPI_EXCLUDE) && defined(XPAR_XQSPIPSU_0_DEVICE_ID))
#define XPMCFW_QSPI
#define XPMCFW_QSPI_BASEADDR	XPAR_XQSPIPS_0_BASEADDR
#endif

#if (!defined(PMCFW_OSPI_EXCLUDE) && defined(XPAR_PSU_OSPI_0_DEVICE_ID))
#define XPMCFW_OSPI
#define XPMCFW_OSPI_DEVICE_ID  XPAR_PSU_OSPI_0_DEVICE_ID
#define XPMCFW_OSPI_BASEADDR   XPAR_PSU_OSPI_0_BASEADDR
#endif

/**
 * Definitions for SD to be included
 */
#if (!defined(PMCFW_SD_EXCLUDE) && (XPAR_XSDPS_0_BASEADDR == 0xF1040000))
#define XPMCFW_SD_0
#endif

#if (!defined(PMCFW_SD_EXCLUDE) && (XPAR_XSDPS_0_BASEADDR == 0xF1050000) ||\
                (XPAR_XSDPS_1_BASEADDR == 0xF1050000))
#define XPMCFW_SD_1
#endif


/**
 * Definition for SBI to be included
 */
#if !defined(PMCFW_SBI_EXCLUDE)
#define XPMCFW_SBI
#endif

/**
 * Definition for SECURE to be included
 */
#if !defined(PMCFW_SECURE_EXCLUDE)
#define XPMCFW_SECURE
#endif

/**
 * Definition for PL bitsream feature to be included
 */
#if !defined(PMCFW_BS_EXCLUDE)
#define XPMCFW_BS
#endif

#if !defined(PMCFW_PERF_EXCLUDE)
#define XPMCFW_PERF
#endif

/**
 * Definition for SSIT config to be included
 */
#if !defined(PMCFW_SSIT_EXCLUDE)
#define XPMCFW_SSIT
#endif


/****************************************************************************/
/**
*
* Read the given register.
*
* @param	BaseAddr is the base address of the device
* @param	RegOfst is the register offset to be read
*
* @return	The 32-bit value of the register
*
* @note		C-style signature:
*		u32 XPmcFw_ReadReg(u32 BaseAddr, u32 RegOfst)
*
*****************************************************************************/
#define XPmcFw_ReadReg(BaseAddr, RegOfst)		\
	Xil_In32((BaseAddr) + (RegOfst))

#define XPmcFw_In32(Addr)		Xil_In32(Addr)

#define XPmcFw_In64(Addr)		lwea(Addr)

/****************************************************************************/
/**
*
* Write to the given register.
*
* @param	BaseAddr is the base address of the device
* @param	RegOfst is the register offset to be written
* @param	Data is the 32-bit value to write to the register
*
* @return	None.
*
* @note		C-style signature:
*		void XPmcFw_WriteReg(u32 BaseAddr, u32 RegOfst, u32 Data)
*
*****************************************************************************/
#define XPmcFw_WriteReg(BaseAddr, RegOfst, Data)	\
	Xil_Out32((BaseAddr) + (RegOfst), (Data))

#define XPmcFw_Out32(Addr, Data)		Xil_Out32(Addr, Data)

#define XPmcFw_Out64(Addr, Data)		swea(Addr, Data)

#define PMC_TAP_VERSION_SILICON			(0x0U)
#define PMC_TAP_VERSION_SPP			(0x1U)
#define PMC_TAP_VERSION_EMU			(0x2U)
#define PMC_TAP_VERSION_QEMU			(0x3U)
#define XPMCFW_PLATFORM_MASK			(0x03000000U)
#define XPMCFW_PLATFORM		((Xil_In32(PMC_TAP_VERSION) & \
					XPMCFW_PLATFORM_MASK) >> \
					PMC_TAP_VERSION_PLATFORM_SHIFT)
#define XPMCFW_PMCRAM_BASEADDR			(0xF2000000U)
#define XPMCFW_R5_0_TCMA_BASE_ADDR		(0xFFE00000U)
#define XPMCFW_R5_0_TCMB_BASE_ADDR		(0xFFE20000U)
#define XPMCFW_R5_1_TCMA_BASE_ADDR		(0xFFE90000U)
#define XPMCFW_R5_1_TCMB_BASE_ADDR		(0xFFEB0000U)
#define XPMCFW_R5_0_TCM_LEN			(0x20000U)
#define XPMCFW_R5_L_TCM_LEN			(0x40000U)
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


#ifdef __cplusplus
}
#endif

#endif  /* XPMCFW_HW_H */
