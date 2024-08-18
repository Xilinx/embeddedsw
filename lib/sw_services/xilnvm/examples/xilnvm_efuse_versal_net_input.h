/******************************************************************************
* Copyright (c) 2024, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xilnvm_efuse_versal_net_input.h.
* This file contains macros which needs to configured by user based on the
* options selected by user operations will be performed.
*
* @note
*
*	User configurable parameters for Versal Net eFUSE
*------------------------------------------------------------------------------
*	#define XNVM_EFUSE_GLITCH_DET_WR_LOCK	FALSE
*	TRUE permanently disables writing to ANLG_TRIM_3 row
*	FALSE will not modify the bits in that row in eFUSE
*
*	#define XNVM_EFUSE_GD_HALT_BOOT_EN_1_0	FALSE
*	TRUE permanently enables halt boot in ROM when a glitch observed
*	FALSE will not modify the control bit in eFUSE

*	#define XNVM_EFUSE_GD_ROM_MONITOR_EN	FALSE
*	TRUE permanently enables the glitch monitoring in ROM
*	FALSE will not modify the control bit in eFUSE
*
*	#define XNVM_EFUSE_GEN_ERR_HALT_BOOT_EN_1_0	FALSE
*	TRUE halts the boot at ROM stage when any error other
*	than environmental and glitch observed
*	FALSE will not modify the control bit in eFUSE
*
*	#define XNVM_EFUSE_ENV_ERR_HALT_BOOT_EN_1_0	FALSE
*	TRUE halts the boot at ROM stage when any
*	environmental error observed
*	FALSE will not modify the control bit in eFUSE
*
* 	#define XNVM_EFUSE_REG_INIT_DIS			FALSE
* 	TRUE permanently disables register initialization
* 	FALSE will not modify this control bit of efuse.
*
* 	#define XNVM_EFUSE_BOOT_ENV_WR_LK		FALSE
* 	TRUE will permanently disable the efuse programming
* 	and hardware testbit programming on row 37 in EFUSE_0_MAP
* 	and hardware testbit programming on row 43 in EFUSE_0_MAP
* 	will be disabled as well.
*	FALSE will not modify this control bit of efuse.
*
*	#define XNVM_EFUSE_PMC_SC_EN		FALSE
*	TRUE will enable PMC Scan Clear during ROM operation or
*	during lockdown.
*	FALSE will not modify this control bit of efuse.
*
* 	#define XNVM_EFUSE_AUTH_JTAG_LOCK_DIS		FALSE
* 	TRUE will disable the Authenticated JTAG feature after
* 	secure lock down.
* 	FALSE will not modify this control bit of efuse.
*
* 	#define XNVM_EFUSE_AUTH_JTAG_DIS		FALSE
* 	TRUE will disable the Authenticated JTAG feature.
* 	FALSE will not modify this control bit of efuse.
*
*	#define XNVM_EFUSE_JTAG_DIS			FALSE
*	TRUE will disable all JTAG instructions.
*	FALSE will not modify this control bit of efuse.
*
*	#define XNVM_EFUSE_JTAG_ERROR_OUT_DIS		FALSE
*	TRUE will mask all errors reported in the JTAG ERROR_STATUS
*	instruction and masks the ROM_STATE[3:0] status reported in
*	the JTAG STATUS instruction.
*	FALSE will not modify this control bit of efuse.
*
* 	#define XNVM_EFUSE_AES_DIS			FALSE
* 	TRUE will disable the AES engine.
* 	FALSE will not modify this control bit of efuse.
*
* 	#define XNVM_EFUSE_UDS_WR_LK			FALSE
* 	TRUE will lock write for DICE UDS
* 	FALSE will not modify this control bit of efuse.
*
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

*	#define XNVM_EFUSE_HWTSTBITS_DIS		FALSE
*	TRUE will disable the hardware testbit mode
*	(soft programming of efuse cache) completely.
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
* 	#define XNVM_EFUSE_CRYPTO_KAT_EN		FALSE
* 	TRUE will run KAT on the CRYPTO engines (AES,SHA,RSA)
* 	prior to the first time being used.
* 	FALSE will not modify this control bit of efuse.
*
* 	#define XNVM_EFUSE_LBIST_EN			FALSE
* 	TRUE will make the firmware to run LBIST when briging up
* 	LPD.
* 	FALSE will not modify this control bit of efuse.
*
* 	#define XNVM_EFUSE_SAFETY_MISSION_EN		FALSE
* 	TRUE will make the ERROR_OUT pin a different meaning.
* 	When the eFUSE cache gets loaded - this pin asserted immediately.
* 	The pin goes low when a safety state has been written
* 	to the safety state register.
* 	This is effectively an external signal to indicate safe state.
*	FALSE will not modify this control bit of efuse.
*
* 	#define XNVM_EFUSE_SYSMON_TEMP_EN		FALSE
* 	TRUE will enable first level architectural temperature check.
* 	FALSE will not modify this control bit of efuse.
*
* 	#define XNVM_EFUSE_SYSMON_VOLT_EN		FALSE
* 	TRUE will enable first level architectural voltage check.
* 	FALSE will not modify this control bit of efuse.
*
* 	#define XNVM_EFUSE_SYSMON_TEMP_HOT		FALSE
* 	TRUE will burn the boot time temparature upper limit provided
* 	in XNVM_EFUSE_SYSMON_TEMP_HOT_FUSES.
*	FALSE will ignore the boot time temparature upper limit provided
*	in XNVM_EFUSE_SYSMON_TEMP_HOT_FUSES.
*
*	#define XNVM_EFUSE_SYSMON_VOLT_PMC		FALSE
*	TRUE will burn the boot time voltage setting for VCCINT_PMC
*	provided in XNVM_EFUSE_SYSMON_VOLT_PMC_FUSES.
*	FALSE will ignore the boot time voltage setting for VCCINT_PMC
*	provided in XNVM_EFUSE_SYSMON_VOLT_PMC_FUSES.
*
*	#define XNVM_EFUSE_SYSMON_VOLT_PSLP		FALSE
*	TRUE will burn the boot time voltage setting for VCCINT_PSLP
*	provided in XNVM_EFUSE_SYSMON_VOLT_PSLP_FUSE.
*	FALSE will ignore the boot time voltage setting for VCCINT_PSLP
*	provided in XNVM_EFUSE_SYSMON_VOLT_PSLP_FUSE.
*
*	#define XNVM_EFUSE_SYSMON_VOLT_SOC		FALSE
*	TRUE will enable boot time voltage monitoring setting to HP
*	voltage level.
*	FALSE will not modify this control bit of efuse.
*	0x0: MP voltage level
*	0x1: HP voltage level
*
*	#define XNVM_EFUSE_SYSMON_TEMP_COLD		FALSE
*	TRUE will burn the boot time temparature lower limit provided
*	in XNVM_EFUSE_SYSMON_TEMP_COLD_FUSES.
*	FALSE will ignore the boot time temparature lower limit provided
*	in XNVM_EFUSE_SYSMON_TEMP_COLD_FUSES.
*
* 	#define XNVM_EFUSE_LPD_MBIST_EN			FALSE
* 	TRUE will make the ROM code to run MBIST-clear on the LPD during
* 	USB boot mode and the FW will run MBIST-clear on the LPD during
* 	all other boot modes. Also checked by FW for secure lock down on the LPD
* 	FALSE will not modify this control bit of efuse.
*
* 	#define XNVM_EFUSE_PMC_MBIST_EN			FALSE
* 	TRUE will make the ROM code to run MBIST-clear on the PMC during
* 	all boot modes. Also checked by ROM for secure lockdown.
*	FALSE will not modify this control bit of efuse.
*
*	#define XNVM_EFUSE_LPD_NOC_SC_EN		FALSE
*	TRUE will make the ROM code to run SCAN-clear on the LPD during
*	USB boot or on the NOC on a SSIT slave SLR.
*	Also checked by FW for secure lockdown
*	FALSE will not modify this control bit of efuse.
*
*	#define XNVM_EFUSE_SYSMON_VOLT_MON_EN		FALSE
*	TRUE will make the ROM will perform the voltage check specified
*	in the BOOT_ENV_CTRL register.
*	FALSE will not modify this control bit of efuse.
*
*	#define XNVM_EFUSE_SYSMON_TEMP_MON_EN		FALSE
*	TRUE will make the ROM to perform the temperature check specified
*	in the BOOT_ENV_CTRL register
*	FALSE will not modify this control bit of efuse.
*
*	#define XNVM_EFUSE_AUTH_KEYS_TO_HASH		FALSE
*	TRUE will indicate that the keys are included into the attestation
*	mwasurement
*	FALSE will not modify this control bit of efuse.
*
*	#define XNVM_EFUSE_IRO_SWAP			FALSE
*	TRUE will indicate that the IRO swap is enabled
*	FALSE will not modify this control bit of efuse.
*
*	#define XNVM_EFUSE_ROM_SWDT_USAGE		FALSE
*	TRUE will indicate that SWDT is enabled at ROM stage
*	FALSE will not modify this control bit of efuse.
*
*	#define XNVM_EFUSE_DISABLE_PLM_UPDATE		FALSE
*	TRUE will disable In Place PLM update.
*	FALSE will not modify this control bit of efuse.
*
*	Following has to be set for programming required keys/data
*------------------------------------------------------------------------------
*	#define XNVM_EFUSE_WRITE_GLITCH_CFG		FALSE
*	TRUE will burn the glitch configuration data in XNVM_EFUSE_GLITCH_CFG
*	FALSE will ignore the data provided in XNVM_EFUSE_GLITCH_CFG
*
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
* 	TRUE will burn the lower 16 bits of Security Misc 0 eFuse row.
*	FALSE will not modify the lower 16 bits of Security Misc 0 eFuse row.
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
*	TRUE will burn the Plm IV provided in XNVM_EFUSE_PLM_IV.
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
*	#define XNVM_EFUSE_WRITE_BOOT_MODE_DISABLE	FALSE
*	TRUE will burn the boot mode disable fuses specified in
*	XNVM_EFUSE_BOOT_MODE_DISABLE.
*	FALSE will ignore the value provided in XNVM_EFUSE_BOOT_MODE_DISABLE.*
*
*	#define XNVM_EFUSE_PRGM_REVOCATION_ID		FALSE
*	TRUE will burn the revocation ID number which is specified
*	in XNVM_EFUSE_WRITE_REVOCATION_ID_NUM
*
*	#define XNVM_EFUSE_WRITE_REVOCATION_ID_NUM	0
*	The number of revocation ID which needs to be programmed.
*	The value should be between 0 and 255.
*
*	#define XNVM_EFUSE_PRGM_OFF_CHIP_REVOKE_ID		FALSE
*	TRUE will burn the off chip revocation ID number which is specified
*	in XNVM_EFUSE_WRITE_OFF_CHIP_REVOKE_ID_NUM
*
*	#define XNVM_EFUSE_WRITE_OFF_CHIP_REVOKE_ID_NUM		0
*	The number of off chip revocation ID which needs to be programmed.
*	The value should be between 0 and 255.
*
*	#define XNVM_EFUSE_PRGM_FIPS_INFO		FALSE
*	TRUE will program the FIPS mode and FIPS version efuses
*	FALSE will ignore the value provided in XNVM_EFUSE_FIPS_MODE and XNVM_EFUSE_FIPS_VERSION
*
*	#define XNVM_EFUSE_FIPS_MODE			(0U)
*	The value of the FIPS mode which should be programmed
*
*	#define XNVM_EFUSE_FIPS_VERSION			(0U)
*	The value of the FIPS version which should be programmed
*
* 	#define XNVM_EFUSE_SYSMON_TEMP_HOT_FUSES		0
*	The value mentioned in this will be used to set the boot time
*	temparature upper limit. Valid values are 0/1/2/3
*	0: 125C
*	1: 100C
*	2: 110C
*	3: 125C
*	By default the value is 0.
*
*	#define XNVM_EFUSE_SYSMON_VOLT_PMC_FUSES		0
*	The value mentioned in this will be used to set the boot time
*	voltage setting for VCCINT_PMC. Valid values are 0/1/2/3
*	limit.
*	0x0: Full range check
*	0x1: LP voltage level
*	0x2: MP voltage level
*	0x3: HP voltage level
*	By default the value is 0.
*
* 	#define XNVM_EFUSE_SYSMON_VOLT_PSLP_FUSE		0
*	The value mentioned in this will be used to set the boot time
*	voltage setting for VCCINT_PSLP. Valid values are 0/1/2/3
*	limit.
*	0x0: Full range check
*	0x1: LP voltage level
*	0x2: MP voltage level
*	0x3: HP voltage level
*	By default the value is 0.
*
*	#define XNVM_EFUSE_SYSMON_TEMP_COLD_FUSES		0
*	The value mentioned in this will be used to set the boot time
*	temparature lower limit. Valid values are 0/1/2/3
*	0: -55C
*	1: 0C
*	2: -40C
*	3: -55C
*	By default the value is 0.
*
*	#define XNVM_EFUSE_ENV_MONITOR_DISABLE			FALSE
*	TRUE will disable the temparature and voltage checks before eFuse
*	programming.
*	FALSE will not disable the temparature and voltage checks before eFuse
*	programming.
*	By default the value will be FALSE.
*
*	#define XNVM_EFUSE_GLITCH_CFG	"00000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the Versal eFuse array when write API used. This value should be
*	given in string format. It should be 8 characters long and make sure
*	that bit[31] is zero. Valid characters are 0-9,a-f,A-F. Any other
*	character is considered as invalid string and will not burn any eFuses.
*	Note that for writing the glitch configuration data,
*	XNVM_EFUSE_WRITE_GLITCH_CFG should be set to TRUE
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
*	While writing Plm IV, length should be 24 characters long.
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
*	#define XNVM_EFUSE_BOOT_MODE_DISABLE		"00000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the Versal Net eFuse array when write API used. This value should be
*	given in string format. It should be 8 characters long and make sure
*	that bit[31:16] are zero. Valid characters are 0-9,a-f,A-F. Any other
*	character is considered as invalid string and will not burn any eFuses.
*	Note that for writing the boot mode disable efuses,
*	XNVM_EFUSE_WRITE_BOOT_MODE_DISABLE should be set to TRUE
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date        Changes
* ----- ------  ----------  ------------------------------------------------------
* 1.0   har     07/01/2024  Initial release
*
* </pre>
*
******************************************************************************/

#ifndef XILNVM_EFUSE_VERSAL_NET_INPUT_H_
#define XILNVM_EFUSE_VERSAL_NET_INPUT_H_

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
#define XNVM_EFUSE_REG_INIT_DIS			FALSE
#define XNVM_EFUSE_BOOT_ENV_WR_LK		FALSE
#define XNVM_EFUSE_PMC_SC_EN			FALSE
#define XNVM_EFUSE_AUTH_JTAG_LOCK_DIS		FALSE
#define XNVM_EFUSE_AUTH_JTAG_DIS		FALSE
#define XNVM_EFUSE_PPK0_WR_LK			FALSE
#define XNVM_EFUSE_PPK1_WR_LK			FALSE
#define	XNVM_EFUSE_PPK2_WR_LK			FALSE
#define XNVM_EFUSE_AES_CRC_LK			FALSE
#define XNVM_EFUSE_AES_WR_LK			FALSE
#define XNVM_EFUSE_USER_KEY_0_CRC_LK		FALSE
#define XNVM_EFUSE_USER_KEY_0_WR_LK		FALSE
#define XNVM_EFUSE_USER_KEY_1_CRC_LK		FALSE
#define XNVM_EFUSE_USER_KEY_1_WR_LK		FALSE
#define XNVM_EFUSE_HWTSTBITS_DIS		FALSE
#define XNVM_EFUSE_JTAG_DIS			FALSE
#define XNVM_EFUSE_JTAG_ERROR_OUT_DIS		FALSE
#define XNVM_EFUSE_AES_DIS			FALSE
#define XNVM_EFUSE_UDS_WR_LK			FALSE

/**
 * Following is the define to select if the user wants to program
 * Misc control bits
 */
#define XNVM_EFUSE_GD_HALT_BOOT_EN_1_0		FALSE
#define XNVM_EFUSE_GD_ROM_MONITOR_EN		FALSE
#define XNVM_EFUSE_GEN_ERR_HALT_BOOT_EN_1_0	FALSE
#define XNVM_EFUSE_ENV_ERR_HALT_BOOT_EN_1_0	FALSE
#define XNVM_EFUSE_CRYPTO_KAT_EN		FALSE
#define XNVM_EFUSE_LBIST_EN			FALSE
#define XNVM_EFUSE_SAFETY_MISSION_EN		FALSE
#define XNVM_EFUSE_PPK0_INVLD			FALSE
#define XNVM_EFUSE_PPK1_INVLD			FALSE
#define XNVM_EFUSE_PPK2_INVLD			FALSE

/**
 * Following is the define to select if the user wants to program
 * BootEnvCtrl control bits
 */
#define XNVM_EFUSE_SYSMON_TEMP_EN		FALSE
#define XNVM_EFUSE_SYSMON_VOLT_EN		FALSE
#define XNVM_EFUSE_SYSMON_TEMP_HOT		FALSE
#define XNVM_EFUSE_SYSMON_VOLT_PMC		FALSE
#define XNVM_EFUSE_SYSMON_VOLT_PSLP		FALSE
#define XNVM_EFUSE_SYSMON_VOLT_SOC		FALSE
#define XNVM_EFUSE_SYSMON_TEMP_COLD		FALSE

/**
 * Following is the define to select if the user wants to program
 * SecurityMisc1 control bits
 */
#define XNVM_EFUSE_LPD_MBIST_EN			FALSE
#define XNVM_EFUSE_PMC_MBIST_EN			FALSE
#define XNVM_EFUSE_LPD_NOC_SC_EN		FALSE
#define XNVM_EFUSE_SYSMON_VOLT_MON_EN		FALSE
#define XNVM_EFUSE_SYSMON_TEMP_MON_EN		FALSE

/**
 * Following is the define to select if the user wants to program
 * ROM Rsvd bits
 */
#define XNVM_EFUSE_AUTH_KEYS_TO_HASH		FALSE
#define XNVM_EFUSE_IRO_SWAP			FALSE
#define XNVM_EFUSE_ROM_SWDT_USAGE		FALSE

/**
 * Following define is to lock the glitch detection row (ANLG_TRIM_3[31])
 */
#define XNVM_EFUSE_GLITCH_DET_WR_LK		FALSE

/**
 * Following define is to disable In Place PLM update
 */
#define XNVM_EFUSE_DISABLE_PLM_UPDATE		FALSE

/**
 * Following define is to program dec only efuses
 */
#define XNVM_EFUSE_WRITE_DEC_EFUSE_ONLY		FALSE

/**
 * Following is the define to select if the user wants to select Glitch configuration,
 * AES key, User Fuses, PPK0/PPK1/PPK2 hash, IVs, Revocation IDs and IVs
 */
/* For writing into eFuse */
#define XNVM_EFUSE_WRITE_GLITCH_CFG		FALSE
#define XNVM_EFUSE_WRITE_AES_KEY		FALSE
#define XNVM_EFUSE_WRITE_USER_KEY_0		FALSE
#define XNVM_EFUSE_WRITE_USER_KEY_1		FALSE

#define XNVM_EFUSE_WRITE_PPK0_HASH		FALSE
#define XNVM_EFUSE_WRITE_PPK1_HASH		FALSE
#define XNVM_EFUSE_WRITE_PPK2_HASH		FALSE

#define XNVM_EFUSE_WRITE_METAHEADER_IV		FALSE
#define XNVM_EFUSE_WRITE_BLACK_OBFUS_IV		FALSE
#define XNVM_EFUSE_WRITE_PLM_IV			FALSE
#define XNVM_EFUSE_WRITE_DATA_PARTITION_IV	FALSE

#define XNVM_EFUSE_WRITE_BOOT_MODE_DISABLE	FALSE

#define XNVM_EFUSE_PRGM_REVOCATION_ID		FALSE

#define XNVM_EFUSE_WRITE_REVOCATION_ID_NUM	(0U)

#define XNVM_EFUSE_PRGM_OFF_CHIP_REVOKE_ID	FALSE

#define XNVM_EFUSE_WRITE_OFF_CHIP_REVOKE_ID_NUM	(0U)

#define XNVM_EFUSE_PRGM_FIPS_INFO		FALSE
#define XNVM_EFUSE_FIPS_MODE			(0U)
#define XNVM_EFUSE_FIPS_VERSION			(0U)

#define XNVM_EFUSE_ENV_MONITOR_DISABLE		FALSE

/**
 * Following defines should be given in the form of hex string.
 * The length of AES_KEY string must be 64, PPK hash should be 64 for
 * and for USER_FUSES.
 */

#define XNVM_EFUSE_GLITCH_CFG	"00000000"

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

#define XNVM_EFUSE_BOOT_MODE_DISABLE	"00000000"

#define XNVM_EFUSE_SYSMON_TEMP_HOT_FUSES	0
#define XNVM_EFUSE_SYSMON_VOLT_PMC_FUSES	0
#define XNVM_EFUSE_SYSMON_VOLT_PSLP_FUSE	0
#define XNVM_EFUSE_SYSMON_TEMP_COLD_FUSES	0

#ifdef __cplusplus
}
#endif

#endif /* XILNVM_EFUSE_VERSAL_INPUT_H_ */
