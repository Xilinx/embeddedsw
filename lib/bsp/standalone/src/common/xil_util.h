/******************************************************************************/
/**
* Copyright (C) 2019 - 2022  Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
* @file xil_util.h
* @addtogroup common_utilities Common Utility APIs
* @{
* @details
*
* xil_util.h file contains xil utility functions declarations
* Except few functions, most of these functions are wrappers to standard functions.
* The standard string functions do not validate the input and that results into
* buffer overflows. To avoid it, the wrapper function validates the input and
* then passed to standard function. There are few constant time functions
* ( xxx_CT() ) which are used to compare the data in constant time.
* The constant time functions should be used while comparing secure data
* like password, keys which prevent disclosing of the data using
* timing analysis.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 6.4   mmd      04/21/19 First release.
* 6.5   kal      02/29/20 Added Xil_ConvertStringToHexBE API
* 7.3   kal      06/30/20 Converted Xil_Ceil macro to API
*       rpo      08/19/20 Added function for read, modify and write
*       bsv      08/21/20 Added XSECURE_TEMPORAL_CHECK macro to add
*                         redundancy in security critical functions, to avoid
*                         glitches from altering the return values of security
*                         critical functions. The macro requires a label to be
*                         passed to "go to" in case of error.
*      kpt       09/03/20 Added XSECURE_TEMPORAL_IMPL macro for redundancy
*      kal       09/22/20 Changed the param type from const char to const char*
*                         to avoid copying key onto stack
*      td        10/16/20 Added Xil_Strcpy, Xil_Strcat, Xil_SecureMemCpy and
*                         Xil_MemCmp functions
*      am        10/13/20 Resolved Coverity warning
*      td        11/19/20 Updated XSECURE_TEMPORAL_CHECK and
*                         XSECURE_TEMPORAL_IMPL to fix MISRA C Rule 15.3
* 7.4  am        11/26/20 Added Xil_StrCpyRange function
* 7.6  kpt       07/15/21 Added Xil_SecureZeroize function
* 7.7  kpt       11/09/21 Added Xil_SMemCmp, Xil_SMemCmp_CT, Xil_SMemCpy,
*                         Xil_SMemSet, Xil_SStrCat, Xil_SStrCmp, Xil_SStrCmp_CT
*                         Xil_SStrCpy functions
* 7.7	sk	 01/10/22 Update functions return type to fix misra_c_2012_
* 			  directive_4_6 violations.
*      mmd       02/28/22 Added Xil_SMemMove function prototype
* 8.0  adk       04/18/22 Added Xil_WaitForEventSet function prototype.
*      ssc       08/25/22 Added Xil_SecureRMW32 prototype
* 8.1  sa        09/29/22 Change the type of first argument passed to Xil_WaitForEvent
*			  API from u32 to UINTPTR for supporting 64 bit addressing.
* 8.1  sa        10/20/22 Change the type of first argument passed to Xil_WaitForEvents
*                         API from u32 to UINTPTR for supporting 64 bit addressing.
* 8.1  akm       01/02/23 Added Xil_RegisterPlmHandler() & Xil_PlmStubHandler() APIs.
*      bm        03/14/23 Added XSECURE_REDUNDANT_CALL and XSECURE_REDUNDANT_IMPL macros
*      sk        03/14/23 Added Status Check Glitch detect Macro
* 9.0  ml        03/03/23 Add description to fix doxygen warnings.
*      mmd       07/09/23 Added macro to build version
*      ml        09/13/23 Replaced numerical types (int) with proper typedefs(s32) to
*                         fix MISRA-C violations for Rule 4.6
* 9.1  kpt       02/21/24 Added Xil_SChangeEndiannessAndCpy function
* 9.2  kpt       06/24/24 Added Xil_SReverseData function
* 9.2  pre       08/16/24 Added Xil_MemCpy64 function
*      kpt       10/17/24 Move API's used in secure libs to xil_sutil.c
*
* </pre>
*
*****************************************************************************/

#ifndef XIL_UTIL_H_
#define XIL_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/
#include "xil_types.h"
#include "xil_io.h"
#include "xstatus.h"
#include "xil_sutil.h"

/*************************** Constant Definitions *****************************/
#define XIL_SIZE_OF_NIBBLE_IN_BITS	4U /**< size of nibble in bits */
#define XIL_SIZE_OF_BYTE_IN_BITS	8U /**< size of byte in bits */

#define XIL_MAX_HEX_STR_LEN	512U /**< Maximum string length handled by
                                          Xil_ValidateHexStr function */

/****************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes ******************************/

/**< Ceils the provided float value */
s32 Xil_Ceil(float Value);

/**< Validate input hex character */
u32 Xil_IsValidHexChar(const char *Ch);

/**< Validate the input string contains only hexadecimal characters */
u32 Xil_ValidateHexStr(const char *HexStr);

/**< Convert string to hex numbers in little enidian format */
u32 Xil_ConvertStringToHexLE(const char *Str, u8 *Buf, u32 Len);

/**< Returns length of the input string */
u32 Xil_Strnlen(const char *Str, u32 MaxLen);

/**< Convert string to hex numbers in big endian format */
u32 Xil_ConvertStringToHexBE(const char *Str, u8 *Buf, u32 Len);

/**< Copies source string to destination string */
s32 Xil_Strcpy(char *DestPtr, const char *SrcPtr, const u32 Size);

/**< Copies specified range from source string to destination string */
s32 Xil_StrCpyRange(const u8 *Src, u8 *Dest, u32 From, u32 To, u32 MaxSrcLen,
		    u32 MaxDstLen);

/**< Appends string2 to string1 */
s32 Xil_Strcat(char *Str1Ptr, const char *Str2Ptr, const u32 Size);

/**< Compares Len bytes from memory1 and memory2 */
s32 Xil_MemCmp(const void *Buf1Ptr, const void *Buf2Ptr, u32 Len);

#ifdef __cplusplus
}
#endif

#endif	/* XIL_UTIL_H_ */
/**
* @} End of "addtogroup common_utilities".
*/
