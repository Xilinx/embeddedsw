/******************************************************************************
 *
 * Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xilfpga.c
 *
 * This file contains the definitions of Bitstream loading functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 4.2  Nava  08/06/16  Refactor the xilfpga library to support
 *			different PL programming Interfaces.
 * 4.2  adk   11/07/18  Added support for readback of PL configuration data.
 *
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xfpga_config.h"
#include "xilfpga.h"

Xilfpga_Ops Fpga_Ops;
/*****************************************************************************/
/**The API is used to load the user provided Bitstream file into the
 * PL region.
 * This function does the following jobs:
 *		- Power-up the PL fabric.
 *		- Performs PL-PS Isolation.
 *		- Initialize PL configuration Interface
 *		- Write a Bitstream into the PL
 *		- Wait for the PL Done Status.
 *		- Restore PS-PL Isolation (Power-up PL fabric).
 *
 *@param BitstreamImageAddr  Linear memory Bitstream image base address
 *
 *@param AddrPtr Aes key address which is used for Decryption.
 *
 *@param flags: Flags are used to specify the type of Bitstream file.
 *			* BIT(0) - Bitstream type
 *					* 0 - Full Bitstream
 *					* 1 - Partial Bitstream
 *			* BIT(1) - Authentication using DDR
 *					* 1 - Enable
 *					* 0 - Disable
 *			* BIT(2) - Authentication using OCM
 *					* 1 - Enable
 *					* 0 - Disable
 *			* BIT(3) - User-key Encryption
 *					* 1 - Enable
 *					* 0 - Disable
 *			* BIT(4) - Device-key Encryption
 *					* 1 - Enable
 *					* 0 - Disable
 *
 *@return Returns Status
 *	- XFPGA_SUCCESS on success
 *	- Error code on failure.
 *	- XFPGA_VALIDATE_ERROR.
 *	- XFPGA_PRE_CONFIG_ERROR.
 *	- XFPGA_WRITE_BITSTREAM_ERROR.
 *	- XFPGA_POST_CONFIG_ERROR.
 *
 *****************************************************************************/
u32 XFpga_PL_BitStream_Load(UINTPTR BitstreamImageAddr,
		UINTPTR AddrPtr, u32 flags)
{
	u32 Status = XFPGA_SUCCESS;
	XSecure_ImageInfo ImageHdrInfo = {0};

	Status = Fpga_Ops.XFpga_ValidateBitstream(BitstreamImageAddr,
						&ImageHdrInfo, flags);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_UPDATE_ERR(XFPGA_VALIDATE_ERROR, Status);
		xil_printf("XFPGA Fail to validate the Bitstream Image "
				"with Error Code: 0x%08x\r\n", Status);
		return Status;
	}

	Status = Fpga_Ops.XFpga_PreConfig(flags);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_UPDATE_ERR(XFPGA_PRE_CONFIG_ERROR, Status);
		xil_printf("XFPGA Fail to PreConfigure the PL Interface "
				"with Error Code: 0x%08x\r\n", Status);
		return Status;
	}

	Status = Fpga_Ops.XFpga_writeToPl(BitstreamImageAddr, AddrPtr,
					&ImageHdrInfo, flags);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_UPDATE_ERR(XFPGA_WRITE_BITSTREAM_ERROR, Status);
		xil_printf("FPGA fail to write Bitstream into PL "
				"with Error Code: 0x%08x\r\n", Status);
		return Status;
	}

	Status = Fpga_Ops.XFpga_PostConfig(AddrPtr, flags);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_UPDATE_ERR(XFPGA_POST_CONFIG_ERROR, Status);
		xil_printf("XFPGA Fail to PostConfigure the PL Interface "
				"with Error Code: 0x%08x\r\n", Status);
	}

	return Status;
}

/*****************************************************************************/
/** This function provides the STATUS of PL programming interface
 *
 * @param	None
 *
 * @return	Status of the PL programming interface.
 *
 *****************************************************************************/
u32 XFpga_InterfaceStatus(void)
{
	return Fpga_Ops.XFpga_InterfaceStatus();
}

/*****************************************************************************/
/**
 * This function provides PL specific configuration register values
 *
 * @param        ConfigReg  is a constant which represents the configuration
 *                       register value to be returned.
 * @param        Address is the DMA buffer address.
 *
 * @return
 *               - XFPGA_SUCCESS if successful
 *               - XFPGA_FAILURE if unsuccessful
 *
 *
 ****************************************************************************/
u32 XFpga_GetPlConfigReg(u32 ConfigReg, UINTPTR Address)
{
	u32 Status = XFPGA_SUCCESS;

	Status = Fpga_Ops.XFpga_GetConfigReg(ConfigReg, Address);
	if (Status != XFPGA_SUCCESS)
		Status = XFPGA_FAILURE;

	return Status;
}

/*****************************************************************************/
/**
 * This function provides functionality to read back the PL configuration data
 *
 * @param        Address is the DMA buffer address.
 * @param	NumFrames is the number fpga configuration frames to read.
 *
 * @return
 *               - XFPGA_SUCCESS if successful
 *               - XFPGA_FAILURE if unsuccessful
 ****************************************************************************/
u32 XFpga_GetPlConfigData(UINTPTR Address, u32 NumFrames)
{
	u32 Status = XFPGA_SUCCESS;

	Status = Fpga_Ops.XFpga_GetConfigData(Address, NumFrames);
	if (Status != XFPGA_SUCCESS)
		Status = XFPGA_FAILURE;

	return Status;
}
