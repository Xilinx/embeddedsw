/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_status.h
 * @addtogroup Overview
 * @{
 *
 * This file comprises error codes and function prototypes essential for updating error codes
 * in ASUFW.
 *
 * Error status of ASUFW :
 * The error status of ASUFW is represented by a 32-bit value, which can be broken down as
 * outlined below. Each error code is referenced through the XAsufw_Status enum or respective
 * BSP error codes.
 * Bit 31-30    : Buffer clear Status, 31st bit high represents failure and 30th bit high
 * 		  represents success.
 * Bit 29-20    : Final error Code
 * Bit 19-10    : Second error code which is responsible for failure
 * Bit 9-0      : First error code which is responsible for failure
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
 *
 * </pre>
 *
 *************************************************************************************************/

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
/* @name ASU error status values, maximum allowed value is 0x3FF */
enum XAsufw_Status {
	XASUFW_SUCCESS, /**< ASUFW success */
	XASUFW_FAILURE, /**< ASUFW failure */
	XASUFW_INVALID_PARAM, /**< Invalid parameters to ASUFW */
	XASUFW_RESOURCE_INVALID, /**< Invalid resource selection */
	XASUFW_RESOURCE_UNAVAILABLE, /**< Resource is busy */
	XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, /**< Releasing the resource is not allowed */
	XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, /**< DMA resource allocation failure */
	XASUFW_TASK_INVALID_HANDLER, /**< Received invalid task with NULL task handler */
	XASUFW_INVALID_USER_CONFIG_RECEIVED, /**< Invalid comm channel user config data received */
	XASUFW_VALIDATE_CMD_MODULE_NOT_REGISTERED, /**< Command received for unregistered module */
	XASUFW_VALIDATE_CMD_INVALID_COMMAND_RECEIVED, /**< Invalid command is received from client */
	XASUFW_MODULE_REGISTRATION_FAILED, /**< Module registration failed as to max supported modules
						are created */
	XASUFW_SSS_INVALID_INPUT_PARAMETERS, /**< Received invalid input parameters to SSS
						configuration APIs */
	XASUFW_IOMODULE_INIT_FAILED, /**< IOModule initialization failed */
	XASUFW_IOMODULE_SELF_TEST_FAILED, /**< IOModule self test failed */
	XASUFW_IOMODULE_START_FAILED, /**< IOModule start failed */
	XASUFW_IOMODULE_CONNECT_FAILED, /**< IOModule connect failed */
	XASUFW_IPI_LOOKUP_CONFIG_FAILED, /**< IPI lookup config failed */
	XASUFW_IPI_INVALID_INPUT_PARAMETERS, /**< Received invalid input arguments for IPI
						send/receive */
	XASUFW_IPI_POLL_FOR_ACK_FAILED, /**< IPI Poll for ack failed*/
	XASUFW_IPI_WRITE_MESSAGE_FAILED, /**< IPI write message failed */
	XASUFW_IPI_TRIGGER_FAILED, /**< IPI trigger failed */
	XASUFW_IPI_READ_MESSAGE_FAILED, /**< IPI read message failed */
	XASUFW_ERR_DMA_INSTANCE_NULL, /**< If DMA instance is NULL */
	XASUFW_ERR_DMA_LOOKUP, /**< DMA driver lookup config failed */
	XASUFW_ERR_DMA_CFG, /**< DMA driver config initialization failed */
	XASUFW_ERR_DMA_SELFTEST, /**< DMA self test failed */
	XASUFW_ERR_NON_BLOCK_DMA_WAIT, /**< Non blocking DMA transfer wait failed in given
						channel WaitForDone */
	XASUFW_INVALID_DMA_SSS_CONFIG, /**< SSS configuration assignment failed as the DMA address is
						invalid */
	/**< Termination code errors 0x21U to 0x2CU are reserved for errors returning from core */
	XASUFW_ECC_INVALID_PARAM = 0x2DU, /**< Invalid parameters to ECC APIs*/
	XASUFW_ECC_INIT_NOT_DONE, /**< ECC is not initialized */
	XASUFW_ECC_INIT_FAILED, /**< ECC initialization failed */
	XASUFW_ECC_GEN_PUB_KEY_OPERATION_FAIL, /**< When ECC Generate public key operation fails */
	XASUFW_ECC_VALIDATE_PUB_KEY_OPERATION_FAIL, /**< When ECC validate public key
							operation fails*/
	XASUFW_ECC_GEN_SIGN_OPERATION_FAIL, /**<  When ECC Generate signature operation fails */
	XASUFW_ECC_VERIFY_SIGN_OPERATION_FAIL, /**<  When ECC verify signature operation fails */
	XASUFW_ECC_WRITE_DATA_FAIL, /**< When writing data to memory fails */
	XASUFW_ECC_READ_DATA_FAIL, /**< When reading data from memory fails */
	XASUFW_ECC_EPHEMERAL_KEY_GEN_FAIL, /**< When generate ephemeral key fails */
	XASUFW_ECC_WAIT_FOR_DONE_TIMEOUT, /**< When wait for done timed out */
	XASUFW_ECC_MODULE_REGISTRATION_FAILED, /**< Module registration failed for ECC module */
	XASUFW_ECC_PUBKEY_COMPARISON_FAILED, /**< ECC public key comparison failed */
	XASUFW_ECC_SIGNATURE_COMPARISON_FAILED, /**< ECC signature comparison failed */
	XASUFW_ECC_KAT_FAILED, /**< ECC Kat failed */
	XASUFW_RSA_ECC_INVALID_PARAM, /**< Invalid parameters to RSA_ECC APIs*/
	XASUFW_RSA_ECC_GEN_PUB_KEY_OPERATION_FAIL, /**< When public key generation fails */
	XASUFW_RSA_ECC_WRITE_DATA_FAIL, /**< When writing data to memory fails */
	XASUFW_RSA_ECC_READ_DATA_FAIL, /**< When reading data from memory fails */
	XASUFW_RSA_ECC_PUBLIC_KEY_ZERO, /**< When public key is zero */
	XASUFW_RSA_ECC_PUBLIC_KEY_WRONG_ORDER, /**< Wrong order of Public key */
	XASUFW_RSA_ECC_PUBLIC_KEY_NOT_ON_CRV, /**< Key not found on curve */
	XASUFW_RSA_ECC_EPHEMERAL_KEY_GEN_FAIL, /**< When generate ephemeral key fails */
	XASUFW_RSA_ECC_GEN_SIGN_BAD_RAND_NUM, /**< Bad random number used for sign generation */
	XASUFW_RSA_ECC_GEN_SIGN_INCORRECT_HASH_LEN, /**< Incorrect hash length for sign
						generation */
	XASUFW_RSA_ECC_BAD_SIGN, /**< Signature provided for verification is bad */
	XASUFW_RSA_ECC_VER_SIGN_INCORRECT_HASH_LEN, /**< Incorrect hash length for sign
						verification */
	XASUFW_RSA_ECC_VER_SIGN_R_ZERO, /**< When provided R is zero */
	XASUFW_RSA_ECC_VER_SIGN_S_ZERO, /**< When provided S is zero */
	XASUFW_RSA_ECC_VER_SIGN_R_ORDER_ERROR, /**< R is not within ECC order */
	XASUFW_RSA_ECC_VER_SIGN_S_ORDER_ERROR, /**< S is not within ECC order */
	XASUFW_RSA_ECC_PUBKEY_COMPARISON_FAILED, /**< RSA ECC public key comparison failed */
	XASUFW_RSA_ECC_SIGNATURE_COMPARISON_FAILED, /**< RSA ECC signature comparison failed */
	XASUFW_RSA_ECC_KAT_FAILED, /**< RSA ECC Kat failed */
	XASUFW_SHA_INVALID_PARAM, /**< Invalid parameters to SHA APIs*/
	XASUFW_SHA_INIT_NOT_DONE, /**< SHA is not initialized */
	XASUFW_SHA_STATE_MISMATCH_ERROR, /**< SHA state mismatch error. Occurs when previous SHA
						state doesn't match before further update */
	XASUFW_SHA_MODE_GLITCH_DETECTED, /**< Configured SHA mode is not matching with the input */
	XASUFW_SHA_HASH_COMPARISON_FAILED, /**< SHA Hash comparison failed */
	XASUFW_SHA_KAT_FAILED, /**< SHA KAT failed */
	XASUFW_SHA2_MODULE_REGISTRATION_FAILED, /**< Module registration failed for SHA2 module */
	XASUFW_SHA2_INIT_FAILED, /**< SHA2 initialization failed */
	XASUFW_SHA2_START_FAILED, /**< SHA2 start failed */
	XASUFW_SHA2_UPDATE_FAILED, /**< SHA2 update failed */
	XASUFW_SHA2_FINISH_FAILED, /**< SHA2 finish failed */
	XASUFW_SHA2_HASH_COMPARISON_FAILED, /**< SHA2 hash comparison failed */
	XASUFW_SHA3_MODULE_REGISTRATION_FAILED, /**< Module registration failed for SHA3 module */
	XASUFW_SHA3_INIT_FAILED, /**< SHA3 initialization failed */
	XASUFW_SHA3_START_FAILED, /**< SHA3 start failed */
	XASUFW_SHA3_UPDATE_FAILED, /**< SHA3 update failed */
	XASUFW_SHA3_FINISH_FAILED, /**< SHA3 finish failed */
	XASUFW_SHA3_HASH_COMPARISON_FAILED, /**< SHA3 hash comparison failed */
	XASUFW_TRNG_MODULE_REGISTRATION_FAILED, /**< TRNG module registration failed */
	XASUFW_TRNG_INVALID_PARAM, /**< TRNG invalid input parameters received */
	XASUFW_TRNG_INVALID_SEED_VALUE, /**< TRNG invalid seed value received */
	XASUFW_TRNG_INVALID_STATE, /**< TRNG invalid state */
	XASUFW_TRNG_UNHEALTHY_STATE, /**< TRNG is in unhealthy state */
	XASUFW_TRNG_INVALID_MODE, /**< TRNG mode input received is invalid */
	XASUFW_TRNG_INVALID_DF_LENGTH, /**< TRNG invalid DF length received */
	XASUFW_TRNG_INVALID_SEED_LENGTH, /**< TRNG invalid seed length received */
	XASUFW_TRNG_INVALID_SEED_LIFE, /**< TRNG invalid seed life received */
	XASUFW_TRNG_INVALID_ADAPTPROPTEST_CUTOFF_VALUE, /**< TRNG invalid adaptproptestcutoff value */
	XASUFW_TRNG_INVALID_REPCOUNTTEST_CUTOFF_VALUE, /**< TRNG invalid precounttestcutoff value */
	XASUFW_TRNG_USER_CFG_COPY_ERROR, /**< TRNG user config structure copy to TRNG instance failed*/
	XASUFW_TRNG_INVALID_BUF_SIZE, /**< TRNG invalid buffer size */
	XASUFW_TRNG_RESEED_REQUIRED_ERROR, /**< TRNG reseed required error */
	XASUFW_TRNG_TIMEOUT_ERROR, /**< TRNG event timeout error */
	XASUFW_TRNG_CATASTROPHIC_DTF_ERROR, /**< TRNG catastrophic DTF error */
	XASUFW_TRNG_CATASTROPHIC_CTF_ERROR, /**< TRNG catastrophic CTF error */
	XASUFW_TRNG_KAT_FAILED_ERROR, /**< TRNG KAT operation failed */
	XASUFW_INVALID_BLOCKING_MODE, /**< TRNG invalid blocking mode input */
	XASUFW_INVALID_PREDRES_VALUE, /**< TRNG invalid prediction resistance value */
	XASUFW_TRNG_FIFO_IS_EMPTY, /**< TRNG random numbers fifo is empty */
	XASUFW_RANDOM_DATA_FAILED_TO_GENERATE, /**< TRNG random data generate failed */
	XASUFW_OSCILLATOR_ENABLE_FAILED, /**< TRNG enabling oscillator as seed source failed */
	XASUFW_OSCILLATOR_DISABLE_FAILED, /**< TRNG disabling oscillator source failed */
	XASUFW_ENABLE_PRNG_FOR_RESEED_FAILED, /**< TRNG enabling PRNG for reseed operation failed */
	XASUFW_START_RESEED_FAILED, /**< TRNG reseed failed */
	XASUFW_TRNG_INVALID_RANDOM_BYTES_SIZE, /**< TRNG invalid random bytes requested */
	XASUFW_TRNG_KAT_NOT_SUPPORTED_ON_QEMU, /**< TRNG DRBG KAT is not supported on QEMU */
	XASUFW_AES_GLITCH_ERROR, /**< AES glitch error */
	XASUFW_AES_INVALID_PARAM, /**< Invalid parameters to AES APIs*/
	XASUFW_AES_STATE_MISMATCH_ERROR, /**< AES state mismatch error. Occurs when previous AES
						state doesn't match before further update */
	XASUFW_AES_INVALID_KEY_OBJECT_ADDRESS, /**< AES invalid key object address */
	XASUFW_AES_INVALID_KEY_SRC, /**< AES invalid key source */
	XASUFW_AES_INVALID_KEY_SIZE, /**< AES invalid key size */
	XASUFW_AES_INVALID_IV, /**< AES invalid IV length/address for respective engine modes */
	XASUFW_AES_INVALID_ENGINE_MODE, /**< AES invalid engine mode */
	XASUFW_AES_ZEROED_KEY_NOT_ALLOWED,  /**< AES zeroed key not allowed */
	XASUFW_AES_INVALID_OPERATION_TYPE, /**< AES invalid encrypt/decrypt operation type */
	XASUFW_AES_INVALID_INPUT_DATA, /**< AES invalid input data */
	XASUFW_AES_INVALID_INPUT_DATA_LENGTH, /**< AES invalid input data length */
	XASUFW_AES_INVALID_ISLAST_CHUNK, /**< AES invalid is last chunk */
	XASUFW_AES_UNALIGNED_BLOCK_SIZE_INPUT_LENGTH, /**< AES ECB and CBC modes input data should be
								16Bytes aligned */
	XASUFW_AES_INVALID_TAG, /**< AES invalid tag length/address for respective engine modes */
	XASUFW_AES_TAG_GENERATE_FAILED, /**< AES tag generation failed */
	XASUFW_AES_TAG_COMPARE_FAILED, /**< AES tag comparison failed */
	XASUFW_AES_MODULE_REGISTRATION_FAILED, /**< AES module registration failed */
	XASUFW_AES_CONFIG_INIT_FAILED, /**< AES config initialization failed */
	XASUFW_AES_WRITE_KEY_FAILED, /**< AES write key failed */
	XASUFW_AES_INIT_FAILED, /**< AES initialization failed */
	XASUFW_AES_UPDATE_FAILED, /**< AES update failed */
	XASUFW_AES_FINAL_FAILED, /**< AES final failed */
	XASUFW_AES_KAT_FAILED, /**< AES KAT failed */
	XASUFW_AES_DPA_CM_KAT_CHECK1_FAILED, /**< AES DPA CM check1 failed */
	XASUFW_AES_DPA_CM_KAT_CHECK2_FAILED, /**< AES DPA CM check2 failed */
	XASUFW_AES_DPA_CM_KAT_CHECK3_FAILED, /**< AES DPA CM check3 failed */
	XASUFW_AES_DPA_CM_KAT_CHECK4_FAILED, /**< AES DPA CM check4 failed */
	XASUFW_AES_DPA_CM_KAT_CHECK5_FAILED, /**< AES DPA CM check5 failed */
	XASUFW_AES_DPA_CM_KAT_FAILED, /**< AES DPA CM KAT failed */
	XASUFW_RSA_INVALID_PARAM, /**< Invalid parameters to RSA APIs */
	XASUFW_RSA_RAND_GEN_ERROR, /**< Random number generation failed to RSA APIs */
	XASUFW_RSA_KEY_PAIR_COMP_ERROR, /**< Key pair comparison failure to RSA APIs */
	XASUFW_RSA_ERROR,		/**< Any other error to RSA APIs */
	XASUFW_RSA_CRT_OP_ERROR, /**< Error in CRT operation */
	XASUFW_RSA_PVT_OP_ERROR, /**< Error in Private exponentiation operation */
	XASUFW_RSA_PUB_OP_ERROR, /**< Error in Public exponentiation operation */
	XASUFW_RSA_MODULE_REGISTRATION_FAILED, /**< RSA module registration failed */
	XASUFW_RSA_ENCRYPT_DATA_COMPARISON_FAILED, /**< Error when RSA encrypt output comparision
							failed */
	XASUFW_RSA_KAT_FAILED, /**< Error when RSA KAT failed */
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
