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
 *       har  02/16/24 Added XLoader_GetOptionalData API
 *       har  03/05/24 Fixed doxygen warnings
 *       kpt  10/04/24 Added support to validate partial and optimized authentication enabled PDI
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

#define XIH_OPT_DATA_HDR_ID_MASK	(0xFFFFU) /**< Optional data id mask */
#define XIH_OPT_DATA_HDR_LEN_MASK	(0xFFFF0000U) /**< Optional data length mask */
#define XIH_OPT_DATA_HDR_LEN_SHIFT	(16U) /**< shift value to extract optional data length */
#define XIH_OPT_DATA_LEN_OFFSET		(4U) /**< Optional data length offset */
#define XIH_OPT_DATA_DEF_LEN		(2U) /**< Default optional data length */
#define XIH_OPT_HASH_TBL_DATA_ID	(3U) /**< Optional data id for hash table */

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

typedef struct {
	u32 PrtnNum; /**< Partition Number */
	u8 PrtnHash[XLOADER_SHA3_HASH_LEN_IN_BYTES]; /**< Partition hash */
} XilPdi_PrtnHashInfo;

typedef struct {
	u32 IsAuthOptimized; /**< Flag to indicate authentication optimization */
	u32 HashTblSize; /**< Hash table size */
	XilPdi_PrtnHashInfo *StartOffset; /**< Start offset of digest table */
} XilPdi_HashTblInfo;

/************************** Function Prototypes ******************************/
static int XLoader_VerifyPrtnAuth(XLoader_ClientInstance *InstancePtr, u64 PrtnAddr, u32 PrtnLen, u32 PrtnIdx, u64 ACAddr,
		XilPdi_HashTblInfo *HashTblInfo);
static int XLoader_VerifyDataAuth(XLoader_ClientInstance *InstancePtr, u64 HashAddr, u64 ACAddr, u32 SignatureSelect);
static int XLoader_ValidateBhAndPlmNPmcCdoAuth(XLoader_ClientInstance *InstancePtr, const u64 PdiAddr, u64 *MhOffset);
static int XLoader_ValidateMhAndPrtnAuth(XLoader_ClientInstance *InstancePtr, const u64 PdiAddr, const u64 MhOffset,
	const u32 PrtnStartIdx);
static int XLoader_VerifyPlmNPmcCdoAuth(XLoader_ClientInstance *InstancePtr, u64 PdiAddr, u64 BhAddr);
u64 XLoader_SearchOptionalData(u64 StartAddress, u64 EndAddress, u32 DataId);
static int XLoader_CheckAndCompareHashFromIHTOptionalData(u64 HashAddr, XilPdi_HashTblInfo *HashTblInfo, u32 PrtnHashIndex);

/************************** Variable Definitions *****************************/

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
	 * - Perform input parameters validation. Return XST_FAILURE if input parameters are invalid
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
 * @param	NodeId		DDR device id.
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
	 * - Perform input parameters validation. Return XST_FAILURE if input parameters are invalid
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


/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to get optional data from the PDI available in DDR or
 * 		Image Store.
 *
 * @param	InstancePtr 		Pointer to the client instance.
 * @param	OptionalDataInfo	Pointer to XLoader_OptionalDataInfo structure
 * @param	DestAddr		Address of the output buffer wheren optional data shall be copied
 * @param	DestSize		Size of destination buffer in bytes
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 **************************************************************************************************/
int XLoader_GetOptionalData(XLoader_ClientInstance *InstancePtr, const XLoader_OptionalDataInfo* OptionalDataInfo,
	u64 DestAddr, u32 *DestSize)
{
	int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_7U];

	/**
	 * - Perform input parameters validation. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = PACK_XLOADER_HEADER(0, XLOADER_CMD_ID_EXTRACT_METAHEADER);

	if (OptionalDataInfo->PdiSrc == XLOADER_PDI_SRC_DDR) {
		Payload[1U] = OptionalDataInfo->PdiAddrHigh;
		Payload[2U] = OptionalDataInfo->PdiAddrLow;
	}
	else if (OptionalDataInfo->PdiSrc == XLOADER_PDI_SRC_IS){
		Payload[1U] = OptionalDataInfo->PdiId;
		Payload[2U] = 0x0U;
	}
	else {
		Status = XST_FAILURE;
		goto END;
	}

	Payload[3U] = (u32)(DestAddr >> XLOADER_ADDR_HIGH_SHIFT);
	Payload[4U] = (u32)(DestAddr);
	Payload[5U] = *DestSize;
	Payload[6U] = (OptionalDataInfo->DataId << XLOADER_DATA_ID_SHIFT) | (XLOADER_GET_OPT_DATA_FLAG |
		OptionalDataInfo->PdiSrc);

	/**
	 * - Send an IPI request to the PLM by using the XLoader_GetOptionalData command
	 * - Wait for IPI response from PLM with a timeout.
	 * - If the timeout exceeds then error is returned otherwise it returns the status of the IPI
	 * response.
	 */
	Status = XLoader_ProcessMailbox(InstancePtr, Payload, sizeof(Payload)/sizeof(u32));
	*DestSize = InstancePtr->Response[1U];

END:
	return Status;
}

/****************************************************************************/
/**
* @brief	This function search offset of optional data address
*
* @param	StartAddress is start address of IHT optional data
* @param	EndAddress is end address of IHT optional data
* @param	DataId is to identify type of data in data structure
*
* @return
*		- Offset on getting successful optional data offset address
*		for given data Id.
*
*****************************************************************************/
u64 XLoader_SearchOptionalData(u64 StartAddress, u64 EndAddress, u32 DataId)
{
	u64 Offset = StartAddress;
	u32 Data;

	while (Offset < EndAddress) {
		Data = (u32)Xil_In64((UINTPTR)Offset);
		if ((Data & XIH_OPT_DATA_HDR_ID_MASK) != DataId) {
			Offset += (u32)(((Data & XIH_OPT_DATA_HDR_LEN_MASK) >>
				XIH_OPT_DATA_HDR_LEN_SHIFT) << XLOADER_WORD_LEN_SHIFT);
		}
		else {
			break;
		}
	}

	return Offset;
}

/****************************************************************************/
/**
* @brief	This function checks whether Authentication optimization is
*               enabled or not.
*
* @param	StartAddress is start address of IHT optional data
* @param	OptionalDataLen is size of IHT optional data
* @param	HashTbl is pointer to XilPdi_HashTblInfo
*
* @return
*		- XST_SUCCESS On Success
*               - XST_FAILURE On Failure
*
*****************************************************************************/
static int XLoader_IsAuthOptimized(u64 OptionalDataStartAddr, u32 OptionalDataLen, XilPdi_HashTblInfo  *HashTbl)
{
	int Status = XST_FAILURE;
	u64 OptionalDataEndAddr = OptionalDataStartAddr + OptionalDataLen;
	u64 Offset;

	HashTbl->IsAuthOptimized = FALSE;
	Offset = XLoader_SearchOptionalData(OptionalDataStartAddr, OptionalDataEndAddr, XIH_OPT_HASH_TBL_DATA_ID);
	if (Offset < OptionalDataEndAddr) {
		HashTbl->IsAuthOptimized = TRUE;
		HashTbl->HashTblSize = ((Xil_In64((UINTPTR)Offset) & XIH_OPT_DATA_HDR_LEN_MASK) >>
				XIH_OPT_DATA_HDR_LEN_SHIFT) << XLOADER_WORD_LEN_SHIFT;
		HashTbl->HashTblSize -= XIH_OPT_DATA_DEF_LEN;
		HashTbl->HashTblSize /= sizeof(XilPdi_PrtnHashInfo);
		HashTbl->StartOffset = (XilPdi_PrtnHashInfo*)(UINTPTR)(Offset + XIH_OPT_DATA_LEN_OFFSET);
	}

	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to validate Boot Header, PLM
 *               and PMC CDO authentication
 *
 * @param	InstancePtr - Pointer to XLoader_ClientInstance
 * @param	PdiAddr - Address where authenticated PDI is present
 *
 * @return
 *		 - XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static int XLoader_ValidateBhAndPlmNPmcCdoAuth(XLoader_ClientInstance *InstancePtr, const u64 PdiAddr,
		u64 *MhOffset)
{
	int Status = XST_FAILURE;
	u64 BhAddr = PdiAddr + XLOADER_SMAP_WD_PATTERN_SIZE;
	u64 BhAcAddr = BhAddr + XLOADER_BH_SIZE;
	u8 Sha3Hash[XLOADER_SHA3_HASH_LEN_IN_BYTES];
	XilLoader_BootHdr* BootHdrPtr = (XilLoader_BootHdr*)(UINTPTR)BhAddr;
	u32 IsSignedImg = BootHdrPtr->ImgAttrb & XLOADER_BH_IMG_ATTRB_SIGNED_IMG_MASK;
	u64 HashAddr = (UINTPTR)&Sha3Hash;

	if (IsSignedImg != XLOADER_BH_IMG_ATTRB_SIGNED_IMG_MASK) {
		xil_printf("Bootloader must be authenticated to authenticate rest of the partitions \r\n");
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
		*MhOffset = BootHdrPtr->BootHdrFwRsvd.MetaHdrOfst;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function checks whether Partition hash is present or not
*               and compares the claculated hash with hash which is present in
*               IHT Optional data for the respective partitions.
*
* @param       HashPtr is pointer to calculated Hash
* @param       XilPdi_HashTblInfo is pointer to Hash table info
* @param       PrtnHashIndex is index of partition hash in IHT optional data
*
* @return	XST_SUCCESS on success
*               error code on failure
*
******************************************************************************/
static int XLoader_CheckAndCompareHashFromIHTOptionalData(u64 HashAddr, XilPdi_HashTblInfo *HashTblInfo,
		u32 PrtnHashIndex)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u32 Index = 0U;

	for (Index = 0U; Index < HashTblInfo->HashTblSize; Index++) {
		if (HashTblInfo->StartOffset[Index].PrtnNum == PrtnHashIndex) {
			break;
		}
	}

	if (Index < HashTblInfo->HashTblSize) {
		 /*
		  * Compare the calculated hash of respective partition with the hash which is
	          * present in IHT Optional data.
	          */
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, Xil_SMemCmp_CT, HashTblInfo->StartOffset[Index].PrtnHash,
			XLOADER_SHA3_HASH_LEN_IN_BYTES, (u8*)(UINTPTR)HashAddr,
			XLOADER_SHA3_HASH_LEN_IN_BYTES, XLOADER_SHA3_HASH_LEN_IN_BYTES);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			goto END;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to validate Meta Header and partition
 *              authentication
 *
 * @param	InstancePtr - Pointer to XLoader_ClientInstance
 * @param	PdiAddr - Address where authenticated PDI is present
 * @param       MhOffset - Offset where meta header is present
 * @param       PrtnStartIdx - Partition start offset i.e. 0 for PPDI and 1 for Full PDI
 *
 * @return
 *		 - XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static int XLoader_ValidateMhAndPrtnAuth(XLoader_ClientInstance *InstancePtr, const u64 PdiAddr, const u64 MhOffset,
	const u32 PrtnStartIdx)
{
	int Status = XST_FAILURE;
	u8 Sha3Hash[XLOADER_SHA3_HASH_LEN_IN_BYTES];
	u64 MhAddr = PdiAddr + MhOffset;
	XilLoader_ImgHdrTbl* IhtPtr = (XilLoader_ImgHdrTbl*)(UINTPTR)MhAddr;
	u64 MhAcAddr = PdiAddr + ((u64)(IhtPtr->AcOffset) << XLOADER_WORD_LEN_SHIFT);
	XilLoader_PrtnHdr PrtnHdr[XIH_MAX_PRTNS];
	u32 TotalLengthOfPrtnHdr = IhtPtr->NoOfPrtns * XLOADER_PARTITION_SIZE;
	u64 HashAddr = (UINTPTR)&Sha3Hash;
	u64 ImgHdrAddr = PdiAddr + (IhtPtr->ImgHdrAddr * XLOADER_WORD_LEN);
	u64 PrtnAddr;
	u64 PrtnAcAddr;
	u32 TotalSize;
	u32 Idx;
	u32 HashIdx = 1U;
	u32 AuthCertSize;
	XilPdi_HashTblInfo HashTblInfo;

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

	/* Check if authentication is optimized */
	Status = XLoader_IsAuthOptimized((MhAddr + XLOADER_IHT_SIZE), (IhtPtr->OptionalDataLen * XLOADER_WORD_LEN),
			&HashTblInfo);
	if (Status != XST_SUCCESS) {
		goto END;
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

	if (HashTblInfo.IsAuthOptimized != TRUE) {
		AuthCertSize = (XLOADER_AUTH_CERT_MIN_SIZE - XLOADER_PARTITION_SIG_SIZE);
	}
	else {
		AuthCertSize = (XLOADER_AUTH_CERT_MIN_SIZE - (2U * XLOADER_PARTITION_SIG_SIZE));
	}

	Status = XSecure_Sha3Update(&SecureClientInstance, MhAcAddr, AuthCertSize);
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

	if (HashTblInfo.IsAuthOptimized != TRUE) {
		/*
		 * Verify signature for Metaheader excluding Image Header Table
		 */
		Status = XLoader_VerifyDataAuth(InstancePtr, HashAddr, MhAcAddr, FALSE);
	}
	else {
		Status = XLoader_CheckAndCompareHashFromIHTOptionalData(HashAddr, &HashTblInfo, 0U);
	}
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

	/* Verify signature for each partition in case it is authenticated */
	for (Idx = PrtnStartIdx; Idx < IhtPtr->NoOfPrtns; ++Idx) {
		if (PrtnHdr[Idx].AuthCertificateOfst != 0U) {
			PrtnAddr = PdiAddr + ((u64)(PrtnHdr[Idx].DataWordOfst) << XLOADER_WORD_LEN_SHIFT);
			PrtnAcAddr = PdiAddr + ((u64)(PrtnHdr[Idx].AuthCertificateOfst) << XLOADER_WORD_LEN_SHIFT);
			Status = XLoader_VerifyPrtnAuth(InstancePtr, PrtnAddr,
				(PrtnHdr[Idx].TotalDataWordLen << XLOADER_WORD_LEN_SHIFT) - XLOADER_AUTH_CERT_MIN_SIZE,
				HashIdx, PrtnAcAddr, &HashTblInfo);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			xil_printf("Verified signature for partition %d \n\r", Idx);
		}
		HashIdx++;
	}
	xil_printf("Verified signature for all partitions \r\n");

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to validate authenticated PDI
 *              based on PDI type
 *
 * @param	InstancePtr - Pointer to XLoader_ClientInstance
 * @param	PdiAddr - Address where authenticated PDI is present
 * @param       PdiType -  Value to indicate Full or Partial PDI
 *
 * @return
 *		 - XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XLoader_ValidatePdiAuth(XLoader_ClientInstance *InstancePtr, const u64 PdiAddr, const u32 PdiType)
{
	int Status = XST_FAILURE;
	u64 MhOffset;
	u32 PrtnIdx = 0U;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr ==  NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if ((PdiType != XLOADER_PDI_TYPE_FULL) && (PdiType != XLOADER_PDI_TYPE_PARTIAL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_ClientInit(&SecureClientInstance, InstancePtr->MailboxPtr);
	if (Status != XST_SUCCESS) {
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

	if (PdiType != XLOADER_PDI_TYPE_PARTIAL) {
		Status = XLoader_ValidateBhAndPlmNPmcCdoAuth(InstancePtr, PdiAddr, &MhOffset);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/**
		 * Fpr full PDI Partition 0 is PLM. Hence Idx starts from 1 for PLM loadable partitions
		 */
		PrtnIdx = 1U;
	}
	else {
		/* For partial PDI Idx starts from 0 for PLM loadable partitions as there is no PLM */
		MhOffset = XLOADER_SMAP_WD_PATTERN_SIZE;
		PrtnIdx = 0U;
	}

	Status = XLoader_ValidateMhAndPrtnAuth(InstancePtr, PdiAddr, MhOffset, PrtnIdx);

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
static int XLoader_VerifyPrtnAuth(XLoader_ClientInstance *InstancePtr, u64 PrtnAddr, u32 PrtnLen, u32 PrtnIdx, u64 ACAddr,
	XilPdi_HashTblInfo *HashTblInfo)
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
			if (HashTblInfo->IsAuthOptimized != TRUE) {
				Status = XLoader_VerifyDataAuth(InstancePtr, HashAddr, ACAddr, FALSE);
			}
			else {
				Status = XLoader_CheckAndCompareHashFromIHTOptionalData(HashAddr, HashTblInfo, PrtnIdx);
			}
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
