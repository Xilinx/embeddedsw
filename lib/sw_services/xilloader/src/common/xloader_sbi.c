/******************************************************************************
* Copyright (c) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xloader_sbi.c
*
* This is the file which contains SBI related code for the platform loader.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- --------   -------------------------------------------------------
* 1.00  kc   09/21/2017 Initial release
* 1.01  kc   04/09/2019 Added support for PCIe secondary boot mode and
*						 partial PDI load
*       kc   05/21/2019 Updated error code for partial PDI load
* 1.02  bsv  04/09/2020 Code clean up
* 1.03  bsv  07/07/2020 Remove unused functions
*       skd  07/14/2020 XLoader_SbiCopy prototype changed
*       kc   08/10/2020 Added release of SBI reset in SbiInit
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       bsv  09/30/2020 Added parallel DMA support for SBI, JTAG, SMAP and PCIE
*                       boot modes
*       bsv  10/13/2020 Code clean up
* 1.04  rb   08/11/2021 Fix compilation warnings
*       bsv  08/31/2021 Code clean up
* 1.05  bsv  11/08/2021 Skip SbiRecovery for SMAP and PCIe boot modes
*       ma   01/17/2022 Enable SLVERR for SLAVE_BOOT registers
* 1.06  bm   07/06/2022 Refactor versal and versal_net code
* 1.07  ng   11/11/2022 Updated doxygen comments
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xloader.h"
#include "xplmi_generic.h"
#include "xplmi_util.h"
#include "xloader_sbi.h"
#include "xplmi.h"
#include "xplmi_dma.h"
#include "xloader_plat.h"

#if defined(XLOADER_SBI)
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/* SBI definitions */
#define XLOADER_SBI_CTRL_INTERFACE_SMAP                  (0x0U)
#define XLOADER_SBI_CTRL_INTERFACE_JTAG                  (0x4U)
#define XLOADER_SBI_CTRL_INTERFACE_AXI_SLAVE             (0x8U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function is used to initialize the SBI for SMAP and
 * 			JTAG boot modes.
 *
 * @param	DeviceFlags used to differentiate between SMAP and JTAG
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_SbiInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;

	/**
     * - Release reset of SBI.
    */
	XPlmi_UtilRMW(CRP_RST_SBI,
	       CRP_RST_SBI_RESET_MASK, ~CRP_RST_SBI_RESET_MASK);

	if (DeviceFlags == XLOADER_PDI_SRC_SMAP) {
		XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL,
				SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
				XLOADER_SBI_CTRL_INTERFACE_SMAP);
	} else if (DeviceFlags == XLOADER_PDI_SRC_JTAG) {
		XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL,
				SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
				XLOADER_SBI_CTRL_INTERFACE_JTAG);
	} else if (DeviceFlags == XLOADER_PDI_SRC_PCIE) {
		XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL,
				SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
				XLOADER_SBI_CTRL_INTERFACE_AXI_SLAVE);
	} else if (DeviceFlags == XLOADER_PDI_SRC_SBI) {
		/* Do nothing */
	} else {
		goto END;
	}

	Status = XST_SUCCESS;

	/**
	 * Enable the SBI. This is required for
	 * error cases and PCIe boot modes.
	 */
	XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL, SLAVE_BOOT_SBI_CTRL_ENABLE_MASK,
		SLAVE_BOOT_SBI_CTRL_ENABLE_MASK);

	/**
     * - Enable SLVERR.
    */
	XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL,
			SLAVE_BOOT_SBI_CTRL_APB_ERR_RES_MASK,
			SLAVE_BOOT_SBI_CTRL_APB_ERR_RES_MASK);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to copy the data from SMAP/JTAG to
 * 			destination address through SBI interface.
 *
 * @param	SrcAddr is unused and is only passed for compatibility
 *			with the copy functions of other boot modes
 * @param	DestAddr is the address of the destination to which the data
 *			should be copied to
 * @param	Length is number of bytes to be copied
 * @param	Flags indicate parameters for DMA transfer
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_SbiCopy(u64 SrcAddr, u64 DestAddr, u32 Length, u32 Flags)
{
	int Status = XST_FAILURE;
	u32 ReadFlags;
	(void) (SrcAddr);

	ReadFlags = Flags & XPLMI_DEVICE_COPY_STATE_MASK;
	/**
     * - Just wait for the Data to be copied.
    */
	if (ReadFlags == XPLMI_DEVICE_COPY_STATE_WAIT_DONE) {
		Status = XPlmi_WaitForNonBlkDestDma(XPLMI_PMCDMA_1);
		goto END;
	}

	/**
     * - Update the flags for NON blocking DMA call.
    */
	if (ReadFlags == XPLMI_DEVICE_COPY_STATE_INITIATE) {
		ReadFlags = XPLMI_DMA_DST_NONBLK;
	}
	ReadFlags |= XPLMI_PMCDMA_1;
    /**
     * - Start the DMA transfer.
     */
	Status = XPlmi_SbiDmaXfer(DestAddr, (Length >> XPLMI_WORD_LEN_SHIFT),
		ReadFlags);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function will reset the SBI interface.
 *
 * @return
 * 			- XST_SUCCESS on success.
 *
 *****************************************************************************/
int XLoader_SbiRecovery(void)
{
	int Status = XST_FAILURE;
	u32 SbiCtrl;

	if (XLoader_IsJtagSbiMode() == (u8)FALSE) {
		goto END;
	}
	SbiCtrl = XPlmi_In32(SLAVE_BOOT_SBI_CTRL);
	/**
     * - Reset DMA1, SBI.
    */
	XPlmi_Out32(CRP_RST_SBI, CRP_RST_SBI_RESET_MASK);
	XPlmi_UtilRMW(CRP_RST_PDMA, CRP_RST_PDMA_RESET1_MASK,
		      CRP_RST_PDMA_RESET1_MASK);
	usleep(10U);
	XPlmi_Out32(CRP_RST_SBI, 0x0U);
	XPlmi_UtilRMW(CRP_RST_PDMA, CRP_RST_PDMA_RESET1_MASK, 0x0U);

	/**
     * - Restore the interface setting.
    */
	XPlmi_Out32(SLAVE_BOOT_SBI_CTRL, SbiCtrl);

END:
	Status = XST_SUCCESS;
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function checks if the boot mode is jtag or not.
 *
 * @return	TRUE if JTAG and FALSE otherwise
 *
 *****************************************************************************/
u8 XLoader_IsJtagSbiMode(void)
{
	return (u8)(((XPlmi_In32(SLAVE_BOOT_SBI_MODE) &
				SLAVE_BOOT_SBI_MODE_JTAG_MASK) ==
				SLAVE_BOOT_SBI_MODE_JTAG_MASK) ?
				(TRUE) : (FALSE));
}
#endif /* end of XLOADER_SBI */
