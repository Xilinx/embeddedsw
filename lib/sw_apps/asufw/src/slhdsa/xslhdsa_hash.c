/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xslhdsa_hash.c
*
* This file contains the SLH-DSA hash function implementation for ASUFW firmware.
*
* Provides the tweakable hash functions and PRFs FIPS 205 requires (F, H, T_l, PRF,
* PRF_msg and H_msg) behind one dispatch layer, together with the backends they route to:
* - The dispatcher, which forwards each call to the bound ops table.
* - The XSha hardware engine driver used by both instantiations.
* - The SHA2 instantiation, including MGF1 and the compressed 22-byte ADRS^c form.
* - The SHAKE instantiation, which absorbs the full 32-byte ADRS.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   kk   07/10/26 Initial release
*
* </pre>
*
*************************************************************************************************/
/*************************************** Include Files *******************************************/
/**
* @addtogroup xslhdsa_server_apis SLH-DSA Server APIs
* @{
*/
#include "xil_sutil.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasu_def.h"
#include "xasu_shainfo.h"
#include "xasu_slhdsainfo.h"
#include "xslhdsa.h"
#include "xslhdsa_addr.h"
#include "xslhdsa_hash.h"

#ifdef XASU_SLHDSA_ENABLE
/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
/** SHA2 mode for F and PRF (always SHA-256). */
#define XSLHDSA_SHA2_MODE_FPRF	(XASU_SHA_MODE_256)

/************************************ Function Prototypes ****************************************/
static const XSlhDsa_HashOps *XSlhDsa_HashGetOps(XSlhDsa_Instance *InstPtr);
static s32 XSlhDsa_MapHashStatus(s32 Status, s32 Error);
static s32 XSlhDsa_HashEngineUpdate(XSlhDsa_HashCtx *HashCtxPtr, u64 Addr, u32 Len,
				   u32 EndLast, u32 AllowInProgress);
static inline s32 XSlhDsa_HashUpdateMsg(XSlhDsa_HashCtx *HashCtxPtr, u64 Addr,
					u32 Len, u32 EndLast);
static inline s32 XSlhDsa_HashZeroizeKeepWait(XSlhDsa_Instance *InstPtr, s32 Status, u8 *Buf,
					      u32 Len);
static s32 XSlhDsa_HashShakePkSeedAddrStream(XSlhDsa_Instance *InstPtr,
			u64 MsgAddr, const u8 *MsgLocal,
			u32 MsgLen, u8 *Out, u32 OutLen);
static s32 XSlhDsa_HashSha2DigestLen(u32 ShaMode, u32 *DigestLenPtr);
static s32 XSlhDsa_HashSha2BlockLen(u32 ShaMode, u32 *BlockLenPtr);
static s32 XSlhDsa_HashSha2PkSeedAddrStream(XSlhDsa_Instance *InstPtr, u64 MsgAddr,
			const u8 *MsgLocal, u32 MsgLen, u32 ShaMode,
			u8 *Out, u32 N);

static u32 XSlhDsa_Sha2ModeHtl(u32 HashWidth);
static s32 XSlhDsa_Sha2MgfBlock(u32 ShaMode, u32 *MgfBlockPtr);
static s32 XSlhDsa_Sha2HmacBlockLen(u32 ShaMode, u32 *BlockLenPtr);
static s32 XSlhDsa_Sha2Mgf1Expand(XSlhDsa_Instance *InstPtr, u8 *Seed, u32 SeedLen,
			u32 ShaMode, u8 *Out);
static s32 XSlhDsa_Sha2HashMsgImpl(XSlhDsa_Instance *InstPtr, u32 ShaMode, u8 *Out);
static s32 XSlhDsa_Sha2HashPrfMsgImpl(XSlhDsa_Instance *InstPtr, const u8 *OptRand,
			const u8 *Msg, u32 MsgLen, const u8 *Ctx, u32 CtxLen,
			u32 HashWidth, u32 BlockLen, u32 ShaMode, u8 *Out);

static s32 XSlhDsa_ShakeFeedHmsgInputs(XSlhDsa_HashCtx *HashCtxPtr,
					    XSlhDsa_Instance *InstPtr);

static s32 XSlhDsa_HashShakeAppendPureContext(XSlhDsa_HashCtx *HashCtxPtr, const u8 *Ctx,
			u32 CtxLen);
static s32 XSlhDsa_HashSha2AppendPureContext(XSlhDsa_HashCtx *HashCtxPtr, const u8 *Ctx,
			u32 CtxLen);

/************************************ Variable Definitions ***************************************/
/**
 * Static hash ops lookup indexed by XSlhDsa_HashFamily.
 *
 * Index XSLHDSA_HASH_SHAKE selects XSlhDsa_HashOpsShake; index
 * XSLHDSA_HASH_SHA2 selects XSlhDsa_HashOpsSha2.
 */
static const XSlhDsa_HashOps *const XSlhDsa_HashOpsByFamily[] = {
		[XSLHDSA_HASH_SHAKE] = &XSlhDsa_HashOpsShake,
		[XSLHDSA_HASH_SHA2] = &XSlhDsa_HashOpsSha2,
};

/**
 * SHA2 hash backend ops table for SLH-DSA-SHA2 parameter sets.
 *
 * Dispatches FIPS 205 H_msg, F, H, TL, PRF, PRF_msg, and chain() to the SHA2
 * implementations in this file.
 */
const XSlhDsa_HashOps XSlhDsa_HashOpsSha2 = {
		.HashMsg = XSlhDsa_Sha2HashMsg,
		.HashF = XSlhDsa_Sha2HashF,
		.HashH = XSlhDsa_Sha2HashH,
		.HashTl = XSlhDsa_Sha2HashTl,
		.HashPrf = XSlhDsa_Sha2HashPrf,
		.HashPrfMsg = XSlhDsa_Sha2HashPrfMsg,
		.Chain = XSlhDsa_Sha2Chain,
};

/**
 * SHAKE hash backend ops table for SLH-DSA-SHAKE parameter sets.
 *
 * Dispatches FIPS 205 H_msg, F, H, TL, PRF, PRF_msg, and chain() to the SHAKE
 * implementations in this file.
 */
const XSlhDsa_HashOps XSlhDsa_HashOpsShake = {
		.HashMsg = XSlhDsa_ShakeHashMsg,
		.HashF = XSlhDsa_ShakeHashF,
		.HashH = XSlhDsa_ShakeHashH,
		.HashTl = XSlhDsa_ShakeHashTl,
		.HashPrf = XSlhDsa_ShakeHashPrf,
		.HashPrfMsg = XSlhDsa_ShakeHashPrfMsg,
		.Chain = XSlhDsa_ShakeChain,
};

/*************************************************************************************************/
/**
* @brief	Compute H_msg through the bound hash backend.
*
* @param	InstPtr	Pointer to SLH-DSA instance with signature and message fields set.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input or unbound ops table.
*		- XASUFW_CMD_IN_PROGRESS if the message SHA update is running non-blocking.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
* @note
* Delegates to FIPS 205 H_msg in the SHA2 or SHAKE backend.
*
*************************************************************************************************/
s32 XSlhDsa_HashMsg(XSlhDsa_Instance *InstPtr)
{
	s32 Status = XASUFW_FAILURE;
	const XSlhDsa_HashOps *Ops = XSlhDsa_HashGetOps(InstPtr);

	/** Validate that the instance carries a bound backend ops table. */
	if (Ops == NULL) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Compute FIPS 205 H_msg(R, PK.seed, PK.root, M) in the bound SHA2 or SHAKE backend. */
	Status = Ops->HashMsg(InstPtr);

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Compute F through the bound hash backend.
*
* @param	InstPtr	Pointer to SLH-DSA instance with PkSeed and Addr loaded.
* @param	Input	Input buffer M1 (n bytes).
* @param	Output	Output buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input or unbound ops table.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
* @note
* Delegates to FIPS 205 F in the SHA2 or SHAKE backend.
*
*************************************************************************************************/
s32 XSlhDsa_HashF(XSlhDsa_Instance *InstPtr, const u8 *Input, u8 *Output)
{
	s32 Status = XASUFW_FAILURE;
	const XSlhDsa_HashOps *Ops = XSlhDsa_HashGetOps(InstPtr);

	/** Validate the bound ops table and the caller supplied input and output buffers. */
	if ((Ops == NULL) || (Input == NULL) || (Output == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Compute FIPS 205 F(PK.seed, ADRS, M1) in the bound SHA2 or SHAKE backend. */
	Status = Ops->HashF(InstPtr, Input, Output);

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Compute H through the bound hash backend.
*
* @param	InstPtr	Pointer to SLH-DSA instance with PkSeed and Addr loaded.
* @param	Left	Left child node (n bytes).
* @param	Right	Right child node (n bytes).
* @param	Output	Output buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input or unbound ops table.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
* @note
* Delegates to FIPS 205 H in the SHA2 or SHAKE backend.
*
*************************************************************************************************/
s32 XSlhDsa_HashH(XSlhDsa_Instance *InstPtr, const u8 *Left,
		       const u8 *Right, u8 *Output)
{
	s32 Status = XASUFW_FAILURE;
	const XSlhDsa_HashOps *Ops = XSlhDsa_HashGetOps(InstPtr);

	/** Validate the bound ops table and the child node and output buffers. */
	if ((Ops == NULL) || (Left == NULL) || (Right == NULL) || (Output == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Compute FIPS 205 H(PK.seed, ADRS, Left || Right) in the bound SHA2 or SHAKE backend. */
	Status = Ops->HashH(InstPtr, Left, Right, Output);

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Compute T_l through the bound hash backend.
*
* @param	InstPtr	Pointer to SLH-DSA instance with PkSeed and Addr loaded.
* @param	Input	Input buffer M.
* @param	InputLen	Input length in bytes.
* @param	Output	Output buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input or unbound ops table.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
* @note
* Delegates to FIPS 205 T_l in the SHA2 or SHAKE backend.
*
*************************************************************************************************/
s32 XSlhDsa_HashTl(XSlhDsa_Instance *InstPtr, const u8 *Input,
			u32 InputLen, u8 *Output)
{
	s32 Status = XASUFW_FAILURE;
	const XSlhDsa_HashOps *Ops = XSlhDsa_HashGetOps(InstPtr);

	/** Validate the bound ops table and the buffers; a zero InputLen is accepted. */
	if ((Ops == NULL) || (Input == NULL) || (Output == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Compute FIPS 205 T_l(PK.seed, ADRS, M) over the l*n byte input in the bound backend. */
	Status = Ops->HashTl(InstPtr, Input, InputLen, Output);

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Compute PRF through the bound hash backend.
*
* @param	InstPtr	Pointer to SLH-DSA instance with SkSeed loaded.
* @param	Output	Output buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input or unbound ops table.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
* @note
* Delegates to FIPS 205 PRF in the SHA2 or SHAKE backend.
*
*************************************************************************************************/
s32 XSlhDsa_HashPrf(XSlhDsa_Instance *InstPtr, u8 *Output)
{
	s32 Status = XASUFW_FAILURE;
	const XSlhDsa_HashOps *Ops = XSlhDsa_HashGetOps(InstPtr);

	/** Validate the bound ops table and the output buffer. */
	if ((Ops == NULL) || (Output == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Compute FIPS 205 PRF(PK.seed, SK.seed, ADRS) using the SK.seed held in the instance. */
	Status = Ops->HashPrf(InstPtr, Output);

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Compute PRF_msg through the bound hash backend.
*
* @param	InstPtr	Pointer to SLH-DSA instance with SkPrf loaded.
* @param	OptRand	Optional randomizer (n bytes).
* @param	Msg	Message buffer.
* @param	MsgLen	Message length in bytes.
* @param	Ctx	Optional context string for pure mode.
* @param	CtxLen	FIPS context length 0..255.
* @param	Output	Output buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input or unbound ops table.
*		- XASUFW_CMD_IN_PROGRESS if the message SHA update is running non-blocking.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
* @note
* Delegates to FIPS 205 PRF_msg in the SHA2 or SHAKE backend.
*
*************************************************************************************************/
s32 XSlhDsa_HashPrfMsg(XSlhDsa_Instance *InstPtr, const u8 *OptRand,
			    const u8 *Msg, u32 MsgLen, const u8 *Ctx,
			    u32 CtxLen, u8 *Output)
{
	s32 Status = XASUFW_FAILURE;
	const XSlhDsa_HashOps *Ops = XSlhDsa_HashGetOps(InstPtr);

	/** Validate the bound ops table and the mandatory buffers; Ctx is optional. */
	if ((Ops == NULL) || (OptRand == NULL) || (Msg == NULL) || (Output == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Compute FIPS 205 PRF_msg(SK.prf, OptRand, M); the backend prepends the pure prefix. */
	Status = Ops->HashPrfMsg(InstPtr, OptRand, Msg, MsgLen, Ctx, CtxLen, Output);

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Apply a WOTS+ chain through the bound hash backend.
*
* @param	InstPtr	Pointer to SLH-DSA instance with Addr loaded.
* @param	Input	Initial chain value (n bytes).
* @param	Cfg	Chain start index and step count.
* @param	Output	Output buffer (n bytes); may alias Input.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input or unbound ops table.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
* @note
* Each chain step invokes F through the SHA2 or SHAKE backend.
*
*************************************************************************************************/
s32 XSlhDsa_Chain(XSlhDsa_Instance *InstPtr, const u8 *Input,
		       const XSlhDsa_ChainCfg *Cfg, u8 *Output)
{
	s32 Status = XASUFW_FAILURE;
	const XSlhDsa_HashOps *Ops = XSlhDsa_HashGetOps(InstPtr);

	/** Validate the bound ops table, the chain configuration and the buffers. */
	if ((Ops == NULL) || (Input == NULL) || (Cfg == NULL) || (Output == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * Apply the WOTS+ chaining function: Cfg->Steps successive F iterations starting from
	 * chain index Cfg->StartIdx. Output may alias Input, so the backend chains in place.
	 */
	Status = Ops->Chain(InstPtr, Input, Cfg, Output);

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Look up the hash ops table for a SLH-DSA hash family.
*
* @param	Family	SHAKE (SHA3/SHAKE256) or SHA2 hash family identifier.
*
* @return
*		Pointer to the static ops table for Family, or NULL if Family is out of range.
*
* @note
* SHAKE maps to XSlhDsa_HashOpsShake; SHA2 maps to XSlhDsa_HashOpsSha2.
*
*************************************************************************************************/
const XSlhDsa_HashOps *XSlhDsa_HashOpsLookup(XSlhDsa_HashFamily Family)
{
	const XSlhDsa_HashOps *Ops = NULL;

	/** Family is unsigned and starts at XSLHDSA_HASH_SHAKE, so only the upper bound matters. */
	if (Family > XSLHDSA_HASH_SHA2) {
		goto END;
	}

	/** Select the static SHAKE or SHA2 ops table for the requested family. */
	Ops = XSlhDsa_HashOpsByFamily[Family];

END:
	return Ops;
}

/*************************************************************************************************/
/**
* @brief	Initialize a hardware hash context from an SLH-DSA instance.
*
* @param	InstPtr	Pointer to SLH-DSA instance with ShaInst and DmaInst set.
* @param	HashCtxPtr	Pointer to hash context to populate.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM if any pointer is NULL or handles are unset.
*
* @note
* ShaInst must reference an ASU XSha engine allocated for SHA2 or SHA3 use.
*
*************************************************************************************************/
s32 XSlhDsa_HashCtxInit(XSlhDsa_Instance *InstPtr, XSlhDsa_HashCtx *HashCtxPtr)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (HashCtxPtr == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Cache the SHA engine and DMA channel handles owned by the SLH-DSA instance. */
	HashCtxPtr->Sha = (XSha *)InstPtr->ShaInst;
	HashCtxPtr->Dma = (XAsufw_Dma *)InstPtr->DmaInst;

	/**
	 * Both engine handles are optional on the instance, so reject a context whose SHA or DMA
	 * engine was never allocated before any streaming call is attempted.
	 */
	if ((HashCtxPtr->Sha == NULL) || (HashCtxPtr->Dma == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Start a SHAKE256 hash operation on the ASU SHA3 engine.
*
* @param	HashCtxPtr	Pointer to initialized hash context.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- XASUFW_SHA_START_FAILED on XSha_Start error (underlying XSha cause preserved).
*
* @note
* Configures XSha for XASU_SHA_MODE_SHAKE256 (ASU SHA3/SHAKE engine).
*
*************************************************************************************************/
s32 XSlhDsa_HashShakeBegin(XSlhDsa_HashCtx *HashCtxPtr)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate the hash context and the bound SHA engine handle. */
	if ((HashCtxPtr == NULL) || (HashCtxPtr->Sha == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Configure the SHA3 engine for SHAKE256 and move it to the started state. */
	Status = XSlhDsa_MapHashStatus(XSha_Start(HashCtxPtr->Sha, XASU_SHA_MODE_SHAKE256),
					    XASUFW_SHA_START_FAILED);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Finish a SHAKE256 operation and squeeze the requested output length.
*
* @param	HashCtxPtr	Pointer to initialized hash context with active SHAKE op.
* @param	Out	Output buffer.
* @param	OutLen	Number of output bytes to squeeze.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- XASUFW_SHA_FINISH_FAILED on XSha_Finish error (underlying XSha cause preserved).
*
* @note
* Uses ASU SHA3/SHAKE256 via XSha_Finish.
*
*************************************************************************************************/
s32 XSlhDsa_HashShakeFinish(XSlhDsa_HashCtx *HashCtxPtr, u8 *Out, u32 OutLen)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate the hash context, the bound engine handles and the output buffer. */
	if ((HashCtxPtr == NULL) || (HashCtxPtr->Sha == NULL) || (HashCtxPtr->Dma == NULL) ||
			(Out == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * Squeeze OutLen bytes of XOF output. NextXofOutput is XASU_FALSE, so the SHA3 engine is
	 * put back under reset once the output is read and this operation ends here.
	 */
	Status = XSlhDsa_MapHashStatus(
			XSha_Finish(HashCtxPtr->Sha, HashCtxPtr->Dma, (u64)(UINTPTR)Out, OutLen,
				    XASU_FALSE),
			XASUFW_SHA_FINISH_FAILED);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute SHAKE256(PK.seed || ADRS || M) over local message data.
*
* @param	InstPtr	Pointer to SLH-DSA instance with PkSeed and Addr loaded.
* @param	Msg	Local message buffer M.
* @param	MsgLen	Message length in bytes.
* @param	Out	Output buffer.
* @param	OutLen	Number of output bytes to squeeze.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* Uses ASU SHA3/SHAKE256 engine.
*
*************************************************************************************************/
s32 XSlhDsa_HashShakePkSeedAddrLocal(XSlhDsa_Instance *InstPtr, const u8 *Msg, u32 MsgLen,
			u8 *Out, u32 OutLen)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate the instance and output buffer; Addr is required because ADRS is absorbed. */
	if ((InstPtr == NULL) || (Out == NULL) || (InstPtr->Addr == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** A message buffer is only mandatory when a non-zero message length is requested. */
	if ((MsgLen > 0U) && (Msg == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Absorb through the shared stream helper with M taken from local memory. */
	Status = XSlhDsa_HashShakePkSeedAddrStream(InstPtr, 0U, Msg, MsgLen, Out, OutLen);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute SHAKE256(PK.seed || ADRS || M) over DMA-resident message data.
*
* @param	InstPtr	Pointer to SLH-DSA instance with PkSeed and Addr loaded.
* @param	MsgAddr	DMA address of message data M.
* @param	MsgLen	Message length in bytes.
* @param	Out	Output buffer.
* @param	OutLen	Number of output bytes to squeeze.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* Uses ASU SHA3/SHAKE256 engine.
*
*************************************************************************************************/
s32 XSlhDsa_HashShakePkSeedAddrDma(XSlhDsa_Instance *InstPtr, u64 MsgAddr, u32 MsgLen,
			u8 *Out, u32 OutLen)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate the instance and output buffer; Addr is required because ADRS is absorbed. */
	if ((InstPtr == NULL) || (Out == NULL) || (InstPtr->Addr == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** A message address is only mandatory when a non-zero message length is requested. */
	if ((MsgLen > 0U) && (MsgAddr == 0U)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Absorb through the shared stream helper; a NULL local pointer selects the DMA path. */
	Status = XSlhDsa_HashShakePkSeedAddrStream(InstPtr, MsgAddr, NULL, MsgLen,
							Out, OutLen);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Feed PK.seed padded to the SHA2 block size for H and T_l prefix.
*
* FIPS 205 SHA2 instantiation: PK.seed || zero-padding to block boundary (Section 11.2).
*
* @param	HashCtxPtr	Pointer to initialized hash context with active SHA2 op.
* @param	InstPtr	Pointer to SLH-DSA instance with PkSeed loaded.
* @param	ShaMode	SHA mode: XASU_SHA_MODE_256 or XASU_SHA_MODE_512.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- XASUFW_SHA_UPDATE_FAIL on XSha_Update error (underlying XSha cause preserved).
*
* @note
* Uses ASU SHA2 engine.
*
*************************************************************************************************/
s32 XSlhDsa_HashSha2UpdatePkSeedPrefix(XSlhDsa_HashCtx *HashCtxPtr,
			XSlhDsa_Instance *InstPtr,
			u32 ShaMode)
{
	static const u8 Zeros[XASUFW_SHA2_384_512_BLOCK_LEN] = {0};
	u32 HashWidth;
	u32 BlockLen;
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. */
	if ((HashCtxPtr == NULL) || (InstPtr == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** PK.seed is n bytes wide; look up the block length it has to be zero padded out to. */
	HashWidth = InstPtr->Param.n;
	Status = XSlhDsa_HashSha2BlockLen(ShaMode, &BlockLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Absorb PK.seed; it is never the last segment, so EndLast stays clear. */
	Status = XSlhDsa_HashUpdate(HashCtxPtr, (u64)(UINTPTR)InstPtr->PkSeed, HashWidth, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * Pad PK.seed with zeros up to the block boundary so that ADRS and M start on a fresh
	 * compression block. Nothing is padded when n already fills the block.
	 */
	if (BlockLen > HashWidth) {
		Status = XSlhDsa_HashUpdate(HashCtxPtr, (u64)(UINTPTR)Zeros,
					    BlockLen - HashWidth, 0U);
		goto END;
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Finish a SHA2 hash and write the first n bytes of the digest.
*
* @param	HashCtxPtr	Pointer to initialized hash context with active SHA2 op.
* @param	ShaMode	SHA mode: XASU_SHA_MODE_256 or XASU_SHA_MODE_512.
* @param	N	Truncation length in bytes (FIPS 205 n).
* @param	Out	Output buffer (N bytes).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- XASUFW_SHA_FINISH_FAILED on XSha_Finish error (underlying XSha cause preserved).
*		- XASUFW_SLHDSA_COPY_FAILED if truncated digest copy to Out fails.
*		- Combined buffer-clear failure status via XAsufw_UpdateBufStatus otherwise.
*
* @note
* Full digest computed on ASU SHA2 engine, then truncated per FIPS 205.
*
*************************************************************************************************/
s32 XSlhDsa_HashSha2FinishTrunc(XSlhDsa_HashCtx *HashCtxPtr, u32 ShaMode, u32 N, u8 *Out)
{
	u8 Digest[XASU_SHA_512_HASH_LEN];
	u32 DigestLen;
	s32 Status = XASUFW_FAILURE;

	/** Validate the hash context, the bound engine handles and the output buffer. */
	if ((HashCtxPtr == NULL) || (HashCtxPtr->Sha == NULL) || (HashCtxPtr->Dma == NULL) ||
			(Out == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Determine the untruncated digest length produced by the active SHA2 mode. */
	DigestLen = 0U;
	Status = XSlhDsa_HashSha2DigestLen(ShaMode, &DigestLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** The engine emits only the complete digest, so read it into a local buffer first. */
	Status = XSha_Finish(HashCtxPtr->Sha, HashCtxPtr->Dma, (u64)(UINTPTR)Digest, DigestLen,
			XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SHA_FINISH_FAILED);
		goto END;
	}

	/** Keep the leading N bytes, which is the FIPS 205 truncation of the digest to n. */
	Status = Xil_SMemCpy(Out, N, Digest, N, N);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_COPY_FAILED;
		goto END;
	}

END:
	/**
	 * Zeroize the local digest buffer on every exit path, including the error paths, and
	 * merge the clear result into Status so a failed scrub is not lost.
	 */
	Status = XAsufw_UpdateBufStatus(Status,
			Xil_SecureZeroize(Digest, (u32)sizeof(Digest)));
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Finish a SHA2 hash and write the full digest.
*
* @param	HashCtxPtr	Pointer to initialized hash context with active SHA2 op.
* @param	ShaMode	SHA mode: XASU_SHA_MODE_256 or XASU_SHA_MODE_512.
* @param	Out	Output buffer sized for the full digest.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- XASUFW_SHA_FINISH_FAILED on XSha_Finish error (underlying XSha cause preserved).
*
* @note
* Used by MGF1 and HMAC on the ASU SHA2 engine.
*
*************************************************************************************************/
s32 XSlhDsa_HashSha2FinishFull(XSlhDsa_HashCtx *HashCtxPtr, u32 ShaMode, u8 *Out)
{
	u32 DigestLen;
	s32 Status = XASUFW_FAILURE;

	/** Validate the hash context, the bound engine handles and the output buffer. */
	if ((HashCtxPtr == NULL) || (HashCtxPtr->Sha == NULL) || (HashCtxPtr->Dma == NULL) ||
			(Out == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Determine the full digest length produced by the active SHA2 mode. */
	DigestLen = 0U;
	Status = XSlhDsa_HashSha2DigestLen(ShaMode, &DigestLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** MGF1 and HMAC consume the whole digest, so write it untruncated to the caller buffer. */
	Status = XSlhDsa_MapHashStatus(
			XSha_Finish(HashCtxPtr->Sha, HashCtxPtr->Dma, (u64)(UINTPTR)Out, DigestLen,
				    XASU_FALSE),
			XASUFW_SHA_FINISH_FAILED);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute truncated SHA2(PK.seed || ADRS || M) over local message data.
*
* @param	InstPtr	Pointer to SLH-DSA instance with PkSeed and Addr loaded.
* @param	Msg	Local message buffer M.
* @param	MsgLen	Message length in bytes.
* @param	ShaMode	SHA mode: XASU_SHA_MODE_256 or XASU_SHA_MODE_512.
* @param	Out	Output buffer (N bytes).
* @param	N	Truncation length in bytes (FIPS 205 n).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* Uses ASU SHA2 engine.
*
*************************************************************************************************/
s32 XSlhDsa_HashSha2PkSeedAddrLocal(XSlhDsa_Instance *InstPtr, const u8 *Msg, u32 MsgLen,
			u32 ShaMode, u8 *Out, u32 N)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate the instance and output buffer; Addr feeds the compressed ADRS field. */
	if ((InstPtr == NULL) || (Out == NULL) || (InstPtr->Addr == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** A message buffer is only mandatory when a non-zero message length is requested. */
	if ((MsgLen > 0U) && (Msg == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Hash through the shared stream helper with M taken from local memory. */
	Status = XSlhDsa_HashSha2PkSeedAddrStream(InstPtr, 0U, Msg, MsgLen, ShaMode, Out, N);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Start a SHA2 hash operation on the ASU SHA2 engine.
*
* @param	HashCtxPtr	Pointer to initialized hash context.
* @param	ShaMode	SHA mode: XASU_SHA_MODE_256 or XASU_SHA_MODE_512.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- XASUFW_SHA_START_FAILED on XSha_Start error (underlying XSha cause preserved).
*
* @note
* Configures XSha for the requested SHA2 mode (not SHA3/SHAKE).
*
*************************************************************************************************/
s32 XSlhDsa_HashSha2Begin(XSlhDsa_HashCtx *HashCtxPtr, u32 ShaMode)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate the hash context and the bound SHA engine handle. */
	if ((HashCtxPtr == NULL) || (HashCtxPtr->Sha == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * Configure the ASU SHA2 engine for the requested mode and move it to the started state.
	 * XSha_Start rejects a mode the bound engine cannot serve, which surfaces here as
	 * XASUFW_SHA_START_FAILED with the XSha cause preserved.
	 */
	Status = XSlhDsa_MapHashStatus(XSha_Start(HashCtxPtr->Sha, ShaMode),
					    XASUFW_SHA_START_FAILED);

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Compute H_msg for SHA2 parameter sets.
*
* @param	InstPtr	Pointer to SLH-DSA instance with signature and message fields set.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- XASUFW_CMD_IN_PROGRESS if the message SHA update is running non-blocking.
*		- XASUFW_SLHDSA_COPY_FAILED, if the randomizer R or PK.seed copy into the
*		  MGF1 seed fails, or if an MGF1 output block copy fails.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED,
*		  XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
* @note
* Writes m-byte digest to InstPtr->MsgDigest via FIPS 205 H_msg (Section 11.2).
* Uses ASU SHA2 engine plus MGF1-SHA2.
*
*************************************************************************************************/
s32 XSlhDsa_Sha2HashMsg(XSlhDsa_Instance *InstPtr)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. The first n bytes at SigAddr hold the randomizer R. */
	if ((InstPtr == NULL) || (InstPtr->SigAddr == 0U) ||
			(InstPtr->MsgDigest == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Compute H_msg with the SHA2 mode selected from n into InstPtr->MsgDigest. */
	Status = XSlhDsa_Sha2HashMsgImpl(InstPtr,
					    XSlhDsa_Sha2ModeHtl(InstPtr->Param.n),
					    InstPtr->MsgDigest);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute F for SHA2 parameter sets.
*
* @param	InstPtr	Pointer to SLH-DSA instance with PkSeed and Addr loaded.
* @param	Input	Input buffer M1 (n bytes).
* @param	Output	Output buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* FIPS 205 F via truncated SHA-256. Uses ASU SHA2 engine (XASU_SHA_MODE_256).
*
*************************************************************************************************/
s32 XSlhDsa_Sha2HashF(XSlhDsa_Instance *InstPtr, const u8 *Input,
			   u8 *Output)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (Input == NULL) || (Output == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * FIPS 205 F = Trunc_n(SHA-256(PK.seed || toByte(0, 64 - n) || ADRS^c || M1)).
	 * F uses SHA-256 for every SHA2 parameter set, irrespective of n.
	 */
	Status = XSlhDsa_HashSha2PkSeedAddrLocal(InstPtr, Input, InstPtr->Param.n,
						    XSLHDSA_SHA2_MODE_FPRF, Output,
						    InstPtr->Param.n);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute H for SHA2 parameter sets.
*
* @param	InstPtr	Pointer to SLH-DSA instance with PkSeed and Addr loaded.
* @param	Left	Left child node (n bytes).
* @param	Right	Right child node (n bytes).
* @param	Output	Output buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* FIPS 205 H with M2 = Left || Right. Uses ASU SHA2 engine (SHA-256 or SHA-512 per n).
*
*************************************************************************************************/
s32 XSlhDsa_Sha2HashH(XSlhDsa_Instance *InstPtr, const u8 *Left,
			   const u8 *Right, u8 *Output)
{
	XSlhDsa_HashCtx HashCtx;
	u8 Sha2AddrField[XSLHDSA_SHA2_ADDR_FIELD_BYTES];
	u32 ShaMode;
	u32 HashWidth;
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (Left == NULL) || (Right == NULL) || (Output == NULL) ||
			(InstPtr->Addr == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * Select SHA-256 or SHA-512 from n and compress the 32-byte ADRS to the 22-byte
	 * SHA2 form (ADRS^c) that the SHA2 instantiation absorbs.
	 */
	HashWidth = InstPtr->Param.n;
	ShaMode = XSlhDsa_Sha2ModeHtl(HashWidth);
	XSlhDsa_AddrPackSha2Compact(InstPtr->Addr, Sha2AddrField);

	Status = XSlhDsa_HashCtxInit(InstPtr, &HashCtx);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * FIPS 205 Section 11.2 H = Trunc_n(SHA-X(PK.seed || toByte(0, BlockLen - n) ||
	 * ADRS^c || Left || Right)), absorbed in that order.
	 */
	Status = XSlhDsa_HashSha2Begin(&HashCtx, ShaMode);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** - Absorb PK.seed zero padded up to the SHA2 block length. */
	Status = XSlhDsa_HashSha2UpdatePkSeedPrefix(&HashCtx, InstPtr, ShaMode);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** - Absorb the compressed address. */
	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)Sha2AddrField,
				    XSLHDSA_SHA2_ADDR_FIELD_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** - Absorb M2 = Left || Right; the right node is flagged as the last segment. */
	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)Left, HashWidth, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)Right, HashWidth, 1U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** - Truncate the digest to the leading n bytes. */
	Status = XSlhDsa_HashSha2FinishTrunc(&HashCtx, ShaMode, HashWidth, Output);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute T_l for SHA2 parameter sets.
*
* @param	InstPtr	Pointer to SLH-DSA instance with PkSeed and Addr loaded.
* @param	Input	Input buffer M.
* @param	InputLen	Input length in bytes.
* @param	Output	Output buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* FIPS 205 T_l via truncated SHA2. Uses ASU SHA2 engine (SHA-256 or SHA-512 per n).
*
*************************************************************************************************/
s32 XSlhDsa_Sha2HashTl(XSlhDsa_Instance *InstPtr, const u8 *Input,
			    u32 InputLen, u8 *Output)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (Input == NULL) || (Output == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * FIPS 205 T_len = Trunc_n(SHA-X(PK.seed || toByte(0, BlockLen - n) || ADRS^c || M)).
	 * Unlike F, T_len follows n in choosing SHA-256 or SHA-512.
	 */
	Status = XSlhDsa_HashSha2PkSeedAddrLocal(InstPtr, Input, InputLen,
						    XSlhDsa_Sha2ModeHtl(InstPtr->Param.n),
						    Output, InstPtr->Param.n);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute PRF for SHA2 parameter sets.
*
* @param	InstPtr	Pointer to SLH-DSA instance with SkSeed loaded.
* @param	Output	Output buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* FIPS 205 PRF = F(PK.seed, ADRS, SK.seed). Uses ASU SHA2 engine (XASU_SHA_MODE_256).
*
*************************************************************************************************/
s32 XSlhDsa_Sha2HashPrf(XSlhDsa_Instance *InstPtr, u8 *Output)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (Output == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * FIPS 205 PRF is F over the secret seed, so it is likewise fixed to SHA-256:
	 * Trunc_n(SHA-256(PK.seed || toByte(0, 64 - n) || ADRS^c || SK.seed)).
	 */
	Status = XSlhDsa_HashSha2PkSeedAddrLocal(InstPtr, InstPtr->SkSeed,
						      InstPtr->Param.n,
						      XSLHDSA_SHA2_MODE_FPRF, Output,
						      InstPtr->Param.n);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute PRF_msg for SHA2 parameter sets.
*
* @param	InstPtr	Pointer to SLH-DSA instance with SkPrf loaded.
* @param	OptRand	Optional randomizer (n bytes).
* @param	Msg	Message buffer.
* @param	MsgLen	Message length in bytes.
* @param	Ctx	Optional context string for pure mode.
* @param	CtxLen	FIPS context length 0..255.
* @param	Output	Output buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- XASUFW_CMD_IN_PROGRESS if the message SHA update is running non-blocking.
*		- XASUFW_SLHDSA_COPY_FAILED, if the SK.prf copy into the HMAC key
*		  block fails.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED,
*		  XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
* @note
* FIPS 205 PRF_msg via HMAC-SHA2 on ASU SHA2 engine.
*
*************************************************************************************************/
s32 XSlhDsa_Sha2HashPrfMsg(XSlhDsa_Instance *InstPtr, const u8 *OptRand,
				const u8 *Msg, u32 MsgLen,
				const u8 *Ctx, u32 CtxLen, u8 *Output)
{
	u32 ShaMode;
	u32 BlockLen;
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (OptRand == NULL) || (Msg == NULL) || (Output == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** Select the SHA2 mode from n and the HMAC block length that sizes the key blocks. */
	ShaMode = XSlhDsa_Sha2ModeHtl(InstPtr->Param.n);
	Status = XSlhDsa_Sha2HmacBlockLen(ShaMode, &BlockLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** FIPS 205 PRF_msg = Trunc_n(HMAC-SHA-X(SK.prf, OptRand || [ctx prefix] || M)). */
	Status = XSlhDsa_Sha2HashPrfMsgImpl(InstPtr, OptRand, Msg, MsgLen, Ctx, CtxLen,
						 InstPtr->Param.n, BlockLen, ShaMode, Output);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Apply a WOTS+ chain of F hashes for SHA2 parameter sets.
*
* @param	InstPtr	Pointer to SLH-DSA instance with Addr loaded.
* @param	Input	Initial chain value (n bytes).
* @param	Cfg	Chain start index and step count.
* @param	Output	Output buffer (n bytes); may alias Input.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- XASUFW_SLHDSA_COPY_FAILED, if the initial chain value copy fails.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* FIPS 205 chain function; each step invokes F on ASU SHA2 engine.
*
*************************************************************************************************/
s32 XSlhDsa_Sha2Chain(XSlhDsa_Instance *InstPtr, const u8 *Input,
			   const XSlhDsa_ChainCfg *Cfg, u8 *Output)
{
	u32 StepIdx;
	u32 HashWidth;
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (Input == NULL) || (Cfg == NULL) || (Output == NULL) ||
			(InstPtr->Addr == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * Seed the output with the initial chain value so that every subsequent F is applied
	 * in place, which also covers the caller aliasing Output with Input.
	 */
	HashWidth = InstPtr->Param.n;
	Status = Xil_SMemCpy(Output, HashWidth, Input, HashWidth, HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_COPY_FAILED;
		goto END;
	}

	/** A zero-step chain returns the initial value unchanged. */
	if (Cfg->Steps == 0U) {
		Status = XASUFW_SUCCESS;
		goto END;
	}

	/**
	 * FIPS 205 chain(): apply F for hash slots StartIdx to StartIdx + Steps - 1. The
	 * ADRS hash-slot field must be refreshed for every step so each F call is domain
	 * separated from its neighbours.
	 */
	for (StepIdx = 0U; StepIdx < Cfg->Steps; StepIdx++) {
		XSlhDsa_AddrSetHashSlot(InstPtr->Addr, Cfg->StartIdx + StepIdx);
		Status = XSlhDsa_Sha2HashF(InstPtr, Output, Output);
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
* @brief	Compute H_msg for SHAKE parameter sets.
*
* @param	InstPtr	Pointer to SLH-DSA instance with signature and message fields set.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- XASUFW_CMD_IN_PROGRESS if the message SHA update is running non-blocking.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* FIPS 205 H_msg (Section 11.1): SHAKE256 absorb then squeeze m bytes to MsgDigest.
* Uses ASU SHA3/SHAKE256 engine.
*
*************************************************************************************************/
s32 XSlhDsa_ShakeHashMsg(XSlhDsa_Instance *InstPtr)
{
	XSlhDsa_HashCtx HashCtx;
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. The first n bytes at SigAddr hold the randomizer R. */
	if ((InstPtr == NULL) || (InstPtr->SigAddr == 0U) ||
			(InstPtr->MsgDigest == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	Status = XSlhDsa_HashCtxInit(InstPtr, &HashCtx);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * A previous message update already absorbed R || PK.seed || PK.root || prefix || M.
	 * Skip Start and the absorb, and squeeze once DMA has completed.
	 */
	if (XSlhDsa_YieldIsHashDmaResume(InstPtr) != 0U) {
		XSlhDsa_YieldClearHashDmaResume(InstPtr);
		goto FINISH;
	}

	/**
	 * FIPS 205 Section 11.1 H_msg = SHAKE256(R || PK.seed || PK.root || [ctx prefix] ||
	 * M, 8m); no MGF1 is needed because the digest length comes from the squeeze.
	 */
	Status = XSlhDsa_HashShakeBegin(&HashCtx);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** - Absorb the H_msg input sequence. */
	Status = XSlhDsa_ShakeFeedHmsgInputs(&HashCtx, InstPtr);
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		XSlhDsa_YieldSetHashDmaResume(InstPtr);
		goto END;
	}
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

FINISH:
	/** - Squeeze m bytes into the message digest buffer. */
	Status = XSlhDsa_HashShakeFinish(&HashCtx, InstPtr->MsgDigest, InstPtr->Param.m);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute F for SHAKE parameter sets.
*
* @param	InstPtr	Pointer to SLH-DSA instance with PkSeed and Addr loaded.
* @param	Input	Input buffer M1 (n bytes).
* @param	Output	Output buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* FIPS 205 F via SHAKE256(PK.seed || ADRS || M1). Uses ASU SHA3/SHAKE256 engine.
*
*************************************************************************************************/
s32 XSlhDsa_ShakeHashF(XSlhDsa_Instance *InstPtr, const u8 *Input,
			    u8 *Output)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (Input == NULL) || (Output == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * FIPS 205 F = SHAKE256(PK.seed || ADRS || M1, 8n). The SHAKE instantiation absorbs
	 * the full 32-byte ADRS and needs no block padding of PK.seed.
	 */
	Status = XSlhDsa_HashShakePkSeedAddrLocal(InstPtr, Input, InstPtr->Param.n, Output,
						       InstPtr->Param.n);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute H for SHAKE parameter sets.
*
* @param	InstPtr	Pointer to SLH-DSA instance with PkSeed and Addr loaded.
* @param	Left	Left child node (n bytes).
* @param	Right	Right child node (n bytes).
* @param	Output	Output buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* FIPS 205 H with M2 = Left || Right via SHAKE256. Uses ASU SHA3/SHAKE256 engine.
*
*************************************************************************************************/
s32 XSlhDsa_ShakeHashH(XSlhDsa_Instance *InstPtr, const u8 *Left,
			    const u8 *Right, u8 *Output)
{
	XSlhDsa_HashCtx HashCtx;
	u32 HashWidth;
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (Left == NULL) || (Right == NULL) || (Output == NULL) ||
			(InstPtr->Addr == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	HashWidth = InstPtr->Param.n;

	Status = XSlhDsa_HashCtxInit(InstPtr, &HashCtx);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * FIPS 205 Section 11.1 H = SHAKE256(PK.seed || ADRS || Left || Right, 8n),
	 * absorbed in that order.
	 */
	Status = XSlhDsa_HashShakeBegin(&HashCtx);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** - Absorb PK.seed followed by the full 32-byte ADRS. */
	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)InstPtr->PkSeed, HashWidth, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)InstPtr->Addr->byte,
				    XSLHDSA_ADDR_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** - Absorb M2 = Left || Right; the right node is flagged as the last segment. */
	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)Left, HashWidth, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)Right, HashWidth, 1U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** - Squeeze n bytes to form the parent node. */
	Status = XSlhDsa_HashShakeFinish(&HashCtx, Output, HashWidth);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute T_l for SHAKE parameter sets.
*
* @param	InstPtr	Pointer to SLH-DSA instance with PkSeed and Addr loaded.
* @param	Input	Input buffer M.
* @param	InputLen	Input length in bytes.
* @param	Output	Output buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* FIPS 205 T_l via SHAKE256(PK.seed || ADRS || M). Uses ASU SHA3/SHAKE256 engine.
*
*************************************************************************************************/
s32 XSlhDsa_ShakeHashTl(XSlhDsa_Instance *InstPtr, const u8 *Input,
			     u32 InputLen, u8 *Output)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (Input == NULL) || (Output == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * FIPS 205 T_len = SHAKE256(PK.seed || ADRS || M, 8n). T_len shares the F absorb
	 * sequence and differs only in accepting an M longer than n bytes.
	 */
	Status = XSlhDsa_HashShakePkSeedAddrLocal(InstPtr, Input, InputLen, Output,
						       InstPtr->Param.n);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute PRF for SHAKE parameter sets.
*
* @param	InstPtr	Pointer to SLH-DSA instance with SkSeed loaded.
* @param	Output	Output buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* FIPS 205 PRF = F(PK.seed, ADRS, SK.seed). Uses ASU SHA3/SHAKE256 engine.
*
*************************************************************************************************/
s32 XSlhDsa_ShakeHashPrf(XSlhDsa_Instance *InstPtr, u8 *Output)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (Output == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * FIPS 205 PRF = SHAKE256(PK.seed || ADRS || SK.seed, 8n). For the SHAKE parameter
	 * sets this is exactly F with SK.seed as M1, so F is reused directly.
	 */
	Status = XSlhDsa_ShakeHashF(InstPtr, InstPtr->SkSeed, Output);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute PRF_msg for SHAKE parameter sets.
*
* @param	InstPtr	Pointer to SLH-DSA instance with SkPrf loaded.
* @param	OptRand	Optional randomizer (n bytes).
* @param	Msg	Message buffer.
* @param	MsgLen	Message length in bytes.
* @param	Ctx	Optional context string for pure mode.
* @param	CtxLen	FIPS context length 0..255.
* @param	Output	Output buffer (n bytes).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- XASUFW_CMD_IN_PROGRESS if the message SHA update is running non-blocking.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* FIPS 205 PRF_msg (Section 11.1): SHAKE256(SK.prf || OptRand || [pure ctx] || M).
* Uses ASU SHA3/SHAKE256 engine.
*
*************************************************************************************************/
s32 XSlhDsa_ShakeHashPrfMsg(XSlhDsa_Instance *InstPtr, const u8 *OptRand,
				 const u8 *Msg, u32 MsgLen,
				 const u8 *Ctx, u32 CtxLen,
				 u8 *Output)
{
	XSlhDsa_HashCtx HashCtx;
	u32 HashWidth;
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (OptRand == NULL) || (Msg == NULL) || (Output == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	HashWidth = InstPtr->Param.n;

	Status = XSlhDsa_HashCtxInit(InstPtr, &HashCtx);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * A previous message update already absorbed SK.prf || OptRand || prefix || M.
	 * Skip Start and the absorb, and squeeze once DMA has completed.
	 */
	if (XSlhDsa_YieldIsHashDmaResume(InstPtr) != 0U) {
		XSlhDsa_YieldClearHashDmaResume(InstPtr);
		goto FINISH;
	}

	/**
	 * FIPS 205 Section 11.1 PRF_msg = SHAKE256(SK.prf || OptRand || [ctx prefix] || M,
	 * 8n); keying by absorbing SK.prf first replaces the HMAC used by SHA2.
	 */
	Status = XSlhDsa_HashShakeBegin(&HashCtx);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** - Absorb the secret PRF key SK.prf followed by the randomizer OptRand. */
	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)InstPtr->SkPrf, HashWidth, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)OptRand, HashWidth, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - Absorb the domain separator prefix. Pre-hash mode reuses the prefix already
	 *   staged in WorkMem, which carries the OID; pure mode emits DS || ctx_len || ctx
	 *   built from the caller supplied context.
	 */
	if (InstPtr->PreHashMode == XASU_SLHDSA_MSG_PREHASH) {
		if (InstPtr->CtxPrefixLen > 0U) {
			Status = XSlhDsa_HashUpdate(&HashCtx,
						    (u64)(UINTPTR)InstPtr->WorkMem,
						    InstPtr->CtxPrefixLen, 0U);
			if (Status != XASUFW_SUCCESS) {
				goto END;
			}
		}
	}
	else {
		Status = XSlhDsa_HashShakeAppendPureContext(&HashCtx, Ctx, CtxLen);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	/**
	 * - Absorb the message and close the absorb phase, preferring the client supplied
	 *   PH(M) digest when the instance is in pre-hash mode.
	 */
	if (InstPtr->MsgLocalPtr != NULL) {
		Status = XSlhDsa_HashUpdateMsg(&HashCtx,
					       (u64)(UINTPTR)InstPtr->MsgLocalPtr,
					       InstPtr->MsgLocalLen, 1U);
	}
	else {
		Status = XSlhDsa_HashUpdateMsg(&HashCtx, (u64)(UINTPTR)Msg, MsgLen, 1U);
	}

	if (Status == XASUFW_CMD_IN_PROGRESS) {
		XSlhDsa_YieldSetHashDmaResume(InstPtr);
		goto END;
	}
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

FINISH:
	/** - Squeeze n bytes to form the signature randomizer R. */
	Status = XSlhDsa_HashShakeFinish(&HashCtx, Output, HashWidth);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Apply a WOTS+ chain of F hashes for SHAKE parameter sets.
*
* @param	InstPtr	Pointer to SLH-DSA instance with Addr loaded.
* @param	Input	Initial chain value (n bytes).
* @param	Cfg	Chain start index and step count.
* @param	Output	Output buffer (n bytes); may alias Input.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- XASUFW_SLHDSA_COPY_FAILED, if the initial chain value copy fails.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* FIPS 205 chain function; each step invokes F on ASU SHA3/SHAKE256 engine.
*
*************************************************************************************************/
s32 XSlhDsa_ShakeChain(XSlhDsa_Instance *InstPtr, const u8 *Input,
			    const XSlhDsa_ChainCfg *Cfg, u8 *Output)
{
	u32 StepIdx;
	u32 HashWidth;
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. */
	if ((InstPtr == NULL) || (Input == NULL) || (Cfg == NULL) || (Output == NULL) ||
			(InstPtr->Addr == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * Seed the output with the initial chain value so that every subsequent F is applied
	 * in place, which also covers the caller aliasing Output with Input.
	 */
	HashWidth = InstPtr->Param.n;
	Status = Xil_SMemCpy(Output, HashWidth, Input, HashWidth, HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_COPY_FAILED;
		goto END;
	}

	/** A zero-step chain returns the initial value unchanged. */
	if (Cfg->Steps == 0U) {
		Status = XASUFW_SUCCESS;
		goto END;
	}

	/**
	 * FIPS 205 chain(): apply F for hash slots StartIdx to StartIdx + Steps - 1. The
	 * ADRS hash-slot field must be refreshed for every step so each F call is domain
	 * separated from its neighbours.
	 */
	for (StepIdx = 0U; StepIdx < Cfg->Steps; StepIdx++) {
		XSlhDsa_AddrSetHashSlot(InstPtr->Addr, Cfg->StartIdx + StepIdx);
		Status = XSlhDsa_ShakeHashF(InstPtr, Output, Output);
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
* @brief	Retrieve the bound hash ops table from an SLH-DSA instance.
*
* @param	InstPtr	Pointer to SLH-DSA instance.
*
* @return
*		Pointer to the bound ops table, or NULL if InstPtr or HashOps is NULL.
*
* @note
* HashOps is set during XSlhDsa_BindParam().
*
*************************************************************************************************/
static const XSlhDsa_HashOps *XSlhDsa_HashGetOps(XSlhDsa_Instance *InstPtr)
{
	const XSlhDsa_HashOps *Ops = NULL;

	/**
	 * The ops table is bound by XSlhDsa_BindParam(). A NULL table means the instance was
	 * never initialized with a parameter set, so no backend can be selected.
	 */
	if ((InstPtr == NULL) || (InstPtr->HashOps == NULL)) {
		goto END;
	}

	Ops = InstPtr->HashOps;

END:
	return Ops;
}
/*************************************************************************************************/
/**
* @brief	Combine an XSha status with the matching ASUFW SHA operation error tag.
*
* @param	Status	Status returned by XSha_Start, XSha_Update, or XSha_Finish.
* @param	Error	ASUFW SHA tag: XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or
*			XASUFW_SHA_FINISH_FAILED.
*
* @return
*		- XASUFW_SUCCESS if Status is XASUFW_SUCCESS.
*		- Otherwise XAsufw_UpdateErrorStatus(Status, Error), preserving the underlying
*		  XSha cause (for example XASUFW_SHA_STATE_MISMATCH_ERROR or
*		  XASUFW_SHA_INVALID_HASH_SIZE) alongside the operation tag.
*
*************************************************************************************************/
static s32 XSlhDsa_MapHashStatus(s32 Status, s32 Error)
{
	/** Append the stage tag to the next free error field so the XSha cause stays decodable. */
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, Error);
	}

	return Status;
}

/*************************************************************************************************/
/**
* @brief	Feed a SHA2 or SHAKE segment, optionally allowing a non-blocking DMA wait.
*
* @param	HashCtxPtr	Pointer to initialized hash context.
* @param	Addr	DMA address of input data.
* @param	Len	Input length in bytes.
* @param	EndLast	Non-zero if this update completes the current absorb segment.
* @param	AllowInProgress	Non-zero to pass XASUFW_CMD_IN_PROGRESS through (message
*				absorb only).
*
* @return
*		- XASUFW_SUCCESS on success (including Len == 0).
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- XASUFW_CMD_IN_PROGRESS if AllowInProgress is set and Len is above
*		  XASUFW_DMA_BLOCKING_SIZE.
*		- XASUFW_SHA_UPDATE_FAIL on XSha_Update error, or if AllowInProgress is
*		  clear and Len is above XASUFW_DMA_BLOCKING_SIZE.
*
*************************************************************************************************/
static s32 XSlhDsa_HashEngineUpdate(XSlhDsa_HashCtx *HashCtxPtr, u64 Addr, u32 Len,
				    u32 EndLast, u32 AllowInProgress)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate the hash context and the bound SHA engine and DMA handles. */
	if ((HashCtxPtr == NULL) || (HashCtxPtr->Sha == NULL) || (HashCtxPtr->Dma == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** A zero length update is a no-op; no transfer is started and the engine state holds. */
	if (Len == 0U) {
		Status = XASUFW_SUCCESS;
		goto END;
	}

	/** F, H, T_l and PRF must not start a non-blocking DMA. */
	if ((AllowInProgress == 0U) && (Len > XASUFW_DMA_BLOCKING_SIZE)) {
		Status = XASUFW_SHA_UPDATE_FAIL;
		goto END;
	}

	Status = XSha_Update(HashCtxPtr->Sha, HashCtxPtr->Dma, Addr, Len, EndLast);
	if ((AllowInProgress != 0U) && (Status == XASUFW_CMD_IN_PROGRESS)) {
		goto END;
	}

	Status = XSlhDsa_MapHashStatus(Status, XASUFW_SHA_UPDATE_FAIL);

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	Feed input data into an active hash, which must not wait on DMA.
*
* @param	HashCtxPtr	Pointer to initialized hash context with an active op.
* @param	Addr	DMA address of input data.
* @param	Len	Input length in bytes.
* @param	EndLast	Non-zero if this update completes the current absorb segment.
*
* @return
*		- XASUFW_SUCCESS on success (including Len == 0).
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- XASUFW_SHA_UPDATE_FAIL on XSha_Update error, or if Len is above
*		  XASUFW_DMA_BLOCKING_SIZE (F, H, T_l and PRF must not wait on DMA).
*
* @note
*	SHAKE256 and SHA2 share this path because the engine and DMA handles come from the hash
*	context, so the absorb is identical for both FIPS 205 instantiations. Message absorbs
*	that may wait use XSlhDsa_HashUpdateMsg().
*
*************************************************************************************************/
s32 XSlhDsa_HashUpdate(XSlhDsa_HashCtx *HashCtxPtr, u64 Addr, u32 Len, u32 EndLast)
{
	return XSlhDsa_HashEngineUpdate(HashCtxPtr, Addr, Len, EndLast, 0U);
}

/*************************************************************************************************/
/**
* @brief	Absorb a message segment that may wait on a non-blocking DMA.
*
* @param	HashCtxPtr	Pointer to initialized hash context with an active op.
* @param	Addr	DMA address of the message segment.
* @param	Len	Segment length in bytes.
* @param	EndLast	Non-zero if this update completes the current absorb segment.
*
* @return
*		- XASUFW_SUCCESS on success (including Len == 0).
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- XASUFW_CMD_IN_PROGRESS if Len is above XASUFW_DMA_BLOCKING_SIZE, so the caller
*		  must flag the resume and return the wait to the handler.
*		- XASUFW_SHA_UPDATE_FAIL on XSha_Update error.
*
* @note
*	Only H_msg and PRF_msg absorb caller-supplied message data, so only they may park on a
*	non-blocking DMA. Every other absorb uses XSlhDsa_HashUpdate().
*
*************************************************************************************************/
static inline s32 XSlhDsa_HashUpdateMsg(XSlhDsa_HashCtx *HashCtxPtr, u64 Addr,
					u32 Len, u32 EndLast)
{
	return XSlhDsa_HashEngineUpdate(HashCtxPtr, Addr, Len, EndLast, 1U);
}

/*************************************************************************************************/
/**
* @brief	Zeroize a buffer without dropping a DMA wait.
*
* @param	InstPtr	Pointer to the SLH-DSA instance.
* @param	Status	Current function status.
* @param	Buf	Buffer to zeroize.
* @param	Len	Buffer length in bytes.
*
* @return
*		- Status unchanged if it is XASUFW_CMD_IN_PROGRESS. A wipe failure is recorded
*		  on the session and reported by XSlhDsa_YieldSessionEnd.
*		- Combined buffer-clear status via XAsufw_UpdateBufStatus otherwise.
*
* @note
*	Folding the buffer-clear bits into XASUFW_CMD_IN_PROGRESS would stop the status comparing
*	equal to it, so the command would be completed and its resources released while the SHA
*	message DMA is still running. The failure is therefore deferred, not discarded.
*	The caller validates InstPtr before the wipe.
*
*************************************************************************************************/
static inline s32 XSlhDsa_HashZeroizeKeepWait(XSlhDsa_Instance *InstPtr, s32 Status, u8 *Buf,
					      u32 Len)
{
	s32 ClearStatus = Xil_SecureZeroize(Buf, Len);

	if (Status != XASUFW_CMD_IN_PROGRESS) {
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
	}
	else if (ClearStatus != XASUFW_SUCCESS) {
		InstPtr->Yield.Session.HashWipeFail = 1U;
	}
	else {
		/* Do nothing */
	}

	return Status;
}
/*************************************************************************************************/
/**
* @brief	Internal: SHAKE256(PK.seed || ADRS || M) over local or DMA message data.
*
* FIPS 205 SHAKE instantiation absorb sequence for F, H, T_l, and PRF (Section 11.1).
*
* @param	InstPtr	Pointer to SLH-DSA instance with PkSeed and Addr loaded.
* @param	MsgAddr	DMA address of M (used when MsgLocal is NULL).
* @param	MsgLocal	Local pointer to M (preferred when non-NULL).
* @param	MsgLen	Message length in bytes.
* @param	Out	Output buffer.
* @param	OutLen	Number of output bytes to squeeze.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* Uses ASU SHA3/SHAKE256 engine throughout.
*
*************************************************************************************************/
static s32 XSlhDsa_HashShakePkSeedAddrStream(XSlhDsa_Instance *InstPtr,
			u64 MsgAddr, const u8 *MsgLocal,
			u32 MsgLen, u8 *Out, u32 OutLen)
{
	XSlhDsa_HashCtx HashCtx;
	s32 Status = XASUFW_FAILURE;

	/** Bind the SHA3 engine and DMA handles of the instance to a local streaming context. */
	Status = XSlhDsa_HashCtxInit(InstPtr, &HashCtx);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Start a fresh SHAKE256 absorb on the engine. */
	Status = XSlhDsa_HashShakeBegin(&HashCtx);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * Absorb the input in the order fixed by FIPS 205, starting with PK.seed (n bytes). The
	 * engine consumes segments as they are pushed, so the sequence cannot be reordered once
	 * the operation is started.
	 */
	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)InstPtr->PkSeed, InstPtr->Param.n, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Absorb the full 32-byte ADRS; the SHAKE instantiation uses the uncompressed address. */
	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)InstPtr->Addr->byte,
				    XSLHDSA_ADDR_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Absorb M as the last segment; a non-NULL MsgLocal wins over the DMA address. */
	if (MsgLen > 0U) {
		if (MsgLocal != NULL) {
			Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)MsgLocal, MsgLen, 1U);
		}
		else {
			Status = XSlhDsa_HashUpdate(&HashCtx, MsgAddr, MsgLen, 1U);
		}
	}

	/** A single check covers both branches of the message absorb. */
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * Squeeze OutLen bytes. When MsgLen is zero no segment was flagged as last, and
	 * XSha_Finish closes the absorb with software padding instead.
	 */
	Status = XSlhDsa_HashShakeFinish(&HashCtx, Out, OutLen);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Return the full digest length for a SHA2 mode.
*
* @param	ShaMode	XASU_SHA_MODE_256 or XASU_SHA_MODE_512.
* @param	DigestLenPtr	Pointer to store the digest length in bytes.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM if DigestLenPtr is NULL or ShaMode is unsupported.
*
* @note
* Used internally for finish and MGF1 block sizing. Only SHA-256 and SHA-512 are accepted.
*
*************************************************************************************************/
static s32 XSlhDsa_HashSha2DigestLen(u32 ShaMode, u32 *DigestLenPtr)
{
	s32 Status = XASUFW_SLHDSA_INVALID_PARAM;

	/** Validate output pointer. */
	if (DigestLenPtr == NULL) {
		goto END;
	}

	/** Only SHA-256 and SHA-512 are used; any other mode leaves Status at the invalid code. */
	if (ShaMode == XASU_SHA_MODE_512) {
		*DigestLenPtr = XSLHDSA_SHA512_DIGEST_BYTES;
		Status = XASUFW_SUCCESS;
	} else if (ShaMode == XASU_SHA_MODE_256) {
		*DigestLenPtr = XSLHDSA_SHA256_DIGEST_BYTES;
		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Return the hash block size for a SHA2 mode.
*
* @param	ShaMode	XASU_SHA_MODE_256 or XASU_SHA_MODE_512.
* @param	BlockLenPtr	Pointer to store the block length in bytes.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM if BlockLenPtr is NULL or ShaMode is unsupported.
*
* @note
* Used for PK.seed zero-padding in FIPS 205 SHA2 H and T_l prefix (Section 11.2).
* Only SHA-256 and SHA-512 are accepted.
*
*************************************************************************************************/
static s32 XSlhDsa_HashSha2BlockLen(u32 ShaMode, u32 *BlockLenPtr)
{
	s32 Status = XASUFW_SLHDSA_INVALID_PARAM;

	/** Validate output pointer. */
	if (BlockLenPtr == NULL) {
		goto END;
	}

	/** Block length drives PK.seed padding; other modes leave Status at the invalid code. */
	if (ShaMode == XASU_SHA_MODE_512) {
		*BlockLenPtr = XASUFW_SHA2_384_512_BLOCK_LEN;
		Status = XASUFW_SUCCESS;
	} else if (ShaMode == XASU_SHA_MODE_256) {
		*BlockLenPtr = XASUFW_SHA2_256_BLOCK_LEN;
		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Internal: truncated SHA2(PK.seed || ADRS || M) over local or DMA message data.
*
* FIPS 205 SHA2 instantiation absorb sequence for F, H, T_l, and PRF (Section 11.2).
*
* @param	InstPtr	Pointer to SLH-DSA instance with PkSeed and Addr loaded.
* @param	MsgAddr	DMA address of M (used when MsgLocal is NULL).
* @param	MsgLocal	Local pointer to M (preferred when non-NULL).
* @param	MsgLen	Message length in bytes.
* @param	ShaMode	SHA mode: XASU_SHA_MODE_256 or XASU_SHA_MODE_512.
* @param	Out	Output buffer (N bytes).
* @param	N	Truncation length in bytes (FIPS 205 n).
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SLHDSA_COPY_FAILED,
*		  XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or XASUFW_SHA_FINISH_FAILED).
*
* @note
* Uses ASU SHA2 engine throughout.
*
*************************************************************************************************/
static s32 XSlhDsa_HashSha2PkSeedAddrStream(XSlhDsa_Instance *InstPtr, u64 MsgAddr,
			const u8 *MsgLocal, u32 MsgLen, u32 ShaMode,
			u8 *Out, u32 N)
{
	XSlhDsa_HashCtx HashCtx;
	u8 Sha2AddrField[XSLHDSA_SHA2_ADDR_FIELD_BYTES];
	s32 Status = XASUFW_FAILURE;

	/** Bind the SHA2 engine and DMA handles of the instance to a local streaming context. */
	Status = XSlhDsa_HashCtxInit(InstPtr, &HashCtx);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * Compress ADRS to its 22-byte SHA2 form into a local buffer before the engine is
	 * started, so the packed field stays resident for the later DMA feed.
	 */
	XSlhDsa_AddrPackSha2Compact(InstPtr->Addr, Sha2AddrField);

	/** Start a fresh SHA2 hash in the requested mode. */
	Status = XSlhDsa_HashSha2Begin(&HashCtx, ShaMode);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Feed the FIPS 205 order: PK.seed padded to a full block, compressed ADRS, then M. */
	Status = XSlhDsa_HashSha2UpdatePkSeedPrefix(&HashCtx, InstPtr, ShaMode);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Feed the 22-byte compact ADRS that follows the padded PK.seed block. */
	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)Sha2AddrField,
				    XSLHDSA_SHA2_ADDR_FIELD_BYTES, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Feed M as the last segment; a non-NULL MsgLocal wins over the DMA address. */
	if (MsgLen > 0U) {
		if (MsgLocal != NULL) {
			Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)MsgLocal, MsgLen, 1U);
		}
		else {
			Status = XSlhDsa_HashUpdate(&HashCtx, MsgAddr, MsgLen, 1U);
		}
	}

	/** A single check covers both branches of the message update. */
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Read the digest and truncate it to N bytes, which zeroizes the scratch digest. */
	Status = XSlhDsa_HashSha2FinishTrunc(&HashCtx, ShaMode, N, Out);

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Select SHA-256 vs SHA-512 for H, H_msg, T_l, and PRF_msg.
*
* FIPS 205 Section 11.2: SHA2 parameter sets with n <= XASU_SLHDSA_128S_N use SHA-256;
* sets with n > XASU_SLHDSA_128S_N use SHA-512.
*
* @param	HashWidth	Security parameter n in bytes.
*
* @return
*		XASU_SHA_MODE_256 when HashWidth <= XASU_SLHDSA_128S_N, else XASU_SHA_MODE_512.
*
* @note
* F and PRF always use SHA-256 regardless of n (see XSLHDSA_SHA2_MODE_FPRF).
*
*************************************************************************************************/
static u32 XSlhDsa_Sha2ModeHtl(u32 HashWidth)
{
	return (HashWidth <= XASU_SLHDSA_128S_N) ? XASU_SHA_MODE_256 : XASU_SHA_MODE_512;
}
/*************************************************************************************************/
/**
* @brief	Return the MGF1 digest block size for a SHA2 mode.
*
* MGF1 (RFC 8017 Appendix B.2.1) uses the underlying hash output length as the expansion
* block size.
*
* @param	ShaMode	XASU_SHA_MODE_256 or XASU_SHA_MODE_512.
* @param	MgfBlockPtr	Pointer to store the MGF1 block size in bytes.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM if MgfBlockPtr is NULL or ShaMode is unsupported.
*
*************************************************************************************************/
static s32 XSlhDsa_Sha2MgfBlock(u32 ShaMode, u32 *MgfBlockPtr)
{
	s32 Status = XASUFW_SLHDSA_INVALID_PARAM;

	if (MgfBlockPtr == NULL) {
		goto END;
	}

	/**
	 * MGF1 expands in units of the underlying digest length. Any mode other than
	 * SHA-256 or SHA-512 leaves Status at XASUFW_SLHDSA_INVALID_PARAM.
	 */
	if (ShaMode == XASU_SHA_MODE_512) {
		*MgfBlockPtr = XSLHDSA_SHA512_DIGEST_BYTES;
		Status = XASUFW_SUCCESS;
	} else if (ShaMode == XASU_SHA_MODE_256) {
		*MgfBlockPtr = XSLHDSA_SHA256_DIGEST_BYTES;
		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Return the HMAC block size for a SHA2 mode.
*
* @param	ShaMode	XASU_SHA_MODE_256 or XASU_SHA_MODE_512.
* @param	BlockLenPtr	Pointer to store the HMAC block size in bytes.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM if BlockLenPtr is NULL or ShaMode is unsupported.
*
*************************************************************************************************/
static s32 XSlhDsa_Sha2HmacBlockLen(u32 ShaMode, u32 *BlockLenPtr)
{
	s32 Status = XASUFW_SLHDSA_INVALID_PARAM;

	if (BlockLenPtr == NULL) {
		goto END;
	}

	/**
	 * HMAC pads the key to the hash block length: 128 bytes for SHA-512, 64 for
	 * SHA-256. Any other mode leaves Status at XASUFW_SLHDSA_INVALID_PARAM.
	 */
	if (ShaMode == XASU_SHA_MODE_512) {
		*BlockLenPtr = XASUFW_SHA2_384_512_BLOCK_LEN;
		Status = XASUFW_SUCCESS;
	} else if (ShaMode == XASU_SHA_MODE_256) {
		*BlockLenPtr = XASUFW_SHA2_256_BLOCK_LEN;
		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Expand a seed buffer with MGF1-SHA2 to m bytes.
*
* FIPS 205 H_msg step 2: MGF1(seed, m) where seed = R || PK.seed || inner_digest || counter.
* Each MGF1 block hashes seed with an incrementing 32-bit big-endian counter appended.
*
* @param	InstPtr	SLH-DSA instance for hardware context binding.
* @param	Seed	Seed buffer with 4-byte counter field at the tail.
* @param	SeedLen	Seed length in bytes (including counter field).
* @param	ShaMode	SHA mode for MGF1 (SHA-256 or SHA-512).
* @param	Out	Output buffer of length InstPtr->Param.m.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_SLHDSA_INVALID_PARAM on invalid input.
*		- XASUFW_SLHDSA_COPY_FAILED, if the partial MGF1 digest copy fails.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* Uses ASU SHA2 engine for each MGF1 block hash.
*
*************************************************************************************************/
static s32 XSlhDsa_Sha2Mgf1Expand(XSlhDsa_Instance *InstPtr, u8 *Seed, u32 SeedLen,
				       u32 ShaMode, u8 *Out)
{
	XSlhDsa_HashCtx HashCtx;
	u32 MgfBlock;
	u8 *CounterBe = Seed + SeedLen - XASUFW_WORD_LEN_IN_BYTES;
	u8 FullDigest[XSLHDSA_SHA512_DIGEST_BYTES] = {0U};
	u32 Produced;
	u32 BlockIndex;
	s32 Status = XASUFW_FAILURE;

	/** Each MGF1 iteration emits one full digest, so the digest length is the step size. */
	Status = XSlhDsa_Sha2MgfBlock(ShaMode, &MgfBlock);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * MGF1 concatenates hash(Seed || I2OSP(counter, 4)) for counter = 0, 1, 2, ... until
	 * m bytes are available, so the requested output length alone drives the iteration
	 * count of ceil(m / MgfBlock) blocks.
	 */
	for (Produced = 0U; Produced < InstPtr->Param.m; Produced += MgfBlock) {
		BlockIndex = Produced / MgfBlock;

		/**
		 * - Overwrite the 4-byte counter field reserved at the tail of the seed. MGF1
		 *   mandates the big-endian I2OSP encoding of the block index.
		 */
		XAsufw_I2Osp(BlockIndex, XASUFW_WORD_LEN_IN_BYTES, CounterBe);

		/** - Each MGF1 block is an independent hash of the whole seed. */
		Status = XSlhDsa_HashCtxInit(InstPtr, &HashCtx);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		Status = XSlhDsa_HashSha2Begin(&HashCtx, ShaMode);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)Seed, SeedLen, 1U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		/**
		 * - Whole blocks land directly in the output buffer. The trailing block is
		 *   normally partial, so it is taken into a scratch digest and only the
		 *   remaining m - Produced bytes are copied out to avoid overrunning Out.
		 */
		if ((InstPtr->Param.m - Produced) >= MgfBlock) {
			Status = XSlhDsa_HashSha2FinishFull(&HashCtx, ShaMode, Out + Produced);
		}
		else {
			Status = XSlhDsa_HashSha2FinishFull(&HashCtx, ShaMode, FullDigest);
			if (Status == XASUFW_SUCCESS) {
				Status = Xil_SMemCpy(Out + Produced,
					InstPtr->Param.m - Produced, FullDigest,
					sizeof(FullDigest),
					InstPtr->Param.m - Produced);
				if (Status != XASUFW_SUCCESS) {
					Status = XASUFW_SLHDSA_COPY_FAILED;
				}
			}
		}

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
* @brief	Compute H_msg for SHA2: inner hash then MGF1 expansion.
*
* FIPS 205 Section 11.2 H_msg step 1: hash R || PK.seed || PK.root || [pure ctx] || M.
* Step 2: MGF1(R || PK.seed || digest, m).
*
* @param	InstPtr	SLH-DSA instance with signature prefix and message fields set.
* @param	ShaMode	SHA-256 or SHA-512 mode selected from n.
* @param	Out	Output m-byte message digest buffer.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_CMD_IN_PROGRESS if the message SHA update is running non-blocking.
*		- XASUFW_SLHDSA_COPY_FAILED, if the randomizer R or PK.seed copy into the
*		  MGF1 seed fails.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* Inner hash and MGF1 both use the ASU SHA2 engine.
*
*************************************************************************************************/
static s32 XSlhDsa_Sha2HashMsgImpl(XSlhDsa_Instance *InstPtr, u32 ShaMode, u8 *Out)
{
	XSlhDsa_HashCtx HashCtx;
	u32 HashWidth = InstPtr->Param.n;
	u32 MgfBlock;
	u8 MgfSeed[(XSLHDSA_HMSG_MGF_N_PREFIX * XSLHDSA_MAX_N) +
		   XSLHDSA_SHA512_DIGEST_BYTES + XASUFW_WORD_LEN_IN_BYTES];
	u32 MgfSeedLen;
	s32 Status = XASUFW_FAILURE;

	Status = XSlhDsa_Sha2MgfBlock(ShaMode, &MgfBlock);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * MGF1 seed layout: R (n bytes) || PK.seed (n bytes) || inner digest (MgfBlock
	 * bytes) || 4-byte counter field consumed by XSlhDsa_Sha2Mgf1Expand.
	 */
	MgfSeedLen = (XSLHDSA_HMSG_MGF_N_PREFIX * HashWidth) + MgfBlock +
		     XASUFW_WORD_LEN_IN_BYTES;

	Status = XSlhDsa_HashCtxInit(InstPtr, &HashCtx);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * A previous message update already absorbed R || PK.seed || PK.root || prefix || M.
	 * Skip Start and the absorb, and finish the inner digest once DMA has completed.
	 */
	if (XSlhDsa_YieldIsHashDmaResume(InstPtr) != 0U) {
		XSlhDsa_YieldClearHashDmaResume(InstPtr);
		goto INNER_FINISH;
	}

	/**
	 * FIPS 205 Section 11.2 H_msg step 1: inner digest over
	 * R || PK.seed || PK.root || [ctx prefix] || M.
	 */
	Status = XSlhDsa_HashSha2Begin(&HashCtx, ShaMode);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** - Absorb the randomizer R, which occupies the first n bytes of the signature. */
	Status = XSlhDsa_HashUpdate(&HashCtx, InstPtr->SigAddr, HashWidth, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** - Absorb the public key fields PK.seed and PK.root. */
	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)InstPtr->PkSeed, HashWidth, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)InstPtr->PkRoot, HashWidth, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - Absorb the domain separator prefix staged in WorkMem: DS || ctx_len || ctx for
	 *   pure signing, with the pre-hash OID appended in HashSLH-DSA mode.
	 */
	if (InstPtr->CtxPrefixLen > 0U) {
		Status = XSlhDsa_HashUpdate(&HashCtx,
					    (u64)(UINTPTR)InstPtr->WorkMem,
					    InstPtr->CtxPrefixLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	/**
	 * - Absorb the message and close the inner hash. A pre-hashed digest PH(M) supplied
	 *   by the client is used when present, otherwise the DMA-resident message. An empty
	 *   message leaves the hash open and it is closed by the padding in XSha_Finish.
	 */
	if (InstPtr->MsgLocalPtr != NULL) {
		Status = XSlhDsa_HashUpdateMsg(&HashCtx,
					       (u64)(UINTPTR)InstPtr->MsgLocalPtr,
					       InstPtr->MsgLocalLen, 1U);
	}
	else if ((InstPtr->MsgAddr != 0U) && (InstPtr->MsgLen > 0U)) {
		Status = XSlhDsa_HashUpdateMsg(&HashCtx, InstPtr->MsgAddr, InstPtr->MsgLen, 1U);
	}
	else {
		/* Do nothing */
	}

	if (Status == XASUFW_CMD_IN_PROGRESS) {
		XSlhDsa_YieldSetHashDmaResume(InstPtr);
		goto END;
	}
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

INNER_FINISH:
	/**
	 * Write the inner digest straight into its slot in the MGF1 seed, just past the
	 * R || PK.seed prefix, so no separate digest buffer is needed.
	 */
	Status = XSlhDsa_HashSha2FinishFull(&HashCtx, ShaMode,
						 MgfSeed + (XSLHDSA_HMSG_MGF_N_PREFIX *
							    HashWidth));
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Fill the seed prefix with R and PK.seed to complete the MGF1 seed. */
	Status = Xil_SMemCpy(MgfSeed, MgfSeedLen, (const u8 *)(UINTPTR)InstPtr->SigAddr,
				     HashWidth, HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_COPY_FAILED;
		goto END;
	}

	Status = Xil_SMemCpy(MgfSeed + HashWidth, MgfSeedLen - HashWidth, InstPtr->PkSeed,
				     HashWidth, HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_COPY_FAILED;
		goto END;
	}

	/** H_msg step 2: expand the seed with MGF1 to the m-byte message digest. */
	Status = XSlhDsa_Sha2Mgf1Expand(InstPtr, MgfSeed, MgfSeedLen, ShaMode, Out);

END:
	/**
	 * Zeroize the MGF1 seed on every exit path. A DMA wait is kept even if the wipe fails;
	 * the failure is then reported when the yield session ends.
	 */
	Status = XSlhDsa_HashZeroizeKeepWait(InstPtr, Status, MgfSeed, (u32)sizeof(MgfSeed));
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Compute PRF_msg via HMAC-SHA2.
*
* FIPS 205 Section 11.2 PRF_msg: HMAC(SK.prf, OptRand || [pure ctx] || M), truncated to n bytes.
*
* @param	InstPtr	SLH-DSA instance with SkPrf loaded.
* @param	OptRand	Signature randomizer R (n bytes).
* @param	Msg	Message buffer M.
* @param	MsgLen	Message length in bytes.
* @param	Ctx	Optional pure-signing context string C.
* @param	CtxLen	FIPS context length 0..255.
* @param	HashWidth	Security parameter n in bytes.
* @param	BlockLen	HMAC block size (64 for SHA-256, 128 for SHA-512).
* @param	ShaMode	SHA-256 or SHA-512 mode.
* @param	Out	Output n-byte buffer.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_CMD_IN_PROGRESS if the message SHA update is running non-blocking.
*		- XASUFW_SLHDSA_COPY_FAILED, if the SK.prf copy into the HMAC key
*		  block fails.
*		- XASUFW_ZEROIZE_MEMSET_FAIL, if the ipad key block padding fill fails.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL,
*		  or XASUFW_SHA_FINISH_FAILED).
*
* @note
* Uses ASU SHA2 engine for HMAC inner and outer hashes. Sensitive buffers are zeroized on exit.
*
*************************************************************************************************/
static s32 XSlhDsa_Sha2HashPrfMsgImpl(XSlhDsa_Instance *InstPtr, const u8 *OptRand,
			const u8 *Msg, u32 MsgLen, const u8 *Ctx, u32 CtxLen,
			u32 HashWidth, u32 BlockLen, u32 ShaMode, u8 *Out)
{
	XSlhDsa_HashCtx HashCtx;
	u8 IpadKey[XASUFW_SHA2_384_512_BLOCK_LEN];
	u8 InnerDigest[XSLHDSA_SHA512_DIGEST_BYTES];
	u32 DigestLen;
	u32 Idx;
	s32 Status = XASUFW_FAILURE;

	Status = XSlhDsa_Sha2MgfBlock(ShaMode, &DigestLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * Build the HMAC inner key block K0 ^ ipad. SK.prf is n bytes and always shorter
	 * than the block length, so the key bytes are XORed with XSLHDSA_HMAC_INNER_PAD_BYTE
	 * and the zero padding that extends K0 to the block length reduces to the same fill.
	 */
	Status = Xil_SMemCpy(IpadKey, sizeof(IpadKey), InstPtr->SkPrf, HashWidth,
				     HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_COPY_FAILED;
		goto END;
	}
	for (Idx = 0U; Idx < HashWidth; Idx++) {
		IpadKey[Idx] ^= XSLHDSA_HMAC_INNER_PAD_BYTE;
	}
	Status = Xil_SMemSet(IpadKey + HashWidth, BlockLen - HashWidth,
			    XSLHDSA_HMAC_INNER_PAD_BYTE, BlockLen - HashWidth);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	Status = XSlhDsa_HashCtxInit(InstPtr, &HashCtx);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * A previous message update already absorbed (K0 ^ ipad) || OptRand || prefix || M.
	 * Skip the inner Start and absorb, and finish the inner digest once DMA has completed.
	 */
	if (XSlhDsa_YieldIsHashDmaResume(InstPtr) != 0U) {
		XSlhDsa_YieldClearHashDmaResume(InstPtr);
		goto INNER_FINISH;
	}

	/** Inner HMAC hash over (K0 ^ ipad) || OptRand || [ctx prefix] || M. */
	Status = XSlhDsa_HashSha2Begin(&HashCtx, ShaMode);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** - Absorb the ipad key block followed by the randomizer OptRand. */
	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)IpadKey, BlockLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)OptRand, HashWidth, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * - Absorb the domain separator prefix. Pre-hash mode reuses the prefix already
	 *   staged in WorkMem, which carries the OID; pure mode emits DS || ctx_len || ctx
	 *   built from the caller supplied context.
	 */
	if (InstPtr->PreHashMode == XASU_SLHDSA_MSG_PREHASH) {
		if (InstPtr->CtxPrefixLen > 0U) {
			Status = XSlhDsa_HashUpdate(&HashCtx,
						    (u64)(UINTPTR)InstPtr->WorkMem,
						    InstPtr->CtxPrefixLen, 0U);
			if (Status != XASUFW_SUCCESS) {
				goto END;
			}
		}
	}
	else {
		Status = XSlhDsa_HashSha2AppendPureContext(&HashCtx, Ctx, CtxLen);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	/**
	 * - Absorb the message and close the inner hash, preferring the client supplied
	 *   PH(M) digest when the instance is in pre-hash mode.
	 */
	if (InstPtr->MsgLocalPtr != NULL) {
		Status = XSlhDsa_HashUpdateMsg(&HashCtx,
					       (u64)(UINTPTR)InstPtr->MsgLocalPtr,
					       InstPtr->MsgLocalLen, 1U);
	}
	else {
		Status = XSlhDsa_HashUpdateMsg(&HashCtx, (u64)(UINTPTR)Msg, MsgLen, 1U);
	}

	if (Status == XASUFW_CMD_IN_PROGRESS) {
		XSlhDsa_YieldSetHashDmaResume(InstPtr);
		goto END;
	}
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

INNER_FINISH:
	Status = XSlhDsa_HashSha2FinishFull(&HashCtx, ShaMode, InnerDigest);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * Convert the key block from K0 ^ ipad to K0 ^ opad in place by XORing with
	 * ipad ^ opad. The loop spans the whole block, including the padding bytes.
	 */
	for (Idx = 0U; Idx < BlockLen; Idx++) {
		IpadKey[Idx] ^= XSLHDSA_HMAC_INNER_PAD_BYTE ^
				XSLHDSA_HMAC_OUTER_PAD_BYTE;
	}

	/** Restart the engine for the outer hash over (K0 ^ opad) || inner digest. */
	Status = XSlhDsa_HashCtxInit(InstPtr, &HashCtx);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Status = XSlhDsa_HashSha2Begin(&HashCtx, ShaMode);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)IpadKey, BlockLen, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Status = XSlhDsa_HashUpdate(&HashCtx, (u64)(UINTPTR)InnerDigest, DigestLen, 1U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Truncate the HMAC output to n bytes as required by FIPS 205 PRF_msg. */
	Status = XSlhDsa_HashSha2FinishTrunc(&HashCtx, ShaMode, HashWidth, Out);

END:
	/**
	 * Zeroize the key block and the inner digest on every exit path. A DMA wait is kept
	 * even if a wipe fails; the failure is then reported when the yield session ends.
	 */
	Status = XSlhDsa_HashZeroizeKeepWait(InstPtr, Status, IpadKey, (u32)sizeof(IpadKey));
	Status = XSlhDsa_HashZeroizeKeepWait(InstPtr, Status, InnerDigest,
					     (u32)sizeof(InnerDigest));
	return Status;
}
/*************************************************************************************************/
/**
* @brief	Feed H_msg absorb inputs into an active SHAKE256 hash.
*
* FIPS 205 Section 11.1 H_msg: absorb R || PK.seed || PK.root || [pure ctx] || M
* into SHAKE256 on the ASU SHA3 engine.
*
* @param	HashCtxPtr	Active SHAKE hash context (already begun).
* @param	InstPtr	SLH-DSA instance with bound signature and message fields.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- XASUFW_CMD_IN_PROGRESS if the message SHA update is running non-blocking.
*		- Error code received from called functions in case of other failure from the
*		  called function (including XASUFW_SHA_UPDATE_FAIL).
*
* @note
* Does not finish the hash; caller invokes XSlhDsa_HashShakeFinish.
*
*************************************************************************************************/
static s32 XSlhDsa_ShakeFeedHmsgInputs(XSlhDsa_HashCtx *HashCtxPtr,
					    XSlhDsa_Instance *InstPtr)
{
	u32 HashWidth = InstPtr->Param.n;
	s32 Status = XASUFW_FAILURE;

	/** Absorb the randomizer R, which occupies the first n bytes of the signature. */
	Status = XSlhDsa_HashUpdate(HashCtxPtr, InstPtr->SigAddr, HashWidth, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Absorb the public key fields PK.seed and PK.root. */
	Status = XSlhDsa_HashUpdate(HashCtxPtr, (u64)(UINTPTR)InstPtr->PkSeed, HashWidth, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	Status = XSlhDsa_HashUpdate(HashCtxPtr, (u64)(UINTPTR)InstPtr->PkRoot, HashWidth, 0U);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * Absorb the domain separator prefix staged in WorkMem: DS || ctx_len || ctx for
	 * pure signing, with the pre-hash OID appended in HashSLH-DSA mode.
	 */
	if (InstPtr->CtxPrefixLen > 0U) {
		Status = XSlhDsa_HashUpdate(HashCtxPtr,
					    (u64)(UINTPTR)InstPtr->WorkMem,
					    InstPtr->CtxPrefixLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	/**
	 * Absorb the message and close the absorb phase. A pre-hashed digest PH(M) supplied
	 * by the client is used when present, otherwise the DMA-resident message. With no
	 * message bytes to absorb, Status carries the result of the preceding update and the
	 * hash is closed by the padding applied in XSlhDsa_HashShakeFinish.
	 */
	if (InstPtr->MsgLocalPtr != NULL) {
		Status = XSlhDsa_HashUpdateMsg(HashCtxPtr,
					       (u64)(UINTPTR)InstPtr->MsgLocalPtr,
					       InstPtr->MsgLocalLen, 1U);
	}
	else if ((InstPtr->MsgAddr != 0U) && (InstPtr->MsgLen > 0U)) {
		Status = XSlhDsa_HashUpdateMsg(HashCtxPtr, InstPtr->MsgAddr, InstPtr->MsgLen, 1U);
	}

END:
	return Status;
}
/*************************************************************************************************/
/**
* @brief	This function appends the FIPS 205 pure-mode context prefix to a SHAKE hash.
*
* @param	HashCtxPtr	Pointer to the active SHAKE hash context.
* @param	Ctx	Optional context string for pure mode.
* @param	CtxLen	FIPS context length 0..255.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- Error code from XSlhDsa_HashUpdate() otherwise.
*
*************************************************************************************************/
static s32 XSlhDsa_HashShakeAppendPureContext(XSlhDsa_HashCtx *HashCtxPtr, const u8 *Ctx,
			u32 CtxLen)
{
	u8 PureHdr[XASU_SLHDSA_PREFIX_HDR_LEN];
	s32 Status;

	PureHdr[XASU_SLHDSA_PREFIX_DS_OFFSET] = XASU_SLHDSA_DS_PURE;
	PureHdr[XASU_SLHDSA_PREFIX_CTXLEN_OFFSET] = (u8)CtxLen;
	Status = XSlhDsa_HashUpdate(HashCtxPtr, (u64)(UINTPTR)PureHdr,
				    XASU_SLHDSA_PREFIX_HDR_LEN, 0U);
	if (Status != XASUFW_SUCCESS) {
		return Status;
	}

	if ((CtxLen > 0U) && (Ctx != NULL)) {
		return XSlhDsa_HashUpdate(HashCtxPtr, (u64)(UINTPTR)Ctx, CtxLen, 0U);
	}

	return XASUFW_SUCCESS;
}
/*************************************************************************************************/
/**
* @brief	This function appends the FIPS 205 pure-mode context prefix to a SHA2 hash.
*
* @param	HashCtxPtr	Pointer to the active SHA2 hash context.
* @param	Ctx	Optional context string for pure mode.
* @param	CtxLen	FIPS context length 0..255.
*
* @return
*		- XASUFW_SUCCESS on success.
*		- Error code from XSlhDsa_HashUpdate() otherwise.
*
*************************************************************************************************/
static s32 XSlhDsa_HashSha2AppendPureContext(XSlhDsa_HashCtx *HashCtxPtr, const u8 *Ctx,
			u32 CtxLen)
{
	u8 PureHdr[XASU_SLHDSA_PREFIX_HDR_LEN];
	s32 Status;

	PureHdr[XASU_SLHDSA_PREFIX_DS_OFFSET] = XASU_SLHDSA_DS_PURE;
	PureHdr[XASU_SLHDSA_PREFIX_CTXLEN_OFFSET] = (u8)CtxLen;
	Status = XSlhDsa_HashUpdate(HashCtxPtr, (u64)(UINTPTR)PureHdr,
				    XASU_SLHDSA_PREFIX_HDR_LEN, 0U);
	if (Status != XASUFW_SUCCESS) {
		return Status;
	}

	if ((CtxLen > 0U) && (Ctx != NULL)) {
		return XSlhDsa_HashUpdate(HashCtxPtr, (u64)(UINTPTR)Ctx, CtxLen, 0U);
	}

	return XASUFW_SUCCESS;
}
#endif /* XASU_SLHDSA_ENABLE */
/** @} */
