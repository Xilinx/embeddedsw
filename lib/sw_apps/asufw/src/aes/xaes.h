/**************************************************************************************************
* Copyright (c) 2024 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xaes.h
 *
 * This file Contains the function prototypes, defines and macros for the AES module.
 *
 * This driver supports the following features:
 *  - Initialization & Configuration
 *  - AES encryption and decryption
 *
 * The AES driver instance can be initialized by calling the following APIs:
 *   - XAes_GetInstance(u16 DeviceId)
 *   - XAes_CfgInitialize(XAes *InstancePtr)
 *
 * A pointer to ASU AES instance has to be passed in initialization as ASU AES will be used
 * for the data transfer to AES module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   am   06/26/24 Initial release
 *       am   08/01/24 Replaced variables of type enums with u32 type.
 *       am   08/24/24 Added AES DPA CM KAT support
 * 1.1   am   01/20/25 Added AES CCM support
 *       yog  02/26/25 Added XAes_Compute() API
 *       am   03/14/25 Fixed alignment of function prototype
 *       yog  04/04/25 Added XAes_KeyClear() API
 *       am   04/14/25 Added macros related to non-blocking update stages.
 * 1.2   am   05/18/25 Moved non-blocking update stages macros to aes handler
 *       am   07/18/25 Modified XAes_SetReset() visibility from static to non-static
 * 1.3   kp   08/21/26 Removed IsContextRestoreReq from XAes_ContextInfo
 *       kp   09/09/26 Added XAes_FetchAndStageEfuseUserKey1() prototype
 *
 * </pre>
 *
**************************************************************************************************/
/**
* @addtogroup xaes_server_apis AES Server APIs
* @{
*/
#ifndef XAES_H_
#define XAES_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *****************************************************/
#include "xasufw_sss.h"
#include "xasufw_dma.h"
#include "xaes_hw.h"
#include "xasu_aesinfo.h"
#include "xasufw_util.h"

/************************** Constant Definitions *************************************************/
/** Number of 32-bit words in a 128-bit AES authentication tag. */
#define XAES_TAG_LEN_IN_WORDS				4U

/** Size of the AES slice of the reserved DDR area, which precedes the key vault. */
#define XAES_CONTEXT_DDR_MAX_SIZE			256U

/************************************** Type Definitions *****************************************/
typedef struct _XAes_Config XAes_Config; /**< This typedef is to create alias name
						for _XAes_Config */
typedef struct _XAes XAes; /**< This typedef is to create alias name for _XAes */

/**
 * AES context structure for saving and restoring AES state.
 *
 * This structure preserves the complete state of an AES operation, including
 * configuration, IV, key information, and operational state. It enables
 * context switching for preemptive AES operations.
 */
typedef struct {
	u32 ReqId;				/**< Request ID associated with this context. */
	u32 ModeConfig;				/**< AES mode configuration register value. */
	u32 IvOut[XASUFW_BUFFER_INDEX_FOUR];	/**< Intermediate IV/counter output. */
	u32 IvMaskOut[XASUFW_BUFFER_INDEX_FOUR]; /**< Intermediate IV mask output (CM). */
	u32 IntMacOut[XASUFW_BUFFER_INDEX_FOUR]; /**< Intermediate MAC output (GCM/CCM). */
	u32 IntMacMaskOut[XASUFW_BUFFER_INDEX_FOUR]; /**< Intermediate MAC mask output (CM). */
	u32 S0Out[XASUFW_BUFFER_INDEX_FOUR];	/**< CCM S0 output. */
	u32 S0MaskOut[XASUFW_BUFFER_INDEX_FOUR]; /**< CCM S0 mask output (CM). */
	u32 GcmLenOut[XASUFW_BUFFER_INDEX_FOUR]; /**< GCM processed bit-length output. */
	u32 SavedMac[XAES_TAG_LEN_IN_WORDS];	/**< Finalized MAC captured for auth-only
						replay. */
	u32 Key[XASU_AES_KEY_SIZE_256BIT_IN_WORDS]; /**< User key array. */
	XAsu_AesKeyObject KeyObject;		/**< Key object address. */
	u8 CcmAadResidue[XASU_AES_BLOCK_SIZE_IN_BYTES]; /**< Partial CCM CBC-MAC block (prefix + AAD
						tail) held across the context switch. */
	u8 CcmAadResidueLen;			/**< Valid bytes in CcmAadResidue (0..16). */
	u8 IsContextSaved;			/**< Flag indicating if context is valid. */
	u8 AesSavedState;			/**< AES internal state. */
	u8 IsAuthCtxSaved;			/**< Flag indicating INT_MAC/S0/GCMLEN context is
						saved. */
	u8 IsMacFinalized;			/**< Flag indicating auth-only MAC was already
						finalized and is held in SavedMac for replay. */
	u8 Reserved[3];				/**< Reserved for alignment. */
} XAes_ContextInfo;

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XAES_AAD_UPDATE_NO_OUTPUT_ADDR	(0U) /**< Output address should be zero during AAD update. */
#define XAES_DPA_IV_LEN_IN_BYTES			(12U)	/**< DPA CM KAT IV length in bytes. */

#define XAES_KEY_SIZE_CONVERSION_OFFSET		(16U) /**< Offset for AES key size enum to byte conversion */
#define XAES_KEY_SIZE_CONVERSION_SHIFT		(3U)  /**< Shift for AES key size enum to byte conversion */

#define XAES_KEY_CLEAR_MULTI_KEY_FLAG		(0x80000000U) /**< When this flag is OR'ed into the
						KeySrc argument of XAes_KeyClear(), the remaining bits
						are treated as a bitmask of key sources to clear in a
						single call instead of a single key source value. */
#define XAES_KEY_SRC_TO_CLEAR_MASK(KeySrc)	((u32)1U << (u32)(KeySrc)) /**< Converts a key source
						value to its corresponding bit in the multi-key clear
						bitmask used by XAes_KeyClear(). XASU_AES_EXPANDED_KEYS
						key source cannot be used with this macro. */

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
XAes *XAes_GetInstance(u16 DeviceId);
s32 XAes_CfgInitialize(XAes *InstancePtr);
s32 XAes_WriteKey(XAes *InstancePtr, XAsufw_Dma *DmaPtr, XAsu_AesKeyObject *KeyObjectPtr);
s32 XAes_Init(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 IvAddr, u32 IvLen,
	u8 EngineMode, u8 OperationType);
s32 XAes_Update(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 InDataAddr, u64 OutDataAddr,
	u32 DataLength, u8 IsLastChunk);
s32 XAes_Final(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 TagAddr, u32 TagLen);
s32 XAes_CcmSendB0Header(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 NonceAddr, u8 NonceLen,
	u32 TotalAadLen, u32 PlainTextLen, u8 TagLen);
s32 XAes_DpaCmOperation(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u32 InputDataAddr,
	u32 *MaskedOutputPtr, u32 *MaskedTagPtr, const u32 *MaskedKeyPtr, const u8 *IvPtr,
	u8 OperationType, u32 SplitMask);
s32 XAes_DecryptEfuseBlackKey(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u32 DecKeySel, u8 DecKeySize,
	u64 IvAddr, u32 IvLen);
s32 XAes_FetchAndStageEfuseUserKey1(XAes *InstancePtr, XAsufw_Dma *DmaPtr);
s32 XAes_EncryptRedKey(XAes *InstancePtr, XAsufw_Dma *DmaPtr, XAsu_AesParams *AesParams);
s32 XAes_Compute(XAes *InstancePtr, XAsufw_Dma *AsuDmaPtr, XAsu_AesParams *AesParams);
u8 XAes_GetEngineMode(const XAes *InstancePtr);
s32 XAes_KeyClear(const XAes *InstancePtr, u32 KeySrc);
void XAes_SetReset(XAes *InstancePtr);
u8 XAes_GetAndValidateInternalState(const XAes *InstancePtr);
s32 XAes_GetContextDdrPtr(XAes_ContextInfo **CtxPtrPtr);
s32 XAes_SaveContext(XAes *InstancePtr);
s32 XAes_RestoreContext(XAes *InstancePtr, XAsufw_Dma *DmaPtr);
s32 XAes_PostNbDmaRoutine(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u8 PayloadFollows);
s32 XAes_CheckAadChunk(u8 EngineMode, u32 AadLen, u8 IsLast, u8 PayloadFollows);

#ifdef __cplusplus
}
#endif

#endif  /* XAES_H_ */
/** @} */
