/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_rsa_core.h
* This file contains Versal specific RSA core APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 4.0   vns  03/09/19 Initial release
* 4.1   kpt  01/07/20 Added Macro's for Magic Numbers in
*                     xsecure_rsa_core.c
* 4.2   kpt  03/26/20 Added Error code XSECURE_RSA_ZEROIZE_ERROR
*
* </pre>
*
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
#define XSECURE_HASH_TYPE_SHA2		(32U) /**< SHA-2 hash size */
#define XSECURE_FSBL_SIG_SIZE		(512U)/**< FSBL signature size */
#define XSECURE_RSA_MAX_BUFF		(6U) /**< RSA RAM Write Buffers */
#define XSECURE_RSA_MAX_RD_WR_CNT	(22U) /**< No of writes or reads to RSA RAM buffers */

/* Key size in bytes */
#define XSECURE_RSA_2048_KEY_SIZE	(2048U/8U) /**< RSA 2048 key size */
#define XSECURE_RSA_3072_KEY_SIZE	(3072U/8U) /**< RSA 3072 key size */
#define XSECURE_RSA_4096_KEY_SIZE	(4096U/8U) /**< RSA 4096 key size */

/* Key size in words */
#define XSECURE_RSA_2048_SIZE_WORDS	(64)	/**< RSA 2048 Size in words */
#define XSECURE_RSA_3072_SIZE_WORDS	(96)	/**< RSA 3072 Size in words */
#define XSECURE_RSA_4096_SIZE_WORDS	(128U)	/**< RSA 4096 Size in words */

#define XSECURE_RSA_RAM_EXPO	(0U) /**< bit for RSA RAM Exponent */
#define XSECURE_RSA_RAM_MOD		(1U) /**< bit for RSA RAM modulus */
#define XSECURE_RSA_RAM_DIGEST	(2U) /**< bit for RSA RAM Digest */
#define XSECURE_RSA_RAM_SPAD	(3U) /**< bit for RSA RAM SPAD */
#define XSECURE_RSA_RAM_RES_Y	(4U) /**< bit for RSA RAM Result(Y) */
#define XSECURE_RSA_RAM_RES_Q	(5U) /**< bit for RSA RAM Result(Q) */

/** @name Control Register
 *
 * Control Register opcode definitions
 */
#define XSECURE_RSA_CONTROL_DCA	(0x08U) /**< Abort Operation */
#define XSECURE_RSA_CONTROL_NOP	(0x00U) /**< No Operation */
#define XSECURE_RSA_CONTROL_EXP	(0x01U) /**< Exponentiation Opcode */
#define XSECURE_RSA_CONTROL_EXP_PRE	(0x05U) /**< Expo. using R*R mod M */

/**
 * Config registers values
 * CFG0 is for Qsel and multiplication passes
 * CFG1 is for Mont digits
 * CFG2 is for location size
 * CFG5 is for No.of groups
 */
#define XSECURE_ECDSA_RSA_CFG0_4096_VALUE	(0x0000006BU)
#define XSECURE_ECDSA_RSA_CFG1_4096_VALUE	(0x00000081U)
#define XSECURE_ECDSA_RSA_CFG2_4096_VALUE	(0x00000016U)
#define XSECURE_ECDSA_RSA_CFG5_4096_VALUE	(0x00000015U)

#define XSECURE_ECDSA_RSA_CFG0_3072_VALUE	(0x000000A0U)
#define XSECURE_ECDSA_RSA_CFG1_3072_VALUE	(0x00000061U)
#define XSECURE_ECDSA_RSA_CFG2_3072_VALUE	(0x00000016U)
#define XSECURE_ECDSA_RSA_CFG5_3072_VALUE	(0x00000010U)

#define XSECURE_ECDSA_RSA_CFG0_2048_VALUE	(0x00000016U)
#define XSECURE_ECDSA_RSA_CFG1_2048_VALUE	(0x00000041U)
#define XSECURE_ECDSA_RSA_CFG2_2048_VALUE	(0x00000016U)
#define XSECURE_ECDSA_RSA_CFG5_2048_VALUE	(0x0000000AU)

/** @name RSA status Register
 *
 * The Status Register(SR) indicates the current state of RSA device.
 *
 * Status Register Bit Definition
 */
#define XSECURE_RSA_STATUS_DONE	(0x1U) /**< Operation Done */
#define XSECURE_RSA_STATUS_BUSY	(0x2U) /**< RSA busy */
#define XSECURE_RSA_STATUS_ERROR	(0x4U) /**< Error */
#define XSECURE_RSA_STATUS_PROG_CNT	(0xF8U) /**< Progress Counter */
/* @}*/

typedef enum {
	XSECURE_RSA_UNINITIALIZED = 0,
	XSECURE_RSA_INITIALIZED			/**< 0x1 */
} XSecure_RsaState;

typedef enum {
	XSECURE_RSA_SIGN_ENC = 0x0U,
	XSECURE_RSA_SIGN_DEC			/**< 0x1 */
}XSecure_RsaOps;

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

/* Versal specific RSA core initialization function */
u32 XSecure_RsaCfgInitialize(XSecure_Rsa *InstancePtr);

/* Versal specific RSA core encryption/decryption function */
u32 XSecure_RsaOperation(XSecure_Rsa *InstancePtr, u8 *Input,
			u8 *Result, u8 RsaOp, u32 Size);

u32 XSecure_RsaPublicEncryptKat(void);

/* Versal specific function for selection of PKCS padding */
u8* XSecure_RsaGetTPadding();

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_RSA_CORE_H */
