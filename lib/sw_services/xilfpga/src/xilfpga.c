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
 * 4.2  Nava  16/08/18	Modified the PL data handling Logic to support
 *			different PL programming interfaces.
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xfpga_config.h"
#include "xilfpga.h"

/************************** Variable Definitions *****************************/
Xilfpga_Ops Fpga_Ops;

/*****************************************************************************/
/**The API is used to load the bitstream file into the PL region.
 * This function performs:
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
 *@param flags Flags are used to specify the type of Bitstream file.
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
 *@return
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
	u32 Status;
	XFpga_Info PLInfo = {0};

	PLInfo.BitstreamAddr = BitstreamImageAddr;
	PLInfo.AddrPtr = AddrPtr;
	PLInfo.Flags = flags;


	/* Validate Bitstream Image */
	Status = XFpga_PL_ValidateImage(&PLInfo);
	if ((Status != XFPGA_OPS_NOT_IMPLEMENTED) &&
			(Status != XFPGA_SUCCESS)) {
		goto END;
	}

	/* Prepare the FPGA to receive configuration Data */
	Status = XFpga_PL_Preconfig(&PLInfo);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	/* write count bytes of configuration data into the PL */
	Status = XFpga_PL_WriteToPl(&PLInfo);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	/* set FPGA to operating state after writing */
	Status = XFpga_PL_PostConfig(&PLInfo);
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to validate the Bitstream Image
 * @param PLInfoPtr Pointer to XFgpa_Info structure
 *
 * @return Codes as mentioned in xilfpga.h
 ******************************************************************************/
u32 XFpga_PL_ValidateImage(XFpga_Info *PLInfoPtr)
{
	u32 Status;

	if (!Fpga_Ops.XFpga_ValidateBitstream) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		Status = Fpga_Ops.XFpga_ValidateBitstream(PLInfoPtr);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_UPDATE_ERR(XFPGA_VALIDATE_ERROR, Status);
		}
	}

	return Status;
}

/*****************************************************************************/
/* This function prepare the FPGA to receive configuration data.
 * @param PLInfoPtr Pointer to XFgpa_Info structure
 *
 * @return Codes as mentioned in xilfpga.h
 *****************************************************************************/
u32 XFpga_PL_Preconfig(XFpga_Info *PLInfoPtr)
{
	u32 Status;

	if (!Fpga_Ops.XFpga_PreConfig) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		Status = Fpga_Ops.XFpga_PreConfig(PLInfoPtr);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_UPDATE_ERR(XFPGA_PRE_CONFIG_ERROR,
						  Status);
		}
	}

	return Status;
}

/*****************************************************************************/
/* This function write count bytes of configuration data into the PL.
 * @param PLInfoPtr Pointer to XFgpa_Info structure
 *
 * @return Codes as mentioned in xilfpga.h
 *****************************************************************************/
u32 XFpga_PL_WriteToPl(XFpga_Info *PLInfoPtr)
{
	 u32 Status;

	if (!Fpga_Ops.XFpga_WriteToPl) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		Status = Fpga_Ops.XFpga_WriteToPl(PLInfoPtr);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_UPDATE_ERR(XFPGA_WRITE_BITSTREAM_ERROR,
						  Status);
		}
	}

	return Status;
}

/*****************************************************************************/
/** This function set FPGA to operating state after writing.
 * @param PLInfoPtr Pointer to XFgpa_Info structure
 *
 * @return Codes as mentioned in xilfpga.h
 *****************************************************************************/
u32 XFpga_PL_PostConfig(XFpga_Info *PLInfoPtr)
{
	u32 Status;

	if (!Fpga_Ops.XFpga_PostConfig) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		Status = Fpga_Ops.XFpga_PostConfig(PLInfoPtr);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_UPDATE_ERR(XFPGA_POST_CONFIG_ERROR,
						  Status);
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function provides PL specific configuration register values
 *
 * @param        ConfigReg  Constant which represents the configuration
 *                       register value to be returned.
 * @param        Address DMA linear buffer address.
 *
 * @return
 *	- XFPGA_SUCCESS if successful
 *	- XFPGA_FAILURE if unsuccessful
 *	- XFPGA_OPS_NOT_IMPLEMENTED if implementation not exists.
 *
 ****************************************************************************/
u32 XFpga_GetPlConfigReg(u32 ConfigReg, UINTPTR Address)
{
	u32 Status;
	XFpga_Info PLInfo = {0};

	PLInfo.ReadbackAddr = Address;
	PLInfo.ConfigReg = ConfigReg;
	if (!Fpga_Ops.XFpga_GetConfigReg) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		Status = Fpga_Ops.XFpga_GetConfigReg(&PLInfo);
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function provides functionality to read back the PL configuration data
 *
 * @param PLInfoPtr Pointer to XFgpa_Info structure
 *
 * @return
 *	- XFPGA_SUCCESS if successful
 *	- XFPGA_FAILURE if unsuccessful
 *	- XFPGA_OPS_NOT_IMPLEMENTED if implementation not exists.
 *
 ****************************************************************************/
u32 XFpga_GetPlConfigData(XFpga_Info *PLInfoPtr)
{
	u32 Status;

	if (!Fpga_Ops.XFpga_GetConfigData) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		Status = Fpga_Ops.XFpga_GetConfigData(PLInfoPtr);
	}

	return Status;
}

/*****************************************************************************/
/** This function provides the STATUS of PL programming interface
 *
 * @param None
 *
 * @return Status of the PL programming interface.
 *
 *****************************************************************************/
u32 XFpga_InterfaceStatus(void)
{
	u32 Status = XFPGA_SUCCESS;
	if (!Fpga_Ops.XFpga_GetInterfaceStatus) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		Status = Fpga_Ops.XFpga_GetInterfaceStatus();
	}

	return Status;
}
