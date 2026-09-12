/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
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
 * 1.00  gnr  02/09/26 Initial release
 * 2.40  gnr  03/18/26 Updated the Payload assignments with XLOADER_PACK_PAYLOAD macros
 * 2.40  sri  03/26/26 Added client API to validate authenticated PDI
 * 2.4   sms  04/16/26 Updated the Payload and Response buffer length parameters in the function
 *       sri  07/21/26 Added RevokeID verification by reading eFuse cache directly
 * 2.5   ng   08/17/26 Added XLoader_ConfigureGlitchDetector client API
 *       ng   09/10/26 Added XLoader_GetTlbinLoadAddr client API
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
#include "xsecure_rsaclient.h"
#include "xsecure_ellipticclient.h"
#include "xsecure_katclient.h"
#include "xil_sutil.h"
#include "xloader_defs.h"
#include "xil_io.h"
#include "xloader_generic.h"

/************************** Constant Definitions *****************************/
#define XLOADER_SMAP_WD_PATTERN_SIZE         (0x00000010U) /**< Size of SMAP width pattern */
#define XLOADER_BH_SIZE                      (0x00001130U) /**< Size of bootheader */
#define XLOADER_BH_IMG_ATTRB_SIGNED_IMG_MASK (0xC0000U) /**< Mask for signed image in BH */

/* Authentication key size macros */
#define XLOADER_SHA3_HASH_LEN_IN_BYTES       (48U) /**< SHA3-384 hash length */
#define XLOADER_WORD_LEN                     (4U) /**< Word length in bytes */
#define XLOADER_WORD_LEN_SHIFT               (2U) /**< Shift to convert word in bytes */

/* Image and Partition sizes */
#define XLOADER_IHT_SIZE                     (0x80U) /**< Size of Image Header Table in bytes*/
#define XLOADER_IH_MAX_PRTNS                 (32U) /**< Maximum number of partitions in the IHT */
#define XLOADER_IH_MAX_IMGS                  (32U) /**< Maximum number of images in the IHT */
#define XLOADER_IH_SIZE                      (0x40U) /**< Size of Image Header in bytes*/
#define XLOADER_PHT_SIZE                     (0x80U) /**< Size of Partition Header Table in bytes*/
#define XLOADER_SECURE_CHUNK_SIZE            (0x8000U) /**< 32K */
#define XLOADER_SECURE_PLM_PMC_CHUNK_SIZE    (XLOADER_SECURE_CHUNK_SIZE / 2) /**< 16K */

/* Hash Block 0 Partition Indices */
#define XLOADER_HB0_INDEX_0_BH               (0U) /**< 0th for Boot Header Hash */
#define XLOADER_HB0_INDEX_1_PLM              (1U) /**< 1st for PLM */
#define XLOADER_HB0_INDEX_2_PMC_DATA         (2U) /**< 2nd for PMC Data */
#define XLOADER_HB0_INDEX_3_HB1              (3U) /**< 3rd for Hash Block 1 */

/* Hash Block 1 Partition Indices */
#define XLOADER_HB1_INDEX_0_MH               (0U) /**< 0th for Meta Header */

/* Revocation ID verification (matches XLoader_VerifyRevokeId on server) */
#define XLOADER_REVOCATION_ID_MAX            (0xFFU) /**< Maximum value of RevokeID */
#define XLOADER_REVOCATION_ROW_SHIFT         (5U) /**< Shift to get row index from RevokeID */
#define XLOADER_REVOCATION_BIT_MASK          (0x1FU) /**< Bit index mask within a revocation row */
#define XLOADER_EFUSE_REVOCATION_ID_0_ADDR   (0xF12500B0U)
	/**< eFuse cache Revocation ID 0 register address (same as server XLOADER_EFUSE_REVOCATION_ID_0_OFFSET) */

/* PMC RTCFG secure-state for partition RevokeID check policy (auth-only validate PDI) */
#define XLOADER_RTCFG_BASEADDR                (0xF2014000U) /**< PMC PLM RTCFG register base address */
#define XLOADER_RTCFG_SECURESTATE_AHWROT_ADDR (XLOADER_RTCFG_BASEADDR + 0x14CU) /**< A-HWRoT secure-state register address */
#define XLOADER_RTCFG_SECURESTATE_AHWROT      (0xA5A5A5A5U) /**< A-HWRoT secure-state register value */
#define XLOADER_RTCFG_SECURESTATE_EMUL_AHWROT (0x5A5A5A5AU) /**< Emulated A-HWRoT secure-state register value */
#define XLOADER_RTCFG_SECURESTATE_NONSECURE   (0xD2D2D2D2U) /**< Non-secure state register value */

#define XLOADER_REVOKEID_OUTOFRANGE_ERR       (0x10U) /**< RevokeID is out of range */
#define XLOADER_REVOKEID_REVOKED_ERR          (0x11U) /**< RevokeID is revoked in eFuse */

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
	u32 WidthDetection;    /**< Width Detection 0xAA995566 */
	u32 ImgIden;           /**< Image Identification */
	u32 EncStatus;         /**< Encryption Status */
	u32 DpiSrcOfst;        /**< Source Offset of PMC FW in DPI */
	u32 DataPrtnOfst;      /**< Data Partition Offset */
	u32 DataPrtnLen;       /**< Data Partition Length */
	u32 TotalDataPrtnLen;  /**< Total Data Partition length */
	u32 PlmLen;            /**< PLM Length */
	u32 TotalPlmLen;       /**< Total PLM length */
	u32 ImgAttrb;          /**< Image Attributes */
	u32 Kek[8U];           /**< Encrypted Key */
	u32 KekIv[3U];         /**< Key IV */
	u32 SecureHdrIv[3U];   /**< Secure Header IV */
	u32 PufShutterVal;     /**< PUF Shutter Value */
	u32 PufRingOscConfig;  /**< PUF ring oscillator configuration */
	u32 EncRevokeId;       /**< Encryption RevokeID */
	u32 UserData[129U];    /**< User Data */
	u32 AuthenticationHdr; /**< Authentication Header */
	u32 HashBlockSize;     /**< HashBlock Size */
	u32 TotalPpkSize;      /**< Total PPK size including alignment */
	u32 ActualPpkSize;     /**< Actual PPK size */
	u32 TotalHBSignSize;   /**< Total HashBlock signature size */
	u32 ActualHBSignSize;  /**< Actual HashBlock signature size */
	u32 RomRsvd[14U];      /**< ROM Reserved */
	XilLoader_BootHdrFwRsvd BootHdrFwRsvd; /**< FW reserved fields */
} XilLoader_BootHdr __attribute__ ((aligned(16U)));

/**
 * Structure to store the image header table details.
 * It contains all the information of image header table in order.
 */
typedef struct {
	u32 Version;           /**< PDI version used  */
	u32 NoOfImgs;          /**< No of images present  */
	u32 ImgHdrAddr;        /**< Address to start of 1st Image header*/
	u32 NoOfPrtns;         /**< No of partitions present  */
	u32 PrtnHdrAddr;       /**< Address to start of 1st partition header*/
	u32 SBDAddr;           /**< Secondary Boot device address */
	u32 Idcode;            /**< Device ID Code */
	u32 Attr;              /**< Attributes */
	u32 PdiId;             /**< PDI ID */
	u32 Rsrvd[3U];         /**< Reserved for future use */
	u32 TotalHdrLen;       /**< Total size of Meta header AC + encryption overload */
	u32 IvMetaHdr[3U];     /**< Iv for decrypting SH of meta header */
	u32 EncKeySrc;         /**< Encryption key source for decrypting SH of headers */
	u32 ExtIdCode;         /**< Extended ID Code */
	u32 AcOffset;          /**< AC offset of Meta header */
	u32 KekIv[3U];         /**< Kek IV for meta header decryption */
	u32 OptionalDataLen;   /**< Len in words of OptionalData */
	u32 AuthenticationHdr; /**< Authentication Header */
	u32 HashBlockSize;     /**< HashBlock Size in words */
	u32 HashBlockOffset;   /**< HashBlock word offset */
	u32 TotalPpkSize;      /**< Total PPK size including alignment */
	u32 ActualPpkSize;     /**< Actual PPK size */
	u32 TotalHBSignSize;   /**< Total HashBlock signature size */
	u32 ActualHBSignSize;  /**< Actual HashBlock signature size */
	u32 Rsvd;              /**< Reserved */
	u32 Checksum;          /**< Checksum of the image header table */
} XilLoader_ImgHdrTbl __attribute__ ((aligned(16U)));

/**
 * Structure to store the image header details.
 */
typedef struct {
	u32 FirstPrtnHdr;      /**< First partition header in the image */
	u32 NoOfPrtns;         /**< Number of partitions in the image */
	u32 EncRevokeID;       /**< Revocation ID of image header */
	u32 ImgAttr;           /**< Image attributes */
	u8 ImgName[16U];       /**< Image name */
	u32 ImgID;             /**< Image ID */
	u32 UID;               /**< Unique ID */
	u32 PUID;              /**< Parent UID */
	u32 FuncID;            /**< Function ID */
	u64 Reserved;          /**< Reserved */
	u32 PcrInfo;           /**< PCR information (unused on Versal 2VE/2VM) */
	u32 Checksum;          /**< Image header checksum */
} XilLoader_ImgHdr __attribute__ ((aligned(16U)));

/**
 * Structure to store the partition header details.
 * It contains all the information of partition header in order.
 */
typedef struct {
	u32 EncDataWordLen;      /**< Enc word length of partition*/
	u32 UnEncDataWordLen;    /**< Unencrypted word length */
	u32 TotalDataWordLen;    /**< Total length including the authentication certificate if any*/
	u32 NextPrtnOfst;        /**< Addr of the next partition header*/
	u64 DstnExecutionAddr;   /**< Execution address */
	u64 DstnLoadAddr;        /**< Load address in DDR/TCM */
	u32 DataWordOfst;        /**< Data word offset */
	u32 PrtnAttrb;           /**< Partition attributes */
	u32 SectionCount;        /**< Section count */
	u32 ChecksumWordOfst;    /**< Address to checksum when enabled */
	u32 PrtnId;              /**< Partition ID */
	u32 AuthCertificateOfst; /**< Address to the authentication certificate when enabled */
	u32 PrtnIv[3U];          /**< IV of the partition's SH */
	u32 EncStatus;           /**< Encryption Status/Key Selection */
	u32 KekIv[3U];           /**< KEK IV for partition decryption */
	u32 EncRevokeID;         /**< Revocation ID of partition for encrypted partition */
	u32 MeasuredBootAddr;    /**< Single Byte Measured Boot Address */
	u32 AuthenticationHdr;   /**< Authentication Header */
	u32 HashBlockSize;       /**< HashBlock Size in words */
	u32 HashBlockOffset;     /**< HashBlock word offset */
	u32 TotalPpkSize;        /**< Total PPK size including alignment */
	u32 ActualPpkSize;       /**< Actual PPK size */
	u32 TotalHBSignSize;     /**< Total HashBlock signature size */
	u32 ActualHBSignSize;    /**< Actual HashBlock signature size */
	u32 Reserved;            /**< Reserved */
	u32 Checksum;            /**< checksum of the partition header */
} XilLoader_PrtnHdr __attribute__ ((aligned(16U)));

/**
 * Partition hash entry information
 */
typedef struct {
	u32 PrtnNum; /**< Partition Number */
	u8 PrtnHash[XLOADER_SHA3_HASH_LEN_IN_BYTES]; /**< Partition hash */
} XilLoader_PrtnHashInfo;

/**
 * HashBlock Definition
 */
typedef struct {
	XilLoader_PrtnHashInfo HashData[XLOADER_IH_MAX_PRTNS + 1U]; /**< Partition hash data */
} XilLoader_HashBlock __attribute__ ((aligned(16U)));

/**
 * Parameters describing the Hash Block authentication certificate layout
 */
typedef struct {
	u32 ReadOffset;       /**< Offset from where the HB auth cert begins */
	u32 TotalPpkSize;     /**< Total PPK size including alignment */
	u32 ActualPpkSize;    /**< Actual PPK size */
	u32 TotalHBSignSize;  /**< Total HB signature size including alignment */
	u32 ActualHBSignSize; /**< Actual HB signature size */
	u32 HBSize;           /**< Hash Block size in bytes */
	u32 AuthHdr;          /**< Authentication header */
} XilLoader_HBSignParams;

/************************** Function Prototypes ******************************/
static int XLoader_ComputeSha3Hash(u64 DataAddr, u32 DataSize, u64 HashOut);
static int XLoader_ValidateBhHBCertAndAuth(XLoader_ClientInstance *InstancePtr, const u64 PdiAddr, u64 *MhOffset);
static int XLoader_VerifyPlmNPmcCdoAuth(u64 PdiAddr, u64 BhAddr);
static int XLoader_ValidateMhAndPrtnAuth(XLoader_ClientInstance *InstancePtr, const u64 PdiAddr,
	const u64 MhOffset, u32 PrtnStartIdx, const u32 PdiType);
static int XLoader_VerifyPrtnAuth(u64 PdiAddr, const XilLoader_PrtnHdr *PrtnHdr, u64 PrtnIdx,
	const u32 PdiType);
static int XLoader_VerifyAuthHashBlock(XLoader_ClientInstance *InstancePtr, u64 HBSignParamsAddr, u64 HBInstanceAddr);
static int XLoader_ReadRtcfgSecureState(u32 *AuthRegPtr);
static int XLoader_IsRtcfgSecureStateConsistent(u32 AuthReg);
static int XLoader_VerifyRevokeId(u32 RevokeId);
static int XLoader_ValidateRevokeIds(u64 MhAddr,
	const XilLoader_ImgHdrTbl *IhtPtr, const XilLoader_PrtnHdr *PrtnHdr, u32 PrtnStartIdx);

/************************** Variable Definitions *****************************/
/**
 * Secure client instance used for XilSecure IPI operations.
 */
static XSecure_ClientInstance SecureClientInstance;
/**
* SHA3 digest output buffer aligned to 64 bytes and placed in .data.Sha3Hash section.
* The attributes ensure alignment and linker section placement for secure hash operations.
*/
static u8 Sha3Hash[XLOADER_SHA3_HASH_LEN_IN_BYTES] __attribute__((aligned(64U)))
	__attribute__ ((section (".data.Sha3Hash")));
/**
 * Buffer holding trailing hash bytes for chunked verification.
 */
static u8 NextHash[XLOADER_SHA3_HASH_LEN_IN_BYTES] __attribute__((aligned(64U)));
/**
 * Hash Block authentication certificate parameters aligned to 64 bytes.
 */
static XilLoader_HBSignParams HBSignParams __attribute__((aligned(64U)));
/**
 * Boot Header Hash Block 0 storage aligned to 64 bytes and placed in .data.HashBlock0 section.
 */
static XilLoader_HashBlock HashBlock0 __attribute__((aligned(64U)))
	__attribute__ ((section (".data.HashBlock0")));
/**
 * Authenticated hash block storage aligned to 64 bytes and placed in .data.HashBlock section.
 */
static XilLoader_HashBlock HashBlock __attribute__((aligned(64U)))
	__attribute__ ((section (".data.HashBlock")));
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
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Perform input parameters validation. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI Payload */
	XLOADER_PACK_PAYLOAD1(Payload, (u32)XLOADER_CMD_ID_CONFIG_JTAG_STATE, Flag);

	/**
	 * - Send an IPI request to the PLM by using the XLoader_ConfigureJtagState CDO command
	 * Wait for IPI response from PLM with a timeout.
	 * - If the timeout exceeds then error is returned otherwise it returns the status of the IPI
	 * response.
	 */
	Status = XLoader_ProcessMailbox(InstancePtr, Payload, PAYLOAD_ARG_CNT);

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
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Perform input parameters validation. Return XST_FAILURE if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL) || (CryptoCounters == NULL)) {
		goto END;
	}

	/** Fill IPI Payload */
	XLOADER_PACK_PAYLOAD1(Payload, (u32)XLOADER_CMD_ID_READ_DDR_CRYPTO_COUNTERS, NodeId);

	/**
	 * - Send an IPI request to the PLM by using the XLoader_ReadDdrCryptoPerfCounters CDO command
	 * Wait for IPI response from PLM with a timeout.
	 * - If the timeout exceeds then error is returned otherwise it returns the status of the IPI
	 * response.
	 */
	Status = XLoader_ProcessMailbox(InstancePtr, Payload, PAYLOAD_ARG_CNT);
	CryptoCounters->DDRCounter0 =  InstancePtr->Response[1];
	CryptoCounters->DDRCounter1 =  InstancePtr->Response[2];
	CryptoCounters->DDRCounter2 =  InstancePtr->Response[3];
	CryptoCounters->DDRCounter3 =  InstancePtr->Response[4];

END:
	return Status;
}


/*****************************************************************************/
/**
 * @brief	This function computes SHA3-384 hash over the given data using
 *		xilsecure IPI.
 *
 * @param	DataAddr	Address of the data to hash
 * @param	DataSize	Size of the data in bytes
 * @param	HashOut		Buffer (minimum 48 bytes, 64-byte aligned) to store hash
 *
 * @return
 *		 - XST_SUCCESS on success
 *		 - Error code on failure
 *
 ******************************************************************************/
static int XLoader_ComputeSha3Hash(u64 DataAddr, u32 DataSize, u64 HashOut)
{
	volatile int Status = XST_FAILURE;
	/* 32-byte alignment is required by XilSecure DMA for IPI transfers */
	XSecure_ShaOpParams Sha3Params __attribute__((aligned(32U)));

	/* Populate source data location, byte length, and digest output buffer */
	Sha3Params.DataAddr = DataAddr;
	Sha3Params.HashAddr = HashOut;
	Sha3Params.DataSize = DataSize;
	/* HashBufSize must match the 48-byte digest length of SHA3-384 */
	Sha3Params.HashBufSize = XLOADER_SHA3_HASH_LEN_IN_BYTES;
	Sha3Params.ShaMode = (u8)XSECURE_SHA3_384;
	/* Single-shot hash: entire data region is hashed in one IPI call */
	Sha3Params.IsLast = (u8)TRUE;
	/* START|UPDATE|FINISH performs the complete hash in a single IPI round-trip */
	Sha3Params.OperationFlags = XSECURE_SHA_START | XSECURE_SHA_UPDATE |
				   XSECURE_SHA_FINISH;

	/* Invoke the SHA3 operation on the XilSecure server via IPI */
	Status = XSecure_Sha3Operation(&SecureClientInstance, &Sha3Params);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function calculates hash and verifies hash of PLM & PMC CDO
 *
 * @param	PdiAddr	Address where authenticated PDI is present
 * @param	BhAddr	Address where boot header is present
 *
 * @return
 *		 - XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static int XLoader_VerifyPlmNPmcCdoAuth(u64 PdiAddr, u64 BhAddr)
{
	volatile int Status = XST_FAILURE;
	/* Cast BhAddr to a Boot Header pointer to access individual fields */
	const XilLoader_BootHdr *BhPtr = (const XilLoader_BootHdr *)(UINTPTR)BhAddr;
	u64 PrtnAddr = PdiAddr + BhPtr->DpiSrcOfst;
	u32 PrtnLen = BhPtr->TotalPlmLen;
	int Block = 0U;
	u64 Sha3DstAddr = (UINTPTR)&Sha3Hash;
	u32 LastChunk = FALSE;
	u32 ChunkLen;

	/* Process PLM data in chunks, computing and verifying SHA3 for each */
	do {
		/* Clamp chunk size to remaining data length on the last iteration */
		if (PrtnLen <= XLOADER_SECURE_PLM_PMC_CHUNK_SIZE) {
			ChunkLen = PrtnLen;
			LastChunk = TRUE;
		}
		else {
			ChunkLen = XLOADER_SECURE_PLM_PMC_CHUNK_SIZE;
		}

		/* Compute SHA3 for PLM (to validate with hash in the Hash Block 0) */
		Xil_DCacheInvalidateRange((INTPTR)Sha3DstAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES);
		Status = XLoader_ComputeSha3Hash(PrtnAddr, ChunkLen, Sha3DstAddr);
		if(Status != XST_SUCCESS) {
			XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "SHA3 digest calculation for PLM failed, Status = 0x%x\r\n", Status);
			goto END;
		}

		if (Block == 0U) {
			/* First block: compare hash against Hash Block 0 PLM entry */
			Block++;
			Status = Xil_SMemCmp((void *)&HashBlock0.HashData[XLOADER_HB0_INDEX_1_PLM].PrtnHash,
					XLOADER_SHA3_HASH_LEN_IN_BYTES,
					(void *)(UINTPTR)Sha3Hash, XLOADER_SHA3_HASH_LEN_IN_BYTES,
					XLOADER_SHA3_HASH_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Hash mismatch for PLM, Status = 0x%x\r\n", Status);
				goto END;
			}
		}
		else {
			/* Subsequent blocks: compare against chained NextHash */
			Status = Xil_SMemCmp((void *)NextHash, XLOADER_SHA3_HASH_LEN_IN_BYTES,
					(void *)(UINTPTR)Sha3DstAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES,
					XLOADER_SHA3_HASH_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
		if (LastChunk == FALSE) {
			/* Save trailing hash bytes as NextHash for chained verification of the next chunk */
			Status = Xil_SMemCpy((void *)NextHash, XLOADER_SHA3_HASH_LEN_IN_BYTES,
					(void *)(UINTPTR)(PrtnAddr + (ChunkLen - XLOADER_SHA3_HASH_LEN_IN_BYTES)),
					XLOADER_SHA3_HASH_LEN_IN_BYTES,
					XLOADER_SHA3_HASH_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		/* Advance to the next chunk */
		PrtnLen = PrtnLen - ChunkLen;
		PrtnAddr = PrtnAddr + ChunkLen;
	} while (LastChunk == FALSE);

	/*
	 * Calculate hash for PMC CDO
	 */
	PrtnLen = BhPtr->TotalDataPrtnLen;
	LastChunk = FALSE;
	Block = 0U;
	/* Process PMC CDO data in chunks, computing and verifying SHA3 for each */
	do {
		/* Clamp chunk size to remaining data length on the last iteration */
		if (PrtnLen <= XLOADER_SECURE_PLM_PMC_CHUNK_SIZE) {
			ChunkLen = PrtnLen;
			LastChunk = TRUE;
		}
		else {
			ChunkLen = XLOADER_SECURE_PLM_PMC_CHUNK_SIZE;
		}

		/* Compute SHA3 for PMC CDO (to validate with hash in the Hash Block 0) */
		Xil_DCacheInvalidateRange((INTPTR)Sha3DstAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES);
		Status = XLoader_ComputeSha3Hash(PrtnAddr, ChunkLen, Sha3DstAddr);
		if(Status != XST_SUCCESS) {
			XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "SHA3 digest calculation for PMC CDO failed, Status = 0x%x\r\n", Status);
			goto END;
		}

		if (Block == 0U) {
			/* First block: compare hash against Hash Block 0 PMC CDO entry */
			Block++;
			Status = Xil_SMemCmp((void *)&HashBlock0.HashData[XLOADER_HB0_INDEX_2_PMC_DATA].PrtnHash,
					XLOADER_SHA3_HASH_LEN_IN_BYTES,	(void *)(UINTPTR)Sha3Hash,
					XLOADER_SHA3_HASH_LEN_IN_BYTES,
					XLOADER_SHA3_HASH_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Hash mismatch for PMC CDO, Status = 0x%x\r\n", Status);
				goto END;
			}
		}
		else {
			/* Subsequent blocks: compare against chained NextHash */
			Status = Xil_SMemCmp((void *)NextHash, XLOADER_SHA3_HASH_LEN_IN_BYTES,
					(void *)(UINTPTR)Sha3DstAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES,
					XLOADER_SHA3_HASH_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
		if (LastChunk == FALSE) {
			/* Save trailing hash bytes as NextHash for chained verification of the next chunk */
			Status = Xil_SMemCpy((void *)NextHash, XLOADER_SHA3_HASH_LEN_IN_BYTES,
					(void *)(UINTPTR)(PrtnAddr + (ChunkLen - XLOADER_SHA3_HASH_LEN_IN_BYTES)),
					XLOADER_SHA3_HASH_LEN_IN_BYTES,
					XLOADER_SHA3_HASH_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
		/* Advance to the next chunk */
		PrtnLen = PrtnLen - ChunkLen;
		PrtnAddr = PrtnAddr + ChunkLen;
	} while (LastChunk == FALSE);


END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function validates the Boot Header Hash Block certificate
 *		and authenticates it. The BH HB cert starts immediately after
 *		the Boot Header (at PdiAddr + SMAP_WD_PATTERN_SIZE + BH_SIZE).
 *		On success, MhOffset is set to the byte offset of the IHT.
 *
 * @param	InstancePtr	Pointer to XLoader_ClientInstance
 * @param	PdiAddr		Base address of the PDI in memory
 * @param	MhOffset	Output: byte offset from PdiAddr to the IHT
 *
 * @return
 *		 - XST_SUCCESS on success
 *		 - Error code on failure
 *
 ******************************************************************************/
static int XLoader_ValidateBhHBCertAndAuth(XLoader_ClientInstance *InstancePtr, const u64 PdiAddr, u64 *MhOffset)
{
	volatile int Status = XST_FAILURE;
	/* Boot Header begins immediately after the SMAP word-dummy pattern */
	u64 BhAddr = PdiAddr + XLOADER_SMAP_WD_PATTERN_SIZE;
	const XilLoader_BootHdr *BhPtr =
		(const XilLoader_BootHdr *)(UINTPTR)BhAddr;
	/* Hash Block AC for Boot Header starts right after the Boot Header */
	u64 BhHbACAddr = BhAddr + XLOADER_BH_SIZE;
	u64 Sha3DstAddr;
	/* Check image attribute to confirm the bootloader is a signed image */
	u32 IsSignedImg = (BhPtr->ImgAttrb & XLOADER_BH_IMG_ATTRB_SIGNED_IMG_MASK);

	if (IsSignedImg != XLOADER_BH_IMG_ATTRB_SIGNED_IMG_MASK) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Bootloader must be authenticated to validate rest of the partitions\r\n");
		goto END;
	}

	/*
	 * The HB cert parameters are stored in the Boot Header.
	 * Hash Block AC along with the HB is available at the end of BH.
	 * Populate HBSignParams from the Boot Header fields.
	 */
	HBSignParams.ReadOffset = (u32)BhHbACAddr;
	HBSignParams.TotalPpkSize = BhPtr->TotalPpkSize;
	HBSignParams.ActualPpkSize = BhPtr->ActualPpkSize;
	HBSignParams.TotalHBSignSize = BhPtr->TotalHBSignSize;
	HBSignParams.ActualHBSignSize = BhPtr->ActualHBSignSize;
	HBSignParams.HBSize = BhPtr->HashBlockSize;
	HBSignParams.AuthHdr = BhPtr->AuthenticationHdr;

	/* Verify the Hash Block 0 AC signature via IPI before trusting any hash data */
	Status = XLoader_VerifyAuthHashBlock(InstancePtr, (UINTPTR)&HBSignParams, (UINTPTR)&HashBlock0);
	if (Status != XST_SUCCESS) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Verification of Hash Block 0 failed, Status = 0x%x\r\n", Status);
		goto END;
	}

	/* Compute SHA3 for Boot Header (to validate with hash in the Hash Block 0) */
	Sha3DstAddr = (UINTPTR)&Sha3Hash;
	Xil_DCacheInvalidateRange((INTPTR)Sha3DstAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES);
	Status = XLoader_ComputeSha3Hash(BhAddr, XLOADER_BH_SIZE, Sha3DstAddr);
	if(Status != XST_SUCCESS) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "SHA3 digest calculation for Boot Header failed, Status = 0x%x\r\n", Status);
		goto END;
	}

	/* Compare computed Boot Header hash with the corresponding Hash Block 0 entry */
	Status = Xil_SMemCmp((void *)&HashBlock0.HashData[XLOADER_HB0_INDEX_0_BH].PrtnHash,
			XLOADER_SHA3_HASH_LEN_IN_BYTES,	(void *)(UINTPTR)Sha3Hash,
			XLOADER_SHA3_HASH_LEN_IN_BYTES,	XLOADER_SHA3_HASH_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Hash mismatch for Boot Header, Status = 0x%x\r\n", Status);
		goto END;
	}

	XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Verified hash of Boot Header\r\n");

	/*
	 * Calculate hash for PLM and PMC CDO and verify signature
	 */
	Status = XLoader_VerifyPlmNPmcCdoAuth(PdiAddr, BhAddr);
	if (Status != XST_SUCCESS) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Authentication of PLM or PMC CDO failed, Status = 0x%x\r\n", Status);
		goto END;
	}
	else {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Verified hash and data of PLM and PMC CDO\r\n");
		/* Store the IHT byte offset for the caller to use in subsequent validation steps */
		*MhOffset = BhPtr->BootHdrFwRsvd.MetaHdrOfst;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function calculates hash and verifies hash of each partition
 *
 * @param	PdiAddr	Address where authenticated PDI is present
 * @param	PrtnHdr	Pointer to partition header information
 * @param	PrtnIdx	Partition index in the hash block
 * @param	PdiType	Type of PDI (full or partial) to determine hash block indexing
 *
 * @return
 *		 - XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static int XLoader_VerifyPrtnAuth(u64 PdiAddr, const XilLoader_PrtnHdr *PrtnHdr, u64 PrtnIdx,
	const u32 PdiType)
{
	volatile int Status = XST_FAILURE;
	/* Convert word offsets to byte addresses for partition data access */
	u64 PrtnAddr = PdiAddr + ((u64)(PrtnHdr->DataWordOfst) << XLOADER_WORD_LEN_SHIFT);
	u32 PrtnLen = PrtnHdr->TotalDataWordLen << XLOADER_WORD_LEN_SHIFT;
	int Block = 0U;
	u64 Sha3DstAddr = (UINTPTR)&Sha3Hash;
	u32 LastChunk = FALSE;
	u32 ChunkLen;
	u64 HBPrtnIdx;

	/* For partial PDI, index starts from 0 for PLM loadable partitions as there is no PLM
	 * but the hash block still starts from index 0 for the IHT, from 1 for partitions.
	 */
	if (PdiType != XLOADER_PDI_TYPE_PARTIAL) {
		HBPrtnIdx = PrtnIdx;
	}
	else {
		HBPrtnIdx = PrtnIdx + 1U;
	}

	/* Iterate over each chunk of the partition, verifying SHA3 hash incrementally */
	do {
		/* Clamp chunk size to remaining data length on the last iteration */
		if (PrtnLen <= XLOADER_SECURE_CHUNK_SIZE) {
			ChunkLen = PrtnLen;
			LastChunk = TRUE;
		}
		else {
			ChunkLen = XLOADER_SECURE_CHUNK_SIZE;
		}

		/* Compute SHA3 for Partition (to validate with hash in the Hash Block) */
		Xil_DCacheInvalidateRange((INTPTR)Sha3DstAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES);
		Status = XLoader_ComputeSha3Hash(PrtnAddr, ChunkLen, Sha3DstAddr);
		if(Status != XST_SUCCESS) {
			XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "SHA3 digest calculation for Partition %d failed, Status = 0x%x\r\n", PrtnIdx, Status);
			goto END;
		}

		if (Block == 0U) {
			/* First block: compare hash against the partition's Hash Block entry */
			Block++;
			Status = Xil_SMemCmp((void *)&HashBlock.HashData[HBPrtnIdx].PrtnHash,
					XLOADER_SHA3_HASH_LEN_IN_BYTES, (void *)(UINTPTR)Sha3Hash,
					XLOADER_SHA3_HASH_LEN_IN_BYTES,
					XLOADER_SHA3_HASH_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Hash mismatch for Partition %d, Status = 0x%x\r\n", PrtnIdx, Status);
				goto END;
			}
		}
		else {
			/* Subsequent blocks: compare against chained NextHash */
			Status = Xil_SMemCmp((void *)NextHash, XLOADER_SHA3_HASH_LEN_IN_BYTES,
					(void *)(UINTPTR)Sha3DstAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES,
					XLOADER_SHA3_HASH_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		if (LastChunk == FALSE) {
			/* Save trailing hash bytes as NextHash for chained verification of the next chunk */
			Status = Xil_SMemCpy((void *)NextHash, XLOADER_SHA3_HASH_LEN_IN_BYTES,
					(void *)(UINTPTR)(PrtnAddr + (ChunkLen - XLOADER_SHA3_HASH_LEN_IN_BYTES)),
					XLOADER_SHA3_HASH_LEN_IN_BYTES,
					XLOADER_SHA3_HASH_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}

		/* Advance to the next chunk */
		PrtnLen = PrtnLen - ChunkLen;
		PrtnAddr = PrtnAddr + ChunkLen;
	} while (LastChunk == FALSE);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function validates the MetaHeader (IHT) Hash Block cert
 *		and authenticates all partitions starting from PrtnStartIdx.
 *
 *		For each partition, a SHA3 hash of the partition data is
 *		computed and compared against the corresponding entry in the
 *		IHT Hash Block.
 *
 * @param	InstancePtr	Pointer to XLoader_ClientInstance
 * @param	PdiAddr		Base address of the PDI in memory
 * @param	MhOffset	Byte offset from PdiAddr to the IHT
 * @param	PrtnStartIdx	Partition index to start from (1 for full PDI, 0 for partial PDI)
 * @param	PdiType		Type of PDI (full or partial) to determine hash block indexing
 *
 * @return
 *		 - XST_SUCCESS on success
 *		 - Error code on failure
 *
 ******************************************************************************/
static int XLoader_ValidateMhAndPrtnAuth(XLoader_ClientInstance *InstancePtr, const u64 PdiAddr,
	const u64 MhOffset, u32 PrtnStartIdx, const u32 PdiType)
{
	volatile int Status = XST_FAILURE;
	u64 MhAddr = PdiAddr + MhOffset;
	const XilLoader_ImgHdrTbl *IhtPtr = (const XilLoader_ImgHdrTbl *)(UINTPTR)MhAddr;
	u64 Sha3DstAddr = (UINTPTR)&Sha3Hash;
	u64 Hb1Addr;
	u32 MhDataSize;
	XilLoader_PrtnHdr PrtnHdr[XLOADER_IH_MAX_PRTNS] = {0U};
	u32 TotalLengthOfPrtnHdr;
	u32 Idx;

	/* Validate number of images in the Image Header Table */
	if (IhtPtr->NoOfImgs > XLOADER_IH_MAX_IMGS) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
			"Invalid number of images in IHT: %u\r\n", IhtPtr->NoOfImgs);
		goto END;
	}

	/* Validate number of partitions in the Image Header Table */
	if (IhtPtr->NoOfPrtns > XLOADER_IH_MAX_PRTNS) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Invalid number of partitions in IHT: %u\r\n", IhtPtr->NoOfPrtns);
		goto END;
	}

	/* Calculate the total length of all partition headers */
	TotalLengthOfPrtnHdr = IhtPtr->NoOfPrtns * XLOADER_PHT_SIZE;

	/*
	 * The HB cert parameters are stored in the Image Header Table.
	 * Read the IHT from the word offset given by Bh->BootHdrFwRsvd.MetaHdrOfst.
	 * Hash Block AC along with the HB is available at HashBlockOffset.
	 */
	HBSignParams.ReadOffset = (u32)(PdiAddr + ((u64)IhtPtr->AcOffset << XLOADER_WORD_LEN_SHIFT));
	HBSignParams.TotalPpkSize = IhtPtr->TotalPpkSize;
	HBSignParams.ActualPpkSize = IhtPtr->ActualPpkSize;
	HBSignParams.TotalHBSignSize = IhtPtr->TotalHBSignSize;
	HBSignParams.ActualHBSignSize = IhtPtr->ActualHBSignSize;
	HBSignParams.HBSize = IhtPtr->HashBlockSize * XLOADER_WORD_LEN;
	HBSignParams.AuthHdr = IhtPtr->AuthenticationHdr;

	/* Verify Hash Block 1 AC signature before trusting any IHT hash data */
	Status = XLoader_VerifyAuthHashBlock(InstancePtr, (UINTPTR)&HBSignParams, (UINTPTR)&HashBlock);
	if (Status != XST_SUCCESS) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Verification of Hash Block 1 failed, Status = 0x%x\r\n", Status);
		goto END;
	}

	/* In case of Partial PDI, there is no HB0 to verify hash of HB1 */
	if (PdiType != XLOADER_PDI_TYPE_PARTIAL) {
		/* Hash Block 1 is located at the word offset stored in the Image Header Table */
		Hb1Addr = PdiAddr + ((u64)IhtPtr->HashBlockOffset << XLOADER_WORD_LEN_SHIFT);

		/* Compute SHA3 for Hash Block 1 (to validate with hash in the Hash Block 0) */
		Xil_DCacheInvalidateRange((INTPTR)Sha3DstAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES);
		Status = XLoader_ComputeSha3Hash(Hb1Addr, (IhtPtr->HashBlockSize * XLOADER_WORD_LEN), Sha3DstAddr);
		if(Status != XST_SUCCESS) {
			XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "SHA3 digest calculation for Hash Block 1 failed, Status = 0x%x\r\n", Status);
			goto END;
		}

		/* Compare computed SHA3 hash with the hash in the Hash Block 0 3rd partition index */
		Status = Xil_SMemCmp((void *)&HashBlock0.HashData[XLOADER_HB0_INDEX_3_HB1].PrtnHash,
				XLOADER_SHA3_HASH_LEN_IN_BYTES,	(void *)(UINTPTR)Sha3Hash,
				XLOADER_SHA3_HASH_LEN_IN_BYTES, XLOADER_SHA3_HASH_LEN_IN_BYTES);
		if (Status != XST_SUCCESS) {
			XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Hash mismatch for Hash Block 1, Status = 0x%x\r\n", Status);
			goto END;
		}

		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Verified hash of Hash Block 1\r\n");
	}

	/* Compute SHA3 for Meta Header (to validate with hash in the Hash Block 1) */
	/* The size of MH for which SHA3 to be computed */
	MhDataSize =  XLOADER_IHT_SIZE;
	MhDataSize += (IhtPtr->OptionalDataLen * XLOADER_WORD_LEN);
	MhDataSize += (IhtPtr->NoOfImgs * XLOADER_IH_SIZE);
	/* Include all partition headers in the MH size */
	MhDataSize += TotalLengthOfPrtnHdr;

	Xil_DCacheInvalidateRange((INTPTR)Sha3DstAddr, XLOADER_SHA3_HASH_LEN_IN_BYTES);
	Status = XLoader_ComputeSha3Hash(MhAddr, MhDataSize, Sha3DstAddr);
	if(Status != XST_SUCCESS) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "SHA3 digest calculation for Meta Header failed, Status = 0x%x\r\n", Status);
		goto END;
	}

	/* Compare computed SHA3 hash with the hash in the Hash Block 1 0th partition index */
	Status = Xil_SMemCmp((void *)&HashBlock.HashData[XLOADER_HB1_INDEX_0_MH].PrtnHash,
			XLOADER_SHA3_HASH_LEN_IN_BYTES, (void *)(UINTPTR)Sha3Hash,
			XLOADER_SHA3_HASH_LEN_IN_BYTES, XLOADER_SHA3_HASH_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Hash mismatch for Meta Header, Status = 0x%x\r\n", Status);
		goto END;
	}

	XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Verified hash of Meta Header\r\n");

	/* Read all the partition headers from the word offset given by IhtPtr->PrtnHdrAddr */
	Status = Xil_SMemCpy((void *)PrtnHdr, TotalLengthOfPrtnHdr,
			(void *)(UINTPTR)(PdiAddr + (IhtPtr->PrtnHdrAddr * XLOADER_WORD_LEN)),
			TotalLengthOfPrtnHdr, TotalLengthOfPrtnHdr);
	if (Status != XST_SUCCESS) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Copying partition headers failed, Status = 0x%x\r\n", Status);
		goto END;
	}

	/* Read eFuse revocation rows and verify image/partition RevokeID fields */
	Status = XLoader_ValidateRevokeIds(MhAddr, IhtPtr, PrtnHdr, PrtnStartIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Check if any more Hash Blocks are present and verify them */
	for (Idx = PrtnStartIdx; Idx < IhtPtr->NoOfPrtns; ++Idx) {
		/* Only partitions with a non-zero auth certificate offset carry their own Hash Block */
		if (PrtnHdr[Idx].AuthCertificateOfst != 0U) {
			/*
			* The HB cert parameters are stored in the Partition Header Table.
			* Hash Block AC along with the HB is available at HashBlockOffset.
			*/
			HBSignParams.ReadOffset = (u32)(PdiAddr + ((u64)PrtnHdr[Idx].AuthCertificateOfst << XLOADER_WORD_LEN_SHIFT));
			HBSignParams.TotalPpkSize = PrtnHdr[Idx].TotalPpkSize;
			HBSignParams.ActualPpkSize = PrtnHdr[Idx].ActualPpkSize;
			HBSignParams.TotalHBSignSize = PrtnHdr[Idx].TotalHBSignSize;
			HBSignParams.ActualHBSignSize = PrtnHdr[Idx].ActualHBSignSize;
			HBSignParams.HBSize = PrtnHdr[Idx].HashBlockSize * XLOADER_WORD_LEN;
			HBSignParams.AuthHdr = PrtnHdr[Idx].AuthenticationHdr;

			Status = XLoader_VerifyAuthHashBlock(InstancePtr, (UINTPTR)&HBSignParams, (UINTPTR)&HashBlock);
			if (Status != XST_SUCCESS) {
				XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Verification of Hash Block from Partition %d failed, Status = 0x%x\r\n", Idx, Status);
				goto END;
			}
		}
	}
	XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Verified all remaining Hash Blocks\r\n");

	/* Validate all the partitions individually with hash from Hash Blocks */
	for (Idx = PrtnStartIdx; Idx < IhtPtr->NoOfPrtns; ++Idx) {
		Status = XLoader_VerifyPrtnAuth(PdiAddr, &PrtnHdr[Idx], Idx, PdiType);
		if (Status != XST_SUCCESS) {
			XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Verification of Partition %d failed, Status = 0x%x\r\n", Idx, Status);
			goto END;
		}
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Verified hash of Partition %d\r\n", Idx);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to validate authenticated PDI
 *              based on PDI type
 *
 * @param	InstancePtr	Pointer to XLoader_ClientInstance
 * @param	PdiAddr		Address where authenticated PDI is present
 * @param	PdiType		Value to indicate full or partial PDI
 *
 * @return
 *		 - XST_SUCCESS on success and error code on failure
 *
 * @note	This function has external linkage and is declared in
 *		xloader_plat_client.h for use by application code.
 *
 ******************************************************************************/
int XLoader_ValidatePdiAuth(XLoader_ClientInstance *InstancePtr, const u64 PdiAddr, const u32 PdiType)
{
	volatile int Status = XST_FAILURE;
	u64 MhOffset = 0U;
	u32 PrtnIdx = 0U;

	/* Validate that the client instance and its mailbox pointer are non-NULL */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Only full PDI and partial PDI types are supported */
	if ((PdiType != XLOADER_PDI_TYPE_FULL) && (PdiType != XLOADER_PDI_TYPE_PARTIAL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Initialize the XilSecure client using the same mailbox as the loader client */
	Status = XSecure_ClientInit(&SecureClientInstance, InstancePtr->MailboxPtr);
	if (Status != XST_SUCCESS) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "XSecure_ClientInit failed, Status = 0x%x\r\n", Status);
		goto END;
	}

	/* Run Known Answer Tests to verify cryptographic primitives before use */
	Status = XSecure_RsaPublicEncKat(&SecureClientInstance);
	if (Status != XST_SUCCESS) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "RSA KAT failed, Status = 0x%x\r\n", Status);
		goto END;
	}

	Status = XSecure_Sha3Kat(&SecureClientInstance);
	if (Status != XST_SUCCESS) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "SHA3 KAT failed, Status = 0x%x\r\n", Status);
		goto END;
	}

	Status = XSecure_EllipticSignVerifyKat(&SecureClientInstance, XSECURE_ECC_PRIME);
	if (Status != XST_SUCCESS) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "ECDSA KAT failed, Status = 0x%x\r\n", Status);
		goto END;
	}

	if (PdiType != XLOADER_PDI_TYPE_PARTIAL) {
		/* Full PDI: validate Boot Header Hash Block and extract IHT offset */
		Status = XLoader_ValidateBhHBCertAndAuth(InstancePtr, PdiAddr, &MhOffset);
		if (Status != XST_SUCCESS) {
			XLoader_Client_Printf(XLOADER_DEBUG_GENERAL, "Boot Header Hash Block validation failed, Status = 0x%x\r\n", Status);
			goto END;
		}
		/* For full PDI, partition 0 is the PLM. Authenticated partitions start from index 1 */
		PrtnIdx = 1U;
	}
	else {
		/* For partial PDI, index starts from 0 for PLM loadable partitions as there is no PLM */
		MhOffset = XLOADER_SMAP_WD_PATTERN_SIZE;
		PrtnIdx = 0U;
	}

	/* Validate the Meta Header and authenticate all partitions from PrtnIdx onwards */
	Status = XLoader_ValidateMhAndPrtnAuth(InstancePtr, PdiAddr, MhOffset, PrtnIdx, PdiType);

END:
	return Status;
}
/*****************************************************************************/
/**
 * @brief	This function reads PLM RTCFG A-HWRoT secure-state register.
 *
 * @param	AuthRegPtr	Pointer to store A-HWRoT secure-state register value
 *
 * @return
 *		 - XST_SUCCESS on success
 *		 - XST_GLITCH_ERROR if redundant reads mismatch
 *
 ******************************************************************************/
static int XLoader_ReadRtcfgSecureState(u32 *AuthRegPtr)
{
	volatile int Status = XST_FAILURE;
	volatile u32 ReadAuthReg = 0U;
	volatile u32 ReadAuthRegTmp = 0U;

	if (AuthRegPtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Xil_DCacheInvalidateRange((INTPTR)XLOADER_RTCFG_SECURESTATE_AHWROT_ADDR, sizeof(u32));

	ReadAuthReg = Xil_In32(XLOADER_RTCFG_SECURESTATE_AHWROT_ADDR);
	ReadAuthRegTmp = Xil_In32(XLOADER_RTCFG_SECURESTATE_AHWROT_ADDR);

	if (ReadAuthReg != ReadAuthRegTmp) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
			"RTCFG secure-state glitch detected, AHWROT = 0x%08x/0x%08x\r\n",
			ReadAuthReg, ReadAuthRegTmp);
		Status = XST_GLITCH_ERROR;
		goto END;
	}

	*AuthRegPtr = ReadAuthReg;

	XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
		"RTCFG secure-state: AHWROT = 0x%08x\r\n", ReadAuthReg);

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function checks PLM RTCFG A-HWRoT secure-state register value.
 *
 * @param	AuthReg	A-HWRoT secure-state register value
 *
 * @return
 *		 - XST_SUCCESS if value is a valid PLM secure-state marker
 *		 - XST_GLITCH_ERROR otherwise
 *
 ******************************************************************************/
static int XLoader_IsRtcfgSecureStateConsistent(u32 AuthReg)
{
	volatile int Status = XST_FAILURE;

	if ((AuthReg != XLOADER_RTCFG_SECURESTATE_AHWROT) &&
		(AuthReg != XLOADER_RTCFG_SECURESTATE_EMUL_AHWROT) &&
		(AuthReg != XLOADER_RTCFG_SECURESTATE_NONSECURE)) {
		Status = XST_GLITCH_ERROR;
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function verifies RevokeID by reading the eFuse revocation
 *		cache registers directly (same approach as XLoader_VerifyRevokeId on
 *		the server), avoiding a XilNvm library dependency.
 *
 * @param	RevokeId	RevokeID field to verify
 *
 * @return
 *		 - XST_SUCCESS if revocation ID is valid
 *		 - XLOADER_REVOKEID_OUTOFRANGE_ERR if out of range
 *		 - XLOADER_REVOKEID_REVOKED_ERR if revoked
 *
 ******************************************************************************/
static int XLoader_VerifyRevokeId(u32 RevokeId)
{
	volatile int Status = XST_FAILURE;
	u32 Row;
	u32 Bit;
	u32 RowVal;
	volatile u32 Value;
	volatile u32 ValueTmp;

	if (RevokeId > XLOADER_REVOCATION_ID_MAX) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
			"RevokeID 0x%x is out of range (max 0x%x)\r\n",
			RevokeId, XLOADER_REVOCATION_ID_MAX);
		Status = XLOADER_REVOKEID_OUTOFRANGE_ERR;
		goto END;
	}

	/*
	 * Read the relevant eFuse revocation ID cache word directly.
	 */
	Row = RevokeId >> XLOADER_REVOCATION_ROW_SHIFT;
	Bit = RevokeId & XLOADER_REVOCATION_BIT_MASK;
	RowVal = Xil_In32(XLOADER_EFUSE_REVOCATION_ID_0_ADDR +
			(Row << XLOADER_WORD_LEN_SHIFT));
	Value = RowVal & ((u32)1U << Bit);
	ValueTmp = Xil_In32(XLOADER_EFUSE_REVOCATION_ID_0_ADDR +
			(Row << XLOADER_WORD_LEN_SHIFT)) & ((u32)1U << Bit);
	if ((Value != 0U) || (ValueTmp != 0U)) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
			"RevokeID 0x%x is revoked (row %u, bit %u, row value 0x%08x)\r\n",
			RevokeId, Row, Bit, RowVal);
		Status = XLOADER_REVOKEID_REVOKED_ERR;
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function validates image and partition RevokeID fields.
 *
 * @param	MhAddr		Meta header base address
 * @param	IhtPtr		Pointer to image header table
 * @param	PrtnHdr		Pointer to partition header array
 * @param	PrtnStartIdx	First partition index to validate
 *
 * @return
 *		 - XST_SUCCESS on success
 *		 - Error code on failure
 *
 ******************************************************************************/
static int XLoader_ValidateRevokeIds(u64 MhAddr,
	const XilLoader_ImgHdrTbl *IhtPtr, const XilLoader_PrtnHdr *PrtnHdr, u32 PrtnStartIdx)
{
	volatile int Status = XST_FAILURE;
	u32 AuthReg = 0U;
	u64 ImgHdrBase;
	u32 Idx;

	if ((IhtPtr == NULL) || (PrtnHdr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
		"Starting RevokeID validation: NoOfImgs = %u, NoOfPrtns = %u, PrtnStartIdx = %u\r\n",
		IhtPtr->NoOfImgs, IhtPtr->NoOfPrtns, PrtnStartIdx);

	Status = XLoader_ReadRtcfgSecureState(&AuthReg);
	if (Status != XST_SUCCESS) {
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
			"Read RTCFG secure-state failed, Status = 0x%x\r\n", Status);
		goto END;
	}

	ImgHdrBase = MhAddr + XLOADER_IHT_SIZE +
		((u64)IhtPtr->OptionalDataLen * XLOADER_WORD_LEN);

	/*
	 * Image-header RevokeID is always verified.
	 * Partition RevokeID is verified only when A-HWRoT is active.
	 */
	for (Idx = 0U; Idx < IhtPtr->NoOfImgs; ++Idx) {
		const XilLoader_ImgHdr *ImgHdrPtr =
			(const XilLoader_ImgHdr *)(UINTPTR)(ImgHdrBase +
			((u64)Idx * XLOADER_IH_SIZE));

		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
			"Verifying Image %u RevokeID = 0x%x\r\n", Idx,
			ImgHdrPtr->EncRevokeID);
		Status = XLoader_VerifyRevokeId(ImgHdrPtr->EncRevokeID);
		if (Status != XST_SUCCESS) {
			XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
				"RevokeID check failed for Image %u, Status = 0x%x\r\n",
				Idx, Status);
			goto END;
		}
		XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
			"RevokeID check passed for Image %u\r\n", Idx);
	}

	for (Idx = PrtnStartIdx; Idx < IhtPtr->NoOfPrtns; ++Idx) {
		if (AuthReg == XLOADER_RTCFG_SECURESTATE_AHWROT) {
			XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
				"Verifying Partition %u RevokeID = 0x%x\r\n",
				Idx, PrtnHdr[Idx].EncRevokeID);
			Status = XLoader_VerifyRevokeId(PrtnHdr[Idx].EncRevokeID);
			if (Status != XST_SUCCESS) {
				XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
					"RevokeID check failed for Partition %u, Status = 0x%x\r\n",
					Idx, Status);
				goto END;
			}
			XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
				"RevokeID check passed for Partition %u\r\n", Idx);
		}
		else {
			XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
				"Skipping Partition %u RevokeID check (A-HWRoT not active)\r\n", Idx);
			Status = XLoader_IsRtcfgSecureStateConsistent(AuthReg);
			if (Status != XST_SUCCESS) {
				XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
					"RTCFG secure-state check failed for Partition %u\r\n", Idx);
				goto END;
			}
			XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
				"RTCFG secure-state check passed for Partition %u\r\n", Idx);
		}
	}

	Status = XST_SUCCESS;
	XLoader_Client_Printf(XLOADER_DEBUG_GENERAL,
		"RevokeID validation completed successfully\r\n");

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function fills payload and sends IPI request for verifying
 * 		signature for the provided hash
 *
 * @param	InstancePtr	Pointer to XLoader_ClientInstance
 * @param	HBSignParamsAddr	Address where hash block sign parameters are present
 * @param	HBInstanceAddr	Address where hash block instance is present
 *
 * @return
 *		 - XST_SUCCESS on success
 *		 - XST_FAILURE on failure
 ******************************************************************************/
static int XLoader_VerifyAuthHashBlock(XLoader_ClientInstance *InstancePtr, u64 HBSignParamsAddr, u64 HBInstanceAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};

	/** Fill IPI Payload */
	XLOADER_PACK_PAYLOAD4(Payload, (u32)XLOADER_CMD_ID_DATA_AUTH,
			(u32)HBSignParamsAddr, (u32)(HBSignParamsAddr >> 32U),
			(u32)HBInstanceAddr, (u32)(HBInstanceAddr >> 32U));

	/* Send IPI request to XilLoader server to verify the Hash Block signature */
	Status = XLoader_ProcessMailbox(InstancePtr, Payload, PAYLOAD_ARG_CNT);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends an IPI request to configure one or more glitch detectors
 *		via XLOADER_CMD_ID_CONFIG_GLITCH_DETECTOR.
 *
 *		Each word in GdCfgWords packs four u8 fields (LSB first):
 *		- [7:0]   GD type (XLOADER_GD_TYPE_*)
 *		- [15:8]  depth
 *		- [23:16] width
 *		- [31:24] ref_voltage
 *
 *		Use XLoader_PackGlitchDetectorCfg() to build each word. Payload length is 1..5
 *
 * @param	InstancePtr	Pointer to the client instance.
 * @param	GdCfgWords	Array of packed glitch-detector config words.
 * @param	NumWords	Number of config words (XLOADER_GD_CFG_MIN_LEN .. MAX_LEN).
 *
 * @return
 *		 - XST_SUCCESS on success.
 *		 - XST_INVALID_PARAM on invalid arguments.
 *		 - Error code from PLM on failure.
 *
 *************************************************************************************************/
int XLoader_ConfigureGlitchDetector(XLoader_ClientInstance *InstancePtr, const u32 *GdCfgWords,
				    u32 NumWords)
{
	volatile int Status = (int)XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};

	/**
	* - Perform input parameter validation. Return error if parameters are invalid.
	*/
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL) || (GdCfgWords == NULL)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	/** Fill Payload: header length equals number of GD config words */
	switch (NumWords) {
	case XLOADER_GD_CFG_LEN_1:
		XLOADER_PACK_PAYLOAD1(Payload, (u32)XLOADER_CMD_ID_CONFIG_GLITCH_DETECTOR,
				      GdCfgWords[0U]);
		break;
	case XLOADER_GD_CFG_LEN_2:
		XLOADER_PACK_PAYLOAD2(Payload, (u32)XLOADER_CMD_ID_CONFIG_GLITCH_DETECTOR,
				      GdCfgWords[0U], GdCfgWords[1U]);
		break;
	case XLOADER_GD_CFG_LEN_3:
		XLOADER_PACK_PAYLOAD3(Payload, (u32)XLOADER_CMD_ID_CONFIG_GLITCH_DETECTOR,
				      GdCfgWords[0U], GdCfgWords[1U], GdCfgWords[2U]);
		break;
	case XLOADER_GD_CFG_LEN_4:
		XLOADER_PACK_PAYLOAD4(Payload, (u32)XLOADER_CMD_ID_CONFIG_GLITCH_DETECTOR,
				      GdCfgWords[0U], GdCfgWords[1U], GdCfgWords[2U],
				      GdCfgWords[3U]);
		break;
	case XLOADER_GD_CFG_LEN_5:
		XLOADER_PACK_PAYLOAD5(Payload, (u32)XLOADER_CMD_ID_CONFIG_GLITCH_DETECTOR,
				      GdCfgWords[0U], GdCfgWords[1U], GdCfgWords[2U],
				      GdCfgWords[3U], GdCfgWords[4U]);
		break;
	default:
		Status = (int)XST_INVALID_PARAM;
		break;
	}

	if (Status == (int)XST_INVALID_PARAM) {
		goto END;
	}

	/** Send request using generic API */
	Status = XLoader_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends an IPI request to get the destination load
 *		address of the tlbin partition (partition type 0x06000000)
 *		cached by the PLM during PDI load. ATF/TF-A uses this command.
 *
 * @param	InstancePtr	Pointer to the client instance.
 * @param	LoadAddr	Pointer to store the 64-bit destination load address.
 *
 * @return
 *		 - XST_SUCCESS on success.
 *		 - XST_INVALID_PARAM if parameters are invalid.
 *		 - XLOADER_ERR_NO_VALID_TLBIN from the PLM if no valid TL bin
 *		   has been loaded.
 *
 **************************************************************************************************/
int XLoader_GetTlbinLoadAddr(XLoader_ClientInstance *InstancePtr, u64 *LoadAddr)
{
	volatile int Status = (int)XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	if ((InstancePtr == NULL) || (LoadAddr == NULL)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	XLOADER_PACK_PAYLOAD0(Payload, (u32)XLOADER_CMD_ID_GET_TLBIN_LOAD_ADDR);

	Status = XLoader_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT,
			Response, (u32)RESPONSE_ARG_CNT);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	*LoadAddr = (((u64)Response[0U] << XLOADER_ADDR_HIGH_SHIFT) | (u64)Response[1U]);

END:
	return Status;
}

/** @} end of xloader_client_apis group */
