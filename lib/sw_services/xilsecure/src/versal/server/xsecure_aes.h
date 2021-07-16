/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_aes.h
* @addtogroup xsecure_aes_versal_apis XilSecure AES Versal APIs
* @{
* @cond xsecure_internal
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
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XSECURE_AES_H_
#define XSECURE_AES_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xpmcdma.h"
#include "xsecure_sss.h"

/************************** Constant Definitions *****************************/
/** @cond xsecure_internal
 * @{
 */

#define XSECURE_AES_BUFFER_SIZE				(4U)
#define XSECURE_AES_KEY_DEC_SEL_BBRAM_RED		(0x0U)
#define XSECURE_AES_KEY_DEC_SEL_BH_RED			(0x1U)
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_RED		(0x2U)
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_USR0_RED		(0x3U)
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_USR1_RED		(0x4U)

#define XSECURE_SECURE_GCM_TAG_SIZE			(16U)
#define XSECURE_AES_KEY_SIZE_128BIT_WORDS		(4U)
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

/** @}
 * @endcond
 */

/**************************** Type Definitions *******************************/
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
	XSECURE_AES_ALL_KEYS,			/**< AES All keys */
} XSecure_AesKeySrc;


typedef enum {
	XSECURE_AES_KEY_SIZE_128 = 0,	/**< Key Length = 32 bytes = 256 bits */
	XSECURE_AES_KEY_SIZE_256 = 2,	/**< Key Length = 16 bytes = 128 bits */
}XSecure_AesKeySize;

/** @cond xsecure_internal
 * @{
 */
typedef enum {
	XSECURE_AES_UNINITIALIZED,
	XSECURE_AES_INITIALIZED,
	XSECURE_AES_ENCRYPT_INITIALIZED,
	XSECURE_AES_DECRYPT_INITIALIZED
} XSecure_AesState;

typedef struct {
	u32 BaseAddress;	   /**< AES Base address */
	XPmcDma *PmcDmaPtr;	   /**< PMCDMA Instance Pointer */
	XSecure_Sss SssInstance;   /**< Secure stream switch instance */
	XSecure_AesState AesState; /**< Current Aes State  */
	XSecure_AesKeySrc KeySrc;  /**< Key Source */
	u32 NextBlkLen;		   /**< Next Block Length */
} XSecure_Aes;
/** @}
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

int XSecure_AesDecryptKat(XSecure_Aes *AesInstance);

int XSecure_AesDecryptCmKat(const XSecure_Aes *AesInstance);

int XSecure_AesUpdateAad(XSecure_Aes *InstancePtr, u64 AadAddr, u32 AadSize);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_AES_H_ */

/* @} */
