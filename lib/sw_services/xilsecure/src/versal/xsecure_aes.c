/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_aes.c
*
* This file contains AES hardware interface APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 4.0   vns  04/24/2019 Initial release
* 4.1   vns  08/06/2019 Added AES encryption APIs
*       har  08/21/2019 Fixed MISRA C violations
*       vns  08/23/2019 Initialized status variables
* 4.2   har  01/03/2020 Added checks for return value of XSecure_SssAes
*       vns  02/10/2020 Added DPA CM enable/disable function
*       rpo  02/27/2020 Removed function prototype and static keyword of
*                       XSecure_AesKeyLoad, XSecure_AesWaitForDone functions
*       har  03/01/2020 Added code to soft reset once key decryption is done
*       rpo  03/23/2020 Replaced timeouts with WaitForEvent and code clean up
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_utils.h"
#include "xsecure_aes.h"
#include "xsecure_aes_core_hw.h"
#include "xil_util.h"
#include "xsecure_error.h"
/************************** Constant Definitions *****************************/

#define XSECURE_MAX_KEY_SOURCES				XSECURE_AES_EXPANDED_KEYS
#define XSECURE_AES_DISABLE_KUP_IV_UPDATE	(0x0U)
#define XSECURE_ENABLE_BYTE_SWAP			(0x1U)
#define XSECURE_DISABLE_BYTE_SWAP			(0x0U)

/**************************** Type Definitions *******************************/
typedef struct {
	u32 RegOffset;
	u32 KeySrcSelVal;
	u8  UsrWrAllowed;
	u8  DecAllowed;
	u8  EncAllowed;
	u8  KeyDecSrcAllowed;
	u32 KeyDecSrcSelVal;
	u32 KeyClearVal;
} XSecure_AesKeyLookup;

/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/

static u32 XSecure_AesWaitForDone(XSecure_Aes *InstancePtr);
static u32 XSecure_AesKeyLoad(XSecure_Aes *InstancePtr,
		XSecure_AesKeySrc KeySrc, XSecure_AesKeySize KeySize);
static void XSecure_AesPmcDmaCfgEndianness(XPmcDma *InstancePtr,
		XPmcDma_Channel Channel, u8 EndianType);
static u32 XSecure_AesKekWaitForDone(XSecure_Aes *InstancePtr);
static u32 XSecure_AesDpaCmDecryptKat(XSecure_Aes *AesInstance,
		u32 *KeyPtr, u32 *DataPtr, u32 *OutputPtr);
/************************** Variable Definitions *****************************/
static const XSecure_AesKeyLookup AesKeyLookupTbl [XSECURE_MAX_KEY_SOURCES] =
{
	/* BBRAM_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_BBRAM_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  TRUE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_INVALID_CFG
	},

	/* BBRAM_RED_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_BBRAM_RD_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_KEY_DEC_SEL_BBRAM_RED,
	  XSECURE_AES_KEY_CLEAR_BBRAM_RED_KEY_MASK
	},

	/* BH_KEY */
	{ XSECURE_AES_BH_KEY_0_OFFSET,
	  XSECURE_AES_KEY_SEL_BH_KEY,
	  TRUE,
	  TRUE,
	  TRUE,
	  TRUE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_BH_KEY_MASK
	},

	/* BH_RED_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_BH_RD_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_KEY_DEC_SEL_BH_RED,
	  XSECURE_AES_KEY_CLEAR_BH_RED_KEY_MASK
	},

	/* EFUSE_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  TRUE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_EFUSE_KEY_MASK
	},

	/* EFUSE_RED_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_RED_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_KEY_DEC_SEL_EFUSE_RED,
	  XSECURE_AES_KEY_CLEAR_EFUSE_RED_KEY_MASK
	},

	/* EFUSE_USER_KEY_0 */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_USR_KEY0,
	  FALSE,
	  TRUE,
	  TRUE,
	  TRUE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_EFUSE_USER_KEY_0_MASK
	},

	/* EFUSE_USER_KEY_1 */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_USR_KEY1,
	  FALSE,
	  TRUE,
	  TRUE,
	  TRUE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_EFUSE_USER_KEY_1_MASK
	},

	/* EFUSE_USER_RED_KEY_0 */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_USR_RD_KEY0,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_KEY_DEC_SEL_EFUSE_USR0_RED,
	  XSECURE_AES_KEY_CLEAR_EFUSE_USER_RED_KEY_0_MASK
	},

	/* EFUSE_USER_RED_KEY_1 */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_USR_RD_KEY1,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_KEY_DEC_SEL_EFUSE_USR1_RED,
	  XSECURE_AES_KEY_CLEAR_EFUSE_USER_RED_KEY_1_MASK
	},

	/* KUP_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_KUP_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_KUP_KEY_MASK
	},

	/* FAMILY_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_FAMILY_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_INVALID_CFG
	},

	/* PUF_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_PUF_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_PUF_KEY_MASK
	},

	/* USER_KEY_0 */
	{ XSECURE_AES_USER_KEY_0_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_0,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_0_MASK
	},

	/* USER_KEY_1 */
	{ XSECURE_AES_USER_KEY_1_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_1,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_1_MASK
	},

	/* USER_KEY_2 */
	{ XSECURE_AES_USER_KEY_2_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_2,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_2_MASK
	},

	/* USER_KEY_3 */
	{ XSECURE_AES_USER_KEY_3_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_3,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_3_MASK
	},

	/* USER_KEY_4 */
	{ XSECURE_AES_USER_KEY_4_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_4,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_4_MASK
	},

	/* USER_KEY_5 */
	{ XSECURE_AES_USER_KEY_5_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_6,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_5_MASK
	},

	/* USER_KEY_6 */
	{ XSECURE_AES_USER_KEY_6_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_6,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_6_MASK
	},

	/* USER_KEY_7 */
	{ XSECURE_AES_USER_KEY_7_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_7,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_7_MASK
	}
};

/*****************************************************************************/
/**
 * @brief
 * This function initializes the instance pointer.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	PmcDmaPtr	Pointer to the XPmcDma instance.
 *
 * @return	XST_SUCCESS if initialization was successful.
 *
 * @note	All the inputs are accepted in little endian format, but AES
 *		engine accepts the data in big endianness, this will be taken
 *		care while passing data to AES engine.
 *
 ******************************************************************************/
u32 XSecure_AesInitialize(XSecure_Aes *InstancePtr, XPmcDma *PmcDmaPtr)
{
	u32 Status = (u32)XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(PmcDmaPtr != NULL);

	/* Initialize the instance */
	InstancePtr->BaseAddress = XSECURE_AES_BASEADDR;
	InstancePtr->PmcDmaPtr = PmcDmaPtr;
	InstancePtr->AesState = XSECURE_AES_INITIALIZED;

	/* Clear all key zeroization register */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_KEY_CLEAR_OFFSET, XSECURE_AES_KEY_CLEAR_ALL_AES_KEYS);

	XSecure_SssInitialize(&(InstancePtr->SssInstance));

	XSecure_ReleaseReset(InstancePtr->BaseAddress,
		XSECURE_AES_SOFT_RST_OFFSET);

	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function enables or disables DPA counter measures in AES engine based
 * on user input.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	DpaCmCfg	User choice to enable/disable DPA CM.
 *			- TRUE - to enable AES DPA counter measure (Default setting)
 *			- FALSE - to disable AES DPA counter measure
 *
 * @return
 *			- XSECURE_AES_DPA_CM_NOT_SUPPORTED if DPA CM is disbaled on chip.
 *			(Enabling/Disabling in AES engine does not impact functionality)
 *			- XST_SUCCESS if configuration is success.
 *
 ******************************************************************************/
u32 XSecure_AesSetDpaCm(XSecure_Aes *InstancePtr, u32 DpaCmCfg)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ReadReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((DpaCmCfg == TRUE) ||
					(DpaCmCfg == FALSE));

	/* Chip has DPA CM support */
	if ((XSecure_In32(XSECURE_EFUSE_SECURITY_MISC1) &
		XSECURE_EFUSE_DPA_CM_DIS_MASK) == 0x00U) {

		/* Disable/enable DPA CM inside AES engine */
		XSecure_WriteReg(InstancePtr->BaseAddress,
						XSECURE_AES_CM_EN_OFFSET, DpaCmCfg);

		/* Verify status of CM */
		ReadReg = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_AES_STATUS_OFFSET);
		ReadReg = (ReadReg & XSECURE_AES_STATUS_CM_ENABLED_MASK) >>
					XSECURE_AES_STATUS_CM_ENABLED_SHIFT;
		if (ReadReg == DpaCmCfg) {
			Status = (u32)XST_SUCCESS;
		}
	}
	else {
		Status = (u32)XSECURE_AES_DPA_CM_NOT_SUPPORTED;
	}

	return Status;
}
/*****************************************************************************/
/**
 *
 * @brief
 * This function writes the key provided into the specified AES key registers.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	KeySrc		Key Source to be selected to which provided
 *		key should be updated.
 *		- XSECURE_AES_USER_KEY_0
 *		- XSECURE_AES_USER_KEY_1
 *		- XSECURE_AES_USER_KEY_2
 *		- XSECURE_AES_USER_KEY_3
 *		- XSECURE_AES_USER_KEY_4
 *		- XSECURE_AES_USER_KEY_5
 *		- XSECURE_AES_USER_KEY_6
 *		- XSECURE_AES_USER_KEY_7
 *		- XSECURE_AES_BH_KEY
 * @param	KeySize		A variable of type XSecure_AesKeySize, which
 *		holds the size of the input key to be written.
 *		 - XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *		 - XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 * @param	KeyAddr		Address of a buffer which should contain
 *		the key to be written.
 *
 * @return
 *		- XST_SUCCESS on successful written
 *		- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_AesWriteKey(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
			XSecure_AesKeySize KeySize, u64 KeyAddr)
{
	u32 Offset;
	u32 Index = 0U;
	u32 *Key;
	u32 KeySizeInWords;
	u32 Status = (u32)XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(AesKeyLookupTbl[KeySrc].UsrWrAllowed == TRUE);
	Xil_AssertNonvoid((XSECURE_AES_KEY_SIZE_128 == KeySize) ||
			(XSECURE_AES_KEY_SIZE_256 == KeySize));
	Xil_AssertNonvoid(KeyAddr != 0x00U);

	Key = (u32 *)(INTPTR)KeyAddr;


	if ((XSECURE_AES_BH_KEY == KeySrc) &&
			(XSECURE_AES_KEY_SIZE_128 == KeySize)) {
		Status = XST_FAILURE;
		goto END;
	}

	if (XSECURE_AES_KEY_SIZE_128 == KeySize) {
		KeySizeInWords = XSECURE_AES_KEY_SIZE_128BIT_WORDS;
	}
	else {
		KeySizeInWords = XSECURE_AES_KEY_SIZE_256BIT_WORDS;
	}

	Offset = AesKeyLookupTbl[KeySrc].RegOffset;
	if (Offset == XSECURE_AES_INVALID_CFG) {
		Status = XST_FAILURE;
		goto END;
	}

	Offset = Offset + (KeySizeInWords * XSECURE_WORD_SIZE) -
				XSECURE_WORD_SIZE;

	for (Index = 0U; Index < KeySizeInWords; Index++) {
		XSecure_WriteReg(InstancePtr->BaseAddress, Offset,
					Xil_Htonl(Key[Index]));
		Offset = Offset - XSECURE_WORD_SIZE;
	}
	Status = XST_SUCCESS;

END:
	return Status;
}


/*****************************************************************************/
/**
 * This function decrypts the key which is in KEK/Obfuscated key form and
 * exist in either of the boot header/Efuse/BBRAM and updates the mentioned
 * destination red key register with corresponding red key.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	KeyType		The source of key to be used for decryption
 * 			- XSECURE_BLACK_KEY
 * 			- XSECURE_OBFUSCATED_KEY
 * @param	DecKeySrc	Select key source which holds KEK and needs to be
 *		decrypted
 * @param	DstKeySrc	Select the key in which decrypted red key should be
 *		updated
 * @param	IvAddr		Address of IV holding buffer for decryption
 *		of the key.
 * @param	KeySize		A variable of type XSecure_AesKeySize, which
 *		specifies the size of the key.
 *		 - XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *		 - XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 *
 * @return
 *		- XST_SUCCESS on successful key decryption.
 *		- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_AesKekDecrypt(XSecure_Aes *InstancePtr, XSecure_AesKekType KeyType,
			  XSecure_AesKeySrc DecKeySrc,
			  XSecure_AesKeySrc DstKeySrc, u64 IvAddr, u32 KeySize)
{
	u32 Status = (u32)XST_FAILURE;
	XSecure_AesKeySrc KeySrc;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((KeyType == XSECURE_BLACK_KEY) ||
			(KeyType == XSECURE_OBFUSCATED_KEY));

	if ((AesKeyLookupTbl[DecKeySrc].KeyDecSrcAllowed != TRUE) ||
	    (AesKeyLookupTbl[DstKeySrc].KeyDecSrcSelVal ==
				XSECURE_AES_INVALID_CFG)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	XSecure_ReleaseReset(InstancePtr->BaseAddress,
		XSECURE_AES_SOFT_RST_OFFSET);

	/* Configure the SSS for AES. */
	if (InstancePtr->PmcDmaPtr->Config.DeviceId == 0) {
		Status = XSecure_SssAes(&InstancePtr->SssInstance,
			XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	}
	else {
		Status = XSecure_SssAes(&InstancePtr->SssInstance,
			XSECURE_SSS_DMA1, XSECURE_SSS_DMA1);
	}
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}


	if (KeyType == XSECURE_OBFUSCATED_KEY) {
		KeySrc = XSECURE_AES_FAMILY_KEY;
	}
	else if (KeyType == XSECURE_BLACK_KEY) {
		KeySrc = XSECURE_AES_PUF_KEY;
	}
	else {
		Status = XST_FAILURE;
		goto END;
	}

	Status = XSecure_AesKeyLoad(InstancePtr, KeySrc, KeySize);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Start the message. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_START_MSG_OFFSET,
			XSECURE_AES_START_MSG_VAL_MASK);

	/* Enable PMC DMA Src channel for byte swapping.*/
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
			XPMCDMA_SRC_CHANNEL, XSECURE_ENABLE_BYTE_SWAP);

	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_DATA_SWAP_OFFSET, 0x1U);

	/* Push IV into the AES engine. */
	XPmcDma_64BitTransfer(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
		IvAddr, IvAddr >> 32, XSECURE_SECURE_GCM_TAG_SIZE/4U, (u8)1);

	XPmcDma_WaitForDone(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL);


	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, 0x0U);

	/* Acknowledge the transfer has completed */
	XPmcDma_IntrClear(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
				XPMCDMA_IXR_DONE_MASK);

	/* Select key decryption */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_KEY_DEC_OFFSET, XSECURE_AES_KEY_DEC_MASK);

	/* Decrypt selection */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_KEY_DEC_SEL_OFFSET,
		AesKeyLookupTbl[DstKeySrc].KeyDecSrcSelVal);

	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_KEY_SEL_OFFSET,
		AesKeyLookupTbl[DecKeySrc].KeySrcSelVal);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_DEC_TRIG_OFFSET, (u32)0x1);

	/* Wait for AES Decryption completion. */
	Status = XSecure_AesKekWaitForDone(InstancePtr);

	if(Status != (u32)XST_SUCCESS)
	{
		Status = (u32)XST_FAILURE;
		goto END;
	}

END:
	XSecure_SetReset(InstancePtr->BaseAddress,
		XSECURE_AES_SOFT_RST_OFFSET);

	/* Select key decryption */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_DEC_OFFSET, 0U);

	return Status;
}
/*****************************************************************************/
/**
 *
 * @brief
 * This function initializes the AES engine for decryption.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	KeySrc		Key Source for decryption of the data.
 * @param	KeySize		Size of the AES key to be used for decryption.
 *		 - XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *		 - XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 * @param	IvAddr		Address to the buffer holding IV.
 *
 * @return
 *		- XST_SUCCESS on successful init
 *		- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_AesDecryptInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
			XSecure_AesKeySize KeySize, u64 IvAddr)
{
	u32 Status = (u32)XST_FAILURE;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(KeySrc < XSECURE_MAX_KEY_SOURCES);
	Xil_AssertNonvoid((KeySize == XSECURE_AES_KEY_SIZE_128) ||
		(KeySize == XSECURE_AES_KEY_SIZE_256));
	Xil_AssertNonvoid(IvAddr != 0x00U);
	Xil_AssertNonvoid(InstancePtr->AesState != XSECURE_AES_UNINITIALIZED);

	/* Key selected does not allow decryption */
	if (AesKeyLookupTbl[KeySrc].DecAllowed == FALSE) {
		Status = XST_FAILURE;
		goto END;
	}

	/* Configure AES for decryption */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_MODE_OFFSET, 0x0);

	/* Configure the SSS for AES. */
	if (InstancePtr->PmcDmaPtr->Config.DeviceId == 0) {
		Status = XSecure_SssAes(&InstancePtr->SssInstance,
			XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	}
	else {
		Status = XSecure_SssAes(&InstancePtr->SssInstance,
			XSECURE_SSS_DMA1, XSECURE_SSS_DMA1);
	}
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}


	/* Load key for decryption */
	Status = XSecure_AesKeyLoad(InstancePtr, KeySrc, KeySize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_AES_DATA_SWAP_VAL_MASK);


	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
			XPMCDMA_SRC_CHANNEL, XSECURE_ENABLE_BYTE_SWAP);

	/* Start the message. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_START_MSG_OFFSET,
			XSECURE_AES_START_MSG_VAL_MASK);

	/* Push IV */
	XPmcDma_64BitTransfer(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
		IvAddr, IvAddr >> 32,
		XSECURE_SECURE_GCM_TAG_SIZE/XSECURE_WORD_SIZE, 0U);

	XPmcDma_WaitForDone(InstancePtr->PmcDmaPtr,
			XPMCDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XPmcDma_IntrClear(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
				XPMCDMA_IXR_DONE_MASK);

	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
				XPMCDMA_SRC_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);

	/* Update the state */
	InstancePtr->AesState = XSECURE_AES_DECRYPT_INITIALIZED;

	Status = XST_SUCCESS;

END:
	if (Status != (u32)XST_SUCCESS) {
		XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}

	return Status;

}

/*****************************************************************************/
/**
 * @brief
 * This function is used to update the AES engine for decryption with provided
 * data and stores the decrypted data at specified address.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	InDataAddr	Address of the encrypted data which needs to be
 *		decrypted.
 * @param	OutDataAddr	Address of output buffer where the decrypted
 *		to be updated.
 * @param	Size		Size of data to be decrypted in bytes,
 *			whereas number of bytes provided should be multiples of 4.
 * @param	IsLastChunk	If this is the last update of data to be
 *		decrypted, this parameter should be set to TRUE otherwise FALSE.
 *
 * @return
 *		- XST_SUCCESS on successful decryption of the data.
 *		- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_AesDecryptUpdate(XSecure_Aes *InstancePtr, u64 InDataAddr,
		u64 OutDataAddr, u32 Size, u8 IsLastChunk)
{
	u32 Status = (u32)XST_FAILURE;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Size % XSECURE_WORD_SIZE) == 0x00U);
	Xil_AssertNonvoid((IsLastChunk == TRUE) || (IsLastChunk == FALSE));
	Xil_AssertNonvoid(InstancePtr->AesState ==
			XSECURE_AES_DECRYPT_INITIALIZED);

	/* Enable PMC DMA Src and Dst channels for byte swapping.*/
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
				XPMCDMA_SRC_CHANNEL, XSECURE_ENABLE_BYTE_SWAP);

	if ((u32)OutDataAddr != XSECURE_AES_NO_CFG_DST_DMA) {
		XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
				XPMCDMA_DST_CHANNEL, XSECURE_ENABLE_BYTE_SWAP);
	}

	/* Configure the SSS for AES. */
	if (InstancePtr->PmcDmaPtr->Config.DeviceId == 0) {
		Status = XSecure_SssAes(&InstancePtr->SssInstance,
			XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	}
	else {
		Status = XSecure_SssAes(&InstancePtr->SssInstance,
			XSECURE_SSS_DMA1, XSECURE_SSS_DMA1);
	}
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	/* Configure destination */
	if ((u32)OutDataAddr != XSECURE_AES_NO_CFG_DST_DMA) {
		XPmcDma_64BitTransfer(InstancePtr->PmcDmaPtr,
				XPMCDMA_DST_CHANNEL,
				OutDataAddr, OutDataAddr >> 32,
				Size/XSECURE_WORD_SIZE, 0);
	}

	XPmcDma_64BitTransfer(InstancePtr->PmcDmaPtr,
				XPMCDMA_SRC_CHANNEL,
				InDataAddr, InDataAddr >> 32,
				Size/XSECURE_WORD_SIZE, IsLastChunk);

	/* Wait for the SRC DMA completion. */
	XPmcDma_WaitForDone(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XPmcDma_IntrClear(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
					XPMCDMA_IXR_DONE_MASK);

	if ((u32)OutDataAddr != XSECURE_AES_NO_CFG_DST_DMA) {
		/* Wait for the DST DMA completion. */
		XPmcDma_WaitForDone(InstancePtr->PmcDmaPtr, XPMCDMA_DST_CHANNEL);

		/* Acknowledge the transfer has completed */
		XPmcDma_IntrClear(InstancePtr->PmcDmaPtr, XPMCDMA_DST_CHANNEL,
						XPMCDMA_IXR_DONE_MASK);
	}

	/* Clear endianness */
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
				XPMCDMA_SRC_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
					XPMCDMA_DST_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
	Status = (u32)XST_SUCCESS;

END:
	if (Status != (u32)XST_SUCCESS) {
		XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}

	return Status;

}

/*****************************************************************************/
/**
 *
 * @brief
 * This function verifies the GCM tag provided for the data decrypted
 * till the point.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.

 * @param	GcmTagAddr	Address of a buffer which should holds
 *		GCM Tag.
 *
 * @return
 *		- XST_SUCCESS on successful GCM tag verification
 *		- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_AesDecryptFinal(XSecure_Aes *InstancePtr, u64 GcmTagAddr)
{
	u32 Status = (u32)XST_FAILURE;
	u32 NextBlkLen = 0U;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(GcmTagAddr != 0x00U);
	Xil_AssertNonvoid(InstancePtr->AesState ==
			XSECURE_AES_DECRYPT_INITIALIZED);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, 0x1U);

	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
					XPMCDMA_SRC_CHANNEL, XSECURE_ENABLE_BYTE_SWAP);

	/* Configure the SSS for AES. */
	if (InstancePtr->PmcDmaPtr->Config.DeviceId == 0) {
		Status = XSecure_SssAes(&InstancePtr->SssInstance,
				XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	}
	else {
		Status = XSecure_SssAes(&InstancePtr->SssInstance,
				XSECURE_SSS_DMA1, XSECURE_SSS_DMA1);
	}
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	XPmcDma_64BitTransfer(InstancePtr->PmcDmaPtr,
		XPMCDMA_SRC_CHANNEL, GcmTagAddr, GcmTagAddr >> 32,
		XSECURE_SECURE_GCM_TAG_SIZE/XSECURE_WORD_SIZE, 0);

	/* Wait for the Src DMA completion. */
	XPmcDma_WaitForDone(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XPmcDma_IntrClear(InstancePtr->PmcDmaPtr,
			XPMCDMA_SRC_CHANNEL,
			XPMCDMA_IXR_DONE_MASK);

	/* Wait for AES Decryption completion. */
	Status = XSecure_AesWaitForDone(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
					XPMCDMA_SRC_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);

	/* Get the AES status to know if GCM check passed. */
	Status = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_AES_STATUS_OFFSET);
	Status &= XSECURE_AES_STATUS_GCM_TAG_PASS_MASK;
	if (Status != XSECURE_AES_STATUS_GCM_TAG_PASS_MASK) {
		Status = XSECURE_AES_GCM_TAG_MISMATCH;
		goto END;
	}
	else {
		Status = XST_SUCCESS;
	}

	Status = XSecure_AesGetNxtBlkLen(InstancePtr, &NextBlkLen);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
END:

	if ((NextBlkLen == 0U) || (Status != (u32)XST_SUCCESS)) {
		XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, 0x0U);

	return Status;

}

/*****************************************************************************/
/**
 *
 * @brief
 * This function decrypts the  size (length) number of bytes of the passed in
 * InDataAddr (source) buffer and stores the decrypted data in the OutDataAddr
 * (destination) buffer and verifies GcmTagAddr.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	InDataAddr	Address of the encrypted data which needs to be
 *		decrypted.
 * @param	OutDataAddr	Address of output buffer where the decrypted
 *		to be updated.
 * @param	Size		Size of data to be decrypted in bytes,
 *			whereas number of bytes provided should be multiples of 4.
 * @param	GcmTagAddr	Address of a buffer which should contain
 *		GCM Tag.
 *
 * @return
 *		- XST_SUCCESS on successful GCM tag verification
 *		- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_AesDecryptData(XSecure_Aes *InstancePtr, u64 InDataAddr,
			u64 OutDataAddr, u32 Size, u64 GcmTagAddr)
{
	u32 Status = (u32)XST_FAILURE;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Size % XSECURE_WORD_SIZE) == 0x00U);
	Xil_AssertNonvoid(GcmTagAddr != 0x00U);
	Xil_AssertNonvoid(InstancePtr->AesState ==
			XSECURE_AES_DECRYPT_INITIALIZED);

	/* Update AES engine with data */
	Status = XSecure_AesDecryptUpdate(InstancePtr, InDataAddr, OutDataAddr,
						Size, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/* Verify GCM tag */
	Status = XSecure_AesDecryptFinal(InstancePtr, GcmTagAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief
 * This function initializes the AES engine for encryption.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	KeySrc		Key Source for encryption.
 * @param	KeySize		Size of the AES key to be used for encryption.
 *			 - XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *			 - XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 * @param	IvAddr		Address to the buffer holding IV.
 *
 * @return
 *		- XST_SUCCESS on successful init
 *		- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_AesEncryptInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
			XSecure_AesKeySize KeySize, u64 IvAddr)
{

	u32 Status = (u32)XST_FAILURE;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(KeySrc < XSECURE_MAX_KEY_SOURCES);
	Xil_AssertNonvoid((KeySize == XSECURE_AES_KEY_SIZE_128) ||
		(KeySize == XSECURE_AES_KEY_SIZE_256));
	Xil_AssertNonvoid(IvAddr != 0x00U);
	Xil_AssertNonvoid(InstancePtr->AesState != XSECURE_AES_UNINITIALIZED);

	/* Key selected does not allow Encryption */
	if (AesKeyLookupTbl[KeySrc].EncAllowed == FALSE) {
		Status = XST_FAILURE;
		goto END;
	}

	/* Configure AES for Encryption */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_MODE_OFFSET, XSECURE_AES_MODE_ENC_DEC_N_MASK);

	/* Configure the SSS for AES. */
	if (InstancePtr->PmcDmaPtr->Config.DeviceId == 0) {
		Status = XSecure_SssAes(&InstancePtr->SssInstance,
			XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	}
	else {
		Status = XSecure_SssAes(&InstancePtr->SssInstance,
			XSECURE_SSS_DMA1, XSECURE_SSS_DMA1);
	}
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Load key for encryption */
	Status = XSecure_AesKeyLoad(InstancePtr, KeySrc, KeySize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_AES_DATA_SWAP_VAL_MASK);


	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
			XPMCDMA_SRC_CHANNEL, XSECURE_ENABLE_BYTE_SWAP);

	/* Start the message. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_START_MSG_OFFSET,
			XSECURE_AES_START_MSG_VAL_MASK);

	/* Push IV */
	XPmcDma_64BitTransfer(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
		IvAddr, IvAddr >> 32,
		XSECURE_SECURE_GCM_TAG_SIZE/XSECURE_WORD_SIZE, 0U);

	XPmcDma_WaitForDone(InstancePtr->PmcDmaPtr,
			XPMCDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XPmcDma_IntrClear(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
				XPMCDMA_IXR_DONE_MASK);

	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
				XPMCDMA_SRC_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);

	/* Update the state */
	InstancePtr->AesState = XSECURE_AES_ENCRYPT_INITIALIZED;

END:
	if (Status != (u32)XST_SUCCESS) {
		XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function is used to update the AES engine for encryption with provided
 * data and stores the decrypted data at specified address.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	InDataAddr	Address of the encrypted data which needs to be
 *		encrypted.
 * @param	OutDataAddr	Address of output buffer where the encrypted data
 *		to be updated.
 * @param	Size		Size of data to be encrypted in bytes,
 *			whereas number of bytes provided should be multiples of 4.
 * @param	IsLastChunk		If this is the last update of data to be
 *		encrypted, this parameter should be set to TRUE otherwise FALSE.
 *
 * @return
 *		- XST_SUCCESS on successful encryption of the data.
 *		- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_AesEncryptUpdate(XSecure_Aes *InstancePtr, u64 InDataAddr,
			u64 OutDataAddr, u32 Size, u8 IsLastChunk)
{
	u32 Status = (u32)XST_FAILURE;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Size % XSECURE_WORD_SIZE) == 0x00U);
	Xil_AssertNonvoid((IsLastChunk == TRUE) || (IsLastChunk == FALSE));
	Xil_AssertNonvoid(InstancePtr->AesState ==
			XSECURE_AES_ENCRYPT_INITIALIZED);

	/* Enable PMC DMA Src and Dst channels for byte swapping.*/
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
				XPMCDMA_SRC_CHANNEL, XSECURE_ENABLE_BYTE_SWAP);

	if ((u32)OutDataAddr != XSECURE_AES_NO_CFG_DST_DMA) {
		XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
				XPMCDMA_DST_CHANNEL, XSECURE_ENABLE_BYTE_SWAP);
	}

	/* Configure the SSS for AES. */
	if (InstancePtr->PmcDmaPtr->Config.DeviceId == 0) {
		Status = XSecure_SssAes(&InstancePtr->SssInstance,
			XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	}
	else {
		Status = XSecure_SssAes(&InstancePtr->SssInstance,
			XSECURE_SSS_DMA1, XSECURE_SSS_DMA1);
	}
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	/* Configure destination */
	if ((u32)OutDataAddr != XSECURE_AES_NO_CFG_DST_DMA) {
		XPmcDma_64BitTransfer(InstancePtr->PmcDmaPtr,
						XPMCDMA_DST_CHANNEL,
						OutDataAddr, OutDataAddr >> 32U,
						Size/XSECURE_WORD_SIZE, FALSE);
	}

	XPmcDma_64BitTransfer(InstancePtr->PmcDmaPtr,
				XPMCDMA_SRC_CHANNEL,
				InDataAddr, InDataAddr >> 32U,
				Size/XSECURE_WORD_SIZE, IsLastChunk);

	/* Wait for the SRC DMA completion. */
	XPmcDma_WaitForDone(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XPmcDma_IntrClear(InstancePtr->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
					XPMCDMA_IXR_DONE_MASK);

	if ((u32)OutDataAddr != XSECURE_AES_NO_CFG_DST_DMA) {
		/* Wait for the DST DMA completion. */
		XPmcDma_WaitForDone(InstancePtr->PmcDmaPtr,
				XPMCDMA_DST_CHANNEL);

		/* Acknowledge the transfer has completed */
		XPmcDma_IntrClear(InstancePtr->PmcDmaPtr,
				XPMCDMA_DST_CHANNEL,
				XPMCDMA_IXR_DONE_MASK);
	}

	/* Clear endianness */
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
				XPMCDMA_SRC_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);
	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
					XPMCDMA_DST_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);

	Status = (u32)XST_SUCCESS;

END:
	if (Status != (u32)XST_SUCCESS) {
		XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	}

	return Status;

}

/*****************************************************************************/
/**
 *
 * @brief
 * This function updates the GCM tag for the encrypted data.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.

 * @param	GcmTagAddr	Address to the buffer of GCM tag size, where the API
 *		updates GCM tag.
 *
 * @return
 *		- XST_SUCCESS on successful GCM tag updation
 *		- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_AesEncryptFinal(XSecure_Aes *InstancePtr, u64 GcmTagAddr)
{
	u32 Status = (u32)XST_FAILURE;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(GcmTagAddr != 0x00U);
	Xil_AssertNonvoid(InstancePtr->AesState ==
			XSECURE_AES_ENCRYPT_INITIALIZED);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET,
			XSECURE_AES_DATA_SWAP_VAL_MASK);

	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
					XPMCDMA_DST_CHANNEL, XSECURE_ENABLE_BYTE_SWAP);

	/* Configure the SSS for AES. */
	if (InstancePtr->PmcDmaPtr->Config.DeviceId == 0) {
		Status = XSecure_SssAes(&InstancePtr->SssInstance,
				XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	}
	else {
		Status = XSecure_SssAes(&InstancePtr->SssInstance,
				XSECURE_SSS_DMA1, XSECURE_SSS_DMA1);
	}
	if (Status != (u32)XST_SUCCESS) {
			goto END;
	}

	XPmcDma_64BitTransfer(InstancePtr->PmcDmaPtr,
			XPMCDMA_DST_CHANNEL,
			GcmTagAddr, GcmTagAddr >> 32,
			XSECURE_SECURE_GCM_TAG_SIZE/XSECURE_WORD_SIZE, 0);
	/* Wait for the DST DMA completion. */
	XPmcDma_WaitForDone(InstancePtr->PmcDmaPtr, XPMCDMA_DST_CHANNEL);

	/* Acknowledge the transfer has completed */
	XPmcDma_IntrClear(InstancePtr->PmcDmaPtr, XPMCDMA_DST_CHANNEL,
					XPMCDMA_IXR_DONE_MASK);

	/* Wait for AES Decryption completion. */
	Status = XSecure_AesWaitForDone(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSecure_AesPmcDmaCfgEndianness(InstancePtr->PmcDmaPtr,
					XPMCDMA_DST_CHANNEL, XSECURE_DISABLE_BYTE_SWAP);

END:
	XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, 0x0U);

	return Status;

}

/*****************************************************************************/
/**
 *
 * @brief
 * This function encrypts size (length) number of bytes of the passed in
 * InDataAddr (source) buffer and stores the encrypted data along with its
 * associated 16 byte tag in the OutDataAddr (destination) buffer and
 * GcmTagAddr (buffer) respectively.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	InDataAddr	Address of the data which needs to be
 *		encrypted.
 * @param	OutDataAddr	Address of output buffer where the encrypted data
 *		to be updated.
 * @param	Size		Size of data to be encrypted in bytes,
 *			whereas number of bytes provided should be multiples of 4.
 *
 * @return
 *		- XST_SUCCESS on successful encryption.
 *		- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_AesEncryptData(XSecure_Aes *InstancePtr, u64 InDataAddr,
			u64 OutDataAddr, u32 Size, u64 GcmTagAddr)
{
	u32 Status = (u32)XST_FAILURE;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Size % XSECURE_WORD_SIZE) == 0x00U);
	Xil_AssertNonvoid(GcmTagAddr != 0x00U);
	Xil_AssertNonvoid(InstancePtr->AesState ==
			XSECURE_AES_ENCRYPT_INITIALIZED);

	/* Update the data to AES engine */
	Status = XSecure_AesEncryptUpdate(InstancePtr, InDataAddr, OutDataAddr,
					Size, TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/* Call Encrypt final */
	Status = XSecure_AesEncryptFinal(InstancePtr, GcmTagAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function waits for AES engine completes key loading.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 *
 * @return
 *		- XST_SUCCESS if the AES engine completes key loading.
 *		- XST_FAILURE if a timeout has occurred.
 *
 ******************************************************************************/
static u32 XSecure_AesWaitKeyLoad(XSecure_Aes *InstancePtr)
{
	u32 Status = (u32)XST_FAILURE;
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Status = Xil_WaitForEvent(((InstancePtr)->BaseAddress +
					XSECURE_AES_STATUS_OFFSET),
			XSECURE_AES_STATUS_KEY_INIT_DONE_MASK,
			XSECURE_AES_STATUS_KEY_INIT_DONE_MASK,
			XSECURE_AES_TIMEOUT_MAX);

	return Status;

}

/*****************************************************************************/
/**
 * @brief
 * This function sets AES engine to update key and IV during decryption of
 * secure header or footer of encrypted partition.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	Config
 *				- TRUE - to enable KUP and IV update
 *				- FALSE -to disable KUP and IV update
 *
 * @return
 *		- XST_SUCCESS on successful configuration.
 *		- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_AesCfgKupIv(XSecure_Aes *InstancePtr, u32 Config)
{
	u32 Status = (u32)XST_FAILURE;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Config == FALSE) ||
					(Config == TRUE));
	if (Config == XSECURE_AES_DISABLE_KUP_IV_UPDATE) {
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_AES_KUP_WR_OFFSET, XSECURE_AES_DISABLE_KUP_IV_UPDATE);
	}
	else {
		XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KUP_WR_OFFSET,
			(XSECURE_AES_KUP_WR_KEY_SAVE_MASK |
			XSECURE_AES_KUP_WR_IV_SAVE_MASK));
	}

	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function gives the AES next block length after decryption of PDI block
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	Size		Pointer to a 32 bit variable where next block
 *		length will be updated.
 *
 * @return	XST_SUCCESS on successful configuration.
 *
 ******************************************************************************/
u32 XSecure_AesGetNxtBlkLen(XSecure_Aes *InstancePtr, u32 *Size)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Size != NULL);

	*Size = Xil_Htonl(XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_AES_IV_3_OFFSET)) * 4;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief
 * This function configures and loads AES key from selected key source.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	KeySrc		Variable is of type XSecure_AesKeySrc
 *			which mentiones the key source to be loaded into AES engine.
 * @param	KeySize		Size of the key selected.
 *
 * @return
 *		- XST_SUCCESS on successful key load
 *		- Error code on failure.
 *
 ******************************************************************************/
u32 XSecure_AesKeyLoad(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
					   XSecure_AesKeySize KeySize)
{
	u32 Status = (u32)XST_FAILURE;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(KeySrc < XSECURE_MAX_KEY_SOURCES);
	Xil_AssertNonvoid((KeySize == XSECURE_AES_KEY_SIZE_128) ||
			  (KeySize == XSECURE_AES_KEY_SIZE_256));

	/* Load Key Size */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_KEY_SIZE_OFFSET, KeySize);

	/* AES key source selection */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_SEL_OFFSET,
			AesKeyLookupTbl[KeySrc].KeySrcSelVal);

	/* Trig loading of key. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_LOAD_OFFSET,
			XSECURE_AES_KEY_LOAD_VAL_MASK);

	/* Wait for AES key loading.*/
	Status = XSecure_AesWaitKeyLoad(InstancePtr);

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function waits for AES completion.
 *
 * @param	InstancePtr Pointer to the XSecure_Aes instance.
 *
 * @return
 *		- XST_SUCCESS on successful key load
 *		- Error code on failure.
 *
 ******************************************************************************/
u32 XSecure_AesWaitForDone(XSecure_Aes *InstancePtr)
{
	u32 Status = (u32)XST_FAILURE;
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Status = Xil_WaitForEvent(((InstancePtr)->BaseAddress +
								XSECURE_AES_STATUS_OFFSET),
								XSECURE_AES_STATUS_DONE_MASK,
								XSECURE_AES_STATUS_DONE_MASK,
								XSECURE_AES_TIMEOUT_MAX);
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function waits for AES key decryption completion.
 *
 * @param	InstancePtr Pointer to the XSecure_Aes instance.
 *
 * @return
 *		- XST_SUCCESS on success
 *		- Error code on failure.
 *
 ******************************************************************************/
static u32 XSecure_AesKekWaitForDone(XSecure_Aes *InstancePtr)
{

	u32 Status = (u32)XST_FAILURE;
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Status = Xil_WaitForEvent(((InstancePtr)->BaseAddress +
								XSECURE_AES_STATUS_OFFSET),
								XSECURE_AES_STATUS_BLK_KEY_DEC_DONE_MASK,
								XSECURE_AES_STATUS_BLK_KEY_DEC_DONE_MASK,
								XSECURE_AES_TIMEOUT_MAX);
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function zeroizes the selected AES key storage register.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	KeySrc	Select the key source which needs to be zeroized.
 *
 * @return
 *		- XST_SUCCESS when key zeroization is success.
 *		- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_AesKeyZero(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc)
{
	u32 KeyClearVal;
	u32 Status = (u32)XST_FAILURE;
	u32 Mask;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid (KeySrc <= XSECURE_AES_EXPANDED_KEYS);

	if (KeySrc == XSECURE_AES_EXPANDED_KEYS) {
		Mask = XSECURE_AES_KEY_CLEAR_AES_KEY_ZEROIZE_MASK;
	}
	else if (AesKeyLookupTbl[KeySrc].KeyClearVal != 0xFFFFFFFF) {
		Mask = AesKeyLookupTbl[KeySrc].KeyClearVal;
	}
	else {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	KeyClearVal = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_CLEAR_OFFSET);

	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_KEY_CLEAR_OFFSET,
					 (KeyClearVal | Mask));

	Status = Xil_WaitForEvent(((InstancePtr)->BaseAddress +
				XSECURE_AES_KEY_ZEROED_STATUS_OFFSET),
				Mask,
				Mask,
				XSECURE_AES_TIMEOUT_MAX);

	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_KEY_CLEAR_OFFSET,
					 KeyClearVal);

	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_AES_KEY_CLEAR_ERROR;
	}

END:
	return Status;

}

/******************************************************************************/
/**
 *
 * @brief
 * This is a helper function to enable/disable byte swapping feature of PMC DMA
 *
 * @param	InstancePtr 	Pointer to the XPmcDma instance.
 * @param	Channel 	Channel Type - XPMCDMA_SRC_CHANNEL
 *				XPMCDMA_DST_CHANNEL
 * @param	EndianType 	1 : Enable Byte Swapping
 *				0 : Disable Byte Swapping
 *
 * @return	None
 *
 ******************************************************************************/
static void XSecure_AesPmcDmaCfgEndianness(XPmcDma *InstancePtr,
		XPmcDma_Channel Channel,
		u8 EndianType)
{
	XPmcDma_Configure ConfigValues = {0};

	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	XPmcDma_GetConfig(InstancePtr, Channel, &ConfigValues);
	ConfigValues.EndianType = EndianType;
	XPmcDma_SetConfig(InstancePtr, Channel, &ConfigValues);
}

/*****************************************************************************/
/**
 * @brief	This function performs known answer test(KAT) on AES engine.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance
 *
 * @return
 *		- XST_SUCCESS when KAT Pass
 *		- Error code on failure
 *
 *****************************************************************************/
u32 XSecure_AesDecryptKat(XSecure_Aes *AesInstance)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Index;

	u32 Key[8U] = {0xD55455D7U, 0x2B247897U, 0xC4BF1CDU , 0x1A2D14EDU,
		       0x4D3B0A53U, 0xF3C6E1AEU, 0xAFC2447AU, 0x7B534D99U};
	u32 Iv[4U] = {0xCCF8E3B9U, 0x11F11746U, 0xD58C03AFU, 0x00000000U};
	u32 Message[4U] = {0xF9ECC5AEU, 0x92B9B870U, 0x31299331U, 0xC4182756U};
	u32 GcmTag[4U] = {0xC3CFB3E5U, 0x49D4FBCAU, 0xD90B2BFCU, 0xC87DBE9BU};
	u32 Output[4U] = {0x9008CFD4U, 0x3882AA74U, 0xD635531U,  0x6C1C1F47U};
	u32 DstVal[4U];

	/* Write AES key */
	Status = XSecure_AesWriteKey(AesInstance, XSECURE_AES_USER_KEY_0,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)Key);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR;
		goto END;
	}

	Status = XSecure_AesDecryptInit(AesInstance, XSECURE_AES_USER_KEY_0,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)Iv);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_AES_KAT_DECRYPT_INIT_FAILED_ERROR;
		goto END;
	}

	Status =  XSecure_AesDecryptData(AesInstance, (UINTPTR)Message,
			(UINTPTR)DstVal, XSECURE_SECURE_GCM_TAG_SIZE, (UINTPTR)GcmTag);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_AES_KAT_GCM_TAG_MISMATCH_ERROR;
		goto END;
	}

	/* Initialized to error */
	Status = XSECURE_AES_KAT_DATA_MISMATCH_ERROR;
	for (Index = 0U; Index < XSECURE_AES_BUFFER_SIZE; Index++) {
		if (DstVal[Index] != Output[Index]) {
			/* Comparison failure of decrypted data */
			goto END;
		}
	}

	Status = (u32)XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs KAT on AES core with DPACM enabled
 *
 * @param 	AesInstance	InstancePtr	Pointer to the XSecure_Aes instance
 *
 * @param 	KeyPtr	Key Pointer
 *
 * @param 	DataPtr	Data Pointer
 *
 * @param 	OutputPtr	Output where the decrypted data to be stored
 *
 * @return
 *		- Returns the error codes
 *		- Returns XST_SUCCESS on success
 *
 *****************************************************************************/
static u32 XSecure_AesDpaCmDecryptKat(XSecure_Aes *AesInstance, u32 *KeyPtr, u32 *DataPtr, u32 *OutputPtr)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Index;

	/* Configure AES for Encryption */
	XSecure_WriteReg(AesInstance->BaseAddress,
		XSECURE_AES_MODE_OFFSET, XSECURE_AES_MODE_ENC);

	/* Configure AES in split mode */
	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_SPLIT_CFG_OFFSET,
		(XSECURE_AES_SPLIT_CFG_KEY_SPLIT |
		XSECURE_AES_SPLIT_CFG_DATA_SPLIT));

	/* Write Key mask value */
	for (Index = 0U; Index < XSECURE_AES_KEY_SIZE_256BIT_WORDS; Index++) {
		XSecure_WriteReg(AesInstance->BaseAddress,
			XSECURE_AES_KEY_MASK_INDEX + (u8)(Index * (u8)XSECURE_WORD_SIZE),
			0x0U);
	}

	/* Write AES key */
	Status = XSecure_AesWriteKey(AesInstance, XSECURE_AES_USER_KEY_0,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)KeyPtr);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_AESDPACM_KAT_WRITE_KEY_FAILED_ERROR;
		goto END;
	}

	Status = XSecure_AesKeyLoad(AesInstance, XSECURE_AES_USER_KEY_0,
			XSECURE_AES_KEY_SIZE_256);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_AESDPACM_KAT_KEYLOAD_FAILED_ERROR;
		goto END;
	}

	Status = XSecure_SssAes(&AesInstance->SssInstance,
			XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	if(Status != (u32)XST_SUCCESS) {
		Status = XSECURE_AESDPACM_SSS_CFG_FAILED_ERROR;
		goto END;
	}

	/* Start the message. */
	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_START_MSG_OFFSET,
		XSECURE_AES_START_MSG_VAL_MASK);

	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_DATA_SWAP_OFFSET,
		XSECURE_AES_DATA_SWAP_VAL_MASK);

	/* Enable PMC DMA Src channel for byte swapping.*/
	XSecure_AesPmcDmaCfgEndianness(AesInstance->PmcDmaPtr,
		XPMCDMA_SRC_CHANNEL, XSECURE_AES_DATA_SWAP_VAL_MASK);
	/* Enable PMC DMA Dst channel for byte swapping.*/
	XSecure_AesPmcDmaCfgEndianness(AesInstance->PmcDmaPtr,
		XPMCDMA_DST_CHANNEL, XSECURE_AES_DATA_SWAP_VAL_MASK);

	/* Configure the PMC DMA Tx/Rx for the incoming Block. */
	XPmcDma_Transfer(AesInstance->PmcDmaPtr, XPMCDMA_DST_CHANNEL,
		(UINTPTR)OutputPtr, XSECURE_AES_DMA_SIZE, XSECURE_AES_DMA_LAST_WORD_DISABLE);

	XPmcDma_Transfer(AesInstance->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
		(UINTPTR)DataPtr, XSECURE_AES_DMA_SIZE, XSECURE_AES_DMA_LAST_WORD_ENABLE);

	XPmcDma_WaitForDone(AesInstance->PmcDmaPtr, XPMCDMA_DST_CHANNEL);

	Status = XSecure_AesWaitForDone(AesInstance);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_AESDPACM_KAT_FAILED_ERROR;
		goto END;
	}

	XSecure_AesPmcDmaCfgEndianness(AesInstance->PmcDmaPtr,
		XPMCDMA_SRC_CHANNEL, XSECURE_AES_DATA_SWAP_VAL_DISABLE);

	/* Disable PMC DMA Dst channel for byte swapping. */
	XSecure_AesPmcDmaCfgEndianness(AesInstance->PmcDmaPtr,
		XPMCDMA_DST_CHANNEL, XSECURE_AES_DATA_SWAP_VAL_DISABLE);

	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_DATA_SWAP_OFFSET,
		XSECURE_AES_DATA_SWAP_VAL_DISABLE);

END:
	XPmcDma_IntrClear(AesInstance->PmcDmaPtr, XPMCDMA_DST_CHANNEL,
		XPMCDMA_IXR_DONE_MASK);
	/* Acknowledge the transfer has completed */
	XPmcDma_IntrClear(AesInstance->PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
		XPMCDMA_IXR_DONE_MASK);
	/* Configure AES in split mode */
	XSecure_WriteReg(AesInstance->BaseAddress, XSECURE_AES_SPLIT_CFG_OFFSET,
		XSECURE_AES_SPLIT_CFG_DATA_KEY_DISABLE);
	/* AES reset */
	XSecure_SetReset(AesInstance->BaseAddress, XSECURE_AES_SOFT_RST_OFFSET);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs known answer test(KAT) on AES engine
 * to confirm DPA counter measures is working fine.
 *
 * @param 	AesInstance	Pointer to the XSecure_Aes instance
 *
 * @return
 *		- XST_SUCCESS when KAT Pass
 *		- Error code on failure
 *
 *****************************************************************************/
u32 XSecure_AesDecryptCmKat(XSecure_Aes *AesInstance)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Ct0[4U];
	u32 MiC0[4U];
	u32 Ct1[4U];
	u32 MiC1[4U];
	u32 Output0[16U];
	u32 Output1[16U];
	u32 Key0[8U];
	u32 Data0[16U];
	u32 Key1[8U];
	u32 Data1[16U];
	u32 *RM0 = &Output0[0U];
	u32 *R0 = &Output0[4U];
	u32 *Mm0 = &Output0[8U];
	u32 *M0 = &Output0[12U];
	u32 *RM1 = &Output1[0U];
	u32 *R1 = &Output1[4U];
	u32 *Mm1 = &Output1[8U];
	u32 *M1 = &Output1[12U];

	Key0[0] = Xil_Htonl(0x56690798U);
	Key0[1] = Xil_Htonl(0x978C154FU);
	Key0[2] = Xil_Htonl(0xF250BA78U);
	Key0[3] = Xil_Htonl(0xE463765FU);
	Key0[4] = Xil_Htonl(0x2F0CE697U);
	Key0[5] = Xil_Htonl(0x09A4551BU);
	Key0[6] = Xil_Htonl(0xD8CB3ADDU);
	Key0[7] = Xil_Htonl(0xEDA087B6U);

	Data0[0] = 0U;
	Data0[1] = 0U;
	Data0[2] = 0U;
	Data0[3] = 0U;
	Data0[4] = Xil_Htonl(0xCF37C286U);
	Data0[5] = Xil_Htonl(0xC18AD4EAU);
	Data0[6] = Xil_Htonl(0x3D0BA6A0U);
	Data0[7] = 0U;
	Data0[8] = 0U;
	Data0[9] = 0U;
	Data0[10] = 0U;
	Data0[11] = 0U;
	Data0[12] = Xil_Htonl(0x2D328124U);
	Data0[13] = Xil_Htonl(0xA8D58D56U);
	Data0[14] = Xil_Htonl(0xD0775EEDU);
	Data0[15] = Xil_Htonl(0x93DE1A88U);

	Key1[0] = Xil_Htonl(0x8A02A33BU);
	Key1[1] = Xil_Htonl(0xDF87E784U);
	Key1[2] = Xil_Htonl(0x5D7A8AE3U);
	Key1[3] = Xil_Htonl(0xC8727E70U);
	Key1[4] = Xil_Htonl(0x4F4FD08CU);
	Key1[5] = Xil_Htonl(0x1F208328U);
	Key1[6] = Xil_Htonl(0x2D8CB3A5U);
	Key1[7] = Xil_Htonl(0xD3CEDEE9U);

	Data1[0]  = 0U;
	Data1[1]  = 0U;
	Data1[2]  = 0U;
	Data1[3]  = 0U;
	Data1[4]  = Xil_Htonl(0x599F5896U);
	Data1[5]  = Xil_Htonl(0x851C968EU);
	Data1[6]  = Xil_Htonl(0xD808323BU);
	Data1[7]  = 0U;
	Data1[8]  = 0U;
	Data1[9]  = 0U;
	Data1[10] = 0U;
	Data1[11] = 0U;
	Data1[12] = Xil_Htonl(0x4ADE8B32U);
	Data1[13] = Xil_Htonl(0xD56723FBU);
	Data1[14] = Xil_Htonl(0x8F65CE40U);
	Data1[15] = Xil_Htonl(0x825E27C9U);

	/*
	 * In DPA counter measure KAT modify CT and
	 * MiC values with expected output
	 */
	Ct0[0] = Xil_Htonl(0x3B0A0267U);
	Ct0[1] = Xil_Htonl(0xF6ECDE3AU);
	Ct0[2] = Xil_Htonl(0x78B30903U);
	Ct0[3] = Xil_Htonl(0xEBD4CA6EU);

	Ct1[0] = Xil_Htonl(0xCB913379U);
	Ct1[1] = Xil_Htonl(0x6B907565U);
	Ct1[2] = Xil_Htonl(0x7840421AU);
	Ct1[3] = Xil_Htonl(0x46022B63U);

	MiC0[0] = Xil_Htonl(0x1FD20064U);
	MiC0[1] = Xil_Htonl(0x09FC6363U);
	MiC0[2] = Xil_Htonl(0x79F3D406U);
	MiC0[3] = Xil_Htonl(0x7ECA0988U);

	MiC1[0] = Xil_Htonl(0xA79E453CU);
	MiC1[1] = Xil_Htonl(0x6FAD8A5AU);
	MiC1[2] = Xil_Htonl(0x4C2A8E87U);
	MiC1[3] = Xil_Htonl(0x821C7F88U);

	/* Test 1 */
	Status = XSecure_AesDpaCmDecryptKat(AesInstance, Key0, Data0, Output0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Initialize AES driver */
	Status = XSecure_AesInitialize(AesInstance, AesInstance->PmcDmaPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesDpaCmDecryptKat(AesInstance, Key1, Data1, Output1);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (((*(RM0) == 0U) && (*(RM0 + 1U) == 0U) && (*(RM0 + 2U) == 0U) &&
				(*(RM0 + 3U) == 0U)) ||
			((RM0[0U] == RM1[0U]) && (RM0[1U] == RM1[1U]) &&
				(RM0[2U] == RM1[2U]) && (RM0[3U] == RM1[3U])) ||
			((RM0[0U] == Mm0[0U]) && (RM0[1U] == Mm0[1U]) &&
				(RM0[2U] == Mm0[2U]) && (RM0[3U] == Mm0[3U])) ||
			((RM0[0U] == Mm1[0U]) && (RM0[1U] == Mm1[1U]) &&
				(RM0[2U] == Mm1[2U]) && (RM0[3U] == Mm1[3U]))) {
		Status = XSECURE_AESDPACM_KAT_CHECK1_FAILED_ERROR;
		goto END;
	}

	if (((*(RM1) == 0U) && (*(RM1 + 1U) == 0U) && (*(RM1 + 2U) == 0U) &&
				(*(RM1 + 3U) == 0U)) ||
			((RM1[0U] == RM0[0U]) && (RM1[1U] == RM0[1U]) &&
				(RM1[2U] == RM0[2U]) && (RM1[3U] == RM0[3U])) ||
			((RM1[0U] == Mm0[0U]) && (RM1[1U] == Mm0[1U]) &&
				(RM1[2U] == Mm0[2U]) && (RM1[3U] == Mm0[3U])) ||
			((RM1[0U] == Mm1[0U]) && (RM1[1U] == Mm1[1]) &&
				(RM1[2U] == Mm1[2U]) && (RM1[3U] == Mm1[3U]))) {
		Status = XSECURE_AESDPACM_KAT_CHECK2_FAILED_ERROR;
		goto END;
	}

	if (((*(Mm0) == 0U) && (*(Mm0 + 1U) == 0U) && (*(Mm0 + 2U) == 0U) &&
				(*(Mm0 + 3U) == 0U)) ||
			((Mm0[0U] == RM0[0U]) && (Mm0[1U] == RM0[1U]) &&
				(Mm0[2U] == RM0[2U]) && (Mm0[3U] == RM0[3U])) ||
			((Mm0[0U] == RM1[0U]) && (Mm0[1U] == RM1[1U]) &&
				(Mm0[2U] == RM1[2U]) && (Mm0[3U] == RM1[3U])) ||
			((Mm0[0U] == Mm1[0U]) && (Mm0[1U] == Mm1[1U]) &&
				(Mm0[2U] == Mm1[2U]) && (Mm0[3U] == Mm1[3U]))) {
		Status = XSECURE_AESDPACM_KAT_CHECK3_FAILED_ERROR;
		goto END;
	}

	if (((*(Mm1) == 0U) && (*(Mm1 + 1U) == 0U) && (*(Mm1 + 2U) == 0U) &&
				(*(Mm1 + 3U) == 0U)) ||
			((Mm1[0U] == RM0[0U]) && (Mm1[1U] == RM0[1U]) &&
				(Mm1[2U] == RM0[2U]) && (Mm1[3U] == RM0[3U])) ||
			((Mm1[0U] == RM1[0U]) && (Mm1[1U] == RM1[1U]) &&
				(Mm1[2U] == RM1[2U]) && (Mm1[3U] == RM1[3U])) ||
			((Mm1[0U] == Mm0[0U]) && (Mm1[1U] == Mm0[1U]) &&
				(Mm1[2U] == Mm0[2U]) && (Mm1[3U] == Mm0[3U]))) {
		Status = XSECURE_AESDPACM_KAT_CHECK4_FAILED_ERROR;
		goto END;
	}

	if ((((R0[0U] ^ RM0[0U]) != Ct0[0U])  || ((R0[1U] ^ RM0[1U]) != Ct0[1U]) ||
		 ((R0[2U] ^ RM0[2U]) != Ct0[2U])  || ((R0[3U] ^ RM0[3U]) != Ct0[3U])) ||
		(((M0[0U] ^ Mm0[0U]) != MiC0[0U]) || ((M0[1U] ^ Mm0[1U]) != MiC0[1U]) ||
		 ((M0[2U] ^ Mm0[2U]) != MiC0[2U]) || ((M0[3U] ^ Mm0[3U]) != MiC0[3U])) ||
		(((R1[0U] ^ RM1[0U]) != Ct1[0U])  || ((R1[1U] ^ RM1[1U]) != Ct1[1U]) ||
		((R1[2U] ^ RM1[2U]) != Ct1[2U])  || ((R1[3U] ^ RM1[3U]) != Ct1[3U])) ||
		(((M1[0U] ^ Mm1[0U]) != MiC1[0U]) || ((M1[1U] ^ Mm1[1U]) != MiC1[1U]) ||
		 ((M1[2U] ^ Mm1[2U]) != MiC1[2U]) || ((M1[3U] ^ Mm1[3U]) != MiC1[3U]))) {
		Status = XSECURE_AESDPACM_KAT_CHECK5_FAILED_ERROR;
		goto END;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}
