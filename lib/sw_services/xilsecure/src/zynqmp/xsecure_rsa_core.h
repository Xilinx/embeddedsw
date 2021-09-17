/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_rsa_core.h
* @addtogroup xsecure_rsa_zynqmp_apis XilSecure RSA ZynqMP APIs
* @{
* @cond xsecure_internal
* This file contains zynqmp specific RSA core APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 4.0   vns  03/09/19 Initial release
* 4.1   kpt  01/07/20 Added Macros for all the Magic Numbers
*                     in xsecure_rsa_core.c
* 4.2   kpt  03/26/20 Added Error code XSECURE_RSA_ZEROIZE_ERROR
*       har  04/06/20 Added function for selection of PKCS padding
* 4.3   har  06/17/20 Removed references to unused algorithms
*       ana  10/15/20 Updated doxygen tags
* 4.6   am   09/17/21 Resolved compiler warnings
*
* </pre>
*
* @endcond
******************************************************************************/
#ifndef XSECURE_RSA_CORE_H
#define XSECURE_RSA_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_utils.h"
/************************** Constant Definitions ****************************/
#define XSECURE_RSA_FAILED		0x1U /**< RSA Failed Error Code */
#define XSECURE_RSA_DATA_VALUE_ERROR	0x2U /**< for RSA private decryption
						* data should be lesser than
						* modulus */
#define XSECURE_RSA_ZEROIZE_ERROR		0x80U /**< for RSA zeroization Error*/

#define XSECURE_HASH_TYPE_SHA3		(48U) /**< SHA-3 hash size */
#define XSECURE_FSBL_SIG_SIZE		(512U)/**< FSBL signature size */
#define XSECURE_RSA_MAX_BUFF		(6U)  /**< RSA RAM Write Buffers */
#define XSECURE_RSA_MAX_RD_WR_CNT	(22U) /**< No of writes or reads to RSA RAM Buffers */
#define XSECURE_RSA_BYTE_MASK		(0XFFU) /**< RSA BYTE MASK */
#define XSECURE_RSA_BYTE_SHIFT		(8U)    /**< RSA BYTE */
#define XSECURE_RSA_HWORD_SHIFT		(16U)   /**< RSA HWORD */
#define XSECURE_RSA_SWORD_SHIFT		(24U)   /**< RSA SWORD */

/* Key size in bytes */
#define XSECURE_RSA_512_KEY_SIZE	(512U/8U) /**< RSA 512 key size */
#define XSECURE_RSA_576_KEY_SIZE	(576U/8U) /**< RSA 576 key size */
#define XSECURE_RSA_704_KEY_SIZE	(704U/8U) /**< RSA 704 key size */
#define XSECURE_RSA_768_KEY_SIZE	(768U/8U) /**< RSA 768 key size */
#define XSECURE_RSA_992_KEY_SIZE	(992U/8U) /**< RSA 992 key size */
#define XSECURE_RSA_1024_KEY_SIZE	(1024U/8U) /**< RSA 1024 key size */
#define XSECURE_RSA_1152_KEY_SIZE	(1152U/8U) /**< RSA 1152 key size */
#define XSECURE_RSA_1408_KEY_SIZE	(1408U/8U) /**< RSA 1408 key size */
#define XSECURE_RSA_1536_KEY_SIZE	(1536U/8U) /**< RSA 1536 key size */
#define XSECURE_RSA_1984_KEY_SIZE	(1984U/8U) /**< RSA 1984 key size */
#define XSECURE_RSA_2048_KEY_SIZE	(2048U/8U) /**< RSA 2048 key size */
#define XSECURE_RSA_3072_KEY_SIZE	(3072U/8U) /**< RSA 3072 key size */
#define XSECURE_RSA_4096_KEY_SIZE	(4096U/8U) /**< RSA 4096 key size */

/* Key size in words */
#define XSECURE_RSA_512_SIZE_WORDS	(16)	/**< RSA 512 Size in words */
#define XSECURE_RSA_576_SIZE_WORDS	(18)	/**< RSA 576 Size in words */
#define XSECURE_RSA_704_SIZE_WORDS	(22)	/**< RSA 704 Size in words */
#define XSECURE_RSA_768_SIZE_WORDS	(24)	/**< RSA 768 Size in words */
#define XSECURE_RSA_992_SIZE_WORDS	(31)	/**< RSA 992 Size in words */
#define XSECURE_RSA_1024_SIZE_WORDS	(32)	/**< RSA 1024 Size in words */
#define XSECURE_RSA_1152_SIZE_WORDS	(36)	/**< RSA 1152 Size in words */
#define XSECURE_RSA_1408_SIZE_WORDS	(44)	/**< RSA 1408 Size in words */
#define XSECURE_RSA_1536_SIZE_WORDS	(48)	/**< RSA 1536 Size in words */
#define XSECURE_RSA_1984_SIZE_WORDS	(62)	/**< RSA 1984 Size in words */
#define XSECURE_RSA_2048_SIZE_WORDS	(64)	/**< RSA 2048 Size in words */
#define XSECURE_RSA_3072_SIZE_WORDS	(96)	/**< RSA 3072 Size in words */
#define XSECURE_RSA_4096_SIZE_WORDS	(128U)	/**< RSA 4096 Size in words */

#define XSECURE_CSU_RSA_RAM_EXPO	(0U) /**< bit for RSA RAM Exponent */
#define XSECURE_CSU_RSA_RAM_MOD		(1U) /**< bit for RSA RAM modulus */
#define XSECURE_CSU_RSA_RAM_DIGEST	(2U) /**< bit for RSA RAM Digest */
#define XSECURE_CSU_RSA_RAM_SPAD	(3U) /**< bit for RSA RAM SPAD */
#define XSECURE_CSU_RSA_RAM_RES_Y	(4U) /**< bit for RSA RAM Result(Y) */
#define XSECURE_CSU_RSA_RAM_RES_Q	(5U) /**< bit for RSA RAM Result(Q) */

#define XSECURE_RSA_SIGN_ENC		0U /**< RSA encryption flag */
#define XSECURE_RSA_SIGN_DEC		1U /**< RSA decryption flag */

/** @name Control Register
 *
 * The Control register (CR) controls the major functions of the device.
 * It is used to set the function to be implemented by the RSA device in
 * the next iteration.
 *
 * Control Register Bit Definition
 */
#define XSECURE_CSU_RSA_CONTROL_512	(0x00U) /**< RSA 512 Length Code */
#define XSECURE_CSU_RSA_CONTROL_576	(0x10U) /**< RSA 576 Length Code */
#define XSECURE_CSU_RSA_CONTROL_704	(0x20U) /**< RSA 704 Length Code */
#define XSECURE_CSU_RSA_CONTROL_768	(0x30U) /**< RSA 768 Length Code */
#define XSECURE_CSU_RSA_CONTROL_992	(0x40U) /**< RSA 992 Length Code */
#define XSECURE_CSU_RSA_CONTROL_1024	(0x50U) /**< RSA 1024 Length Code */
#define XSECURE_CSU_RSA_CONTROL_1152	(0x60U) /**< RSA 1152 Length Code */
#define XSECURE_CSU_RSA_CONTROL_1408	(0x70U) /**< RSA 1408 Length Code */
#define XSECURE_CSU_RSA_CONTROL_1536	(0x80U) /**< RSA 1536 Length Code */
#define XSECURE_CSU_RSA_CONTROL_1984	(0x90U) /**< RSA 1984 Length Code */
#define XSECURE_CSU_RSA_CONTROL_2048	(0xA0U) /**< RSA 2048 Length Code */
#define XSECURE_CSU_RSA_CONTROL_3072	(0xB0U) /**< RSA 3072 Length Code */
#define XSECURE_CSU_RSA_CONTROL_4096	(0xC0U) /**< RSA 4096 Length Code */
#define XSECURE_CSU_RSA_CONTROL_DCA	(0x08U) /**< Abort Operation */
#define XSECURE_CSU_RSA_CONTROL_NOP	(0x00U) /**< No Operation */
#define XSECURE_CSU_RSA_CONTROL_EXP	(0x01U) /**< Exponentiation Opcode */
#define XSECURE_CSU_RSA_CONTROL_EXP_PRE	(0x05U) /**< Expo. using R*R mod M */
#define XSECURE_CSU_RSA_CONTROL_MASK	(XSECURE_CSU_RSA_CONTROL_4096 + \
					XSECURE_CSU_RSA_CONTROL_EXP_PRE)

/** @name RSA status Register
 *
 * The Status Register(SR) indicates the current state of RSA device.
 *
 * Status Register Bit Definition
 */
#define XSECURE_CSU_RSA_STATUS_DONE	(0x1U) /**< Operation Done */
#define XSECURE_CSU_RSA_STATUS_BUSY	(0x2U) /**< RSA busy */
#define XSECURE_CSU_RSA_STATUS_ERROR	(0x4U) /**< Error */
#define XSECURE_CSU_RSA_STATUS_PROG_CNT	(0xF8U) /**< Progress Counter */
/* @}*/

typedef enum {
	XSECURE_RSA_UNINITIALIZED = 0,
	XSECURE_RSA_INITIALIZED
} XSecure_RsaState;

/***************************** Type Definitions ******************************/
/**
 * The RSA driver instance data structure. A pointer to an instance data
 * structure is passed around by functions to refer to a specific driver
 * instance.
 */

typedef struct {
	u32 BaseAddress; /**< Device Base Address */
	u8* Mod; /**< Modulus */
	u8* ModExt; /**< Precalc. R sq. mod N */
	u8* ModExpo; /**< Exponent */
	u8 EncDec; /**< 0 for signature verification and 1 for generation */
	u32 SizeInWords;/** RSA key size in words */
	XSecure_RsaState RsaState;
} XSecure_Rsa;

/***************************** Function Prototypes ***************************/

/* ZynqMP specific RSA core initialization function */
u32 XSecure_RsaCfgInitialize(XSecure_Rsa *InstancePtr);

/* ZynqMP specific RSA core encryption/decryption function */
u32 XSecure_RsaOperation(XSecure_Rsa *InstancePtr, u8 *Input,
		u8 *Result, u8 EncDecFlag, u32 Size);

/* ZynqMP specific function for selection of PKCS padding */
u8* XSecure_RsaGetTPadding(void);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_RSA_CORE_H */

/* @} */