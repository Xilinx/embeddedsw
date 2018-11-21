/******************************************************************************
*
* (c) Copyright 2013 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_aes.h
*
* This file contains hardware interface related information for CSU AES device
*
* This driver supports the following features:
*
* - AES decryption with/without keyrolling
* - Authentication using GCM tag
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
*
* </pre>
*
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

#define XSECURE_CSU_AES_STS_AES_BUSY	(0x00000001U << 0) /**< AES busy */
#define XSECURE_CSU_AES_STS_AES_READY	(0x00000001U << 1)
					/**< Ready to Receive Data */
#define XSECURE_CSU_AES_STS_AES_DONE	(0x00000001U << 2)
					/**< Operation Complete */
#define XSECURE_CSU_AES_STS_GCM_TAG_OK	(0x00000001U << 3) /**< GCM Tag Passed */
#define XSECURE_CSU_AES_STS_KEY_INIT_DONE	(0x00000001U << 4)
					/**< Key Initialize */
#define XSECURE_CSU_AES_STS_AES_KEY_ZERO	(0x00000001U << 8)
					/**< AES key zeroed */
#define XSECURE_CSU_AES_STS_KUP_ZEROED	(0x00000001U << 9) /**< KUP key Zeroed */
#define XSECURE_CSU_AES_STS_BOOT_KEY_ZERO	(0x00000001U << 10)
					/**< Boot Key zeroed */
#define XSECURE_CSU_AES_STS_OKR_ZERO 	(0x00000001U << 11)
					/**< Operational Key zeroed */

#define XSECURE_CSU_AES_KEY_SRC_KUP	(0x0U) /**< KUP key source */
#define XSECURE_CSU_AES_KEY_SRC_DEV	(0x1U) /**< Device Key source */

#define XSECURE_CSU_AES_CHUNKING_DISABLED (0x0U)
#define XSECURE_CSU_AES_CHUNKING_ENABLED (0x1U)

#define XSECURE_CSU_AES_KEY_LOAD	(0x00000001U << 0)
					/**< Load AES key from Source */

#define XSECURE_CSU_AES_START_MSG	(0x00000001U << 0) /**< AES Start message */

#define XSECURE_CSU_AES_KUP_WR		(0x00000001U << 0)
					/**< Direct AES Output to KUP */
#define XSECURE_CSU_AES_IV_WR		(0x00000001U << 1)
					/**< Direct AES Output to IV Reg */

#define XSECURE_CSU_AES_RESET		(0x00000001U << 0) /**< Reset Value */

#define XSECURE_CSU_AES_KEY_ZERO	(0x00000001U << 0)
					/**< set AES key to zero */
#define XSECURE_CSU_AES_KUP_ZERO	(0x00000001U << 1)
					/**< Set KUP Reg. to zero */

#define XSECURE_CSU_AES_CFG_DEC		(0x0U) /**< AES mode Decrypt */
#define XSECURE_CSU_AES_CFG_ENC		(0x1U) /**< AES Mode Encrypt */

#define XSECURE_CSU_KUP_WR		(0x00000001U << 0)
					/**< Direct output to KUP */
#define XSECURE_CSU_IV_WR		(0x00000001U << 4)
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

#define XSECURE_SECURE_HDR_SIZE		(48U)
					/**< Secure Header Size in Bytes*/
#define XSECURE_SECURE_GCM_TAG_SIZE	(16U) /**< GCM Tag Size in Bytes */

#define XSECURE_DESTINATION_PCAP_ADDR    (0XFFFFFFFFU)

#define AES_AES_STATUS_BLACK_KEY_DEC_DONE_MASK	(0x00000020U)

#define XSECURE_AES_BBRAM_KEY		(0xBBDE6600U)
#define XSECURE_AES_RED_BBRAM_KEY	(0xBBDE8200U)
#define XSECURE_AES_BH_KEY			(0xBDB06600U)
#define XSECURE_AES_RED_BH_KEY 		(0xBDB08200U)
#define XSECURE_AES_EFUSE_KEY 		(0xEFDE6600U)
#define XSECURE_AES_RED_EFUSE_KEY 	(0xEFDE8200U)
#define XSECURE_AES_EFUSE_USER_KEY_0 (0xEF856601U)
#define XSECURE_AES_EFUSE_RED_USER_KEY_0	(0xEF858201U)
#define XSECURE_AES_EFUSE_USER_KEY_1 (0xEF856602U)
#define XSECURE_AES_EFUSE_RED_USER_KEY_1 (0xEF858202U)
#define XSECURE_AES_KUP_KEY		(0xBDC98200U)
#define XSECURE_AES_FAMILY_KEY (0xFEDE8200U)
#define XSECURE_AES_PUF_KEK_KEY (0xDBDE8200U)
#define XSECURE_AES_USER_KEY_0	(0xBD858201U)
#define XSECURE_AES_USER_KEY_1	(0xBD858202U)
#define XSECURE_AES_USER_KEY_2	(0xBD858204U)
#define XSECURE_AES_USER_KEY_3	(0xBD858208U)
#define XSECURE_AES_USER_KEY_4	(0xBD858210U)
#define XSECURE_AES_USER_KEY_5	(0xBD858220U)
#define XSECURE_AES_USER_KEY_6	(0xBD858240U)
#define XSECURE_AES_USER_KEY_7	(0xBD858280U)



enum {
	XSECURE_USER_KEY_0,
	XSECURE_USER_KEY_1,
	XSECURE_USER_KEY_2,
	XSECURE_USER_KEY_3,
	XSECURE_USER_KEY_4,
	XSECURE_USER_KEY_5,
	XSECURE_USER_KEY_6,
	XSECURE_USER_KEY_7,
	XSECURE_EFUSE_KEY,
	XSECURE_EFUSE_USER_KEY_0,
	XSECURE_EFUSE_USER_KEY_1,
	XSECURE_BBRAM_KEY,
	XSECURE_KUP_KEY,
	XSECURE_BH_KEY = 15,
};

typedef enum {
	XSECURE_KEY, /* key stored in BBRAM, EFUSE or BH(KEK key), User keys */
	XSECURE_RED_KEY /* Red version of key stored in eFUSE, BBRAM or BH */
}XSecure_KeyType;

typedef enum {
	XSECURE_BLACK_KEY,
	XSECURE_OBFUSCATED_KEY
}XSecure_KekType;

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
	u8 DstEndinaness; /* Don't change the endianess of Destination if this flag is been set */
} XSecure_Aes;

/************************** Function Prototypes ******************************/

/* Initialization Functions */

u32  XSecure_AesInitialize(XSecure_Aes *InstancePtr, XCsuDma *CsuDmaPtr,
				u32 KeySel, const u8 *Iv);


/* Decryption of boot image created by using bootgen */
u32 XSecure_AesDecrypt(XSecure_Aes *InstancePtr, u8 *Dst, const u8 *Src,
				u32 Length);

/* Reset */
void XSecure_AesReset(XSecure_Aes  *InstancePtr);

u32 XSecure_AesWaitForDone(XSecure_Aes *InstancePtr);

/** @cond xsecure_internal
@{ */
u32 XSecure_AesKeySelNLoad(XSecure_Aes *InstancePtr);
u32 XSecure_AesDecryptBlk(XSecure_Aes *InstancePtr, u8 *Dst,
			const u8 *Src, const u8 *Tag, u32 Len, u32 Flag);
u32 XSecure_AesKekDecrypt(XSecure_Aes *InstancePtr,
		XSecure_KekType KeyType);

u32 XSecure_AesKeyClear(XSecure_Aes *InstancePtr, u32 Mask);
u32 XSecure_WriteKey(XSecure_Aes *InstancePtr, u32 KeySel, u32 *Key);
#endif /* XSECURE_AES_H_ */
