/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/


/*************************************************************************************************/
/**
* @file xilnvm_bbram_versal_ssit_client_example.c
* @addtogroup xnvm_apis XilNvm APIs
* @{
* This file illustrates how to program AES keys of Versal BBRAM. The keys provided by
* XNVM_BBRAM_AES_KEY[XNVM_MAX_SLRS] are written to the BBRAM of master, slave SLR1, slave
* SLR2 and slave SLR3.
*
* To build this application, xilmailbox library must be included in BSP and
* xilnvm library must be in client mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- --------------------------------------------------------------------------
*   	pre     09/11/24 First release
*
* </pre>
*
*
* User configurable parameters for BBRAM
* -------------------------------------------------------------------------------------------------
* #define 		XNVM_BBRAM_AES_KEY_MASTER
* "0000000000000000000000000000000000000000000000000000000000000000"
* #define 		XNVM_BBRAM_AES_KEY_SLAVE_SLR1
* "0000000000000000000000000000000000000000000000000000000000000000"
* #define 		XNVM_BBRAM_AES_KEY_SLAVE_SLR2
* "0000000000000000000000000000000000000000000000000000000000000000"
* #define 		XNVM_BBRAM_AES_KEY_SLAVE_SLR3
* "0000000000000000000000000000000000000000000000000000000000000000"
* User should replace zeros with 256 bit AES key of master, slave SLR1, slave SLR2 and slave SLR3
* in XNVM_BBRAM_AES_KEY_MASTER, XNVM_BBRAM_AES_KEY_SLAVE_SLR1, XNVM_BBRAM_AES_KEY_SLAVE_SLR2 and
* XNVM_BBRAM_AES_KEY_SLAVE_SLR3 macros respectively in hexadecimal string format. Also should set
* the key length in bits to macro XNVM_BBRAM_AES_KEY_LEN_IN_BITS.
*
* #define 		XNVM_BBRAM_USER_DATA_MASTER    	   0x00000000
* #define 		XNVM_BBRAM_USER_DATA_SLAVE_SLR1    0x00000000
* #define 		XNVM_BBRAM_USER_DATA_SLAVE_SLR2    0x00000000
* #define 		XNVM_BBRAM_USER_DATA_SLAVE_SLR3    0x00000000
* User should replace zeros with 32 bit value for master, slave SLR1, slave SLR2 and slave SLR3 in
* XNVM_BBRAM_USER_DATA_MASTER, XNVM_BBRAM_USER_DATA_SLAVE_SLR1, XNVM_BBRAM_USER_DATA_SLAVE_SLR2 and
* XNVM_BBRAM_USER_DATA_SLAVE_SLR3 macros respectively in hexadecimal.
*
* #define 		XNVM_BBRAM_AES_KEY_LEN_IN_BITS		(256U)
* User should replace this value based on length of the AES Key defined
* by macro XNVM_BBRAM_AES_KEY. Supported values - 256
*
* This example is supported for Versal devices.
*
* Procedure to link and compile the example for the default ddr less designs
* -------------------------------------------------------------------------------------------------
* The default linker settings places a software stack, heap and data in DDR memory. For this
* example to work, any data shared between client running on A72/R5/PL and server running on PMC,
* should be placed in area which is accessible to both client and server.
*
* Following is the procedure to compile the example on OCM or any memory region which can be
* accessed by server
*
*	 1. Open example linker script(lscript.ld) in Vitis project and section to memory mapping
*		should be updated to point all the required sections to shared memory(OCM or TCM)
*		using a memory region drop down selection
*
*						OR
*
*	 1. In linker script(lscript.ld) user can add new memory section in source tab as shown below
*		.sharedmemory : {
*   		. = ALIGN(4);
*   		__sharedmemory_start = .;
*   		*(.sharedmemory)
*   		*(.sharedmemory.*)
*   		*(.gnu.linkonce.d.*)
*   		__sharedmemory_end = .;
* 		} > versal_cips_0_pspmc_0_psv_ocm_ram_0_psv_ocm_ram_0
*
* 	 2. In this example, ".data" section elements that are passed by reference to the server side
* 		should be stored in the above shared memory section. To make it happen in below example,
* 		replace ".data" in attribute section with ".sharedmemory". For example,
* 		static SharedMem[XNVM_SHARED_MEM_SIZE] __attribute__((aligned(64U)))
* 		__attribute__((section(".data.SharedMem")));
* 		should be changed to
* 		static SharedMem[XNVM_SHARED_MEM_SIZE] __attribute__((aligned(64U)))
* 		__attribute__((section(".sharedmemory.SharedMem")));
*
* To keep things simple, by default the cache is disabled for this example
*
**************************************************************************************************/

/***************************** Include Files *****************************************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xnvm_bbramclient.h"

/***************** Macros (Inline Functions) Definitions *****************************************/
#define XNVM_MAX_SLR_NUM XNVM_SLR_INDEX_3 /**< Max SLR number */

/* AES Key of master SLR */
#define XNVM_BBRAM_AES_KEY_MASTER \
             "0000000000000000000000000000000000000000000000000000000000000000"
/* AES Key of slave SLR1 */
#define XNVM_BBRAM_AES_KEY_SLAVE_SLR1 \
             "0000000000000000000000000000000000000000000000000000000000000000"
/* AES Key of slave SLR2 */
#define XNVM_BBRAM_AES_KEY_SLAVE_SLR2 \
             "0000000000000000000000000000000000000000000000000000000000000000"
/* AES Key of slave SLR3 */
#define XNVM_BBRAM_AES_KEY_SLAVE_SLR3 \
             "0000000000000000000000000000000000000000000000000000000000000000"

/* User data of master SLR */
#define XNVM_BBRAM_USER_DATA_MASTER	    0x00000000
/* User data of slave SLR1 */
#define XNVM_BBRAM_USER_DATA_SLAVE_SLR1	0x00000000
/* User data of slave SLR2 */
#define XNVM_BBRAM_USER_DATA_SLAVE_SLR2	0x00000000
/* User data of slave SLR3 */
#define XNVM_BBRAM_USER_DATA_SLAVE_SLR3	0x00000000

#define XNVM_MAX_SLRS 4U
#define XNVM_BBRAM_AES_KEY_LEN_IN_BITS	(256U) /**< AES key length in Bits */

/* Key Length of AES Key defined by XNVM_BBRAM_AES_KEY */
#define XNVM_BBRAM_AES_KEY_LEN_IN_BYTES	(XNVM_BBRAM_AES_KEY_LEN_IN_BITS / 8U)
					/**< AES key length in Bytes */

#define XNVM_BBRAM_AES_KEY_STR_LEN	(XNVM_BBRAM_AES_KEY_LEN_IN_BITS / 4U)
					/**< String length of Key */

#define XNVM_256_BITS_AES_KEY_LEN_IN_BYTES (256U / XIL_SIZE_OF_BYTE_IN_BITS)
#define XNVM_256_BITS_AES_KEY_LEN_IN_CHARS (XNVM_256_BITS_AES_KEY_LEN_IN_BYTES * 2U)
#define XNVM_128_BITS_AES_KEY_LEN_IN_BYTES (128U / XIL_SIZE_OF_BYTE_IN_BITS)
#define XNVM_128_BITS_AES_KEY_LEN_IN_CHARS (XNVM_128_BITS_AES_KEY_LEN_IN_BYTES * 2U)

#define XNVM_MAX_AES_KEY_LEN_IN_CHARS	XNVM_256_BITS_AES_KEY_LEN_IN_CHARS

#define XNVM_CACHE_LINE_LEN		(64U)

/**************************** Type Definitions ***************************************************/


/************************** Variable Definitions *************************************************/
static const char XNVM_BBRAM_AES_KEY[XNVM_MAX_SLRS][XNVM_BBRAM_AES_KEY_STR_LEN + 1U]
__attribute__((aligned(64U)))__attribute__((section(".data.XNVM_BBRAM_AES_KEY"))) = {
	XNVM_BBRAM_AES_KEY_MASTER,
	XNVM_BBRAM_AES_KEY_SLAVE_SLR1,
	XNVM_BBRAM_AES_KEY_SLAVE_SLR2,
	XNVM_BBRAM_AES_KEY_SLAVE_SLR3
};

static const u32 XNVM_BBRAM_USER_DATA[XNVM_MAX_SLRS]__attribute__((aligned(64U)))
		__attribute__((section(".data.XNVM_BBRAM_USER_DATA"))) = {
	XNVM_BBRAM_USER_DATA_MASTER,
	XNVM_BBRAM_USER_DATA_SLAVE_SLR1,
	XNVM_BBRAM_USER_DATA_SLAVE_SLR2,
	XNVM_BBRAM_USER_DATA_SLAVE_SLR3
};

/*
 * if cache is enabled, User need to make sure the data is aligned to cache line
 */

/* shared memory allocation */
static u8 SharedMem[XNVM_SHARED_MEM_SIZE] __attribute__((aligned(64U)))
		__attribute__((section(".data.SharedMem")));

/************************** Function Prototypes **************************************************/
static int BbramWriteAesKey(XNvm_ClientInstance *InstancePtr, u32 SlrIndex);
static int XNvm_ValidateAesKey(const char *Key);
static int BbramWriteUsrData(XNvm_ClientInstance *InstancePtr, u32 SlrIndex);
static int BbramReadUsrData(XNvm_ClientInstance *InstancePtr);
static int BbramLockUsrData(XNvm_ClientInstance *InstancePtr);
static int XNvm_InputSlrIndex(XNvm_ClientInstance *InstancePtr, u32 SlrIndex);

/*************************************************************************************************/
/**
*
* Main function to call the Bbram AES key write example function.
*
* @return
*		- XST_FAILURE if the BBRAM programming failed.
*
* @note		By default PLM does not include the NVM client code, it is
* 		disabled by a macro PLM_NVM_EXCLUDE. So, to run this
* 		client application successfully we need to enable NVM code in
* 		PLM before executing this application.
*
**************************************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XNvm_ClientInstance NvmClientInstance;

	xil_printf("BBRAM AES Key writing SSIT client example for Versal\n\r");

	#ifdef XNVM_CACHE_DISABLE
		Xil_DCacheDisable();
	#endif

	#ifndef SDT
	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	#else
	Status = XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	#endif
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_ClientInit(&NvmClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Set shared memory */
	Status = XMailbox_SetSharedMem(&MailboxInstance, (u64)(UINTPTR)&SharedMem[0U],
			XNVM_SHARED_MEM_SIZE);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n shared memory initialization failed");
		goto END;
	}

	Status = XNvm_BbramZeroize(&NvmClientInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n Zeroization failed");
		goto END;
	}

	for (u32 SlrIndex = XNVM_SLR_INDEX_0; SlrIndex <= XNVM_MAX_SLR_NUM; SlrIndex++) {
		Status = XNvm_InputSlrIndex(&NvmClientInstance, SlrIndex);
		if (Status != XST_SUCCESS) {
			xil_printf("invalid SlrIndex \r\n");
			goto END;
		}

		/**
		 * If any error occurs during programming of any SLR, keys in all the SLRs get zeroized.
		 * User has to reload the example
		 */
		Status = BbramWriteAesKey(&NvmClientInstance, SlrIndex);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = BbramWriteUsrData(&NvmClientInstance, SlrIndex);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = BbramReadUsrData(&NvmClientInstance);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = BbramLockUsrData(&NvmClientInstance);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

END:
	if (XST_SUCCESS == Status) {
		xil_printf("Successfully ran Versal SSIT BBRAM Client example....\n\r");
	}
	else {
		xil_printf("BBRAM programming failed with error code = %08x\n\r",
		          Status);
	}

	return Status;
}

/*************************************************************************************************/
/**
* BbramWriteAesKey function writes AES key defined by macro XNVM_BBRAM_AES_KEY
* in BBRAM.
*
* @param	InstancePtr Pointer to the client instance
* @param    SlrIndex SLR index to which key has to be written
*
* @return
*		- XST_SUCCESS if the Aes key write successful
*		- XST_FAILURE if the Aes key write failed
*
**************************************************************************************************/
static int BbramWriteAesKey(XNvm_ClientInstance *InstancePtr, u32 SlrIndex)
{
	int Status = XST_FAILURE;
	u8 *AesKey = &SharedMem[0U];

	/* Validate the key */
	Status = XNvm_ValidateAesKey(&XNVM_BBRAM_AES_KEY[SlrIndex][0U]);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	/* Convert key given in macro and assign it to the variable */
	Xil_ConvertStringToHexLE(&XNVM_BBRAM_AES_KEY[SlrIndex][0U],
				AesKey, XNVM_BBRAM_AES_KEY_LEN_IN_BITS);

	Xil_DCacheFlushRange((UINTPTR)AesKey, XNVM_BBRAM_AES_KEY_LEN_IN_BYTES);

	/* Write AES key to BBRAM */
	Status = XNvm_BbramWriteAesKey(InstancePtr, (UINTPTR)AesKey,
				       XNVM_BBRAM_AES_KEY_LEN_IN_BYTES);

END:
	return Status;
}

/*************************************************************************************************/
/**
* BbramWriteUsrData function writes user data defined by macro
* XNVM_BBRAM_USER_DATA in BBRAM.
*
* @param	InstancePtr Pointer to the client instance
* @param    SlrIndex SLR index to which user data has to be written
*
* @return
*		- XST_SUCCESS if the user data write successful
*		- XST_FAILURE if the user data write failed
*
**************************************************************************************************/
static int BbramWriteUsrData(XNvm_ClientInstance *InstancePtr, u32 SlrIndex)
{
	int Status = XST_FAILURE;

	Status = XNvm_BbramWriteUsrData(InstancePtr, XNVM_BBRAM_USER_DATA[SlrIndex]);

	return Status;
}

/*************************************************************************************************/
/**
* BbramReadUsrData function reads user data present in BBRAM
*
* @return
*		- XST_SUCCESS if the user data read successful
*		- XST_FAILURE if the user data read failed
*
**************************************************************************************************/
static int BbramReadUsrData(XNvm_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
	u32 *UsrData = (u32*)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)UsrData, XNVM_CACHE_LINE_LEN);
	Status = XNvm_BbramReadUsrData(InstancePtr, (UINTPTR)UsrData);
	Xil_DCacheInvalidateRange((UINTPTR)UsrData, XNVM_CACHE_LINE_LEN);

	if (Status == XST_SUCCESS) {
		xil_printf("Bbram UserData : %x\n\r", *UsrData);
	}

	return Status;
}

/*************************************************************************************************/
/**
* BbramLockUsrData function locks  user data written in BBRAM
*
* @return
*		- XST_SUCCESS if the user data lock successful
*		- XST_FAILURE if the user data lock failed
*
**************************************************************************************************/
static int BbramLockUsrData(XNvm_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;

	Status = XNvm_BbramLockUsrDataWrite(InstancePtr);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Validate the input string contains valid AES key.
 *
 * @param   	Key - Pointer to AES key.
 *
 *@return	- XST_SUCCESS - On valid input AES key string.
 *		- XST_INVALID_PARAM - On invalid length of the input string.
 *		- XST_FAILURE	- On non hexadecimal character in string
 *
**************************************************************************************************/
static int XNvm_ValidateAesKey(const char *Key)
{
	int Status = XST_INVALID_PARAM;
	u32 Len;

	if(NULL == Key) {
		goto END;
	}

	Len = Xil_Strnlen(Key, XNVM_MAX_AES_KEY_LEN_IN_CHARS + 1U);

	if ((Len != XNVM_256_BITS_AES_KEY_LEN_IN_CHARS) &&
		(Len != XNVM_128_BITS_AES_KEY_LEN_IN_CHARS)) {
		goto END;
	}

	Status = (int)Xil_ValidateHexStr(Key);
END:
	return Status;
}
/*************************************************************************************************/
/**
 * @brief	Adds the SLR Index.
 *
 * @param  InstancePtr is a pointer to instance XNvm_ClientInstance
 *
 * @param   SlrIndex - Number for slrId
 *
 *@return	- XST_SUCCESS - On valid input SlrIndex.
 *		    - XST_FAILURE - On non valid input SlrIndex
 *
 **************************************************************************************************/
static int XNvm_InputSlrIndex(XNvm_ClientInstance *InstancePtr, u32 SlrIndex)
{
	int Status = XST_FAILURE;

	if (SlrIndex <= XNVM_MAX_SLR_NUM) {
		InstancePtr->SlrIndex = SlrIndex;
	    Status = XST_SUCCESS;
	}

	return Status;
}
/** //! [XNvm BBRAM example] */
/** @} */
