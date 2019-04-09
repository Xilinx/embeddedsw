/******************************************************************************
* Copyright (C) 2017-2019 Xilinx, Inc. All rights reserved.
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
* @file xloader_sbi.c
*
* This is the file which contains SBI related code for the platfrom loader.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   09/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xloader.h"
#include "xplmi_generic.h"
#include "xplmi_util.h"
#include "xloader_sbi.h"
#include "xplmi_hw.h"

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
 * This function is used to initialize the SBI for SMAP and JTAG boot modes
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
int XLoader_SbiInit(u32 DeviceFlags)
{
	switch (DeviceFlags)
	{
		case XLOADER_PDI_SRC_SMAP:
			XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL,
			       SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
			       XLOADER_SBI_CTRL_INTERFACE_SMAP);
			break;

		case XLOADER_PDI_SRC_JTAG:
			XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL,
			       SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
			       XLOADER_SBI_CTRL_INTERFACE_JTAG);
			break;

		case XLOADER_PDI_SRC_PCIE:
			XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL,
			       SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
			       XLOADER_SBI_CTRL_INTERFACE_AXI_SLAVE);
			break;

		default:
			break;
	}

	/**
	 * Enable the SBI. This is required for
	 * error cases and PCIe boot modes
	 */
	XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL, SLAVE_BOOT_SBI_CTRL_ENABLE_MASK,
		     SLAVE_BOOT_SBI_CTRL_ENABLE_MASK);

	return XLOADER_SUCCESS;
}

/*****************************************************************************/
/**
 * This function is used to initialize the SBI with user inputs
 *
 * @param	CtrlInterface	is interface value for configuring SBI
 *
 * @return	None
 *
 *****************************************************************************/
void XLoader_SbiConfig(u32 CtrlInterface)
{
	XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL,
			SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK, CtrlInterface);
	XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL,
			SLAVE_BOOT_SBI_CTRL_ENABLE_MASK, XLOADER_SBI_CTRL_ENABLE);
}

/*****************************************************************************/
/**
 * This function is used to initialize the SBI for Slave SLRs
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
int XLoader_SlaveSbiConfig(u64 SlrBaseAddr)
{
	u64 SbiCtrlAddr = ((u64)SlrBaseAddr +
					((u64)(SLAVE_BOOT_SBI_CTRL - PMC_LOCAL_BASEADDR)));

	XPlmi_UtilRMW64((SbiCtrlAddr >> 32), (SbiCtrlAddr & XLOADER_32BIT_MASK),
				SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
				XLOADER_SBI_CTRL_INTERFACE_AXI_SLAVE);

	XPlmi_UtilRMW64((SbiCtrlAddr >> 32), (SbiCtrlAddr & XLOADER_32BIT_MASK),
				SLAVE_BOOT_SBI_CTRL_ENABLE_MASK,
				XLOADER_SBI_CTRL_ENABLE);

	return XLOADER_SUCCESS;
}

/*****************************************************************************/
/**
 * This function is used to copy the data from SMAP/JTAG to destination
 * address through SBI interface
 *
 * @param SrcAddress
 *
 * @param DestAddress is the address of the destination where it
 * should copy to
 *
 * @param Length Length of the bytes to be copied
 *
 * @return
 *		- XLOADER_SUCCESS for successful copy
 *		- errors as mentioned in xloader_error.h
 *
 *****************************************************************************/
XStatus XLoader_SbiCopy(u32 SrcAddress, u64 DestAddress, u32 Length, u32 Flags)
{
	int Status;
	u32 ReadFlags;

	/**
	 * This parameter is required as per the prototype
	 */
	(void) (SrcAddress);

	ReadFlags = Flags | XPLMI_PMCDMA_1;
	Status = XPlmi_SbiDmaXfer(DestAddress, Length/4, ReadFlags);

	return Status;
}

/*****************************************************************************/
/**
 * This function is used to release the SBI settings
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
int XLoader_SbiRelease(void )
{

	return XLOADER_SUCCESS;
}

/*****************************************************************************/
/**
 * This function will reset the SBI interfave
 * @param	None
 * @return	None
 *
 *****************************************************************************/
void XLoader_SbiRecovery()
{
	u32 SbiCtrl;
	SbiCtrl = Xil_In32(SLAVE_BOOT_SBI_CTRL);

	/** Reset the SBI */
	Xil_Out32(CRP_RST_SBI, 0x1);
	usleep(10);
	Xil_Out32(CRP_RST_SBI, 0x0);

	/* Restore the interface setting */
	Xil_Out32(SLAVE_BOOT_SBI_CTRL, SbiCtrl);
}
#endif /* end of XLOADER_SBI */
