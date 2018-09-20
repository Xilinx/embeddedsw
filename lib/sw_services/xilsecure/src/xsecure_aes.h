/******************************************************************************
*
* Copyright (C) 2014 - 18 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_aes.h
* @addtogroup xsecure_aes_apis XilSecure AES APIs
* @{
* @cond xsecure_internal
* This file contains hardware interface related information for CSU AES device
*
* This driver supports the following features:
*
* - AES decryption with/without keyrolling
* - Authentication using GCM tag
* - AES encryption
*
* <b>Initialization & Configuration</b>
*
* The Aes driver instance can be initialized
* in the following way:
*
*   - XSecure_AesInitialize(XSecure_Aes *InstancePtr, XCsuDma *CsuDmaPtr,
*					u32 KeySel, u32* Iv, u32* Key)
*
* The key for decryption can be the device key or user provided key.
* KeySel variable denotes the key to be used. In case the key is user
* provided, key has to be provided in Key variable. If it is device key,
* the key variable will be ignored and device key will be used
*
* The initial Initialization vector will be used for decrypting secure header
* and block 0 of given encrypted data.
*
*
* @note
*	-The format of encrypted data(boot image) has to be exactly as
*	 specified by the bootgen. Any encrypted data has to start with a
*	 secure header first and then the data blocks.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ba   10/10/14 Initial release
* 1.1   ba   11/10/15 Modified Key loading logic in AES encryption
* 2.0   vns  01/28/17 Added APIs for decryption which can be used for decrypting
*                     data rather than a boot image.
*       vns  02/03/17 Added APIs for encryption in generic way.
*                     Modified existing XSecure_AesEncrypt to
*                     XSecure_AesEncryptData, and added XSecure_AesEncryptInit
*                     and XSecure_AesEncryptUpdate APIs for generic usage.
* 2.2   vns  07/06/16 Added doxygen tags
* 3.0   vns  02/19/18 Added error code for key clear
*                     XSECURE_CSU_AES_KEY_CLEAR_ERROR and timeout macro
*                     XSECURE_AES_TIMEOUT_MAX
* 3.1   ka   03/16/18 Added Zeroization of Aes Decrypted data in case of
*                    GCM_TAG_MISMATCH
* </pre>
* @endcond
*
******************************************************************************/

#ifndef XSECURE_CSU_AES_H
#define XSECURE_CSU_AES_H

/************************** Include Files ***********************************/

#include "xsecure_hw.h"
#include "xcsudma.h"
#include "xstatus.h"
#include "xil_io.h"

/************************** Constant Definitions ****************************/
/** @cond xsecure_internal
@{
*/
#define XSECURE_CSU_AES_STS_AES_BUSY	(1U << 0) /**< AES busy */
#define XSECURE_CSU_AES_STS_AES_READY	(1U << 1)
					/**< Ready to Receive Data */
#define XSECURE_CSU_AES_STS_AES_DONE	(1U << 2)
					/**< Operation Complete */
#define XSECURE_CSU_AES_STS_GCM_TAG_OK	(1U << 3) /**< GCM Tag Passed */
#define XSECURE_CSU_AES_STS_KEY_INIT_DONE	(1U << 4)
					/**< Key Initialize */
#define XSECURE_CSU_AES_STS_AES_KEY_ZERO	(1U << 8)
					/**< AES key zeroed */
#define XSECURE_CSU_AES_STS_KUP_ZEROED	(1U << 9) /**< KUP key Zeroed */
#define XSECURE_CSU_AES_STS_BOOT_KEY_ZERO	(1U << 10)
					/**< Boot Key zeroed */
#define XSECURE_CSU_AES_STS_OKR_ZERO 	(1U << 11)
					/**< Operational Key zeroed */

#define XSECURE_CSU_AES_KEY_SRC_KUP	(0x0U) /**< KUP key source */
#define XSECURE_CSU_AES_KEY_SRC_DEV	(0x1U) /**< Device Key source */

#define XSECURE_CSU_AES_CHUNKING_DISABLED (0x0U)
#define XSECURE_CSU_AES_CHUNKING_ENABLED (0x1U)

#define XSECURE_CSU_AES_KEY_LOAD	(1U << 0)
					/**< Load AES key from Source */

#define XSECURE_CSU_AES_START_MSG	(1U << 0) /**< AES Start message */

#define XSECURE_CSU_AES_KUP_WR		(1U << 0)
					/**< Direct AES Output to KUP */
#define XSECURE_CSU_AES_IV_WR		(1U << 1)
					/**< Direct AES Output to IV Reg */

#define XSECURE_CSU_AES_RESET		(1U << 0) /**< Reset Value */

#define XSECURE_CSU_AES_KEY_ZERO	(1U << 0)
					/**< set AES key to zero */
#define XSECURE_CSU_AES_KUP_ZERO	(1U << 1)
					/**< Set KUP Reg. to zero */

#define XSECURE_CSU_AES_CFG_DEC		(0x0U) /**< AES mode Decrypt */
#define XSECURE_CSU_AES_CFG_ENC		(0x1U) /**< AES Mode Encrypt */

#define XSECURE_CSU_KUP_WR		(1U << 0)
					/**< Direct output to KUP */
#define XSECURE_CSU_IV_WR		(1U << 4)
					/**< image length mismatch */

/* Error Codes and Statuses */
#define XSECURE_CSU_AES_DECRYPTION_DONE	(0L)
					/**< AES Decryption successful */
#define XSECURE_CSU_AES_GCM_TAG_MISMATCH	(1L)
					/**< user provided GCM tag does
						not match calculated tag */
#define XSECURE_CSU_AES_IMAGE_LEN_MISMATCH	(2L)
					/**< image length mismatch */
#define XSECURE_CSU_AES_DEVICE_COPY_ERROR	(3L)
					/**< device copy failed */
#define XSECURE_CSU_AES_ZEROIZATION_ERROR	(4L)
					/**< Zeroization error*/
#define XSECURE_CSU_AES_KEY_CLEAR_ERROR		(0x20)
					/**< AES key clear error */

#define XSECURE_SECURE_HDR_SIZE		(48U)
					/**< Secure Header Size in Bytes*/
#define XSECURE_SECURE_GCM_TAG_SIZE	(16U) /**< GCM Tag Size in Bytes */

#define XSECURE_DESTINATION_PCAP_ADDR    (0XFFFFFFFFU)

#define XSECURE_AES_TIMEOUT_MAX		(0x1FFFFU)

/************************** Type Definitions ********************************/

/**
 * The AES-GCM driver instance data structure. A pointer to an instance data
 * structure is passed around by functions to refer to a specific driver
 * instance.
 */
typedef struct {
	u32 BaseAddress; /**< Device Base Address */
	XCsuDma *CsuDmaPtr; /**< CSUDMA Instance Pointer */
	u32* Iv; /**< Initialization Vector */
	u32* Key; /**< AES Key */
	u32* GcmTagAddr; /**< GCM tag address for decryption */
	u32  KeySel; /**< Key Source selection */
	u8 IsChunkingEnabled; /**< Data Chunking enabled/disabled */
	u8* ReadBuffer; /**< Data Buffer to be used in case of chunking */
	u32 ChunkSize; /**< Size of one chunk in bytes */
	u32 (*DeviceCopy) (u32 SrcAddress, UINTPTR DestAddress, u32 Length);
		/**< Function pointer for copying data chunk from device to buffer.
		 * Arguments are:
		 * SrcAddress: Address of data in device.
		 * DestAddress: Address where data will be copied
		 * Length: Length of data in bytes.
		 * Return value should be 0 in case of success and 1 for failure.
		 */
	u32 SizeofData; /**< Size of Data to be encrypted or decrypted */
	u8  *Destination; /**< Destination for decrypted/encrypted data */
} XSecure_Aes;

/** @}
@endcond */
/************************** Function Prototypes ******************************/

/* Initialization Functions */

s32  XSecure_AesInitialize(XSecure_Aes *InstancePtr, XCsuDma *CsuDmaPtr,
				u32 KeySel, u32* Iv,  u32* Key);

/* Decryption of data */
void XSecure_AesDecryptInit(XSecure_Aes *InstancePtr, u8 * DecData,
		u32 Size, u8 * GcmTagAddr);
s32 XSecure_AesDecryptUpdate(XSecure_Aes *InstancePtr, u8 *EncData, u32 Size);

s32 XSecure_AesDecryptData(XSecure_Aes *InstancePtr, u8 * DecData, u8 *EncData,
		u32 Size, u8 * GcmTagAddr);

/* Decryption of boot image created by using bootgen */
s32 XSecure_AesDecrypt(XSecure_Aes *InstancePtr, u8 *Dst, const u8 *Src,
				u32 Length);

/* Encryption */
void XSecure_AesEncryptInit(XSecure_Aes *InstancePtr, u8 *EncData, u32 Size);
void XSecure_AesEncryptUpdate(XSecure_Aes *InstancePtr, const u8 *Data,
							u32 Size);
void XSecure_AesEncryptData(XSecure_Aes *InstancePtr, u8 *Dst, const u8 *Src,
				u32 Len);

/* Reset */
void XSecure_AesReset(XSecure_Aes  *InstancePtr);

void XSecure_AesWaitForDone(XSecure_Aes *InstancePtr);

/** @cond xsecure_internal
@{ */
void XSecure_AesKeySelNLoad(XSecure_Aes *InstancePtr);
s32 XSecure_AesDecryptBlk(XSecure_Aes *InstancePtr, u8 *Dst,
			const u8 *Src, const u8 *Tag, u32 Len, u32 Flag);
/* Enable/Disable chunking */
void XSecure_AesSetChunking(XSecure_Aes *InstancePtr, u8 Chunking);

/* Configuring Data chunking settings */
void XSecure_AesSetChunkConfig(XSecure_Aes *InstancePtr, u8 *ReadBuffer,
		u32 ChunkSize, u32(*DeviceCopy)(u32, UINTPTR, u32));

/* Zerioze the Aes key */
u32 XSecure_AesKeyZero(XSecure_Aes *InstancePtr);

/** @}
@endcond */

#endif /* XSECURE_AES_H_ */

/**@}*/
