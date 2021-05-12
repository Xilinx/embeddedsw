/******************************************************************************
* Copyright (c) 2016 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilfpga_pcap.c
 *
 * This file contains the definitions of bitstream loading functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   Nava  06/08/16 Initial release
 * 1.1   Nava  11/16/16 Added PL power-up sequence.
 * 2.0	 Nava  01/10/17 Added Encrypted bitstream loading support.
 * 2.0   Nava  02/16/17 Added Authenticated bitstream loading support.
 * 2.1	 Nava  05/06/17	Correct the check logic issues in
 *			XFpga_PL_BitStream_Load()
 *			to avoid the unwanted blocking conditions.
 * 3.0   Nava  05/12/17 Added PL configuration registers readback support.
 * 4.0   Nava  02/08/18 Added Authenticated and Encypted Bitstream
 *			loading	support.
 * 4.0   Nava  03/02/18 Added the legacy bit file loading feature support
 *			from U-boot.
 *			and improve the error handling support by returning the
 *			proper ERROR value upon error conditions.
 * 4.1   Nava  03/07/18  For Secure Bitstream loading to avoid the Security
 *			violations Need to Re-validate the User Crypto flags
 *			with the Image Crypto operation by using the internal
 *			memory.To Fix this added a new API
 *			XFpga_ReValidateCryptoFlags().
 * 4.1   Nava  04/16/18 Added partial bitstream loading support.
 * 4.2   Nava  06/08/16 Refactor the xilfpga library to support
 *			different PL programming Interfaces.
 * 4.2   adk   07/11/18 Added support for readback of PL configuration data.
 * 4.2   Nava  07/22/18 Added XFpga_SelectEndianess() new API to Support
 *                      programming the vivado generated .bit and .bin files.
 * 4.2   Nava  08/16/18 Modified the PL data handling Logic to support
 *                      different PL programming interfaces.
 * 4.2	 adk   08/23/18 Added support for unaligned bitstream programming.
 * 4.2   adk   08/28/18 Fixed misra-c required standard violations.
 * 4.2   Nava  09/15/18 Fixed global function call-backs issue.
 * 5.0	 Nava  01/10/19	Improve the PS-PL resets handling.
 * 5.0   Nava  01/10/19 Improve the Image validation handling logic for
 *			bootgen created Bitstream Images.
 * 5.0	 Div   01/21/19	Fixed misra-c required standard violations.
 * 5.0  Nava   02/06/19 Remove redundant API's from the interface agnostic layer
 *                      and make the existing API's generic to support both
 *                      ZynqMP and versal platforms.
 * 5.0  Nava  02/26/19  Fix for power-up PL issue with pmufw.
 * 5.0  Nava  02/26/19  Update the data handling logic to avoid the code
 *                      duplication
 * 5.0  Nava  02/28/19  Handling all the 4 PS-PL resets irrespective of the
 *                      design configuration.
 * 5.0  vns   03/12/19  Modified secure stream switch related functions.
 * 5.0  Nava  03/19/19 In the current implementation, the SecureIv variable
 *                     is sharing between xilfpga and Xilsecure libraries.
 *                     To avoid data sharing conflicts removed SecureIV
 *                     shared variable dependency.
 * 5.0 Nava  03/21/19  Added Address alignment check. As CSUDMA expects word
 *                     aligned address. In case user passes an unaligned
 *                     address return error.
 * 5.0 sne   03/27/19  Fixed misra-c violations.
 * 5.0 Nava  04/23/19  Optimize the API's logic to avoid code duplication.
 * 5.1 Nava  06/27/19  Adds support to clear out the SHA3 engine.
 * 5.1 Nava  07/05/19  Zeroize the Secure data to avoid security violations.
 * 5.1 Nava  07/16/19  Begin all functions return status with failure and return
 *		       to success only on successful completion of the operation
 *                     of the functions.
 * 5.1 Nava  07/16/19  Improve error handling in the bitstream validation path.
 * 5.2 Nava  10/11/19  Clear the key info from DDR or Physical memory Once it
 *                     preserves into the internal memory.
 * 5.2 Nava  11/01/19  Clear the Aes-key info from internal memory after
 *                     completion of its usage.
 * 5.2 Nava  11/14/19  Rename the XFpga_GetPLConfigData() function name
 *                     to improve the code readability.
 * 5.2 Nava  12/06/19  Removed unwanted pcap interface status check In
 *                     XFpga_DecrypSecureHdr path.
 * 5.2 Nava  12/18/19  Fix for security violation in the readback path.
 * 5.2 Nava  01/02/20  Added conditional compilation support for readback feature.
 * 5.2 Nava  01/21/20  Replace event poll logic with Xil_WaitForEvent() API.
 * 5.2 Nava  02/20/20  Updated SECURE_MODE check handling logic by using conditional
 *                     compilation macro to Optimize XFpga_Validate BitstreamImage
 *                     function
 * 5.3 Nava  06/16/20  Modified the date format from dd/mm to mm/dd.
 * 5.3 Nava  06/16/20  clear the AES key from the KUP register upon GCM-tag
 *                     check failure.
 * 5.3 Nava  06/29/20  Added asserts to validate input params.
 * 5.3 Nava  09/09/20  Replaced the asserts with input validations for non void
 *                     API's.
 * 5.3 Nava  09/16/20  Added user configurable Enable/Disable Options for
 *                     readback operations.
 * 6.0 Nava  12/14/20  In XFpga_PL_BitStream_Load() API the argument
 *                     AddrPtr_Size is being used for multiple purposes.
 *                     Use of the same variable for multiple purposes can
 *                     make it more difficult for a person to read (or)
 *                     understand the code and also it leads to a safety
 *                     violation. fixes this  issue by adding a separate
 *                     function arguments to read KeyAddr and
 *                     Size(Bitstream size).
 * 6.0 Nava  01/07/21  Fixed misra-c required standard violations.
 * 6.0 Nava  01/20/21  Reset the status variable to fail to avoid safety
 *                     violations.
 * 6.0 Nava  01/21/21  Make Status variable volatile to avoid compiler
 *                     optimizations.
 * 6.0 Nava  02/22/21  Fixed doxygen issues.
 * 6.0 Nava  05/07/21  Added support to load the authenticated bitstream image
 *                     as non-secure image if RSA_EN is not programmed.
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xilfpga.h"

/* @cond nocomments */
/************************** Constant Definitions *****************************/
#ifdef __MICROBLAZE__
#define XPBR_SERV_EXT_PWRUPPLD		119U
#define XPBR_SERV_EXT_PLNONPCAPISO	162U
#define XPBR_SERV_EXT_TBL_MAX		256U
#endif

#define WORD_LEN			4U	/* Bytes */
#ifdef XFPGA_SECURE_MODE
#define KEY_LEN				64U	/* Bytes */
#define IV_LEN				24U	/* Bytes */
#define GCM_TAG_LEN			128U /* Bytes */
#define MAX_NIBBLES			8U
#define SIGNATURE_LEN			512U /* Bytes */
#define RSA_HASH_LEN			48U /* Bytes */
#define HASH_LEN			48U /* Bytes */
#define OCM_PL_ADDR			XFPGA_OCM_ADDRESS
#define AC_LEN				(0xEC0U)
#define PL_PARTATION_SIZE		(0x800000U)
#define PL_CHUNK_SIZE_BYTES		(1024U * 56U)
#define NUM_OF_PL_CHUNKS(Size)	((Size) / PL_CHUNK_SIZE_BYTES)
#endif

/**
 * Name Configuration Type1 packet headers masks
 */
#define XDC_TYPE_SHIFT                  29U
#define XDC_REGISTER_SHIFT              13U
#define XDC_OP_SHIFT                    27U
#define XDC_TYPE_1                      1U
#define XDC_TYPE_2			2U
#define OPCODE_NOOP			0U
#define OPCODE_READ                     1U
#define OPCODE_WRITE			2U
#define XFPGA_DESTINATION_PCAP_ADDR	(0XFFFFFFFFU)
#define XFPGA_PART_IS_ENC		(0x00000080U)
#define XFPGA_PART_IS_AUTH		(0x00008000U)
#define DUMMY_BYTE			(0xFFU)
#define SYNC_BYTE_POSITION		64U
#define BOOTGEN_DATA_OFFSET		0x2800U
#define XFPGA_ADDR_WORD_ALIGN_MASK	(0x3U)

#define XFPGA_AES_TAG_SIZE	(XSECURE_SECURE_HDR_SIZE + \
		XSECURE_SECURE_GCM_TAG_SIZE) /* AES block decryption tag size */

#define XFPGA_REG_CONFIG_CMD_LEN	9U
#define XFPGA_DATA_CONFIG_CMD_LEN	88U

/* Firmware State Definitions */
#define XFPGA_FIRMWARE_STATE_UNKNOWN	0U
#define XFPGA_FIRMWARE_STATE_SECURE	1U
#define XFPGA_FIRMWARE_STATE_NONSECURE	2U

/* PS-PL Reset Time */
#define XFPGA_PS_PL_RESET_TIME_US	1U

/**************************** Type Definitions *******************************/
#ifdef __MICROBLAZE__
typedef u32 (*XpbrServHndlr_t) (void);
#endif

/***************** Macros (Inline Functions) Definitions *********************/
#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))
#ifdef XFPGA_SECURE_READBACK_MODE
#define XFPGA_SECURE_READBACK_MODE_EN	1U
#else
#define XFPGA_SECURE_READBACK_MODE_EN	0U
#endif

/************************** Function Prototypes ******************************/
static u32 XFpga_PcapWaitForDone(void);
static u32 XFpga_WriteToPcap(u32 Size, UINTPTR BitstreamAddr);
static u32 XFpga_PcapInit(u32 Flags);
static u32 XFpga_PLWaitForDone(void);
static u32 XFpga_PowerUpPl(void);
static u32 XFpga_IsolationRestore(void);
void XFpga_PsPlGpioResetsLow(void);
void XFpga_PsPlGpioResetsHigh(void);
static u32 XFpga_ValidateCryptoFlags(const XSecure_ImageInfo *ImageInfo,
							u32 flags);
static u32 XFpga_ValidateBitstreamImage(XFpga  *InstancePtr);
static u32 XFpga_PreConfigPcap(XFpga *InstancePtr);
static u32 XFpga_WriteToPlPcap(XFpga *InstancePtr);
static u32 XFpga_PostConfigPcap(XFpga *InstancePtr);
static u32 XFpga_PcapStatus(void);
static void XFpga_SetFirmwareState(u8 State);
static u32 XFpga_SelectEndianess(u8 *Buf, u32 Size, u32 *Pos);
#if defined(XFPGA_READ_CONFIG_REG)
static u32 XFpga_GetConfigRegPcap(const XFpga *InstancePtr);
#endif
#if defined(XFPGA_READ_CONFIG_DATA)
static u32 XFpga_GetPLConfigDataPcap(const XFpga *InstancePtr);
static u32 XFpga_PcapWaitForidle(void);
static u32 Xfpga_Type2Pkt(u8 OpCode, u32 Size);
#endif
#if defined(XFPGA_READ_CONFIG_DATA) || defined(XFPGA_READ_CONFIG_REG)
static u32 Xfpga_RegAddr(u8 Register, u8 OpCode, u16 Size);
static u32 XFpga_GetFirmwareState(void);
#endif
#ifdef XFPGA_SECURE_MODE
static u32 XFpga_SecureLoadToPl(XFpga *InstancePtr);
static u32 XFpga_WriteEncryptToPcap(XFpga *InstancePtr);
static u32 XFpga_SecureBitstreamLoad(XFpga *InstancePtr);
static u32 XFpga_AuthPlChunksDdrOcm(XFpga *InstancePtr, u32 Size);
static u32 XFpga_AuthPlChunks(UINTPTR BitstreamAddr, u32 Size, UINTPTR AcAddr);
static u32 XFpga_ReAuthPlChunksWriteToPl(XFpgaPs_PlPartition *PlAesInfo,
					 UINTPTR BitstreamAddr,
					 u32 Size, u32 Flags);
static u32 XFpga_DecrptPlChunks(XFpgaPs_PlPartition *PartitionParams,
				UINTPTR ChunkAdrs, u32 ChunkSize);
static u32 XFpga_DecrptSetUpNextBlk(XFpgaPs_PlPartition *PartitionParams);
static void XFpga_DmaPlCopy(XCsuDma *InstancePtr, UINTPTR Src,
					u32 Size, u8 EnLast);
static u32 XFpga_DecrptPl(XFpgaPs_PlPartition *PartitionParams,
			  UINTPTR ChunkAdrs, u32 ChunkSize);
static u32 XFpga_DecrypSecureHdr(XSecure_Aes *InstancePtr, UINTPTR SrcAddr);
static u32 XFpga_AesInit(XSecure_Aes *InstancePtr, u32 *AesKupKey,
			 u32* IvPtr, char *KeyPtr, u32 Flags);
#endif
#ifdef __MICROBLAZE__
extern const XpbrServHndlr_t XpbrServHndlrTbl[XPBR_SERV_EXT_TBL_MAX];
#endif
/************************** Variable Definitions *****************************/
static XCsuDma *CsuDmaPtr;

/* Xilinx ZynqMp Vivado generated Bitstream header format */
static const u8 VivadoBinFormat[] = {
        0x00U, 0x00U, 0x00U, 0xBBU, /* Bus Width Sync Word */
        0x11U, 0x22U, 0x00U, 0x44U, /* Bus Width Detect Pattern */
        0xFFU, 0xFFU, 0xFFU, 0xFFU,
        0xFFU, 0xFFU, 0xFFU, 0xFFU,
        0xAAU, 0x99U, 0x55U, 0x66U, /* Sync Word */
};

/* Xilinx ZynqMp Bootgen generated Bitstream header format */
static const u8 BootgenBinFormat[] = {
        0xBBU, 0x00U, 0x00U, 0x00U, /* Bus Width Sync Word */
        0x44U, 0x00U, 0x22U, 0x11U, /* Bus Width Detect Pattern */
        0xFFU, 0xFFU, 0xFFU, 0xFFU,
        0xFFU, 0xFFU, 0xFFU, 0xFFU,
        0x66U, 0x55U, 0x99U, 0xAAU, /* Sync Word */
};
/* @endcond */

/*****************************************************************************/
/**
 * This API, when called, initializes the XFPGA interface with default settings.
 *
 * @param InstancePtr Pointer to the XFpga structure.
 *
 * @return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 ******************************************************************************/
u32 XFpga_Initialize(XFpga *InstancePtr) {
	u32 Status = XFPGA_FAILURE;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = XFPGA_INVALID_PARAM;
		goto END;
	}

	(void)memset(InstancePtr, 0, sizeof(*InstancePtr));
	InstancePtr->XFpga_ValidateBitstream = XFpga_ValidateBitstreamImage;
	InstancePtr->XFpga_PreConfig = XFpga_PreConfigPcap;
	InstancePtr->XFpga_WriteToPl = XFpga_WriteToPlPcap;
	InstancePtr->XFpga_PostConfig = XFpga_PostConfigPcap;
	InstancePtr->XFpga_GetInterfaceStatus = XFpga_PcapStatus;
#if defined(XFPGA_READ_CONFIG_REG)
	InstancePtr->XFpga_GetConfigReg = XFpga_GetConfigRegPcap;
#endif
#if defined(XFPGA_READ_CONFIG_DATA)
	InstancePtr->XFpga_GetConfigData = XFpga_GetPLConfigDataPcap;
#endif
	/* Initialize CSU DMA driver */
	CsuDmaPtr = Xsecure_GetCsuDma();
	if (CsuDmaPtr == NULL) {
		Status = XFPGA_PCAP_UPDATE_ERR(XFPGA_ERROR_CSUDMA_INIT_FAIL,
					       XFPGA_FAILURE);
	} else {
		Status = XFPGA_SUCCESS;
	}

END:
	return Status;
}

/*  @cond nocomments */

/*****************************************************************************/
/**
 * This function validate the Bitstream Image boot header and image header info,
 * also copies all the required details to the ImageInfo pointer.
 *
 * @param InstancePtr Pointer to the XFpga structure.
 *
 * @return	Returns Status
 *		- XST_SUCCESS on success
 *		- Error code on failure
 *		- XSECURE_AUTH_NOT_ENABLED - represents image is not
 *		authenticated.
 *
 *****************************************************************************/
static u32 XFpga_ValidateBitstreamImage(XFpga *InstancePtr)
{
	volatile u32 Status = XFPGA_FAILURE;
	XSecure_ImageInfo *ImageHdrDataPtr =
			&InstancePtr->PLInfo.SecureImageInfo;
	u32 BitstreamPos = 0U;
	u32 PartHeaderOffset = 0U;

#ifndef XFPGA_SECURE_MODE
	if ((InstancePtr->WriteInfo.Flags & XFPGA_SECURE_FLAGS) != 0U) {
		Status = XFPGA_PCAP_UPDATE_ERR((u32)XFPGA_ERROR_SECURE_MODE_EN,
				(u32)0U);
		Xfpga_Printf(XFPGA_DEBUG, "Fail to load: Enable secure mode "
			"and try Error Code: 0x%08x\r\n", Status);
		goto END;
	}
#endif

	if ((InstancePtr->WriteInfo.BitstreamAddr &
		XFPGA_ADDR_WORD_ALIGN_MASK) != 0U) {
		/* If the Address is not Word aligned return failure */
		Status = XFPGA_ERROR_UNALIGN_ADDR;
		goto END;
	}

	if ((InstancePtr->WriteInfo.Flags & XFPGA_SECURE_FLAGS) == 0U) {

		/* eFUSE checks */
		if ((XSecure_IsRsaEnabled() == XSECURE_ENABLED) ||
		    (XSecure_IsEncOnlyEnabled() == XSECURE_ENABLED)) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					(u32)XFPGA_ERROR_EFUSE_CHECK, (u32)0U);
			goto END;
		}

		if((u32)(memcmp((u8 *)(InstancePtr->WriteInfo.BitstreamAddr +
		   BOOTGEN_DATA_OFFSET + SYNC_BYTE_POSITION),
		   BootgenBinFormat, ARRAY_LENGTH(BootgenBinFormat)))== 0U) {
			BitstreamPos = BOOTGEN_DATA_OFFSET;
		} else {
			Status = XFpga_SelectEndianess(
				(u8 *)InstancePtr->WriteInfo.BitstreamAddr,
				(u32)InstancePtr->WriteInfo.Size,
				&BitstreamPos);

			if (Status != XFPGA_SUCCESS) {
				Status = XFPGA_PCAP_UPDATE_ERR(Status, (u32)0U);
				goto END;
			}

			goto UPDATE;
		}
	}

	Status = XSecure_AuthenticationHeaders(
				(u8 *)InstancePtr->WriteInfo.BitstreamAddr,
				ImageHdrDataPtr);
	if (Status != XFPGA_SUCCESS) {
		if (Status == XSECURE_ONLY_BHDR_AUTH_ALLOWED) {
			InstancePtr->WriteInfo.Flags &=
				~(XFPGA_AUTHENTICATION_DDR_EN |
				  XFPGA_AUTHENTICATION_OCM_EN);
		} else if (Status != XSECURE_AUTH_NOT_ENABLED) {
			Status = XFPGA_PCAP_UPDATE_ERR(XFPGA_ERROR_HDR_AUTH, Status);
			goto END;
		} else {
			Xfpga_Printf(XFPGA_DEBUG, "Image Authentication not enabled\r\n");
		}
	}

	/* Validate the User Flags for the Image Crypto operation */
	Status = XFPGA_FAILURE;
	Status = XFpga_ValidateCryptoFlags(ImageHdrDataPtr,
					   InstancePtr->WriteInfo.Flags);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_PCAP_UPDATE_ERR((u32)XFPGA_ERROR_CRYPTO_FLAGS, (u32)0U);
		Xfpga_Printf(XFPGA_DEBUG,
		"Crypto flags not matched with Image crypto operation "
		"with Error Code:0x%08x\r\n", Status);
		goto END;
	}

UPDATE:
	if ((InstancePtr->WriteInfo.Flags & XFPGA_SECURE_FLAGS) == 0U) {
		if (BitstreamPos == BOOTGEN_DATA_OFFSET) {
			PartHeaderOffset = Xil_In32(
					InstancePtr->WriteInfo.BitstreamAddr
					+ PARTATION_HEADER_OFFSET);
			InstancePtr->WriteInfo.Size =
			Xil_In32(InstancePtr->WriteInfo.BitstreamAddr +
						PartHeaderOffset) * WORD_LEN;
		} else {
			InstancePtr->WriteInfo.Size -= BitstreamPos;
		}

		InstancePtr->WriteInfo.BitstreamAddr += BitstreamPos;
	}
END:

	return Status;

}
/*****************************************************************************/
/**
 * This function prepare the FPGA to receive configuration data.
 *
 * @param InstancePtr Pointer to the XFpga structure.
 *
 * @return      Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *		- XFPGA_ERROR_PL_POWER_UP
 *		- XFPGA_ERROR_PL_ISOLATION
 *		- XPFGA_ERROR_PCAP_INIT
 *
 *****************************************************************************/
static u32 XFpga_PreConfigPcap(XFpga *InstancePtr)
{
	volatile u32 Status = XFPGA_FAILURE;
	u32 RegVal;

	/* Enable the PCAP clk */
	RegVal = Xil_In32(PCAP_CLK_CTRL);
	Xil_Out32(PCAP_CLK_CTRL, RegVal | PCAP_CLK_EN_MASK);

	if ((InstancePtr->WriteInfo.Flags & XFPGA_PARTIAL_EN) == 0U) {
		/* Power-Up PL */
		Status = XFpga_PowerUpPl();
		if (Status != XFPGA_SUCCESS) {
			Xfpga_Printf(XFPGA_DEBUG,
				"XFPGA_ERROR_PL_POWER_UP\r\n");
			Status = XFPGA_PCAP_UPDATE_ERR(
						(u32)XFPGA_ERROR_PL_POWER_UP, (u32)0U);
			goto END;
		}

		/* PS PL Isolation Restore */
		Status = XFPGA_FAILURE;
		Status = XFpga_IsolationRestore();
		if (Status != XFPGA_SUCCESS) {
			Xfpga_Printf(XFPGA_DEBUG,
				"XFPGA_ERROR_PL_ISOLATION\r\n");
			Status = XFPGA_PCAP_UPDATE_ERR(
						(u32)XFPGA_ERROR_PL_ISOLATION, (u32)0U);
			goto END;
		}
	}

	Status = XFPGA_FAILURE;
	Status = XFpga_PcapInit(InstancePtr->WriteInfo.Flags);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_PCAP_UPDATE_ERR((u32)XPFGA_ERROR_PCAP_INIT, (u32)0U);
	}

END:
	return Status;
}
/*****************************************************************************/
/**
 * This function write count bytes of configuration data into the PL.
 *
 * @param InstancePtr Pointer to the XFpga structure.
 *
 * @return	Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *		- XFPGA_ERROR_BITSTREAM_LOAD_FAIL
 *
 *****************************************************************************/
static u32 XFpga_WriteToPlPcap(XFpga *InstancePtr)
{
	volatile u32 Status = XFPGA_FAILURE;
	u32 BitstreamSize;

	if ((InstancePtr->WriteInfo.Flags & XFPGA_SECURE_FLAGS) != 0U)
#ifdef XFPGA_SECURE_MODE
	{
		Status = XFpga_SecureLoadToPl(InstancePtr);
		if (Status != XFPGA_SUCCESS) {
			/* Clear the PL house */
			Xil_Out32(CSU_PCAP_PROG, 0x0U);
			usleep(PL_RESET_PERIOD_IN_US);
			Xil_Out32(CSU_PCAP_PROG,
				CSU_PCAP_PROG_PCFG_PROG_B_MASK);
		}

	}
#else
	{
		Status = XFPGA_PCAP_UPDATE_ERR(XFPGA_ERROR_SECURE_MODE_EN, 0U);
		goto END;
	}
#endif
	else {
		BitstreamSize = (u32)InstancePtr->WriteInfo.Size;
		Status = XFpga_WriteToPcap(BitstreamSize/WORD_LEN,
				InstancePtr->WriteInfo.BitstreamAddr);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					(u32)XFPGA_ERROR_BITSTREAM_LOAD_FAIL, (u32)0U);
		}
	}
	if (Status != XFPGA_SUCCESS) {
		Xfpga_Printf(XFPGA_DEBUG,
		"FPGA fail to write Bitstream into PL Error Code: 0x%08x\r\n",
		Status);
		goto END;
	}

	Status = XFPGA_FAILURE;
	Status = XFpga_PLWaitForDone();
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_PCAP_UPDATE_ERR(Status, (u32)0U);
		Xfpga_Printf(XFPGA_DEBUG,
			"FPGA fail to get the PCAP Done status Error Code:0x%08x\r\n",
			Status);
	}
END:
	return Status;
}
/*****************************************************************************/
/**
 * This Function sets FPGA into operating state after writing Configuration
 * data.
 *
 * @param InstancePtr Pointer to the XFpga structure.
 *
 * @return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *		- XFPGA_ERROR_PL_POWER_UP
 * @note:
 *        PS_PL Isolation (Non PCAP) Enable/Disable should be done before
 *        and after writing the configuring data into the PL.The relevant
 *        register to enable/disable the PS_PL Isolation is part of the
 *        XPBR domain. So those registers can be accessed only through
 *        PMU_ROM code. PMU_ROM interface is having a separate call for
 *        PS_PL Isolation Enable and we are handlings this logic as part
 *        XFpga_PreConfigPcap() API. But to Disable the isolation PMU_ROM
 *        interface doesn't have any separate call. The PS_PL isolation
 *        disable logic is part of the Power_up sequence. So to disable
 *        the PS_PL Isolation we should call PL Power_up again in the
 *        Post_config sequence.
 *
 *****************************************************************************/
static u32 XFpga_PostConfigPcap(XFpga *InstancePtr)
{
	u32 Status = XFPGA_FAILURE;
	u8 EndianType = 0U;
	u32 RegVal;

	if ((InstancePtr->WriteInfo.Flags & XFPGA_PARTIAL_EN) == 0U) {
		/* PS-PL reset Low */
		XFpga_PsPlGpioResetsLow();
		usleep(XFPGA_PS_PL_RESET_TIME_US);
		/* Power-Up PL */
		Status = XFpga_PowerUpPl();
		if (Status != XFPGA_SUCCESS) {
			Xfpga_Printf(XFPGA_DEBUG,
				"XFPGA_ERROR_PL_POWER_UP\r\n");
			Status = XFPGA_PCAP_UPDATE_ERR(
					(u32)XFPGA_ERROR_PL_POWER_UP, (u32)0U);
		}
		/* PS-PL reset high*/
		if (Status == XFPGA_SUCCESS) {
			XFpga_PsPlGpioResetsHigh();
		}
	} else {
		Status = XFPGA_SUCCESS;
	}

	/* Disable the PCAP clk */
	RegVal = Xil_In32(PCAP_CLK_CTRL);
	Xil_Out32(PCAP_CLK_CTRL, RegVal & ~(PCAP_CLK_EN_MASK));
	if ((Status == XFPGA_SUCCESS) &&
	    ((InstancePtr->WriteInfo.Flags & XFPGA_SECURE_FLAGS) != 0U)) {
		XFpga_SetFirmwareState(XFPGA_FIRMWARE_STATE_SECURE);
	} else if (Status == XFPGA_SUCCESS) {
		XFpga_SetFirmwareState(XFPGA_FIRMWARE_STATE_NONSECURE);
	} else {
		XFpga_SetFirmwareState(XFPGA_FIRMWARE_STATE_UNKNOWN);
	}

	RegVal = XCsuDma_ReadReg(CsuDmaPtr->Config.BaseAddress,
				 ((u32)(XCSUDMA_CTRL_OFFSET) +
				 ((u32)XCSUDMA_SRC_CHANNEL *
				 (u32)(XCSUDMA_OFFSET_DIFF))));

	RegVal |= ((u32)EndianType << (u32)(XCSUDMA_CTRL_ENDIAN_SHIFT)) &
			  (u32)(XCSUDMA_CTRL_ENDIAN_MASK);

	XCsuDma_WriteReg(CsuDmaPtr->Config.BaseAddress,
			 ((u32)(XCSUDMA_CTRL_OFFSET) +
			 ((u32)XCSUDMA_SRC_CHANNEL *
			 (u32)(XCSUDMA_OFFSET_DIFF))), RegVal);

	return Status;
}

/*****************************************************************************/
/**
 * Performs the necessary initialization of PCAP interface
 *
 * @param Flags Provides information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 *
 *@return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *
 *****************************************************************************/
static u32 XFpga_PcapInit(u32 Flags)
{
	u32 Status = XFPGA_FAILURE;
	u32 RegVal;

	/* Take PCAP out of Reset */
	RegVal = Xil_In32(CSU_PCAP_RESET);
	RegVal &= (~CSU_PCAP_RESET_RESET_MASK);
	Xil_Out32(CSU_PCAP_RESET, RegVal);

	/* Select PCAP mode and change PCAP to write mode */
	RegVal = CSU_PCAP_CTRL_PCAP_PR_MASK;
	Xil_Out32(CSU_PCAP_CTRL, RegVal);
	Xil_Out32(CSU_PCAP_RDWR, 0x0U);

	/* Reset PL */
	if ((Flags & XFPGA_PARTIAL_EN) == 0U) {
		Xil_Out32(CSU_PCAP_PROG, 0x0U);
		usleep(PL_RESET_PERIOD_IN_US);
		Xil_Out32(CSU_PCAP_PROG, CSU_PCAP_PROG_PCFG_PROG_B_MASK);
	}

	/*
	 *  Wait for PL_init completion
	 */
	Status = Xil_WaitForEvent(CSU_PCAP_STATUS,
				CSU_PCAP_STATUS_PL_INIT_MASK,
				CSU_PCAP_STATUS_PL_INIT_MASK,
				PL_DONE_POLL_COUNT);
	return Status;
}

/*****************************************************************************/
/**
 * Waits for PCAP transfer to complete
 *
 * @param	None
 *
 * @return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *
 *****************************************************************************/
static u32 XFpga_PcapWaitForDone(void)
{
	u32 Status = XFPGA_FAILURE;

	Status = Xil_WaitForEvent(CSU_PCAP_STATUS,
				PCAP_STATUS_PCAP_WR_IDLE_MASK,
				PCAP_STATUS_PCAP_WR_IDLE_MASK,
				PL_DONE_POLL_COUNT);
	return Status;
}

/*****************************************************************************/
/**
 * Writes data to PCAP interface
 *
 * @param Size Number of bytes that the DMA should write to the
 *        PCAP interface
 * @param BitstreamAddr Linear Bitstream memory base address
 *
 * @return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *****************************************************************************/
static u32 XFpga_WriteToPcap(u32 Size, UINTPTR BitstreamAddr)
{
	volatile u32 Status = XFPGA_FAILURE;

	/*
	 * Setup the  SSS, setup the PCAP to receive from DMA source
	 */
	Xil_Out32(CSU_CSU_SSS_CFG, XFPGA_CSU_SSS_SRC_SRC_DMA);
	Xil_Out32(CSU_PCAP_RDWR, 0x0U);

	/* Setup the source DMA channel */
	XCsuDma_Transfer(CsuDmaPtr, XCSUDMA_SRC_CHANNEL, BitstreamAddr, Size, 0U);

	/* wait for the SRC_DMA to complete and the pcap to be IDLE */
	Status = XCsuDma_WaitForDoneTimeout(CsuDmaPtr, XCSUDMA_SRC_CHANNEL);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(CsuDmaPtr, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	Status = XFPGA_FAILURE;
	Status = XFpga_PcapWaitForDone();
END:
	return Status;
}

#if defined(XFPGA_READ_CONFIG_DATA)
/*****************************************************************************/
/**
 * This function waits for PCAP to come to idle state.
 *
 * @param	None
 *
 * @return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *****************************************************************************/
static u32 XFpga_PcapWaitForidle(void)
{
	u32 Status = XFPGA_FAILURE;

	Status = Xil_WaitForEvent(CSU_PCAP_STATUS,
				PCAP_STATUS_PCAP_RD_IDLE_MASK,
				PCAP_STATUS_PCAP_RD_IDLE_MASK,
				PL_DONE_POLL_COUNT);
	return Status;
}
#endif

/*****************************************************************************/
/**
 * This function is used to Validate the user provided crypto flags
 * with Image crypto flags.
 *
 * @param ImageInfo Pointer to XSecure_ImageInfo structure.
 * @param Flags It provides the information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 *
 * @return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *
 *****************************************************************************/
static u32 XFpga_ValidateCryptoFlags(const XSecure_ImageInfo *ImageInfo,
								u32 Flags)
{
	u32 Status = XFPGA_FAILURE;
	u8 IsImageAuthenticated = 0U;
	u8 IsImageUserKeyEncrypted = 0U;
	u8 IsImageDevKeyEncrypted = 0U;
	u8 IsFlagSetToAuthentication = 0U;
	u8 IsFlagSetToUserKeyEncryption = 0U;
	u8 IsFlagSetToDevKeyEncryption = 0U;

	if ((ImageInfo->PartitionHdr->PartitionAttributes &
				XSECURE_PH_ATTR_AUTH_ENABLE) != 0U) {
		IsImageAuthenticated = 1U;
	}

	if ((ImageInfo->PartitionHdr->PartitionAttributes &
					XSECURE_PH_ATTR_ENC_ENABLE) != 0U) {
		if ((ImageInfo->KeySrc == XFPGA_KEY_SRC_EFUSE_RED) ||
			(ImageInfo->KeySrc == XFPGA_KEY_SRC_BBRAM_RED) ||
			(ImageInfo->KeySrc == XFPGA_KEY_SRC_EFUSE_BLK) ||
			(ImageInfo->KeySrc == XFPGA_KEY_SRC_BH_BLACK) ||
			(ImageInfo->KeySrc == XFPGA_KEY_SRC_EFUSE_GRY) ||
			(ImageInfo->KeySrc == XFPGA_KEY_SRC_BH_GRY)) {
			IsImageDevKeyEncrypted = 1U;
		} else if (ImageInfo->KeySrc == XFPGA_KEY_SRC_KUP) {
			IsImageUserKeyEncrypted = 1U;
		} else {
			goto END;
		}
     }

	if (((Flags & XFPGA_AUTHENTICATION_DDR_EN) != 0U) ||
			((Flags & XFPGA_AUTHENTICATION_OCM_EN) != 0U)) {
		IsFlagSetToAuthentication = 1U;
	}

	if ((Flags & XFPGA_ENCRYPTION_USERKEY_EN) != 0U) {
		IsFlagSetToUserKeyEncryption = 1U;
	}

	if ((Flags & XFPGA_ENCRYPTION_DEVKEY_EN) != 0U) {
		IsFlagSetToDevKeyEncryption = 1U;
	}

	if ((IsImageAuthenticated == IsFlagSetToAuthentication) &&
		(IsImageDevKeyEncrypted == IsFlagSetToDevKeyEncryption) &&
		(IsImageUserKeyEncrypted == IsFlagSetToUserKeyEncryption)) {
		Status = XFPGA_SUCCESS;
	}

END:
	return Status;
}
#ifdef XFPGA_SECURE_MODE
/*****************************************************************************/
/**
 * Loads the secure Bitstream into the PL.
 *
 * @param InstancePtr Pointer to the XFpga structure.
 *
 * @return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *
 *****************************************************************************/
static u32 XFpga_SecureLoadToPl(XFpga *InstancePtr)
{
	u32 Status = XFPGA_FAILURE;

	switch (InstancePtr->WriteInfo.Flags &
			XFPGA_SECURE_FLAGS) {

	case XFPGA_AUTHENTICATION_DDR_EN:
	case XFPGA_AUTH_ENC_USERKEY_DDR:
	case XFPGA_AUTH_ENC_DEVKEY_DDR:
	case XFPGA_AUTHENTICATION_OCM_EN:
	case XFPGA_AUTH_ENC_USERKEY_OCM:
	case XFPGA_AUTH_ENC_DEVKEY_OCM:
		Status = XFpga_SecureBitstreamLoad(InstancePtr);
		break;

	case XFPGA_ENCRYPTION_USERKEY_EN:

#ifdef XSECURE_TRUSTED_ENVIRONMENT
	case XFPGA_ENCRYPTION_DEVKEY_EN:
#endif
		Status = XFpga_WriteEncryptToPcap(InstancePtr);
		break;

	default:

		Xfpga_Printf(XFPGA_DEBUG, "Invalid Option\r\n");
        break;

	}

return Status;
}

/*****************************************************************************/
/**
 * This function authenticates the Bitstream by using on-chip/external memory.
 * Sends the data to PCAP in blocks via AES engine if encryption
 * exists or directly to PCAP by CSUDMA if an encryption is not enabled.
 *
 * @param InstancePtr Pointer to the XFpga structure.
 *
 * @return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *
 *****************************************************************************/
static u32 XFpga_SecureBitstreamLoad(XFpga *InstancePtr)
{
	u32 Status = XFPGA_FAILURE;
	XFpgaPs_PlPartition *PlAesInfoPtr = &InstancePtr->PLInfo.PlAesInfo;
	const XSecure_ImageInfo *ImageInfo = &InstancePtr->PLInfo.SecureImageInfo;
	u32 PartationLen;
	u32 PartationOffset;
	u32 PartationAcOffset;
	u32 AesKupKey[XSECURE_KEY_LEN];

	/* Authenticate the PL Partation's */
	if ( InstancePtr->PLInfo.SecureOcmState == 0U) {
		PartationOffset = ImageInfo->PartitionHdr->DataWordOffset
						* XSECURE_WORD_LEN;
		PartationAcOffset =
			ImageInfo->PartitionHdr->AuthCertificateOffset
							* XSECURE_WORD_LEN;
		PartationLen = PartationAcOffset - PartationOffset;
		InstancePtr->PLInfo.TotalBitPartCount =
					(u32)(PartationLen/PL_PARTATION_SIZE);
		InstancePtr->PLInfo.RemaningBytes = PartationLen -
			((u32)InstancePtr->PLInfo.TotalBitPartCount *
			 PL_PARTATION_SIZE);
		InstancePtr->PLInfo.BitAddr = PartationOffset +
				InstancePtr->WriteInfo.BitstreamAddr;
		InstancePtr->PLInfo.AcPtr = PartationAcOffset +
				InstancePtr->WriteInfo.BitstreamAddr;

		if (((InstancePtr->WriteInfo.Flags &
			XFPGA_ENCRYPTION_USERKEY_EN) != 0U)||
			((InstancePtr->WriteInfo.Flags &
			XFPGA_ENCRYPTION_DEVKEY_EN) != 0U)) {
			PlAesInfoPtr->PlEncrypt.NextBlkLen = 0U;
			PlAesInfoPtr->Hdr = 0U;
			(void)memset(PlAesInfoPtr->SecureHdr, 0,
			XSECURE_SECURE_HDR_SIZE + XSECURE_SECURE_GCM_TAG_SIZE);
			PlAesInfoPtr->PlEncrypt.SecureAes =
						&InstancePtr->PLInfo.Secure_Aes;
			Status = XFpga_AesInit(
					PlAesInfoPtr->PlEncrypt.SecureAes,
					AesKupKey, ImageInfo->Iv,
					(char *)(InstancePtr->WriteInfo.KeyAddr),
					InstancePtr->WriteInfo.Flags);
			if (Status != XFPGA_SUCCESS) {
				Status = XFPGA_PCAP_UPDATE_ERR(
						XFPGA_ERROR_AES_INIT, Status);
				goto END;
			}
		}

		InstancePtr->PLInfo.SecureOcmState = 1U;
	}

	while ((u32)InstancePtr->PLInfo.TotalBitPartCount != 0U) {

		Status = XFPGA_FAILURE;
		Status = XFpga_AuthPlChunksDdrOcm(InstancePtr, PL_PARTATION_SIZE);
		if (Status != XFPGA_SUCCESS) {
			goto END;
		}
	}

	if ((InstancePtr->PLInfo.RemaningBytes != 0U) &&
		((u32)InstancePtr->PLInfo.TotalBitPartCount == 0U)){

		Status = XFPGA_FAILURE;
		Status = XFpga_AuthPlChunksDdrOcm(InstancePtr,
					InstancePtr->PLInfo.RemaningBytes);
		if (Status != XFPGA_SUCCESS) {
			goto END;
		} else {
			InstancePtr->PLInfo.RemaningBytes = 0U;
		}
	}

END:
	/* Clear local user key */
	(void)memset(AesKupKey, 0, XSECURE_KEY_LEN * XSECURE_WORD_LEN);
	/* Zeroize the Secure data*/
	(void)memset(&InstancePtr->PLInfo.SecureImageInfo, 0,
			sizeof(InstancePtr->PLInfo.SecureImageInfo));
	(void)memset(&InstancePtr->PLInfo.PlAesInfo, 0,
				sizeof(InstancePtr->PLInfo.PlAesInfo));

	return Status;
}

/*****************************************************************************/
/* This function authenticates the Bitstream by using on-chip/external memory.
 * Sends the data to PCAP in blocks via AES engine if encryption
 * exists or directly to PCAP by CSUDMA if an encryption is not enabled.
 *
 * @param InstancePtr Pointer to the XFpga structure.
 * @param Size Number of bytes that the DMA should write to the
 *        PCAP interface.
 *
 * @return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *
 *****************************************************************************/
static u32 XFpga_AuthPlChunksDdrOcm(XFpga *InstancePtr, u32 Size)
{
	volatile u32 Status = XFPGA_FAILURE;
	XFpgaPs_PlPartition *PlAesInfoPtr = &InstancePtr->PLInfo.PlAesInfo;
	XSecure_ImageInfo *ImageInfo = &InstancePtr->PLInfo.SecureImageInfo;

	/* Copy authentication certificate to internal memory */
	Status = XSecure_MemCopy((u8 *)AcBuf, (u8 *)InstancePtr->PLInfo.AcPtr,
		XSECURE_AUTH_CERT_MIN_SIZE/XSECURE_WORD_LEN);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/*Verify Spk */
	Status = XFPGA_FAILURE;
	Status = XSecure_VerifySpk((u8 *)AcBuf, ImageInfo->EfuseRsaenable);
	if (Status != (u32)XST_SUCCESS) {
		Status = XFPGA_PCAP_UPDATE_ERR(
				XFPGA_ERROR_OCM_AUTH_VERIFY_SPK,
				Status);
		goto END;
	}

	/* Authenticate Partition */
	Status = XFPGA_FAILURE;
	if ((InstancePtr->WriteInfo.Flags & XFPGA_AUTHENTICATION_DDR_EN) ==
	    XFPGA_AUTHENTICATION_DDR_EN) {
		Status = XSecure_PartitionAuthentication(CsuDmaPtr,
					(u8 *)InstancePtr->PLInfo.BitAddr, Size,
					(u8 *)(UINTPTR)AcBuf);
		if (Status != (u32)XST_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ERROR_DDR_AUTH_PARTITION,
					Status);
				goto END;
		}

		Status = XFPGA_FAILURE;
		if (((InstancePtr->WriteInfo.Flags &
			XFPGA_ENCRYPTION_USERKEY_EN)!= 0U)||
			((InstancePtr->WriteInfo.Flags &
			XFPGA_ENCRYPTION_DEVKEY_EN) != 0U)) {
			Status = XFpga_DecrptPlChunks(PlAesInfoPtr,
						InstancePtr->PLInfo.BitAddr,
						Size);
		} else {
			Status = XFpga_WriteToPcap(Size/WORD_LEN,
					InstancePtr->PLInfo.BitAddr);
		}

		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
				(u32)XFPGA_ERROR_DDR_AUTH_WRITE_PL, (u32)0U);
			goto END;
		}
	} else {
		Status = XFpga_AuthPlChunks(
				(UINTPTR)InstancePtr->PLInfo.BitAddr,
				Size, (UINTPTR)AcBuf);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ERROR_OCM_AUTH_PARTITION, Status);
			goto END;
		}

		Status = XFPGA_FAILURE;
		Status = XFpga_ReAuthPlChunksWriteToPl(PlAesInfoPtr,
					(UINTPTR)InstancePtr->PLInfo.BitAddr,
					Size, InstancePtr->WriteInfo.Flags);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
				(u32)XFPGA_ERROR_OCM_REAUTH_WRITE_PL, (u32)0U);
			goto END;
		}
	}

	if (InstancePtr->PLInfo.TotalBitPartCount != 0U) {
		InstancePtr->PLInfo.AcPtr += AC_LEN;
		InstancePtr->PLInfo.BitAddr += Size;
		InstancePtr->PLInfo.TotalBitPartCount--;
	}

END:

	return Status;
}


/*****************************************************************************/
/**
 * This function performs authentication the Blocks and store the
 * This SHA3 hashes on secure memory.
 *
 * @param BitstreamAddr Linear memory address
 * @param Size Number of bytes that the DMA should write to the
 *        PCAP interface.
 * @param AcAddr authentication certificate base address.
 *
 * @return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *
 * @note None.
 *
 ******************************************************************************/
static u32 XFpga_AuthPlChunks(UINTPTR BitstreamAddr, u32 Size, UINTPTR AcAddr)
{
	volatile u32 Status = XFPGA_FAILURE;
	XSecure_Sha3 Secure_Sha3 = {0U};
	u64 OcmAddr = OCM_PL_ADDR;
	u32 ChunkSize;
	u32 OcmChunkAddr = (u32)OCM_PL_ADDR + PL_CHUNK_SIZE_BYTES;
	u32 RemainingBytes;
	XSecure_RsaKey Key;
	u8 *AcPtr = (u8 *)(UINTPTR)AcAddr;
	u8 *Signature = (AcPtr + XSECURE_AUTH_CERT_PARTSIG_OFFSET);
	u8 Sha3Hash[HASH_LEN] = {0U};
	UINTPTR Temp_BitstreamAddr = BitstreamAddr;

	Status = (u32)XSecure_Sha3Initialize(&Secure_Sha3, CsuDmaPtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	(void)XSecure_Sha3Start(&Secure_Sha3);

	RemainingBytes = Size;
	while (RemainingBytes > 0U) {
		if (RemainingBytes > PL_CHUNK_SIZE_BYTES) {
			ChunkSize = PL_CHUNK_SIZE_BYTES;
		} else {
			ChunkSize = RemainingBytes;
		}

		Status = XFPGA_FAILURE;
		Status = XSecure_MemCopy((u8 *)(UINTPTR)OcmAddr,
					 (u8 *)(UINTPTR)Temp_BitstreamAddr,
					 ChunkSize/WORD_LEN);
		if (Status != XFPGA_SUCCESS) {
			goto END;
		}

		/* Generating SHA3 hash */
		Status = XFPGA_FAILURE;
		Status = XSecure_Sha3Update(&Secure_Sha3,
				(u8 *)(UINTPTR)OcmAddr, ChunkSize);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		XSecure_Sha3_ReadHash(&Secure_Sha3, Sha3Hash);

		/* Copy SHA3 hash into the OCM */
		(void)memcpy((u8 *)(UINTPTR)OcmChunkAddr, (u8 *)Sha3Hash, HASH_LEN);
		OcmChunkAddr = OcmChunkAddr + HASH_LEN;
		Temp_BitstreamAddr = Temp_BitstreamAddr + ChunkSize;
		RemainingBytes = RemainingBytes - ChunkSize;
	}

	/* Copy AC into the OCM */
	Status = XFPGA_FAILURE;
	Status = XSecure_MemCopy((u8 *)(UINTPTR)OcmAddr, (u8 *)(UINTPTR)AcAddr,
							 AC_LEN/WORD_LEN);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	Status = XFPGA_FAILURE;
	Status = XSecure_Sha3Update(&Secure_Sha3, (u8 *)(UINTPTR)OcmAddr,
				AC_LEN - XSECURE_PARTITION_SIG_SIZE);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = XFPGA_FAILURE;
	Status = XSecure_Sha3Finish(&Secure_Sha3, Sha3Hash);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Calculate Hash on the given signature  and compare with Sha3Hash */
	AcPtr += ((u32)XSECURE_RSA_AC_ALIGN + XSECURE_PPK_SIZE);
	Key.Modulus = AcPtr;

	AcPtr += XSECURE_SPK_MOD_SIZE;
	Key.Exponentiation = AcPtr;

	AcPtr += XSECURE_SPK_MOD_EXT_SIZE;
	Key.Exponent = AcPtr;

	Status = XFPGA_FAILURE;
	Status = XSecure_DataAuth(Signature, &Key, Sha3Hash);
END:
	/* Set SHA under reset */
	XSecure_SetReset(Secure_Sha3.BaseAddress,
					XSECURE_CSU_SHA3_RESET_OFFSET);
	return Status;
}
/*****************************************************************************/
/**
 * This function Re-authenticates the Bitstream by using on-chip memory.
 * Sends the data to PCAP in blocks via AES engine if encryption
 * exists or directly to PCAP by CSUDMA if an encryption is not enabled.
 *
 * @param PlAesInfo is a pointer to XFpgaPs_PlPartition
 * @param BitstreamAddr Linear memory secure image base address
 * @param Flags It provides the information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 * @param Size Number of bytes that the DMA should write to the
 *        PCAP interface.
 *
 * @return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *
 *****************************************************************************/
static u32 XFpga_ReAuthPlChunksWriteToPl(XFpgaPs_PlPartition *PlAesInfo,
					 UINTPTR BitstreamAddr,
					 u32 Size, u32 Flags)
{
	volatile u32 Status = XFPGA_FAILURE;
	XSecure_Sha3 Secure_Sha3;
	UINTPTR OcmAddr = OCM_PL_ADDR;
	u32 ChunkSize;
	u32 OcmChunkAddr = (u32)OCM_PL_ADDR + PL_CHUNK_SIZE_BYTES;
	u32 RemainingBytes;
	u8 Sha3Hash[HASH_LEN] = {0U};
	UINTPTR Temp_BitstreamAddr = BitstreamAddr;

	Status = (u32)XSecure_Sha3Initialize(&Secure_Sha3, CsuDmaPtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	(void)XSecure_Sha3Start(&Secure_Sha3);

	RemainingBytes = Size;
	while (RemainingBytes > 0U) {
		if (RemainingBytes > PL_CHUNK_SIZE_BYTES) {
			ChunkSize = PL_CHUNK_SIZE_BYTES;
		} else {
			ChunkSize = RemainingBytes;
		}

		Status = XFPGA_FAILURE;
		Status = XSecure_MemCopy((u8 *)OcmAddr,
					 (u8 *)(UINTPTR)Temp_BitstreamAddr,
					 ChunkSize/WORD_LEN);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_FAILURE;
			goto END;
		}
		/* Generating SHA3 hash */
		Status = XFPGA_FAILURE;
		Status = XSecure_Sha3Update(&Secure_Sha3,
				(u8 *)OcmAddr, ChunkSize);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}

		XSecure_Sha3_ReadHash(&Secure_Sha3, Sha3Hash);

		/* Compare SHA3 hash with OCM Stored hash*/
		if (memcmp((u8 *)((UINTPTR)OcmChunkAddr),
			(u8 *)Sha3Hash, HASH_LEN)!= 0) {
			Status = XFPGA_FAILURE;
		} else {
			OcmChunkAddr = OcmChunkAddr + HASH_LEN;
		}

		if (Status == XFPGA_FAILURE) {
			goto END;
		}

		Status = XFPGA_FAILURE;
		if (((Flags & XFPGA_ENCRYPTION_USERKEY_EN) != 0U)
				|| ((Flags & XFPGA_ENCRYPTION_DEVKEY_EN) != 0U)) {
			Status = XFpga_DecrptPlChunks(PlAesInfo, OcmAddr,
						      ChunkSize);
		} else {
			Status = XFpga_WriteToPcap(ChunkSize/WORD_LEN, OcmAddr);
		}

		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_FAILURE;
			goto END;
		}

		Temp_BitstreamAddr = Temp_BitstreamAddr + ChunkSize;
		RemainingBytes = RemainingBytes - ChunkSize;
	}

	Status = XFPGA_FAILURE;
	Status = XSecure_Sha3Finish(&Secure_Sha3, Sha3Hash);

END:
	/* Set SHA under reset */
	XSecure_SetReset(Secure_Sha3.BaseAddress,
					XSECURE_CSU_SHA3_RESET_OFFSET);
	return Status;
}

/*****************************************************************************/
/**
 * This is the function to write Encrypted data into PCAP interface
 *
 *
 * @param InstancePtr Pointer to the XFpga structure.
 *
 * @return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *
 *****************************************************************************/
static u32 XFpga_WriteEncryptToPcap(XFpga *InstancePtr)
{
	volatile u32 Status = XFPGA_FAILURE;
	XSecure_ImageInfo *ImageHdrInfo = &InstancePtr->PLInfo.SecureImageInfo;
	XSecure_Aes Secure_Aes = {0};
	u8 *EncSrc;
	u32 AesKupKey[XSECURE_KEY_LEN];

	Status = XFpga_AesInit(&Secure_Aes, AesKupKey, ImageHdrInfo->Iv,
				(char *)InstancePtr->WriteInfo.KeyAddr,
				InstancePtr->WriteInfo.Flags);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_PCAP_UPDATE_ERR(
				XFPGA_ERROR_AES_INIT, Status);
		goto END;
	}

	EncSrc = (u8 *)(UINTPTR)(InstancePtr->WriteInfo.BitstreamAddr +
			((ImageHdrInfo->PartitionHdr->DataWordOffset) *
						XSECURE_WORD_LEN));
	Status = XFPGA_FAILURE;
	Status = (u32)XSecure_AesDecrypt(&Secure_Aes,
			(u8 *) XFPGA_DESTINATION_PCAP_ADDR, EncSrc,
			ImageHdrInfo->PartitionHdr->UnEncryptedDataWordLength *
							XSECURE_WORD_LEN);

	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_PCAP_UPDATE_ERR(
				XFPGA_ERROR_AES_DECRYPT_PL, Status);
		goto END;
	}
	Status = XFPGA_FAILURE;
	Status = XFpga_PcapWaitForDone();
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_PCAP_UPDATE_ERR(Status, (u32)0U);
	}
END:
	/* Clear local user key */
	(void)memset(AesKupKey, 0, XSECURE_KEY_LEN * XSECURE_WORD_LEN);

	return Status;
}

/******************************************************************************/
/**
 * This API decrypts the chunks of data
 *
 * @param PartitionParams is a pointer to XFpgaPs_PlPartition
 * @param ChunkAdrs holds the address of chunk address
 * @param ChunkSize holds the size of chunk
 *
 * @return
 *	Error code on failure
 *	XFPGA_SUCCESS on success
 *
 * @note	 None.
 *
 ******************************************************************************/
static u32 XFpga_DecrptPlChunks(XFpgaPs_PlPartition *PartitionParams,
				UINTPTR ChunkAdrs, u32 ChunkSize)
{
	volatile u32 Status = XFPGA_FAILURE;
	UINTPTR SrcAddr = ChunkAdrs;
	u32 Size = ChunkSize;
	UINTPTR NextBlkAddr = 0U;

	XSecure_SssInitialize(&PartitionParams->SssInstance);

	/* If this is the first block to be decrypted it is the secure header */
	if (PartitionParams->PlEncrypt.NextBlkLen == 0x00U) {
		Status = XSecure_AesDecryptInit(PartitionParams->PlEncrypt.SecureAes,
                                (u8 *)XSECURE_DESTINATION_PCAP_ADDR, XSECURE_SECURE_HDR_SIZE,
                                (u8 *)(SrcAddr + XSECURE_SECURE_HDR_SIZE));
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}

		/*
		 * Configure AES engine to push decrypted Key and IV in the
		 * block to the CSU KEY and IV registers.
		 */
		XSecure_WriteReg(
			PartitionParams->PlEncrypt.SecureAes->BaseAddress,
				XSECURE_CSU_AES_KUP_WR_OFFSET,
				XSECURE_CSU_AES_IV_WR | XSECURE_CSU_AES_KUP_WR);
		/* Decrypting the Secure header */
		Status = XFPGA_FAILURE;
		Status = (u32)XSecure_AesDecryptUpdate(
			PartitionParams->PlEncrypt.SecureAes,
			(u8 *)(SrcAddr), XSECURE_SECURE_HDR_SIZE);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_FAILURE;
			goto END;
		}
		PartitionParams->PlEncrypt.SecureAes->KeySel =
				XSECURE_CSU_AES_KEY_SRC_KUP;
		Status = XFPGA_FAILURE;
		Status = XSecure_AesKeySelNLoad(PartitionParams->PlEncrypt.SecureAes);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		/* Point IV to the CSU IV register. */
		PartitionParams->PlEncrypt.SecureAes->Iv =
		(u32 *)(PartitionParams->PlEncrypt.SecureAes->BaseAddress +
					(UINTPTR)XSECURE_CSU_AES_IV_0_OFFSET);
		/*
		 * Remaining size and source address
		 * of the data to be processed
		 */
		Size = ChunkSize -
			XSECURE_SECURE_HDR_SIZE - XSECURE_SECURE_GCM_TAG_SIZE;
		SrcAddr = ChunkAdrs +
			XSECURE_SECURE_HDR_SIZE+XSECURE_SECURE_GCM_TAG_SIZE;

		/*
		 * Decrypt next block after Secure header and
		 * update the required fields
		 */
		Status = XFPGA_FAILURE;
		Status = XFpga_DecrptSetUpNextBlk(PartitionParams);
		if (Status != XFPGA_SUCCESS) {
			goto END;
		}

		Status = XFPGA_FAILURE;
		Status = XFpga_DecrptPl(PartitionParams, SrcAddr, Size);
		if (Status != XFPGA_SUCCESS) {
			goto END;
		}
		/*
		 * If status is true or false return the status
		 * As remaining data also processed in above API
		 */
		goto END;
	}
	/*
	 * If previous chunk has portion of left header,
	 * which needs to be processed along with this chunk
	 */
	else  if (PartitionParams->Hdr != 0x00U) {
		/* Configure AES engine */
		Status = XSecure_SssAes(&PartitionParams->SssInstance,
					XSECURE_SSS_DMA0, XSECURE_SSS_PCAP);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}

		(void)memcpy((u8 *)(PartitionParams->SecureHdr
				+ PartitionParams->Hdr), (u8 *)SrcAddr,
				XFPGA_AES_TAG_SIZE - PartitionParams->Hdr);
		Status = XFPGA_FAILURE;
		Status = XFpga_DecrypSecureHdr(
			PartitionParams->PlEncrypt.SecureAes,
			(UINTPTR)PartitionParams->SecureHdr);
		if (Status != XFPGA_SUCCESS) {
			goto END;
		}

		Size = Size - (XFPGA_AES_TAG_SIZE - PartitionParams->Hdr);
		if (Size != 0x00U) {
			NextBlkAddr = SrcAddr +
				(XFPGA_AES_TAG_SIZE - PartitionParams->Hdr);
		}
		PartitionParams->Hdr = 0U;
		(void)memset(PartitionParams->SecureHdr, 0, XFPGA_AES_TAG_SIZE);
		/*
		 * This means we are done with Secure header and Block 0
		 * And now we can change the AES key source to KUP.
		 */
		PartitionParams->PlEncrypt.SecureAes->KeySel =
				XSECURE_CSU_AES_KEY_SRC_KUP;

		Status = XFPGA_FAILURE;
		Status = XSecure_AesKeySelNLoad(PartitionParams->PlEncrypt.SecureAes);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}

		Status = XFPGA_FAILURE;
		Status = XFpga_DecrptSetUpNextBlk(PartitionParams);
		if (Status != XFPGA_SUCCESS) {
			goto END;
		}

		if ((NextBlkAddr != 0x00U) &&
		(PartitionParams->PlEncrypt.SecureAes->SizeofData != 0U)) {
			Status = XFPGA_FAILURE;
			Status = XFpga_DecrptPl(PartitionParams,
						NextBlkAddr, Size);
			if (Status != XFPGA_SUCCESS) {
				goto END;
			}
		}
	} else {
		Status = XFpga_DecrptPl(PartitionParams, SrcAddr, Size);
	}
END:
	return Status;

}

/******************************************************************************/
/**
 * This function calculates the next block size and updates the required
 * parameters.
 *
 * @param PartitionParams is a pointer to XFpgaPs_PlPartition
 *
 * @return
 *	Error code on failure
 *	XFPGA_SUCCESS on success
 *
 * @note	None
 *
 ******************************************************************************/
static u32 XFpga_DecrptSetUpNextBlk(XFpgaPs_PlPartition *PartitionParams)
{
	u32 Status = XFPGA_FAILURE;

	/* Length of next block */
	PartitionParams->PlEncrypt.NextBlkLen =
			Xil_Htonl(XSecure_ReadReg(
			PartitionParams->PlEncrypt.SecureAes->BaseAddress,
				XSECURE_CSU_AES_IV_3_OFFSET)) * WORD_LEN;
	PartitionParams->PlEncrypt.SecureAes->Iv =
		(u32 *)(PartitionParams->PlEncrypt.SecureAes->BaseAddress +
			(UINTPTR)XSECURE_CSU_AES_IV_0_OFFSET);

	/* Configure the SSS for AES. */
	Status = XSecure_SssAes(&PartitionParams->SssInstance,
				XSECURE_SSS_DMA0, XSECURE_SSS_PCAP);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Start the message. */
	XSecure_WriteReg(PartitionParams->PlEncrypt.SecureAes->BaseAddress,
				XSECURE_CSU_AES_START_MSG_OFFSET,
				XSECURE_CSU_AES_START_MSG);

	/* Transfer IV of the next block */
	XFpga_DmaPlCopy(PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
			(UINTPTR)PartitionParams->PlEncrypt.SecureAes->Iv,
				XSECURE_SECURE_GCM_TAG_SIZE/WORD_LEN, 0U);

	PartitionParams->PlEncrypt.SecureAes->SizeofData =
				PartitionParams->PlEncrypt.NextBlkLen;

	XSecure_WriteReg(PartitionParams->PlEncrypt.SecureAes->BaseAddress,
					XSECURE_CSU_AES_KUP_WR_OFFSET, 0x0U);

END:
	return Status;

}

/******************************************************************************/
/**
 * This function is used to copy data to AES/PL.
 *
 * @param InstancePtr is an instance of CSUDMA
 * @param Src holds the source Address
 * @param Size of the data
 * @param EnLast - 0 or 1
 *
 * @return None
 *
 * @note	None
 *
 ******************************************************************************/
static void XFpga_DmaPlCopy(XCsuDma *InstancePtr, UINTPTR Src, u32 Size,
			u8 EnLast)
{

	/* Data transfer */
	XCsuDma_Transfer(InstancePtr, XCSUDMA_SRC_CHANNEL, (UINTPTR)Src,
							Size, EnLast);
	/* Polling for transfer to be done */
	XCsuDma_WaitForDone(InstancePtr, XCSUDMA_SRC_CHANNEL);
	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr, XCSUDMA_SRC_CHANNEL,
					XCSUDMA_IXR_DONE_MASK);

}

/******************************************************************************/
/**
 * This function sends data to AES engine which needs to be decrypted till the
 * end of the encryption block.
 *
 * @param PartitionParams is a pointer to XFpgaPs_PlPartition
 * @param ChunkAdrs is a pointer to the data location
 * @param ChunkSize is the remaining chunk size
 *
 * @return
 *	Error code on failure
 *	XFPGA_SUCCESS on success
 *
 * @note None
 *
 *****************************************************************************/
static u32 XFpga_DecrptPl(XFpgaPs_PlPartition *PartitionParams,
			  UINTPTR ChunkAdrs, u32 ChunkSize)
{

	u32 Size = ChunkSize;
	volatile u32 Status = XFPGA_FAILURE;
	UINTPTR SrcAddr = ChunkAdrs;
	XCsuDma_Configure ConfigurValues = {0U};
	UINTPTR NextBlkAddr = 0U;

	do {

		/* Enable byte swapping */
		XCsuDma_GetConfig(
			PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
					XCSUDMA_SRC_CHANNEL, &ConfigurValues);
		ConfigurValues.EndianType = 1U;
		XCsuDma_SetConfig(
			PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL, &ConfigurValues);

		/* Configure SSS for AES engine */
		Status = XSecure_SssAes(&PartitionParams->SssInstance,
					XSECURE_SSS_DMA0, XSECURE_SSS_PCAP);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}

		if (PartitionParams->PlEncrypt.SecureAes->SizeofData != 0U) {

			/* Send whole chunk of data to AES */
			if ((Size <=
			    (PartitionParams->PlEncrypt.SecureAes->
			    SizeofData))) {
				XFpga_DmaPlCopy(
				PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
					SrcAddr, Size/WORD_LEN, 0U);
				PartitionParams->
				PlEncrypt.SecureAes->SizeofData =
				PartitionParams->PlEncrypt.SecureAes->SizeofData
									 - Size;
				Size = 0U;
			} else {
			/*
			 * If data to be processed is not zero
			 * and chunk of data is greater
			 */

			/* First transfer whole data other than secure header */
				XFpga_DmaPlCopy(
				PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
				SrcAddr, PartitionParams->PlEncrypt.SecureAes->SizeofData/WORD_LEN, 0U);
				SrcAddr = SrcAddr + PartitionParams->PlEncrypt.SecureAes->SizeofData;
				Size = Size - PartitionParams->PlEncrypt.SecureAes->SizeofData;
				PartitionParams->PlEncrypt.SecureAes->SizeofData = 0U;
				/*
				 * when data to be processed is greater than
				 * remaining data of the encrypted block
				 * and part of GCM tag and secure header of
				 * next block also exists with chunk,
				 * copy that portion for proceessing along
				 *  with next chunk of data
				 */
				if (Size < (XSECURE_SECURE_HDR_SIZE +
					XSECURE_SECURE_GCM_TAG_SIZE)) {

					if(SrcAddr == 0U) {
						goto END;
					}

					(void)memcpy(PartitionParams->SecureHdr,
						(u8 *)(UINTPTR)SrcAddr, Size);
					PartitionParams->Hdr = (u8)Size;
					Size = 0U;
				}
			}
		}

		/* Wait PCAP done */
		Status = XFPGA_FAILURE;
		Status = XFpga_PcapWaitForDone();
		if (Status != XFPGA_SUCCESS) {
			goto END;
		}

		/* Configure AES engine */
		Status = XFPGA_FAILURE;
		Status = XSecure_SssAes(&PartitionParams->SssInstance,
					XSECURE_SSS_DMA0, XSECURE_SSS_PCAP);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}

		XCsuDma_GetConfig(
			PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL, &ConfigurValues);
		ConfigurValues.EndianType = 0U;
		XCsuDma_SetConfig(
			PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL, &ConfigurValues);

		/* Decrypting secure header and GCM tag address */
		if ((PartitionParams->PlEncrypt.SecureAes->SizeofData == 0U) &&
						(Size != 0U)) {
			Status = XFPGA_FAILURE;
			Status = XFpga_DecrypSecureHdr(
				PartitionParams->PlEncrypt.SecureAes, SrcAddr);
			if (Status != XFPGA_SUCCESS) {
				goto END;
			}
			Size = Size - (XSECURE_SECURE_HDR_SIZE +
					XSECURE_SECURE_GCM_TAG_SIZE);
			if (Size != 0x00U) {
				NextBlkAddr = SrcAddr +
					XSECURE_SECURE_HDR_SIZE +
					XSECURE_SECURE_GCM_TAG_SIZE;
			}
			/*
			 * This means we are done with Secure header and Block 0
			 * And now we can change the AES key source to KUP.
			 */
			PartitionParams->PlEncrypt.SecureAes->KeySel =
					XSECURE_CSU_AES_KEY_SRC_KUP;
			Status = XFPGA_FAILURE;
			Status = XSecure_AesKeySelNLoad(
				PartitionParams->PlEncrypt.SecureAes);
			if (Status != (u32)XST_SUCCESS) {
				goto END;
			}
			Status = XFPGA_FAILURE;
			Status = XFpga_DecrptSetUpNextBlk(PartitionParams);
			if (Status != XFPGA_SUCCESS) {
				goto END;
			}
			if ((NextBlkAddr != 0x00U) &&
			(PartitionParams->PlEncrypt.SecureAes->SizeofData != 0U)) {
				SrcAddr = NextBlkAddr;
			} else {
				break;
			}
		}

	} while (Size != 0x00U);
END:
	return Status;

}

/******************************************************************************/
/**
 * This function decrypts the secure header when key rolling is enabled
 *
 * @param InstancePtr is an instance AES engine.
 * @param SrcAddr holds the address of secure header
 *
 * @return
 *	Error code on failure
 *	XFPGA_SUCCESS on success
 *
 * @note	None
 *
 ******************************************************************************/
static u32 XFpga_DecrypSecureHdr(XSecure_Aes *InstancePtr, UINTPTR SrcAddr)
{
	XCsuDma_Configure ConfigurValues = {0U};
	u32 GcmStatus;
	u32 Status = XFPGA_FAILURE;

	XCsuDma_GetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
							&ConfigurValues);
	ConfigurValues.EndianType = 1U;
	XCsuDma_SetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
							&ConfigurValues);

	/*
	 * Push secure header before that configure to
	 * push IV and key to csu engine
	 */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_AES_KUP_WR_OFFSET,
			XSECURE_CSU_AES_IV_WR | XSECURE_CSU_AES_KUP_WR);
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);
	/* PUSH Secure hdr */
	XFpga_DmaPlCopy(InstancePtr->CsuDmaPtr, SrcAddr,
			XSECURE_SECURE_HDR_SIZE/WORD_LEN, 1U);

	/* Restore Key write register to 0. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_CSU_AES_KUP_WR_OFFSET, 0x0U);
	/* Push the GCM tag. */
	XFpga_DmaPlCopy(InstancePtr->CsuDmaPtr,
		SrcAddr + XSECURE_SECURE_HDR_SIZE,
		XSECURE_SECURE_GCM_TAG_SIZE/WORD_LEN, 1U);

	/* Disable CSU DMA Src channel for byte swapping. */
	XCsuDma_GetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
							&ConfigurValues);
	ConfigurValues.EndianType = 0U;
	XCsuDma_SetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
							&ConfigurValues);

	Status = XSecure_AesWaitForDone(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Get the AES status to know if GCM check passed. */
	GcmStatus = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_STS_OFFSET) &
				XSECURE_CSU_AES_STS_GCM_TAG_OK;

	if (GcmStatus == 0U) {
		/* Zerioze the Aes key */
		Status = XFPGA_FAILURE;
		Status = XSecure_AesKeyZero(InstancePtr);
		XSecure_SetReset(InstancePtr->BaseAddress,
				 XSECURE_CSU_AES_RESET_OFFSET);
		if (Status != (u32)XST_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR((u32)Status,
						       XFPGA_FAILURE);
		} else {
			Status = XFPGA_FAILURE;
		}
	} else {
		Status = XFPGA_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function initializes the instance pointer.
 *
 * @param InstancePtr Pointer to the XSecure_Aes instance.
 * @param CsuDmaPtr Pointer to the XCsuDma instance.
 * @param IvPtr Pointer to the Initialization Vector for decryption
 * @param KeyPtr Pointer to Aes decryption key in case KUP key is used.
 *               Passes `Null` if device key is to be used.
 * @param Flags It provides the information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 *
 * @return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *
 *****************************************************************************/

static u32 XFpga_AesInit(XSecure_Aes *InstancePtr,
			 u32 *AesKupKey, u32* IvPtr,
			 char *KeyPtr, u32 Flags)
{
	u32 Status = XFPGA_FAILURE;

	if ((Flags & XFPGA_ENCRYPTION_USERKEY_EN) != 0U) {
		Status = Xil_ConvertStringToHex(KeyPtr,
						    AesKupKey, KEY_LEN);
		if (Status != XFPGA_SUCCESS) {
			goto END;
		}

		/* Xilsecure expects Key in big endian form */
		for (u8 Index = 0U; Index < XSECURE_KEY_LEN; Index++) {
			AesKupKey[Index] = Xil_Htonl(AesKupKey[Index]);
		}

		/* Initialize the Aes driver so that it's ready to use */
		Status = (u32)XSecure_AesInitialize(InstancePtr, CsuDmaPtr,
						XSECURE_CSU_AES_KEY_SRC_KUP,
						IvPtr, AesKupKey);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	} else {
		/* Initialize the Aes driver so that it's ready to use */
		Status = (u32)XSecure_AesInitialize(InstancePtr, CsuDmaPtr,
						XSECURE_CSU_AES_KEY_SRC_DEV,
						IvPtr, NULL);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
END:
	return Status;
}

#endif

/****************************************************************************/
/**
 * This function waits for PL Done bit to be set or till timeout and resets
 * PCAP after this.
 *
 * @param	None
 *
 * @return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *
 *****************************************************************************/
static u32 XFpga_PLWaitForDone(void)
{
	volatile u32 Status = XFPGA_FAILURE;
	u32 RegVal = 0U;

	Status = Xil_WaitForEvent(CSU_PCAP_STATUS,
				  CSU_PCAP_STATUS_PL_DONE_MASK,
				  CSU_PCAP_STATUS_PL_DONE_MASK,
				  PL_DONE_POLL_COUNT);
	if (Status != (u32)XST_SUCCESS) {
		Status = XFPGA_ERROR_PCAP_PL_DONE;
		goto END;
	}

	/* Reset PCAP after data transfer */
	RegVal = Xil_In32(CSU_PCAP_RESET);
	RegVal = RegVal | CSU_PCAP_RESET_RESET_MASK;
	Xil_Out32(CSU_PCAP_RESET, RegVal);

	Status = XFPGA_ERROR_PCAP_PL_DONE;
	Status = Xil_WaitForEvent(CSU_PCAP_RESET,
				  CSU_PCAP_RESET_RESET_MASK,
				  CSU_PCAP_RESET_RESET_MASK,
				  PL_DONE_POLL_COUNT);
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to power-up the PL
 *
 * @param	None
 *
 * @return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *
 *****************************************************************************/
static u32 XFpga_PowerUpPl(void)
{

	u32 Status = XFPGA_FAILURE;

#ifdef __MICROBLAZE__
	Status = XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPPLD]();
#else
	Xil_Out32(PMU_GLOBAL_PWRUP_EN, PMU_GLOBAL_PWR_PL_MASK);
	Xil_Out32(PMU_GLOBAL_PWRUP_TRIG, PMU_GLOBAL_PWR_PL_MASK);

	Status = Xil_WaitForEvent(PMU_GLOBAL_PWRUP_STATUS,
				  PMU_GLOBAL_PWR_PL_MASK, 0U,
				  PL_DONE_POLL_COUNT);
	if (Status != (u32)XST_SUCCESS) {
		Status = XFPGA_ERROR_PL_POWER_UP;
	} else {
		Status = XFPGA_SUCCESS;
	}
#endif
	return Status;
}
/*************************************************************************/
/**
 * This function is used to request isolation restore, through PMU
 *
 * @param None.
 *
 * @return
 * 	- XFPGA_SUCCESS if successful
 *  - XFPGA_ERROR_PL_ISOLATION if unsuccessful
 *
 * @note None.
 *
 **************************************************************************/
static u32 XFpga_IsolationRestore(void)
{
	u32 Status = XFPGA_FAILURE;

#ifdef __MICROBLAZE__
	Status = XpbrServHndlrTbl[XPBR_SERV_EXT_PLNONPCAPISO]();
#else
	/* Isolation request enable */
	Xil_Out32(PMU_GLOBAL_ISO_INT_EN, PMU_GLOBAL_ISO_NONPCAP_MASK);

	/* Trigger Isolation request */
	Xil_Out32(PMU_GLOBAL_ISO_TRIG, PMU_GLOBAL_ISO_NONPCAP_MASK);

	/* Poll for Isolation complete */
	Status = Xil_WaitForEvent(PMU_GLOBAL_ISO_STATUS,
				  PMU_GLOBAL_ISO_NONPCAP_MASK, 0U,
				  PL_DONE_POLL_COUNT);
	if (Status != (u32)XST_SUCCESS) {
		Status = XFPGA_ERROR_PL_ISOLATION;
	} else {
		Status = XFPGA_SUCCESS;
	}
#endif
	return Status;
}

/**************************************************************************/
/**
 * This function is used to start reset of the PL from PS EMIO pins
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *
 ****************************************************************************/
void XFpga_PsPlGpioResetsLow(void)
{
	u32 RegVal = 0U;

	/* Set EMIO Direction */
	RegVal = Xil_In32(GPIO_DIRM_5_EMIO) | GPIO_PS_PL_DIRM_MASK;
	Xil_Out32(GPIO_DIRM_5_EMIO, RegVal);

	/*De-assert the EMIO with the required Mask */
	Xil_Out32(GPIO_MASK_DATA_5_MSW, GPIO_LOW_DATA_MSW_VAL);
}

/***************************************************************************/
/**
 * This function is used to release reset of the PL from PS EMIO pins
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *
 ***************************************************************************/
void XFpga_PsPlGpioResetsHigh(void)
{
	u32 RegVal = 0U;

	/* Set EMIO Direction */
	RegVal = Xil_In32(GPIO_DIRM_5_EMIO) | GPIO_PS_PL_DIRM_MASK;
	Xil_Out32(GPIO_DIRM_5_EMIO, RegVal);

	/*Assert the EMIO with the required Mask */
	Xil_Out32(GPIO_MASK_DATA_5_MSW, GPIO_HIGH_DATA_MSW_VAL);
}


/*****************************************************************************/
/**
 * Provides the STATUS of PCAP interface
 *
 * @param	None
 *
 * @return	Status of the PCAP interface.
 *
 *****************************************************************************/
static u32 XFpga_PcapStatus(void)
{

	return Xil_In32(CSU_PCAP_STATUS);
}

#if defined(XFPGA_READ_CONFIG_REG)
/*****************************************************************************/
/**
 * Returns the value of the specified configuration register.
 *
 * @param InstancePtr Pointer to the XFpga structure.
 *
 * @return
 *               - XFPGA_SUCCESS if successful
 *               - XFPGA_FAILURE if unsuccessful
 *
 *
 ****************************************************************************/
static u32 XFpga_GetConfigRegPcap(const XFpga *InstancePtr)
{
	volatile u32 Status = XFPGA_FAILURE;
	u32 RegVal;
	UINTPTR Address = InstancePtr->ReadInfo.ReadbackAddr;
	u32 CmdIndex;
	u32 CmdBuf[XFPGA_REG_CONFIG_CMD_LEN];

	Status = XFpga_GetFirmwareState();
	if ((Status == XFPGA_FIRMWARE_STATE_SECURE) &&
	    (XFPGA_SECURE_READBACK_MODE_EN == 0U)) {
		Xfpga_Printf(XFPGA_DEBUG, "Operation not permitted\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	/* Enable the PCAP clk */
	RegVal = Xil_In32(PCAP_CLK_CTRL);
	Xil_Out32(PCAP_CLK_CTRL, RegVal | PCAP_CLK_EN_MASK);

	/*
	 * Register Readback in non secure mode
	 * Create the data to be written to read back the
	 * Configuration Registers from PL Region.
	 */
	CmdIndex = 0U;
	CmdBuf[CmdIndex] = 0xFFFFFFFFU; /* Dummy Word */
	CmdIndex++;
	CmdBuf[CmdIndex] = 0x000000BBU; /* Bus Width Sync Word */
	CmdIndex++;
	CmdBuf[CmdIndex] = 0x11220044U; /* Bus Width Detect */
	CmdIndex++;
	CmdBuf[CmdIndex] = 0xFFFFFFFFU; /* Dummy Word */
	CmdIndex++;
	CmdBuf[CmdIndex] = 0xAA995566U; /* Sync Word */
	CmdIndex++;
	CmdBuf[CmdIndex] = 0x20000000U; /* Type 1 NOOP Word 0 */
	CmdIndex++;
	CmdBuf[CmdIndex] =
			Xfpga_RegAddr((u8)(InstancePtr->ReadInfo.ConfigReg_NumFrames),
					   OPCODE_READ, 0x1U);
	CmdIndex++;
	CmdBuf[CmdIndex] = 0x20000000U; /* Type 1 NOOP Word 0 */
	CmdIndex++;
	CmdBuf[CmdIndex] = 0x20000000U; /* Type 1 NOOP Word 0 */
	CmdIndex++;
	/* Take PCAP out of Reset */
	RegVal = Xil_In32(CSU_PCAP_RESET);
	RegVal &= (~CSU_PCAP_RESET_RESET_MASK);
	Xil_Out32(CSU_PCAP_RESET, RegVal);

	/* Flush the DMA buffer */
	Xil_DCacheFlushRange(Address, 256U);

	/* Set up the Destination DMA Channel*/
	XCsuDma_Transfer(CsuDmaPtr, XCSUDMA_DST_CHANNEL, Address, 1U, 0U);

	/* Setup the source DMA channel */
	Status = XFPGA_FAILURE;
	Status = XFpga_PcapWaitForDone();
	if (Status != XFPGA_SUCCESS) {
		Xfpga_Printf(XFPGA_DEBUG, "Write to PCAP Failed\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	Status = XFPGA_FAILURE;
	Status = XFpga_WriteToPcap(CmdIndex, (UINTPTR)CmdBuf);
	if (Status != XFPGA_SUCCESS) {
		Xfpga_Printf(XFPGA_DEBUG, "Write to PCAP Failed\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	/*
	 * Setup the  SSS, setup the DMA to receive from PCAP source
	 */
	Xil_Out32(CSU_CSU_SSS_CFG, XFPGA_CSU_SSS_SRC_DST_DMA);
	Xil_Out32(CSU_PCAP_RDWR, 0x1U);

	/* wait for the DST_DMA to complete and the pcap to be IDLE */
	Status = XFPGA_FAILURE;
	Status = XCsuDma_WaitForDoneTimeout(CsuDmaPtr, XCSUDMA_DST_CHANNEL);
	if (Status != XFPGA_SUCCESS) {
		Xfpga_Printf(XFPGA_DEBUG, "Read from PCAP Failed\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(CsuDmaPtr, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	CmdIndex = 0U;
	CmdBuf[CmdIndex] = 0x30008001U; /* Type 1 Write 1 word to CMD */
	CmdIndex++;
	CmdBuf[CmdIndex] = 0x0000000DU; /* DESYNC command */
	CmdIndex++;
	CmdBuf[CmdIndex] = 0x20000000U; /* NOOP Word*/
	CmdIndex++;
	CmdBuf[CmdIndex] = 0x20000000U; /* NOOP Word */
	CmdIndex++;

	Status = XFPGA_FAILURE;
	Status = XFpga_WriteToPcap(CmdIndex, (UINTPTR)CmdBuf);
	if (Status != XFPGA_SUCCESS) {
		Xfpga_Printf(XFPGA_DEBUG, "Write to PCAP Failed\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

END:
	/* Disable the PCAP clk */
	RegVal = Xil_In32(PCAP_CLK_CTRL);
	Xil_Out32(PCAP_CLK_CTRL, RegVal & ~(PCAP_CLK_EN_MASK));
	return Status;
}
#endif

#if defined(XFPGA_READ_CONFIG_DATA)
/*****************************************************************************/
/**
 * This function performs the readback of fpga configuration data.
 *
 * @param InstancePtr Pointer to the XFpga structure.
 *
 * @return
 *               - XFPGA_SUCCESS if successful
 *               - XFPGA_FAILURE if unsuccessful
 *
 * @note None.
 ****************************************************************************/
static u32 XFpga_GetPLConfigDataPcap(const XFpga *InstancePtr)
{
	volatile u32 Status = XFPGA_FAILURE;
	UINTPTR Address = InstancePtr->ReadInfo.ReadbackAddr;
	u32 NumFrames = InstancePtr->ReadInfo.ConfigReg_NumFrames;
	u32 RegVal;
	u32 cmdindex;
	u32 CmdBuf[XFPGA_DATA_CONFIG_CMD_LEN];
	s32 i;

	Status = XFpga_GetFirmwareState();

	if (Status == XFPGA_FIRMWARE_STATE_UNKNOWN) {
		Xfpga_Printf(XFPGA_DEBUG, "Error while reading configuration "
			   "data from FPGA\n\r");
		Status = XFPGA_ERROR_PLSTATE_UNKNOWN;
		goto END;
	}

	if ((Status == XFPGA_FIRMWARE_STATE_SECURE) &&
	    (XFPGA_SECURE_READBACK_MODE_EN == 0U)) {
		Xfpga_Printf(XFPGA_DEBUG, "Operation not permitted\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	/* Enable the PCAP clk */
	RegVal = Xil_In32(PCAP_CLK_CTRL);

	/*
	 * There is no h/w flow control for pcap read
	 * to prevent the FIFO from over flowing, reduce
	 * the PCAP operating frequency.
	 */
	RegVal |= 0x3F00U;
	Xil_Out32(PCAP_CLK_CTRL, RegVal | PCAP_CLK_EN_MASK);

	/* Take PCAP out of Reset */
	Status = XFPGA_FAILURE;
	Status = XFpga_PcapInit(1U);
	if (Status != XFPGA_SUCCESS) {
		Status = XPFGA_ERROR_PCAP_INIT;
		Xfpga_Printf(XFPGA_DEBUG, "PCAP init failed\n\r");
		goto END;
	}

	cmdindex = 0U;

	/* Step 1 */
	CmdBuf[cmdindex] = 0xFFFFFFFFU; /* Dummy Word */
	cmdindex++;
	CmdBuf[cmdindex] = 0x000000BBU; /* Bus Width Sync Word */
	cmdindex++;
	CmdBuf[cmdindex] = 0x11220044U; /* Bus Width Detect */
	cmdindex++;
	CmdBuf[cmdindex] = 0xFFFFFFFFU; /* Dummy Word */
	cmdindex++;
	CmdBuf[cmdindex] = 0xAA995566U; /* Sync Word */
	cmdindex++;

	/* Step 2 */
	CmdBuf[cmdindex] = 0x02000000U; /* Type 1 NOOP Word 0 */
	cmdindex++;
	/* Step 3 */         /* Type 1 Write 1 Word to CMD */
	CmdBuf[cmdindex] = Xfpga_RegAddr(CMD, OPCODE_WRITE, 0x1U);
	cmdindex++;
	CmdBuf[cmdindex] = 0x0000000BU; /* SHUTDOWN Command */
	cmdindex++;
	CmdBuf[cmdindex] = 0x02000000U; /* Type 1 NOOP Word 0 */
	cmdindex++;

	/* Step 4 */         /* Type 1 Write 1 Word to CMD */
	CmdBuf[cmdindex] = Xfpga_RegAddr(CMD, OPCODE_WRITE, 0x1U);
	cmdindex++;
	CmdBuf[cmdindex] = 0x00000007U; /* RCRC Command */
	cmdindex++;
	CmdBuf[cmdindex] = 0x20000000U; /* Type 1 NOOP Word 0 */
	cmdindex++;

	/* Step 5 --- 5 NOOPS Words */
	for (i = 0 ; i < (s32)5 ; i++) {
		CmdBuf[cmdindex] = 0x20000000U;
		cmdindex++;
	}

	/* Step 6 */         /* Type 1 Write 1 Word to CMD */
	CmdBuf[cmdindex] = Xfpga_RegAddr(CMD, OPCODE_WRITE, 0x1U);
	cmdindex++;
	CmdBuf[cmdindex] = 0x00000004U; /* RCFG Command */
	cmdindex++;
	CmdBuf[cmdindex] = 0x20000000U; /* Type 1 NOOP Word 0 */
	cmdindex++;

	/* Step 7 */         /* Type 1 Write 1 Word to FAR */
	CmdBuf[cmdindex] = Xfpga_RegAddr(FAR1, OPCODE_WRITE, 0x1U);
	cmdindex++;
	CmdBuf[cmdindex] = 0x00000000U; /* FAR Address = 00000000 */
	cmdindex++;

	/* Step 8 */          /* Type 1 Read 0 Words from FDRO */
	CmdBuf[cmdindex] =  Xfpga_RegAddr(FDRO, OPCODE_READ, 0U);
	cmdindex++;
			      /* Type 2 Read Wordlenght Words from FDRO */
	CmdBuf[cmdindex] = Xfpga_Type2Pkt(OPCODE_READ, NumFrames);
	cmdindex++;

	/* Step 9 --- 64 NOOPS Words */
	for (i = 0 ; i < (s32)64 ; i++) {
		CmdBuf[cmdindex] = 0x20000000U;
		cmdindex++;
	}

	XCsuDma_EnableIntr(CsuDmaPtr, XCSUDMA_DST_CHANNEL,
			   XCSUDMA_IXR_DST_MASK);

	/* Flush the DMA buffer */
	Xil_DCacheFlushRange(Address, NumFrames * 4U);

	/* Set up the Destination DMA Channel*/
	XCsuDma_Transfer(CsuDmaPtr, XCSUDMA_DST_CHANNEL,
			 Address, NumFrames, 0U);

	Status = XFPGA_FAILURE;
	Status = XFpga_PcapWaitForDone();
	if (Status != XFPGA_SUCCESS) {
		Xfpga_Printf(XFPGA_DEBUG, "Write to PCAP Failed\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	Status = XFPGA_FAILURE;
	Status = XFpga_WriteToPcap(cmdindex, (UINTPTR)CmdBuf);
	if (Status != XFPGA_SUCCESS) {
		Xfpga_Printf(XFPGA_DEBUG, "Write to PCAP Failed\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	/*
	 * Setup the  SSS, setup the DMA to receive from PCAP source
	 */
	Xil_Out32(CSU_CSU_SSS_CFG, XFPGA_CSU_SSS_SRC_DST_DMA);
	Xil_Out32(CSU_PCAP_RDWR, 0x1U);


	/* wait for the DST_DMA to complete and the pcap to be IDLE */
	Status = XFPGA_FAILURE;
	Status = XCsuDma_WaitForDoneTimeout(CsuDmaPtr, XCSUDMA_DST_CHANNEL);
	if (Status != XFPGA_SUCCESS) {
		Xfpga_Printf(XFPGA_DEBUG, "Read from PCAP Failed\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(CsuDmaPtr, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	Status = XFPGA_FAILURE;
	Status = XFpga_PcapWaitForidle();
	if (Status != XFPGA_SUCCESS) {
		Xfpga_Printf(XFPGA_DEBUG, "Reading data from PL through PCAP Failed\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	cmdindex = 0U;
	/* Step 11 */
	CmdBuf[cmdindex] = 0x20000000U; /* Type 1 NOOP Word 0 */
	cmdindex++;

	/* Step 12 */
	CmdBuf[cmdindex] = 0x30008001U; /* Type 1 Write 1 Word to CMD */
	cmdindex++;
	CmdBuf[cmdindex] = 0x00000005U; /* START Command */
	cmdindex++;
	CmdBuf[cmdindex] = 0x20000000U; /* Type 1 NOOP Word 0 */
	cmdindex++;

	/* Step 13 */
	CmdBuf[cmdindex] = 0x30008001U; /* Type 1 Write 1 Word to CMD */
	cmdindex++;
	CmdBuf[cmdindex] = 0x00000007U; /* RCRC Command */
	cmdindex++;
	CmdBuf[cmdindex] = 0x20000000U; /* Type 1 NOOP Word 0 */
	cmdindex++;

	/* Step 14 */
	CmdBuf[cmdindex] = 0x30008001U; /* Type 1 Write 1 Word to CMD */
	cmdindex++;
	CmdBuf[cmdindex] = 0x0000000DU; /* DESYNC Command */
	cmdindex++;

	/* Step 15 */
	CmdBuf[cmdindex] = 0x20000000U; /* Type 1 NOOP Word 0 */
	cmdindex++;
	CmdBuf[cmdindex] = 0x20000000U; /* Type 1 NOOP Word 0 */
	cmdindex++;

	Status = XFPGA_FAILURE;
	Status = XFpga_WriteToPcap(cmdindex, (UINTPTR)CmdBuf);
	if (Status != XFPGA_SUCCESS) {
		Xfpga_Printf(XFPGA_DEBUG, "Write to PCAP 1 Failed\n\r");
		Status = XFPGA_FAILURE;
	}
END:
	/* Disable the PCAP clk */
	RegVal = Xil_In32(PCAP_CLK_CTRL);
	Xil_Out32(PCAP_CLK_CTRL, RegVal & ~(PCAP_CLK_EN_MASK));

	return Status;
}
#endif

#if defined(XFPGA_READ_CONFIG_DATA) || defined(XFPGA_READ_CONFIG_REG)
/****************************************************************************/
/**
 * Generates a Type 1 packet header that reads back the requested Configuration
 * register.
 *
 * @param        Register is the address of the register to be read back.
 * @param        OpCode is the read/write operation code.
 * @param        Size is the size of the word to be read.
 *
 * @return       Type 1 packet header to read the specified register
 *
 * @note         None.
 *
 *****************************************************************************/
static u32 Xfpga_RegAddr(u8 Register, u8 OpCode, u16 Size)
{

	/*
	 * Type 1 Packet Header Format
	 * The header section is always a 32-bit word.
	 *
	 * HeaderType | Opcode | Register Address | Reserved | Word Count
	 * [31:29]      [28:27]         [26:13]      [12:11]     [10:0]
	 * --------------------------------------------------------------
	 *   001          xx      RRRRRRRRRxxxxx        RR      xxxxxxxxxxx
	 *
	 * �R� means the bit is not used and reserved for future use.
	 * The reserved bits should be written as 0s.
	 *
	 * Generating the Type 1 packet header which involves sifting of Type 1
	 * Header Mask, Register value and the OpCode which is 01 in this case
	 * as only read operation is to be carried out and then performing OR
	 * operation with the Word Length.
	 */
	return ((u32)(((u32)XDC_TYPE_1 << (u32)XDC_TYPE_SHIFT) |
		((u32)Register << (u32)XDC_REGISTER_SHIFT) |
		((u32)OpCode << (u32)XDC_OP_SHIFT)) | (u32)Size);
}
#endif

#if defined(XFPGA_READ_CONFIG_DATA)
/****************************************************************************/
/**
 * Generates a Type 2 packet header that reads back the requested Configuration
 * register.
 *
 * @param        OpCode is the read/write operation code.
 * @param        Size is the size of the word to be read.
 *
 * @return       Type 2 packet header to read the specified register
 *
 * @note         None.
 *****************************************************************************/
static u32 Xfpga_Type2Pkt(u8 OpCode, u32 Size)
{

	/*
	 * Type 2 Packet Header Format
	 * The header section is always a 32-bit word.
	 *
	 * HeaderType | Opcode |  Word Count
	 * [31:29]      [28:27]         [26:0]
	 * --------------------------------------------------------------
	 *   010          xx      xxxxxxxxxxxxx
	 *
	 * �R� means the bit is not used and reserved for future use.
	 * The reserved bits should be written as 0s.
	 *
	 * Generating the Type 2 packet header which involves sifting of Type 2
	 * Header Mask, OpCode and then performing OR
	 * operation with the Word Length.
	 */
	return ((u32)(((u32)XDC_TYPE_2 << (u32)XDC_TYPE_SHIFT) |
		((u32)OpCode << (u32)XDC_OP_SHIFT)) | (u32)Size);
}
#endif

/*****************************************************************************/
/**
 * Sets the library firmware state
 *
 * @param	State xilfpga firmware state
 *
 * @return	None
 *****************************************************************************/
static void XFpga_SetFirmwareState(u8 State)
{
	u32 RegVal;

	/* Set Firmware State in PMU GLOBAL GEN STORAGE1 Register */
	RegVal = Xil_In32(PMU_GLOBAL_GEN_STORAGE5);
	RegVal &= ~XFPGA_STATE_MASK;
	RegVal |= ((u32)State << XFPGA_STATE_SHIFT);
	Xil_Out32(PMU_GLOBAL_GEN_STORAGE5, RegVal);
}

#if defined(XFPGA_READ_CONFIG_DATA) || defined(XFPGA_READ_CONFIG_REG)
/*****************************************************************************/
/** Returns the library firmware state
 *
 * @param	None
 *
 * @return	library firmware state
 *****************************************************************************/
static u32 XFpga_GetFirmwareState(void)
{
	return (Xil_In32(PMU_GLOBAL_GEN_STORAGE5) & XFPGA_STATE_MASK) >>
		XFPGA_STATE_SHIFT;
}
#endif

/*****************************************************************************/
/**
 * This function is responsible for  identifying the Bitstream Endianness,
 * and set the required csudma configurations before transfer the data
 * into the PL.
 *
 * @param Buf  Linear memory image base address
 * @param Size Size of the Bitstream Image(Number of bytes).
 * @Param Pos Bitstream First Dummy Word position.
 *
 * @return
 *	- XFPGA_SUCCESS if successful
 *	- XFPGA_ERROR_BITSTREAM_FORMAT if unsuccessful
 *
 *****************************************************************************/
static u32 XFpga_SelectEndianess(u8 *Buf, u32 Size, u32 *Pos)
{
	u32 Index;
	u32 RegVal;
	u32 Status = XFPGA_ERROR_BITSTREAM_FORMAT;
	u8 EndianType = 0U;
	u8 BitHdrSize = ARRAY_LENGTH(BootgenBinFormat);
	u32 IsBitNonAligned;

	/* Check for Bitstream Size */
	if(Size ==0U){
		goto END;
	}

	/* Check For Header length */
	if(BitHdrSize ==0U){
		goto END;
	}

	for (Index = 0U; Index <= BOOTGEN_DATA_OFFSET; Index++) {
	/* Find the First Dummy Byte */
		if (Buf[Index] == DUMMY_BYTE) {
			/* For Bootgen generated Bin files */
			if ((memcmp(&Buf[Index + SYNC_BYTE_POSITION],
			    BootgenBinFormat, BitHdrSize)) == 0) {
				EndianType = 0U;
				Status = XFPGA_SUCCESS;
				break;
			}
			/* For Vivado generated Bit files  */
			if ((memcmp(&Buf[Index + SYNC_BYTE_POSITION],
				   VivadoBinFormat, BitHdrSize)) == 0) {
				EndianType = 1U;
				Status = XFPGA_SUCCESS;
				break;
			}
		}
	}

	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	IsBitNonAligned = Index % 4U;
	if (IsBitNonAligned != 0U) {
		(void)memcpy(Buf, Buf + IsBitNonAligned, Size - IsBitNonAligned);
		Index -= IsBitNonAligned;
	}

	RegVal = XCsuDma_ReadReg(CsuDmaPtr->Config.BaseAddress,
				((u32)(XCSUDMA_CTRL_OFFSET) +
				((u32)XCSUDMA_SRC_CHANNEL *
				(u32)(XCSUDMA_OFFSET_DIFF))));
	RegVal |= ((u32)EndianType << (u32)(XCSUDMA_CTRL_ENDIAN_SHIFT)) &
				(u32)(XCSUDMA_CTRL_ENDIAN_MASK);
	XCsuDma_WriteReg(CsuDmaPtr->Config.BaseAddress,
			((u32)(XCSUDMA_CTRL_OFFSET) +
			((u32)XCSUDMA_SRC_CHANNEL *
			(u32)(XCSUDMA_OFFSET_DIFF))), RegVal);
	*Pos = Index;

END:

	return Status;
}

/* @endcond */
