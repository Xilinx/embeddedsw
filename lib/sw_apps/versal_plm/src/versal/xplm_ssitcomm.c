/**************************************************************************************************
* Copyright (c) 2024, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/


/*************************************************************************************************/
/**
*
* @file versal/xplm_ssitcomm.c
*
* This file contains the PLMI ssit secure communication code. Secure communication is established
* with configure secure communictaion command. The communication between master and slaves occurs
* in encrypted format after establishment of secure communication. Each message gets encrypted with
* unique key-IV pair during communication.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- ---------------------------------------------------------------------------
* 1.00  pre  07/11/2024 Initial release
*       pre  07/30/2024 Fixed misrac and coverity violations
*
* </pre>
*
* @note
*
**************************************************************************************************/

/***************************** Include Files *****************************************************/
#include "xplm_ssitcomm.h"
#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
#include "xil_types.h"
#include "xplmi_status.h"
#include "xplmi_hw.h"
#ifdef PLM_ENABLE_SECURE_PLM_TO_PLM_COMM
#include "xplmi_plat.h"
#include "xsecure_init.h"

/************************** Constant Definitions *************************************************/
#define XPLMI_ZERO                       (0U) /**< Zero value */
#define XPLMI_BYTE_MASK                  0XFF /**< Mask for extracting byte */
#define XPLMI_BYTE_SHIFT                 (8U) /**< Byte shift value*/
#define XPLM_CMD_SIZE                  (8U) /**< Command size */
#define XPLM_NUM_OF_AES_OP              (2U) /**< Number of operations */
#define XPLM_IV2_AND_KEY_SIZE_BYTES     (48U) /**< IV + key size in bytes */
#define XPLM_AAD_LEN_BYTES              (32U) /**< Size of AAD in first config secure
                                         communication command */
#define XPLM_HEADER_LEN_BYTES           (4U) /**< Header size in bytes */
#define XPLM_SSIT_COMM_SECURE_MSG       (0x01) /**< Constant value for secure message */
#define XPLM_SSIT_COMM_NON_SECURE_MSG   (0x00) /**< Constant value for non-secure message */
#define XPLM_IV_OFFSET_IN_ENCRYPTED_CMD (8U) /**< IV offset in encrypted command */
#define XPLM_LEN_CALC(Value)            (((Value & XPLMI_CMD_LEN_MASK) >> LEN_BYTES_SHIFT) * \
                                        XPLMI_WORD_LEN) /**< calculation of length in bytes
										                from header */
#define XPLM_MODULE_AND_CFG_SEC_COMMCMD (0x12B) /**< module and API ID for config secure
                                    communictaion command */

/**************************** Type Definitions ***************************************************/
typedef enum{
	ENCRYPTION = 0, /**< Encryption operation */
	DECRYPTION, /**< Decryption operation */
} XPlmi_Operation;

typedef struct
{
	int (*Init)(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
	           XSecure_AesKeySize KeySize, u64 IvAddr); /**< AES operation Initialization */
	int (*UpdateAad)(XSecure_Aes *InstancePtr, u64 AadAddr, u32 AadSize); /**< AAD update */
	int (*UpdateData)(XSecure_Aes *InstancePtr, u64 InDataAddr,
	           u64 OutDataAddr, u32 Size, u8 IsLastChunk); /**< Data Update */
	int (*Final)(XSecure_Aes *InstancePtr, u64 GcmTagAddr); /**< GCM tag generation/verification */
} XPlm_SsitCommOps;

typedef struct
{
	u64 AadAddr; /**< Address of AAD */
	u64 InDataAddr; /**< Input data address */
	u64 TagAddr; /**< Address of GCM tag */
	u32 *IvPtr; /**< Pointer to IV */
	u32 *OutDataPtr; /**< Pointer to Output buffer which contains output data */
	u32 AADLen; /**< Length of AAD */
	u32 DataLen; /**< Length of Data */
	XSecure_AesKeySrc KeySrc; /**< Key Source */
	XPlmi_Operation OperationFlag; /**< Operation */
	u32 SlrIndex; /**< SLR index */
	u32 IsCfgSecCommCmd; /**< Configure secure communication command or not */
	u32 TempRespBuf[XPLMI_CMD_RESP_SIZE]; /**< Temporary buffer for response */
} XPlm_SsitCommParams;
#endif

/***************** Macros (Inline Functions) Definitions *****************************************/

/*************************************************************************************************/
/**
* @brief This function is used to fetch SLR type
*
* @return SLR Type
*
**************************************************************************************************/
static inline u32 XPlm_SsitCommGetSlrType(void)
{
	return (XPlmi_In32(PMC_TAP_SLR_TYPE) & PMC_TAP_SLR_TYPE_VAL_MASK);
}

#ifdef PLM_ENABLE_SECURE_PLM_TO_PLM_COMM
/*************************************************************************************************/
/**
* @brief This function is used to fetch message type based on secure communication establishment
*        status and command(configure secure communication or not).
*
* @param SecSsitCommEst Status of secure communication establishment
* @param IsCfgSecCommCmd Indicates whether the command is configure secure communication command
*                        or not
*
* @return XPLM_SSIT_COMM_SECURE_MSG for secure message
*         XPLM_SSIT_COMM_NON_SECURE_MSG for non-secure message
**************************************************************************************************/
static inline u32 XPlm_SsitCommGetMsgType(XPlmi_SecCommEstFlag SecSsitCommEst,u32 IsCfgSecCommCmd)
{
	u32 MsgType = XPLM_SSIT_COMM_SECURE_MSG;

	if ((IsCfgSecCommCmd  == FALSE) && (SecSsitCommEst != ESTABLISHED)) {
		MsgType = XPLM_SSIT_COMM_NON_SECURE_MSG;
	}
	return MsgType;
}

/*************************************************************************************************/
/**
* @brief This function is used to fetch key source for the given SLR index
*
* @param SlrIndex SLR number whose key source is needed
*
* @return key source
**************************************************************************************************/
static inline XSecure_AesKeySrc XPlm_SsitSecCommGetKeySrcofSlr(u32 SlrIndex)
{
	return (XSECURE_AES_USER_KEY_0 + (SlrIndex - 1U));
}

/*************************************************************************************************/
/**
* @brief This function is used to fetch IV address for the given SLR index
*
* @param SlrIndex SLR number whose IV address is needed
*
* @return IV address
*
**************************************************************************************************/
static inline  u32 *XPlm_SsitSecCommGetIvofSlr(u32 SlrIndex)
{
	return ((u32 *)(XPLMI_RTCFG_SSLR1_IV_ADDR + (XPLMI_IV_SIZE_BYTES * (SlrIndex - 1U))));
}

/*************************************************************************************************/
/**
* @brief This function writes AAD of first configure secure communication command
*
* @param DestAddr Address to which the AAD has to be written
* @param SrcAddr  Address where AAD data is present
*
**************************************************************************************************/
static inline void XPlm_SsitComm1stSecCfgCmdSetAad(u64 DestAddr, u64 SrcAddr)
{
	XSecure_MemCpy64(DestAddr, SrcAddr,
					(XPLMI_IV_SIZE_BYTES + XPLMI_WORD_LEN + XPLM_HEADER_LEN_BYTES));
}

/*************************************************************************************************/
/**
* @brief This function writes AAD for all the commands
*
* @param DestAddr Address to which the AAD has to be written
* @param SrcAddr  Address where AAD data is present
*
**************************************************************************************************/
static inline void XPlm_SsitCommSetAad(u64 DestAddr, u64 SrcAddr)
{
	XSecure_MemCpy64(DestAddr, SrcAddr, XPLM_HEADER_LEN_BYTES);
}

/************************** Function Prototypes **************************************************/
static int XPlm_SsitCommSendMessage(u32* Buf, u32 BufSize, u32 SlrIndex, u32 IsCfgSecCommCmd);
static int XPlm_SsitCommReceiveMessage(u32* Buf, u32 BufSize, u32 SlrIndex, u32 IsCfgSecCommCmd);
static int XPlm_SsitCommAesKeyWrite(u32 SlrIndex, u32 KeyAddr);
static int XPlm_SsitCommKeyIvUpdate(XPlmi_Cmd *Cmd);

/************************** Variable Definitions *************************************************/
static u32 XPlm_SsitCommIV[XPLMI_IV_SIZE_WORDS];

static XPlmi_SsitCommFunctions XPlm_SsitCommFuncs = {.SendMessage = XPlm_SsitCommSendMessage,
											.ReceiveMessage = XPlm_SsitCommReceiveMessage,
											.AesKeyWrite = XPlm_SsitCommAesKeyWrite,
											.KeyIvUpdate = XPlm_SsitCommKeyIvUpdate
											};

/*****************************************************************************/
/**
* @brief   This function increments the IV with given value.
*
* @param   AesIv - Pointer to an array of 16 bytes which holds IV to be
*          increment.
* @param   IncrValue - Value with which IV needs to be incremented.
*
* @return  None.
******************************************************************************/
static void XPlmi_SsitIvIncrement(u8 *AesIv, u8 IncrValue)
{
	u8 *IvPtr = AesIv;
    u32 Carry = IncrValue;
    u32 Result;
    s32 Index;

    /**
    * IV increment is done as below
    * Repeat I = 0 to 11 OR till Carry becomes zero
    * Get (AesIv[I], carry) by performing AesIv[I] + carry
    */
   for (Index = XPLMI_IV_SIZE_BYTES - 1U; Index >= (s8)XPLMI_ZERO; Index--) {
		Result = IvPtr[Index] + Carry;
		IvPtr[Index] = (u8)(Result & XPLMI_BYTE_MASK);
		Carry = Result >> XPLMI_BYTE_SHIFT;
		/* If carry is non zero continue else break */
		if (Carry == (u32)XPLMI_ZERO) {
			break;
		}
	}
}

/*************************************************************************************************/
/**
* @brief This function performs encryption or decryption operation
*
* @param EncDecParamPtr This contains the parameters required for operation
*
* @return XST_SUCCESS on successful encryption/decryption
*         error code  on failure
*
**************************************************************************************************/
static int XPlm_SsitCommPerformAesOperation(XPlm_SsitCommParams *EncDecParamPtr)
{
	int Status = XST_FAILURE;

	/* Get AES instance and DMA instance*/
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE_ID);
	const XPlm_SsitCommOps XPlm_SsitCommAesOps[XPLM_NUM_OF_AES_OP] = {
		[ENCRYPTION] = {
			.Init = XSecure_AesEncryptInit,
			.UpdateAad = XSecure_AesUpdateAad,
			.UpdateData = XSecure_AesEncryptUpdate,
			.Final = XSecure_AesEncryptFinal
		},
		[DECRYPTION] = {
			.Init = XSecure_AesDecryptInit,
			.UpdateAad = XSecure_AesUpdateAad,
			.UpdateData = XSecure_AesDecryptUpdate,
			.Final = XSecure_AesDecryptFinal
		}
	};

	const XPlm_SsitCommOps *OpsPtr = &XPlm_SsitCommAesOps[EncDecParamPtr->OperationFlag];

	/* Validate DMA instance pointer */
	if (NULL == PmcDmaInstPtr) {
		Status = XPLMI_SSIT_INAVLID_DMA;
		goto END;
	}

	/* Initialize the Aes driver so that it's ready to use */
	Status = XSecure_AesInitialize(XSecureAesInstPtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_SSIT_AES_INIT_FAIL;
		goto END;
	}

	/* Initializes the AES engine for encryption/decryption */
	Status = OpsPtr->Init(XSecureAesInstPtr, EncDecParamPtr->KeySrc,
				XSECURE_AES_KEY_SIZE_256, (UINTPTR)EncDecParamPtr->IvPtr);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_SSIT_ENCDEC_INIT_FAIL;
		goto END;
	}

	/* Updates the AES engine with Additional Authenticated Data(AAD) */
	if (EncDecParamPtr->AADLen > 0U) {
		Status = OpsPtr->UpdateAad(XSecureAesInstPtr, EncDecParamPtr->AadAddr,
		        EncDecParamPtr->AADLen);
		if (Status != XST_SUCCESS) {
			Status = XPLMI_SSIT_AAD_UPDATE_FAIL;
			goto END;
		}
	}

	/**
	 * Updates the AES engine for encryption/decryption with provided data and
	 * stores the encrypted/decrypted data at specified address */
	Status = OpsPtr->UpdateData(XSecureAesInstPtr, EncDecParamPtr->InDataAddr,
	        (UINTPTR)EncDecParamPtr->OutDataPtr, EncDecParamPtr->DataLen, TRUE);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_SSIT_DATA_UPDATE_FAIL;
		goto END;
	}

	/* Generates the GCM tag during encryption and Verifies during decryption */
	Status = OpsPtr->Final(XSecureAesInstPtr, EncDecParamPtr->TagAddr);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_SSIT_TAG_GENERATION_FAIL;
		goto END;
	}
END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief This function does encryption
*
* @param EncParam   Contains parameters to encrypt the data
*
* @return Status of encryption operation
*
**************************************************************************************************/
static int XPlm_SsitCommEncryptMsg(XPlm_SsitCommParams *EncParam)
{
	EncParam->OperationFlag = ENCRYPTION;
	return XPlm_SsitCommPerformAesOperation(EncParam);
}

/*************************************************************************************************/
/**
* @brief This function does decryption
*
* @param DecParam   Contains parameters to decryot the data
*
* @return Status of decryption operation
*
**************************************************************************************************/
static int XPlm_SsitCommDecryptMsg(XPlm_SsitCommParams *DecParam)
{
	DecParam->OperationFlag = DECRYPTION;
	return XPlm_SsitCommPerformAesOperation(DecParam);
}

/*************************************************************************************************/
/**
* @brief This function writes parameters needed to encrypt the data in buffer
*
* @param SecSsitCommEst Status of secure communication establishment status
* @param EncParams   Contains parameters to encrypt the data
* @param SsitBufAddr    Address of SSIT buffer to write after encryption
* @param Buf            Address of buffer which contains original data
*
**************************************************************************************************/
static void XPlm_SsitCommPrepareEncParams(XPlmi_SecCommEstFlag SecSsitCommEst, XPlm_SsitCommParams *EncParams,
                                          u64 SsitBufAddr, u32 *Buf)
{
	/* Fetch SLR type */
	u32 SlrType = XPlm_SsitCommGetSlrType();

	/* Prepare parameters based on SLR type */
	if (SlrType == XPLMI_SSIT_MASTER_SLR) {
		EncParams->KeySrc = XPlm_SsitSecCommGetKeySrcofSlr(EncParams->SlrIndex);
	    EncParams->IvPtr = XPlm_SsitSecCommGetIvofSlr(EncParams->SlrIndex);

		/**
		 * Prepare AAD,start address and length of data to be encrypted
		 * based on secure communication status and command
		 */
		if ((EncParams->IsCfgSecCommCmd == TRUE) && (SecSsitCommEst != ESTABLISHED)) {
			XPlm_SsitComm1stSecCfgCmdSetAad(SsitBufAddr, (u64)(UINTPTR)Buf);
			EncParams->InDataAddr = (u64)(UINTPTR)&Buf[XPLMI_IV2_OFFSET_INCMD];
			EncParams->DataLen = XPLM_IV2_AND_KEY_SIZE_BYTES;
		}
		else {
			XPlm_SsitCommSetAad(SsitBufAddr, (u64)(UINTPTR)Buf);
			EncParams->InDataAddr = (u64)(UINTPTR)&Buf[PAYLOAD_OFFSET];
			if(EncParams->IsCfgSecCommCmd == TRUE) {
				EncParams->DataLen = XPLM_LEN_CALC(Buf[HEADER_OFFSET]);
			}
			else {
				EncParams->DataLen = (XPLM_CMD_SIZE - 1U) * XPLMI_WORD_LEN;
			}

			/* Increment IV by 2 to use unique IV for every data set*/
			XPlmi_SsitIvIncrement((u8 *)EncParams->IvPtr, 2U);
		}
		EncParams->AADLen = XPLM_AAD_LEN_BYTES;
	}
	else {
		EncParams->KeySrc = XPlm_SsitSecCommGetKeySrcofSlr(SECCOMM_SLAVE_INDEX);
		EncParams->IvPtr = XPlm_SsitSecCommGetIvofSlr(SECCOMM_SLAVE_INDEX);

		/* Prepare start address and length of data to be decrypted */
		EncParams->InDataAddr = (u64)(UINTPTR)Buf;
		EncParams->DataLen = (XPLMI_CMD_RESP_SIZE * XPLMI_WORD_LEN);

		/* Increment IV by 2 to use unique IV for every data set */
		XPlmi_SsitIvIncrement((u8 *)EncParams->IvPtr, 2U);

		EncParams->AADLen = 0U;
	}
	/* Take AadAddr, TagAddr, start address of buf to place encrypted data */
	EncParams->AadAddr = SsitBufAddr;
	EncParams->OutDataPtr = (u32 *)(UINTPTR)(SsitBufAddr + EncParams->AADLen);
	EncParams->TagAddr = SsitBufAddr + EncParams->AADLen + EncParams->DataLen;
}

/*************************************************************************************************/
/**
* @brief This function writes parameters needed to decrypt the data in buffer
*
* @param SecSsitCommEst Status of secure communication establishment status
* @param DecParams   Contains parameters to decrypt the data
* @param SsitBufAddr    Address of SSIT buffer which contains encrypted data
* @param Buf            Address of buffer to which decrypted data has to written
*
**************************************************************************************************/
static void XPlm_SsitCommPrepareDecParams(XPlmi_SecCommEstFlag SecSsitCommEst, XPlm_SsitCommParams
                                         *DecParams, u64 SsitBufAddr, u32 *Buf)
{
	/* Fetch SLR type */
	u32 SlrType = XPlm_SsitCommGetSlrType();

	/* Prepare parameters based on SLR type */
	if (SlrType == XPLMI_SSIT_MASTER_SLR) {
		DecParams->KeySrc =  XPlm_SsitSecCommGetKeySrcofSlr(DecParams->SlrIndex);
	    DecParams->IvPtr = XPlm_SsitSecCommGetIvofSlr(DecParams->SlrIndex);

		DecParams->DataLen = (XPLMI_CMD_RESP_SIZE * XPLMI_WORD_LEN);

		/* Use (currentIV+1) for decryption */
		XSecure_MemCpy64((u64)(UINTPTR)XPlm_SsitCommIV, (u64)(UINTPTR)DecParams->IvPtr,
		                 XPLMI_IV_SIZE_BYTES);
		XPlmi_SsitIvIncrement((u8 *)XPlm_SsitCommIV, 1U);

		/* Take IvPtr, start address of buf to place decrypted data */
		DecParams->OutDataPtr =  &DecParams->TempRespBuf[0];
		DecParams->IvPtr = XPlm_SsitCommIV;
		DecParams->AADLen = 0U;
	}
	else {
		DecParams->KeySrc = XPlm_SsitSecCommGetKeySrcofSlr(SECCOMM_SLAVE_INDEX);
		DecParams->IvPtr = XPlm_SsitSecCommGetIvofSlr(SECCOMM_SLAVE_INDEX);

		/**
		 * Prepare AAD,length of data to be decrypted and  start address of buffer to place
		 * decrypted data based on secure communication status and command
		 */
		if ((DecParams->IsCfgSecCommCmd == TRUE) && (SecSsitCommEst != ESTABLISHED)) {
			XPlm_SsitComm1stSecCfgCmdSetAad((u64)(UINTPTR)Buf, SsitBufAddr);

			/* Prepare DataLen */
			DecParams->DataLen = XPLM_IV2_AND_KEY_SIZE_BYTES;

			/* Update IV with IV1 */
			XSecure_MemCpy64((u64)XPLMI_SLR_CURRENTIV_ADDR,
			                (SsitBufAddr + XPLM_IV_OFFSET_IN_ENCRYPTED_CMD), XPLMI_IV_SIZE_BYTES);

			/* Take start address of buf to place decrypted data */
			DecParams->OutDataPtr = &Buf[XPLMI_IV2_OFFSET_INCMD];
		}
		else {
			XPlm_SsitCommSetAad((u64)(UINTPTR)Buf, SsitBufAddr);

			/* Prepare DataLen */
			if(DecParams->IsCfgSecCommCmd == TRUE) {
				DecParams->DataLen = XPLM_LEN_CALC(Buf[HEADER_OFFSET]);
			}
			else {
				DecParams->DataLen = (XPLM_CMD_SIZE - 1U) * XPLMI_WORD_LEN;
			}

			/* use (currentIV+1) for decryption */
			XSecure_MemCpy64((u64)(UINTPTR)XPlm_SsitCommIV, (u64)XPLMI_SLR_CURRENTIV_ADDR,
					                 XPLMI_IV_SIZE_BYTES);
			XPlmi_SsitIvIncrement((u8 *)XPlm_SsitCommIV, 1U);

			/* Take IvPtr, start address of buf to place decrypted data */
			DecParams->OutDataPtr = &Buf[PAYLOAD_OFFSET];
			DecParams->IvPtr = XPlm_SsitCommIV;
		}
		DecParams->AADLen = XPLM_AAD_LEN_BYTES;
	}
	/* Take AAD address , start address of data to be decrypted and address of GCM tag */
	DecParams->AadAddr = SsitBufAddr;
	DecParams->InDataAddr = SsitBufAddr + DecParams->AADLen;
	DecParams->TagAddr = SsitBufAddr + DecParams->AADLen + DecParams->DataLen;
}

/*************************************************************************************************/
/**
 * @brief   Writes the key provided into respective SLR's AES key registers
 *
 * @param   SlrIndex     SLR number whose key has to be written
 * @param	KeyAddr		 Address of a buffer which should contain the key
 * 				         to be written
 *
 * @return
 *	-	XST_SUCCESS - On successful key written on AES registers
 *	-	Error code - On failure
 *
 *************************************************************************************************/
static int XPlm_SsitCommAesKeyWrite(u32 SlrIndex, u32 KeyAddr)
{
	XSecure_AesKeySrc KeySrc = XSECURE_AES_USER_KEY_0 + (SlrIndex - 1U);
	const XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();
	return XSecure_AesWriteKey(XSecureAesInstPtr, KeySrc, XSECURE_AES_KEY_SIZE_256, (u64)KeyAddr);
}

/*************************************************************************************************/
/**
 * @brief	This function updates the key and IV with new key and new IV
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	Returns the Status of key write
 *		- XST_SUCCESS on success
 *		- Error code on failure
 *
 *************************************************************************************************/
static int XPlm_SsitCommKeyIvUpdate(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;

	/* Check response status and command ID */
    if ((Cmd->Response[0U] == XST_SUCCESS) && ((Cmd->CmdId & XPLMI_MODULE_AND_APIDMASK) ==
	    XPLM_MODULE_AND_CFG_SEC_COMMCMD)) {
		/**
		 *  Update existing key-IV pair with new key-IV pair and set SecKeyIVEstablished
		 * flag
		 */
		Status = XPlmi_MemCpy64((u64)XPLMI_SLR_CURRENTIV_ADDR, (u64)XPLMI_SLR_NEWIV_ADDR,
		                      XPLMI_IV_SIZE_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/* Write key */
		Status = XPlm_SsitCommAesKeyWrite(SECCOMM_SLAVE_INDEX, XPLMI_SLR_NEWKEY_ADDR);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/* Set secure communication established flag and increment IV */
		XPlmi_SsitSetCommEstFlag(SECCOMM_SLAVE_INDEX);
		XPlmi_SsitIvIncrement((u8 *)XPLMI_SLR_CURRENTIV_ADDR, 1U);
	}
	else {
		Status = XST_SUCCESS;
	}
END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief This function sends the message/response in the Buf with or without encryption
*        using key and IV of given SlrNum based on CfgSecCommCmdFlag,SecSsitCommEst Flag
*
* @param Buf             is the address of buffer which contains message/response to be encrypted
* @param BufSize         is the size of buffer
* @param SlrIndex        is the SLR number to which message/response has to be sent
* @param IsCfgSecCommCmd indicates whether the present message is configure secure communication
*                        command or not
*
* @return XST_SUCCESS on successful encryption
*         error code on failure
*
**************************************************************************************************/
static int XPlm_SsitCommSendMessage(u32* Buf, u32 BufSize, u32 SlrIndex, u32 IsCfgSecCommCmd)
{
	u32 SsitBufAddr;
	XPlmi_SecCommEstFlag SecSsitCommEst;
	XPlm_SsitCommParams EncParams;
	u32 MsgType;
	int Status = XST_FAILURE;
	/* Fetch SLR type */
	u32 SlrType = XPlm_SsitCommGetSlrType();
	u32 IsCfgSecCommCmdTemp = IsCfgSecCommCmd;

	/* Fetch secure communication status, SSIT buffer address based on SLR type */
	if (SlrType == XPLMI_SSIT_MASTER_SLR) {
		SsitBufAddr = XPLMI_GET_MSGBUFF_ADDR(SlrIndex);
		SecSsitCommEst = XPlmi_SsitGetSecCommEstFlag(SlrIndex);
	}
	else {
		SsitBufAddr = XPLMI_SLR_EVENT_RESP_BUFFER_ADDR;
		SecSsitCommEst = XPlmi_SsitGetSecCommEstFlag(SECCOMM_SLAVE_INDEX);
		/* Keeping it as false to nullify effect of this flag in slave */
		IsCfgSecCommCmdTemp = FALSE;
	}

	/* Decide message type */
	MsgType = XPlm_SsitCommGetMsgType(SecSsitCommEst, IsCfgSecCommCmdTemp);

	/* Prepare parameters to encrypt data based on message type */
	if (MsgType == XPLM_SSIT_COMM_SECURE_MSG) {
		EncParams.SlrIndex = SlrIndex;
		EncParams.IsCfgSecCommCmd = IsCfgSecCommCmdTemp;
		XPlm_SsitCommPrepareEncParams(SecSsitCommEst, &EncParams, (u64)SsitBufAddr, Buf);
		Status = XPlm_SsitCommEncryptMsg(&EncParams);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	else {
		/* Copy data from buf to SSIT buf */
		Status = XPlmi_MemCpy64((u64)SsitBufAddr, (u64)(UINTPTR)Buf,
		                       (BufSize * XPLMI_WORD_LEN));
		if (Status != XST_SUCCESS) {
			Status =  XPLMI_SSIT_COPYTOSSITBUF_FAILED;
		}
	}
END:
	if(Status != XST_SUCCESS) {
		if (SlrType == XPLMI_SSIT_MASTER_SLR) {
			Status = XPlmi_UpdateStatus(XPLMI_SSIT_SECURE_COMM_FAIL_AT_MASTER_ENCRYPTION, Status);
		}
		else {
			Status = XPlmi_UpdateStatus(XPLMI_SSIT_SECURE_COMM_FAIL_AT_SLAVE_ENCRYPTION, Status);
		}
	}
	return Status;
}

/*************************************************************************************************/
/**
* @brief This function receives the message/response with or without decryption
*        using key and IV of given SlrNum based on CfgSecCommCmdFlag,SecSsitCommEst
*        flag and writes the received message/response to Buf
*
* @param Buf             is the address of buffer to which decrypted message/response is to be written
* @param BufSize         is the size of buffer
* @param SlrIndex        is the SLR number to which message/response has to be sent
* @param IsCfgSecCommCmd indicates whether the present message is configure secure communication
*                        command or not
*
* @return XST_SUCCESS on successful decryption
*         error code  on failure
*
**************************************************************************************************/
static int XPlm_SsitCommReceiveMessage(u32* Buf, u32 BufSize, u32 SlrIndex, u32 IsCfgSecCommCmd)
{
	u32 BufAddr;
	u64 SsitBufAddr;
	XPlmi_SecCommEstFlag SecSsitCommEst;
	XPlm_SsitCommParams DecParams;
	u32 MsgType;
	int Status = XST_FAILURE;
	/* Fetch SLR type */
	u32 SlrType = XPlm_SsitCommGetSlrType();
	u32 IsCfgSecCommCmdTemp = IsCfgSecCommCmd;

	/* Fetch secure communication status, SSIT buffer address based on SLR type */
	if (SlrType == XPLMI_SSIT_MASTER_SLR) {
		SsitBufAddr = XPlmi_SsitGetSlrAddr(XPLMI_SLR_EVENT_RESP_BUFFER_ADDR, (u8)SlrIndex);
		SecSsitCommEst = XPlmi_SsitGetSecCommEstFlag(SlrIndex);
		/* Keeping it as false to nullify effect of this in master */
		IsCfgSecCommCmdTemp = FALSE;
	}
	else {
		BufAddr = XPLMI_GET_MSGBUFF_ADDR(SlrIndex);
		SsitBufAddr = XPlmi_SsitGetSlrAddr(BufAddr, XPLMI_SSIT_MASTER_SLR_INDEX);
		SecSsitCommEst = XPlmi_SsitGetSecCommEstFlag(SECCOMM_SLAVE_INDEX);
	}

	/* Decide message type */
	MsgType = XPlm_SsitCommGetMsgType(SecSsitCommEst, IsCfgSecCommCmdTemp);

	/* Prepare parameters to encrypt data based on message type*/
	if (MsgType == XPLM_SSIT_COMM_SECURE_MSG) {
		DecParams.SlrIndex = SlrIndex;
		DecParams.IsCfgSecCommCmd = IsCfgSecCommCmdTemp;
		XPlm_SsitCommPrepareDecParams(SecSsitCommEst, &DecParams, SsitBufAddr, Buf);
		Status = XPlm_SsitCommDecryptMsg(&DecParams);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (SlrType == XPLMI_SSIT_MASTER_SLR) {
			XSecure_MemCpy64((u64)(UINTPTR)Buf, (u64)(UINTPTR)DecParams.TempRespBuf,
			                 (BufSize * XPLMI_WORD_LEN));
		}
	}
	else {
		/* Copy data from buf to ssit msg buf */
		Status = XPlmi_MemCpy64((u64)(UINTPTR)Buf, SsitBufAddr,
						(BufSize * XPLMI_WORD_LEN));
		if (Status != XST_SUCCESS) {
			Status =  XPLMI_SSIT_COPYFROMSSITBUF_FAILED;
		}
	}
END:
	if(Status != XST_SUCCESS) {
		if (SlrType == XPLMI_SSIT_MASTER_SLR) {
			Status = XPlmi_UpdateStatus(XPLMI_SSIT_SECURE_COMM_FAIL_AT_MASTER_DECRYPTION, Status);
		}
		else {
			Status = XPlmi_UpdateStatus(XPLMI_SSIT_SECURE_COMM_FAIL_AT_SLAVE_DECRYPTION, Status);
		}
	}
	return Status;
}
#else
/************************** Function Prototypes **************************************************/
static int XPlm_SsitCommSendMessage(u32* Buf, u32 BufSize, u32 SlrIndex, u32 IsCfgSecCommCmd);
static int XPlm_SsitCommReceiveMessage(u32* Buf, u32 BufSize, u32 SlrIndex, u32 IsCfgSecCommCmd);

/************************** Variable Definitions *************************************************/
static XPlmi_SsitCommFunctions XPlm_SsitCommFuncs = {.SendMessage = XPlm_SsitCommSendMessage,
											.ReceiveMessage = XPlm_SsitCommReceiveMessage,
											};

/*************************************************************************************************/
/**
* @brief This function sends the message/response in the Buf with or without encryption
*        using key and IV of given SlrNum based on CfgSecCommCmdFlag,SecSsitCommEst Flag
*
* @param Buf             is the address of buffer which contains message/response to be encrypted
* @param BufSize         is the size of buffer
* @param SlrIndex        is the SLR number to which message/response has to be sent
* @param IsCfgSecCommCmd indicates whether the present message is configure secure communication
*                        command or not
*
* @return XST_SUCCESS on successful encryption
*         error code on failure
*
**************************************************************************************************/
static int XPlm_SsitCommSendMessage(u32* Buf, u32 BufSize, u32 SlrIndex, u32 IsCfgSecCommCmd)
{
	u32 SsitBufAddr;
	int Status = XST_FAILURE;
	/* Fetch SLR type */
	u32 SlrType = XPlm_SsitCommGetSlrType();
	(void)IsCfgSecCommCmd;

	/* Fetch secure communication status, SSIT buffer address based on SLR type */
	if (SlrType == XPLMI_SSIT_MASTER_SLR) {
		SsitBufAddr = XPLMI_GET_MSGBUFF_ADDR(SlrIndex);
	}
	else {
		SsitBufAddr = XPLMI_SLR_EVENT_RESP_BUFFER_ADDR;
	}

	/* Copy data from buf to SSIT buf */
	Status = XPlmi_MemCpy64((u64)SsitBufAddr, (u64)(UINTPTR)Buf,
		                    (BufSize * XPLMI_WORD_LEN));
	return Status;
}


/*************************************************************************************************/
/**
* @brief This function receives the message/response with or without decryption
*        using key and IV of given SlrNum based on CfgSecCommCmdFlag,SecSsitCommEst
*        flag and writes the received message/response to Buf
*
* @param Buf             is the address of buffer to which decrypted message/response is to be written
* @param BufSize         is the size of buffer
* @param SlrIndex        is the SLR number to which message/response has to be sent
* @param IsCfgSecCommCmd indicates whether the present message is configure secure communication
*                        command or not
*
* @return XST_SUCCESS on successful decryption
*         error code  on failure
*
**************************************************************************************************/
static int XPlm_SsitCommReceiveMessage(u32* Buf, u32 BufSize, u32 SlrIndex, u32 IsCfgSecCommCmd)
{
	u32 BufAddr;
	u64 SsitBufAddr;
	int Status = XST_FAILURE;
	/* Fetch SLR type */
	u32 SlrType = XPlm_SsitCommGetSlrType();
	(void)IsCfgSecCommCmd;

	/* Fetch secure communication status, SSIT buffer address based on SLR type */
	if (SlrType == XPLMI_SSIT_MASTER_SLR) {
		SsitBufAddr = XPlmi_SsitGetSlrAddr(XPLMI_SLR_EVENT_RESP_BUFFER_ADDR, (u8)SlrIndex);
	}
	else {
		BufAddr = XPLMI_GET_MSGBUFF_ADDR(SlrIndex);
		SsitBufAddr = XPlmi_SsitGetSlrAddr(BufAddr, XPLMI_SSIT_MASTER_SLR_INDEX);
	}

	/* Copy data from buf to ssit msg buf */
	Status = XPlmi_MemCpy64((u64)(UINTPTR)Buf, SsitBufAddr,
						(BufSize * XPLMI_WORD_LEN));
	return Status;
}

#endif
/*************************************************************************************************/
/**
 * @brief	This function is used to get the pointer for SsitComm functions
 *
 * @return	Returns
 *		- Address of XPlm_SsitCommFuncs if secure PLM to PLM communictaion is enabled
 *		- NULL if secure PLM to PLM communictaion is not enabled
 *
 *************************************************************************************************/
XPlmi_SsitCommFunctions *XPlm_SsitCommGetFuncsPtr(void)
{
	/* Returning address of XPlm_SsitCommFuncs */
	return (&XPlm_SsitCommFuncs);
}
#endif