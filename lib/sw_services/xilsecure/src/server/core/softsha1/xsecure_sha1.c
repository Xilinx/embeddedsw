/***************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
 *
 * @file xsecure_sha1.c
 *
 * This file contains SHA1 driver as per FIPS PUB 180-1.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   mmd  12/20/24 Initial release
 *
 * </pre>
 *
 **************************************************************************************************/
/**
* @addtogroup xsecure_sha_server_apis XilSecure SHA Server APIs
* @{
*/
/*************************************** Include Files ********************************************/
#include "xil_sutil.h"
#include "xsecure_sha1.h"

/************************** Constant Definitions **************************************************/
#define BYTE_SIZE_IN_BITS				(8U)
#define WORD_SIZE_IN_BITS				(sizeof(u32) * BYTE_SIZE_IN_BITS)
#define XSECURE_SHA1_HASH_BLK_SIZE			(64U)		/* 512-bits = 64 bytes */
#define XSECURE_SHA1_LEN_FIELD_SIZE			(8U)
#define XSECURE_SHA1_MIN_PADDING_LEN			(XSECURE_SHA1_LEN_FIELD_SIZE + 1U)
#define XSECURE_SHA1_PADDING_START			(0x80)
#define XSECURE_SHA1_PADDING_MIDDLE			(0x00)
#define XSECURE_SHA_TOTAL_NUM_OF_STATES			(5U)

#define K0	0x5A827999U
#define K1	0x6ED9EBA1U
#define K2	0x8F1BBCDCU
#define K3	0xCA62C1D6U

#define H0	0x67452301U
#define H1	0xEFCDAB89U
#define H2	0x98BADCFEU
#define H3	0x10325476U
#define H4	0xC3D2E1F0U


/**************************** Type Definitions ****************************************************/
/* SHA1 Context */
typedef struct {
	u32 State[XSECURE_SHA_TOTAL_NUM_OF_STATES];
	u32 Buffer[XSECURE_SHA1_HASH_BLK_SIZE / sizeof(u32)];
} XSecure_Sha1Ctx;

/***************** Macros (Inline Functions) Definitions ******************************************/
#define RotateLeft32(Value, Bits) \
	(((Value) << (Bits)) | ((Value) >> (WORD_SIZE_IN_BITS - (Bits))))

/* BLK0() and BLK() perform the initial expand. */
/* Inspired by SSLeay */
#define BLK0(i) (Block[(i)] = (RotateLeft32(Block[(i)], 24U)& 0xFF00FF00UL) \
	|(RotateLeft32(Block[(i)], 8U) & 0x00FF00FFUL))

#define BLK(i) (Block[(i) & 15U] = RotateLeft32(Block[((i) + 13U) & 15U] ^ Block[((i) + 8U) & 15U] \
	^Block[((i) + 2U) & 15U] ^ Block[(i) & 15U], 1U))

/* R0, R1, R2, R3, R4 are the different operations used in SHA1 */
#define R0(v, w, x, y, z, i)   \
	do {	\
		BLK0(i); \
		u32 t = Block[(i)]; \
		(z) += ((((w) & ((x) ^ (y))) ^ (y)) + t + K0 + RotateLeft32((v), 5U)); \
		(w) = RotateLeft32((w), 30U); \
	} while (FALSE)

#define R1(v, w, x, y, z, i)   \
	do {	\
		BLK(i); \
		u32 t = Block[(i) & 15U]; \
		(z) += ((((w) & ((x) ^ (y))) ^ (y)) + t + K0 + RotateLeft32((v), 5U)); \
		(w) = RotateLeft32((w), 30U); \
	} while (FALSE)

#define R2(v, w, x, y, z, i)   \
	do {	\
		BLK(i); \
		u32 t = Block[(i) & 15U]; \
		(z) += (((w) ^ (x) ^ (y)) + t + K1 + RotateLeft32((v), 5U)); \
		(w) = RotateLeft32((w), 30U); \
	} while (FALSE)

#define R3(v, w, x, y, z, i)   \
	do {	\
		BLK(i); \
		u32 t = Block[(i) & 15U]; \
		(z) += (((((w) | (x)) & (y)) | ((w) & (x))) + t + K2 + RotateLeft32((v), 5U)); \
		(w) = RotateLeft32((w), 30U); \
	} while (FALSE)

#define R4(v, w, x, y, z, i)   \
	do {	\
		BLK(i); \
		u32 t = Block[(i) & 15U]; \
		(z) += (((w) ^ (x) ^ (y)) + t + K3 + RotateLeft32((v), 5U)); \
		(w) = RotateLeft32((w), 30U); \
	} while (FALSE)

/* Muliply 32-bit Len by 8 and store 64-bit result in LenHigh, LenLow */
#define CONVERT_BYTE_TO_BITS(Len, LenHigh, LenLow)	\
	do { \
		LenHigh = (Len) >> 29U;	\
		LenLow = (Len) << 3U;	\
	} while(FALSE)

/* Extracts specified byte from 32-bit word. Make sure ByteNo is between 0 to 3. */
#define GET_BYTE_FROM_U32(Data, ByteNo)			((u8)(((Data) >> ((ByteNo) * 8U)) & 0xFFU))

/************************************ Function Prototypes *****************************************/
static s32 XSecure_Sha1Transform(XSecure_Sha1Ctx *const Ctx, const u8 *const Buffer);
static void XSecure_Sha1Init (XSecure_Sha1Ctx* Ctx);
static void XSecure_ReadHash (const XSecure_Sha1Ctx* Ctx, u8 * const Hash);
static void XSecure_Sha1AddDataLenToPadding(u8 * const Buffer, u32 Len);

/**************************************************************************************************/
/**
 * @brief	This function calculates SHA1 hash on given input data.
 *
 * @param	Data	Pointer to input data
 * @param	Len	Length of input data
 * @param	Hash	Pointer to memory location where calculated hash to be stored
 *
 * @return	XST_FAILURE	In case of failure in SHA1 digest calculation
 * 		XST_SUCCESS	In case of successful calculation of SHA1 digest
 *
 **************************************************************************************************/
s32 XSecure_Sha1Digest(const u8 * const Data, u32 Len, u8 *const Hash)
{
	s32 Status = XST_FAILURE;
	XSecure_Sha1Ctx Ctx;
	u32 PaddingSize;
	XSecure_Sha1Init(&Ctx);
	u32 Idx;
	u32 AvailableBytes;
	u32 RemianingData;
	u8 *CtxBufferPtr = (u8 *) Ctx.Buffer;

	for (Idx = 0U; Idx < (Len / XSECURE_SHA1_HASH_BLK_SIZE); Idx++) {
		(void)XSecure_Sha1Transform(&Ctx, &Data[Idx * XSECURE_SHA1_HASH_BLK_SIZE]);
	}

	RemianingData = Len % XSECURE_SHA1_HASH_BLK_SIZE;
	if (RemianingData > 0U) {
		Status = Xil_SMemCpy(CtxBufferPtr, RemianingData,
			 &Data[Idx * XSECURE_SHA1_HASH_BLK_SIZE], RemianingData,
			 RemianingData);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	Idx = RemianingData;

	/* Available bytes for padding in last block */
	AvailableBytes = XSECURE_SHA1_HASH_BLK_SIZE - RemianingData;

	PaddingSize = AvailableBytes;
	if (AvailableBytes < XSECURE_SHA1_MIN_PADDING_LEN)
	{
		PaddingSize += XSECURE_SHA1_HASH_BLK_SIZE;
		CtxBufferPtr[Idx] = XSECURE_SHA1_PADDING_START;
		Idx++;
		PaddingSize--;
		while (Idx < XSECURE_SHA1_HASH_BLK_SIZE) {
			CtxBufferPtr[Idx] = XSECURE_SHA1_PADDING_MIDDLE;
			Idx++;
			PaddingSize--;
		}
		(void)XSecure_Sha1Transform(&Ctx, (u8 *) Ctx.Buffer);
		Idx = 0U;
	}
	else {
		CtxBufferPtr[Idx] = XSECURE_SHA1_PADDING_START;
		Idx++;
		PaddingSize--;
	}

	if(PaddingSize > 0U)	{
		while (Idx < (XSECURE_SHA1_HASH_BLK_SIZE - XSECURE_SHA1_LEN_FIELD_SIZE)) {
			CtxBufferPtr[Idx] = XSECURE_SHA1_PADDING_MIDDLE;
			Idx++;
			PaddingSize--;
		}
	}

	XSecure_Sha1AddDataLenToPadding(&CtxBufferPtr[Idx], Len);
	(void)XSecure_Sha1Transform(&Ctx, (u8 *) Ctx.Buffer);
	XSecure_ReadHash(&Ctx, Hash);
	Status = Xil_SMemSet(&Ctx, sizeof(Ctx), 0, sizeof(Ctx));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	Initializies the SHA1 context
 *
 * @param	Ctx		Pointer to SHA1 context
 *
 **************************************************************************************************/
static void XSecure_Sha1Init (XSecure_Sha1Ctx* Ctx)
{
	/* Set the intial state of the context */
	Ctx->State[0] = H0;
	Ctx->State[1] = H1;
	Ctx->State[2] = H2;
	Ctx->State[3] = H3;
	Ctx->State[4] = H4;
}

/**************************************************************************************************/
/**
 * @brief	Hash a single 512-bit block. This is the core of the algorithm.
 *
 * @param	Ctx	Pointer to SHA1 context
 * @param	Buffer	Pointer to 512-bit block data
 *
 * @return	XST_FAILURE	In case of failure in SHA1 transform function
 * 		XST_SUCCESS	In case of success in SHA1 transform function
 *
 **************************************************************************************************/
static s32 XSecure_Sha1Transform(XSecure_Sha1Ctx *const Ctx, const u8 *const Buffer)
{
	s32 Status = XST_FAILURE;
	u32 W0;
	u32 W1;
	u32 W2;
	u32 W3;
	u32 W4;
	u32 *Block = Ctx->Buffer;

	Status = Xil_SMemCpy(Ctx->Buffer, XSECURE_SHA1_HASH_BLK_SIZE, Buffer, XSECURE_SHA1_HASH_BLK_SIZE,
				XSECURE_SHA1_HASH_BLK_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Copy context->state[] to working variables */
	W0 = Ctx->State[0];
	W1 = Ctx->State[1];
	W2 = Ctx->State[2];
	W3 = Ctx->State[3];
	W4 = Ctx->State[4];

	/* Loop unrolled. 80 iteration are unrolled for 4 rounds of 20 operations each. */
	R0(W0, W1, W2, W3, W4,  0U); R0(W4, W0, W1, W2, W3,  1U); R0(W3, W4, W0, W1, W2,  2U); R0(W2, W3, W4, W0, W1,  3U);
	R0(W1, W2, W3, W4, W0,  4U); R0(W0, W1, W2, W3, W4,  5U); R0(W4, W0, W1, W2, W3,  6U); R0(W3, W4, W0, W1, W2,  7U);
	R0(W2, W3, W4, W0, W1,  8U); R0(W1, W2, W3, W4, W0,  9U); R0(W0, W1, W2, W3, W4, 10U); R0(W4, W0, W1, W2, W3, 11U);
	R0(W3, W4, W0, W1, W2, 12U); R0(W2, W3, W4, W0, W1, 13U); R0(W1, W2, W3, W4, W0, 14U); R0(W0, W1, W2, W3, W4, 15U);
	R1(W4, W0, W1, W2, W3, 16U); R1(W3, W4, W0, W1, W2, 17U); R1(W2, W3, W4, W0, W1, 18U); R1(W1, W2, W3, W4, W0, 19U);
	R2(W0, W1, W2, W3, W4, 20U); R2(W4, W0, W1, W2, W3, 21U); R2(W3, W4, W0, W1, W2, 22U); R2(W2, W3, W4, W0, W1, 23U);
	R2(W1, W2, W3, W4, W0, 24U); R2(W0, W1, W2, W3, W4, 25U); R2(W4, W0, W1, W2, W3, 26U); R2(W3, W4, W0, W1, W2, 27U);
	R2(W2, W3, W4, W0, W1, 28U); R2(W1, W2, W3, W4, W0, 29U); R2(W0, W1, W2, W3, W4, 30U); R2(W4, W0, W1, W2, W3, 31U);
	R2(W3, W4, W0, W1, W2, 32U); R2(W2, W3, W4, W0, W1, 33U); R2(W1, W2, W3, W4, W0, 34U); R2(W0, W1, W2, W3, W4, 35U);
	R2(W4, W0, W1, W2, W3, 36U); R2(W3, W4, W0, W1, W2, 37U); R2(W2, W3, W4, W0, W1, 38U); R2(W1, W2, W3, W4, W0, 39U);
	R3(W0, W1, W2, W3, W4, 40U); R3(W4, W0, W1, W2, W3, 41U); R3(W3, W4, W0, W1, W2, 42U); R3(W2, W3, W4, W0, W1, 43U);
	R3(W1, W2, W3, W4, W0, 44U); R3(W0, W1, W2, W3, W4, 45U); R3(W4, W0, W1, W2, W3, 46U); R3(W3, W4, W0, W1, W2, 47U);
	R3(W2, W3, W4, W0, W1, 48U); R3(W1, W2, W3, W4, W0, 49U); R3(W0, W1, W2, W3, W4, 50U); R3(W4, W0, W1, W2, W3, 51U);
	R3(W3, W4, W0, W1, W2, 52U); R3(W2, W3, W4, W0, W1, 53U); R3(W1, W2, W3, W4, W0, 54U); R3(W0, W1, W2, W3, W4, 55U);
	R3(W4, W0, W1, W2, W3, 56U); R3(W3, W4, W0, W1, W2, 57U); R3(W2, W3, W4, W0, W1, 58U); R3(W1, W2, W3, W4, W0, 59U);
	R4(W0, W1, W2, W3, W4, 60U); R4(W4, W0, W1, W2, W3, 61U); R4(W3, W4, W0, W1, W2, 62U); R4(W2, W3, W4, W0, W1, 63U);
	R4(W1, W2, W3, W4, W0, 64U); R4(W0, W1, W2, W3, W4, 65U); R4(W4, W0, W1, W2, W3, 66U); R4(W3, W4, W0, W1, W2, 67U);
	R4(W2, W3, W4, W0, W1, 68U); R4(W1, W2, W3, W4, W0, 69U); R4(W0, W1, W2, W3, W4, 70U); R4(W4, W0, W1, W2, W3, 71U);
	R4(W3, W4, W0, W1, W2, 72U); R4(W2, W3, W4, W0, W1, 73U); R4(W1, W2, W3, W4, W0, 74U); R4(W0, W1, W2, W3, W4, 75U);
	R4(W4, W0, W1, W2, W3, 76U); R4(W3, W4, W0, W1, W2, 77U); R4(W2, W3, W4, W0, W1, 78U); R4(W1, W2, W3, W4, W0, 79U);

	/* Store the working context */
	Ctx->State[0] += W0;
	Ctx->State[1] += W1;
	Ctx->State[2] += W2;
	Ctx->State[3] += W3;
	Ctx->State[4] += W4;

	/* Clear intermediate variables */
	W0 = 0U;
	W1 = 0U;
	W2 = 0U;
	W3 = 0U;
	W4 = 0U;

	Status = XST_SUCCESS;

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	Read calculated hash from the context and store in user provided
 *		memory
 *
 * @param	Ctx	Pointer to SHA1 context
 * @param	Hash	Pointer to memory location where calculated hash to be stored
 *
 **************************************************************************************************/
static void XSecure_ReadHash (const XSecure_Sha1Ctx* Ctx, u8 * const Hash)
{
	u32 Idx;

	for (Idx = 0; Idx < (XSECURE_SHA1_HASH_SIZE / sizeof(u32)); Idx++) {

		Hash[Idx * sizeof(u32)] = GET_BYTE_FROM_U32(Ctx->State[Idx], 3U);
		Hash[(Idx * sizeof(u32)) + 1U] = GET_BYTE_FROM_U32(Ctx->State[Idx], 2U);
		Hash[(Idx * sizeof(u32)) + 2U] = GET_BYTE_FROM_U32(Ctx->State[Idx], 1U);
		Hash[(Idx * sizeof(u32)) + 3U] = GET_BYTE_FROM_U32(Ctx->State[Idx], 0U);
	}
}

/**************************************************************************************************/
/**
 * @brief	This function adds padding to data length
 *
 * @param	Buffer	Pointer to memory location where Length is to be padded
 * @param	Len	SHA1 padding length field in bytes
 *
 **************************************************************************************************/
static void XSecure_Sha1AddDataLenToPadding(u8 * const Buffer, u32 Len)
{
	u32 LenHigh;
	u32 LenLow;

	CONVERT_BYTE_TO_BITS(Len, LenHigh, LenLow);

	Buffer[0] = GET_BYTE_FROM_U32(LenHigh, 3U);
	Buffer[1] = GET_BYTE_FROM_U32(LenHigh, 2U);
	Buffer[2] = GET_BYTE_FROM_U32(LenHigh, 1U);
	Buffer[3] = GET_BYTE_FROM_U32(LenHigh, 0U);
	Buffer[4] = GET_BYTE_FROM_U32(LenLow, 3U);
	Buffer[5] = GET_BYTE_FROM_U32(LenLow, 2U);
	Buffer[6] = GET_BYTE_FROM_U32(LenLow, 1U);
	Buffer[7] = GET_BYTE_FROM_U32(LenLow, 0U);
}
/** @} */