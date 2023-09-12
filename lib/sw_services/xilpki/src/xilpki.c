/******************************************************************************
* Copyright (C) 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xilpki.c
 * <pre>
 *
 * This file contains the implementation of the interface functions for the
 * PKI ECDSA hardware module.
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date      Changes
 * ----- ----  --------  ------------------------------------------------------
 * 1.0   Nava  12/05/22  Initial Release
 * 1.1   Nava  06/06/23  Fix the issues relevant to the pki mux selection/deselection
 *                       logic.
 * 2.0   Nava  06/21/23  Added PKI multi-queue support for ECC operations.
 * 2.0   Nava  08/02/23  Added a new API XPki_GetVersion() to access the library
 *                       version info.
 * 2.0   Nava  09/06/23  Updated the XPki_GetVersion() API prototype to inline with
 *                       other secure library version info API's.
 * 2.0   Nava  09/07/23  Fixed issues with IRQ signal.
 * 2.0   Nava  09/11/23  Fixed doxygen warnings.
 *
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xilpki.h"
#include "xscugic.h"

/***************** Macros (Inline Functions) Definitions ********************/
#define XPKI_INTERRUPT_ID		(155U + 32U)
#define XPKI_RESET_DELAY_US		10U
#define	XPKI_ASSERT_RESET		1U
#define XPKI_DEASSERT_RESET		0U
#define FPD_CLEAR_WRITE_PROTECT		0U
#define XPKI_MUX_SELECT			1U
#define XPKI_ADDR_WORD_ALIGN_MASK	(0x3U)

#define XPKI_TRNG_INSTANCE		1U
#define XPKI_TRNG_BUF_SIZE		32U
#define XPKI_MAX_PRIV_KEY_LEN		96U

#define XPKI_PAGE_SIZE			4096U	/* Bytes */
#define XPKI_INDEX_1_VAL		0x80U
#define XPKI_INDEX_2_VAL		0x1000U

#define XPKI_RQ_CFG_PERMISSIONS_SAFE    0x0U
#define XPKI_RQ_CFG_PAGE_SIZE_4096      0x10U
#define XPKI_RQ_CFG_CQID                0x0U
#define XPKI_CQ_CFG_SIZE_4096           0x10U
#define XPKI_CQ_CFG_IRQ_ID_VAL          0x0U
#define XPKI_RQ_CFG_QUEUE_DEPTH_VAL     0x80U
#define XPKI_IRQ_ENABLE_VAL             0xFFFFU
#define XPKI_CQ_CTL_TRIGPOS_VAL         0x201U
#define XPKI_MAX_CQ_REQ			512U
#define XPKI_RID_MASK			0xFFFF0000U

#define DESC_TAG_START		0x00000002U
#define DESC_TAG_TFRI(sz)       ((u32)0x6 | (sz) << 16)
#define DESC_TAG_TFR0(sz)       ((u32)0xE | (sz) << 16)
#define DESC_TAG_NTFY(rid) 	((u32)0x16 | (rid))
#define DESC_TAG_END		0x00000000U
#define DESC_REQ_ID(QId, SlotId)	((u32)((QId + 1) << 28) | (SlotId + 1) << 16)

#define NIST_P384_LEN_BYTES     48U /* Bytes */
#define NIST_P256_LEN_BYTES     32U /* Bytes */
#define NIST_P521_LEN_BYTES     66U /* Bytes */

#define P521_PADD_2_BYTES       0x2U
#define P521_PADD_6_BYTES       0x6U

#define SIGN_INPUT_OP_COUNT		3U
#define SIGN_VERIFY_INPUT_OP_COUNT	5U
#define PRIV_KEY_INPUT_OP_COUNT		3U
#define PUB_KEY_INPUT_OP_COUNT		3U

#define SIGN_OUTPUT_OP_COUNT             2U
#define SIGN_VERIFY_OUTPUT_OP_COUNT      0U
#define PRIV_KEY_OUTPUT_OP_COUNT         1U
#define PUB_KEY_OUTPUT_OP_COUNT          2U

#define PKI_ECC_NIST_P192_SIGN_CMD		0x00401730U
#define PKI_ECC_NIST_P256_SIGN_CMD		0x00101F30U
#define PKI_ECC_NIST_P384_SIGN_CMD		0x00202F30U
#define PKI_ECC_NIST_P521_SIGN_CMD		0x00304130U
#define PKI_ECC_NIST_P192_SIGN_VERIFY_CMD	0x00401731U
#define PKI_ECC_NIST_P256_SIGN_VERIFY_CMD	0x00101F31U
#define PKI_ECC_NIST_P384_SIGN_VERIFY_CMD	0x00202F31U
#define PKI_ECC_NIST_P521_SIGN_VERIFY_CMD	0x00304131U
#define PKI_ECC_NIST_P192_KEY_PRIV_GEN_CMD	0x00001701U
#define PKI_ECC_NIST_P256_KEY_PRIV_GEN_CMD	0x00001F01U
#define PKI_ECC_NIST_P384_KEY_PRIV_GEN_CMD	0x00002F02U
#define PKI_ECC_NIST_P521_KEY_PRIV_GEN_CMD	0x00004101U
#define PKI_ECC_NIST_P192_KEY_PUB_GEN_CMD	0x00401722U
#define PKI_ECC_NIST_P256_KEY_PUB_GEN_CMD	0x00101F22U
#define PKI_ECC_NIST_P384_KEY_PUB_GEN_CMD	0x00202f22U
#define PKI_ECC_NIST_P521_KEY_PUB_GEN_CMD	0x00304122U

#define XPKI_DESC_LEN_BYTES             0x20U
#define XPKI_SIGN_P521_PADD_BYTES       0x2U
#define XPKI_VERIFY_P521_PADD_BYTES     0x6U
#define XPKI_ECDSA_NEW_REQUEST_MASK     0x00000FFFU

#define OP_SIZE(CurveLen, OPCount)		((CurveLen * OPCount) + 32U)
#define INPUT_OP_SIZE(CurveLen, OPCount)	(CurveLen * OPCount)
#define OUTPUT_OP_SIZE(CurveLen, OPCount)	(CurveLen * OPCount)

#define GET_QUEUE_ID(RequestID)         (((RequestID >> 28U) & 0xFU) - 1U)
#define GET_QUEUE_SLOT_ID(RequestID)    (((RequestID >> 16U) & 0xFFFU) - 1U)

/* Queue Page Addresses */
#define GET_QUEUE_PAGE_ADDR(PkiMem, QueueID)    ((UINTPTR)(PkiMem + (QueueID * 3 * XPKI_PAGE_SIZE)))

/**************************** Type Definitions *******************************/
typedef struct {
    u32 Size;  /**< Crypto Operation required total size */
    u32 OpsCmd; /**< Crypto Operation Command */
    u32 InPutSize; /**< Crypto Operation Input size */
    u32 OutPutSize; /**< Crypto Operation Output size */
    Xpki_OpsType OpsType; /**< Crypto Operation Type */
} XPki_OpsInfo;

/************************** Variable Definitions *****************************/
static XScuGic InterruptController;    /** The instance of the Interrupt Controller. */
static u8 PkiMemory[0x200000] __attribute__((aligned(0x200000)));
static XPki_OpsInfo OpsInfo[] = {
	[PKI_ECC_NIST_P192_SIGN] = {
		.Size = OP_SIZE(NIST_P192_LEN_BYTES, SIGN_INPUT_OP_COUNT),
		.InPutSize = INPUT_OP_SIZE(NIST_P192_LEN_BYTES, SIGN_INPUT_OP_COUNT),
		.OutPutSize = OUTPUT_OP_SIZE(NIST_P192_LEN_BYTES, SIGN_OUTPUT_OP_COUNT),
		.OpsType = PKI_ECC_NIST_P192_SIGN,
		.OpsCmd = PKI_ECC_NIST_P192_SIGN_CMD
	},
	[PKI_ECC_NIST_P256_SIGN] = {
		.Size = OP_SIZE(NIST_P256_LEN_BYTES, SIGN_INPUT_OP_COUNT),
		.InPutSize = INPUT_OP_SIZE(NIST_P256_LEN_BYTES, SIGN_INPUT_OP_COUNT),
		.OutPutSize = OUTPUT_OP_SIZE(NIST_P256_LEN_BYTES, SIGN_OUTPUT_OP_COUNT),
		.OpsType = PKI_ECC_NIST_P256_SIGN,
		.OpsCmd = PKI_ECC_NIST_P256_SIGN_CMD
	},
	[PKI_ECC_NIST_P384_SIGN] = {
		.Size = OP_SIZE(NIST_P384_LEN_BYTES, SIGN_INPUT_OP_COUNT),
		.InPutSize = INPUT_OP_SIZE(NIST_P384_LEN_BYTES, SIGN_INPUT_OP_COUNT),
		.OutPutSize = OUTPUT_OP_SIZE(NIST_P384_LEN_BYTES, SIGN_OUTPUT_OP_COUNT),
		.OpsType = PKI_ECC_NIST_P384_SIGN,
		.OpsCmd = PKI_ECC_NIST_P384_SIGN_CMD
	},
	[PKI_ECC_NIST_P521_SIGN] = {
		.Size = OP_SIZE(NIST_P521_LEN_BYTES, SIGN_INPUT_OP_COUNT) + P521_PADD_2_BYTES,
		.InPutSize = INPUT_OP_SIZE(NIST_P521_LEN_BYTES, SIGN_INPUT_OP_COUNT),
		.OutPutSize = OUTPUT_OP_SIZE(NIST_P521_LEN_BYTES, SIGN_OUTPUT_OP_COUNT),
		.OpsType = PKI_ECC_NIST_P521_SIGN,
		.OpsCmd = PKI_ECC_NIST_P521_SIGN_CMD
	},
	[PKI_ECC_NIST_P192_SIGN_VERIFY] = {
		.Size = OP_SIZE(NIST_P192_LEN_BYTES, SIGN_VERIFY_INPUT_OP_COUNT),
		.InPutSize = INPUT_OP_SIZE(NIST_P192_LEN_BYTES, SIGN_VERIFY_INPUT_OP_COUNT),
		.OutPutSize = OUTPUT_OP_SIZE(NIST_P192_LEN_BYTES, SIGN_VERIFY_OUTPUT_OP_COUNT),
		.OpsType = PKI_ECC_NIST_P192_SIGN_VERIFY,
		.OpsCmd = PKI_ECC_NIST_P192_SIGN_VERIFY_CMD
	},
	[PKI_ECC_NIST_P256_SIGN_VERIFY] = {
		.Size = OP_SIZE(NIST_P256_LEN_BYTES, SIGN_VERIFY_INPUT_OP_COUNT),
		.InPutSize = INPUT_OP_SIZE(NIST_P256_LEN_BYTES, SIGN_VERIFY_INPUT_OP_COUNT),
		.OutPutSize = OUTPUT_OP_SIZE(NIST_P256_LEN_BYTES, SIGN_VERIFY_OUTPUT_OP_COUNT),
		.OpsType = PKI_ECC_NIST_P256_SIGN_VERIFY,
		.OpsCmd = PKI_ECC_NIST_P256_SIGN_VERIFY_CMD
	},
	[PKI_ECC_NIST_P384_SIGN_VERIFY] = {
		.Size = OP_SIZE(NIST_P384_LEN_BYTES, SIGN_VERIFY_INPUT_OP_COUNT),
		.InPutSize = INPUT_OP_SIZE(NIST_P384_LEN_BYTES, SIGN_VERIFY_INPUT_OP_COUNT),
		.OutPutSize = OUTPUT_OP_SIZE(NIST_P384_LEN_BYTES, SIGN_VERIFY_OUTPUT_OP_COUNT),
		.OpsType = PKI_ECC_NIST_P384_SIGN_VERIFY,
		.OpsCmd = PKI_ECC_NIST_P384_SIGN_VERIFY_CMD
	},
	[PKI_ECC_NIST_P521_SIGN_VERIFY] = {
		.Size = OP_SIZE(NIST_P521_LEN_BYTES, SIGN_VERIFY_INPUT_OP_COUNT) + P521_PADD_6_BYTES,
		.InPutSize = INPUT_OP_SIZE(NIST_P521_LEN_BYTES, SIGN_VERIFY_INPUT_OP_COUNT),
		.OutPutSize = OUTPUT_OP_SIZE(NIST_P521_LEN_BYTES, SIGN_VERIFY_OUTPUT_OP_COUNT),
		.OpsType = PKI_ECC_NIST_P521_SIGN_VERIFY,
		.OpsCmd = PKI_ECC_NIST_P521_SIGN_VERIFY_CMD
	},
	[PKI_ECC_NIST_P192_KEY_PRIV_GEN] = {
		.Size = OP_SIZE(NIST_P192_LEN_BYTES, PRIV_KEY_INPUT_OP_COUNT),
		.InPutSize = INPUT_OP_SIZE(NIST_P192_LEN_BYTES, PRIV_KEY_INPUT_OP_COUNT),
		.OutPutSize = OUTPUT_OP_SIZE(NIST_P192_LEN_BYTES, PRIV_KEY_OUTPUT_OP_COUNT),
		.OpsType = PKI_ECC_NIST_P192_KEY_PRIV_GEN,
		.OpsCmd = PKI_ECC_NIST_P192_KEY_PRIV_GEN_CMD
	},
	[PKI_ECC_NIST_P256_KEY_PRIV_GEN] = {
		.Size = OP_SIZE(NIST_P256_LEN_BYTES, PRIV_KEY_INPUT_OP_COUNT),
		.InPutSize = INPUT_OP_SIZE(NIST_P256_LEN_BYTES, PRIV_KEY_INPUT_OP_COUNT),
		.OutPutSize = OUTPUT_OP_SIZE(NIST_P256_LEN_BYTES, PRIV_KEY_OUTPUT_OP_COUNT),
		.OpsType = PKI_ECC_NIST_P256_KEY_PRIV_GEN,
		.OpsCmd = PKI_ECC_NIST_P256_KEY_PRIV_GEN_CMD
	},
	[PKI_ECC_NIST_P384_KEY_PRIV_GEN] = {
		.Size = OP_SIZE(NIST_P384_LEN_BYTES, PRIV_KEY_INPUT_OP_COUNT),
		.InPutSize = INPUT_OP_SIZE(NIST_P384_LEN_BYTES, PRIV_KEY_INPUT_OP_COUNT),
		.OutPutSize = OUTPUT_OP_SIZE(NIST_P384_LEN_BYTES, PRIV_KEY_OUTPUT_OP_COUNT),
		.OpsType = PKI_ECC_NIST_P384_KEY_PRIV_GEN,
		.OpsCmd = PKI_ECC_NIST_P384_KEY_PRIV_GEN_CMD
	},
	[PKI_ECC_NIST_P521_KEY_PRIV_GEN] = {
		.Size = OP_SIZE(NIST_P521_LEN_BYTES, PRIV_KEY_INPUT_OP_COUNT) + P521_PADD_2_BYTES,
		.InPutSize = INPUT_OP_SIZE(NIST_P521_LEN_BYTES, PRIV_KEY_INPUT_OP_COUNT),
		.OutPutSize = OUTPUT_OP_SIZE(NIST_P521_LEN_BYTES, PRIV_KEY_OUTPUT_OP_COUNT),
		.OpsType = PKI_ECC_NIST_P521_KEY_PRIV_GEN,
		.OpsCmd = PKI_ECC_NIST_P521_KEY_PRIV_GEN_CMD
	},
	[PKI_ECC_NIST_P192_KEY_PUB_GEN] = {
		.Size = OP_SIZE(NIST_P192_LEN_BYTES, PUB_KEY_INPUT_OP_COUNT),
		.InPutSize = INPUT_OP_SIZE(NIST_P192_LEN_BYTES, PUB_KEY_INPUT_OP_COUNT),
		.OutPutSize = OUTPUT_OP_SIZE(NIST_P192_LEN_BYTES, PUB_KEY_OUTPUT_OP_COUNT),
		.OpsType = PKI_ECC_NIST_P192_KEY_PUB_GEN,
		.OpsCmd = PKI_ECC_NIST_P192_KEY_PUB_GEN_CMD
	},
	[PKI_ECC_NIST_P256_KEY_PUB_GEN] = {
		.Size = OP_SIZE(NIST_P256_LEN_BYTES, PUB_KEY_INPUT_OP_COUNT),
		.InPutSize = INPUT_OP_SIZE(NIST_P256_LEN_BYTES, PUB_KEY_INPUT_OP_COUNT),
		.OutPutSize = OUTPUT_OP_SIZE(NIST_P256_LEN_BYTES, PUB_KEY_OUTPUT_OP_COUNT),
		.OpsType = PKI_ECC_NIST_P256_KEY_PUB_GEN,
		.OpsCmd = PKI_ECC_NIST_P256_KEY_PUB_GEN_CMD
	},
	[PKI_ECC_NIST_P384_KEY_PUB_GEN] = {
		.Size = OP_SIZE(NIST_P384_LEN_BYTES, PUB_KEY_INPUT_OP_COUNT),
		.InPutSize = INPUT_OP_SIZE(NIST_P384_LEN_BYTES, PUB_KEY_INPUT_OP_COUNT),
		.OutPutSize = OUTPUT_OP_SIZE(NIST_P384_LEN_BYTES, PUB_KEY_OUTPUT_OP_COUNT),
		.OpsType = PKI_ECC_NIST_P384_KEY_PUB_GEN,
		.OpsCmd = PKI_ECC_NIST_P384_KEY_PUB_GEN_CMD
	},
	[PKI_ECC_NIST_P521_KEY_PUB_GEN] = {
		.Size = OP_SIZE(NIST_P521_LEN_BYTES, PUB_KEY_INPUT_OP_COUNT) + P521_PADD_2_BYTES,
		.InPutSize = INPUT_OP_SIZE(NIST_P521_LEN_BYTES, PUB_KEY_INPUT_OP_COUNT),
		.OutPutSize = OUTPUT_OP_SIZE(NIST_P521_LEN_BYTES, PUB_KEY_OUTPUT_OP_COUNT),
		.OpsType = PKI_ECC_NIST_P521_KEY_PUB_GEN,
		.OpsCmd = PKI_ECC_NIST_P521_KEY_PUB_GEN_CMD
	},
};

static const u8 Order_P192[] = { 0x31, 0x28, 0xd2, 0xb4, 0xb1, 0xc9, 0x6b,
				 0x14, 0x36, 0xf8, 0xde, 0x99, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0x00
			       };

static const u8 Order_P256[] = { 0x51, 0x25, 0x63, 0xfc, 0xc2, 0xca, 0xb9,
				 0xf3, 0x84, 0x9e, 0x17, 0xa7, 0xad, 0xfa,
				 0xe6, 0xbc, 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
				 0xff, 0xff, 0xff, 0xff, 0x00
			       };


static const u8 Order_P384[] = { 0x73, 0x29, 0xc5, 0xcc, 0x6a, 0x19, 0xec,
				 0xec, 0x7a, 0xa7, 0xb0, 0x48, 0xb2, 0x0d,
				 0x1a, 0x58, 0xdf, 0x2d, 0x37, 0xf4, 0x81,
				 0x4d, 0x63, 0xc7, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00
			       };

static const u8 Order_P521[] = { 0x09, 0x64, 0x38, 0x91, 0x1e, 0xb7, 0x6f,
				 0xbb, 0xae, 0x47, 0x9c, 0x89, 0xb8, 0xc9,
				 0xb5, 0x3b, 0xd0, 0xa5, 0x09, 0xf7, 0x48,
				 0x01, 0xcc, 0x7f, 0x6b, 0x96, 0x2f, 0xbf,
				 0x83, 0x87, 0x86, 0x51, 0xfa, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				 0xff, 0xff, 0x01
			       };

static const u8 EcdsaGpoint_P192_Gx[] = { 0x12, 0x10, 0xff, 0x82, 0xfd, 0x0a,
					  0xff, 0xf4, 0x00, 0x88, 0xa1, 0x43,
					  0xeb, 0x20, 0xbf, 0x7c, 0xf6, 0x90,
					  0x30, 0xb0, 0x0e, 0xa8, 0x8d, 0x18
					};

static const u8 EcdsaGpoint_P192_Gy[] = { 0x11, 0x48, 0x79, 0x1e, 0xa1, 0x77,
					  0xf9, 0x73, 0xd5, 0xcd, 0x24, 0x6b,
					  0xed, 0x11, 0x10, 0x63, 0x78, 0xda,
					  0xc8, 0xff, 0x95, 0x2b, 0x19, 0x07
					};

static const u8 EcdsaGpoint_P256_Gx[] = { 0x96, 0xc2, 0x98, 0xd8, 0x45, 0x39,
					  0xa1, 0xf4, 0xa0, 0x33, 0xeb, 0x2d,
					  0x81, 0x7d, 0x03, 0x77, 0xf2, 0x40,
					  0xa4, 0x63, 0xe5, 0xe6, 0xbc, 0xf8,
					  0x47, 0x42, 0x2c, 0xe1, 0xf2, 0xd1,
					  0x17, 0x6b
					};

static const u8 EcdsaGpoint_P256_Gy[] = { 0xf5, 0x51, 0xbf, 0x37, 0x68, 0x40,
					  0xb6, 0xcb, 0xce, 0x5e, 0x31, 0x6b,
					  0x57, 0x33, 0xce, 0x2b, 0x16, 0x9e,
					  0x0f, 0x7c, 0x4a, 0xeb, 0xe7, 0x8e,
					  0x9b, 0x7f, 0x1a, 0xfe, 0xe2, 0x42,
					  0xe3, 0x4f
					};

static const u8 EcdsaGpoint_P384_Gx[] = { 0xb7, 0x0a, 0x76, 0x72, 0x38, 0x5e,
					  0x54, 0x3a, 0x6c, 0x29, 0x55, 0xbf,
					  0x5d, 0xf2, 0x02, 0x55, 0x38, 0x2a,
					  0x54, 0x82, 0xe0, 0x41, 0xf7, 0x59,
					  0x98, 0x9b, 0xa7, 0x8b, 0x62, 0x3b,
					  0x1d, 0x6e, 0x74, 0xad, 0x20, 0xf3,
					  0x1e, 0xc7, 0xb1, 0x8e, 0x37, 0x05,
					  0x8b, 0xbe, 0x22, 0xca, 0x87, 0xaa
					};

static const u8 EcdsaGpoint_P384_Gy[] = { 0x5f, 0x0e, 0xea, 0x90, 0x7c, 0x1d,
					  0x43, 0x7a, 0x9d, 0x81, 0x7e, 0x1d,
					  0xce, 0xb1, 0x60, 0x0a, 0xc0, 0xb8,
					  0xf0, 0xb5, 0x13, 0x31, 0xda, 0xe9,
					  0x7c, 0x14, 0x9a, 0x28, 0xbd, 0x1d,
					  0xf4, 0xf8, 0x29, 0xdc, 0x92, 0x92,
					  0xbf, 0x98, 0x9e, 0x5d, 0x6f, 0x2c,
					  0x26, 0x96, 0x4a, 0xde, 0x17, 0x36
					};

static const u8 EcdsaGpoint_P521_Gx[] = { 0x66, 0xbd, 0xe5, 0xc2, 0x31, 0x7e,
					  0x7e, 0xf9, 0x9b, 0x42, 0x6a, 0x85,
					  0xc1, 0xb3, 0x48, 0x33, 0xde, 0xa8,
					  0xff, 0xa2, 0x27, 0xc1, 0x1d, 0xfe,
					  0x28, 0x59, 0xe7, 0xef, 0x77, 0x5e,
					  0x4b, 0xa1, 0xba, 0x3d, 0x4d, 0x6b,
					  0x60, 0xaf, 0x28, 0xf8, 0x21, 0xb5,
					  0x3f, 0x05, 0x39, 0x81, 0x64, 0x9c,
					  0x42, 0xb4, 0x95, 0x23, 0x66, 0xcb,
					  0x3e, 0x9e, 0xcd, 0xe9, 0x04, 0x04,
					  0xb7, 0x06, 0x8e, 0x85, 0xc6, 0x00
					};

static const u8 EcdsaGpoint_P521_Gy[] = { 0x50, 0x66, 0xd1, 0x9f, 0x76, 0x94,
					  0xbe, 0x88, 0x40, 0xc2, 0x72, 0xa2,
					  0x86, 0x70, 0x3c, 0x35, 0x61, 0x07,
					  0xad, 0x3f, 0x01, 0xb9, 0x50, 0xc5,
					  0x40, 0x26, 0xf4, 0x5e, 0x99, 0x72,
					  0xee, 0x97, 0x2c, 0x66, 0x3e, 0x27,
					  0x17, 0xbd, 0xaf, 0x17, 0x68, 0x44,
					  0x9b, 0x57, 0x49, 0x44, 0xf5, 0x98,
					  0xd9, 0x1b, 0x7d, 0x2c, 0xb4, 0x5f,
					  0x8a, 0x5c, 0x04, 0xc0, 0x3b, 0x9a,
					  0x78, 0x6a, 0x29, 0x39, 0x18, 0x01
					};

/************************** Function Prototypes ******************************/
static int XPki_TrngInit(void);
static XTrngpsx_Instance *XPki_Get_Trng_InstancePtr(u8 DeviceId);
static inline int XPki_GetFreeSlot(XPki_Instance *InstancePtr,
				   XPki_QueueID QueueID, u32 *FreeIndex);
static inline int XPki_GetQueueID(XPki_Instance *InstancePtr,
				  u32 OpSize, XPki_QueueID *QueueID);
static void XPki_PrepDescriptor(XPki_Instance *InstancePtr, Xpki_OpsType Ops,
				u32 RequestID, u32 *QDescPtr);
static inline void XPki_UpdateQueueInfo(XPki_Instance *InstancePtr,
					XPki_Request_Info *Request_InfoPtr, u32 RequestID);
static u8 XPki_GetEccCurveLen(XPki_EcdsaCrvType CrvType);
static int XPki_LoadInputData_EcdsaGenerateSign(XPki_EcdsaSignInputData *SignParams,
		UINTPTR Addr, u32 *DescPtr, u64 *EcdsaReqVal);
static int XPki_ValidateEcdsaGenerateSignParam(XPki_EcdsaSignInputData *SignParams);
static void XPki_TrigQueueOps(XPki_QueueID QueueID, u64 TrigVal);
static int XPki_LoadQueueData(XPki_Instance *InstancePtr,
			      XPki_Request_Info *Request_InfoPtr, u32 *QDescPtr, u64 *TrigVal);
static int XPki_ValidateEcdsaVerifySignParam(XPki_EcdsaVerifyInputData *VerifyParams);
static int XPki_LoadInputData_EcdsaVerifySign(XPki_EcdsaVerifyInputData *VerifyParams,
		UINTPTR Addr, u32 *DescPtr, u64 *EcdsaReqVal);
static const u8 *XPki_GetEccCurveOrder(XPki_EcdsaCrvType CrvType);
static int XPki_GenEccPrivateKey(XPki_EcdsaCrvType CrvType, UINTPTR Addr,
				 u32 *DescPtr, u64 *EcdsaReqVa);
static int XPki_LoadInputData_ModulusAdd(XPki_ModAddInputData *ModAddParams,
		UINTPTR Addr, u32 *DescPtr, u64 *EcdsaReqVal);
static XPki_EcdsaGpoint *XPki_GetEccGpoint(XPki_EcdsaCrvType CrvType);
static int XPki_GetEccCurveType(Xpki_OpsType OpsTpe, XPki_EcdsaCrvType *CrvType);
static int XPki_EcdsaGenPubKey(XPki_EcdsaCrvType CrvType, u8 *PrivKey, UINTPTR Addr,
			       u32 *DescPtr, u64 *EcdsaReqVa);
static int XPki_FreeQueueInfo(XPki_Instance *InstancePtr, u32 RequestID);
static int XPki_DeQueueData(XPki_Instance *InstancePtr,
			    XPki_Request_Info *Request_InfoPtr,  u32 RequestID);
static int XPki_CopyEcdsaSigature(XPki_EcdsaCrvType CrvType, u8 *Addr,
				  XPki_EcdsaSign *SignPtr);
static int XPki_CopyEcdsaPubKey(XPki_EcdsaCrvType CrvType, u8 *Addr,
				XPki_EcdsaKey *PubKey);
static int XPki_CopyEcdsaPrivKey(XPki_EcdsaCrvType CrvType, u8 *Addr,
				 u8 *PrivKey);
static void XPki_IntrHandler(XPki_Instance *InstancePtr);
static int XPki_SetupInterruptSystem(XPki_Instance *InstancePtr);
static int XilPki_Queue_Init(XPki_Instance *InstancePtr, XPki_QueueID QueueID);
static void XPki_IntrCallbackHandler(XPki_Instance *InstancePtr, XPki_QueueID Id);
static int XPki_ValidateReqID(XPki_Instance *InstancePtr, u32 RequestID);
static inline void XilPki_Queue_MapPageAddr(XPki_Instance *InstancePtr);
/*****************************************************************************/
/**
 * @brief	This function reset the PKI module.
 *
******************************************************************************/
void XPki_Reset(void)
{
	/* Reset PKI module */
	Xil_Out32(PSX_CRF_RST_PKI, XPKI_ASSERT_RESET);
	usleep(XPKI_RESET_DELAY_US);
	Xil_Out32(PSX_CRF_RST_PKI, XPKI_DEASSERT_RESET);
}

/*****************************************************************************/
/**
 * @brief	This function performs the PKI module soft reset.
 *
******************************************************************************/
void XPki_SoftReset(void)
{
	/* PKI Soft reset */
	Xil_Out32(FPD_PKI_SOFT_RESET, XPKI_ASSERT_RESET);
	usleep(XPKI_RESET_DELAY_US);
	Xil_Out32(FPD_PKI_SOFT_RESET, XPKI_DEASSERT_RESET);
}

/*****************************************************************************/
/**
 * @brief	This function performs the PKI module initialization.
 *
 * @param	InstancePtr	Pointer to the XPki instance
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XPKI_INVALID_PARAM - On invalid argument
 *	-	XPKI_ERROR_UNALIGN_ADDR - On Request/Completion queue
 *						  Address are not Word aligned.
 *	-	XST_FAILURE - On failure
******************************************************************************/
int XPki_Initialize(XPki_Instance *InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u64 RegVal;
	u8 QId;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = XPKI_INVALID_PARAM;
		goto END;
	}

	InstancePtr->RQCount = 0U;

	/* Clear fpd slcr write protection reg */
	Xil_Out32(FPD_SLCR_WPROT0, FPD_CLEAR_WRITE_PROTECT);

	/* Pki mux selection */
	Xil_UtilRMW32(FPD_SLCR_PKI_MUX_SEL, FPD_SLCR_PKI_MUX_SEL_FULLRWMASK,
		      XPKI_MUX_SELECT);

	/* Enable fpd slcr write protection reg */
	Xil_Out32(FPD_SLCR_WPROT0, FPD_SLCR_WPROT0_DEFVAL);

	/* Release pki reset */
	XPki_Reset();

	/* Enable/Disable CM */
	RegVal = Xil_In64(FPD_PKI_ENGINE_CTRL);

	if (InstancePtr->Is_Cm_Enabled) {
		RegVal &=  ~(FPD_PKI_ENGINE_CM_MASK);
	} else {
		RegVal |= FPD_PKI_ENGINE_CM_MASK;
	}

	Xil_Out64(FPD_PKI_ENGINE_CTRL, RegVal);

	XPki_SoftReset();

	Status = XPki_TrngInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_SetTlbAttributes(GET_QUEUE_PAGE_ADDR(PkiMemory, PKI_QUEUE_ID_0),
			     NORM_NONCACHE | INNER_SHAREABLE);

	XilPki_Queue_MapPageAddr(InstancePtr);

	for (QId = PKI_QUEUE_ID_0; QId <= PKI_QUEUE_ID_3; QId++) {
		Status = XilPki_Queue_Init(InstancePtr, QId);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XPki_SetupInterruptSystem(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to set IRQ error: 0x%x\r\n", Status);
		goto END;
	}

	/* Enable PKI Interrupts */
	Xil_Out64(FPD_PKI_IRQ_ENABLE, XPKI_IRQ_ENABLE_VAL);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to disable access to the PKI Hardware.
 *
******************************************************************************/
void XPki_Close(void)
{
	/* Clear fpd slcr write protection reg */
	Xil_Out32(FPD_SLCR_WPROT0, FPD_CLEAR_WRITE_PROTECT);

	/* Pki mux Deselection */
	Xil_UtilRMW32(FPD_SLCR_PKI_MUX_SEL, FPD_SLCR_PKI_MUX_SEL_FULLRWMASK,
		      FPD_SLCR_PKI_MUX_SEL_DEFVAL);

	/* Enable fpd slcr write protection reg */
	Xil_Out32(FPD_SLCR_WPROT0, FPD_SLCR_WPROT0_DEFVAL);
}

/*****************************************************************************/
/**
 * @brief	This function is used to get the TRNG Instance.
 *
 * @return	NULL or Pointer to the TRNG Instance.
 *
 *****************************************************************************/
static XTrngpsx_Instance *XPki_Get_Trng_InstancePtr(u8 DeviceId)
{
	static XTrngpsx_Instance Trngpsx[XPAR_XTRNGPSX_NUM_INSTANCES];
	if ((DeviceId == 0) || ( DeviceId >= XPAR_XTRNGPSX_NUM_INSTANCES)) {
		return NULL;
	}

	return &Trngpsx[DeviceId];
}

/*****************************************************************************/
/**
 * @brief       This function is used to initialize the TRNG driver.
 *
 * @return      Error code on any error
 *              XST_SUCCESS on success
 *
 *****************************************************************************/
static int XPki_TrngInit(void)
{
	volatile int Status = XST_FAILURE;
	XTrngpsx_Config *Config;
	XTrngpsx_Instance *Trngpsx;
	XTrngpsx_UserConfig UsrCfg = {
		.Mode = XTRNGPSX_HRNG_MODE,
		.SeedLife = XTRNGPSX_USER_CFG_SEED_LIFE,
		.DFLength = XTRNGPSX_USER_CFG_DF_LENGTH,
		.AdaptPropTestCutoff = XTRNGPSX_USER_CFG_ADAPT_TEST_CUTOFF,
		.RepCountTestCutoff = XTRNGPSX_USER_CFG_REP_TEST_CUTOFF,
	};

	for (u8 DeviceId = 1U; DeviceId < XPAR_XTRNGPSX_NUM_INSTANCES; DeviceId++) {
		/*
		 * Initialize the TRNGPSX driver so that it's ready to use look up
		 * configuration in the config table, then initialize it.
		 */
		Config = XTrngpsx_LookupConfig(DeviceId);
		if (NULL == Config) {
			goto END;
		}

		Trngpsx = XPki_Get_Trng_InstancePtr(DeviceId);
		if (Trngpsx == NULL) {
			goto END;
		}

		/* Initialize the TRNGPSX driver so that it is ready to use. */
		XSECURE_TEMPORAL_CHECK(END, Status, XTrngpsx_CfgInitialize,
				       Trngpsx, Config, Config->BaseAddress);

		XSECURE_TEMPORAL_CHECK(END, Status, XTrngpsx_PreOperationalSelfTests, Trngpsx);

		/* Instantiate to complete initialization and for (initial) reseeding */
		XSECURE_TEMPORAL_CHECK(END, Status, XTrngpsx_Instantiate, Trngpsx,
				       NULL, 0U, NULL, &UsrCfg);
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to generate the Random number for the give
 *		size.
 * @param   GenSize Required Random Number length in bytes.
 * @param	RandBuf Pointer to store the Random Number.
 *
 * @return
 *      -       XST_SUCCESS - On success
 *      -       XST_FAILURE - On failure
******************************************************************************/
int XPki_TrngGenerateRandomNum(u8 GenSize, u8 *RandBuf)
{
	XTrngpsx_Instance *Trngpsx = XPki_Get_Trng_InstancePtr(XPKI_TRNG_INSTANCE);
	u8 Count = (GenSize / XPKI_TRNG_BUF_SIZE);
	volatile int Status = XST_FAILURE;

	if (GenSize % XPKI_TRNG_BUF_SIZE != 0) {
		Count++;
	}

	for (u8 i = 0U; i < Count; i++) {
		XSECURE_TEMPORAL_CHECK(END, Status, XTrngpsx_Generate, Trngpsx,
				       RandBuf, XPKI_TRNG_BUF_SIZE, FALSE);
		RandBuf += XPKI_TRNG_BUF_SIZE;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function is used to assign the page address to the PKI
 *		Queues.
 * @param       InstancePtr     Pointer to the XPki instance.
 *
******************************************************************************/
static inline void XilPki_Queue_MapPageAddr(XPki_Instance *InstancePtr)
{
	u8 QueueID = 0U;
	UINTPTR Addr = (UINTPTR)PkiMemory;

        for (QueueID = 0; QueueID < 4; QueueID++) {
                InstancePtr->MultiQinfo[QueueID].RQInputAddr = GET_QUEUE_PAGE_ADDR(Addr, QueueID);
                InstancePtr->MultiQinfo[QueueID].RQOutputAddr = InstancePtr->MultiQinfo[QueueID].RQInputAddr + XPKI_PAGE_SIZE;
                InstancePtr->MultiQinfo[QueueID].CQAddr = InstancePtr->MultiQinfo[QueueID].RQOutputAddr + XPKI_PAGE_SIZE;
        }
}

/*****************************************************************************/
/**
 * @brief       This function initialize the requested queue.
 *
 * @param       InstancePtr     Pointer to the XPki instance
 * @param	QueueID		Queue ID(PKI_QUEUE_ID_0/1/2/3).
 *
 * @return
 *      -XST_SUCCESS - On success
 *      -XPKI_INVALID_PARAM - On invalid argument
 *      -XPKI_ERROR_UNALIGN_ADDR - On Request/Completion queue Address are not Word aligned.
 *		-XPKI_INVALID_QUEUE_ID - If the provided QueueID is Invalid.
 *      -XST_FAILURE - On failure
******************************************************************************/

static int XilPki_Queue_Init(XPki_Instance *InstancePtr, XPki_QueueID QueueID)
{
	UINTPTR RQOutputAddr = InstancePtr->MultiQinfo[QueueID].RQOutputAddr;
	UINTPTR RQInputAddr = InstancePtr->MultiQinfo[QueueID].RQInputAddr;
	UINTPTR CQAddr = InstancePtr->MultiQinfo[QueueID].CQAddr;
	volatile int Status = XST_FAILURE;
	u32 RegOffset, QSlotSize;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = XPKI_INVALID_PARAM;
		goto END;
	}

	if (QueueID > PKI_QUEUE_ID_3) {
		Status = XPKI_INVALID_QUEUE_ID;
		goto END;
	}

	if (((RQInputAddr & XPKI_ADDR_WORD_ALIGN_MASK) != 0U) ||
	    ((RQOutputAddr & XPKI_ADDR_WORD_ALIGN_MASK) != 0U) ||
	    ((CQAddr & XPKI_ADDR_WORD_ALIGN_MASK) != 0U)) {
		Status = XPKI_ERROR_UNALIGN_ADDR;
		goto END;
	}

	switch (QueueID) {
		case PKI_QUEUE_ID_0:
			QSlotSize = PKI_QUEUE_0_SLOT_SIZE_BYTES;
			break;
		case PKI_QUEUE_ID_1:
			QSlotSize = PKI_QUEUE_1_SLOT_SIZE_BYTES;
			break;
		case PKI_QUEUE_ID_2:
			QSlotSize = PKI_QUEUE_2_SLOT_SIZE_BYTES;
			break;
		case PKI_QUEUE_ID_3:
			QSlotSize = PKI_QUEUE_3_SLOT_SIZE_BYTES;
			break;
		default:
			Status = XPKI_INVALID_QUEUE_ID;
			goto END;
	}

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)RQInputAddr,
			       XPKI_PAGE_SIZE, 0U, XPKI_PAGE_SIZE);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)RQOutputAddr,
			       XPKI_PAGE_SIZE, 0U, XPKI_PAGE_SIZE);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)CQAddr,
			       XPKI_PAGE_SIZE, 0U, XPKI_PAGE_SIZE);

	InstancePtr->MultiQinfo[QueueID].QSlotSize = QSlotSize;
	InstancePtr->MultiQinfo[QueueID].QMaxSlots = XPKI_PAGE_SIZE / QSlotSize;
	InstancePtr->MultiQinfo[QueueID].QFreeSlots = XPKI_PAGE_SIZE / QSlotSize;

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet,
			       (u8 *)InstancePtr->MultiQinfo[QueueID].RQ_SubmitList,
			       InstancePtr->MultiQinfo[QueueID].QMaxSlots * sizeof(int), 0U,
			       InstancePtr->MultiQinfo[QueueID].QMaxSlots * sizeof(int));

	RegOffset = QueueID * XPKI_INDEX_1_VAL;

	Xil_Out32(FPD_PKI_RQ_CFG_PERMISSIONS + RegOffset, XPKI_RQ_CFG_PERMISSIONS_SAFE);
	Xil_Out64(FPD_PKI_RQ_CFG_PAGE_ADDR_INPUT + RegOffset, RQInputAddr);
	Xil_Out64(FPD_PKI_RQ_CFG_PAGE_ADDR_OUTPUT + RegOffset, RQOutputAddr);
	Xil_Out64(FPD_PKI_CQ_CFG_ADDR + RegOffset, CQAddr);
	Xil_Out32(FPD_PKI_RQ_CFG_PAGE_SIZE + RegOffset, XPKI_RQ_CFG_PAGE_SIZE_4096);
	Xil_Out32(FPD_PKI_RQ_CFG_CQID + RegOffset, XPKI_RQ_CFG_CQID + QueueID);
	Xil_Out32(FPD_PKI_CQ_CFG_SIZE + RegOffset, XPKI_CQ_CFG_SIZE_4096);
	Xil_Out32(FPD_PKI_CQ_CFG_IRQ_IDX + RegOffset, XPKI_CQ_CFG_IRQ_ID_VAL + QueueID);
	Xil_Out32(FPD_PKI_RQ_CFG_QUEUE_DEPTH + RegOffset, XPKI_RQ_CFG_QUEUE_DEPTH_VAL);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief      This function is used to submit the crypto operation to the PKI
 *		Queue.
 *
 * @param   InstancePtr     Pointer to the XPki instance
 * @param   Request_InfoPtr	Pointer to the queue info structure
 * @param	RequestID	Pointer to the RequestID. if the request is
 *				successfully submitted it filled with unique id
 * @return
 *      -       XST_SUCCESS - On success
 *      -       XPKI_INVALID_PARAM - On invalid argument
 *      -       XPKI_UNSUPPORTED_OPS - If the requested operation is
 *						  not supported.
 *      -       XST_FAILURE - On failure
******************************************************************************/
int XilPki_EnQueue(XPki_Instance *InstancePtr, XPki_Request_Info *Request_InfoPtr,
		   u32 *RequestID)
{
	volatile int Status = XST_FAILURE;
	XPki_QueueID QueueID = 0;
	u32 FreeSlotIndex = 0;
	u32 QDescData[8];
	u64 TrigVal = 0;
	u32 OpSize = 0;

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (Request_InfoPtr == NULL)) {
		Status = XPKI_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->RQCount > XPKI_REQ_MAX_COUNT) {
		Status = XPKI_QUEUE_FULL;
		goto END;
	}

	if (Request_InfoPtr->OpsType >= PKI_MAX_OPS) {
		Status = XPKI_UNSUPPORTED_OPS;
		goto END;
	}

	OpSize = OpsInfo[Request_InfoPtr->OpsType].Size;
	Status = XPki_GetQueueID(InstancePtr, OpSize, &QueueID);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPki_GetFreeSlot(InstancePtr, QueueID, &FreeSlotIndex);
	if (Status != XST_SUCCESS) {
		Status = XPKI_QUEUE_FULL;
		goto END;
	}

	*RequestID = DESC_REQ_ID(QueueID, FreeSlotIndex);

	XPki_PrepDescriptor(InstancePtr, Request_InfoPtr->OpsType,
			    *RequestID, QDescData);

	Status = XPki_LoadQueueData(InstancePtr, Request_InfoPtr, QDescData, &TrigVal);
	if (Status != XST_SUCCESS) {
		Status = XPKI_UNSUPPORTED_OPS;
		goto END;
	}

	/* Update Queue Info before trigger the Operation */
	XPki_UpdateQueueInfo(InstancePtr, Request_InfoPtr, *RequestID);

	/* Trigger the Operation */
	XPki_TrigQueueOps(QueueID, TrigVal);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function is used to get the crypto operation results from
 * the PKI Queue.
 *
 * @param       InstancePtr     Pointer to the XPki instance
 * @param       Request_InfoPtr   Pointer to the queue info structure
 * @param       RequestID       Unique request ID Filled by the XilPki_EnQueue()
 *				API.
 * @return
 *		- XST_SUCCESS - On success
 *		- XPKI_INVALID_PARAM - On invalid argument
 *		- XPKI_UNSUPPORTED_OPS - If the requested operation is not supported.
 *		- XST_FAILURE - On failure
******************************************************************************/
int XilPki_DeQueue(XPki_Instance *InstancePtr, XPki_Request_Info *Request_InfoPtr,
		   u32 RequestID)
{
	volatile int Status = XST_FAILURE;

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (Request_InfoPtr == NULL)) {
		Status = XPKI_INVALID_PARAM;
		goto END;
	}

	Status = XPki_ValidateReqID(InstancePtr, RequestID);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPki_DeQueueData(InstancePtr, Request_InfoPtr, RequestID);
END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function copy the ecdsa generated signature data into the
 *		user-pointed buffer.
 *
 * @param	CrvType  Type of ECC curve(NIST-P192, NIST-P256, P384 and P521)
 * @param	Addr  Pointer to the Request queue out address.
 * @param	SignPtr    Pointer to the XPki_EcdsaSign.
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XST_FAILURE - On failure
 *	-	XPKI_ECDSA_NON_SUPPORTED_CRV - On Un-supported elliptic curves
 *
 ******************************************************************************/
static int XPki_CopyEcdsaSigature(XPki_EcdsaCrvType CrvType, u8 *Addr,
				  XPki_EcdsaSign  *SignPtr)
{
	volatile int Status = XST_FAILURE;
	u8 EcdsaCrvlen = XPki_GetEccCurveLen(CrvType);

	if (EcdsaCrvlen == 0U) {
		Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
		goto END;
	}

	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, SignPtr->SignR,
			       EcdsaCrvlen, Addr, EcdsaCrvlen, EcdsaCrvlen);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, SignPtr->SignS,
			       EcdsaCrvlen, Addr + EcdsaCrvlen, EcdsaCrvlen,
			       EcdsaCrvlen);
END:

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function copy the ecdsa generated signature data into the
 *              user-pointed buffer.
 *
 * @param CrvType  Type of ECC curve(NIST-P192, NIST-P256, P384 and P521)
 * @param  Addr  Pointer to the Request queue out address
 * @param PubKey  Pointer to the pulic-key buffer
 *
 * @return
 *      -       XST_SUCCESS - On success
 *      -       XST_FAILURE - On failure
 *      -       XPKI_ECDSA_NON_SUPPORTED_CRV - On Un-supported elliptic curves
 *
 ******************************************************************************/
static int XPki_CopyEcdsaPubKey(XPki_EcdsaCrvType CrvType, u8 *Addr,
				XPki_EcdsaKey *PubKey)
{
	volatile int Status = XST_FAILURE;
	u8 EcdsaCrvlen = XPki_GetEccCurveLen(CrvType);

	if (EcdsaCrvlen == 0U) {
		Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
		goto END;
	}

	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)PubKey->Qx,
			       EcdsaCrvlen, Addr, EcdsaCrvlen, EcdsaCrvlen);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)PubKey->Qy,
			       EcdsaCrvlen, Addr + EcdsaCrvlen, EcdsaCrvlen,
			       EcdsaCrvlen);
END:

	return Status;

}

/*****************************************************************************/
/**
 * @brief       This function copy the ecdsa generated signature data into the
 *              user-pointed buffer.
 *
 * @param   CrvType  Type of ECC curve(NIST-P192, NIST-P256, P384 and P521)
 * @param   Addr  Pointer to the Request queue out address
 * @param	PrivKey Pointer to the Private-key buffer
 *
 * @return
 *      -       XST_SUCCESS - On success
 *      -       XST_FAILURE - On failure
 *      -       XPKI_ECDSA_NON_SUPPORTED_CRV - On Un-supported elliptic curves
 *
 ******************************************************************************/
static int XPki_CopyEcdsaPrivKey(XPki_EcdsaCrvType CrvType, u8 *Addr,
				 u8 *PrivKey)
{
	volatile int Status = XST_FAILURE;
	u8 EcdsaCrvlen = XPki_GetEccCurveLen(CrvType);

	if (EcdsaCrvlen == 0U) {
		Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
		goto END;
	}

	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, PrivKey,
			       EcdsaCrvlen, Addr, EcdsaCrvlen, EcdsaCrvlen);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function is used to get the crypto operation results from
 *              the PKI Queue.
 *
 * @param       InstancePtr     Pointer to the XPki instance
 * @param       Request_InfoPtr  Pointer to the queue info structure
 * @param       RequestID       Unique request ID Filled by the XilPki_EnQueue()
 *                              API
 * @return
 *      -       XST_SUCCESS - On success
 *      -       XPKI_UNSUPPORTED_OPS - If the requested operation is
 *                                                not supported
 *      -       XST_FAILURE - On failure
******************************************************************************/
static int XPki_DeQueueData(XPki_Instance *InstancePtr,
			    XPki_Request_Info *Request_InfoPtr,  u32 RequestID)
{
	XPki_EcdsaCrvType CrvType;
	volatile int Status = XST_FAILURE;
	u32 QueueID = GET_QUEUE_ID(RequestID);
	u32 QSlotSize = InstancePtr->MultiQinfo[QueueID].QSlotSize;
	u32 QSlotOffset = QSlotSize * GET_QUEUE_SLOT_ID(RequestID);
	u8 *Addr = (u8 *)InstancePtr->MultiQinfo[QueueID].RQOutputAddr + QSlotOffset;

	Status = XPki_GetEccCurveType(Request_InfoPtr->OpsType, &CrvType);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	switch (Request_InfoPtr->OpsType) {
		case PKI_ECC_NIST_P192_SIGN:
		case PKI_ECC_NIST_P256_SIGN:
		case PKI_ECC_NIST_P384_SIGN:
		case PKI_ECC_NIST_P521_SIGN:
			Status = XPki_CopyEcdsaSigature(CrvType, Addr,
							(XPki_EcdsaSign *)Request_InfoPtr->PtrOutputData);
			break;
		case PKI_ECC_NIST_P192_KEY_PRIV_GEN:
		case PKI_ECC_NIST_P256_KEY_PRIV_GEN:
		case PKI_ECC_NIST_P384_KEY_PRIV_GEN:
		case PKI_ECC_NIST_P521_KEY_PRIV_GEN:
			Status = XPki_CopyEcdsaPrivKey(CrvType, Addr,
						       (u8 *)Request_InfoPtr->PtrOutputData);
			break;
		case PKI_ECC_NIST_P192_KEY_PUB_GEN:
		case PKI_ECC_NIST_P256_KEY_PUB_GEN:
		case PKI_ECC_NIST_P384_KEY_PUB_GEN:
		case PKI_ECC_NIST_P521_KEY_PUB_GEN:
			Status = XPki_CopyEcdsaPubKey(CrvType, Addr,
						      (XPki_EcdsaKey *)Request_InfoPtr->PtrOutputData);
			break;
		case PKI_ECC_NIST_P192_SIGN_VERIFY:
		case PKI_ECC_NIST_P256_SIGN_VERIFY:
		case PKI_ECC_NIST_P384_SIGN_VERIFY:
		case PKI_ECC_NIST_P521_SIGN_VERIFY:
		case PKI_MAX_OPS:
		default:
			xil_printf("invalid Operation\r\n");
			Status = XPKI_UNSUPPORTED_OPS;
			goto END;
	}

	Status = XPki_FreeQueueInfo(InstancePtr, RequestID);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to initialize and trigger the
 *		PKI Operations.
 *
 * @param   QueueID  Queue ID(PKI_QUEUE_ID_0/1/2/3)
 * @param	TrigVal  RQ page offset value which points to Crypto OP
 *				descriptor buffer
 *
 ******************************************************************************/
static inline void XPki_TrigQueueOps(XPki_QueueID QueueID, u64 TrigVal)
{
	u32 RegOffset = QueueID * XPKI_INDEX_2_VAL;

	Xil_Out32(FPD_PKI_CQ_CTL_TRIGPOS + RegOffset, XPKI_CQ_CTL_TRIGPOS_VAL);
	Xil_Out64(FPD_PKI_RQ_CTL_NEW_REQUEST + RegOffset, TrigVal);
}

/*****************************************************************************/
/**
 * @brief       This function load the user-pointed crypto data (inputs) into
 *		the Request queue buffer (as per the PKI Spec).
 *
 * @param   InstancePtr     Pointer to the XPki instance
 * @param   Request_InfoPtr   Pointer to the queue info structure.
 * @param	QDescPtr	Pointer to the descriptor buffer
 * @param   TrigVal		Pointer to the Descriptor offset.
 * @return
 *      -       XST_SUCCESS                     - On success
 *      -       XPKI_UNSUPPORTED_OPS            - If the requested operation is
 *                                                not supported.
 *      -       XST_FAILURE                     - On failure
******************************************************************************/
static int XPki_LoadQueueData(XPki_Instance *InstancePtr,
			      XPki_Request_Info *Request_InfoPtr,
			      u32 *QDescPtr, u64 *TrigVal)
{
	volatile int Status = XST_FAILURE;
	XPki_EcdsaCrvType CrvType;
	u32 QueueID = GET_QUEUE_ID(QDescPtr[6]);
	u32 Offset = QDescPtr[3];
	UINTPTR Addr = InstancePtr->MultiQinfo[QueueID].RQInputAddr + Offset;

	switch (Request_InfoPtr->OpsType) {
		case PKI_ECC_NIST_P192_SIGN:
		case PKI_ECC_NIST_P256_SIGN:
		case PKI_ECC_NIST_P384_SIGN:
		case PKI_ECC_NIST_P521_SIGN:
			Status = XPki_ValidateEcdsaGenerateSignParam(
					 (XPki_EcdsaSignInputData *)Request_InfoPtr->PtrInputData);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Status = XPki_LoadInputData_EcdsaGenerateSign(
					 (XPki_EcdsaSignInputData *)Request_InfoPtr->PtrInputData,
					 Addr, QDescPtr, TrigVal);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			break;
		case PKI_ECC_NIST_P192_SIGN_VERIFY:
		case PKI_ECC_NIST_P256_SIGN_VERIFY:
		case PKI_ECC_NIST_P384_SIGN_VERIFY:
		case PKI_ECC_NIST_P521_SIGN_VERIFY:
			Status = XPki_ValidateEcdsaVerifySignParam(
					 (XPki_EcdsaVerifyInputData *)Request_InfoPtr->PtrInputData);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Status = XPki_LoadInputData_EcdsaVerifySign(
					 (XPki_EcdsaVerifyInputData *)Request_InfoPtr->PtrInputData,
					 Addr, QDescPtr, TrigVal);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			break;
		case PKI_ECC_NIST_P192_KEY_PRIV_GEN:
		case PKI_ECC_NIST_P256_KEY_PRIV_GEN:
		case PKI_ECC_NIST_P384_KEY_PRIV_GEN:
		case PKI_ECC_NIST_P521_KEY_PRIV_GEN:
			Status = XPki_GetEccCurveType(Request_InfoPtr->OpsType, &CrvType);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Status = XPki_GenEccPrivateKey(CrvType, Addr, QDescPtr, TrigVal);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			break;
		case PKI_ECC_NIST_P192_KEY_PUB_GEN:
		case PKI_ECC_NIST_P256_KEY_PUB_GEN:
		case PKI_ECC_NIST_P384_KEY_PUB_GEN:
		case PKI_ECC_NIST_P521_KEY_PUB_GEN:
			Status = XPki_GetEccCurveType(Request_InfoPtr->OpsType, &CrvType);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Status = XPki_EcdsaGenPubKey(CrvType, (u8 *)Request_InfoPtr->PtrInputData,
						     Addr, QDescPtr, TrigVal);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			break;
		case PKI_MAX_OPS:
		default:
			xil_printf("invalid Operation\r\n");
			Status = XST_INVALID_PARAM;
			goto END;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function copy the user-pointed data into the Request queue
 *		buffer in the below order(as per the PKI Spec) to generate the
 *		signature.
 *		-Private key
 *		-Ehimeral key
 *		-Data Hash
 *		-Signature Descriptor Data
 *
 * @param	SignParams  Pointer to the XPki_EcdsaSignInputData
 * @param   Addr  Pointer to the Request queue input address
 * @param   DescPtr Pointer to the descriptor buffer
 * @param	EcdsaReqVal Pointer to the Generate Signature Descriptor Data offset
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XPki_LoadInputData_EcdsaGenerateSign(XPki_EcdsaSignInputData *SignParams,
		UINTPTR Addr, u32 *DescPtr, u64 *EcdsaReqVal)
{

	u8 EcdsaCrvlen = XPki_GetEccCurveLen(SignParams->CrvType);
	volatile int Status = XST_FAILURE;
	u32 ReqLen;

	/* Copy Private key */
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       SignParams->Dlen, SignParams->D,
			       SignParams->Dlen, SignParams->Dlen);

	/* Copy Ehimeral key */
	Addr += SignParams->Dlen;
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       SignParams->Klen, SignParams->K,
			       SignParams->Klen, SignParams->Klen);

	/* Copy Data hash */
	Addr += SignParams->Klen;
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       SignParams->Hashlen, SignParams->Hash,
			       SignParams->Hashlen, SignParams->Hashlen);

	Addr += SignParams->Hashlen;
	ReqLen = EcdsaCrvlen - SignParams->Hashlen;
	if (ReqLen != 0) {
		Status = XST_FAILURE;
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)Addr,
				       ReqLen, 0U, ReqLen);
		Addr += ReqLen;
	}

	/* Copy Write Descriptor */
	Status = XST_FAILURE;
	if (SignParams->CrvType == ECC_NIST_P521) {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)Addr,
				       XPKI_SIGN_P521_PADD_BYTES, 0U,
				       XPKI_SIGN_P521_PADD_BYTES);
		Addr += XPKI_SIGN_P521_PADD_BYTES;
		Status = XST_FAILURE;
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, (u8 *)DescPtr,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	} else {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, (u8 *)DescPtr,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	}

	*EcdsaReqVal = XPKI_ECDSA_NEW_REQUEST_MASK & (Addr + 1);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function  copy the user-pointed data into the Request queue
 *		buffer in the below order(as per the PKI Spec) to validate the
 *		signature.
 *		-Public key(Qx, Qy)
 *		-Signature(SignR, SignS)
 *		-Data Hash
 *		-Verify Signature Descriptor Data
 *
 * @param	VerifyParams  Pointer to the XPki_EcdsaVerifyInputData
 * @param   Addr  Pointer to the Request queue input address
 * @param   DescPtr  Pointer to the descriptor buffer
 * @param	EcdsaReqVal Pointer to the Verify Signature Descriptor Data offset
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XPki_LoadInputData_EcdsaVerifySign(XPki_EcdsaVerifyInputData *VerifyParams,
		UINTPTR Addr, u32 *DescPtr, u64 *EcdsaReqVal)
{
	u8 EcdsaCrvlen = XPki_GetEccCurveLen(VerifyParams->CrvType);
	volatile int Status = XST_FAILURE;
	u8 ReqLen;

	/* Copy Public key */
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       EcdsaCrvlen, VerifyParams->PubKey.Qx,
			       EcdsaCrvlen, EcdsaCrvlen);

	Addr += EcdsaCrvlen;
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       EcdsaCrvlen, VerifyParams->PubKey.Qy,
			       EcdsaCrvlen, EcdsaCrvlen);

	/* Copy signature */
	Addr += EcdsaCrvlen;
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       EcdsaCrvlen, VerifyParams->Sign.SignR,
			       EcdsaCrvlen, EcdsaCrvlen);

	Addr += EcdsaCrvlen;
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       EcdsaCrvlen, VerifyParams->Sign.SignS,
			       EcdsaCrvlen, EcdsaCrvlen);

	/* Copy Data hash */
	Addr += EcdsaCrvlen;
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       VerifyParams->Hashlen, VerifyParams->Hash,
			       VerifyParams->Hashlen, VerifyParams->Hashlen);

	Addr += VerifyParams->Hashlen;
	ReqLen = EcdsaCrvlen - VerifyParams->Hashlen;
	if (ReqLen != 0) {
		Status = XST_FAILURE;
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)Addr,
				       ReqLen, 0U, ReqLen);
		Addr += ReqLen;
	}

	/* Copy Write Descriptor */
	Status = XST_FAILURE;
	if (VerifyParams->CrvType == ECC_NIST_P521) {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)Addr,
				       XPKI_VERIFY_P521_PADD_BYTES, 0U,
				       XPKI_VERIFY_P521_PADD_BYTES);
		Addr += XPKI_VERIFY_P521_PADD_BYTES;
		Status = XST_FAILURE;
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, (u8 *)DescPtr,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	} else {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, (u8 *)DescPtr,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	}

	*EcdsaReqVal = XPKI_ECDSA_NEW_REQUEST_MASK & (Addr + 1);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function  copy the user-pointed data into the Request queue
 *		buffer in the below order(as per the PKI Spec) to perform
 *		modulus addition.
 *		- Curve Order
 *		- TRGN Generated Random value(A)
 *		- Value B = 1 -fixed
 *		- Modular addition Descriptor Data
 *
 * @param	ModAddParams  Pointer to the XPki_ModAddInputData
 * @param   Addr  Pointer to the Request queue input address
 * @param   DescPtr  Pointer to the descriptor buffer
 * @param	EcdsaReqVal  Pointer to the modular addition Descriptor Data offset
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XPki_LoadInputData_ModulusAdd(XPki_ModAddInputData *ModAddParams,
		UINTPTR Addr, u32 *DescPtr, u64 *EcdsaReqVal)
{
	volatile int Status = XST_FAILURE;

	/* Copy Order */
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       ModAddParams->Dlen, ModAddParams->Order,
			       ModAddParams->Dlen, ModAddParams->Dlen);

	Addr += ModAddParams->Dlen;
	/* Copy A */
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       ModAddParams->Dlen, ModAddParams->D,
			       ModAddParams->Dlen, ModAddParams->Dlen);

	Addr += ModAddParams->Dlen;
	/* Copy B */
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)Addr, 1U, 1U, 1U);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)Addr + 1U,
			       ModAddParams->Dlen - 1U, 0U, ModAddParams->Dlen - 1U);

	Addr += ModAddParams->Dlen;
	/* Copy Write Descriptor */
	Status = XST_FAILURE;
	if (ModAddParams->CrvType == ECC_NIST_P521) {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)Addr,
				       XPKI_SIGN_P521_PADD_BYTES, 0U, XPKI_SIGN_P521_PADD_BYTES);
		Addr += XPKI_SIGN_P521_PADD_BYTES;
		Status = XST_FAILURE;
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, (u8 *)DescPtr,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	} else {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, (u8 *)DescPtr,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	}

	*EcdsaReqVal = XPKI_ECDSA_NEW_REQUEST_MASK & (Addr + 1);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function  copy the user-pointed data into the Request queue
 *		buffer in the below order(as per the PKI Spec) to generate the
 *		Public-key.
 *		-Private key
 *		-Generator points(Gx, Gy)
 *		-Point Multiplication Descriptor Data
 *
 * @param	PointMultiParams  Pointer to the XPki_EcdsaPointMultiInputData
 * @param   Addr  Pointer to the request queue input address
 * @param   DescPtr  Pointer to the descriptor buffer
 * @param	EcdsaReqVal  Pointer to the Point-Multiplication Descriptor Data offset
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XPki_LoadInputData_EcdsaPointMulti(XPki_EcdsaPointMultiInputData *PointMultiParams,
		UINTPTR Addr, u32 *DescPtr, u64 *EcdsaReqVal)
{
	volatile int Status = XST_FAILURE;

	/* Copy Private key */
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       PointMultiParams->Dlen, PointMultiParams->D,
			       PointMultiParams->Dlen, PointMultiParams->Dlen);

	/* Copy Generator point Gx */
	Addr += PointMultiParams->Dlen;
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       PointMultiParams->Gpoint.Gxlen, PointMultiParams->Gpoint.Gx,
			       PointMultiParams->Gpoint.Gxlen, PointMultiParams->Gpoint.Gxlen);

	/* Copy Generator point Gy */
	Addr += PointMultiParams->Gpoint.Gxlen;
	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
			       PointMultiParams->Gpoint.Gylen, PointMultiParams->Gpoint.Gy,
			       PointMultiParams->Gpoint.Gylen, PointMultiParams->Gpoint.Gylen);

	Addr += PointMultiParams->Gpoint.Gylen;

	/* Copy Write Descriptor */
	Status = XST_FAILURE;
	if (PointMultiParams->CrvType == ECC_NIST_P521) {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)Addr,
				       XPKI_SIGN_P521_PADD_BYTES, 0U, XPKI_SIGN_P521_PADD_BYTES);
		Addr += XPKI_SIGN_P521_PADD_BYTES;
		Status = XST_FAILURE;
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, (u8 *)DescPtr,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	} else {
		XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemCpy, (u8 *)Addr,
				       XPKI_DESC_LEN_BYTES, (u8 *)DescPtr,
				       XPKI_DESC_LEN_BYTES, XPKI_DESC_LEN_BYTES);
	}

	*EcdsaReqVal = XPKI_ECDSA_NEW_REQUEST_MASK & (Addr + 1);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function is used to fill the RQ to generate the
 *		Private key for the request curve.
 *
 * @param       CrvType  Type of ECC curve (NIST-P192, P256, P384, and P521)
 * @param       Addr  Pointer to the Request queue input address
 * @param       DescPtr  Pointer to the descriptor buffer
 * @param       EcdsaReqVal Pointer to the Point-Multiplication Descriptor Data offset
 *
 * @return
 *      -       XST_SUCCESS - On success
 *      -       XST_FAILURE - On failure
 *
 *****************************************************************************/
static int XPki_GenEccPrivateKey(XPki_EcdsaCrvType CrvType, UINTPTR Addr,
				 u32 *DescPtr, u64 *EcdsaReqVal)
{
	volatile int Status = XST_FAILURE;
	XPki_ModAddInputData ModAddParams = {0U};
	const u8 *Order = XPki_GetEccCurveOrder(CrvType);
	u8 Size = XPki_GetEccCurveLen(CrvType);
	u8 Priv[XPKI_MAX_PRIV_KEY_LEN];

	if (Size == 0) {
		Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
		goto END;
	}

	Status = XST_FAILURE;
	Status = XPki_TrngGenerateRandomNum(Size, Priv);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	ModAddParams.CrvType = CrvType;
	ModAddParams.Order = Order;
	ModAddParams.Dlen = Size;
	ModAddParams.D = Priv;

	Status = XPki_LoadInputData_ModulusAdd(&ModAddParams, Addr, DescPtr, EcdsaReqVal);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function is used to fill the RQ to generate the
 *              Public key for the request curve.
 *
 * @param       CrvType Type of ECC curve (NIST-P192, P256, P384, and P521)
 * @param	    PrivKey Pointer to the private-key buffer
 * @param       Addr  Pointer to the Request queue input address
 * @param       DescPtr  Pointer to the descriptor buffer
 * @param       EcdsaReqVa  Pointer to the Point-Multiplication Descriptor Data offset
 *
 * @return
 *		- XST_SUCCESS - On success
 * 		- XST_FAILURE - On failure
 *
 *****************************************************************************/
static int XPki_EcdsaGenPubKey(XPki_EcdsaCrvType CrvType, u8 *PrivKey, UINTPTR Addr,
			       u32 *DescPtr, u64 *EcdsaReqVa)
{
	XPki_EcdsaGpoint *Gpoint =  XPki_GetEccGpoint(CrvType);
	XPki_EcdsaPointMultiInputData PointMulti_Inputs = {0};
	u8 Size = XPki_GetEccCurveLen(CrvType);
	volatile int Status = XST_FAILURE;

	if ((Gpoint->Gx == NULL) || (Gpoint->Gy == NULL)) {
		Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
		goto END;
	}

	PointMulti_Inputs.CrvType = CrvType;
	PointMulti_Inputs.Gpoint.Gx = Gpoint->Gx;
	PointMulti_Inputs.Gpoint.Gy = Gpoint->Gy;
	PointMulti_Inputs.Gpoint.Gxlen = Gpoint->Gxlen;
	PointMulti_Inputs.Gpoint.Gylen = Gpoint->Gylen;
	PointMulti_Inputs.D = PrivKey;
	PointMulti_Inputs.Dlen = Size;

	Status = XST_FAILURE;
	Status = XPki_LoadInputData_EcdsaPointMulti(&PointMulti_Inputs, Addr, DescPtr, EcdsaReqVa);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used the get the ECC curve Order
 *
 * @param	CrvType Type of ECC curve (NIST-P192, P256, P384, and P521)
 *
 * @return
 *		ECC Curve Order.
 *
 *****************************************************************************/
static const u8 *XPki_GetEccCurveOrder(XPki_EcdsaCrvType CrvType)
{
	static const u8 *Order;

	if (CrvType == ECC_NIST_P192) {
		Order = Order_P192;
	} else if (CrvType == ECC_NIST_P256) {
		Order = Order_P256;
	} else if (CrvType == ECC_NIST_P384) {
		Order = Order_P384;
	} else if (CrvType == ECC_NIST_P521) {
		Order = Order_P521;
	} else {
		Order = NULL;
	}

	return Order;
}

/*****************************************************************************/
/**
 * @brief       This function is used the get the ECC curve type for the
 *		given operation.
 *
 * @param	OpsType Crypto Operation type
 * @param   CrvType Type of ECC curve (NIST-P192, P256, P384, and P521)
 *
 * @return
 *      -       XST_SUCCESS - On success
 *      -       XST_FAILURE - On failure
 *
 *****************************************************************************/
static int XPki_GetEccCurveType(Xpki_OpsType OpsType, XPki_EcdsaCrvType *CrvType)
{
	volatile int Status = XST_FAILURE;

	switch (OpsType) {
		case PKI_ECC_NIST_P192_SIGN:
		case PKI_ECC_NIST_P192_SIGN_VERIFY:
		case PKI_ECC_NIST_P192_KEY_PRIV_GEN:
		case PKI_ECC_NIST_P192_KEY_PUB_GEN:
			*CrvType = ECC_NIST_P192;
			Status = XST_SUCCESS;
			break;
		case PKI_ECC_NIST_P256_SIGN:
		case PKI_ECC_NIST_P256_SIGN_VERIFY:
		case PKI_ECC_NIST_P256_KEY_PRIV_GEN:
		case PKI_ECC_NIST_P256_KEY_PUB_GEN:
			*CrvType = ECC_NIST_P256;
			Status = XST_SUCCESS;
			break;
		case PKI_ECC_NIST_P384_SIGN:
		case PKI_ECC_NIST_P384_SIGN_VERIFY:
		case PKI_ECC_NIST_P384_KEY_PRIV_GEN:
		case PKI_ECC_NIST_P384_KEY_PUB_GEN:
			*CrvType = ECC_NIST_P384;
			Status = XST_SUCCESS;
			break;
		case PKI_ECC_NIST_P521_SIGN:
		case PKI_ECC_NIST_P521_SIGN_VERIFY:
		case PKI_ECC_NIST_P521_KEY_PRIV_GEN:
		case PKI_ECC_NIST_P521_KEY_PUB_GEN:
			*CrvType = ECC_NIST_P521;
			Status = XST_SUCCESS;
			break;
		case PKI_MAX_OPS:
		default:
			xil_printf("invalid Operation\r\n");
			Status = XST_INVALID_PARAM;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to get the ECC curve generator points.
 *
 * @param	CrvType  Type of ECC curve (NIST-P192, P256, P384, and P521)
 *
 * @return
 *		ECC Generator Points(Gx, Gy)
 *
 *****************************************************************************/
static XPki_EcdsaGpoint *XPki_GetEccGpoint(XPki_EcdsaCrvType CrvType)
{
	static XPki_EcdsaGpoint Gpoint = {0U};

	if (CrvType == ECC_NIST_P192) {
		Gpoint.Gx = EcdsaGpoint_P192_Gx;
		Gpoint.Gy = EcdsaGpoint_P192_Gy;
		Gpoint.Gxlen = NIST_P192_LEN_BYTES;
		Gpoint.Gylen = NIST_P192_LEN_BYTES;
	} else if (CrvType == ECC_NIST_P256) {
		Gpoint.Gx = EcdsaGpoint_P256_Gx;
		Gpoint.Gy = EcdsaGpoint_P256_Gy;
		Gpoint.Gxlen = NIST_P256_LEN_BYTES;
		Gpoint.Gylen = NIST_P256_LEN_BYTES;
	} else if (CrvType == ECC_NIST_P384) {
		Gpoint.Gx = EcdsaGpoint_P384_Gx;
		Gpoint.Gy = EcdsaGpoint_P384_Gy;
		Gpoint.Gxlen = NIST_P384_LEN_BYTES;
		Gpoint.Gylen = NIST_P384_LEN_BYTES;
	} else if (CrvType == ECC_NIST_P521) {
		Gpoint.Gx = EcdsaGpoint_P521_Gx;
		Gpoint.Gy = EcdsaGpoint_P521_Gy;
		Gpoint.Gxlen = NIST_P521_LEN_BYTES;
		Gpoint.Gylen = NIST_P521_LEN_BYTES;
	} else {
		Gpoint.Gx = NULL;
		Gpoint.Gy = NULL;
	}

	return &Gpoint;
}

/*****************************************************************************/
/**
 * @brief	This function is used to validate the elliptic curve signature
 *		generation input parameters.
 *
 * @param	SignParams  Pointer to the XPki_EcdsaSignInputData
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XPKI_ECDSA_INVALID_PARAM - On invalid argument
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XPki_ValidateEcdsaGenerateSignParam(XPki_EcdsaSignInputData *SignParams)
{
	u8 EcdsaCrvlen = XPki_GetEccCurveLen(SignParams->CrvType);
	volatile int Status = XST_FAILURE;

	if ((SignParams->D == NULL) || (SignParams->K == NULL) ||
	    (SignParams->Hash == NULL)) {
		Status = XPKI_ECDSA_INVALID_PARAM;
		goto END;
	}

	if ((SignParams->Dlen != EcdsaCrvlen) ||
	    (SignParams->Klen != EcdsaCrvlen)) {
		Status = XPKI_ECDSA_INVALID_PARAM;
		goto END;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to validate the elliptic curve signature
 *		verification input parameters.
 *
 * @param	VerifyParams Pointer to the XPki_EcdsaVerifyInputData
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XPKI_ECDSA_NON_SUPPORTED_CRV - For Un-supported elliptic curves
 *	-	XPKI_ECDSA_INVALID_PARAM - On invalid argument
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XPki_ValidateEcdsaVerifySignParam(XPki_EcdsaVerifyInputData *VerifyParams)
{
	volatile int Status = XST_FAILURE;

	if ((VerifyParams->CrvType != ECC_NIST_P192) &&
	    (VerifyParams->CrvType != ECC_NIST_P256) &&
	    (VerifyParams->CrvType != ECC_NIST_P384) &&
	    (VerifyParams->CrvType != ECC_NIST_P521)) {
		Status = XPKI_ECDSA_NON_SUPPORTED_CRV;
		goto END;
	}

	if ((VerifyParams->PubKey.Qx == NULL) ||
	    (VerifyParams->PubKey.Qy == NULL) ||
	    (VerifyParams->Sign.SignR == NULL) ||
	    (VerifyParams->Sign.SignS == NULL) ||
	    (VerifyParams->Hash == NULL)) {
		Status = XPKI_ECDSA_INVALID_PARAM;
		goto END;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used the get the ECC curve length.
 *
 * @param	CrvType  Type of ECC curve (NIST-P192, 256, P384, and P521)
 *
 * @return
 *		ECC Curve lenth.
 *
 *****************************************************************************/
static u8 XPki_GetEccCurveLen(XPki_EcdsaCrvType CrvType)
{
	static u8 DLen;

	if (CrvType == ECC_NIST_P192) {
		DLen = NIST_P192_LEN_BYTES;
	} else if (CrvType == ECC_NIST_P256) {
		DLen = NIST_P256_LEN_BYTES;
	} else if (CrvType == ECC_NIST_P384) {
		DLen = NIST_P384_LEN_BYTES;
	} else if (CrvType == ECC_NIST_P521) {
		DLen = NIST_P521_LEN_BYTES;
	} else {
		DLen = 0;
	}

	return DLen;
}

/*****************************************************************************/
/**
 * @brief       This function is used to update the Queue info for the EnQueued
 *		requests.
 *
 * @param       InstancePtr     Pointer to the XPki instance
 * @param       Request_InfoPtr  Pointer to the queue info structure
 * @param       RequestID       Request ID for the EnQueued request API
 *
 * @return
 *      -       XST_SUCCESS                     - On success
 *      -       XPKI_UNSUPPORTED_OPS            - If the requested operation is
 *                                                not supported
 *      -       XST_FAILURE                     - On failure
******************************************************************************/
static inline void XPki_UpdateQueueInfo(XPki_Instance *InstancePtr,
					XPki_Request_Info *Request_InfoPtr,
					u32 RequestID)
{
	u32 QueueID = GET_QUEUE_ID(RequestID);
	u32 SlotID = GET_QUEUE_SLOT_ID(RequestID);

	InstancePtr->RQCount++;
	InstancePtr->MultiQinfo[QueueID].QFreeSlots--;
	InstancePtr->MultiQinfo[QueueID].RQ_SubmitList[SlotID] = RequestID;
	InstancePtr->MultiQinfo[QueueID].XPki_IntrCallBack[SlotID] = Request_InfoPtr->XPki_CompletionCallBack;
}

/*****************************************************************************/
/**
 * @brief       This function is used to free the Queue info for the DeQueued
 *              requests.
 *
 * @param       InstancePtr     Pointer to the XPki instance
 * @param       RequestID       Request ID for the DeQueued request API
 *
 * @return
 *      -       XST_SUCCESS                     - On success
 *      -       XST_FAILURE                     - On failure
 *
******************************************************************************/
static int XPki_FreeQueueInfo(XPki_Instance *InstancePtr, u32 RequestID)
{
	volatile int Status = XST_FAILURE;
	u32 QueueID = GET_QUEUE_ID(RequestID);
	u32 SlotID = GET_QUEUE_SLOT_ID(RequestID);
	u32 QSlotSize = InstancePtr->MultiQinfo[QueueID].QSlotSize;
	u32 QSlotOffset = QSlotSize * GET_QUEUE_SLOT_ID(RequestID);
	UINTPTR InputAddr = InstancePtr->MultiQinfo[QueueID].RQInputAddr + QSlotOffset;
	UINTPTR OutputAddr = InstancePtr->MultiQinfo[QueueID].RQOutputAddr + QSlotOffset;


	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)InputAddr,
			       QSlotSize, 0U, QSlotSize);

	Status = XST_FAILURE;
	XSECURE_TEMPORAL_CHECK(END, Status, Xil_SMemSet, (u8 *)OutputAddr,
			       QSlotSize, 0U, QSlotSize);

	InstancePtr->RQCount--;
	InstancePtr->MultiQinfo[QueueID].QFreeSlots++;
	InstancePtr->MultiQinfo[QueueID].RQ_SubmitList[SlotID] = 0U;
	InstancePtr->MultiQinfo[QueueID].XPki_IntrCallBack[SlotID] = NULL;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function is used to prepare the descriptor data for the
 *		requested crypto opration.
 *
 * @param       InstancePtr  Pointer to the XPki instance
 * @param       Ops  Type of the crypto operation
 * @param       RequestID  Unique ID
 * @param       QDescPtr  Pointer to the descriptor buffer
 *
 ******************************************************************************/

static void XPki_PrepDescriptor(XPki_Instance *InstancePtr, Xpki_OpsType Ops,
				u32 RequestID, u32 *QDescPtr)
{
	u32 QSlotOffset, QSlotSize, Index = 0U;
	u32 QueueID = GET_QUEUE_ID(RequestID);

	QSlotSize = InstancePtr->MultiQinfo[QueueID].QSlotSize;
	QSlotOffset = QSlotSize * GET_QUEUE_SLOT_ID(RequestID);
	QDescPtr[Index++] = DESC_TAG_START;
	QDescPtr[Index++] = OpsInfo[Ops].OpsCmd;
	QDescPtr[Index++] = DESC_TAG_TFRI(OpsInfo[Ops].InPutSize);
	QDescPtr[Index++] = QSlotOffset;
	QDescPtr[Index++] = DESC_TAG_TFR0(OpsInfo[Ops].OutPutSize);
	QDescPtr[Index++] = QSlotOffset | 0x10000;
	QDescPtr[Index++] = DESC_TAG_NTFY(RequestID);
	QDescPtr[Index] = DESC_TAG_END;
}

/*****************************************************************************/
/**
 * @brief       This function is used to get the free slot ID from the request
 *		QueueID.
 *
 * @param       InstancePtr Pointer to the XPki instance
 * @param       QueueID		QueueID(0/1/2/3)
 * @param       FreeIndex	Pointer to the free slot index.
 *
 * @return
 *      -       XST_SUCCESS                     - On success
 *      -       XST_FAILURE                     - On failure
 *
******************************************************************************/
static inline int XPki_GetFreeSlot(XPki_Instance *InstancePtr,
				   XPki_QueueID QueueID, u32 *FreeIndex)
{
	volatile int Status = XST_FAILURE;
	u32 Index = 0U;

	while (Index < InstancePtr->MultiQinfo[QueueID].QMaxSlots) {
		if (InstancePtr->MultiQinfo[QueueID].RQ_SubmitList[Index] == 0U) {
			*FreeIndex = Index;
			Status = XST_SUCCESS;
			break;
		}
		Index++;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function is used to get the Queue ID.
 *
 * @param   InstancePtr   Pointer to the XPki instance
 * @param	OpSize Crypto Opration size.
 * @param    QueueID  Pointer to the QueueID(0/1/2/3)
 *
 * @return
 *      -       XST_SUCCESS                     - On success
 *      -       XST_FAILURE                     - On failure
 *
******************************************************************************/
static inline int XPki_GetQueueID(XPki_Instance *InstancePtr,
				  u32 OpSize, XPki_QueueID *QueueID)
{
	volatile int Status = XST_FAILURE;
	u32 Index = 0U;

	while (Index <= PKI_QUEUE_ID_3) {
		if ((OpSize <= InstancePtr->MultiQinfo[Index].QSlotSize) &&
		    (InstancePtr->MultiQinfo[Index].QFreeSlots != 0U)) {
			*QueueID = Index;
			Status = XST_SUCCESS;
			break;
		}
		Index++;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function is used to validate the given request ID against
		with submitted request IDs.
 *
 * @param       InstancePtr     Pointer to the XPki instance
 * @param       RequestID	Unique ID provided by user
 *
 * @return
 *      -       XST_SUCCESS                     - On success
 *      -       XST_FAILURE                     - On failure
 *
******************************************************************************/

static int XPki_ValidateReqID(XPki_Instance *InstancePtr, u32 RequestID)
{
	volatile int Status = XPKI_INVALID_REQ_ID;
	u32 Index = 0U;
	u32 QueueID;

	if ((RequestID >> 28U & 0xF) == 0U) {
		goto END;
	}

	QueueID = GET_QUEUE_ID(RequestID);
	if (QueueID > PKI_QUEUE_ID_3) {
		goto END;
	}

	while (Index < InstancePtr->MultiQinfo[QueueID].QMaxSlots) {
		if (InstancePtr->MultiQinfo[QueueID].RQ_SubmitList[Index] == RequestID) {
			Status = XST_SUCCESS;
			break;
		}
		Index++;
	}

END:
	return Status;
}

/******************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the PKI.
*
* @param        InstancePtr Pointer to the instance of the PKI
*               which is going to be connected to the interrupt controller.
*
* @return       XST_SUCCESS if successful, otherwise XST_FAILURE.
*
*
*******************************************************************************/
static int XPki_SetupInterruptSystem(XPki_Instance *InstancePtr)
{
	int Status;
	u32 Int_Id = XPKI_INTERRUPT_ID;
	XScuGic_Config *IntcConfig; /* Instance of the interrupt controller */

	Xil_ExceptionInit();

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				     &InterruptController);

	/*
	 * Connect the device handler that will be called when an interrupt
	 * for the device occurs, the handler defined above performs the
	 * specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(&InterruptController, Int_Id,
				 (Xil_InterruptHandler)XPki_IntrHandler,
				 (void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for PKI device.
	 */
	XScuGic_Enable(&InterruptController, Int_Id);


	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function implements the interrupt handler
*
* @param       InstancePtr     Pointer to the XPki instance
*
*******************************************************************************/
static void XPki_IntrHandler(XPki_Instance *InstancePtr)
{
	u64 RegVal = Xil_In64(FPD_PKI_IRQ_STATUS);

	if ((RegVal & 0x1) != 0U) {
		XPki_IntrCallbackHandler(InstancePtr, PKI_QUEUE_ID_0);
	}

	if ((RegVal & 0x2) != 0U) {
		XPki_IntrCallbackHandler(InstancePtr, PKI_QUEUE_ID_1);
	}

	if ((RegVal & 0x4) != 0U) {
		XPki_IntrCallbackHandler(InstancePtr, PKI_QUEUE_ID_2);
	}

	if ((RegVal & 0x8) != 0U) {
		XPki_IntrCallbackHandler(InstancePtr, PKI_QUEUE_ID_3);
	}
}

/******************************************************************************/
/**
*
* This function is use to notify the user if the submited request completed.
*
* @param	InstancePtr	Pointer to the XPki instance
* @param	Id Queue ID(0/1/2/3)
*
*******************************************************************************/
static void XPki_IntrCallbackHandler(XPki_Instance *InstancePtr, XPki_QueueID Id)
{
	UINTPTR CQAddr =  InstancePtr->MultiQinfo[Id].CQAddr;
	u32 RegOffset = Id * XPKI_INDEX_2_VAL;
	u32 RequestID, Status;
	u32 SlotID, i, j;

	/* Check for pending requests */
	if (Xil_In32(FPD_PKI_CTL_PENDING_REQS + RegOffset) == 0U) {
		/* Reset IRQ signal */
		Xil_Out64(FPD_PKI_IRQ_RESET, (1 << Id));
	}

	for (j = 0, i = 0; j < XPKI_MAX_CQ_REQ; j++, i += 8) {
		RequestID = Xil_In32(CQAddr + i + 4) & XPKI_RID_MASK;
		if (RequestID != 0U) {
			SlotID = GET_QUEUE_SLOT_ID(RequestID);
			Status = Xil_In32(CQAddr + i);
			if (InstancePtr->MultiQinfo[Id].XPki_IntrCallBack[SlotID] != NULL) {
				InstancePtr->MultiQinfo[Id].XPki_IntrCallBack[SlotID](RequestID, Status);
			}
			Xil_Out32(CQAddr + i, 0U);
			Xil_Out32(CQAddr + i + 4, 0U);
		}
	}
}
