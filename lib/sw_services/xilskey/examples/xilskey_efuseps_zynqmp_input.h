/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
* @file xilskey_efuseps_zynqmp_input.h.
* This file contains macros which needs to configured by user based on the
* options selected by user operations will be performed.
*
* @note
*
*               User configurable parameters for ZynqMP PS eFUSE
*------------------------------------------------------------------------------
*
*	#define	XSK_EFUSEPS_AES_RD_LOCK			FALSE
*	TRUE will permanently disables the CRC check of FUSE_AES.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_AES_WR_LOCK			FALSE
*	TRUE will permanently disables the writing to FUSE_AES block.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPs_FORCE_USE_AES_ONLY		FALSE
*	TRUE will permanently enables encrypted booting only using the Fuse
*	key.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_BBRAM_DISABLE		FALSE
*	TRUE will permanently disables the BBRAM key.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_ERR_OUTOF_PMU_DISABLE	FALSE
*	TRUE will permanently disables the error output from the PMU.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_JTAG_DISABLE		FALSE
*	TRUE will permanently disables JTAG controller.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_DFT_DISABLE			FALSE
*	TRUE will permanently disables DFT boot mode.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_PROG_GATE_0_DISABLE		FALSE
*	TRUE will permanently disables PROG_GATE feature in PPD.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_PROG_GATE_1_DISABLE		FALSE
*	TRUE will permanently disables PROG_GATE feature in PPD.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_PROG_GATE_2_DISABLE		FALSE
*	TRUE will permanently disables PROG_GATE feature in PPD.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_SECURE_LOCK			FALSE
*	TRUE will permanently disables reboot into JTAG mode when doing
*	a secure lockdown.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_RSA_ENABLE			FALSE
*	TRUE will permanently enables RSA authentication during boot.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_PPK0_WR_LOCK		FALSE
*	TRUE will permanently disables writing to PPK0 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_PPK0_REVOKE			FALSE
*	TRUE will permanently revokes PPK0.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_PPK1_WR_LOCK		FALSE
*	TRUE will permanently disables writing PPK1 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_PPK1_REVOKE			FALSE
*	TRUE will permanently revokes PPK1.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_USER_WRLK_0			FALSE
*	TRUE will permanently disables writing to USER_0 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_USER_WRLK_1			FALSE
*	TRUE will permanently disables writing to USER_1 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_USER_WRLK_2			FALSE
*	TRUE will permanently disables writing to USER_2 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_USER_WRLK_3			FALSE
*	TRUE will permanently disables writing to USER_3 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_USER_WRLK_4			FALSE
*	TRUE will permanently disables writing to USER_4 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_USER_WRLK_5			FALSE
*	TRUE will permanently disables writing to USER_5 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_USER_WRLK_6			FALSE
*	TRUE will permanently disables writing to USER_6 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_USER_WRLK_7			FALSE
*	TRUE will permanently disables writing to USER_7 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*          Following has to be set for programming required keys
*------------------------------------------------------------------------------
*	#define XSK_EFUSEPS_WRITE_AES_KEY		TRUE
*	TRUE will burn the AES key provided in XSK_EFUSEPS_AES_KEY.
*	FALSE will ignore the key provide XSK_EFUSEPS_AES_KEY.
*
*	#define XSK_EFUSEPS_WRITE_USER_KEY		TRUE
*	TRUE will burn User key provided in XSK_EFUSEPS_USER_KEY.
*	FALSE will ignore the key provided in XSK_EFUSEPS_USER_KEY.
*
*	#define XSK_EFUSEPS_WRITE_PPK0_SHA3_HASH	TRUE
*	TRUE will burn PPK0 sha3 hash provided in XSK_EFUSEPS_PPK0_SHA3_HASH.
*	FALSE will ignore the hash provided in XSK_EFUSEPS_PPK0_SHA3_HASH.
*
*	#define XSK_EFUSEPS_WRITE_PPK1_SHA3_HASH	TRUE
*	TRUE will burn PPK1 sha3 hash provided in XSK_EFUSEPS_PPK1_SHA3_HASH.
*	FALSE will ignore the hash provided in XSK_EFUSEPS_PPK1_SHA3_HASH.
*
*	#define XSK_EFUSEPS_WRITE_SPKID			TRUE
*	TRUE will burn SPKID provided in XSK_EFUSEPS_SPK_ID.
*	FALSE will ignore the hash provided in XSK_EFUSEPS_SPK_ID.
*
*	#define XSK_EFUSEPS_WRITE_JTAG_USERCODE		TRUE
*	TRUE will burn JTAG user code provided in XSK_EFUSEPS_JTAG_USERCODE.
*	FALSE will ignore the JTAG user code provided in
*	XSK_EFUSEPS_JTAG_USERCODE.
*
*	#define		XSK_EFUSEPS_AES_KEY
*	"0000000000000000000000000000000000000000000000000000000000000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the ZynqMP Ps eFUSE array when write API used. This value should
*	be given in string format. It should be 64 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn AES Key.
*	Note that,for writing the AES Key, XSK_EFUSEPS_WRITE_AES_KEY should
*	have TRUE value.
*
*	#define		XSK_EFUSEPS_USER_KEY
*	"0000000000000000000000000000000000000000000000000000000000000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the ZynqMP Ps eFUSE array when write API used. This value should
*	be given in string format. It should be 64 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn User Key.
*	Note that,for writing the User Key, XSK_EFUSEPS_WRITE_USER_KEY should
*	have TRUE value.
*
*	#define XSK_EFUSEPS_PPK0_IS_SHA3	TRUE
*	Default value is TRUE.
*	TRUE XSK_EFUSEPS_PPK0_SHA3_HASH should be of string length 96 it specifies
*	that PPK0 is used to program SHA3 hash.
*	FALSE XSK_EFUSEPS_PPK0_SHA3_HASH  should be of string length 64 it specifies
*	that PPK0 is used to program SHA2 hash.
*
*	#define		XSK_EFUSEPS_PPK0_HASH
*	"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the ZynqMP Ps eFUSE array when write API used. This value should
*	be given in string format. It should be 96 or 64 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn PPK0 hash.
*	Note that,for writing the PPK0 hash, XSK_EFUSEPS_WRITE_PPK0_SHA3_HASH
*	should have TRUE value.
*	While writing SHA2 hash, length should be 64 characters long
*	XSK_EFUSEPS_PPK0_IS_SHA3 macro has to be made FALSE.
*	While writing SHA3 hash, length should be 96 characters long and
*	XSK_EFUSEPS_PPK0_IS_SHA3 macro should be made TRUE
*
*	#define XSK_EFUSEPS_PPK1_IS_SHA3	FALSE
*	Default value is FALSE.
*	TRUE XSK_EFUSEPS_PPK1_SHA3_HASH should be of string length 96 it specifies
*	that PPK1 is used to program SHA3 hash.
*	FALSE XSK_EFUSEPS_PPK1_SHA3_HASH  should be of string length 64 it specifies
*	that PPK1 is used to program SHA2 hash.
*
*	#define		XSK_EFUSEPS_PPK1_HASH
*	"0000000000000000000000000000000000000000000000000000000000000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the ZynqMP Ps eFUSE array when write API used. This value should
*	be given in string format. It should be 64 or 96 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn PPK1 hash.
*	Note that,for writing the PPK11 hash, XSK_EFUSEPS_WRITE_PPK1_SHA3_HASH
*	should have TRUE value.
*	By default PPK1 hash will be provided with 64 character length to
*	program PPK1 hash with sha2 hash so XSK_EFUSEPS_PPK1_IS_SHA3
*	also will be in FALSE state
*	But to program PPK1 hash with SHA3 hash make XSK_EFUSEPS_PPK1_IS_SHA3
*	to TRUE and provide sha3 hash of length 96 characters XSK_EFUSEPS_PPK1_HASH
*	so that one can program sha3 hash.
*
*	#define		XSK_EFUSEPS_SPK_ID		"00000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the ZynqMP Ps eFUSE array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn SPK ID.
*	Note that,for writing the SPK ID, XSK_EFUSEPS_WRITE_SPKID
*	should have TRUE value.
*
*	#define		XSK_EFUSEPS_JTAG_USERCODE	"00000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the ZynqMP Ps eFUSE array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn JTAG user code.
*	Note that,for writing the JTAG user code, XSK_EFUSEPS_WRITE_JTAG_USERCODE
*	should have TRUE value.
*
*	NOTE: PPK hash should be unmodified hash generated by bootgen.
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 4.0   vns     10/01/15 First release
* </pre>
*
******************************************************************************/

#ifndef XILSKEY_EFUSEPS_ZYNQMP_INPUT_H_
#define XILSKEY_EFUSEPS_ZYNQMP_INPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xilskey_eps_zynqmp.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/* Following defines should be defined either TRUE or FALSE */

/**
 * Following is the define to select if the user wants to program
 * Secure control bits
 */
#define XSK_EFUSEPS_AES_RD_LOCK			FALSE
#define XSK_EFUSEPS_AES_WR_LOCK			FALSE
#define XSK_EFUSEPs_FORCE_USE_AES_ONLY		FALSE
#define XSK_EFUSEPS_BBRAM_DISABLE		FALSE
#define XSK_EFUSEPS_ERR_OUTOF_PMU_DISABLE	FALSE
#define XSK_EFUSEPS_JTAG_DISABLE		FALSE
#define XSK_EFUSEPS_DFT_DISABLE			FALSE
#define XSK_EFUSEPS_PROG_GATE_0_DISABLE		FALSE
#define XSK_EFUSEPS_PROG_GATE_1_DISABLE		FALSE
#define XSK_EFUSEPS_PROG_GATE_2_DISABLE		FALSE
#define XSK_EFUSEPS_SECURE_LOCK			FALSE
#define XSK_EFUSEPS_RSA_ENABLE			FALSE
#define XSK_EFUSEPS_PPK0_WR_LOCK		FALSE
#define XSK_EFUSEPS_PPK0_REVOKE			FALSE
#define XSK_EFUSEPS_PPK1_WR_LOCK		FALSE
#define XSK_EFUSEPS_PPK1_REVOKE			FALSE

/**
 * Following is the define to select if the user wants to program
 * user control bits
 */
#define XSK_EFUSEPS_USER_WRLK_0			FALSE
#define XSK_EFUSEPS_USER_WRLK_1			FALSE
#define XSK_EFUSEPS_USER_WRLK_2			FALSE
#define XSK_EFUSEPS_USER_WRLK_3			FALSE
#define XSK_EFUSEPS_USER_WRLK_4			FALSE
#define XSK_EFUSEPS_USER_WRLK_5			FALSE
#define XSK_EFUSEPS_USER_WRLK_6			FALSE
#define XSK_EFUSEPS_USER_WRLK_7			FALSE

/**
 * Following is the define to select if the user wants to select AES,
 * User keys, PPK0 Sha3 hash, PPK1 Sha3 hash, SPKID and JTAG user code
 * for Zynq MP
 */
/* For writing into eFuse */
#define XSK_EFUSEPS_WRITE_AES_KEY		FALSE
#define XSK_EFUSEPS_WRITE_USER_KEY		FALSE
#define XSK_EFUSEPS_WRITE_PPK0_HASH	FALSE
#define XSK_EFUSEPS_WRITE_PPK1_HASH	FALSE
#define XSK_EFUSEPS_WRITE_SPKID			FALSE
#define XSK_EFUSEPS_WRITE_JTAG_USERCODE		FALSE

/**
 * Following defines should be given in the form of hex string.
 * The length of AES_KEY string must me 64 and for USER_KEY must be 64.
 */
#define XSK_EFUSEPS_AES_KEY		"0000000000000000000000000000000000000000000000000000000000000000"
#define XSK_EFUSEPS_USER_KEY		"0000000000000000000000000000000000000000000000000000000000000000"

#define XSK_EFUSEPS_PPK0_IS_SHA3	TRUE
#define XSK_EFUSEPS_PPK0_HASH	"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

#define XSK_EFUSEPS_PPK1_IS_SHA3	FALSE
#define XSK_EFUSEPS_PPK1_HASH	"0000000000000000000000000000000000000000000000000000000000000000"
#define XSK_EFUSEPS_SPK_ID		"00000000"
#define XSK_EFUSEPS_JTAG_USERCODE	"00000000"

#ifdef __cplusplus
}
#endif

#endif /* XILSKEY_EFUSEPS_ZYNQMP_INPUT_H_ */
