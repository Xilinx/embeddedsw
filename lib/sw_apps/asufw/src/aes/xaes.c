/**************************************************************************************************
* Copyright (c) 2024 - 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xaes.c
 *
 * This file contains low level implementation of AES hardware interface APIs
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   06/26/24 Initial release
 *       am   08/01/24 Replaced variables of type enums with u32 type.
 *       am   08/24/24 Added AES DPA CM KAT support
 *       yog  08/25/24 Integrated FIH library
 *       am   08/25/24 Initialized ASU DMA pointer before XAsufw_DmaXfr() function call.
 * 1.1   am   01/20/25 Added AES CCM support
 *       vns  02/12/25 Removed XAsufw_RCMW() API
 *       yog  02/26/25 Added XAes_Compute() API
 *       am   03/14/25 Updated doxygen comments
 *       am   04/01/25 Add key read-back verification for key integrity.
 *       yog  04/04/25 Added XAes_KeyClear() API
 *       am   04/10/25 Fixed incorrect AES base address usage in XAes_DecryptEfuseBlackKey()
 *       am   04/14/25 Added support for DMA non-blocking wait
 *       am   04/26/25 Fixes IV & AAD formatting logic for different Iv lengths
 * 1.2   am   05/18/25 Fixed implicit conversion of operands
 *       am   07/18/25 Modified XAes_SetReset() visibility from static to non-static
 *       kd   07/23/25 Fixed gcc warnings
 *       am   07/23/25 Replaced runtime AesCmConfig checks with compile-time macro
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xaes_server_apis AES Server APIs
* @{
*/
/***************************** Include Files *****************************************************/
#include "xaes.h"
#include "xasufw_status.h"
#include "xasufw_config.h"
#include "xasu_def.h"
#include "xasu_aes_common.h"
#include "xfih.h"

/************************** Constant Definitions *************************************************/

/************************** Macros Definitions ***************************************************/
#define XAES_TIMEOUT_MAX		(0x1FFFFU) /**<  AES maximum timeout value in micro seconds */
#define XAES_INVALID_CFG		(0xFFFFFFFFU) /**<  AES invalid configuration */
#define XAES_KEY_NOT_ZEROIZED		(0x0) /**< Key in AES subsystem is not zeroized */
#define XAES_KEY_ZEROIZED		(0xF) /**< Key in AES subsystem is zeroized */
#define XAES_FULL_WORD_MASK		(0xFFFFFFFFU) /**< Mask to compare a full 32-bit word. */
#define XAES_BIT_MASK(Bytes)		((1UL << ((Bytes) * XASUFW_BYTE_LEN_IN_BITS)) - 1U)
						/**< Bit mask with lower (Bytes * 8) bits set. */
#define XAES_WORD_COUNT_LEN(Length)	(((Length) + 3U) / 4U) /**< Number of 4-byte words for
								given length. */
/**
 * Bit 6 flag indicating the presence of Additional Authenticated Data (AAD).
 * This flag is set in the B0 block of AES-CCM mode when AAD is present.
 */
#define XAES_CCM_AAD_FLAG		(0x40U)

/**
 * Number of bits to shift for tag length encoding in the B0 flag.
 * Bits 5-3 in the B0 flag store the tag length as (t-2)/2.
 */
#define XAES_CCM_TAG_SHIFT      	(3U)

/**
 * Mask for extracting the 3-bit tag length encoding in the B0 flag.
 * The tag length is stored in bits 5-3 and extracted using this mask.
 */
#define XAES_CCM_TAG_MASK       	(0x07U)

/**
 * Constant used for computing q, where q = 15 - nonce_len.
 * q represents the number of bytes used to encode the message length.
 */
#define XAES_CCM_Q_CONST        	(15U)

/**
 * Mask to extract the lower 3 bits for q-1 encoding in the B0 flag.
 * The message length encoding q-1 is stored in bits 2-0.
 */
#define XAES_CCM_Q_MASK         	(0x07U)

/**
 * @brief Macro to compute the B0 flag for AES-CCM mode.
 *
 * The B0 flag structure:
 *
 * Table 1: Formatting of the Flags Octet in B0
 * | Bit Number |  7  |   6   | 5 | 4 | 3 | 2 | 1 | 0 |
 * |------------|-----|-------|-----------|-----------|
 * | Contents   |  0  | Adata |  (t-2)/2  |    q-1    |
 *
 *
 * @param	AadLen		Length of the Additional Authenticated Data(AAD)
 * @param	TagLen		Length of the authentication tag
 * @param	NonceLen	Length of the nonce
 *
 * @return
 *		Computed B0 flag as an 8-bit unsigned integer
 *
 */
#define XAES_B0FLAG(AadLen, TagLen, NonceLen) \
	((u8)(((((AadLen) > 0U) ? XAES_CCM_AAD_FLAG : 0x00U) | \
	((((TagLen - 2U) / 2U) & XAES_CCM_TAG_MASK) << XAES_CCM_TAG_SHIFT) | \
	((XAES_CCM_Q_CONST - NonceLen - 1U) & XAES_CCM_Q_MASK))))

#define XAES_AAD_LENGTH_SHORT_LIMIT	(0xFF00U) /**< (1 << 16) - (1 << 8) = 65280. */
#define XAES_HEADER_6BYTE_INDICATOR	(0xFFFEU) /**< Header 6-byte indicator. */
#define XAES_HEADER_10BYTE_INDICATOR	(0xFFFFU) /**< Header 10-byte indicator. */
#define XAES_TWO_BYTE_ENCODING		(2U) /**< 2-byte encoding for small values. */
#define XAES_SIX_BYTE_ENCODING		(6U) /**< 6-byte encoding for medium values. */
#define XAES_TEN_BYTE_ENCODING		(10U) /**< 10-byte encoding for large values. */
#define XAES_BYTE_MASK			(0xFFU) /**< Byte mask. */
#define XAES_MSB_SHIFT_32		(24U) /**< Most significant byte shift of 32-bit word. */

#define XAES_NONCE_HEADER_FLAG		(0x1U) /**< Nonce header flag size. */
#define XAES_MAX_PLEN_AAD_ENCODING_SIZE	(10U) /**< Maximum bytes needed to encode plaintext and
						   AAD length. */
#define	XAES_MAX_NONCE_HEADER_LEN	(XAES_NONCE_HEADER_FLAG + XASU_AES_CCM_MAX_NONCE_LEN + \
						XAES_MAX_PLEN_AAD_ENCODING_SIZE)
					/**< AES CCM maximum header length
					(0x1U (Flag) + XASU_AES_CCM_MAX_NONCE_LEN +
					XAES_MAX_PLEN_AAD_ENCODING_SIZE. */
#define XAES_U64_ONE			(1ULL) /**< Constant value 1 as an unsigned 64-bit integer. */
#define XAES_NONCE_HEADER_FIRST_IDX	(0U) /**< First index of the NonceHeader array, used to
					store the high byte. */
#define XAES_NONCE_HEADER_SECOND_IDX	(1U) /**< Second index of the NonceHeader array, used to
					store the low byte. */
#define XAES_CM_SPLIT_MASK		(0xFFFFFFFFU) /**< AES CM split mask. */
#define XAES_CM_SPLIT_ALIGNED_LENGTH	(0x10U) /**< AES CM split configuration aligned length. */
#define XAES_CM_MASK_BUF_WORD_LEN	(32U) /**< AES CM mask data buffer word length. */
#define XAES_CM_OUTPUT_ADDR_INDEX	(256U) /**< AES CM output data address index. */
#define XAES_CCM_FLAG_SIZE_IN_BYTES	(1U) /**< Size of the flag field prepended to IV in
						CCM mode (in bytes). */
#define XAES_GCM_J0_IV_INIT_VAl		(0x01U) /**< Initial counter value used in GCM mode IV formatting. */
#define XAES_TAG_LEN_IN_WORDS		(4U) /**< Number of 32-bit words in a 128-bit AES authentication tag. */

typedef enum {
	XAES_INITIALIZED = 0x1, /**< AES is in initialized state */
	XAES_STARTED, /**< AES is in start state */
	XAES_ONLY_AAD_UPDATE_IN_PROGRESS, /**< AES only AAD update is in progress during AAD update */
	XAES_AAD_UPDATE_IN_PROGRESS, /**< AES AAD update is in progress state during AAD updates */
	XAES_DATA_UPDATE_IN_PROGRESS, /**< AES update is in progress state during data chunk updates */
	XAES_UPDATE_COMPLETED, /**< AES update is in completed state after the final data chunk */
} XAes_State;
/** @} */
/**************************** Type Definitions ***************************************************/
/** This structure defines look up table for AES key. */
typedef struct {
	u32 RegOffset; /**< Register offset for key source */
	u32 KeySrcSelVal; /**< Selection value for key source */
	u32 KeyClearVal; /**< Key source clear value */
	u32 KeyZeroedStatusMask; /**< Key zeroed status mask */
} XAes_KeyLookup;

/**
 * This structure contains configuration information for AES core.
 * Each core should have an associated configuration structure.
 */
struct _XAes_Config {
	u16 DeviceId; /**< Unique ID of the device */
	u32 AesBaseAddress; /**< Base address of ASU_AES module */
	u32 KeyBaseAddress; /**< Base address of ASU_KEY module */
};

/**
 * AES driver instance data structure. A pointer to an instance data
 * structure is passed around by functions to refer to a specific driver
 * instance.
 */
struct _XAes {
	u32 AesBaseAddress;	/**< AES Base address */
	u32 KeyBaseAddress;	/**< Key Vault Base address */
	u16 DeviceId;		/**< DeviceId is the unique ID of the device */
	u16 Reserved1;		/**< Reserved for alignment */
	XAsufw_Dma *AsuDmaPtr;	/**< ASU DMA instance pointer */
	XAes_State AesState;	/**< AES internal state machine */
	u8 OperationType;	/**< AES operation type (Encryption/Decryption) */
	u8 EngineMode;		/**< Aes Engine mode */
	u8 CcmAadZeroBlockPadLen; /**< Number of zero bytes needed to pad AAD to AES block length in CCM. */
	u8 Reserved2;		/**< Reserved for alignment */
};

/**
* @addtogroup xaes_server_apis AES Server APIs
* @{
*/
/************************** Variable Definitions *************************************************/
/**
 * @brief AES Key Lookup Table
 *
 * This table contains details about different AES key sources, including their configuration,
 * selection values, write permissions, and zeroed status masks.
 *
 */
 static const XAes_KeyLookup AesKeyLookupTbl [XASU_AES_MAX_KEY_SOURCES] = {
	/** XASU_AES_USER_KEY_0 */
	{
		XAES_USER_KEY_0_0_OFFSET,       /**< Register offset of USER KEY0 */
		XAES_KEY_SEL_USER_KEY_0_VALUE,  /**< USER KEY0 selection value */
		XAES_KEY_CLEAR_USER_KEY_0_MASK, /**< Mask to clear the USER KEY0 */
		XAES_KEY_ZEROED_STATUS_USER_KEY_0_MASK /**< Mask to check if USER KEY0 is zeroed */
	},

	/** XASU_AES_USER_KEY_1 */
	{
		XAES_USER_KEY_1_0_OFFSET,       /**< Register offset of USER KEY1 */
		XAES_KEY_SEL_USER_KEY_1_VALUE,  /**< USER KEY1 selection value */
		XAES_KEY_CLEAR_USER_KEY_1_MASK, /**< Mask to clear the USER KEY1 */
		XAES_KEY_ZEROED_STATUS_USER_KEY_1_MASK /**< Mask to check if USER KEY1 is zeroed */
	},

	/** XASU_AES_USER_KEY_2 */
	{
		XAES_USER_KEY_2_0_OFFSET,       /**< Register offset of USER KEY2 */
		XAES_KEY_SEL_USER_KEY_2_VALUE,  /**< USER KEY2 selection value */
		XAES_KEY_CLEAR_USER_KEY_2_MASK, /**< Mask to clear the USER KEY2 */
		XAES_KEY_ZEROED_STATUS_USER_KEY_2_MASK /**< Mask to check if USER KEY2 is zeroed */
	},

	/** XASU_AES_USER_KEY_3 */
	{
		XAES_USER_KEY_3_0_OFFSET,       /**< Register offset of USER KEY3 */
		XAES_KEY_SEL_USER_KEY_3_VALUE,  /**< USER KEY3 selection value */
		XAES_KEY_CLEAR_USER_KEY_3_MASK, /**< Mask to clear the USER KEY3 */
		XAES_KEY_ZEROED_STATUS_USER_KEY_3_MASK /**< Mask to check if USER KEY3 is zeroed */
	},

	/** XASU_AES_USER_KEY_4 */
	{
		XAES_USER_KEY_4_0_OFFSET,       /**< Register offset of USER KEY4 */
		XAES_KEY_SEL_USER_KEY_4_VALUE,  /**< USER KEY4 selection value */
		XAES_KEY_CLEAR_USER_KEY_4_MASK, /**< Mask to clear the USER KEY4 */
		XAES_KEY_ZEROED_STATUS_USER_KEY_4_MASK /**< Mask to check if USER KEY4 is zeroed */
	},

	/** XASU_AES_USER_KEY_5 */
	{
		XAES_USER_KEY_5_0_OFFSET,       /**< Register offset of USER KEY5 */
		XAES_KEY_SEL_USER_KEY_5_VALUE,  /**< USER KEY5 selection value */
		XAES_KEY_CLEAR_USER_KEY_5_MASK, /**< Mask to clear the USER KEY5 */
		XAES_KEY_ZEROED_STATUS_USER_KEY_5_MASK /**< Mask to check if USER KEY5 is zeroed */
	},

	/** XASU_AES_USER_KEY_6 */
	{
		XAES_USER_KEY_6_0_OFFSET,       /**< Register offset of USER KEY6 */
		XAES_KEY_SEL_USER_KEY_6_VALUE,  /**< USER KEY6 selection value */
		XAES_KEY_CLEAR_USER_KEY_6_MASK, /**< Mask to clear the USER KEY6 */
		XAES_KEY_ZEROED_STATUS_USER_KEY_6_MASK /**< Mask to check if USER KEY6 is zeroed */
	},

	/** XASU_AES_USER_KEY_7 */
	{
		XAES_USER_KEY_7_0_OFFSET,       /**< Register offset of USER KEY7 */
		XAES_KEY_SEL_USER_KEY_7_VALUE,  /**< USER KEY7 selection value */
		XAES_KEY_CLEAR_USER_KEY_7_MASK, /**< Mask to clear the USER KEY7 */
		XAES_KEY_ZEROED_STATUS_USER_KEY_7_MASK /**< Mask to check if USER KEY7 is zeroed */
	},

	/** XASU_AES_EFUSE_KEY_0 */
	{
		XAES_INVALID_CFG,               /**< Invalid register offset of EFUSE KEY0 */
		XASU_FALSE,                     /**< Invalid EFUSE KEY0 selection value */
		XAES_KEY_CLEAR_EFUSE_KEY_0_MASK, /**< Mask to clear the EFUSE KEY0 */
		XAES_KEY_ZEROED_STATUS_EFUSE_KEY_0_MASK /**< Mask to check if EFUSE KEY0 is zeroed */
	},

	/** XASU_AES_EFUSE_KEY_1 */
	{
		XAES_INVALID_CFG,               /**< Invalid register offset of EFUSE KEY1 */
		XASU_FALSE,                     /**< Invalid EFUSE KEY1 selection value */
		XAES_KEY_CLEAR_EFUSE_KEY_1_MASK, /**< Mask to clear the EFUSE KEY1 */
		XAES_KEY_ZEROED_STATUS_EFUSE_KEY_1_MASK /**< Mask to check if EFUSE KEY1 is zeroed */
	},

	/** XASU_AES_PUF_KEY */
	{
		XAES_INVALID_CFG,               /**< Invalid register offset of PUF KEY */
		XAES_KEY_SEL_PUF_KEY_VALUE,     /**< PUF KEY selection value */
		XAES_KEY_CLEAR_PUF_KEY_MASK,    /**< Mask to clear the PUF KEY */
		XAES_KEY_ZEROED_STATUS_PUF_KEY_MASK /**< Mask to check if PUF KEY is zeroed */
	},

	/** XASU_AES_EFUSE_KEY_RED_0 */
	{
		XAES_INVALID_CFG,               /**< Invalid register offset of EFUSE KEY RED0 */
		XAES_KEY_SEL_EFUSE_KEY_RED_0_VALUE, /**< EFUSE KEY RED0 selection value */
		XAES_KEY_CLEAR_EFUSE_KEY_RED_0_MASK, /**< Mask to clear the EFUSE KEY RED0 */
		XAES_KEY_ZEROED_STATUS_EFUSE_RED_KEY_0_MASK /**< Mask to check if EFUSE KEY RED0 is zeroed */
	},

	/** XASU_AES_EFUSE_KEY_RED_1 */
	{
		XAES_INVALID_CFG,               /**< Invalid register offset of EFUSE KEY RED1 */
		XAES_KEY_SEL_EFUSE_KEY_RED_1_VALUE, /**< EFUSE KEY RED1 selection value */
		XAES_KEY_CLEAR_EFUSE_KEY_RED_1_MASK, /**< Mask to clear the EFUSE KEY RED1 */
		XAES_KEY_ZEROED_STATUS_EFUSE_RED_KEY_1_MASK /**< Mask to check if EFUSE KEY RED1 is zeroed */
	},
};

/** AES configuration table for AES crypto devices */
static XAes_Config AesConfigTable[XASU_XAES_NUM_INSTANCES] = {
	{
		XASU_XAES_0_DEVICE_ID,
		XASU_XAES_0_BASEADDR,
		XASU_XKEY_0_BASEADDR
	}
};

static XAes XAes_Instance[XASU_XAES_NUM_INSTANCES]; /**< ASUFW AES HW instances */

#if XASUFW_ENABLE_PERF_MEASUREMENT
static u64 StartTime; /**< Performance measurement start time. */
static XAsufw_PerfTime PerfTime; /**< Structure holding performance timing results. */
#endif

static XAes_ContextInfo AesContext; /**< AES context. */

/************************** Inline Function Definitions ******************************************/

/************************** Function Prototypes **************************************************/
static XAes_Config *XAes_LookupConfig(u16 DeviceId);
static s32 XAes_IsKeyZeroized(const XAes *InstancePtr, u32 KeySrc);
static inline void XAes_ConfigCounterMeasures(const XAes *InstancePtr);
static void XAes_ConfigAesOperation(const XAes *InstancePtr);
static void XAes_LoadKey(const XAes *InstancePtr, u32 KeySrc, u32 KeySize);
static s32 XAes_ValidateKeyConfig(const XAes *InstancePtr, u32 KeySrc, u32 KeySize);
static s32 XAes_ProcessAndLoadIv(XAes *InstancePtr, u64 IvAddr, u32 IvLen);
static s32 XAes_GHashCal(const XAes *InstancePtr, u64 IvAddr, u32 IvGen, u32 IvLen);
static s32 XAes_ReadTag(const XAes *InstancePtr, u32 TagOutAddr);
static s32 XAes_ReadNVerifyTag(const XAes *InstancePtr, u32 TagInAddr, u32 TagLen);
static s32 XAes_ProcessTag(const XAes *InstancePtr, u64 TagAddr, u32 TagLen);
static void XAes_ConfigAad(const XAes *InstancePtr);
static void XAes_ClearConfigAad(const XAes *InstancePtr);
static s32 XAes_CfgDmaWithAesAndXfer(const XAes *InstancePtr, u64 InDataAddr, u64 OutDataAddr,
	u32 Size, u8 IsLastChunk);
static s32 XAes_DummyEncryption(XAes *InstancePtr);
static s32 XAes_FinalizeAadUpdate(XAes *InstancePtr);
static s32 XAes_CheckAndRestoreUserKeyContext(const XAes *InstancePtr);
static s32 XAes_WaitForDone(const XAes *InstancePtr);
static s32 XAes_WaitForReady(const XAes *InstancePtr);

/*************************************************************************************************/
/**
 * @brief	This function returns an AES instance pointer of the provided device ID.
 *
 * @param	DeviceId	DeviceID of the AES core.
 *
 * @return
 * 		- XAes_Instance corresponding to the Device ID.
 * 		- NULL, if the device ID is invalid.
 *
 *************************************************************************************************/
XAes *XAes_GetInstance(u16 DeviceId)
{
	XAes *XAes_InstancePtr = NULL;

	if (DeviceId >= XASU_XAES_NUM_INSTANCES) {
		goto END;
	}

	XAes_InstancePtr = &XAes_Instance[DeviceId];
	XAes_InstancePtr->DeviceId = DeviceId;

END:
	return XAes_InstancePtr;
}

/*************************************************************************************************/
/**
 * @brief	This function initializes the AES instance.
 *
 * @param	InstancePtr 	Pointer to the XAes instance.
 *
 * @return
 *		- XASUFW_SUCCESS, if initialization is successful.
 *		- XASUFW_AES_INVALID_PARAM, if InstancePtr or CfgPtr is NULL.
 *
 *************************************************************************************************/
s32 XAes_CfgInitialize(XAes *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XAes_Config *CfgPtr;

	/** Validate input parameters. */
	if (InstancePtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	CfgPtr = XAes_LookupConfig(InstancePtr->DeviceId);
	if (CfgPtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	/** Initialize AES instance. */
	InstancePtr->AesBaseAddress = CfgPtr->AesBaseAddress;
	InstancePtr->KeyBaseAddress = CfgPtr->KeyBaseAddress;

	InstancePtr->AesState = XAES_INITIALIZED;

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function gets the AES context instance.
 *
 * @return
 * 		- Returns pointer to XAes_ContextInfo.
 *
 *************************************************************************************************/
XAes_ContextInfo *XAes_GetAesContext(void)
{
	return &AesContext;
}

/*************************************************************************************************/
/**
 * @brief	This function writes the provided user key into specified AES USER key registers.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	KeyObjectAddr	Address of AES key object structure, which contains
 *				- KeyAddress is address of user key.
 *				- KeySize is size of the user key.
 *			  	  - XASUFW_AES_KEY_SIZE_128 for 128-bit key size
 *				  - XASUFW_AES_KEY_SIZE_256 for 256-bit key size
 *				- KeySrc which indicates which hardware user key to be used to
 * 				  store the user key.
 *			  	  - XASU_AES_USER_KEY_0
 *				  - XASU_AES_USER_KEY_1
 *				  - XASU_AES_USER_KEY_2
 *				  - XASU_AES_USER_KEY_3
 *				  - XASU_AES_USER_KEY_4
 *				  - XASU_AES_USER_KEY_5
 *				  - XASU_AES_USER_KEY_6
 *				  - XASU_AES_USER_KEY_7
 *
 * @return
 *		- XASUFW_SUCCESS, if key write is successful.
 *		- XASUFW_AES_INVALID_PARAM, if InstancePtr or DmaPtr is NULL or ASU DMA is not ready.
 *		- XASUFW_AES_INVALID_KEY_OBJECT_ADDRESS, if KeyObjectAddr is invalid.
 *		- XASUFW_AES_INVALID_KEY_SRC, if key source is invalid.
 *		- XASUFW_AES_INVALID_KEY_SIZE, if key size is invalid.
 *
 *************************************************************************************************/
/* TODO: In future, this API can be moved/modified after Key vault implementation. */
s32 XAes_WriteKey(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 KeyObjectAddr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 ClearStatus = XASUFW_FAILURE;
	XFih_Var XFihKeyClear;
	u32 Key[XASU_AES_KEY_SIZE_256BIT_IN_WORDS];
	u32 Offset;
	u32 KeySizeInWords = 0U;
	XAsu_AesKeyObject KeyObject;

	/** Validate the input arguments. */
	if ((InstancePtr == NULL) || (DmaPtr == NULL)) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if (DmaPtr->AsuDma.IsReady != XIL_COMPONENT_IS_READY) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->AesState != XAES_INITIALIZED) {
		Status = XASUFW_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	if (KeyObjectAddr == 0U) {
		Status = XASUFW_AES_INVALID_KEY_OBJECT_ADDRESS;
		goto END;
	}

	/** Initialize the AES instance with ASU DMA pointer. */
	InstancePtr->AsuDmaPtr = DmaPtr;

	/** Copy KeyObject structure from 64-bit address space to local structure using ASU DMA. */
	Status = XAsufw_DmaXfr(InstancePtr->AsuDmaPtr, KeyObjectAddr,
		(u64)(UINTPTR)&KeyObject, sizeof(XAsu_AesKeyObject), 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (KeyObject.KeyAddress == 0U) {
		Status = XASUFW_AES_INVALID_KEY_ADDRESS;
		goto END_CLR;
	}

	if (KeyObject.KeySrc > XASU_AES_USER_KEY_7) {
		Status = XASUFW_AES_INVALID_KEY_SRC;
		goto END_CLR;
	}

	if (KeyObject.KeySize == XASU_AES_KEY_SIZE_128_BITS) {
		KeySizeInWords = XASU_AES_KEY_SIZE_128BIT_IN_WORDS;
	}
	else if (KeyObject.KeySize == XASU_AES_KEY_SIZE_256_BITS) {
		KeySizeInWords = XASU_AES_KEY_SIZE_256BIT_IN_WORDS;
	}
	else {
		Status = XASUFW_AES_INVALID_KEY_SIZE;
		goto END_CLR;
	}

	Offset = AesKeyLookupTbl[KeyObject.KeySrc].RegOffset;
	if (Offset == XAES_INVALID_CFG) {
		Status = XASUFW_AES_INVALID_KEY_SRC;
		goto END_CLR;
	}

	/** Copy Key from 64-bit address space to local array using ASU DMA. */
	Status = XAsufw_DmaXfr(InstancePtr->AsuDmaPtr,  (u64)KeyObject.KeyAddress,
			(u64)(UINTPTR)Key, (KeySizeInWords * XASUFW_WORD_LEN_IN_BYTES), 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/**
	 * Traverse to Offset of last word of respective USER key register
	 * (i.e., XAES_USER_KEY_X_7), where X = 0 to 7 USER key.
	 * Write the key to the respective key source registers by changing the endianness.
	 */
	Offset = Offset + (KeySizeInWords * XASUFW_WORD_LEN_IN_BYTES) - XASUFW_WORD_LEN_IN_BYTES;

	/** Write user key to the respective user key registers by changing the endianness. */
	Status = XAsufw_WriteDataToRegsWithEndianSwap(InstancePtr->KeyBaseAddress, Offset, Key, KeySizeInWords);

	/** Save the key information for context switching. */
	if (AesContext.IsContextSaved != XASU_TRUE) {
		/**
		 * Copy KeyObject structure from 64-bit address space to AES context
		 * structure using ASU DMA.
		 */
		Status = XAsufw_DmaXfr(InstancePtr->AsuDmaPtr, KeyObjectAddr,
			(u64)(UINTPTR)&AesContext.KeyObject, sizeof(XAsu_AesKeyObject), 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}

		/** Copy Key from 64-bit address space to AES context structure using ASU DMA. */
		Status = XAsufw_DmaXfr(InstancePtr->AsuDmaPtr,  (u64)KeyObject.KeyAddress,
			(u64)(UINTPTR)AesContext.Key, (KeySizeInWords * XASUFW_WORD_LEN_IN_BYTES), 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}
	}

END_CLR:
	/** Clear local key object structure. */
	XFIH_CALL(Xil_SecureZeroize, XFihKeyClear, ClearStatus, (u8 *)(UINTPTR)&KeyObject,
		sizeof(XAsu_AesKeyObject));

	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	/** Clear local key array. */
	XFIH_CALL(Xil_SecureZeroize, XFihKeyClear, ClearStatus, (u8 *)Key,
		XASU_AES_KEY_SIZE_256BIT_IN_BYTES);

	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function initializes the AES engine for encrypt/decrypt operation and loads
 * 		the provided key and IV to the AES engine.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	KeyObjectAddr	Address of Aes key object structure, which contains
 *				- KeyAddress is address of user key.
 *				- KeySize is size of the user key.
 *			  	  - XASUFW_AES_KEY_SIZE_128 for 128-bit key size
 *				  - XASUFW_AES_KEY_SIZE_256 for 256-bit key size
 *				- KeySrc which indicates which hardware user key to be used to
 * 				  store the user key.
 *			  	  - XASU_AES_USER_KEY_0
 *				  - XASU_AES_USER_KEY_1
 *				  - XASU_AES_USER_KEY_2
 *				  - XASU_AES_USER_KEY_3
 *				  - XASU_AES_USER_KEY_4
 *				  - XASU_AES_USER_KEY_5
 *				  - XASU_AES_USER_KEY_6
 *				  - XASU_AES_USER_KEY_7
 * @param	IvAddr		Address of the buffer holding IV.
 * @param	IvLen		Length of the IV in bytes.
 * @param	EngineMode	AES engine mode.
 * @param	OperationType	AES operation type.
 *
 * @return
 *		- XASUFW_SUCCESS, if initialization of AES engine is successful.
 *		- XASUFW_AES_INVALID_PARAM, if InstancePtr or DmaPtr is NULL or ASU DMA is not
 * 			ready.
 *		- XASUFW_AES_STATE_MISMATCH_ERROR, if AES state is invalid.
 *		- XASUFW_AES_INVALID_KEY_OBJECT_ADDRESS, if KeyObjectAddr is invalid.
 *		- XASUFW_AES_INVALID_KEY_SRC, if key source is invalid.
 *		- XASUFW_AES_INVALID_KEY_SIZE, if key size is invalid.
 *		- XASUFW_AES_INVALID_ENGINE_MODE, if AES engine mode is invalid.
 *		- XASUFW_AES_INVALID_OPERATION_TYPE, if AES operation type is invalid.
 *
 *************************************************************************************************/
s32 XAes_Init(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 KeyObjectAddr, u64 IvAddr, u32 IvLen,
	u8 EngineMode, u8 OperationType)
{
	/**
	 * Capture the start time of the AES initialization operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START();

	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 ClearStatus = XASUFW_FAILURE;
	XFih_Var XFihKeyClear;
	XAsu_AesKeyObject KeyObject;

	/** Validate the input arguments.*/
	if (InstancePtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto RET;
	}

	if ((DmaPtr == NULL) || (DmaPtr->AsuDma.IsReady != XIL_COMPONENT_IS_READY)) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->AesState != XAES_INITIALIZED) {
		Status = XASUFW_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	if (KeyObjectAddr == 0U) {
		Status = XASUFW_AES_INVALID_KEY_OBJECT_ADDRESS;
		goto END;
	}

	/** Initialize the AES instance with ASU DMA pointer. */
	InstancePtr->AsuDmaPtr = DmaPtr;

	/** Copy KeyObject structure from 64-bit address space to local structure using ASU DMA. */
	Status = XAsufw_DmaXfr(InstancePtr->AsuDmaPtr, KeyObjectAddr,
		(u64)(UINTPTR)&KeyObject, sizeof(XAsu_AesKeyObject), 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (KeyObject.KeySrc >= XASU_AES_MAX_KEY_SOURCES) {
		Status = XASUFW_AES_INVALID_KEY_SRC;
		goto END;
	}

	if ((KeyObject.KeySize != XASU_AES_KEY_SIZE_128_BITS) &&
			(KeyObject.KeySize != XASU_AES_KEY_SIZE_256_BITS)) {
		Status = XASUFW_AES_INVALID_KEY_SIZE;
		goto END;
	}

	if ((EngineMode > XASU_AES_GCM_MODE) && (EngineMode != XASU_AES_CMAC_MODE) &&
			(EngineMode != XASU_AES_GHASH_MODE)) {
		Status = XASUFW_AES_INVALID_ENGINE_MODE;
		goto END;
	}

	if ((OperationType != XASU_AES_ENCRYPT_OPERATION) &&
			(OperationType != XASU_AES_DECRYPT_OPERATION)) {
		Status = XASUFW_AES_INVALID_OPERATION_TYPE;
		goto END;
	}

	/** Check whether key is zeroed or not and return failure if the key is zeroed. */
	Status = XAes_IsKeyZeroized(InstancePtr, KeyObject.KeySrc);
	if (Status != XAES_KEY_NOT_ZEROIZED) {
		Status = XASUFW_AES_KEY_ZEROED;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Validate the IV with respect to the user provided engine mode. */
	Status = XAsu_AesValidateIvParams(EngineMode, IvAddr, IvLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_INVALID_IV;
		goto END;
	}

	/** Initialize the AES instance with engine mode and operation type. */
	InstancePtr->EngineMode = EngineMode;
	InstancePtr->OperationType = OperationType;

	/** Release reset of AES engine. */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->AesBaseAddress, XAES_SOFT_RST_OFFSET);

	/** Load key to AES engine. */
	XAes_LoadKey(InstancePtr, KeyObject.KeySrc, KeyObject.KeySize);

	/** Configure AES DPA counter measures. */
	XAes_ConfigCounterMeasures(InstancePtr);

	/** Configure AES engine to encrypt/decrypt operation. */
	XAes_ConfigAesOperation(InstancePtr);

	/** Verify the key source and size by reading them back from the AES key vault registers. */
	Status = XAes_ValidateKeyConfig(InstancePtr, KeyObject.KeySrc, KeyObject.KeySize);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_KEY_CONFIG_READBACK_ERROR;
		goto END;
	}

	/** Perform dummy encryption only for CBC and ECB mode during decryption operation. */
	if (((InstancePtr->EngineMode == XASU_AES_CBC_MODE) ||
			(InstancePtr->EngineMode == XASU_AES_ECB_MODE)) &&
			(InstancePtr->OperationType == XASU_AES_DECRYPT_OPERATION)) {
		/** Set the AES state to valid state before dummy encryption. */
		InstancePtr->AesState = XAES_STARTED;

		Status = XAes_DummyEncryption(InstancePtr);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_AES_ECB_CBC_DUMMY_ENCRYPTION_FAILED;
			goto END;
		}

		/** Restore correct AES state. */
		InstancePtr->AesState = XAES_INITIALIZED;
	}

	/** Process and load IV to AES engine. */
	if (InstancePtr->EngineMode != XASU_AES_ECB_MODE) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAes_ProcessAndLoadIv(InstancePtr, IvAddr, IvLen);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	/** Update AES state machine to XAES_STARTED. */
	InstancePtr->AesState = XAES_STARTED;

END:
	/** Clear local key object buffer. */
	XFIH_CALL(Xil_SecureZeroize, XFihKeyClear, ClearStatus, (u8 *)(UINTPTR)&KeyObject,
		sizeof(XAsu_AesKeyObject));
	if (Status != XASUFW_SUCCESS) {
		/** Set AES under reset upon any failure. */
		XAes_SetReset(InstancePtr);
		/** Clear the XASU_AES_EXPANDED_KEYS. */
		Status = XAsufw_UpdateErrorStatus(Status,
			XAes_KeyClear(InstancePtr, XASU_AES_EXPANDED_KEYS));
	}

	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function updates AES engine with the provided input data and stores the
 * 		resultant data at specified output address.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	InDataAddr	Address of the input data for encrypt/decrypt operation.
 * @param	OutDataAddr	Address of output buffer where the encrypted/decrypted data to be updated.
 * 						If input data is AAD then, OutDataAddr will be zero.
 * @param	DataLength	Length of the data in bytes shall never be zero.
 * 				For AES-CBC and AES-ECB modes, length shall be aligned to 16 bytes.
 * @param	IsLastChunk	For the last update of data, this parameter should be set
 *		 	 	to TRUE. Otherwise, FALSE.
 *
 * @return
 *		- XASUFW_SUCCESS, if data update is successful.
 *		- XASUFW_AES_INVALID_PARAM, if InstancePtr or DmaPtr is NULL or ASU DMA is not ready.
 *		- XASUFW_AES_STATE_MISMATCH_ERROR, if AES state is invalid.
 *		- XASUFW_AES_INVALID_INPUT_DATA, if input data address is invalid.
 *		- XASUFW_AES_INVALID_INPUT_DATA_LENGTH, if input data length is invalid.
 *		- XASUFW_AES_INVALID_ISLAST_CHUNK, if IsLastChunk is invalid.
 *		- XASUFW_AES_UNALIGNED_BLOCK_SIZE_INPUT_LENGTH, if input data length is not 16 bytes
 * 			aligned for CBC and ECB mode.
 *
 *************************************************************************************************/
s32 XAes_Update(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 InDataAddr, u64 OutDataAddr,
	u32 DataLength, u8 IsLastChunk)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate the input arguments. */
	if (InstancePtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto RET;
	}

	if ((DmaPtr == NULL) || (DmaPtr->AsuDma.IsReady != XIL_COMPONENT_IS_READY)) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if ((InstancePtr->AesState != XAES_STARTED) &&
			(InstancePtr->AesState != XAES_AAD_UPDATE_IN_PROGRESS) &&
			(InstancePtr->AesState != XAES_DATA_UPDATE_IN_PROGRESS)) {
		Status = XASUFW_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	if (InDataAddr == 0U) {
		Status = XASUFW_AES_INVALID_INPUT_DATA;
		goto END;
	}

	/**
	 * The minimum length of plaintext/AAD data must be at least 1 byte, while the
	 * maximum length should be less than 0x1FFFFFFC bytes, which is the
	 * ASU DMA's maximum supported data transfer length.
	 */
	if ((DataLength == 0U) || (DataLength > XASU_ASU_DMA_MAX_TRANSFER_LENGTH)) {
		Status = XASUFW_AES_INVALID_INPUT_DATA_LENGTH;
		goto END;
	}

	if ((IsLastChunk != XASU_TRUE) && (IsLastChunk != XASU_FALSE)) {
		Status = XASUFW_AES_INVALID_ISLAST_CHUNK;
		goto END;
	}

	/**
	 * For the ECB, CBC, and CFB modes, the plaintext must be a sequence of one or more
	 * complete data blocks.
	 */
	if (((InstancePtr->EngineMode == XASU_AES_ECB_MODE) ||
			(InstancePtr->EngineMode == XASU_AES_CBC_MODE) ||
			(InstancePtr->EngineMode == XASU_AES_CFB_MODE)) &&
			((DataLength % XASU_AES_BLOCK_SIZE_IN_BYTES) != 0U)) {
		Status = XASUFW_AES_UNALIGNED_BLOCK_SIZE_INPUT_LENGTH;
		goto END;
	}

	/**
	 * During initial payload update, finalize the AAD update phase by optionally sending
	 * zero padding (for CCM mode) and clearing the AAD configuration for all modes.
	 */
	if (InstancePtr->AesState == XAES_AAD_UPDATE_IN_PROGRESS) {
		Status = XAes_FinalizeAadUpdate(InstancePtr);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	/** Initialize the AES instance with ASU DMA pointer. */
	InstancePtr->AsuDmaPtr = DmaPtr;

	/**
	 * If the output data address is zero, the input data will be considered as AAD data.
	 * Configure AES AAD configurations before pushing AAD data to AES engine and clear AAD
	 * configuration post DMA transfer.
	 */
	if (OutDataAddr == XAES_AAD_UPDATE_NO_OUTPUT_ADDR) {
		InstancePtr->AesState = (IsLastChunk == XASU_TRUE) ? XAES_ONLY_AAD_UPDATE_IN_PROGRESS :
			XAES_AAD_UPDATE_IN_PROGRESS;
		XAes_ConfigAad(InstancePtr);
	}
	else {
		/**
		 * Update the AES state machine to XAES_DATA_UPDATE_IN_PROGRESS or XAES_UPDATE_COMPLETED based
		 * on the IsLastChunk flag set by the user during update of data.
		 */
		InstancePtr->AesState = (IsLastChunk == XASU_TRUE) ? XAES_UPDATE_COMPLETED :
			XAES_DATA_UPDATE_IN_PROGRESS;
	}

	/** Configure DMA with AES and transfer the data to AES engine. */
	Status = XAes_CfgDmaWithAesAndXfer(InstancePtr, InDataAddr, OutDataAddr, DataLength,
		IsLastChunk);

END:
	/**
	 * Reset AES engine if InstancePtr is valid and Status is not SUCCESS or
	 * XASUFW_CMD_IN_PROGRESS.
	 */
	if ((Status != XASUFW_SUCCESS) && (Status != XASUFW_CMD_IN_PROGRESS)) {
		/** Set AES under reset upon any failure. */
		XAes_SetReset(InstancePtr);
		/** Clear the XASU_AES_EXPANDED_KEYS. */
		Status = XAsufw_UpdateErrorStatus(Status,
			XAes_KeyClear(InstancePtr, XASU_AES_EXPANDED_KEYS));
	}

RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function waits for AES operation to complete. For MAC modes, it
 * 		generates/verifies the tag based on the AES operation type.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	TagAddr		Address of the tag buffer.
 * @param	TagLen		Length of tag in bytes and it will be zero for all AES
 *				standard modes like, ECB, CBC, OFB, CFB, CTR
 *
 * @return
 *		- XASUFW_SUCCESS, if completion of final AES operation is successful.
 *		- XASUFW_AES_INVALID_PARAM, if InstancePtr or DmaPtr is NULL or ASU DMA is not ready.
 *		- XASUFW_AES_STATE_MISMATCH_ERROR, if AES state is invalid.
 *
 *************************************************************************************************/
s32 XAes_Final(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 TagAddr, u32 TagLen)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate the input arguments. */
	if (InstancePtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto RET;
	}

	if ((DmaPtr == NULL) || (DmaPtr->AsuDma.IsReady != XIL_COMPONENT_IS_READY)) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if ((InstancePtr->AesState != XAES_UPDATE_COMPLETED) &&
			(InstancePtr->AesState != XAES_ONLY_AAD_UPDATE_IN_PROGRESS)) {
		Status = XASUFW_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** Validate the tag with respect to the user provided engine mode. */
	Status = XAsu_AesValidateTagParams(InstancePtr->EngineMode, TagAddr, TagLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * During only AAD update (no payload) after non-blocking DMA wait, finalize the AAD update
	 * phase by optionally sending zero padding (for CCM mode) and clearing the AAD
	 * configuration for all modes.
	 */
	if (InstancePtr->AesState == XAES_ONLY_AAD_UPDATE_IN_PROGRESS) {
		Status = XAes_FinalizeAadUpdate(InstancePtr);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	/** Initialize the AES instance with ASU DMA pointer. */
	InstancePtr->AsuDmaPtr = DmaPtr;

	if (InstancePtr->AesState == XAES_UPDATE_COMPLETED) {
		/** Wait for AES operation to complete. */
		Status = XAes_WaitForDone(InstancePtr);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	/** Wait for AES engine to be in idle and ready state. */
	Status = XAes_WaitForReady(InstancePtr);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/**
	 * For AES MAC modes,
	 * - For encryption, read and store the generated tag at specified TagAddr.
	 * - For decryption, compare the generated tag with the provided tag.
	 */
	if ((InstancePtr->EngineMode == XASU_AES_CCM_MODE) ||
			(InstancePtr->EngineMode == XASU_AES_GCM_MODE) ||
			(InstancePtr->EngineMode == XASU_AES_CMAC_MODE)) {
		Status = XAes_ProcessTag(InstancePtr, TagAddr, TagLen);
	}

	/**
	 * Measure and print the performance time for the AES finalization operation, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(__func__);

END:
	/** Set AES under reset after AES operation is complete. */
	XAes_SetReset(InstancePtr);
	/** Clear the XASU_AES_EXPANDED_KEYS. */
	Status = XAsufw_UpdateErrorStatus(Status,
		XAes_KeyClear(InstancePtr, XASU_AES_EXPANDED_KEYS));

RET:
	if (AesContext.IsContextSaved == XASU_TRUE) {
		/** Set restore context flag to true. */
		AesContext.IsContextRestoreReq = XASU_TRUE;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function formats the Additional Authenticated Data (AAD), Nonce, and associated
 * 		parameters into a structure compliant with AES-CCM input formatting requirements.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	AadAddr		Address of the AAD data.
 * @param	AadLen		Length of the AAD in bytes.
 * @param	NonceAddr	Address of the nonce.
 * @param	NonceLen	Length of the nonce in bytes.
 * @param	PlainTextLen	Length of the plaintext (payload) in bytes.
 * @param	TagLen		Length of the authentication tag in bytes.
 *
 * @return
 *		- XASUFW_SUCCESS, if formatting of AAD data is successful.
 *		- XASUFW_AES_INVALID_PARAM, if InstancePtr or DmaPtr is NULL or AAD and Nonce Address
 * 			and lengths are zero.
 *		- XASUFW_FAILURE, if transfer of data is failed.
 *
 *************************************************************************************************/
s32 XAes_CcmFormatAadAndXfer(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 AadAddr, u32 AadLen,
	u64 NonceAddr, u8 NonceLen, u32 PlainTextLen, u8 TagLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 NonceHeader[XASU_AES_BLOCK_SIZE_IN_BYTES];
	volatile u8 Index = 0U;
	u8 TotalAadLen;
	u8 LengthFieldSize;
	u64 PtLenCpy = PlainTextLen;
	u8 IsLast;
	u8 NonceHeaderIndex;

	/** Validate the input arguments. */
	if (InstancePtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto RET;
	}

	if ((DmaPtr == NULL) || (NonceAddr == 0U)) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if ((AadLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH) ||
			(PlainTextLen > XASU_ASU_DMA_MAX_TRANSFER_LENGTH)) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->AesState != XAES_STARTED) {
		Status = XASUFW_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** Validate the IV with respect to the AES-CCM engine mode. */
	Status = XAsu_AesValidateIvParams(XASU_AES_CCM_MODE, NonceAddr, NonceLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	/** Validate the tag with respect to the AES-CCM engine mode. */
	if (((TagLen % XASU_AES_EVEN_MODULUS) != 0U) ||
			((TagLen < XASU_AES_MIN_TAG_LENGTH_IN_BYTES) ||
			(TagLen > XASU_AES_MAX_TAG_LENGTH_IN_BYTES))) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	/** Number of bytes for payload length encoding. */
	LengthFieldSize = XAES_CCM_Q_CONST - NonceLen;

	/** Check if plaintext length fits in LengthFieldSize bytes. */
	if (PlainTextLen >= (XAES_U64_ONE << (XASUFW_BYTE_LEN_IN_BITS * LengthFieldSize))) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	/** Initialize the AES instance with ASU DMA pointer. */
	InstancePtr->AsuDmaPtr = DmaPtr;

	/** Configure AES AAD configurations before pushing AAD data to AES engine. */
	InstancePtr->AesState = (PlainTextLen == 0U) ? XAES_ONLY_AAD_UPDATE_IN_PROGRESS :
		XAES_AAD_UPDATE_IN_PROGRESS;
	XAes_ConfigAad(InstancePtr);

	/**
	 * AAD formatting:
	 * B0 Flag | Nonce | Encoded PT Length | Encoded AAD Length | AAD | Zero padding for 16 bytes alignment.
	 */

	/** Calculate the B0 flag dynamically based on tag length and nonce length. */
	NonceHeader[XAES_NONCE_HEADER_FIRST_IDX] = XAES_B0FLAG(AadLen, TagLen, NonceLen);

	/** Copy the nonce into NonceHeader local array. */
	Status = XAsufw_DmaXfr(InstancePtr->AsuDmaPtr, NonceAddr, (u64)(UINTPTR)&NonceHeader[1U],
		NonceLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	NonceHeaderIndex = XAES_NONCE_HEADER_FLAG + NonceLen + (LengthFieldSize - 1U);

	for (Index = 0U; Index < LengthFieldSize; Index++) {
		NonceHeader[NonceHeaderIndex - Index]  = (u8)(PtLenCpy & XAES_BYTE_MASK);
		PtLenCpy >>= XASUFW_ONE_BYTE_SHIFT_VALUE;
	}

	/** Update IsLast based on the presence of AAD. */
	IsLast = (AadLen == 0U) ? XASU_TRUE : XASU_FALSE;

	/** Configure DMA with AES and transfer the Nonce Header to AES engine. */
	Status = XAes_CfgDmaWithAesAndXfer(InstancePtr, (u64)(UINTPTR)NonceHeader, 0U,
		XASU_AES_BLOCK_SIZE_IN_BYTES, IsLast);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if (AadLen > 0U) {
		/** Add AAD length (AadLen encoding based on size). */
		if (AadLen < XAES_AAD_LENGTH_SHORT_LIMIT) {
			NonceHeader[XAES_NONCE_HEADER_FIRST_IDX] = (u8)((AadLen >>
				XASUFW_BYTE_LEN_IN_BITS) & XAES_BYTE_MASK);
			NonceHeader[XAES_NONCE_HEADER_SECOND_IDX] = (u8)(AadLen & XAES_BYTE_MASK);
			TotalAadLen = XAES_TWO_BYTE_ENCODING;
		}
		else if (AadLen < XASU_ASU_DMA_MAX_TRANSFER_LENGTH) {
			NonceHeader[XAES_NONCE_HEADER_FIRST_IDX] = (u8)((XAES_HEADER_6BYTE_INDICATOR >>
				XASUFW_BYTE_LEN_IN_BITS) & XAES_BYTE_MASK);
			NonceHeader[XAES_NONCE_HEADER_SECOND_IDX] = (u8)(XAES_HEADER_6BYTE_INDICATOR &
				XAES_BYTE_MASK);
			for (Index = 0U; Index < XASUFW_WORD_LEN_IN_BYTES; Index++) {
				NonceHeader[XAES_TWO_BYTE_ENCODING + Index] = (u8)((AadLen >>
					(XAES_MSB_SHIFT_32 - (Index * XASUFW_BYTE_LEN_IN_BITS))) & XAES_BYTE_MASK);
			}
			TotalAadLen = XAES_SIX_BYTE_ENCODING;
		}
		else {
			/* Required for MISRA C compliance. */
		}

		/** Configure DMA with AES and transfer the AAD data to AES engine. */
		Status = XAes_CfgDmaWithAesAndXfer(InstancePtr, (u64)(UINTPTR)NonceHeader, 0U, TotalAadLen,
			XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		InstancePtr->CcmAadZeroBlockPadLen = (u8)((XASU_AES_BLOCK_SIZE_IN_BYTES -
			((TotalAadLen + AadLen) % XASU_AES_BLOCK_SIZE_IN_BYTES)) %
			XASU_AES_BLOCK_SIZE_IN_BYTES);

		/** Update IsLast based on whether zero padding is required. */
		IsLast = (InstancePtr->CcmAadZeroBlockPadLen == 0U) ? XASU_TRUE : XASU_FALSE;

		/** Configure DMA with AES and transfer the AAD data to AES engine. */
		Status = XAes_CfgDmaWithAesAndXfer(InstancePtr, AadAddr, 0U, AadLen,
			IsLast);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

END:
	/**
	 * Reset AES engine if InstancePtr is valid and Status is not SUCCESS or
	 * XASUFW_CMD_IN_PROGRESS.
	 */
	if ((Status != XASUFW_SUCCESS) && (Status != XASUFW_CMD_IN_PROGRESS)) {
		/** Set AES under reset on failure. */
		XAes_SetReset(InstancePtr);
	}

RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function decrypts the black key (encrypted key) from the eFuse and decrypted
 * 		key will be stored in the corresponding efuse red key registers of AES key vault.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	DecKeySel	Select the source for the key to be decrypted
 * 		 - EFUSE_Key_0 - 0xEF856601
 * 		 - EFUSE_Key_1 - 0xEF856602
 * @param	DecKeySize	Size of the key to be decrypted.
 * 		 - 0x0 - 128 bit key
 * 		 - 0x2 - 256 bit key
 * @param	IvAddr		Address of the buffer holding IV.
 * @param	IvLen		Length of the IV in bytes.
 *
 * @return
 *		- XASUFW_SUCCESS, if decryption of black key is successful.
 *		- XASUFW_AES_INVALID_PARAM, if InstancePtr or DmaPtr is NULL or ASU DMA is not
 * 			ready.
 *		- XASUFW_AES_INVALID_KEY_SRC, if key source is invalid.
 *		- XASUFW_AES_INVALID_KEY_SIZE, if key size is invalid.
 *		- XASUFW_AES_INVALID_IV, if IV is invalid.
 *
 *************************************************************************************************/
s32 XAes_DecryptEfuseBlackKey(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u32 DecKeySel, u8 DecKeySize,
	u64 IvAddr, u32 IvLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate the input arguments. */
	if (InstancePtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if ((DmaPtr == NULL) || (DmaPtr->AsuDma.IsReady != XIL_COMPONENT_IS_READY)) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if ((DecKeySel != XAES_KEY_TO_BE_DEC_SEL_EFUSE_KEY_0_VALUE) &&
			(DecKeySel != XAES_KEY_TO_BE_DEC_SEL_EFUSE_KEY_1_VALUE)) {
		Status = XASUFW_AES_INVALID_KEY_SRC;
		goto END;
	}

	if ((DecKeySize != XASU_AES_KEY_SIZE_128_BITS) &&
			(DecKeySize != XASU_AES_KEY_SIZE_256_BITS)) {
		Status = XASUFW_AES_INVALID_KEY_SIZE;
		goto END;
	}

	/** Initialize the AES instance. */
	InstancePtr->AsuDmaPtr = DmaPtr;
	InstancePtr->EngineMode = XASU_AES_GCM_MODE;
	InstancePtr->OperationType = XASU_AES_DECRYPT_OPERATION;

	/** Validate the IV with respect to the user provided engine mode. */
	Status = XAsu_AesValidateIvParams(InstancePtr->EngineMode, IvAddr, IvLen);
	if(Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_INVALID_IV;
		goto END;
	}

	/** Release soft reset of AES engine. */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->AesBaseAddress, XAES_SOFT_RST_OFFSET);

	/** Configure AES DPA counter measures. */
	XAes_ConfigCounterMeasures(InstancePtr);

	/** Set the AES engine to enable key decrypt operation mode. */
	XAsufw_WriteReg((InstancePtr->KeyBaseAddress + XAES_KEY_DEC_MODE_OFFSET),
		XAES_KEY_DEC_MODE_VALUE);

	/**
	 * Select the key to be decrypted.
	 * Note: If any other key source is used for decryption, the decryption will fail, and
	 * the core will load zeroes on the data port when such invalid key is selected.
	 */
	XAsufw_WriteReg((InstancePtr->KeyBaseAddress + XAES_KEY_TO_BE_DEC_SEL_OFFSET), (u32)DecKeySel);

	/** Select the size of the key to be decrypted. */
	XAsufw_WriteReg((InstancePtr->KeyBaseAddress + XAES_KEY_TO_BE_DEC_SIZE_OFFSET), DecKeySize);

	/** Select key source as PUF_key. */
	XAsufw_WriteReg((InstancePtr->KeyBaseAddress + XAES_KEY_SEL_OFFSET), XAES_KEY_SEL_PUF_KEY_VALUE);

	/** Select the size of PUF key as 256-bit.  */
	XAsufw_WriteReg((InstancePtr->KeyBaseAddress + XAES_KEY_SIZE_OFFSET), XASU_AES_KEY_SIZE_256_BITS);

	/** Configure AES engine mode and select decrypt operation. */
	XAes_ConfigAesOperation(InstancePtr);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Process and load IV to AES engine. */
	Status = XAes_ProcessAndLoadIv(InstancePtr, IvAddr, IvLen);
	if (Status != XASUFW_SUCCESS) {
		goto END_CLR;
	}

	/** Load key to AES engine. */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_OPERATION_OFFSET), XAES_KEY_LOAD_MASK);

	/** Trigger the key decryption operation. */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_KEY_DEC_TRIG_OFFSET),
		XAES_KEY_DEC_TRIG_MASK);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Wait for AES decrypt operation to complete. */
	Status = XAes_WaitForDone(InstancePtr);

END_CLR:
	/** Set AES under reset. */
	XAes_SetReset(InstancePtr);

	/** Disable the key decryption operation. */
	XAsufw_WriteReg((InstancePtr->KeyBaseAddress + XAES_KEY_DEC_TRIG_OFFSET),
		XAES_KEY_DEC_TRIG_DISABLE);

	/** Set the AES engine to disable key decrypt operation mode. */
	XAsufw_WriteReg((InstancePtr->KeyBaseAddress + XAES_KEY_DEC_MODE_OFFSET),
		XAES_KEY_DEC_MODE_DISABLE);

	/** Reset source of key to be decrypted. */
	XAsufw_WriteReg((InstancePtr->KeyBaseAddress + XAES_KEY_TO_BE_DEC_SEL_OFFSET),
		XAES_KEY_TO_BE_DEC_SEL_DISABLE);

	/** Clear mode config mask. */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET),
		XAES_MODE_CONFIG_DISABLE);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function updates data and key to the AES engine in split mode with DPA CM
 * 		enabled.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param 	InputDataAddr	Input data address.
 * @param 	MaskedOutputPtr	Pointer to buffer holding masked output buffer.
 * @param 	MaskedKeyPtr	Pointer to buffer holding masked key buffer.
 * @param	IvPtr		Pointer to buffer holding IV.
 * @param	OperationType	AES encrypt/decrypt operation type.
 *
 * @return
 *	- XASUFW_SUCCESS, if decryption of data is successful.
 *	- XASUFW_AES_INVALID_PARAM, if InstancePtr or DmaPtr or MaskedOutputPtr is NULL or
 * 		MaskedKeyPtr or IvPtrASU DMA is not ready, or input or output address is invalid.
 * 	- XASUFW_AES_INVALID_OPERATION_TYPE, if operation type is invalid.
 *
 *************************************************************************************************/
s32 XAsufw_AesDpaCmOperation(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u32 InputDataAddr,
	u32 *MaskedOutputPtr, u32 *MaskedKeyPtr, const u32 *IvPtr, u8 OperationType)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Index;

	/** Validate the input arguments. */
	if (InstancePtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if (InputDataAddr == 0U) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if ((DmaPtr == NULL) || (DmaPtr->AsuDma.IsReady != XIL_COMPONENT_IS_READY)) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if ((MaskedOutputPtr == NULL) || (MaskedKeyPtr == NULL) || (IvPtr == NULL)) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if ((OperationType != XASU_AES_ENCRYPT_OPERATION) &&
			(OperationType != XASU_AES_DECRYPT_OPERATION)) {
		Status = XASUFW_AES_INVALID_OPERATION_TYPE;
		goto END;
	}

	/** Initialize the AES instance with ASU DMA pointer. */
	InstancePtr->AsuDmaPtr = DmaPtr;

	/** Initialize the AES instance with engine mode and operation type. */
	InstancePtr->EngineMode = XASU_AES_CTR_MODE;
	InstancePtr->OperationType = OperationType;

	/** Release reset of AES engine. */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->AesBaseAddress, XAES_SOFT_RST_OFFSET);

	/** Configure AES DPA counter measures. */
	XAes_ConfigCounterMeasures(InstancePtr);

	/** Configure AES engine to encrypt/decrypt operation. */
	XAes_ConfigAesOperation(InstancePtr);

	/** Load masked key to user key registers. */
	for (Index = 0U; Index < XASUFW_BUFFER_INDEX_FOUR; Index++) {
		XAsufw_WriteReg(((InstancePtr->KeyBaseAddress + XAES_USER_KEY_0_3_OFFSET) -
			(Index * XASUFW_WORD_LEN_IN_BYTES)), MaskedKeyPtr[Index]);
	}

	/** Load mask key to key mask registers. */
	for (Index = 0U; Index < XASUFW_BUFFER_INDEX_FOUR; Index++) {
		XAsufw_WriteReg(((InstancePtr->KeyBaseAddress + XAES_KEY_MASK_3_OFFSET) -
			(Index * XASUFW_WORD_LEN_IN_BYTES)), XAES_CM_SPLIT_MASK);
	}

	/** Load Iv to Iv registers. */
	for (Index = 0U; Index < XASUFW_BUFFER_INDEX_FOUR; Index++) {
		XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_IV_IN_3_OFFSET) -
			(Index * XASUFW_WORD_LEN_IN_BYTES), IvPtr[Index]);
	}

	/** Configure user key size as 128-bit.  */
	XAsufw_WriteReg((InstancePtr->KeyBaseAddress + XAES_KEY_SIZE_OFFSET), XASU_AES_KEY_SIZE_128_BITS);

	/** Configure AES engine in split mode to update data and key to AES core. */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_SPLIT_CFG_OFFSET),
		(XAES_SPLIT_CFG_KEY_SPLIT_VALUE | XAES_SPLIT_CFG_DATA_SPLIT_VALUE));

	/** Select key source as user key0. */
	XAsufw_WriteReg((InstancePtr->KeyBaseAddress + XAES_KEY_SEL_OFFSET),
		XAES_KEY_SEL_USER_KEY_0_VALUE);

	/** Load key to AES engine. */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_OPERATION_OFFSET), XAES_KEY_LOAD_MASK);

	/** Load Iv to AES engine. */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_OPERATION_OFFSET), XAES_IV_LOAD_MASK);

	/** Configure DMA with AES and transfer the data to AES engine. */
	Status = XAes_CfgDmaWithAesAndXfer(InstancePtr, InputDataAddr,
		(InputDataAddr + XAES_CM_OUTPUT_ADDR_INDEX), XAES_CM_MASK_BUF_WORD_LEN, XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	for (Index = 0U; Index < XASUFW_BUFFER_INDEX_FOUR; Index++ ) {
		MaskedOutputPtr[Index] = Xil_In32((InputDataAddr + XAES_CM_OUTPUT_ADDR_INDEX) +
			(Index * XASUFW_WORD_LEN_IN_BYTES));
		MaskedOutputPtr[XASUFW_BUFFER_INDEX_FOUR + Index] =  Xil_In32(((InputDataAddr +
			XAES_CM_OUTPUT_ADDR_INDEX) + XAES_CM_SPLIT_ALIGNED_LENGTH) +
			(Index * XASUFW_WORD_LEN_IN_BYTES));
	}

END:
	/** Clear the split configuration. */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_SPLIT_CFG_OFFSET), 0U);

	/** Clear the XASU_AES_EXPANDED_KEYS. */
	Status = XAsufw_UpdateErrorStatus(Status, XAes_KeyClear(InstancePtr, XASU_AES_EXPANDED_KEYS));

	/** Set AES under reset. */
	XAes_SetReset(InstancePtr);

	return Status;
}

/*************************************************************************************************/
/**
 *
 * @brief	This function performs AES encryption or decryption for a single data blob,
 * 		which supports additional authentication data (AAD) for all modes except CCM.
 * 		The data blob is processed according to the selected AES operation mode.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	AsuDmaPtr	Pointer to the XAsufw_Dma instance.
 * @param	AesParams	Pointer to the XAsu_AesParams structure containing user input
 * 				parameters.
 *
 * @return
 * 		- XASUFW_SUCCESS, if initialization is successful.
 * 		- XASUFW_AES_INVALID_PARAM, if InstancePtr or AsuDmaPtr is NULL or AesParams is NULL.
 * 		- XASUFW_AES_INIT_FAILED, if AES initialization fails.
 * 		- XASUFW_AES_UPDATE_FAILED, if AES AAD or data update fails.
 * 		- XASUFW_AES_FINAL_FAILED, if AES final fails.
 *
 *************************************************************************************************/
s32 XAes_Compute(XAes *InstancePtr, XAsufw_Dma *AsuDmaPtr, XAsu_AesParams *AesParams)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate the input arguments. */
	if ((InstancePtr == NULL) || (AsuDmaPtr == NULL) || (AesParams == NULL)) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}
	/** Validate mode (CCM mode is not supported). */
	if (AesParams->EngineMode == XASU_AES_CCM_MODE) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	/** Initialize AES engine and load provided key and IV to AES engine. */
	Status = XAes_Init(InstancePtr, AsuDmaPtr, AesParams->KeyObjectAddr,
		AesParams->IvAddr, AesParams->IvLen, AesParams->EngineMode,
		AesParams->OperationType);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_INIT_FAILED;
		goto END;
	}

	/**
	 * Update AAD data to AES engine as per the input from the user.
	 * User should pass AAD address as zero for AES standard modes (CBC, ECB, CFB, OFB, CTR).
	 * If AAD address is zero, skip AAD update.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((AesParams->AadAddr != 0U) && (AesParams->EngineMode != XASU_AES_CBC_MODE) &&
	    (AesParams->EngineMode != XASU_AES_CFB_MODE) &&
	    (AesParams->EngineMode != XASU_AES_OFB_MODE) &&
	    (AesParams->EngineMode != XASU_AES_CTR_MODE) &&
	    (AesParams->EngineMode != XASU_AES_ECB_MODE)) {
		Status = XAes_Update(InstancePtr, AsuDmaPtr, AesParams->AadAddr, 0U,
				     AesParams->AadLen, XASU_FALSE);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_AES_UPDATE_FAILED;
			goto END;
		}
	}

	/**
	 * Update input data to be encrypted to AES engine and store the resultant data at
	 * specified output address. User should provide input data address as zero in case of
	 * CMAC mode.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if ((AesParams->InputDataAddr != 0U) &&
	    (AesParams->EngineMode != XASU_AES_CMAC_MODE)) {
		Status = XAes_Update(InstancePtr, AsuDmaPtr, AesParams->InputDataAddr,
				     AesParams->OutputDataAddr, AesParams->DataLen, XASU_TRUE);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_AES_UPDATE_FAILED;
			goto END;
		}
	}

	/**
	 * Wait for AES operation to complete and generate/verify the tag based on the AES
	 * operation type and mode.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAes_Final(InstancePtr, AsuDmaPtr, AesParams->TagAddr, AesParams->TagLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_FINAL_FAILED;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function saves the complete state of an AES operation.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 * @return
 * 		- XASUFW_SUCCESS, if context was successfully saved.
 * 		- XASUFW_AES_INVALID_PARAM, if any parameter is invalid.
 * 		- XASUFW_FAILURE, upon any other failure.
 *
 *************************************************************************************************/
s32 XAes_SaveContext(XAes *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Index;

	/** Validate the input arguments.*/
	if (InstancePtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	/** Save mode configuration. */
	AesContext.ModeConfig = XAsufw_ReadReg(InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET);

	/** Save the intermediate context. */
	for (Index = 0U; Index < XASUFW_BUFFER_INDEX_FOUR; Index++) {
		AesContext.IvOut[Index] = XAsufw_ReadReg(InstancePtr->AesBaseAddress +
			XAES_IV_OUT_0_OFFSET + (Index * XASUFW_WORD_LEN_IN_BYTES));
#ifdef XASU_AES_CM_ENABLE
		AesContext.IvMaskOut[Index] = XAsufw_ReadReg(InstancePtr->AesBaseAddress +
			XAES_IV_MASK_OUT_0_OFFSET + (Index * XASUFW_WORD_LEN_IN_BYTES));
#endif
	}

	/** Save operational state. */
	AesContext.IsContextSaved = XASU_TRUE;
	AesContext.AesSavedState = (u8)InstancePtr->AesState;

	Status = XASUFW_SUCCESS;

END:
	/** Set AES under reset upon any failure. */
	XAes_SetReset(InstancePtr);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function restores the complete state of an AES operation from a previously
 * 		saved context.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 * @return
 * 		- XASUFW_SUCCESS, if context was successfully restored.
 * 		- XASUFW_AES_INVALID_PARAM, if any parameter is invalid.
 * 		- XASUFW_FAILURE, upon any other failure.
 *
 *************************************************************************************************/
s32 XAes_RestoreContext(XAes *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 ClearStatus = XASUFW_FAILURE;
	XFih_Var XFihAesCtxClear;
	u32 Index;

	/** Validate the input arguments.*/
	if (InstancePtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if ((AesContext.IsContextSaved != XASU_TRUE) ||
			(AesContext.IsContextRestoreReq != XASU_TRUE)) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	/** Release reset of AES engine. */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->AesBaseAddress, XAES_SOFT_RST_OFFSET);

	/** Configure AES DPA counter measures. */
	XAes_ConfigCounterMeasures(InstancePtr);

	/** Check and restore the keys if provided key source is a user key. */
	Status = XAes_CheckAndRestoreUserKeyContext(InstancePtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_CONTEXT_USER_KEY_RESTORE_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Load key to AES engine. */
	XAes_LoadKey(InstancePtr, AesContext.KeyObject.KeySrc, AesContext.KeyObject.KeySize);

	/** Restore mode configuration */
	XAsufw_WriteReg(InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET,
		AesContext.ModeConfig);

	for (Index = 0U; Index < XASUFW_BUFFER_INDEX_FOUR; Index++) {
		XAsufw_WriteReg(InstancePtr->AesBaseAddress + XAES_IV_IN_0_OFFSET +
			(Index * XASUFW_WORD_LEN_IN_BYTES), AesContext.IvOut[Index]);
#ifdef XASU_AES_CM_ENABLE
		XAsufw_WriteReg(InstancePtr->AesBaseAddress + XAES_IV_MASK_IN_0_OFFSET +
			(Index * XASUFW_WORD_LEN_IN_BYTES), AesContext.IvMaskOut[Index]);
#endif
	}

	/** Trigger IV Load. */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_OPERATION_OFFSET),
		XAES_IV_LOAD_MASK);

	/** Restore the AES state machine to saved state. */
	InstancePtr->AesState = (XAes_State)AesContext.AesSavedState;

	Status = XASUFW_SUCCESS;

END:
	/** Clear saved AES context. */
	XFIH_CALL(Xil_SecureZeroize, XFihAesCtxClear, ClearStatus, (u8 *)(UINTPTR)&AesContext,
		sizeof(XAes_ContextInfo));

	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	if (Status != XASUFW_SUCCESS) {
		/** Set AES under reset upon any failure. */
		XAes_SetReset(InstancePtr);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function zeroizes the selected AES key storage register.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	KeySrc		Select the key source which needs to be zeroized.
 *
 * @return
 *		- XASUFW_SUCCESS, if key zeroization is successful.
 *		- XASUFW_AES_INVALID_PARAM, if any input parameter is invalid.
 *		- XASUFW_AES_KEY_CLEAR_ERROR, if AES key clear fails.
 *
 *************************************************************************************************/
s32 XAes_KeyClear(const XAes *InstancePtr, u32 KeySrc)
{
	s32 Status = XASUFW_FAILURE;
	u32 KeyClearMask;
	u32 ZeroedStatusMask;

	/** Validate the input arguments. */
	if (InstancePtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if (KeySrc > XASU_AES_MAX_KEY_SOURCES) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	/** Get the keyclear and zeroedstatus masks based on KeySrc. */
	if (KeySrc == XASU_AES_EXPANDED_KEYS) {
		KeyClearMask = XAES_KEY_CLR_AES_KEY_ZEROIZE_MASK;
		ZeroedStatusMask = XAES_KEY_ZEROED_STATUS_AES_KEY_MASK;
	} else if (AesKeyLookupTbl[KeySrc].KeyClearVal != XAES_INVALID_CFG) {
		KeyClearMask = AesKeyLookupTbl[KeySrc].KeyClearVal;
		ZeroedStatusMask = AesKeyLookupTbl[KeySrc].KeyZeroedStatusMask;
	} else {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	/** Release reset of AES engine. */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->AesBaseAddress, XAES_SOFT_RST_OFFSET);

	/** Zeroize the specified key source. */
	XAsufw_WriteReg(InstancePtr->KeyBaseAddress + XAES_KEY_CLEAR_OFFSET, KeyClearMask);
	/** Check for key zeroed status within Timeout(10sec). */
	Status = (s32)Xil_WaitForEvent((InstancePtr->KeyBaseAddress +
				XAES_KEY_ZEROED_STATUS_OFFSET),	ZeroedStatusMask, ZeroedStatusMask,
				XAES_TIMEOUT_MAX);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_AES_KEY_CLEAR_ERROR;
	}

	XAsufw_WriteReg(InstancePtr->KeyBaseAddress + XAES_KEY_CLEAR_OFFSET,
			XAES_KEY_CLEAR_REG_CLR_MASK);
	/** Set AES under reset. */
	XAsufw_CryptoCoreSetReset(InstancePtr->AesBaseAddress, XAES_SOFT_RST_OFFSET);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns a pointer reference of XAes_Config structure based on the
 * 		device ID.
 *
 * @param	DeviceId	The device ID of the AES core.
 *
 * @return
 * 		- CfgPtr, a reference to a config record in the configuration table
 * 			corresponding to <i>DeviceId</i>.
 * 		- NULL, if no valid device ID is found.
 *
 *************************************************************************************************/
static XAes_Config *XAes_LookupConfig(u16 DeviceId)
{
	XAes_Config *CfgPtr = NULL;
	u32 Index = 0U;

	/* Checks for all the instances */
	for (Index = 0U; Index < XASU_XAES_NUM_INSTANCES; Index++) {
		if (AesConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &AesConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

/*************************************************************************************************/
/**
 * @brief	This function checks whether given key source is in zeroized status or not.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	KeySrc		Key source.
 *
 * @return
 *		- XAES_KEY_ZEROIZED, if key is zeroized.
 * 		- XAES_KEY_NOT_ZEROIZED, if key is not zeroized
 *
 *************************************************************************************************/
static s32 XAes_IsKeyZeroized(const XAes *InstancePtr, u32 KeySrc)
{
	CREATE_VOLATILE(Status, XAES_KEY_ZEROIZED);
	volatile u32 ReadKeyZeroedStatus = XAES_KEY_ZEROED_STATUS_RESET_VAL;

	/** Read the key zeroized status register. */
	ReadKeyZeroedStatus = XAsufw_ReadReg(InstancePtr->KeyBaseAddress +
		XAES_KEY_ZEROED_STATUS_OFFSET);

	/** Check the key zeroized status with its zeroized mask. */
	if ((ReadKeyZeroedStatus & AesKeyLookupTbl[KeySrc].KeyZeroedStatusMask) == 0U) {
		Status = XAES_KEY_NOT_ZEROIZED;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function configures AES DPA counter measures by reading system configuration.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 * @note	On soft reset, the DPA CM is enabled by default. So, it needs to be configured at
 * 		every start of AES operation.
 *
 *************************************************************************************************/
static inline void XAes_ConfigCounterMeasures(const XAes *InstancePtr)
{
#ifdef XASU_AES_CM_ENABLE
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_CM_OFFSET),	XAES_CM_ENABLE_MASK);
#else
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_CM_OFFSET), XAES_CM_DISABLE);
#endif
}

/*************************************************************************************************/
/**
 * @brief	This function configures the AES engine for encrypt/decrypt operation based on the
 * 		OperationType.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 *************************************************************************************************/
static void XAes_ConfigAesOperation(const XAes *InstancePtr)
{
	if (InstancePtr->OperationType == XASU_AES_ENCRYPT_OPERATION) {
		XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET),
			((u32)InstancePtr->EngineMode | (XAES_MODE_CONFIG_ENC_DEC_MASK)));
	} else {
		XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET),
			InstancePtr->EngineMode);
	}
}

/*************************************************************************************************/
/**
 * @brief	This function loads key from selected key source to AES engine.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	KeySrc		Key source.
 * @param	KeySize		Size of the key.
 *
 *************************************************************************************************/
static void XAes_LoadKey(const XAes *InstancePtr, u32 KeySrc, u32 KeySize)
{
	XAsufw_WriteReg((InstancePtr->KeyBaseAddress + XAES_KEY_SIZE_OFFSET), KeySize);

	XAsufw_WriteReg((InstancePtr->KeyBaseAddress + XAES_KEY_SEL_OFFSET),
		AesKeyLookupTbl[KeySrc].KeySrcSelVal);

	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_OPERATION_OFFSET), XAES_KEY_LOAD_MASK);
}

/*************************************************************************************************/
/**
* @brief	This function validates the key source and size by reading them back from the
* 		registers and comparing with expected value.
*
* @param	InstancePtr	Pointer to the XAes instance.
* @param	KeySrc		Expected key source.
* @param	KeySize		Expected key size.
*
* @return
* 		- XASUFW_SUCCESS, if the read values match the expected values.
* 		- XASUFW_AES_KEY_CONFIG_READBACK_ERROR, if there is a mismatch.
*
*************************************************************************************************/
static s32 XAes_ValidateKeyConfig(const XAes *InstancePtr, u32 KeySrc, u32 KeySize)
{
	CREATE_VOLATILE(Status, XASUFW_AES_KEY_CONFIG_READBACK_ERROR);

	/** Compare read-back values. */
	if ((XAsufw_ReadReg(InstancePtr->KeyBaseAddress + XAES_KEY_SIZE_OFFSET) != KeySize) ||
		(XAsufw_ReadReg(InstancePtr->KeyBaseAddress + XAES_KEY_SEL_OFFSET) !=
			AesKeyLookupTbl[KeySrc].KeySrcSelVal)) {
		XFIH_GOTO(END);
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function processes the provided IV and writes it into the appropriate AES IV
 * 		registers based on length of the IV and loads IV to the AES engine.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	IvAddr		Address of an IV buffer.
 * @param	IvLen		Length of an IV in bytes.
 *
 * @return
 *		- XASUFW_SUCCESS, if processing and loading of IV to AES engine is successful.
 *		- XASUFW_FAILURE, if processing or loading of IV fails.
 *
 *************************************************************************************************/
static s32 XAes_ProcessAndLoadIv(XAes *InstancePtr, u64 IvAddr, u32 IvLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	s32 ClearStatus = XASUFW_FAILURE;
	XFih_Var XFihIvClear;
	u32 Iv[XASU_AES_IV_SIZE_128BIT_IN_WORDS] = {0U};
	u32 IvLength = IvLen;
	u32 ZeroPadLen = 0U;
	u8 *FormattedIv;

	/**
	 * For AES-GCM mode, if the IV length is not 96 bits, calculate GHASH and
	 * generate a new IV.
	 * In all other cases, copy IV from 64-bit address space to local array using ASU DMA.
	 */
	if ((InstancePtr->EngineMode == XASU_AES_GCM_MODE) &&
			(IvLength != XASU_AES_IV_SIZE_96BIT_IN_BYTES)) {
		Status = XAes_GHashCal(InstancePtr, IvAddr, (u32)Iv, IvLength);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		IvLength = XASU_AES_IV_SIZE_128BIT_IN_BYTES;
	}
	else if (InstancePtr->EngineMode == XASU_AES_CCM_MODE) {
		FormattedIv = (u8 *)(UINTPTR)Iv;

		/** Set flag byte for AES-CCM nonce. */
		FormattedIv[0] = (u8)((XAES_CCM_Q_CONST - IvLen - XAES_CCM_FLAG_SIZE_IN_BYTES) &
			XAES_CCM_Q_MASK);

		Status = XAsufw_DmaXfr(InstancePtr->AsuDmaPtr, IvAddr,
			(u64)(UINTPTR)&FormattedIv[XAES_CCM_FLAG_SIZE_IN_BYTES], IvLength, 0U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		/** Add zero padding if nonce length is less than 16 bytes in case of CCM mode. */
		if ((IvLength + XAES_CCM_FLAG_SIZE_IN_BYTES) < XASU_AES_IV_SIZE_128BIT_IN_BYTES) {
			ZeroPadLen = XASU_AES_IV_SIZE_128BIT_IN_BYTES -
				(IvLength + XAES_CCM_FLAG_SIZE_IN_BYTES);
			Status = Xil_SMemSet(&FormattedIv[IvLength + XAES_CCM_FLAG_SIZE_IN_BYTES],
				ZeroPadLen, 0U, ZeroPadLen);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
		IvLength = XASU_AES_IV_SIZE_128BIT_IN_BYTES;
	}
	else {
		Status = XAsufw_DmaXfr(InstancePtr->AsuDmaPtr, IvAddr, (u64)(UINTPTR)Iv,
			IvLength, 0U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	/** Write IV to the respective IV registers by changing the endianness. */
	Status = XAsufw_WriteDataToRegsWithEndianSwap(InstancePtr->AesBaseAddress, XAES_IV_IN_3_OFFSET, Iv,
		XASUFW_CONVERT_BYTES_TO_WORDS(IvLength));
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	if ((InstancePtr->EngineMode == XASU_AES_GCM_MODE) &&
			(IvLength  == XASU_AES_IV_SIZE_96BIT_IN_BYTES)) {
		XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_IV_IN_0_OFFSET), XAES_GCM_J0_IV_INIT_VAl);
	}

	/** Trigger IV Load. */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_OPERATION_OFFSET), XAES_IV_LOAD_MASK);

END:
	/** Zeroize local IV buffer. */
	XFIH_CALL(Xil_SecureZeroize, XFihIvClear, ClearStatus, (u8 *)Iv,
		XASU_AES_IV_SIZE_128BIT_IN_BYTES);

	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function calculates the GHASH on provided data.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	IvAddr		Address of IV buffer.
 * @param	IvGen		Address of the buffer to store the generated MAC.
 * @param	IvLen		Length of the input IV buffer in bytes.
 *
 * @return
 *		- XASUFW_SUCCESS, if GHASH calculation on given Iv is successful.
 * 		- XASUFW_AES_INVALID_ENGINE_MODE, if engine mode not AES GCM.
 *		- XASUFW_FAILURE, if GHASH calculation on given Iv fails.
 *
 *************************************************************************************************/
static s32 XAes_GHashCal(const XAes *InstancePtr, u64 IvAddr, u32 IvGen, u32 IvLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 ReadModeConfigReg;

	if (InstancePtr->EngineMode != XASU_AES_GCM_MODE) {
		Status = XASUFW_AES_INVALID_ENGINE_MODE;
		goto END;
	}

	/** Store the mode configuration of previous mode i.e., GCM mode. */
	ReadModeConfigReg = XAsufw_ReadReg(InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET);

	/** Configure engine mode to GHASH mode. */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET),
		(XAES_MODE_CONFIG_AUTH_MASK | XASU_AES_GHASH_MODE));

	/** Configure DMA with AES and transfer the data to AES engine. */
	Status = XAes_CfgDmaWithAesAndXfer(InstancePtr, IvAddr, 0U, IvLen, XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Disable auth mask and restore mode configuration. */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET),
		(ReadModeConfigReg & (~(XAES_MODE_CONFIG_AUTH_MASK))));

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Get newly generated IV from the MAC registers. */
	Status = XAes_ReadTag(InstancePtr, IvGen);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function reads the tag from AES MAC registers during encryption operation.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	TagOutAddr	Address of the buffer to store tag.
 *
 * @return
 *		- XASUFW_SUCCESS, if tag read is successful.
 *		- XASUFW_AES_TAG_GENERATE_FAILED, if tag read fails.
 *
 *************************************************************************************************/
static s32 XAes_ReadTag(const XAes *InstancePtr, u32 TagOutAddr)
{
	CREATE_VOLATILE(Status, XASUFW_AES_TAG_GENERATE_FAILED);
	volatile u32 Index = 0U;
	u32 *TagPtr = (u32 *)TagOutAddr;

	for (Index = 0U; Index < XAES_TAG_LEN_IN_WORDS; Index++) {
		TagPtr[Index] = XAsufw_ReadReg(InstancePtr->AesBaseAddress +
			(XAES_MAC_OUT_3_MASK - (Index * XASUFW_WORD_LEN_IN_BYTES)));
		/*
		 * If AES DPA CM is enabled then, read MAC from both MAC_OUT and MAC_MASK_OUT registers,
		 * If disabled, no need to read the MAC_MASK_OUT register as it contains zeros.
		 */
#ifdef XASU_AES_CM_ENABLE
		TagPtr[Index] ^= XAsufw_ReadReg(InstancePtr->AesBaseAddress +
			(XAES_MAC_MASK_OUT_3_MASK - (Index * XASUFW_WORD_LEN_IN_BYTES)));
#endif
		/* Perform endian swap. */
		TagPtr[Index] = Xil_EndianSwap32(TagPtr[Index]);
	}
	if (Index == XASUFW_WORD_LEN_IN_BYTES) {
		Status = XASUFW_SUCCESS;
		ReturnStatus = XASUFW_AES_TAG_READ;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function reads from AES MAC registers and verifies it with the user provided
 * 		tag during decryption operation.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	TagInAddr	Address of the buffer which holds the expected MAC.
 * @param	TagLen		Length of the tag in bytes.
 *
 * @return
 *		- XASUFW_SUCCESS, if tag comparison is successful.
 *		- XASUFW_AES_TAG_COMPARE_FAILED, if tag comparison fails.
 *
 *************************************************************************************************/
static s32 XAes_ReadNVerifyTag(const XAes *InstancePtr, u32 TagInAddr, u32 TagLen)
{
	CREATE_VOLATILE(Status, XASUFW_AES_TAG_COMPARE_FAILED);
	u32 ReadReg;
	u32 Mask;
	u32 RemainingBytes = TagLen;
	volatile u32 Index = 0U;
	const u32 *TagPtr = (const u32 *)TagInAddr;

	while (RemainingBytes > 0U) {
		ReadReg = XAsufw_ReadReg(InstancePtr->AesBaseAddress +
			(XAES_MAC_OUT_3_MASK - (Index * XASUFW_WORD_LEN_IN_BYTES)));
		/*
		 * If AES DPA CM is enabled then, read MAC from both MAC_OUT and MAC_MASK_OUT registers,
		 * If disabled, no need to read the MAC_MASK_OUT register as it contains zeros.
		 */
#ifdef XASU_AES_CM_ENABLE
		ReadReg ^= XAsufw_ReadReg(InstancePtr->AesBaseAddress +
			(XAES_MAC_MASK_OUT_3_MASK - (Index * XASUFW_WORD_LEN_IN_BYTES)));
#endif
		/* Perform endian swap. */
		ReadReg = Xil_EndianSwap32(ReadReg);

		Mask = (RemainingBytes >= XASUFW_WORD_LEN_IN_BYTES) ? XAES_FULL_WORD_MASK :
			XAES_BIT_MASK(RemainingBytes);

		if ((ReadReg & Mask) != (TagPtr[Index] & Mask)) {
			XFIH_GOTO(END);
		}

		RemainingBytes = (RemainingBytes > XASUFW_WORD_LEN_IN_BYTES) ?
			(RemainingBytes - XASUFW_WORD_LEN_IN_BYTES) : 0U;

		Index++;
	}

	if ((RemainingBytes == 0U) && (Index == XAES_WORD_COUNT_LEN(TagLen))) {
		Status = XASUFW_SUCCESS;
		ReturnStatus = XASUFW_AES_TAG_MATCHED;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function processes tag based on AES operation type, if the Operation type is
 * 		encryption then, it generates the tag and stores it into output TagAddr and for
 * 		decryption it generates and compares the tag with input TagAddr.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	TagAddr		Address of the tag buffer.
 * @param	TagLen		Length of the tag in bytes.
 *
 * @return
 *		- XASUFW_SUCCESS, if read/verification of the tag is successful based on AES
 *			operation type.
 *		- XASUFW_FAILURE, if read/verification of the tag fails.
 *
 *************************************************************************************************/
static s32 XAes_ProcessTag(const XAes *InstancePtr, u64 TagAddr, u32 TagLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 Tag[XASU_AES_MAX_TAG_LENGTH_IN_BYTES] = {0U};

	/** If AES operation type is encryption, */
	if (InstancePtr->OperationType == XASU_AES_ENCRYPT_OPERATION) {
		/** - Read the generated tag from AES engine to local array. */
		Status = XAes_ReadTag(InstancePtr, (u32)Tag);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		/** - Copy generated tag from local array to user provided address space using DMA. */
		Status = XAsufw_DmaXfr(InstancePtr->AsuDmaPtr, (u64)(UINTPTR)Tag, TagAddr,
				       TagLen, 0U);
	} else {
		/**
		 * If AES operation type is decryption,
		 * - Copy tag from user provided address space to local array using DMA.
		 */
		Status = XAsufw_DmaXfr(InstancePtr->AsuDmaPtr, TagAddr, (u64)(UINTPTR)Tag,
				       TagLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		/** - Verify the generated tag with user provided tag. */
		Status = XAes_ReadNVerifyTag(InstancePtr, (u32)Tag, TagLen);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function configures AAD.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 *************************************************************************************************/
static void XAes_ConfigAad(const XAes *InstancePtr)
{
	if (InstancePtr->AesState == XAES_AAD_UPDATE_IN_PROGRESS) {
		/**
		 * For modes like AES-GCM and AES-CCM, where payload is expected after AAD,
		 * configure only AUTH_MASK bit.
		 */
		XAsufw_RMW((InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET),
			    XAES_MODE_CONFIG_AUTH_MASK, XAES_MODE_CONFIG_AUTH_MASK);
	} else {
		/**
		 * For MAC only modes like AES-GMAC and AES-CMAC, where no payload is expected to
		 * follow AAD data, configure both AUTH_MASK and AUTH_WITH_NO_PAYLOAD bit.
		 */
		XAsufw_RMW((InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET),
			    (XAES_MODE_CONFIG_AUTH_MASK | XAES_MODE_CONFIG_AUTH_WITH_NO_PAYLOAD_MASK),
			    (XAES_MODE_CONFIG_AUTH_MASK | XAES_MODE_CONFIG_AUTH_WITH_NO_PAYLOAD_MASK));
	}
}

/*************************************************************************************************/
/**
 * @brief	This function clears the configuration of AAD after pushing final AAD data to
 * 		AES engine.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 *************************************************************************************************/
static void XAes_ClearConfigAad(const XAes *InstancePtr)
{
	/** Clear AUTH and AUTH_WITH_NO_PAYLOAD bits in AES mode configuration register. */
	XAsufw_RMW((InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET),
		   (XAES_MODE_CONFIG_AUTH_MASK | XAES_MODE_CONFIG_AUTH_WITH_NO_PAYLOAD_MASK), 0U);
}

/*************************************************************************************************/
/**
 * @brief	This function configures SSS and transfers data to AES engine using DMA.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	InDataAddr	Address of the input buffer.
 * @param	OutDataAddr	Address of output buffer to hold the resultant data.
 * @param	Size		Size of data to send/receive to/from AES engine in bytes.
 * @param	IsLastChunk	For the last update of data, this parameter should be set
 *		 	 	to TRUE otherwise, FALSE.
 *
 * @return
 *		- XASUFW_SUCCESS, if transfer is successful.
 *		- XASUFW_FAILURE, if any failure occurs.
 *
 *************************************************************************************************/
static s32 XAes_CfgDmaWithAesAndXfer(const XAes *InstancePtr, u64 InDataAddr, u64 OutDataAddr, u32 Size,
	u8 IsLastChunk)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Configure DMA to AES. */
	Status = XAsufw_SssAesWithDma(InstancePtr->AsuDmaPtr->SssDmaCfg);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * If OutDataAddr is zero, configure DMA source channel and transfer AAD to AES engine.
	 * If OutDataAddr is non-zero, configure both source and destination channels of DMA and
	 * transfer data to AES engine.
	 */
	if (OutDataAddr != XAES_AAD_UPDATE_NO_OUTPUT_ADDR) {
		XAsuDma_ByteAlignedTransfer(&InstancePtr->AsuDmaPtr->AsuDma, XCSUDMA_DST_CHANNEL,
			OutDataAddr, Size, IsLastChunk);
	}

	XAsuDma_ByteAlignedTransfer(&InstancePtr->AsuDmaPtr->AsuDma, XCSUDMA_SRC_CHANNEL,
		InDataAddr, Size, IsLastChunk);

	/** If the data length is greater than XASUFW_DMA_BLOCKING_SIZE, do not wait for DMA done. */
	if (Size <= XASUFW_DMA_BLOCKING_SIZE) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		/** Wait till the ASU source DMA done bit to set. */
		Status = XAsuDma_WaitForDoneTimeout(&InstancePtr->AsuDmaPtr->AsuDma,
			XCSUDMA_SRC_CHANNEL);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		/** Acknowledge the transfer has completed from source. */
		XAsuDma_IntrClear(&InstancePtr->AsuDmaPtr->AsuDma, XCSUDMA_SRC_CHANNEL,
			XCSUDMA_IXR_DONE_MASK);

		if (OutDataAddr != XAES_AAD_UPDATE_NO_OUTPUT_ADDR) {
			ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
			/** Wait till the ASU destination DMA done bit to set. */
			Status = XAsuDma_WaitForDoneTimeout(&InstancePtr->AsuDmaPtr->AsuDma,
				XCSUDMA_DST_CHANNEL);
			if (Status != XASUFW_SUCCESS) {
				goto END;
			}

			/** Acknowledge the transfer has completed from destination. */
			XAsuDma_IntrClear(&InstancePtr->AsuDmaPtr->AsuDma, XCSUDMA_DST_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);
		}
	} else {
		Status = XASUFW_CMD_IN_PROGRESS;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function executes a dummy AES encryption operation to initialize the
 * 		decryption key schedule. It is primarily used in AES key expansion when
 * 		transitioning from encryption to decryption mode.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 * @return
 *		- XASUFW_SUCCESS, if the operation completes successfully.
 *		- XASUFW_FAILURE, upon any failure.
 *
 *************************************************************************************************/
static s32 XAes_DummyEncryption(XAes *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	/* Same array is used for both input and output to perform dummy encryption. */
	u8 DummyData[XASU_AES_BLOCK_SIZE_IN_BYTES] = {0U};

	/** Validate OperationType. */
	if (InstancePtr->OperationType != XASU_AES_DECRYPT_OPERATION) {
		Status = XAsufw_UpdateErrorStatus(Status,
			XASUFW_AES_ECB_CBC_DUMMY_ENCRYPTION_FAILED);
		goto END;
	}

	/** Set AES to encryption mode for dummy encryption. */
	InstancePtr->OperationType = XASU_AES_ENCRYPT_OPERATION;
	XAes_ConfigAesOperation(InstancePtr);

	/** Perform a dummy encryption to generate the decryption key. */
	Status = XAes_Update(InstancePtr, InstancePtr->AsuDmaPtr, (u64)(UINTPTR)DummyData,
		(u64)(UINTPTR)DummyData, XASU_AES_BLOCK_SIZE_IN_BYTES, XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
			XASUFW_AES_ECB_CBC_DUMMY_ENCRYPTION_FAILED);
		goto END;
	}

	/** Restore the original operation type. */
	InstancePtr->OperationType = XASU_AES_DECRYPT_OPERATION;
	XAes_ConfigAesOperation(InstancePtr);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Retrieves the current AES engine mode.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 * @return
 *		- EngineMode if the instance pointer is valid.
 *		- XASUFW_AES_INVALID_ENGINE_MODE, upon NULL instance pointer.
 *
 *************************************************************************************************/
u8 XAes_GetEngineMode(const XAes *InstancePtr)
{
	return (InstancePtr != NULL) ? InstancePtr->EngineMode :
		(u8)XASUFW_AES_INVALID_ENGINE_MODE;
}

/*************************************************************************************************/
/**
 * @brief	This function retrieves the current internal AES state and validates it.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 * @return
 *		- XASU_TRUE, if the Aes state is update in progress or completed state.
 * 		- XASU_FALSE, if the AES state is not in update state.
 *		- XASUFW_AES_INVALID_INTERNAL_STATE, upon NULL instance pointer.
 *
 *************************************************************************************************/
u8 XAes_GetAndValidateInternalState(const XAes *InstancePtr)
{
	u8 AesState = (InstancePtr != NULL) ? (u8)InstancePtr->AesState :
		(u8)XASUFW_AES_INVALID_INTERNAL_STATE;
	return (((AesState == (u8)XAES_UPDATE_COMPLETED) ||
		(AesState == (u8)XAES_DATA_UPDATE_IN_PROGRESS)) ? XASU_TRUE : XASU_FALSE);
}

/*************************************************************************************************/
/**
 * @brief	This function finalizes the AAD update phase by optionally sending zero padding
 * 		(for CCM mode) and clearing the AAD configuration for all modes.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 * @return
 *		- XASUFW_SUCCESS, if finalization of AAD successful.
 *		- XASUFW_FAILURE, if finalization of AAD fails.
 *
 *************************************************************************************************/
static s32 XAes_FinalizeAadUpdate(XAes *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 MaxCcmAadZeroPadding[XASU_AES_BLOCK_SIZE_IN_BYTES];

	/**
	 * After AAD update:
	 * - If the engine mode is AES-CCM and CcmAadZeroBlockPadLen is non-zero, push zero padding
	 *   data to AES engine and clear the AAD configuration after dma non-blocking.
	 * - For other MAC modes, just clear the AAD configuration after dma non-blocking.
	 */
	if ((InstancePtr->EngineMode == XASU_AES_CCM_MODE) &&
			(InstancePtr->CcmAadZeroBlockPadLen != 0U)) {
		Status = Xil_SMemSet(MaxCcmAadZeroPadding, InstancePtr->CcmAadZeroBlockPadLen, 0U,
			InstancePtr->CcmAadZeroBlockPadLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = XAes_CfgDmaWithAesAndXfer(InstancePtr,
			(u64)(UINTPTR)MaxCcmAadZeroPadding, 0U,
			InstancePtr->CcmAadZeroBlockPadLen, XASU_TRUE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
		InstancePtr->CcmAadZeroBlockPadLen = 0U;
	}

	XAes_ClearConfigAad(InstancePtr);
	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks and restores the keys to the register if the provided
 * 		key source is a user key.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 * @return
 *		- XASUFW_SUCCESS, if key restore is successful.
 * 		- XASUFW_FAILURE, upon any other failure.
 *
 *************************************************************************************************/
static s32 XAes_CheckAndRestoreUserKeyContext(const XAes *InstancePtr)
{
	s32 Status = XASUFW_FAILURE;
	u32 Offset;
	u32 KeySizeInWords = 0U;

	/** Restore key. */
	if (AesContext.KeyObject.KeySrc <= XASU_AES_USER_KEY_7) {
		KeySizeInWords = (AesContext.KeyObject.KeySize == XASU_AES_KEY_SIZE_128_BITS) ?
			XASU_AES_KEY_SIZE_128BIT_IN_WORDS : XASU_AES_KEY_SIZE_256BIT_IN_WORDS;

		Offset = AesKeyLookupTbl[AesContext.KeyObject.KeySrc].RegOffset;
		/**
		 * Traverse to Offset of last word of respective USER key register
		 * (i.e., XAES_USER_KEY_X_7), where X = 0 to 7 USER key.
		 * Write the key to the respective key source registers by changing the endianness.
		 */
		Offset = Offset + (KeySizeInWords * XASUFW_WORD_LEN_IN_BYTES) - XASUFW_WORD_LEN_IN_BYTES;

		/** Write user key to the respective user key registers by changing the endianness. */
		Status = XAsufw_WriteDataToRegsWithEndianSwap(InstancePtr->KeyBaseAddress, Offset, AesContext.Key, KeySizeInWords);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}
	Status = XASUFW_SUCCESS;

END:
	return Status;

}

/*************************************************************************************************/
/**
 * @brief	This function waits for AES operation to complete.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 * @return
 *		- XASUFW_SUCCESS, if AES operation is successful.
 *		- XASUFW_FAILURE, if DONE bit is not set within given timeout.
 *
 *************************************************************************************************/
static s32 XAes_WaitForDone(const XAes *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/* Check for AES operation is completed within Timeout(10sec) or not. */
	Status = (s32)Xil_WaitForEvent((InstancePtr->AesBaseAddress + XAES_INTERRUPT_STATUS_OFFSET),
				  XAES_INTERRUPT_STATUS_DONE_MASK, XAES_INTERRUPT_STATUS_DONE_MASK, XAES_TIMEOUT_MAX);

	return Status;
}

/*************************************************************************************************/
/**
* @brief This function waits for the AES engine to become idle and ready.
*
* @param	InstancePtr	Pointer to the XAes instance.
*
* @return
* 		- XASUFW_SUCCESS, if AES engine becomes ready before timeout.
* 		- XASUFW_FAILURE, if READY bit is not set within given timeout.
*
*************************************************************************************************/
static s32 XAes_WaitForReady(const XAes *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/* Check for AES engine is idle and ready within Timeout(10sec) or not. */
	Status = (s32)Xil_WaitForEvent((InstancePtr->AesBaseAddress + XAES_STATUS_OFFSET),
		(XAES_STATUS_BUSY_MASK | XAES_STATUS_READY_MASK),
		XAES_STATUS_READY_MASK, XAES_TIMEOUT_MAX);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function places the AES hardware core into reset state and updates AES state
 * 		machine to initialized state.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 *************************************************************************************************/
void XAes_SetReset(XAes *InstancePtr)
{
	u32 Index = 0U;

	/*
	* - Hardware workaround for clearing the IV as it is not being cleared after
	*   AES engine is set into reset.
	*/

	for (Index = 0U; Index < XASUFW_BUFFER_INDEX_FOUR; Index++) {
		XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_IV_IN_3_OFFSET) -
			(Index * XASUFW_WORD_LEN_IN_BYTES), 0U);
	}

	InstancePtr->AesState = XAES_INITIALIZED;
	XAsufw_CryptoCoreSetReset(InstancePtr->AesBaseAddress, XAES_SOFT_RST_OFFSET);
}
/** @} */
