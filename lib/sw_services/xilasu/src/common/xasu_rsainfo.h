/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_rsainfo.h
 *
 * This file contains the RSA definitions which are common across the
 * client and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   08/20/24 Initial release
 *       ss   09/23/24 Fixed doxygen comments
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/
#ifndef XASU_RSAINFO_H
#define XASU_RSAINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/* RSA module command IDs */
#define XASU_RSA_PUB_ENC_CMD_ID		(0U) /**< Command ID for RSA public encryption */
#define XASU_RSA_PVT_DEC_CMD_ID		(1U) /**< Command ID for RSA private decryption */
#define XASU_RSA_PVT_CRT_DEC_CMD_ID	(2U) /**< Command ID for RSA private CRT decryption */
#define XASU_RSA_KAT_CMD_ID		(3U) /**< Command ID for RSA KAT command */
#define XASU_RSA_GET_INFO_CMD_ID	(4U) /**< Command ID for RSA Get Info command */

/* RSA key size */
#define XRSA_2048_KEY_SIZE		(256U) /**< 2048 bit key size in bytes */
#define XRSA_3072_KEY_SIZE		(384U) /**< 3072 bit key size in bytes */
#define XRSA_4096_KEY_SIZE		(512U) /**< 4096 bit key size in bytes */

#define XRSA_MAX_KEY_SIZE_IN_WORDS	(128U) /**< RSA max key size in words */
#define XRSA_MAX_PRIME_SIZE_IN_WORDS	(64U) /**< RSA max prime size in words */

/************************************** Type Definitions *****************************************/
/**
 * @brief This structure contains RSA public key parameters info.
 */
typedef struct {
	u32 Keysize;					/**< Key size in bytes*/
	u32 Modulus[XRSA_MAX_KEY_SIZE_IN_WORDS];	/**< Modulus Address */
	u32 PubExp;					/**< Public exponent Address*/
} XAsu_RsaPubKeyComp;

/**
 * @brief This structure contains RSA private key parameters info.
 */
typedef struct {
	XAsu_RsaPubKeyComp PubKeyComp;			/**< Contains public key components */
	u32 PvtExp[XRSA_MAX_KEY_SIZE_IN_WORDS];		/**< Private Exponent Address*/
	u32 PrimeCompOrTotient[XRSA_MAX_KEY_SIZE_IN_WORDS];	/**< Prime component or
									Totient Address*/
	u32 PrimeCompOrTotientPrsnt;		/**< 0 : Ignore PrimeComporTotient array
						     1 : Array Consists totient
						     2 : Array Consists Prime component*/
} XAsu_RsaPvtKeyComp;

/**
 * @brief This structure contains RSA private key CRT parameters info.
 */
typedef struct {
	XAsu_RsaPubKeyComp PubKeyComp;			/**< Contains public key components */
	u32 Prime1[XRSA_MAX_PRIME_SIZE_IN_WORDS];	/**< Prime number 1 */
	u32 Prime2[XRSA_MAX_PRIME_SIZE_IN_WORDS];	/**< Prime number 2 */
	u32 DP[XRSA_MAX_PRIME_SIZE_IN_WORDS];		/**< Derived component from Prime1*/
	u32 DQ[XRSA_MAX_PRIME_SIZE_IN_WORDS];		/**< Derived component from Prime2*/
	u32 QInv[XRSA_MAX_PRIME_SIZE_IN_WORDS];		/**< Inverse of Q*/
} XAsu_RsaCrtKeyComp;

/**
 * @brief This structure contains RSA exponent value (R square mod N).
 */
typedef struct {
	u32 RRn[XRSA_MAX_KEY_SIZE_IN_WORDS];	/**< R square modN exponent value */
} XAsu_RsaRRModN;

/**
 * @brief This structure contains RSA exponent values (R mod N,R square mod N).
 */
typedef struct {
	XAsu_RsaRRModN RRModn;			/**< Contains R square modN component */
	u32 Rn[XRSA_MAX_KEY_SIZE_IN_WORDS];	/**< R modN exponent value */
} XAsu_RsaRModN;

/**
 * @brief This structure contains RSA params info.
 */
typedef struct {
	u64 InputDataAddr; 	/**< RSA input data address */
	u64 OutputDataAddr; 	/**< RSA output data address */
	u64 ExpoCompAddr;	/**< RSA exponent data address */
	u64 KeyCompAddr; 	/**< RSA key component address */
	u32 Len;		/**< Data Len */
} XAsu_RsaClientParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_RSAINFO_H */
/** @} */
