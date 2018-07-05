/******************************************************************************
*
* Copyright (C) 2016 - 17 Xilinx, Inc.  All rights reserved.
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
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xilsecure_sha2_example.c
* @addtogroup xsecure_apis XilSecure SHA2 APIs
* @{
*
* This file contains the example which generates SHA2 hash on provided
* data and compares with expected SHA2 hash.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.2   vns     25/08/16 First release
* 2.2   vns     07/06/16 Added doxygen tags
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsecure_sha2.h"
#include "xil_types.h"
#include "xil_printf.h"
#include "xstatus.h"

/************************** Constant Definitions ******************************/

#define XSECURE_EXPECTED_SHA2_HASH \
	"9e4a0e8785dc081ef9b19feba1f8b827335fe6b14da2640c81d353fe50dec30c"

#define XSECURE_DATA_SIZE	(1024)

/************************** Function Prototypes ******************************/
static u32 XSecure_ConvertCharToNibble (char InChar, u8 *Num);
u32 XSecure_ConvertStringToHexBE(const char * Str, u8 * Buf, u32 Len);
u32 XSecure_Sha2_Hash_Gn();

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

u8 Data[XSECURE_DATA_SIZE] = 	"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5"
				"A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A";

/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int main(void)
{

	u8 Status;

	xil_printf("Started generating SHA2 hash\n\r");

	Status = XSecure_Sha2_Hash_Gn();
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to generated SHA2 hash \n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully generated SHA2 hash\n\r");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function generates SHA2 hash on data provided by using XilSecure library
* and compares with expected value.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		Here we are generating hash on 1KB data.
*
******************************************************************************/
/** //! [SHA2 example] */
u32 XSecure_Sha2_Hash_Gn()
{
	sha2_context Sha2;
	u8 Output_Hash[32];
	u8 IntermediateHash[32];
	u8 Cal_Hash[32];
	u32 Index;
	u32 Size = XSECURE_DATA_SIZE;
	u32 Status;

	/* Generating SHA2 hash */
	sha2_starts(&Sha2);
	sha2_update(&Sha2, (u8 *)Data, Size - 1);

	/* If required we can read intermediate hash */
	sha2_hash(&Sha2, IntermediateHash);
	xil_printf("Intermediate SHA2 Hash is: ");
	for (Index = 0; Index < 32; Index++) {
		xil_printf("%02x", IntermediateHash[Index]);
	}
	xil_printf("\n");

	sha2_finish(&Sha2, Output_Hash);

	xil_printf("Generated SHA2 Hash is: ");
	for (Index = 0; Index < 32; Index++) {
		xil_printf("%02x", Output_Hash[Index]);
	}
	xil_printf("\n");

	/* Convert expected Hash value into hexa */
	Status = XSecure_ConvertStringToHexBE(XSECURE_EXPECTED_SHA2_HASH,
							Cal_Hash, 64);
	if (Status != XST_SUCCESS) {
		xil_printf("Error: While converting expected "
				"string of SHA2 hash to hexa\n\r");
		return XST_FAILURE;
	}

	/* Compare generated hash with expected hash value */
	for (Index = 0; Index < 32; Index++) {
		if (Cal_Hash[Index] != Output_Hash[Index]) {
			xil_printf("Error: SHA2 Hash generated through "
			"XilSecure library does not match with "
			"expected hash value\n\r");
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
/** //! [SHA2 example] */

/****************************************************************************/
/**
* Converts the string into the equivalent Hex buffer.
*	Ex: "abc123" -> {0xab, 0xc1, 0x23}
*
* @param	Str is a Input String. Will support the lower and upper case
*		values. Value should be between 0-9, a-f and A-F
*
* @param	Buf is Output buffer.
* @param	Len of the input string. Should have even values
*
* @return
*		- XST_SUCCESS no errors occured.
*		- XST_FAILURE an error when input parameters are not valid
*		- an error when input buffer has invalid values
*
* @note	None
*
****************************************************************************/
u32 XSecure_ConvertStringToHexBE(const char * Str, u8 * Buf, u32 Len)
{
	u32 ConvertedLen=0;
	u8 LowerNibble, UpperNibble;

	if (Str == NULL)
		return XST_FAILURE;
	if (Buf == NULL)
		return XST_FAILURE;
	/* Len has to be multiple of 2 */
	if ((Len == 0) || (Len%2 == 1)) {
		return XST_FAILURE;
	}

	ConvertedLen = 0;
	while (ConvertedLen < Len) {
		/* Convert char to nibble */
		if (XSecure_ConvertCharToNibble(Str[ConvertedLen],&UpperNibble)
				==XST_SUCCESS) {
			/* Convert char to nibble */
			if (XSecure_ConvertCharToNibble(Str[ConvertedLen+1],
					&LowerNibble)==XST_SUCCESS) {
				/* Merge upper and lower nibble to Hex */
				Buf[ConvertedLen/2] =
					(UpperNibble << 4) | LowerNibble;
			}
			else {
				/* Error converting Lower nibble */
				return XST_FAILURE;
			}
		}
		else {
			/* Error converting Upper nibble */
			return XST_FAILURE;
		}

		ConvertedLen += 2;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* Converts the char into the equivalent nibble.
*	Ex: 'a' -> 0xa, 'A' -> 0xa, '9'->0x9
*
* @param	InChar is input character. It has to be between 0-9,a-f,A-F
* @param	Num is the output nibble.
*
* @return
*		- XST_SUCCESS no errors occured.
*		- XST_FAILURE an error when input parameters are not valid
*
* @note		None.
*
****************************************************************************/
static u32 XSecure_ConvertCharToNibble (char InChar, u8 *Num)
{
	/* Convert the char to nibble */
	if ((InChar >= '0') && (InChar <= '9'))
		*Num = InChar - '0';
	else if ((InChar >= 'a') && (InChar <= 'f'))
		*Num = InChar - 'a' + 10;
	else if ((InChar >= 'A') && (InChar <= 'F'))
		*Num = InChar - 'A' + 10;
	else
		return XST_FAILURE;

	return XST_SUCCESS;
}
/** @} */