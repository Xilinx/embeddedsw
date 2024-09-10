/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xaes.c
 * @addtogroup Overview
 * @{
 * This file contains low level implementation of AES hardware interface APIs
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   06/26/24 Initial release
 *       am   08/01/24 Replaced variables of type enums with u32 type.
 *       am   08/24/24 Added AES DPA CM KAT support
 *       yog  08/25/24 Integrated FIH library
 *       am   08/25/24 Initialized ASU DMA pointer before XAsufw_DmaXfr() function call.
 *
 * </pre>
 *
 *************************************************************************************************/

/***************************** Include Files *****************************************************/
#include "xaes.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_config.h"
#include "xfih.h"

/************************** Constant Definitions *************************************************/

/************************** Macros Definitions ***************************************************/
#define XAES_PUF_KEY			(0xAU)			/**< PUF key */
#define XAES_EXPANDED_KEYS		(0xBU)			/**< Expanded keys in AES engine */
#define XAES_MAX_KEY_SOURCES		XAES_EXPANDED_KEYS	/**< Maximum key source value */
#define XAES_TIMEOUT_MAX		(0x1FFFFU)		/**<  AES maximum timeout value */
#define XAES_INVALID_CFG		(0xFFFFFFFFU)		/**<  AES invalid configuration */

/* AES state */
#define XAES_INITIALIZED		(0x1U)			/**< AES in initialized state */
#define XAES_STARTED			(0x2U)			/**< AES in start state */
#define XAES_UPDATED			(0x3U)			/**< AES in update state */

/**************************** Type Definitions ***************************************************/
/**
 * @name AES key look up table
 * @{
 */
/**< This structure defines look up table for AES key */
typedef struct {
	u32 RegOffset;			/**< Register offset for key source */
	u32 KeySrcSelVal;		/**< Selection value for key source */
	u32 UsrWrAllowed;		/**< User write allowed or not for key source */
	u32 KeyDecSrcAllowed;		/**< Key decryption source allowed */
	u32 KeyDecSrcSelVal;		/**< Selection value for key decryption source*/
	u32 KeyClearVal;		/**< Key source clear value*/
	u32 KeyZeroedStatusMask;	/**< Key zeroed status mask */
} XAes_KeyLookup;
/** @} */

/**
 * @name AES configuration
 * @{
 */
/**< This structure contains configuration information for AES core */
struct _XAes_Config {
	u16 DeviceId;			/**< Unique ID of the device */
	u32 AesBaseAddress;		/**< Base address of ASU_AES module */
	u32 KeyBaseAddress;		/**< Base address of ASU_KEY module */
};
/** @} */

/************************** Variable Definitions *************************************************/
/**
 * @name AES key look up table array
 * @{
 */
/**< This array of structure defines look up table for AES key */
static const XAes_KeyLookup AesKeyLookupTbl [XAES_MAX_KEY_SOURCES] = {
	/* EFUSE_KEY_RED_0 */
	{
		XAES_INVALID_CFG,
		XAES_KEY_SEL_EFUSE_KEY_RED_0_VALUE,
		FALSE,
		TRUE,
		XAES_KEY_TO_BE_DEC_SEL_EFUSE_KEY_0_VALUE,
		XAES_KEY_CLEAR_EFUSE_KEY_RED_0_MASK,
		XAES_KEY_ZEROED_STATUS_EFUSE_RED_KEY_0_MASK
	},

	/* EFUSE_KEY_RED_1 */
	{
		XAES_INVALID_CFG,
		XAES_KEY_SEL_EFUSE_KEY_RED_1_VALUE,
		FALSE,
		TRUE,
		XAES_KEY_TO_BE_DEC_SEL_EFUSE_KEY_1_VALUE,
		XAES_KEY_CLEAR_EFUSE_KEY_RED_1_MASK,
		XAES_KEY_ZEROED_STATUS_EFUSE_RED_KEY_1_MASK
	},

	/* USER_KEY_0 */
	{
		XAES_USER_KEY_0_0_OFFSET,
		XAES_KEY_SEL_USER_KEY_0_VALUE,
		TRUE,
		FALSE,
		XAES_INVALID_CFG,
		XAES_KEY_CLEAR_USER_KEY_0_MASK,
		XAES_KEY_ZEROED_STATUS_USER_KEY_0_MASK
	},

	/* USER_KEY_1 */
	{
		XAES_USER_KEY_1_0_OFFSET,
		XAES_KEY_SEL_USER_KEY_1_VALUE,
		TRUE,
		FALSE,
		XAES_INVALID_CFG,
		XAES_KEY_CLEAR_USER_KEY_1_MASK,
		XAES_KEY_ZEROED_STATUS_USER_KEY_1_MASK
	},

	/* USER_KEY_2 */
	{
		XAES_USER_KEY_2_0_OFFSET,
		XAES_KEY_SEL_USER_KEY_2_VALUE,
		TRUE,
		FALSE,
		XAES_INVALID_CFG,
		XAES_KEY_CLEAR_USER_KEY_2_MASK,
		XAES_KEY_ZEROED_STATUS_USER_KEY_2_MASK
	},

	/* USER_KEY_3 */
	{
		XAES_USER_KEY_3_0_OFFSET,
		XAES_KEY_SEL_USER_KEY_3_VALUE,
		TRUE,
		FALSE,
		XAES_INVALID_CFG,
		XAES_KEY_CLEAR_USER_KEY_3_MASK,
		XAES_KEY_ZEROED_STATUS_USER_KEY_3_MASK
	},

	/* USER_KEY_4 */
	{
		XAES_USER_KEY_4_0_OFFSET,
		XAES_KEY_SEL_USER_KEY_4_VALUE,
		TRUE,
		FALSE,
		XAES_INVALID_CFG,
		XAES_KEY_CLEAR_USER_KEY_4_MASK,
		XAES_KEY_ZEROED_STATUS_USER_KEY_4_MASK
	},

	/* USER_KEY_5 */
	{
		XAES_USER_KEY_5_0_OFFSET,
		XAES_KEY_SEL_USER_KEY_5_VALUE,
		TRUE,
		FALSE,
		XAES_INVALID_CFG,
		XAES_KEY_CLEAR_USER_KEY_5_MASK,
		XAES_KEY_ZEROED_STATUS_USER_KEY_5_MASK
	},

	/* USER_KEY_6 */
	{
		XAES_USER_KEY_6_0_OFFSET,
		XAES_KEY_SEL_USER_KEY_6_VALUE,
		TRUE,
		FALSE,
		XAES_INVALID_CFG,
		XAES_KEY_CLEAR_USER_KEY_6_MASK,
		XAES_KEY_ZEROED_STATUS_USER_KEY_6_MASK
	},

	/* USER_KEY_7 */
	{
		XAES_USER_KEY_7_0_OFFSET,
		XAES_KEY_SEL_USER_KEY_7_VALUE,
		TRUE,
		FALSE,
		XAES_INVALID_CFG,
		XAES_KEY_CLEAR_USER_KEY_7_MASK,
		XAES_KEY_ZEROED_STATUS_USER_KEY_7_MASK
	},

	/* PUF_KEY */
	{
		XAES_INVALID_CFG,
		XAES_KEY_SEL_PUF_KEY_VALUE,
		FALSE,
		FALSE,
		XAES_INVALID_CFG,
		XAES_KEY_CLEAR_PUF_KEY_MASK,
		XAES_KEY_ZEROED_STATUS_PUF_KEY_MASK
	},
};
/** @} */

/**
 * @name AES configuration table
 * @{
 */
/**< The configuration table for devices */
static XAes_Config AesConfigTable[XASU_XAES_NUM_INSTANCES] = {
	{
		XASU_XAES_0_DEVICE_ID,
		XASU_XAES_0_BASEADDR,
		XASU_XKEY_0_BASEADDR
	}
};
/** @} */

/**
 * @name AES Instance
 * @{
 */
/**< This structure contains AES instance information */
struct _XAes {
	u32 AesBaseAddress;	/**< AES Base address */
	u32 KeyBaseAddress;	/**< Key Vault Base address */
	u16 DeviceId;		/**< DeviceId is the unique ID of the device */
	u16 AesCmConfig;	/**< AES counter Measure configuration */
	XAsufw_Dma *AsuDmaPtr;	/**< PMCDMA Instance Pointer */
	u8 OperationType;	/**< AES operation type (Encryption/Decryption) */
	u8 AesState;		/**< Aes internal State machine */
	u8 EngineMode;		/**< Aes Engine mode*/
	u8 Reserved;		/**< Reserved */
};
/** @} */

static XAes XAes_Instance[XASU_XAES_NUM_INSTANCES]; /**< ASUFW AES HW instances */

/************************** Inline Function Definitions ******************************************/

/*************************************************************************************************/
/**
 * @brief	This function validates IV for given AES engine mode.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	IvAddr		Address of buffer holding IV.
 * @param	IvLen		Lenght of the IV in bytes.
 *
 * @return
 *		- Upon successful validation of IV, it returns XASUFW_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
static inline s32 XAes_ValidateIv(XAes *InstancePtr, u64 IvAddr, u32 IvLen)
{
	s32 Status = XASUFW_AES_INVALID_IV;

	/*
	 * IV Validation for respective AES engine modes
	 * AES Standard mode (ECB, CBC, CTR, CFB, OFB).
	 * AES MAC mode (GCM, CCM, GMAC, CMAC).
	 *
	 * |   Engine Mode     |   IvAddress    |   IvLength           |
	 * |-------------------|----------------|----------------------|
	 * | AES-ECB, AES-CMAC |     N/A        |      N/A             |
	 * | AES-GCM           |   Non-zero     |  Any non-zero Length |
	 * | Remaining modes   |   Non-zero     |  12 or 16 Bytes      |
	 */
	switch (InstancePtr->EngineMode) {
		case XASU_AES_ECB_MODE:
		case XASU_AES_CMAC_MODE:
			if ((IvAddr == 0U) && (IvLen == 0U)) {
				Status = XASUFW_SUCCESS;
			}
			break;
		case XASU_AES_GCM_MODE:
			if ((IvAddr != 0U) && (IvLen != 0U)) {
				Status = XASUFW_SUCCESS;
			}
			break;
		case XASU_AES_CBC_MODE:
		case XASU_AES_CFB_MODE:
		case XASU_AES_OFB_MODE:
		case XASU_AES_CTR_MODE:
			if ((IvAddr != 0U) && ((IvLen == XASU_AES_IV_SIZE_96BIT_IN_BYTES) ||
					       (IvLen == XASU_AES_IV_SIZE_128BIT_IN_BYTES))) {
				Status = XASUFW_SUCCESS;
			}
			break;
		default:
			Status = XASUFW_AES_INVALID_ENGINE_MODE;
			break;
	}
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates Tag for given AES engine mode.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	TagAddr		Address of the Input/Output Tag.
 * @param	TagLen		Length of Tag in bytes and it will be zero for all AES
 *				standard modes like, ECB, CBC, OFB, CFB, CTR
 *
 * @return
 *		- Upon successful validation of Tag, it returns XASUFW_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
static inline s32 XAes_ValidateTag(XAes *InstancePtr, u64 TagAddr, u32 TagLen)
{
	s32 Status = XASUFW_AES_INVALID_TAG;

	/*
	 * Tag Validation for respective AES engine modes
	 * AES Standard mode (ECB, CBC, CTR, CFB, OFB).
	 * AES MAC mode (GCM, CCM, GMAC, CMAC).
	 *
	 * |   Engine Mode       |   TagAddress   |   TagLength          |
	 * |---------------------|----------------|----------------------|
	 * | Standard mode       |     N/A        |      N/A             |
	 * | AES-GCM, CMAC       |   Non-zero     |  8<=TagLen<=16       |
	 * | AES-CCM             |   Non-zero     |  4,6,8,10,12,14,16   |
	 *
	 * NIST recommends using a tag length of atleast 64 bits to provide adequate protection
	 * against guessing attacks.
	 */
	switch (InstancePtr->EngineMode) {
		case XASU_AES_CBC_MODE:
		case XASU_AES_CFB_MODE:
		case XASU_AES_OFB_MODE:
		case XASU_AES_CTR_MODE:
		case XASU_AES_ECB_MODE:
			if ((TagAddr == 0U) && (TagLen == 0U)) {
				Status = XASUFW_SUCCESS;
			}
			break;
		case XASU_AES_CCM_MODE:
			if ((TagAddr != 0U) && (((TagLen % XASUFW_EVEN_MODULUS) == 0U) &&
						(TagLen >= XASU_AES_RECOMMENDED_TAG_LENGTH_IN_BYTES) &&
						(TagLen <= XASU_AES_MAX_TAG_LENGTH_IN_BYTES))) {
				Status = XASUFW_SUCCESS;
			}
			break;
		case XASU_AES_GCM_MODE:
		case XASU_AES_CMAC_MODE:
			if ((TagAddr != 0U) && ((TagLen >= XASU_AES_RECOMMENDED_TAG_LENGTH_IN_BYTES) &&
						(TagLen <= XASU_AES_MAX_TAG_LENGTH_IN_BYTES))) {
				Status = XASUFW_SUCCESS;
			}
			break;
		default:
			Status = XASUFW_AES_INVALID_ENGINE_MODE;
			break;
	}
	return Status;
}

/************************** Function Prototypes **************************************************/
static XAes_Config *XAes_LookupConfig(u16 DeviceId);
static s32 XAes_CheckKeyZeroedStatus(XAes *InstancePtr, u32 KeySrc);
static void XAes_ConfigCounterMeasures(XAes *InstancePtr);
static void XAes_ConfigAesOperation(XAes *InstancePtr);
static void XAes_LoadKey(XAes *InstancePtr, u32 KeySrc, u32 KeySize);
static s32 XAes_ProcessAndLoadIv(XAes *InstancePtr, u64 IvAddr, u32 IvLen);
static s32 XAes_GHashCal(XAes *InstancePtr, u64 IvAddr, u32 IvGen, u32 IvLen);
static s32 XAes_ReadTag(XAes *InstancePtr, u32 TagOutAddr, u32 TagLen);
static s32 XAes_ReadNVerifyTag(XAes *InstancePtr, u32 TagInAddr, u32 TagLen);
static s32 XAes_ProcessTag(XAes *InstancePtr, u64 TagAddr, u32 TagLen);
static void XAes_ConfigAad(XAes *InstancePtr);
static void XAes_ClearConfigAad(XAes *InstancePtr);
static s32 XAes_CfgDmaWithAesAndXfer(XAes *InstancePtr, u64 InDataAddr, u64 OutDataAddr, u32 Size,
				     u8 IsLastChunk);
static s32 XAes_WaitForDone(XAes *InstancePtr);
static void XAes_SetReset(XAes *InstancePtr);

/*************************************************************************************************/
/**
 * @brief	This function returns pointer to the Aes driver based on device Id.
 *
 * @param	DeviceId	Unique device ID of the device for the lookup operation.
 *
 * @return
 *		- Pointer to the XAes instance corresponding to the Device ID.
 *		- It returns NULL, if Device ID is invalid.
 *
 *************************************************************************************************/
XAes *XAes_GetInstance(u16 DeviceId)
{
	XAes *XAes_InstancePtr = NULL;

	if (DeviceId >= XASU_XAES_NUM_INSTANCES) {
		XFIH_GOTO(END);
	}

	XAes_InstancePtr = &XAes_Instance[DeviceId];
	XAes_InstancePtr->DeviceId = DeviceId;

END:
	return XAes_InstancePtr;
}

/*************************************************************************************************/
/**
 * @brief	This function configures and initializes AES instance. This function must be
 *		called prior to using a AES core. Initialization of AES includes setting up the
 *		instance data and ensuring the hardware is in a quiescent state.
 *
 * @param	InstancePtr 	Pointer to the XAes instance.
 *
 * @return
 *		- Upon successful initialization of AES instance, it returns XASUFW_SUCCESS.
 *		- Otherwise, it returns an error code.
 *
 *************************************************************************************************/
s32 XAes_CfgInitialize(XAes *InstancePtr)
{
	s32 Status = XASUFW_FAILURE;
	XAes_Config *CfgPtr;

	/* Validate input parameters */
	if (InstancePtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	CfgPtr = XAes_LookupConfig(InstancePtr->DeviceId);
	if (CfgPtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/* Initialize the instance */
	InstancePtr->AesBaseAddress = CfgPtr->AesBaseAddress;
	InstancePtr->KeyBaseAddress = CfgPtr->KeyBaseAddress;
	InstancePtr->AesCmConfig = XASUFW_AES_CM_CONFIG;
	InstancePtr->AesState = XAES_INITIALIZED;

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function writes the provided user key into specified AES USER key registers.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	KeyObjectAddr	Address of Aes key object structure, which contains-
 *				 Key pointer to Key address which contains the key to be written.
 *				 Key size is the size of the input key to be written.
 *			  	  - XASUFW_AES_KEY_SIZE_128 for 128 bit key size
 *				  - XASUFW_AES_KEY_SIZE_256 for 256 bit key size
 *				 Key Source to be selected to which provided key to be written.
 *			  	  - XASU_AES_USER_KEY_0
 *				  - XASU_AES_USER_KEY_1
 *				  - XASU_AES_USER_KEY_2
 *				  - XASU_AES_USER_KEY_3
 *				  - XASU_AES_USER_KEY_4
 *				  - XASU_AES_USER_KEY_5
 *				  - XASU_AES_USER_KEY_6
 *				  - XASU_AES_USER_KEY_7
 *
 * @return
 *		- Upon successful Key write to AES USER key register, it returns XASUFW_SUCCESS.
 *		- Otherwise, it returns an error code.
 *
 *************************************************************************************************/
//TODO: In future, this API can be moved/modified after Key vault implementation.
s32 XAes_WriteKey(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 KeyObjectAddr)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_AES_GLITCH_ERROR);
	s32 Sstatus = XFih_VolatileAssign((s32)XASUFW_AES_INVALID_IV);
	s32 ClearStatus = XFih_VolatileAssign((s32)XASUFW_AES_INVALID_IV);
	s32 ClearStatusTmp = XFih_VolatileAssign((s32)XASUFW_AES_INVALID_IV);
	XFih_Var XFihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);
	u32 Key[XASU_AES_KEY_SIZE_256BIT_IN_WORDS];
	u32 Offset;
	u32 Index;
	u32 KeySizeInWords;
	XAsu_AesKeyObject KeyObject;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if ((DmaPtr == NULL) || (DmaPtr->AsuDma.IsReady != XIL_COMPONENT_IS_READY)) {
		Status = XASUFW_AES_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if (KeyObjectAddr == 0U) {
		Status = XASUFW_AES_INVALID_KEY_OBJECT_ADDRESS;
		XFIH_GOTO(END);
	}

	/* Initialize the AES ASU Instance DMA pointer */
	InstancePtr->AsuDmaPtr = DmaPtr;

	/* Copy KeyObject strucure from 64-bit address space to local structure using SSS DMA loopback */
	Status = XAsufw_DmaXfr(InstancePtr->AsuDmaPtr, KeyObjectAddr,
			       (u64)(UINTPTR)&KeyObject, sizeof(XAsu_AesKeyObject), 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END_CLR);
	}

	if ((KeyObject.KeySrc < XASU_AES_USER_KEY_0) ||
	    (KeyObject.KeySrc > XASU_AES_USER_KEY_7)) {
		Status = XASUFW_AES_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if (AesKeyLookupTbl[KeyObject.KeySrc].UsrWrAllowed != TRUE) {
		Status = XASUFW_AES_INVALID_KEY_SRC;
		XFIH_GOTO(END);
	}

	if ((KeyObject.KeySize != XASU_AES_KEY_SIZE_128_BITS) &&
	    (KeyObject.KeySize != XASU_AES_KEY_SIZE_256_BITS)) {
		Status = XASUFW_AES_INVALID_KEY_SIZE;
		XFIH_GOTO(END);
	}

	Offset = AesKeyLookupTbl[KeyObject.KeySrc].RegOffset;
	if (Offset == XAES_INVALID_CFG) {
		Status = XASUFW_AES_INVALID_KEY_SRC;
		XFIH_GOTO(END);
	}

	if (KeyObject.KeySize == XASU_AES_KEY_SIZE_128_BITS) {
		KeySizeInWords = XASU_AES_KEY_SIZE_128BIT_IN_WORDS;
	} else {
		KeySizeInWords = XASU_AES_KEY_SIZE_256BIT_IN_WORDS;
	}

	/* Copy Key from 64-bit address space to local array using SSS DMA loopback */
	XFIH_CALL_GOTO(XAsufw_DmaXfr, XFihVar, Status, END_CLR, InstancePtr->AsuDmaPtr,
		       (u64)KeyObject.KeyAddress, (u64)(UINTPTR)Key,
		       (KeySizeInWords * XASUFW_WORD_LEN_IN_BYTES), 0U);

	/*
	 * Traverse to Offset of last word of respective USER key register
	 * (i.e., XAES_USER_KEY_X_7), where X = 0 to 7 USER key.
	 */
	Offset = Offset + (KeySizeInWords * XASUFW_WORD_LEN_IN_BYTES) - XASUFW_WORD_LEN_IN_BYTES;

	for (Index = 0U; Index < KeySizeInWords; Index++) {
		XAsufw_WriteReg((InstancePtr->KeyBaseAddress + Offset), Xil_Htonl(Key[Index]));
		Offset = Offset - XASUFW_WORD_LEN_IN_BYTES;
	}

END_CLR:
	Sstatus = Xil_SMemSet(&KeyObject, sizeof(XAsu_AesKeyObject), 0U, sizeof(XAsu_AesKeyObject));
	ClearStatus = Xil_SecureZeroize((u8 *)Key, XASU_AES_KEY_SIZE_256BIT_IN_BYTES);
	ClearStatusTmp = Xil_SecureZeroize((u8 *)Key, XASU_AES_KEY_SIZE_256BIT_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = (ClearStatus | ClearStatusTmp | Sstatus);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function initializes the AES engine for encrypt/decrypt operation and loads
 *		the provided key and IV to the AES engine.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	DmaPtr		ASU DMA pointer allocated for AES operation.
 * @param	KeyObjectAddr	Address of Aes key object structure, which contains-
 *				 Key pointer to Key address which contains the key to be written.
 *				 Key size is the size of the input key to be written.
 *			  	  - XASUFW_AES_KEY_SIZE_128 for 128 bit key size
 *				  - XASUFW_AES_KEY_SIZE_256 for 256 bit key size
 *				 Key Source to be selected to which provided key to be written.
 *			  	  - XASU_AES_USER_KEY_0
 *				  - XASU_AES_USER_KEY_1
 *				  - XASU_AES_USER_KEY_2
 *				  - XASU_AES_USER_KEY_3
 *				  - XASU_AES_USER_KEY_4
 *				  - XASU_AES_USER_KEY_5
 *				  - XASU_AES_USER_KEY_6
 *				  - XASU_AES_USER_KEY_7
 * @param	IvAddr		Address of buffer holding IV.
 * @param	IvLen		Lenght of the IV in bytes.
 * @param	EngineMode	AES engine mode.
 * @param	OperationType	AES encrypt/decrypt operation type.
 *
 * @return
 *		- Upon successful Initialization of AES engine, it returns XASUFW_SUCCESS.
 *		- Otherwise, it returns an error code.
 *
 *************************************************************************************************/
s32 XAes_Init(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 KeyObjectAddr, u64 IvAddr, u32 IvLen,
	      u8 EngineMode, u8 OperationType)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	s32 Sstatus = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	XAsu_AesKeyObject KeyObject;;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if ((DmaPtr == NULL) || (DmaPtr->AsuDma.IsReady != XIL_COMPONENT_IS_READY)) {
		Status = XASUFW_AES_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if ((InstancePtr->AesState != XAES_INITIALIZED)) {
		Status = XASUFW_AES_STATE_MISMATCH_ERROR;
		XFIH_GOTO(END);
	}

	if (KeyObjectAddr == 0U) {
		Status = XASUFW_AES_INVALID_KEY_OBJECT_ADDRESS;
		XFIH_GOTO(END);
	}

	/* Initialize the AES ASU Instance DMA pointer */
	InstancePtr->AsuDmaPtr = DmaPtr;

	/* Copy KeyObject strucure from 64-bit address space to local structure using SSS DMA loopback */
	Status = XAsufw_DmaXfr(InstancePtr->AsuDmaPtr, KeyObjectAddr,
			       (u64)(UINTPTR)&KeyObject, sizeof(XAsu_AesKeyObject), 0U);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	if ((KeyObject.KeySrc >= XAES_MAX_KEY_SOURCES) ||
	    (KeyObject.KeySrc < XASU_AES_EFUSE_KEY_RED_0)) {
		Status = XASUFW_AES_INVALID_KEY_SRC;
		XFIH_GOTO(END);
	}

	if ((KeyObject.KeySize != XASU_AES_KEY_SIZE_128_BITS) &&
	    (KeyObject.KeySize != XASU_AES_KEY_SIZE_256_BITS)) {
		Status = XASUFW_AES_INVALID_KEY_SIZE;
		XFIH_GOTO(END);
	}

	if ((EngineMode < XASU_AES_CBC_MODE) || ((EngineMode > XASU_AES_GCM_MODE) &&
		(EngineMode != XASU_AES_CMAC_MODE) && (EngineMode != XASU_AES_GHASH_MODE))) {
		Status = XASUFW_AES_INVALID_ENGINE_MODE;
		XFIH_GOTO(END);
	}

	/* Validate the IV with respect to the user provided engine mode */
	Status = XAes_ValidateIv(InstancePtr, IvAddr, IvLen);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	if ((OperationType != XASU_AES_ENCRYPT_OPERATION) &&
	    (OperationType != XASU_AES_DECRYPT_OPERATION)) {
		Status = XASUFW_AES_INVALID_OPERATION_TYPE;
		XFIH_GOTO(END);
	}

	/* Check whether key is zeroed or not */
	Status = XAes_CheckKeyZeroedStatus(InstancePtr, KeyObject.KeySrc);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* Initialize the AES instance */
	InstancePtr->EngineMode = EngineMode;
	InstancePtr->OperationType = OperationType;

	/* Release soft reset of AES engine */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->AesBaseAddress, XAES_SOFT_RST_OFFSET);

	/* Configure AES DPA counter measures */
	XAes_ConfigCounterMeasures(InstancePtr);

	/* Configure AES engine for encrypt/decrypt operation */
	XAes_ConfigAesOperation(InstancePtr);

	/* Load key to AES engine */
	XAes_LoadKey(InstancePtr, KeyObject.KeySrc, KeyObject.KeySize);

	/* Process and load Iv to AES engine */
	if (InstancePtr->EngineMode != XASU_AES_ECB_MODE) {
		Status = XAes_ProcessAndLoadIv(InstancePtr, IvAddr, IvLen);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
	}

	/* Update AES state machine to STARTED */
	InstancePtr->AesState = XAES_STARTED;

END:
	Sstatus = Xil_SMemSet(&KeyObject, sizeof(XAsu_AesKeyObject), 0U, sizeof(XAsu_AesKeyObject));
	if ((InstancePtr != NULL) && (Status != XASUFW_SUCCESS)) {
		/* Set AES under reset on failure */
		XAes_SetReset(InstancePtr);
	}
	if (Status == XASUFW_SUCCESS) {
		Status = Sstatus;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function updates the provided data to AES engine for encrypt/decrypt operation
 * 		and stores the output data at specified output address.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	DmaPtr		ASU DMA pointer allocated for AES operation.
 * @param	InDataAddr	Address of the input data for encrypt/decrypt operation.
 * @param	OutDataAddr	Address of output buffer where the encrypted/decrypted data to be
 *				updated and if input data is AAD then, OutDataAddr will be zero.
 * @param	DataLength	Length of both input/output data in bytes.
 * @param	IsLastChunk	On last update of data to AES engine, this parameter should be set
 *		 	 	to TRUE otherwise, FALSE.
 *
 * @return
 *		- Upon successful update of data to AES engine, it returns XASUFW_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
s32 XAes_Update(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 InDataAddr, u64 OutDataAddr,
		u32 DataLength, u8 IsLastChunk)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if ((DmaPtr == NULL) || (DmaPtr->AsuDma.IsReady != XIL_COMPONENT_IS_READY)) {
		Status = XASUFW_AES_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if ((InstancePtr->AesState != XAES_STARTED) &&
	    (InstancePtr->AesState != XAES_UPDATED)) {
		Status = XASUFW_AES_STATE_MISMATCH_ERROR;
		XFIH_GOTO(END);
	}

	if (InDataAddr == 0U) {
		Status = XASUFW_AES_INVALID_INPUT_DATA;
		XFIH_GOTO(END);
	}

	/* Minimum Length of Plaintext/Aad should be of atleast 8bits */
	if (DataLength == 0U) {
		Status = XASUFW_AES_INVALID_INPUT_DATA_LENGTH;
		XFIH_GOTO(END);
	}

	if ((IsLastChunk != TRUE) && (IsLastChunk != FALSE)) {
		Status = XASUFW_AES_INVALID_ISLAST_CHUNK;
		XFIH_GOTO(END);
	}

	/* For AES-CBC and AES-ECB modes, user should always send 16 Bytes aligned payload data */
	if (((InstancePtr->EngineMode == XASU_AES_CBC_MODE) ||
	     (InstancePtr->EngineMode == XASU_AES_ECB_MODE)) &&
	    ((DataLength % XASU_AES_BLOCK_SIZE_IN_BYTES) != 0U)) {
		Status = XASUFW_AES_UNALIGNED_BLOCK_SIZE_INPUT_LENGTH;
		XFIH_GOTO(END);
	}

	/* Initialize the AES ASU Instance DMA pointer */
	InstancePtr->AsuDmaPtr = DmaPtr;

	/*
	 * If the Output data address is zero, the input data will be considered as AAD data,
	 * configure AES AAD configurations before pushing AAD data to AES engine.
	 */
	if (OutDataAddr == 0U) {
		XAes_ConfigAad(InstancePtr);
	}

	/* Configure DMA with AES and transfer the data to AES engine */
	Status = XAes_CfgDmaWithAesAndXfer(InstancePtr, InDataAddr, OutDataAddr, DataLength,
					   IsLastChunk);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/*
	 * If the Output data address is zero, the input data will be considered as AAD data,
	 * clear configuration of AAD.
	 */
	if (OutDataAddr == 0U) {
		XAes_ClearConfigAad(InstancePtr);
	}

	/* Update AES state machine to encrypt updated */
	InstancePtr->AesState = XAES_UPDATED;

END:
	if ((InstancePtr != NULL) && (Status != XASUFW_SUCCESS)) {
		/* Set AES under reset on failure */
		XAes_SetReset(InstancePtr);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function waits for AES encrypt/decrypt operation to complete, for MAC modes
 *		it generates and stores the the Tag at specified output TagAddr for encrypt
 *		operation and in Case of decrypt operation, it receives the Tag at TagAddr as input
 *		and it compares with the generated Tag.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	DmaPtr		ASU DMA pointer allocated for AES operation.
 * @param	TagAddr		Address of the Input/Output Tag.
 * @param	TagLen		Length of Tag in bytes and it will be zero for all AES
 *				standard modes like, ECB, CBC, OFB, CFB, CTR
 *
 * @return
 *		- Upon successful completion of final AES operation, it returns XASUFW_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
s32 XAes_Final(XAes *InstancePtr, XAsufw_Dma *DmaPtr, u64 TagAddr, u32 TagLen)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if ((DmaPtr == NULL) || (DmaPtr->AsuDma.IsReady != XIL_COMPONENT_IS_READY)) {
		Status = XASUFW_AES_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if (InstancePtr->AesState != XAES_UPDATED) {
		Status = XASUFW_AES_STATE_MISMATCH_ERROR;
		XFIH_GOTO(END);
	}

	/* Validate the Tag with respect to the user provided engine mode */
	Status = XAes_ValidateTag(InstancePtr, TagAddr, TagLen);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* Initialize the AES ASU Instance DMA pointer */
	InstancePtr->AsuDmaPtr = DmaPtr;

	/* Wait for AES encrypt/decrypt operation to complete */
	Status = XAes_WaitForDone(InstancePtr);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* For AES MAC modes, generate/verify the Tag based on AES operation type */
	if ((InstancePtr->EngineMode == XASU_AES_CCM_MODE) ||
	    (InstancePtr->EngineMode == XASU_AES_GCM_MODE) ||
	    (InstancePtr->EngineMode == XASU_AES_CMAC_MODE)) {
		Status = XAes_ProcessTag(InstancePtr, TagAddr, TagLen);
	}

END:
	if (InstancePtr != NULL) {
		/* Set AES under reset on failure */
		XAes_SetReset(InstancePtr);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns a reference to an XAes_Config structure based on the unique
 * 		device id. The return value will refer to an entry in the device configuration
 * 		table.
 *
 * @param	DeviceId	Unique device ID of the device for the lookup operation.
 *
 * @return
 * 		- It returns CfgPtr, which is a reference to a config record in the configuration
 *		  table corresponding to the given DeviceId if match is found.
 *		- It returns NULL, if no match is found.
 *
 *************************************************************************************************/
static XAes_Config *XAes_LookupConfig(u16 DeviceId)
{
	XAes_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks for all the instances */
	for (Index = 0U; Index < XASU_XAES_NUM_INSTANCES; Index++) {
		if (AesConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &AesConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

/*************************************************************************************************/
/**
 * @brief	This function checks whether given key is zeroed or not.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	KeySrc		Key source to check whether given key is zeroed or not.
 *
 * @return
 *		- Upon successful non zeroed key, it returns XASUFW_SUCCESS.
 *		- Otherwise, it returns an error code.
 *
 *************************************************************************************************/
static s32 XAes_CheckKeyZeroedStatus(XAes *InstancePtr, u32 KeySrc)
{
	s32 Status = XASUFW_AES_ZEROED_KEY_NOT_ALLOWED;
	u32 KeyZeroedStatus;

	KeyZeroedStatus = XAsufw_ReadReg(InstancePtr->KeyBaseAddress + XAES_KEY_ZEROED_STATUS_OFFSET);

	if ((KeyZeroedStatus & AesKeyLookupTbl[KeySrc].KeyZeroedStatusMask) == 0U) {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function configures AES DPA counter measures by reading system configuration.
 *		Note that on soft reset the DPA CM is enabled by default So, it needs to be
 *		configured every start of AES operation.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 *************************************************************************************************/
static void XAes_ConfigCounterMeasures(XAes *InstancePtr)
{
	if (InstancePtr->AesCmConfig == XASUFW_CONFIG_ENABLE) {
		XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_CM_OFFSET),
				XAES_CM_ENABLE_MASK);
	} else {
		XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_CM_OFFSET),
				XAES_CM_DISABLE_MASK);
	}
}

/*************************************************************************************************/
/**
 * @brief	This function configures the AES engine for encrypt/decrypt operation based on the
 *		OperationType.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 *************************************************************************************************/
static void XAes_ConfigAesOperation(XAes *InstancePtr)
{
	/* Configure AES engine for encrypt/decrypt operation */
	if (InstancePtr->OperationType == XASU_AES_ENCRYPT_OPERATION) {
		XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET),
				(InstancePtr->EngineMode | (XAES_MODE_CONFIG_ENC_DEC_MASK)));
	} else {
		XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET),
				InstancePtr->EngineMode);
	}
}

/*************************************************************************************************/
/**
 * @brief	This function loads key from selected key source to AES engine.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	KeySrc		key source from which the key will be loaded into AES engine.
 * @param	KeySize		Size of the key to be loaded into AES engine..
 *
 *************************************************************************************************/
static void XAes_LoadKey(XAes *InstancePtr, u32 KeySrc, u32 KeySize)
{
	/* Load Key Size */
	XAsufw_WriteReg((InstancePtr->KeyBaseAddress + XAES_KEY_SIZE_OFFSET), (u32)KeySize);

	/* AES key source selection value */
	XAsufw_WriteReg((InstancePtr->KeyBaseAddress + XAES_KEY_SEL_OFFSET),
			AesKeyLookupTbl[KeySrc].KeySrcSelVal);

	/* Load key to AES engine */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_OPERATION_OFFSET), XAES_KEY_LOAD_VAL_MASK);
}

/*************************************************************************************************/
/**
 * @brief	This function processes the provided IV and writes it into the appropriate AES IV
 *		registers based on the IV length and loads the IV to AES engine.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	IvAddr		Address of a buffer which contains the IV to be written.
 * @param	IvLen		Lenght of IV in bytes.
 *
 * @return
 *		- Upon successful loading of IV to AES engine, it returns XASUFW_SUCCESS.
 *		- Otherwise, it returns an error code.
 *
 *************************************************************************************************/
static s32 XAes_ProcessAndLoadIv(XAes *InstancePtr, u64 IvAddr, u32 IvLen)
{
	volatile s32 Status = XASUFW_AES_GLITCH_ERROR;
	volatile s32 ClearStatus = XASUFW_FAILURE;
	volatile s32 ClearStatusTmp = XASUFW_FAILURE;
	u32 Index;
	u32 Iv[XASU_AES_IV_SIZE_128BIT_IN_WORDS];

	/*
	 * For AES-GCM mode, if the IV length is not 96 bits, calculate GHASH and
	 * generate a new IV.
	 */
	if ((InstancePtr->EngineMode == XASU_AES_GCM_MODE) &&
	    (IvLen != XASU_AES_IV_SIZE_96BIT_IN_BYTES)) {
		Status = XAes_GHashCal(InstancePtr, IvAddr, (u32)Iv, IvLen);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
		IvLen = XASU_AES_IV_SIZE_128BIT_IN_BYTES;
	} else {
		/* Copy IV from 64-bit address space to local array using SSS DMA loopback */
		Status = XAsufw_DmaXfr(InstancePtr->AsuDmaPtr, IvAddr, (u64)(UINTPTR)Iv,
				       IvLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
	}

	for (Index = 0U; Index < XASUFW_CONVERT_BYTES_TO_WORDS(IvLen); Index++) {
		XAsufw_WriteReg((InstancePtr->AesBaseAddress +
				 (XAES_IV_IN_3_OFFSET - (Index * XASUFW_WORD_LEN_IN_BYTES))),
				Xil_Htonl(Iv[Index]));
	}

	if ((InstancePtr->EngineMode == XASU_AES_GCM_MODE) &&
	    (IvLen == XASU_AES_IV_SIZE_96BIT_IN_BYTES)) {
		XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_IV_IN_0_OFFSET), 0x01U);
	}

	/* Trigger IV Load */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_OPERATION_OFFSET), XAES_IV_LOAD_VAL_MASK);

END:
	ClearStatus = Xil_SecureZeroize((u8 *)Iv, XASU_AES_IV_SIZE_128BIT_IN_BYTES);
	ClearStatusTmp = Xil_SecureZeroize((u8 *)Iv, XASU_AES_IV_SIZE_128BIT_IN_BYTES);
	if (Status == XASUFW_SUCCESS) {
		Status = (ClearStatus | ClearStatusTmp);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs GHASH calculation for provided IV, when AES engine mode is
 *		GCM and IV length is not 96bits.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	IvAddr		Address of IV for which GHASH operation needs to be performed.
 * @param	IvGen		Address at which the generated MAC i.e., new IV will be copied.
 * @param	IvLen		Lenght of IV in bytes on which GHASH operation is performed.
 *
 * @return
 *		- Upon successful GHASH calculation, it returns XASUFW_SUCCESS.
 *		- Otherwise, it returns an error code.
 *
 *************************************************************************************************/
static s32 XAes_GHashCal(XAes *InstancePtr, u64 IvAddr, u32 IvGen, u32 IvLen)
{
	s32 Status = XASUFW_FAILURE;
	u32 ReadModeConfigReg;

	/* Store the mode configuration of previous mode i.e., GCM mode */
	ReadModeConfigReg = XAsufw_ReadReg(InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET);

	InstancePtr->EngineMode = XASU_AES_GHASH_MODE;

	/* Enable auth mask and configure engine mode to GHASH mode */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET),
			(XAES_MODE_CONFIG_AUTH_MASK | InstancePtr->EngineMode));

	/* Configure DMA with AES and transfer the data to AES engine */
	Status = XAes_CfgDmaWithAesAndXfer(InstancePtr, IvAddr, 0U, IvLen, TRUE);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* Disable auth mask and restore mode configuration */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET),
			(ReadModeConfigReg & (~(XAES_MODE_CONFIG_AUTH_MASK))));

	/* Get newly generated IV from the MAC registers */
	Status = XAes_ReadTag(InstancePtr, IvGen, XASU_AES_MAX_TAG_LENGTH_IN_BYTES);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates tag from AES engine.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	TagOutAddr	Address at which generated Tag will be stored.
 * @param	TagLen		Length of Tag in bytes.
 *
 * @return
 *		- Upon successful generation of tag, it returns XASUFW_SUCCESS.
 *		- Otherwise, it returns an error code.
 *
 *************************************************************************************************/
static s32 XAes_ReadTag(XAes *InstancePtr, u32 TagOutAddr, u32 TagLen)
{
	s32 Status = XASUFW_AES_TAG_GENERATE_FAILED;
	u32 Index;
	u32 *TagPtr = (u32 *)TagOutAddr;

	for (Index = 0U; Index < XASUFW_CONVERT_BYTES_TO_WORDS(TagLen); Index++) {
		TagPtr[Index] = Xil_EndianSwap32(XAsufw_ReadReg(InstancePtr->AesBaseAddress +
						 (XAES_MAC_OUT_3_MASK - (Index * XASUFW_WORD_LEN_IN_BYTES))));
		/*
		 * If AES DPA CM is enabled then, read MAC from both MAC_OUT and MAC_MASK_OUT registers,
		 * If disabled, no need read the MAC_MASK_OUT register as it contains zero.
		 */
#ifdef XASUFW_AES_CM_CONFIG
		TagPtr[Index] ^= Xil_EndianSwap32(XAsufw_ReadReg(InstancePtr->AesBaseAddress +
						  (XAES_MAC_MASK_OUT_3_MASK - (Index * XASUFW_WORD_LEN_IN_BYTES))));
#endif
	}

	if (Index == XASUFW_CONVERT_BYTES_TO_WORDS(TagLen)) {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function generates tag from AES engine and verifies the generated Tag with the
 *		Tag from TagInAddr.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	TagInAddr	Address of input Tag from which generated tag is compared.
 * @param	TagLen		Length of Tag in bytes.
 *
 * @return
 *		- Upon successful generation and verification of tag, it returns XASUFW_SUCCESS.
 *		- Otherwise, it returns an error code.
 *
 *************************************************************************************************/
static s32 XAes_ReadNVerifyTag(XAes *InstancePtr, u32 TagInAddr, u32 TagLen)
{
	s32 Status = XASUFW_AES_TAG_COMPARE_FAILED;
	u32 Index ;
	u32 ReadReg;
	u32 *TagPtr = (u32 *)TagInAddr;

	for (Index = 0U; Index < XASUFW_CONVERT_BYTES_TO_WORDS(TagLen); Index++) {
		ReadReg = Xil_EndianSwap32(XAsufw_ReadReg(InstancePtr->AesBaseAddress +
					   (XAES_MAC_OUT_3_MASK - (Index * XASUFW_WORD_LEN_IN_BYTES))));
		/*
		 * If AES DPA CM is enabled then, read MAC from both MAC_OUT and MAC_MASK_OUT registers,
		 * If disabled, no need read the MAC_MASK_OUT register as it contains zero.
		 */
#ifdef XASUFW_AES_CM_CONFIG
		ReadReg ^= Xil_EndianSwap32(XAsufw_ReadReg(InstancePtr->AesBaseAddress +
					    (XAES_MAC_MASK_OUT_3_MASK - (Index * XASUFW_WORD_LEN_IN_BYTES))));
#endif
		if (ReadReg != TagPtr[Index]) {
			XFIH_GOTO(END);
		}
	}

	if (Index == XASUFW_CONVERT_BYTES_TO_WORDS(TagLen)) {
		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function processes Tag based on AES operation type, if the Operation type is
 *		encryption then, it generates the Tag and stores it into output TagAddr and for
 *		decryption it gnerates and compares the tag with input TagAddr.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	TagAddr		Address of the output Tag for encryption and input Tag for
 *				decryption.
 * @param	TagLen		Length of Tag in bytes and it will be zero for all AES standard
 *				modes like, ECB, CBC, OFB, CFB, CTR.
 *
 * @return
 *		- Upon successful completion of final AES operation, it returns XASUFW_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
static s32 XAes_ProcessTag(XAes *InstancePtr, u64 TagAddr, u32 TagLen)
{
	s32 Status = XASUFW_FAILURE;
	u8 Tag[XASU_AES_MAX_TAG_LENGTH_IN_BYTES];

	if (InstancePtr->OperationType == XASU_AES_ENCRYPT_OPERATION) {
		/* Generate tag */
		Status = XAes_ReadTag(InstancePtr, (u32)Tag, TagLen);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
		/*
		 * Copy generated Tag from local array to 64-bit address space using
		 * SSS DMA loopback.
		 */
		Status = XAsufw_DmaXfr(InstancePtr->AsuDmaPtr, (u64)(UINTPTR)Tag, TagAddr,
				       TagLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
	} else {
		/*
		 * Copy Tag from 64-bit address space to local array using SSS DMA
		 * loopback.
		 */
		Status = XAsufw_DmaXfr(InstancePtr->AsuDmaPtr, TagAddr, (u64)(UINTPTR)Tag,
				       TagLen, 0U);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
		/* Generate and verify the tag */
		Status = XAes_ReadNVerifyTag(InstancePtr, (u32)Tag, TagLen);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function configures AAD before pushing AAD data to AES engine.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 *************************************************************************************************/
static void XAes_ConfigAad(XAes *InstancePtr)
{
	if ((InstancePtr->EngineMode == XASU_AES_GCM_MODE) ||
	    (InstancePtr->EngineMode == XASU_AES_CCM_MODE)) {
		/*
		 * For modes like AES-GCM and AES-CCM, where payload is expected after AAD,
		 * configure only AUTH_MASK bit.
		 */
		XAsufw_RCMW((InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET),
			    XAES_MODE_CONFIG_AUTH_MASK, XAES_MODE_CONFIG_AUTH_MASK);
	} else {
		/*
		 * For MAC only modes like AES-GMAC and AES-CMAC, where no payload is expected to
		 * follow AAD data, configure both AUTH_MASK and AUTH_WITH_NO_PAYLOAD bit.
		 */
		XAsufw_RCMW((InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET),
			    (XAES_MODE_CONFIG_AUTH_MASK | XAES_MODE_CONFIG_AUTH_WITH_NO_PAYLOAD_MASK),
			    (XAES_MODE_CONFIG_AUTH_MASK | XAES_MODE_CONFIG_AUTH_WITH_NO_PAYLOAD_MASK));
	}
}

/*************************************************************************************************/
/**
 * @brief	This function clears configuration of AAD after pushing final AAD data to
 *		AES engine.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 *************************************************************************************************/
static void XAes_ClearConfigAad(XAes *InstancePtr)
{
	/* Clear AUTH and AUTH_WITH_NO_PAYLOAD bits in AES mode configuration register */
	XAsufw_RMW((InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET),
		   (XAES_MODE_CONFIG_AUTH_MASK | XAES_MODE_CONFIG_AUTH_WITH_NO_PAYLOAD_MASK), 0U);
}

/*************************************************************************************************/
/**
 * @brief	This function configures SSS DMA with AES and transfers data to AES engine to get
 * 		encrypted/decrypted data.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	InDataAddr	Address of the input buffer which needs to be pushed to AES engine.
 * @param	OutDataAddr	Address of output buffer where the encrypted/decrypted Data
 *				will be updated.
 * @param	Size		Size of data to send/receive to/from AES engine in bytes.
 * @param	IsLastChunk	On last update of data to be encrypted/decrypted, this parameter
 *				should be set to TRUE otherwise FALSE.
 *
 * @return
 *		- Upon successful configuration of SSS DMA and transfer of data to AES engine,
 *		  it returns XASUFW_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
static s32 XAes_CfgDmaWithAesAndXfer(XAes *InstancePtr, u64 InDataAddr, u64 OutDataAddr, u32 Size,
				     u8 IsLastChunk)
{
	s32 Status = XASUFW_FAILURE;

	/* Configure DMA and transfer the data to AES engine */
	Status = XAsufw_SssAesWithDma(InstancePtr->AsuDmaPtr->SssDmaCfg);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* If OutDataAddr is non-zero address then, configure destination channel and transfer */
	if (OutDataAddr != 0U) {
		XAsuDma_ByteAlignedTransfer(&InstancePtr->AsuDmaPtr->AsuDma, XCSUDMA_DST_CHANNEL,
					    OutDataAddr, Size, IsLastChunk);
	}

	XAsuDma_ByteAlignedTransfer(&InstancePtr->AsuDmaPtr->AsuDma, XCSUDMA_SRC_CHANNEL,
				    InDataAddr, Size, IsLastChunk);

	/* Wait till the ASU source DMA done bit to set */
	Status = XAsuDma_WaitForDoneTimeout(&InstancePtr->AsuDmaPtr->AsuDma,
					    XCSUDMA_SRC_CHANNEL);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* Acknowledge the transfer has completed from source */
	XAsuDma_IntrClear(&InstancePtr->AsuDmaPtr->AsuDma, XCSUDMA_SRC_CHANNEL,
			  XCSUDMA_IXR_DONE_MASK);

	if (OutDataAddr != 0U) {
		/* Wait till the ASU destination DMA done bit to set */
		Status = XAsuDma_WaitForDoneTimeout(&InstancePtr->AsuDmaPtr->AsuDma,
						    XCSUDMA_DST_CHANNEL);
		if (Status != XASUFW_SUCCESS) {
			XFIH_GOTO(END);
		}

		/* Acknowledge the transfer has completed from destination */
		XAsuDma_IntrClear(&InstancePtr->AsuDmaPtr->AsuDma, XCSUDMA_DST_CHANNEL,
				  XCSUDMA_IXR_DONE_MASK);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function waits for AES encrypt/decrypt operation to complete.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 * @return
 *		- Upon successful completion of AES encrypt/decrypt operation, it returns
 *		  XASUFW_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
static s32 XAes_WaitForDone(XAes *InstancePtr)
{
	s32 Status = XASUFW_FAILURE;

	/* Check for AES operation is completed within Timeout(10sec) or not */
	Status = Xil_WaitForEvent((InstancePtr->AesBaseAddress + XAES_INTERRUPT_STATUS_OFFSET),
				  XAES_INTERRUPT_STATUS_DONE_MASK, XAES_INTERRUPT_STATUS_DONE_MASK, XAES_TIMEOUT_MAX);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function places the AES hardware core into reset state and updates AES state
 *		machine to initialized state.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 *
 *************************************************************************************************/
static void XAes_SetReset(XAes *InstancePtr)
{
	InstancePtr->AesState = XAES_INITIALIZED;
	XAsufw_CryptoCoreSetReset(InstancePtr->AesBaseAddress, XAES_SOFT_RST_OFFSET);
}

/*************************************************************************************************/
/**
 * @brief	This function updates data and key to the AES engine in split mode with DPA CM
 *		enabled.
 *
 * @param	InstancePtr	Pointer to the XAes instance.
 * @param	DmaPtr		ASU DMA pointer allocated for AES operation.
 * @param 	KeyObjPtr	Key object Pointer.
 * @param 	InputDataAddr	Input data address.
 * @param 	OutputDataAddr	Output address where the decrypted data to be stored.
 * @param	DataLength	Length of both input/output data in bytes.
 *
 * @return
 *		- Upon successful decryption of data, it returns XASUFW_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
s32 XAes_DpaCmDecryptData(XAes *InstancePtr, XAsufw_Dma *DmaPtr, XAsu_AesKeyObject *KeyObjPtr,
			  u32 InputDataAddr, u32 OutputDataAddr, u32 DataLength)
{
	s32 Status = XASUFW_FAILURE;;
	u32 Index;
	u32 ReadReg = 0U;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if ((DmaPtr == NULL) || (DmaPtr->AsuDma.IsReady != XIL_COMPONENT_IS_READY)) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if ((KeyObjPtr == NULL) || (InputDataAddr == 0U) || (OutputDataAddr == 0U)) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	if (DataLength == 0U) {
		Status = XASUFW_AES_INVALID_PARAM;
		goto END;
	}

	InstancePtr->AsuDmaPtr = DmaPtr;

	/* Release soft reset of AES engine */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->AesBaseAddress, XAES_SOFT_RST_OFFSET);

	/* Configure AES engine for decrypt operation */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_MODE_CONFIG_OFFSET),
			XASU_AES_GCM_MODE);

	/* Configure AES engine in split mode to update data and key to aes core */
	XAsufw_WriteReg((InstancePtr->AesBaseAddress + XAES_SPLIT_CFG_OFFSET),
			(XAES_SPLIT_CFG_KEY_SPLIT_VALUE | XAES_SPLIT_CFG_DATA_SPLIT_VALUE));

	/* Configure AES DPA counter measures */
	XAes_ConfigCounterMeasures(InstancePtr);

	/* Write Key mask value */
	for (Index = 0U; Index < XASU_AES_KEY_SIZE_256BIT_IN_WORDS; Index++) {
		XAsufw_WriteReg((InstancePtr->KeyBaseAddress + (XAES_KEY_MASK_0_OFFSET +
				 (u32)(Index * XASUFW_WORD_LEN_IN_BYTES))), 0x0U);
	}

	/* Write AES key */
	Status = XAes_WriteKey(InstancePtr, DmaPtr, (u64)(UINTPTR)KeyObjPtr);
	if (Status != (u32)XASUFW_SUCCESS) {
		goto END;
	}

	/* Load key to AES engine */
	XAes_LoadKey(InstancePtr, KeyObjPtr->KeySrc, KeyObjPtr->KeySize);

	Status = XAes_CfgDmaWithAesAndXfer(InstancePtr, InputDataAddr, OutputDataAddr, DataLength, TRUE);

END:
	return Status;
}
/** @} */