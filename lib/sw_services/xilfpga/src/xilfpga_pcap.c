/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
#include "xsecure_aes.h"
#include "xsecure_rsa.h"
#include "xsecure_sha2.h"
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
#ifdef XFPGA_SECURE_MODE
#define KEY_LEN		64	/* Bytes */
#define IV_LEN		24 	/* Bytes */
#define GCM_TAG_LEN	128 	/* Bytes */
#define WORD_LEN	4	/* Bytes */
#define MAX_NIBBLES	8
#define PUBLIC_KEY_LEN	512 /* Bytes */
#define SIGNATURE_LEN	512 /* Bytes */
#define RSA_HASH_LEN	32 /* Bytes */
#define SHA2_HASH_LEN	32 /* BYtes */
#define MOD_LEN			512 /* Bytes */
#define MODEXT_LEN		512 /* Bytes */
#define OCM_PL_ADDR		XFPGA_OCM_ADDRESS
#endif

#define XFSBL_DESTINATION_PCAP_ADDR		(0XFFFFFFFFU)
#define XFPGA_ENCRYPTION_EN				(0x00000008U)
#define XFPGA_AUTHENTICATION_EN			(0x00000004U)
#define XFPGA_PARTIAL_EN				(0x00000001U)

/**************************** Type Definitions *******************************/
#ifdef __MICROBLAZE__
typedef u32 (*XpbrServHndlr_t) (void);
#endif
/***************** Macros (Inline Functions) Definitions *********************/
#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))
# define PL_CHUNK_SIZE_BYTES (1024 * 40)
#define NUM_OF_PL_CHUNKS(Size) (Size / PL_CHUNK_SIZE_BYTES)

/************************** Function Prototypes ******************************/
static u32 XFpga_PcapWaitForDone();
static u32 XFpga_WriteToPcap(u32 WrSize, u32 WrAddrHigh, u32 WrAddrLow);
static u32 XFpga_PcapInit(u32 flags);
static u32 XFpga_CsuDmaInit();
static u32 XFpga_PLWaitForDone(void);
static u32 XFpga_PowerUpPl(void);
static u32 XFpga_IsolationRestore(void);
static u32 XFpga_PsPlGpioReset(u32 TotalResets);
#ifdef XFPGA_SECURE_MODE
static u32 XFpga_WriteEncryptToPcap(u32 WrSize, u32 WrAddrHigh, u32 WrAddrLow);
static u32 XFpga_WriteAuthToPcap(u32 WrSize, u32 WrAddrHigh, u32 WrAddrLow);
static u32 Xilfpga_ConvertCharToNibble(char InChar, u8 *Num);
static u32 Xilfpga_ConvertStringToHex(const char * Str, u32 * buf, u8 Len);
static u32 XFpga_XSecureRsaHashGn(u8 *mod, u8 *exp, u8 *sig, u8 *RsaHash);
static u32 XFpga_WriteChunksToPcap(u32 WrSize, u32 WrAddrHigh, u32 WrAddrLow);
static u32 XFpga_PlChunksSha2HashGe(u32 WrSize, u32 WrAddrHigh, u32 WrAddrLow, u8 *sha2);
static u32 XFpga_CopyDataToOcm(u8 *DstPtr, u8 *SrcPtr, u32 Size);
static u32 XFpga_AdmaCopy(void * DestPtr, void * SrcPtr, u64 Size);
#endif
#ifdef __MICROBLAZE__
extern const XpbrServHndlr_t XpbrServHndlrTbl[XPBR_SERV_EXT_TBL_MAX];
#endif
/************************** Variable Definitions *****************************/
XCsuDma CsuDma;
#ifdef XFPGA_SECURE_MODE
XSecure_Rsa Secure_Rsa;
XSecure_Aes Secure_Aes;
u32 iv[3];
u32 key[8];
#endif

/*****************************************************************************/

/*****************************************************************************/
/** This function does the calls the necessary PCAP interfaces based on flags.
 *
 *@param	WrAddrHigh: Higher 32-bit Linear memory space from where CSUDMA
 *		will read the data to be written to PCAP interface
 *
 *@param        WrAddrLow: Lower 32-bit Linear memory space from where CSUDMA
 *              will read the data to be written to PCAP interface
 *
 *@param        WrSize: Number of 32bit words that the DMA should write to
 *              the PCAP interface
 *
 *@param	flags:
 *		BIT(0) - Bit-stream type.
 *			 0 - Full Bit-stream.
 *			 1 - Partial Bit-stream.
 *		BIT(1) - Authentication.
 *			 1 - Enable.
 *		 	 0 - Disable.
 *		BIT(2) - Encryption.
 *			 1 - Enable.
 *			 0 - Disable.
 * NOTE -
 *	The current implementation supports only Full Bit-stream.
 *
 *@return	error status based on implemented functionality (SUCCESS by default)
 *
 *****************************************************************************/
u32 XFpga_PL_BitSream_Load (u32 WrAddrHigh, u32 WrAddrLow,
				u32 WrSize, u32 flags )
{
	u32 Status = XFPGA_SUCCESS;
	u32 RegVal;

	/* Enable the PCAP clk */
	RegVal = Xil_In32(PCAP_CLK_CTRL);
	Xil_Out32(PCAP_CLK_CTRL, RegVal | PCAP_CLK_EN_MASK );

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

	Status = XFpga_PcapInit(flags);
	if(Status != XFPGA_SUCCESS) {
		xil_printf("FPGA Init fail\n");
		goto END;
	}

	if ((flags & XFPGA_ENCRYPTION_EN ) || (flags & XFPGA_ENCRYPTION_EN))
#ifdef XFPGA_SECURE_MODE
	{
		if (flags & XFPGA_ENCRYPTION_EN)
			Status = XFpga_WriteEncryptToPcap(WrSize * WORD_LEN,
							WrAddrHigh, WrAddrLow);
			else if (flags & XFPGA_AUTHENTICATION_EN)
				Status = XFpga_WriteAuthToPcap(WrSize * WORD_LEN,
							WrAddrHigh, WrAddrLow);
	}
#else
	{
		xil_printf("Fail to load: Enable secure mode and try...\r\n");
		Status = XFPGA_ERROR_BITSTREAM_LOAD_FAIL;
		goto END;
	}
#endif
	else
		Status = XFpga_WriteToPcap(WrSize, WrAddrHigh, WrAddrLow);

	if(Status != XFPGA_SUCCESS) {
		xil_printf("FPGA fail to write Bit-stream into PL\n");
		goto END;
	}
	Status = XFpga_PLWaitForDone();
	if(Status != XFPGA_SUCCESS) {
		xil_printf("FPGA fail to get the done status\n");
		goto END;
	}

	/* Power-Up PL */
	Status = XFpga_PowerUpPl();
	if (Status != XFPGA_SUCCESS) {
		xil_printf("XFPGA_ERROR_PL_POWER_UP\r\n");
		Status = XFPGA_ERROR_PL_POWER_UP;
		goto END;
	}

	/* PS-PL reset */
	XFpga_PsPlGpioReset(FPGA_NUM_FABRIC_RESETS);

	END:

	/* Disable the PCAP clk */
	RegVal = Xil_In32(PCAP_CLK_CTRL);
	Xil_Out32(PCAP_CLK_CTRL, RegVal & ~(PCAP_CLK_EN_MASK) );

	return Status;
}

/*****************************************************************************/
/** This function does the necessary initialization of PCAP interface
 *
 * @param	None
 *
 * @return	error status based on implemented functionality (SUCCESS by default)
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
	Status = XFpga_CsuDmaInit();

	return Status;
}
/*****************************************************************************/
/** This function waits for PCAP transfer to complete
 *
 * @param	None
 *
 * @return	error status based on implemented functionality (SUCCESS by default)
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
	return Status;
}

/*****************************************************************************/
/** This is the function to write data to PCAP interface
 *
 * @param	WrSize: Number of 32bit words that the DMA should write to
 *         	the PCAP interface
 *
 * @param       WrAddrHigh: Higher 32-bit Linear memory space from where CSUDMA
 *              will read the data to be written to PCAP interfacae
 *
 * @param       WrAddrLow: Lower 32-bit Linear memory space from where CSUDMA
 *              will read the data to be written to PCAP interface
 *
 * @return	None
 *
 *****************************************************************************/
static u32 XFpga_WriteToPcap(u32 WrSize, u32 WrAddrHigh, u32 WrAddrLow) {
	u32 Status = XFPGA_SUCCESS;
	u64 WrAddr;

	WrAddr = ((u64)WrAddrHigh << 32)|WrAddrLow;

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

#ifdef XFPGA_SECURE_MODE
/*****************************************************************************/
/** This is the function to write Encrypted data into PCAP interface
 *
 * @param	WrSize: Number of bytes that the DMA should write to the
 * 			PCAP interface
 *
 * @param       WrAddrHigh: Higher 32-bit Linear memory space from where CSUDMA
 *              will read the data to be written to PCAP interfacae
 *
 * @param       WrAddrLow: Lower 32-bit Linear memory space from where CSUDMA
 *              will read the data to be written to PCAP interface
 *
 * @return	None
 *
 *****************************************************************************/
static u32 XFpga_WriteEncryptToPcap(u32 WrSize, u32 WrAddrHigh, u32 WrAddrLow) {
	u32 Status = XFPGA_SUCCESS;
	u64 WrAddr;

	WrAddr = ((u64)WrAddrHigh << 32)|WrAddrLow;

	Xilfpga_ConvertStringToHex((char *)(UINTPTR)WrAddr + WrSize,
							key, KEY_LEN);
	Xilfpga_ConvertStringToHex((char *)(UINTPTR)WrAddr  + KEY_LEN + WrSize,
								iv, IV_LEN);

	/* Xilsecure expects Key in big endian form */
	for (u8 i = 0; i < ARRAY_LENGTH(key); i++)
		key[i] = Xil_Htonl(key[i]);
	for (u8 i = 0; i < ARRAY_LENGTH(iv); i++)
		iv[i] = Xil_Htonl(iv[i]);

	/* Initialize the Aes driver so that it's ready to use */
	XSecure_AesInitialize(&Secure_Aes, &CsuDma, XSECURE_CSU_AES_KEY_SRC_KUP,
			                           (u32 *)iv, (u32 *)key);
	Status = XSecure_AesDecrypt(&Secure_Aes,
				(u8 *) XFSBL_DESTINATION_PCAP_ADDR,
				(u8 *)(UINTPTR)WrAddr, WrSize - GCM_TAG_LEN);

	Status = XFpga_PcapWaitForDone();

	return Status;
}

/*****************************************************************************/
/** This is the function to write data into PCAP interface.If the data
 *  authenticated properly.
 *
 * @param	WrSize: Number of bytes that the DMA should write to the
 * 			PCAP interface
 *
 * @param       WrAddrHigh: Higher 32-bit Linear memory space from where CSUDMA
 *              will read the data to be written to PCAP interfacae
 *
 * @param       WrAddrLow: Lower 32-bit Linear memory space from where CSUDMA
 *              will read the data to be written to PCAP interface
 *
 * @return	error status based on implemented functionality
 * 		(SUCCESS by default).
 *
 *****************************************************************************/
static u32 XFpga_WriteAuthToPcap(u32 WrSize, u32 WrAddrHigh, u32 WrAddrLow) {
	u32 Status = XFPGA_SUCCESS;
	u32 RegVal;
	u8 Sha2[32];
	u64 WrAddr;
	u8 RsaHash[32];
	u8 *SigAddr;
	u8 *PubKeyAddr;

	WrAddr = ((u64)WrAddrHigh << 32)|WrAddrLow;
	SigAddr = (u8 *)(UINTPTR)(WrAddr +  WrSize);
	PubKeyAddr = (u8 *)(UINTPTR)(WrAddr + WrSize + SIGNATURE_LEN);

	/* Enable the ADMA clk */
	RegVal = Xil_In32(ADMA_CLK_CTRL);
	Xil_Out32(ADMA_CLK_CTRL, RegVal | ADMA_CLK_EN_MASK );

	/* Calculate Hash on the given signature */
	Status = XFpga_XSecureRsaHashGn(PubKeyAddr,
					PubKeyAddr + MOD_LEN,
					SigAddr, RsaHash);

	if(Status != XST_SUCCESS) {
		xil_printf("RSA Decryption Failed\r\n");
		goto END;
	}

	/* Generating SHA2 hash and copy into the OCM */
	Status = XFpga_PlChunksSha2HashGe(WrSize, WrAddrHigh, WrAddrLow, Sha2);
	if (Status != XFPGA_SUCCESS) {
		goto END;
	}

	/* Compare Sha2 Hash with RSA Hash */
	if (!memcmp(Sha2, RsaHash, ARRAY_LENGTH(RsaHash)))
		Status = XFpga_WriteChunksToPcap(WrSize, WrAddrHigh, WrAddrLow);

END:
	Xil_Out32(ADMA_CLK_CTRL, RegVal);

	return Status;
}

/****************************************************************************/
/**
*
* This function generates RSA hash on data provided by using XilSecure library
*
* @return
*		- XST_FAILURE if the authentication failed.
*
* @note		None.
*
****************************************************************************/
static u32 XFpga_XSecureRsaHashGn(u8 *mod, u8 *exp, u8 *sig, u8 *RsaHash)
{
	u8 RsaSha3Array[SIGNATURE_LEN];
	u32 Status = XST_SUCCESS;

	/*
	 * Initialize the Rsa driver so that it's ready to use
	 * Look up the configuration in the config table and then initialize it.
	 */

	XSecure_RsaInitialize(&Secure_Rsa, mod, NULL, exp);

	/*
	 * Decrypt Boot Image Signature.
	 */

	Status = XSecure_RsaDecrypt(&Secure_Rsa, (u8 *)sig, RsaSha3Array);
	if (Status != XST_SUCCESS)
		goto END;

	memcpy(RsaHash, RsaSha3Array +
		   ARRAY_LENGTH(RsaSha3Array) - RSA_HASH_LEN,
		   RSA_HASH_LEN);

END:
	return Status;
}

/*****************************************************************************/
/** This is the function is used to write data into the PCAP interface
 *  in chunks wise through OCM.
 *
 * @param	WrSize: Number of 32bit words that the DMA should write to
 *         	the PCAP interface
 *
 * @param       WrAddrHigh: Higher 32-bit Linear memory space from where CSUDMA
 *              will read the data to be written to PCAP interface
 *
 * @param       WrAddrLow: Lower 32-bit Linear memory space from where CSUDMA
 *              will read the data to be written to PCAP interface
 *
 * @return	error status based on implemented functionality
 * 		(SUCCESS by default).
 *
 *****************************************************************************/
static u32 XFpga_WriteChunksToPcap(u32 WrSize, u32 WrAddrHigh, u32 WrAddrLow) {
	u32 Status = XFPGA_SUCCESS;
	u64 OcmAddr = OCM_PL_ADDR;
	u32 NumChunks = NUM_OF_PL_CHUNKS(WrSize);
	u32 ChunkSize = PL_CHUNK_SIZE_BYTES;
	u32 OcmChunkAddr = OCM_PL_ADDR + ChunkSize;
	u32 RemaningBytes;
	u32 Count;
	u64 WrAddr;
	sha2_context Sha2;
	u8 Sha2Hash[SHA2_HASH_LEN];

	WrAddr = ((u64)WrAddrHigh << 32)|WrAddrLow;
	RemaningBytes = (WrSize  - (ChunkSize * NumChunks));
	if (!RemaningBytes) {
		RemaningBytes = ChunkSize;
		NumChunks--;
	}

	sha2_starts(&Sha2);

	for(Count = 0; Count < NumChunks; Count++) {
		Status = XFpga_CopyDataToOcm((u8 *)(UINTPTR)OcmAddr,
					(u8 *)(UINTPTR)WrAddr, ChunkSize);
		if (Status != XFPGA_SUCCESS) {
					goto END;
		}

		/* Generating SHA2 hash */
		sha2_update(&Sha2, (u8 *)(UINTPTR)OcmAddr, ChunkSize);
		sha2_hash(&Sha2, Sha2Hash);

		/* Compare SHA2 hash with OCM Stored hash*/
		if (memcmp((u32 *)(UINTPTR)OcmChunkAddr, Sha2Hash, SHA2_HASH_LEN)) {
			Status = XFPGA_FAILURE;
			goto END;
		} else
			OcmChunkAddr = OcmChunkAddr + SHA2_HASH_LEN;

		Status = XFpga_WriteToPcap(ChunkSize/WORD_LEN,
					(u32)(OcmAddr >> 32), (u32)OcmAddr);
		if (Status != XFPGA_SUCCESS)
			goto END;

		WrAddr = WrAddr + ChunkSize;
		xil_printf(".");
	}

	if (RemaningBytes) {
		Status = XFpga_CopyDataToOcm((u8 *)(UINTPTR)OcmAddr,
					(u8 *)(UINTPTR)WrAddr, RemaningBytes);
		if (Status != XFPGA_SUCCESS)
			goto END;

		/* Generating SHA2 hash */
		sha2_update(&Sha2, (u8 *)(UINTPTR)OcmAddr, RemaningBytes);
		sha2_finish(&Sha2, Sha2Hash);

		/* Compare SHA2 hash with OCM Stored hash*/
		if (memcmp((u32 *)(UINTPTR)OcmChunkAddr,
			Sha2Hash, SHA2_HASH_LEN)) {
			Status = XFPGA_FAILURE;
			goto END;
		}
		Status = XFpga_WriteToPcap(RemaningBytes/WORD_LEN,
				(u32)(OcmAddr >> 32),(u32)OcmAddr);

	}
	xil_printf("Done\r\n");
END:
	return Status;
}
/*****************************************************************************/
/**
*
* This function generates SHA2 hash on the given Bit-Stream using
* XilSecure library
*
* @return	error status based on implemented functionality
* 		(SUCCESS by default)
*
* @note		None.
*
******************************************************************************/

static u32 XFpga_PlChunksSha2HashGe(u32 WrSize, u32 WrAddrHigh, u32 WrAddrLow, u8 *sha2)
{
	u32 Status = XFPGA_SUCCESS;
	u64 OcmAddr = OCM_PL_ADDR;
	u32 NumChunks = NUM_OF_PL_CHUNKS(WrSize);
	u32 ChunkSize = PL_CHUNK_SIZE_BYTES;
	u32 OcmChunkAddr = OCM_PL_ADDR + ChunkSize;
	u32 RemaningBytes;
	u32 Count;
	u64 WrAddr;
	sha2_context Sha2;
	u8 Sha2Hash[SHA2_HASH_LEN];

	WrAddr = ((u64)WrAddrHigh << 32)|WrAddrLow;
	RemaningBytes = (WrSize - (ChunkSize * NumChunks));
	if (!RemaningBytes) {
		RemaningBytes = ChunkSize;
		NumChunks--;
	}

	xil_printf("Loading...");
	sha2_starts(&Sha2);

	for(Count = 0; Count < NumChunks; Count++) {
		Status = XFpga_CopyDataToOcm((u8 *)(UINTPTR)OcmAddr,
					(u8 *)(UINTPTR)WrAddr, ChunkSize);
		if (Status != XFPGA_SUCCESS)
			return Status;

		/* Generating SHA2 hash */
		sha2_update(&Sha2, (u8 *)(UINTPTR)OcmAddr, ChunkSize);
		sha2_hash(&Sha2, Sha2Hash);

		/* Copy SHA2 hash into the OCM */
		memcpy((u32 *)(UINTPTR)OcmChunkAddr, Sha2Hash, SHA2_HASH_LEN);
		OcmChunkAddr = OcmChunkAddr + SHA2_HASH_LEN;
		WrAddr = WrAddr + ChunkSize;

		xil_printf(".");
	}

	if (RemaningBytes) {
		Status = XFpga_CopyDataToOcm((u8 *)(UINTPTR)OcmAddr,
					(u8 *)(UINTPTR)WrAddr, RemaningBytes);
		if (Status != XFPGA_SUCCESS)
				return Status;

		/* Generating SHA2 hash */
		sha2_update(&Sha2, (u8 *)(UINTPTR)OcmAddr, RemaningBytes);
		sha2_finish(&Sha2, Sha2Hash);

		/* Copy SHA2 hash into the OCM */
		memcpy((u32 *)(UINTPTR)OcmChunkAddr, Sha2Hash, SHA2_HASH_LEN);
		memcpy(sha2, Sha2Hash, SHA2_HASH_LEN);

	}

	return Status;

}

/*****************************************************************************
*
* This function copies the data from DDR/flash to OCM.
* For DDR systems uses ADMA and for DDR-less uses devices DMA copy
*
* @param	PartitionPtr is the pointer to XFsblPs_PlPartition
* @param	DstPtr holds destination address.
* @param	SrcPtr holds source address
* @param	Size of the data to be copied
*
* @return
* 		- Success on successful data transfer
* 		- Error on failure
*
* @note		None
*
******************************************************************************/
static u32 XFpga_CopyDataToOcm(u8 *DstPtr, u8 *SrcPtr, u32 Size)
{
	u32 *Dst = (u32 *)DstPtr;
	u32 *Src = (u32 *)SrcPtr;
	u32 Status;

	Xil_DCacheFlushRange((INTPTR)Src, Size);
	Xil_DCacheInvalidateRange((INTPTR)Dst, Size);
	Status = XFpga_AdmaCopy(Dst, Src, Size);
	if (Status != XFPGA_SUCCESS) {
		xil_printf("Fail to copy Adma\r\n");
		goto END;
	}

END:
	return Status;
}
/******************************************************************************
*
* This function copies data memory to memory using ADMA.
*
* @param	DestPtr is a pointer to destination buffer to which data needs
*		to be copied.
* @param	SrcPtr is a pointer to the source buffer.
* @param	size holds the size of the data to be transfered.
*
* @return
*		Success on successful copy
*		Error on failure.
*
* @note		Cache invalidation and flushing should be taken care by user
*		Before calling this API ADMA also should be configured to
*		simple DMA.
*
******************************************************************************/
static u32 XFpga_AdmaCopy(void * DestPtr, void * SrcPtr, u64 Size)
{
	u32 RegVal;
	u64 SrcAddr = (UINTPTR)SrcPtr;
	u64 DstAddr = (UINTPTR)DestPtr;
	u32 Status = XFPGA_SUCCESS;

	/* Wait until the DMA is in idle state */
	do {
		RegVal = Xil_In32(ADMA_CH0_ZDMA_CH_STATUS);
		RegVal &= ADMA_CH0_ZDMA_CH_STATUS_STATE_MASK;
	} while ((RegVal != ADMA_CH0_ZDMA_CH_STATUS_STATE_DONE) &&
			(RegVal != ADMA_CH0_ZDMA_CH_STATUS_STATE_ERR));

	/* Write source Address */
	Xil_Out32(ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD0,
			(SrcAddr & ADMA_CH0_ZDMA_CH_DST_DSCR_WORD0_LSB_MASK));
	Xil_Out32(ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD1,
		(((u64)SrcAddr >> 32U) &
				ADMA_CH0_ZDMA_CH_DST_DSCR_WORD1_MSB_MASK));

	/* Write Destination Address */
	Xil_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD0,
		(u32)(DstAddr & ADMA_CH0_ZDMA_CH_DST_DSCR_WORD0_LSB_MASK));
	Xil_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD1,
			(u32)((DstAddr >> 32U) &
			ADMA_CH0_ZDMA_CH_DST_DSCR_WORD1_MSB_MASK));

	/* Size to be Transferred. Recommended to set both src and dest sizes */
	Xil_Out32(ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD2, Size);
	Xil_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD2, Size);


	/* coherence enable */
	RegVal = Xil_In32(ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD3);
	Xil_Out32(ADMA_CH0_ZDMA_CH_SRC_DSCR_WORD3, RegVal | 0x1U);

	RegVal = Xil_In32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD3);
	Xil_Out32(ADMA_CH0_ZDMA_CH_DST_DSCR_WORD3, RegVal | 0x1U);

	/* DMA Enable */
	RegVal = Xil_In32(ADMA_CH0_ZDMA_CH_CTRL2);
	RegVal |= ADMA_CH0_ZDMA_CH_CTRL2_EN_MASK;
	Xil_Out32(ADMA_CH0_ZDMA_CH_CTRL2, RegVal);

	/* Check the status of the transfer by polling on DMA Done */
	do {
		RegVal = Xil_In32(ADMA_CH0_ZDMA_CH_ISR);
		RegVal &= ADMA_CH0_ZDMA_CH_ISR_DMA_DONE_MASK;
	} while (RegVal != ADMA_CH0_ZDMA_CH_ISR_DMA_DONE_MASK);

	/* Clear DMA status */
	RegVal = Xil_In32(ADMA_CH0_ZDMA_CH_ISR);
	RegVal |= ADMA_CH0_ZDMA_CH_ISR_DMA_DONE_MASK;
	Xil_Out32(ADMA_CH0_ZDMA_CH_ISR, ADMA_CH0_ZDMA_CH_ISR_DMA_DONE_MASK);

	/* Read the channel status for errors */
	RegVal = Xil_In32(ADMA_CH0_ZDMA_CH_STATUS);
	if (RegVal == ADMA_CH0_ZDMA_CH_STATUS_STATE_ERR) {
		Status = XFPGA_FAILURE;
	}

	return Status;

}

#endif

/**
 * This function waits for PL Done bit to be set or till timeout and resets
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
/**
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
/**
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
/**
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
/**
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
/** This function  provides the STATUS of PCAP interface
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
/**
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
/**
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
