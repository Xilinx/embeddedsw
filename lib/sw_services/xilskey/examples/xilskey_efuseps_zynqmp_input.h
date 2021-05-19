/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
*	TRUE permanently disables the CRC check of FUSE_AES.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_AES_WR_LOCK			FALSE
*	TRUE permanently disables the writing to FUSE_AES block.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_ENC_ONLY		FALSE
*	TRUE permanently enables encrypted booting only using the Fuse
*	key. It forces to use AES key from eFUSE.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_BBRAM_DISABLE		FALSE
*	TRUE permanently disables the BBRAM key.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_ERR_DISABLE	FALSE
*	TRUE permanently disables the error messages in the
*	JTAG status register.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_JTAG_DISABLE		FALSE
*	TRUE permanently disables the JTAG controller.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_DFT_DISABLE			FALSE
*	TRUE permanently disables DFT boot mode.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_PROG_GATE_DISABLE		FALSE
*	TRUE permanently disables the PROG_GATE feature in PPD.
*	If you set this then the PROG_GATE can never be activated
*	and the PL will always be reset when the PS goes down.
*	Also prevents reboot into JTAG mode after a secure lock down.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_SECURE_LOCK			FALSE
*	TRUE permanently disables reboot into JTAG mode when performing
*	a secure lockdown.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_RSA_ENABLE			FALSE
*	TRUE permanently enables RSA authentication during boot.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_PPK0_WR_LOCK		FALSE
*	TRUE permanently disables writing to PPK0 eFUSEs.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_PPK0_INVLD			FALSE
*	TRUE permanently revokes PPK0.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_PPK1_WR_LOCK		FALSE
*	TRUE permanently disables writing to PPK1 eFUSEs.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_PPK1_INVLD			FALSE
*	TRUE permanently revokes PPK1.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_LBIST_EN			FALSE
*	TRUE permanently enables logic BIST to be run during boot.
*	FALSE will not modify this control bit of eFUSE.
*
*	#define XSK_EFUSEPS_LPD_SC_EN			FALSE
*	TRUE permanently enables zeroization of registers in Low Power
*	Domain(LPD) during boot.
*	FALSE will not modify this control bit of eFUSE.
*
*	#define XSK_EFUSEPS_FPD_SC_EN			FALSE
*	TRUE permanently enables zeroization of registers in the Full Power
*	Domain(FPD) during boot.
*	FALSE will not modify this control bit of eFUSE.
*
*	#define XSK_EFUSEPS_PBR_BOOT_ERR		FALSE
*	TRUE forces the boot to halt when there is any error detected in the PMU.
*	FALSE will not modify this control bit of eFUSE.
*
*	#define XSK_EFUSEPS_USER_WRLK_0			FALSE
*	TRUE permanently disables writing to USER_0 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_USER_WRLK_1			FALSE
*	TRUE permanently disables writing to USER_1 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_USER_WRLK_2			FALSE
*	TRUE permanently disables writing to USER_2 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_USER_WRLK_3			FALSE
*	TRUE permanently disables writing to USER_3 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_USER_WRLK_4			FALSE
*	TRUE permanently disables writing to USER_4 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_USER_WRLK_5			FALSE
*	TRUE permanently disables writing to USER_5 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_USER_WRLK_6			FALSE
*	TRUE permanently disables writing to USER_6 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPS_USER_WRLK_7			FALSE
*	TRUE permanently disables writing to USER_7 efuses.
*	FALSE will not modify this control bit of eFuse.
*
*          Following has to be set for programming required keys
*------------------------------------------------------------------------------
*	#define XSK_EFUSEPS_WRITE_AES_KEY		FALSE
*	TRUE will burn the AES key provided in XSK_EFUSEPS_AES_KEY.
*	FALSE will ignore the key provided in XSK_EFUSEPS_AES_KEY.
*
*	#define XSK_EFUSEPS_WRITE_PPK0_SHA3_HASH	FALSE
*	TRUE will burn PPK0 with it's SHA3 hash provided in XSK_EFUSEPS_PPK0_SHA3_HASH.
*	FALSE will ignore the hash provided in XSK_EFUSEPS_PPK0_SHA3_HASH.
*
*	#define XSK_EFUSEPS_WRITE_PPK1_SHA3_HASH	FALSE
*	TRUE will burn PPK1 with it's SHA3 hash provided in XSK_EFUSEPS_PPK1_SHA3_HASH.
*	FALSE will ignore the hash provided in XSK_EFUSEPS_PPK1_SHA3_HASH.
*
*	#define XSK_EFUSEPS_WRITE_SPKID			FALSE
*	TRUE will burn SPKID provided in XSK_EFUSEPS_SPK_ID.
*	FALSE will ignore the hash provided in XSK_EFUSEPS_SPK_ID.
*
*	#define XSK_EFUSEPS_WRITE_USER0_FUSE		FALSE
*	TRUE will burn User0 Fuse provided in XSK_EFUSEPS_USER0_FUSES.
*	FALSE will ignore the value provided in XSK_EFUSEPS_USER0_FUSES
*
*	#define XSK_EFUSEPS_WRITE_USER1_FUSE		FALSE
*	TRUE will burn User1 Fuse provided in XSK_EFUSEPS_USER1_FUSES.
*	FALSE will ignore the value provided in XSK_EFUSEPS_USER1_FUSES
*
*	#define XSK_EFUSEPS_WRITE_USER2_FUSE		FALSE
*	TRUE will burn User2 Fuse provided in XSK_EFUSEPS_USER2_FUSES.
*	FALSE will ignore the value provided in XSK_EFUSEPS_USER2_FUSES
*
*	#define XSK_EFUSEPS_WRITE_USER3_FUSE		FALSE
*	TRUE will burn User3 Fuse provided in XSK_EFUSEPS_USER3_FUSES.
*	FALSE will ignore the value provided in XSK_EFUSEPS_USER3_FUSES
*
*	#define XSK_EFUSEPS_WRITE_USER4_FUSE		FALSE
*	TRUE will burn User4 Fuse provided in XSK_EFUSEPS_USER4_FUSES.
*	FALSE will ignore the value provided in XSK_EFUSEPS_USER4_FUSES
*
*	#define XSK_EFUSEPS_WRITE_USER5_FUSE		FALSE
*	TRUE will burn User5 Fuse provided in XSK_EFUSEPS_USER5_FUSES.
*	FALSE will ignore the value provided in XSK_EFUSEPS_USER5_FUSES
*
*	#define XSK_EFUSEPS_WRITE_USER6_FUSE		FALSE
*	TRUE will burn User6 Fuse provided in XSK_EFUSEPS_USER6_FUSES.
*	FALSE will ignore the value provided in XSK_EFUSEPS_USER6_FUSES
*
*	#define XSK_EFUSEPS_WRITE_USER7_FUSE		FALSE
*	TRUE will burn User7 Fuse provided in XSK_EFUSEPS_USER7_FUSES.
*	FALSE will ignore the value provided in XSK_EFUSEPS_USER7_FUSES
*
*	#define		XSK_EFUSEPS_AES_KEY
*	"0000000000000000000000000000000000000000000000000000000000000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the ZynqMP Ps eFUSE array when write API used. This value should
*	be given in string format. It should be 64 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn any Fuses.
*	Note that,for writing the AES Key, XSK_EFUSEPS_WRITE_AES_KEY should
*	be set to TRUE.
*	NOTE: This AES key is only red key or gray key. To program black key to eFuse,
* 	please use xilskey_puf_registeration application and refer
*
*	#define		XSK_EFUSEPS_PPK0_HASH
*	"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
*	The value will be converted to a hex buffer and will be written
*	into the ZynqMP Ps eFUSE array. This value should
*	be given in string format. It should be 96 or 64 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered invalid
*	and will not burn PPK0 hash.
*	Note that,for writing the PPK0 hash, XSK_EFUSEPS_WRITE_PPK0_SHA3_HASH
*	should be set to TRUE.
*	While writing SHA3 hash, length should be 96 characters long.
*
*	#define		XSK_EFUSEPS_PPK1_HASH
*	"0000000000000000000000000000000000000000000000000000000000000000"
*	The value will be converted to a hex buffer and will be written
*	into the ZynqMP Ps eFUSE array. This value should
*	be given in string format. It should be 96 or 64 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered invalid
*	and will not burn PPK0 hash.
*	Note that,for writing the PPK1 hash, XSK_EFUSEPS_WRITE_PPK1_SHA3_HASH
*	should be set to TRUE.
*	While writing SHA3 hash, length should be 96 characters long.
*
*	#define		XSK_EFUSEPS_SPK_ID		"00000000"
*	The value  will be converted to a hex buffer and written
*	into the ZynqMP Ps eFUSE array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn SPK ID.
*	Note that,for writing the SPK ID, XSK_EFUSEPS_WRITE_SPKID
*	should be set to TRUE.
*
*	#define		XSK_EFUSEPS_USER0_FUSES		"00000000"
*	The value  will be converted to hex buffer and written
*	into the ZynqMP Ps eFUSE array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn User Fuse.
*	Note that,for writing the User0 Fuse, XSK_EFUSEPS_WRITE_USER0_FUSE
*	should be set to TRUE.
*
*	#define		XSK_EFUSEPS_USER1_FUSES		"00000000"
*	The value  will be converted to hex buffer and written
*	into the ZynqMP Ps eFUSE array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn User Fuse.
*	Note that,for writing the User1 Fuse, XSK_EFUSEPS_WRITE_USER1_FUSE
*	should be set to TRUE.
*
*	#define		XSK_EFUSEPS_USER2_FUSES		"00000000"
*	The value will be converted to hex buffer and written
*	into the ZynqMP Ps eFUSE array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn User Fuse.
*	Note that,for writing the User2 Fuse, XSK_EFUSEPS_WRITE_USER2_FUSE
*	should be set to TRUE.
*
*	#define		XSK_EFUSEPS_USER3_FUSES		"00000000"
*	The value will be converted to hex buffer and written
*	into the ZynqMP Ps eFUSE array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn User Fuse.
*	Note that,for writing the User3 Fuse, XSK_EFUSEPS_WRITE_USER3_FUSE
*	should be set to TRUE.
*
*	#define		XSK_EFUSEPS_USER4_FUSES		"00000000"
*	The value will be converted to hex buffer and written
*	into the ZynqMP Ps eFUSE array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn User Fuse.
*	Note that,for writing the User4 Fuse, XSK_EFUSEPS_WRITE_USER4_FUSE
*	should be set to TRUE.
*
*	#define		XSK_EFUSEPS_USER5_FUSES		"00000000"
*	The value will be converted to hex buffer and written
*	into the ZynqMP Ps eFUSE array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn User Fuse.
*	Note that,for writing the User5 Fuse, XSK_EFUSEPS_WRITE_USER5_FUSE
*	should be set to TRUE.
*
*	#define		XSK_EFUSEPS_USER6_FUSES		"00000000"
*	The value will be converted to hex buffer and written
*	into the ZynqMP Ps eFUSE array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn User Fuse.
*	Note that,for writing the User6 Fuse, XSK_EFUSEPS_WRITE_USER6_FUSE
*	should be set to TRUE.
*
*	#define		XSK_EFUSEPS_USER7_FUSES		"00000000"
*	The value  will be converted to hex buffer and written
*	into the ZynqMP Ps eFUSE array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn User Fuse.
*	Note that,for writing the User7 Fuse, XSK_EFUSEPS_WRITE_USER7_FUSE
*	should be set to TRUE.
*
*	#define XSK_EFUSEPS_CHECK_AES_KEY_CRC		FALSE
*	Default value is FALSE
*	TRUE will check the CRC provided in XSK_EFUSEPS_AES_KEY.
*	CRC verification is done after programming AES key to verify the key
*	is programmed properly or not, if not library error outs the same.
*	So While programming AES key it is not necessary to verify
*	the AES key again.
*	NOTE:
*	Please make sure if intention is to check only CRC of the provided key
*	and not programming AES key then do not modify XSK_EFUSEPS_WRITE_AES_KEY
*	(TRUE will Program key).
*
*	NOTE: The PPK hash should be the unmodified hash generated by bootgen.
*	Single bit programming is allowed for User FUSEs (0 through 7), however
*	if the user specifies a value that tries to set a bit that was previously
*	programmed to 1 back to 0, throws an error.
*   Even if the bits are already programmed user must pass these already
*   programmed bits along with the new bits that need to be programmed.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 4.0   vns     10/01/15 First release
* 6.0   vns     07/18/16 Removed JTAG user code programming and reading
*                        feature. Modified User FUSE programming, such that
*                        library accepts separate requests for programming
*                        and reading USER FUSES(0 to 7). Provided single bit
*                        programming feature for USER FUSEs.
* 6.2   vns     03/10/17 Modified XSK_EFUSEPs_FORCE_USE_AES_ONLY->
*                        XSK_EFUSEPS_ENC_ONLY,
*                        XSK_EFUSEPS_ERR_OUTOF_PMU_DISABLE ->
*                        XSK_EFUSEPS_ERR_DISABLE,
*                        XSK_EFUSEPS_PPK0_REVOKE->XSK_EFUSEPS_PPK0_INVLD
*                        XSK_EFUSEPS_PPK1_REVOKE->XSK_EFUSEPS_PPK1_INVLD
*                        Added support for programming LBIST, LPD and FPD
*                        SC enable bits by providing corresponding macros
*                        Removed 3 macros for PROG GATE disable, now it can
*                        programmed by setting only one macro.
* 6.7	psl     03/13/19 Added XSK_EFUSEPS_CHECK_AES_KEY_CRC, to check for
* 						 AES key CRC if TRUE.
* 	psl     03/28/19 Updated Description for XSK_EFUSEPS_CHECK_AES_KEY_CRC
* 6.8   psl     06/07/19 Added doxygen tags.
*       vns     08/30/19 Corrected string for PPK1 hash to 384 bit
* 6.9   har     06/17/20 Removed macros XSK_EFUSEPS_PPK0_IS_SHA3 and
*                        XSK_EFUSEPS_PPK1_IS_SHA3
* 7.1   kpt     05/11/21 Added BareMetal support for programming PUF Fuses as
*                        general purpose Fuses
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
#define XSK_EFUSEPS_ENC_ONLY		FALSE
#define XSK_EFUSEPS_BBRAM_DISABLE		FALSE
#define XSK_EFUSEPS_ERR_DISABLE			FALSE
#define XSK_EFUSEPS_JTAG_DISABLE		FALSE
#define XSK_EFUSEPS_DFT_DISABLE			FALSE
#define XSK_EFUSEPS_PROG_GATE_DISABLE		FALSE
#define XSK_EFUSEPS_SECURE_LOCK			FALSE
#define XSK_EFUSEPS_RSA_ENABLE			FALSE
#define XSK_EFUSEPS_PPK0_WR_LOCK		FALSE
#define XSK_EFUSEPS_PPK0_INVLD			FALSE
#define XSK_EFUSEPS_PPK1_WR_LOCK		FALSE
#define XSK_EFUSEPS_PPK1_INVLD			FALSE
#define XSK_EFUSEPS_LBIST_EN			FALSE
#define XSK_EFUSEPS_LPD_SC_EN			FALSE
#define XSK_EFUSEPS_FPD_SC_EN			FALSE
#define XSK_EFUSEPS_PBR_BOOT_ERR		FALSE

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
 * Following is the define to select if the user wants to select AES key,
 * User Fuses, PPK0 Sha3 hash, PPK1 Sha3 hash and SPKID for Zynq MP
 */
/* For writing into eFuse */
#define XSK_EFUSEPS_WRITE_AES_KEY		FALSE
#define XSK_EFUSEPS_WRITE_PPK0_HASH	FALSE
#define XSK_EFUSEPS_WRITE_PPK1_HASH	FALSE
#define XSK_EFUSEPS_WRITE_SPKID			FALSE

#define XSK_EFUSEPS_WRITE_USER0_FUSE		FALSE
#define XSK_EFUSEPS_WRITE_USER1_FUSE		FALSE
#define XSK_EFUSEPS_WRITE_USER2_FUSE		FALSE
#define XSK_EFUSEPS_WRITE_USER3_FUSE		FALSE
#define XSK_EFUSEPS_WRITE_USER4_FUSE		FALSE
#define XSK_EFUSEPS_WRITE_USER5_FUSE		FALSE
#define XSK_EFUSEPS_WRITE_USER6_FUSE		FALSE
#define XSK_EFUSEPS_WRITE_USER7_FUSE		FALSE

#if defined (XSK_ACCESS_PUF_USER_EFUSE)
#define XSK_EFUSEPS_WRITE_PUF_FUSE FALSE
#define XSK_EFUSEPS_READ_PUF_FUSE  FALSE

#define XSK_EFUSEPS_PUF_START_ROW  1U
#define XSK_EFUSEPS_PUF_NUM_OF_ROWS 1U
/**
 * For row 1 and row 65 upper 16 bits are ignored
 * i.e 0th row in efuse page 2 and page 3
 * For row 128 only upper 28bits are valid
 * i.e 63rd row in efuse page 3
 */
#define XSK_EFUSEPS_PUF_FUSE_DATA   "00000000"

#define XSK_EFUSEPS_PUF_READ_START_ROW XSK_EFUSEPS_PUF_START_ROW
#define XSK_EFUSEPS_PUF_READ_NUM_OF_ROWS  XSK_EFUSEPS_PUF_NUM_OF_ROWS

#endif

/**
 * Following defines should be given in the form of hex string.
 * The length of AES_KEY string must be 64, PPK hash should be 96 for
 * SHA3 selection and and for USER_FUSES, SPK ID  must be 32.
 */
#define XSK_EFUSEPS_AES_KEY		"0000000000000000000000000000000000000000000000000000000000000000"

#define XSK_EFUSEPS_PPK0_HASH	"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

#define XSK_EFUSEPS_PPK1_HASH	"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
#define XSK_EFUSEPS_SPK_ID		"00000000"

#define XSK_EFUSEPS_USER0_FUSES		"00000000"
#define XSK_EFUSEPS_USER1_FUSES		"00000000"
#define XSK_EFUSEPS_USER2_FUSES		"00000000"
#define XSK_EFUSEPS_USER3_FUSES		"00000000"
#define XSK_EFUSEPS_USER4_FUSES		"00000000"
#define XSK_EFUSEPS_USER5_FUSES		"00000000"
#define XSK_EFUSEPS_USER6_FUSES		"00000000"
#define XSK_EFUSEPS_USER7_FUSES		"00000000"

/* Checks CRC of provided AES key if TRUE */
#define XSK_EFUSEPS_CHECK_AES_KEY_CRC		FALSE

#ifdef __cplusplus
}
#endif

#endif /* XILSKEY_EFUSEPS_ZYNQMP_INPUT_H_ */
