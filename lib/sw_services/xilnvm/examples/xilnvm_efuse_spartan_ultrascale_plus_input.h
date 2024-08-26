/******************************************************************************
* Copyright (c) 2024, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilnvm_efuse_spartan_ultrascale_input.h.
* This file contains macros which needs to configured by user based on the
* options selected by user operations will be performed.
*
* @note
*
*	User configurable parameters for spartan eFUSE
*------------------------------------------------------------------------------
*
*	Following has to be set for programming required keys/data
*------------------------------------------------------------------------------
*	#define XNVM_EFUSE_WRITE_AES_KEY		FALSE
*	TRUE will burn the AES key provided in XNVM_EFUSE_AES_KEY.
*	FALSE will ignore the key provided in XNVM_EFUSE_AES_KEY.
*
* 	#define XNVM_EFUSE_WRITE_DEC_EFUSE_ONLY		FALSE
* 	TRUE will burn the lower 16 bits of Security Misc 0 eFuse row.
*	FALSE will not modify the lower 16 bits of Security Misc 0 eFuse row.
*
*	#define XNVM_EFUSE_WRITE_AES_PLM_IV		FALSE
*	TRUE will burn the Plm IV provided in XNVM_EFUSE_PLM_IV.
*	FALSE will ignore the data provided in XNVM_EFUSE_PLM_IV.
*
* 	#define XNVM_EFUSE_WRITE_PPK0_HASH		FALSE
* 	TRUE will burn PPK0 with it's SHA3 hash provided in
* 		XNVM_EFUSE_PPK0_SHA3_HASH.
*	FALSE will ignore the hash provided in XSK_EFUSEPS_PPK0_SHA3_HASH.
*
*	#define XNVM_EFUSE_WRITE_PPK1_HASH		FALSE
*	TRUE will burn PPK1 with it's SHA3 hash provided in
*		XNVM_EFUSE_PPK1_HASH.
*	FALSE will ignore the hash provided in XNVM_EFUSE_PPK1_HASH.
*
*	#define XNVM_EFUSE_WRITE_PPK2_HASH		FALSE
*	TRUE will burn PPK2 with it's SHA3 hash provided in
*		XNVM_EFUSE_PPK2_HASH.
*	FALSE will ignore the hash provided in XNVM_EFUSEPS_PPK2_HASH.
*
*	#define		XNVM_EFUSE_AES_KEY
*	"0000000000000000000000000000000000000000000000000000000000000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the Versal eFuse array when write API used. This value should
*	be given in string format. It should be 64 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as
*	invalid string and will not burn any eFuses.
*	Note that,for writing the AES Key, XNVM_EFUSE_WRITE_AES_KEY should
*	be set to TRUE.
*	NOTE: This AES key is only red key or gray key.
*	To program black key to eFuse, please use xnvm_puf_registration
*
*	#define		XNVM_EFUSE_PPK0_HASH
*	"0000000000000000000000000000000000000000000000000000000000000000"
*	The value will be converted to a hex buffer and will be written
*	into the Versal eFUSE array. This value should
*	be given in string format. It should be 64 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered invalid
*	and will not burn PPK0 hash.
*	Note that,for writing the PPK0 hash, XNVM_EFUSE_WRITE_PPK0_HASH
*	should be set to TRUE.
*
*	#define		XNVM_EFUSE_PPK1_HASH
*	"0000000000000000000000000000000000000000000000000000000000000000"
*	The value will be converted to a hex buffer and will be written
*	into the Versal eFuse array. This value should
*	be given in string format. It should be 64 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered invalid
*	and will not burn PPK1 hash.
*	Note that,for writing the PPK1 hash, XNVM_EFUSE_WRITE_PPK1_HASH
*	should be set to TRUE.
*
* 	#define		XNVM_EFUSE_PPK2_HASH
*	"0000000000000000000000000000000000000000000000000000000000000000"
*	The value will be converted to a hex buffer and will be written
*	into the Versal eFuse array. This value should
*	be given in string format. It should be 64 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered invalid
*	and will not burn PPK2 hash.
*	Note that,for writing the PPK2 hash, XNVM_EFUSE_WRITE_PPK2_HASH
*	should be set to TRUE.
*
*	#define 	XNVM_EFUSE_AES_PLM_IV
*	"000000000000000000000000"
*	The value will be converted to a hex buffer and will be written
*	into the Versal eFuse array. This value should
*	be given in string format. It should be 24 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered invalid
*	and will not burn Metaheader IV.
*	Note that,for writing the PLM IV, XNVM_EFUSE_WRITE_PLM_IV
*	should be set to TRUE.
*	While writing Plm IV, length should be 24 characters long.
*
*	#define		XNVM_EFUSE_REVOCATION_ID_0_FUSES	"00000000"
*	The value  will be converted to hex buffer and written
*	into the Versal eFuse array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn Revocation Id 0 eFuse.
*	Note that,for writing the Revocation Id 0 eFuse,
*	XNVM_EFUSE_WRITE_REVOCATION_ID_0 should be set to TRUE.
*
*	#define		XNVM_EFUSE_REVOCATION_ID_1_FUSES	"00000000"
*	The value  will be converted to hex buffer and written
*	into the Versal eFuse array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn Revocation Id 1 eFuse.
*	Note that,for writing the Revocation Id 1 eFuse,
*	XNVM_EFUSE_WRITE_REVOCATION_ID_1 should be set to TRUE.
*
*	#define		XNVM_EFUSE_REVOCATION_ID_2_FUSES	"00000000"
*	The value will be converted to hex buffer and written
*	into the Versal eFUSE array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn the Revocation Id 2 Fuse.
*	Note that,for writing the Revocation Id 2 Fuse,
*	XNVM_EFUSE_WRITE_REVOCATION_ID_2 should be set to TRUE.
*
*	#define XNVM_EFUSE_CHECK_AES_KEY_CRC			FALSE
*	Default value is FALSE
*	TRUE will validate AES key stored in eFuse by calculating its CRC
*	and comparing with CRC provided in XNVM_EFUSE_EXPECTED_AES_KEY_CRC.
*	CRC verification is done after programming the key to verify the key
*	is programmed properly or not, if not library error outs the same.
*	NOTE:
*	Please make sure if intention is to check only CRC of the provided key
*	and not programming AES key then do not modify XNVM_EFUSE_WRITE_AES_KEY
*	(TRUE will Program key).
*	This check is only applicable for server mode of eFuse example
*
*	#define XNVM_EFUSE_EXPECTED_AES_KEY_CRC		XNVM_EFUSE_CRC_AES_ZEROS
*	This is expected crc of the programmed Aes key given in hexa decimal.
*	Default is XNVM_EFUSE_CRC_AES_ZEROS Crc of the zero key
*
*	NOTE: The PPK hash should be the unmodified hash generated by bootgen.
*	Single bit programming is allowed for User eFuses (1 through 63),
*	however if the user specifies a value that tries to set a bit that was
*	previously programmed to 1 back to 0, program throws an error.
*	Even if the bits are already programmed user must pass these already
*	programmed bits along with the new bits that need to be programmed.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date	Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   kal    03/02/20 First release to add different configurable options
*
* </pre>
*
******************************************************************************/

#ifndef XILNVM_EFUSE_VERSAL_INPUT_H_
#define XILNVM_EFUSE_VERSAL_INPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/* Following defines should be defined either TRUE or FALSE */

/**
 * Following is the define to select if the user wants to program
 * Secure control bits
 */
#define XNVM_EFUSE_XNVM_EFUSE_AES_CM_DIS        FALSE
#define XNVM_EFUSE_XNVM_EFUSE_AES_DIS           FALSE
#define XNVM_EFUSE_XNVM_AES_RD_LK               FALSE
#define XNVM_EFUSE_XNVM_AES_WR_LK               FALSE
#define XNVM_EFUSE_XNVM_PPK0_LK                 FALSE
#define XNVM_EFUSE_XNVM_PPK1_LK                 FALSE
#define XNVM_EFUSE_XNVM_PPK2_LK                 FALSE
#define XNVM_EFUSE_XNVM_JTAG_DIS                FALSE
#define XNVM_EFUSE_XNVM_USER_WR_LK              FALSE
#define XNVM_EFUSE_XNVM_MEM_CLR_EN              FALSE
#define XNVM_EFUSE_XNVM_DNA_WR_LK               FALSE
#define XNVM_EFUSE_XNVM_JTAG_ERR_DIS            FALSE
#define XNVM_EFUSE_XNVM_JTAG_DIS                FALSE
#define XNVM_EFUSE_XNVM_SCAN_CLR_EN             FALSE
#define XNVM_EFUSE_XNVM_HASH_PUF_OR_KEY         FALSE
#define XNVM_EFUSE_XNVM_AXI_DIS                 FALSE
#define XNVM_EFUSE_XNVM_MDM_DIS                 FALSE
#define XNVM_EFUSE_XNVM_ICAP_DIS                FALSE
#define XNVM_EFUSE_XNVM_RMA_DIS                 FALSE
#define XNVM_EFUSE_XNVM_RMA_EN                  FALSE
#define XNVM_EFUSE_XNVM_CRC_EN                  FALSE
#define XNVM_EFUSE_XNVM_DFT_DIS                 FALSE
#define XNVM_EFUSE_XNVM_LCKDWN_EN               FALSE
#define XNVM_EFUSE_XNVM_PUF_TEST_2_DIS          FALSE
#define XNVM_EFUSE_XNVM_PPK0_INVLD              FALSE
#define XNVM_EFUSE_XNVM_PPK1_INVLD              FALSE
#define XNVM_EFUSE_XNVM_PPK2_INVLD              FALSE
#define XNVM_EFUSE_XNVM_EXP_CTRL                FALSE

/**
 * Following is the define to select if the user wants to select
 * AES key, User Fuses, PPK0/PPK1/PPK2 hash, IVs, Revocation IDs and IVs
 */
/* For writing into eFuse */
#define XNVM_EFUSE_WRITE_AES_KEY		FALSE

#define XNVM_EFUSE_WRITE_PPK0_HASH		FALSE
#define XNVM_EFUSE_WRITE_PPK1_HASH		FALSE
#define XNVM_EFUSE_WRITE_PPK2_HASH		FALSE

#define XNVM_EFUSE_WRITE_DEC_EFUSE_ONLY	FALSE

#define XNVM_EFUSE_WRITE_AES_IV			FALSE

#define XNVM_EFUSE_WRITE_USER_FUSES		FALSE

#define XNVM_EFUSE_WRITE_REVOKE_ID      FALSE

#define XNVM_EFUSE_WRITE_AES_REVOKE_ID  FALSE

/**
 * Following defines should be given in the form of hex string.
 * The length of AES_KEY string must be 64, PPK hash should be 64 for
 * and for USER_FUSES.
 */

#define XNVM_EFUSE_AES_KEY	"0000000000000000000000000000000000000000000000000000000000000000"

#define XNVM_EFUSE_PPK0_HASH	"0000000000000000000000000000000000000000000000000000000000000000"

#define XNVM_EFUSE_PPK1_HASH	"0000000000000000000000000000000000000000000000000000000000000000"

#define XNVM_EFUSE_PPK2_HASH	"0000000000000000000000000000000000000000000000000000000000000000"

#define XNVM_EFUSE_AES_IV		"000000000000000000000000"


#define XNVM_EFUSE_REVOCATION_ID_0_FUSES	"00000000"
#define XNVM_EFUSE_REVOCATION_ID_1_FUSES	"00000000"
#define XNVM_EFUSE_REVOCATION_ID_2_FUSES	"00000000"

#define XNVM_EFUSE_AES_REVOCATION_ID_EFUSE  "00000000"

#define XNVM_EFUSE_USER_FUSE			"00000000"

/* Checks CRC of provided AES key if TRUE */
#define XNVM_EFUSE_CHECK_AES_KEY_CRC		FALSE

#define XNVM_EFUSE_EXPECTED_AES_KEY_CRC		XNVM_EFUSE_CRC_AES_ZEROS

#ifdef __cplusplus
}
#endif

#endif /* XILNVM_EFUSE_VERSAL_INPUT_H_ */