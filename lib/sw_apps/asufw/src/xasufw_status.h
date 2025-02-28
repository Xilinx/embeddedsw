/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_server_error_codes Server Error Codes
* @{
*/
#ifndef XASUFW_STATUS_H
#define XASUFW_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_ECC_TERMINATION_CODE_MASK       (0x20U) /**< Mask for termination code indication */

/************************************** Type Definitions *****************************************/
/**
 * @brief ASU error status values, maximum allowed value is 0x3FF.
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
	XASUFW_INVALID_USER_CONFIG_RECEIVED, /**< 0x08U - Invalid comm channel user config data received */
	XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED, /**< 0x09U - Command received for unregistered module */
	XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED, /**< 0x0AU - Invalid command is received from client */
	XASUFW_MODULE_REGISTRATION_FAILED, /**< 0x0BU - Module registration failed as to max supported modules
						are created */
	XASUFW_SSS_INVALID_INPUT_PARAMETERS, /**< 0x0CU - Received invalid input parameters to SSS
						configuration APIs */
	XASUFW_IOMODULE_INIT_FAILED, /**< 0x0DU - IOModule initialization failed */
	XASUFW_IOMODULE_SELF_TEST_FAILED, /**< 0x0EU - IOModule self test failed */
	XASUFW_IOMODULE_START_FAILED, /**< 0x0FU - IOModule start failed */
	XASUFW_IOMODULE_CONNECT_FAILED, /**< 0x10U - IOModule connect failed */
	XASUFW_IPI_LOOKUP_CONFIG_FAILED, /**< 0x11U - IPI lookup config failed */
	XASUFW_IPI_INVALID_INPUT_PARAMETERS, /**< 0x12U - Received invalid input arguments for IPI
						send/receive */
	XASUFW_IPI_POLL_FOR_ACK_FAILED, /**< 0x13U - IPI Poll for ack failed*/
	XASUFW_IPI_WRITE_MESSAGE_FAILED, /**< 0x14U - IPI write message failed */
	XASUFW_IPI_TRIGGER_FAILED, /**< 0x15U - IPI trigger failed */
	XASUFW_IPI_READ_MESSAGE_FAILED, /**< 0x16U - IPI read message failed */
	XASUFW_ERR_DMA_INSTANCE_NULL, /**< 0x17U - If DMA instance is NULL */
	XASUFW_ERR_DMA_LOOKUP, /**< 0x18U - DMA driver lookup config failed */
	XASUFW_ERR_DMA_CFG, /**< 0x19U - DMA driver config initialization failed */
	XASUFW_ERR_DMA_SELFTEST, /**< 0x1AU - DMA self test failed */
	XASUFW_ERR_NON_BLOCK_DMA_WAIT, /**< 0x1BU - Non blocking DMA transfer wait failed in given
						channel WaitForDone */
	XASUFW_INVALID_DMA_SSS_CONFIG, /**< 0x1CU - SSS configuration assignment failed as the DMA address is
						invalid */
	XASUFW_ERR_EXCEPTION, /**< 0x1DU - Processor exception received */

	XASUFW_ERR_IPI_SEND_PLM_EFUSE_PRGM, /**< 0x1E - Failed while sending IPI for efuse write */
        XASUFW_ERR_IPI_RSP_PLM_EFUSE_PRGM, /**< 0x1F - IPI response failure to program efuse */
	/**< Termination code errors 0x21U to 0x2CU are reserved for errors returning from core */
	XASUFW_ECC_INVALID_PARAM = 0x2DU, /**< 0x2DU - Invalid parameters to ECC APIs*/
	XASUFW_ECC_INIT_NOT_DONE, /**< 0x2EU - ECC is not initialized */
	XASUFW_ECC_INIT_FAILED, /**< 0x2FU - ECC initialization failed */
	XASUFW_ECC_GEN_PUB_KEY_OPERATION_FAIL, /**< 0x30U - When ECC Generate public key operation fails */
	XASUFW_ECC_VALIDATE_PUB_KEY_OPERATION_FAIL, /**< 0x31U - When ECC validate public key
							operation fails */
	XASUFW_ECC_GEN_SIGN_OPERATION_FAIL, /**<  0x32U - When ECC Generate signature operation fails */
	XASUFW_ECC_VERIFY_SIGN_OPERATION_FAIL, /**<  0x33U - When ECC verify signature operation fails */
	XASUFW_ECC_WRITE_DATA_FAIL, /**< 0x34U - When writing data to memory fails */
	XASUFW_ECC_READ_DATA_FAIL, /**< 0x35U - When reading data from memory fails */
	XASUFW_ECC_EPHEMERAL_KEY_GEN_FAIL, /**< 0x36U - When generate ephemeral key fails */
	XASUFW_ECC_WAIT_FOR_DONE_TIMEOUT, /**< 0x37U - When wait for done timed out */
	XASUFW_ECC_MODULE_REGISTRATION_FAILED, /**< 0x38U - Module registration failed for ECC module */
	XASUFW_ECC_PUBKEY_COMPARISON_FAILED, /**< 0x39U - ECC public key comparison failed */
	XASUFW_ECC_SIGNATURE_COMPARISON_FAILED, /**< 0x3AU - ECC signature comparison failed */
	XASUFW_ECC_KAT_FAILED, /**< 0x3BU - ECC Kat failed */
	XASUFW_RSA_ECC_INVALID_PARAM, /**< 0x3CU - Invalid parameters to RSA_ECC APIs*/
	XASUFW_RSA_ECC_GEN_PUB_KEY_OPERATION_FAIL, /**< 0x3DU - When public key generation fails */
	XASUFW_RSA_ECC_WRITE_DATA_FAIL, /**< 0x3EU - When writing data to memory fails */
	XASUFW_RSA_ECC_READ_DATA_FAIL, /**< 0x3FU - When reading data from memory fails */
	XASUFW_RSA_ECC_PUBLIC_KEY_ZERO, /**< 0x40U - When public key is zero */
	XASUFW_RSA_ECC_PUBLIC_KEY_WRONG_ORDER, /**< 0x41U - Wrong order of Public key */
	XASUFW_RSA_ECC_PUBLIC_KEY_NOT_ON_CRV, /**< 0x42U - Key not found on curve */
	XASUFW_RSA_ECC_EPHEMERAL_KEY_GEN_FAIL, /**< 0x43U - When generate ephemeral key fails */
	XASUFW_RSA_ECC_GEN_SIGN_BAD_RAND_NUM, /**< 0x44U - Bad random number used for sign generation */
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
	XASUFW_SHA_STATE_MISMATCH_ERROR, /**< 0x50U - SHA state mismatch error. Occurs when previous SHA
						state doesn't match before further update */
	XASUFW_SHA_INVALID_INPUT_DATA_ADDRESS, /**< 0x51U - SHA invalid input data address */
	XASUFW_SHA_INVALID_INPUT_DATA_SIZE, /**< 0x52U - SHA invalid input data size */
	XASUFW_SHA_INVALID_END_LAST, /**< 0x53U - SHA invalid end last */
	XASUFW_SHA_INVALID_HASH_ADDRESS, /**< 0x54U - SHA invalid hash address */
	XASUFW_SHA_INVALID_HASH_SIZE, /**< 0x55U - SHA invalid hash size */
	XASUFW_SHA_NEXT_XOF_INVALID_MASK, /**< 0x56U - SHA invalid next xof mask */
	XASUFW_SHA_INVALID_SHA_TYPE, /**< 0x57U - SHA invalid type */
	XASUFW_SHA_INVALID_SHA_MODE, /**< 0x58U - SHA invalid mode */
	XASUFW_SHA_MODE_GLITCH_DETECTED, /**< 0x59U - Configured SHA mode is not matching with the input */
	XASUFW_SHA_HASH_COMPARISON_FAILED, /**< 0x5AU - SHA Hash comparison failed */
	XASUFW_SHA_KAT_FAILED, /**< 0x5BU - SHA KAT failed */
	XASUFW_SHA2_MODULE_REGISTRATION_FAILED, /**< 0x5CU - Module registration failed for SHA2 module */
	XASUFW_SHA2_INIT_FAILED, /**< 0x5DU - SHA2 initialization failed */
	XASUFW_SHA2_START_FAILED, /**< 0x5EU - SHA2 start failed */
	XASUFW_SHA2_UPDATE_FAILED, /**< 0x5FU - SHA2 update failed */
	XASUFW_SHA2_FINISH_FAILED, /**< 0x60U - SHA2 finish failed */
	XASUFW_SHA2_HASH_COMPARISON_FAILED, /**< 0x61U - SHA2 hash comparison failed */
	XASUFW_SHA3_MODULE_REGISTRATION_FAILED, /**< 0x62U - Module registration failed for SHA3 module */
	XASUFW_SHA3_INIT_FAILED, /**< 0x63U - SHA3 initialization failed */
	XASUFW_SHA3_START_FAILED, /**< 0x64U - SHA3 start failed */
	XASUFW_SHA3_UPDATE_FAILED, /**< 0x65U - SHA3 update failed */
	XASUFW_SHA3_FINISH_FAILED, /**< 0x66U - SHA3 finish failed */
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
	XASUFW_INVALID_BLOCKING_MODE, /**< 0x7AU - TRNG invalid blocking mode input */
	XASUFW_INVALID_PREDRES_VALUE, /**< 0x7BU - TRNG invalid prediction resistance value */
	XASUFW_TRNG_FIFO_IS_EMPTY, /**< 0x7CU - TRNG random numbers fifo is empty */
	XASUFW_RANDOM_DATA_FAILED_TO_GENERATE, /**< 0x7DU - TRNG random data generate failed */
	XASUFW_OSCILLATOR_ENABLE_FAILED, /**< 0x7EU - TRNG enabling oscillator as seed source failed */
	XASUFW_OSCILLATOR_DISABLE_FAILED, /**< 0x7FU - TRNG disabling oscillator source failed */
	XASUFW_ENABLE_PRNG_FOR_RESEED_FAILED, /**< 0x80U - TRNG enabling PRNG for reseed operation failed */
	XASUFW_START_RESEED_FAILED, /**< 0x81U - TRNG reseed failed */
	XASUFW_TRNG_INVALID_RANDOM_BYTES_SIZE, /**< 0x82U - TRNG invalid random bytes requested */
	XASUFW_TRNG_KAT_NOT_SUPPORTED_ON_QEMU, /**< 0x83U - TRNG DRBG KAT is not supported on QEMU */
	XASUFW_TRNG_DFLEN_CONFIG_ERROR, /**< 0x84U - DF length configuration failure */
	XASUFW_TRNG_ADAPTCUTOFF_CONFIG_ERROR, /**< 0x85U - Adaptive test cutoff configuration failure */
	XASUFW_TRNG_REPCUTOFF_CONFIG_ERROR, /**< 0x86U - Repetitive test cutoff configuration failure */
	XASUFW_TRNG_DIT_CONFIG_ERROR, /**< 0x87U - DIT value configuration failure */
	XASUFW_AES_GLITCH_ERROR, /**< 0x88U - AES glitch error */
	XASUFW_AES_INVALID_PARAM, /**< 0x89U - Invalid parameters to AES APIs */
	XASUFW_AES_STATE_MISMATCH_ERROR, /**< 0x8AU - AES state mismatch error. Occurs when previous AES
						state doesn't match before further update */
	XASUFW_AES_KEY_CLEAR_ERROR, /**< 0x8BU - AES key clear error */
	XASUFW_AES_INVALID_KEY_OBJECT_ADDRESS, /**< 0x8CU - AES invalid key object address */
	XASUFW_AES_INVALID_KEY_ADDRESS, /**< 0x8DU - AES invalid key address */
	XASUFW_AES_INVALID_KEY_SRC, /**< 0x8EU - AES invalid key source */
	XASUFW_AES_INVALID_KEY_SIZE, /**< 0x8FU - AES invalid key size */
	XASUFW_AES_INVALID_IV, /**< 0x90U - AES invalid IV length/address for respective engine modes */
	XASUFW_AES_INVALID_ENGINE_MODE, /**< 0x91U - AES invalid engine mode */
	XASUFW_AES_KEY_ZEROED,  /**< 0x92U - AES zeroed key not allowed */
	XASUFW_AES_INVALID_OPERATION_TYPE, /**< 0x93U - AES invalid encrypt/decrypt operation type */
	XASUFW_AES_INVALID_INPUT_DATA, /**< 0x94U - AES invalid input data */
	XASUFW_AES_INVALID_INPUT_DATA_LENGTH, /**< 0x95U - AES invalid input data length */
	XASUFW_AES_INVALID_ISLAST_CHUNK, /**< 0x96U - AES invalid is last chunk */
	XASUFW_AES_UNALIGNED_BLOCK_SIZE_INPUT_LENGTH, /**< 0x97U - AES ECB and CBC modes input data should be
								16Bytes aligned */
	XASUFW_AES_INVALID_TAG, /**< 0x98U - AES invalid tag length/address for respective engine modes */
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
	XASUFW_AES_KAT_FAILED, /**< 0xA3U - AES KAT failed */
	XASUFW_AES_DPA_CM_KAT_CHECK1_FAILED, /**< 0xA4U - AES DPA CM check1 failed */
	XASUFW_AES_DPA_CM_KAT_CHECK2_FAILED, /**< 0xA5U - AES DPA CM check2 failed */
	XASUFW_AES_DPA_CM_KAT_CHECK3_FAILED, /**< 0xA6U - AES DPA CM check3 failed */
	XASUFW_AES_DPA_CM_KAT_CHECK4_FAILED, /**< 0xA7U - AES DPA CM check4 failed */
	XASUFW_AES_DPA_CM_KAT_CHECK5_FAILED, /**< 0xA8U - AES DPA CM check5 failed */
	XASUFW_AES_DPA_CM_KAT_FAILED, /**< 0xA9U - AES DPA CM KAT failed */
	XASUFW_RSA_INVALID_PARAM, /**< 0xAAU - Invalid parameters to RSA APIs */
	XASUFW_RSA_PUB_EXP_INVALID_VALUE, /**< 0xABU - Error in Public exponent value */
	XASUFW_RSA_MOD_DATA_IS_ZERO, /**< 0xACU - Modulus data is zero */
	XASUFW_RSA_MOD_DATA_INVALID, /**< 0xADU - Modulus data is less than input data */
	XASUFW_RSA_MOD_DATA_INPUT_DATA_EQUAL, /**< 0xAEU - Modulus data is equal to input data */
	XASUFW_RSA_RAND_GEN_ERROR, /**< 0xAFU - Random number generation failed to RSA APIs */
	XASUFW_RSA_KEY_PAIR_COMP_ERROR, /**< 0xB0U - Key pair comparison failure to RSA APIs */
	XASUFW_RSA_ERROR, /**< 0xB1U - Any other error to RSA APIs */
	XASUFW_RSA_CRT_OP_ERROR, /**< 0xB2U - Error in CRT operation */
	XASUFW_RSA_PVT_OP_ERROR, /**< 0xB3U - Error in Private exponentiation operation */
	XASUFW_RSA_PUB_OP_ERROR, /**< 0xB4U - Error in Public exponentiation operation */
	XASUFW_RSA_MASK_GEN_DATA_BLOCK_ERROR, /**< 0xB5U - Error when MGF returns error for data block */
	XASUFW_RSA_MASK_GEN_SEED_BUFFER_ERROR, /**< 0xB6U - Error when MGF returns error seed buffer*/
	XASUFW_RSA_ZEROIZE_MEMSET_FAIL, /**< 0xB7U - Error when memory zeroization fails */
	XASUFW_RSA_OAEP_ENCRYPT_ERROR, /**< 0xB8U - Error when OAEP encryption operation fails */
	XASUFW_RSA_OAEP_ENCODE_ERROR, /**< 0xB9U - Error when OAEP encode operation fails */
	XASUFW_RSA_OAEP_INVALID_LEN, /**< 0xBAU - Error when OAEP input len is invalid */
	XASUFW_RSA_OAEP_DECRYPT_ERROR, /**< 0xBBU - Error when OAEP decryption operation fails */
	XASUFW_RSA_OAEP_DECODE_ERROR, /**< 0xBCU - Error when OAEP decode operation fails */
	XASUFW_RSA_OAEP_HASH_CMP_FAIL,	/**< 0xBDU - Error when OAEP decode operation fails for hash
							comparison failure */
	XASUFW_RSA_OAEP_ONE_SEP_CMP_FAIL, /**< 0xBEU - Error when OAEP decode operation fails for message
							separation comparison failure */
	XASUFW_RSA_PSS_INVALID_LEN,	/**< 0xBFU - Error when PSS signature len is invalid */
	XASUFW_RSA_PSS_INVALID_SALT_LEN, /**< 0xC0U - Error when PSS salt len is invalid */
	XASUFW_RSA_PSS_SIGN_GEN_ERROR, /**< 0xC1U - Error when PSS sign generation operation fails */
	XASUFW_RSA_PSS_ENCRYPT_ERROR, /**< 0xC2U - Error when PSS encryption operation fails */
	XASUFW_RSA_PSS_NO_SALT_NO_RANDOM_STRING, /**< 0xC3U - Error when PSS encode operation fails for
							invalid salt input */
	XASUFW_RSA_PSS_RIGHT_MOST_CMP_FAIL, /**< 0xC4U - Error when PSS decode operation fails for last
							octet comparison failure */
	XASUFW_RSA_PSS_LEFT_MOST_BIT_CMP_FAIL, /**< 0xC5U - Error when PSS decode operation fails for first
							bit in first octet comparison failure */
	XASUFW_RSA_PSS_DB_LEFT_MOST_BYTE_CMP_FAIL, /**< 0xC6U - Error when PSS decode operation fails for first
							octet comparison failure */
	XASUFW_RSA_PSS_DB_BYTE_ONE_CMP_FAIL, /**< 0xC7U - Error when PSS decode operation fails for first
							second octet comparison failure */
	XASUFW_RSA_PSS_HASH_CMP_FAIL, /**< 0xC8U - Error when PSS decode operation fails for hash
							comparison failure */
	XASUFW_RSA_PSS_SIGN_VER_ERROR, /**< 0xC9U - Error when PSS sign verification operation fails */
	XASUFW_RSA_PSS_DECRYPT_ERROR, /**< 0xCAU - Error when PSS decryption operation fails */
	XASUFW_RSA_MODULE_REGISTRATION_FAILED, /**< 0xCBU - RSA module registration failed */
	XASUFW_RSA_ENCRYPT_DATA_COMPARISON_FAILED, /**< 0xCCU - Error when RSA encrypt output comparison
							failed */
	XASUFW_RSA_DECRYPT_DATA_COMPARISON_FAILED, /**< 0xCDU - Error when RSA decrypt output comparison
							failed */
	XASUFW_RSA_KAT_FAILED, /**< 0xCEU - Error when RSA KAT failed */
	XASUFW_RSA_DMA_COPY_FAIL, /**< 0xCFU - When data transfer to/from memory using DMA fails in RSA */
	XASUFW_RSA_MEM_COPY_FAIL,  /**< 0xD0U - When copy data to memory fails in RSA */
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
	XASUFW_HMAC_STATE_MISMATCH_ERROR, /**< 0xDBU - HMAC state mismatch error. Occurs when previous HMAC
						state doesn't match before further update */
	XASUFW_HMAC_INITIALIZATION_FAILED, /**< 0xDCU - HMAC initialization failed */
	XASUFW_HMAC_UPDATE_FAILED, /**< 0xDDU - HMAC update failed */
	XASUFW_HMAC_FINAL_FAILED, /**< 0xDEU - HMAC final failed */
	XASUFW_HMAC_KAT_COMPARISON_FAILED, /**< 0xDFU - HMAC comparison failed in KAT */
	XASUFW_HMAC_KAT_FAILED, /**< 0xE0U - HMAC KAT failed */
	XASUFW_HMAC_MODULE_REGISTRATION_FAILED, /**< 0xE1U - HMAC module registration failed */
	XASUFW_KDF_INVALID_PARAM, /**< 0xE2U - Invalid parameters to the KDF APIs */
	XASUFW_KDF_ITERATION_COUNT_MISMATCH, /** 0xE3U - Failure in running desired number of iterations for
						KDF output */
	XASUFW_KDF_MODULE_REGISTRATION_FAILED, /**< 0xE4U - KDF module registration failed */
	XASUFW_KDF_COMPUTE_FAILED, /**< 0xE5U - KDF compute failed */
	XASUFW_KDF_KAT_COMPARISON_FAILED, /**< 0xE6U - KDF KAT comparison failed */
	XASUFW_KDF_KAT_FAILED, /**< 0xE7U - KDF KAT failed */
	XASUFW_TRNG_GET_RANDOM_NUMBERS_TIMEDOUT, /**< 0xE8U - TRNG Get Random numbers timed out */
	XASUFW_TRNG_INVALID_RANDOM_NUMBER, /**< 0xE9U - Invalid random number generated */
	XASUFW_RSA_CHANGE_ENDIANNESS_ERROR, /**< 0xEAU - Error when change endianness in RSA fails */
	XASUFW_RSA_INVALID_PRIME_TOT_FLAG, /**< 0xEBU - Error when invalid flag for prime number or
						totient is given */
	XASUFW_ECIES_INVALID_PARAM, /**< 0xECU - Invalid parameters to ECIES APIs */
	XASUFW_ECIES_TRNG_FAILED, /**< 0xEDU - Error when random number generation failed */
	XASUFW_ECIES_KEY_GEN_FAILURE, /**< 0xEEU - Error when public key generation failed */
	XASUFW_ECIES_ECDH_FAILURE, /**< 0xEFU - Error when ECDH operation failed */
	XASUFW_ECIES_KDF_FAILURE, /**< 0xF0U - Error when KDF operation failed */
	XASUFW_ECIES_AES_WRITE_KEY_FAILURE, /**< 0xF1U - Error when AES write key operation failed */
	XASUFW_ECIES_AES_FAILURE, /**< 0xF2U - Error when AES operation failed */
	XASUFW_ECIES_MODULE_REGISTRATION_FAILED, /**< 0xF3U - ECIES module registration failed */
	XASUFW_ECIES_ENCRYPT_FAILED, /**< 0xF4U - ECIES encryption failed */
	XASUFW_ECIES_DECRYPT_FAILED, /**< 0xF5U - ECIES decryption failed */
	XASUFW_ECIES_KAT_FAILED, /**< 0xF6U - ECIES KAT failed */
	XASUFW_ECIES_KAT_COMPARISON_FAILED, /**< 0xF7U - ECIES KAT comparision failed */
	XASUFW_ERR_KV_INTERRUPT_DONE_TIMEOUT, /**< 0xF8U - KV interrupt done timeout error */

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

#endif  /* XASUFW_STATUS_H */
/** @} */
