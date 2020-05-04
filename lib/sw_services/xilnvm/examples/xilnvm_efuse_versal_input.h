/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xilnvm_efuse_versal_input.h.
* This file contains macros which needs to configured by user based on the
* options selected by user operations will be performed.
*
* @note
*
*	User configurable parameters for Versal eFUSE
*------------------------------------------------------------------------------
*	#define XNVM_EFUSE_PPK0_WR_LK			FALSE
*	TRUE permanently disables writing to PPK0 eFuse.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XNVM_EFUSE_PPK1_WR_LK			FALSE
*	TRUE permanently disables writing to PPK1 eFuse.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XNVM_EFUSE_PPK2_WR_LK			FALSE
*	TRUE permanently disables writing to PPK2 eFuse.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XNVM_EFUSE_AES_CRC_LK			FALSE
*	TRUE permanently disables the CRC checks on AES key.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XNVM_EFUSE_AES_WR_LK			FALSE
*	TRUE permanently disables writing to AES key
*	FALSE will not modify this control bit of eFuse.
*
*	#define XNVM_EFUSE_USER_KEY_0_CRC_LK		FALSE
*	TRUE permanently disables the CRC checks on User 0 key.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XNVM_EFUSE_USER_KEY_0_WR_LK		FALSE
*	TRUE permanently disables writing to User 0 key
*	FALSE will not modify this control bit of eFuse.
*
*	#define XNVM_EFUSE_USER_KEY_1_CRC_LK		FALSE
*	TRUE permanently disables the CRC checks on User 1 key.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XNVM_EFUSE_USER_KEY_1_WR_LK		FALSE
*	TRUE permanently disables writing to User 1 key
*	FALSE will not modify this control bit of eFuse.
*
*	#define XNVM_EFUSE_PPK0_INVLD			FALSE
*	TRUE invalidates the PPK0 hash stored in eFuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XNVM_EFUSE_PPK1_INVLD			FALSE
*	TRUE invalidates the PPK1 hash stored in eFuses.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XNVM_EFUSE_PPK2_INVLD			FALSE
*	TRUE invalidates the PPK2 hash stored in eFuses.
*	FALSE will not modify this control bit of eFuse.
*
*	Following has to be set for programming required keys
*------------------------------------------------------------------------------
*	#define XNVM_EFUSE_WRITE_AES_KEY		FALSE
*	TRUE will burn the AES key provided in XNVM_EFUSE_AES_KEY.
*	FALSE will ignore the key provided in XNVM_EFUSE_AES_KEY.
*
* 	#define XNVM_EFUSE_WRITE_USER_KEY_0		FALSE
* 	TRUE will burn the User key 0 provided in XNVM_EFUSE_USER_KEY_0.
* 	FALSE will ignore the key provided in XNVM_EFUSE_USER_KEY_0.
*
* 	#define XNVM_EFUSE_WRITE_USER_KEY_1		FALSE
* 	TRUE will burn the User key 1 provided in XNVM_EFUSE_USER_KEY_1.
* 	FALSE will ignore the key provided in XNVM_EFUSE_USER_KEY_1.
*
* 	#define XNVM_EFUSE_WRITE_DEC_EFUSE_ONLY		FALSE
* 	TRUE will burn the Dec_Efuse_Only Efuse bits provided in
* 		XNVM_EFUSE_WRITE_DEC_EFUSE_ONLY to Security Misc 0 Efuse.
*	FALSE will ignore the data provided in XNVM_EFUSE_WRITE_DEC_EFUSE_ONLY
*
* 	#define XNVM_EFUSE_WRITE_METAHEADER_IV		FALSE
* 	TRUE will burn the MetaHeader IV provided in XNVM_EFUSE_METAHEADER_IV
* 	FALSE will ignore the data provided in XNVM_EFUSE_METAHEADER_IV
*
*	#define XNVM_EFUSE_WRITE_BLACK_OBFUS_IV		FALSE
*	TRUE will burn Black Obfus IV provided in XNVM_EFUSE_BLACK_OBFUS_IV.
*	FALSE will ignore the data provided in XNVM_EFUSE_BLACK_OBFUS_IV.
*
*	#define XNVM_EFUSE_WRITE_PLM_IV		FALSE
*	TRUE will burn the Plml IV provided in XNVM_EFUSE_PLM_IV.
*	FALSE will ignore the data provided in XNVM_EFUSE_PLM_IV.
*
*	#define XNVM_EFUSE_WRITE_DATA_PARTITION_IV	FALSE
*	TRUE will burn the Data Partition IV in  XNVM_EFUSE_DATA_PARTITION_IV.
*	FALSE will ignore the data provided in XNVM_EFUSE_DATA_PARTITION_IV.
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
*	#define XNVM_EFUSE_WRITE_REVOCATION_ID_0	FALSE
*	TRUE will burn Revocation Id 0 eFuse with the data provided in
*		XNVM_EFUSE_REVOCATION_ID_0 FUSES.
*	FALSE will ignore the value provided in XNVM_EFUSE_REVOCATION_ID_0
*
*	#define XNVM_EFUSE_WRITE_REVOCATION_ID_1	FALSE
*	TRUE will burn Revocation Id 1 eFuse with the data provided in
*		XNVM_EFUSE_REVOCATION_ID_1 FUSES.
*	FALSE will ignore the value provided in XNVM_EFUSE_REVOCATION_ID_1
*
*	#define XNVM_EFUSE_WRITE_REVOCATION_ID_2		FALSE
*	TRUE will burn Revocation Id 2 eFuse with the data provided in
*		XNVM_EFUSE_REVOCATION_ID_2 FUSES.
*	FALSE will ignore the value provided in XNVM_EFUSE_REVOCATION_ID_2.
*
*	#define XNVM_EFUSE_WRITE_REVOCATION_ID_3		FALSE
*	TRUE will burn Revocation Id 3 eFuse with the data provided in
*		XNVM_EFUSE_REVOCATION_ID_3 FUSES.
*	FALSE will ignore the value provided in XNVM_EFUSE_REVOCATION_ID_3
*
*	#define XNVM_EFUSE_WRITE_REVOCATION_ID_4		FALSE
*	TRUE will burn Revocation Id 4 eFuse with the data provided in
*		XNVM_EFUSE_REVOCATION_ID_4 FUSES.
*	FALSE will ignore the value provided in XNVM_EFUSE_REVOCATION_ID_4
*
*	#define XNVM_EFUSE_WRITE_REVOCATION_ID_5		FALSE
*	TRUE will burn Revocation Id 5 eFuse with the data provided in
*		XNVM_EFUSE_REVOCATION_ID_5 FUSES.
*	FALSE will ignore the value provided in XNVM_EFUSE_REVOCATION_ID_5
*
*	#define XNVM_EFUSE_WRITE_REVOCATION_ID_6		FALSE
*	TRUE will burn Revocation Id 6 eFuse with the data provided in
*		XNVM_EFUSE_REVOCATION_ID_6 FUSES.
*	FALSE will ignore the value provided in XNVM_EFUSE_REVOCATION_ID_6.
*
*	#define XNVM_EFUSE_WRITE_REVOCATION_ID_7		FALSE
*	TRUE will burn Revocation Id 7 eFuse with the data provided in
*		XNVM_EFUSE_REVOCATION_ID_7 FUSES.
*	FALSE will ignore the value provided in XNVM_EFUSE_REVOCATION_ID_7.
*
*	#define XNVM_EFUSE_WRITE_USER_FUSES			FALSE
*	TRUE will burn User eFuses provided in XNVM_EFUSE_USER_FUSES.
*	FALSE will ignore the value provided in XNVM_EFUSE_USER_FUSES.
*
*	Note: To program user eFuses you have to give the start User fuse number
*	to be programmed at XNVM_EFUSE_PRGM_USER_FUSE_NUM and starting with
*	that number of user fuses to be programmed should be mentioned at
*	XNVM_EFUSE_NUM_OF_USER_FUSES.
*	XNVM_EFUSE_USER_FUSES string should be multiples of 8 bytes always.
*	Which means that if the string is 16 bytes, it will program 2 user
*	eFuses.
*	XNVM_EFUSE_PRGM_USER_FUSE_NUM range is  1 to 63.
*
*	Example :
*	When XNVM_EFUSE_WRITE_USER_FUSES is TRUE and
*	XNVM_EFUSE_PRGM_USER_FUSE_NUM is 4 - User eFuse number from where to
*						start write.
*	XNVM_EFUSE_NUM_OF_USER_FUSES is 2 - Number of User eFuses to program
*	XNVM_EFUSE_USER_FUSES "1234578ABCDEF58" - User eFuse data to program
*	UserFuse 4 will be programmed with 0x1234578 and
*	UserFuse 5 will be programmed with 0xABCDEF58.
*
*	#define XNVM_EFUSE_READ_USER_FUSE_NUM	XNVM_EFUSE_PRGM_USER_FUSE_NUM
*	XNVM_EFUSE_READ_USER_FUSE_NUM - User eFuse number from where to
*					start read.
*	By default it reads from XNVM_EFUSE_PRGM_USER_FUSE_NUM
*
*	#define XNVM_EFUSE_READ_NUM_OF_USER_FUSES  XNVM_EFUSE_NUM_OF_USER_FUSES
*	XNVM_EFUSE_READ_NUM_OF_USER_FUSES - Number of eFuses to be read
*	By default it reads XNVM_EFUSE_NUM_OF_USER_FUSES
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
*	#define		XNVM_EFUSE_USER_KEY_0
*	"0000000000000000000000000000000000000000000000000000000000000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the Versal eFuse array when write API used. This value should
*	be given in string format. It should be 64 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as
*	invalid string and will not burn any eFuses.
*	Note that,for writing the User Key 0, XNVM_EFUSE_WRITE_USER_KEY_0
*	should be set to TRUE.
*	NOTE: This AES key is only red key or gray key.
*	To program black key to eFuse, please use xnvm_puf_registration
*
*	#define		XNVM_EFUSE_USER_KEY_1
*	"0000000000000000000000000000000000000000000000000000000000000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the Versal eFUSE array when write API used. This value should
*	be given in string format. It should be 64 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as
*	invalid string and will not burn any eFuses.
*	Note that,for writing the User Key 1, XNVM_EFUSE_WRITE_USER_KEY_1
*	should be set to TRUE.
*	NOTE: This AES key is only red key or gray key.
*	To program black key to eFuse, please use xnvm_puf_registration.
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
*	#define		XNVM_EFUSE_DEC_EFUSE_ONLY		"00000000"
*	The value  will be converted to a hex buffer and written
*	into the Versal eFuse array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as
*	invalid string and will not burn DEC_EFUSE_ONLY eFuse.
*	Note that,for writing the DEC_EFUSE_ONLY,
*	XNVM_EFUSE_WRITE_DEC_EFUSE_ONLY should be set to TRUE.
*
*	#define 	XNVM_EFUSE_META_HEADER_IV
*	"000000000000000000000000"
*	The value will be converted to a hex buffer and will be written
*	into the Versal eFuse array. This value should
*	be given in string format. It should be 24 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered invalid
*	and will not burn Metaheader IV.
*	Note that,for writing the Metaheader IV, XNVM_EFUSE_WRITE_METAHEADER_IV
*	should be set to TRUE.
*	While writing Metaheader IV, length should be 24 characters long.
*
*	#define 	XNVM_EFUSE_PLM_IV
*	"000000000000000000000000"
*	The value will be converted to a hex buffer and will be written
*	into the Versal eFuse array. This value should
*	be given in string format. It should be 24 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered invalid
*	and will not burn Metaheader IV.
*	Note that,for writing the PLM IV, XNVM_EFUSE_WRITE_PLM_IV
*	should be set to TRUE.
*	While writing Plml IV, length should be 24 characters long.
*
*	#define 	XNVM_EFUSE_BLACK_OBFUS_IV
*	"000000000000000000000000"
*	The value will be converted to a hex buffer and will be written
*	into the Versal eFUSE array. This value should
*	be given in string format. It should be 24 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered invalid
*	and will not burn Black Obfus IV.
*	Note that,for writing the Black Obfus IV,
*	XNVM_EFUSE_WRITE_BLACK_OBFUS_IV should be set to TRUE.
*	While writing Black Obfus IV, length should be 24 characters long.
*
*	#define 	XNVM_EFUSE_DATA_PARTITION_IV
*	"000000000000000000000000"
*	The value will be converted to a hex buffer and will be written
*	into the Versal eFuse array. This value should
*	be given in string format. It should be 24 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered invalid
*	and will not burn Data Partition IV.
*	Note that,for writing the Data Partition IV,
*	XNVM_EFUSE_WRITE_DATA_PARTITION_IV should be set to TRUE.
*	While writing Data Partition IV, length should be 24 characters long.
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
*	#define		XNVM_EFUSE_REVOCATION_ID_3_FUSES	"00000000"
*	The value will be converted to hex buffer and written
*	into the Versal eFuse array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn Revocation Id 3 eFuse.
*	Note that,for writing the Revocation Id 3 eFuse,
*	XNVM_EFUSE_WRITE_REVOCATION_ID_3 should be set to TRUE.
*
*	#define		XNVM_EFUSE_REVOCATION_ID_4_FUSES	"00000000"
*	The value will be converted to hex buffer and written
*	into the Versal eFuse array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn Revocation Id 4 eFuse.
*	Note that,for writing the Revocation Id 4 eFuse,
*	XNVM_EFUSE_WRITE_REVOCATION_ID_4 should be set to TRUE.
*
*	#define		XNVM_EFUSE_REVOCATION_ID_5_FUSES	"00000000"
*	The value will be converted to hex buffer and written
*	into the Versal eFuse array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn Revocation Id 5 eFuse.
*	Note that,for writing the Revocation Id 5 eFuse,
*	XNVM_EFUSE_WRITE_REVOCATION_ID_5 should be set to TRUE.
*
*	#define		XNVM_EFUSE_REVOCATION_ID_6_FUSES	"00000000"
*	The value will be converted to hex buffer and written
*	into the Versal eFuse array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn Revocation Id 6 eFuse.
*	Note that,for writing the Revocation Id 6 eFuse,
*	XNVM_EFUSE_WRITE_REVOCATION_ID_6 should be set to TRUE.
*
*	#define		XNVM_EFUSE_REVOCATION_ID_7_FUSES	"00000000"
*	The value  will be converted to hex buffer and written
*	into the Versal eFuse array when write API used. This value should
*	be given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn Revocation Id 7 eFuse.
*	Note that,for writing the Revocation Id 7 eFuse,
*	XNVM_EFUSE_WRITE_REVOCATION_ID_7 should be set to TRUE.
*
*	#define
*	#define XNVM_EFUSE_CHECK_AES_KEY_CRC			FALSE
*	Default value is FALSE
*	TRUE will check the CRC provided in XNVM_EFUSE_AES_KEY.
*	CRC verification is done after programming AES key to verify the key
*	is programmed properly or not, if not library error outs the same.
*	So while programming AES key it is not necessary to verify
*	the AES key again.
*	NOTE:
*	Please make sure if intention is to check only CRC of the provided key
*	and not programming AES key then do not modify XNVM_EFUSE_WRITE_AES_KEY
*	(TRUE will Program key).
*
*	#define XNVM_EFUSE_CHECK_USER_KEY_0_CRC			FALSE
*	Default value is FALSE
*	TRUE will check the CRC provided in XNVM_EFUSE_USER_KEY_0.
*	CRC verification is done after programming AES key to verify the key
*	is programmed properly or not, if not library error outs the same.
*	So While programming AES key it is not necessary to verify
*	the AES key again.
*	NOTE:
*	Please make sure if intention is to check only CRC of the provided key
*	and not programming AES key then do not modify
*	XNVM_EFUSE_WRITE_USER_KEY_0 (TRUE will Program key).
*
*	#define XNVM_EFUSE_CHECK_USER_KEY_1_CRC			FALSE
*	Default value is FALSE
*	TRUE will check the CRC provided in XNVM_EFUSE_USER_KEY_1.
*	CRC verification is done after programming AES key to verify the key
*	is programmed properly or not, if not library error outs the same.
*	So While programming AES key it is not necessary to verify
*	the AES key again.
*	NOTE:
*	Please make sure if intention is to check only CRC of the provided key
*	and not programming AES key then do not modify XNVM_EFUSE_WRITE_USER_KEY_1
*	(TRUE will Program key).
*
*	#define XNVM_EFUSE_EXPECTED_AES_KEY_CRC		XNVM_EFUSE_CRC_AES_ZEROS
*	This is expected crc of the programmed Aes key given in hexa decimal.
*	Default is XNVM_EFUSE_CRC_AES_ZEROS Crc of the zero key
*
*	#define XNVM_EFUSE_EXPECTED_USER_KEY0_CRC	XNVM_EFUSE_CRC_AES_ZEROS
*	This is expected crc of the programmed user key0 given in hexa decimal.
*	Default is XNVM_EFUSE_CRC_AES_ZEROS Crc of the zero key
*
*	#define XNVM_EFUSE_EXPECTED_USER_KEY1_CRC	XNVM_EFUSE_CRC_AES_ZEROS
*	This is expected crc of the programmed user key1 given in hexa decimal.
*	Default is XNVM_EFUSE_CRC_AES_ZEROS Crc of the zero key
*
*	NOTE: The PPK hash should be the unmodified hash generated by bootgen.
*	Single bit programming is allowed for Revocation Id eFuses (0 through 7),
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
* 1.0   kal    10/01/20 First release
* 2.0   kal    08/04/20 Addressed Security review comments
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
#include "xnvm_efuse.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/* Following defines should be defined either TRUE or FALSE */

/**
 * Following is the define to select if the user wants to program
 * Secure control bits
 */
#define XNVM_EFUSE_PPK0_WR_LK			FALSE
#define XNVM_EFUSE_PPK1_WR_LK			FALSE
#define	XNVM_EFUSE_PPK2_WR_LK			FALSE
#define XNVM_EFUSE_AES_CRC_LK			FALSE
#define XNVM_EFUSE_AES_WR_LK			FALSE
#define XNVM_EFUSE_USER_KEY_0_CRC_LK		FALSE
#define XNVM_EFUSE_USER_KEY_0_WR_LK		FALSE
#define XNVM_EFUSE_USER_KEY_1_CRC_LK		FALSE
#define XNVM_EFUSE_USER_KEY_1_WR_LK		FALSE
#define XNVM_EFUSE_PPK0_INVLD			FALSE
#define XNVM_EFUSE_PPK1_INVLD			FALSE
#define XNVM_EFUSE_PPK2_INVLD			FALSE

/**
 * Following is the define to select if the user wants to select AES key,
 * User Fuses, PPK0/PPK1/PPK2 hash, IVs, Revocation IDs and IVs
 */
/* For writing into eFuse */
#define XNVM_EFUSE_WRITE_AES_KEY		FALSE
#define XNVM_EFUSE_WRITE_USER_KEY_0		FALSE
#define XNVM_EFUSE_WRITE_USER_KEY_1		FALSE

#define XNVM_EFUSE_WRITE_PPK0_HASH		FALSE
#define XNVM_EFUSE_WRITE_PPK1_HASH		FALSE
#define XNVM_EFUSE_WRITE_PPK2_HASH		FALSE

#define XNVM_EFUSE_WRITE_DEC_EFUSE_ONLY		FALSE

#define XNVM_EFUSE_WRITE_METAHEADER_IV		FALSE
#define XNVM_EFUSE_WRITE_BLACK_OBFUS_IV		FALSE
#define XNVM_EFUSE_WRITE_PLM_IV			FALSE
#define XNVM_EFUSE_WRITE_DATA_PARTITION_IV	FALSE

#define XNVM_EFUSE_WRITE_REVOCATION_ID_0	FALSE
#define XNVM_EFUSE_WRITE_REVOCATION_ID_1	FALSE
#define XNVM_EFUSE_WRITE_REVOCATION_ID_2	FALSE
#define XNVM_EFUSE_WRITE_REVOCATION_ID_3	FALSE
#define XNVM_EFUSE_WRITE_REVOCATION_ID_4	FALSE
#define XNVM_EFUSE_WRITE_REVOCATION_ID_5	FALSE
#define XNVM_EFUSE_WRITE_REVOCATION_ID_6	FALSE
#define XNVM_EFUSE_WRITE_REVOCATION_ID_7	FALSE

#define XNVM_EFUSE_WRITE_USER_FUSES		FALSE
/**
 * Following defines should be given in the form of hex string.
 * The length of AES_KEY string must be 64, PPK hash should be 64 for
 * and for USER_FUSES.
 */
#define XNVM_EFUSE_AES_KEY	"0000000000000000000000000000000000000000000000000000000000000000"

#define XNVM_EFUSE_USER_KEY_0	"0000000000000000000000000000000000000000000000000000000000000000"

#define XNVM_EFUSE_USER_KEY_1	"0000000000000000000000000000000000000000000000000000000000000000"

#define XNVM_EFUSE_PPK0_HASH	"0000000000000000000000000000000000000000000000000000000000000000"

#define XNVM_EFUSE_PPK1_HASH	"0000000000000000000000000000000000000000000000000000000000000000"

#define XNVM_EFUSE_PPK2_HASH	"0000000000000000000000000000000000000000000000000000000000000000"

#define XNVM_EFUSE_META_HEADER_IV	"000000000000000000000000"

#define XNVM_EFUSE_BLACK_OBFUS_IV	"000000000000000000000000"

#define XNVM_EFUSE_PLM_IV		"000000000000000000000000"

#define XNVM_EFUSE_DATA_PARTITION_IV	"000000000000000000000000"

#define XNVM_EFUSE_DEC_EFUSE_ONLY	"00000000"

#define XNVM_EFUSE_REVOCATION_ID_0_FUSES	"00000000"
#define XNVM_EFUSE_REVOCATION_ID_1_FUSES	"00000000"
#define XNVM_EFUSE_REVOCATION_ID_2_FUSES	"00000000"
#define XNVM_EFUSE_REVOCATION_ID_3_FUSES	"00000000"
#define XNVM_EFUSE_REVOCATION_ID_4_FUSES	"00000000"
#define XNVM_EFUSE_REVOCATION_ID_5_FUSES	"00000000"
#define XNVM_EFUSE_REVOCATION_ID_6_FUSES	"00000000"
#define XNVM_EFUSE_REVOCATION_ID_7_FUSES	"00000000"


#define XNVM_EFUSE_PRGM_USER_FUSE_NUM		1U
#define XNVM_EFUSE_NUM_OF_USER_FUSES		1U
#define XNVM_EFUSE_USER_FUSES			"00000000"

#define XNVM_EFUSE_READ_USER_FUSE_NUM		XNVM_EFUSE_PRGM_USER_FUSE_NUM
#define XNVM_EFUSE_READ_NUM_OF_USER_FUSES	XNVM_EFUSE_NUM_OF_USER_FUSES

/* Checks CRC of provided AES key if TRUE */
#define XNVM_EFUSE_CHECK_AES_KEY_CRC		FALSE
#define XNVM_EFUSE_CHECK_USER_KEY_0_CRC		FALSE
#define XNVM_EFUSE_CHECK_USER_KEY_1_CRC		FALSE

#define XNVM_EFUSE_CRC_AES_ZEROS                0x6858A3D5U

#define XNVM_EFUSE_EXPECTED_AES_KEY_CRC		XNVM_EFUSE_CRC_AES_ZEROS
#define XNVM_EFUSE_EXPECTED_USER_KEY0_CRC	XNVM_EFUSE_CRC_AES_ZEROS
#define XNVM_EFUSE_EXPECTED_USER_KEY1_CRC	XNVM_EFUSE_CRC_AES_ZEROS

#ifdef __cplusplus
}
#endif

#endif /* XILNVM_EFUSE_VERSAL_INPUT_H_ */
