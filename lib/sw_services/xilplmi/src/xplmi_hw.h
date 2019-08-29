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
#include "pmc_global.h"

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


/**
 * Definitions required from pmc_tap.h
 */
#define PMC_TAP_BASEADDR      0XF11A0000
#define PMC_TAP_IDCODE    ( ( PMC_TAP_BASEADDR ) + 0X00000000 )
#define PMC_TAP_VERSION    ( ( PMC_TAP_BASEADDR ) + 0X00000004 )
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
#define XPLMI_PLATFORM_MASK			(0x03000000U)

#define XPLMI_PLATFORM		((Xil_In32(PMC_TAP_VERSION) & \
					XPLMI_PLATFORM_MASK) >> \
					PMC_TAP_VERSION_PLATFORM_SHIFT)
#define XPLMI_PMCRAM_BASEADDR			(0xF2000000U)
#define XPLMI_PMCRAM_LEN			(0x20000U)

/**
 * Definitions required from Efuse
 */
#define EFUSE_CACHE_BASEADDR      0XF1250000
#define EFUSE_CACHE_ANLG_TRIM_5    ( ( EFUSE_CACHE_BASEADDR ) + 0X000000E0 )
#define EFUSE_CACHE_ANLG_TRIM_7    ( ( EFUSE_CACHE_BASEADDR ) + 0X000000F8 )

#define EFUSE_TRIM_LP_MASK		(0xFFFF)

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
#if (!defined(PLM_OSPI_EXCLUDE) && defined(XPAR_XOSPIPSV_0_DEVICE_ID))
#define XLOADER_OSPI
#define XLOADER_OSPI_DEVICE_ID  XPAR_XOSPIPSV_0_DEVICE_ID
#define XLOADER_OSPI_BASEADDR   XPAR_XOSPIPSV_0_BASEADDR
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
#define CRP_BOOT_MODE_POR    ( ( CRP_BASEADDR ) + 0X00000204 )
#define CRP_RESET_REASON    ( ( CRP_BASEADDR ) + 0X00000220 )
#define CRP_RST_SBI    ( ( CRP_BASEADDR ) + 0X00000324 )
#define CRP_RST_PDMA    ( ( CRP_BASEADDR ) + 0X00000328 )
#define CRP_RST_PDMA_RESET1_MASK    0X00000002
/**
 * Register: CRP_RST_PS
 */
#define CRP_RST_PS    ( ( CRP_BASEADDR ) + 0x0000031CU )
#define CRP_RST_PS_PMC_SRST_MASK    (0x00000008U)

/**
 * PMC_ANALOG Base Address
 */
#define PMC_ANALOG_BASEADDR      0XF1160000
/**
 * Register: PMC_ANALOG_VGG_CTRL
 */
#define PMC_ANALOG_VGG_CTRL    ( ( PMC_ANALOG_BASEADDR ) + 0X0000000C )
#define PMC_ANALOG_VGG_CTRL_EN_VGG_CLAMP_MASK    0X00000001

/**
 * Definitions required from slave_boot.h
 */
#define SLAVE_BOOT_BASEADDR      0XF1220000
#define SLAVE_BOOT_SBI_MODE    ( ( SLAVE_BOOT_BASEADDR ) + 0X00000000 )
#define SLAVE_BOOT_SBI_MODE_JTAG_MASK    0X00000002
#define SLAVE_BOOT_SBI_MODE_SELECT_MASK    0x00000001

#define SLAVE_BOOT_SBI_CTRL    ( ( SLAVE_BOOT_BASEADDR ) + 0X00000004 )
#define SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK    0X0000001C
#define SLAVE_BOOT_SBI_CTRL_ENABLE_MASK    0X00000001

/**
 * Register: SLAVE_BOOT_SBI_STATUS
 */
#define SLAVE_BOOT_SBI_STATUS    ( ( SLAVE_BOOT_BASEADDR ) + 0X0000000C )
#define SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_MASK    0XF0000000
#define SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_MASK    0X00F00000
#define SLAVE_BOOT_SBI_STATUS_CMN_BUF_SPACE_MASK    0X00001FF8

#define SLAVE_BOOT_SBI_IRQ_STATUS    ( ( SLAVE_BOOT_BASEADDR ) + 0X00000300 )
#define SLAVE_BOOT_SBI_IRQ_STATUS_DATA_RDY_MASK    0X00000004

#define SLAVE_BOOT_SBI_IRQ_ENABLE    ( ( SLAVE_BOOT_BASEADDR ) + 0X00000308 )
#define SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK    0X00000004

#define	XPLMI_SBI_CTRL_INTERFACE_SMAP			(0x0U)
#define	XPLMI_SBI_CTRL_INTERFACE_JTAG			(0x4U)
#define XPLMI_SBI_CTRL_INTERFACE_AXI_SLAVE		(0x8U)
#define XPLMI_SBI_CTRL_ENABLE					(0x1U)

/**
 * Definitions required from psm_gloabl_reg
 */
#define PSM_GLOBAL_REG_BASEADDR      0XFFC90000
#define PSM_GLOBAL_REG_PSM_CR_ERR1_DIS    ( ( PSM_GLOBAL_REG_BASEADDR ) + 0X00001028 )
#define PSM_GLOBAL_REG_PSM_NCR_ERR1_DIS    ( ( PSM_GLOBAL_REG_BASEADDR ) + 0X00001048 )
#define PSM_GLOBAL_REG_PSM_IRQ1_DIS    ( ( PSM_GLOBAL_REG_BASEADDR ) + 0X00001068 )
#define PSM_GLOBAL_REG_PSM_SRST1_DIS    ( ( PSM_GLOBAL_REG_BASEADDR ) + 0X00001088 )
#define PSM_GLOBAL_REG_PSM_CR_ERR2_DIS    ( ( PSM_GLOBAL_REG_BASEADDR ) + 0X00001038 )
#define PSM_GLOBAL_REG_PSM_NCR_ERR2_DIS    ( ( PSM_GLOBAL_REG_BASEADDR ) + 0X00001058 )
#define PSM_GLOBAL_REG_PSM_IRQ2_DIS    ( ( PSM_GLOBAL_REG_BASEADDR ) + 0X00001078 )
#define PSM_GLOBAL_REG_PSM_SRST2_DIS    ( ( PSM_GLOBAL_REG_BASEADDR ) + 0X00001098 )
#define PSM_GLOBAL_REG_PSM_SRST1_EN    ( ( PSM_GLOBAL_REG_BASEADDR ) + 0X00001084 )
#define PSM_GLOBAL_REG_PSM_SRST2_EN    ( ( PSM_GLOBAL_REG_BASEADDR ) + 0X00001094 )
#define PSM_GLOBAL_REG_PSM_IRQ1_EN    ( ( PSM_GLOBAL_REG_BASEADDR ) + 0X00001064 )
#define PSM_GLOBAL_REG_PSM_IRQ2_EN    ( ( PSM_GLOBAL_REG_BASEADDR ) + 0X00001074 )

/**
 * Definitions required from pmc_tap.h
 */
/**
 * PMC_TAP Base Address
 */
#define PMC_TAP_BASEADDR      0XF11A0000

/**
 * Register: PMC_TAP_SLR_TYPE
 */
#define PMC_TAP_SLR_TYPE    ( ( PMC_TAP_BASEADDR ) + 0X00000024 )
#define PMC_TAP_SLR_TYPE_VAL_MASK    0X00000007
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_HW_H */
