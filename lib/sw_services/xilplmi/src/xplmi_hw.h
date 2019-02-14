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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
* @file xplmi_hw.h
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

#ifndef XPLMI_HW_H
#define XPLMI_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#include "xparameters.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

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
*		u32 XPlmi_ReadReg(u32 BaseAddr, u32 RegOfst)
*
*****************************************************************************/
#define XPlmi_ReadReg(BaseAddr, RegOfst)		\
	Xil_In32((BaseAddr) + (RegOfst))

#define XPlmi_In32(Addr)		Xil_In32(Addr)

#define XPlmi_In64(Addr)		lwea(Addr)

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
*		void XPlmi_WriteReg(u32 BaseAddr, u32 RegOfst, u32 Data)
*
*****************************************************************************/
#define XPlmi_WriteReg(BaseAddr, RegOfst, Data)	\
	Xil_Out32((BaseAddr) + (RegOfst), (Data))

#define XPlmi_Out32(Addr, Data)		Xil_Out32(Addr, Data)

#define XPlmi_Out64(Addr, Data)		swea(Addr, Data)


#define PMC_TAP_VERSION				(0xF11A0004U)
#define PMC_TAP_VERSION_PLATFORM_VERSION_SHIFT   28
#define PMC_TAP_VERSION_PLATFORM_VERSION_MASK    0XF0000000
#define PMC_TAP_VERSION_PLATFORM_SHIFT   24
#define PMC_TAP_VERSION_PLATFORM_MASK    0X0F000000
#define PMC_TAP_VERSION_RTL_VERSION_SHIFT   16
#define PMC_TAP_VERSION_RTL_VERSION_MASK    0X00FF0000
#define PMC_TAP_VERSION_PS_VERSION_SHIFT   8
#define PMC_TAP_VERSION_PS_VERSION_MASK    0X0000FF00
#define PMC_TAP_VERSION_PMC_VERSION_SHIFT   0
#define PMC_TAP_VERSION_PMC_VERSION_MASK    0X000000FF

#define PMC_TAP_VERSION_SILICON			(0x0U)
#define PMC_TAP_VERSION_SPP			(0x1U)
#define PMC_TAP_VERSION_EMU			(0x2U)
#define PMC_TAP_VERSION_QEMU			(0x3U)

#define XPLMI_PLATFORM		((Xil_In32(PMC_TAP_VERSION) & \
					PMC_TAP_VERSION_PLATFORM_MASK) >> \
					PMC_TAP_VERSION_PLATFORM_SHIFT)
#define XPLMI_PMCRAM_BASEADDR			(0xF2000000U)
#define XPLMI_PMCRAM_LEN			(0x20000U)

/**
 * Definition for QSPI to be included
 */
#if (!defined(PLM_QSPI_EXCLUDE) && defined(XPAR_XQSPIPSU_0_DEVICE_ID))
#define XLOADER_QSPI
#define XLOADER_QSPI_BASEADDR    XPAR_XQSPIPS_0_BASEADDR
#endif

/**
 * Definition for OSPI to be included
 */
#if (!defined(PLM_OSPI_EXCLUDE) && defined(XPAR_PSU_OSPI_0_DEVICE_ID))
#define XLOADER_OSPI
#define XLOADER_OSPI_DEVICE_ID  XPAR_PSU_OSPI_0_DEVICE_ID
#define XLOADER_OSPI_BASEADDR   XPAR_PSU_OSPI_0_BASEADDR
#endif

/**
 * Definitions for SD to be included
 */
#if (!defined(PLM_SD_EXCLUDE) && (XPAR_XSDPS_0_BASEADDR == 0xF1040000))
#define XLOADER_SD_0
#endif

#if (!defined(PLM_SD_EXCLUDE) && (XPAR_XSDPS_0_BASEADDR == 0xF1050000) ||\
                (XPAR_XSDPS_1_BASEADDR == 0xF1050000))
#define XLOADER_SD_1
#endif


/**
 * Definition for SBI to be included
 */
#if !defined(PLM_SBI_EXCLUDE)
#define XLOADER_SBI
#endif

/**
 * Definitions required from crp.h
 */
#define CRP_BASEADDR      0XF1260000
#define CRP_BOOT_MODE_USER    ( ( CRP_BASEADDR ) + 0X00000200 )
#define CRP_BOOT_MODE_USER_BOOT_MODE_MASK    0X0000000F

/**
 * Definitions required from pmc_global.h
 */
#define PMC_GLOBAL_BASEADDR      0XF1110000
#define PMC_GLOBAL_PMC_SSS_CFG    ( ( PMC_GLOBAL_BASEADDR ) + 0X00000500 )

/**
 * Definitions required from slave_boot.h
 */
#define SLAVE_BOOT_BASEADDR      0XF1220000
#define SLAVE_BOOT_SBI_MODE    ( ( SLAVE_BOOT_BASEADDR ) + 0X00000000 )
#define SLAVE_BOOT_SBI_MODE_JTAG_MASK    0X00000002

#define SLAVE_BOOT_SBI_CTRL    ( ( SLAVE_BOOT_BASEADDR ) + 0X00000004 )
#define SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK    0X0000001C
#define SLAVE_BOOT_SBI_CTRL_ENABLE_MASK    0X00000001
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_HW_H */
