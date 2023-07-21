/******************************************************************************
* Copyright (c) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
*	#define XSK_EFUSEPS_DISABLE_DFT_JTAG		FALSE
*	TRUE will disable DFT JTAG permanently.
*	FALSE will not modify the eFuse PS DFT JTAG disable bit
*
*	#define XSK_EFUSEPS_DISABLE_DFT_MODE		FALSE
*	TRUE will disable DFT mode permanently.
*	FALSE will not modify the eFuse PS DFT mode disable bit
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
* 	   			User configurable parameters for PL eFUSE for Zynq
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
*	TRUE will force eFUSE key to be used if booting Secure Image.
*	FALSE will not set the eFUSE control bit so that user can use secure boot
*	with BBRAM key.
*
*	Following are the MIO pins used for PL JTAG operations.
*	User can change these pins as their discretion.
*	#define		XSK_EFUSEPL_MIO_JTAG_TDI				(17)
*	#define		XSK_EFUSEPL_MIO_JTAG_TDO				(21)
*	#define		XSK_EFUSEPL_MIO_JTAG_TCK				(19)
*	#define		XSK_EFUSEPL_MIO_JTAG_TMS				(20)
*
*	MUX selection pin:
*	#define		XSK_EFUSEPL_MIO_JTAG_MUX_SELECT		(11)
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
*	AES key given in string format. It should be 64
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
*	User configurable parameters for PL eFUSE for Kintex Ultrascale
*	-----------------------------------------------------------------------
*	#define XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_0	FALSE
*	#define XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_1	FALSE
*	#define XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_2	FALSE
*	#define XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_3	FALSE
*	TRUE will select particular SLR(1/2/3/4) to program:
*	- AES Keys(256bits/SLR)
*	- User Fuse bits (32bits/SLR)
*	- User Keys (128bits/SLR)
*	- RSA Public Key HASH (384bits/SLR)
*	FALSE will disable programming.
*
*	#define	XSK_EFUSEPL_DISABLE_AES_KEY_READ	FALSE
*	TRUE will permanently disables the write to FUSE_AES and
*	check CRC for AES key by programming control bit of FUSE.
*	FALSE will not modify this control bit of eFuse.
*
*	#define	XSK_EFUSEPL_DISABLE_USER_KEY_READ	FALSE
*	TRUE will permanently disables the write to 32 bit FUSE_USER and
*	read of FUSE_USER key by programming control bit of FUSE.
*	FALSE will not modify this control bit of eFuse.
*
*	#define	XSK_EFUSEPL_DISABLE_SECURE_READ		FALSE
*	TRUE will permanently disables the write to FUSE_Secure block
*	and reading of secure block by programming control bit of FUSE.
*	FALSE will not modify this control bit of eFuse.
*
*	#define	XSK_EFUSEPL_DISABLE_FUSE_CNTRL_WRITE 	FALSE
*	TRUE will permanently disables the write to FUSE_CNTRL block
*	by programming control bit of FUSE.
*	FALSE will not modify this control bit of eFuse.
*
*	#define	XSK_EFUSEPL_DISABLE_RSA_KEY_READ	FALSE
*	TRUE will permanently disables the write to FUSE_RSA block and
*	reading of FUSE_RSA Hash by programming control bit of FUSE.
*	FALSE will not modify this control bit of eFuse.
*
*	#define	XSK_EFUSEPL_DISABLE_KEY_WRITE		FALSE
*	TRUE will permanently disables the write to FUSE_AES block by
*	programming control bit of FUSE.
*	FALSE will not modify this control bit of eFuse.
*
*	#define	XSK_EFUSEPL_DISABLE_USER_KEY_WRITE	FALSE
*	TRUE will permanently disables the write to FUSE_USER block
*	by programming control bit of FUSE.
*	FALSE will not modify this control bit of eFuse.
*
*	#define	XSK_EFUSEPL_DISABLE_SECURE_WRITE	FALSE
*	TRUE will permanently disables the write to FUSE_SECURE block
*	by programming control bit of FUSE.
*	FALSE will not modify this control bit of eFuse.
*
*	#define	XSK_EFUSEPL_DISABLE_RSA_HASH_WRITE	FALSE
*	TRUE will permanently disables the write to FUSE_RSA authentication
*	key by programming control bit of FUSE.
*	FALSE will not modify this control bit of eFuse.
*
*	#define XSK_EFUSEPL_DISABLE_128BIT_USER_KEY_WRITE	FALSE
*	TRUE will permanently disables the write to 128 bit FUSE_USER
*	by programming control bit of FUSE.
*	FALSE will not modify this control bit of eFuse
*
*	#define	XSK_EFUSEPL_ALLOW_ENCRYPTED_ONLY	FALSE
*	TRUE will permanently allows encrypted bitstream only.
*	FALSE will not modify this Secure bit of eFuse.
*
*	#define	XSK_EFUSEPL_FORCE_USE_FUSE_AES_ONLY	FALSE
*	TRUE then allows only FUSE's AES key as source of encryption
*	FALSE then allows FPGA to configure an unencrypted bitstream or
*	bitstream encrypted using key stored BBRAM or eFuse.
*
*	#define	XSK_EFUSEPL_ENABLE_RSA_AUTH		FALSE
*	TRUE will enable RSA authentication of bitstream
*	FALSE will not modify this secure bit of eFuse.
*
*	#define	XSK_EFUSEPL_DISABLE_JTAG_CHAIN		FALSE
*	TRUE will disable JTAG permanently.
*	FALSE will not modify this secure bit of eFuse.
*
*	#define	XSK_EFUSEPL_DISABLE_TEST_ACCESS		FALSE
*	TRUE will disables Xilinx test access.
*	FALSE will not modify this secure bit of eFuse.
*
*	#define	XSK_EFUSEPL_DISABLE_AES_DECRYPTOR	FALSE
*	TRUE will disables decoder completely.
*	FALSE will not modify this secure bit of eFuse.
*
*	#define XSK_EFUSEPL_ENABLE_OBFUSCATION_EFUSEAES	FALSE
*	TRUE will enable obfuscation feature for eFUSE AES key,
*	this instructs the device to decode the eFUSE AES key
*	to actual AES key before decrypting the bitstream.
*	FALSE will not modify this secure bit of eFUSE.
*
*       #define XSK_EFUSEPL_AXI_GPIO_DEVICE_ID  XPAR_AXI_GPIO_0_DEVICE_ID
*       Default value is XPAR_AXI_GPIO_0_DEVICE_ID
*       This macro is for providing exact GPIO device ID, based on the
*       design configuration this macro should be modified to provide
*       GPIO device ID which is used for connecting MASTER JTAG pins.
*
*	In Ultrascale GPIO pins used for connecting MASTER_JTAG pins and
*	hardware module to access eFUSE.
*	Following are the GPIO pins and user can change these pins
*	#define XSK_EFUSEPL_AXI_GPIO_JTAG_TDO	(0)
*	#define XSK_EFUSEPL_AXI_GPIO_HWM_READY	(1)
*	#define XSK_EFUSEPL_AXI_GPIO_HWM_END	(2)
*
*	#define XSK_EFUSEPL_AXI_GPIO_JTAG_TDI	(0)
*	#define XSK_EFUSEPL_AXI_GPIO_JTAG_TMS	(1)
*	#define XSK_EFUSEPL_AXI_GPIO_JTAG_TCK	(2)
*	#define XSK_EFUSEPL_AXI_GPIO_HWM_START	(3)
*
*	#define XSK_EFUSEPL_GPIO_INPUT_CH			(2)
*	This macro is for providing channel number of ALL INPUTS connected
*	(Master JTAG's - TDO, Hardware module's - READY and END)
*	#define XSK_EFUSEPL_GPIO_OUTPUT_CH			(1)
*	This macro is for providing channel number of ALL OUTPUTS connected
*	(Master JTAG's - TDI, TCK, TMS,and Hardware module's - START)
*
*	NOTE: All inputs and outputs of GPIO can be configured in single
*	channel also
*	i.e XSK_EFUSEPL_GPIO_INPUT_CH = XSK_EFUSEPL_GPIO_OUTPUT_CH = 1 or 2.
*	Among (TDI, TCK, TMS, START) Outputs of GPIO cannot be connected to
*	different GPIO channels all the 4 signals should be in same channel.
*	(TDO, READY and END) can be a other channel of (TDI, TCK, TMS, START)
*	or the same.
*
*	#define XSK_EFUSEPL_PROGRAM_AES_KEY		FALSE
*	TRUE will burn the AES key given in XSK_EFUSEPL_AES_KEY.
*	FALSE will ignore the values given.
*
*	#define XSK_EFUSEPL_PROGRAM_USER_KEY	FALSE
*	TRUE will burn 32 bit User key given in XSK_EFUSEPL_USER_KEY
*	FALSE will ignore the values given.
*
*	#define XSK_EFUSEPL_PROGRAM_RSA_HASH	FALSE
*	TRUE will burn RSA hash given in XSK_EFUSEPL_RSA_KEY_HASH_VALUE
*	FALSE will ignore the values given.
*
*	#define XSK_EFUSEPL_PROGRAM_USER_KEY_128BIT	FALSE
*	TRUE will burn 128 bit User key given in XSK_EFUSEPL_USER_KEY_128BIT_CONFIG_ORDER_0,
*	XSK_EFUSEPL_USER_KEY_128BIT_CONFIG_ORDER_1, XSK_EFUSEPL_USER_KEY_128BIT_CONFIG_ORDER_2,
*	XSK_EFUSEPL_USER_KEY_128BIT_CONFIG_ORDER_3
*	FALSE will ignore the values given.
*
*	#define XSK_EFUSEPL_CHECK_AES_KEY		FALSE
*	TRUE will perform CRC check of FUSE_AES with provided CRC value in macro
*	XSK_EFUSEPL_CRC_OF_EXPECTED_AES_KEY.
*	And result of CRC check will be updated in XilSKey_EPl instance
*	parameter AESKeyMatched with either TRUE or FALSE.
*	FALSE CRC check of FUSE_AES will not be performed.
*
*	#define XSK_EFUSEPL_READ_USER_KEY		FALSE
*	TRUE will read 32 bit FUSE_USER from Ultrascale's eFuse and updates in
*	XilSKey_EPl instance parameter UserKeyReadback
*	FALSE 32 bit FUSE_USER key read will not be performed.
*
*	#define XSK_EFUSEPL_READ_RSA_HASH		FALSE
*	TRUE will read FUSE_USER from Ultrascale's eFuse and updates in
*	XilSKey_EPl instance parameter RSAHashReadback
*	FALSE FUSE_RSA_HASH read will not be performed.
*
*	#define XSK_EFUSEPL_READ_USER_KEY128_BIT	FALSE
*	TRUE will read 128 bit USER key from Ultrascale's eFuse and updates in
*	XilSKey_EPl instance parameter User128BitReadBack
*	FALSE 128 bit USER key read will not be performed.
*
*	#define XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_0
*	"0000000000000000000000000000000000000000000000000000000000000000"
*	#define	XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_1
*	"0000000000000000000000000000000000000000000000000000000000000000"
*	#define	XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_2
*	"0000000000000000000000000000000000000000000000000000000000000000"
*	#define	XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_3
*	"0000000000000000000000000000000000000000000000000000000000000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the PL eFUSE array when write API used. This value should be the
*	AES key given in string format. It should be 64
*	characters long, valid characters are 0-9,a-f,A-F. Any other character
*	is considered as invalid string and will not burn AES Key. Note that,
*	for writing the AES Key, XSK_EFUSEPL_PROGRAM_AES_KEY for
*	particular SLR(1/2/3/4)should have TRUE value.
*
*	#define XSK_EFUSEPL_USER_KEY_CONFIG_ORDER_0	"00000000"
*	#define	XSK_EFUSEPL_USER_KEY_CONFIG_ORDER_1	"00000000"
*	#define	XSK_EFUSEPL_USER_KEY_CONFIG_ORDER_2	"00000000"
*	#define	XSK_EFUSEPL_USER_KEY_CONFIG_ORDER_3		"00000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the PL eFUSE array when write API used. This value should be the
*	User Key given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn User Key. Note that, for writing the User Key,
*	XSK_EFUSEPL_PROGRAM_USER_KEY_ULTRA for particular SLR(1/2/3/4)should
*	have TRUE value.
*
*	#define XSK_EFUSEPL_RSA_KEY_HASH_VALUE_CONFIG_ORDER_0
*		"0000000000000000000000000000000000000000000000 \
*		00000000000000000000000000000000000000000000000000"
*	#define	XSK_EFUSEPL_RSA_KEY_HASH_VALUE_CONFIG_ORDER_1
*		"0000000000000000000000000000000000000000000000  \
*		00000000000000000000000000000000000000000000000000"
*	#define	XSK_EFUSEPL_RSA_KEY_HASH_VALUE_CONFIG_ORDER_2
*		"0000000000000000000000000000000000000000000000  \
*		00000000000000000000000000000000000000000000000000"
*	#define	XSK_EFUSEPL_RSA_KEY_HASH_VALUE_CONFIG_ORDER_3
*		"0000000000000000000000000000000000000000000000  \
*		00000000000000000000000000000000000000000000000000"
*	The value mentioned in this will be converted to hex buffer and written
*	into the PL eFUSE array when write API used. This value should be the
*	RSA Key hash given in string format. It should be 96 characters long,
*	valid characters are 0-9,a-f,A-F. Any other character is considered as
*	invalid string and will not burn RSA hash value. Note that, for writing
*	the RSA hash, XSK_EFUSEPL_PROGRAM_RSA_HASH_ULTRA for particular SLR(1/2/3/4)
*	should have TRUE value.
*
*	#define XSK_EFUSEPL_USER_KEY_128BIT_0_CONFIG_ORDER_0	"00000000"
*	#define XSK_EFUSEPL_USER_KEY_128BIT_1_CONFIG_ORDER_0	"00000000"
*	#define XSK_EFUSEPL_USER_KEY_128BIT_2_CONFIG_ORDER_0	"00000000"
*	#define XSK_EFUSEPL_USER_KEY_128BIT_3_CONFIG_ORDER_0	"00000000"
*
*	#define XSK_EFUSEPL_USER_KEY_128BIT_0_CONFIG_ORDER_1	"00000000"
*	#define XSK_EFUSEPL_USER_KEY_128BIT_1_CONFIG_ORDER_1	"00000000"
*	#define XSK_EFUSEPL_USER_KEY_128BIT_2_CONFIG_ORDER_1	"00000000"
*	#define XSK_EFUSEPL_USER_KEY_128BIT_3_CONFIG_ORDER_1	"00000000"
*
*	#define XSK_EFUSEPL_USER_KEY_128BIT_0_CONFIG_ORDER_2	"00000000"
*	#define XSK_EFUSEPL_USER_KEY_128BIT_1_CONFIG_ORDER_2	"00000000"
*	#define XSK_EFUSEPL_USER_KEY_128BIT_2_CONFIG_ORDER_2	"00000000"
*	#define XSK_EFUSEPL_USER_KEY_128BIT_3_CONFIG_ORDER_2	"00000000"
*
*	#define XSK_EFUSEPL_USER_KEY_128BIT_0_CONFIG_ORDER_3	"00000000"
*	#define XSK_EFUSEPL_USER_KEY_128BIT_1_CONFIG_ORDER_3	"00000000"
*	#define XSK_EFUSEPL_USER_KEY_128BIT_2_CONFIG_ORDER_3	"00000000"
*	#define XSK_EFUSEPL_USER_KEY_128BIT_3_CONFIG_ORDER_3	"00000000"
*	The above four macros are meant for providing 128 bit User key for SLR(1/2/3/4),
*	XSK_EFUSEPL_USER_KEY_128BIT_CONFIG_ORDER_0 holds 31:0 bits,
*	XSK_EFUSEPL_USER_KEY_128BIT_CONFIG_ORDER_1 holds 63:32 bits,
*	XSK_EFUSEPL_USER_KEY_128BIT_CONFIG_ORDER_2 holds 95:64 bits and
*	XSK_EFUSEPL_USER_KEY_128BIT_CONFIG_ORDER_3 holds 127:96 bits of whole 128 bit User
*	key.
*	The value mentioned in this will be converted to hex buffer and written
*	into the PL eFUSE array when write API used. This value should be the
*	User Key given in string format. It should be 8 characters long, valid
*	characters are 0-9,a-f,A-F. Any other character is considered as invalid
*	string and will not burn User Key. Note that, for writing the User Key,
*	XSK_EFUSEPL_PROGRAM_USER_KEY_128BIT should have TRUE value.
*
*	#define	XSK_EFUSEPL_CRC_OF_EXPECTED_AES_KEY_CONFIG_ORDER_0
*	XSK_EFUSEPL_AES_CRC_OF_ALL_ZEROS
*   #define	XSK_EFUSEPL_CRC_OF_EXPECTED_AES_KEY_CONFIG_ORDER_1
*   XSK_EFUSEPL_AES_CRC_OF_ALL_ZEROS
*   #define	XSK_EFUSEPL_CRC_OF_EXPECTED_AES_KEY_CONFIG_ORDER_2
*   XSK_EFUSEPL_AES_CRC_OF_ALL_ZEROS
*   #define	XSK_EFUSEPL_CRC_OF_EXPECTED_AES_KEY_CONFIG_ORDER_3
*   XSK_EFUSEPL_AES_CRC_OF_ALL_ZEROS
*
*   XSK_EFUSEPL_AES_CRC_OF_ALL_ZEROS is the default value for all SLRs
*   and is hexadecimal CRC value of FUSE_AES with all Zeros.
*	Please provide CRC of the AES key programmed.
*	For Checking CRC of FUSE_AES XSK_EFUSEPL_CHECK_AES_KEY_ULTRA macro
*	should be TRUE otherwise CRC check will not be performed.
*	For calculation of AES key's CRC one can use
*	u32 XilSKey_CrcCalculation(u8 *Key) API
*
*	NOTE:Please make sure you have sufficient heap and stack to run this
*	application.
*	For more information on creating vivado design please refer to xapp1283.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.00a rpoolla 04/26/13 First release
* 1.01a hk      09/18/13 Added BBRAM related definitions
* 3.00  vns     31/07/15 Added efuse functionality for Ultrascale.
*                        In Ultrascale GPIO pins and channels macros are added
*                        to access Master Jtag primitive and also added
*                        extra control bits and secure bits for Ultrascale's
*                        eFuse.
* 4.00  vns     09/10/15 Added DFT JTAG disable and DFT MODE disable programming
*                        options for Zynq eFuse PS.
* 6.0   vns     07/07/16 Added Gpio pin numbers connected to hardware module.
*               27/08/16 Modified XSK_EFUSEPL_DISABLE_DECODER macro to
*                        XSK_EFUSEPL_DISABLE_AES_DECRYPTOR
* 6.1   vns     10/25/16 Removed XSK_BBRAM_FORCE_PCYCLE_RECONFIG and
*                        XSK_BBRAM_DISABLE_JTAG_CHAIN, from Zynq BBRAM
*                        configurations as they are not actually
*                        programming any control bit. These 2 are part of the
*                        eFUSE PL and they already exist at eFUSE PL
*                        configurations (XSK_EFUSEPL_FORCE_PCYCLE_RECONFIG and
*                        XSK_EFUSEPL_DISABLE_JTAG_CHAIN)
* 6.4   vns     02/27/18 Added support for programming secure bit -
*                        enable obfuscation feature for eFUSE AES key
* 6.7   psl     03/20/19 Added eFuse key write support for SSIT devices.
*       psl     03/29/19 Added support for user configurable GPIO for
*                        jtag control.
* 6.8   psl     05/28/19 Added Macro for expected CRC of AES key for different
*                        SLR.
* 6.8   psl     06/07/19 Added doxygen tags.
* 7.5   ng      07/13/23 added SDT support
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
#include "xparameters.h"
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
 *  Definition of Zynq PL
 *	----------------------------------------------------------
 */
#ifdef XSK_ARM_PLATFORM
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
/**
 * Definitions for Kintex Ultrascale's eFuse
 * ---------------------------------------------
 */
#else

/*	Select the SLR(s) to write
 *	All the control information cannot be independently selected
 *	with following exceptions
 *	- AES Keys(256bits/SLR)
 *	- User Fuse bits (32bits/SLR)
 *	- User Keys (128bits/SLR)
 *	- RSA Public Key HASH (384bits/SLR)
 **/
#define XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_0	FALSE
#define XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_1	FALSE
#define XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_2	FALSE
#define XSK_EFUSEPL_PGM_SLR_CONFIG_ORDER_3	FALSE

/* Definition of CRC of all zeros AES key */
#ifdef XSK_MICROBLAZE_ULTRA_PLUS
#define XSK_EFUSEPL_AES_CRC_OF_ALL_ZEROS	XSK_EFUSEPL_CRC_FOR_AES_ZEROS_ULTRA_PLUS
#else
#define XSK_EFUSEPL_AES_CRC_OF_ALL_ZEROS	XSK_EFUSEPL_CRC_FOR_AES_ZEROS
#endif

/**
 * Following is the define to select if the user wants to program control bits
 */
#define	XSK_EFUSEPL_DISABLE_AES_KEY_READ	FALSE	/**< If TRUE will disable
							  *  AES key crc check
							  *  andprogramming
							  */
#define	XSK_EFUSEPL_DISABLE_USER_KEY_READ	FALSE	/**< If TRUE will disable
							  *  32 bit User key reading and
							  *  programming
							  */
#define	XSK_EFUSEPL_DISABLE_SECURE_READ		FALSE	/**< If TRUE will disable
							  *  secure block reading
							  *  and programming
							  */
#define	XSK_EFUSEPL_DISABLE_FUSE_CNTRL_WRITE 	FALSE	/**< If TRUE will disable
							  *  programming control
							  *  bits
							  */
#define	XSK_EFUSEPL_DISABLE_RSA_KEY_READ	FALSE	/**< If TRUE will disable
							  *  RSA key hash reading
							  *  and programming
							  */
#define	XSK_EFUSEPL_DISABLE_KEY_WRITE		FALSE	/**< If TRUE will disable
							  *  AES key programming
							  */
#define	XSK_EFUSEPL_DISABLE_USER_KEY_WRITE	FALSE	/**< If TRUE will disable
							  *  Programming 32 bit User
							  *  key
							  */
#define	XSK_EFUSEPL_DISABLE_SECURE_WRITE	FALSE	/**< If TRUE will disable
							  *  programming Secure block
							  */
#define	XSK_EFUSEPL_DISABLE_RSA_HASH_WRITE	FALSE	/**< If TRUE will disable
							  *  programming to RSA key
							  *  hash
							  */
#define XSK_EFUSEPL_DISABLE_128BIT_USER_KEY_WRITE	FALSE
						/**< If TRUE will disable
						  *  Programming 128 bit User
						  *  key
						  */
/**
 * Following is the define to select if the user wants to program Secure bits
 */

#define	XSK_EFUSEPL_ALLOW_ENCRYPTED_ONLY	FALSE	/**< If TRUE will force
							  *  to use only encrypted
							  *  bitstreams
							  */

#define	XSK_EFUSEPL_FORCE_USE_FUSE_AES_ONLY	FALSE	/**< If TRUE will force
							  * to use Secure boot
							  * with eFUSE key only.
							  */

#define	XSK_EFUSEPL_ENABLE_RSA_AUTH		FALSE	/**< If TRUE will enable
							  *  RSA authentication of
							  * bitstream */
#define	XSK_EFUSEPL_DISABLE_JTAG_CHAIN		FALSE	/**< If TRUE then
							  *  permanently sets the Ultrascale
							  *  device in bypass mode
							  */

#define	XSK_EFUSEPL_DISABLE_TEST_ACCESS		FALSE	/**< If TRUE will disable internal
							  * test access for ULTRASCALE
							  */
#define	XSK_EFUSEPL_DISABLE_AES_DECRYPTOR	FALSE	/**< If TRUE will
							  *  Disable AES decryptor
							  */
#define XSK_EFUSEPL_ENABLE_OBFUSCATION_EFUSEAES	FALSE	/**< If TRUE will
							 * enable obfuscation
							 * feature for eFUSE's
							 * AES key
							 */

/**
 * Following defines should be given in decimal/hexa-decimal values.
 * These are to be defined for Ultrascale Microblaze
 * AXI GPIO pin numbers connected to MASTER JTAG primitive and corresponding
 * channel numbers for GPIO pins
 */

/* GPIO device ID */
#ifndef SDT
#define XSK_BBRAM_AXI_GPIO_DEVICE_ID	XPAR_AXI_GPIO_0_DEVICE_ID
#else
#define XSK_BBRAM_AXI_GPIO_DEVICE_ID	XPAR_XGPIOPS_0_BASEADDR
#endif

/* Signals connect as Input to GPIO */
#define	XSK_EFUSEPL_AXI_GPIO_JTAG_TDO	(0)	/**< MASTER JTAG GPIO
						  *  pin for TDO */
#define XSK_EFUSEPL_AXI_GPIO_HWM_READY	(1)	/**< Tells whether
						* hardware module is ready or not */
#define XSK_EFUSEPL_AXI_GPIO_HWM_END	(2)	/**< Notifies
						* hardware module programming
						* completion */

/* Signals connect as Output from GPIO */
#define	XSK_EFUSEPL_AXI_GPIO_JTAG_TDI	(0)	/**< MASTER JTAG GPIO
						  *  pin for TDI */
#define	XSK_EFUSEPL_AXI_GPIO_JTAG_TMS	(1)	/**< MASTER JTAG GPIO
						  *  pin for TMS */
#define	XSK_EFUSEPL_AXI_GPIO_JTAG_TCK	(2)	/**< MASTER JTAG GPIO
						  *  pin for TCK */
#define XSK_EFUSEPL_AXI_GPIO_HWM_START	(3)	/**< Triggers
						* Hardware module, for programming
						* start */

#define	XSK_EFUSEPL_GPIO_INPUT_CH	(2)	/**< GPIO Channel for which TDO,
						  *  Hardware module ready and end pins connected */
#define	XSK_EFUSEPL_GPIO_OUTPUT_CH	(1)	/**< GPIO Channel for which
						 * Hardware module Starte, TDI,
*						  * TMS and TCK pin connected */

/**
 * Following is the define to select if the user wants to select AES, User, RSA and RES keys
 * for Ultrascale
 */
/* For Programming keys */
#define	XSK_EFUSEPL_PROGRAM_AES_KEY		FALSE 	/**< TRUE burns
							  * the AES key
							  */
#define	XSK_EFUSEPL_PROGRAM_USER_KEY		FALSE	/**< TRUE burns
							  * the USER key
							  */
#define	XSK_EFUSEPL_PROGRAM_RSA_KEY_HASH	FALSE	/**< TRUE burns
							  * the RSA hash
							  */
#define XSK_EFUSEPL_PROGRAM_USER_KEY_128BIT	FALSE	/**< TRUE burns
							  * 128 bit USER key
							  */
/* For reading keys */
#define	XSK_EFUSEPL_CHECK_AES_KEY_CRC		FALSE	/**< TRUE checks
							  * AES key with
							  * below provided CRC
							  */
#define	XSK_EFUSEPL_READ_USER_KEY		FALSE	/**< TRUE read 32 bit
							  *  USER key
							  */
#define	XSK_EFUSEPL_READ_RSA_KEY_HASH		FALSE	/**< TRUE read
							  *  RSA Hash value
							  */
#define XSK_EFUSEPL_READ_USER_KEY128_BIT	FALSE	/**< TRUE reads
							  * 128 bit USER key
							  */

/**
 * Following defines should be given in the form of hex string.
 * The length of AES_KEY string must me 64 and for 32 bit USER_KEY must be 8.
 */
#define	XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_0		"0000000000000000000000000000000000000000000000000000000000000000"
#define	XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_1		"0000000000000000000000000000000000000000000000000000000000000000"
#define	XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_2		"0000000000000000000000000000000000000000000000000000000000000000"
#define	XSK_EFUSEPL_AES_KEY_CONFIG_ORDER_3		"0000000000000000000000000000000000000000000000000000000000000000"

#define	XSK_EFUSEPL_USER_KEY_CONFIG_ORDER_0		"00000000"
#define	XSK_EFUSEPL_USER_KEY_CONFIG_ORDER_1		"00000000"
#define	XSK_EFUSEPL_USER_KEY_CONFIG_ORDER_2		"00000000"
#define	XSK_EFUSEPL_USER_KEY_CONFIG_ORDER_3		"00000000"
/**
 * Following defines should be given only for Ultrascale the length of
 * RSA string must be 96
 */
#define	XSK_EFUSEPL_RSA_KEY_HASH_VALUE_CONFIG_ORDER_0	"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
#define	XSK_EFUSEPL_RSA_KEY_HASH_VALUE_CONFIG_ORDER_1	"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
#define	XSK_EFUSEPL_RSA_KEY_HASH_VALUE_CONFIG_ORDER_2	"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
#define	XSK_EFUSEPL_RSA_KEY_HASH_VALUE_CONFIG_ORDER_3	"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
/**
 * Following define should be given only for Ultrascale the total length of
 * User 128-bit register is 128bit, to make it easier 128 bit register is broken
 * into four 32 bit, single bit programming is also available.
 *
 */
#define XSK_EFUSEPL_USER_KEY_128BIT_0_CONFIG_ORDER_0	"00000000"	/* 31:0 */
#define XSK_EFUSEPL_USER_KEY_128BIT_1_CONFIG_ORDER_0	"00000000"	/* 63:32 */
#define XSK_EFUSEPL_USER_KEY_128BIT_2_CONFIG_ORDER_0	"00000000"	/* 95:64 */
#define XSK_EFUSEPL_USER_KEY_128BIT_3_CONFIG_ORDER_0	"00000000"	/* 127:96 */

#define XSK_EFUSEPL_USER_KEY_128BIT_0_CONFIG_ORDER_1	"00000000"	/* 31:0 */
#define XSK_EFUSEPL_USER_KEY_128BIT_1_CONFIG_ORDER_1	"00000000"	/* 63:32 */
#define XSK_EFUSEPL_USER_KEY_128BIT_2_CONFIG_ORDER_1	"00000000"	/* 95:64 */
#define XSK_EFUSEPL_USER_KEY_128BIT_3_CONFIG_ORDER_1	"00000000"	/* 127:96 */

#define XSK_EFUSEPL_USER_KEY_128BIT_0_CONFIG_ORDER_2	"00000000"	/* 31:0 */
#define XSK_EFUSEPL_USER_KEY_128BIT_1_CONFIG_ORDER_2	"00000000"	/* 63:32 */
#define XSK_EFUSEPL_USER_KEY_128BIT_2_CONFIG_ORDER_2	"00000000"	/* 95:64 */
#define XSK_EFUSEPL_USER_KEY_128BIT_3_CONFIG_ORDER_2	"00000000"	/* 127:96 */

#define XSK_EFUSEPL_USER_KEY_128BIT_0_CONFIG_ORDER_3	"00000000"	/* 31:0 */
#define XSK_EFUSEPL_USER_KEY_128BIT_1_CONFIG_ORDER_3	"00000000"	/* 63:32 */
#define XSK_EFUSEPL_USER_KEY_128BIT_2_CONFIG_ORDER_3	"00000000"	/* 95:64 */
#define XSK_EFUSEPL_USER_KEY_128BIT_3_CONFIG_ORDER_3	"00000000"	/* 127:96 */

/**
 * Following define is CRC value of expected AES key
 */
#define	XSK_EFUSEPL_CRC_OF_EXPECTED_AES_KEY_CONFIG_ORDER_0  XSK_EFUSEPL_AES_CRC_OF_ALL_ZEROS
#define	XSK_EFUSEPL_CRC_OF_EXPECTED_AES_KEY_CONFIG_ORDER_1	XSK_EFUSEPL_AES_CRC_OF_ALL_ZEROS
#define	XSK_EFUSEPL_CRC_OF_EXPECTED_AES_KEY_CONFIG_ORDER_2	XSK_EFUSEPL_AES_CRC_OF_ALL_ZEROS
#define	XSK_EFUSEPL_CRC_OF_EXPECTED_AES_KEY_CONFIG_ORDER_3	XSK_EFUSEPL_AES_CRC_OF_ALL_ZEROS

#endif

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
#define XSK_EFUSEPS_DISABLE_DFT_JTAG		FALSE /**< DFT jtag
											* Disable
											*/
#define XSK_EFUSEPS_DISABLE_DFT_MODE		FALSE /**< DFT mode
											* Disable
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
