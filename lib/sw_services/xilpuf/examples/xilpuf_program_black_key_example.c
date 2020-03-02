/******************************************************************************
 * *
 * * Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
 * *
 * * Permission is hereby granted, free of charge, to any person obtaining a
 * * copy of this software and associated documentation files (the "Software"),
 * * to deal in the Software without restriction, including without limitation
 * * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * * and/or sell copies of the Software, and to permit persons to whom the
 * * Software is furnished to do so, subject to the following conditions:
 * *
 * * The above copyright notice and this permission notice shall be included in
 * * all copies or substantial portions of the Software.
 * *
 * * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
 * * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * * DEALINGS IN THE SOFTWARE.
 * *
 * *
 * *
******************************************************************************/
/*****************************************************************************/
/**
  *
  *@file xilpuf_program_black_key_example.c
  *
  *This file illustrates encryption and decryption of red key using PUF KEK and
  *programming the black key and helper data in a user specified location
  *
  *<pre>
  *MODIFICATION HISTORY:
  *
  *Ver   Who   Date     Changes
  *----- ---  -------- -------------------------------------------------------
  *1.0	har   01/30/2019 Initial release
  *
  *@note
  *
* User configurable parameters for PUF
*------------------------------------------------------------------------------
*	#define XPUF_RED_KEY	"000000000000000000000000"
*	Red Key to be encrypted by PUF KEK should be provided in string format
*
*	#define XPUF_RED_KEY_LEN_IN_BYTES		(0U)
*	Length of red key to be encrypted should be provided in bytes
*
*	#define XPUF_IV		"000000000000000000000000"
*	IV should be provided in string format

*	#define XPUF_KEK_GENERATE_OPTION	(XPUF_REGISTRATION)
*							(or)
*						(XPUF_REGEN_ON_DEMAND)
*	XPUF_KEK_GENERATE_OPTION can be any value among above two options
*
*	#define XPUF_WRITE_OPTION		(XPUF_WRITE_INTO_RAM)
*							(or)
*						(XPUF_WRITE_INTO_EFUSE)
*	This selects the location where helper data must be written by the
*	application. This option must be configured if XPUF_KEK_GENERATE_OPTION
*	is configured as XPUF_REGISTRATION.
*
*	#define XPUF_READ_OPTION		(XPUF_READ_FROM_RAM)
*							(or)
*						(XPUF_READ_FROM_EFUSE_CACHE)
*	This selects the location from where the helper data must be read by the
*	application. This option must be configured if XPUF_KEK_GENERATE_OPTION
*	is configured as XPUF_REGEN_ON_DEMAND.
*
 *	#define XPUF_RAM_CHASH		(0x00000000)
*	CHASH value should be supplied if XPUF_READ_OPTION is configured as
*	XPUF_READ_FROM_RAM
*
*	#define XPUF_RAM_AUX		(0x00000000)
*	AUX value should be supplied if XPUF_READ_OPTION is configured as
*	XPUF_READ_FROM_RAM
*
*	#define XPUF_SYN_DATA_ADDRES	(0x00000000)
*	Address of syndrome data should be supplied if XPUF_READ_OPTION is
*	configured as XPUF_READ_FROM_RAM.
*
*	#define XPUF_WRITE_BLACK_KEY_OPTION	(XPUF_WRITE_INTO_EFUSE)
*							(or)
*						(XPUF_WRITE_INTO_BBRAM)
*	This selects the location where the black key must be programmed.
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xpuf.h"
#include "xsecure_aes.h"
#include "xnvm_bbram.h"
#include "xnvm_efuse.h"
#include "xil_util.h"
#include "xil_mem.h"
/************************** Constant Definitions ****************************/
#define XPUF_CSUDMA_DEVICEID	XPAR_XCSUDMA_0_DEVICE_ID

#define XPUF_RED_KEY "17d27863dc6b1872f0d64e851c1b0a63a00c511ae62b747d14e1ad5a0feaada8"

#define XPUF_RED_KEY_LEN_IN_BYTES			(32U)
						/* Red Key length in Bytes */
#define XPUF_IV	"D2450E07EA5DE0426C0FA133"

#define XPUF_KEK_GENERATE_OPTION		(XPUF_REGISTRATION)

#define XPUF_WRITE_OPTION 			(XPUF_WRITE_INTO_RAM)

#define XPUF_READ_OPTION			(XPUF_READ_FROM_RAM)

#define XPUF_RAM_CHASH				(0x00000000U)
#define XPUF_RAM_AUX				(0x00000000U)
#define XPUF_SYN_DATA_ADDRESS			(0x00000000U)

#define XPUF_WRITE_BLACK_KEY_OPTION		(XPUF_WRITE_INTO_EFUSE)

#define XPUF_IV_LEN_IN_BYTES				(12U)
						/* IV Length in bytes */
#define XPUF_GCM_TAG_SIZE			(XSECURE_SECURE_GCM_TAG_SIZE)
						/* GCM tag Length in bytes */
#define XPUF_HELPER_DATA_LEN_IN_BYTES		(386U)
#define XPUF_DEBUG_INFO				(1U)

/************************** Type Definitions ********************************/
static XPuf_Data PufData;
static Xnvm_PufHelperData PrgmPufHelperData;

static u8 Iv[XPUF_IV_LEN_IN_BYTES];
#if defined (__GNUC__)
static u8 RedKey[XPUF_RED_KEY_LEN_IN_BYTES]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.RedKey")));
static u8 BlackKey[XPUF_RED_KEY_LEN_IN_BYTES]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.BlackKey")));
static u8 GcmTag[XPUF_GCM_TAG_SIZE]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.GcmTag")));
#elif defined (__ICCARM__)
#pragma data_alignment = 64
static u8 RedKey[XPUF_RED_KEY_LEN_IN_BYTES];
#pragma data_alignment = 64
static u8 DecRedKey[XPUF_RED_KEY_LEN_IN_BYTES];
#pragma data_alignment = 64
static u8 BlackKey[XPUF_RED_KEY_LEN_IN_BYTES];
#pragma data_alignment = 64
static u8 GcmTag[XPUF_GCM_TAG_SIZE];
#endif

/************************** Function Prototypes ******************************/
static u32 ConvertStringToHexBE(const char * Str, u8 * Buf, u32 Len);
static s32 XPuf_KekGenerationExample();
static s32 XPuf_EncryptRedKeyExample(void);
static s32 XPuf_ProgramBlackKeyExample(void);

/************************** Function Definitions *****************************/
int main()
{
	int Status = (u32)XST_FAILURE;

	/* Generate PUF KEK
	 */
	Status = XPuf_KekGenerationExample();
	if (Status == XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO,"\r\n Successfully generated "
					"PUF KEK %x", Status);
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO,"\r\n PUF KEK generation failed %x",
					Status);
		goto END;
	}

	/* Encrypt red key using PUF KEK
	 */
	Status = XPuf_EncryptRedKeyExample();
	if (Status == XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO,"\r\nSuccessfully encrypted red key"
				"%x", Status);
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO,"\r\nEncryption/Decryption failed"
				"%x", Status);
		goto END;
	}

	/* Program black key and helper data into NVM
	 */
	Status = XPuf_ProgramBlackKeyExample();
	if (Status == XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO,"\r\n Successfully programmed Black"
				"key %x", Status);
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO,"\r\n Programming into NVM"
				"failed %x", Status);
	}
END:
	return Status;
}
/****************************************************************************/
/**
 * *
 * * This function generates PUF KEK by PUF registration or PUF on demand
 * * regeneration as per the user provided inputs.
 * *
 * * @param	None
 * *
 * * @return
 * *		- XST_SUCCESS if PUF_KEK generation was successful
 * *		- XST_FAILURE if PUF KEK generation failed
 * *
 * * @note	None.
 * *
 * ****************************************************************************/
static s32 XPuf_KekGenerationExample()
{
	s32 Status = XST_FAILURE;
	u32 Subindex;
	u8 *Buffer;
	u32 SynIndex;
	u32 Idx;

	PufData.RegMode = XPUF_SYNDROME_MODE_4K;
	PufData.ShutterValue = XPUF_SHUTTER_VALUE;
	PufData.PufOperation = XPUF_KEK_GENERATE_OPTION;
	u32 PUF_HelperData[XPUF_HELPER_DATA_LEN_IN_BYTES] = {0};

	if (PufData.PufOperation == XPUF_REGISTRATION) {
		PufData.WriteOption = XPUF_WRITE_OPTION;
		Status = XPuf_Registration(&PufData);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		xPuf_printf(XPUF_DEBUG_INFO,"PUF Helper data Start!!!\r\n");
			Xil_MemCpy(PUF_HelperData,PufData.SyndromeData,
				XPUF_4K_PUF_SYN_LEN_IN_WORDS * sizeof(u32));
			PUF_HelperData[XPUF_HELPER_DATA_LEN_IN_BYTES - 2] =PufData.Chash;
			PUF_HelperData[XPUF_HELPER_DATA_LEN_IN_BYTES - 1] = PufData.Aux;

			for (SynIndex = 0; SynIndex < XPUF_HELPER_DATA_LEN_IN_BYTES; SynIndex++) {
				Buffer = (u8*) &(PUF_HelperData[SynIndex]);
				for (Subindex = 0; Subindex < 4; Subindex++) {
					xPuf_printf(XPUF_DEBUG_INFO,"%02x", Buffer[Subindex]);
				}
			}
			xPuf_printf(XPUF_DEBUG_INFO,"\r\n");
			xPuf_printf(XPUF_DEBUG_INFO,"PUF Helper data End\r\n");

			xPuf_printf(XPUF_DEBUG_INFO,"PUF ID : ");
			for (Idx = 0; Idx < XPUF_ID_LENGTH; Idx++) {
				xPuf_printf(XPUF_DEBUG_INFO,"%02x", PufData.PufID[Idx]);
			}
			xPuf_printf(XPUF_DEBUG_INFO,"\r\n");

		if(PufData.WriteOption == XPUF_WRITE_INTO_RAM) {
			xPuf_printf(XPUF_DEBUG_INFO,"\r\nProvided PUF helper "
						"for writing in DDR");
		}
		else if(PufData.WriteOption == XPUF_WRITE_INTO_EFUSE) {
			XPuf_GenerateFuseFormat(&PufData);
			xPuf_printf(XPUF_DEBUG_INFO,"\r\nFormatted syndrome "
					"data written in eFuse");
			for (SynIndex = 0; SynIndex < XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS; SynIndex++) {
				Buffer = (u8*) &(PufData.EfuseSynData[SynIndex]);
				for (Subindex = 0; Subindex < 4; Subindex++) {
					xPuf_printf(XPUF_DEBUG_INFO,"%02x", Buffer[Subindex]);
				}
			}
			Xil_MemCpy(PrgmPufHelperData.EfuseSynData,
				PufData.EfuseSynData,
				XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS * sizeof(u32));

			PrgmPufHelperData.Chash = PufData.Chash;
			PrgmPufHelperData.Aux = PufData.Aux;

			PrgmPufHelperData.PrgmSynData = TRUE;
			PrgmPufHelperData.PrgmChash = TRUE;
			PrgmPufHelperData.PrgmAux = TRUE;

			Status = XNvm_EfuseWritePufHelperData(&PrgmPufHelperData);
			if (Status != XST_SUCCESS)
			{
				xPuf_printf(XPUF_DEBUG_INFO,"Programming Helper data"
						 "into eFUSE failed");
			}
			xPuf_printf(XPUF_DEBUG_INFO,"\r\n PUF helper data "
					"written in eFUSE");
		}
		else {
			xPuf_printf(XPUF_DEBUG_INFO,"Invalid option selected"
						" to write Syndrome Data");
		}
	}
	else if (PufData.PufOperation == XPUF_REGEN_ON_DEMAND) {
		PufData.ReadOption = XPUF_READ_OPTION;
		if (PufData.ReadOption == XPUF_READ_FROM_RAM) {
				PufData.Chash = XPUF_RAM_CHASH;
				PufData.Aux = XPUF_RAM_AUX;
				PufData.SyndromeAddr = XPUF_SYN_DATA_ADDRESS;
				xPuf_printf(XPUF_DEBUG_INFO,"\r\nReading helper"
						"data from DDR");
		}
		else if (PufData.ReadOption == XPUF_READ_FROM_EFUSE_CACHE) {
			xPuf_printf(XPUF_DEBUG_INFO,"\r\nReading helper data "
					"from eFUSE");
		}
		else {
			xPuf_printf(XPUF_DEBUG_INFO,"Invalid read option for "
				"reading helper data");
			goto END;
		}
		Status = XPuf_Regeneration(&PufData);
		if (Status != XST_SUCCESS) {
			xPuf_printf(XPUF_DEBUG_INFO,"Puf Regeneration failed"
			"	with error:%x", Status);
		}
	}
	else {
	/* PUF KEK is generated by Registration and On-demand Regeneration only
 * 	   ID only regeneration cannot be used for generating PUF KEK */
		Status = XPUF_ERROR_INVALID_PUF_OPERATION;
	}
END:
	return Status;
}
/****************************************************************************/
/**
 * @brief
 * This function encrypts the red key with PUF KEK and IV
 *
 * @param	None
 *
 * @return
 *		- XST_SUCCESS if encryption was successful
 *		- error code if encryption failed
 *
 * @note		None.
 *
 ****************************************************************************/

static s32  XPuf_EncryptRedKeyExample(void)
{
	XCsuDma_Config *Config;
	s32 Status = XST_FAILURE;
	u32 Index;
	XCsuDma CsuDmaInstance;
	XSecure_Aes SecureAes;

	Xil_DCacheDisable();

	if (Xil_Strnlen(XPUF_IV, (XPUF_IV_LEN_IN_BYTES * 2)) ==
				(XPUF_IV_LEN_IN_BYTES * 2)) {
		Status = ConvertStringToHexBE((const char *)(XPUF_IV), Iv,
						XPUF_IV_LEN_IN_BYTES * 2);
		if (Status != XST_SUCCESS) {
			xPuf_printf(XPUF_DEBUG_INFO,
			"String Conversion error (IV):%08x !!!\r\n", Status);
			goto END;
		}
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO,"Provided IV length is wrong");
		goto END;
	}

	if (Xil_Strnlen(XPUF_RED_KEY, (XPUF_RED_KEY_LEN_IN_BYTES * 2)) ==
				(XPUF_RED_KEY_LEN_IN_BYTES * 2)) {
		Status = ConvertStringToHexBE(
			(const char *) (XPUF_RED_KEY),
				RedKey, XPUF_RED_KEY_LEN_IN_BYTES * 2);
		if (Status != XST_SUCCESS) {
			xPuf_printf(XPUF_DEBUG_INFO,
			"String Conversion error (Red Key):%08x !!!\r\n", Status);
			goto END;
		}
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO,"Provided red key length is wrong");
		goto END;
	}

	/*Initialize CSU DMA driver */
	Config = XCsuDma_LookupConfig(XPUF_CSUDMA_DEVICEID);
	if (Config == NULL) {
		goto END;
	}

	Status = XCsuDma_CfgInitialize(&CsuDmaInstance, Config,
					Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Initialize the Aes driver so that it's ready to use */
	XSecure_AesInitialize(&SecureAes, &CsuDmaInstance);

	/* Take core out of reset */
	XSecure_ReleaseReset(SecureAes.BaseAddress,
				XSECURE_AES_SOFT_RST_OFFSET);

	xPuf_printf(XPUF_DEBUG_INFO,"Red Key to be encrypted: \n\r");
	for (Index = 0; Index < XPUF_RED_KEY_LEN_IN_BYTES; Index++) {
		xPuf_printf(XPUF_DEBUG_INFO,"%02x", RedKey[Index]);
	}
	xPuf_printf(XPUF_DEBUG_INFO,"\r\n\n");

	/* Encryption of Red Key */

	Status = XSecure_AesEncryptInit(&SecureAes, XSECURE_AES_PUF_KEY,
				XSECURE_AES_KEY_SIZE_256, (u64)Iv);
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO," Aes encrypt init is failed "
			"%x\n\r", Status);
		goto END;
	}

	Status = XSecure_AesEncryptUpdate(&SecureAes, (u64)RedKey,
			(u64)BlackKey, XPUF_RED_KEY_LEN_IN_BYTES, TRUE);
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO," Aes encrypt update is failed"
			"%x\n\r", Status);
		goto END;
	}

	Status = XSecure_AesEncryptFinal(&SecureAes, (u64)GcmTag);
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO,"Aes encryption failed\n\r%x",
				Status);
		goto END;
	}

	xPuf_printf(XPUF_DEBUG_INFO,"Black Key: \n\r");
	for (Index = 0; Index < XPUF_RED_KEY_LEN_IN_BYTES; Index++) {
		xPuf_printf(XPUF_DEBUG_INFO,"%02x", BlackKey[Index]);
	}
	xPuf_printf(XPUF_DEBUG_INFO,"\r\n");

	xPuf_printf(XPUF_DEBUG_INFO,"GCM tag: \n\r");
	for (Index = 0; Index < XPUF_GCM_TAG_SIZE; Index++) {
		xPuf_printf(XPUF_DEBUG_INFO,"%02x", GcmTag[Index]);
	}
	xPuf_printf(XPUF_DEBUG_INFO,"\r\n\n");

END:
	return Status;
}
/****************************************************************************/
/**
 * @brief
 * This function programs black key into efuse or BBRAM
 *
 * @param	None
 *
 * @return
 *			- XST_SUCCESS if programming was successful
 *			- XST_FAILURE if programming failed.
 *
 * @note	None.
 *
****************************************************************************/
static s32 XPuf_ProgramBlackKeyExample(void)
{
	u32 Status = XST_FAILURE;
	Xnvm_EfuseWriteData WriteData;
	u32 BlackKeyWriteOption = XPUF_WRITE_BLACK_KEY_OPTION;

	if (BlackKeyWriteOption == XPUF_WRITE_INTO_BBRAM) {
		Status = XNvm_BbramWriteAesKey((u8 *)BlackKey,
					 XNVM_EFUSE_AES_KEY_LEN_IN_BYTES);
		if (Status != XST_SUCCESS) {
			xPuf_printf(XPUF_DEBUG_INFO,"Error in programming "
					"Black key to BBRAM %x", Status);
		}
	}
	else if (BlackKeyWriteOption == XPUF_WRITE_INTO_EFUSE) {
		WriteData.CheckWriteFlags.AesKey = TRUE;
		Xil_MemCpy(WriteData.AesKey,BlackKey,
				XNVM_EFUSE_AES_KEY_LEN_IN_BYTES);
		Status = XNvm_EfuseWrite(&WriteData);
		if (Status != XST_SUCCESS) {
			xPuf_printf(XPUF_DEBUG_INFO,"Error in programming Black"
				"key to eFuse %x", Status);
		}
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO, "Incorrect option selected to"
				"write black key");
	}

	return Status;
}
/****************************************************************************/
/**
 * @brief
 * Converts the string into the equivalent Hex buffer.
 * Ex: "abc123" -> {Buf[2] = 0x23, Buf[1] = 0xc1, Buf[0] = 0xab}
 *
 * @param	Str is a Input String. Will support the lower and upper
 * 		case values. Value should be between 0-9, a-f and A-F
 *
 * @param	Buf is Output buffer.
 * @param	Len of the input string. Should have even values
 *
 * @return
 * 		- XST_SUCCESS successfully converted
 * 		- XST_FAILURE Conversion failed
 *
 * @note	None.
 *
 ****************************************************************************/
static u32 ConvertStringToHexBE(const char * Str, u8 * Buf, u32 Len)
{
	u32 ConvertedLen = 0;
	u8 LowerNibble, UpperNibble;
	u32 Status = XST_FAILURE;

	/* Check the parameters */
	if (Str == NULL){
		Status = XST_FAILURE;
		goto END;
	}

	if (Buf == NULL){
		Status = XST_FAILURE;
		goto END;
	}

	/* Len has to be multiple of 2 */
	if ((Len == 0) || (Len % 2 == 1)) {
		Status = XST_FAILURE;
		goto END;
	}

	ConvertedLen = 0;
	while (ConvertedLen < Len) {
		/* Convert char to nibble */
		if (Xil_ConvertCharToNibble(Str[ConvertedLen],
				&UpperNibble) ==XST_SUCCESS) {
			/* Convert char to nibble */
			if (Xil_ConvertCharToNibble(
					Str[ConvertedLen + 1],
					&LowerNibble) == XST_SUCCESS) {
				/* Merge upper and lower nibble to Hex */
				Buf[ConvertedLen / 2] =
					(UpperNibble << 4) | LowerNibble;
			} else {
				/* Error converting Lower nibble */
				Status = XST_FAILURE;
				goto END;
			}
		} else {
			/* Error converting Upper nibble */
			Status = XST_FAILURE;
			goto END;
		}
		ConvertedLen += 2;
	}

	Status = XST_SUCCESS;
END:

	return Status;
}
