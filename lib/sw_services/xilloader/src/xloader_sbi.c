/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
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

#if defined(XLOADER_SBI)
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/* SBI definitions */
#define XLOADER_SBI_CTRL_INTERFACE_SMAP                  (0x0U)
#define XLOADER_SBI_CTRL_INTERFACE_JTAG                  (0x4U)
#define XLOADER_SBI_CTRL_INTERFACE_AXI_SLAVE             (0x8U)
#define XLOADER_SBI_CTRL_ENABLE                          (0x1U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function is used to initialize the SBI for SMAP and
 * JTAG boot modes.
 *
 * @param	DeviceFlags used to differentiate between SMAP and JTAG
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_SbiInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;

	/* Release reset of SBI */
	XPlmi_UtilRMW(CRP_RST_SBI,
	       CRP_RST_SBI_RESET_MASK, ~CRP_RST_SBI_RESET_MASK);

	switch ((PdiSrc_t)DeviceFlags) {
		case XLOADER_PDI_SRC_SMAP:
			XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL,
			       SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
			       XLOADER_SBI_CTRL_INTERFACE_SMAP);
			Status = XST_SUCCESS;
			break;
		case XLOADER_PDI_SRC_JTAG:
			XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL,
			       SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
			       XLOADER_SBI_CTRL_INTERFACE_JTAG);
			Status = XST_SUCCESS;
			break;
		case XLOADER_PDI_SRC_PCIE:
			XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL,
			       SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
			       XLOADER_SBI_CTRL_INTERFACE_AXI_SLAVE);
			Status = XST_SUCCESS;
			break;
		case XLOADER_PDI_SRC_SBI:
			Status = XST_SUCCESS;
			break;
		default:
			break;
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/*
	 * Enable the SBI. This is required for
	 * error cases and PCIe boot modes.
	 */
	XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL, SLAVE_BOOT_SBI_CTRL_ENABLE_MASK,
		     SLAVE_BOOT_SBI_CTRL_ENABLE_MASK);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to copy the data from SMAP/JTAG to
 * destination address through SBI interface.
 *
 * @param	SrcAddress is unused and is only passed for compatibility
 *		with the copy functions of other boot modes
 * @param	DestAddress is the address of the destination to which the data
 *		should be copied to
 * @param	Length is number of bytes to be copied
 * @param	Flags indicate parameters for DMA transfer
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_SbiCopy(u64 SrcAddress, u64 DestAddress, u32 Length, u32 Flags)
{
	int Status = XST_FAILURE;
	u32 ReadFlags;
	(void) (SrcAddress);

	ReadFlags = Flags | XPLMI_PMCDMA_1;
	Status = XPlmi_SbiDmaXfer(DestAddress, Length / XPLMI_WORD_LEN,
		ReadFlags);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function will reset the SBI interface.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void XLoader_SbiRecovery(void)
{
	u32 SbiCtrl;

	SbiCtrl = XPlmi_In32(SLAVE_BOOT_SBI_CTRL);
	/* Reset DMA1, SBI */
	XPlmi_Out32(CRP_RST_SBI, 0x1U);
	XPlmi_UtilRMW(CRP_RST_PDMA, CRP_RST_PDMA_RESET1_MASK,
		      CRP_RST_PDMA_RESET1_MASK);
	usleep(10U);
	XPlmi_Out32(CRP_RST_SBI, 0x0U);
	XPlmi_UtilRMW(CRP_RST_PDMA, CRP_RST_PDMA_RESET1_MASK, 0x0U);

	/* Restore the interface setting */
	XPlmi_Out32(SLAVE_BOOT_SBI_CTRL, SbiCtrl);
}
#endif /* end of XLOADER_SBI */
