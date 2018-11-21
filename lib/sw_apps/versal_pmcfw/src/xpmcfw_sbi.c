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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
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
* @file xpmcfw_sbi.c
*
* This is the file which contains SBI related code for the PMC FW.
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
#include "xpmcfw_hw.h"
#include "xpmcfw_main.h"

#if defined(XPMCFW_SBI)

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

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
XStatus XPmcFw_SbiInit(u32 DeviceFlags)
{

	if (DeviceFlags == XPMCFW_SMAP_BOOT_MODE)
	{
		XPmcFw_UtilRMW(SLAVE_BOOT_SBI_CTRL,
			       SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
			       XPMCFW_SBI_CTRL_INTERFACE_SMAP);
	} else {
		XPmcFw_UtilRMW(SLAVE_BOOT_SBI_CTRL,
			       SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
			       XPMCFW_SBI_CTRL_INTERFACE_JTAG);
		/**
		 * Check if it is SBI JTAG boot mode
		 */
		if (DeviceFlags == XPMCFW_JTAG_BOOT_MODE)
		{
			/* Reset the DMA */
			Xil_Out32(CRP_RST_PDMA, 0x3);
			Xil_Out32(CRP_RST_PDMA, 0x0);
		}
	}

	return XPMCFW_SUCCESS;
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
void XPmcFw_SbiConfig(u32 CtrlInterface)
{
	XPmcFw_UtilRMW(SLAVE_BOOT_SBI_CTRL,
			SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK, CtrlInterface);
	XPmcFw_UtilRMW(SLAVE_BOOT_SBI_CTRL,
			SLAVE_BOOT_SBI_CTRL_ENABLE_MASK, XPMCFW_SBI_CTRL_ENABLE);
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
XStatus XPmcFw_SlaveSbiConfig(u64 SlrBaseAddr)
{
	u64 SbiCtrlAddr = ((u64)SlrBaseAddr +
					((u64)(SLAVE_BOOT_SBI_CTRL - PMC_LOCAL_BASEADDR)));

	XPmcFw_UtilRMW64((SbiCtrlAddr >> 32), (SbiCtrlAddr & XPMCFW_32BIT_MASK),
				SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
				XPMCFW_SBI_CTRL_INTERFACE_AXI_SLAVE);

	XPmcFw_UtilRMW64((SbiCtrlAddr >> 32), (SbiCtrlAddr & XPMCFW_32BIT_MASK),
				SLAVE_BOOT_SBI_CTRL_ENABLE_MASK,
				XPMCFW_SBI_CTRL_ENABLE);

	return XPMCFW_SUCCESS;
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
 *		- XPMCFW_SUCCESS for successful copy
 *		- errors as mentioned in xpmcfw_error.h
 *
 *****************************************************************************/
XStatus XPmcFw_SbiCopy(u32 SrcAddress, u64 DestAddress, u32 Length, u32 Flags)
{
	XStatus Status;
	u32 ReadFlags;

	if (Flags == XPMCFW_READ_AXI_FIXED)
	{
		ReadFlags = XPMCFW_DST_CH_AXI_FIXED | XPMCFW_PMCDMA_1;
	} else {
		ReadFlags = XPMCFW_PMCDMA_1;
	}

	Status = XPmcFw_SbiDmaXfer(DestAddress, Length/4, ReadFlags);

	return Status;
}

/*****************************************************************************/
/**
 * This function is used to release the sd settings
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
XStatus XPmcFw_SbiRelease(void )
{

	return XPMCFW_SUCCESS;
}

#endif /* end of XPMCFW_SBI */
