/******************************************************************************
 *
 * Copyright (C) 2016-2018 Xilinx, Inc.  All rights reserved.
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
 * @file xilfpga_pcap.c
 *
 * This file contains the definitions of bitstream loading functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   Nava  08/06/16 Initial release
 * 1.1   Nava  16/11/16 Added PL power-up sequence.
 * 2.0	 Nava  10/1/17  Added Encrypted bitstream loading support.
 * 2.0   Nava  16/02/17 Added Authenticated bitstream loading support.
 * 2.1	 Nava  06/05/17	Correct the check logic issues in
 *			XFpga_PL_BitStream_Load()
 *			to avoid the unwanted blocking conditions.
 * 3.0   Nava  12/05/17 Added PL configuration registers readback support.
 * 4.0   Nava  08/02/18 Added Authenticated and Encypted Bitstream
 *			loading	support.
 * 4.0   Nava  02/03/18 Added the legacy bit file loading feature support
 *			from U-boot.
 *			and improve the error handling support by returning the
 *			proper ERROR value upon error conditions.
 * 4.1   Nava  7/03/18  For Secure Bitstream loading to avoid the Security
 *			violations Need to Re-validate the User Crypto flags
 *			with the Image Crypto operation by using the internal
 *			memory.To Fix this added a new API
 *			XFpga_ReValidateCryptoFlags().
 * 4.1   Nava  16/04/18 Added partial bitstream loading support.
 * 4.2   Nava  08/06/16 Refactor the xilfpga library to support
 *			different PL programming Interfaces.
 * 4.2   adk   11/07/18 Added support for readback of PL configuration data.
 * 4.2   Nava  22/07/18 Added XFpga_SelectEndianess() new API to Support
 *                      programming the vivado generated .bit and .bin files.
 * 4.2   Nava  16/08/18 Modified the PL data handling Logic to support
 *                      different PL programming interfaces.
 * 4.2	 adk   23/08/18 Added support for unaligned bitstream programming.
 * 4.2   adk   28/08/18 Fixed misra-c required standard violations.
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xilfpga.h"

/************************** Constant Definitions *****************************/
#ifdef __MICROBLAZE__
#define XPBR_SERV_EXT_PWRUPPLD		119
#define XPBR_SERV_EXT_PLNONPCAPISO	162
#define XPBR_SERV_EXT_TBL_MAX		256
#endif

#ifdef XPAR_NUM_FABRIC_RESETS
#define FPGA_NUM_FABRIC_RESETS	XPAR_NUM_FABRIC_RESETS
#else
#define FPGA_NUM_FABRIC_RESETS	1
#endif

#define MAX_REG_BITS	31
#define WORD_LEN			4	/* Bytes */
#ifdef XFPGA_SECURE_MODE
#define KEY_LEN				64	/* Bytes */
#define IV_LEN				24	/* Bytes */
#define GCM_TAG_LEN			128 /* Bytes */
#define MAX_NIBBLES			8
#define SIGNATURE_LEN			512 /* Bytes */
#define RSA_HASH_LEN			48 /* Bytes */
#define HASH_LEN			48 /* Bytes */
#define OCM_PL_ADDR			XFPGA_OCM_ADDRESS
#define AC_LEN				(0xEC0)
#define PL_PARTATION_SIZE		(0x800000U)
#define PL_CHUNK_SIZE_BYTES		(1024 * 56)
#define NUM_OF_PL_CHUNKS(Size)	(Size / PL_CHUNK_SIZE_BYTES)
#endif

/**
 * Name Configuration Type1 packet headers masks
 */
#define XDC_TYPE_SHIFT                  29
#define XDC_REGISTER_SHIFT              13
#define XDC_OP_SHIFT                    27
#define XDC_TYPE_1                      1
#define XDC_TYPE_2			2
#define OPCODE_NOOP			0
#define OPCODE_READ                     1
#define OPCODE_WRITE			2

#define XFPGA_DESTINATION_PCAP_ADDR	(0XFFFFFFFFU)
#define XFPGA_PART_IS_ENC		(0x00000080U)
#define XFPGA_PART_IS_AUTH		(0x00008000U)
#define DUMMY_BYTE			(0xFFU)
#define SYNC_BYTE_POSITION		64
#define BOOTGEN_DATA_OFFSET		0x2800U

#define XFPGA_AES_TAG_SIZE	(XSECURE_SECURE_HDR_SIZE + \
		XSECURE_SECURE_GCM_TAG_SIZE) /* AES block decryption tag size */

/* Firmware State Definitions */
#define XFPGA_FIRMWARE_STATE_UNKNOWN	0
#define XFPGA_FIRMWARE_STATE_SECURE	1
#define XFPGA_FIRMWARE_STATE_NONSECURE	2

/**************************** Type Definitions *******************************/
#ifdef __MICROBLAZE__
typedef u32 (*XpbrServHndlr_t) (void);
#endif

#ifdef XFPGA_SECURE_MODE
typedef struct {
	XSecure_Aes *SecureAes;	/* AES initialized structure */
	u32 NextBlkLen;		/* Not required for user, used
				 * for storing next block size
				 */
} XFpgaPs_PlEncryption;

typedef struct {
	XFpgaPs_PlEncryption PlEncrypt;	/* Encryption parameters */
	u8 SecureHdr[XSECURE_SECURE_HDR_SIZE + XSECURE_SECURE_GCM_TAG_SIZE];
	u8 Hdr;
} XFpgaPs_PlPartition;
#endif

/***************** Macros (Inline Functions) Definitions *********************/
#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))
#ifdef XFPGA_SECURE_MODE
#define XFPGA_SECURE_MODE_EN    1U
#else
#define XFPGA_SECURE_MODE_EN    0U
#endif

/************************** Function Prototypes ******************************/
static u32 XFpga_PcapWaitForDone(void);
static u32 XFpga_PcapWaitForidle(void);
static u32 XFpga_WriteToPcap(u32 Size, UINTPTR BitStreamAddrLow);
static u32 XFpga_PcapInit(u32 flags);
static u32 XFpga_CsuDmaInit(void);
static u32 XFpga_PLWaitForDone(void);
static u32 XFpga_PowerUpPl(void);
static u32 XFpga_IsolationRestore(void);
static u32 XFpga_PsPlGpioResetsLow(u32 TotalResets);
static u32 XFpga_PsPlGpioResetsHigh(u32 TotalResets);
static u32 Xfpga_RegAddr(u8 Register, u8 OpCode, u16 Size);
static u32 Xfpga_Type2Pkt(u8 OpCode, u32 Size);
static u32 XFpga_ValidateCryptoFlags(XSecure_ImageInfo *ImageInfo, u32 flags);
static u32 XFpga_ValidateBitstreamImage(XFpga_Info *PLInfoPtr);
static u32 XFpga_PreConfigPcap(XFpga_Info *PLInfoPtr);
static u32 XFpga_WriteToPlPcap(XFpga_Info *PLInfoPtr);
static u32 XFpga_PostConfigPcap(XFpga_Info *PLInfoPtr);
static u32 XFpga_PcapStatus(void);
static u32 XFpga_GetConfigRegPcap(XFpga_Info *PLInfoPtr);
static u32 XFpga_GetPLConfigData(XFpga_Info *PLInfoPtr);
static void XFpga_SetFirmwareState(u8 State);
static u8 XFpga_GetFirmwareState(void);
static u32 XFpga_SelectEndianess(u8 *Buf, u32 Size, u32 *Pos);
#ifdef XFPGA_SECURE_MODE
static u32 XFpga_SecureLoadToPl(UINTPTR BitstreamAddr,	UINTPTR KeyAddr,
				XSecure_ImageInfo *ImageInfo, u32 flags);
static u32 XFpga_WriteEncryptToPcap(UINTPTR BitstreamAddr, UINTPTR KeyAddr,
				XSecure_ImageInfo *ImageInfo, u32 flags);
static u32 XFpga_SecureBitstreamDdrLoad(UINTPTR BitstreamAddr, UINTPTR KeyAddr,
				XSecure_ImageInfo *ImageInfo, u32 flags);
static u32 XFpga_AesInit(UINTPTR KeyAddr, u32 *AesIv, u32 flags);
static u32 XFpga_SecureBitstreamOcmLoad(UINTPTR BitstreamAddr, UINTPTR KeyAddr,
				XSecure_ImageInfo *ImageInfo, u32 flags);
static u32 XFpga_ConvertCharToNibble(char InChar, u8 *Num);
static u32 XFpga_ConvertStringToHex(const char *Str, u32 *buf, u8 Len);
static u32 XFpga_CopyToOcm(UINTPTR Src, UINTPTR Dst, u32 Size);
static u32 XFpga_AuthPlChunks(UINTPTR BitstreamAddr, u32 Size, UINTPTR AcAddr);
static u32 XFpga_ReAuthPlChunksWriteToPl(UINTPTR BitstreamAddr,
					u32 Size, u32 flags);
static u32 XFpga_DecrptPlChunks(XFpgaPs_PlPartition *PartitionParams,
				u64 ChunkAdrs, u32 ChunkSize);
static u32 XFpga_DecrptSetUpNextBlk(XFpgaPs_PlPartition *PartitionParams);
static void XFpga_DmaPlCopy(XCsuDma *InstancePtr, UINTPTR Src,
					u32 Size, u8 EnLast);
static u32 XFpga_DecrptPl(XFpgaPs_PlPartition *PartitionParams,
				u64 ChunkAdrs, u32 ChunkSize);
static u32 XFpga_DecrypSecureHdr(XSecure_Aes *InstancePtr, u64 SrcAddr);
#endif
#ifdef __MICROBLAZE__
extern const XpbrServHndlr_t XpbrServHndlrTbl[XPBR_SERV_EXT_TBL_MAX];
#endif
/************************** Variable Definitions *****************************/
XCsuDma CsuDma;

/* Xilinx ZynqMp Vivado generated Bitstream header format */
static const u8 VivadoBinFormat[] = {
        0x00, 0x00, 0x00, 0xBB, /* Bus Width Sync Word */
        0x11, 0x22, 0x00, 0x44, /* Bus Width Detect Pattern */
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF,
        0xAA, 0x99, 0x55, 0x66, /* Sync Word */
};

/* Xilinx ZynqMp Bootgen generated Bitstream header format */
static const u8 BootgenBinFormat[] = {
        0xBB, 0x00, 0x00, 0x00, /* Bus Width Sync Word */
        0x44, 0x00, 0x22, 0x11, /* Bus Width Detect Pattern */
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF,
        0x66, 0x55, 0x99, 0xAA, /* Sync Word */
};

#ifdef XFPGA_SECURE_MODE
XSecure_Aes Secure_Aes;
u32 key[XSECURE_KEY_LEN];
XFpgaPs_PlPartition PlAesInfo;
XSecure_Sha3 Secure_Sha3;
XSecure_Rsa Secure_Rsa;
#endif
Xilfpga_Ops Fpga_Ops = {
		.XFpga_ValidateBitstream = XFpga_ValidateBitstreamImage,
		.XFpga_PreConfig = XFpga_PreConfigPcap,
		.XFpga_WriteToPl = XFpga_WriteToPlPcap,
		.XFpga_PostConfig = XFpga_PostConfigPcap,
		.XFpga_GetInterfaceStatus = XFpga_PcapStatus,
		.XFpga_GetConfigReg = XFpga_GetConfigRegPcap,
		.XFpga_GetConfigData = XFpga_GetPLConfigData,

};
/*****************************************************************************/
/* This function validate the Image image's boot header and image header,
 * also copies all the required details to the ImageInfo pointer.
 *
 * @param PLInfoPtr Pointer to the XFpga_info structure.
 *
 * @return	Returns Status
 *		- XST_SUCCESS on success
 *		- Error code on failure
 *		- XSECURE_AUTH_NOT_ENABLED - represents image is not
 *		authenticated.
 *
 * @note	Copies the header and authentication certificate to internal
 *		buffer.
 *
 *****************************************************************************/
static u32 XFpga_ValidateBitstreamImage(XFpga_Info *PLInfoPtr)
{
	u32 Status = XFPGA_SUCCESS;
	XSecure_ImageInfo *ImageHdrDataPtr = &PLInfoPtr->SecureImageInfo;
	u32 EncOnly;
	u8 IsEncrypted = 0;
	u8 NoAuth = 0;
	u8 *IvPtr = (u8 *)(UINTPTR)Iv;
	u32 BitstreamPos = 0;
	u32 PartHeaderOffset;

	if (!(XFPGA_SECURE_MODE_EN) &&
		(PLInfoPtr->Flags & XFPGA_SECURE_FLAGS)) {
		Status = XFPGA_PCAP_UPDATE_ERR(XFPGA_ERROR_SECURE_MODE_EN, 0);
		Xfpga_Printf(XFPGA_DEBUG, "Fail to load: Enable secure mode "
			"and try Error Code: 0x%08x\r\n", Status);
		goto END;
	}

	/* Initialize CSU DMA driver */
	Status = XFpga_CsuDmaInit();
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_PCAP_UPDATE_ERR(XFPGA_ERROR_CSUDMA_INIT_FAIL,
								Status);
		goto END;
	}

	if (!(PLInfoPtr->Flags & XFPGA_SECURE_FLAGS)) {
		Status = XFpga_SelectEndianess((u8 *)PLInfoPtr->BitstreamAddr,
						(u32)PLInfoPtr->AddrPtr, &BitstreamPos);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(Status, 0);
			goto END;
		} else if (BitstreamPos != BOOTGEN_DATA_OFFSET) {
			Status = XFPGA_SUCCESS;
			goto END;
		}
	}

	Status = XSecure_AuthenticationHeaders((u8 *)PLInfoPtr->BitstreamAddr,
						ImageHdrDataPtr);
	if (Status != XFPGA_SUCCESS) {
		if (Status == XSECURE_AUTH_NOT_ENABLED) {
		/* Here Buffer still contains Boot header */
			NoAuth = 1;
		}
	}

	/* Error other than XSECURE_AUTH_NOT_ENABLED error will be an error */
	if ((Status != XFPGA_SUCCESS) && (!NoAuth)) {
		Status = XFPGA_PCAP_UPDATE_ERR(XFPGA_ERROR_HDR_AUTH, Status);
		goto END;
	}

	if (NoAuth != 0x00) {
		XSecure_PartitionHeader *Ph =
				(XSecure_PartitionHeader *)(UINTPTR)
				(PLInfoPtr->BitstreamAddr +
				Xil_In32((UINTPTR)Buffer +
				XSECURE_PH_TABLE_OFFSET));
		ImageHdrDataPtr->PartitionHdr = Ph;
		if ((ImageHdrDataPtr->PartitionHdr->PartitionAttributes &
				XSECURE_PH_ATTR_AUTH_ENABLE) != 0x00U) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_HDR_NOAUTH_PART_AUTH, 0);
			goto END;
		}
	}
	if (ImageHdrDataPtr->PartitionHdr->PartitionAttributes &
				XSECURE_PH_ATTR_ENC_ENABLE) {
		IsEncrypted = 1;
	}

	EncOnly = XSecure_IsEncOnlyEnabled();
	if (EncOnly) {

		if (!IsEncrypted) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ENC_ISCOMPULSORY, 0);
			goto END;
		}
	}

	if ((IsEncrypted) && (NoAuth)) {
		ImageHdrDataPtr->KeySrc = Xil_In32((UINTPTR)Buffer +
					XSECURE_KEY_SOURCE_OFFSET);
		XSecure_MemCopy(ImageHdrDataPtr->Iv,
				(Buffer + XSECURE_IV_OFFSET), XSECURE_IV_SIZE);

		/* Add partition header IV to boot header IV */
		*(IvPtr + XSECURE_IV_LEN) = (*(IvPtr + XSECURE_IV_LEN)) +
			(ImageHdrDataPtr->PartitionHdr->Iv & XSECURE_PH_IV_MASK);
	}

	/*
	 * When authentication exists and requesting
	 * for device key other than eFUSE and KUP key
	 * when ENC_ONLY bit is blown
	 */
	if (EncOnly) {
		if ((ImageHdrDataPtr->KeySrc == XSECURE_KEY_SRC_BBRAM) ||
			(ImageHdrDataPtr->KeySrc == XSECURE_KEY_SRC_GREY_BH) ||
			(ImageHdrDataPtr->KeySrc == XSECURE_KEY_SRC_BLACK_BH)) {
			Status = XFPGA_PCAP_UPDATE_ERR(
						XFPGA_DEC_WRONG_KEY_SOURCE, 0);
			goto END;
		}
	}

	/* Validate the User Flags for the Image Crypto operation */
	Status = XFpga_ValidateCryptoFlags(ImageHdrDataPtr, PLInfoPtr->Flags);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_PCAP_UPDATE_ERR(XFPGA_ERROR_CRYPTO_FLAGS, 0);
		Xfpga_Printf(XFPGA_DEBUG,
		"Crypto flags not matched with Image crypto operation "
		"with Error Code:0x%08x\r\n", Status);
	}

END:
	if (!(PLInfoPtr->Flags & XFPGA_SECURE_FLAGS)) {
		if (BitstreamPos == BOOTGEN_DATA_OFFSET) {
			PartHeaderOffset = *((UINTPTR *)(PLInfoPtr->BitstreamAddr +
						PARTATION_HEADER_OFFSET));
			PLInfoPtr->AddrPtr = *((UINTPTR *)(PLInfoPtr->BitstreamAddr +
					      PartHeaderOffset)) * WORD_LEN;
		} else {
			PLInfoPtr->AddrPtr -= BitstreamPos;
		}
		PLInfoPtr->BitstreamAddr += BitstreamPos;
	}
	return Status;

}
/*****************************************************************************/
/** This function prepare the FPGA to receive configuration data.
 *
 * @param PLInfoPtr Pointer to the XFpga_info structure.
 *
 * @return      Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *		- XFPGA_ERROR_PL_POWER_UP
 *		- XFPGA_ERROR_PL_ISOLATION
 *		- XPFGA_ERROR_PCAP_INIT
 *
 *****************************************************************************/
static u32 XFpga_PreConfigPcap(XFpga_Info *PLInfoPtr)
{
	u32 Status = XFPGA_SUCCESS;
	u32 RegVal;

	/* Enable the PCAP clk */
	RegVal = Xil_In32(PCAP_CLK_CTRL);
	Xil_Out32(PCAP_CLK_CTRL, RegVal | PCAP_CLK_EN_MASK);

	if (!(PLInfoPtr->Flags & XFPGA_PARTIAL_EN)) {
		/* Power-Up PL */
		Status = XFpga_PowerUpPl();
		if (Status != XFPGA_SUCCESS) {
			Xfpga_Printf(XFPGA_DEBUG,
				"XFPGA_ERROR_PL_POWER_UP\r\n");
			Status = XFPGA_PCAP_UPDATE_ERR(
						XFPGA_ERROR_PL_POWER_UP, 0);
			goto END;
		}

		/* PS PL Isolation Restore */
		Status = XFpga_IsolationRestore();
		if (Status != XFPGA_SUCCESS) {
			Xfpga_Printf(XFPGA_DEBUG,
				"XFPGA_ERROR_PL_ISOLATION\r\n");
			Status = XFPGA_PCAP_UPDATE_ERR(
						XFPGA_ERROR_PL_ISOLATION, 0);
			goto END;
		}
	}

	Status = XFpga_PcapInit(PLInfoPtr->Flags);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_PCAP_UPDATE_ERR(XPFGA_ERROR_PCAP_INIT, 0);
	}
END:
	return Status;
}
/*****************************************************************************/
/* This function write count bytes of configuration data into the PL.
 *
 * @param PLInfoPtr Pointer to the XFpga_info structure.
 *
 * @return	Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *		- XFPGA_ERROR_BITSTREAM_LOAD_FAIL
 *
 *****************************************************************************/
static u32 XFpga_WriteToPlPcap(XFpga_Info *PLInfoPtr)
{
	u32 Status = XFPGA_SUCCESS;
	XSecure_ImageInfo *ImageInfo = &PLInfoPtr->SecureImageInfo;
	u32 BitstreamSize;

	if (PLInfoPtr->Flags & XFPGA_SECURE_FLAGS)
#ifdef XFPGA_SECURE_MODE
	{
		Status = XFpga_SecureLoadToPl(PLInfoPtr->BitstreamAddr,
					      PLInfoPtr->AddrPtr,
					      ImageInfo, PLInfoPtr->Flags);
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
		Status = XFPGA_PCAP_UPDATE_ERR(XFPGA_ERROR_SECURE_MODE_EN, 0);
		goto END;
	}
#endif
	else {
		BitstreamSize	= (u32)PLInfoPtr->AddrPtr;
		Status = XFpga_WriteToPcap(BitstreamSize/WORD_LEN,
				PLInfoPtr->BitstreamAddr);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ERROR_BITSTREAM_LOAD_FAIL, 0);
		}
	}
	if (Status != XFPGA_SUCCESS) {
		Xfpga_Printf(XFPGA_DEBUG,
		"FPGA fail to write Bitstream into PL Error Code: 0x%08x\r\n",
		Status);
		goto END;
	}
	Status = XFpga_PLWaitForDone();
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_PCAP_UPDATE_ERR(Status, 0);
		Xfpga_Printf(XFPGA_DEBUG,
		"FPGA fail to get the PCAP Done status Error Code:0x%08x\r\n",
		Status);
	}
END:
	return Status;
}
/*****************************************************************************/
/** This Function sets FPGA into operating state after writing Configuration
 *  data.
 *
 * @param PLInfoPtr Pointer to the XFpga_info structure.
 *
 * @return      Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *		- XFPGA_ERROR_PL_POWER_UP
 *
 *****************************************************************************/
static u32 XFpga_PostConfigPcap(XFpga_Info *PLInfoPtr)
{
	u32 Status = XFPGA_SUCCESS;
	u32 RegVal;

	if (!(PLInfoPtr->Flags & XFPGA_PARTIAL_EN)) {
		/* PS-PL reset Low */
		XFpga_PsPlGpioResetsLow(FPGA_NUM_FABRIC_RESETS);
		/* Power-Up PL */
		Status = XFpga_PowerUpPl();
		if (Status != XFPGA_SUCCESS) {
			Xfpga_Printf(XFPGA_DEBUG,
				"XFPGA_ERROR_PL_POWER_UP\r\n");
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ERROR_PL_POWER_UP, 0);
		}
		/* PS-PL reset high*/
		if (Status == XFPGA_SUCCESS) {
			XFpga_PsPlGpioResetsHigh(FPGA_NUM_FABRIC_RESETS);
		}
	}

	/* Disable the PCAP clk */
	RegVal = Xil_In32(PCAP_CLK_CTRL);
	Xil_Out32(PCAP_CLK_CTRL, RegVal & ~(PCAP_CLK_EN_MASK));
#ifdef XFPGA_SECURE_MODE
	if (((u8 *)PLInfoPtr->AddrPtr != NULL) &&
	    (PLInfoPtr->Flags & XFPGA_ENCRYPTION_USERKEY_EN))
		memset((u8 *)PLInfoPtr->AddrPtr, 0, KEY_LEN);
#endif
	if ((Status == XFPGA_SUCCESS) &&
	    (PLInfoPtr->Flags & XFPGA_SECURE_FLAGS)) {
		XFpga_SetFirmwareState(XFPGA_FIRMWARE_STATE_SECURE);
	} else if (Status == XFPGA_SUCCESS) {
		XFpga_SetFirmwareState(XFPGA_FIRMWARE_STATE_NONSECURE);
	} else {
		XFpga_SetFirmwareState(XFPGA_FIRMWARE_STATE_UNKNOWN);
	}

	return Status;
}

/*****************************************************************************/
/** Performs the necessary initialization of PCAP interface
 *
 * @param flags Provides information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 *
 * @return Error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_PcapInit(u32 flags)
{
	u32 RegVal;
	u32 PollCount;
	u32 Status = XFPGA_SUCCESS;


	/* Take PCAP out of Reset */
	RegVal = Xil_In32(CSU_PCAP_RESET);
	RegVal &= (~CSU_PCAP_RESET_RESET_MASK);
	Xil_Out32(CSU_PCAP_RESET, RegVal);

	/* Select PCAP mode and change PCAP to write mode */
	RegVal = CSU_PCAP_CTRL_PCAP_PR_MASK;
	Xil_Out32(CSU_PCAP_CTRL, RegVal);
	Xil_Out32(CSU_PCAP_RDWR, 0x0);

	/* Reset PL */
	if (!(flags & XFPGA_PARTIAL_EN)) {
		Xil_Out32(CSU_PCAP_PROG, 0x0U);
		usleep(PL_RESET_PERIOD_IN_US);
		Xil_Out32(CSU_PCAP_PROG, CSU_PCAP_PROG_PCFG_PROG_B_MASK);
	}

	/*
	 *  Wait for PL_init completion
	 */
	RegVal = 0U;
	PollCount = (PL_DONE_POLL_COUNT);
	while (PollCount) {
		RegVal = Xil_In32(CSU_PCAP_STATUS) &
		CSU_PCAP_STATUS_PL_INIT_MASK;
		if (RegVal == CSU_PCAP_STATUS_PL_INIT_MASK)
			break;
		PollCount--;
	}
	if (!PollCount) {
		Status = XFPGA_FAILURE;
	}

	return Status;
}
/*****************************************************************************/
/** Waits for PCAP transfer to complete
 *
 * @param	None
 *
 * @return	Error status based on implemented functionality
 *		(SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_PcapWaitForDone(void)
{
	u32 RegVal = 0U;
	u32 PollCount;
	u32 Status = XFPGA_SUCCESS;

	PollCount = (PL_DONE_POLL_COUNT);
	while (PollCount) {
		RegVal = Xil_In32(CSU_PCAP_STATUS);
		RegVal = RegVal & CSU_PCAP_STATUS_PCAP_WR_IDLE_MASK;
		if (RegVal == CSU_PCAP_STATUS_PCAP_WR_IDLE_MASK)
			break;
		PollCount--;
	}
	if (!PollCount) {
		Status = XFPGA_ERROR_CSU_PCAP_TRANSFER;
	}

	return Status;
}

/*****************************************************************************/
/** Writes data to PCAP interface
 *
 * @param Size Number of bytes that the DMA should write to the
 *        PCAP interface
 * @param BitstreamAddr Linear Bitstream memory base address
 *
 * @return Error status based on implemented functionality (SUCCESS by default)
 *****************************************************************************/
static u32 XFpga_WriteToPcap(u32 Size, UINTPTR BitstreamAddr)
{
	u32 Status;

	/*
	 * Setup the  SSS, setup the PCAP to receive from DMA source
	 */
	Xil_Out32(CSU_CSU_SSS_CFG, XFPGA_CSU_SSS_SRC_SRC_DMA);
	Xil_Out32(CSU_PCAP_RDWR, 0x0);

	/* Setup the source DMA channel */
	XCsuDma_Transfer(&CsuDma, XCSUDMA_SRC_CHANNEL, BitstreamAddr, Size, 0);

	/* wait for the SRC_DMA to complete and the pcap to be IDLE */
	Status = XCsuDma_WaitForDoneTimeout(&CsuDma, XCSUDMA_SRC_CHANNEL);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	Status = XFpga_PcapWaitForDone();
END:
	return Status;
}

/*****************************************************************************/
/** This function waits for PCAP to come to idle state.
 *
 * @param	None
 *
 * @return	error status based on implemented functionality
 *		(SUCCESS by default)
 *****************************************************************************/
static u32 XFpga_PcapWaitForidle(void)
{
	u32 RegVal = 0U;
	u32 PollCount;
	u32 Status = XFPGA_SUCCESS;

	PollCount = (PL_DONE_POLL_COUNT);
	while (PollCount) {
		RegVal = Xil_In32(CSU_PCAP_STATUS);
		RegVal = RegVal & CSU_PCAP_STATUS_PCAP_RD_IDLE_MASK;
		if (RegVal == CSU_PCAP_STATUS_PCAP_RD_IDLE_MASK)
			break;
		PollCount--;
	}
	if (!PollCount) {
		Status = XFPGA_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/** This function is used to Validate the user provided crypto flags
 *  with Image crypto flags.
 * @param ImageInfo	Pointer to XSecure_ImageInfo structure.
 * @param flags It provides the information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 *
 * @return error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_ValidateCryptoFlags(XSecure_ImageInfo *ImageInfo, u32 flags)
{
	u32 Status = XFPGA_SUCCESS;
	u8 IsImageAuthenticated = 0;
	u8 IsImageUserKeyEncrypted = 0;
	u8 IsImageDevKeyEncrypted = 0;
	u8 IsFlagSetToAuthentication = 0;
	u8 IsFlagSetToUserKeyEncryption = 0;
	u8 IsFlagSetToDevKeyEncryption = 0;

	if (ImageInfo->PartitionHdr->PartitionAttributes &
				XSECURE_PH_ATTR_AUTH_ENABLE)
		IsImageAuthenticated = 1;

	if (ImageInfo->PartitionHdr->PartitionAttributes &
					XSECURE_PH_ATTR_ENC_ENABLE) {
		if ((ImageInfo->KeySrc == XFPGA_KEY_SRC_EFUSE_RED) ||
			(ImageInfo->KeySrc == XFPGA_KEY_SRC_BBRAM_RED) ||
			(ImageInfo->KeySrc == XFPGA_KEY_SRC_EFUSE_BLK) ||
			(ImageInfo->KeySrc == XFPGA_KEY_SRC_BH_BLACK) ||
			(ImageInfo->KeySrc == XFPGA_KEY_SRC_EFUSE_GRY) ||
			(ImageInfo->KeySrc == XFPGA_KEY_SRC_BH_GRY)) {
			IsImageDevKeyEncrypted = 1;
		} else if (ImageInfo->KeySrc == XFPGA_KEY_SRC_KUP) {
			IsImageUserKeyEncrypted = 1;
		}
	}

	if ((flags & XFPGA_AUTHENTICATION_DDR_EN) ||
			(flags & XFPGA_AUTHENTICATION_OCM_EN)) {
		IsFlagSetToAuthentication = 1;
	}

	if (flags & XFPGA_ENCRYPTION_USERKEY_EN) {
		IsFlagSetToUserKeyEncryption = 1;
	}

	if (flags & XFPGA_ENCRYPTION_DEVKEY_EN) {
		IsFlagSetToDevKeyEncryption = 1;
	}

	if ((IsImageAuthenticated == IsFlagSetToAuthentication) &&
		(IsImageDevKeyEncrypted == IsFlagSetToDevKeyEncryption) &&
		(IsImageUserKeyEncrypted == IsFlagSetToUserKeyEncryption)) {
		Status = XFPGA_SUCCESS;
	} else {
		Status = XFPGA_FAILURE;
	}

	return Status;
}
#ifdef XFPGA_SECURE_MODE
/*****************************************************************************/
/** Loads the secure Bitstream into the PL.
 *
 * @param BitstreamAddr Linear memory secure image base address
 * @param KeyAddr Aes key address which is used for Decryption.
 * @param flags It provides the information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 * @return Error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_SecureLoadToPl(UINTPTR BitstreamAddr, UINTPTR KeyAddr,
			XSecure_ImageInfo *ImageInfo, u32 flags)
{
	u32 Status = XFPGA_FAILURE;

	switch (flags & XFPGA_SECURE_FLAGS) {

	case XFPGA_AUTHENTICATION_DDR_EN:
	case XFPGA_AUTH_ENC_USERKEY_DDR:
	case XFPGA_AUTH_ENC_DEVKEY_DDR:
		Status = XFpga_SecureBitstreamDdrLoad(BitstreamAddr,
					KeyAddr, ImageInfo, flags);
		break;

	case XFPGA_AUTHENTICATION_OCM_EN:
	case XFPGA_AUTH_ENC_USERKEY_OCM:
	case XFPGA_AUTH_ENC_DEVKEY_OCM:
		Status = XFpga_SecureBitstreamOcmLoad(BitstreamAddr,
					KeyAddr, ImageInfo, flags);
		break;

	case XFPGA_ENCRYPTION_USERKEY_EN:

#ifdef XSECURE_TRUSTED_ENVIRONMENT
	case XFPGA_ENCRYPTION_DEVKEY_EN:
#endif
		Status = XFpga_WriteEncryptToPcap(BitstreamAddr,
					KeyAddr, ImageInfo, flags);
		break;

	default:

		Xfpga_Printf(XFPGA_DEBUG, "Invalid Option\r\n");


	}

return Status;
}
/*****************************************************************************/
/* Authenticates the Bitstream by using external memory.
 * Sends the data to PCAP via AES engine if encryption exists or directly
 * to PCAP by CSUDMA if an encryption is not enabled.
 *
 * @param BitstreamAddr Linear memory secure image base address
 * @param KeyAddr Aes key address which used for decryption.
 * @param flags It provides the information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 * @return error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_SecureBitstreamDdrLoad(UINTPTR BitstreamAddr, UINTPTR KeyAddr,
					XSecure_ImageInfo *ImageInfo, u32 flags)
{
	u32 Status = XFPGA_SUCCESS;
	u32 PartationLen;
	u32 PartationOffset;
	u32 PartationAcOffset;
	u8  TotalBitPartCount;
	u32 RemaningBytes;
	UINTPTR AcPtr;
	UINTPTR BitAddr;

	/* Authenticate the PL Partation's */
	PartationOffset = ImageInfo->PartitionHdr->DataWordOffset
						* XSECURE_WORD_LEN;
	PartationAcOffset = ImageInfo->PartitionHdr->AuthCertificateOffset
						* XSECURE_WORD_LEN;
	PartationLen = PartationAcOffset - PartationOffset;
	TotalBitPartCount = PartationLen/PL_PARTATION_SIZE;
	RemaningBytes = PartationLen - (TotalBitPartCount * PL_PARTATION_SIZE);
	BitAddr = PartationOffset + BitstreamAddr;
	AcPtr = PartationAcOffset + BitstreamAddr;

	if ((flags & XFPGA_ENCRYPTION_USERKEY_EN)
			|| (flags & XFPGA_ENCRYPTION_DEVKEY_EN)) {
		XFpga_AesInit(KeyAddr, ImageInfo->Iv, flags);
	}

	for (int i = 0; i < TotalBitPartCount; i++) {
		/* Copy authentication certificate to internal memory */
		XSecure_MemCopy(AcBuf, (u8 *)AcPtr,
				XSECURE_AUTH_CERT_MIN_SIZE/XSECURE_WORD_LEN);
		/*Verify Spk */
		Status = XSecure_VerifySpk(AcBuf, ImageInfo->EfuseRsaenable);
		if (Status != XST_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ERROR_DDR_AUTH_VERIFY_SPK,
					Status);
			goto END;
		}

		/* Authenticate Partition */
		Status = XSecure_PartitionAuthentication(&CsuDma, (u8 *)BitAddr,
							 PL_PARTATION_SIZE,
							(u8 *)(UINTPTR)AcBuf);
		if (Status != XST_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ERROR_DDR_AUTH_PARTITION,
					Status);
			goto END;
		}
		if ((flags & XFPGA_ENCRYPTION_USERKEY_EN)
				|| (flags & XFPGA_ENCRYPTION_DEVKEY_EN)) {
			Status = XFpga_DecrptPlChunks(&PlAesInfo, BitAddr,
							PL_PARTATION_SIZE);
		} else {
			Status = XFpga_WriteToPcap(PL_PARTATION_SIZE/WORD_LEN,
								BitAddr);
		}

		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ERROR_DDR_AUTH_WRITE_PL, 0);
			goto END;
		}

		AcPtr += AC_LEN;
		BitAddr += PL_PARTATION_SIZE;
	}

	if (RemaningBytes) {

		/* Copy authentication certificate to internal memory */
		XSecure_MemCopy(AcBuf, (u8 *)AcPtr,
				XSECURE_AUTH_CERT_MIN_SIZE/XSECURE_WORD_LEN);
		/*Verify Spk */
		Status = XSecure_VerifySpk(AcBuf, ImageInfo->EfuseRsaenable);
		if (Status != XST_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ERROR_DDR_AUTH_VERIFY_SPK,
					Status);
			goto END;
		}

		/* Authenticate Partition */
		Status = XSecure_PartitionAuthentication(&CsuDma, (u8 *)BitAddr,
							 RemaningBytes,
							(u8 *)(UINTPTR)AcBuf);
		if (Status != XST_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ERROR_DDR_AUTH_PARTITION,
					Status);
			goto END;
		}

		if ((flags & XFPGA_ENCRYPTION_USERKEY_EN)
				|| (flags & XFPGA_ENCRYPTION_DEVKEY_EN)) {
			Status = XFpga_DecrptPlChunks(&PlAesInfo, BitAddr,
							RemaningBytes);
		} else {
			Status = XFpga_WriteToPcap(RemaningBytes/WORD_LEN,
								BitAddr);
		}

		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ERROR_DDR_AUTH_WRITE_PL, 0);
			goto END;
		}
	}
END:
	return Status;
}

/*****************************************************************************/
/* This function authenticates the Bitstream by using on-chip memory.
 * Sends the data to PCAP in blocks via AES engine if encryption
 * exists or directly to PCAP by CSUDMA if an encryption is not enabled.
 *
 * @param BitstreamAddr Linear memory secure image base address
 * @param KeyAddr Aes key address which used for decryption.
 * @param flags It provides the information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 * @return error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_SecureBitstreamOcmLoad(UINTPTR BitstreamAddr, UINTPTR KeyAddr,
				XSecure_ImageInfo *ImageInfo, u32 flags)
{
	u32 Status = XFPGA_SUCCESS;
	u32 PartationLen;
	u32 PartationOffset;
	u32 PartationAcOffset;
	u8  TotalBitPartCount;
	u32 RemaningBytes;
	UINTPTR AcPtr;
	UINTPTR BitAddr;

	/* Authenticate the PL Partation's */
	PartationOffset = ImageInfo->PartitionHdr->DataWordOffset
						* XSECURE_WORD_LEN;
	PartationAcOffset = ImageInfo->PartitionHdr->AuthCertificateOffset
							* XSECURE_WORD_LEN;
	PartationLen = PartationAcOffset - PartationOffset;
	TotalBitPartCount = PartationLen/PL_PARTATION_SIZE;
	RemaningBytes = PartationLen - (TotalBitPartCount * PL_PARTATION_SIZE);
	BitAddr = PartationOffset + BitstreamAddr;
	AcPtr = PartationAcOffset + BitstreamAddr;

	if ((flags & XFPGA_ENCRYPTION_USERKEY_EN)
			|| (flags & XFPGA_ENCRYPTION_DEVKEY_EN)) {
		XFpga_AesInit(KeyAddr, ImageInfo->Iv, flags);
	}

	for (int i = 0; i < TotalBitPartCount; i++) {

		/* Copy authentication certificate to internal memory */
		XSecure_MemCopy(AcBuf, (u8 *)AcPtr,
				XSECURE_AUTH_CERT_MIN_SIZE/XSECURE_WORD_LEN);
		/*Verify Spk */
		Status = XSecure_VerifySpk(AcBuf, ImageInfo->EfuseRsaenable);
		if (Status != XST_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ERROR_OCM_AUTH_VERIFY_SPK,
					Status);
			goto END;
		}

		Status = XFpga_AuthPlChunks((UINTPTR)BitAddr,
				PL_PARTATION_SIZE, (UINTPTR)AcBuf);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ERROR_OCM_AUTH_PARTITION, Status);
			goto END;
		}

		Status = XFpga_ReAuthPlChunksWriteToPl((UINTPTR)BitAddr,
						PL_PARTATION_SIZE, flags);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ERROR_OCM_REAUTH_WRITE_PL, 0);
			goto END;
		}

		AcPtr += AC_LEN;
		BitAddr += PL_PARTATION_SIZE;
	}

	if (RemaningBytes) {
		/* Copy authentication certificate to internal memory */
		XSecure_MemCopy(AcBuf, (u8 *)AcPtr,
				XSECURE_AUTH_CERT_MIN_SIZE/XSECURE_WORD_LEN);
		/*Verify Spk */
		Status = XSecure_VerifySpk(AcBuf, ImageInfo->EfuseRsaenable);
		if (Status != XST_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ERROR_OCM_AUTH_VERIFY_SPK,
					Status);
			goto END;
		}

		Status = XFpga_AuthPlChunks((UINTPTR)BitAddr,
				RemaningBytes, (UINTPTR)AcBuf);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ERROR_OCM_AUTH_PARTITION,
					Status);
			goto END;
		}
		Status = XFpga_ReAuthPlChunksWriteToPl((UINTPTR)BitAddr,
						RemaningBytes, flags);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_PCAP_UPDATE_ERR(
					XFPGA_ERROR_OCM_REAUTH_WRITE_PL, 0);
			goto END;
		}

	}
END:
	return Status;
}

/*****************************************************************************/
/*
 * This function performs authentication the Blocks and store the
 * This SHA3 hashes on secure memory.
 * @return      error status based on implemented functionality
 *              (SUCCESS by default)
 *
 * @note                None.
 *
 ******************************************************************************/
static u32 XFpga_AuthPlChunks(UINTPTR BitstreamAddr, u32 Size, UINTPTR AcAddr)
{
	u32 Status;
	u64 OcmAddr = OCM_PL_ADDR;
	u32 NumChunks = NUM_OF_PL_CHUNKS(Size);
	u32 ChunkSize = PL_CHUNK_SIZE_BYTES;
	u32 OcmChunkAddr = OCM_PL_ADDR + ChunkSize;
	u32 RemaningBytes;
	u32 Count;
	XSecure_RsaKey Key;
	u8 *AcPtr = (u8 *)(UINTPTR)AcAddr;
	u8 *Signature = (AcPtr + XSECURE_AUTH_CERT_PARTSIG_OFFSET);
	u8 Sha3Hash[HASH_LEN];

	RemaningBytes = (Size - (ChunkSize * NumChunks));

	XSecure_Sha3Initialize(&Secure_Sha3, &CsuDma);
	XSecure_Sha3Start(&Secure_Sha3);
	for (Count = 0; Count < NumChunks; Count++) {
		Status = XFpga_CopyToOcm((UINTPTR)BitstreamAddr,
				(UINTPTR)OcmAddr, ChunkSize/WORD_LEN);
		if (Status != XFPGA_SUCCESS) {
			goto END;
		}

		/* Generating SHA3 hash */
		XSecure_Sha3Update(&Secure_Sha3,
				(u8 *)(UINTPTR)OcmAddr, ChunkSize);
		XSecure_Sha3_ReadHash(&Secure_Sha3, Sha3Hash);

		/* Copy SHA3 hash into the OCM */
		memcpy((u32 *)(UINTPTR)OcmChunkAddr, Sha3Hash, HASH_LEN);
		OcmChunkAddr = OcmChunkAddr + HASH_LEN;
		BitstreamAddr = BitstreamAddr + ChunkSize;
	}

	if (RemaningBytes) {
		Status = XFpga_CopyToOcm((UINTPTR)BitstreamAddr,
					(UINTPTR)OcmAddr,
					RemaningBytes/WORD_LEN);
		if (Status != XFPGA_SUCCESS) {
			goto END;
		}

		/* Generating SHA3 hash */
		XSecure_Sha3Update(&Secure_Sha3,
				(u8 *)(UINTPTR)OcmAddr, RemaningBytes);
		XSecure_Sha3_ReadHash(&Secure_Sha3, Sha3Hash);
		/* Copy SHA3 hash into the OCM */
		memcpy((u32 *)(UINTPTR)OcmChunkAddr, Sha3Hash, HASH_LEN);
	}

	/* Copy AC into the OCM */
	Status = XFpga_CopyToOcm((UINTPTR)AcAddr,
				(UINTPTR)OcmAddr, AC_LEN/WORD_LEN);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}
	XSecure_Sha3Update(&Secure_Sha3, (u8 *)(UINTPTR)OcmAddr,
				AC_LEN - XSECURE_PARTITION_SIG_SIZE);
	XSecure_Sha3Finish(&Secure_Sha3, Sha3Hash);

	/* Calculate Hash on the given signature  and compare with Sha3Hash */
	AcPtr += (XSECURE_RSA_AC_ALIGN + XSECURE_PPK_SIZE);
	Key.Modulus = AcPtr;

	AcPtr += XSECURE_SPK_MOD_SIZE;
	Key.Exponentiation = AcPtr;

	AcPtr += XSECURE_SPK_MOD_EXT_SIZE;
	Key.Exponent = AcPtr;

	Status = XSecure_DataAuth(Signature, &Key, Sha3Hash);
END:
	return Status;
}
/*****************************************************************************/
/* This function Re-authenticates the Bitstream by using on-chip memory.
 * Sends the data to PCAP in blocks via AES engine if encryption
 * exists or directly to PCAP by CSUDMA if an encryption is not enabled.
 *
 * @param Size Number of bytes that the DMA should write to the
 *        PCAP interface.
 * @param BitstreamAddr Linear memory secure image base address
 * @param flags It provides the information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 * @return error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_ReAuthPlChunksWriteToPl(UINTPTR BitstreamAddr,
				u32 Size, u32 flags)
{
	u32 Status = XFPGA_SUCCESS;
	u64 OcmAddr = OCM_PL_ADDR;
	u32 NumChunks = NUM_OF_PL_CHUNKS(Size);
	u32 ChunkSize = PL_CHUNK_SIZE_BYTES;
	u32 OcmChunkAddr = OCM_PL_ADDR + ChunkSize;
	u32 RemaningBytes;
	u32 Count;
	u8 Sha3Hash[HASH_LEN];

	RemaningBytes = (Size  - (ChunkSize * NumChunks));

	XSecure_Sha3Initialize(&Secure_Sha3, &CsuDma);
	XSecure_Sha3Start(&Secure_Sha3);
	for (Count = 0; Count < NumChunks; Count++) {
		Status = XFpga_CopyToOcm((UINTPTR)BitstreamAddr,
					(UINTPTR)OcmAddr, ChunkSize/WORD_LEN);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_FAILURE;
			goto END;
		}
		/* Generating SHA3 hash */
		XSecure_Sha3Update(&Secure_Sha3,
				(u8 *)(UINTPTR)OcmAddr, ChunkSize);
		XSecure_Sha3_ReadHash(&Secure_Sha3, Sha3Hash);

		/* Compare SHA3 hash with OCM Stored hash*/
		if (memcmp((u32 *)(UINTPTR)OcmChunkAddr, Sha3Hash, HASH_LEN)) {
			Status = XFPGA_FAILURE;
		} else {
			OcmChunkAddr = OcmChunkAddr + HASH_LEN;
		}

		if (Status == XFPGA_FAILURE) {
			goto END;
		}

		if ((flags & XFPGA_ENCRYPTION_USERKEY_EN)
				|| (flags & XFPGA_ENCRYPTION_DEVKEY_EN)) {
			Status = XFpga_DecrptPlChunks(&PlAesInfo,
						OcmAddr, ChunkSize);
		} else {
			Status = XFpga_WriteToPcap(ChunkSize/WORD_LEN, OcmAddr);
		}

		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_FAILURE;
			goto END;
		}

		BitstreamAddr = BitstreamAddr + ChunkSize;
	}
	if (RemaningBytes) {
		Status = XFpga_CopyToOcm((UINTPTR)BitstreamAddr,
					(UINTPTR)OcmAddr,
					RemaningBytes/WORD_LEN);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_FAILURE;
			goto END;
		}
		/* Generating SHA3 hash */
		XSecure_Sha3Update(&Secure_Sha3,
				(u8 *)(UINTPTR)OcmAddr, RemaningBytes);
		XSecure_Sha3_ReadHash(&Secure_Sha3, Sha3Hash);

		/* Compare SHA3 hash with OCM Stored hash*/
		if (memcmp((u32 *)(UINTPTR)OcmChunkAddr, Sha3Hash, HASH_LEN)) {
			Status = XFPGA_FAILURE;
			goto END;
		}

		if ((flags & XFPGA_ENCRYPTION_USERKEY_EN)
				|| (flags & XFPGA_ENCRYPTION_DEVKEY_EN)) {
			Status = XFpga_DecrptPlChunks(&PlAesInfo,
						OcmAddr, RemaningBytes);
		} else {
			Status = XFpga_WriteToPcap(RemaningBytes/WORD_LEN,
								OcmAddr);
		}

		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_FAILURE;
			goto END;
		}
	}

	XSecure_Sha3Finish(&Secure_Sha3, Sha3Hash);
END:
	return Status;
}

/*****************************************************************************/
/* This is the function to write Encrypted data into PCAP interface
 *
 * @param Size Number of bytes that the DMA should write to the
 * PCAP interface
 *
 * @param BitstreamAddr Linear memory secure image base address.
 *
 * @return error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_WriteEncryptToPcap(UINTPTR BitstreamAddr, UINTPTR KeyAddr,
				XSecure_ImageInfo *ImageHdrInfo, u32 flags)
{
	u32 Status = XFPGA_SUCCESS;
	u8 *EncSrc;

	if (flags & XFPGA_ENCRYPTION_USERKEY_EN) {
		XFpga_ConvertStringToHex((char *)(UINTPTR)(KeyAddr),
							key, KEY_LEN);
		/* Xilsecure expects Key in big endian form */
		for (u8 i = 0; i < ARRAY_LENGTH(key); i++) {
			key[i] = Xil_Htonl(key[i]);
		}
		/* Initialize the Aes driver so that it's ready to use */
		XSecure_AesInitialize(&Secure_Aes, &CsuDma,
					XSECURE_CSU_AES_KEY_SRC_KUP,
					(u32 *)ImageHdrInfo->Iv, (u32 *)key);
	} else {

		/* Initialize the Aes driver so that it's ready to use */
		XSecure_AesInitialize(&Secure_Aes, &CsuDma,
					XSECURE_CSU_AES_KEY_SRC_DEV,
					(u32 *)ImageHdrInfo->Iv, NULL);
	}

	EncSrc = (u8 *)(UINTPTR)(BitstreamAddr +
			(ImageHdrInfo->PartitionHdr->DataWordOffset) *
						XSECURE_WORD_LEN);
	Status = XSecure_AesDecrypt(&Secure_Aes,
			(u8 *) XFPGA_DESTINATION_PCAP_ADDR, EncSrc,
			ImageHdrInfo->PartitionHdr->UnEncryptedDataWordLength *
							XSECURE_WORD_LEN);

	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_PCAP_UPDATE_ERR(
				XFPGA_ERROR_AES_DECRYPT_PL, Status);
		goto END;
	}
	Status = XFpga_PcapWaitForDone();
	if (Status != XFPGA_SUCCESS)
		Status = XFPGA_PCAP_UPDATE_ERR(Status, 0);
END:
	return Status;
}

/*****************************************************************************/
/* This function is used initialize the Aes H/W Engine.
 * @param KeyAddr Aes key address which used for decryption.
 * @param AesIv   Aes IV address which used for decryption.
 * @param flags It provides the information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 * @return error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_AesInit(UINTPTR KeyAddr, u32 *AesIv, u32 flags)
{
	u32 Status = XFPGA_SUCCESS;

	PlAesInfo.PlEncrypt.SecureAes = &Secure_Aes;
	PlAesInfo.PlEncrypt.NextBlkLen = 0;
	PlAesInfo.Hdr = 0;
	memset(PlAesInfo.SecureHdr, 0,
			XSECURE_SECURE_HDR_SIZE + XSECURE_SECURE_GCM_TAG_SIZE);

	if (flags & XFPGA_ENCRYPTION_USERKEY_EN) {
		XFpga_ConvertStringToHex((char *)(UINTPTR)(KeyAddr),
							key, KEY_LEN);
		/* Xilsecure expects Key in big endian form */
		for (u8 i = 0; i < ARRAY_LENGTH(key); i++) {
			key[i] = Xil_Htonl(key[i]);
		}
		/* Initialize the Aes driver so that it's ready to use */
		XSecure_AesInitialize(PlAesInfo.PlEncrypt.SecureAes, &CsuDma,
					XSECURE_CSU_AES_KEY_SRC_KUP,
					AesIv, (u32 *)key);
	} else {
		/* Initialize the Aes driver so that it's ready to use */
		XSecure_AesInitialize(PlAesInfo.PlEncrypt.SecureAes, &CsuDma,
					XSECURE_CSU_AES_KEY_SRC_DEV,
					AesIv, NULL);
	}

	return Status;
}

/*****************************************************************************/
/*
 * This function copies data using CSU DMA.
 *
 * @param DestPtr pointer to the destination address.
 * @param SrcPtr pointer to the source address.
 * @param Size of the data to be copied.
 *
 * @return Returns Status
 *	- XFPGA_SUCCESS on success
 *	- XFPGA_FAILURE on failure
 *
 * @note None.
 *
 *****************************************************************************/
static u32 XFpga_CopyToOcm(UINTPTR Src, UINTPTR Dst, u32 Size)
{
	u32 Status;

	/*
	 * Setup the  SSS, To copy the contains from DDR to OCM
	 */
	Xil_Out32(CSU_CSU_SSS_CFG, XFPGA_CSU_SSS_DMA_TO_DMA);


	/* Data transfer to OCM */
	XCsuDma_Transfer(&CsuDma, XCSUDMA_DST_CHANNEL, Dst, Size, 0);
	XCsuDma_Transfer(&CsuDma, XCSUDMA_SRC_CHANNEL, Src, Size, 0);

	/* Polling for transfer to be done */
	Status = XCsuDma_WaitForDoneTimeout(&CsuDma, XCSUDMA_DST_CHANNEL);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);
END:
	return Status;
}
/******************************************************************************/
/*
 * This API decrypts the chunks of data
 *
 * @param PartitionParams is a pointer to XFpgaPs_PlPartition
 * @param ChunkAdrs holds the address of chunk address
 * @param ChunkSize holds the size of chunk
 *
 * @return
 *	Error code on failure
 *	XFPGA_SUCESS on success
 *
 * @note	 None.
 *
 ******************************************************************************/
static u32 XFpga_DecrptPlChunks(XFpgaPs_PlPartition *PartitionParams,
		u64 ChunkAdrs, u32 ChunkSize)
{
	u32 Status = XFPGA_SUCCESS;
	UINTPTR SrcAddr = (u64)ChunkAdrs;
	u32 Size = ChunkSize;
	u64 NextBlkAddr = 0;
	u32 SssAes;
	u32 SssCfg;

	/* If this is the first block to be decrypted it is the secure header */
	if (PartitionParams->PlEncrypt.NextBlkLen == 0x00) {
		XSecure_AesDecryptInit(PartitionParams->PlEncrypt.SecureAes,
		(u8 *)XSECURE_DESTINATION_PCAP_ADDR, XSECURE_SECURE_HDR_SIZE,
			(u8 *)(SrcAddr + XSECURE_SECURE_HDR_SIZE));

		/*
		 * Configure AES engine to push decrypted Key and IV in the
		 * block to the CSU KEY and IV registers.
		 */
		XSecure_WriteReg(
			PartitionParams->PlEncrypt.SecureAes->BaseAddress,
				XSECURE_CSU_AES_KUP_WR_OFFSET,
				XSECURE_CSU_AES_IV_WR | XSECURE_CSU_AES_KUP_WR);
		/* Decrypting the Secure header */
		Status = XSecure_AesDecryptUpdate(
			PartitionParams->PlEncrypt.SecureAes,
			(u8 *)(SrcAddr), XSECURE_SECURE_HDR_SIZE);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_FAILURE;
			goto END;
		}
		PartitionParams->PlEncrypt.SecureAes->KeySel =
				XSECURE_CSU_AES_KEY_SRC_KUP;
		XSecure_AesKeySelNLoad(PartitionParams->PlEncrypt.SecureAes);
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
		Status = XFpga_DecrptSetUpNextBlk(PartitionParams);
		if (Status != XFPGA_SUCCESS) {
			goto END;
		}

		Status = XFpga_DecrptPl(PartitionParams,
					(UINTPTR)SrcAddr, Size);
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
	else  if (PartitionParams->Hdr != 0x00) {
		/* Configure AES engine */
		SssAes = XSecure_SssInputAes(XSECURE_CSU_SSS_SRC_SRC_DMA);
		SssCfg = SssAes | XSecure_SssInputPcap(XSECURE_CSU_SSS_SRC_AES);
		XSecure_SssSetup(SssCfg);
		memcpy((u8 *)(PartitionParams->SecureHdr
				+ PartitionParams->Hdr), (u8 *)(UINTPTR)SrcAddr,
				XFPGA_AES_TAG_SIZE - PartitionParams->Hdr);
		Status = XFpga_DecrypSecureHdr(
			PartitionParams->PlEncrypt.SecureAes,
			(u64)(UINTPTR)PartitionParams->SecureHdr);
		if (Status != XFPGA_SUCCESS) {
			goto END;
		}

		Size = Size - (XFPGA_AES_TAG_SIZE - PartitionParams->Hdr);
		if (Size != 0x00) {
			NextBlkAddr = SrcAddr +
				(XFPGA_AES_TAG_SIZE - PartitionParams->Hdr);
		}
		PartitionParams->Hdr = 0;
		memset(PartitionParams->SecureHdr, 0, XFPGA_AES_TAG_SIZE);
		/*
		 * This means we are done with Secure header and Block 0
		 * And now we can change the AES key source to KUP.
		 */
		PartitionParams->PlEncrypt.SecureAes->KeySel =
				XSECURE_CSU_AES_KEY_SRC_KUP;

		XSecure_AesKeySelNLoad(PartitionParams->PlEncrypt.SecureAes);
		Status = XFpga_DecrptSetUpNextBlk(PartitionParams);
		if (Status != XFPGA_SUCCESS) {
			goto END;
		}

		if ((NextBlkAddr != 0x00U) &&
		(PartitionParams->PlEncrypt.SecureAes->SizeofData != 0)) {
			Status = XFpga_DecrptPl(PartitionParams,
					(UINTPTR)NextBlkAddr, Size);
			if (Status != XFPGA_SUCCESS) {
				goto END;
			}
		}
	} else {
		Status = XFpga_DecrptPl(PartitionParams, SrcAddr, Size);
		goto END;
	}
END:
	return Status;

}

/******************************************************************************/
/*
 * This function calculates the next block size and updates the required
 * parameters.
 *
 * @param PartitionParams is a pointer to XFpgaPs_PlPartition
 * @param ChunkAdrs is a pointer to the data location
 * @param ChunkSize is the remaining chunk size
 *
 * @return
 *	Error code on failure
 *	XFPGA_SUCESS on success
 *
 * @note	None
 *
 ******************************************************************************/
static u32 XFpga_DecrptSetUpNextBlk(XFpgaPs_PlPartition *PartitionParams)
{
	u32 Status = XFPGA_SUCCESS;
	u32 SssAes;
	u32 SssCfg;

	/* Length of next block */
	PartitionParams->PlEncrypt.NextBlkLen =
			Xil_Htonl(XSecure_ReadReg(
			PartitionParams->PlEncrypt.SecureAes->BaseAddress,
				XSECURE_CSU_AES_IV_3_OFFSET)) * WORD_LEN;
	PartitionParams->PlEncrypt.SecureAes->Iv =
		(u32 *)(PartitionParams->PlEncrypt.SecureAes->BaseAddress +
			(UINTPTR)XSECURE_CSU_AES_IV_0_OFFSET);

	/* Configure the SSS for AES. */
	SssAes = XSecure_SssInputAes(XSECURE_CSU_SSS_SRC_SRC_DMA);
	SssCfg = SssAes | XSecure_SssInputPcap(XSECURE_CSU_SSS_SRC_AES);
	XSecure_SssSetup(SssCfg);

	/* Start the message. */
	XSecure_WriteReg(PartitionParams->PlEncrypt.SecureAes->BaseAddress,
				XSECURE_CSU_AES_START_MSG_OFFSET,
				XSECURE_CSU_AES_START_MSG);

	/* Transfer IV of the next block */
	XFpga_DmaPlCopy(PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
			(UINTPTR)PartitionParams->PlEncrypt.SecureAes->Iv,
				XSECURE_SECURE_GCM_TAG_SIZE/WORD_LEN, 0);

	PartitionParams->PlEncrypt.SecureAes->SizeofData =
				PartitionParams->PlEncrypt.NextBlkLen;

	XSecure_WriteReg(PartitionParams->PlEncrypt.SecureAes->BaseAddress,
					XSECURE_CSU_AES_KUP_WR_OFFSET, 0x0);


	return Status;

}

/******************************************************************************/
/*
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
/*
 * This function sends data to AES engine which needs to be decrypted till the
 * end of the encryption block.
 *
 * @param PartitionParams is a pointer to XFpgaPs_PlPartition
 * @param ChunkAdrs is a pointer to the data location
 * @param ChunkSize is the remaining chunk size
 *
 * @return
 *	Error code on failure
 *	XFPGA_SUCESS on success
 *
 * @note None
 *
 *****************************************************************************/
static u32 XFpga_DecrptPl(XFpgaPs_PlPartition *PartitionParams,
					u64 ChunkAdrs, u32 ChunkSize)
{

	u32 Size = ChunkSize;
	u32 Status = XFPGA_SUCCESS;
	u64 SrcAddr = (u64)ChunkAdrs;
	XCsuDma_Configure ConfigurValues = {0};
	UINTPTR NextBlkAddr = 0;
	u32 SssAes;
	u32 SssCfg;

	do {

		/* Enable byte swapping */
		XCsuDma_GetConfig(
			PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
					XCSUDMA_SRC_CHANNEL, &ConfigurValues);
		ConfigurValues.EndianType = 1U;
		XCsuDma_SetConfig(
			PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL, &ConfigurValues);

		/* Configure AES engine */
		SssAes = XSecure_SssInputAes(XSECURE_CSU_SSS_SRC_SRC_DMA);
		SssCfg = SssAes | XSecure_SssInputPcap(XSECURE_CSU_SSS_SRC_AES);
		XSecure_SssSetup(SssCfg);

		/* Send whole chunk of data to AES */
		if ((Size <=
			(PartitionParams->PlEncrypt.SecureAes->SizeofData)) &&
		   (PartitionParams->PlEncrypt.SecureAes->SizeofData != 0)) {
			XFpga_DmaPlCopy(
				PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
					(UINTPTR)SrcAddr, Size/WORD_LEN, 0);
			PartitionParams->PlEncrypt.SecureAes->SizeofData =
			PartitionParams->PlEncrypt.SecureAes->SizeofData - Size;
			Size = 0;
		}

		/*
		 * If data to be processed is not zero
		 * and chunk of data is greater
		 */
		else if (
			PartitionParams->PlEncrypt.SecureAes->SizeofData != 0) {
			/* First transfer whole data other than secure header */
			XFpga_DmaPlCopy(
				PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
				(UINTPTR)SrcAddr,
			PartitionParams->PlEncrypt.SecureAes->SizeofData/
								WORD_LEN, 0);
			SrcAddr = SrcAddr +
			PartitionParams->PlEncrypt.SecureAes->SizeofData;
			Size = Size -
			PartitionParams->PlEncrypt.SecureAes->SizeofData;
			PartitionParams->PlEncrypt.SecureAes->SizeofData = 0;
			/*
			 * when data to be processed is greater than
			 * remaining data of the encrypted block
			 * and part of GCM tag and secure header of next block
			 * also exists with chunk, copy that portion for
			 * proceessing along with next chunk of data
			 */

			if (Size <
			 (XSECURE_SECURE_HDR_SIZE +
				XSECURE_SECURE_GCM_TAG_SIZE)) {
				memcpy(PartitionParams->SecureHdr,
						(u8 *)(UINTPTR)SrcAddr, Size);
				PartitionParams->Hdr = Size;
				Size = 0;
			}
		}

		/* Wait PCAP done */
		Status = XFpga_PcapWaitForDone();
		if (Status != XFPGA_SUCCESS) {
			goto END;
		}

		/* Configure AES engine */
		SssAes = XSecure_SssInputAes(XSECURE_CSU_SSS_SRC_SRC_DMA);
		SssCfg = SssAes | XSecure_SssInputPcap(XSECURE_CSU_SSS_SRC_AES);
		XSecure_SssSetup(SssCfg);

		XCsuDma_GetConfig(
			PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL, &ConfigurValues);
		ConfigurValues.EndianType = 0U;
		XCsuDma_SetConfig(
			PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL, &ConfigurValues);

		/* Decrypting secure header and GCM tag address */
		if ((PartitionParams->PlEncrypt.SecureAes->SizeofData == 0) &&
						(Size != 0)) {
			Status = XFpga_DecrypSecureHdr(
				PartitionParams->PlEncrypt.SecureAes, SrcAddr);
			if (Status != XFPGA_SUCCESS) {
				goto END;
			}
			Size = Size - (XSECURE_SECURE_HDR_SIZE +
					XSECURE_SECURE_GCM_TAG_SIZE);
			if (Size != 0x00) {
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
			XSecure_AesKeySelNLoad(
				PartitionParams->PlEncrypt.SecureAes);
			Status = XFpga_DecrptSetUpNextBlk(PartitionParams);
			if (Status != XFPGA_SUCCESS) {
				goto END;
			}
			if ((NextBlkAddr != 0x00U) &&
			(PartitionParams->PlEncrypt.SecureAes->SizeofData != 0)) {
				SrcAddr = NextBlkAddr;
			} else {
				break;
			}
		}

	} while (Size != 0x00);
END:
	return Status;

}

/******************************************************************************/
/*
 * This function decrypts the secure header when key rolling is enabled
 *
 * @param InstancePtr is an instance AES engine.
 * @param SrcAddr holds the address of secure header
 * @param Size holds size
 *
 * @return
 *	Error code on failure
 *	XFPGA_SUCESS on success
 *
 * @note	None
 *
 ******************************************************************************/
static u32 XFpga_DecrypSecureHdr(XSecure_Aes *InstancePtr, u64 SrcAddr)
{
	XCsuDma_Configure ConfigurValues = {0};
	u32 GcmStatus;
	u32 Status = XFPGA_SUCCESS;

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
			XSECURE_SECURE_HDR_SIZE/WORD_LEN, 1);

	/* Restore Key write register to 0. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_CSU_AES_KUP_WR_OFFSET, 0x0);
	/* Push the GCM tag. */
	XFpga_DmaPlCopy(InstancePtr->CsuDmaPtr,
		SrcAddr + XSECURE_SECURE_HDR_SIZE,
		XSECURE_SECURE_GCM_TAG_SIZE/WORD_LEN, 1);

	/* Disable CSU DMA Src channel for byte swapping. */
	XCsuDma_GetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
							&ConfigurValues);
	ConfigurValues.EndianType = 0U;
	XCsuDma_SetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
							&ConfigurValues);

	XSecure_PcapWaitForDone();

	XSecure_AesWaitForDone(InstancePtr);
	/* Get the AES status to know if GCM check passed. */
	GcmStatus = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_STS_OFFSET) &
				XSECURE_CSU_AES_STS_GCM_TAG_OK;

	if (GcmStatus == 0) {
		Xfpga_Printf(XFPGA_DEBUG, "GCM TAG NOT Matched\r\n");
		Status = XFPGA_FAILURE;
	}

	return Status;
}

#endif

/****************************************************************************/
/* This function waits for PL Done bit to be set or till timeout and resets
 * PCAP after this.
 *
 * @param	None
 *
 * @return	error status based on implemented functionality
 *		(SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_PLWaitForDone(void)
{
	u32 Status = XFPGA_SUCCESS;
	u32 PollCount;
	u32 RegVal = 0U;

	PollCount = (PL_DONE_POLL_COUNT);
	while (PollCount) {
		/* Read PCAP Status register and check for PL_DONE bit */
		RegVal = Xil_In32(CSU_PCAP_STATUS);
		RegVal &= CSU_PCAP_STATUS_PL_DONE_MASK;
		if (RegVal == CSU_PCAP_STATUS_PL_DONE_MASK)
			break;
		PollCount--;
	}

	if (RegVal != CSU_PCAP_STATUS_PL_DONE_MASK) {
		Status = XFPGA_ERROR_PCAP_PL_DONE;
		goto END;
	}

	/* Reset PCAP after data transfer */
	RegVal = Xil_In32(CSU_PCAP_RESET);
	RegVal = RegVal | CSU_PCAP_RESET_RESET_MASK;
	Xil_Out32(CSU_PCAP_RESET, RegVal);

	PollCount = (PL_DONE_POLL_COUNT);
	RegVal = 0U;
	while (PollCount) {
		RegVal = Xil_In32(CSU_PCAP_RESET);
		RegVal = RegVal & CSU_PCAP_RESET_RESET_MASK;
		if (RegVal == CSU_PCAP_RESET_RESET_MASK)
			break;
		PollCount--;
	}
END:
	return Status;
}

/*****************************************************************************/
/*
 * This function is used to initialize the DMA driver
 *
 * @param	None
 *
 * @return	error status based on implemented functionality
 *		(SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_CsuDmaInit(void)
{
	u32 Status = XFPGA_SUCCESS;
	XCsuDma_Config *CsuDmaConfig;

	CsuDmaConfig = XCsuDma_LookupConfig(0);
	if (CsuDmaConfig == NULL)
		goto END;

	Status = XCsuDma_CfgInitialize(&CsuDma, CsuDmaConfig,
			CsuDmaConfig->BaseAddress);
END:
	return Status;
}
/*****************************************************************************/
/*
 * This function is used to power-up the PL
 *
 * @param	None
 *
 * @return	error status based on implemented functionality
 *		(SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_PowerUpPl(void)
{

	u32 Status;

#ifdef __MICROBLAZE__
	Status = XpbrServHndlrTbl[XPBR_SERV_EXT_PWRUPPLD]();
#else

	u32 RegVal;
	u32 PollCount;


	Xil_Out32(PMU_GLOBAL_PWRUP_EN, PMU_GLOBAL_PWR_PL_MASK);
	Xil_Out32(PMU_GLOBAL_PWRUP_TRIG, PMU_GLOBAL_PWR_PL_MASK);
	PollCount = (PL_DONE_POLL_COUNT);
	do {
		RegVal = Xil_In32(PMU_GLOBAL_PWRUP_STATUS) &
					PMU_GLOBAL_PWR_PL_MASK;
		PollCount--;
	} while ((RegVal != 0) && PollCount);

	if (PollCount == 0) {
		Status = XFPGA_ERROR_PL_POWER_UP;
	} else {
		Status = XFPGA_SUCCESS;
	}
#endif
	return Status;
}
/*************************************************************************/
/*
 * This function is used to request isolation restore, through PMU
 *
 * @param	Mask of the entries for which isolation is to be restored
 *
 * @return	XFSBL_SUCCESS (for now always returns this)
 *
 * @note		None.
 *
 **************************************************************************/
static u32 XFpga_IsolationRestore(void)
{
	u32 Status;

#ifdef __MICROBLAZE__
	Status = XpbrServHndlrTbl[XPBR_SERV_EXT_PLNONPCAPISO]();
#else

	u32 PollCount;
	u32 RegVal;


	/* Isolation request enable */
	Xil_Out32(PMU_GLOBAL_ISO_INT_EN, PMU_GLOBAL_PWR_PL_MASK);

	/* Trigger Isolation request */
	Xil_Out32(PMU_GLOBAL_ISO_TRIG, PMU_GLOBAL_PWR_PL_MASK);

	/* Poll for Isolation complete */
	PollCount = (PL_DONE_POLL_COUNT);
	do {
		RegVal = Xil_In32(PMU_GLOBAL_ISO_STATUS) &
					PMU_GLOBAL_PWR_PL_MASK;
		PollCount--;
	} while ((RegVal != 0) && PollCount);

	if (PollCount == 0) {
		Status = XFPGA_ERROR_PL_ISOLATION;
	} else {
		Status = XFPGA_SUCCESS;
	}
#endif
	return Status;
}

/**************************************************************************/
/*
 * This function is used to start reset of the PL from PS EMIO pins
 *
 * @param	TotalResets
 *
 * @return	XFSBL_SUCCESS (for now always returns this)
 *
 * @note	None.
 *
 ****************************************************************************/
static u32 XFpga_PsPlGpioResetsLow(u32 TotalResets)
{
	u32 Status = XFPGA_SUCCESS;
	u32 RegVal = 0;

	/* Set EMIO Direction */
	RegVal = Xil_In32(GPIO_DIRM_5_EMIO) |
		~(~0U << TotalResets) << (MAX_REG_BITS + 1 - TotalResets);
	Xil_Out32(GPIO_DIRM_5_EMIO, RegVal);

	/*De-assert the EMIO with the required Mask */
	RegVal = ~(~(~0U << TotalResets) << (MAX_REG_BITS + 1 - TotalResets))
		& 0xFFFF0000;
	Xil_Out32(GPIO_MASK_DATA_5_MSW, RegVal);
	usleep(1000);

	return Status;
}

/***************************************************************************/
/*
 * This function is used to release reset of the PL from PS EMIO pins
 *
 * @param	TotalResets
 *
 * @return	XFSBL_SUCCESS (for now always returns this)
 *
 * @note		None.
 *
 ***************************************************************************/
static u32 XFpga_PsPlGpioResetsHigh(u32 TotalResets)
{
	u32 Status = XFPGA_SUCCESS;
	u32 RegVal = 0;
	u32 MaskVal;

	/* Set EMIO Direction */
	RegVal = Xil_In32(GPIO_DIRM_5_EMIO) |
		~(~0U << TotalResets) << (MAX_REG_BITS + 1 - TotalResets);
	Xil_Out32(GPIO_DIRM_5_EMIO, RegVal);

	/*Assert the EMIO with the required Mask */
	MaskVal = ~(~0U << TotalResets) << (MAX_REG_BITS/2 + 1 - TotalResets) |
								0xFFFF0000;
	RegVal = MaskVal & ~(~(~0U << TotalResets) <<
				(MAX_REG_BITS + 1 - TotalResets));
	Xil_Out32(GPIO_MASK_DATA_5_MSW, RegVal);
	usleep(1000);

	return Status;
}


/*****************************************************************************/
/** Provides the STATUS of PCAP interface
 *
 * @param	None
 *
 * @return	Status of the PCAP interface.
 *
 *****************************************************************************/
u32 XFpga_PcapStatus(void)
{

	return Xil_In32(CSU_PCAP_STATUS);
}

#ifdef XFPGA_SECURE_MODE
/****************************************************************************/
/*
 * Converts the char into the equivalent nibble.
 *	Ex: 'a' -> 0xa, 'A' -> 0xa, '9'->0x9
 *
 * @param	InChar is input character. It has to be between 0-9,a-f,A-F
 * @param	Num is the output nibble.
 *
 * @return
 *	- XST_SUCCESS no errors occured.
 *	- ERROR when input parameters are not valid
 *
 * @note	None.
 *
 *****************************************************************************/
static u32 XFpga_ConvertCharToNibble(char InChar, u8 *Num)
{
	u32 Status = XFPGA_SUCCESS;

	/* Convert the char to nibble */
	if ((InChar >= '0') && (InChar <= '9'))
		*Num = InChar - '0';
	else if ((InChar >= 'a') && (InChar <= 'f'))
		*Num = InChar - 'a' + 10;
	else if ((InChar >= 'A') && (InChar <= 'F'))
		*Num = InChar - 'A' + 10;
	else
		Status = XFPGA_STRING_INVALID_ERROR;

	return Status;
}

/****************************************************************************/
/*
 * Converts the string into the equivalent Hex buffer.
 *	Ex: "abc123" -> {0xab, 0xc1, 0x23}
 *
 * @param	Str is a Input String. Will support the lower and upper
 *		case values. Value should be between 0-9, a-f and A-F
 *
 * @param	Buf is Output buffer.
 * @param	Len of the input string. Should have even values
 *
 * @return
 *		- XST_SUCCESS no errors occured.
 *		- ERROR when input parameters are not valid
 *		- an error when input buffer has invalid values
 *
 * @note	None.
 *
 *****************************************************************************/
static u32 XFpga_ConvertStringToHex(const char *Str, u32 *buf, u8 Len)
{
	u32 Status = XFPGA_SUCCESS;
	u8 ConvertedLen = 0, index = 0;
	u8 Nibble[MAX_NIBBLES];

	while (ConvertedLen < Len) {
		/* Convert char to nibble */
		for (u8 i = 0; i < ARRAY_LENGTH(Nibble); i++) {
			Status = XFpga_ConvertCharToNibble(
					Str[ConvertedLen++], &Nibble[i]);

			if (Status != XFPGA_SUCCESS) {
				/* Error converting char to nibble */
				Status = XFPGA_STRING_INVALID_ERROR;
				goto END;
			}
		}

		buf[index++] = Nibble[0] << 28 | Nibble[1] << 24 |
				Nibble[2] << 20 | Nibble[3] << 16 |
				Nibble[4] << 12 | Nibble[5] << 8 |
				Nibble[6] << 4 | Nibble[7];
	}
END:
	return Status;
}

#endif

/*****************************************************************************/
/**
 * @ingroup xfpga_apis
 * Returns the value of the specified configuration register.
 *
 * @param PLInfoPtr Pointer to the XFpga_info structure.
 *
 * @return
 *               - XFPGA_SUCCESS if successful
 *               - XFPGA_FAILURE if unsuccessful
 *
 *
 ****************************************************************************/
static u32 XFpga_GetConfigRegPcap(XFpga_Info *PLInfoPtr)
{
	u32 Status;
	u32 RegVal;
	UINTPTR Address = PLInfoPtr->ReadbackAddr;
	unsigned int CmdIndex;
	u32 *CmdBuf;

	CmdBuf = (void *)(UINTPTR)Address;

	Status = XFpga_GetFirmwareState();
	if (Status == XFPGA_FIRMWARE_STATE_SECURE) {
		xil_printf("Operation not permitted\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	/* Initialize the CSU DMA */
	Status = XFpga_CsuDmaInit();
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_SUCCESS;
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
	CmdIndex = 2;
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; /* Dummy Word */
	CmdBuf[CmdIndex++] = 0x000000BB; /* Bus Width Sync Word */
	CmdBuf[CmdIndex++] = 0x11220044; /* Bus Width Detect */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; /* Dummy Word */
	CmdBuf[CmdIndex++] = 0xAA995566; /* Sync Word */
	CmdBuf[CmdIndex++] = 0x20000000; /* Type 1 NOOP Word 0 */
	CmdBuf[CmdIndex++] = Xfpga_RegAddr(PLInfoPtr->ConfigReg,
					   OPCODE_READ, 0x1);
	CmdBuf[CmdIndex++] = 0x20000000; /* Type 1 NOOP Word 0 */
	CmdBuf[CmdIndex++] = 0x20000000; /* Type 1 NOOP Word 0 */

	/* Take PCAP out of Reset */
	RegVal = Xil_In32(CSU_PCAP_RESET);
	RegVal &= (~CSU_PCAP_RESET_RESET_MASK);
	Xil_Out32(CSU_PCAP_RESET, RegVal);

	/* Flush the DMA buffer */
	Xil_DCacheFlushRange(Address, 256);

	/* Set up the Destination DMA Channel*/
	XCsuDma_Transfer(&CsuDma, XCSUDMA_DST_CHANNEL, Address, 1, 0);

	/* Setup the source DMA channel */
	Status = XFpga_PcapWaitForDone();
	if (Status != XFPGA_SUCCESS) {
		xil_printf("Write to PCAP Failed\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	Status = XFpga_WriteToPcap(CmdIndex, Address + CFGREG_SRCDMA_OFFSET);
	if (Status != XFPGA_SUCCESS) {
		xil_printf("Write to PCAP Failed\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	/*
	 * Setup the  SSS, setup the DMA to receive from PCAP source
	 */
	Xil_Out32(CSU_CSU_SSS_CFG, XFPGA_CSU_SSS_SRC_DST_DMA);
	Xil_Out32(CSU_PCAP_RDWR, 0x1);

	/* wait for the DST_DMA to complete and the pcap to be IDLE */
	Status = XCsuDma_WaitForDoneTimeout(&CsuDma, XCSUDMA_DST_CHANNEL);
	if (Status != XFPGA_SUCCESS) {
		xil_printf("Read from PCAP Failed\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	CmdIndex = 2;
	CmdBuf[CmdIndex++] = 0x30008001; /* Type 1 Write 1 word to CMD */
	CmdBuf[CmdIndex++] = 0x0000000D; /* DESYNC command */
	CmdBuf[CmdIndex++] = 0x20000000; /* NOOP Word*/
	CmdBuf[CmdIndex++] = 0x20000000; /* NOOP Word */

	Status = XFpga_WriteToPcap(CmdIndex, Address + CFGREG_SRCDMA_OFFSET);
	if (Status != XFPGA_SUCCESS) {
		xil_printf("Write to PCAP Failed\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

END:
	/* Disable the PCAP clk */
	RegVal = Xil_In32(PCAP_CLK_CTRL);
	Xil_Out32(PCAP_CLK_CTRL, RegVal & ~(PCAP_CLK_EN_MASK));
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function performs the readback of fpga configuration data.
 *
 * @param PLInfoPtr Pointer to the XFpga_info structure.
 *
 * @return
 *               - XFPGA_SUCCESS if successful
 *               - XFPGA_FAILURE if unsuccessful
 *
 * @note None.
 ****************************************************************************/
static u32 XFpga_GetPLConfigData(XFpga_Info *PLInfoPtr)
{
	u32 Status;
	UINTPTR Address = PLInfoPtr->ReadbackAddr;
	u32 NumFrames = PLInfoPtr->NumFrames;
	u32 RegVal;
	unsigned int cmdindex;
	u32 *CmdBuf;
	int i;

	Status = XFpga_GetFirmwareState();

	if (Status == XFPGA_FIRMWARE_STATE_UNKNOWN) {
		xil_printf("Error while reading configuration "
			   "data from FPGA\n\r");
		Status = XFPGA_ERROR_PLSTATE_UNKNOWN;
		goto END;
	}

	if (Status == XFPGA_FIRMWARE_STATE_SECURE) {
		xil_printf("Operation not permitted\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	CmdBuf = (void *)(UINTPTR)Address;

	/* Enable the PCAP clk */
	RegVal = Xil_In32(PCAP_CLK_CTRL);

	/*
	 * There is no h/w flow control for pcap read
	 * to prevent the FIFO from over flowing, reduce
	 * the PCAP operating frequency.
	 */
	RegVal |= 0x3F00;
	Xil_Out32(PCAP_CLK_CTRL, RegVal | PCAP_CLK_EN_MASK);

	/* Initialize the CSU DMA */
	Status = XFpga_CsuDmaInit();
	if (Status != XFPGA_SUCCESS) {
		xil_printf("CUSDMA init failed\n\r");
		goto END;
	}

	/* Take PCAP out of Reset */
	Status = XFpga_PcapInit(1);
	if (Status != XFPGA_SUCCESS) {
		Status = XPFGA_ERROR_PCAP_INIT;
		xil_printf("PCAP init failed\n\r");
		goto END;
	}

	cmdindex = 0;

	/* Step 1 */
	CmdBuf[cmdindex++] = 0xFFFFFFFF; /* Dummy Word */
	CmdBuf[cmdindex++] = 0x000000BB; /* Bus Width Sync Word */
	CmdBuf[cmdindex++] = 0x11220044; /* Bus Width Detect */
	CmdBuf[cmdindex++] = 0xFFFFFFFF; /* Dummy Word */
	CmdBuf[cmdindex++] = 0xAA995566; /* Sync Word */

	/* Step 2 */
	CmdBuf[cmdindex++] = 0x02000000; /* Type 1 NOOP Word 0 */

	/* Step 3 */         /* Type 1 Write 1 Word to CMD */
	CmdBuf[cmdindex++] = Xfpga_RegAddr(CMD, OPCODE_WRITE, 0x1);
	CmdBuf[cmdindex++] = 0x0000000B; /* SHUTDOWN Command */
	CmdBuf[cmdindex++] = 0x02000000; /* Type 1 NOOP Word 0 */

	/* Step 4 */         /* Type 1 Write 1 Word to CMD */
	CmdBuf[cmdindex++] = Xfpga_RegAddr(CMD, OPCODE_WRITE, 0x1);
	CmdBuf[cmdindex++] = 0x00000007; /* RCRC Command */
	CmdBuf[cmdindex++] = 0x20000000; /* Type 1 NOOP Word 0 */

	/* Step 5 --- 5 NOOPS Words */
	for (i = 0 ; i < 5 ; i++) {
		CmdBuf[cmdindex++] = 0x20000000;
	}

	/* Step 6 */         /* Type 1 Write 1 Word to CMD */
	CmdBuf[cmdindex++] = Xfpga_RegAddr(CMD, OPCODE_WRITE, 0x1);
	CmdBuf[cmdindex++] = 0x00000004; /* RCFG Command */
	CmdBuf[cmdindex++] = 0x20000000; /* Type 1 NOOP Word 0 */

	/* Step 7 */         /* Type 1 Write 1 Word to FAR */
	CmdBuf[cmdindex++] = Xfpga_RegAddr(FAR, OPCODE_WRITE, 0x1);
	CmdBuf[cmdindex++] = 0x00000000; /* FAR Address = 00000000 */

	/* Step 8 */          /* Type 1 Read 0 Words from FDRO */
	CmdBuf[cmdindex++] =  Xfpga_RegAddr(FDRO, OPCODE_READ, 0);
			      /* Type 2 Read Wordlenght Words from FDRO */
	CmdBuf[cmdindex++] = Xfpga_Type2Pkt(OPCODE_READ, NumFrames);

	/* Step 9 --- 64 NOOPS Words */
	for (i = 0 ; i < 64 ; i++) {
		CmdBuf[cmdindex++] = 0x20000000;
	}

	XCsuDma_EnableIntr(&CsuDma, XCSUDMA_DST_CHANNEL,
			   XCSUDMA_IXR_DST_MASK);

	/* Flush the DMA buffer */
	Xil_DCacheFlushRange(Address, NumFrames * 4);

	/* Set up the Destination DMA Channel*/
	XCsuDma_Transfer(&CsuDma, XCSUDMA_DST_CHANNEL,
			 Address + CFGDATA_DSTDMA_OFFSET, NumFrames, 0);

	Status = XFpga_PcapWaitForDone();
	if (Status != XFPGA_SUCCESS) {
		xil_printf("Write to PCAP Failed\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	Status = XFpga_WriteToPcap(cmdindex, Address);
	if (Status != XFPGA_SUCCESS) {
		xil_printf("Write to PCAP Failed\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	/*
	 * Setup the  SSS, setup the DMA to receive from PCAP source
	 */
	Xil_Out32(CSU_CSU_SSS_CFG, XFPGA_CSU_SSS_SRC_DST_DMA);
	Xil_Out32(CSU_PCAP_RDWR, 0x1);


	/* wait for the DST_DMA to complete and the pcap to be IDLE */
	Status = XCsuDma_WaitForDoneTimeout(&CsuDma, XCSUDMA_DST_CHANNEL);
	if (Status != XFPGA_SUCCESS) {
		xil_printf("Read from PCAP Failed\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	Status = XFpga_PcapWaitForidle();
	if (Status != XFPGA_SUCCESS) {
		xil_printf("Reading data from PL through PCAP Failed\n\r");
		Status = XFPGA_FAILURE;
		goto END;
	}

	cmdindex = 0;
	/* Step 11 */
	CmdBuf[cmdindex++] = 0x20000000; /* Type 1 NOOP Word 0 */

	/* Step 12 */
	CmdBuf[cmdindex++] = 0x30008001; /* Type 1 Write 1 Word to CMD */
	CmdBuf[cmdindex++] = 0x00000005; /* START Command */
	CmdBuf[cmdindex++] = 0x20000000; /* Type 1 NOOP Word 0 */

	/* Step 13 */
	CmdBuf[cmdindex++] = 0x30008001; /* Type 1 Write 1 Word to CMD */
	CmdBuf[cmdindex++] = 0x00000007; /* RCRC Command */
	CmdBuf[cmdindex++] = 0x20000000; /* Type 1 NOOP Word 0 */

	/* Step 14 */
	CmdBuf[cmdindex++] = 0x30008001; /* Type 1 Write 1 Word to CMD */
	CmdBuf[cmdindex++] = 0x0000000D; /* DESYNC Command */

	/* Step 15 */
	CmdBuf[cmdindex++] = 0x20000000; /* Type 1 NOOP Word 0 */
	CmdBuf[cmdindex++] = 0x20000000; /* Type 1 NOOP Word 0 */

	Status = XFpga_WriteToPcap(cmdindex, (UINTPTR)Address);
	if (Status != XFPGA_SUCCESS) {
		xil_printf("Write to PCAP 1 Failed\n\r");
		Status = XFPGA_FAILURE;
	}
END:
	/* Disable the PCAP clk */
	RegVal = Xil_In32(PCAP_CLK_CTRL);
	Xil_Out32(PCAP_CLK_CTRL, RegVal & ~(PCAP_CLK_EN_MASK));

	return Status;
}

/****************************************************************************/
/*
 *
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
	return (((XDC_TYPE_1 << XDC_TYPE_SHIFT) |
		(Register << XDC_REGISTER_SHIFT) |
		(OpCode << XDC_OP_SHIFT)) | Size);
}

/****************************************************************************/
/**
 *
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
	return (((XDC_TYPE_2 << XDC_TYPE_SHIFT) |
		(OpCode << XDC_OP_SHIFT)) | Size);
}

/*****************************************************************************/
/** Sets the library firmware state
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
	RegVal |= (State << XFPGA_STATE_SHIFT);
	Xil_Out32(PMU_GLOBAL_GEN_STORAGE5, RegVal);
}

/*****************************************************************************/
/** Returns the library firmware state
 *
 * @param	None
 *
 * @return	library firmware state
 *****************************************************************************/
static u8 XFpga_GetFirmwareState(void)
{
	return (Xil_In32(PMU_GLOBAL_GEN_STORAGE5) & XFPGA_STATE_MASK) >>
		XFPGA_STATE_SHIFT;
}

/*****************************************************************************/
/* This function is responsible for  identifying the Bitstream Endianess,
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
	u8 EndianType;
	u8 BitHdrSize = ARRAY_LENGTH(BootgenBinFormat);
	u32 IsBitNonAligned;

	for (Index = 0; Index < Size; Index++) {
	/* Find the First Dummy Byte */
		if (Buf[Index] == DUMMY_BYTE) {
			if (!(memcmp(&Buf[Index + SYNC_BYTE_POSITION],
			    BootgenBinFormat, BitHdrSize))) {
				EndianType = 0U;
				Status = XFPGA_SUCCESS;
				break;
			} else if (!(memcmp(&Buf[Index + SYNC_BYTE_POSITION],
				   VivadoBinFormat, BitHdrSize))) {
				EndianType = 1U;
				Status = XFPGA_SUCCESS;
				break;
			}
		}
	}

	if (Status == XFPGA_SUCCESS) {
		IsBitNonAligned = Index % 4;
		if (IsBitNonAligned) {
			memcpy(Buf, Buf + IsBitNonAligned, Size - IsBitNonAligned);
			Index -= IsBitNonAligned;
		}

		RegVal = XCsuDma_ReadReg(CsuDma.Config.BaseAddress,
					((u32)(XCSUDMA_CTRL_OFFSET) +
					((u32)XCSUDMA_SRC_CHANNEL *
					(u32)(XCSUDMA_OFFSET_DIFF))));
		RegVal |= (EndianType << (u32)(XCSUDMA_CTRL_ENDIAN_SHIFT)) &
					(u32)(XCSUDMA_CTRL_ENDIAN_MASK);
		XCsuDma_WriteReg(CsuDma.Config.BaseAddress,
				((u32)(XCSUDMA_CTRL_OFFSET) +
				((u32)XCSUDMA_SRC_CHANNEL *
				(u32)(XCSUDMA_OFFSET_DIFF))), RegVal);
		*Pos = Index;
	}

	return Status;
}
