/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
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

#define XSECURE_AES_KEY_DEC_SEL_BBRAM_RED		(0x0U)
#define XSECURE_AES_KEY_DEC_SEL_BH_RED			(0x1U)
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_RED		(0x2U)
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_USR0_RED		(0x3U)
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_USR1_RED		(0x4U)

#define XSECURE_SECURE_GCM_TAG_SIZE			(16U)
#define XSECURE_AES_KEY_SIZE_128BIT_WORDS		(4U)
#define XSECURE_AES_KEY_SIZE_128BIT_BYTES		(16U)
#define XSECURE_AES_KEY_SIZE_256BIT_WORDS		(8U)
#define XSECURE_AES_KEY_SIZE_256BIT_BYTES		(32U)

#define XSECURE_AES_TIMEOUT_MAX				(0x1FFFFU)

#define XSECURE_AES_INVALID_CFG				(0xFFFFFFFFU)

#define XSECURE_AES_NO_CFG_DST_DMA			(0xFFFFFFFFU)

#define XSECURE_AES_KEY_MASK_INDEX			(0xA0U)
#define XSECURE_AES_DMA_SIZE				(16U)
#define XSECURE_AES_DMA_LAST_WORD_ENABLE		(0x1U)
#define XSECURE_AES_DMA_LAST_WORD_DISABLE		(0x0U)
/* Key select values */
#define XSECURE_AES_KEY_SEL_BBRAM_KEY			(0xBBDE6600U)
#define XSECURE_AES_KEY_SEL_BBRAM_RD_KEY		(0xBBDE8200U)
#define XSECURE_AES_KEY_SEL_BH_KEY			(0xBDB06600U)
#define XSECURE_AES_KEY_SEL_BH_RD_KEY			(0xBDB08200U)
#define XSECURE_AES_KEY_SEL_EFUSE_KEY			(0xEFDE6600U)
#define XSECURE_AES_KEY_SEL_EFUSE_RED_KEY		(0xEFDE8200U)
#define XSECURE_AES_KEY_SEL_EFUSE_USR_KEY0		(0xEF856601U)
#define XSECURE_AES_KEY_SEL_EFUSE_USR_KEY1		(0xEF856602U)
#define XSECURE_AES_KEY_SEL_EFUSE_USR_RD_KEY0		(0xEF858201U)
#define XSECURE_AES_KEY_SEL_EFUSE_USR_RD_KEY1		(0xEF858202U)
#define XSECURE_AES_KEY_SEL_KUP_KEY			(0xBDC98200U)
#define XSECURE_AES_KEY_SEL_PUF_KEY			(0xDBDE8200U)
#define XSECURE_AES_KEY_SEL_USR_KEY_0			(0xBD858201U)
#define XSECURE_AES_KEY_SEL_USR_KEY_1			(0xBD858202U)
#define XSECURE_AES_KEY_SEL_USR_KEY_2			(0xBD858204U)
#define XSECURE_AES_KEY_SEL_USR_KEY_3			(0xBD858208U)
#define XSECURE_AES_KEY_SEL_USR_KEY_4			(0xBD858210U)
#define XSECURE_AES_KEY_SEL_USR_KEY_5			(0xBD858220U)
#define XSECURE_AES_KEY_SEL_USR_KEY_6			(0xBD858240U)
#define XSECURE_AES_KEY_SEL_USR_KEY_7			(0xBD858280U)
#define XSECURE_ENABLE_BYTE_SWAP		(0x1U)	/**< Enables data swap in AES */
#define XSECURE_DISABLE_BYTE_SWAP		(0x0U)	/**< Disables data swap in AES */

#define XSecure_AesDpaCmDecryptKat 		XSecure_AesDpaCmDecryptData
						/**< XSecure_AesDpaCmDecryptKat is deprecated to XSecure_AesDpaCmDecryptData */

/** @}
 * @endcond
 */

/**************************** Type Definitions *******************************/
/** Used for selecting the Key source of AES Core. */
typedef enum {
	XSECURE_AES_BBRAM_KEY = 0,		/**< BBRAM Key */
	XSECURE_AES_BBRAM_RED_KEY,		/**< BBRAM Red Key */
	XSECURE_AES_BH_KEY,			/**< BH Key */
	XSECURE_AES_BH_RED_KEY,			/**< BH Red Key */
	XSECURE_AES_EFUSE_KEY,			/**< eFUSE Key */
	XSECURE_AES_EFUSE_RED_KEY,		/**< eFUSE Red Key */
	XSECURE_AES_EFUSE_USER_KEY_0,		/**< eFUSE User Key 0 */
	XSECURE_AES_EFUSE_USER_KEY_1,		/**< eFUSE User Key 1 */
	XSECURE_AES_EFUSE_USER_RED_KEY_0,	/**< eFUSE User Red Key 0 */
	XSECURE_AES_EFUSE_USER_RED_KEY_1,	/**< eFUSE User Red Key 1 */
	XSECURE_AES_KUP_KEY,			/**< KUP key */
	XSECURE_AES_PUF_KEY,			/**< PUF key */
	XSECURE_AES_USER_KEY_0,			/**< User Key 0 */
	XSECURE_AES_USER_KEY_1,			/**< User Key 1 */
	XSECURE_AES_USER_KEY_2,			/**< User Key 2 */
	XSECURE_AES_USER_KEY_3,			/**< User Key 3 */
	XSECURE_AES_USER_KEY_4,			/**< User Key 4 */
	XSECURE_AES_USER_KEY_5,			/**< User Key 5 */
	XSECURE_AES_USER_KEY_6,			/**< User Key 6 */
	XSECURE_AES_USER_KEY_7,			/**< User Key 7 */
	XSECURE_AES_EXPANDED_KEYS,		/**< Expanded keys */
	XSECURE_AES_PUF_RED_EXPANDED_KEYS,	/**< AES PUF,RED,KUP keys */
	XSECURE_AES_ALL_KEYS,			/**< AES All keys */
	XSECURE_AES_INVALID_KEY,		/**< AES Invalid Key */
} XSecure_AesKeySrc;

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
	XSECURE_AES_ENCRYPT_INITIALIZED,	/**< Encrypt initialized state */
	XSECURE_AES_DECRYPT_INITIALIZED		/**< Decrypt initialised state */
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
	u32 IsResourceBusy;   /**< Flag to check whether resource is busy or not */
	u32 IpiMask;               /**< Used to store Ipimask value */
	u32 DataContextLost;  /**< If data context is lost for an IPI channel
					it's corresponding bit position is set */
	u32 PreviousAesIpiMask; /**< Used to store the Ipi mask of previous aes operation */
#ifdef VERSAL_NET
	u32 IsEcbEn;           /**< ECB mode enable or disable */
#endif
} XSecure_Aes;

/*****************************************************************************/
/**
 * @brief	This function returns AES ECB mode status
 *
 * @param	InstancePtr Pointer to the XSecure_Aes instance
 *
 * @return
 *			- TRUE  - If ECB mode is enabled
 *			- FALSE - If ECB mode is not supported/disabled
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

int XSecure_AesGmacCfg(XSecure_Aes *InstancePtr, u32 IsGmacEn);

int XSecure_AesDpaCmDecryptData(const XSecure_Aes *AesInstance,
	const u32 *KeyPtr, const u32 *DataPtr, u32 *OutputPtr);

void XSecure_AesSetDataContext(XSecure_Aes *InstancePtr);
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_AES_H_ */
