/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xloader_plat_client.c
 *
 * This file contains the implementation of the client interface functions
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  dd   01/09/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

/**
 * @addtogroup xloader_client_apis XilLoader Client APIs
 * @{
 */

/*************************************** Include Files *******************************************/

#include "xloader_plat_client.h"
#include "xloader_defs.h"
#include "xsecure_shaclient.h"
#include "xsecure_katclient.h"

/************************** Constant Definitions *****************************/
#define XLOADER_SMAP_WD_PATTERN_SIZE				(0x00000010U)
				/**< Size of SMAP width pattern */
#define XLOADER_BH_SIZE_WO_PADDING				(0x00000F24U)
				/**< Size of bootheader without padding */
#define XLOADER_BH_SIZE						(0x00000F70U)
				/**< Size of bootheader */
#define XLOADER_IHT_SIZE 					(128U)
				/**< Size of Image Header Table */
#define XLOADER_BH_IMG_ATTRB_SIGNED_IMG_MASK			(0xC0000U)
				/**< Mask for signed image in bootheader */

#define XLOADER_RSA_4096_KEY_SIZE	(4096U/8U) /**< RSA 4096 key size */

#define XLOADER_SPK_SIZE		(XLOADER_RSA_4096_KEY_SIZE + \
					XLOADER_RSA_4096_KEY_SIZE \
					+ 4U +4U)
	/**< Size of Secondary Public Key(in bytes) in Authentication Certificate */
#define XLOADER_PPK_SIZE		(XLOADER_RSA_4096_KEY_SIZE + \
					XLOADER_RSA_4096_KEY_SIZE \
					+ 4U +12U)
	/**< Size of Primary Public Key(in bytes) in Authentication Certificate */
#define XLOADER_SPK_SIG_SIZE		XLOADER_RSA_4096_KEY_SIZE
	/**< Size of SPK signature(in bytes) in Authentication Certificate */
#define XLOADER_BHDR_SIG_SIZE		XLOADER_RSA_4096_KEY_SIZE
	/**< Size of Bootheader signature(in bytes) in Authentication Certificate */
#define XLOADER_PARTITION_SIG_SIZE	XLOADER_RSA_4096_KEY_SIZE
	/**< Size of Partition signature(in bytes) in Authentication Certificate */

#define XLOADER_AUTH_HEADER_SIZE	(8U)
	/**< Size of Authentication header(in bytes) in Authentication Certificate */

#define XLOADER_AUTH_CERT_USER_DATA	((u32)64U - XLOADER_AUTH_HEADER_SIZE)
		/**< Size of User Data(in bytes) in Authentication Certificate */

#define XLOADER_AUTH_CERT_MIN_SIZE	(XLOADER_AUTH_HEADER_SIZE \
					+ XLOADER_AUTH_CERT_USER_DATA \
					+ XLOADER_PPK_SIZE  \
					+ XLOADER_SPK_SIZE \
					+ XLOADER_SPK_SIG_SIZE \
					+ 8U \
					+ XLOADER_BHDR_SIG_SIZE \
					+ XLOADER_PARTITION_SIG_SIZE)
			/**< Minimum Size of Authentication Certificate(in bytes) */

#define XIH_MAX_PRTNS				(32U) /**< Max number of partitions */
#define XLOADER_SECURE_CHUNK_SIZE		(0x8000U) /**< 32K */
#define XLOADER_SHA3_HASH_LEN_IN_BYTES		(48U)	/**< Length of SHA3 Hash in bytes*/
#define XLOADER_WORD_LEN			(4U)	/**< Length of word in bytes*/
#define XLOADER_WORD_LEN_SHIFT			(2U)	/**< Shift to convert word in bytes */
#define XLOADER_PARTITION_SIZE			(128U) /**< Size of partition in bytes*/

/**************************** Type Definitions *******************************/
/**
 * Structure to store the Boot Header PMC FW fields
 */
typedef struct {
	u32 MetaHdrOfst; /**< Offset to the start of meta header */
	u32 FwRsvd[24U]; /**< FW Reserved fields */
} XilLoader_BootHdrFwRsvd;

/**
 * Structure to store the boot header table details.
 * It contains all the information of boot header table in order.
 */
typedef struct {
	u32 WidthDetection; /**< Width Detection 0xAA995566 */
	u32 ImgIden;  /**< Image Identification */
	u32 EncStatus;  /**< Encryption Status */
	u32 DpiSrcOfst;  /**< Source Offset of PMC FW in DPI */
	u32 DpiStartOfst;  /**< PMC FW start offset in RAM */
	u32 DataPrtnLen;  /**< Data Partition Length */
	u32 TotalDataPrtnLen;  /**< Total Data Partition length */
	u32 PlmLen;  /**< PLM Length */
	u32 TotalPlmLen;  /**< Total PLM length */
	u32 ImgAttrb;  /**< Image Attributes */
	u32 Kek[8U];  /**< Encrypted Key */
	u32 KekIv[3U];  /**< Key Iv */
	u32 SecureHdrIv[3U];  /**< Secure Header IV */
	u32 PufShutterVal; /**< PUF Shutter Value */
	u32 RomRsvd[20U]; /**< ROM Reserved */
	XilLoader_BootHdrFwRsvd BootHdrFwRsvd; /**< FW reserved fields */
} XilLoader_BootHdr __attribute__ ((aligned(16U)));

/**
 * Structure to store the image header table details.
 * It contains all the information of image header table in order.
 */
typedef struct {
	u32 Version; /**< PDI version used  */
	u32 NoOfImgs; /**< No of images present  */
	u32 ImgHdrAddr; /**< Address to start of 1st Image header*/
	u32 NoOfPrtns; /**< No of partitions present  */
	u32 PrtnHdrAddr; /**< Address to start of 1st partition header*/
	u32 SBDAddr; /**< Secondary Boot device address */
	u32 Idcode; /**< Device ID Code */
	u32 Attr; /**< Attributes */
	u32 PdiId; /**< PDI ID */
	u32 Rsrvd[3U]; /**< Reserved for future use */
	u32 TotalHdrLen; /**< Total size of Meta header AC + encryption overload */
	u32 IvMetaHdr[3U]; /**< Iv for decrypting SH of meta header */
	u32 EncKeySrc; /**< Encryption key source for decrypting SH of headers */
	u32 ExtIdCode;  /**< Extended ID Code */
	u32 AcOffset; /**< AC offset of Meta header */
	u32 KekIv[3U]; /**< Kek IV for meta header decryption */
	u32 OptionalDataLen; /**< Len in words of OptionalData */
	u32 Rsvd[8U]; /**< Reserved */
	u32 Checksum; /**< Checksum of the image header table */
} XilLoader_ImgHdrTbl __attribute__ ((aligned(16U)));

typedef struct {
	u32 EncDataWordLen; /**< Enc word length of partition*/
	u32 UnEncDataWordLen; /**< Unencrypted word length */
	u32 TotalDataWordLen; /**< Total word length including the authentication
							certificate if any*/
	u32 NextPrtnOfst; /**< Addr of the next partition header*/
	u64 DstnExecutionAddr; /**< Execution address */
	u64 DstnLoadAddr; /**< Load address in DDR/TCM */
	u32 DataWordOfst; /**< Data word offset */
	u32 PrtnAttrb; /**< Partition attributes */
	u32 SectionCount; /**< Section count */
	u32 ChecksumWordOfst; /**< Address to checksum when enabled */
	u32 PrtnId; /**< Partition ID */
	u32 AuthCertificateOfst; /**< Address to the authentication certificate
							when enabled */
	u32 PrtnIv[3U]; /**< IV of the partition's SH */
	u32 EncStatus; /**< Encryption Status/Key Selection */
	u32 KekIv[3U]; /**< KEK IV for partition decryption */
	u32 EncRevokeID; /**< Revocation ID of partition for encrypted partition */
	u32 Reserved[9U]; /**< Reserved */
	u32 Checksum; /**< checksum of the partition header */
} XilLoader_PrtnHdr __attribute__ ((aligned(16U)));

/************************** Function Prototypes ******************************/
static int XLoader_VerifyPrtnAuth(XLoader_ClientInstance *InstancePtr, u64 PrtnAddr, u32 PrtnLen, u64 ACAddr);
static int XLoader_VerifyDataAuth(XLoader_ClientInstance *InstancePtr, u64 HashAddr, u64 ACAddr, u32 SignatureSelect);
static int XLoader_VerifyPlmNPmcCdoAuth(XLoader_ClientInstance *InstancePtr, u64 PdiAddr, u64 BhAddr);

/************************** Variable Definitions *****************************/
XMailbox MailboxInstance;
XSecure_ClientInstance SecureClientInstance;
u8 NextHash[XLOADER_SHA3_HASH_LEN_IN_BYTES];

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to configure jtag status.
 *
 * @param	InstancePtr Pointer to the client instance.
 * @param	Flag 		To enable / disable jtag.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 **************************************************************************************************/
int XLoader_ConfigureJtagState(XLoader_ClientInstance *InstancePtr, u32 Flag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

    /**
	 * - Performs input parameters validation. Return error code if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = PACK_XLOADER_HEADER(XLOADER_HEADER_LEN_1, XLOADER_CMD_ID_CONFIG_JTAG_STATE);
	Payload[1U] = Flag;

	/**
	 * - Send an IPI request to the PLM by using the XLoader_ConfigureJtagState CDO command
	 * Wait for IPI response from PLM with a timeout.
	 * - If the timeout exceeds then error is returned otherwise it returns the status of the IPI
	 * response.
	 */
	Status = XLoader_ProcessMailbox(InstancePtr, Payload, sizeof(Payload) / sizeof(u32));

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to read DDR crypto performance counters.
 *
 * @param	InstancePtr 	Pointer to the client instance.
 * @param	Id				DDR device id.
 * @param	CryptoCounters	To read DDR crypto counters.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 **************************************************************************************************/
int XLoader_ReadDdrCryptoPerfCounters(XLoader_ClientInstance *InstancePtr, u32 NodeId,
		XLoader_DDRCounters *CryptoCounters)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

    /**
	 * - Performs input parameters validation. Return error code if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = PACK_XLOADER_HEADER(XLOADER_HEADER_LEN_1,
					XLOADER_CMD_ID_READ_DDR_CRYPTO_COUNTERS);
	Payload[1U] = NodeId;

	/**
	 * - Send an IPI request to the PLM by using the XLoader_ReadDdrCryptoPerfCounters CDO command
	 * Wait for IPI response from PLM with a timeout.
	 * - If the timeout exceeds then error is returned otherwise it returns the status of the IPI
	 * response.
	 */
	Status = XLoader_ProcessMailbox(InstancePtr, Payload, sizeof(Payload) / sizeof(u32));
	CryptoCounters->DDRCounter0 =  InstancePtr->Response[1];
	CryptoCounters->DDRCounter1 =  InstancePtr->Response[2];
	CryptoCounters->DDRCounter2 =  InstancePtr->Response[3];
	CryptoCounters->DDRCounter3 =  InstancePtr->Response[4];

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to validate authenticated PDI
 *
 * @param	InstancePtr - Pointer to XLoader_ClientInstance
 * @param	PdiAddr - Address where authenticated PDI is present
 *
 * @return
 *		 - XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XLoader_ValidatePdiAuth(XLoader_ClientInstance *InstancePtr, const u64 PdiAddr)
{
	int Status = XST_FAILURE;
	u64 BhAddr = PdiAddr + XLOADER_SMAP_WD_PATTERN_SIZE;
	u64 BhAcAddr = BhAddr + XLOADER_BH_SIZE;
	u8 Sha3Hash[XLOADER_SHA3_HASH_LEN_IN_BYTES];
	XilLoader_BootHdr* BootHdrPtr = (XilLoader_BootHdr*)(UINTPTR)BhAddr;
	u64 MhAddr = PdiAddr + BootHdrPtr->BootHdrFwRsvd.MetaHdrOfst;
	XilLoader_ImgHdrTbl* IhtPtr = (XilLoader_ImgHdrTbl*)(UINTPTR)MhAddr;
	u64 MhAcAddr = PdiAddr + ((u64)(IhtPtr->AcOffset) << XLOADER_WORD_LEN_SHIFT);
	XilLoader_PrtnHdr PrtnHdr[XIH_MAX_PRTNS];
	u32 TotalLengthOfPrtnHdr = IhtPtr->NoOfPrtns * XLOADER_PARTITION_SIZE;
	u32 IsSignedImg = BootHdrPtr->ImgAttrb & XLOADER_BH_IMG_ATTRB_SIGNED_IMG_MASK;
	u64 HashAddr = (UINTPTR)&Sha3Hash;
	u64 ImgHdrAddr = PdiAddr + (IhtPtr->ImgHdrAddr * XLOADER_WORD_LEN);
	u64 PrtnAddr;
	u64 PrtnAcAddr;
	u32 TotalSize;
	u32 Idx;

	if (IsSignedImg != XLOADER_BH_IMG_ATTRB_SIGNED_IMG_MASK) {
		xil_printf("Bootloader must be authenticated to authenticate rest of the partitions \r\n");
		goto END;
	}

	Status = (int)XMailbox_Initialize(&MailboxInstance, 0U);
	if (Status != XST_SUCCESS) {
		xil_printf("Mailbox initialize failed:%08x \r\n", Status);
		goto END;
	}

	Status = XSecure_ClientInit(&SecureClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

	/*
	 * Run all Known Answer Tests
	*/
	Status =  XSecure_RsaPublicEncKat(&SecureClientInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status =  XSecure_Sha3Kat(&SecureClientInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_EllipticSignVerifyKat(&SecureClientInstance, XSECURE_ECC_PRIME);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/*
	 * Calculate bootheader hash
	*/
	Status = XSecure_Sha3Digest(&SecureClientInstance, BhAddr, HashAddr, XLOADER_BH_SIZE_WO_PADDING);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	/*
	 * Verify bootheader signature
	*/
	Status = XLoader_VerifyDataAuth(InstancePtr, HashAddr, BhAcAddr, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		xil_printf("Verified signature for bootheader \r\n");
	}

	/*
	 * Calculate hash for PLM and PMC CDO and verify signature
	*/
	Status = XLoader_VerifyPlmNPmcCdoAuth(InstancePtr, PdiAddr, BhAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		xil_printf("Verified signature for PLM and PMC CDO \r\n");
	}

	/*
	 * Calculate Image Header Table hash
	*/
	Status = XSecure_Sha3Digest(&SecureClientInstance, MhAddr, HashAddr,
		XLOADER_IHT_SIZE + (IhtPtr->OptionalDataLen * 4));
	if(Status != XST_SUCCESS) {
		goto END;
	}

	/*
	 * Verify Image Header Table signature
	*/
	Status = XLoader_VerifyDataAuth(InstancePtr, HashAddr, MhAcAddr, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		xil_printf("Verified signature for Image Header Table \r\n");
	}

	/*
	 * Calculate hash for metaheader excluding Image Header Table
	 * This hash calculation also includes the authentication certificate excluding the image
	 * signature
	*/
	Status = XSecure_Sha3Initialize();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_Sha3Update(&SecureClientInstance, MhAcAddr,
		(XLOADER_AUTH_CERT_MIN_SIZE - XLOADER_PARTITION_SIG_SIZE));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	TotalSize = IhtPtr->TotalHdrLen * 4U;
	TotalSize -= XLOADER_AUTH_CERT_MIN_SIZE;

	Status = XSecure_Sha3Update(&SecureClientInstance, ImgHdrAddr, TotalSize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_Sha3Finish(&SecureClientInstance, HashAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/*
	 * Verify signature for Metaheader excluding Image Header Table
	*/
	Status = XLoader_VerifyDataAuth(InstancePtr, HashAddr, MhAcAddr, FALSE);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		xil_printf("Verified signature for Metaheader excluding Image Header Table \n\r");
	}

	Status = Xil_SMemCpy((void *)PrtnHdr, TotalLengthOfPrtnHdr,
		(void*)(UINTPTR)(PdiAddr + (IhtPtr->PrtnHdrAddr * XLOADER_WORD_LEN)), TotalLengthOfPrtnHdr,
		TotalLengthOfPrtnHdr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Partition 0 is PLM. Hence Idx starts from 1 for PLM loadable partitions
	 * Verify signature for each partition in case it is authenticated
	 */
	for (Idx = 1U; Idx < IhtPtr->NoOfPrtns; ++Idx) {
		if (PrtnHdr[Idx].AuthCertificateOfst != 0U) {
			PrtnAddr = PdiAddr + ((u64)(PrtnHdr[Idx].DataWordOfst) << XLOADER_WORD_LEN_SHIFT);
			PrtnAcAddr = PdiAddr + ((u64)(PrtnHdr[Idx].AuthCertificateOfst) << XLOADER_WORD_LEN_SHIFT);

			Status = XLoader_VerifyPrtnAuth(InstancePtr, PrtnAddr,
				(PrtnHdr[Idx].TotalDataWordLen << XLOADER_WORD_LEN_SHIFT) - XLOADER_AUTH_CERT_MIN_SIZE,
				PrtnAcAddr);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			xil_printf("Verified signature for partition %d \n\r", Idx);
		}
	}
	xil_printf("Verified signature for all partitions \r\n");

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function calculates hash and verifies signature for
 * 		each partition
 *
 * @param	InstancePtr - Pointer to XLoader_ClientInstance
 * @param	PrtnAddr - Address where partition is present
 * @param	PrtnLen - Length of partition
 * @param	ACAddr - Address where Authentication Certificate of partition
 * 			is present
 *
 * @return
 *		 - XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static int XLoader_VerifyPrtnAuth(XLoader_ClientInstance *InstancePtr, u64 PrtnAddr, u32 PrtnLen, u64 ACAddr)
{
	int Status = XST_FAILURE;
	int Block = 0U;
	u64 HashAddr;
	u8 Sha3Hash[XLOADER_SHA3_HASH_LEN_IN_BYTES];
	HashAddr = (UINTPTR)&Sha3Hash;
	u32 LastChunk = FALSE;
	u32 Size = PrtnLen;
	u32 ChunkLen;

	do {
		if (PrtnLen <= XLOADER_SECURE_CHUNK_SIZE) {
			ChunkLen = Size;
			LastChunk = TRUE;
		}
		else {
			Size = Size - XLOADER_SECURE_CHUNK_SIZE;
			ChunkLen = XLOADER_SECURE_CHUNK_SIZE;
		}

		Status = XSecure_Sha3Initialize();
		if (Status != XST_SUCCESS) {
			goto END;
		}

		if (Block == 0U) {
			Status = XSecure_Sha3Update(&SecureClientInstance, ACAddr,
			(XLOADER_AUTH_CERT_MIN_SIZE - XLOADER_PARTITION_SIG_SIZE));
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		Status = XSecure_Sha3Update(&SecureClientInstance, PrtnAddr, ChunkLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Xil_DCacheInvalidateRange((INTPTR)HashAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES);

		Status = XSecure_Sha3Finish(&SecureClientInstance, HashAddr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		if (Block == 0U) {
			Status = XLoader_VerifyDataAuth(InstancePtr, HashAddr, ACAddr, FALSE);
			if (Status != XST_SUCCESS) {
				goto END;
			}

			Status = Xil_SMemCpy((void*)NextHash, XLOADER_SHA3_HASH_LEN_IN_BYTES,
				(void*)(UINTPTR)(PrtnAddr - (ChunkLen - XLOADER_SHA3_HASH_LEN_IN_BYTES)),
				XLOADER_SHA3_HASH_LEN_IN_BYTES, XLOADER_SHA3_HASH_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
		else {
			Status = Xil_SMemCmp((void*)NextHash, XLOADER_SHA3_HASH_LEN_IN_BYTES,
				(void*)(UINTPTR)HashAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES,
				XLOADER_SHA3_HASH_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if (LastChunk == FALSE) {
			Status = Xil_SMemCpy((void*)NextHash, XLOADER_SHA3_HASH_LEN_IN_BYTES,
				(void*)(UINTPTR)(PrtnAddr + (ChunkLen - XLOADER_SHA3_HASH_LEN_IN_BYTES)),
				XLOADER_SHA3_HASH_LEN_IN_BYTES, XLOADER_SHA3_HASH_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		Block++;
		PrtnLen = PrtnLen - ChunkLen;
		PrtnAddr = PrtnAddr + ChunkLen;
	} while (LastChunk == FALSE);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function calculates hash and verifies signature for
 * 		each partition
 *
 * @param	InstancePtr - Pointer to XLoader_ClientInstance
 * @param	PdiAddr - Address where authenticated PDI is present
 * @param	BhAddr - Address where bootheader is present
 *
 * @return
 *		 - XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static int XLoader_VerifyPlmNPmcCdoAuth(XLoader_ClientInstance *InstancePtr, u64 PdiAddr, u64 BhAddr)
{
	int Status = XST_FAILURE;
	XilLoader_BootHdr* BootHdrPtr = (XilLoader_BootHdr*)(UINTPTR)BhAddr;
	u64 PrtnAddr = PdiAddr + BootHdrPtr->DpiSrcOfst;
	u32 PrtnLen = BootHdrPtr->TotalPlmLen - XLOADER_AUTH_CERT_MIN_SIZE;
	u64 ACAddr = BhAddr + XLOADER_BH_SIZE;
	int Block = 0U;
	u64 HashAddr;
	u8 Sha3Hash[XLOADER_SHA3_HASH_LEN_IN_BYTES];
	HashAddr = (UINTPTR)&Sha3Hash;
	u32 LastChunk = FALSE;
	u32 ChunkLen;

	do {
		if (PrtnLen <= XLOADER_SECURE_CHUNK_SIZE / 2) {
			ChunkLen = PrtnLen;
			LastChunk = TRUE;
		}
		else {
			ChunkLen = XLOADER_SECURE_CHUNK_SIZE / 2;
		}

		Status = XSecure_Sha3Initialize();
		if (Status != XST_SUCCESS) {
			goto END;
		}

		if (Block == 0U) {
			Status = XSecure_Sha3Update(&SecureClientInstance, ACAddr,
				(XLOADER_AUTH_CERT_MIN_SIZE - XLOADER_PARTITION_SIG_SIZE));
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		Status = XSecure_Sha3Update(&SecureClientInstance, PrtnAddr, ChunkLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Xil_DCacheInvalidateRange((INTPTR)HashAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES);

		Status = XSecure_Sha3Finish(&SecureClientInstance, HashAddr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Xil_DCacheInvalidateRange((INTPTR)HashAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES);

		if (Block == 0U) {
			Status = XLoader_VerifyDataAuth(InstancePtr, HashAddr, ACAddr, FALSE);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
		else {
			Status = Xil_SMemCmp((void*)NextHash, XLOADER_SHA3_HASH_LEN_IN_BYTES,
				(void*)(UINTPTR)HashAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES,
				XLOADER_SHA3_HASH_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		Status = Xil_SMemCpy((void*)NextHash, XLOADER_SHA3_HASH_LEN_IN_BYTES,
			(void*)(UINTPTR)(PrtnAddr + (ChunkLen - XLOADER_SHA3_HASH_LEN_IN_BYTES)),
			XLOADER_SHA3_HASH_LEN_IN_BYTES, XLOADER_SHA3_HASH_LEN_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Block++;
		PrtnLen = PrtnLen - ChunkLen;
		PrtnAddr = PrtnAddr + ChunkLen;
	} while (LastChunk == FALSE);

	/*
	 * Calculate hash for PMC CDO
	*/
	PrtnLen = BootHdrPtr->TotalDataPrtnLen;
	LastChunk = FALSE;
	do {
		if (PrtnLen <= XLOADER_SECURE_CHUNK_SIZE / 2) {
			ChunkLen = PrtnLen;
			LastChunk = TRUE;
		}
		else {
			ChunkLen = XLOADER_SECURE_CHUNK_SIZE / 2;
		}


		Status = XSecure_Sha3Initialize();
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = XSecure_Sha3Update(&SecureClientInstance, PrtnAddr, ChunkLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Xil_DCacheInvalidateRange((INTPTR)HashAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES);

		Status = XSecure_Sha3Finish(&SecureClientInstance, HashAddr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Xil_DCacheInvalidateRange((INTPTR)HashAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES);

		Status = Xil_SMemCmp((void*)NextHash, XLOADER_SHA3_HASH_LEN_IN_BYTES,
			(void*)(UINTPTR)HashAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES,
			XLOADER_SHA3_HASH_LEN_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = Xil_SMemCpy((void*)NextHash, XLOADER_SHA3_HASH_LEN_IN_BYTES,
			(void*)(UINTPTR)(PrtnAddr + (ChunkLen - XLOADER_SHA3_HASH_LEN_IN_BYTES)),
			XLOADER_SHA3_HASH_LEN_IN_BYTES, XLOADER_SHA3_HASH_LEN_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Block++;
		PrtnLen = PrtnLen - ChunkLen;
		PrtnAddr = PrtnAddr + ChunkLen;
	} while (LastChunk == FALSE);

END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function fills payload and sends IPI request for verifying
 * 		signature for the provided hash
 *
 * @param	InstancePtr - Pointer to XLoader_ClientInstance
 * @param	HashAddr - Address where hash is present
 * @param	ACAddr - Address where authentication certificate is present
 * @param	SignatureSelect - To select signature in authentication certificate
 * 			- TRUE - Header signature
 * 			- FALSE - Image signature
 *
 * @return
 *		 - XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static int XLoader_VerifyDataAuth(XLoader_ClientInstance *InstancePtr, u64 HashAddr, u64 ACAddr, u32 SignatureSelect)
{
	int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_6U];

	Payload[0U] = PACK_XLOADER_HEADER(0,XLOADER_CMD_ID_DATA_AUTH);
	Payload[1U] = SignatureSelect;
	Payload[2U] = (u32)HashAddr;
	Payload[3U] = (u32)(HashAddr >> 32U);
	Payload[4U] = (u32)ACAddr;
	Payload[5U] = (u32)(ACAddr >> 32U);

	Status = XLoader_ProcessMailbox(InstancePtr, Payload, sizeof(Payload)/sizeof(u32));

	return Status;
}
