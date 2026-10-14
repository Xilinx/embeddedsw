/**************************************************************************************************
* Copyright (c) 2025 - 2026, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xocp_ude.c
*
* This file contains the implementation of the interface functions for OCP UDE(Unique Device
* Endorsement) functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- -------- ---------------------------------------------------------------------------
* 1.0   yog  08/21/25 Initial release
*       yog  04/17/26 Updated code to zeroize UDE KEK immediately after use to avoid exposure in memory.
*       yog  08/24/26 Encrypt/decrypt UDE private keys directly with the PUF KEK,
*                     removing the derived UDE KEK (CMAC-KDF from eFUSE-0).
*       rmv  09/03/26 Used RSA core ECC APIs for ECC operations.
*       vm   09/15/26 Added UDE private key recovery helper, the UDE data sign and verify APIs,
*                     the cached UDE public key with the invalidation provisioning calls and the
*                     query for UDE key availability
*
* </pre>
*
**************************************************************************************************/
/**
 * @addtogroup xocp_ude_server_apis OCP UDE server APIs
 * @{
 */
/************************************** Include Files ********************************************/
#include "xasufw_hw.h"
#include "xasufw_status.h"
#include "xasufw_trnghandler.h"
#include "xasufw_util.h"
#include "xocp_ude.h"
#include "xocp.h"
#include "xaes.h"
#include "xasufw_keytransfer.h"
#include "xsha.h"
#include "xsha_hw.h"
#include "xasu_generic.h"
#include "xasu_def.h"
#include "xil_sutil.h"
#include "xasufw_memory.h"
#include "xrsa_ecc.h"
#include "xasufw_perf.h"
#include "x509_cert.h"
#include "x509_asufw.h"

#ifdef XASU_OCP_ENABLE
/********************************** Constant Definitions *****************************************/

/************************************ Macro Definitions ******************************************/
#define XOCP_UDE0_IV_INC_VAL		(0x02U)	/**< UDE0 IV increment value */
#define XOCP_UDE1_IV_INC_VAL		(0x03U)	/**< UDE1 IV increment value */
#define XOCP_UDE2_IV_INC_VAL		(0x04U)	/**< UDE2 IV increment value */
#define XASU_OCP_UDE_TAG_SIZE_IN_BYTES	(16U)	/**< UDE tag size in bytes */

/* UDE self-signed X.509 certificate distinguished name and validity constants. */
#define XOCP_UDE_CERT_ORG_NAME_LEN	(3U)	/**< Length of the UDE certificate organization
						name ("AMD") */
#define XOCP_UDE_CERT_DNA_STR_LEN	(EFUSE_CACHE_DNA_SIZE * 2U)	/**< Length of the Device
						DNA rendered as a hex string (2 chars per byte) */
#define XOCP_UDE_CERT_VALID_FROM_YEAR	(2025U)	/**< UDE certificate validity start year
						(250101000000Z) */
#define XOCP_UDE_CERT_VALID_TO_YEAR	(2045U)	/**< UDE certificate validity end year
						(451231235959Z) */
#define XOCP_UDE_CERT_MONTH_JAN		(1U)	/**< January (validity NotBefore month) */
#define XOCP_UDE_CERT_MONTH_DEC		(12U)	/**< December (validity NotAfter month) */
#define XOCP_UDE_CERT_DAY_FIRST		(1U)	/**< First day of the month */
#define XOCP_UDE_CERT_DAY_LAST		(31U)	/**< Last day of December */
#define XOCP_UDE_CERT_HOUR_LAST		(23U)	/**< Last hour of the day */
#define XOCP_UDE_CERT_MIN_LAST		(59U)	/**< Last minute of the hour */
#define XOCP_UDE_CERT_SEC_LAST		(59U)	/**< Last second of the minute */
#define XOCP_NIBBLE_MASK		(0xFU)	/**< Mask to extract a nibble */
#define XOCP_NIBBLE_SHIFT		(4U)	/**< Shift to extract the high nibble of a byte */

/************************************ Type Definitions *******************************************/

/********************************** Variable Definitions *****************************************/
static u8 UdePubKeyCache[XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES];
					/**< UDE public key of the slot the cache was filled from */
static u32 UdePubKeyCacheAddr;		/**< Encrypted key address naming that slot. It is an eFUSE
					     cache register of the selected slot, so it is never zero
					     and zero marks the cache as empty. */

/************************************ Function Prototypes ****************************************/
static s32 XOcp_AesCompute(XAsufw_Dma *DmaPtr, u64 IvAddr, u64 InAddr, u64 OutAddr);
static s32 XOcp_EncryptUdePvtKey(XAsufw_Dma *DmaPtr, u32 UdeKeyId, const u8 *PvtKey,
				 u64 EncPvtKeyAddr);
static s32 XOcp_GenerateUdeSelfSignedCert(XAsufw_Dma *DmaPtr, XSha *ShaPtr, const u8 *PubKey,
					  const u8 *PvtKey, u64 CertBufAddr, u32 CertBufMaxSize,
					  u32 *CertSize);
static s32 XOcp_BuildUdeX509Cert(XAsufw_Dma *DmaPtr, XSha *ShaPtr, const u8 *SubjectPubKey,
				 const u8 *IssuerPubKey, const u8 *IssuerPvtKey, u32 KeyUsage,
				 u32 IsSelfSigned, u32 IsCaCert, u64 CertBufAddr,
				 u32 CertBufMaxSize, u32 *CertSize);
static s32 XOcp_DecryptPvtKey(XAsufw_Dma *DmaPtr, u32 UdeUserKeyAddr, u8* Iv, u8* UdeDecPvtKey);
static s32 XOcp_GetActiveUdeKeyInfo(u32 *UdeUserKeyPtr, u8 *IvIncValPtr);
static s32 XOcp_ResolveUdeUserKeyAddr(u32 *UdeUserKeyPtr, u8 *IvIncValPtr);
static s32 XOcp_BuildUdeResponseData(XAsufw_Dma *DmaPtr, const u8 *UdeDecPvtKey,
				     XAsu_OcpUdeResponse *OcpUdeResp,
				     const XAsu_OcpUdeParams *OcpUdeParamsPtr);
static s32 XOcp_PrepareUdeDecKey(XAsufw_Dma *DmaPtr, u8 *UdeKekIv, u8 *UdeDecPvtKey);
static s32 XOcp_GetUdePvtKey(XAsufw_Dma *DmaPtr, u8 *UdeDecPvtKey);
static s32 XOcp_GetUdePubKey(XAsufw_Dma *DmaPtr, u8 *UdePubKey);
static s32 XOcp_ProcessUdeResponse(XAsufw_Dma *DmaPtr, XAsu_OcpUdeResponse *OcpUdeResp,
				   const XAsu_OcpUdeParams *OcpUdeParamsPtr);
static s32 XOcp_UdeHashData(XAsufw_Dma *DmaPtr, const u8 *Data, u32 DataLen, u8 *HashBuf);
static s32 XOcp_UdeSignDataWithKey(XAsufw_Dma *DmaPtr, const u8 *UdeDecPvtKey, const u8 *Data,
				   u32 DataLen, u8 *SignatureOut);

/*************************************************************************************************/
/**
 * @brief	This function generates a UDE ECC NIST P-384 key pair inside the ASU, encrypts the
 *		private key with the PUF KEK, and generates a self-signed X.509 certificate over the
 *		generated public key. The private key is used only to compute the encrypted private key
 *		and to sign the certificate, and is zeroized before returning; it never leaves the ASU.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaPtr		Pointer to the SHA3 (XSHA_1) instance used to hash the certificate.
 * @param	UdeKeyId	UDE key ID (XASU_OCP_UDE_USER_KEY_0/1/2_ID) that selects the PUF KEK
 *				IV increment value used for encryption.
 * @param	EncPvtKey	Pointer to the buffer that receives the encrypted UDE private key
 *				(XASU_OCP_UDE_KEY_SIZE_IN_BYTES bytes).
 * @param	CertBufAddr	64-bit address of the buffer that receives the DER encoded UDE
 *				self-signed X.509 certificate.
 * @param	CertBufMaxSize	Size in bytes of the buffer at CertBufAddr.
 * @param	CertSize	Pointer to a variable that receives the actual size in bytes of the
 *				generated certificate.
 *
 * @return
 *	- XASUFW_SUCCESS, if the UDE key pair, the encrypted private key and the certificate are
 *	  generated successfully.
 *	- XASUFW_OCP_INVALID_PARAM, if any input param is invalid.
 *	- XASUFW_OCP_UDE_PVT_KEY_GEN_FAIL, if UDE private key generation fails.
 *	- XASUFW_OCP_UDE_PUBLIC_KEY_GEN_FAIL, with the XRsa_EccGeneratePubKey error embedded, if
 *	  UDE public key derivation fails.
 *	- Error code from XOcp_EncryptUdePvtKey, if encrypting the UDE private key with the PUF
 *	  KEK fails.
 *	- Error code from XOcp_GenerateUdeSelfSignedCert, if UDE certificate generation fails.
 *	- Error code from XAsufw_UpdateBufStatus, if local buffer zeroization fails.
 *
 *************************************************************************************************/
s32 XOcp_EncryptUdeKeys(XAsufw_Dma *DmaPtr, XSha *ShaPtr, u32 UdeKeyId, u8 *EncPvtKey,
			u64 CertBufAddr, u32 CertBufMaxSize, u32 *CertSize)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihUde = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 PvtKey[XASU_OCP_UDE_KEY_SIZE_IN_BYTES] = {0U};
	u8 PubKey[XASU_OCP_UDE_PUB_KEY_SIZE_IN_BYTES] = {0U};

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (ShaPtr == NULL) || (EncPvtKey == NULL) || (CertBufAddr == 0U) ||
	    (CertSize == NULL) || (UdeKeyId > XASU_OCP_UDE_USER_KEY_2_ID)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Generate the UDE private key inside the ASU. */
	Status = XRsa_EccGeneratePvtKey(DmaPtr, XASU_ECC_NIST_P384,
				XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES, PvtKey, NULL, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_UDE_PVT_KEY_GEN_FAIL;
		goto END_CLR;
	}

	/** Derive the UDE public key from the generated private key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccGeneratePubKey(DmaPtr, XASU_ECC_NIST_P384,
				XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES, (u64)(UINTPTR)PvtKey,
				(u64)(UINTPTR)PubKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_PUBLIC_KEY_GEN_FAIL);
		goto END_CLR;
	}

	/** Encrypt the generated UDE private key with the PUF KEK. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_EncryptUdePvtKey(DmaPtr, UdeKeyId, PvtKey, (u64)(UINTPTR)EncPvtKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_PVT_KEY_ENCRYPT_FAIL);
		goto END_CLR;
	}

	/**
	 * Generate the self-signed X.509 certificate over the generated public key. The UDE
	 * private key signs the certificate while it is still available in the local buffer.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_GenerateUdeSelfSignedCert(DmaPtr, ShaPtr, PubKey, PvtKey, CertBufAddr,
						CertBufMaxSize, CertSize);


END_CLR:
	/** Zeroize local buffers. */
	XFIH_CALL(Xil_SecureZeroize, XFihUde, ClearStatus, PvtKey,
					XASU_OCP_UDE_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	XFIH_CALL(Xil_SecureZeroize, XFihUde, ClearStatus, PubKey,
					XASU_OCP_UDE_PUB_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function encrypts a caller-provided UDE private key with the PUF KEK and writes
 *		the encrypted key to the provided address. The KEK IV is derived from the eFUSE-0 IV
 *		and incremented based on the UDE key ID, matching the decryption performed during UDE
 *		challenge response generation. This allows other flows (e.g. Provisioning) to reuse
 *		the exact UDE key encryption scheme without duplicating the KEK/AES logic.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	UdeKeyId	UDE key ID (XASU_OCP_UDE_USER_KEY_0/1/2_ID) that selects the KEK IV
 *				increment value.
 * @param	PvtKey		Pointer to the plaintext UDE private key
 *				(XASU_OCP_UDE_KEY_SIZE_IN_BYTES bytes).
 * @param	EncPvtKeyAddr	Address of the buffer that receives the encrypted UDE private key.
 *
 * @return
 *	- XASUFW_SUCCESS, if UDE key encryption is successful.
 *	- XASUFW_OCP_INVALID_PARAM, if any input param is invalid.
 *	- XASUFW_MEM_COPY_FAIL, if the IV copy operation fails.
 *	- XASUFW_OCP_UDE_IV_IS_ZERO, if the eFUSE-0 IV is zero.
 *	- XASUFW_OCP_UDE_AES_COMPUTE_FAIL, if UDE key encryption operation fails.
 *
 *************************************************************************************************/
static s32 XOcp_EncryptUdePvtKey(XAsufw_Dma *DmaPtr, u32 UdeKeyId, const u8 *PvtKey,
				 u64 EncPvtKeyAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const u8 *IvPtr = (const u8*)(UINTPTR)XASUFW_PLM_RTCA_EFUSE_0_IV_ADDR;
	u8 UdeKekIv[XASU_OCP_UDE_IV_SIZE_IN_BYTES] = {0U};
	u8 IvIncVal = 0U;

	/** Copy IV from RTCA to local buffer. */
	Status = Xil_SMemCpy(UdeKekIv, XASU_OCP_UDE_IV_SIZE_IN_BYTES, IvPtr,
			XASU_OCP_UDE_IV_SIZE_IN_BYTES, XASU_OCP_UDE_IV_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Check if IV is non-zero. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_IsBufferNonZero(UdeKekIv, XASU_OCP_UDE_IV_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_UDE_IV_IS_ZERO;
		goto END;
	}
	/**
	 * Based on UDE key ID, update the KEK IV with the offset to the local buffer.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_GetUdeSlotInfo(UdeKeyId, NULL, &IvIncVal);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_GET_SLOT_INFO_FAIL);
		goto END;
	}
	Xil_IncrementBuffer(UdeKekIv, XASU_OCP_UDE_IV_SIZE_IN_BYTES, IvIncVal);

	/** Encrypt UDE private key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_AesCompute(DmaPtr, (u64)(UINTPTR)UdeKekIv, (u64)(UINTPTR)PvtKey,
			EncPvtKeyAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_AES_COMPUTE_FAIL);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs the certificate-field setup shared by the UDE self-signed
 *		certificate and the UDE-signed provisioning key certificates and generates the
 *		certificate through the shared X.509 generator. It builds the subject distinguished
 *		name (organization "AMD" and common name set to the Device DNA hex string), copies
 *		the subject name to the issuer name, sets the fixed validity period and the supplied
 *		key usage, populates the certificate generation parameters with the supplied subject
 *		and issuer keys and the self-signed/CA flags, and zeroizes the working data before
 *		return.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaPtr		Pointer to the SHA3 (XSHA_1) instance used to hash the certificate.
 * @param	SubjectPubKey	Uncompressed (X || Y) subject public key
 *				(XASU_OCP_UDE_PUB_KEY_SIZE_IN_BYTES bytes).
 * @param	IssuerPubKey	Uncompressed (X || Y) issuer public key
 *				(XASU_OCP_UDE_PUB_KEY_SIZE_IN_BYTES bytes).
 * @param	IssuerPvtKey	Issuer private key that signs the certificate.
 * @param	KeyUsage	X.509 key usage bit mask placed in the certificate.
 * @param	IsSelfSigned	XASU_TRUE if the certificate is self-signed, XASU_FALSE otherwise.
 * @param	IsCaCert	XASU_TRUE to emit the BasicConstraints (CA) extension, XASU_FALSE
 *				otherwise.
 * @param	CertBufAddr	64-bit address of the buffer that receives the DER encoded
 *				certificate.
 * @param	CertBufMaxSize	Size in bytes of the buffer at CertBufAddr.
 * @param	CertSize	Pointer to a variable that receives the actual size in bytes of the
 *				generated certificate.
 *
 * @return
 *	- XASUFW_SUCCESS, if the certificate is generated and copied successfully.
 *	- XASUFW_OCP_INVALID_PARAM, if any input param is invalid.
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if clearing the certificate fields fails.
 *	- XASUFW_MEM_COPY_FAIL, if copying the subject name to the issuer name fails.
 *	- XASUFW_OCP_USER_X509_CERT_GEN_FAIL, if certificate generation fails.
 *	- Error code from XAsufw_UpdateBufStatus, if certificate working data zeroization fails.
 *
 *************************************************************************************************/
static s32 XOcp_BuildUdeX509Cert(XAsufw_Dma *DmaPtr, XSha *ShaPtr, const u8 *SubjectPubKey,
				 const u8 *IssuerPubKey, const u8 *IssuerPvtKey, u32 KeyUsage,
				 u32 IsSelfSigned, u32 IsCaCert, u64 CertBufAddr,
				 u32 CertBufMaxSize, u32 *CertSize)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihClr = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XOcp_UserCertData *CertData = XOcp_GetUserCertData();
	XAsu_X509CertFields *CertFields = &CertData->UserCfg.CertCfg;
	XOcp_X509CertGenParams CertGenParams;
	X509_PlatData PlatData;
	const u8 *DnaPtr = (const u8 *)(UINTPTR)EFUSE_CACHE_DNA_0;
	static const char XOcp_HexLut[] = "0123456789ABCDEF";
	u32 Index;

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (ShaPtr == NULL) || (SubjectPubKey == NULL) ||
	    (IssuerPubKey == NULL) || (IssuerPvtKey == NULL) || (CertBufAddr == 0U) ||
	    (CertSize == NULL)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Clear the certificate generation working data. */
	Status = Xil_SMemSet(CertData, sizeof(XOcp_UserCertData), 0U, sizeof(XOcp_UserCertData));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/**
	 * Build the subject distinguished name: organization name "AMD" and common name set to
	 * the Device DNA rendered as an uppercase hex string.
	 */
	CertFields->Subject.OrgName[0U] = (u8)'A';
	CertFields->Subject.OrgName[1U] = (u8)'M';
	CertFields->Subject.OrgName[2U] = (u8)'D';
	CertFields->Subject.OrgNameLen = XOCP_UDE_CERT_ORG_NAME_LEN;
	for (Index = 0U; Index < EFUSE_CACHE_DNA_SIZE; Index++) {
		CertFields->Subject.CommonName[Index * 2U] =
			(u8)XOcp_HexLut[(DnaPtr[Index] >> XOCP_NIBBLE_SHIFT) & XOCP_NIBBLE_MASK];
		CertFields->Subject.CommonName[(Index * 2U) + 1U] =
			(u8)XOcp_HexLut[DnaPtr[Index] & XOCP_NIBBLE_MASK];
	}
	CertFields->Subject.CommonNameLen = XOCP_UDE_CERT_DNA_STR_LEN;

	/** The issuer distinguished name uses the same AMD/DNA name as the subject. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(&CertFields->Issuer, sizeof(CertFields->Issuer),
			     &CertFields->Subject, sizeof(CertFields->Subject),
			     sizeof(CertFields->Subject));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END_CLR;
	}

	/** Set the fixed validity period (250101000000Z .. 451231235959Z). */
	CertFields->Validity.NotBefore.Year = XOCP_UDE_CERT_VALID_FROM_YEAR;
	CertFields->Validity.NotBefore.Month = XOCP_UDE_CERT_MONTH_JAN;
	CertFields->Validity.NotBefore.Day = XOCP_UDE_CERT_DAY_FIRST;
	CertFields->Validity.NotBefore.Hour = 0U;
	CertFields->Validity.NotBefore.Minute = 0U;
	CertFields->Validity.NotBefore.Second = 0U;
	CertFields->Validity.NotAfter.Year = XOCP_UDE_CERT_VALID_TO_YEAR;
	CertFields->Validity.NotAfter.Month = XOCP_UDE_CERT_MONTH_DEC;
	CertFields->Validity.NotAfter.Day = XOCP_UDE_CERT_DAY_LAST;
	CertFields->Validity.NotAfter.Hour = XOCP_UDE_CERT_HOUR_LAST;
	CertFields->Validity.NotAfter.Minute = XOCP_UDE_CERT_MIN_LAST;
	CertFields->Validity.NotAfter.Second = XOCP_UDE_CERT_SEC_LAST;

	/** Set the caller supplied key usage. */
	CertFields->KeyUsage = KeyUsage;

	/**
	 * Generate the certificate through the shared X.509 generator using the supplied subject
	 * key, issuer key pair and self-signed/CA flags. ReturnStatus is the redundant success
	 * indicator set by the generator; it is checked and then reset.
	 */
	PlatData.ShaPtr = ShaPtr;
	PlatData.DmaPtr = DmaPtr;
	CertGenParams.CertFields = CertFields;
	CertGenParams.SubjectPubKey = SubjectPubKey;
	CertGenParams.SubjectPubKeyLen = XASU_OCP_UDE_PUB_KEY_SIZE_IN_BYTES;
	CertGenParams.IssuerPubKey = IssuerPubKey;
	CertGenParams.IssuerPubKeyLen = XASU_OCP_UDE_PUB_KEY_SIZE_IN_BYTES;
	CertGenParams.IssuerPvtKey = IssuerPvtKey;
	CertGenParams.CertBufAddr = CertBufAddr;
	CertGenParams.CertBufMaxSize = CertBufMaxSize;
	CertGenParams.IsSelfSigned = IsSelfSigned;
	CertGenParams.IsCaCert = IsCaCert;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_GetUserX509Cert(&CertGenParams, CertSize, (void *)(UINTPTR)&PlatData);
	if ((Status != XASUFW_SUCCESS) || (ReturnStatus != XASUFW_OCP_CERT_GENERATION_SUCCESS)) {
		Status = XAsufw_UpdateBufStatus(Status, XASUFW_OCP_USER_X509_CERT_GEN_FAIL);
	}
	ReturnStatus = XASUFW_FAILURE;

END_CLR:
	/** Zeroize the certificate generation working data. */
	XFIH_CALL(Xil_SecureZeroize, XFihClr, ClearStatus, (u8 *)(UINTPTR)CertData,
		  (u32)sizeof(XOcp_UserCertData));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates a self-signed X.509 certificate over the supplied UDE public
 *		key, signed with the supplied UDE private key. The certificate follows the provisioning
 *		certificate specification: version v3, SHA-384 signature hash, issuer and
 *		subject distinguished name with organization "AMD" and common name set to the Device
 *		DNA hex string, a fixed validity period (as there is no RTC), the generated public key,
 *		the BasicConstraints (CA=TRUE, pathLenConstraint=0) extension, and the digitalSignature
 *		and keyCertSign key usages. The serial number, subject key identifier and authority key
 *		identifier are derived from SHA-384 hashes by the X.509 generation layer.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaPtr		Pointer to the SHA3 (XSHA_1) instance used to hash the certificate.
 * @param	PubKey		Pointer to the UDE public key (XASU_OCP_UDE_PUB_KEY_SIZE_IN_BYTES
 *				bytes, X || Y). Used as both the subject and issuer public key.
 * @param	PvtKey		Pointer to the UDE private key used to sign the certificate.
 * @param	CertBufAddr	64-bit address of the buffer that receives the DER encoded
 *				certificate.
 * @param	CertBufMaxSize	Size in bytes of the buffer at CertBufAddr.
 * @param	CertSize	Pointer to a variable that receives the actual size in bytes of the
 *				generated certificate.
 *
 * @return
 *	- XASUFW_SUCCESS, if the certificate is generated and copied successfully.
 *	- XASUFW_OCP_INVALID_PARAM, if any input param is invalid.
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if clearing the certificate fields fails.
 *	- XASUFW_MEM_COPY_FAIL, if copying the subject name to the issuer name fails.
 *	- XASUFW_OCP_USER_X509_CERT_GEN_FAIL, if certificate generation fails.
 *	- Error code from XAsufw_UpdateBufStatus, if certificate working data zeroization fails.
 *
 *************************************************************************************************/
static s32 XOcp_GenerateUdeSelfSignedCert(XAsufw_Dma *DmaPtr, XSha *ShaPtr, const u8 *PubKey,
					  const u8 *PvtKey, u64 CertBufAddr, u32 CertBufMaxSize,
					  u32 *CertSize)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/**
	 * A self-signed UDE CA certificate uses the same UDE key pair as both subject and issuer,
	 * the digitalSignature and keyCertSign key usages, and IsSelfSigned/IsCaCert set to TRUE
	 * so the BasicConstraints (CA=TRUE, pathLenConstraint=0) extension is emitted.
	 */
	Status = XOcp_BuildUdeX509Cert(DmaPtr, ShaPtr, PubKey, PubKey, PvtKey,
			XASU_X509_KEY_USAGE_DIGITAL_SIGNATURE | XASU_X509_KEY_USAGE_KEY_CERT_SIGN,
			(u32)XASU_TRUE, (u32)XASU_TRUE, CertBufAddr, CertBufMaxSize, CertSize);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates the UDE response into an ASU local buffer and copies the
 *		completed response to the client supplied address using DMA.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	OcpUdeParamsPtr	Pointer to XAsu_OcpUdeParams structure.
 *
 * @return
 *	- XASUFW_SUCCESS, if the UDE response is generated successfully.
 *	- XASUFW_OCP_INVALID_PARAM, if any input param is invalid.
 *	- XASUFW_DMA_COPY_FAIL, if copying the response to the client buffer fails.
 *	- Error codes from XOcp_ProcessUdeResponse, if UDE response generation fails.
 *	- Error code from XAsufw_UpdateBufStatus, if local buffer zeroization fails.
 *
 *************************************************************************************************/
s32 XOcp_GenerateUdeResponse(XAsufw_Dma *DmaPtr, const XAsu_OcpUdeParams *OcpUdeParamsPtr)
{
	/**
	 * Capture the start time of the OCP UDE response generation operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START(XASU_MODULE_OCP_ID);

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihUde = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XAsu_OcpUdeResponse OcpUdeResp = {0U};

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (OcpUdeParamsPtr == NULL)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	if ((OcpUdeParamsPtr->OcpUdeResponseAddr == 0U) || (OcpUdeParamsPtr->NonceAddr == 0U)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Perform core UDE response generation. */
	Status = XOcp_ProcessUdeResponse(DmaPtr, &OcpUdeResp, OcpUdeParamsPtr);
	if (Status == XASUFW_SUCCESS) {
		/** DMA the generated response to the client buffer. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)&OcpUdeResp,
				       OcpUdeParamsPtr->OcpUdeResponseAddr,
				       (u32)sizeof(XAsu_OcpUdeResponse), 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
		}
	}

	/**
	 * Measure and print the performance time for the OCP UDE response generation operation,
	 * if performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_OCP_ID);

	/** Zeroize the local response buffer. */
	XFIH_CALL(Xil_SecureZeroize, XFihUde, ClearStatus, (u8 *)&OcpUdeResp,
		  (u32)sizeof(XAsu_OcpUdeResponse));
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function decodes a UDE key slot into the eFUSE cache address that holds its
 *		encrypted private key and the KEK IV increment value used for that slot. This is the
 *		single place where the UDE slot layout is decoded, so the encryption and decryption
 *		paths cannot drift apart.
 *
 * @param	UdeKeyId		UDE key ID (XASU_OCP_UDE_USER_KEY_0/1/2_ID).
 * @param	UdeUserKeyAddrPtr	Pointer to store the eFUSE cache address of the UDE key.
 *					Pass NULL when only the IV increment value is needed.
 * @param	IvIncValPtr		Pointer to store the KEK IV increment value. Pass NULL when
 *					only the eFUSE cache address is needed.
 *
 * @return
 *	- XASUFW_SUCCESS, if the UDE key ID is a supported slot.
 *	- XASUFW_OCP_INVALID_PARAM, if the UDE key ID is not a supported slot.
 *
 *************************************************************************************************/
s32 XOcp_GetUdeSlotInfo(u32 UdeKeyId, u32 *UdeUserKeyAddrPtr, u8 *IvIncValPtr)
{
	s32 Status = XASUFW_FAILURE;
	u32 UdeUserKeyAddr = 0U;
	u8 IvIncVal = 0U;

	/**
	 * UDE slot layout:
	 * | UDE key ID | eFUSE user key | KEK IV increment |
	 * |     0      |   USERKEY_0    |        2         |
	 * |     1      |   USERKEY_1    |        3         |
	 * |     2      |   USERKEY_2    |        4         |
	 */
	switch (UdeKeyId) {
		case XASU_OCP_UDE_USER_KEY_0_ID:
			UdeUserKeyAddr = EFUSE_CACHE_USERKEY_0_ADDR;
			IvIncVal = XOCP_UDE0_IV_INC_VAL;
			break;
		case XASU_OCP_UDE_USER_KEY_1_ID:
			UdeUserKeyAddr = EFUSE_CACHE_USERKEY_1_ADDR;
			IvIncVal = XOCP_UDE1_IV_INC_VAL;
			break;
		case XASU_OCP_UDE_USER_KEY_2_ID:
			UdeUserKeyAddr = EFUSE_CACHE_USERKEY_2_ADDR;
			IvIncVal = XOCP_UDE2_IV_INC_VAL;
			break;
		default:
			Status = XASUFW_OCP_INVALID_PARAM;
			goto END;
			break;
	}

	/** Return only the information the caller asked for. */
	if (UdeUserKeyAddrPtr != NULL) {
		*UdeUserKeyAddrPtr = UdeUserKeyAddr;
	}

	if (IvIncValPtr != NULL) {
		*IvIncValPtr = IvIncVal;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function decrypts the UDE encrypted private key using the PUF KEK.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	UdeUserKeyAddr	Address of UDE user key
 * @param	Iv		Pointer to the IV (Initialization Vector).
 * @param	UdeDecPvtKey	Pointer to store the decrypted key.
 *
 * @return
 *	- XASUFW_SUCCESS, if the UDE private key is decrypted successfully.
 *	- XASUFW_MEM_COPY_FAIL, if mem copy fails.
 *	- XASUFW_CHANGE_ENDIANNESS_ERROR, if endianness change fails.
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if mem set fails.
 *	- XASUFW_OCP_UDE_AES_COMPUTE_FAIL, with the XOcp_AesCompute error embedded, if the AES
 *	  operation fails.
 *	- XASUFW_OCP_UDE_KEY_DECRYPT_FAIL, if key decryption fails.
 *	- Error code from XAsufw_UpdateBufStatus, if local buffer zeroization fails.
 *
 *************************************************************************************************/
static s32 XOcp_DecryptPvtKey(XAsufw_Dma *DmaPtr, u32 UdeUserKeyAddr, u8* Iv, u8* UdeDecPvtKey)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 ZeroData[XASU_OCP_UDE_KEY_SIZE_IN_BYTES];
	u8 UdeEncPvtKey[XASU_OCP_UDE_KEY_SIZE_IN_BYTES] = {0U};
	volatile u32 Index = 0U;

	/** Copy the UDE user key data to the local buffer. */
	Status = Xil_SMemCpy(UdeEncPvtKey, XASU_OCP_UDE_KEY_SIZE_IN_BYTES,
			(u8*)(UINTPTR)UdeUserKeyAddr, XASU_OCP_UDE_KEY_SIZE_IN_BYTES,
			XASU_OCP_UDE_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Change endianness of UDE encrypted private key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_ChangeEndianness(UdeEncPvtKey, XASU_OCP_UDE_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_CHANGE_ENDIANNESS_ERROR;
		goto END;
	}

	/* Zeroize the zero data buffer. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SecureZeroize(ZeroData, XASU_OCP_UDE_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/**
	 * Decrypt the UDE encrypted private key.
	 *  - Encrypt the zero data with the PUF KEK and updated IV.
	 *  - XOR the resultant data with the UDE user key data (UDE encrypted private key).
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_AesCompute(DmaPtr, (u64)(UINTPTR)Iv, (u64)(UINTPTR)ZeroData,
			(u64)(UINTPTR)UdeDecPvtKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_AES_COMPUTE_FAIL);
		goto END;
	}
	for (Index = 0U; Index < XASU_OCP_UDE_KEY_SIZE_IN_BYTES; Index++) {
		UdeDecPvtKey[Index] ^= UdeEncPvtKey[Index];
	}

	if (Index != XASU_OCP_UDE_KEY_SIZE_IN_BYTES) {
		Status = XASUFW_OCP_UDE_KEY_DECRYPT_FAIL;
	}

END:
	/** Zeroize local buffers. */
	ClearStatus = Xil_SecureZeroize(UdeEncPvtKey, XASU_OCP_UDE_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs the AES-GCM operation used for UDE private key encryption
 *		and decryption, using the PUF KEK directly as the AES key source.
 *		- Ensures the PUF KEK is transferred from the PMC and selects it as the AES key
 *		  source through XAes_EncryptRedKey.
 *		- The operation is always performed in encrypt mode:
 *		    - Encryption: the UDE private key is encrypted directly.
 *		    - Decryption: the zero buffer is encrypted to reproduce the keystream, which the
 *		      caller XORs with the UDE encrypted private key.
 *		- Clears the PUF key and the eFUSE keys from the AES engine on every path.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	IvAddr		Pointer to the IV (Initialization Vector) address.
 * @param	InAddr		Pointer to the input data address.
 * @param	OutAddr		Pointer to the output data address.
 *
 * @return
 *	- XASUFW_SUCCESS, if AES operation is successful.
 *	- XASUFW_PUF_KEK_GEN_FAIL, if PUF KEK generation fails.
 *	- Error codes from XAsufw_PmcKeyTransfer, if the eFUSE and PUF key transfer from the PMC
 *	  fails.
 *	- Error codes from XAes_EncryptRedKey, if the AES operation fails.
 *	- Error code from XAes_KeyClear, if clearing the keys from the AES engine fails.
 *	- Error code from XAsufw_UpdateBufStatus, if local buffer zeroization fails.
 *
 *************************************************************************************************/
static s32 XOcp_AesCompute(XAsufw_Dma *DmaPtr, u64 IvAddr, u64 InAddr, u64 OutAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihUde = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	XAsu_AesParams AesParams = {0U};
	u8 TagBuf[XASU_OCP_UDE_TAG_SIZE_IN_BYTES] = {0U};
	XAes *AesInstancePtr = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);
	u32 PufKekGenStatus = XASUFW_FAILURE;

	/**
	 * Ensure the PUF KEK (and eFUSE keys) are transferred from the PMC before the PUF key is
	 * selected as the AES key source, and validate the PUF KEK generation status.
	 */
	XFIH_CALL_GOTO(XAsufw_PmcKeyTransfer, XFihUde, Status, END, &PufKekGenStatus,
			XASU_EFUSE_BLACK_KEY);
	if (PufKekGenStatus != (u32)XASUFW_SUCCESS) {
		Status = XASUFW_PUF_KEK_GEN_FAIL;
		goto END;
	}

	/**
	 * Encrypt with the PUF KEK using AES-GCM. XAes_EncryptRedKey selects the PUF key as the
	 * AES key source, so no key material is written or derived here. The same encrypt
	 * operation serves both encryption and (keystream-based) decryption of the UDE key.
	 */
	AesParams.EngineMode = (u8)XASU_AES_GCM_MODE;
	AesParams.OperationType = XASU_AES_ENCRYPT_OPERATION;
	AesParams.IvObj.IvAddr = (u64)(UINTPTR)IvAddr;
	AesParams.IvObj.IvLen = XASU_OCP_UDE_IV_SIZE_IN_BYTES;
	AesParams.InputDataAddr = (u64)(UINTPTR)InAddr;
	AesParams.OutputDataAddr = (u64)(UINTPTR)OutAddr;
	AesParams.DataLen = XASU_OCP_UDE_KEY_SIZE_IN_BYTES;
	AesParams.AadAddr = 0U;
	AesParams.AadLen = 0U;
	AesParams.TagAddr = (u64)(UINTPTR)TagBuf;
	AesParams.TagLen = XASU_OCP_UDE_TAG_SIZE_IN_BYTES;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAes_EncryptRedKey(AesInstancePtr, DmaPtr, &AesParams);

	/** Zeroize the local GCM tag buffer. */
	ClearStatus = Xil_SecureZeroize(TagBuf, XASU_OCP_UDE_TAG_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	/** Clear the PUF key and eFUSE keys from the AES engine after use. */
	XFIH_CALL(XAes_KeyClear, XFihUde, ClearStatus, AesInstancePtr, XAES_KEY_CLEAR_MULTI_KEY_FLAG |
		XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_EFUSE_KEY_0) |
		XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_EFUSE_KEY_RED_0) |
		XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_PUF_KEY) |
		XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_EFUSE_KEY_1) |
		XAES_KEY_SRC_TO_CLEAR_MASK(XASU_AES_EFUSE_KEY_RED_1));
	Status = XAsufw_UpdateErrorStatus(Status, ClearStatus);

	return Status;
}

/*************************************************************************************************/
 /**
 * @brief	This function checks the revocation status of UDE private keys and gets the active
 *		UDE key information (Key address and IV increment value).
 *
 * @param	UdeUserKeyPtr	Pointer to the variable that will hold active UDE user key address.
 * @param	IvIncValPtr	Pointer to the variable that will hold IV increment value.
 *
 * @return
 *	- XASUFW_SUCCESS, if active UDE key information is retrieved successfully.
 *	- XASUFW_OCP_UDE_ALL_PVT_KEYS_REVOKED, if all UDE private keys are revoked.
 *	- XASUFW_FAILURE, if no UDE key revocation bit reads back as unrevoked.
 *
 *************************************************************************************************/
static s32 XOcp_GetActiveUdeKeyInfo(u32 *UdeUserKeyPtr, u8 *IvIncValPtr)
{
	s32 Status = XASUFW_FAILURE;
	XFih_Var UdeFipsRegValue = XFih_VolatileAssignU32(EFUSE_CACHE_UDE_REVOKE_ALL_MASK);

	/** Get the UDE revocation bits from the FIPS register with FIH protection. */
	UdeFipsRegValue = XFih_VolatileAssignU32(Xil_In32(EFUSE_CACHE_UDE_FIPS_ADDRESS));

	/** If all keys are revoked, return error. */
	XFIH_IF_FAILOUT_WITH_MASK(UdeFipsRegValue, EFUSE_CACHE_UDE_REVOKE_ALL_MASK, ==,
	                          EFUSE_CACHE_UDE_REVOKE_ALL_MASK) {
		Status = XASUFW_OCP_UDE_ALL_PVT_KEYS_REVOKED;
		goto END;
	}

	/** Check UDE key 0 revocation status and get the UDE key information if not revoked. */
	XFIH_IF_FAILOUT_WITH_MASK(UdeFipsRegValue, EFUSE_CACHE_UDE_REVOKE_0_MASK, ==, 0U) {
		Status = XOcp_GetUdeSlotInfo(XASU_OCP_UDE_USER_KEY_0_ID, UdeUserKeyPtr,
					     IvIncValPtr);
		XFIH_GOTO(END);
	}

	/** If UDE key 0 is revoked, get the UDE key information for the UDE key 1 if not revoked. */
	XFIH_IF_FAILOUT_WITH_MASK(UdeFipsRegValue, EFUSE_CACHE_UDE_REVOKE_1_MASK, ==, 0U) {
		Status = XOcp_GetUdeSlotInfo(XASU_OCP_UDE_USER_KEY_1_ID, UdeUserKeyPtr,
					     IvIncValPtr);
		XFIH_GOTO(END);
	}

	/** If UDE key 0 and 1 are revoked, get the UDE key information for the UDE key 2 if not revoked. */
	XFIH_IF_FAILOUT_WITH_MASK(UdeFipsRegValue, EFUSE_CACHE_UDE_REVOKE_2_MASK, ==, 0U) {
		Status = XOcp_GetUdeSlotInfo(XASU_OCP_UDE_USER_KEY_2_ID, UdeUserKeyPtr,
					     IvIncValPtr);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function resolves the active UDE encrypted private key address from the
 *		eFUSE cache.
 *
 * @param	UdeUserKeyPtr	Pointer to the UDE encrypted key address. It may be NULL for a
 *				caller that only asks whether a key is available.
 * @param	IvIncValPtr	Pointer to the IV increment value for the active key. It may be
 *				NULL on the same terms.
 *
 * @return
 *	- XASUFW_SUCCESS, if an active UDE key slot is found and it holds a key.
 *	- XASUFW_OCP_UDE_KEY_NOT_PROGRAMMED, if the active slot is blank and no key is staged.
 *	- Error codes from XOcp_GetActiveUdeKeyInfo(), otherwise.
 *
 * @note	An un-revoked slot is not the same as a programmed one, so the slot contents are
 *		checked here rather than in the decrypt that would otherwise be the first caller
 *		to notice a blank slot.
 *
 *************************************************************************************************/
static s32 XOcp_ResolveUdeUserKeyAddr(u32 *UdeUserKeyPtr, u8 *IvIncValPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 UdeUserKeyAddr = 0U;
	u8 IvIncVal = 0U;

	Status = XOcp_GetActiveUdeKeyInfo(&UdeUserKeyAddr, &IvIncVal);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	/** The slot the revoke bits select still has to hold a key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_IsBufferNonZero((u8 *)(UINTPTR)UdeUserKeyAddr,
				      XASU_OCP_UDE_KEY_SIZE_IN_BYTES);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_OCP_UDE_KEY_NOT_PROGRAMMED;
		goto END;
	}

	if (UdeUserKeyPtr != NULL) {
		*UdeUserKeyPtr = UdeUserKeyAddr;
	}

	if (IvIncValPtr != NULL) {
		*IvIncValPtr = IvIncVal;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function computes the SHA-384 digest over the data covered by a UDE signature.
 *
 * @param	DmaPtr			Pointer to the XAsufw_Dma instance.
 * @param	Data			Pointer to the data covered by the signature.
 * @param	DataLen			Length of the data in bytes.
 * @param	HashBuf			Pointer to the buffer the digest is written to.
 *
 * @return
 *		- XASUFW_SUCCESS, if the digest is computed.
 *		- XASUFW_OCP_INVALID_PARAM, if the data does not fit the blocking DMA window.
 *		- Error codes from XSha_Digest(), if the digest fails.
 *
 * @note	The digest is fixed at SHA-384 to match the P-384 curve used by the UDE key.
 *		The caller names the failure, since the digest a sign covers and the digest a
 *		verify covers fail for different reasons.
 *
 *************************************************************************************************/
static s32 XOcp_UdeHashData(XAsufw_Dma *DmaPtr, const u8 *Data, u32 DataLen, u8 *HashBuf)
{
	XSha *ShaInstancePtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	XAsu_ShaOperationCmd ShaCmd = {0U};
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/**
	 * Hold the digest to the blocking DMA window. Above it XSha_Digest streams the data
	 * non-blocking and returns XASUFW_CMD_IN_PROGRESS, leaving its command stage waiting for
	 * a re-dispatch that these callers have no resume point for.
	 */
	if (DataLen > XASUFW_DMA_BLOCKING_SIZE) {
		Status = XASUFW_OCP_INVALID_PARAM;
	} else {
		ShaCmd.ShaMode = XASU_SHA_MODE_384;
		ShaCmd.DataAddr = (u64)(UINTPTR)Data;
		ShaCmd.DataSize = DataLen;
		ShaCmd.HashAddr = (u64)(UINTPTR)HashBuf;
		ShaCmd.HashBufSize = XASU_SHA_384_HASH_LEN;
		Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaCmd);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function signs data using a UDE private key supplied by the caller.
 *
 * @param	DmaPtr			Pointer to the XAsufw_Dma instance.
 * @param	UdeDecPvtKey		Pointer to the decrypted UDE private key.
 * @param	Data			Pointer to the data covered by the signature.
 * @param	DataLen			Length of the data in bytes.
 * @param	SignatureOut		Output buffer for the 96-byte P-384 signature.
 *
 * @return
 *		- XASUFW_SUCCESS, if signing is successful.
 *		- XASUFW_OCP_SHA_DIGEST_FAIL, if the data digest fails.
 *		- XASUFW_OCP_UDE_SIGNATURE_GEN_FAIL, if signature generation fails.
 *		- Error code from XAsufw_UpdateBufStatus, if local buffer zeroization fails.
 *
 * @note	The private key buffer stays owned by the caller, which lets a flow that has
 *		already recovered the key sign without recovering it again.
 *
 *************************************************************************************************/
static s32 XOcp_UdeSignDataWithKey(XAsufw_Dma *DmaPtr, const u8 *UdeDecPvtKey, const u8 *Data,
				   u32 DataLen, u8 *SignatureOut)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihUde = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 HashBuf[XASU_SHA_384_HASH_LEN] = {0U};

	/** Sign the digest of the data rather than the data, whatever its length. */
	Status = XOcp_UdeHashData(DmaPtr, Data, DataLen, HashBuf);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_SHA_DIGEST_FAIL);
		goto END;
	}

	/** Sign the digest with the caller's decrypted UDE private key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	XFIH_CALL(XRsa_EccGenerateSignature, XFihUde, Status, DmaPtr,
		  XASU_ECC_NIST_P384, XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES,
		  (u64)(UINTPTR)UdeDecPvtKey, NULL, (u64)(UINTPTR)HashBuf,
		  XASU_SHA_384_HASH_LEN, (u64)(UINTPTR)SignatureOut);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_SIGNATURE_GEN_FAIL);
	}

END:
	ClearStatus = Xil_SecureZeroize(HashBuf, XASU_SHA_384_HASH_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function signs data using the active UDE private key.
 *
 * @param	DmaPtr			Pointer to the XAsufw_Dma instance.
 * @param	Data			Pointer to the data covered by the signature.
 * @param	DataLen			Length of the data in bytes.
 * @param	SignatureOut		Output buffer for the 96-byte P-384 signature.
 *
 * @return
 *		- XASUFW_SUCCESS, if signing is successful.
 *		- XASUFW_OCP_INVALID_PARAM, if any pointer is NULL, the length is zero or the
 *		  length is above XASUFW_DMA_BLOCKING_SIZE.
 *		- XASUFW_OCP_UDE_PVT_KEY_FETCH_FAIL, with the recovery error embedded, if the
 *		  active UDE private key cannot be recovered.
 *		- XASUFW_OCP_SHA_DIGEST_FAIL, if the data digest fails.
 *		- XASUFW_OCP_UDE_SIGNATURE_GEN_FAIL, if signature generation fails.
 *
 * @note	The UDE key is the device endorsement key, so this signs on behalf of the device.
 *		It is restricted to firmware-constructed payloads and must not be reachable from
 *		a client command with caller-supplied data. The ECC work it drives lives in the
 *		overlay window, so the caller must already hold the overlay resource with the
 *		ECC image active.
 *
 *************************************************************************************************/
s32 XOcp_UdeSignData(XAsufw_Dma *DmaPtr, const u8 *Data, u32 DataLen, u8 *SignatureOut)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihUde = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 UdeDecPvtKey[XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES] = {0U};

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (Data == NULL) || (SignatureOut == NULL) || (DataLen == 0U) ||
	    (DataLen > XASUFW_DMA_BLOCKING_SIZE)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Recover the private key of the active UDE slot. */
	Status = XOcp_GetUdePvtKey(DmaPtr, UdeDecPvtKey);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_PVT_KEY_FETCH_FAIL);
		goto END;
	}

	/** Sign the data with the recovered key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_UdeSignDataWithKey(DmaPtr, UdeDecPvtKey, Data, DataLen, SignatureOut);

END:
	/** Zeroize the local private key buffer. */
	XFIH_CALL(Xil_SecureZeroize, XFihUde, ClearStatus, UdeDecPvtKey,
		  XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function verifies a UDE signature over data using the UDE public key.
 *
 * @param	DmaPtr			Pointer to the XAsufw_Dma instance.
 * @param	Data			Pointer to the data covered by the signature.
 * @param	DataLen			Length of the data in bytes.
 * @param	Signature		Pointer to the 96-byte P-384 signature.
 *
 * @return
 *		- XASUFW_SUCCESS, if signature verification succeeds.
 *		- XASUFW_OCP_INVALID_PARAM, if any pointer is NULL, the length is zero or the
 *		  length is above XASUFW_DMA_BLOCKING_SIZE.
 *		- XASUFW_OCP_UDE_PUB_KEY_FETCH_FAIL, with the fetch error embedded, if the active
 *		  UDE public key cannot be fetched.
 *		- XASUFW_OCP_SHA_DIGEST_FAIL, if the data digest fails.
 *		- Error codes from signature verification, if the signature does not match.
 *
 * @note	The device keeps no stored public key, so the first verification of a boot derives
 *		it from the private key and every one after that is served from the cache. A
 *		caller feeding the firmware signatures it knows will fail therefore cannot make it
 *		recover the private key more than once per key slot. The ECC work it drives lives
 *		in the overlay window, so the caller must already hold the overlay resource with
 *		the ECC image active.
 *
 *************************************************************************************************/
s32 XOcp_UdeVerifyData(XAsufw_Dma *DmaPtr, const u8 *Data, u32 DataLen, const u8 *Signature)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihUde = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 UdePubKey[XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES] = {0U};
	u8 HashBuf[XASU_SHA_384_HASH_LEN] = {0U};

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (Data == NULL) || (Signature == NULL) || (DataLen == 0U) ||
	    (DataLen > XASUFW_DMA_BLOCKING_SIZE)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Fetch the public key of the active UDE slot. */
	Status = XOcp_GetUdePubKey(DmaPtr, UdePubKey);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_PUB_KEY_FETCH_FAIL);
		goto END;
	}

	/** Compute the digest the signature covers. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_UdeHashData(DmaPtr, Data, DataLen, HashBuf);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_SHA_DIGEST_FAIL);
		goto END;
	}

	/** Verify the signature over that digest. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	XFIH_CALL(XRsa_EccVerifySignature, XFihUde, Status, DmaPtr,
		  XASU_ECC_NIST_P384, XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES,
		  (u64)(UINTPTR)UdePubKey, (u64)(UINTPTR)HashBuf,
		  XASU_SHA_384_HASH_LEN, (u64)(UINTPTR)Signature);

END:
	/** Zeroize the local digest buffer. */
	ClearStatus = Xil_SecureZeroize(HashBuf, XASU_SHA_384_HASH_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}

/*************************************************************************************************/
 /**
 * @brief	This function populates the UDE response fields (Device ID, Nonce, Measurement),
 *		calculates the hash over them, and generates the UDE signature.
 *
 * @param	DmaPtr			Pointer to the XAsufw_Dma instance.
 * @param	UdeDecPvtKey		Pointer to the decrypted UDE private key.
 * @param	OcpUdeResp		Pointer to the UDE response structure to populate.
 * @param	OcpUdeParamsPtr		Pointer to the XAsu_OcpUdeParams structure.
 *
 * @return
 *	- XASUFW_SUCCESS, if UDE response data population and signing is successful.
 *	- XASUFW_OCP_DEVICE_ID_CALC_FAIL, if device ID calculation fails.
 *	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if DevIk public key cannot be fetched from the
 *	  ASU key vault.
 *	- XASUFW_OCP_NONCE_UPDATE_FAIL, if nonce update fails.
 *	- XASUFW_OCP_MEASUREMENT_UPDATE_FAIL, if measurement update fails.
 *	- XASUFW_OCP_SHA_DIGEST_FAIL, if SHA digest generation fails.
 *	- XASUFW_OCP_UDE_SIGNATURE_GEN_FAIL, if UDE signature generation fails.
 *	- Error code from XAsufw_UpdateBufStatus, if local buffer zeroization fails.
 *
 *************************************************************************************************/
static s32 XOcp_BuildUdeResponseData(XAsufw_Dma *DmaPtr, const u8 *UdeDecPvtKey,
				     XAsu_OcpUdeResponse *OcpUdeResp,
				     const XAsu_OcpUdeParams *OcpUdeParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XSha *ShaInstancePtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	XAsu_ShaOperationCmd ShaCmd;
	const XOcp_DeviceKeys *DevIkDataPtr = NULL;
	const u8 *DevIkPubKey = NULL;

	/** Get the DevIk structure pointer. */
	DevIkDataPtr = XOcp_GetDevIk();

	/** Device ID update. */
	if (DevIkDataPtr->IsKeyReady == XASU_TRUE) {
		/**
		 * - If DevIk public key is generated, DeviceId is SHA2-384(DevIk_public_key).
		 */
		Status = XOcp_GetDevIkPublicKey(&DevIkPubKey);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}

		ShaCmd.ShaMode = XASU_SHA_MODE_384;
		ShaCmd.DataAddr = (u64)(UINTPTR)DevIkPubKey;
		ShaCmd.DataSize = XASUFW_DOUBLE_VALUE(XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES);
		ShaCmd.HashAddr = (u64)(UINTPTR)OcpUdeResp->Ude.DeviceId;
		ShaCmd.HashBufSize = XASU_SHA_384_HASH_LEN;
		Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaCmd);
	} else {
		/** - Else, DeviceId shall be all 0's. */
		Status = Xil_SMemSet((u8 *)OcpUdeResp->Ude.DeviceId,
				XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES, 0U,
				XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES);
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_DEVICE_ID_CALC_FAIL);
		goto END;
	}

	/** Nonce update. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, OcpUdeParamsPtr->NonceAddr,
			(u64)(UINTPTR)OcpUdeResp->Ude.Nonce, XASU_OCP_UDE_NONCE_SIZE_IN_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_NONCE_UPDATE_FAIL;
		goto END;
	}

	/** Measurement update. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)PMC_GLOBAL_HW_PCR_0_ADDR,
			(u64)(UINTPTR)OcpUdeResp->Ude.Measurement,
			XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_MEASUREMENT_UPDATE_FAIL;
		goto END;
	}

	/**
	 * Hash the fields DeviceId, Nonce and Measurement using SHA2-384 and sign the digest with
	 * the UDE private key already recovered by the caller.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_UdeSignDataWithKey(DmaPtr, UdeDecPvtKey,
					 (const u8 *)OcpUdeResp->Ude.DeviceId,
					 (u32)sizeof(XAsu_OcpUde),
					 (u8 *)OcpUdeResp->UdeSignatureR);

END:
	return Status;
}

/*************************************************************************************************/
 /**
 * @brief	This function prepares the decrypted UDE private key by copying the IV, checking
 *		key revocation status, validating eFUSE programming, and decrypting the key.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	UdeKekIv	Pointer to the IV buffer (pre-populated with BH IV).
 * @param	UdeDecPvtKey	Pointer to the buffer to store the decrypted private key.
 *
 * @return
 *	- XASUFW_SUCCESS, if UDE key preparation is successful.
 *	- XASUFW_OCP_UDE_ALL_PVT_KEYS_REVOKED, if all UDE private keys are revoked.
 *	- XASUFW_OCP_UDE_KEY_NOT_PROGRAMMED, if UDE key is not programmed into eFUSEs.
 *	- Error codes from XOcp_ResolveUdeUserKeyAddr(), if the active key address cannot be
 *	  resolved for another reason.
 *	- XASUFW_OCP_UDE_KEY_DECRYPT_FAIL, with the XOcp_DecryptPvtKey error embedded, if UDE
 *	  private key decryption fails.
 *
 *************************************************************************************************/
static s32 XOcp_PrepareUdeDecKey(XAsufw_Dma *DmaPtr, u8 *UdeKekIv, u8 *UdeDecPvtKey)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 UdeUserKeyAddr = 0U;
	u8 IvIncVal = 0U;

	/**
	 * Resolve the active UDE encrypted key address from the eFUSE cache. The resolve already
	 * separates a revoked slot from a blank one, so its status is returned as it stands.
	 */
	Status = XOcp_ResolveUdeUserKeyAddr(&UdeUserKeyAddr, &IvIncVal);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Increment IV based on the UDE revoke bits. */
	Xil_IncrementBuffer(UdeKekIv, XASU_OCP_UDE_IV_SIZE_IN_BYTES, IvIncVal);

	/** Decrypt the UDE encrypted private key from eFUSEs. */
	Status = XOcp_DecryptPvtKey(DmaPtr, UdeUserKeyAddr, UdeKekIv, UdeDecPvtKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_KEY_DECRYPT_FAIL);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function stages the eFUSE-0 KEK IV and recovers the active UDE private key.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	UdeDecPvtKey	Pointer to the buffer the decrypted private key is written to.
 *
 * @return
 *		- XASUFW_SUCCESS, if the UDE private key is recovered.
 *		- XASUFW_MEM_COPY_FAIL, if the IV copy fails.
 *		- XASUFW_OCP_UDE_IV_IS_ZERO, if the eFUSE-0 KEK IV is not programmed.
 *		- XASUFW_OCP_UDE_PREPARE_DEC_KEY_FAIL, if the UDE private key cannot be prepared.
 *
 * @note	The KEK is derived on demand inside the decrypt and never outlives it, so the
 *		caller owns nothing but the private key and has to zeroize it.
 *
 *************************************************************************************************/
static s32 XOcp_GetUdePvtKey(XAsufw_Dma *DmaPtr, u8 *UdeDecPvtKey)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihUde = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 UdeKekIv[XASU_OCP_UDE_IV_SIZE_IN_BYTES] = {0U};
	const u8 *IvPtr = (const u8*)(UINTPTR)XASUFW_PLM_RTCA_EFUSE_0_IV_ADDR;

	/** Copy the eFUSE-0 KEK IV from RTCA to the local buffer. */
	Status = Xil_SMemCpy(UdeKekIv, XASU_OCP_UDE_IV_SIZE_IN_BYTES, IvPtr,
			     XASU_OCP_UDE_IV_SIZE_IN_BYTES, XASU_OCP_UDE_IV_SIZE_IN_BYTES);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsu_IsBufferNonZero(UdeKekIv, XASU_OCP_UDE_IV_SIZE_IN_BYTES);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_OCP_UDE_IV_IS_ZERO;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	XFIH_CALL(XOcp_PrepareUdeDecKey, XFihUde, Status, DmaPtr, UdeKekIv, UdeDecPvtKey);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_PREPARE_DEC_KEY_FAIL);
	}

END:
	return Status;
}

/*************************************************************************************************/
 /**
 * @brief	This function performs the core UDE response generation operations including
 *		IV copy, key preparation, public key generation, and response data building.
 *
 * @param	DmaPtr			Pointer to the XAsufw_Dma instance.
 * @param	OcpUdeResp		Pointer to the UDE response structure to populate.
 * @param	OcpUdeParamsPtr		Pointer to the XAsu_OcpUdeParams structure.
 *
 * @return
 *	- XASUFW_SUCCESS, if UDE response generation is successful.
 *	- XASUFW_MEM_COPY_FAIL, if IV copy fails.
 *	- XASUFW_OCP_UDE_IV_IS_ZERO, if the IV read from RTCA is all zeros.
 *	- XASUFW_OCP_UDE_PREPARE_DEC_KEY_FAIL, with the XOcp_PrepareUdeDecKey error embedded, if
 *	  the UDE private key could not be prepared.
 *	- XASUFW_OCP_UDE_PUBLIC_KEY_GEN_FAIL, if UDE public key generation fails.
 *	- Error codes from XOcp_BuildUdeResponseData, if response data population or signing fails.
 *	- Error code from XAsufw_UpdateBufStatus, if local buffer zeroization fails.
 *
 *************************************************************************************************/
static s32 XOcp_ProcessUdeResponse(XAsufw_Dma *DmaPtr, XAsu_OcpUdeResponse *OcpUdeResp,
				   const XAsu_OcpUdeParams *OcpUdeParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihUde = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 UdeDecPvtKey[XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES] = {0U};

	/** Stage the eFUSE-0 KEK IV and recover the UDE private key. */
	Status = XOcp_GetUdePvtKey(DmaPtr, UdeDecPvtKey);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Generate the UDE public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccGeneratePubKey(DmaPtr, XASU_ECC_NIST_P384,
			XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES, (u64)(UINTPTR)UdeDecPvtKey,
			(u64)(UINTPTR)OcpUdeResp->UdePublicKeyX);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_PUBLIC_KEY_GEN_FAIL);
		goto END;
	}

	/** Populate UDE response fields and generate signature. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_BuildUdeResponseData(DmaPtr, UdeDecPvtKey, OcpUdeResp, OcpUdeParamsPtr);

END:
	/** Zeroize local buffer. */
	XFIH_CALL(Xil_SecureZeroize, XFihUde, ClearStatus, UdeDecPvtKey,
					XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks whether an active UDE private key is available.
 *
 * @return
 *		- XASUFW_SUCCESS, if an un-revoked UDE key slot holds a key.
 *		- XASUFW_OCP_UDE_KEY_NOT_PROGRAMMED, if the active slot is blank.
 *		- Error codes from UDE key lookup, if no slot is available.
 *
 *************************************************************************************************/
s32 XOcp_IsUdeAvailable(void)
{
	/** Resolve the active UDE key slot, which is what tells whether a key is available. */
	return XOcp_ResolveUdeUserKeyAddr(NULL, NULL);
}

/*************************************************************************************************/
/**
 * @brief	This function returns the public key of the active UDE key slot.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	UdePubKey	Output buffer for the 96-byte P-384 public key.
 *
 * @return
 *		- XASUFW_SUCCESS, if the public key is returned.
 *		- XASUFW_OCP_INVALID_PARAM, if any pointer is NULL.
 *		- XASUFW_OCP_UDE_PVT_KEY_FETCH_FAIL, with the recovery error embedded, if the
 *		  private key the derivation needs cannot be fetched.
 *		- XASUFW_OCP_UDE_PREPARE_DEC_KEY_FAIL, if the UDE private key cannot be prepared.
 *		- XASUFW_OCP_UDE_PUBLIC_KEY_GEN_FAIL, if public key generation fails.
 *		- XASUFW_MEM_COPY_FAIL, if the copy out of the cache fails.
 *
 * @note	The device stores no public key, so the first caller of a boot recovers the private
 *		key to derive it and every caller after that is served from the cache. A path that
 *		only checks a signature therefore recovers the private key at most once per key
 *		slot, so a caller feeding the firmware signatures it knows will fail cannot make it
 *		do so repeatedly. The cache is keyed on the encrypted key address, so a slot the
 *		revoke bits move to is derived again rather than answered from the previous slot.
 *		A slot written in the same boot keeps its address, so provisioning drops the cache
 *		through XOcp_InvalidateUdePubKeyCache() instead.
 *
 *************************************************************************************************/
static s32 XOcp_GetUdePubKey(XAsufw_Dma *DmaPtr, u8 *UdePubKey)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XFih_Var XFihUde = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u8 UdeDecPvtKey[XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES] = {0U};
	u32 UdeUserKeyAddr = 0U;
	u8 IvIncVal = 0U;

	if ((DmaPtr == NULL) || (UdePubKey == NULL)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** The slot the revoke bits select names the key the cache may answer for. */
	Status = XOcp_ResolveUdeUserKeyAddr(&UdeUserKeyAddr, &IvIncVal);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}

	if (UdePubKeyCacheAddr != UdeUserKeyAddr) {
		/**
		 * The tag is dropped before the cache is written, so a failure part way through
		 * the derivation cannot leave a tag naming a slot the contents no longer match.
		 */
		UdePubKeyCacheAddr = 0U;

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XOcp_GetUdePvtKey(DmaPtr, UdeDecPvtKey);
		if (XASUFW_SUCCESS != Status) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_OCP_UDE_PVT_KEY_FETCH_FAIL);
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XRsa_EccGeneratePubKey(DmaPtr, XASU_ECC_NIST_P384,
						XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES,
						(u64)(UINTPTR)UdeDecPvtKey,
						(u64)(UINTPTR)UdePubKeyCache);
		if (XASUFW_SUCCESS != Status) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_OCP_UDE_PUBLIC_KEY_GEN_FAIL);
			goto END;
		}

		UdePubKeyCacheAddr = UdeUserKeyAddr;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(UdePubKey, XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES, UdePubKeyCache,
			     XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES,
			     XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES);
	if (XASUFW_SUCCESS != Status) {
		Status = XASUFW_MEM_COPY_FAIL;
	}

END:
	XFIH_CALL(Xil_SecureZeroize, XFihUde, ClearStatus, UdeDecPvtKey,
		  XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function empties the UDE public key cache, so the next caller derives the
 *		public key again from the key its slot holds.
 *
 * @note	The cache is keyed on the address of the encrypted key, and that address stays the
 *		same when provisioning programs the slot it names. Provisioning calls this after a
 *		slot write so a key cached earlier in the boot is never served for the new key.
 *
 *************************************************************************************************/
void XOcp_InvalidateUdePubKeyCache(void)
{
	UdePubKeyCacheAddr = 0U;
}

/*************************************************************************************************/
/**
 * @brief	This function returns the active (non-revoked) UDE key pair: the decrypted UDE
 *		private key and the ECC NIST P-384 public key derived from it. The private key is
 *		decrypted with the PUF KEK inside the ASU and returned to the caller for signing;
 *		the caller must zeroize it as soon as it is no longer needed. This is used by the
 *		Device Provisioning Certificate flow to sign the provisioning certificates and the
 *		DPC container with the UDE identity.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	UdeDecPvtKey	Buffer that receives the decrypted UDE private key
 *				(XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES bytes). Caller zeroizes it.
 * @param	UdePubKey	Buffer that receives the uncompressed (X || Y) UDE public key
 *				(XASU_OCP_UDE_PUB_KEY_SIZE_IN_BYTES bytes).
 *
 * @return
 *	- XASUFW_SUCCESS, if the active UDE key pair is returned successfully.
 *	- XASUFW_OCP_INVALID_PARAM, if any input parameter is invalid.
 *	- XASUFW_MEM_COPY_FAIL, if copying the IV from RTCA fails.
 *	- XASUFW_OCP_UDE_IV_IS_ZERO, if the IV read from RTCA is all zeros.
 *	- XASUFW_OCP_UDE_PREPARE_DEC_KEY_FAIL, with the XOcp_PrepareUdeDecKey error embedded, if
 *	  the UDE private key could not be prepared.
 *	- XASUFW_OCP_UDE_PUBLIC_KEY_GEN_FAIL, if UDE public key generation fails.
 *
 *************************************************************************************************/
s32 XOcp_GetActiveUdeKeyPair(XAsufw_Dma *DmaPtr, u8 *UdeDecPvtKey, u8 *UdePubKey)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (UdeDecPvtKey == NULL) || (UdePubKey == NULL)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/**
	 * Decrypt the active UDE private key from eFUSEs. XOcp_GetUdePvtKey copies the KEK IV
	 * from RTCA, checks it is non-zero, and decrypts the key through XFIH_CALL so the key
	 * decrypt stays glitch protected.
	 */
	Status = XOcp_GetUdePvtKey(DmaPtr, UdeDecPvtKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_PVT_KEY_FETCH_FAIL);
		goto END;
	}

	/** Derive the UDE public key from the decrypted private key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XRsa_EccGeneratePubKey(DmaPtr, XASU_ECC_NIST_P384,
			XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES, (u64)(UINTPTR)UdeDecPvtKey,
			(u64)(UINTPTR)UdePubKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_PUBLIC_KEY_GEN_FAIL);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates a UDE-signed X.509 certificate over a provisioning public
 *		key. The certificate is issued (signed) by the UDE identity: the issuer public key
 *		and issuer private key are the active UDE key pair, and the subject public key is the
 *		provisioning authentication or wrapping public key. The certificate is neither
 *		self-signed nor a CA certificate, so no BasicConstraints extension is emitted.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	ShaPtr		Pointer to the SHA3 (XSHA_1) instance used to hash the certificate.
 * @param	SubjectPubKey	Uncompressed (X || Y) provisioning public key that is certified
 *				(XASU_OCP_UDE_PUB_KEY_SIZE_IN_BYTES bytes).
 * @param	IsForSigning	XASU_TRUE for the authentication (signing) key certificate,
 *				XASU_FALSE for the wrapping (ECIES key agreement) key certificate.
 * @param	UdePubKey	Uncompressed (X || Y) active UDE public key (issuer).
 * @param	UdePvtKey	Active UDE private key that signs the certificate (issuer).
 * @param	CertBufAddr	64-bit address of the buffer that receives the DER encoded
 *				certificate.
 * @param	CertBufMaxSize	Size in bytes of the buffer at CertBufAddr.
 * @param	CertSize	Pointer to a variable that receives the actual size in bytes of the
 *				generated certificate.
 *
 * @return
 *	- XASUFW_SUCCESS, if the certificate is generated and copied successfully.
 *	- XASUFW_OCP_INVALID_PARAM, if any input param is invalid.
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if clearing the certificate fields fails.
 *	- XASUFW_MEM_COPY_FAIL, if copying the subject name to the issuer name fails.
 *	- XASUFW_OCP_USER_X509_CERT_GEN_FAIL, if certificate generation fails.
 *	- Error code from XAsufw_UpdateBufStatus, if certificate working data zeroization fails.
 *
 *************************************************************************************************/
s32 XOcp_GenerateUdeSignedKeyCert(XAsufw_Dma *DmaPtr, XSha *ShaPtr, const u8 *SubjectPubKey,
				  u8 IsForSigning, const u8 *UdePubKey, const u8 *UdePvtKey,
				  u64 CertBufAddr, u32 CertBufMaxSize, u32 *CertSize)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 KeyUsage;

	/**
	 * Key usage depends on the provisioning key role: the authentication key certifies a
	 * signing key (digitalSignature); the wrapping key certifies an ECIES key agreement key
	 * (keyAgreement and dataEncipherment).
	 */
	if (IsForSigning == (u8)XASU_TRUE) {
		KeyUsage = XASU_X509_KEY_USAGE_DIGITAL_SIGNATURE;
	} else {
		KeyUsage = XASU_X509_KEY_USAGE_KEY_AGREEMENT |
			   XASU_X509_KEY_USAGE_DATA_ENCIPHERMENT;
	}

	/**
	 * A UDE-signed leaf certificate uses the provisioning key as the subject and the active
	 * UDE key pair as the issuer, with IsSelfSigned/IsCaCert set to FALSE so no
	 * BasicConstraints extension is emitted.
	 */
	Status = XOcp_BuildUdeX509Cert(DmaPtr, ShaPtr, SubjectPubKey, UdePubKey, UdePvtKey, KeyUsage,
			(u32)XASU_FALSE, (u32)XASU_FALSE, CertBufAddr, CertBufMaxSize, CertSize);

	return Status;
}

#endif /* XASU_OCP_ENABLE */
/** @} */
