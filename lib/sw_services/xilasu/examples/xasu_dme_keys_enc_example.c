/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_dme_keys_enc_example.c
 * @addtogroup Overview
 * @{
 *
 * This example illustrates the usage of ASU OCP-DME challenge request client APIs.
 *
 * Procedure to link and compile the example for the default ddr less designs
 * ------------------------------------------------------------------------------------------------
 * The default linker settings places a software stack, heap and data in DDR memory.
 * For this example to work, any data shared between client running on A78/R52/PL and server
 * running on ASU, should be placed in area which is acccessible to both client and server.
 *
 * Following is the procedure to compile the example on any memory region which can be accessed
 * by the server.
 *
 *      1. Open ASU application linker script(lscript.ld) and there will be an memory
 *         mapping section which should be updated to point all the required sections
 *         to shared memory using a memory region selection
 *
 *                                      OR
 *
 *      1. In linker script(lscript.ld) user can add new memory section in source tab as shown below
 *              .sharedmemory : {
 *              . = ALIGN(4);
 *              __sharedmemory_start = .;
 *              *(.sharedmemory)
 *              *(.sharedmemory.*)
 *              *(.gnu.linkonce.d.*)
 *              __sharedmemory_end = .;
 *              } > Shared_memory_area
 *
 *      2. In this example ".data" section elements that are passed by reference to the server-side
 *         should be stored in the above shared memory section.
 *         Replace ".data" in attribute section with ".sharedmemory", as shown below-
 *      static u8 Data __attribute__ ((aligned (64U)) __attribute__ ((section (".data.Data")));
 *                              should be changed to
 *      static u8 Data __attribute__ ((aligned (64U))
 *						__attribute__ ((section (".sharedmemory.Data")));
 *
 * To keep things simple, by default the cache is disabled in this example using
 * XASU_ENABLE_CACHE macro.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  08/20/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xasu_client.h"
#include "xasu_ocp.h"
#include "xil_cache.h"
#include "xil_util.h"
#include "xasu_dme_keys_enc_input_example.h"
#include "xnvm_efuseclient.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_CACHE_DISABLE
#define XASU_VALUE_TWO		(2U)	/**< Value Two */
#define XASU_VALUE_EIGHT	(8U)	/**< Value Eight */

/************************************ Function Prototypes ****************************************/
static void XAsu_OcpDmeKeyCallBackRef(void *CallBackRef, u32 Status);
static void XAsu_OcpDmeKeyPrintData(const u8 *Data, u32 DataLen);
static s32 XAsu_OcpDmeEncryptData(XAsu_ClientParams *ClientParamPtr, char* Data, u32 DataLen,
	u8* OutputData, u32 DmeKeyId);
static s32 XAsu_OcpDmeEncNdPrgmDmeKey(XAsu_ClientParams *ClientParamPtr,
	XNvm_ClientInstance *NvmClientInstancePtr, u8 EncFlag, u8 PrgmFlag, char* DmePvtKey,
	u8 DmeKeyId);

/************************************ Variable Definitions ***************************************/
static u8 Notify = 0;			/**< To notify the call back from client library */
static u32 ErrorStatus = XST_FAILURE;	/**< Variable holds the status of the OCP-DME operation from
					client library and gets updated during callback. */
static u8 DmePvtKey[XASU_OCP_DME_KEY_SIZE_IN_BYTES] __attribute__ ((section (".data.DmePvtKey")));
static u8 EncDmeKey[XASU_OCP_DME_KEY_SIZE_IN_BYTES] __attribute__ ((section (".data.EncDmeKey")));

/************************************ Function Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This main function to perform DME private keys encryption and updates to efuses.
 *
 * @return
 *	- XST_SUCCESS, if example is run successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
int main(void)
{
	s32 Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XAsu_ClientParams ClientParam = {0U};
	XNvm_ClientInstance NvmClientInstance;

#ifdef	XASU_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	/** Initialize mailbox instance. */
	Status = (s32)XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Mailbox initialize failed: %08x \r\n", Status);
		goto END;
	}

	/** Initialize the client instance. */
	Status = XAsu_ClientInit(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

	Status = XNvm_ClientInit(&NvmClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("NVM Client initialization failed %x\r\n", Status);
		goto END;
	}

	/* Set Queue priority and register call back function. */
	ClientParam.Priority = XASU_PRIORITY_HIGH;
	ClientParam.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_OcpDmeKeyCallBackRef);
	ClientParam.CallBackRefPtr = (void *)&ClientParam;
	ClientParam.SecureFlag = XASU_CMD_SECURE;

	Status = XAsu_OcpDmeEncNdPrgmDmeKey(&ClientParam, &NvmClientInstance,
		XOCP_ENCRYPT_DME_PRIV_KEY_0, XOCP_PRGM_ENC_DME_PRIV_KEY_0,
		XOCP_DME_PRIV_KEY_0, XASU_OCP_DME_USER_KEY_0_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n OCP-DME Encrypt and Program DME Key 0 failed with Status = %08x",
			Status);
		goto END;
	}

	Status = XAsu_OcpDmeEncNdPrgmDmeKey(&ClientParam, &NvmClientInstance,
		XOCP_ENCRYPT_DME_PRIV_KEY_1, XOCP_PRGM_ENC_DME_PRIV_KEY_1,
		XOCP_DME_PRIV_KEY_1, XASU_OCP_DME_USER_KEY_1_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n OCP-DME Encrypt and Program DME Key 1 failed with Status = %08x",
			Status);
		goto END;
	}

	Status = XAsu_OcpDmeEncNdPrgmDmeKey(&ClientParam, &NvmClientInstance,
		XOCP_ENCRYPT_DME_PRIV_KEY_2, XOCP_PRGM_ENC_DME_PRIV_KEY_2,
		XOCP_DME_PRIV_KEY_2, XASU_OCP_DME_USER_KEY_2_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n OCP-DME Encrypt and Program DME Key 2 failed with Status = %08x",
			Status);
	}

END:
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n DME private keys enc and efuse update failed with Status = %08x",
			Status);
	} else if (ErrorStatus != XST_SUCCESS) {
		xil_printf("\r\n DME private keys enc and efuse update failed with error from server = %08x", ErrorStatus);
	} else {
		xil_printf("\r\n Successfully encrypted DME private keys and updated to efuses.");
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function encrypts the DME private keys and program them to efuses.
 *
 * @param	ClientParamsPtr		Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 * @param	NvmClientInstance	Pointer to XilNvm client instance.
 * @param	EncFlag			Flag to indicate whether to encrypt the key.
 * @param	PrgmFlag		Flag to indicate whether to program the key.
 * @param	DmePvtKey		Pointer to the DME private key data.
 * @param	DmeKeyId		ID of the DME key.
 *
 * @return
 *		- XST_SUCCESS - On successfully programming encrypted DME private keys
 *		- XST_FAILURE - On Failure.
 *
 *************************************************************************************************/
static s32 XAsu_OcpDmeEncNdPrgmDmeKey(XAsu_ClientParams *ClientParamPtr,
	XNvm_ClientInstance *NvmClientInstancePtr, u8 EncFlag, u8 PrgmFlag, char* DmePvtKey,
	u8 DmeKeyId)
{
	s32 Status = XST_FAILURE;

	if (EncFlag == TRUE) {
		Status = XAsu_OcpDmeEncryptData(ClientParamPtr, DmePvtKey, XASU_OCP_DME_KEY_SIZE_IN_BYTES,
			EncDmeKey, DmeKeyId);
		if(Status != XST_SUCCESS) {
			xil_printf("\r\n DME Priv Key 0 encryption failed");
			goto END;
		}
		else {
			xil_printf("\r\n Encrypted DME Private Key 0: ");
			XAsu_OcpDmeKeyPrintData((u8*)EncDmeKey, XASU_OCP_DME_KEY_SIZE_IN_BYTES);
		}
	}

	if (PrgmFlag == TRUE) {
		Status = XNvm_WriteDmePrivateKey(NvmClientInstancePtr, DmeKeyId, (u64)(UINTPTR)EncDmeKey, XOCP_ENV_MONITOR_DISABLE);
		if (Status != XST_SUCCESS) {
			xil_printf("\r\n Failed to program encrypted DME private key 0 into eFuses: Status = %08x", Status);
			goto END;
		}
		else {
			xil_printf("\r\n Successfully programmed encrypted DME private key 0 into eFuses");
		}
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function encrypts the data using DME KEK in ASUFW.
 *
 * @param	ClientParamPtr	Pointer to XAsu_ClientParams instance
 * @param	Data		Pointer to data to be encrypted
 * @param	DataLen		Length of data to be encrypted in bytes
 * @param	OutputData	Pointer to buffer where encrypted data will be stored
 * @param	DmeKeyId	Identifier for the DME key
 *
 * @return
 *		- XST_SUCCESS - if encryption was successful
 *		- XST_FAILURE - On failure of AES Encrypt Initialization,
 *			AES Encrypt data.
 *
 *************************************************************************************************/
static s32 XAsu_OcpDmeEncryptData(XAsu_ClientParams *ClientParamPtr, char* Data, u32 DataLen,
	u8* OutputData, u32 DmeKeyId)
{
	s32 Status = XST_FAILURE;
	XAsu_OcpDmeKeyEncrypt OcpDmeKeyEncParam;

	if (Xil_Strnlen(Data, (DataLen * XASU_VALUE_TWO)) == (DataLen * XASU_VALUE_TWO)) {
		Status = Xil_ConvertStringToHexBE((const char *)Data,
			DmePvtKey, DataLen * XASU_VALUE_EIGHT);
		if (Status != XST_SUCCESS) {
			xil_printf("\r\n String Conversion error (Data):%08x", Status);
			goto END;
		}
	}
	else {
		xil_printf("\r\n Provided input data length is wrong");
		goto END;
	}

	xil_printf("\r\n Data to be encrypted: ");
	XAsu_OcpDmeKeyPrintData(DmePvtKey, DataLen);

	Xil_DCacheFlushRange((UINTPTR)DmePvtKey, DataLen);

	ErrorStatus = XST_FAILURE;
	OcpDmeKeyEncParam.DmePvtKeyAddr = (u64)(UINTPTR)DmePvtKey;
	OcpDmeKeyEncParam.DmeEncPvtKeyAddr = (u64)(UINTPTR)OutputData;
	OcpDmeKeyEncParam.DmeKeyId = DmeKeyId;

	/** Generate DME Response. */
	Status = XAsu_OcpDmeKeysEncrypt(ClientParamPtr, &OcpDmeKeyEncParam);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("\r\n DME Challenge request failed:Status = %08x", Status);
		goto END;
	}

	/* Wait for the operation to be completed. */
	while (!Notify);
	Notify = 0;

	Xil_DCacheInvalidateRange((UINTPTR)OutputData, DataLen);

END:
	if ((ErrorStatus == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		XilAsu_Printf("\r\n OCP-DME private key encryption of Key ID %02x successful", DmeKeyId);
	} else {
		XilAsu_Printf("\r\n OCP-DME private key encryption of Key ID %02x failed", DmeKeyId);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function prints the data from given array on the console.
 *
 * @param	Data	Pointer to data array.
 * @param	DataLen	Length of the data to be printed on console.
 *
 *************************************************************************************************/
static void XAsu_OcpDmeKeyPrintData(const u8 *Data, u32 DataLen)
{
	u32 Index;

	for (Index = 0U; Index < DataLen; Index++) {
		XilAsu_Printf("%02x", Data[Index]);
	}
}

/*************************************************************************************************/
/**
 * @brief	Callback function which is registered with library to get request completion
 *		request.
 *
 * @param	CallBackRef	Pointer to the callback reference.
 * @param	Status		Status of the request is passed as an argument during callback.
 *
 *************************************************************************************************/
static void XAsu_OcpDmeKeyCallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;

	ErrorStatus = Status;

	/* Update the variable to notify the callback */
	Notify = 1U;
}
