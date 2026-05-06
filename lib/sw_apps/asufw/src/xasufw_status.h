/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_status.h
 *
 * This file comprises error codes and function prototypes essential for updating error codes
 * in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  01/16/24 Initial release
 * 1.1   vns  02/20/23 Added error codes for error management
 *       ma   03/16/24 Added error codes for init, IPI, task and shared memory code
 *       ma   03/23/24 Added error codes for DMA functionality
 *       ma   04/18/24 Added error codes for modules functionality
 *       ma   05/14/24 Added error codes for SHA functionality
 *       ma   05/20/24 Added error codes for TRNG functionality
 *       ma   06/14/24 Added error code for DMA resource allocation failure
 *       yog  06/19/24 Added error codes for ECC functionality
 *       am   06/26/24 Added error codes for AES functionality
 *       ss   07/11/24 Added error codes for RSA functionality
 *       yog  07/11/24 Added error codes for ECC_RSA functionality
 *       ma   07/26/24 Added XASUFW_TRNG_KAT_NOT_SUPPORTED_ON_QEMU error code
 *       ss   08/20/24 Added XASUFW_RSA_MODULE_REGISTRATION_FAILED error code
 *       yog  08/21/24 Rearranged ECC and RSA_ECC error codes
 *       am   08/01/24 Added error codes for AES handler functionality
 *       am   08/24/24 Added error codes related to AES decrypt CM KAT
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 * 1.2   ss   12/02/24 Added error codes for ECDH
 *       ma   12/12/24 Added support for command in progress
 *       ma   01/03/25 Added error codes for TRNG core register configuration failures
 *       yog  01/03/25 Added error codes for HMAC functionality
 *       am   01/20/25 Added error codes for AES CCM functionality
 *       ma   01/21/25 Added error codes for KDF functionality
 *       LC   02/07/25 Listed actual error codes in description
 *       ma   02/11/25 Added TRNG related error codes
 *       ma   02/21/25 Added error code for ASU exceptions
 *       yog  02/24/25 Added error codes for ECIES functionality
 *       am   02/24/25 Added XASUFW_ERR_KV_INTERRUPT_DONE_TIMEOUT error code
 *       ss   02/24/25 Added error codes for Key wrap functionality
 *       ma   03/17/25 Added XASUFW_VALIDATE_COMMAND_FAILED error code
 *       yog  03/21/25 Added PWCT error codes
 *       am   03/21/25 Added XASUFW_AES_ECB_CBC_DUMMY_ENCRYPTION_FAILED error code
 *       am   04/01/25 Added XASUFW_AES_KEY_CONFIG_READBACK_ERROR error code
 *       LP   04/07/25 Added HKDF error codes
 *       rmv  08/04/25 Added error code for TRNG mode configuration failure
 *       rmv  08/04/25 Added error codes for OCP functionality
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_server_error_codes Server Error Codes
* @{
*/
#ifndef XASUFW_STATUS_H_
#define XASUFW_STATUS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_ECC_TERMINATION_CODE_MASK       (0x20U) /**< Mask for termination code indication */

/************************************** Type Definitions *****************************************/
/**
 * ASU error status values, maximum allowed value is 0x3FF.
 */
enum {
	XASUFW_SUCCESS, /**< 0x00U - ASUFW success */
	XASUFW_FAILURE, /**< 0x01U - ASUFW failure */
	XASUFW_INVALID_PARAM, /**< 0x02U -  Invalid parameters to ASUFW */
	XASUFW_RESOURCE_INVALID, /**< 0x03U - Invalid resource selection */
	XASUFW_RESOURCE_UNAVAILABLE, /**< 0x04U - Resource is busy */
	XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, /**< 0x05U - Releasing the resource is not allowed */
	XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, /**< 0x06U - DMA resource allocation failure */
	XASUFW_TASK_INVALID_HANDLER, /**< 0x07U - Received invalid task with NULL task handler */
	XASUFW_INVALID_USER_CONFIG_RECEIVED, /**< 0x08U - Invalid comm channel user config data
						received */
	XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED, /**< 0x09U - Command received for unregistered
						module */
	XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED, /**< 0x0AU - Invalid command is received from
						client */
	XASUFW_ERR_RESERVED_0B, /**< 0x0BU - Reserved */
	XASUFW_SSS_INVALID_INPUT_PARAMETERS, /**< 0x0CU - Received invalid input parameters to SSS
						configuration APIs */
	XASUFW_IOMODULE_INIT_FAILED, /**< 0x0DU - IOModule initialization failed */
	XASUFW_IOMODULE_SELF_TEST_FAILED, /**< 0x0EU - IOModule self-test failed */
	XASUFW_IOMODULE_START_FAILED, /**< 0x0FU - IOModule start failed */
	XASUFW_IOMODULE_CONNECT_FAILED, /**< 0x10U - IOModule connect failed */
	XASUFW_ERR_RESERVED_11, /**< 0x11U - Reserved */
	XASUFW_IPI_INVALID_INPUT_PARAMETERS, /**< 0x12U - Received invalid input arguments for IPI
						send/receive */
	XASUFW_IPI_POLL_FOR_ACK_FAILED, /**< 0x13U - IPI Poll for ack failed */
	XASUFW_IPI_WRITE_MESSAGE_FAILED, /**< 0x14U - IPI write message failed */
	XASUFW_IPI_TRIGGER_FAILED, /**< 0x15U - IPI trigger failed */
	XASUFW_IPI_READ_MESSAGE_FAILED, /**< 0x16U - IPI read message failed */
	XASUFW_ERR_DMA_INSTANCE_NULL, /**< 0x17U - If DMA instance is NULL */
	XASUFW_ERR_DMA_LOOKUP, /**< 0x18U - DMA driver lookup config failed */
	XASUFW_ERR_DMA_CFG, /**< 0x19U - DMA driver config initialization failed */
	XASUFW_ERR_DMA_SELFTEST, /**< 0x1AU - DMA self-test failed */
	XASUFW_DMA_WAIT_FOR_DONE_TIMED_OUT, /**< 0x1BU - Non blocking DMA transfer wait failed in given
						channel WaitForDone */
	XASUFW_INVALID_DMA_SSS_CONFIG, /**< 0x1CU - SSS configuration assignment failed as the DMA
						address is invalid */
	XASUFW_ERR_EXCEPTION, /**< 0x1DU - Processor exception received */
	XASUFW_ERR_IPI_SEND_PLM_EFUSE_PRGM, /**< 0x1E - Failed while sending IPI for efuse write */
	XASUFW_ERR_IPI_RSP_PLM_EFUSE_PRGM, /**< 0x1F - IPI response failure to program efuse */
	XASUFW_VALIDATE_COMMAND_FAILED, /**< 0x20 - Command validation failed */
	/* Termination code errors 0x21U to 0x2CU are reserved for errors returning from core */
	XASUFW_ECC_INVALID_PARAM = 0x2DU, /**< 0x2DU - Invalid parameters to ECC APIs*/
	XASUFW_ECC_INIT_NOT_DONE, /**< 0x2EU - ECC is not initialized */
	XASUFW_ECC_INIT_FAILED, /**< 0x2FU - ECC initialization failed */
	XASUFW_ECC_GEN_PUB_KEY_OPERATION_FAIL, /**< 0x30U - When ECC Generate public key operation
						fails */
	XASUFW_ECC_VALIDATE_PUB_KEY_OPERATION_FAIL, /**< 0x31U - When ECC validate public key
						operation fails */
	XASUFW_ECC_GEN_SIGN_OPERATION_FAIL, /**<  0x32U - When ECC Generate signature operation
						fails */
	XASUFW_ECC_VERIFY_SIGN_OPERATION_FAIL, /**<  0x33U - When ECC verify signature operation
						fails */
	XASUFW_ECC_WRITE_DATA_FAIL, /**< 0x34U - When writing data to memory fails */
	XASUFW_ECC_READ_DATA_FAIL, /**< 0x35U - When reading data from memory fails */
	XASUFW_ECC_EPHEMERAL_KEY_GEN_FAIL, /**< 0x36U - When generate ephemeral key fails */
	XASUFW_ECC_WAIT_FOR_DONE_TIMEOUT, /**< 0x37U - When wait for done timed out */
	XASUFW_ECC_MODULE_REGISTRATION_FAILED, /**< 0x38U - Module registration failed for ECC
						module */
	XASUFW_ECC_PUBKEY_COMPARISON_FAILED, /**< 0x39U - ECC public key comparison failed */
	XASUFW_ECC_SIGNATURE_COMPARISON_FAILED, /**< 0x3AU - ECC signature comparison failed */
	XASUFW_ECC_KAT_FAILED, /**< 0x3BU - ECC Kat failed */
	XASUFW_RSA_ECC_INVALID_PARAM, /**< 0x3CU - Invalid parameters to RSA_ECC APIs */
	XASUFW_RSA_ECC_GEN_PUB_KEY_OPERATION_FAIL, /**< 0x3DU - When public key generation fails */
	XASUFW_RSA_ECC_WRITE_DATA_FAIL, /**< 0x3EU - When writing data to memory fails */
	XASUFW_RSA_ECC_READ_DATA_FAIL, /**< 0x3FU - When reading data from memory fails */
	XASUFW_RSA_ECC_PUBLIC_KEY_ZERO, /**< 0x40U - When public key is zero */
	XASUFW_RSA_ECC_PUBLIC_KEY_WRONG_ORDER, /**< 0x41U - Wrong order of Public key */
	XASUFW_RSA_ECC_PUBLIC_KEY_NOT_ON_CRV, /**< 0x42U - Key not found on curve */
	XASUFW_RSA_ECC_EPHEMERAL_KEY_GEN_FAIL, /**< 0x43U - When generate ephemeral key fails */
	XASUFW_RSA_ECC_GEN_SIGN_BAD_RAND_NUM, /**< 0x44U - Bad random number used for sign
						generation */
	XASUFW_RSA_ECC_GEN_SIGN_INCORRECT_HASH_LEN, /**< 0x45U - Incorrect hash length for sign
						generation */
	XASUFW_RSA_ECC_BAD_SIGN, /**< 0x46U - Signature provided for verification is bad */
	XASUFW_RSA_ECC_VER_SIGN_INCORRECT_HASH_LEN, /**< 0x47U - Incorrect hash length for sign
						verification */
	XASUFW_RSA_ECC_VER_SIGN_R_ZERO, /**< 0x48U - When provided R is zero */
	XASUFW_RSA_ECC_VER_SIGN_S_ZERO, /**< 0x49U - When provided S is zero */
	XASUFW_RSA_ECC_VER_SIGN_R_ORDER_ERROR, /**< 0x4AU - R is not within ECC order */
	XASUFW_RSA_ECC_VER_SIGN_S_ORDER_ERROR, /**< 0x4BU - S is not within ECC order */
	XASUFW_RSA_ECC_PUBKEY_COMPARISON_FAILED, /**< 0x4CU - RSA ECC public key comparison failed */
	XASUFW_RSA_ECC_SIGNATURE_COMPARISON_FAILED, /**< 0x4DU - RSA ECC signature comparison failed */
	XASUFW_RSA_ECC_KAT_FAILED, /**< 0x4EU - RSA ECC Kat failed */
	XASUFW_SHA_INVALID_PARAM, /**< 0x4FU - Invalid parameters to SHA APIs */
	XASUFW_SHA_STATE_MISMATCH_ERROR, /**< 0x50U - SHA state mismatch error. Occurs when previous
						SHA state doesn't match before further update */
	XASUFW_SHA_INVALID_INPUT_DATA_ADDRESS, /**< 0x51U - SHA invalid input data address */
	XASUFW_SHA_INVALID_INPUT_DATA_SIZE, /**< 0x52U - SHA invalid input data size */
	XASUFW_SHA_INVALID_END_LAST, /**< 0x53U - SHA invalid end last */
	XASUFW_SHA_INVALID_HASH_ADDRESS, /**< 0x54U - SHA invalid hash address */
	XASUFW_SHA_INVALID_HASH_SIZE, /**< 0x55U - SHA invalid hash size */
	XASUFW_SHA_NEXT_XOF_INVALID_MASK, /**< 0x56U - SHA invalid next xof mask */
	XASUFW_SHA_INVALID_SHA_TYPE, /**< 0x57U - SHA invalid type */
	XASUFW_SHA_INVALID_SHA_MODE, /**< 0x58U - SHA invalid mode */
	XASUFW_SHA_MODE_GLITCH_DETECTED, /**< 0x59U - Configured SHA mode is not matching with the
						input */
	XASUFW_SHA_HASH_COMPARISON_FAILED, /**< 0x5AU - SHA Hash comparison failed */
	XASUFW_SHA_KAT_FAILED, /**< 0x5BU - SHA KAT failed */
	XASUFW_SHA2_MODULE_REGISTRATION_FAILED, /**< 0x5CU - Module registration failed for SHA2
						module */
	XASUFW_SHA2_INIT_FAILED, /**< 0x5DU - SHA2 initialization failed */
	XASUFW_ERR_RESERVED_5E, /**< 0x5EU - Reserved */
	XASUFW_ERR_RESERVED_5F, /**< 0x5FU - Reserved */
	XASUFW_ERR_RESERVED_60, /**< 0x60U - Reserved */
	XASUFW_SHA2_HASH_COMPARISON_FAILED, /**< 0x61U - SHA2 hash comparison failed */
	XASUFW_SHA3_MODULE_REGISTRATION_FAILED, /**< 0x62U - Module registration failed for SHA3
						module */
	XASUFW_SHA3_INIT_FAILED, /**< 0x63U - SHA3 initialization failed */
	XASUFW_ERR_RESERVED_64, /**< 0x64U - Reserved */
	XASUFW_ERR_RESERVED_65, /**< 0x65U - Reserved */
	XASUFW_ERR_RESERVED_66, /**< 0x66U - Reserved */
	XASUFW_SHA3_HASH_COMPARISON_FAILED, /**< 0x67U - SHA3 hash comparison failed */
	XASUFW_TRNG_MODULE_REGISTRATION_FAILED, /**< 0x68U - TRNG module registration failed */
	XASUFW_TRNG_INVALID_PARAM, /**< 0x69U - TRNG invalid input parameters received */
	XASUFW_TRNG_INVALID_SEED_VALUE, /**< 0x6AU - TRNG invalid seed value received */
	XASUFW_TRNG_INVALID_STATE, /**< 0x6BU - TRNG invalid state */
	XASUFW_TRNG_UNHEALTHY_STATE, /**< 0x6CU - TRNG is in unhealthy state */
	XASUFW_TRNG_INVALID_MODE, /**< 0x6DU - TRNG mode input received is invalid */
	XASUFW_TRNG_INVALID_DF_LENGTH, /**< 0x6EU - TRNG invalid DF length received */
	XASUFW_TRNG_INVALID_SEED_LENGTH, /**< 0x6FU - TRNG invalid seed length received */
	XASUFW_TRNG_INVALID_SEED_LIFE, /**< 0x70U - TRNG invalid seed life received */
	XASUFW_TRNG_INVALID_ADAPTPROPTEST_CUTOFF_VALUE, /**< 0x71U - TRNG invalid adaptproptestcutoff
						value */
	XASUFW_TRNG_INVALID_REPCOUNTTEST_CUTOFF_VALUE, /**< 0x72U - TRNG invalid precounttestcutoff
						value */
	XASUFW_TRNG_USER_CFG_COPY_ERROR, /**< 0x73U - TRNG user config structure copy to TRNG instance
						failed */
	XASUFW_TRNG_INVALID_BUF_SIZE, /**< 0x74U - TRNG invalid buffer size */
	XASUFW_TRNG_RESEED_REQUIRED_ERROR, /**< 0x75U - TRNG reseed required error */
	XASUFW_TRNG_TIMEOUT_ERROR, /**< 0x76U - TRNG event timeout error */
	XASUFW_TRNG_CATASTROPHIC_DTF_ERROR, /**< 0x77U - TRNG catastrophic DTF error */
	XASUFW_TRNG_CATASTROPHIC_CTF_ERROR, /**< 0x78U - TRNG catastrophic CTF error */
	XASUFW_TRNG_KAT_FAILED_ERROR, /**< 0x79U - TRNG KAT operation failed */
	XASUFW_RSA_ECC_HASH_CALC_FAIL, /**< 0x7AU - RSA ECC hash calculation failed for input message. */
	XASUFW_INVALID_PREDRES_VALUE, /**< 0x7BU - TRNG invalid prediction resistance value */
	XASUFW_TRNG_FIFO_IS_EMPTY, /**< 0x7CU - TRNG random numbers fifo is empty */
	XASUFW_RANDOM_DATA_FAILED_TO_GENERATE, /**< 0x7DU - TRNG random data generate failed */
	XASUFW_OSCILLATOR_ENABLE_FAILED, /**< 0x7EU - TRNG enabling oscillator as seed source failed */
	XASUFW_OSCILLATOR_DISABLE_FAILED, /**< 0x7FU - TRNG disabling oscillator source failed */
	XASUFW_ENABLE_PRNG_FOR_RESEED_FAILED, /**< 0x80U - TRNG enabling PRNG for reseed operation
						failed */
	XASUFW_START_RESEED_FAILED, /**< 0x81U - TRNG reseed failed */
	XASUFW_TRNG_INVALID_RANDOM_BYTES_SIZE, /**< 0x82U - TRNG invalid random bytes requested */
	XASUFW_KAT_NOT_SUPPORTED_ON_QEMU, /**< 0x83U - KAT is not supported on QEMU */
	XASUFW_TRNG_DFLEN_CONFIG_ERROR, /**< 0x84U - DF length configuration failure */
	XASUFW_TRNG_ADAPTCUTOFF_CONFIG_ERROR, /**< 0x85U - Adaptive test cutoff configuration
						failure */
	XASUFW_TRNG_REPCUTOFF_CONFIG_ERROR, /**< 0x86U - Repetitive test cutoff configuration
						failure */
	XASUFW_TRNG_DIT_CONFIG_ERROR, /**< 0x87U - DIT value configuration failure */
	XASUFW_AES_GLITCH_ERROR, /**< 0x88U - AES glitch error */
	XASUFW_AES_INVALID_PARAM, /**< 0x89U - Invalid parameters to AES APIs */
	XASUFW_AES_STATE_MISMATCH_ERROR, /**< 0x8AU - AES state mismatch error. Occurs when previous
						AES state doesn't match before further update */
	XASUFW_AES_KEY_CLEAR_ERROR, /**< 0x8BU - AES key clear error */
	XASUFW_AES_INVALID_KEY_OBJECT_ADDRESS, /**< 0x8CU - AES invalid key object address */
	XASUFW_AES_INVALID_KEY_ADDRESS, /**< 0x8DU - AES invalid key address */
	XASUFW_AES_INVALID_KEY_SRC, /**< 0x8EU - AES invalid key source */
	XASUFW_AES_INVALID_KEY_SIZE, /**< 0x8FU - AES invalid key size */
	XASUFW_AES_INVALID_IV, /**< 0x90U - AES invalid IV length/address for respective engine
						modes */
	XASUFW_AES_INVALID_ENGINE_MODE, /**< 0x91U - AES invalid engine mode */
	XASUFW_AES_KEY_ZEROED,  /**< 0x92U - AES zeroed key not allowed */
	XASUFW_AES_INVALID_OPERATION_TYPE, /**< 0x93U - AES invalid encrypt/decrypt operation type */
	XASUFW_AES_INVALID_INPUT_DATA, /**< 0x94U - AES invalid input data */
	XASUFW_AES_INVALID_INPUT_DATA_LENGTH, /**< 0x95U - AES invalid input data length */
	XASUFW_AES_INVALID_ISLAST_CHUNK, /**< 0x96U - AES invalid is last chunk */
	XASUFW_AES_UNALIGNED_BLOCK_SIZE_INPUT_LENGTH, /**< 0x97U - AES ECB and CBC modes input data
						should be 16Bytes aligned */
	XASUFW_AES_INVALID_TAG, /**< 0x98U - AES invalid tag length/address for respective engine
						modes */
	XASUFW_AES_TAG_GENERATE_FAILED, /**< 0x99U - AES tag generation failed */
	XASUFW_AES_TAG_COMPARE_FAILED, /**< 0x9AU - AES tag comparison failed */
	XASUFW_AES_MODULE_REGISTRATION_FAILED, /**< 0x9BU - AES module registration failed */
	XASUFW_AES_CONFIG_INIT_FAILED, /**< 0x9CU - AES config initialization failed */
	XASUFW_AES_WRITE_KEY_FAILED, /**< 0x9DU - AES write key failed */
	XASUFW_AES_INIT_FAILED, /**< 0x9EU - AES initialization failed */
	XASUFW_AES_UPDATE_FAILED, /**< 0x9FU - AES update failed */
	XASUFW_AES_FINAL_FAILED, /**< 0xA0U - AES final failed */
	XASUFW_AES_CCM_INVALID_OPERATION_FLAGS, /**< 0xA1U - AES CCM invalid operation flags */
	XASUFW_AES_CCM_AAD_FORMATTING_FAILED, /**< 0xA2U - AES CCM AAD formatting failed */
	XASUFW_AES_ECB_CBC_DUMMY_ENCRYPTION_FAILED, /**< 0xA3U - Error when AES CBC/ECB dummy
						encryption fails */
	XASUFW_AES_DPA_CM_KAT_CHECK_FAILED, /**< 0xA4U - AES DPA CM check1 failed */
	XASUFW_AES_KAT_FAILED, /**< 0xA5U - AES KAT failed */
	XASUFW_AES_DPA_CM_KAT_FAILED, /**< 0xA6U - AES DPA CM KAT failed */
	XASUFW_AES_DPA_CM_ENC_OP_FAILED, /**< 0xA7U - AES DPA CM KAT encrypt operation failed */
	XASUFW_AES_DPA_CM_DEC_OP_FAILED, /**< 0xA8U - AES DPA CM KAT decrypt operation failed */
	XASUFW_RESOURCE_DISABLED, /**< 0xA9U - Resource disabled due to KAT failure. */
	XASUFW_RSA_INVALID_PARAM, /**< 0xAAU - Invalid parameters to RSA APIs */
	XASUFW_RSA_PUB_EXP_INVALID_VALUE, /**< 0xABU - Error in Public exponent value */
	XASUFW_RSA_MOD_DATA_IS_ZERO, /**< 0xACU - Modulus data is zero */
	XASUFW_RSA_MOD_DATA_INVALID, /**< 0xADU - if Input data is out of valid range
							(must satisfy: 1 < InputData < (Modulus - 1)) */
	XASUFW_ERR_RESERVED_0AE, /**< 0xAEU - Reserved */
	XASUFW_RSA_RAND_GEN_ERROR, /**< 0xAFU - Random number generation failed to RSA APIs */
	XASUFW_RSA_KEY_PAIR_COMP_ERROR, /**< 0xB0U - Key pair comparison failure to RSA APIs */
	XASUFW_RSA_ERROR, /**< 0xB1U - Any other error to RSA APIs */
	XASUFW_RSA_CRT_OP_ERROR, /**< 0xB2U - Error in CRT operation */
	XASUFW_RSA_PVT_OP_ERROR, /**< 0xB3U - Error in Private exponentiation operation */
	XASUFW_RSA_PUB_OP_ERROR, /**< 0xB4U - Error in Public exponentiation operation */
	XASUFW_RSA_MASK_GEN_DATA_BLOCK_ERROR, /**< 0xB5U - Error when MGF returns error for data
						block */
	XASUFW_RSA_MASK_GEN_SEED_BUFFER_ERROR, /**< 0xB6U - Error when MGF returns error seed buffer */
	XASUFW_ZEROIZE_MEMSET_FAIL, /**< 0xB7U - Error when memory zeroization fails */
	XASUFW_RSA_OAEP_ENCRYPT_ERROR, /**< 0xB8U - Error when OAEP encryption operation fails */
	XASUFW_RSA_OAEP_ENCODE_ERROR, /**< 0xB9U - Error when OAEP encode operation fails */
	XASUFW_RSA_OAEP_INVALID_LEN, /**< 0xBAU - Error when OAEP input len is invalid */
	XASUFW_RSA_OAEP_DECRYPT_ERROR, /**< 0xBBU - Error when OAEP decryption operation fails */
	XASUFW_RSA_OAEP_DECODE_ERROR, /**< 0xBCU - Error when OAEP decode operation fails */
	XASUFW_RSA_OAEP_HASH_CMP_FAIL,	/**< 0xBDU - Error when OAEP decode operation fails for hash
						comparison failure */
	XASUFW_RSA_LOOP_INDEX_CMP_ERROR, /**< 0xBEU - Error when index of loop comparison failure */
	XASUFW_RSA_PSS_INVALID_LEN,	/**< 0xBFU - Error when PSS signature len is invalid */
	XASUFW_RSA_PSS_INVALID_SALT_LEN, /**< 0xC0U - Error when PSS salt len is invalid */
	XASUFW_RSA_PSS_SIGN_GEN_ERROR, /**< 0xC1U - Error when PSS sign generation operation fails */
	XASUFW_RSA_PSS_ENCRYPT_ERROR, /**< 0xC2U - Error when PSS encryption operation fails */
	XASUFW_RSA_SHA_DIGEST_CALC_FAIL, /**< 0xC3U - Error when SHA digest calculation fail to RSA
						API */
	XASUFW_RSA_PSS_RIGHT_MOST_CMP_FAIL, /**< 0xC4U - Error when PSS decode operation fails for
						last octet comparison failure */
	XASUFW_RSA_PSS_LEFT_MOST_BIT_CMP_FAIL, /**< 0xC5U - Error when PSS decode operation fails for
						first bit in first octet comparison failure */
	XASUFW_RSA_MASK_GEN_ERROR, /**< 0xC6U - Error when MGF fails in RSA */
	XASUFW_RSA_PSS_DECODE_ERROR, /**< 0xC7U - Error when PSS decode operation fails */
	XASUFW_RSA_PSS_HASH_CMP_FAIL, /**< 0xC8U - Error when PSS decode operation fails for hash
						comparison failure */
	XASUFW_RSA_PSS_SIGN_VER_ERROR, /**< 0xC9U - Error when PSS sign verification operation fails */
	XASUFW_RSA_PSS_DECRYPT_ERROR, /**< 0xCAU - Error when PSS decryption operation fails */
	XASUFW_RSA_MODULE_REGISTRATION_FAILED, /**< 0xCBU - RSA module registration failed */
	XASUFW_RSA_ENCRYPT_DATA_COMPARISON_FAILED, /**< 0xCCU - Error when RSA encrypt output
						comparison failed */
	XASUFW_RSA_DECRYPT_DATA_COMPARISON_FAILED, /**< 0xCDU - Error when RSA decrypt output
						comparison failed */
	XASUFW_RSA_KAT_FAILED, /**< 0xCEU - Error when RSA KAT failed */
	XASUFW_DMA_COPY_FAIL, /**< 0xCFU - When data transfer to/from memory using DMA fails */
	XASUFW_MEM_COPY_FAIL,  /**< 0xD0U - Error When copy data to memory fails */
	XASUFW_ECDH_INVALID_POINT_ON_CRV, /**< 0x0D1U - Error when generated point is invalid */
	XASUFW_ECDH_RAND_GEN_ERROR, /**< 0xD2U - Random number generation failed to ECDH APIs */
	XASUFW_ECDH_OTHER_ERROR, /**< 0xD3U - Any generic error from ECDH APIs */
	XASUFW_ECDH_GEN_SECRET_OPERATION_FAIL, /**< 0xD4U - Error when generate secret failed */
	XASUFW_ECDH_SECRET_COMPARISON_FAILED, /**< 0xD5U - Error when generated secret comparison
						failed */
	XASUFW_ECDH_KAT_FAILED, /**< 0xD6U - Error when ECDH KAT failed */
	XASUFW_HMAC_INVALID_PARAM, /**< 0xD7U - Invalid parameters to HMAC APIs */
	XASUFW_HMAC_INVALID_KEY_LENGTH, /**< 0xD8U - Invalid key length */
	XASUFW_HMAC_INIT_FAILED, /**< 0xD9U - HMAC init failed */
	XASUFW_HMAC_INVALID_HASHLEN, /**< 0xDAU - HMAC invalid hash length */
	XASUFW_HMAC_STATE_MISMATCH_ERROR, /**< 0xDBU - HMAC state mismatch error. Occurs when previous
						HMAC state doesn't match before further update */
	XASUFW_HMAC_INITIALIZATION_FAILED, /**< 0xDCU - HMAC initialization failed */
	XASUFW_HMAC_UPDATE_FAILED, /**< 0xDDU - HMAC update failed */
	XASUFW_HMAC_FINAL_FAILED, /**< 0xDEU - HMAC final failed */
	XASUFW_HMAC_KAT_COMPARISON_FAILED, /**< 0xDFU - HMAC comparison failed in KAT */
	XASUFW_HMAC_KAT_FAILED, /**< 0xE0U - HMAC KAT failed */
	XASUFW_HMAC_MODULE_REGISTRATION_FAILED, /**< 0xE1U - HMAC module registration failed */
	XASUFW_KDF_INVALID_PARAM, /**< 0xE2U - Invalid parameters to the KDF APIs */
	XASUFW_KDF_ITERATION_COUNT_MISMATCH, /**< 0xE3U - Failure in running desired number of
						iterations for KDF output */
	XASUFW_KDF_MODULE_REGISTRATION_FAILED, /**< 0xE4U - KDF module registration failed */
	XASUFW_KDF_GENERATE_FAILED, /**< 0xE5U - KDF generate failed */
	XASUFW_KDF_KAT_COMPARISON_FAILED, /**< 0xE6U - KDF KAT comparison failed */
	XASUFW_KDF_KAT_FAILED, /**< 0xE7U - KDF KAT failed */
	XASUFW_TRNG_GET_RANDOM_NUMBERS_TIMEDOUT, /**< 0xE8U - TRNG Get Random numbers timed out */
	XASUFW_TRNG_INVALID_RANDOM_NUMBER, /**< 0xE9U - Invalid random number generated */
	XASUFW_RSA_CHANGE_ENDIANNESS_ERROR, /**< 0xEAU - Error when change endianness in RSA fails */
	XASUFW_RSA_INVALID_PRIME_TOT_FLAG, /**< 0xEBU - Error when invalid flag for prime number or
						totient is given */
	XASUFW_ECIES_INVALID_PARAM, /**< 0xECU - Invalid parameters to ECIES APIs */
	XASUFW_ECIES_PVT_KEY_GEN_FAILURE, /**< 0xEDU - Error when private key generation failed */
	XASUFW_ECIES_PUB_KEY_GEN_FAILURE, /**< 0xEEU - Error when public key generation failed */
	XASUFW_ECIES_ECDH_FAILURE, /**< 0xEFU - Error when ECDH operation failed */
	XASUFW_ECIES_HKDF_FAILURE, /**< 0xF0U - Error when HKDF operation failed */
	XASUFW_ECIES_AES_WRITE_KEY_FAILURE, /**< 0xF1U - Error when AES write key operation failed */
	XASUFW_ECIES_AES_FAILURE, /**< 0xF2U - Error when AES operation failed */
	XASUFW_ECIES_MODULE_REGISTRATION_FAILED, /**< 0xF3U - ECIES module registration failed */
	XASUFW_ECIES_ENCRYPT_FAILED, /**< 0xF4U - ECIES encryption failed */
	XASUFW_ECIES_DECRYPT_FAILED, /**< 0xF5U - ECIES decryption failed */
	XASUFW_ECIES_KAT_FAILED, /**< 0xF6U - ECIES KAT failed */
	XASUFW_ECIES_KAT_COMPARISON_FAILED, /**< 0xF7U - ECIES KAT comparison failed */
	XASUFW_ERR_KV_INTERRUPT_DONE_TIMEOUT, /**< 0xF8U - KV interrupt done timeout error */
	XASUFW_KEYWRAP_INVALID_PARAM, /**< 0xF9U - Invalid parameters to Key wrap unwrap APIs */
	XASUFW_KEYWRAP_MODULE_REGISTRATION_FAILED, /**< 0xFAU - Key wrap unwrap module registration
						failed */
	XASUFW_KEYWRAP_GEN_WRAPPED_KEY_OPERATION_FAIL, /**< 0xFBU - Key wrap output generation
						failed */
	XASUFW_KEYWRAP_GEN_UNWRAPPED_KEY_OPERATION_FAIL,  /**< 0xFCU - Key unwrap output generation
						failed */
	XASUFW_KEYWRAP_AES_WRAPPED_KEY_ERROR,  /**< 0xFDU - AES Key wrap output generation failed */
	XASUFW_KEYWRAP_AES_UNWRAPPED_KEY_ERROR, /**< 0xFEU - AES Key unwrap output generation failed */
	XASUFW_KEYWRAP_ICV_CMP_FAIL, /**< 0xFFU - Error when integrity check value fails for key
						unwrap */
	XASUFW_KDF_ERROR, /**< 0x100U - Generic error in KDF */
	XASUFW_KEYWRAP_AES_KEY_CLEAR_FAIL, /**< 0x101U - Error when AES key clear fails */
	XASUFW_HMAC_ERROR, /**< 0x102U - Generic error in HMAC */
	XASUFW_KEYWRAP_AES_DATA_CALC_FAIL, /**< 0x103U - When AES operation fails in key wrap unwrap */
	XASUFW_KEYWRAP_UNWRAPPED_DATA_COMPARISON_FAILED, /**< 0x104U -Error when unwrapped output
						comparison failed in KAT */
	XASUFW_KEYWRAP_KAT_FAILED, /**< 0x105U - Error when key wrap unwrap KAT failed */
	XASUFW_RSA_ECC_PWCT_SIGN_GEN_FAIL, /**< 0x106U - Sign generation failure in PWCT */
	XASUFW_RSA_ECC_PWCT_SIGN_VER_FAIL, /**< 0x107U - Sign verification failure in PWCT */
	XASUFW_RSA_ECC_INCORRECT_CURVE, /**< 0x108U - Error when the received curvelen or curve size
						is incorrect. */
	XASUFW_RSA_ECC_TRNG_FAILED, /**< 0x109U -  Error in TRNG while generating ECC private key */
	XASUFW_RSA_ECC_MOD_ORDER_FAILED, /**< 0x10AU - Error when ModEccOrder failed in generate
						private key */
	XASUFW_AES_KEY_CONFIG_READBACK_ERROR, /**< 0x10BU -  Error when AES KEY configuration is
						incorrect. */
	XASUFW_HKDF_INVALID_PARAM, /**< 0x10CU - Error when Invalid parameters to the HKDF APIs */
	XASUFW_HKDF_KAT_COMPARISON_FAILED, /**< 0x10DU - Error when HKDF KAT comparison failed */
	XASUFW_HKDF_EXTRACT_FAILED, /**< 0x10EU - Error when HKDF Key extract failed */
	XASUFW_HKDF_GET_HASHLEN_FAILED, /**< 0x10FU - Error when Unable to get hash length from SHA Mode */
	XASUFW_HKDF_HMAC_INIT_FAILED, /**< 0x110U - Error when HMAC init operation failed in HKDF */
	XASUFW_HKDF_HMAC_UPDATE_FAILED, /**< 0x111U - Error when HMAC update failed in HKDF */
	XASUFW_HKDF_HMAC_FINAL_FAILED, /**< 0x112U - Error when HMAC hash generation failed in HKDF */
	XASUFW_HKDF_GENERATE_FAILED, /**< 0x113U - Error when HKDF generate operation failed  */
	XASUFW_KEYWRAP_INVALID_OUTPUT_BUF_LEN, /**< 0x114U -  Error when output buffer length provided by
							user is less than required. */
	XASUFW_KEYWRAP_INVALID_PAD_LEN, /**< 0x115U -  Error when output data has invalid padding
							length. */
	XASUFW_KEYWRAP_INVALID_PAD_VALUE, /**< 0x116U -  Error when output data has invalid padding
							value i.e other than zeroes. */
	XASUFW_KEYWRAP_CHANGE_ENDIANNESS_ERROR, /**< 0x117U - Error when change endianness in key
							wrap fails */
	XASUFW_KEYWRAP_LOOP_INDEX_CMP_ERROR, /**< 0x118U - Error when index of loop comparison failure */
	XASUFW_ECC_SCP_DISABLE_FAILED, /**< 0x119U - Error when SCP is not disabled for ECC */
	XASUFW_ECC_SCP_RANDOM_NUM_GEN_FAIL, /**< 0x11AU - Error when SCP random number generation
						is failed */
	XASUFW_ECC_SCP_RANDOM_NUM_UPDATE_FAIL, /**< 0x11BU - Error when SCP random number update
						is failed */
	XASUFW_ECC_SCP_RANDOM_NUM_COUNT_FAIL, /**< 0x11CU - Error when SCP random number generation
						count is not matched with the input count */
	XASUFW_ECC_CONFIGURE_AND_START_FAIL, /**< 0x11DU - Error when ECC configuration failed */

	XASUFW_X509_UNEXPECTED_TAG = 0x11EU, /**< 0x11EU - Error when unexpected tag is found */
	XASUFW_X509_INVALID_FIELD_LEN, /**< 0x11FU - Error when field length is invalid */
	XASUFW_X509_INVALID_BUFFER_SIZE, /**< 0x120U - Error when certificate size is invalid */
	XASUFW_X509_INVALID_DATA, /**< 0x121U - Error when data is invalid */
	XASUFW_X509_UNSUPPORTED_ALGORITHM, /**< 0x122U - Error when algorithm id is unsupported */
	XASUFW_X509_UNSUPPORTED_EXTN, /**< 0x123U - Error when extension is unsupported */
	XASUFW_X509_BOOLEAN_TAG_NOT_FOUND, /**< 0x124 - Error when boolean tag is not found */
	XASUFW_X509_INVALID_PARAM, /**< 0X125U - Error when parameter is invalid */
	XASUFW_X509_PARSER_TAG_VALIDATION_FAILED, /**< 0x126U - Error when tag validation failed
							during x509 parsing */
	XASUFW_X509_PARSER_TBS_INVALID_TAG, /**< 0x127U - Error when invalid tag is found during
							TBS parsing */
	XASUFW_X509_PARSER_GET_VERSION_FAIL, /**< 0x128U - Error when parsing x509 version
							parsing failed */
	XASUFW_X509_PARSER_SERIAL_NO_INVALID_TAG, /**< 0x129U - Error when invalid tag found while
							x509 serial number parsing */
	XASUFW_X509_PARSER_SIGN_ALGO_INVALID_TAG, /**< 0x12AU - Error when invalid tag found while
							x509 parsing signature algorithm */
	XASUFW_X509_PARSER_ISSUER_INVALID_TAG, /**< 0x12BU - Error when invalid tag found while
							parsing x509 issuer */
	XASUFW_X509_PARSER_VALIDITY_INVALID_INFO, /**< 0x12CU - Error when invalid information found
							while parsing x509 validity */
	XASUFW_X509_PARSER_VALIDITY_FROM_FAIL, /**< 0x12DU - Error when parsing x509 validity
							"From" field */
	XASUFW_X509_PARSER_VALIDITY_TO_FAIL, /**< 0x12EU - Error when parsing x509 validity
							"To" field */
	XASUFW_X509_PARSER_SUB_INVALID_TAG, /**< 0x12FU - Error when invalid tag found while parsing
							x509 subject */
	XASUFW_X509_PARSER_PUB_KEY_ALGO_FAIL, /**< 0x130U - Error when parsing public key algorithm
							failed */
	XASUFW_X509_PARSER_EXT_KEY_USAGE_FAIL, /**< 0x131U - Error when x509 key usage extension
							parsing failed */
	XASUFW_X509_PARSER_UPDATE_OFFSET_FAIL, /**< 0x132U - Error when update offset failed */
	XASUFW_X509_PARSER_GET_FIELD_LEN_FAIL, /**< 0x133U - Error when get field length failed */
	XASUFW_X509_PARSER_VALIDITY_FAIL, /**< 0x134U - Error when parsing validity is failed */
	XASUFW_X509_PARSER_PUBLIC_KEY_INFO_FAIL, /**< 0x135U - Error when public key info parsing
							is failed */
	XASUFW_X509_PARSER_EXTENSION_INFO_FAIL, /**< 0x136U - Error when extension parsing is
							failed */
	XASUFW_X509_PARSER_GET_EXTN_OID_FAIL, /**< 0x137U - Error when get the extension OID is
							failed */
	XASUFW_X509_UNSUPPORTED_CURVE_TYPE, /**< 0x138U - Error when curve type is unsupported */
	XASUFW_X509_UNSUPPORTED_SIGN_TYPE, /**< 0x139U - Error when signature type in unsupported */
	XASUFW_X509_PARSER_ASCII_TO_INT_CONV_ERROR, /**< 0x13AU - Error when ASCII to integer conversion failed during x509 parsing */
	XASUFW_X509_GEN_SIGN_ECC_FAIL, /**< 0x13BU - Error when ECC key generation is failed */
	XASUFW_X509_SHA_DIGEST_FAIL, /**< 0x13CU - Error when SHA digest calculation is failed */
	XASUFW_X509_DMA_RELEASE_FAIL, /**< 0x13DU - Error when DMA release is failed */
	XASUFW_X509_GEN_PUB_KEY_INFO_FIELD_FAIL, /**< 0x13EU - Error when public key information
						field generation is failed */
	XASUFW_X509_GEN_VERSION_FIELD_FAIL, /**< 0x13FU - Error when version field generation is
						failed */
	XASUFW_X509_GEN_SIGN_ALGO_FIELD_FAIL, /**< 0x140U - Error when signature algorithm field
						generation is failed */
	XASUFW_X509_GEN_ISSUER_FIELD_FAIL, /**< 0x141U - Error when issuer field generation is
						failed*/
	XASUFW_X509_GEN_VALIDITY_FIELD_FAIL, /**< 0x142U - Error when validity field generation
						is failed */
	XASUFW_X509_GEN_EXTENSION_FIELD_FAIL, /**< 0x143U - Error when extension field generation
						is failed */
	XASUFW_X509_GEN_SUB_KEY_IDENTIFIER_FIELD_FAIL, /**< 0x144U - Error when subject key
							identifier field generation is failed */
	XASUFW_X509_GEN_AUTH_KEY_IDENTIFIER_FIELD_FAIL,/**< 0x145U - Error when authority key
							identifier field generation is failed */
	XASUFW_X509_GEN_KEY_USAGE_FIELD_FAIL, /**< 0x146U - Error when key usage field generation
						is failed */
	XASUFW_X509_GEN_EXT_KEY_USAGE_FIELD_FAIL, /**< 0x147U - Error when extended key usage field
							generation is failed */
	XASUFW_X509_GEN_SUB_ALT_NAME_FIELD_FAIL, /**< 0x148U - Error when subject alternate name
							field generation is failed */
	XASUFW_X509_GEN_SUBJECT_FIELD_FAIL, /**< 0x149U - Error when subject field generation is
						failed */
	XASUFW_X509_GEN_SERIAL_FIELD_FAIL, /**< 0x14AU - Error when serial field generation is
						failed */
	XASUFW_X509_GEN_SIGN_FIELD_FAIL, /**< 0x14BU - Error when signature field generation is
						failed */
	XASUFW_X509_UPDATE_ENCODED_LEN_FAIL, /**< 0x14CU - Error when encoded length is not
						updated */
	XASUFW_X509_DIGEST_SIGN_CALL_BACK_NOT_REGISTERED, /**< 0x14DU - Error when call back not
							registered for digest/sign calculation */
	XASUFW_X509_INVALID_PLAT_DATA, /**< 0x14EU - Error when platform data is invalid */
	XASUFW_X509_GENERATE_DIGEST_FAIL, /**< 0x14FU - Error when digest generation is failed */
	XASUFW_X509_GENERATE_SIGN_FAIL, /**< 0x150U - Error when signature generation is failed */
	XASUFW_X509_GENERATE_PUB_KEY_ALGO_FIELD_FAIL, /**< 0x151U - Error when public key algorithm
							field generation is failed */
	XASUFW_X509_CREATE_INTEGER_FIELD_FROM_ARRAY_FAIL, /**< 0x152U - Error when create integer
								field from array is failed */
	XASUFW_X509_CREATE_INTEGER_FIELD_FAIL, /**< 0x153U - Error when integer field creation
						failed */
	XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL, /**< 0x154U - Error when raw data field
								creation is failed */
	XASUFW_X509_CREATE_BIT_STRING_FAIL, /**< 0x155U - Error when bit string creation is
						failed */
	XASUFW_X509_CREATE_OCTET_STRING_FAIL, /**< 0x156U - Error when octet string creation is
						failed */
	XASUFW_MEM_MOVE_FAIL, /**< 0x157U - Error when move memory failed */
	XASUFW_X509_CERT_GEN_FAIL, /**< 0x158U - Error when certificate generation failed */
	XASUFW_HMAC_KEY_PROCESS_ERROR, /**< 0x159U - Error when HMAC key processing failed */
	XASUFW_TRNG_MODE_INIT_CFG_FAIL, /**< 0x15AU - Error when TRNG mode init and configuration is
					failed */
	XASUFW_OCP_TRNG_INSTANTIATE_FAIL, /**< 0x15BU - Error when TRNG instantiate is failed */
	XASUFW_OCP_TRNG_UNINSTANTIATE_FAIL, /**< 0x15CU - Error when TRNG uninstantiate is
						failed */
	XASUFW_OCP_TRNG_FAIL, /**< 0x15DU - Error when TRNG is failed */
	XASUFW_OCP_GEN_ECC_PVT_KEY_FAIL, /**< 0x15EU - Error when ECC private key generation is
						failed */
	XASUFW_OCP_INVALID_PARAM, /**< 0x15FU - Error when parameter is invalid */
	XASUFW_OCP_GEN_DEVIK_PVT_KEY_FAIL, /**< 0x160U - Error when DevIk private key generation
						is failed */
	XASUFW_OCP_GEN_DEVIK_PUB_KEY_FAIL, /**< 0x161U - Error when DevIk public key generation
						is failed */
	XASUFW_OCP_GET_ASU_CDI_FAIL, /**< 0x162U - Error when get ASU CDI is failed */
	XASUFW_OCP_SEND_IPI_REQ_FAIL, /**< 0x163U - Error when ipi request is failed */
	XASUFW_OCP_READ_IPI_RESP_FAIL, /**< 0x164U - Error when ipi response is failed */
	XASUFW_OCP_GEN_DEVAK_PVT_KEY_FAIL, /**< 0x165U - Error when DevAk private key generation
						is failed */
	XASUFW_OCP_GEN_DEVAK_PUB_KEY_FAIL, /**< 0x166U - Error when DevAk public key generation
						is failed */
	XASUFW_OCP_GEN_SUBSYSTEM_CDI_FAIL, /**< 0x167U - Error when subsystem CDI generation is
						failed */
	XASUFW_OCP_INVALID_SUBSYSTEM_INDEX, /**< 0x168U Error when parameter is invalid */
	XASUFW_OCP_GET_SUBSYSTEM_HASH_FAIL, /**< 0x169U - Error when retrieved subsystem
							hash is failed */
	XASUFW_READ_IPI_MSG_FROM_PLM_FAIL, /**< 0x16AU - Error when reading IPI message from PLM */
	XASUFW_PLM_EVENT_HANDLING_FAIL, /**< 0x16BU - Error when PLM event handling is failed */
	XASUFW_PLM_MODULE_REGISTRATION_FAIL, /**< 0x16CU - Error when PLM module registration is
						failed */
	XASUFW_OCP_DEVIK_GENERATION_FAIL, /**< 0x16DU - Error when DevIk key pair generation is
						failed */
	XASUFW_OCP_DEVAK_GENERATION_FAIL, /**< 0x16EU - Error when DevAk key pair generation is
						failed */
	XASUFW_PLM_INVALID_CMD_HEADER, /**< 0x16FU - Error when IPI command header is invalid */
	XASUFW_PLM_DEV_KEYS_GEN_FAIL, /**< 0x170U - Error when device keys generation is failed */
	XASUFW_OCP_X509_MODULE_INIT_FAIL, /**< 0x171U - OCP X.509 module initialization is failed */
	XASUFW_OCP_MODULE_REGISTRATION_FAIL, /**< 0x172U - OCP module registration is failed */
	XASUFW_OCP_X509_CERT_GEN_FAIL, /**< 0x173U - X.509 certificate generation is failed */
	XASUFW_OCP_DEVICE_KEY_TYPE_INVALID, /**< 0x174U - Error when device key type is invalid */
	XASUFW_INVALID_SUBSYSTEM_ID, /**< 0x175U - Error when subsystem ID is invalid */
	XASUFW_OCP_KEY_MGMT_NOT_READY, /**< 0x176U - Error when DevIk pair is not ready */
	XASUFW_OCP_DEVAK_NOT_READY, /**< 0x177U - Error when DevAk pair is not ready */
	XASUFW_OCP_UDE_KEK_ZEROIZE_FAIL, /**< 0x178U - Error when UDE KEK zeroization fails */
	XASUFW_OCP_UDE_KEK_GEN_FAIL, /**< 0x179U - Error when UDE KEK generation fails */
	XASUFW_OCP_UDE_PUF_KEK_GEN_FAIL, /**< 0x17AU - Error when PUF KEK generation fails */
	XASUFW_OCP_INVALID_BUF_SIZE, /**< 0x17B - Error when OCP buffer size is invalid */
	XASUFW_VALIDATE_CMD_INVALID_CHANNEL_INDEX, /**< 0x17CU - Error when channel index is invalid */
	XASUFW_ERR_VALIDATE_IPI_NO_IPI_ACCESS, /**< 0x17DU - Error when the requested command lacks
						required IPI access permissions for the request type */
	XASUFW_ERR_RESERVED_17E, /**< 0x17EU - Reserved */
	XASUFW_ERR_RESERVED_17F, /**< 0x17FU - Reserved */
	XASUFW_ERR_NO_ACCESS_PERMISSIONS, /**< 0x180U - Error when no access permissions are
						available */
	XASUFW_UPDATE_ACCESS_PERM_INVALID_MODULE_INFO, /**< 0x181U - Error when module information is
						invalid */
	XASUFW_ERR_UPDATE_ACCESS_PERM_FAILED, /**< 0x182U - Error when updating access permissions */
	XASUFW_OCP_UDE_ALL_PVT_KEYS_REVOKED, /**< 0x183U - Error when all UDE private keys are revoked */
	XASUFW_OCP_UDE_IV_IS_ZERO, /**< 0x184U - Error when UDE IV is zero */
	XASUFW_OCP_DEVICE_ID_CALC_FAIL, /**< 0x185U - Error when device ID calculation is failed */
	XASUFW_OCP_NONCE_UPDATE_FAIL, /**< 0x186U - Error when nonce update is failed */
	XASUFW_OCP_MEASUREMENT_UPDATE_FAIL, /**< 0x187U - Error when measurement update is failed */
	XASUFW_OCP_UDE_SIGNATURE_GEN_FAIL, /**< 0x188U - Error when UDE signature generation is failed */
	XASUFW_OCP_AES_WRITE_KEY_FAILURE, /**< 0x189U - Error when AES write key operation fails */
	XASUFW_OCP_UDE_KEY_ENCRYPT_FAIL, /**< 0x18AU - Error when UDE key encryption is failed */
	XASUFW_OCP_UDE_KEY_DECRYPT_FAIL, /**< 0x18BU - Error when UDE key decryption is failed */
	XASUFW_OCP_UDE_PVT_KEY_GEN_FAIL, /**< 0x18CU - Error when UDE private key generation fails */
	XASUFW_CRYPTO_DISABLED, /**< 0x18DU - Error when crypto is disabled via eFuse */
	XASUFW_OCP_UDE_AES_COMPUTE_FAIL, /**< 0x18EU - Error when UDE AES compute operation fails */
	XASUFW_OCP_UDE_CHALLENGE_RESPONSE_FAIL, /**< 0x18FU - Error when UDE challenge response operation fails */
	XASUFW_OCP_SHA_DIGEST_FAIL, /**< 0x190U - Error when SHA digest operation fails */
	XASUFW_OCP_UDE_CHANGE_ENDIANNESS_ERROR, /**< 0x191U - Error when UDE change endianness operation fails */
	XASUFW_X509_PARSER_TBS_FAIL, /**< 0x192U - Error when TBS parsing is failed. */
	XASUFW_X509_PARSER_SIGN_INFO_FAIL, /**< 0x193U - Error when signature information parsing
							is failed. */
	XASUFW_X509_PARSER_SIGN_FAIL, /**< 0x194U - Error when parsing signature is failed */
	XASUFW_X509_PARSER_SIGN_ALGO_FAIL, /**< 0x195U - Error when signature algorithm parsing is
						failed */

	XASUFW_ECIES_PUB_KEY_VALIDATE_FAILURE, /**< 0x196U - Error when public key validation fails
						in ECIES */
	XASUFW_OCP_UDE_KEK_NOT_PRESENT, /**< 0x197U - Error when UDE KEK is not present */

	XASUFW_AES_CONTEXT_SAVE_FAIL, /**< 0x198U - Error when AES context saving fails */
	XASUFW_AES_CONTEXT_USER_KEY_RESTORE_FAIL, /**< 0x199U - Error when AES context user key restore fails */
	XASUFW_AES_CONTEXT_RESTORE_FAIL, /**< 0x19AU - Error when AES context restore fails */
	XASUFW_AES_INVALID_INTERNAL_STATE, /**< 0x19BU - Error when AES invalid internal state. */
	XASUFW_SW_ERR_INIT_FAIL, /**< 0x19CU - Error when error management initialization fails. */
	XASUFW_ECC_KEY_PAIR_GENERATION_FAIL, /**< 0x19DU - Error when ECC key pair generation fails. */
	XASUFW_RSA_ECC_GEN_PVT_KEY_OPERATION_FAIL, /**< 0x19EU - Error when ECC private key generation operation fails. */
	XASUFW_SSS_CFG_GET_FAIL, /**< 0x19FU - Error when retrieved SSS configuration fails. */
	XASUFW_SSS_CFG_CHECK_FAIL, /**< 0x1A0U - Error when SSS configuration is invalid. */
	XASUFW_SHA_UPDATE_FAIL, /**< 0x1A1U - Error when SHA update fails. */
	XASUFW_INVALID_CMD_STAGE, /**< 0x1A2U - Error for invalid command stage. */
	XASUFW_OCP_GET_ASU_CDI_FROM_PLM_FAIL, /**< 0x1A3U - Error when getting ASU CDI from PLM
						fails. */
	XASUFW_OCP_UDE_KEY_NOT_PROGRAMMED, /**< 0x1A4U - Error when UDE key is not programmed in eFuses. */
	XASUFW_OCP_UDE_PUBLIC_KEY_GEN_FAIL, /**< 0x1A5U - Error when UDE public key generation fails. */
	XASUFW_KEYMANAGER_INVALID_PARAM, /**< 0x1A6U - Invalid parameters to Keyvault APIs. */
	XASUFW_KEYMANAGER_MODULE_REGISTRATION_FAILED, /**< 0x1A7U - Error when Keyvault module registration fails. */
	XASUFW_KEYMANAGER_KEY_VAULT_DDR_INIT_FAILED, /**< 0x1A8U - Error when Keyvault DDR initialization fails. */
	XASUFW_LOOP_INDEX_CMP_ERROR, /**< 0x1A9U - Error when loop index comparison fails. */
	XASUFW_KEYMANAGER_INVALID_KEY_TYPE, /**< 0x1AAU - Error when Keyvault key type is invalid. */
	XASUFW_KEYMANAGER_VAULT_GEN_ERROR, /**< 0x1ABU - Error when vault generation fails. */
	XASUFW_KEYMANAGER_VAULT_DELETE_ERROR, /**< 0x1ACU - Error when vault deletion fails. */
	XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR, /**< 0x1ADU - Error when key object generation fails. */
	XASUFW_KEYMANAGER_IV_GEN_ERROR, /**< 0x1AEU - Error when IV generation fails. */
	XASUFW_KEYMANAGER_MAX_CAPACITY_REACHED, /**< 0x1AFU - Error when key vault maximum capacity is reached. */
	XASUFW_RAND_GEN_ERROR, /**< 0x1B0U - Error when random number generation fails. */
	XASUFW_KEYMANAGER_UPDATE_INFO_FAIL, /**< 0x1B1U - Error when key vault information update fails. */
	XASUFW_KEYMANAGER_INVALID_VAULT_ACCESS, /**< 0x1B2U - Error when vault access is invalid for the operation. */
	XASUFW_KEYMANAGER_VAULT_NOT_FOUND, /**< 0x1B3U - Error when specified vault for a subsystem is not found. */
	XASUFW_KEYMANAGER_INVALID_DDR_ADDRESS, /**< 0x1B4U - Error when key vault DDR address is invalid. */
	XASUFW_KEYMANAGER_NO_SPACE, /**< 0x1B5U - Error when there is no space left to create key vault. */
	XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, /**< 0x1B6U - Error when getting key object fails. */
	XASUFW_KEYMANAGER_KEY_USE_CASE_ERROR, /**< 0x1B7U - Error when key use case is invalid. */
	XASUFW_KEYMANAGER_ACCESS_DENIED, /**< 0x1B8U - Error when access right violation occurs. */
	XASUFW_KEYMANAGER_KEY_EXPIRED, /**< 0x1B9U - Error when key is expired. */
	XASUFW_OCP_MAX_SUBSYSTEMS_EXCEEDED, /**< 0x1BAU - Error when number of subsystems in CDO
						exceeds maximum */
	XASUFW_AES_ERR_CTX_RESTORE_GLITCH, /**< 0x1BBU - Error when AES context restore did not
						complete, possibly due to glitch or control flow
						disruption. */
	XASUFW_KEYMANAGER_KEY_NOT_FOUND, /**< 0x1BCU - Error when specified key is not found in vault. */
	XASUFW_KEYMANAGER_KEY_DELETE_ERROR, /**< 0x1BDU - Error when key deletion fails. */
	XASUFW_KEYMANAGER_UPDATE_PVT_KEY_FAIL, /**< 0x1BEU - Error when updating private key fails. */
	XASUFW_KEYMANAGER_UPDATE_PUB_KEY_FAIL, /**< 0x1BFU - Error when updating public key fails. */
	XASUFW_RSA_KEY_PAIR_GENERATION_FAIL, /**< 0x1C0U - Error when RSA key pair generation fails. */
	XASUFW_KEYMANAGER_GET_KEYOBJ_PTR_FAILED, /**< 0x1C1U - Error when getting key object pointer fails. */
	XASUFW_SHA_DIGEST_CALC_FAILED, /**< 0x1C2U - Error when SHA digest calculation is failed. */
	XASUFW_SHA_START_FAILED, /**< 0x1C3U - Error when SHA start operation fails. */
	XASUFW_LMS_INVALID_PARAM, /**< 0x1C4U - LMS invalid parameter error */
	XASUFW_LMS_OTS_PUB_KEY_SIGN_TYPE_MISMATCH_ERROR, /**< 0x1C5U - LMS OTS public key and signature type mismatch */
	XASUFW_LMS_OTS_SIGN_UNSUPPORTED_TYPE_ERROR, /**< 0x1C6U - LMS OTS signature unsupported type */
	XASUFW_LMS_OTS_DIGEST_CHECKSUM_OP_FAILED_ERROR, /**< 0x1C7U - LMS OTS digest checksum operation failed */
	XASUFW_LMS_DIGEST_CHECKSUM_FAILED_ERROR, /**< 0x1C8U - LMS digest checksum failed */
	XASUFW_LMS_OTS_SIGN_SHA_DIGEST_FAILED_ERROR, /**< 0x1C9U - LMS OTS signature SHA digest failed */
	XASUFW_LMS_PUB_KEY_UNSUPPORTED_TYPE_1_ERROR, /**< 0x1CAU - LMS public key unsupported type 1 */
	XASUFW_LMS_SIGN_EXPECTED_PUB_KEY_LEN_2_ERROR, /**< 0x1CBU - LMS signature expected public key length error 2 */
	XASUFW_LMS_SIGN_LEN_1_ERROR, /**< 0x1CCU - LMS signature length error 1 */
	XASUFW_LMS_OTS_PUB_KEY_LMS_OTS_SIGN_TYPE_MISMATCH_ERROR, /**< 0x1CDU - LMS OTS public key and LMS OTS signature type mismatch */
	XASUFW_LMS_SIGN_UNSUPPORTED_OTS_TYPE_1_ERROR, /**< 0x1CEU - LMS signature unsupported OTS type 1 */
	XASUFW_LMS_SIGN_LEN_2_ERROR, /**< 0x1CFU - LMS signature length error 2 */
	XASUFW_LMS_PUB_KEY_LMS_SIGN_TYPE_MISMATCH_ERROR, /**< 0x1D0U - LMS public key and LMS signature type mismatch */
	XASUFW_LMS_SIGN_UNSUPPORTED_TYPE_1_ERROR, /**< 0x1D1U - LMS signature unsupported type 1 */
	XASUFW_LMS_SIGN_INVALID_NODE_NUMBER_ERROR, /**< 0x1D2U - LMS signature invalid node number */
	XASUFW_LMS_SIGN_LEN_3_ERROR, /**< 0x1D3U - LMS signature length error 3 */
	XASUFW_LMS_SIGN_OTS_OP_ERROR, /**< 0x1D4U - LMS signature OTS operation error */
	XASUFW_LMS_SIGN_VERIF_SHA_DIGEST_LEAF_FAILED_ERROR, /**< 0x1D5U - LMS signature verification SHA digest leaf failed */
	XASUFW_LMS_PUB_KEY_AUTHENTICATION_FAILED_ERROR, /**< 0x1D6U - LMS public key authentication failed */
	XASUFW_LMS_PUB_KEY_AUTHENTICATION_GLITCH_ERROR, /**< 0x1D7U - LMS public key authentication glitch error */
	XASUFW_LMS_HSS_PUB_KEY_INVALID_LEN_2_ERROR, /**< 0x1D8U - LMS HSS public key invalid length 2 */
	XASUFW_LMS_HSS_SIGN_LEVEL_UNSUPPORTED_ERROR, /**< 0x1D9U - LMS HSS signature level unsupported */
	XASUFW_LMS_HSS_SIGN_PUB_KEY_LEVEL_MISMATCH_ERROR, /**< 0x1DAU - LMS HSS signature public key level mismatch */
	XASUFW_LMS_HSS_L0_PUB_KEY_LMS_TYPE_UNSUPPORTED_ERROR, /**< 0x1DBU - LMS HSS L0 public key LMS type unsupported */
	XASUFW_LMS_HSS_L0_PUB_KEY_LMS_OTS_TYPE_UNSUPPORTED_ERROR, /**< 0x1DCU - LMS HSS L0 public key LMS OTS type unsupported */
	XASUFW_LMS_SIGN_VERIFY_BH_AND_TYPE_SHA_ALGO_MISMATCH_L0_ERROR, /**< 0x1DDU - LMS signature verify BH and type SHA algo mismatch L0 */
	XASUFW_LMS_HSS_L0_SIGN_INVALID_LEN_2_ERROR, /**< 0x1DEU - LMS HSS L0 signature invalid length 2 */
	XASUFW_LMS_HSS_L0_PUB_KEY_AUTH_FAILED_ERROR, /**< 0x1DFU - LMS HSS L0 public key authentication failed */
	XASUFW_LMS_PUB_OP_FAILED_ERROR, /**< 0x1E0U - LMS public operation failed */
	XASUFW_LMS_PUB_OP_FAILED_1_ERROR, /**< 0x1E1U - LMS public operation failed 1 */
	XASUFW_LMS_HSS_L1_PUB_KEY_LMS_TYPE_1_UNSUPPORTED_ERROR, /**< 0x1E2U - LMS HSS L1 public key LMS type 1 unsupported */
	XASUFW_LMS_HSS_L1_PUB_KEY_LMS_TYPE_2_UNSUPPORTED_ERROR, /**< 0x1E3U - LMS HSS L1 public key LMS type 2 unsupported */
	XASUFW_LMS_HSS_L1_PUB_KEY_LMS_OTS_TYPE_UNSUPPORTED_ERROR, /**< 0x1E4U - LMS HSS L1 public key LMS OTS type unsupported */
	XASUFW_LMS_HSS_OTS_SIGN_INVALID_LEN_1_ERROR, /**< 0x1E5U - LMS HSS OTS signature invalid length 1 */
	XASUFW_LMS_HSS_L1_SIGN_INVALID_LEN_2_ERROR, /**< 0x1E6U - LMS HSS L1 signature invalid length 2 */
	XASUFW_LMS_MODULE_REGISTRATION_FAILED, /**< 0x1E7U - LMS module registration failed */
	XASUFW_LMS_SIGN_VERIFY_FAILED, /**< 0x1E8U - LMS signature verification failed */
	XASUFW_HSS_SIGN_VERIFY_FAILED, /**< 0x1E9U - HSS signature verification failed */
	XASUFW_LMS_TYPE_LOOKUP_GLITCH_ERROR, /**< 0x1EAU - LMS OTS type lookup glitch error */
	XASUFW_LMS_TYPE_UNSUPPORTED_ERROR, /**< 0x1EBU - LMS type unsupported error */
	XASUFW_LMS_OTS_TYPE_UNSUPPORTED_ERROR, /**< 0x1ECU - LMS OTS type unsupported error */
	XASUFW_HSS_KAT_FAILED, /**< 0x1EDU - HSS KAT failed */
	XASUFW_LMS_KAT_COMPARISON_FAILED, /**< 0x1EEU - LMS KAT comparison failed */
	XASUFW_LMS_OTS_CHECKSUM_BUFF_INVALID_LEN_ERROR, /**< 0x1EFU - LMS OTS checksum buffer invalid length */
	XASUFW_LMS_SIGN_INPUT_VALIDATION_FAILED, /**< 0x1F0U - LMS signature input validation failed */
	XASUFW_OCP_GET_EVENT_MASK_FAILED, /**< 0x1F1U - Error when getting ASU event from PLM
						fails */
	XASUFW_REGISTER_NOTIFIER_SEND_IPI_FAILED, /**< 0x1F2U - Error when sending IPI to
							register notifier fails */
	XASUFW_REGISTER_NOTIFIER_READ_IPI_FAILED, /**< 0x1F3U - Error when reading IPI response
							for register notifier fails */

	XASUFW_AES_GHASHCAL_FAILED, /**< 0x1F4U - Error while calculating GHASH. */
	XASUFW_AES_WRITE_WITH_ENDIAN_SWAP_FAIL, /**< 0x1F5U - Error when writing data with endian swap fail. */
	XASUFW_AES_WAIT_FOR_DONE_TIMEOUT, /**< 0x1F6U - When wait for done timed out */
	XASUFW_AES_READ_TAG_FAIL, /**< 0x1F7U - Error when reading AES tag fails. */
	XASUFW_AES_FINALIZE_AAD_UPDATE_FAIL, /**< 0x1F8U - Error when finalizing AAD update fails. */
	XASUFW_AES_CFG_SSS_WITH_DMA_XFER_FAIL, /**< 0x1F9U - Error when configuring SSS with DMA and transferring data fails. */
	XASUFW_AES_MODE_CONFIG_READBACK_ERROR, /**< 0x1FAU - Error when AES mode configuration readback fails,
							possibly due to glitch or control flow disruption. */
	XASUFW_AES_ERR_CTX_SAVED_GLITCH, /**< 0x1FBU - Error when AES context save did not
						complete, possibly due to glitch or control flow
						disruption. */
	XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, /**< 0x1FCU - Error when buffer space validation
						fails */
	XASUFW_X509_CREATE_BOOLEAN_FAIL, /**< 0x1FDU - Error when creating ASN.1 boolean
						field fails*/
	XASUFW_ASU_KAT_FAILED_SECURE_LOCKDOWN, /**< 0x1FEU - ASU secure lockdown triggered due to KAT failure */
	XASUFW_KEYMANAGER_ASU_VAULT_CREATION_FAILED, /**< 0x1FFU - Error when ASU vault creation fails. */
	XASUFW_RSA_INVALID_OUTPUT_BUF_LEN, /**< 0x200U - Error when RSA output buffer length
						provided by user is less than required */
	XASUFW_RSA_KEY_GEN_ADD_TASK_ERROR, /**< 0x201U - Error during RSA key pair generation
						in scheduler task */
	XASUFW_RSA_PWCT_ENCRYPT_FAIL, /**< 0x202U - RSA PWCT encrypt operation failed */
	XASUFW_RSA_PWCT_DECRYPT_FAIL, /**< 0x203U - RSA PWCT decrypt operation failed */
	XASUFW_RSA_PWCT_COMPARISON_FAIL, /**< 0x204U - RSA PWCT comparison failed */
	XASUFW_OCP_UDE_PREPARE_DEC_KEY_FAIL, /**< 0x205U - Error when UDE prepare decryption key operation fails */
	XASUFW_X509_CERT_PARSE_FAIL, /**< 0x206U - Error when parsing x509 certificate fails */
	XASUFW_KEYMANAGER_UPDATE_KEY_VAULT_FAIL, /**< 0x207U - Error when updating key vault
							fails */
	XASUFW_KEYMANAGER_STORE_KEY_ERROR, /**< 0x208U - Error when storing key fails */
	XASUFW_KEYMANAGER_STORE_X509_RAW_KEY_ERROR, /**< 0x209U - Error when storing X.509 raw key
							fails */
	XASUFW_KEYMANAGER_INVALID_BUF_SIZE, /**< 0x20AU - Error when provided buffer size is
						invalid for key object */
	XASUFW_KEYMANAGER_INVALID_EXPONENT_SIZE, /**< 0x20BU - Error when provided exponent size is
						invalid for RSA key object */
	XASUFW_INSUFFICIENT_RSVD_DDR_SIZE, /**< 0x20CU - Error when reserved DDR size is insufficient for the operation. */
	XASUFW_DATA_BACKUP_FAIL, /**< 0x20DU - Error when data backup operation fails. */
	XASUFW_DATA_RESTORE_FAIL, /**< 0x20EU - Error when data restore operation fails. */
	XASUFW_UPDATE_MGR_RELOCATION_FAIL, /**< 0x20FU - Error when update manager relocation fails. */
	XASUFW_ERR_RAM_ECC_UE, /**< 0x210U - RAM ECC uncorrectable error detected */
	XASUFW_ERR_RAM_ECC_CE, /**< 0x211U - RAM ECC correctable error detected */
	XASUFW_KEYMANAGER_EXPORT_VAULT_ERROR, /**< 0x212U - Error when exporting vault fails */
	XASUFW_KEYMANAGER_IMPORT_VAULT_ERROR, /**< 0x213U - Error when importing vault fails */
	XASUFW_KEYMANAGER_KV_KEK_GEN_FAIL, /**< 0x214U - Error when Key Manager KEK generation
						fails */
	XASUFW_KEYMANAGER_AES_KEY_SETUP_FAIL, /**< 0x215U - Error when Key Manager AES key setup
						fails */
	XASUFW_KEYMANAGER_INVALID_ID_STRING, /**< 0x216U - Error when invalid identification string
						is detected. */
	XASUFW_KEYMANAGER_INVALID_VAULT_VERSION, /**< 0x217U - Error when invalid vault version is
						provided for vault import/export. */
	XASUFW_KEYMANAGER_VAULT_REVOKE_ID_MISMATCH, /**< 0x218U - Error when vault revoke ID does
							not match with expected value. */
	XASUFW_KEYMANAGER_INVALID_VAULT_ID, /**< 0x219U - Error when vault ID is invalid. */
	XASUFW_KDF_IV_IS_ZERO, /**< 0x21AU - Error when KDF IV is zero */
	XASUFW_KDF_DECRYPT_BLACK_KEY_0_FAIL, /**< 0x21BU - Error when KDF decrypts black key 0
						fails */
	XASUFW_SHA_FINISH_FAILED, /**< 0x21CU - Error when SHA finish operation fails */
	XASUFW_KEYMANAGER_ECC_INVALID_KEY, /**< 0x21DU - Error when both key Id and key address
						are invalid */
	XASUFW_AES_GET_CONTEXT_FAIL, /**< 0x21EU - Error when AES get context operation fails. */


	/* Additional status success codes */
	XASUFW_OCP_CERT_GENERATION_SUCCESS = 0x3F7U, /**< 0x3F7U - OCP certificate generation
							  successful */
	XASUFW_LMS_SIGNATURE_VERIFIED,	/**< 0x3F8U - LMS signature verification is successful */
	XASUFW_RSA_ECDH_SUCCESS, /**< 0x3F9U - RSA ECDH operation is successful */
	XASUFW_RSA_PSS_SIGNATURE_VERIFIED, /**< 0x3FAU - RSA PSS decode and sign verify
						operation is successful */
	XASUFW_RSA_DECRYPTION_SUCCESS, /**< 0x3FBU - Successfully decrypted RSA provided
					message */
	XASUFW_ECC_SIGNATURE_VERIFIED, /**< 0x3FCU - Successfully verified ECC signature */
	XASUFW_AES_TAG_MATCHED, /**< 0x3FDU - Successfully verified AES tag */
	XASUFW_AES_TAG_READ, /**< 0x3FEU - Successfully read AES tag */
	XASUFW_CMD_IN_PROGRESS = 0x3FF, /**< 0x3FFU - Command is in progress */
};

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_UpdateErrorStatus(s32 ErrorStatus, s32 Error);
s32 XAsufw_UpdateBufStatus(s32 ErrorStatus, s32 BufStatus);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_STATUS_H_ */
/** @} */
