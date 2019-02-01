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
 * 4.2  Nava  15/09/18 Fixed global function call-backs issue.
 * 5.0  Nava 11/05/18  Added full bitstream loading support for versal Platform.
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xfpga_config.h"
#include "xilfpga.h"

/************************** Variable Definitions *****************************/

#if defined(PLATFORM_ZYNQMP) || (PSU_PMU)
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
 *@param AddrPtr_Size Aes key address which is used for Decryption (or)
 *			In none Secure Bitstream used it is used to store size
 *			of Bitstream Image.
 *
 *@param Flags Flags are used to specify the type of Bitstream file.
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
			    UINTPTR AddrPtr, u32 Flags)
{
	u32 Status;
	XFpga_Info PLInfo = {0U};

	PLInfo.BitstreamAddr = BitstreamImageAddr;
	PLInfo.AddrPtr = AddrPtr;
	PLInfo.Flags = Flags;


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
 * @param PLInfoPtr Pointer to the XFgpa_Info structure
 *
 * @return Codes as mentioned in xilfpga.h
 ******************************************************************************/
u32 XFpga_PL_ValidateImage(XFpga_Info *PLInfoPtr)
{
	u32 Status;
	XFpga XFpgaInstance = {0U};
	XFpga_Write WriteInfo = {0U};

	/* Initialize the XFpga Instance */
	Status = XFpga_Initialize(&XFpgaInstance);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	if (XFpgaInstance.XFpga_ValidateBitstream == NULL) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
			"%s Implementation not exists..\r\n", __FUNCTION__);
		goto END;
	}

	WriteInfo.BitstreamAddr = PLInfoPtr->BitstreamAddr;
	WriteInfo.AddrPtr_Size = PLInfoPtr->AddrPtr;
	WriteInfo.Flags = PLInfoPtr->Flags;
	XFpgaInstance.WriteInfoPtr = &WriteInfo;

	Status = XFpgaInstance.XFpga_ValidateBitstream(&XFpgaInstance);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_UPDATE_ERR(XFPGA_VALIDATE_ERROR, Status);
	} else {
		PLInfoPtr->BitstreamAddr =
			XFpgaInstance.WriteInfoPtr->BitstreamAddr;
		PLInfoPtr->AddrPtr = XFpgaInstance.WriteInfoPtr->AddrPtr_Size;
		(void)memcpy(&PLInfoPtr->SecureImageInfo,
			   &XFpgaInstance.PLInfo.SecureImageInfo,
			   sizeof(PLInfoPtr->SecureImageInfo));
	}

END:
	return Status;
}

/*****************************************************************************/
/* This function prepare the FPGA to receive configuration data.
 * @param PLInfoPtr Pointer to the XFgpa_Info structure
 *
 * @return Codes as mentioned in xilfpga.h
 *****************************************************************************/
u32 XFpga_PL_Preconfig(const XFpga_Info *PLInfoPtr)
{
	u32 Status;
	XFpga XFpgaInstance = {0U};

	/* Initialize the XFpga Instance */
	Status = XFpga_Initialize(&XFpgaInstance);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	if (XFpgaInstance.XFpga_PreConfig == NULL) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
			"%s Implementation not exists..\r\n", __FUNCTION__);
		goto END;
	}

	XFpgaInstance.PLInfo.Flags = PLInfoPtr->Flags;

	Status = XFpgaInstance.XFpga_PreConfig(&XFpgaInstance);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_UPDATE_ERR(XFPGA_PRE_CONFIG_ERROR, Status);
	}

END:
	return Status;
}

/*****************************************************************************/
/* This function write count bytes of configuration data into the PL.
 *
 * @param PLInfoPtr Pointer to the XFgpa_Info structure
 *
 * @return Codes as mentioned in xilfpga.h
 *****************************************************************************/
u32 XFpga_PL_WriteToPl(const XFpga_Info *PLInfoPtr)
{
	 u32 Status;
	 XFpga_Write WriteInfo = {0U};
	 XFpga XFpgaInstance = {0U};

	 /* Initialize the XFpga Instance */
	 Status = XFpga_Initialize(&XFpgaInstance);
	 if (Status != XFPGA_SUCCESS) {
		 goto END;
	 }

	 if (XFpgaInstance.XFpga_WriteToPl == NULL) {
		 Status = XFPGA_OPS_NOT_IMPLEMENTED;
		 Xfpga_Printf(XFPGA_DEBUG,
			 "%s Implementation not exists..\r\n", __FUNCTION__);
		 goto END;
	 }

	 WriteInfo.BitstreamAddr = PLInfoPtr->BitstreamAddr;
	 WriteInfo.AddrPtr_Size = PLInfoPtr->AddrPtr;
	 WriteInfo.Flags = PLInfoPtr->Flags;
	 XFpgaInstance.WriteInfoPtr = &WriteInfo;
	 (void)memcpy(&XFpgaInstance.PLInfo.SecureImageInfo,
			 &PLInfoPtr->SecureImageInfo,
			 sizeof(PLInfoPtr->SecureImageInfo));

	 Status = XFpgaInstance.XFpga_WriteToPl(&XFpgaInstance);
	 if (Status != XFPGA_SUCCESS) {
		 Status = XFPGA_UPDATE_ERR(XFPGA_WRITE_BITSTREAM_ERROR, Status);
	}

END:
	return Status;
}

/*****************************************************************************/
/** This function set FPGA to operating state after writing.
 * @param PLInfoPtr Pointer to the XFgpa_Info structure
 *
 * @return Codes as mentioned in xilfpga.h
 *****************************************************************************/
u32 XFpga_PL_PostConfig(const XFpga_Info *PLInfoPtr)
{
	u32 Status;
	XFpga XFpgaInstance = {0U};
	XFpga_Write WriteInfo = {0U};

	/* Initialize the XFpga Instance */
	Status = XFpga_Initialize(&XFpgaInstance);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	if (XFpgaInstance.XFpga_PostConfig == NULL) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
			"%s Implementation not exists..\r\n", __FUNCTION__);
		goto END;
	}

	WriteInfo.BitstreamAddr = PLInfoPtr->BitstreamAddr;
	WriteInfo.AddrPtr_Size = PLInfoPtr->AddrPtr;
	WriteInfo.Flags = PLInfoPtr->Flags;
	XFpgaInstance.WriteInfoPtr = &WriteInfo;
	XFpgaInstance.PLInfo.Flags = PLInfoPtr->Flags;

	Status = XFpgaInstance.XFpga_PostConfig(&XFpgaInstance);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_UPDATE_ERR(XFPGA_POST_CONFIG_ERROR, Status);
	}

END:
	return Status;
}
/*****************************************************************************/
/**
 * This function provides functionality to read back the PL configuration data
 *
 * @param PLInfoPtr Pointer to the XFgpa_Info structure
 *
 * @return
 *	- XFPGA_SUCCESS if successful
 *	- XFPGA_FAILURE if unsuccessful
 *	- XFPGA_OPS_NOT_IMPLEMENTED if implementation not exists.
 *
 ****************************************************************************/
u32 XFpga_GetPlConfigData(const XFpga_Info *PLInfoPtr)
{
	u32 Status;
	XFpga_Read ReadInfo = {0U};
	XFpga XFpgaInstance = {0U};

	/* Initialize the XFpga Instance */
	Status = XFpga_Initialize(&XFpgaInstance);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	if (XFpgaInstance.XFpga_GetConfigData == NULL) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
			"%s Implementation not exists..\r\n", __FUNCTION__);
		goto END;
	}

	ReadInfo.ReadbackAddr = PLInfoPtr->ReadbackAddr;
	ReadInfo.ConfigReg_NumFrames = PLInfoPtr->NumFrames;
	XFpgaInstance.ReadInfoPtr = &ReadInfo;
END:
	Status = XFpgaInstance.XFpga_GetConfigData(&XFpgaInstance);

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
	XFpga XFpgaInstance = {0U};
	XFpga_Read ReadInfo = {0U};

	/* Initialize the XFpga Instance */
	Status = XFpga_Initialize(&XFpgaInstance);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	if (XFpgaInstance.XFpga_GetConfigReg == NULL) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
			"%s Implementation not exists..\r\n", __FUNCTION__);
		goto END;
	}

	ReadInfo.ReadbackAddr = Address;
	ReadInfo.ConfigReg_NumFrames = ConfigReg;
	XFpgaInstance.ReadInfoPtr = &ReadInfo;

	Status = XFpgaInstance.XFpga_GetConfigReg(&XFpgaInstance);

END:
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
	u32 Status;
	XFpga XFpgaInstance = {0U};

	(void)XFpga_Initialize(&XFpgaInstance);
	if (XFpgaInstance.XFpga_GetInterfaceStatus == NULL) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		Status = XFpgaInstance.XFpga_GetInterfaceStatus();
	}

	return Status;
}
#else
/*****************************************************************************/
/**The API is used to load the bitstream file into the PL region.
 *@param InstancePtr Pointer to the XFgpa structure.
 *
 *@param BitstreamImageAddr  Linear memory Bitstream image base address
 *
 *@param AddrPtr_Size Aes key address which is used for Decryption (or)
 *			In none Secure Bitstream used it is used to store size
 *			of Bitstream Image.
 *
 *@param Flags Flags are used to specify the type of Bitstream file.
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
u32 XFpga_PL_BitStream_Load(XFpga *InstancePtr,
			    UINTPTR BitstreamImageAddr,
			    UINTPTR AddrPtr_Size, u32 Flags)
{
	u32 Status;

	/* Prepare the FPGA to receive configuration Data */
	Status = XFpga_PL_Preconfig(InstancePtr);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	/* write count bytes of configuration data into the PL */
	Status = XFpga_PL_Write(InstancePtr, BitstreamImageAddr,
				AddrPtr_Size, Flags);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	/* set FPGA to operating state after writing */
	Status = XFpga_PL_PostConfig(InstancePtr);
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to validate the Bitstream Image
 * @param InstancePtr Pointer to the XFgpa structure
 *
 * @param BitstreamImageAddr  Linear memory Bitstream image base address
 *
 * @param AddrPtr_Size Aes key address which is used for Decryption (or)
 *			In none Secure Bitstream used it is used to store size
 *			of Bitstream Image.
 *
 * @param Flags Flags are used to specify the type of Bitstream file.
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
 * @return Codes as mentioned in xilfpga.h
 *****************************************************************************/
u32 XFpga_PL_ValidateImage(XFpga *InstancePtr,
			   UINTPTR BitstreamImageAddr,
			   UINTPTR AddrPtr_Size, u32 Flags)
{
	u32 Status;
	XFpga_Write WriteInfo = {0U};

	Xil_AssertNonvoid(InstancePtr != NULL);

	WriteInfo.BitstreamAddr = BitstreamImageAddr;
	WriteInfo.AddrPtr_Size = AddrPtr_Size;
	WriteInfo.Flags = Flags;

	InstancePtr->WriteInfoPtr = &WriteInfo;

	if (!InstancePtr->XFpga_ValidateBitstream) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		Status = InstancePtr->XFpga_ValidateBitstream(InstancePtr);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_UPDATE_ERR(XFPGA_VALIDATE_ERROR, Status);
		}
	}

	return Status;
}

/*****************************************************************************/
/* This function prepare the FPGA to receive configuration data.
 * @param InstancePtr is the pointer to the XFgpa.
 *
 * @return Codes as mentioned in xilfpga.h
 *****************************************************************************/
u32 XFpga_PL_Preconfig(XFpga *InstancePtr)
{
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);

	if (!InstancePtr->XFpga_PreConfig) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		Status = InstancePtr->XFpga_PreConfig(InstancePtr);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_UPDATE_ERR(XFPGA_PRE_CONFIG_ERROR,
						  Status);
		}
	}

	return Status;
}

/*****************************************************************************/
/* This function write count bytes of configuration data into the PL.
 *
 * @param InstancePtr Pointer to the XFgpa structure
 *
 * @param BitstreamImageAddr  Linear memory Bitstream image base address
 *
 * @param AddrPtr_Size Aes key address which is used for Decryption (or)
 *			In none Secure Bitstream used it is used to store size
 *			of Bitstream Image.
 *
 * @param Flags Flags are used to specify the type of Bitstream file.
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
 * @return Codes as mentioned in xilfpga.h
 *****************************************************************************/
u32 XFpga_PL_Write(XFpga *InstancePtr,UINTPTR BitstreamImageAddr,
		   UINTPTR AddrPtr_Size, u32 Flags)
{
	 u32 Status;
	 XFpga_Write WriteInfo = {0U};

	 Xil_AssertNonvoid(InstancePtr != NULL);

	 WriteInfo.BitstreamAddr = BitstreamImageAddr;
	 WriteInfo.AddrPtr_Size = AddrPtr_Size;
	 WriteInfo.Flags = Flags;

	 InstancePtr->WriteInfoPtr = &WriteInfo;

	if (!InstancePtr->XFpga_WriteToPl) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		Status = InstancePtr->XFpga_WriteToPl(InstancePtr);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_UPDATE_ERR(XFPGA_WRITE_BITSTREAM_ERROR,
						  Status);
		}
	}

	return Status;
}

/*****************************************************************************/
/** This function set FPGA to operating state after writing.
 * @param InstancePtr Pointer to the XFgpa structure
 *
 * @return Codes as mentioned in xilfpga.h
 *****************************************************************************/
u32 XFpga_PL_PostConfig(XFpga *InstancePtr)
{
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);

	if (!InstancePtr->XFpga_PostConfig) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		Status = InstancePtr->XFpga_PostConfig(InstancePtr);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_UPDATE_ERR(XFPGA_POST_CONFIG_ERROR,
						  Status);
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function provides functionality to read back the PL configuration data
 *
 * @param InstancePtr Pointer to the XFgpa structure
 * @ReadbackAddr Address which is used to store the PL readback data.
 * @ConfigReg_NumFrames Configuration register value to be returned (or)
 * 			The number of Fpga configuration frames to read
 *
 * @return
 *	- XFPGA_SUCCESS if successful
 *	- XFPGA_FAILURE if unsuccessful
 *	- XFPGA_OPS_NOT_IMPLEMENTED if implementation not exists.
 *
 ****************************************************************************/
u32 XFpga_GetPlConfigData(XFpga *InstancePtr, UINTPTR ReadbackAddr,
			  u32 ConfigReg_NumFrames)
{
	u32 Status;
	XFpga_Read ReadInfo = {0U};

	Xil_AssertNonvoid(InstancePtr != NULL);

	ReadInfo.ReadbackAddr = ReadbackAddr;
	ReadInfo.ConfigReg_NumFrames = ConfigReg_NumFrames;

	InstancePtr->ReadInfoPtr = &ReadInfo;

	if (!InstancePtr->XFpga_GetConfigData) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		Status = InstancePtr->XFpga_GetConfigData(InstancePtr);
	}

	return Status;
}

/*****************************************************************************/
/** This function is used to get the Dma Instance Pointer from the user
 * @param InstancePtr Pointer to the XFgpa structure.
 *
 * @DmaPtr CSUDMA  driver instance pointer.
 *
 * @return None.
 *
 *****************************************************************************/
void XFpga_GetDmaPtr(XFpga *InstancePtr, XCsuDma *DmaPtr)
{
	memcpy(&InstancePtr->PLInfo.PmcDmaIns, DmaPtr, sizeof(*DmaPtr));
}
#endif

