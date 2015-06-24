/******************************************************************************
*
* Copyright (C) 2013 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xilskey_input.h
*
*
* @note
*
*  					User configurable parameters for PS eFUSE
*  	---------------------------------------------------------------------------
* 	#define XSK_EFUSEPS_ENABLE_WRITE_PROTECT				FALSE
*
*	TRUE to burn the write protect bits in eFUSE array. Write protect
*	has two bits, when any of the bit is blown, it is considered as write
*	protected. So, while burning the Write protect bits, even if one bit is
*	blown, write API returns success. Note that, POR reset is required after
*	burning, for write protection of the eFUSE bits to come into effect.
*	It is recommended to do the POR reset after write protection.
*	Also note that, once write protect bits are burned, no more eFUSE writes
*	are possible. So, please be sure when burning the write protect bits.
*	If the Write protect macro is TRUE with other macros, write protect will
*	be burned in the last, after burning all the defined values, so that for
*	any error while burning other macros will not effect the total eFUSE array.
*	FALSE will not modify the write protect bits.
*
*	#define XSK_EFUSEPS_ENABLE_RSA_AUTH					FALSE
*
*	TRUE to burn the RSA enable bit in PS eFUSE array. After enabling the bit,
*	every successive boot must be RSA enabled apart from JTAG. Before burning
*	this bit, make sure that eFUSE array has the valid PPK hash.If the PPK hash
*	burning is enabled, only after writing the hash successfully, RSA enable
*	bit will be blown. Note that, for RSA enable bit to take effect, POR reset
*	is required.
*	FALSE will not modify the RSA enable bit.
*
*	#define XSK_EFUSEPS_ENABLE_ROM_128K_CRC				FALSE
*	TRUE will burn the ROM 128k crc bit. Every successive boot after this,
*	BootROM will calculate 128k crc. FALSE will not modify the ROM CRC128K bit.
*
*	#define XSK_EFUSEPS_ENABLE_RSA_KEY_HASH				FALSE
*	TRUE will burn the eFUSE hash, that is given in XSK_EFUSEPS_RSA_KEY_HASH_VALUE
*	when write API is used. TRUE will read the eFUSE hash when read API is used
*	and will be read into structure. FALSE will ignore the value given.
*
*	#define XSK_EFUSEPS_RSA_KEY_HASH_VALUE
*			"c8bb4d9e1fcdbd27b99d48a3df5720b98f35bafabb1e10333a78322fb82ce63d"
*
*	The value mentioned in this will be converted to hex buffer and written
*	into the PS eFUSE array when write API used. This value should be the
*	PPK(Primary Public Key) hash given in string format. It should be 64
*	characters long, valid characters are 0-9,a-f,A-F. Any other character
*	is considered as invalid string and will not burn RSA hash.
*
* 	Note: When XilSKey_EfusePs_Write() API is used, above mentioned RSA hash
* 	is written and  XSK_EFUSEPS_ENABLE_RSA_KEY_HASH should have TRUE value.
*
* 	   					User configurable parameters for PL eFUSE
*  	-----------------------------------------------------------------------
*  	#define 	XSK_EFUSEPL_FORCE_PCYCLE_RECONFIG			FALSE
*	TRUE then part has to be power cycled to be able to be reconfigured.
*	FALSE will not set the eFUSE control bit.
*
*	#define		XSK_EFUSEPL_DISABLE_KEY_WRITE				FALSE
*	TRUE will disable eFUSE write to FUSE_AES and FUSE_USER blocks
*	XFLASE will enable eFUSE write to FUSE_AES and FUSE_USER blocks
*
*	#define		XSK_EFUSEPL_DISABLE_AES_KEY_READ			FALSE
*	TRUE will disable the write to FUSE_AES & FUSE_USER key & disables
*	read of FUSE_AES.
*	FALSE will enable eFUSE read from & write to FUSE_AES and FUSE_USER blocks
*
*	#define		XSK_EFUSEPL_DISABLE_USER_KEY_READ			FALSE
*	TRUE will disable the write to FUSE_AES & FUSE_USER key & disables read of
*	FUSE_USER
*	FALSE will enable eFUSE read from & write to FUSE_AES and FUSE_USER blocks
*
*	Note: If any one of the above two definitions are FALSE then reading of
*	FUSE_AES & FUSE_USER is not possible
*
*	#define		XSK_EFUSEPL_DISABLE_FUSE_CNTRL_WRITE		FALSE
*	TRUE will disable the eFUSE write to FUSE_CTRL block
*	FALSE will not set the eFUSE control bit, so that user can write into
*	FUSE_CTRL block later.
*
*	#define		XSK_EFUSEPL_FORCE_USE_AES_ONLY				FALSE
*	TRUE will force to use secure boot with eFUSE AES key only
*	FALSE will not set the eFUSE control bit so that user can use non-secure
*	boot.
*
*	#define 	XSK_EFUSEPL_DISABLE_JTAG_CHAIN				FALSE
*	If TRUE then permanently sets the Zynq ARM DAP controller in bypass mode.
*	FALSE will allow Zynq ARM DAP visible through JTAG.
*
*	#define		XSK_EFUSEPL_BBRAM_KEY_DISABLE				FALSE
*	XTURE will force eFUSE key to be used if booting Secure Image.
*	FALSE will not set the eFUSE control bit so that user can use secure boot
*	with BBRAM key.
*
*	Following are the MIO pins used for PL JTAG operations.
*	User can change these pins as their discretion.
*	#define		XSK_EFUSEPL_MIO_JTAG_TDI				(17)
*	#define		XSK_EFUSEPL_MIO_JTAG_TDO				(18)
*	#define		XSK_EFUSEPL_MIO_JTAG_TCK				(19)
*	#define		XSK_EFUSEPL_MIO_JTAG_TMS				(20)
*
*	MUX selection pin:
*	#define		XSK_EFUSEPL_MIO_JTAG_MUX_SELECT		(21)
*	This pin is used to select between the external JTAG or MIO driving JTAG
*	operations.
*
*	#define 	XSK_EFUSEPL_MIO_MUX_SEL_DEFAULT_VAL				LOW
*	LOW writes zero on the mux select line before writing the PL eFUSE
*	HIGH writes one on the mux select line before writing the PL eFUSE
*
*	#define XSK_EFUSEPL_PROGRAM_AES_AND_USER_LOW_KEY		FALSE
*	TRUE will burn the AES & User Low hash key, that is given in
*	XSK_EFUSEPL_AES_KEY & XSK_EFUSEPL_USER_LOW_KEY respectively.
*	FALSE will ignore the values given.
*
*	Note: User cannot write AES Key & User Low Key separately.
*
*	#define XSK_EFUSEPL_PROGRAM_USER_HIGH_KEY				FALSE
*	TRUE will burn the User High hash key, that is given in
*	XSK_EFUSEPL_AES_KEY & XSK_EFUSEPL_USER_LOW_KEY respectively.
*	FALSE will ignore the values given.
*
*	#define 	XSK_EFUSEPL_AES_KEY
*		"0000000000000000000000000000000000000000000000000000000000000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the PL eFUSE array when write API used. This value should be the
*	PPK(Primary Public Key) hash given in string format. It should be 64
*	characters long, valid characters are 0-9,a-f,A-F. Any other character
*	is considered as invalid string and will not burn AES Key. Note that,
*	for writing the AES Key, XSK_EFUSEPL_PROGRAM_AES_AND_USER_LOW_KEY should
*	have TRUE value.
*
*	#define 	XSK_EFUSEPL_USER_LOW_KEY			"00"
*	The value mentioned in this will be converted to hex buffer and written
*	into the PL eFUSE array when write API used. This value should be the
*	User Low Key given in string format. It should be 2 characters long, valid
*	 characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	 string and will not burn User Low Key. Note that, for writing the AES Key,
*	 XSK_EFUSEPL_PROGRAM_AES_AND_USER_LOW_KEY should have TRUE value.
*
*
*	#define 	XSK_EFUSEPL_USER_HIGH_KEY			"000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the PL eFUSE array when write API used. This value should be the User
*	 High Key given in string format. It should be 6 characters long, valid
*	 characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	 string and will not burn User High Key. Note that, for writing the AES
*	 Key, XSK_EFUSEPL_PROGRAM_USER_HIGH_KEY should have TRUE value.
*
* BBRAM related definitions:
*-----------------------------------------------------------------------------
*	#define 	XSK_BBRAM_FORCE_PCYCLE_RECONFIG		FALSE
*			If TRUE then part has to be power cycled to be
*			able to be reconfigured
*	#define 	XSK_BBRAM_DISABLE_JTAG_CHAIN		FALSE
*			If TRUE then permanently sets the Zynq
* 			ARM DAP controller in bypass mode
*	MIO pins used for JTAG signals. Can be changed as per hardware.
*	#define		XSK_BBRAM_MIO_JTAG_TDI		(17)
*	#define		XSK_BBRAM_MIO_JTAG_TDO		(21)
*	#define		XSK_BBRAM_MIO_JTAG_TCK		(19)
*	#define		XSK_BBRAM_MIO_JTAG_TMS		(20)
*	#define		XSK_BBRAM_MIO_JTAG_MUX_SELECT	(11)
*	#define 	XSK_BBRAM_MIO_MUX_SEL_DEFAULT_VAL	LOW
*			Default value to enable the PL JTAG
* This is the 256 bit key to be programmed into BBRAM.
* This should entered by user in HEX.
* #define 	XSK_BBRAM_AES_KEY
*	"349de4571ae6d88de23de65489acf67000ff5ec901ae3d409aabbce4549812dd"
* #define	XSK_BBRAM_AES_KEY_SIZE_IN_BITS	256
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.00a rpoolla 04/26/13 First release
* 1.01a hk      09/18/13 Added BBRAM related definitions
* </pre>
*
*
******************************************************************************/

#ifndef XILSKEY_INPUT_H
#define XILSKEY_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
/************************** Constant Definitions *****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
#define XSK_EFUSEPL_DRIVER
#define XSK_EFUSEPS_DRIVER

#ifdef XSK_EFUSEPL_DRIVER

/**
 *  Voltage level definitions
 */
#define 	LOW								0
#define 	HIGH							1

/**
 * Following defines should be defined either TRUE or FALSE.
 * --------------------------------------------------------
 */

#define 	XSK_EFUSEPL_FORCE_PCYCLE_RECONFIG	FALSE /**< If TRUE then part
 	 	 	 	 	 	 	 	 	 	 	 *  has to be power cycled to be
 	 	 	 	 	 	 	 	 	 	 	 *  able to be reconfigured
 	 	 	 	 	 	 	 	 	 	 	 */
#define		XSK_EFUSEPL_DISABLE_KEY_WRITE		FALSE /**< If TRUE will disable
 	 	 	 	 	 	 	 	 	 	 	 	*  eFUSE write to FUSE_AES and
 	 	 	 	 	 	 	 	 	 	 	 	*  FUSE_USER blocks
 	 	 	 	 	 	 	 	 	 	 	 	*/
#define		XSK_EFUSEPL_DISABLE_AES_KEY_READ	FALSE /**< If TRUE will disable
											* eFUSE read to FUSE_AES block and
											* also disables eFUSEwrite to
											* FUSE_AES and FUSE_USER blocks
											*/
#define		XSK_EFUSEPL_DISABLE_USER_KEY_READ	FALSE /**< If TRUE will disable
												* eFUSE read to FUSE_USER block
												* and also disables eFUSE write
												* to FUSE_AES and FUSE_USER
												* blocks
 	 	 	 	 	 	 	 	 	 	 	 	*/
#define		XSK_EFUSEPL_DISABLE_FUSE_CNTRL_WRITE 	FALSE /**< If TRUE will
													* disable eFUSE write to
													* FUSE_CNTRL block
													*/
#define		XSK_EFUSEPL_FORCE_USE_AES_ONLY		FALSE /**< If TRUE will force
												* to use Secure boot with eFUSE
												* key only
												*/
#define 	XSK_EFUSEPL_DISABLE_JTAG_CHAIN		FALSE /**< If TRUE then
												* permanently sets the Zynq
												* ARM DAP controller in bypass
												* mode
    	 	 	 	 	 	 	 	 	 	 	*/
#define		XSK_EFUSEPL_BBRAM_KEY_DISABLE		FALSE /**< If TRUE will force
												* eFUSE key to be used if
												* booting Secure Image
												*/

/**
 * Following defines should be given in the decimal/hexa-decimal values.
 * For example :
 * XSK_EFUSEPL_MIO_JTAG_TCK		34 OR 0x22
 * XSK_EFUSEPL_MIO_JTAG_TMS		35 OR 0x23
 * etc...
 */

#define		XSK_EFUSEPL_MIO_JTAG_TDI	(17) /**< JTAG MIO pin for TDI */
#define		XSK_EFUSEPL_MIO_JTAG_TDO	(21) /**< JTAG MIO pin for TDO */
#define		XSK_EFUSEPL_MIO_JTAG_TCK	(19) /**< JTAG MIO pin for TCK */
#define		XSK_EFUSEPL_MIO_JTAG_TMS	(20) /**< JTAG MIO pin for TMS */
#define		XSK_EFUSEPL_MIO_JTAG_MUX_SELECT	(11) /**< JTAG MIO pin for
 	 	 	 	 	 	 	 	 	 	 	 	 * MUX selection line
 	 	 	 	 	 	 	 	 	 	 	 	 */
/**
 *
 */
#define 	XSK_EFUSEPL_MIO_MUX_SEL_DEFAULT_VAL		LOW /**< Default value to
													* enable the PL JTAG
													*/

/**
 * Following is the define to select if the user wants to select AES key and USER
 * low key OR USER high key or BOTH
 */

#define XSK_EFUSEPL_PROGRAM_AES_AND_USER_LOW_KEY		FALSE /**< TRUE burns
														* the AES & user low key
														*/
#define XSK_EFUSEPL_PROGRAM_USER_HIGH_KEY				FALSE /**< TRUE burns
														* the user high key
														*/
/**
 * Following defines should be given in the form of hex string.
 * The length of AES_KEY string must me 64 and for USER_KEY must be 8.
 */
#define 	XSK_EFUSEPL_AES_KEY			"0000000000000000000000000000000000000000000000000000000000000000"
#define 	XSK_EFUSEPL_USER_LOW_KEY	"00"
#define 	XSK_EFUSEPL_USER_HIGH_KEY	"000000"

#endif	/*XSK_EFUSEPL_DRIVER*/


/**
 *  Similarly we can define PS eFUSE related data
 *  ---------------------------------------------
 */
#ifdef XSK_EFUSEPS_DRIVER

#define XSK_EFUSEPS_ENABLE_WRITE_PROTECT	FALSE /**< Enable the eFUSE Array
											* write protection
											*/
#define XSK_EFUSEPS_ENABLE_RSA_AUTH			FALSE /**< Enable the RSA
											* Authentication eFUSE Bit
											*/
#define XSK_EFUSEPS_ENABLE_ROM_128K_CRC		FALSE /**< Enable the ROM
											* code 128K crc  eFUSE Bit
											*/
#define XSK_EFUSEPS_ENABLE_RSA_KEY_HASH		FALSE /**< Enabling this
											* RsaKeyHashValue[64] is
											* written to eFUSE array
											*/
#define XSK_EFUSEPS_RSA_KEY_HASH_VALUE	"0000000000000000000000000000000000000000000000000000000000000000"

#endif /* End of XSK_EFUSEPS_DRIVER */

/*
 * Definitions for BBRAM
 */

/**< If TRUE then part
  *  has to be power cycled to be
  *  able to be reconfigured
  */
#define 	XSK_BBRAM_FORCE_PCYCLE_RECONFIG		FALSE

/**< If TRUE then
  * permanently sets the Zynq
  * ARM DAP controller in bypass
  * mode
  */
#define 	XSK_BBRAM_DISABLE_JTAG_CHAIN		FALSE

#define		XSK_BBRAM_MIO_JTAG_TDI	(17) /**< JTAG MIO pin for TDI */
#define		XSK_BBRAM_MIO_JTAG_TDO	(21) /**< JTAG MIO pin for TDO */
#define		XSK_BBRAM_MIO_JTAG_TCK	(19) /**< JTAG MIO pin for TCK */
#define		XSK_BBRAM_MIO_JTAG_TMS	(20) /**< JTAG MIO pin for TMS */
#define		XSK_BBRAM_MIO_JTAG_MUX_SELECT	(11) /**< JTAG MIO pin for
 	 	 	 	 	 	       * MUX selection line
 	 	 	 	 	 	       */
/**< Default value to
  * enable the PL JTAG
  */
#define 	XSK_BBRAM_MIO_MUX_SEL_DEFAULT_VAL	0
/**
 * This is the 256 bit key to be programmed into BBRAM.
 * This should entered by user in HEX.
 */
#define 	XSK_BBRAM_AES_KEY	"349de4571ae6d88de23de65489acf67000ff5ec901ae3d409aabbce4549812dd"

#define		XSK_BBRAM_AES_KEY_SIZE_IN_BITS	256

/*
 * End of definitions for BBRAM
 */

/************************** Function Prototypes *****************************/
/****************************************************************************/
#ifdef __cplusplus
}
#endif

#endif	/*XILSKEY_INPUT_H*/
