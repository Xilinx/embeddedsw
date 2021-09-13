/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
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
 * 6.0 Nava   12/14/20  In XFpga_PL_BitStream_Load() API the argument
 *                      AddrPtr_Size is being used for multiple purposes.
 *                      Use of the same variable for multiple purposes can
 *                      make it more difficult for a person to read (or)
 *                      understand the code and also it leads to a safety
 *                      violation. fixes this  issue by adding a separate
 *                      function arguments to read KeyAddr and
 *                      Size(Bitstream size).
 * 6.0 Nava   01/07/21  Fixed misra-c required standard violations.
 * 6.0 Nava   01/08/21  Removed unwanted if else conditions.
 * 6.0 Nava   01/20/21  Reset the status variable to fail to avoid safety
 *                      violations.
 * 6.0 Nava   01/21/21  Make Status variable volatile to avoid compiler
 *                      optimizations.
 * 6.0 Nava   02/22/21  Fixed doxygen issues.
 * 6.0 Nava   05/17/21  Fixed misra-c violations.
 * 6.1 Nava   09/13/21  Fixed compilation warning for versal platform.
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xilfpga.h"

/************************** Function Prototypes ******************************/
#ifndef versal
/* @cond nocomments */
static u32 XFpga_ValidateBitstreamParam(const XFpga *InstancePtr,
					UINTPTR BitstreamImageAddr,
					UINTPTR KeyAddr, u32 Flags);
/* @endcond */
#endif
/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**The API is used to load the bitstream file into the PL region.
 * It supports the Vivado-generated bitstream(*.bit, *.bin) and
 * Bootgen-generated bitstream(*.bin) loading, Passing valid
 * bitstream size(Size) information is mandatory for Vivado-generated
 * bitstream, For Bootgen-generated bitstreams bitstream size is taken
 * from the bitstream header.
 *
 *@param InstancePtr Pointer to the XFpga structure.
 *
 *@param BitstreamImageAddr  Linear memory bitstream image base address
 *
 *@param AddrPtr_Size  Aes key address which is used for decryption (or)
 *                     In none secure bitstream used it is used store size
 *                     of bitstream image.
 *
 *@param Flags Flags are used to specify the type of bitstream file.
 *                      * BIT(0) - Bitstream type
 *                                      * 0 - Full bitstream
 *                                      * 1 - Partial bitstream
 *                                      * 1 - Enable
 *                      * BIT(1) - Authentication using DDR
 *                                      * 1 - Enable
 *                                      * 0 - Disable
 *                      * BIT(2) - Authentication using OCM
 *                                      * 1 - Enable
 *                                      * 0 - Disable
 *                      * BIT(3) - User-key Encryption
 *                                      * 1 - Enable
 *                                      * 0 - Disable
 *                      * BIT(4) - Device-key Encryption
 *                                      * 1 - Enable
 *                                      * 0 - Disable
 *
 *@return
 *      - XFPGA_SUCCESS on success
 *      - Error code on failure.
 *      - XFPGA_VALIDATE_ERROR.
 *      - XFPGA_PRE_CONFIG_ERROR.
 *      - XFPGA_WRITE_BITSTREAM_ERROR.
 *      - XFPGA_POST_CONFIG_ERROR.
 *
 *@note
 *      - This API will be deprecated in the 2022.1 release.
 *        Use the updated 'XFpga_BitStream_Load()' API to
 *        perform the same functionality.
 *
 *****************************************************************************/
u32 XFpga_PL_BitStream_Load(XFpga *InstancePtr,
			    UINTPTR BitstreamImageAddr,
			    UINTPTR AddrPtr_Size, u32 Flags)
{
	u32 Status = XFPGA_FAILURE;
	UINTPTR KeyPtr;
	u32 Size;

	xil_printf(" %s: API will be deprecated in the 2022.1 release.\n"
		   "Use the updated 'XFpga_BitStream_Load()' API to perform\n"
		   "the same functionality\r\n", __func__);

	if ((Flags & XFPGA_ENCRYPTION_USERKEY_EN) == XFPGA_ENCRYPTION_USERKEY_EN) {
		KeyPtr = AddrPtr_Size;
		Size = 0U;
	} else {
		KeyPtr = 0U;
		Size = AddrPtr_Size;
	}

	Status = XFpga_BitStream_Load(InstancePtr, BitstreamImageAddr,
				      KeyPtr, Size, Flags);

	return Status;
}

/*****************************************************************************/
/**The API is used to load the bitstream file into the PL region.
 * It supports the Vivado-generated bitstream(*.bit, *.bin) and
 * Bootgen-generated bitstream(*.bin) loading, Passing valid
 * bitstream size(Size) information is mandatory for Vivado-generated
 * bitstream, For Bootgen-generated bitstreams bitstream size is taken
 * from the bitstream header.
 *
 *@param InstancePtr Pointer to the XFpga structure.
 *
 *@param BitstreamImageAddr  Linear memory bitstream image base address
 *
 *@param KeyAddr Aes key address which is used for decryption.
 *
 *@param Size Used to store size of bitstream image.
 *
 *@param Flags Flags are used to specify the type of bitstream file.
 *			* BIT(0) - Bitstream type
 *					* 0 - Full bitstream
 *					* 1 - Partial bitstream
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
u32 XFpga_BitStream_Load(XFpga *InstancePtr,
			 UINTPTR BitstreamImageAddr,
			 UINTPTR KeyAddr, u32 Size, u32 Flags)
{
	volatile u32 Status = XFPGA_INVALID_PARAM;
	UINTPTR	KeyPtr = KeyAddr;

	/* Validate the input arguments */
#ifdef versal
	if ((Flags != XFPGA_PDI_LOAD) && (Flags != XFPGA_DELAYED_PDI_LOAD)) {
		goto END;
	}
#else
	Status = XFpga_ValidateBitstreamParam(InstancePtr, BitstreamImageAddr,
					      KeyPtr, Flags);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}
#endif
	/* Validate Bitstream Image */
	Status = XFPGA_VALIDATE_ERROR;
	Status = XFpga_ValidateImage(InstancePtr, BitstreamImageAddr,
				     KeyPtr, Size, Flags);
	if ((Status != XFPGA_OPS_NOT_IMPLEMENTED) &&
	    (Status != XFPGA_SUCCESS)) {
		goto END;
	}

	if (Status == XFPGA_OPS_NOT_IMPLEMENTED) {
		InstancePtr->WriteInfo.BitstreamAddr = BitstreamImageAddr;
		InstancePtr->WriteInfo.Flags = Flags;
		InstancePtr->WriteInfo.KeyAddr = KeyAddr;
		InstancePtr->WriteInfo.Size = Size;
	}

	/* Prepare the FPGA to receive configuration Data */
	Status = XFPGA_PRE_CONFIG_ERROR;
	Status = XFpga_PL_Preconfig(InstancePtr);
	if ((Status != XFPGA_SUCCESS) &&
	    (Status != XFPGA_OPS_NOT_IMPLEMENTED)) {
		goto END;
	}

	/* write count bytes of configuration data into the PL */
	Status = XFPGA_WRITE_BITSTREAM_ERROR;
	Status = XFpga_Write_Pl(InstancePtr,
			     InstancePtr->WriteInfo.BitstreamAddr,
			     InstancePtr->WriteInfo.KeyAddr,
			     InstancePtr->WriteInfo.Size,
			     InstancePtr->WriteInfo.Flags);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	/* set FPGA to operating state after writing */
	Status = XFPGA_POST_CONFIG_ERROR;
	Status = XFpga_PL_PostConfig(InstancePtr);
	if (Status == XFPGA_OPS_NOT_IMPLEMENTED) {
		Status = XFPGA_SUCCESS;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to validate the bitstream image.
 *
 * @param InstancePtr Pointer to the XFpga structure
 *
 * @param BitstreamImageAddr  Linear memory bitstream image base address
 *
 * @param AddrPtr_Size  Aes key address which is used for decryption (or)
 *                      In none secure bitstream used it is used store size
 *                      of bitstream image.
 *
 * @param Flags Flags are used to specify the type of bitstream file.
 *                      * BIT(0) - bitstream type
 *                                      * 0 - Full bitstream
 *                                      * 1 - Partial bitstream
 *                      * BIT(1) - Authentication using DDR
 *                                      * 1 - Enable
 *                                      * 0 - Disable
 *                      * BIT(2) - Authentication using OCM
 *                                      * 1 - Enable
 *                                      * 0 - Disable
 *                      * BIT(3) - User-key Encryption
 *                                      * 1 - Enable
 *                                      * 0 - Disable
 *                      * BIT(4) - Device-key Encryption
 *                                      * 1 - Enable
 *                                      * 0 - Disable
 *
 * @return Codes as mentioned in xilfpga.h
 *
 * @note
 *      - This API will be deprecated in the 2022.1 release.
 *        Use the updated 'XFpga_ValidateImage()' API to
 *        perform the same functionality.
 *
 *****************************************************************************/
u32 XFpga_PL_ValidateImage(XFpga *InstancePtr,
                           UINTPTR BitstreamImageAddr,
                           UINTPTR AddrPtr_Size, u32 Flags)
{
        u32 Status = XFPGA_VALIDATE_ERROR;
	UINTPTR KeyPtr;
	u32 Size;

	xil_printf(" %s: API will be deprecated in the 2022.1 release.\n"
                   "Use the updated 'XFpga_ValidateImage()' API to perform\n"
                   "the same functionality\r\n", __func__);

	if ((Flags & XFPGA_ENCRYPTION_USERKEY_EN) == XFPGA_ENCRYPTION_USERKEY_EN) {
		KeyPtr = AddrPtr_Size;
		Size = 0U;
	} else {
		KeyPtr = 0U;
		Size = AddrPtr_Size;
	}

	Status = XFpga_ValidateImage(InstancePtr, BitstreamImageAddr,
				KeyPtr, Size, Flags);

	return Status;
}

/*****************************************************************************/
/**
 * This function is used to validate the bitstream image.
 *
 * @param InstancePtr Pointer to the XFpga structure
 *
 * @param BitstreamImageAddr  Linear memory bitstream image base address
 *
 * @param KeyAddr Aes key address which is used for decryption.
 *
 * @param Size Used to store size of bitstream image.
 *
 * @param Flags Flags are used to specify the type of bitstream file.
 *			* BIT(0) - Bitstream type
 *					* 0 - Full bitstream
 *					* 1 - Partial bitstream
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
u32 XFpga_ValidateImage(XFpga *InstancePtr,
			UINTPTR BitstreamImageAddr,
			UINTPTR KeyAddr, u32 Size, u32 Flags)
{
	volatile u32 Status = XFPGA_INVALID_PARAM;

	/* Validate the input arguments */
#ifdef versal
	if ((Flags != XFPGA_PDI_LOAD) && (Flags != XFPGA_DELAYED_PDI_LOAD)) {
		goto END;
	}
#else
	Status = XFpga_ValidateBitstreamParam(InstancePtr, BitstreamImageAddr,
					      KeyAddr, Flags);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}
#endif
	if (InstancePtr->XFpga_ValidateBitstream == NULL) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"XFpga_PL_ValidateImage Implementation not exists..\r\n");
		goto END;
	}

	InstancePtr->WriteInfo.BitstreamAddr = BitstreamImageAddr;
	InstancePtr->WriteInfo.Flags = Flags;
	InstancePtr->WriteInfo.KeyAddr = KeyAddr;
	InstancePtr->WriteInfo.Size = Size;
	Status = XFPGA_VALIDATE_ERROR;
	Status = InstancePtr->XFpga_ValidateBitstream(InstancePtr);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_UPDATE_ERR(XFPGA_VALIDATE_ERROR, Status);
	}

END:
	return Status;
}

/*****************************************************************************/
/**This function prepares the FPGA to receive configuration data.
 *
 * @param InstancePtr is the pointer to the XFpga.
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
		"XFpga_PL_Preconfig Implementation not exists..\r\n");
		goto END;
	}

	Status = InstancePtr->XFpga_PreConfig(InstancePtr);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_UPDATE_ERR(XFPGA_PRE_CONFIG_ERROR,
					  Status);
	}
END:
	return Status;
}

/*****************************************************************************/
/**This function writes the count bytes of configuration data into the PL.
 *
 * @param InstancePtr Pointer to the XFpga structure
 *
 * @param BitstreamImageAddr  Linear memory bitstream image base address
 *
 * @param AddrPtr_Size  Aes key address which is used for decryption (or)
 *                      In none secure bitstream used it is used store size
 *                      of bitstream image.
 *
 * @param Flags Flags are used to specify the type of bitstream file.
 *                      * BIT(0) - Bitstream type
 *                                      * 0 - Full bitstream
 *                                      * 1 - Partial bitstream
 *                      * BIT(1) - Authentication using DDR
 *                                      * 1 - Enable
 *                                      * 0 - Disable
 *                      * BIT(2) - Authentication using OCM
 *                                      * 1 - Enable
 *                                      * 0 - Disable
 *                      * BIT(3) - User-key Encryption
 *                                      * 1 - Enable
 *                                      * 0 - Disable
 *                      * BIT(4) - Device-key Encryption
 *                                      * 1 - Enable
 *                                      * 0 - Disable
 *
 * @return Codes as mentioned in xilfpga.h
 *
 * @note
 *      - This API will be deprecated in the 2022.1 release.
 *        Use the updated 'XFpga_Write_Pl()' API to perform
 *        the same functionality.
 *
 *****************************************************************************/
u32 XFpga_PL_Write(XFpga *InstancePtr,UINTPTR BitstreamImageAddr,
                   UINTPTR AddrPtr_Size, u32 Flags)
{
	u32 Status = XFPGA_WRITE_BITSTREAM_ERROR;
	UINTPTR KeyPtr;
	u32 Size;

	xil_printf(" %s: API will be deprecated in the 2022.1 release.\n"
                   "Use the updated 'XFpga_Write_Pl()' API to perform\n"
                   "the same functionality\r\n", __func__);

	if ((Flags & XFPGA_ENCRYPTION_USERKEY_EN) == XFPGA_ENCRYPTION_USERKEY_EN) {
		KeyPtr = AddrPtr_Size;
		Size = 0U;
	} else {
		KeyPtr = 0U;
		Size = AddrPtr_Size;
	}

	Status = XFpga_Write_Pl(InstancePtr, BitstreamImageAddr,
			     KeyPtr, Size, Flags);

	return Status;
}

/*****************************************************************************/
/**This function writes the count bytes of configuration data into the PL.
 *
 * @param InstancePtr Pointer to the XFpga structure
 *
 * @param BitstreamImageAddr  Linear memory bitstream image base address
 *
 * @param KeyAddr Aes key address which is used for decryption.
 *
 * @param Size Used to store size of bitstream image.
 *
 * @param Flags Flags are used to specify the type of bitstream file.
 *			* BIT(0) - Bitstream type
 *					* 0 - Full bitstream
 *					* 1 - Partial bitstream
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
u32 XFpga_Write_Pl(XFpga *InstancePtr,UINTPTR BitstreamImageAddr,
		UINTPTR KeyAddr, u32 Size, u32 Flags)
{
	volatile u32 Status = XFPGA_INVALID_PARAM;

	/* Validate the input arguments */
#ifdef versal
	if ((Flags != XFPGA_PDI_LOAD) && (Flags != XFPGA_DELAYED_PDI_LOAD)) {
		goto END;
	}
#else
	Status = XFpga_ValidateBitstreamParam(InstancePtr, BitstreamImageAddr,
					      KeyAddr, Flags);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}
#endif
	if (InstancePtr->XFpga_WriteToPl == NULL) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"XFpga_Write_Pl Implementation not exists..\r\n");
		goto END;
	}

	InstancePtr->WriteInfo.BitstreamAddr = BitstreamImageAddr;
	InstancePtr->WriteInfo.Flags = Flags;
	InstancePtr->WriteInfo.KeyAddr = KeyAddr;
	InstancePtr->WriteInfo.Size = Size;
	Status = XFPGA_WRITE_BITSTREAM_ERROR;
	Status = InstancePtr->XFpga_WriteToPl(InstancePtr);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_UPDATE_ERR(XFPGA_WRITE_BITSTREAM_ERROR,
					  Status);
	}

END:
	return Status;
}

/*****************************************************************************/
/** This function sets the FPGA to the operating state after writing.
 *
 * @param InstancePtr Pointer to the XFpga structure
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
		"XFpga_PL_PostConfig Implementation not exists..\r\n");
		goto END;
	}

	Status = InstancePtr->XFpga_PostConfig(InstancePtr);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_UPDATE_ERR(XFPGA_POST_CONFIG_ERROR,
					  Status);
	}

END:
	return Status;
}

#ifndef versal
/*****************************************************************************/
/**
 * This function provides functionality to read back the PL configuration data
 *
 * @param InstancePtr Pointer to the XFpga structure
 *
 * @param ReadbackAddr Address which is used to store the PL readback data.
 *
 * @param NumFrames The number of FPGA configuration frames to read.
 *
 * @return
 *	- XFPGA_SUCCESS, if successful
 *	- XFPGA_FAILURE, if unsuccessful
 *	- XFPGA_OPS_NOT_IMPLEMENTED, if implementation not exists.
 * @note
 *	- This API is not supported for the Versal platform.
 *
 ****************************************************************************/
u32 XFpga_GetPlConfigData(XFpga *InstancePtr, UINTPTR ReadbackAddr,
			  u32 NumFrames)
{
	u32 Status = XFPGA_FAILURE;

	/* Assert validates the input arguments */
	if ((InstancePtr == NULL) || (ReadbackAddr == 0U) ||
	    (NumFrames == 0U)) {
		Status = XFPGA_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->XFpga_GetConfigData == NULL) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"XFpga_GetPlConfigData Implementation not exists..\r\n");
		goto END;
	}

	InstancePtr->ReadInfo.ReadbackAddr = ReadbackAddr;
	InstancePtr->ReadInfo.ConfigReg_NumFrames = NumFrames;
	Status = InstancePtr->XFpga_GetConfigData(InstancePtr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function provides PL specific configuration register values
 *
 * @param InstancePtr Pointer to the XFpga structure
 *
 * @param ReadbackAddr Address which is used to store the PL Configuration
 *		       register data.
 *
 * @param ConfigRegAddr Configuration register address as mentioned in the
 *			UG570.
 *
 * @return
 *	- XFPGA_SUCCESS if, successful
 *	- XFPGA_FAILURE if, unsuccessful
 *	- XFPGA_OPS_NOT_IMPLEMENTED, if implementation not exists.
 * @note
 *      - This API is not supported for the Versal platform.
 *
 ****************************************************************************/
u32 XFpga_GetPlConfigReg(XFpga *InstancePtr, UINTPTR ReadbackAddr,
						 u32 ConfigRegAddr)
{
	u32 Status = XFPGA_FAILURE;

	/* Validates the input arguments */
	if ((InstancePtr == NULL) ||(ReadbackAddr == 0U)) {
		Status = XFPGA_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->XFpga_GetConfigReg == NULL) {
		Status = XFPGA_OPS_NOT_IMPLEMENTED;
		Xfpga_Printf(XFPGA_DEBUG,
		"XFpga_GetPlConfigReg Implementation not exists..\r\n");
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
	Status = InstancePtr->XFpga_GetConfigReg(InstancePtr);

END:
	return Status;
}

/*****************************************************************************/
/** This function provides the status of the PL programming interface
 *
 * @param InstancePtr Pointer to the XFpga structure
 *
 * @return Status of the PL programming interface
 *
 * @note
 *      - This API is not supported for the Versal platform.
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
		"XFpga_InterfaceStatus Implementation not exists..\r\n");
		goto END;
	}

	RegVal = InstancePtr->XFpga_GetInterfaceStatus();

END:
	return RegVal;
}

/*****************************************************************************/
/**
 * @cond nocomments
 *
 * @brief This function is used to validate the bitstream parameters.
 *
 * @param InstancePtr Pointer to the XFpga structure.
 *
 * @param BitstreamImageAddr Linear memory bitstream image base address
 *
 * @param KeyAddr Aes key address which is used for decryption.
 *
 * @param Size Used to store size of bitstream image.
 *
 * @param Flags Flags are used to specify the type of bitstream file.
 *
 * @return
 *      - XFPGA_SUCCESS on success
 *      - Error code on failure.
 *
 *****************************************************************************/
static u32 XFpga_ValidateBitstreamParam(const XFpga *InstancePtr,
					UINTPTR BitstreamImageAddr,
					UINTPTR KeyAddr, u32 Flags)
{
	u32 Status = XFPGA_INVALID_PARAM;

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (BitstreamImageAddr == 0U)) {
		goto END;
	}

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
	    (KeyAddr == 0U)) {
		goto END;
	}

	Status = XFPGA_SUCCESS;

END:
	return Status;
}

/*  @endcond */
#endif
