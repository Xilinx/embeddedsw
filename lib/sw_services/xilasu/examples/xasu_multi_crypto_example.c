/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_multi_crypto_example.c
 * @addtogroup Overview
 * @{
 *
 * This example demonstrates how to use the ASU's queueing mechanism to send multiple
 * cryptographic commands with different priorities and different crypto functions.
 * It illustrates:
 *   1. Sending multiple concurrent commands to the ASU using different priorities
 *   2. Using different crypto functions (AES, SHA2, RSA, and ECC/ECDSA)
 *   3. Tracking which response corresponds to which request using unique callback contexts
 *   4. Proper handling of the ASU queue scheduler with mixed priority operations
 *
 * The example sends multiple crypto operations simultaneously:
 *   - SHA2 hash operation (LOW priority)
 *   - AES encryption operation (HIGH priority)
 *   - ECC sign generation operation (HIGH priority)
 *   - RSA PSS sign generation operation (LOW priority)
 *   - AES decryption operation (HIGH priority) - depends on encryption result
 *   - RSA PSS sign verification operation (HIGH priority) - depends on sign generation result
 *
 * Each operation uses a unique callback context structure to identify which operation
 * completed when the callback is triggered. This demonstrates the proper way to handle
 * multiple concurrent requests in the ASU architecture.
 *
 * Procedure to link and compile the example for the default ddr less designs
 * ------------------------------------------------------------------------------------------------
 * The default linker settings places a software stack, heap and data in DDR memory.
 * For this example to work, any data shared between client running on A78/R52/PL and server
 * running on ASU, should be placed in area which is accessible to both client and server.
 *
 * Following is the procedure to compile the example on any memory region which can be accessed
 * by the server.
 *
 *      1. Open ASU application linker script(lscript.ld) and there will be a memory
 *         mapping section which should be updated to point all the required sections
 *         to shared memory using a memory region selection
 *
 *                                      OR
 *
 *      1. In linker script(lscript.ld) user can add new memory section in source tab as shown below
 *              .sharedmemory : {
 *              . = ALIGN(4);
 *              __sharedmemory_start = .;
 *              *(.sharedmemory)
 *              *(.sharedmemory.*)
 *              *(.gnu.linkonce.d.*)
 *              __sharedmemory_end = .;
 *              } > Shared_memory_area
 *
 *      2. In this example ".data" section elements that are passed by reference to the server-side
 *         should be stored in the above shared memory section.
 *         Replace ".data" in attribute section with ".sharedmemory", as shown below-
 *      static u8 Data __attribute__ ((aligned (64U)) __attribute__ ((section (".data.Data")));
 *                              should be changed to
 *      static u8 Data __attribute__ ((aligned (64U)) __attribute__ ((section (".sharedmemory.Data")));
 *
 * To keep things simple, by default the cache is disabled in this example using
 * XASU_DISABLE_CACHE macro.
 *
 * NOTE: This example is for demonstration purposes only. The test data and keys used
 * here are not intended for production use.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   kp   01/28/26 Initial release - Multi-crypto queue example
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xasu_client.h"
#include "xasu_aes.h"
#include "xasu_sha2.h"
#include "xasu_ecc.h"
#include "xasu_rsa.h"

/************************************ Constant Definitions ***************************************/

/************************************ Type Definitions *******************************************/

/*
 * NOTE: The XAsu_OperationType enum and XAsu_CallbackContext structure below are provided
 * for ease of use in this example. Users are free to develop their own callback mapping
 * mechanism based on application requirements.
 */

/**
 * @brief Operation identifiers for tracking different crypto operations
 */
typedef enum {
	XASU_OP_SHA2_HASH = 0U,         /**< SHA2 hash operation */
	XASU_OP_AES_ENCRYPT,            /**< AES encryption operation */
	XASU_OP_AES_DECRYPT,            /**< AES decryption operation */
	XASU_OP_ECC_SIGN_GEN,           /**< ECC signature generation operation */
	XASU_OP_RSA_PSS_SIGN_GEN,       /**< RSA PSS signature generation operation */
	XASU_OP_RSA_PSS_SIGN_VER,       /**< RSA PSS signature verification operation */
	XASU_OP_MAX                     /**< Maximum number of operations */
} XAsu_OperationType;

/**
 * @brief Callback context structure to track individual crypto operations
 *
 * This structure is used to identify which operation completed when the
 * callback is triggered. Each operation has its own context instance.
 */
typedef struct {
	XAsu_OperationType OpType;      /**< Type of crypto operation */
	u32 Status;                     /**< Operation completion status */
	u8 IsCompleted;                 /**< Flag indicating operation completion */
	const char *OpName;             /**< Human-readable operation name */
} XAsu_CallbackContext;

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_DISABLE_CACHE /**< Disable data cache for shared memory access */

/**
 * @brief Wait for Event - efficient low-power wait for interrupt/event
 *
 * Uses architecture-specific wait instruction to put CPU in low-power state until an event occurs.
 */
#if defined(__aarch64__) || defined(__ARM_ARCH)
/* ARM AArch64/AArch32: Use WFE (Wait For Event) instruction */
#define XAsu_WaitForEvent()  __asm__ volatile("wfe" ::: "memory")
#elif defined(__riscv)
/* RISC-V: Use WFI (Wait For Interrupt) instruction */
#define XAsu_WaitForEvent()  __asm__ volatile("wfi" ::: "memory")
#else
/*
 * Fallback for unsupported architectures: No low-power wait available.
 * This results in busy-polling which consumes CPU cycles but ensures
 * correct functionality. The volatile counter ensures the loop condition
 * is re-evaluated each iteration.
 */
#define XAsu_WaitForEvent()  do { } while (0)
#endif

/* AES parameters */
#define XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES	(32U)  /**< AES payload data length in bytes */
#define XASU_AES_KEY_LEN_IN_BYTES		(32U)  /**< AES Key length in bytes */
#define XASU_AES_IV_LEN_IN_BYTES		(12U)  /**< AES IV length in bytes */
#define XASU_AES_TAG_LEN_IN_BYTES		(16U)  /**< AES Tag length in bytes */
#define XASU_AES_AAD_LEN_IN_BYTES		(16U)  /**< AES AAD length in bytes */

/* SHA2 parameters */
#define XASU_SHA2_HASH_LEN_IN_BYTES		(32U)  /**< SHA2 hash length in bytes */
#define XASU_SHA2_INPUT_DATA_LEN		(5U)   /**< SHA2 input data length */

/* ECC parameters */
#define XASU_ECC_P256_KEY_SIZE_IN_BYTES		(32U)  /**< ECC P256 key size in bytes */
#define XASU_ECC_DOUBLE_P256_SIZE_IN_BYTES	(64U)  /**< Double ECC P256 size in bytes */

/* RSA parameters */
#define XASU_RSA_2048_KEY_SIZE_IN_BYTES		(256U) /**< RSA 2048 key size in bytes */
#define XASU_RSA_PSS_SALT_LEN_IN_BYTES		(20U)  /**< RSA PSS salt length in bytes */
#define XASU_RSA_INPUT_DATA_LEN_IN_BYTES	(80U)  /**< RSA input data length in bytes */

/* Operation count parameters */
#define XASU_SET1_OPS_COUNT			(4U)   /**< Set 1: SHA2, AES Encrypt, ECC Sign, RSA Sign */

/************************************ Function Prototypes ****************************************/
static s32 XAsu_MultiCryptoExample(void);
static s32 XAsu_SendCryptoRequestsSet1(void);
static s32 XAsu_SendCryptoRequestsSet2(void);
static void XAsu_MultiCryptoCallBack(void *CallBackRef, u32 Status);
static void XAsu_PrintResults(void);
static void XAsu_PrintData(const u8 *Data, u32 DataLen, const char *Label);

/************************************ Variable Definitions ***************************************/

/** SHA2 input data for hashing */
static const char XAsu_Sha2Data[XASU_SHA2_INPUT_DATA_LEN + 1U]
	__attribute__ ((section (".data.XAsu_Sha2Data"))) = "HELLO";

/** SHA2 hash output buffer */
static u8 XAsu_Sha2Hash[XASU_SHA2_HASH_LEN_IN_BYTES]
	__attribute__ ((section (".data.XAsu_Sha2Hash")));

static const u8 XAsu_Sha2ExpHash[XASU_SHA2_HASH_LEN_IN_BYTES] = {
	0x37, 0x33, 0xCD, 0x97, 0x7F, 0xF8, 0xEB, 0x18,
	0xB9, 0x87, 0x35, 0x7E, 0x22, 0xCE, 0xD9, 0x9F,
	0x46, 0x09, 0x7F, 0x31, 0xEC, 0xB2, 0x39, 0xE8,
	0x78, 0xAE, 0x63, 0x76, 0x0E, 0x83, 0xE4, 0xD5
};

/* AES Test Data */
/** AES plaintext data */
static u8 XAsu_AesInputData[XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES]
	__attribute__ ((section (".data.XAsu_AesInputData"))) = {
	0x2FU, 0xBFU, 0x02U, 0x9EU, 0xE9U, 0xFBU, 0xD6U, 0x11U,
	0xC2U, 0x4DU, 0x81U, 0x4EU, 0x6AU, 0xFFU, 0x26U, 0x77U,
	0xC3U, 0x5AU, 0x83U, 0xBCU, 0xE5U, 0x63U, 0x2CU, 0xE7U,
	0x89U, 0x43U, 0x6CU, 0x68U, 0x82U, 0xCAU, 0x1CU, 0x71U
};

/** AES AAD data */
static u8 XAsu_AesAad[XASU_AES_AAD_LEN_IN_BYTES]
	__attribute__ ((section (".data.XAsu_AesAad"))) = {
	0x9AU, 0x7BU, 0x86U, 0xE7U, 0x82U, 0xCCU, 0xAAU, 0x6AU,
	0xB2U, 0x21U, 0xBDU, 0x03U, 0x47U, 0x0BU, 0xDCU, 0x2EU
};

/** AES key */
static u8 XAsu_AesKey[XASU_AES_KEY_LEN_IN_BYTES]
	__attribute__ ((section (".data.XAsu_AesKey"))) = {
	0xD4U, 0x16U, 0xA6U, 0x93U, 0x1DU, 0x52U, 0xE0U, 0xF5U,
	0x0AU, 0xA0U, 0x89U, 0xA7U, 0x57U, 0xB1U, 0x1AU, 0x89U,
	0x1CU, 0xBDU, 0x1BU, 0x83U, 0x84U, 0x7DU, 0x4BU, 0xEDU,
	0x9EU, 0x29U, 0x38U, 0xCDU, 0x4CU, 0x54U, 0xA8U, 0xBAU
};

/** AES IV */
static u8 XAsu_AesIv[XASU_AES_IV_LEN_IN_BYTES]
	__attribute__ ((section (".data.XAsu_AesIv"))) = {
	0x85U, 0x36U, 0x5FU, 0x88U, 0xB0U, 0xB5U,
	0x62U, 0x98U, 0xDFU, 0xEAU, 0x5AU, 0xB2U
};

/** AES-GCM expected CipherText */
static u8 XAsu_AesGcmExpCt[XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES]
	__attribute__ ((section (".data.XAsu_AesGcmExpCt"))) = {
	0x59U, 0x8CU, 0xD1U, 0x9FU, 0x16U, 0x83U, 0xB4U, 0x1BU,
	0x4CU, 0x59U, 0xE1U, 0xC1U, 0x57U, 0xD4U, 0x15U, 0x01U,
	0xA3U, 0xC0U, 0x89U, 0x02U, 0xF0U, 0xEAU, 0x3AU, 0x37U,
	0x6AU, 0x8BU, 0x0DU, 0x99U, 0x88U, 0xCFU, 0xF8U, 0xC1U
};

/** AES-GCM expected Tag */
static u8 XAsu_AesGcmExpTag[XASU_AES_TAG_LEN_IN_BYTES]
	__attribute__ ((section (".data.XAsu_AesGcmExpTag"))) = {
	0xADU, 0xCEU, 0xFEU, 0x2FU, 0x6EU, 0xE4U, 0xC7U, 0x06U,
	0x0EU, 0x44U, 0xAAU, 0x5EU, 0xDFU, 0x0DU, 0xBEU, 0xBCU
};

/** AES Tag output buffer */
static u8 XAsu_AesTag[XASU_AES_TAG_LEN_IN_BYTES]
	__attribute__ ((section (".data.XAsu_AesTag")));

/** AES Encrypted data output buffer */
static u8 XAsu_AesEncData[XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES]
	__attribute__ ((section (".data.XAsu_AesEncData")));

/** AES Decrypted data output buffer */
static u8 XAsu_AesDecData[XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES]
	__attribute__ ((section (".data.XAsu_AesDecData")));

/* ECC Test Data */
/** ECC P256 private key for sign generation */
static const u8 XAsu_EccPrivKey[XASU_ECC_P256_KEY_SIZE_IN_BYTES]
	__attribute__ ((section (".data.XAsu_EccPrivKey"))) = {
	0xC4U, 0x77U, 0xF9U, 0xF6U, 0x5CU, 0x22U, 0xCCU, 0xE2U,
	0x06U, 0x57U, 0xFAU, 0xA5U, 0xB2U, 0xD1U, 0xD8U, 0x12U,
	0x23U, 0x36U, 0xF8U, 0x51U, 0xA5U, 0x08U, 0xA1U, 0xEDU,
	0x04U, 0xE4U, 0x79U, 0xC3U, 0x49U, 0x85U, 0xBFU, 0x96U
};

/** ECC hash/digest input for sign generation */
static const u8 XAsu_EccHash[XASU_ECC_P256_KEY_SIZE_IN_BYTES]
	__attribute__ ((section (".data.XAsu_EccHash"))) = {
	0xA4U, 0x1AU, 0x41U, 0xA1U, 0x2AU, 0x79U, 0x95U, 0x48U,
	0x21U, 0x1CU, 0x41U, 0x0CU, 0x65U, 0xD8U, 0x13U, 0x3AU,
	0xFDU, 0xE3U, 0x4DU, 0x28U, 0xBDU, 0xD5U, 0x42U, 0xE4U,
	0xB6U, 0x80U, 0xCFU, 0x28U, 0x99U, 0xC8U, 0xA8U, 0xC4U
};

/** ECC signature output buffer */
static u8 XAsu_EccSign[XASU_ECC_DOUBLE_P256_SIZE_IN_BYTES]
	__attribute__ ((section (".data.XAsu_EccSign")));

/* RSA Test Data */
/** RSA modulus (2048 bits) */
static const u8 XAsu_RsaModulus[XASU_RSA_2048_KEY_SIZE_IN_BYTES]
	__attribute__ ((section (".data.XAsu_RsaModulus"))) = {
	0xAEU, 0xFEU, 0xC6U, 0x93U, 0xF1U, 0x06U, 0x01U, 0x47U,
	0x17U, 0x28U, 0xA2U, 0x49U, 0x6FU, 0xA3U, 0x1DU, 0x8CU,
	0xDBU, 0xF9U, 0xB3U, 0x57U, 0x67U, 0xEFU, 0x31U, 0xCDU,
	0xACU, 0x13U, 0x1CU, 0x20U, 0xE0U, 0x9DU, 0x55U, 0x55U,
	0xFDU, 0x0CU, 0x30U, 0xB7U, 0x83U, 0x28U, 0xA5U, 0x4CU,
	0x77U, 0xC0U, 0x85U, 0x11U, 0x70U, 0xB4U, 0x4AU, 0xFCU,
	0x98U, 0xDFU, 0x75U, 0x69U, 0xD8U, 0xF9U, 0x21U, 0x6AU,
	0x65U, 0xAAU, 0x30U, 0x84U, 0xCFU, 0x2EU, 0xCBU, 0x6CU,
	0x91U, 0xD5U, 0x6AU, 0x0DU, 0x46U, 0xE7U, 0x37U, 0xB0U,
	0x3FU, 0x1FU, 0x71U, 0xD9U, 0x5DU, 0x0BU, 0xD6U, 0x5CU,
	0x61U, 0xCBU, 0xD2U, 0x8AU, 0x96U, 0xB9U, 0x7EU, 0x1BU,
	0xA9U, 0x33U, 0x69U, 0xD3U, 0xBAU, 0x50U, 0xA1U, 0x10U,
	0xFEU, 0x53U, 0x1FU, 0x5CU, 0x63U, 0xB1U, 0xF6U, 0x3BU,
	0xB0U, 0xE8U, 0x83U, 0x0BU, 0x5FU, 0x30U, 0x00U, 0x35U,
	0x1FU, 0xA3U, 0x4EU, 0x7AU, 0x3EU, 0xE1U, 0x51U, 0xAEU,
	0x7CU, 0x62U, 0xAFU, 0x06U, 0x99U, 0x5EU, 0x14U, 0x73U,
	0xF5U, 0x7CU, 0x35U, 0x40U, 0xBBU, 0xB7U, 0xA3U, 0x3CU,
	0x13U, 0xE0U, 0x92U, 0x7EU, 0x03U, 0x16U, 0x73U, 0xADU,
	0x78U, 0x26U, 0x0BU, 0x13U, 0x29U, 0x2FU, 0x5FU, 0x29U,
	0x40U, 0xD8U, 0xBFU, 0x5EU, 0x73U, 0xFAU, 0x55U, 0x2DU,
	0x3EU, 0xABU, 0x7FU, 0x3CU, 0xB3U, 0x58U, 0xA5U, 0x9FU,
	0xA8U, 0x3CU, 0x12U, 0x58U, 0x27U, 0xC5U, 0xE3U, 0x0BU,
	0x11U, 0xCDU, 0x3FU, 0x1DU, 0xAAU, 0x98U, 0x77U, 0x0FU,
	0x6DU, 0x99U, 0x26U, 0xE2U, 0x73U, 0x28U, 0x31U, 0x8EU,
	0x9CU, 0x40U, 0x51U, 0xC8U, 0x58U, 0x52U, 0xA1U, 0x07U,
	0xADU, 0xAFU, 0x36U, 0xB8U, 0xC8U, 0x08U, 0x49U, 0xD7U,
	0xCEU, 0x28U, 0x7FU, 0xE3U, 0xB2U, 0xF1U, 0xB1U, 0xE9U,
	0x6AU, 0x59U, 0x43U, 0xDCU, 0xD8U, 0x4DU, 0x68U, 0xC0U,
	0x11U, 0x21U, 0xEEU, 0xEDU, 0xB0U, 0x0BU, 0xA8U, 0x24U,
	0xE0U, 0xD5U, 0x36U, 0xC2U, 0xFDU, 0x3EU, 0x35U, 0xC3U,
	0x37U, 0xC4U, 0xA2U, 0x84U, 0xA4U, 0xC2U, 0xD6U, 0xECU,
	0xAEU, 0xFFU, 0xFBU, 0xC5U, 0xD1U, 0x05U, 0xD2U, 0x23U
};

/** RSA private exponent */
static const u8 XAsu_RsaPvtExp[XASU_RSA_2048_KEY_SIZE_IN_BYTES]
	__attribute__ ((section (".data.XAsu_RsaPvtExp"))) = {
	0x0AU, 0xA8U, 0x1BU, 0x41U, 0xB1U, 0x20U, 0xD3U, 0x7DU,
	0x17U, 0xCCU, 0xF2U, 0xADU, 0x14U, 0x2EU, 0x53U, 0xC3U,
	0x5BU, 0x36U, 0x06U, 0x94U, 0xE1U, 0x10U, 0x70U, 0xF0U,
	0xFCU, 0x74U, 0xA1U, 0x76U, 0xE3U, 0x16U, 0xD1U, 0xB6U,
	0x8DU, 0xD5U, 0x6BU, 0x36U, 0x11U, 0xB7U, 0xACU, 0xF1U,
	0x4EU, 0x2DU, 0x9CU, 0x2CU, 0xE6U, 0xB7U, 0x24U, 0x05U,
	0xE3U, 0xEDU, 0x5FU, 0xC2U, 0x15U, 0x63U, 0x7EU, 0x84U,
	0x73U, 0x32U, 0x7DU, 0x07U, 0xE9U, 0x72U, 0x09U, 0x13U,
	0x50U, 0x82U, 0x35U, 0x96U, 0x1FU, 0x66U, 0x3FU, 0x3EU,
	0xEDU, 0x69U, 0x25U, 0xCEU, 0xBDU, 0xDAU, 0xD5U, 0xB0U,
	0x04U, 0x88U, 0x9CU, 0x06U, 0xB2U, 0x8DU, 0x13U, 0x3FU,
	0xEDU, 0xFAU, 0xE2U, 0x8BU, 0xF1U, 0x41U, 0xADU, 0xBDU,
	0x52U, 0x2FU, 0x8FU, 0xAEU, 0x59U, 0xA7U, 0xE1U, 0xBDU,
	0xDAU, 0xD5U, 0x1DU, 0xFDU, 0xD8U, 0x4BU, 0x1DU, 0x08U,
	0x1FU, 0x28U, 0x1BU, 0xC4U, 0x58U, 0x05U, 0xF2U, 0xAAU,
	0x74U, 0x8AU, 0xB1U, 0xEBU, 0xEDU, 0xF5U, 0x0BU, 0xBBU,
	0xB6U, 0x16U, 0x8DU, 0x2BU, 0xE3U, 0x81U, 0xC5U, 0x23U,
	0xC8U, 0x34U, 0x37U, 0x6DU, 0xE0U, 0xE6U, 0xF3U, 0xA8U,
	0x57U, 0xAFU, 0xA2U, 0xABU, 0x74U, 0xAEU, 0xA1U, 0x33U,
	0x6EU, 0x81U, 0x0BU, 0x73U, 0x23U, 0x39U, 0xE2U, 0xCBU,
	0xD6U, 0xA0U, 0xE5U, 0xBFU, 0x6DU, 0x4AU, 0x23U, 0x10U,
	0x1BU, 0x5BU, 0xAAU, 0x6EU, 0xDAU, 0x76U, 0x11U, 0x7CU,
	0xB5U, 0xFBU, 0xCAU, 0xE2U, 0xF8U, 0xB5U, 0x54U, 0x10U,
	0x29U, 0x5CU, 0x30U, 0x19U, 0x0DU, 0x09U, 0x85U, 0x9AU,
	0x2DU, 0xFBU, 0x7AU, 0xB7U, 0xA2U, 0xFBU, 0xCBU, 0xA7U,
	0x83U, 0x08U, 0xB2U, 0x87U, 0x81U, 0xDDU, 0x6BU, 0x52U,
	0x91U, 0xC1U, 0x10U, 0x4DU, 0x1DU, 0x55U, 0xA1U, 0x5EU,
	0xACU, 0xFCU, 0x3CU, 0x6AU, 0x1CU, 0x0FU, 0xDCU, 0x55U,
	0x64U, 0x0FU, 0x56U, 0x2CU, 0x37U, 0x2FU, 0xF7U, 0xE6U,
	0x90U, 0xE8U, 0x99U, 0xE3U, 0x06U, 0x34U, 0xF8U, 0xF2U,
	0xE2U, 0x90U, 0x1CU, 0x5CU, 0xD9U, 0xA8U, 0x45U, 0x72U,
	0x40U, 0x94U, 0x5CU, 0x3CU, 0x28U, 0x32U, 0x44U, 0xD1U
};

/** RSA public exponent */
static const u32 XAsu_RsaPublicExp
	__attribute__ ((section (".data.XAsu_RsaPublicExp"))) = 0x1000100U;

/** RSA input data for signing */
static const u8 XAsu_RsaInputData[XASU_RSA_INPUT_DATA_LEN_IN_BYTES]
	__attribute__ ((section (".data.XAsu_RsaInputData"))) = {
	0x05U, 0x95U, 0x87U, 0x3FU, 0xA5U, 0x76U, 0x50U, 0xBFU,
	0x76U, 0x6FU, 0x2CU, 0xB3U, 0x97U, 0x80U, 0x8BU, 0x7BU,
	0x99U, 0xE8U, 0x56U, 0x2FU, 0x4BU, 0xCDU, 0x66U, 0x25U,
	0x6FU, 0xA9U, 0x51U, 0x7CU, 0x42U, 0x57U, 0x95U, 0x95U,
	0x6AU, 0xF0U, 0xF8U, 0x7EU, 0x56U, 0x0DU, 0x28U, 0x07U,
	0xE7U, 0x1CU, 0x7CU, 0xD8U, 0x9EU, 0xF2U, 0xD6U, 0x37U,
	0xE8U, 0x7AU, 0x5AU, 0xDBU, 0xA5U, 0xB5U, 0xD2U, 0x93U,
	0xFAU, 0x29U, 0x28U, 0x76U, 0xB8U, 0xCEU, 0xC3U, 0x66U,
	0xFFU, 0x15U, 0xFBU, 0x28U, 0x3BU, 0xC2U, 0x82U, 0x32U,
	0xB7U, 0x5FU, 0xE4U, 0xD5U, 0x54U, 0x8EU, 0xE5U, 0x43U
};

/** RSA PSS signature output buffer */
static u8 XAsu_RsaSignature[XASU_RSA_2048_KEY_SIZE_IN_BYTES]
	__attribute__ ((section (".data.XAsu_RsaSignature")));

/* Callback Context for Each Operation */
/**
 * Array of callback contexts, one for each crypto operation.
 * These contexts allow the callback function to identify which operation completed.
 */
static XAsu_CallbackContext XAsu_OpContexts[XASU_OP_MAX] = {
	{ .OpType = XASU_OP_SHA2_HASH,    .Status = XST_FAILURE, .IsCompleted = FALSE,
	  .OpName = "SHA2 Hash (LOW priority)" },
	{ .OpType = XASU_OP_AES_ENCRYPT,  .Status = XST_FAILURE, .IsCompleted = FALSE,
	  .OpName = "AES-GCM Encrypt (HIGH priority)" },
	{ .OpType = XASU_OP_AES_DECRYPT,  .Status = XST_FAILURE, .IsCompleted = FALSE,
	  .OpName = "AES-GCM Decrypt (HIGH priority)" },
	{ .OpType = XASU_OP_ECC_SIGN_GEN, .Status = XST_FAILURE, .IsCompleted = FALSE,
	  .OpName = "ECC Sign Gen (HIGH priority)" },
	{ .OpType = XASU_OP_RSA_PSS_SIGN_GEN, .Status = XST_FAILURE, .IsCompleted = FALSE,
	  .OpName = "RSA PSS Sign Gen (LOW priority)" },
	{ .OpType = XASU_OP_RSA_PSS_SIGN_VER, .Status = XST_FAILURE, .IsCompleted = FALSE,
	  .OpName = "RSA PSS Sign Ver (HIGH priority)" }
};

/* Volatile completion counter - incremented by callback, read by main */
static volatile u32 XAsu_CompletedOpsCount = 0U;

/************************************ Function Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	Main function to demonstrate multi-crypto queue operations.
 *
 * @return
 *		- XST_SUCCESS if all operations complete successfully.
 *		- XST_FAILURE if any operation fails.
 *
 *************************************************************************************************/
int main(void)
{
	s32 Status = XST_FAILURE;
	XMailbox MailboxInstance;

	xil_printf("\r\n=====================================================\r\n");
	xil_printf("ASU Multi-Crypto Queue Example\r\n");
	xil_printf("=====================================================\r\n");
	xil_printf("\r\nThis example demonstrates:\r\n");
	xil_printf("  1. Sending multiple crypto commands with different priorities\r\n");
	xil_printf("  2. Using unique callback contexts to identify responses\r\n");
	xil_printf("  3. Concurrent AES, SHA2, ECC, and RSA operations\r\n");
	xil_printf("  4. Proper handling of ASU queue scheduler\r\n");
	xil_printf("=====================================================\r\n\r\n");

#ifdef XASU_DISABLE_CACHE
	Xil_DCacheDisable();
#endif

	/** Initialize mailbox instance. */
	Status = (s32)XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR: Mailbox initialize failed: %08x\r\n", Status);
		goto END;
	}

	/* Initialize the ASU client instance */
	Status = XAsu_ClientInit(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR: ASU Client initialize failed: %08x\r\n", Status);
		goto END;
	}

	xil_printf("ASU Client initialized successfully\r\n\r\n");

	/* Run the multi-crypto example */
	Status = XAsu_MultiCryptoExample();

END:
	if (Status == XST_SUCCESS) {
		xil_printf("\r\n=====================================================\r\n");
		xil_printf("SUCCESS: Multi-Crypto Queue Example completed!\r\n");
		xil_printf("=====================================================\r\n");
	} else {
		xil_printf("\r\n=====================================================\r\n");
		xil_printf("FAILURE: Multi-Crypto Queue Example failed\r\n");
		xil_printf("=====================================================\r\n");
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Executes the multi-crypto example demonstrating concurrent queue operations.
 *
 * This function demonstrates how to:
 * 1. Send multiple crypto commands simultaneously with different priorities
 * 2. Use unique callback contexts to track which operation completed
 * 3. Wait for all operations to complete
 * 4. Verify results from each operation
 *
 * @return
 *		- XST_SUCCESS if all operations complete successfully.
 *		- XST_FAILURE if any operation fails.
 *
 *************************************************************************************************/
static s32 XAsu_MultiCryptoExample(void)
{
	s32 Set1Status = XST_FAILURE;
	s32 Set2Status = XST_FAILURE;

	xil_printf("Phase 1: Sending crypto requests Set 1...\r\n");
	xil_printf("----------------------------------------------------------\r\n");

	/* Send Set 1 crypto commands and wait for completion */
	Set1Status = XAsu_SendCryptoRequestsSet1();
	if (Set1Status != XST_SUCCESS) {
		xil_printf("ERROR: Crypto requests Set 1 failed\r\n");
	}

	xil_printf("\r\nPhase 2: Sending crypto requests Set 2 (dependent operations)...\r\n");
	xil_printf("----------------------------------------------------------\r\n");

	/* Send Set 2 crypto commands and wait for completion */
	Set2Status = XAsu_SendCryptoRequestsSet2();
	if (Set2Status != XST_SUCCESS) {
		xil_printf("ERROR: Crypto requests Set 2 failed\r\n");
	}

	xil_printf("\r\nPhase 3: Verifying results...\r\n");
	xil_printf("----------------------------------------------------------\r\n");

	/* Print and verify results */
	XAsu_PrintResults();

	/* Success only if both sets succeeded */
	return (Set1Status | Set2Status);
}

/*************************************************************************************************/
/**
 * @brief	Sends crypto requests Set 1 and waits for completion.
 *
 * Set 1 operations:
 * - SHA2 hash with LOW priority
 * - AES encryption with HIGH priority
 * - ECC signature generation with HIGH priority
 * - RSA PSS signature generation with LOW priority
 *
 * @return
 *		- XST_SUCCESS if all operations complete successfully.
 *		- XST_FAILURE if any operation fails.
 *
 *************************************************************************************************/
static s32 XAsu_SendCryptoRequestsSet1(void)
{
	s32 Status = XST_FAILURE;
	u8 Set1OpsCount = 0U;

	XAsu_ClientParams Sha2HashClientParams;
	XAsu_ClientParams AesEncryptClientParams;
	XAsu_ClientParams EccSignGenClientParams;
	XAsu_ClientParams RsaPssSignGenClientParams;

	XAsu_ShaOperationCmd Sha2HashParams;
	XAsu_AesParams AesEncryptParams;
	XAsu_EccParams EccSignGenParams;
	XAsu_RsaPaddingParams RsaPssSignGenParams;

	XAsu_AesKeyObject AesKeyObj;
	XAsu_RsaPubKeyComp RsaPubKeyComp;
	XAsu_RsaPvtKeyComp RsaPvtKeyComp;

	/* Reset completion counter for this set */
	XAsu_CompletedOpsCount = 0U;

	/*
	 * Operation 1: SHA2 Hash (LOW Priority)
	 */
	xil_printf("  [1] Sending SHA2 Hash operation (LOW priority)...\r\n");

	Sha2HashClientParams.Priority = XASU_PRIORITY_LOW;
	Sha2HashClientParams.SecureFlag = XASU_CMD_SECURE;
	Sha2HashClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_MultiCryptoCallBack);
	Sha2HashClientParams.CallBackRefPtr = (void *)&XAsu_OpContexts[XASU_OP_SHA2_HASH];

	Sha2HashParams.DataAddr = (u64)(UINTPTR)XAsu_Sha2Data;
	Sha2HashParams.HashAddr = (u64)(UINTPTR)XAsu_Sha2Hash;
	Sha2HashParams.DataSize = XASU_SHA2_INPUT_DATA_LEN;
	Sha2HashParams.HashBufSize = XASU_SHA2_HASH_LEN_IN_BYTES;
	Sha2HashParams.ShaMode = XASU_SHA_MODE_256;
	Sha2HashParams.IsLast = TRUE;
	Sha2HashParams.OperationFlags = (XASU_INIT | XASU_UPDATE | XASU_FINISH);

	Status = XAsu_Sha2Operation(&Sha2HashClientParams, &Sha2HashParams);
	if (Status != XST_SUCCESS) {
		xil_printf("      ERROR: SHA2 operation send failed: %08x\r\n", Status);
	} else {
		xil_printf("      SHA2 command queued successfully\r\n");
		Set1OpsCount++;
	}

	/*
	 * Operation 2: AES-GCM Encryption (HIGH Priority)
	 */
	xil_printf("  [2] Sending AES-GCM Encryption operation (HIGH priority)...\r\n");

	AesEncryptClientParams.Priority = XASU_PRIORITY_HIGH;
	AesEncryptClientParams.SecureFlag = XASU_CMD_SECURE;
	AesEncryptClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_MultiCryptoCallBack);
	AesEncryptClientParams.CallBackRefPtr = (void *)&XAsu_OpContexts[XASU_OP_AES_ENCRYPT];

	AesKeyObj.KeyAddress = (u64)(UINTPTR)XAsu_AesKey;
	AesKeyObj.KeySize = XASU_AES_KEY_SIZE_256_BITS;
	AesKeyObj.KeySrc = XASU_AES_USER_KEY_0;
	AesKeyObj.KeyId = 0U;

	AesEncryptParams.EngineMode = XASU_AES_GCM_MODE;
	AesEncryptParams.OperationFlags = (XASU_INIT | XASU_UPDATE | XASU_FINISH);
	AesEncryptParams.IsLast = TRUE;
	AesEncryptParams.OperationType = XASU_AES_ENCRYPT_OPERATION;
	AesEncryptParams.KeyObjectAddr = (u64)(UINTPTR)&AesKeyObj;
	AesEncryptParams.IvAddr = (u64)(UINTPTR)XAsu_AesIv;
	AesEncryptParams.IvLen = XASU_AES_IV_LEN_IN_BYTES;
	AesEncryptParams.IvId = 0U;
	AesEncryptParams.InputDataAddr = (u64)(UINTPTR)XAsu_AesInputData;
	AesEncryptParams.OutputDataAddr = (u64)(UINTPTR)XAsu_AesEncData;
	AesEncryptParams.DataLen = XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES;
	AesEncryptParams.AadAddr = (u64)(UINTPTR)XAsu_AesAad;
	AesEncryptParams.AadLen = XASU_AES_AAD_LEN_IN_BYTES;
	AesEncryptParams.TagAddr = (u64)(UINTPTR)XAsu_AesTag;
	AesEncryptParams.TagLen = XASU_AES_TAG_LEN_IN_BYTES;

	Status = XAsu_AesOperation(&AesEncryptClientParams, &AesEncryptParams);
	if (Status != XST_SUCCESS) {
		xil_printf("      ERROR: AES Encrypt operation send failed: %08x\r\n", Status);
	} else {
		xil_printf("      AES-GCM Encrypt command queued successfully\r\n");
		Set1OpsCount++;
	}

	/*
	 * Operation 3: ECC Signature Generation (HIGH Priority)
	 */
	xil_printf("  [3] Sending ECC Sign Generation operation (HIGH priority)...\r\n");

	EccSignGenClientParams.Priority = XASU_PRIORITY_HIGH;
	EccSignGenClientParams.SecureFlag = XASU_CMD_SECURE;
	EccSignGenClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_MultiCryptoCallBack);
	EccSignGenClientParams.CallBackRefPtr = (void *)&XAsu_OpContexts[XASU_OP_ECC_SIGN_GEN];

	EccSignGenParams.CurveType = XASU_ECC_NIST_P256;
	EccSignGenParams.DigestAddr = (u64)(UINTPTR)XAsu_EccHash;
	EccSignGenParams.Key.KeyAddr = (u64)(UINTPTR)XAsu_EccPrivKey;
	EccSignGenParams.SignAddr = (u64)(UINTPTR)XAsu_EccSign;
	EccSignGenParams.DigestLen = XASU_ECC_P256_KEY_SIZE_IN_BYTES;
	EccSignGenParams.Key.KeyLen = XASU_ECC_P256_KEY_SIZE_IN_BYTES;
	EccSignGenParams.Key.KeyId = 0U;

	Status = XAsu_EccGenSign(&EccSignGenClientParams, &EccSignGenParams);
	if (Status != XST_SUCCESS) {
		xil_printf("      ERROR: ECC Sign Gen operation send failed: %08x\r\n", Status);
	} else {
		xil_printf("      ECC Sign Gen command queued successfully\r\n");
		Set1OpsCount++;
	}

	/*
	 * Operation 4: RSA PSS Signature Generation (LOW Priority)
	 */
	xil_printf("  [4] Sending RSA PSS Sign Generation operation (LOW priority)...\r\n");

	RsaPubKeyComp.Keysize = XASU_RSA_2048_KEY_SIZE_IN_BYTES;
	RsaPubKeyComp.PubExp = XAsu_RsaPublicExp;
	(void)Xil_SMemCpy(RsaPubKeyComp.Modulus, XASU_RSA_2048_KEY_SIZE_IN_BYTES,
			  XAsu_RsaModulus, XASU_RSA_2048_KEY_SIZE_IN_BYTES,
			  XASU_RSA_2048_KEY_SIZE_IN_BYTES);

	RsaPvtKeyComp.PubKeyComp = RsaPubKeyComp;
	RsaPvtKeyComp.PrimeCompOrTotientPrsnt = 0U;
	(void)Xil_SMemCpy(RsaPvtKeyComp.PvtExp, XASU_RSA_2048_KEY_SIZE_IN_BYTES,
			  XAsu_RsaPvtExp, XASU_RSA_2048_KEY_SIZE_IN_BYTES,
			  XASU_RSA_2048_KEY_SIZE_IN_BYTES);

	RsaPssSignGenClientParams.Priority = XASU_PRIORITY_LOW;
	RsaPssSignGenClientParams.SecureFlag = XASU_CMD_SECURE;
	RsaPssSignGenClientParams.CallBackFuncPtr =
		(XAsuClient_ResponseHandler)((void *)XAsu_MultiCryptoCallBack);
	RsaPssSignGenClientParams.CallBackRefPtr =
		(void *)&XAsu_OpContexts[XASU_OP_RSA_PSS_SIGN_GEN];

	RsaPssSignGenParams.XAsu_RsaOpComp.InputDataAddr = (u64)(UINTPTR)XAsu_RsaInputData;
	RsaPssSignGenParams.XAsu_RsaOpComp.OutputDataAddr = (u64)(UINTPTR)XAsu_RsaSignature;
	RsaPssSignGenParams.XAsu_RsaOpComp.OutputDataLen = XASU_RSA_2048_KEY_SIZE_IN_BYTES;
	RsaPssSignGenParams.XAsu_RsaOpComp.Len = XASU_RSA_INPUT_DATA_LEN_IN_BYTES;
	RsaPssSignGenParams.XAsu_RsaOpComp.KeySize = XASU_RSA_2048_KEY_SIZE_IN_BYTES;
	RsaPssSignGenParams.XAsu_RsaOpComp.KeyCompAddr = (u64)(UINTPTR)&RsaPvtKeyComp;
	RsaPssSignGenParams.XAsu_RsaOpComp.ExpoCompAddr = 0U;
	RsaPssSignGenParams.SaltLen = XASU_RSA_PSS_SALT_LEN_IN_BYTES;
	RsaPssSignGenParams.InputDataType = 0U;
	RsaPssSignGenParams.SignatureLen = XASU_RSA_2048_KEY_SIZE_IN_BYTES;
	RsaPssSignGenParams.SignatureDataAddr = 0U;
	RsaPssSignGenParams.ShaType = XASU_SHA2_TYPE;
	RsaPssSignGenParams.ShaMode = XASU_SHA_MODE_256;

	Status = XAsu_RsaPssSignGen(&RsaPssSignGenClientParams, &RsaPssSignGenParams);
	if (Status != XST_SUCCESS) {
		xil_printf("      ERROR: RSA PSS Sign Gen operation send failed: %08x\r\n", Status);
	} else {
		xil_printf("      RSA PSS Sign Gen command queued successfully\r\n");
		Set1OpsCount++;
	}

	if (Set1OpsCount == 0U) {
		xil_printf("\r\n  ERROR: No Set 1 operations queued\r\n");
		Status = XST_FAILURE;
		goto END;
	}

	xil_printf("\r\n  %d of %d Set 1 commands sent to ASU queue\r\n", Set1OpsCount, XASU_SET1_OPS_COUNT);
	xil_printf("  Waiting for completion...\r\n");

	/* Wait for all queued Set 1 operations to complete */
	while (XAsu_CompletedOpsCount < Set1OpsCount) {
		XAsu_WaitForEvent();
	}

	xil_printf("  Set 1 completed!\r\n\r\n");

	/* Print completion status */
	xil_printf("  Operation completion status:\r\n");
	for (u8 Index = 0U; Index < XASU_OP_MAX; Index++) {
		if ((Index == XASU_OP_AES_DECRYPT) || (Index == XASU_OP_RSA_PSS_SIGN_VER)) {
			continue;
		}
		xil_printf("    [%s] - Status: %08x\r\n",
			   XAsu_OpContexts[Index].OpName,
			   XAsu_OpContexts[Index].Status);
	}

	/* Combine all Set 1 operation statuses using bitwise OR */
	Status = (XAsu_OpContexts[XASU_OP_SHA2_HASH].Status |
		  XAsu_OpContexts[XASU_OP_AES_ENCRYPT].Status |
		  XAsu_OpContexts[XASU_OP_ECC_SIGN_GEN].Status |
		  XAsu_OpContexts[XASU_OP_RSA_PSS_SIGN_GEN].Status);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Sends crypto requests Set 2 and waits for completion.
 *
 * Set 2 operations (dependent on Set 1 results):
 * - AES decryption with HIGH priority (uses encryption output from Set 1)
 * - RSA PSS signature verification with HIGH priority (uses signature from Set 1)
 *
 * @return
 *		- XST_SUCCESS if all operations complete successfully.
 *		- XST_FAILURE if any operation fails.
 *
 *************************************************************************************************/
static s32 XAsu_SendCryptoRequestsSet2(void)
{
	s32 Status = XST_FAILURE;
	u8 Set2OpsCount = 0U;

	XAsu_ClientParams AesDecryptClientParams;
	XAsu_ClientParams RsaPssSignVerClientParams;

	XAsu_AesParams AesDecryptParams;
	XAsu_RsaPaddingParams RsaPssSignVerParams;

	XAsu_AesKeyObject AesKeyObj;
	XAsu_RsaPubKeyComp RsaPubKeyComp;

	/* Reset completion counter for this set */
	XAsu_CompletedOpsCount = 0U;

	/*
	 * Operation 5: AES-GCM Decryption (depends on encryption result)
	 */
	if (XAsu_OpContexts[XASU_OP_AES_ENCRYPT].Status == XST_SUCCESS) {
		xil_printf("  [5] Sending AES-GCM Decryption operation (HIGH priority)...\r\n");

		AesDecryptClientParams.Priority = XASU_PRIORITY_HIGH;
		AesDecryptClientParams.SecureFlag = XASU_CMD_SECURE;
		AesDecryptClientParams.CallBackFuncPtr =
			(XAsuClient_ResponseHandler)((void *)XAsu_MultiCryptoCallBack);
		AesDecryptClientParams.CallBackRefPtr = (void *)&XAsu_OpContexts[XASU_OP_AES_DECRYPT];
		AesDecryptClientParams.AdditionalStatus = XST_FAILURE;

		AesKeyObj.KeyAddress = (u64)(UINTPTR)XAsu_AesKey;
		AesKeyObj.KeySize = XASU_AES_KEY_SIZE_256_BITS;
		AesKeyObj.KeySrc = XASU_AES_USER_KEY_0;
		AesKeyObj.KeyId = 0U;

		AesDecryptParams.EngineMode = XASU_AES_GCM_MODE;
		AesDecryptParams.OperationFlags = (XASU_INIT | XASU_UPDATE | XASU_FINISH);
		AesDecryptParams.IsLast = TRUE;
		AesDecryptParams.OperationType = XASU_AES_DECRYPT_OPERATION;
		AesDecryptParams.KeyObjectAddr = (u64)(UINTPTR)&AesKeyObj;
		AesDecryptParams.IvAddr = (u64)(UINTPTR)XAsu_AesIv;
		AesDecryptParams.IvLen = XASU_AES_IV_LEN_IN_BYTES;
		AesDecryptParams.IvId = 0U;
		AesDecryptParams.InputDataAddr = (u64)(UINTPTR)XAsu_AesEncData;
		AesDecryptParams.OutputDataAddr = (u64)(UINTPTR)XAsu_AesDecData;
		AesDecryptParams.DataLen = XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES;
		AesDecryptParams.AadAddr = (u64)(UINTPTR)XAsu_AesAad;
		AesDecryptParams.AadLen = XASU_AES_AAD_LEN_IN_BYTES;
		AesDecryptParams.TagAddr = (u64)(UINTPTR)XAsu_AesTag;
		AesDecryptParams.TagLen = XASU_AES_TAG_LEN_IN_BYTES;

		Status = XAsu_AesOperation(&AesDecryptClientParams, &AesDecryptParams);
		if (Status != XST_SUCCESS) {
			xil_printf("      ERROR: AES Decrypt send failed: %08x\r\n", Status);
		} else {
			xil_printf("      AES-GCM Decrypt command queued\r\n");
			Set2OpsCount++;
		}
	}

	/*
	 * Operation 6: RSA PSS Signature Verification (depends on sign generation result)
	 */
	if (XAsu_OpContexts[XASU_OP_RSA_PSS_SIGN_GEN].Status == XST_SUCCESS) {
		xil_printf("  [6] Sending RSA PSS Sign Verification operation (HIGH priority)...\r\n");

		RsaPssSignVerClientParams.Priority = XASU_PRIORITY_HIGH;
		RsaPssSignVerClientParams.SecureFlag = XASU_CMD_SECURE;
		RsaPssSignVerClientParams.CallBackFuncPtr =
			(XAsuClient_ResponseHandler)((void *)XAsu_MultiCryptoCallBack);
		RsaPssSignVerClientParams.CallBackRefPtr =
			(void *)&XAsu_OpContexts[XASU_OP_RSA_PSS_SIGN_VER];
		RsaPssSignVerClientParams.AdditionalStatus = XST_FAILURE;

		RsaPubKeyComp.Keysize = XASU_RSA_2048_KEY_SIZE_IN_BYTES;
		RsaPubKeyComp.PubExp = XAsu_RsaPublicExp;
		(void)Xil_SMemCpy(RsaPubKeyComp.Modulus, XASU_RSA_2048_KEY_SIZE_IN_BYTES,
				  XAsu_RsaModulus, XASU_RSA_2048_KEY_SIZE_IN_BYTES,
				  XASU_RSA_2048_KEY_SIZE_IN_BYTES);

		RsaPssSignVerParams.XAsu_RsaOpComp.InputDataAddr = (u64)(UINTPTR)XAsu_RsaInputData;
		RsaPssSignVerParams.XAsu_RsaOpComp.OutputDataAddr = 0U;
		RsaPssSignVerParams.XAsu_RsaOpComp.Len = XASU_RSA_INPUT_DATA_LEN_IN_BYTES;
		RsaPssSignVerParams.XAsu_RsaOpComp.KeySize = XASU_RSA_2048_KEY_SIZE_IN_BYTES;
		RsaPssSignVerParams.XAsu_RsaOpComp.KeyCompAddr = (u64)(UINTPTR)&RsaPubKeyComp;
		RsaPssSignVerParams.XAsu_RsaOpComp.ExpoCompAddr = 0U;
		RsaPssSignVerParams.SaltLen = XASU_RSA_PSS_SALT_LEN_IN_BYTES;
		RsaPssSignVerParams.InputDataType = 0U;
		RsaPssSignVerParams.SignatureLen = XASU_RSA_2048_KEY_SIZE_IN_BYTES;
		RsaPssSignVerParams.SignatureDataAddr = (u64)(UINTPTR)XAsu_RsaSignature;
		RsaPssSignVerParams.ShaType = XASU_SHA2_TYPE;
		RsaPssSignVerParams.ShaMode = XASU_SHA_MODE_256;

		Status = XAsu_RsaPssSignVer(&RsaPssSignVerClientParams, &RsaPssSignVerParams);
		if (Status != XST_SUCCESS) {
			xil_printf("      ERROR: RSA PSS Sign Ver send failed: %08x\r\n", Status);
		} else {
			xil_printf("      RSA PSS Sign Ver command queued\r\n");
			Set2OpsCount++;
		}
	}

	if (Set2OpsCount == 0U) {
		xil_printf("  No Set 2 operations queued\r\n");
		Status = XST_FAILURE;
		goto END;
	}

	xil_printf("\r\n  All %d Set 2 commands sent to ASU queue\r\n", Set2OpsCount);
	xil_printf("  Waiting for completion...\r\n");

	/* Wait for all Set 2 operations to complete */
	while (XAsu_CompletedOpsCount < Set2OpsCount) {
		XAsu_WaitForEvent();
	}

	xil_printf("  Set 2 completed!\r\n\r\n");

	/* Print completion status */
	xil_printf("  Operation completion status:\r\n");
	xil_printf("    [%s] - Status: %08x\r\n",
		   XAsu_OpContexts[XASU_OP_AES_DECRYPT].OpName,
		   XAsu_OpContexts[XASU_OP_AES_DECRYPT].Status);
	xil_printf("    [%s] - Status: %08x\r\n",
		   XAsu_OpContexts[XASU_OP_RSA_PSS_SIGN_VER].OpName,
		   XAsu_OpContexts[XASU_OP_RSA_PSS_SIGN_VER].Status);

	/* Verify AES Decrypt status and AdditionalStatus */
	if (XAsu_OpContexts[XASU_OP_AES_DECRYPT].Status != XST_SUCCESS) {
		Status |= XAsu_OpContexts[XASU_OP_AES_DECRYPT].Status;
	} else if (AesDecryptClientParams.AdditionalStatus != XASU_AES_TAG_MATCHED) {
		xil_printf("    ERROR: AES Decrypt AdditionalStatus mismatch: %08x\r\n",
			   AesDecryptClientParams.AdditionalStatus);
		XAsu_OpContexts[XASU_OP_AES_DECRYPT].Status = XST_FAILURE;
		Status |= XST_FAILURE;
	}

	/* Verify RSA PSS Sign Ver status and AdditionalStatus */
	if (XAsu_OpContexts[XASU_OP_RSA_PSS_SIGN_VER].Status != XST_SUCCESS) {
		Status |= XAsu_OpContexts[XASU_OP_RSA_PSS_SIGN_VER].Status;
	} else if (RsaPssSignVerClientParams.AdditionalStatus != XASU_RSA_PSS_SIGNATURE_VERIFIED) {
		xil_printf("    ERROR: RSA PSS Sign Ver AdditionalStatus mismatch: %08x\r\n",
			   RsaPssSignVerClientParams.AdditionalStatus);
		XAsu_OpContexts[XASU_OP_RSA_PSS_SIGN_VER].Status = XST_FAILURE;
		Status |= XST_FAILURE;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Unified callback function for all crypto operations.
 *
 * This callback function demonstrates how to identify which operation completed
 * when multiple operations are pending. The CallBackRef points to the unique
 * XAsu_CallbackContext structure for each operation.
 *
 * NOTE: Each operation passes its own context structure as CallBackRefPtr.
 * When the callback is triggered, we cast CallBackRef back to XAsu_CallbackContext
 * to identify which specific operation completed and update its status.
 *
 * @param	CallBackRef	Pointer to the callback context (XAsu_CallbackContext)
 * @param	Status		Status of the completed operation
 *
 *************************************************************************************************/
static void XAsu_MultiCryptoCallBack(void *CallBackRef, u32 Status)
{
	XAsu_CallbackContext *Context = (XAsu_CallbackContext *)CallBackRef;

	if (Context != NULL) {
		/* Update the context with the operation result */
		Context->Status = Status;
		Context->IsCompleted = TRUE;

		/* Increment completion counter (callback runs in interrupt context) */
		XAsu_CompletedOpsCount++;

		/* Debug print to show operation completion order */
		XilAsu_Printf("  >> Callback: %s completed (Status: %08x)\r\n",
			      Context->OpName, Status);
	}
}

/*************************************************************************************************/
/**
 * @brief	Prints and verifies the results of all crypto operations.
 *
 *************************************************************************************************/
static void XAsu_PrintResults(void)
{
	s32 Status = XST_FAILURE;

	xil_printf("\r\n=== Operation Results ===\r\n\r\n");

	/* SHA2 Results */
	xil_printf("[SHA2 Hash Result]\r\n");
	if (XAsu_OpContexts[XASU_OP_SHA2_HASH].Status == XST_SUCCESS) {
		XAsu_PrintData(XAsu_Sha2Hash, XASU_SHA2_HASH_LEN_IN_BYTES, "  Computed Hash");

		/* Verify SHA2 hash */
		Status = Xil_SMemCmp_CT(XAsu_Sha2ExpHash, XASU_SHA2_HASH_LEN_IN_BYTES,
				       XAsu_Sha2Hash, XASU_SHA2_HASH_LEN_IN_BYTES,
				       XASU_SHA2_HASH_LEN_IN_BYTES);
		if (Status == XST_SUCCESS) {
			xil_printf("  Verification: PASSED\r\n");
		} else {
			xil_printf("  Verification: FAILED\r\n");
		}
	} else {
		xil_printf("  Status: FAILED (%08x)\r\n", XAsu_OpContexts[XASU_OP_SHA2_HASH].Status);
	}

	/* AES Encrypt Results */
	xil_printf("\r\n[AES-GCM Encryption Result]\r\n");
	if (XAsu_OpContexts[XASU_OP_AES_ENCRYPT].Status == XST_SUCCESS) {
		XAsu_PrintData(XAsu_AesEncData, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES, "  Ciphertext");
		XAsu_PrintData(XAsu_AesTag, XASU_AES_TAG_LEN_IN_BYTES, "  GCM Tag");

		/* Verify encrypted data */
		Status = Xil_SMemCmp_CT(XAsu_AesGcmExpCt, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES,
				       XAsu_AesEncData, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES,
				       XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES);
		if (Status == XST_SUCCESS) {
			/* Verify GCM tag */
			Status = Xil_SMemCmp_CT(XAsu_AesGcmExpTag, XASU_AES_TAG_LEN_IN_BYTES,
					       XAsu_AesTag, XASU_AES_TAG_LEN_IN_BYTES,
					       XASU_AES_TAG_LEN_IN_BYTES);
		}
		if (Status == XST_SUCCESS) {
			xil_printf("  Verification: PASSED\r\n");
		} else {
			xil_printf("  Verification: FAILED\r\n");
		}
	} else {
		xil_printf("  Status: FAILED (%08x)\r\n", XAsu_OpContexts[XASU_OP_AES_ENCRYPT].Status);
	}

	/* AES Decrypt Results */
	xil_printf("\r\n[AES-GCM Decryption Result]\r\n");
	if (XAsu_OpContexts[XASU_OP_AES_DECRYPT].Status == XST_SUCCESS) {
		XAsu_PrintData(XAsu_AesDecData, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES, "  Plaintext");

		/* Verify decrypted data matches original */
		Status = Xil_SMemCmp_CT(XAsu_AesInputData, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES,
				       XAsu_AesDecData, XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES,
				       XASU_AES_PAYLOAD_DATA_LEN_IN_BYTES);
		if (Status == XST_SUCCESS) {
			xil_printf("  Verification: PASSED (matches original plaintext)\r\n");
		} else {
			xil_printf("  Verification: FAILED\r\n");
		}
	} else {
		xil_printf("  Status: FAILED (%08x)\r\n", XAsu_OpContexts[XASU_OP_AES_DECRYPT].Status);
	}

	/* ECC Sign Results */
	xil_printf("\r\n[ECC Signature Generation Result]\r\n");
	if (XAsu_OpContexts[XASU_OP_ECC_SIGN_GEN].Status == XST_SUCCESS) {
		XAsu_PrintData(XAsu_EccSign, XASU_ECC_DOUBLE_P256_SIZE_IN_BYTES, "  Signature (R||S)");
		xil_printf("  Verification: Signature generated successfully\r\n");
	} else {
		xil_printf("  Status: FAILED (%08x)\r\n", XAsu_OpContexts[XASU_OP_ECC_SIGN_GEN].Status);
	}

	/* RSA PSS Sign Generation Results */
	xil_printf("\r\n[RSA PSS Sign Generation Result]\r\n");
	if (XAsu_OpContexts[XASU_OP_RSA_PSS_SIGN_GEN].Status == XST_SUCCESS) {
		XAsu_PrintData(XAsu_RsaSignature, 64U, "  Signature (first 64 bytes)");
		xil_printf("  Verification: Signature generated successfully\r\n");
	} else {
		xil_printf("  Status: FAILED (%08x)\r\n", XAsu_OpContexts[XASU_OP_RSA_PSS_SIGN_GEN].Status);
	}

	/* RSA PSS Sign Verification Results */
	xil_printf("\r\n[RSA PSS Sign Verification Result]\r\n");
	if (XAsu_OpContexts[XASU_OP_RSA_PSS_SIGN_VER].Status == XST_SUCCESS) {
		xil_printf("  Verification: PASSED (signature verified successfully)\r\n");
	} else {
		xil_printf("  Status: FAILED (%08x)\r\n", XAsu_OpContexts[XASU_OP_RSA_PSS_SIGN_VER].Status);
	}

	xil_printf("\r\n=== Summary ===\r\n");
	xil_printf("  Total Operations: 6\r\n");
	xil_printf("  - SHA2 Hash (LOW priority):          %s\r\n",
		   XAsu_OpContexts[XASU_OP_SHA2_HASH].Status == XST_SUCCESS ? "PASS" : "FAIL");
	xil_printf("  - AES-GCM Encrypt (HIGH priority):   %s\r\n",
		   XAsu_OpContexts[XASU_OP_AES_ENCRYPT].Status == XST_SUCCESS ? "PASS" : "FAIL");
	xil_printf("  - AES-GCM Decrypt (HIGH priority):   %s\r\n",
		   XAsu_OpContexts[XASU_OP_AES_DECRYPT].Status == XST_SUCCESS ? "PASS" : "FAIL");
	xil_printf("  - ECC Sign Gen (HIGH priority):      %s\r\n",
		   XAsu_OpContexts[XASU_OP_ECC_SIGN_GEN].Status == XST_SUCCESS ? "PASS" : "FAIL");
	xil_printf("  - RSA PSS Sign Gen (LOW priority):   %s\r\n",
		   XAsu_OpContexts[XASU_OP_RSA_PSS_SIGN_GEN].Status == XST_SUCCESS ? "PASS" : "FAIL");
	xil_printf("  - RSA PSS Sign Ver (HIGH priority):  %s\r\n",
		   XAsu_OpContexts[XASU_OP_RSA_PSS_SIGN_VER].Status == XST_SUCCESS ? "PASS" : "FAIL");
}

/*************************************************************************************************/
/**
 * @brief	Prints data array with a label.
 *
 * @param	Data	Pointer to data array
 * @param	DataLen	Length of the data
 * @param	Label	Label string to print before data
 *
 *************************************************************************************************/
static void XAsu_PrintData(const u8 *Data, u32 DataLen, const char *Label)
{
	u32 Index;

	xil_printf("%s: ", Label);
	for (Index = 0U; Index < DataLen; Index++) {
		xil_printf("%02x", Data[Index]);
		if ((Index + 1U) % 16U == 0U && Index + 1U < DataLen) {
			xil_printf("\r\n              ");
		}
	}
	xil_printf("\r\n");
}

/** @} */
