/******************************************************************************
* Copyright (C) 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xilpki.h
 * @addtogroup xilpki-api XilPKI Library APIs
 * The XilPKI is a standalone library designed to run in bare-metal for the Keystone B device. This
 * library provides the implementation of the interface functions for the PKI hardware module.
 * Though the hardware supports many other security related operations, currently, this library
 * provides interface for the following operations.
 *
 * This library supports the following features:
 *	- ECDSA signature generation support - for P256, P384, and P521 curves
 *	- ECDSA signature verification support- for P256, P384, and P521 curves
 *	- ECC Point multiplication support - for P256, P384, and P521 curves
 *	- ECC Key pair generation support - for  P256, P384, and P521 curves
 *	- ECC P256 sign generation and verification KAT
 *	- ECC P384 sign generation and verification KAT
 *	- ECC P521 sign generation and verification KAT
 *	- ECC P256 pairwise consistency known answer test
 *	- ECC P384 pairwise consistency known answer test
 *	- ECC P521 pairwise consistency known answer test
 *
 * @{
 * @details
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date      Changes
 * ----- ----  --------  -------------------------------------------------------
 * 1.0   Nava  12/05/22  Initial Release
 * 2.0   Nava  06/21/23  Added PKI multi-queue support for ECC operations.
 * 2.0   Nava  08/02/23  Added a new API XPki_GetVersion() to access the library
 *			 version info.
 * 2.0   Nava  09/06/23  Updated the XPki_GetVersion() API prototype to inline with
 *                       other secure library version info API's.
 * 2.0   Nava  09/07/23  Fixed issues with IRQ signal.
 * 2.0   Nava  09/11/23  Fixed doxygen warnings.
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef XILPKI_H
#define XILPKI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#include "sleep.h"
#include "xil_assert.h"
#include "xil_cache.h"
#include "xil_io.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xil_util.h"
#include "xparameters.h"
#include "xilpki_ecdsa.h"
#include "xtrngpsx.h"
#include "xil_mmu.h"
/***************** Macros (Inline Functions) Definitions ********************/

/************************** Constant Definitions *****************************/
/*
@cond internal
*/

/**
 * FPD_SLCR Base Address
 */
#define FPD_SLCR_BASEADDR		0xEC8C0000U

/**
 * Register: FPD_SLCR_WPROT0
 */
#define FPD_SLCR_WPROT0			( ( FPD_SLCR_BASEADDR ) + 0x00000000U )
#define FPD_SLCR_WPROT0_FULLMASK	0x00000001U
#define FPD_SLCR_WPROT0_FULLRWMASK	0x00000001U
#define FPD_SLCR_WPROT0_DEFVAL		0x1U

/**
 * Access_type: rw
 */
#define FPD_SLCR_WPROT0_ACTIVE_SHIFT	0U
#define FPD_SLCR_WPROT0_ACTIVE_WIDTH	1U
#define FPD_SLCR_WPROT0_ACTIVE_MASK	0x00000001U
#define FPD_SLCR_WPROT0_ACTIVE_DEFVAL	0x1U

/**
 * Register: FPD_SLCR_PKI_MUX_SEL
 */
#define FPD_SLCR_PKI_MUX_SEL		(( FPD_SLCR_BASEADDR ) + 0x00002000U )
#define FPD_SLCR_PKI_MUX_SEL_FULLMASK	0x00000001U
#define FPD_SLCR_PKI_MUX_SEL_FULLRWMASK	0x00000001U
#define FPD_SLCR_PKI_MUX_SEL_DEFVAL	0x0U

/**
 * Access_type: rw
 */
#define FPD_SLCR_PKI_MUX_SEL_PKI_SMUX_PATHSEL_SHIFT	0U
#define FPD_SLCR_PKI_MUX_SEL_PKI_SMUX_PATHSEL_WIDTH	1U
#define FPD_SLCR_PKI_MUX_SEL_PKI_SMUX_PATHSEL_MASK	0x00000001U
#define FPD_SLCR_PKI_MUX_SEL_PKI_SMUX_PATHSEL_DEFVAL	0x0U


/** PKI reset reg */
#define PSX_CRF_RST_PKI			(0xEC200340U)

#define FPD_PKI_CRYPTO_BASEADDR		(0x20400000000U)
#define FPD_PKI_CTRLSTAT_BASEADDR	(0x20400050000U)

/**
 * Register: PKI_ENGINE_CTRL
 */
#define FPD_PKI_ENGINE_CTRL		( ( FPD_PKI_CTRLSTAT_BASEADDR ) + 0x00000C00U )
#define FPD_PKI_ENGINE_CM_MASK		0x1U
#define FPD_PKI_ENGINE_CM_ENABLE_VAL	0x0U
#define FPD_PKI_ENGINE_CM_DISABLE_VAL	0x1U


/** PKI Soft Reset */
#define FPD_PKI_SOFT_RESET			( ( FPD_PKI_CRYPTO_BASEADDR ) + 0x00000038U )

/** PKI: IRQ_ENABLE */
#define FPD_PKI_IRQ_STATUS			( ( FPD_PKI_CRYPTO_BASEADDR ) + 0x00000088U )

/** PKI: IRQ_ENABLE */
#define FPD_PKI_IRQ_ENABLE			( ( FPD_PKI_CRYPTO_BASEADDR ) + 0x00000090U )

/** PKI: IRQ_RESET */
#define FPD_PKI_IRQ_RESET			( ( FPD_PKI_CRYPTO_BASEADDR ) + 0x000000A0U )

/** PKI RQ_CFG_PAGE_ADDR */
#define FPD_PKI_RQ_CFG_PAGE_ADDR_INPUT		( ( FPD_PKI_CRYPTO_BASEADDR ) + 0x00000100U )
#define FPD_PKI_RQ_CFG_PAGE_ADDR_OUTPUT		( ( FPD_PKI_CRYPTO_BASEADDR ) + 0x00000108U )

/** PKI: RQ_CFG_PAGE_SIZE */
#define FPD_PKI_RQ_CFG_PAGE_SIZE		( ( FPD_PKI_CRYPTO_BASEADDR ) + 0x00000120U )

/** PKI: RQ_CFG_CQID_00: Index of the completion queue associated to this request queue */
#define FPD_PKI_RQ_CFG_CQID			( ( FPD_PKI_CRYPTO_BASEADDR ) + 0x00000128U )

/** Type of allowed descriptors */
#define FPD_PKI_RQ_CFG_PERMISSIONS		( ( FPD_PKI_CRYPTO_BASEADDR ) + 0x00000130U )

/** PKI: RQ_CFG_QUEUE_DEPTH_00 */
#define FPD_PKI_RQ_CFG_QUEUE_DEPTH		( ( FPD_PKI_CRYPTO_BASEADDR ) + 0x00000140U )


/** PKI: CQ_CFG_ADDR_00: Absolute address of the completion queue */
#define FPD_PKI_CQ_CFG_ADDR			( ( FPD_PKI_CRYPTO_BASEADDR ) + 0x00001100U )

/** PKI: CQ_CFG_SIZE_00 */
#define FPD_PKI_CQ_CFG_SIZE			( ( FPD_PKI_CRYPTO_BASEADDR ) + 0x00001108U )

/** PKI: CQ_CFG_IRQ_IDX_00 */
#define FPD_PKI_CQ_CFG_IRQ_IDX			( ( FPD_PKI_CRYPTO_BASEADDR ) + 0x00001110U )

/** PKI: RQ_CTL_NEW_REQUEST_00 */
#define FPD_PKI_RQ_CTL_NEW_REQUEST		( ( FPD_PKI_CRYPTO_BASEADDR ) + 0x00002000U )

/** PKI: RQ_CTL_PENDING_REQS_00 */
#define FPD_PKI_CTL_PENDING_REQS		( ( FPD_PKI_CRYPTO_BASEADDR ) + 0x00002018U )

/** PKI: CQ_CTL_TRIGPOS_00 */
#define FPD_PKI_CQ_CTL_TRIGPOS			( ( FPD_PKI_CRYPTO_BASEADDR ) + 0x00002028U )

#define XPKI_REQ_MAX_COUNT		256U

/** @endcond*/

/** Error codes */
#define XPKI_INVALID_PARAM		0x2U /**< Invalid parameters */
#define XPKI_ERROR_UNALIGN_ADDR		0x3U /**< For unaligned address */
#define XPKI_INVALID_QUEUE_ID		0x4U /**< Invalid Queue ID */
#define XPKI_UNSUPPORTED_OPS		0x5U /**< Unsupported operation */
#define XPKI_QUEUE_FULL			0x6U /**< Queue full */
#define XPKI_SLOT_SIZE_ERR		0x7U /**< For incompatiable slot size */
#define XPKI_INVALID_REQ_ID		0x8U /**< Invalid request ID */

/*
@cond internal
*/
/** Queue Slot Size */
#define PKI_QUEUE_0_SLOT_SIZE_BYTES	128U
#define PKI_QUEUE_1_SLOT_SIZE_BYTES	256U
#define PKI_QUEUE_2_SLOT_SIZE_BYTES	512U
#define PKI_QUEUE_3_SLOT_SIZE_BYTES	512U

/** Library version info */
#define XPKI_MAJOR_VERSION		2U
#define XPKI_MINOR_VERSION		0U

/**************************** Type Definitions *******************************/

typedef enum {
	PKI_QUEUE_ID_0 = 0,
	PKI_QUEUE_ID_1,
	PKI_QUEUE_ID_2,
	PKI_QUEUE_ID_3
} XPki_QueueID;
/** @endcond*/

/** PKI ECC supported operation */
typedef enum {
	PKI_ECC_NIST_P192_SIGN = 1,	/**< ECC NIST P-192 signature generation */
	PKI_ECC_NIST_P256_SIGN,		/**< ECC NIST P-256 signature generation */
	PKI_ECC_NIST_P384_SIGN,		/**< ECC NIST P-384 signature generation */
	PKI_ECC_NIST_P521_SIGN,		/**< ECC NIST P-521 signature generation */
	PKI_ECC_NIST_P192_SIGN_VERIFY,	/**< ECC NIST P-192 signature verification */
	PKI_ECC_NIST_P256_SIGN_VERIFY,	/**< ECC NIST P-256 signature verification */
	PKI_ECC_NIST_P384_SIGN_VERIFY,	/**< ECC NIST P-384 signature verification */
	PKI_ECC_NIST_P521_SIGN_VERIFY,	/**< ECC NIST P-521 signature verification */
	PKI_ECC_NIST_P192_KEY_PRIV_GEN,	/**< ECC NIST P-192 private-key generation */
	PKI_ECC_NIST_P256_KEY_PRIV_GEN,	/**< ECC NIST P-256 private-key generation */
	PKI_ECC_NIST_P384_KEY_PRIV_GEN,	/**< ECC NIST P-384 private-key generation */
	PKI_ECC_NIST_P521_KEY_PRIV_GEN,	/**< ECC NIST P-521 private-key generation */
	PKI_ECC_NIST_P192_KEY_PUB_GEN,	/**< ECC NIST P-192 public-key generation */
	PKI_ECC_NIST_P256_KEY_PUB_GEN,	/**< ECC NIST P-256 public-key generation */
	PKI_ECC_NIST_P384_KEY_PUB_GEN,	/**< ECC NIST P-384 public-key generation */
	PKI_ECC_NIST_P521_KEY_PUB_GEN,	/**< ECC NIST P-521 public-key generation */
	PKI_MAX_OPS			/**< Maximum supported ops count */
} Xpki_OpsType;

/** PKI Request info struct */
typedef struct {
	void *PtrInputData;	/**< Pointer to the input data */
	void *PtrOutputData;	/**< Pointer to the outout data */
	Xpki_OpsType OpsType;	/**< ECC operation type */
	void (*XPki_CompletionCallBack)(u32 RequestID, u32 Status); /**< Request completion callback function pointer */
} XPki_Request_Info;

/*
@cond internal
*/
typedef struct {
	UINTPTR RQInputAddr;
	UINTPTR RQOutputAddr;
	UINTPTR CQAddr;
	u32 QSlotSize;
	u32 QMaxSlots;
	u32 QFreeSlots;
	u32 RQ_SubmitList[XPKI_REQ_MAX_COUNT];
	void (*XPki_IntrCallBack[XPKI_REQ_MAX_COUNT])(u32 RequestID, u32 Status);
} XPki_MultiQueueData;

typedef struct {
	UINTPTR RQInputAddr; /**< Request Queue Input Address */
	UINTPTR RQOutputAddr;  /**< Request Queue Output Address */
	UINTPTR CQAddr;  /**< Completion Queue Address */
	u8 Is_Cm_Enabled; /**< To enable or disable the counter measures */
	XPki_MultiQueueData MultiQinfo[4];
	u32 RQCount;
} XPki_Instance;
/** @endcond*/

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the version number of xilpki library.
 *
 * @return	32-bit version number
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
u32 XPki_GetLibVersion (void)
{
	return (XIL_BUILD_VERSION(XPKI_MAJOR_VERSION, XPKI_MINOR_VERSION));
}

/************************** Function Prototypes ******************************/

/** @addtogroup xilpki-api XilPKI Library APIs
 @{ */

void XPki_Reset(void);
void XPki_SoftReset(void);
int XPki_Initialize(XPki_Instance *InstancePtr);
int XPki_EcdsaGenerateKeyPair(XPki_Instance *InstancePtr, XPki_EcdsaCrvType CrvType,
			      XPki_EcdsaKey *PubKey, u8 *PrivKey);
int XPki_EcdsaVerifySignKat(XPki_Instance *InstancePtr, XPki_EcdsaCrvInfo *CrvInfo);
int XPki_EcdsaSignGenerateKat(XPki_Instance *InstancePtr, XPki_EcdsaCrvInfo *CrvInfo);
int XPki_EcdsaPwct(XPki_Instance *InstancePtr, XPki_EcdsaCrvInfo *CrvInfo,
		   XPki_EcdsaKey *PubKey, u8 *PrivKey);
int XPki_TrngGenerateRandomNum(u8 GenSize, u8 *RandBuf);
int XilPki_EnQueue(XPki_Instance *InstancePtr, XPki_Request_Info *Request_InfoPtr,
		   u32 *RequestID);
int XilPki_DeQueue(XPki_Instance *InstancePtr, XPki_Request_Info *Request_InfoPtr,
		   u32 RequestID);
void XPki_Close(void);

#ifdef __cplusplus
}
#endif

#endif  /* XILPKI_H */

/**@}*/
