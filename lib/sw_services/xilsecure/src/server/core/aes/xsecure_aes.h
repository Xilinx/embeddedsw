/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_aes.h
*
* This file contains AES hardware interface APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 4.0   vns  04/24/2019 Initial release
* 4.1   vns  08/06/2019 Added AES encryption APIs
* 4.2   kpt  01/07/2020 Removed Macro XSECURE_WORD_SIZE
*                       and added in xsecure_utils.h
*       vns  02/10/2020 Added DPA CM enable/disable function
*       rpo  04/02/2020 Added Crypto KAT APIs
*                       Moved AES error codes to xsecure_error.h
*       bvi  04/07/2020 Renamed csudma as pmcdma
* 4.3   ana  06/04/2020 Added NextBlkLen in Xsecure_Aes structure
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       am   09/24/2020 Resolved MISRA C violations
*       har  09/30/2020 Deprecated Family Key support
*       har  10/12/2020 Addressed security review comments
*       ana  10/15/2020 Updated doxygen tags
* 4.5   har  03/02/2021 Added prototype for XSecure_AesUpdateAad
* 4.6   har  07/14/2021 Fixed doxygen warnings
* 5.0   kpt  07/24/2022 Moved XSecure_AesDecryptKat into XSecure_Kat.c
*       kpt  08/19/2022 Added GMAC support
*       vss  05/16/2023 Fixed coverity warning NO_EFFECT
* 5.2   yog  07/10/2023 Added support of unaligned data sizes for Versal Net
*       mmd  07/11/2023 Included header file for crypto algorithm information
*       vss  07/14/2023 Added IsResourceBusy flag and IpiMask variable in Xsecure_Aes instance
*       kpt  07/20/2023 Renamed XSecure_AesDpaCmDecryptKat to XSecure_AesDpaCmDecryptData
*	    vss  09/07/2023 Reverted the fix for NO_EFFECT coverity warning
* 5.3   kpt  11/28/2023 Added XSECURE_AES_PUF_RED_EXPANDED_KEYS
* 5.4   yog  04/29/2024 Fixed doxygen grouping and doxygen warnings.
*	vss  10/23/2024 Removed AES duplicate code
*       vss  11/20/2024 Fix for data corruption of GCM tag when any other
*                       operation uses DMA0 after encrypt update.
*       pre  03/02/2025 Removed data context setting and resource busy functionality for AES
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_aes_server_apis XilSecure AES Server APIs
* @{
*/
#ifndef XSECURE_AES_H_
#define XSECURE_AES_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xpmcdma.h"
#include "xsecure_sss.h"
#include "xsecure_aesalginfo.h"

/************************** Constant Definitions *****************************/
/**
 * @cond xsecure_internal
 * @{
 */

#define XSECURE_SECURE_GCM_TAG_SIZE			(16U)
#define XSECURE_AES_KEY_SIZE_128BIT_WORDS		(4U)
#define XSECURE_AES_KEY_SIZE_128BIT_BYTES		(16U)
#define XSECURE_AES_KEY_SIZE_256BIT_WORDS		(8U)
#define XSECURE_AES_KEY_SIZE_256BIT_BYTES		(32U)

#define XSECURE_AES_TIMEOUT_MAX				(0x1FFFFU)

#define XSECURE_AES_NO_CFG_DST_DMA			(0xFFFFFFFFU)

#define XSECURE_AES_KEY_MASK_INDEX			(0xA0U)
#define XSECURE_AES_DMA_SIZE				(16U)
#define XSECURE_AES_DMA_LAST_WORD_ENABLE		(0x1U)
#define XSECURE_AES_DMA_LAST_WORD_DISABLE		(0x0U)
#define XSECURE_ENABLE_BYTE_SWAP		(0x1U)	/**< Enables data swap in AES */
#define XSECURE_DISABLE_BYTE_SWAP		(0x0U)	/**< Disables data swap in AES */

#define XSecure_AesDpaCmDecryptKat 		XSecure_AesDpaCmDecryptData
						/**< XSecure_AesDpaCmDecryptKat is deprecated to XSecure_AesDpaCmDecryptData */

/** @}
 * @endcond
 */

/**************************** Type Definitions *******************************/

/** Used for selecting the Key size of AES Core. */
typedef enum {
	XSECURE_AES_KEY_SIZE_128 = 0,	/**< Key Length = 32 bytes = 256 bits */
	XSECURE_AES_KEY_SIZE_256 = 2,	/**< Key Length = 16 bytes = 128 bits */
}XSecure_AesKeySize;

/**
 * @cond xsecure_internal
 * @{
 */

/** Used to know the state of AES core. */
typedef enum {
	XSECURE_AES_UNINITIALIZED,		/**< Uninitialized state */
	XSECURE_AES_INITIALIZED,		/**< Initialized state */
	XSECURE_AES_OPERATION_INITIALIZED, /**< Encrypt/Decrypt initialized */
	XSECURE_AES_UPDATE_IN_PROGRESS, /**< Encrypt/Decrypt update is in progress */
	XSECURE_AES_UPDATE_DONE /**< Encrypt/Decrypt update is in done */
} XSecure_AesState;

/**
 * The AES driver instance data structure. A pointer to an instance data
 * structure is passed around by functions to refer to a specific driver
 * instance.
 */
typedef struct {
	UINTPTR BaseAddress;	   /**< AES Base address */
	XPmcDma *PmcDmaPtr;	   /**< PMCDMA Instance Pointer */
	XSecure_Sss SssInstance;   /**< Secure stream switch instance */
	XSecure_AesState AesState; /**< Current Aes State  */
	XSecure_AesKeySrc KeySrc;  /**< Key Source */
	u32 NextBlkLen;		   /**< Next Block Length */
	u32 IsGmacEn;          /**< GMAC enable or disable */
	u32 DmaSwapEn;          /**< DMA byte swap enable/disable */
#ifdef VERSAL_NET
	u32 IsEcbEn;           /**< ECB mode enable or disable */
#endif
	u8 GcmTag[XSECURE_SECURE_GCM_TAG_SIZE]; /**< GCM tag */
	XSecure_AesOp OperationId;           /* Operation Id to select encrypt/decrypt operation */
} XSecure_Aes;

extern const XSecure_AesKeyLookup AesKeyLookupTbl[XSECURE_MAX_KEY_SOURCES];

/*****************************************************************************/
/**
 * @brief	This function returns AES ECB mode status
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 *
 * @return
 *		 - TRUE  If ECB mode is enabled
 *		 - FALSE  If ECB mode is not supported/disabled
 *
 *****************************************************************************/
static INLINE u32 XSecure_AesIsEcbModeEn(const XSecure_Aes *InstancePtr)
{
	u32 IsEcbModeEn = (u32)FALSE;

#ifdef VERSAL_NET
	IsEcbModeEn = InstancePtr->IsEcbEn;
#else
	(void)InstancePtr;
#endif

	return IsEcbModeEn;
}

/**
 * @}
 * @endcond
 */
/************************** Function Prototypes ******************************/
int XSecure_AesInitialize(XSecure_Aes *InstancePtr, XPmcDma *PmcDmaPtr);

int XSecure_AesSetDpaCm(const XSecure_Aes *InstancePtr, u32 DpaCmCfg);

int XSecure_AesKeyZero(const XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc);

int XSecure_AesWriteKey(const XSecure_Aes *InstancePtr,
	XSecure_AesKeySrc KeySrc, XSecure_AesKeySize KeySize, u64 KeyAddr);

int XSecure_AesKekDecrypt(const XSecure_Aes *InstancePtr,
	XSecure_AesKeySrc DecKeySrc,XSecure_AesKeySrc DstKeySrc, u64 IvAddr,
	XSecure_AesKeySize KeySize);

int XSecure_AesCfgKupKeyNIv(const XSecure_Aes *InstancePtr, u8 Config);

int XSecure_AesGetNxtBlkLen(const XSecure_Aes *InstancePtr, u32 *Size);

int XSecure_AesDecryptInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
	XSecure_AesKeySize KeySize, u64 IvAddr);

int XSecure_AesDecryptUpdate(XSecure_Aes *InstancePtr, u64 InDataAddr,
	u64 OutDataAddr, u32 Size, u8 IsLastChunk);
int XSecure_AesDecryptFinal(XSecure_Aes *InstancePtr, u64 GcmTagAddr);

int XSecure_AesDecryptData(XSecure_Aes *InstancePtr, u64 InDataAddr,
	u64 OutDataAddr, u32 Size, u64 GcmTagAddr);

int XSecure_AesEncryptInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
	XSecure_AesKeySize KeySize, u64 IvAddr);

int XSecure_AesEncryptUpdate(XSecure_Aes *InstancePtr, u64 InDataAddr,
	u64 OutDataAddr, u32 Size, u8 IsLastChunk);
int XSecure_AesEncryptFinal(XSecure_Aes *InstancePtr, u64 GcmTagAddr);

int XSecure_AesEncryptData(XSecure_Aes *InstancePtr, u64 InDataAddr,
	u64 OutDataAddr, u32 Size, u64 GcmTagAddr);

int XSecure_AesUpdateAad(XSecure_Aes *InstancePtr, u64 AadAddr, u32 AadSize);

int XSecure_AesUpdateAadAndValidate(XSecure_Aes *InstancePtr, u64 AadAddr, u32 AadSize, u64 GcmTagAddr);

int XSecure_AesGmacCfg(XSecure_Aes *InstancePtr, u32 IsGmacEn);

int XSecure_AesDpaCmDecryptData(const XSecure_Aes *AesInstance,
	const u32 *KeyPtr, const u32 *DataPtr, u32 *OutputPtr);

int XSecure_CfgSssAes(XPmcDma *DmaPtr, const XSecure_Sss *SssInstance);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_AES_H_ */
/** @} */
