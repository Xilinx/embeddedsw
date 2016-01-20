/******************************************************************************
*
* Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xhdcp22_tx_i.h
*
* This file contains data which is shared between files and internal to the
* XIntc component. It is intended for internal use only.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* X.XX  XX     MM/DD/YY ...
* 1.00  JO     06/24/15 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XHDCP22_TX_I_H
/** prevent circular inclusions by using protection macros. */
#define XHDCP22_TX_I_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xhdcp22_tx.h"

/************************** Constant Definitions *****************************/

/** Max number of pairing info structures containing non-confidential keys
* that can be stored for fast authentication with stored Km.
*/
#define XHDCP22_TX_MAX_STORED_PAIRINGINFO       32
/** Maximum allowed re-checking locality, prescribed by LLC. */
#define XHDCP22_TX_MAX_ALLOWED_LOCALITY_CHECKS  1024

/* Message Ids. */
#define XHDCP22_TX_MSG_UNDEFINED                0   /**< Undefined. */
#define XHDCP22_TX_AKE_INIT                     2   /**< AKE Init message. */
#define XHDCP22_TX_AKE_INIT_SIZE                12  /**< AKE Init message size. */
#define XHDCP22_TX_AKE_SEND_CERT                3   /**< AKE Send Certificate message. */
#define XHDCP22_TX_AKE_SEND_CERT_SIZE           534 /**< AKE Send Certificate message size. */
#define XHDCP22_TX_AKE_NO_STORED_KM             4   /**< AKE No Stored Km message. */
#define XHDCP22_TX_AKE_NO_STORED_KM_SIZE        129 /**< AKE No Stored Km message size. */
#define XHDCP22_TX_AKE_STORED_KM                5   /**< AKE Stored Km message size. */
#define XHDCP22_TX_AKE_STORED_KM_SIZE           33  /**< AKE Stored Km message size. */
#define XHDCP22_TX_AKE_SEND_H_PRIME             7   /**< AKE H' message. */
#define XHDCP22_TX_AKE_SEND_H_PRIME_SIZE        33  /**< AKE H' message size. */
#define XHDCP22_TX_AKE_SEND_PAIRING_INFO        8   /**< AKE Pairing info message.*/
#define XHDCP22_TX_AKE_SEND_PAIRING_INFO_SIZE   17  /**< AKE Pairing info message size. */
#define XHDCP22_TX_LC_INIT                      9   /**< LC Init message. */
#define XHDCP22_TX_LC_INIT_SIZE                 9   /**< LC Init message size. */
#define XHDCP22_TX_LC_SEND_L_PRIME              10  /**< Send L' message. */
#define XHDCP22_TX_LC_SEND_L_PRIME_SIZE         33  /**< Send L' message size. */
#define XHDCP22_TX_SKE_SEND_EKS                 11  /**< Send Eks message. */
#define XHDCP22_TX_SKE_SEND_EKS_SIZE            25  /**< Send Eks message size.*/
#define XHDCP22_TX_LC128_SIZE                   16  /**< Lc128 global constant size */

/** Reason why the timer was started: Undefined. */
#define XHDCP22_TX_TS_UNDEFINED                 XHDCP22_TX_MSG_UNDEFINED

/** Reason why the timer was started:
* Mandatory wait of 200 ms before the cipher may be activated.
* Authenticated flag is only set after this period has expired.
* @note: The message ids also double as a reason identifier.
*        Thus, the value of this define should NOT overlap a message Id.*/
#define XHDCP22_TX_TS_WAIT_FOR_CIPHER           0xFE
/** Reason why the timer was started: Status checking.
* @note: The message ids also double as a reason identifier.
*        Thus, the value of this define should NOT overlap a message Id.*/
#define XHDCP22_TX_TS_RX_REAUTH_CHECK           0xFF

/** Internal used timer counter for timeout checking. */
#define XHDCP22_TX_TIMER_CNTR_0                 0
/** Internal used timer counter for logging. */
#define XHDCP22_TX_TIMER_CNTR_1                 1

#define XHDCP22_TX_HDCPPORT_VERSION_OFFSET      0x50 /**< DDC version offset. */
#define XHDCP22_TX_HDCPPORT_WRITE_MSG_OFFSET    0x60 /**< DDC write message buffer offset. */
#define XHDCP22_TX_HDCPPORT_RXSTATUS_OFFSET     0x70 /**< DDC RX status offset. */
#define XHDCP22_TX_HDCPPORT_READ_MSG_OFFSET     0x80 /**< DDC read message buffer offset. */

/** HDCP Port DDC Rx status register masks. */
#define XHDCP22_TX_RXSTATUS_REAUTH_REQ_MASK     (1<<11) /**< RX status REAUTHENTICATION bit. */
#define XHDCP22_TX_RXSTATUS_READY_MASK          (1<<10) /**< RX status READY bit. */
#define XHDCP22_TX_RXSTATUS_AVAIL_BYTES_MASK    (0x3FF) /**< RX status available bytes in read message buffer. */

/** RX certificate and Tx public key sizes in bytes. */
#define XHDCP22_TX_CERT_RCVID_SIZE              5   /**< Unique receiver Id size in the RX certificate. */
#define XHDCP22_TX_CERT_PUB_KEY_N_SIZE          128 /**< Public key-N size in the RX certificate. */
#define XHDCP22_TX_CERT_PUB_KEY_E_SIZE          3   /**< Public key-E size in the RX certificate. */
#define XHDCP22_TX_CERT_RSVD_SIZE               2   /**< Reserved size in the RX certificate. */
#define XHDCP22_TX_CERT_SIGNATURE_SIZE          384 /**< Signature size in the RX certificate. */
/** Total size of the RX certificate. */
#define XHDCP22_TX_CERT_SIZE                     \
       ( XHDCP22_TX_CERT_RCVID_SIZE +            \
         XHDCP22_TX_CERT_PUB_KEY_N_SIZE +        \
         XHDCP22_TX_CERT_PUB_KEY_E_SIZE +        \
         XHDCP22_TX_CERT_RSVD_SIZE +             \
         XHDCP22_TX_CERT_SIGNATURE_SIZE )
#define XHDCP22_TX_RXCAPS_SIZE                  3   /**< RX capabilities size. */
#define XHDCP22_TX_TXCAPS_SIZE                  3   /**< TX capabilities size. */
#define XHDCP22_TX_KPUB_DCP_LLC_N_SIZE          384 /**< LLC public key-N size. */
#define XHDCP22_TX_KPUB_DCP_LLC_E_SIZE          1   /**< LLC public key-E size. */

/* defines for de/encryption. */
#define XHDCP22_TX_SHA256_HASH_SIZE             32 /**< SHA256 hash size in bytes. */
#define XHDCP22_TX_AES128_SIZE                  16 /**< AES128 keys in bytes. */

/* Sizes of keys in bytes. */
#define XHDCP22_TX_RTX_SIZE                     8     /**< 64 bits. */
#define XHDCP22_TX_RRX_SIZE                     8     /**< 64 bits. */
#define XHDCP22_TX_KM_SIZE                      XHDCP22_TX_AES128_SIZE
#define XHDCP22_TX_E_KPUB_KM_SIZE               128   /**< 1024 bits. */
#define XHDCP22_TX_H_PRIME_SIZE                 32    /**< 256 bits. */
#define XHDCP22_TX_EKH_KM_SIZE                  16    /**< 128 bits. */
#define XHDCP22_TX_KM_MSK_SEED_SIZE             XHDCP22_TX_SHA256_HASH_SIZE
#define XHDCP22_TX_RN_SIZE                      8     /**< 64-bits. */
#define XHDCP22_TX_RIV_SIZE                     8     /**< 64-bits. */
#define XHDCP22_TX_L_PRIME_SIZE                 32    /**< 256 bits. */
#define XHDCP22_TX_KS_SIZE                      16    /**< 128 bits. */
#define XHDCP22_TX_EDKEY_KS_SIZE                16    /**< 128 bits. */

/**************************** Type Definitions *******************************/

/**
* This typedef contains the the internal (non-confidential) used keys used for
* authentication with stored Km. After initial pairing, these are stored in
* secure non-volatile storage.
*/
typedef struct {
	u8 ReceiverId[XHDCP22_TX_CERT_RCVID_SIZE]; /**< Unique receiver Id. */
	u8 RxCaps[XHDCP22_TX_RXCAPS_SIZE];         /**< Capabilities of the receiver. */
	u8 Rtx[XHDCP22_TX_RTX_SIZE];               /**< Random nonce for tx. */
	u8 Rrx[XHDCP22_TX_RRX_SIZE];               /**< Random nonce for Rx (m: Rtx || Rrx). */
	u8 Km[XHDCP22_TX_KM_SIZE];                 /**< Km. */
	u8 Ekh_Km[XHDCP22_TX_EKH_KM_SIZE];         /**< Ekh(Km). */
} XHdcp22_Tx_PairingInfo;

/**
* This typedef contains the public key certificate of Receiver that is received
* with AKE_Send_Cert.
*/
typedef	struct {
	u8 ReceiverId[XHDCP22_TX_CERT_RCVID_SIZE];
	u8 N[XHDCP22_TX_CERT_PUB_KEY_N_SIZE];
	u8 e[XHDCP22_TX_CERT_PUB_KEY_E_SIZE];
	u8 Reserved[XHDCP22_TX_CERT_RSVD_SIZE];
	u8 Signature[XHDCP22_TX_CERT_SIGNATURE_SIZE];
} XHdcp22_Tx_CertRx;

/**
* This typedef contains the received AKE_Send_Cert message definition.
*/
typedef struct
{
	u8 MsgId;

	XHdcp22_Tx_CertRx CertRx;
	u8 Rrx[XHDCP22_TX_RRX_SIZE];
	u8 RxCaps[XHDCP22_TX_RXCAPS_SIZE];
} XHdcp22_Tx_AKESendCert;

/**
* This typedef contains the received AKE_Send_H_prime message definition.
*/
typedef struct
{
	u8 MsgId;

	u8 HPrime[XHDCP22_TX_H_PRIME_SIZE];
} XHdcp22_Tx_AKESendHPrime;

/**
* This typedef contains the received AKE_Send_Pairing_Info message definition.
*/
typedef struct
{
	u8 MsgId;

	u8 EKhKm[XHDCP22_TX_EKH_KM_SIZE];
} XHdcp22_Tx_AKESendPairingInfo;

/**
* This typedef contains the received AKE_Send_L_prime message definition.
*/
typedef struct
{
	u8 MsgId;

	u8 LPrime[XHDCP22_TX_L_PRIME_SIZE];
} XHdcp22_Tx_LCSendLPrime;

/**
* This typedef contains the transmitted AKE_Init message definition.
*/
typedef struct
{
	u8 MsgId;

	u8 Rtx[XHDCP22_TX_RTX_SIZE];
	u8 TxCaps[XHDCP22_TX_TXCAPS_SIZE];
} XHdcp22_Tx_AKEInit;

/**
* This typedef contains the transmitted AKE_No_Stored_km message definition.
*/
typedef struct
{
	u8 MsgId;

	u8 EKpubKm[XHDCP22_TX_E_KPUB_KM_SIZE];
} XHdcp22_Tx_AKENoStoredKm;


/**
* This typedef contains the transmitted AKE_Stored_km message definition.
*/
typedef struct
{
	u8 MsgId;

	u8 EKhKm[XHDCP22_TX_EKH_KM_SIZE];
	u8 Rtx[XHDCP22_TX_RTX_SIZE];	// In the protocol defined as M=Rtx || Rrx
	u8 Rrx[XHDCP22_TX_RRX_SIZE];
} XHdcp22_Tx_AKEStoredKm;

/**
* This typedef contains the transmitted LC_Init message definition.
*/
typedef struct
{
	u8 MsgId;

	u8 Rn[XHDCP22_TX_RN_SIZE];
} XHdcp22_Tx_LCInit;

/**
* This typedef contains the transmitted SKE_Send_Eks message definition.
*/
typedef struct
{
	u8 MsgId;

	u8 EDkeyKs[XHDCP22_TX_EDKEY_KS_SIZE];
	u8 Riv[XHDCP22_TX_RIV_SIZE];
} XHdcp22_Tx_SKESendEks;

/**
* Message buffer structure.
*/
typedef union
{
	/* Message Id. */
	u8 MsgId;

	/* Received messages. */
	XHdcp22_Tx_AKESendCert        AKESendCert;
	XHdcp22_Tx_AKESendHPrime      AKESendHPrime;
	XHdcp22_Tx_AKESendPairingInfo AKESendPairingInfo;
	XHdcp22_Tx_LCSendLPrime       LCSendLPrime;

	/* Transmitted messages. */
	XHdcp22_Tx_AKEInit            AKEInit;
	XHdcp22_Tx_AKENoStoredKm      AKENoStoredKm;
	XHdcp22_Tx_AKEStoredKm        AKEStoredKm;
	XHdcp22_Tx_LCInit             LCInit;
	XHdcp22_Tx_SKESendEks         SKESendEks;
} XHdcp22_Tx_Message;

/**
* Message including the DDC Address.
*/
typedef struct
{
	u8 DdcAddress;
	XHdcp22_Tx_Message Message;
} XHdcp22_Tx_DDCMessage;

/**
* Value definitions for debugging.
* These values are used as parameter for the #XHDCP22_TX_LOG_EVT_DBG
* logging event.
*/
typedef enum
{
	XHDCP22_TX_LOG_DBG_STARTIMER,
	XHDCP22_TX_LOG_DBG_MSGAVAILABLE,
	XHDCP22_TX_LOG_DBG_TX_AKEINIT,
	XHDCP22_TX_LOG_DBG_RX_CERT,
	XHDCP22_TX_LOG_DBG_VERIFY_SIGNATURE,
	XHDCP22_TX_LOG_DBG_VERIFY_SIGNATURE_DONE,
	XHDCP22_TX_LOG_DBG_ENCRYPT_KM,
	XHDCP22_TX_LOG_DBG_ENCRYPT_KM_DONE,
	XHDCP22_TX_LOG_DBG_TX_NOSTOREDKM,
	XHDCP22_TX_LOG_DBG_TX_STOREDKM,
	XHDCP22_TX_LOG_DBG_RX_H1,
	XHDCP22_TX_LOG_DBG_RX_EKHKM,
	XHDCP22_TX_LOG_DBG_COMPUTE_H,
	XHDCP22_TX_LOG_DBG_COMPUTE_H_DONE,
	XHDCP22_TX_LOG_DBG_TX_LCINIT,
	XHDCP22_TX_LOG_DBG_RX_L1,
	XHDCP22_TX_LOG_DBG_COMPUTE_L,
	XHDCP22_TX_LOG_DBG_COMPUTE_L_DONE,
	XHDCP22_TX_LOG_DBG_TX_EKS,
	XHDCP22_TX_LOG_DBG_COMPUTE_EDKEYKS,
	XHDCP22_TX_LOG_DBG_COMPUTE_EDKEYKS_DONE,
	XHDCP22_TX_LOG_DBG_CHECK_REAUTH,
	XHDCP22_TX_LOG_DBG_TIMEOUT,
	XHDCP22_TX_LOG_DBG_TIMESTAMP,
	XHDCP22_TX_LOG_DBG_AES128ENC,
	XHDCP22_TX_LOG_DBG_AES128ENC_DONE,
	XHDCP22_TX_LOG_DBG_SHA256HASH,
	XHDCP22_TX_LOG_DBG_SHA256HASH_DONE,
	XHDCP22_TX_LOG_DBG_OEAPENC,
	XHDCP22_TX_LOG_DBG_OEAPENC_DONE,
	XHDCP22_TX_LOG_DBG_RSAENC,
	XHDCP22_TX_LOG_DBG_RSAENC_DONE
} XHdcp22_Tx_LogDebugValue;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/* Crypto functionality. */
void XHdcp22Tx_MemXor(u8 *Output, const u8 *InputA, const u8 *InputB,
                      unsigned int Size);
int XHdcp22Tx_VerifyCertificate(const XHdcp22_Tx_CertRx *CertificatePtr,
                                const u8* KpubDcpNPtr, int KpubDcpNSize,
                                const u8* KpubDcpEPtr, int KpubDcpESize);
void XHdcp22Tx_ComputeHPrime(const u8 *Rrx, const u8 *RxCaps,
                             const u8* Rtx,  const u8 *TxCaps,
                             const u8 *Km, u8 *HPrime);
void XHdcp22Tx_ComputeLPrime(const u8* Rn, const u8 *Km,
                             const u8 *Rrx, const u8 *Rtx,
                             u8 *LPrime);
void XHdcp22Tx_ComputeEdkeyKs(const u8* Rn, const u8* Km,
                              const u8 *Ks, const u8 *Rrx,
                              const u8 *Rtx,  u8 *EdkeyKs);
int XHdcp22Tx_EncryptKm(const XHdcp22_Tx_CertRx* CertificatePtr,
                        const u8* KmPtr, u8 *MaskingSeedPtr,
                        u8* EncryptedKmPtr);

/* Testing functionality. */
void XHdcp22Tx_Dump(const char *string, const u8 *m, u32 mlen);
u32 XHdcp22Tx_LogGetTimeUSecs(XHdcp22_Tx *InstancePtr);
u8 XHdcp22Tx_TestSimulateTimeout(XHdcp22_Tx *InstancePtr);
void XHdcp22Tx_LogWrNoInst(XHdcp22_Tx_LogEvt Evt, u16 Data);

/* Functionality offloaded to hardware, but in testmode generated by software. */
void XHdcp22Tx_GenerateRandom_Test(int NumOctets, u8* RandomNumberPtr);

/* Basic encryption functions. */
void XHdcp22Tx_GenerateRandom(XHdcp22_Tx *InstancePtr, int NumOctets,
                              u8* RandomNumberPtr);

/* Functionality only used for self-testing. */
void XHdcp22Tx_GenerateRtx_Test(XHdcp22_Tx *InstancePtr, u8* RtxPtr);
void XHdcp22Tx_GenerateKm_Test(XHdcp22_Tx *InstancePtr, u8* KmPtr);
void XHdcp22Tx_GenerateKmMaskingSeed_Test(XHdcp22_Tx *InstancePtr, u8* SeedPtr);
void XHdcp22Tx_GenerateRn_Test(XHdcp22_Tx *InstancePtr, u8* RnPtr);
void XHdcp22Tx_GenerateRiv_Test(XHdcp22_Tx *InstancePtr, u8* RivPtr);
void XHdcp22Tx_GenerateKs_Test(XHdcp22_Tx *InstancePtr, u8* KsPtr);
const u8* XHdcp22Tx_GetKPubDpc_Test(XHdcp22_Tx *InstancePtr);

/************************** Variable Definitions *****************************/
#ifdef __cplusplus
}
#endif

#endif /* End of protection macro. */
