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
*******************************************************************************/
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
 * 			XFpga_PL_BitStream_Load()
 * 			to avoid the unwanted blocking conditions.
 * 3.0   Nava  12/05/17 Added PL configuration registers readback support.
 * 4.0   Nava  08/02/18 Added Authenticated and Encypted Bitstream loading support.
 * 4.0  Nava  02/03/18 Added the legacy bit file loading feature support from U-boot.
 *                     and improve the error handling support by returning the
 *                     proper ERROR value upon error conditions.
 * 4.1  Nava   27/03/18 For Secure Bitstream loading to avoid the Security violations
 *                      Need to Re-validate the User Crypto flags with the Image
 *                      Crypto operation by using the internal memory.To Fix this
 *                      added a new API XFpga_ReValidateCryptoFlags().
 * 4.1 Nava   16/04/18  Added partial bitstream loading support.
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xil_types.h"
#include "xcsudma.h"
#include "sleep.h"
#include "xil_printf.h"
#include "xfpga_config.h"
#include "xilfpga_pcap.h"
#include "xparameters.h"
#ifdef XFPGA_SECURE_MODE
#include "xsecure.h"
#include "xsecure_aes.h"
#include "xsecure_sha.h"
#include "xsecure_rsa.h"
#endif

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
#define IV_LEN				24 	/* Bytes */
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
#define OPCODE_READ                     1

#define XFPGA_DESTINATION_PCAP_ADDR	(0XFFFFFFFFU)
#define XFPGA_PART_IS_ENC		(0x00000080U)
#define XFPGA_PART_IS_AUTH		(0x00008000U)

#define XFPGA_PARTIAL_EN		(0x00000001U)
#define XFPGA_AUTHENTICATION_DDR_EN	(0x00000002U)
#define XFPGA_AUTHENTICATION_OCM_EN	(0x00000004U)
#define XFPGA_ENCRYPTION_USERKEY_EN	(0x00000008U)
#define XFPGA_ENCRYPTION_DEVKEY_EN 	(0x00000010U)
#define XFPGA_ONLY_BIN_EN		(0x00000020U)

#define XFPGA_AES_TAG_SIZE	(XSECURE_SECURE_HDR_SIZE + \
		XSECURE_SECURE_GCM_TAG_SIZE) /* AES block decryption tag size */


#define XFPGA_SECURE_FLAGS	(				\
				XFPGA_AUTHENTICATION_DDR_EN	\
				| XFPGA_AUTHENTICATION_OCM_EN	\
				| XFPGA_ENCRYPTION_USERKEY_EN	\
				| XFPGA_ENCRYPTION_DEVKEY_EN	\
				)

#define XFPGA_AUTH_ENC_USERKEY_DDR	(				\
					XFPGA_AUTHENTICATION_DDR_EN	\
					| XFPGA_ENCRYPTION_USERKEY_EN	\
					)

#define XFPGA_AUTH_ENC_DEVKEY_DDR	(				\
					XFPGA_AUTHENTICATION_DDR_EN	\
					| XFPGA_ENCRYPTION_DEVKEY_EN	\
					)

#define XFPGA_AUTH_ENC_USERKEY_OCM	(				\
					XFPGA_AUTHENTICATION_OCM_EN	\
					| XFPGA_ENCRYPTION_USERKEY_EN	\
					)

#define XFPGA_AUTH_ENC_DEVKEY_OCM	(				\
					XFPGA_AUTHENTICATION_OCM_EN	\
					| XFPGA_ENCRYPTION_DEVKEY_EN	\
					)

/**************************** Type Definitions *******************************/
#ifdef __MICROBLAZE__
typedef u32 (*XpbrServHndlr_t) (void);
#endif

#ifdef XFPGA_SECURE_MODE
typedef struct {
	XSecure_Aes *SecureAes;	/* AES initialized structure */
	u32 NextBlkLen;		/* Not required for user, used
				 * for storing next block size */
} XFpgaPs_PlEncryption;

typedef struct {
	XFpgaPs_PlEncryption PlEncrypt;	/* Encryption parameters */
	u8 SecureHdr[XSECURE_SECURE_HDR_SIZE + XSECURE_SECURE_GCM_TAG_SIZE];
	u8 Hdr;
} XFpgaPs_PlPartition;
#endif

/***************** Macros (Inline Functions) Definitions *********************/
#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

/************************** Function Prototypes ******************************/
static u32 XFpga_PcapWaitForDone();
static u32 XFpga_WriteToPcap(u32 WrSize, UINTPTR WrAddrLow);
static u32 XFpga_PcapInit(u32 flags);
static u32 XFpga_CsuDmaInit();
static u32 XFpga_PLWaitForDone(void);
static u32 XFpga_PowerUpPl(void);
static u32 XFpga_IsolationRestore(void);
static u32 XFpga_PsPlGpioReset(u32 TotalResets);
static u32 Xfpga_RegAddr(u8 Register, u8 OpCode, u8 Size);
static u32 XFpga_GetBitstreamInfo(UINTPTR WrAddr,
				u32 *BitstreamAddress, u32 *BitstreamSize);
static u32 XFpga_ValidateCryptoFlags(UINTPTR WrAddr, u32 flags);
#ifdef XFPGA_SECURE_MODE
static u32 XFpga_SecureLoadToPl(UINTPTR WrAddr,	UINTPTR KeyAddr,
				XSecure_ImageInfo *ImageInfo, u32 flags );
static u32 XFpga_WriteEncryptToPcap(UINTPTR WrAddr, UINTPTR KeyAddr,
				XSecure_ImageInfo *ImageInfo, u32 flags);
static u32 XFpga_SecureBitstreamDdrLoad (UINTPTR WrAddr, UINTPTR KeyAddr,
				XSecure_ImageInfo *ImageInfo, u32 flags);
static u32 XFpga_AesInit(UINTPTR KeyAddr, u32 *AesIv, u32 flags);
static u32 XFpga_AuthBitstreamOcmLoad (UINTPTR WrAddr, UINTPTR KeyAddr,
				XSecure_ImageInfo *ImageInfo, u32 flags);
static u32 Xilfpga_ConvertCharToNibble(char InChar, u8 *Num);
static u32 Xilfpga_ConvertStringToHex(const char * Str, u32 * buf, u8 Len);
static u32 XFpga_CopyToOcm(UINTPTR Src, UINTPTR Dst, u32 WrSize);
static u32 XFpga_AuthPlChunks(UINTPTR WrAddr, u32 WrSize, UINTPTR AcAddr);
static u32 XFpga_ReAuthPlChunksWriteToPl(UINTPTR WrAddr, u32 WrSize, u32 flags);
static u32 XFpga_DecrptPlChunks(XFpgaPs_PlPartition *PartitionParams,
				u64 ChunkAdrs, u32 ChunkSize);
static u32 XFpga_DecrptSetUpNextBlk(XFpgaPs_PlPartition *PartitionParams);
static void XFpga_DmaPlCopy(XCsuDma *InstancePtr, UINTPTR Src,
					u32 Size, u8 EnLast);
static u32 XFpga_DecrptPl(XFpgaPs_PlPartition *PartitionParams,
				u64 ChunkAdrs, u32 ChunkSize);
static u32 XFpga_DecrypSecureHdr(XSecure_Aes *InstancePtr, u64 SrcAddr);
static u32 XFpga_ReValidateCryptoFlags(XSecure_ImageInfo *ImageInfo, u32 flags);
#endif
#ifdef __MICROBLAZE__
extern const XpbrServHndlr_t XpbrServHndlrTbl[XPBR_SERV_EXT_TBL_MAX];
#endif
/************************** Variable Definitions *****************************/
XCsuDma CsuDma;
#ifdef XFPGA_SECURE_MODE
XSecure_Aes Secure_Aes;
u32 key[8];
XFpgaPs_PlPartition PlAesInfo;
XSecure_Sha3 Secure_Sha3;
XSecure_Rsa Secure_Rsa;
#endif

/*****************************************************************************/

/*****************************************************************************/
/** The API is used to load the user provided bitstream file into Zync MPSoC PL region. 
 * This function does the following jobs:
 *		- Power-up the PL fabric.
 *		- Performs PL-PS Isolation.
 *		- Initialize PCAP Interface
 *		- Write a bitstream into the PL
 *		- Wait for the PL Done Status.
 *		- Restore PS-PL Isolation (Power-up PL fabric).

 * @note This function contains the polling implementation to provide the PL reset wait time due to this
 * polling implementation the function call is blocked till the time out value expires or gets the appropriate status value from the PL Done Status register.
 *
 *@param WrAddr Linear memory image base address
 *
 *@param AddrPtr Aes key address which is used for Decryption.
 *
 *@param flags: Flags are used to specify the type of bitstream file.
 * 			* BIT(0) - Bit-stream type
 *					* 0 - Full Bit-stream
 *					* 1 - Partial Bit-stream
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
 * @note The current implementation will not support partial  Bit-stream loading.
 *
 * @return 
 *			- Error status based on implemented functionality (SUCCESS by default).
 *
 *
 *****************************************************************************/
u32 XFpga_PL_BitSream_Load (UINTPTR WrAddr, UINTPTR AddrPtr, u32 flags)
{
	u32 Status = XFPGA_SUCCESS;
	u32 BitstreamAddress;
	u32 BitstreamSize;
	u32 RegVal;
#ifdef XFPGA_SECURE_MODE
	u32 EncOnly;
	u8 IsEncrypted = 0;
	u8 NoAuth = 0;
	u8 *IvPtr = (u8 *)(UINTPTR)Iv;
	XSecure_ImageInfo ImageHdrInfo = {0};
#endif

	/* Address provided is null */
	if ((u8 *)(UINTPTR)WrAddr == NULL)
		return XST_FAILURE;

#ifndef XFPGA_SECURE_MODE
	if (!(flags & XFPGA_ONLY_BIN_EN))
#endif
	/* validate the User Flags for the Image Crypto operation */
	Status = XFpga_ValidateCryptoFlags(WrAddr, flags);
	if (Status != XFPGA_SUCCESS) {
		xil_printf("Crypto flags not matched with Image crypto operation\r\n");
		Status = XFPGA_ERROR_CRYPTO_FLAGS;
		goto END;
	}

	/* Initialize CSU DMA driver */
	Status = XFpga_CsuDmaInit();
	if (Status != XFPGA_SUCCESS)
		goto END;

#ifdef XFPGA_SECURE_MODE
	Status = XSecure_AuthenticationHeaders((u8 *)WrAddr, &ImageHdrInfo);
	if (Status != XST_SUCCESS) {
	/* Error other than XSECURE_AUTH_NOT_ENABLED error will be an error */
		if (Status != XSECURE_AUTH_NOT_ENABLED) {
			Status = XFPGA_ERROR_HDR_AUTH;
			goto END;
		} else {
		/* Here Buffer still contains Boot header */
			NoAuth = 1;
		}
	}

	if (NoAuth != 0x00 ) {
		XSecure_PartitionHeader *Ph =
				(XSecure_PartitionHeader *)(UINTPTR)
				(WrAddr + Xil_In32((UINTPTR)Buffer +
						XSECURE_PH_TABLE_OFFSET));
		ImageHdrInfo.PartitionHdr = Ph;
		if ((ImageHdrInfo.PartitionHdr->PartitionAttributes &
				XSECURE_PH_ATTR_AUTH_ENABLE) != 0x00U) {
			Status = XFPGA_ERROR_CRYPTO_FLAGS;
			goto END;
		}
	}

	if (ImageHdrInfo.PartitionHdr->PartitionAttributes &
				XSECURE_PH_ATTR_ENC_ENABLE)
		IsEncrypted = 1;

	EncOnly = XSecure_IsEncOnlyEnabled();
	if (EncOnly != 0x00) {

		if (!IsEncrypted) {
			Status = XFPGA_ENC_ISCOMPULSORY;
			goto END;
		}
	}

	if ((IsEncrypted) && (NoAuth)) {
		ImageHdrInfo.KeySrc = Xil_In32((UINTPTR)Buffer +
					XSECURE_KEY_SOURCE_OFFSET);
		XSecure_MemCopy(ImageHdrInfo.Iv,
				(Buffer + XSECURE_IV_OFFSET), XSECURE_IV_SIZE);
		/* Add partition header IV to boot header IV */
		*(IvPtr + XSECURE_IV_LEN) = (*(IvPtr + XSECURE_IV_LEN)) +
			(ImageHdrInfo.PartitionHdr->Iv & XSECURE_PH_IV_MASK);
	}

	/*
	 * When authentication exists and requesting
	 * for device key other than eFUSE and KUP key
	 * when ENC_ONLY bit is blown
	 */
	if (EncOnly != 0x00) {
		if ((ImageHdrInfo.KeySrc == XSECURE_KEY_SRC_BBRAM) ||
			(ImageHdrInfo.KeySrc == XSECURE_KEY_SRC_GREY_BH) ||
			(ImageHdrInfo.KeySrc == XSECURE_KEY_SRC_BLACK_BH)) {
			Status = XSECURE_DEC_WRONG_KEY_SOURCE;
			goto END;
		}
	}

	/* Re-Validate the User Flags for the Image Crypto operation */
	Status = XFpga_ReValidateCryptoFlags(&ImageHdrInfo, flags);
	if (Status != XFPGA_SUCCESS) {
		xil_printf("Crypto flags not matched with Image crypto operation\r\n");
		Status = XFPGA_ERROR_SECURE_CRYPTO_FLAGS;
		goto END;
	}


#endif

	/* Enable the PCAP clk */
	RegVal = Xil_In32(PCAP_CLK_CTRL);
	Xil_Out32(PCAP_CLK_CTRL, RegVal | PCAP_CLK_EN_MASK );

	if (!(flags & XFPGA_PARTIAL_EN)) {
		/* Power-Up PL */
		Status = XFpga_PowerUpPl();
		if (Status != XFPGA_SUCCESS) {
			xil_printf("XFPGA_ERROR_PL_POWER_UP\r\n");
			Status = XFPGA_ERROR_PL_POWER_UP;
			goto END;
		}

		/* PS PL Isolation Restore */
		Status = XFpga_IsolationRestore();
		if (Status != XFPGA_SUCCESS) {
			xil_printf("XFPGA_ERROR_PL_ISOLATION\r\n");
			Status = XFPGA_ERROR_PL_ISOLATION;
			goto END;
		}
	}

	Status = XFpga_PcapInit(flags);
	if(Status != XFPGA_SUCCESS) {
		Status = XPFGA_ERROR_PCAP_INIT;
		goto END;
	}

	if (flags & XFPGA_SECURE_FLAGS)
#ifdef XFPGA_SECURE_MODE
	{
		Status = XFpga_SecureLoadToPl(WrAddr, AddrPtr,
					&ImageHdrInfo, flags );
		if (Status != XFPGA_SUCCESS) {
			/* Clear the PL house */
			Xil_Out32(CSU_PCAP_PROG, 0x0U);
			usleep(PL_RESET_PERIOD_IN_US);
			Xil_Out32(CSU_PCAP_PROG, CSU_PCAP_PROG_PCFG_PROG_B_MASK);
		}

	}
#else
	{
		xil_printf("Fail to load: Enable secure mode and try...\r\n");
		Status = XFPGA_ERROR_BITSTREAM_LOAD_FAIL;
		goto END;
	}
#endif
	else {
		if (!(flags & XFPGA_ONLY_BIN_EN))
			XFpga_GetBitstreamInfo(WrAddr,
				&BitstreamAddress, &BitstreamSize);
		else {
			/* It provides the legacy full Bit-stream
			 * loading support (Bit file without Headers).
			 */
			BitstreamAddress = WrAddr;
			BitstreamSize	= *((UINTPTR *)(AddrPtr));
		}

		Status = XFpga_WriteToPcap(BitstreamSize/WORD_LEN,
						BitstreamAddress);
	}

	if (Status != XFPGA_SUCCESS) {
		xil_printf("FPGA fail to write Bit-stream into PL\n");
		Status = XFPGA_ERROR_BITSTREAM_LOAD_FAIL;
		goto END;
	}

	Status = XFpga_PLWaitForDone();
	if(Status != XFPGA_SUCCESS) {
		xil_printf("FPGA fail to get the done status\n");
		goto END;
	}

	if (!(flags & XFPGA_PARTIAL_EN)) {

		/* Power-Up PL */
		Status = XFpga_PowerUpPl();
		if (Status != XFPGA_SUCCESS) {
			xil_printf("XFPGA_ERROR_PL_POWER_UP\r\n");
			Status = XFPGA_ERROR_PL_POWER_UP;
			goto END;
		}

		/* PS-PL reset */
		XFpga_PsPlGpioReset(FPGA_NUM_FABRIC_RESETS);
	}
END:
	/* Disable the PCAP clk */
	RegVal = Xil_In32(PCAP_CLK_CTRL);
	Xil_Out32(PCAP_CLK_CTRL, RegVal & ~(PCAP_CLK_EN_MASK) );
#ifdef XFPGA_SECURE_MODE
	if ((u8 *)AddrPtr != NULL)
		memset((u8 *)AddrPtr, 0, KEY_LEN);
#endif
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
static u32 XFpga_PcapInit(u32 flags) {
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
	if (!PollCount)
			return XFPGA_FAILURE;

	return Status;
}
/*****************************************************************************/
/** Waits for PCAP transfer to complete
 *
 * @param	None
 *
 * @return	Error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_PcapWaitForDone() {
	u32 RegVal = 0U;
	u32 PollCount;
	u32 Status = XFPGA_SUCCESS;

	PollCount = (PL_DONE_POLL_COUNT);
	while(PollCount) {
		RegVal = Xil_In32(CSU_PCAP_STATUS);
		RegVal = RegVal & CSU_PCAP_STATUS_PCAP_WR_IDLE_MASK;
		if (RegVal == CSU_PCAP_STATUS_PCAP_WR_IDLE_MASK)
			break;
		PollCount--;
	}
	if (!PollCount)
		return XFPGA_FAILURE;

	return Status;
}

/*****************************************************************************/
/** Writes data to PCAP interface
 *
 * @param WrSize Number of bytes that the DMA should write to the
 *        PCAP interface
 * @param WrAddr Linear Bitstream memory base address
 *
 * @return Error status based on implemented functionality (SUCCESS by default)
 *****************************************************************************/
static u32 XFpga_WriteToPcap(u32 WrSize, UINTPTR WrAddr) {
	u32 Status = XFPGA_SUCCESS;

	/*
	 * Setup the  SSS, setup the PCAP to receive from DMA source
	 */
	Xil_Out32(CSU_CSU_SSS_CFG, XFPGA_CSU_SSS_SRC_SRC_DMA);

	/* Setup the source DMA channel */
	XCsuDma_Transfer(&CsuDma, XCSUDMA_SRC_CHANNEL, WrAddr, WrSize, 0);

	/* wait for the SRC_DMA to complete and the pcap to be IDLE */
	XCsuDma_WaitForDone(&CsuDma, XCSUDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	Status = XFpga_PcapWaitForDone();
	return Status;
}

/*****************************************************************************/
/** Used to Bit-stream info from the Image.
 *
 * @param WrAddr Linear memory secure image base address
 * @param BitstreamAddress: Bit-stream Base address.
 * @param BitstreamSize: Bit-stream Size.
 *
 * @return Error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_GetBitstreamInfo(UINTPTR WrAddr,
				u32 *BitstreamAddress,
				u32 *BitstreamSize) {
	u32 Status = XFPGA_SUCCESS;
	u32 PartHeaderOffset;
	u32 BitstreamOffset;

	PartHeaderOffset = *((UINTPTR *)(WrAddr + PARTATION_HEADER_OFFSET));
	BitstreamOffset = *((UINTPTR *)(WrAddr + PartHeaderOffset + BITSTREAM_PARTATION_OFFSET));
	*BitstreamAddress = (BitstreamOffset * WORD_LEN) + WrAddr;
	*BitstreamSize = *((UINTPTR *)(WrAddr + PartHeaderOffset)) * WORD_LEN;

	return Status;
}
/*****************************************************************************/
/** Validates the user provided crypto flags
 *  with Image crypto flags.
 * @param WrAddr Linear memory secure image base address
 * @param flags It provides the information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 *
 * @return Error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_ValidateCryptoFlags(UINTPTR WrAddr, u32 flags) {
	u32 Status = XFPGA_SUCCESS;
	u32 *PartAttributesPtr;
	u32 PartOffset;
	u8 IsImageAuthenticated;
	u8 IsImageEncrypted;
	u8 IsFlagSetToAuthentication;
	u8 IsFlagSetToEncryption;

	PartOffset = *((u32 *)(WrAddr + PARTATION_HEADER_OFFSET));
	PartAttributesPtr = (u32 *)(WrAddr + PartOffset + PARTATION_ATTRIBUTES_OFFSET);


	if (*PartAttributesPtr & XFPGA_PART_IS_AUTH)
		IsImageAuthenticated = 1;
	else
		IsImageAuthenticated = 0;

	if (*PartAttributesPtr & XFPGA_PART_IS_ENC)
		IsImageEncrypted = 1;
	else
		IsImageEncrypted = 0;

	if ((flags & XFPGA_AUTHENTICATION_DDR_EN) ||
			(flags & XFPGA_AUTHENTICATION_OCM_EN))
		IsFlagSetToAuthentication = 1;
	else
		IsFlagSetToAuthentication = 0;

	if ((flags & XFPGA_ENCRYPTION_USERKEY_EN) ||
			(flags & XFPGA_ENCRYPTION_DEVKEY_EN))
		IsFlagSetToEncryption = 1;
	else
		IsFlagSetToEncryption = 0;

	if ((IsImageAuthenticated == IsFlagSetToAuthentication) &&
			(IsImageEncrypted == IsFlagSetToEncryption))
		Status = XFPGA_SUCCESS;
	else
		Status = XFPGA_FAILURE;


	return Status;
}
#ifdef XFPGA_SECURE_MODE
/*****************************************************************************/
/** This function is used to Re-Validate the user provided crypto flags
 *  with Image crypto flags.
 * @param ImageInfo	Pointer to XSecure_ImageInfo structure.
 * @param flags It provides the information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 *
 * @return error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_ReValidateCryptoFlags(XSecure_ImageInfo *ImageInfo, u32 flags) {
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
				(ImageInfo->KeySrc == XFPGA_KEY_SRC_BH_GRY))
			IsImageDevKeyEncrypted = 1;
		else if (ImageInfo->KeySrc == XFPGA_KEY_SRC_KUP)
			IsImageUserKeyEncrypted = 1;
	}

	if ((flags & XFPGA_AUTHENTICATION_DDR_EN) ||
			(flags & XFPGA_AUTHENTICATION_OCM_EN))
		IsFlagSetToAuthentication = 1;

	if (flags & XFPGA_ENCRYPTION_USERKEY_EN)
			IsFlagSetToUserKeyEncryption = 1;

	if (flags & XFPGA_ENCRYPTION_DEVKEY_EN)
		IsFlagSetToDevKeyEncryption = 1;

	if ((IsImageAuthenticated == IsFlagSetToAuthentication) &&
			(IsImageDevKeyEncrypted == IsFlagSetToDevKeyEncryption) &&
			(IsImageUserKeyEncrypted == IsFlagSetToUserKeyEncryption))
		Status = XFPGA_SUCCESS;
	else
		Status = XFPGA_FAILURE;

	return Status;
}

/*****************************************************************************/
/** Loads the secure Bit-stream into the PL.
 *
 * @param WrAddr Linear memory secure image base address
 * @param KeyAddr Aes key address which is used for Decryption.
 * @param flags It provides the information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 * @return Error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_SecureLoadToPl(UINTPTR WrAddr, UINTPTR KeyAddr,
			XSecure_ImageInfo *ImageInfo, u32 flags ){
	u32 Status;

	switch (flags) {

	case XFPGA_AUTHENTICATION_DDR_EN:
	case XFPGA_AUTH_ENC_USERKEY_DDR:
	case XFPGA_AUTH_ENC_DEVKEY_DDR:
		Status = XFpga_SecureBitstreamDdrLoad(WrAddr,
					KeyAddr, ImageInfo, flags);
		break;

	case XFPGA_AUTHENTICATION_OCM_EN:
	case XFPGA_AUTH_ENC_USERKEY_OCM:
	case XFPGA_AUTH_ENC_DEVKEY_OCM:
		Status = XFpga_AuthBitstreamOcmLoad(WrAddr,
					KeyAddr, ImageInfo, flags);
		break;

	case XFPGA_ENCRYPTION_USERKEY_EN:
		Status = XFpga_WriteEncryptToPcap(WrAddr,
					KeyAddr, ImageInfo, flags);
		break;

	default:

		xil_printf("Invalid Option\r\n");


	}

return Status;
}
/*****************************************************************************/
/* Authenticates the bit-stream by using external memory.
 * Sends the data to PCAP via AES engine if encryption exists or directly
 * to PCAP by CSUDMA if an encryption is not enabled.
 *
 * @param WrAddr Linear memory secure image base address
 * @param KeyAddr Aes key address which used for decryption.
 * @param flags It provides the information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 * @return error status based on implemented functionality (SUCCESS by default)

 *****************************************************************************/
static u32 XFpga_SecureBitstreamDdrLoad (UINTPTR WrAddr, UINTPTR KeyAddr,
				XSecure_ImageInfo *ImageInfo, u32 flags) {
	u32 Status = XFPGA_SUCCESS;
	u32 PartationLen;
	u32 PartationOffset;
	u32 PartationAcOffset;
	u8 	TotalBitPartCount;
	u32	RemaningBytes;
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
	BitAddr = PartationOffset + WrAddr;
	AcPtr = PartationAcOffset + WrAddr;

	if ((flags & XFPGA_ENCRYPTION_USERKEY_EN)
			|| (flags & XFPGA_ENCRYPTION_DEVKEY_EN))
		XFpga_AesInit(KeyAddr, ImageInfo->Iv, flags);

	for(int i = 0; i < TotalBitPartCount; i++)
	{
		/* Copy authentication certificate to internal memory */
		XSecure_MemCopy(AcBuf, (u8 *)AcPtr,
				XSECURE_AUTH_CERT_MIN_SIZE/XSECURE_WORD_LEN);
		/*Verify Spk */
		Status = XSecure_VerifySpk(AcBuf, ImageInfo->EfuseRsaenable);
		if (Status != XST_SUCCESS) {
			Status = XFPGA_PARTITION_AUTH_FAILURE;
			goto END;
		}

		/* Authenticate Partition */
		Status = XSecure_PartitionAuthentication(&CsuDma, (u8 *)BitAddr,
							 PL_PARTATION_SIZE,
							(u8 *)(UINTPTR)AcBuf);
		if (Status != XST_SUCCESS) {
			Status = XFPGA_PARTITION_AUTH_FAILURE;
			goto END;
		}
		if ((flags & XFPGA_ENCRYPTION_USERKEY_EN)
				|| (flags & XFPGA_ENCRYPTION_DEVKEY_EN))
			Status = XFpga_DecrptPlChunks(&PlAesInfo, BitAddr,
							PL_PARTATION_SIZE);
		else
			Status = XFpga_WriteToPcap(PL_PARTATION_SIZE/WORD_LEN,
								BitAddr);

		if (Status != XFPGA_SUCCESS){
			Status = XFPGA_ERROR_BITSTREAM_LOAD_FAIL;
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
			Status = XFPGA_PARTITION_AUTH_FAILURE;
			goto END;
		}

		/* Authenticate Partition */
		Status = XSecure_PartitionAuthentication(&CsuDma, (u8 *)BitAddr,
							 RemaningBytes,
							(u8 *)(UINTPTR)AcBuf);
		if (Status != XST_SUCCESS) {
			Status = XFPGA_PARTITION_AUTH_FAILURE;
			goto END;
		}

		if ((flags & XFPGA_ENCRYPTION_USERKEY_EN)
				|| (flags & XFPGA_ENCRYPTION_DEVKEY_EN))
			Status = XFpga_DecrptPlChunks(&PlAesInfo, BitAddr,
							RemaningBytes);
		else
			Status = XFpga_WriteToPcap(RemaningBytes/WORD_LEN,
								BitAddr);

		if (Status != XFPGA_SUCCESS){
			Status = XFPGA_ERROR_BITSTREAM_LOAD_FAIL;
			goto END;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/* This function authenticates the bit-stream by using on-chip memory.
 * Sends the data to PCAP in blocks via AES engine if encryption
 * exists or directly to PCAP by CSUDMA if an encryption is not enabled.
 *
 * @param WrAddr Linear memory secure image base address
 * @param KeyAddr Aes key address which used for decryption.
 * @param flags It provides the information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 * @return error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_AuthBitstreamOcmLoad (UINTPTR WrAddr, UINTPTR KeyAddr,
				XSecure_ImageInfo *ImageInfo, u32 flags) {
	u32 Status = XFPGA_SUCCESS;
	u32 PartationLen;
	u32 PartationOffset;
	u32 PartationAcOffset;
	u8 	TotalBitPartCount;
	u32	RemaningBytes;
	UINTPTR AcPtr;
	UINTPTR BitAddr = WrAddr;

	/* Authenticate the PL Partation's */
	PartationOffset = ImageInfo->PartitionHdr->DataWordOffset
						* XSECURE_WORD_LEN;
	PartationAcOffset = ImageInfo->PartitionHdr->AuthCertificateOffset
							* XSECURE_WORD_LEN;
	PartationLen = PartationAcOffset - PartationOffset;
	TotalBitPartCount = PartationLen/PL_PARTATION_SIZE;
	RemaningBytes = PartationLen - (TotalBitPartCount * PL_PARTATION_SIZE);
	BitAddr = PartationOffset + WrAddr;
	AcPtr = PartationAcOffset + WrAddr;

	if ((flags & XFPGA_ENCRYPTION_USERKEY_EN)
			|| (flags & XFPGA_ENCRYPTION_DEVKEY_EN))
		XFpga_AesInit(KeyAddr, ImageInfo->Iv, flags);

	for(int i = 0; i < TotalBitPartCount; i++)
	{

		/* Copy authentication certificate to internal memory */
		XSecure_MemCopy(AcBuf, (u8 *)AcPtr,
				XSECURE_AUTH_CERT_MIN_SIZE/XSECURE_WORD_LEN);
		/*Verify Spk */
		Status = XSecure_VerifySpk(AcBuf, ImageInfo->EfuseRsaenable);
		if (Status != XST_SUCCESS) {
			Status = XFPGA_PARTITION_AUTH_FAILURE;
			goto END;
		}

		Status = XFpga_AuthPlChunks((UINTPTR)BitAddr,
				PL_PARTATION_SIZE, (UINTPTR)AcBuf);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_PARTITION_AUTH_FAILURE;
			goto END;
		}

		Status = XFpga_ReAuthPlChunksWriteToPl((UINTPTR)BitAddr,
						PL_PARTATION_SIZE, flags);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_PARTITION_AUTH_FAILURE;
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
			Status = XFPGA_PARTITION_AUTH_FAILURE;
			goto END;
		}

		Status =XFpga_AuthPlChunks((UINTPTR)BitAddr,
				RemaningBytes, (UINTPTR)AcBuf);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_PARTITION_AUTH_FAILURE;
			goto END;
		}
		Status = XFpga_ReAuthPlChunksWriteToPl((UINTPTR)BitAddr,
						RemaningBytes, flags);
		if (Status != XFPGA_SUCCESS) {
			Status = XFPGA_PARTITION_AUTH_FAILURE;
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
static u32 XFpga_AuthPlChunks(UINTPTR WrAddr, u32 WrSize, UINTPTR AcAddr) {
	u32 Status = XFPGA_SUCCESS;
	u64 OcmAddr = OCM_PL_ADDR;
	u32 NumChunks = NUM_OF_PL_CHUNKS(WrSize);
	u32 ChunkSize = PL_CHUNK_SIZE_BYTES;
	u32 OcmChunkAddr = OCM_PL_ADDR + ChunkSize;
	u32 RemaningBytes;
	u32 Count;
	XSecure_RsaKey Key;
	u8 *AcPtr = (u8 *)(UINTPTR)AcAddr;
	u8 *Signature = (AcPtr + XSECURE_AUTH_CERT_PARTSIG_OFFSET);
	u8 Sha3Hash[HASH_LEN];

	RemaningBytes = (WrSize - (ChunkSize * NumChunks));

	XSecure_Sha3Initialize(&Secure_Sha3, &CsuDma);
	XSecure_Sha3Start(&Secure_Sha3);
	for(Count = 0; Count < NumChunks; Count++) {
		Status = XFpga_CopyToOcm((UINTPTR)WrAddr,
				(UINTPTR)OcmAddr, ChunkSize/WORD_LEN);
		if (Status != XFPGA_SUCCESS)
			return Status;

		/* Generating SHA3 hash */
		XSecure_Sha3Update(&Secure_Sha3,
				(u8 *)(UINTPTR)OcmAddr, ChunkSize);
		XSecure_Sha3_ReadHash(&Secure_Sha3, Sha3Hash);

		/* Copy SHA3 hash into the OCM */
		memcpy((u32 *)(UINTPTR)OcmChunkAddr, Sha3Hash, HASH_LEN);
		OcmChunkAddr = OcmChunkAddr + HASH_LEN;
		WrAddr = WrAddr + ChunkSize;
	}

	if (RemaningBytes) {
		Status = XFpga_CopyToOcm((UINTPTR)WrAddr,(UINTPTR)OcmAddr,
							RemaningBytes/WORD_LEN);
		if (Status != XFPGA_SUCCESS)
			return Status;

		/* Generating SHA3 hash */
		XSecure_Sha3Update(&Secure_Sha3,
				(u8 *)(UINTPTR)OcmAddr, RemaningBytes);
		XSecure_Sha3_ReadHash(&Secure_Sha3, Sha3Hash);
		/* Copy SHA3 hash into the OCM */
		memcpy((u32 *)(UINTPTR)OcmChunkAddr, Sha3Hash, HASH_LEN);
	}

	/* Copy AC into the OCM */
	Status = XFpga_CopyToOcm((UINTPTR)AcAddr, (UINTPTR)OcmAddr, AC_LEN/WORD_LEN);
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

	return Status;
}
/*****************************************************************************/
/* This function Re-authenticates the bit-stream by using on-chip memory.
 * Sends the data to PCAP in blocks via AES engine if encryption
 * exists or directly to PCAP by CSUDMA if an encryption is not enabled.
 *
 * @param WrSize Number of bytes that the DMA should write to the
 *        PCAP interface.
 * @param WrAddr Linear memory secure image base address
 * @param flags It provides the information about Crypto operation needs
 *        to be performed on the given Image (or) Data.
 * @return error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_ReAuthPlChunksWriteToPl(UINTPTR WrAddr,
				u32 WrSize, u32 flags) {
	u32 Status = XFPGA_SUCCESS;
	u64 OcmAddr = OCM_PL_ADDR;
	u32 NumChunks = NUM_OF_PL_CHUNKS(WrSize);
	u32 ChunkSize = PL_CHUNK_SIZE_BYTES;
	u32 OcmChunkAddr = OCM_PL_ADDR + ChunkSize;
	u32 RemaningBytes;
	u32 Count;
	u8 Sha3Hash[HASH_LEN];

	RemaningBytes = (WrSize  - (ChunkSize * NumChunks));

	XSecure_Sha3Initialize(&Secure_Sha3, &CsuDma);
	XSecure_Sha3Start(&Secure_Sha3);
	for(Count = 0; Count < NumChunks; Count++) {
		Status = XFpga_CopyToOcm((UINTPTR)WrAddr,
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
			goto END;
		} else
			OcmChunkAddr = OcmChunkAddr + HASH_LEN;

		if ((flags & XFPGA_ENCRYPTION_USERKEY_EN)
				|| (flags & XFPGA_ENCRYPTION_DEVKEY_EN))
			Status = XFpga_DecrptPlChunks(&PlAesInfo,
						OcmAddr, ChunkSize);
		else
			Status = XFpga_WriteToPcap(ChunkSize/WORD_LEN, OcmAddr);

		if (Status != XFPGA_SUCCESS){
			Status = XFPGA_FAILURE;
			goto END;
		}

		WrAddr = WrAddr + ChunkSize;
	}
	if (RemaningBytes) {
		Status = XFpga_CopyToOcm((UINTPTR)WrAddr,
					(UINTPTR)OcmAddr, RemaningBytes/WORD_LEN);
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
				|| (flags & XFPGA_ENCRYPTION_DEVKEY_EN))
			Status = XFpga_DecrptPlChunks(&PlAesInfo,
						OcmAddr, RemaningBytes);
		else
			Status = XFpga_WriteToPcap(RemaningBytes/WORD_LEN,
								OcmAddr);

		if (Status != XFPGA_SUCCESS){
			Status = XFPGA_FAILURE;
			goto END;
		}
	}

END:
	XSecure_Sha3Finish(&Secure_Sha3, Sha3Hash);
	return Status;
}

/*****************************************************************************/
/* This is the function to write Encrypted data into PCAP interface
 *
 * @param WrSize Number of bytes that the DMA should write to the
 * PCAP interface
 *
 * @param WrAddr Linear memory secure image base address.
 *
 * @return error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_WriteEncryptToPcap (UINTPTR WrAddr, UINTPTR KeyAddr,
				XSecure_ImageInfo *ImageHdrInfo, u32 flags) {
	u32 Status = XFPGA_SUCCESS;
	u8 *EncSrc;

	if (flags & XFPGA_ENCRYPTION_USERKEY_EN) {
		Xilfpga_ConvertStringToHex((char *)(UINTPTR)(KeyAddr),
							key, KEY_LEN);
		/* Xilsecure expects Key in big endian form */
		for (u8 i = 0; i < ARRAY_LENGTH(key); i++)
			key[i] = Xil_Htonl(key[i]);
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

	EncSrc = (u8 *)(UINTPTR)(WrAddr +
			(ImageHdrInfo->PartitionHdr->DataWordOffset) *
						XSECURE_WORD_LEN);
	Status = XSecure_AesDecrypt(&Secure_Aes,
			(u8 *) XFPGA_DESTINATION_PCAP_ADDR, EncSrc,
			ImageHdrInfo->PartitionHdr->UnEncryptedDataWordLength *
							XSECURE_WORD_LEN);

	Status = XFpga_PcapWaitForDone();

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
static u32 XFpga_AesInit(UINTPTR KeyAddr, u32 *AesIv, u32 flags) {
	u32 Status = XFPGA_SUCCESS;

	PlAesInfo.PlEncrypt.SecureAes = &Secure_Aes;
	PlAesInfo.PlEncrypt.NextBlkLen = 0;
	PlAesInfo.Hdr = 0;
	memset(PlAesInfo.SecureHdr, 0,
			XSECURE_SECURE_HDR_SIZE + XSECURE_SECURE_GCM_TAG_SIZE);

	if (flags & XFPGA_ENCRYPTION_USERKEY_EN) {
		Xilfpga_ConvertStringToHex((char *)(UINTPTR)(KeyAddr),
							key, KEY_LEN);
		/* Xilsecure expects Key in big endian form */
		for (u8 i = 0; i < ARRAY_LENGTH(key); i++)
			key[i] = Xil_Htonl(key[i]);
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
 * 	- XFPGA_SUCCESS on success
 * 	- XFPGA_FAILURE on failure
 *
 * @note None.
 *
 *****************************************************************************/
static u32 XFpga_CopyToOcm(UINTPTR Src, UINTPTR Dst, u32 WrSize) {
	u32 Status = XFPGA_SUCCESS;

	/*
	 * Setup the  SSS, To copy the contains from DDR to OCM
	 */
	Xil_Out32(CSU_CSU_SSS_CFG, XFPGA_CSU_SSS_DMA_TO_DMA);


	/* Data transfer to OCM */
	XCsuDma_Transfer(&CsuDma, XCSUDMA_DST_CHANNEL, Dst, WrSize, 0);
	XCsuDma_Transfer(&CsuDma, XCSUDMA_SRC_CHANNEL, Src, WrSize, 0);

	/* Polling for transfer to be done */
	XCsuDma_WaitForDone(&CsuDma, XCSUDMA_DST_CHANNEL);

	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);

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
* 	Error code on failure
* 	XFPGA_SUCESS on success
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
			xil_printf("Fail to decrypt the Secure Header\r\n");
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
			xil_printf("Fail to decrypt the NextBlock\r\n");
			goto END;
		}

		Status = XFpga_DecrptPl(PartitionParams,
					(UINTPTR)SrcAddr, Size);
		if (Status != XFPGA_SUCCESS) {
			xil_printf("Fail to decrypt the XFpga_DecrptPl\r\n");
			goto END;
		}
		/*
		 * If status is true or false also goto END
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
			xil_printf("Fail to decrypt the XFpga_DecrypSecureHdr\r\n");
			return Status;
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
			xil_printf("Fail to decrypt the XFpga_DecrptSetUpNextBlk\r\n");
			return Status;
		}

		if ((NextBlkAddr != 0x00U) &&
			(PartitionParams->PlEncrypt.SecureAes->SizeofData != 0)) {
			Status = XFpga_DecrptPl(PartitionParams,
					(UINTPTR)NextBlkAddr, Size);
			if (Status != XFPGA_SUCCESS) {
				xil_printf("Fail to decrypt the XFpga_DecrptPl1\r\n");
				return Status;
			}
		}
	}
	else {
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
* 	Error code on failure
* 	XFPGA_SUCESS on success
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
* 	Error code on failure
* 	XFPGA_SUCESS on success
*
* @note None
*
******************************************************************************/
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
		else if (PartitionParams->PlEncrypt.SecureAes->SizeofData != 0) {
			/* First transfer whole data other than secure header */
			XFpga_DmaPlCopy(
				PartitionParams->PlEncrypt.SecureAes->CsuDmaPtr,
				(UINTPTR)SrcAddr,
			PartitionParams->PlEncrypt.SecureAes->SizeofData/WORD_LEN, 0);
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
				xil_printf("Secure header size remaining %d\n\r", Size);
				memcpy(PartitionParams->SecureHdr,
						(u8 *)(UINTPTR)SrcAddr, Size);
				PartitionParams->Hdr = Size;
				Size = 0;
			}
		}

		/* Wait PCAP done */
		Status = XFpga_PcapWaitForDone();
		if (Status != XFPGA_SUCCESS) {
			return Status;
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
				return Status;
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
				return Status;
			}
			if ((NextBlkAddr != 0x00U) &&
			(PartitionParams->PlEncrypt.SecureAes->SizeofData != 0)) {
				SrcAddr = NextBlkAddr;
			}
			else {
				break;
			}


		}

	} while (Size != 0x00);

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
* 	Error code on failure
* 	XFPGA_SUCESS on success
*
* @note	None
*
******************************************************************************/
static u32 XFpga_DecrypSecureHdr(XSecure_Aes *InstancePtr, u64 SrcAddr)
{
	XCsuDma_Configure ConfigurValues = {0};
	u32 GcmStatus;

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
		xil_printf("GCM TAG NOT Matched\r\n");
		return XFPGA_FAILURE;
	}

	return XFPGA_SUCCESS;
}

#endif

/****************************************************************************/
 /* This function waits for PL Done bit to be set or till timeout and resets
 * PCAP after this.
 *
 * @param	None
 *
 * @return	error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_PLWaitForDone(void) {
	u32 Status = XFPGA_SUCCESS;
	u32 PollCount;
	u32 RegVal = 0U;

	PollCount = (PL_DONE_POLL_COUNT);
	while (PollCount) {
		/* Read PCAP Status register and check for PL_DONE bit */
		RegVal = Xil_In32(CSU_PCAP_STATUS);
		RegVal &= CSU_PCAP_STATUS_PL_DONE_MASK;
		if (RegVal == CSU_PCAP_STATUS_PL_DONE_MASK) {
			break;
		}
		PollCount--;
	}

	if (RegVal != CSU_PCAP_STATUS_PL_DONE_MASK) {
		Status = XFPGA_ERROR_BITSTREAM_LOAD_FAIL;
		goto END;
	}

	/* Reset PCAP after data transfer */
	RegVal = Xil_In32(CSU_PCAP_RESET);
	RegVal = RegVal | CSU_PCAP_RESET_RESET_MASK;
	Xil_Out32(CSU_PCAP_RESET, RegVal);

	PollCount = (PL_DONE_POLL_COUNT);
	RegVal = 0U;
	while(PollCount) {
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
 * @return	error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_CsuDmaInit()
{
	u32 Status = XFPGA_SUCCESS;
	XCsuDma_Config * CsuDmaConfig;

	CsuDmaConfig = XCsuDma_LookupConfig(0);
	if (NULL == CsuDmaConfig) {
		Status = XFPGA_ERROR_CSUDMA_INIT_FAIL;
		goto END;
	}

	Status = XCsuDma_CfgInitialize(&CsuDma, CsuDmaConfig,
			CsuDmaConfig->BaseAddress);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_ERROR_CSUDMA_INIT_FAIL;
		goto END;
	}
END:
	return Status;
}
/*****************************************************************************/
/*
 * This function is used to power-up the PL
 *
 * @param	None
 *
 * @return	error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
static u32 XFpga_PowerUpPl(void) {

	u32 Status = XFPGA_SUCCESS;

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

	if (PollCount == 0)
		Status = XFPGA_ERROR_PL_POWER_UP;
#endif
	return Status;

}
/*
*
* This function is used to request isolation restore, through PMU
*
* @param	Mask of the entries for which isolation is to be restored
*
* @return	XFSBL_SUCCESS (for now always returns this)
*
* @note		None.
*
****************************************************************************/
static u32 XFpga_IsolationRestore()
{
	u32 Status = XFPGA_SUCCESS;

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
		RegVal = Xil_In32(PMU_GLOBAL_ISO_STATUS) & PMU_GLOBAL_PWR_PL_MASK;
		PollCount--;
	} while ((RegVal != 0) && PollCount);

	if (PollCount == 0)
		Status = XFPGA_ERROR_PL_ISOLATION;
#endif
	return Status;
}
/*
*
* This function is used to reset the PL from PS EMIO pins
*
* @param	TotalResets
*
* @return	XFSBL_SUCCESS (for now always returns this)
*
* @note		None.
*
****************************************************************************/
static u32 XFpga_PsPlGpioReset(u32 TotalResets) {
	u32 Status = XFPGA_SUCCESS;
	u32 RegVal = 0;
	u32 MaskVal;

	/* Set EMIO Direction */
	RegVal = Xil_In32(GPIO_DIRM_5_EMIO) |
		~(~0U << TotalResets) << (MAX_REG_BITS + 1 - TotalResets);
	Xil_Out32(GPIO_DIRM_5_EMIO, RegVal);

	/*Assert the EMIO with the required Mask */
	MaskVal = ~(~0U << TotalResets) << (MAX_REG_BITS/2 + 1 - TotalResets) | 0xFFFF0000;
	RegVal = MaskVal & ~(~(~0U << TotalResets) << (MAX_REG_BITS + 1 - TotalResets));
	Xil_Out32(GPIO_MASK_DATA_5_MSW,RegVal);
	usleep(1000);

	/*De-assert the EMIO with the required Mask */
	RegVal = ~(~(~0U << TotalResets) << (MAX_REG_BITS + 1 - TotalResets)) & 0xFFFF0000;
	Xil_Out32(GPIO_MASK_DATA_5_MSW, RegVal);
	usleep(1000);

	/*Assert the EMIO with the required Mask */
	MaskVal = ~(~0U << TotalResets) << (MAX_REG_BITS/2 + 1 - TotalResets) | 0xFFFF0000;
	RegVal = MaskVal & ~(~(~0U << TotalResets) << (MAX_REG_BITS + 1 - TotalResets));
	Xil_Out32(GPIO_MASK_DATA_5_MSW,RegVal);
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
u32 XFpga_PcapStatus() {

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
 * 		- XST_SUCCESS no errors occured.
 *		- ERROR when input parameters are not valid
 *
 * @note	None.
 *
 *****************************************************************************/
static u32 Xilfpga_ConvertCharToNibble(char InChar, u8 *Num) {
	/* Convert the char to nibble */
	if ((InChar >= '0') && (InChar <= '9'))
		*Num = InChar - '0';
	else if ((InChar >= 'a') && (InChar <= 'f'))
		*Num = InChar - 'a' + 10;
	else if ((InChar >= 'A') && (InChar <= 'F'))
		*Num = InChar - 'A' + 10;
	else
		return XFPGA_STRING_INVALID_ERROR;

	return XFPGA_SUCCESS;
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
static u32 Xilfpga_ConvertStringToHex(const char * Str, u32 * buf, u8 Len)
{
	u32 Status = XFPGA_SUCCESS;
	u8 ConvertedLen = 0,index=0;
	u8 Nibble[MAX_NIBBLES];

	while (ConvertedLen < Len) {
		/* Convert char to nibble */
		for (u8 i = 0; i < ARRAY_LENGTH(Nibble); i++) {
			Status = Xilfpga_ConvertCharToNibble(
					Str[ConvertedLen++],&Nibble[i]);

			if (Status != XFPGA_SUCCESS)
				/* Error converting char to nibble */
				return XFPGA_STRING_INVALID_ERROR;

		}

		buf[index++] = Nibble[0] << 28 | Nibble[1] << 24 |
				Nibble[2] << 20 | Nibble[3] << 16 |
				Nibble[4] << 12 | Nibble[5] << 8 |
				Nibble[6] << 4 | Nibble[7];
	}
	return XFPGA_SUCCESS;
}

#endif

/*****************************************************************************/
/**
* @ingroup xfpga_apis
* Returns the value of the specified configuration register.
*
* @param        InstancePtr is a pointer to the XHwIcap instance.
* @param        ConfigReg  is a constant which represents the configuration
*                       register value to be returned.
* @param        RegData is the value of the specified configuration
*                       register.
*
* @return
*               - XST_SUCCESS if successful
*               - XST_FAILURE if unsuccessful
*
*
****************************************************************************/
u32 Xfpga_GetConfigReg(u32 ConfigReg, u32 *RegData)
{
	u32 Status;
	u32 RegVal;
	unsigned int CmdIndex;
	unsigned int CmdBuf[18];

	/* Initialize the CSU DMA */
	Status = XFpga_CsuDmaInit();
	if (Status != XFPGA_SUCCESS)
		return Status;
	/*
	 * Register Readback in non secure mode
	 * Create the data to be written to read back the
	 * Configuration Registers from PL Region.
	 */

	CmdIndex = 0;
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0x000000BB; 	/* Bus Width Sync Word */
	CmdBuf[CmdIndex++] = 0x11220044; 	/* Bus Width Detect */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xAA995566; 	/* Sync Word */
	CmdBuf[CmdIndex++] = 0x20000000; 	/* Type 1 NOOP Word 0 */
	CmdBuf[CmdIndex++] = Xfpga_RegAddr(ConfigReg,OPCODE_READ,0x1);
	CmdBuf[CmdIndex++] = 0x20000000; 	/* Type 1 NOOP Word 0 */
	CmdBuf[CmdIndex++] = 0x20000000; 	/* Type 1 NOOP Word 0 */

	/* Take PCAP out of Reset */
	RegVal = Xil_In32(CSU_PCAP_RESET);
	RegVal &= (~CSU_PCAP_RESET_RESET_MASK);
	Xil_Out32(CSU_PCAP_RESET, RegVal);

	/*
	 * Setup the  SSS, setup the PCAP to receive from DMA source
	 */
	Xil_Out32(CSU_CSU_SSS_CFG, XFPGA_CSU_SSS_SRC_SRC_DMA);
	Xil_Out32(CSU_PCAP_RDWR, 0x0);

	/* Set up the Destination DMA Channel*/
	XCsuDma_Transfer(&CsuDma, XCSUDMA_DST_CHANNEL, (UINTPTR)RegData, 1, 0);

	/* Setup the source DMA channel */
	XCsuDma_Transfer(&CsuDma, XCSUDMA_SRC_CHANNEL, (UINTPTR)CmdBuf,
								CmdIndex, 0);

	/* wait for the SRC_DMA to complete and the pcap to be IDLE */
	XCsuDma_WaitForDone(&CsuDma, XCSUDMA_SRC_CHANNEL);

	/*
	 * Setup the  SSS, setup the DMA to receive from PCAP source
	 */
	Xil_Out32(CSU_CSU_SSS_CFG, XFPGA_CSU_SSS_SRC_DST_DMA);
	Xil_Out32(CSU_PCAP_RDWR, 0x1);

	/* wait for the DST_DMA to complete and the pcap to be IDLE */
	XCsuDma_WaitForDone(&CsuDma, XCSUDMA_DST_CHANNEL);

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	CmdIndex = 0;
	CmdBuf[CmdIndex++] = 0x30008001;        /* Dummy Word */
	CmdBuf[CmdIndex++] = 0x0000000D;        /* Bus Width Sync Word */
	CmdBuf[CmdIndex++] = 0x20000000;        /* Bus Width Detect */
	CmdBuf[CmdIndex++] = 0x20000000;        /* Dummy Word */
	CmdBuf[CmdIndex++] = 0x20000000;        /* Bus Width Detect */
	CmdBuf[CmdIndex++] = 0x20000000;        /* Dummy Word */

	/*
	 * Setup the  SSS, setup the PCAP to receive from DMA source
	 */
	Xil_Out32(CSU_CSU_SSS_CFG, XFPGA_CSU_SSS_SRC_SRC_DMA);
	Xil_Out32(CSU_PCAP_RDWR, 0x0);

	/* Setup the source DMA channel */
	XCsuDma_Transfer(&CsuDma, XCSUDMA_SRC_CHANNEL, (UINTPTR)CmdBuf, CmdIndex, 0);

	/* wait for the SRC_DMA to complete and the pcap to be IDLE */
	XCsuDma_WaitForDone(&CsuDma, XCSUDMA_SRC_CHANNEL);
	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	return XST_SUCCESS;
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
static u32 Xfpga_RegAddr(u8 Register, u8 OpCode, u8 Size)
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
        return ( ((XDC_TYPE_1 << XDC_TYPE_SHIFT) |
		(Register << XDC_REGISTER_SHIFT) |
		(OpCode << XDC_OP_SHIFT)) | Size);
}
