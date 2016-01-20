/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc.  All rights reserved.
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
* @file xhdcp22_rx_i.h
*
* This header file contains internal data types and functions declarations
* for the Xilinx HDCP 2.2 Receiver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00  MH   10/30/15 First Release
*</pre>
*
*****************************************************************************/

#ifndef XHDCP22_RX_I_H_		/* prevent circular inclusions */
#define XHDCP22_RX_I_H_		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#include "xhdcp22_mmult.h"

/************************** Constant Definitions ****************************/
#define XHDCP22_RX_HASH_SIZE		32	/**< Hash size */
#define XHDCP22_RX_N_SIZE			128	/**< Modulus size */
#define XHDCP22_RX_P_SIZE			64	/**< RSA private parameter size */
#define XHDCP22_RX_KM_SIZE			16	/**< Km size */
#define XHDCP22_RX_EKH_SIZE			16	/**< Ekh size */
#define XHDCP22_RX_KD_SIZE			32	/**< Kd size */
#define XHDCP22_RX_HPRIME_SIZE		32	/**< HPrime size */
#define XHDCP22_RX_LPRIME_SIZE		32	/**< LPrime size */
#define XHDCP22_RX_RN_SIZE			8	/**< Rn size */
#define XHDCP22_RX_RIV_SIZE			8	/**< Riv size */
#define XHDCP22_RX_KS_SIZE			16	/**< Ks size */
#define XHDCP22_RX_AES_SIZE			16	/**< AES size */
#define XHDCP22_RX_RTX_SIZE			8	/**< Rtx size */
#define XHDCP22_RX_RRX_SIZE			8	/**< Rrx size */
#define XHDCP22_RX_TXCAPS_SIZE		3	/**< TxCaps size */
#define XHDCP22_RX_RXCAPS_SIZE		3	/**< RxCaps size*/
#define XHDCP22_RX_CERT_SIZE		522	/**< DCP certificate size */
#define XHDCP22_RX_PRIVATEKEY_SIZE	320	/**< RSA private key size (64*5) */
#define XHDCP22_RX_LC128_SIZE		16	/**< Lc128 global constant size */

/**************************** Type Definitions ******************************/

/**
 * These constants are the message identification codes.
 */
typedef enum
{
	XHDCP22_RX_MSG_ID_AKEINIT				= 2,  /**< AKE_Init message ID */
	XHDCP22_RX_MSG_ID_AKESENDCERT			= 3,  /**< AKE_Send_Cert message ID */
	XHDCP22_RX_MSG_ID_AKENOSTOREDKM			= 4,  /**< AKE_No_Stored_km message ID */
	XHDCP22_RX_MSG_ID_AKESTOREDKM			= 5,  /**< AKE_Stored_km message ID */
	XHDCP22_RX_MSG_ID_AKESENDHPRIME			= 7,  /**< AKE_Send_H_prime message ID */
	XHDCP22_RX_MSG_ID_AKESENDPAIRINGINFO	= 8,  /**< AKE_Send_Pairing_Info message ID */
	XHDCP22_RX_MSG_ID_LCINIT				= 9,  /**< LC_Init message ID */
	XHDCP22_RX_MSG_ID_LCSENDLPRIME			= 10, /**< LC_Send_L_prime message ID */
	XHDCP22_RX_MSG_ID_SKESENDEKS			= 11  /**< SKE_Send_Eks message ID */
} XHdcp22_Rx_MessageIds;

/**
 * These constants are the discrete event states for standalone unit testing.
 */
typedef enum
{
	XHDCP22_RX_TEST_STATE_UNAUTHENTICATED		= 0xB00,
	XHDCP22_RX_TEST_STATE_SEND_AKEINIT			= 0xB10,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDCERT		= 0xB11,
	XHDCP22_RX_TEST_STATE_SEND_AKENOSTOREDKM	= 0xB12,
	XHDCP22_RX_TEST_STATE_SEND_AKESTOREDKM		= 0xB13,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDHPRIME	= 0xB14,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDPAIRING	= 0xB15,
	XHDCP22_RX_TEST_STATE_SEND_LCINIT			= 0xB20,
	XHDCP22_RX_TEST_STATE_WAIT_LCSENDLPRIME		= 0xB21,
	XHDCP22_RX_TEST_STATE_SEND_SKESENDEKS		= 0xB30,
	XHDCP22_RX_TEST_STATE_WAIT_AUTHENTICATED	= 0xB40
} XHdcp22_Rx_TestState;

/**
 * These constants define the test ddc access types for standalone unit testing.
 */
typedef enum
{
	XHDCP22_RX_TEST_DDC_ACCESS_WO,	/**< Write-Only */
	XHDCP22_RX_TEST_DDC_ACCESS_RO,	/**< Read-Only */
	XHDCP22_RX_TEST_DDC_ACCESS_RW	/**< Read-Write */
} XHdcp22_Rx_TestDdcAccess;

/**
 * This typedef is the test DDC register definition.
 */
typedef struct
{
	u8 Address;
	u8 Name[20];
	XHdcp22_Rx_TestDdcAccess Access;
	u8 Value;
} XHdcp22_Rx_TestDdcReg;

/**
 * This typedef is the RSA private key quintuple definition.
 */
typedef struct
{
	u8 p[64];
	u8 q[64];
	u8 dp[64];
	u8 dq[64];
	u8 qinv[64];
} XHdcp22_Rx_KprivRx;

/**
 * This typedef is the RSA public key definition.
 */
typedef struct
{
	u8 N[128];
	u8 e[3];
} XHdcp22_Rx_KpubRx;

/**
 * This typedef is the DCP public certificate definition.
 */
typedef struct
{
	u8 ReceiverId[5];
	u8 KpubRx[131];
	u8 Reserved[2];
	u8 Signature[384];
} XHdcp22_Rx_CertRx;

/**
 * This typedef is the AKE_Init message definition.
 */
typedef struct
{
	u8 MsgId;
	u8 Rtx[8];
	u8 TxCaps[3];
} XHdcp22_Rx_AKEInit;

/**
 * This typedef is the AKE_Send_Cert message definition.
 */
typedef struct
{
	u8 MsgId;
	u8 CertRx[522];
	u8 Rrx[8];
	u8 RxCaps[3];
} XHdcp22_Rx_AKESendCert;

/**
 * This typedef is the AKE_No_Stored_km message definition.
 */
typedef struct
{
	u8 MsgId;
	u8 EKpubKm[128];
} XHdcp22_Rx_AKENoStoredKm;

/**
 * This typedef is the AKE_Stored_km message definition.
 */
typedef struct
{
	u8 MsgId;
	u8 EKhKm[16];
	u8 M[16];
} XHdcp22_Rx_AKEStoredKm;

/**
 * This typedef is the AKE_Send_H_prime message definition.
 */
typedef struct
{
	u8 MsgId;
	u8 HPrime[32];
} XHdcp22_Rx_AKESendHPrime;

/**
 * This typedef is the AKE_Send_Pairing_Info message definition.
 */
typedef struct
{
	u8 MsgId;
	u8 EKhKm[16];
} XHdcp22_Rx_AKESendPairingInfo;

/**
 * This typedef is the LC_Init message definition.
 */
typedef struct
{
	u8 MsgId;
	u8 Rn[8];
} XHdcp22_Rx_LCInit;

/**
 * This typdef is the LC_Send_L_prime message definition.
 */
typedef struct
{
	u8 MsgId;
	u8 LPrime[32];
} XHdcp22_Rx_LCSendLPrime;

/**
 * This typedef is the SKE_Send_Eks message definition.
 */
typedef struct
{
	u8 MsgId;
	u8 EDkeyKs[16];
	u8 Riv[8];
} XHdcp22_Rx_SKESendEks;

/**
 * This typedef is the union of all the message types.
 */
typedef union
{
	u8 MsgId;
	XHdcp22_Rx_AKEInit            AKEInit;
	XHdcp22_Rx_AKESendCert        AKESendCert;
	XHdcp22_Rx_AKENoStoredKm      AKENoStoredKm;
	XHdcp22_Rx_AKEStoredKm        AKEStoredKm;
	XHdcp22_Rx_AKESendHPrime      AKESendHPrime;
	XHdcp22_Rx_AKESendPairingInfo AKESendPairingInfo;
	XHdcp22_Rx_LCInit             LCInit;
	XHdcp22_Rx_LCSendLPrime       LCSendLPrime;
	XHdcp22_Rx_SKESendEks         SKESendEks;
} XHdcp22_Rx_Message;

/***************** Macros (Inline Functions) Definitions ********************/
#define XHdcp22Rx_Printf(type, ...) (((type) & TRUE) ? printf (__VA_ARGS__) : 0)

/************************** Function Prototypes *****************************/
void XHdcp22Rx_GenerateRandom(int NumOctets, u8* RandomNumberPtr);
int  XHdcp22Rx_RsaesOaepEncrypt(const XHdcp22_Rx_KpubRx *KpubRx, const u8 *Message, const u32 MessageLen, const u8 *MaskingSeed, u8 *EncryptedMessage);
void XHdcp22Rx_ComputeHPrime(const u8* Rrx, const u8 *RxCaps, const u8* Rtx, const u8 *TxCaps, const u8 *Km, u8 *Kd, u8 *HPrime);
void XHdcp22Rx_ComputeEkh(const u8 *KprivRx, const u8 *Km, const u8 *M, u8 *Ekh);
void XHdcp22Rx_ComputeLPrime(const u8 *Rn, const u8 *Kd, const u8 *Rrx, u8 *LPrime);
void XHdcp22Rx_ComputeKs(const u8* Rrx, const u8* Rtx, const u8 *Km, const u8 *Rn, const u8 *Eks, u8 * Ks);

#ifdef __cplusplus
}
#endif

#endif /* XHDCP22_RX_I_H_ */
