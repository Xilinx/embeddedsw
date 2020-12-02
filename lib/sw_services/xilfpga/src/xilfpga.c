/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
 * 4.2  Nava  06/08/16  Refactor the xilfpga library to support
 *			different PL programming Interfaces.
 * 4.2  adk   07/11/18  Added support for readback of PL configuration data.
 * 4.2  Nava  08/16/18	Modified the PL data handling Logic to support
 *			different PL programming interfaces.
 * 4.2  Nava  09/15/18  Fixed global function call-backs issue.
 * 5.0  Nava  05/11/18  Added full bitstream loading support for versal Platform.
 * 5.0  Div   01/21/19  Fixed misra-c required standard violation for zynqmp.
 * 5.0  Nava  02/06/19  Remove redundant API's from the interface agnostic layer
 *                      and make the existing API's generic to support both
 *                      ZynqMP and versal platforms.
 * 5.0 Nava   02/26/19  Update the data handling logic to avoid the code
 *		        duplication
 * 5.0 sne    03/27/19  Fixed misra-c violations.
 * 5.0 Nava   03/29/19  Removed vesal platform related changes.As per the new
 *                      design, the Bitstream loading for versal platform is
 *                      done by PLM based on the CDO's data exists in the PDI
 *                      images. So there is no need of xilfpga API's for versal
 *                      platform to configure the PL.
 * 5.1 Nava   06/27/19  Updated documentation for readback API's.
 * 5.1 Nava   07/16/19  Initialize empty status (or) status success to status failure
 *                      to avoid security violations.
 * 5.2 Nava   12/05/19  Added Versal platform support.
 * 5.2 Nava   02/14/20  Added Bitstream loading support by using IPI services
 *                      for ZynqMP platform.
 * 5.3 Nava   06/16/20  Modified the date format from dd/mm to mm/dd.
 * 5.3 Nava   06/23/20  Added asserts to validate input params.
 * 5.3 Nava   09/09/20  Replaced the asserts with input validations for non void
 *                      API's.
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xilfpga.h"

/************************** Function Prototypes ******************************/
static u32 XFpga_ValidateBitstreamParam(XFpga *InstancePtr,
					UINTPTR BitstreamImageAddr,
					UINTPTR AddrPtr_Size, u32 Flags);
/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**The API is used to load the bitstream file into the PL region.
 * It supports vivado generated Bitstream(*.bit, *.bin) and bootgen
 * generated Bitstream(*.bin) loading, Passing valid Bitstream size
 * (AddrPtr_Size) info is mandatory for vivado * generated Bitstream,
 * For bootgen generated Bitstreams it will take Bitstream size from
 * the Bitstream Header.
 *
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
	u32 Status = XFPGA_FAILURE;

	/* Validate the input arguments */
	Status = XFpga_ValidateBitstreamParam(InstancePtr, BitstreamImageAddr,
					      AddrPtr_Size, Flags);
	if(Status != XFPGA_SUCCESS) {
		goto END;
	}

	/* Validate Bitstream Image */
	Status = XFpga_PL_ValidateImage(InstancePtr, BitstreamImageAddr,
			AddrPtr_Size, Flags);
	if ((Status != XFPGA_OPS_NOT_IMPLEMENTED) &&
		(Status != XFPGA_SUCCESS)) {
		goto END;
	}

	/* Prepare the FPGA to receive configuration Data */
	Status = XFpga_PL_Preconfig(InstancePtr);
	if (Status != XFPGA_SUCCESS &&
	    Status != XFPGA_OPS_NOT_IMPLEMENTED) {
		goto END;
	}

	/* write count bytes of configuration data into the PL */
	Status = XFpga_PL_Write(InstancePtr,
				InstancePtr->WriteInfo.BitstreamAddr,
				InstancePtr->WriteInfo.AddrPtr_Size,
				InstancePtr->WriteInfo.Flags);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	/* set FPGA to operating state after writing */
	Status = XFpga_PL_PostConfig(InstancePtr);
	if (Status == XFPGA_OPS_NOT_IMPLEMENTED) {
		Status = XFPGA_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to validate the Bitstream Image
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
u32 XFpga_PL_ValidateImage(XFpga *InstancePtr,
			   UINTPTR BitstreamImageAddr,
			   UINTPTR AddrPtr_Size, u32 Flags)
{
	u32 Status = XFPGA_VALIDATE_ERROR;

	/* Validate the input arguments */
	Status = XFpga_ValidateBitstreamParam(InstancePtr, BitstreamImageAddr,
					      AddrPtr_Size, Flags);
	if(Status != XFPGA_SUCCESS) {
		goto END;
	}

	InstancePtr->WriteInfo.BitstreamAddr = BitstreamImageAddr;
	InstancePtr->WriteInfo.AddrPtr_Size = AddrPtr_Size;
	InstancePtr->WriteInfo.Flags = Flags;

	if (InstancePtr->XFpga_ValidateBitstream == NULL) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		Status = InstancePtr->XFpga_ValidateBitstream(InstancePtr);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_UPDATE_ERR(XFPGA_VALIDATE_ERROR, Status);
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/* This function prepare the FPGA to receive configuration data.
 *
 * @param InstancePtr is the pointer to the XFgpa.
 *
 * @return Codes as mentioned in xilfpga.h
 *****************************************************************************/
u32 XFpga_PL_Preconfig(XFpga *InstancePtr)
{
	u32 Status = XFPGA_PRE_CONFIG_ERROR;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = XFPGA_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->XFpga_PreConfig == NULL) {
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
END:
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
	 u32 Status = XFPGA_WRITE_BITSTREAM_ERROR;

	/* Validate the input arguments */
	Status = XFpga_ValidateBitstreamParam(InstancePtr, BitstreamImageAddr,
					      AddrPtr_Size, Flags);
	if(Status != XFPGA_SUCCESS) {
		goto END;
	}

	InstancePtr->WriteInfo.BitstreamAddr = BitstreamImageAddr;
	InstancePtr->WriteInfo.AddrPtr_Size = AddrPtr_Size;
	InstancePtr->WriteInfo.Flags = Flags;

	if (InstancePtr->XFpga_WriteToPl == NULL) {
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

END:
	return Status;
}

/*****************************************************************************/
/** This function set FPGA to operating state after writing.
 *
 * @param InstancePtr Pointer to the XFgpa structure
 *
 * @return Codes as mentioned in xilfpga.h
 *****************************************************************************/
u32 XFpga_PL_PostConfig(XFpga *InstancePtr)
{
	u32 Status = XFPGA_POST_CONFIG_ERROR;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = XFPGA_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->XFpga_PostConfig == NULL) {
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

END:
	return Status;
}

#ifndef versal
/*****************************************************************************/
/**
 * This function provides functionality to read back the PL configuration data
 *
 * @param InstancePtr Pointer to the XFgpa structure
 *
 * @param ReadbackAddr Address which is used to store the PL readback data.
 *
 * @param NumFrames The number of Fpga configuration frames to read.
 *
 * @return
 *	- XFPGA_SUCCESS if successful
 *	- XFPGA_FAILURE if unsuccessful
 *	- XFPGA_OPS_NOT_IMPLEMENTED if implementation not exists.
 *
 ****************************************************************************/
u32 XFpga_GetPlConfigData(XFpga *InstancePtr, UINTPTR ReadbackAddr,
			  u32 NumFrames)
{
	u32 Status = XFPGA_FAILURE;

	/* Assert validates the input arguments */
	if ((InstancePtr == NULL) || (ReadbackAddr == (UINTPTR)NULL) ||
	    (NumFrames == 0U)) {
		Status = XFPGA_INVALID_PARAM;
		goto END;
	}

	InstancePtr->ReadInfo.ReadbackAddr = ReadbackAddr;
	InstancePtr->ReadInfo.ConfigReg_NumFrames = NumFrames;
	if (InstancePtr->XFpga_GetConfigData == NULL) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		Status = InstancePtr->XFpga_GetConfigData(InstancePtr);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function provides PL specific configuration register values
 *
 * @param InstancePtr Pointer to the XFgpa structure
 *
 * @param ReadbackAddr Address which is used to store the PL Configuration
 *		       register data.
 *
 * @param ConfigRegAddr Configuration register address as mentioned in the
 *			ug570.
 *
 * @return
 *	- XFPGA_SUCCESS if successful
 *	- XFPGA_FAILURE if unsuccessful
 *	- XFPGA_OPS_NOT_IMPLEMENTED if implementation not exists.
 *
 ****************************************************************************/
u32 XFpga_GetPlConfigReg(XFpga *InstancePtr, UINTPTR ReadbackAddr,
						 u32 ConfigRegAddr)
{
	u32 Status = XFPGA_FAILURE;

	/* Assert validates the input arguments */
	if ((InstancePtr == NULL) ||(ReadbackAddr == (UINTPTR)NULL)) {
		Status = XFPGA_INVALID_PARAM;
		goto END;
	}

	if ((ConfigRegAddr != CRC) && (ConfigRegAddr != FAR1) &&
	    (ConfigRegAddr != FDRI) && (ConfigRegAddr != FDRO) &&
	    (ConfigRegAddr != CMD) && (ConfigRegAddr != CTL0) &&
	    (ConfigRegAddr != MASK) && (ConfigRegAddr != STAT) &&
	    (ConfigRegAddr != LOUT) && (ConfigRegAddr != COR0) &&
	    (ConfigRegAddr != MFWR) && (ConfigRegAddr != CBC) &&
	    (ConfigRegAddr != IDCODE) && (ConfigRegAddr != AXSS) &&
	    (ConfigRegAddr != COR1) && (ConfigRegAddr != WBSTAR) &&
	    (ConfigRegAddr != TIMER) && (ConfigRegAddr != BOOTSTS) &&
	    (ConfigRegAddr != CTL1)) {
		Status = XFPGA_INVALID_PARAM;
		goto END;
	}

	InstancePtr->ReadInfo.ReadbackAddr = ReadbackAddr;
	InstancePtr->ReadInfo.ConfigReg_NumFrames = ConfigRegAddr;

	if (InstancePtr->XFpga_GetConfigReg == NULL) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
			"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		Status = InstancePtr->XFpga_GetConfigReg(InstancePtr);
	}

END:
	return Status;
}

/*****************************************************************************/
/** This function provides the STATUS of PL programming interface
 *
 * @param InstancePtr Pointer to the XFgpa structure
 *
 * @return Status of the PL programming interface
 *
 *****************************************************************************/
u32 XFpga_InterfaceStatus(XFpga *InstancePtr)
{
	u32 RegVal = XFPGA_INVALID_INTERFACE_STATUS;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		goto END;
	}

	if (InstancePtr->XFpga_GetInterfaceStatus == NULL) {
		Xfpga_Printf(XFPGA_DEBUG,
		"%s Implementation not exists..\r\n", __FUNCTION__);
	} else {
		RegVal = InstancePtr->XFpga_GetInterfaceStatus();
	}

END:
	return RegVal;
}
#endif

/*****************************************************************************/
/**
 * @brief This function is used to validate the Bitstream parameters.
 *
 * @param InstancePtr Pointer to the XFgpa structure.
 *
 * @param BitstreamImageAddr Linear memory Bitstream image base address
 *
 * @param AddrPtr_Size Aes key address which is used for Decryption (or)
 *                      In none Secure Bitstreams it is used to store size
 *                      of Bitstream Image.
 * @param Flags Flags are used to specify the type of Bitstream file.
 *
 * @return
 *      - XFPGA_SUCCESS on success
 *      - Error code on failure.
 *
 *****************************************************************************/
static u32 XFpga_ValidateBitstreamParam(XFpga *InstancePtr,
					UINTPTR BitstreamImageAddr,
					UINTPTR AddrPtr_Size, u32 Flags)
{
	u32 Status = XFPGA_INVALID_PARAM;

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (BitstreamImageAddr == NULL)) {
		goto END;
	}
#ifdef versal
	if ((Flags != XFPGA_PDI_LOAD) && (Flags != XFPGA_DELAYED_PDI_LOAD)) {
		goto END;
	}
#else
	if ((Flags & (~(XFPGA_SECURE_FLAGS | XFPGA_PARTIAL_EN))) != 0U) {
		goto END;
	}

	if (((Flags & XFPGA_AUTHENTICATION_DDR_EN) != 0U) &&
	    ((Flags & XFPGA_AUTHENTICATION_OCM_EN) != 0U)) {
		goto END;
	}

	if (((Flags & XFPGA_ENCRYPTION_USERKEY_EN) != 0U) &&
	    ((Flags & XFPGA_ENCRYPTION_DEVKEY_EN) != 0U)) {
		goto END;
	}

	if (((Flags & XFPGA_ENCRYPTION_USERKEY_EN) != 0U) &&
	    (AddrPtr_Size == (UINTPTR)NULL)) {
		goto END;
	}
#endif
	Status = XFPGA_SUCCESS;

END:
	return Status;
}
