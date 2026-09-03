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
#include "xecc.h"
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

/************************************ Function Prototypes ****************************************/
static s32 XOcp_AesCompute(XAsufw_Dma *DmaPtr, u64 IvAddr, u64 InAddr, u64 OutAddr);
static s32 XOcp_EncryptUdePvtKey(XAsufw_Dma *DmaPtr, u32 UdeKeyId, const u8 *PvtKey,
				 u64 EncPvtKeyAddr);
static s32 XOcp_GenerateUdeSelfSignedCert(XAsufw_Dma *DmaPtr, const u8 *PubKey, const u8 *PvtKey,
					  u64 CertBufAddr, u32 CertBufMaxSize, u32 *CertSize);
static s32 XOcp_DecryptPvtKey(XAsufw_Dma *DmaPtr, u32 UdeUserKeyAddr, u8* Iv, u8* UdeDecPvtKey);
static s32 XOcp_GetActiveUdeKeyInfo(u32 *UdeUserKeyPtr, u8 *IvIncValPtr);
static s32 XOcp_BuildUdeResponseData(XAsufw_Dma *DmaPtr, const u8 *UdeDecPvtKey,
				     XAsu_OcpUdeResponse *OcpUdeResp,
				     const XAsu_OcpUdeParams *OcpUdeParamsPtr);
static s32 XOcp_PrepareUdeDecKey(XAsufw_Dma *DmaPtr, u8 *UdeKekIv, u8 *UdeDecPvtKey);
static s32 XOcp_ProcessUdeResponse(XAsufw_Dma *DmaPtr, XAsu_OcpUdeResponse *OcpUdeResp,
				   const XAsu_OcpUdeParams *OcpUdeParamsPtr);

/*************************************************************************************************/
/**
 * @brief	This function generates a UDE ECC NIST P-384 key pair inside the ASU, encrypts the
 *		private key with the PUF KEK, and generates a self-signed X.509 certificate over the
 *		generated public key. The private key is used only to compute the encrypted private key
 *		and to sign the certificate, and is zeroized before returning; it never leaves the ASU.
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
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
s32 XOcp_EncryptUdeKeys(XAsufw_Dma *DmaPtr, u32 UdeKeyId, u8 *EncPvtKey, u64 CertBufAddr,
			u32 CertBufMaxSize, u32 *CertSize)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XFih_Var XFihUde = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	u8 PvtKey[XASU_OCP_UDE_KEY_SIZE_IN_BYTES] = {0U};
	u8 PubKey[XASU_OCP_UDE_PUB_KEY_SIZE_IN_BYTES] = {0U};

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (EncPvtKey == NULL) || (CertBufAddr == 0U) || (CertSize == NULL) ||
	    (UdeKeyId > XASU_OCP_UDE_USER_KEY_2_ID)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	/** Generate the UDE private key inside the ASU. */
	Status = XRsa_EccGeneratePvtKey(XASU_ECC_NIST_P384, XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES,
				PvtKey, NULL, 0U);
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
	Status = XOcp_GenerateUdeSelfSignedCert(DmaPtr, PubKey, PvtKey, CertBufAddr,
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
	if (Status != XST_SUCCESS) {
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
 *	- Error codes from XOcp_GetUserX509Cert, if certificate generation fails.
 *	- Error code from XAsufw_UpdateBufStatus, if certificate working data zeroization fails.
 *
 *************************************************************************************************/
static s32 XOcp_GenerateUdeSelfSignedCert(XAsufw_Dma *DmaPtr, const u8 *PubKey, const u8 *PvtKey,
					  u64 CertBufAddr, u32 CertBufMaxSize, u32 *CertSize)
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
	if ((DmaPtr == NULL) || (PubKey == NULL) || (PvtKey == NULL) || (CertBufAddr == 0U) ||
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

	/** For a self-signed certificate the issuer name is identical to the subject name. */
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

	/** UDE certificate key usage: digitalSignature and keyCertSign. */
	CertFields->KeyUsage = XASU_X509_KEY_USAGE_DIGITAL_SIGNATURE |
			       XASU_X509_KEY_USAGE_KEY_CERT_SIGN;

	/**
	 * Generate the self-signed UDE CA certificate through the shared certificate generator.
	 * The generated UDE public key is both the subject and the issuer public key (self-signed)
	 * and the UDE private key signs the certificate. IsSelfSigned and IsCaCert are TRUE so the
	 * BasicConstraints (CA=TRUE, pathLenConstraint=0) extension is emitted.
	 */
	PlatData.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	PlatData.DmaPtr = DmaPtr;
	CertGenParams.CertFields = CertFields;
	CertGenParams.SubjectPubKey = PubKey;
	CertGenParams.SubjectPubKeyLen = XASU_OCP_UDE_PUB_KEY_SIZE_IN_BYTES;
	CertGenParams.IssuerPubKey = PubKey;
	CertGenParams.IssuerPubKeyLen = XASU_OCP_UDE_PUB_KEY_SIZE_IN_BYTES;
	CertGenParams.IssuerPvtKey = PvtKey;
	CertGenParams.CertBufAddr = CertBufAddr;
	CertGenParams.CertBufMaxSize = CertBufMaxSize;
	CertGenParams.IsSelfSigned = (u32)XASU_TRUE;
	CertGenParams.IsCaCert = (u32)XASU_TRUE;

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
 * @brief	This function generates the UDE response
 *
 * @param	DmaPtr		Pointer to the XAsufw_Dma instance.
 * @param	OcpUdeParamsPtr	Pointer to XAsu_OcpUdeParams structure.
 *
 * @return
 *	- XASUFW_SUCCESS, if the UDE response is generated successfully.
 *	- XASUFW_OCP_INVALID_PARAM, if any input param is invalid.
 *	- Error codes from XOcp_ProcessUdeResponse, if UDE response generation fails.
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
	XAsu_OcpUdeResponse *OcpUdeResp = NULL;

	/** Validate input parameters. */
	if ((DmaPtr == NULL) || (OcpUdeParamsPtr == NULL)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	if ((OcpUdeParamsPtr->OcpUdeResponseAddr == 0U) || (OcpUdeParamsPtr->NonceAddr == 0U)) {
		Status = XASUFW_OCP_INVALID_PARAM;
		goto END;
	}

	OcpUdeResp = (XAsu_OcpUdeResponse *)(UINTPTR)OcpUdeParamsPtr->OcpUdeResponseAddr;

	/** Perform core UDE response generation. */
	Status = XOcp_ProcessUdeResponse(DmaPtr, OcpUdeResp, OcpUdeParamsPtr);

	/**
	 * Measure and print the performance time for the OCP UDE response generation operation,
	 * if performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(XASU_MODULE_OCP_ID);

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
 *	- XASUFW_OCP_UDE_CHANGE_ENDIANNESS_ERROR, if endianness change fails.
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
		Status = XASUFW_OCP_UDE_CHANGE_ENDIANNESS_ERROR;
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
	XFIH_CALL_GOTO(XAsufw_PmcKeyTransfer, XFihUde, Status, END, &PufKekGenStatus);
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
	CREATE_VOLATILE(ClearStatus, XASUFW_FAILURE);
	XEcc *EccInstancePtr = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	XSha *ShaInstancePtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	XAsu_ShaOperationCmd ShaCmd;
	const XOcp_DeviceKeys *DevIkDataPtr = NULL;
	const u8 *DevIkPubKey = NULL;
	u8 HashBuf[XASU_SHA_384_HASH_LEN] = {0U};

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
	Status = XAsufw_DmaXfr(DmaPtr, (u64)(UINTPTR)OcpUdeParamsPtr->NonceAddr,
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

	/** Calculate hash of the fields DeviceId, Nonce and Measurement using SHA2-384. */
	ShaCmd.ShaMode = XASU_SHA_MODE_384;
	ShaCmd.DataAddr = (u64)(UINTPTR)OcpUdeResp->Ude.DeviceId;
	ShaCmd.DataSize = sizeof(XAsu_OcpUde);
	ShaCmd.HashAddr = (u64)(UINTPTR)HashBuf;
	ShaCmd.HashBufSize = XASU_SHA_384_HASH_LEN;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Digest(ShaInstancePtr, DmaPtr, &ShaCmd);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_SHA_DIGEST_FAIL);
		goto END;
	}

	/** Generate signature for UDE. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcc_GenerateSignature(EccInstancePtr, DmaPtr, XASU_ECC_NIST_P384,
			XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES, (u64)(UINTPTR)UdeDecPvtKey,
			NULL, (u64)(UINTPTR)HashBuf, XASU_SHA_384_HASH_LEN,
			(u64)(UINTPTR)OcpUdeResp->UdeSignatureR);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_SIGNATURE_GEN_FAIL);
	}

END:
	/** Zeroize the local hash buffer. */
	ClearStatus = Xil_SecureZeroize(HashBuf, XASU_SHA_384_HASH_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

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
 *	- XASUFW_OCP_UDE_KEY_DECRYPT_FAIL, with the XOcp_DecryptPvtKey error embedded, if UDE
 *	  private key decryption fails.
 *
 *************************************************************************************************/
static s32 XOcp_PrepareUdeDecKey(XAsufw_Dma *DmaPtr, u8 *UdeKekIv, u8 *UdeDecPvtKey)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 UdeUserKeyAddr = 0U;
	u8 IvIncVal = 0U;

	/** Check UDE key revocation status and get the key address and Iv increment value. */
	Status = XOcp_GetActiveUdeKeyInfo(&UdeUserKeyAddr, &IvIncVal);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_UDE_ALL_PVT_KEYS_REVOKED;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Return error if UDE encrypted key is not programmed into eFUSEs. */
	Status = XAsu_IsBufferNonZero((u8 *)(UINTPTR)UdeUserKeyAddr, XASU_OCP_UDE_KEY_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_UDE_KEY_NOT_PROGRAMMED;
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
	u8 UdeKekIv[XASU_OCP_UDE_IV_SIZE_IN_BYTES] = {0U};
	const u8 *IvPtr = (const u8*)(UINTPTR)XASUFW_PLM_RTCA_EFUSE_0_IV_ADDR;
	XEcc *EccInstancePtr = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);

	/** Copy IV from RTCA to local buffer. */
	Status = Xil_SMemCpy(UdeKekIv, XASU_OCP_UDE_IV_SIZE_IN_BYTES, IvPtr,
			XASU_OCP_UDE_IV_SIZE_IN_BYTES, XASU_OCP_UDE_IV_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Check if IV is non-zero. */
	Status = XAsu_IsBufferNonZero(UdeKekIv, XASU_OCP_UDE_IV_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_UDE_IV_IS_ZERO;
		goto END;
	}

	/** Prepare the decrypted UDE private key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_PrepareUdeDecKey(DmaPtr, UdeKekIv, UdeDecPvtKey);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_UDE_PREPARE_DEC_KEY_FAIL);
		goto END;
	}

	/** Generate the UDE public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XEcc_GeneratePublicKey(EccInstancePtr, DmaPtr, XASU_ECC_NIST_P384,
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

#endif /* XASU_OCP_ENABLE */
/** @} */
