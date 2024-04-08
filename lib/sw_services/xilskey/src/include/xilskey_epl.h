/******************************************************************************
* Copyright (c) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file
*
* 		xilskey_epl.h
* @addtogroup xilskey_zynq_ultra_efuse EFUSE PL
* @{
* @cond xilskey_internal
* @{
* @note
*		 Contains the function prototypes, defines and macros for the PL eFUSE
*		 functionality.
*
* MODIFICATION HISTORY:
*
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.00a rpoolla 04/26/13 First release
* 1.02a hk      10/28/13 Added API's to read status bits and key :
* 			 u32 XilSKey_EfusePl_ReadKey(XilSKey_EPl *InstancePtr)
*			 u32 XilSKey_EfusePl_ReadKey(XilSKey_EPl *InstancePtr)
* 2.00  hk      22/01/14 Corrected PL voltage checks to VCCINT and VCCAUX.
*                        CR#768077
* 3.00  vns     31/07/15 Added efuse functionality for Ultrascale.
* 6.0   vns     07/07/16 Added Hardware module pins in eFUSE PL instance.
* 6.4   vns     02/27/18 Added support for programming secure bit 6 -
*                        enable obfuscation feature for eFUSE AES key
*       vns     03/09/18 Added correct status bit positions to Ultrascale plus
* 6.6   vns     06/06/18 Added doxygen tags
* 6.7   arc     01/05/19 Fixed MISRA-C violations.
*       psl     03/20/19 Added eFuse key write support for SSIT devices.
*       psl     03/29/19 Added Support for user configurable GPIO for jtag
*                        control
* 7.2   am      07/13/21 Fixed doxygen warnings
* 7.6   vns     04/04/24 Updated efuse PL secure bits of ultrascale devices
*
* </pre>
*
****************************************************************************/
#ifndef XILSKEY_EPL_H
#define XILSKEY_EPL_H

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
/**
 *  AES Key size in Bytes
 */
#define XSK_EFUSEPL_AES_KEY_SIZE_IN_BYTES				(32U)
/**
 *  User Key size in Bytes
 */
#define XSK_EFUSEPL_USER_KEY_SIZE_IN_BYTES				(4U)
/**
 *  CRC of AES key with all zeros
 */
#define XSK_EFUSEPL_CRC_FOR_AES_ZEROS					(0x621C42AAU)

/* CRC of AES key with all zeros for Ultrascale plus */
#define XSK_EFUSEPL_CRC_FOR_AES_ZEROS_ULTRA_PLUS			(0x3117503AU)
/**
 * AES key String length
 */
#define XSK_EFUSEPL_AES_KEY_STRING_LEN					(64U)

/*
 * Status register index values of Ultrascale's Fuse
 */
#ifdef XSK_MICROBLAZE_ULTRA
typedef enum {
	XSK_EFUSEPL_STATUS_DISABLE_KEY_READ_ULTRA,	/**< Bit 0 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_USER_KEY_READ_ULTRA,	/**< Bit 1 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_SECURE_READ_ULTRA,	/**< Bit 2 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_CNTRL_WRITE_ULTRA = 5,/**< Bit 5 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_RSA_KEY_READ_ULTRA,	/**< Bit 6 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_KEY_WRITE_ULTRA,	/**< Bit 7 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_USER_KEY_WRITE_ULTRA,/**< Bit 8 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_SECURE_WRITE_ULTRA,	/**< Bit 9 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_RSA_KEY_WRITE_ULTRA = 15,/**< Bit 15 of Status reg */
	XSK_EFUSEPL_STATUS_DIABLE_128BIT_USER_KEY_WRITE_ULTRA,
	XSK_EFUSEPL_STATUS_FUSE_LOGIC_IS_BUSY_ULTRA = 23,/**< Bit 23 of Status reg */
	XSK_EFUSEPL_STATUS_AES_ONLY_ENABLED_ULTRA,/**< Bit 24 of Status reg */
	XSK_EFUSEPL_STATUS_FUSE_SHAD_SEC1_ULTRA,/**< Bit 25 of Status reg */
	XSK_EFUSEPL_STATUS_RSA_AUTH_ENABLED_ULTRA,/**< Bit 26 of Status reg */
	XSK_EFUSEPL_STATUS_FUSE_SHAD_SEC3_ULTRA,/**< Bit 27 of Status reg */
	XSK_EFUSEPL_STATUS_SCAN_DISABLE_ULTRA,/**< Bit 28 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_CRYPTO_ULTRA,/**< Bit 29 of Status reg */
	XSK_EFUSEPL_STATUS_ENABLE_OBFUSCATED_EFUSE_KEY /**< Bit 30 of Status reg */
}XSKEfusePl_FuseStatusBits_F8Series;
#else
/*
 * Status register index values of Ultrascale plus's Fuse
 */
typedef enum {
	XSK_EFUSEPL_STATUS_DISABLE_KEY_READ_ULTRA,	/**< Bit 0 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_USER_KEY_READ_ULTRA,	/**< Bit 1 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_SECURE_READ_ULTRA,	/**< Bit 2 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_CNTRL_WRITE_ULTRA = 5,/**< Bit 5 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_RSA_KEY_READ_ULTRA,	/**< Bit 6 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_KEY_WRITE_ULTRA,	/**< Bit 7 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_USER_KEY_WRITE_ULTRA,/**< Bit 8 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_SECURE_WRITE_ULTRA,	/**< Bit 9 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_RSA_KEY_WRITE_ULTRA = 15,
							/**< Bit 15 of Status reg */
	XSK_EFUSEPL_STATUS_DIABLE_128BIT_USER_KEY_WRITE_ULTRA,
	XSK_EFUSEPL_STATUS_AES_ONLY_ENABLED_ULTRA = 25,/**< Bit 25 of Status reg */
	XSK_EFUSEPL_STATUS_EFUSE_KEY_ONLY_DECRYPTION_ULTRA, /**< Bit 26 of Status reg */
	XSK_EFUSEPL_STATUS_RSA_AUTH_ENABLED_ULTRA,  /**< Bit 27 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_JTAG_ULTRA, /**< Bit 28 of Status reg */
	XSK_EFUSEPL_STATUS_SCAN_DISABLE_ULTRA, /**< Bit 29 of Status reg */
	XSK_EFUSEPL_STATUS_DISABLE_CRYPTO_ULTRA,/**< Bit 30 of Status reg */
	XSK_EFUSEPL_STATUS_ENABLE_OBFUSCATED_EFUSE_KEY /**< Bit 31 of Status reg */
}XSKEfusePl_FuseStatusBits_F8Series;
#endif
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/
typedef struct {
    /* Number of SLRs to iterate through */
    u32 NumSlr;
    /* Current SLR to iterate through */
    u32 CurSlr;
    /* Device IR length */
    u32 IrLen;
}XilSKey_JtagSlr;

/**
 * XSK_EfusePl is the PL eFUSE driver instance. Using this
 * structure, user can define the eFUSE bits to be
 * blown.
 */
typedef struct {
	/**
	 * Following are the FUSE CNTRL bits[1:5, 8-10]
	 */

	/**
	 * If XTRUE then part has to be power cycled to be able to be reconfigured only for zynq
	 */
	u32	ForcePowerCycle;/* Only for ZYNQ */
	/**
	 * If XTRUE will disable eFUSE write to FUSE_AES and FUSE_USER blocks valid
	 * only for zynq but in ultrascale If XTRUE will disable eFUSE write to
	 * FUSE_AESKEY block in Ultrascale
	 */
	u32 KeyWrite;		/* For ZYNQ and Ultrascale */
	/**
	 * If XTRUE will disable eFUSE read to FUSE_AES block and also disables eFUSE write to FUSE_AES and FUSE_USER blocks
	 * in Zynq Pl.but in Ultrascale if XTRUE will disable eFUSE read to FUSE_KEY block and also
	 * disables eFUSE write to FUSE_KEY blocks
	 */
	u32 AESKeyRead;		/* For Zynq and Ultrascale */
	/**
	 * If XTRUE will disable eFUSE read to FUSE_USER block and also disables eFUSE write to FUSE_AES and FUSE_USER blocks
	 * in zynq but in ultrascale if XTRUE will disable eFUSE read to FUSE_USER block
	 * and also disables eFUSE write to FUSE_USER blocks
	 */
	u32 UserKeyRead;	/* For Zynq and Ultrascale */
	/**
	 * If XTRUE will disable eFUSE write to FUSE_CNTRL block in both Zynq and
	 * Ultrascale
	 */
	u32 CtrlWrite;		/* For Zynq and Ultrascale */
	/**
	 * If XTRUE will disable eFuse read to FUSE_RSA block and also disables
	 * eFuse write to FUSE_RSA block in Ultrascale
	 */
	u32 RSARead;		/* only For Ultrascale */
	/**
	 * If XTRUE will disable eFUSE write to FUSE_USER block
	 * in Ultrascale
	 */
	u32 UserKeyWrite;	/* only For Ultrascale */
	/**
	 * If XTRUE will disable eFUSE write to FUSE_SEC block
	 * in Ultrascale
	 */
	u32 SecureWrite;	/* only For Ultrascale */
	/**
	 *  If XTRUE will disable eFUSE write to FUSE_RSA block
	 *  in Ultrascale
	 */
	u32 RSAWrite;	/* only For Ultrascale */
	/**
	 * If TRUE will disable eFUSE write to 128BIT FUSE_USER
	 * block in Ultrascale
	 */
	u32 User128BitWrite;	/* only For Ultrascale */
	/**
	 * IF XTRUE will disable eFuse read to FUSE_SEC block and also disables
	 * eFuse write to FUSE_SEC block in Ultrascale
	 */
	u32 SecureRead;		/* only For Ultrascale */
	/**
	 * If XTRUE will force eFUSE key to be used if booting
	 * Secure Image In Zynq
	 */
	u32 AESKeyExclusive;	/* Only for Zynq */
	/**
	 * If XTRUE then permanently sets the Zynq ARM DAP controller in bypass mode
	 * in both zynq and ultrascale.
	 */
	u32 JtagDisable;	/* for Zynq and Ultrascale */
	/**
	 * If XTRUE will force to use Secure boot with eFUSE key only
	 * for both Zynq and Ultrascale
	 */
	u32 UseAESOnly;		/* For Zynq and Ultrascale */
	/**
	 * If XTRUE will only allow encrypted bitstreams only
	 */
	 u32 EncryptOnly;	/* For Ultrascale only */
	/**
	 * If XTRUE then sets the disable's Xilinx internal test access
	 * in Ultrascale
	 */
	u32 IntTestAccessDisable;	/* Only for Ultrascale */
	/**
	 * If XTRUE then permanently disables the decryptor in Ultrascale
	 */
	u32 DecoderDisable;	/* Only for Ultrascale */
	/**
	 * Enable RSA authentication in ultrascale
	 */
	u32 RSAEnable;		/* only for Ultrascale */
	/*
	 * Enable Obfuscated feature for decryption of eFUSE AES
	 */
	u32 FuseObfusEn;	/* only for Ultrascale */
	/**
	 * Following is the define to select if the user wants to select AES key
	 * and User Low Key for Zynq
	 */
	u32 ProgAESandUserLowKey;	/* Only for Zynq */
	/**
	 * Following is the define to select if the user wants to select
	 * User Low Key for Zynq
	 */
	u32 ProgUserHighKey;	/* Only for Zynq */
	/**
	 * Following is the define to select if the user wants to select
	 * User key for Ultrascale
	 */
	u32 ProgAESKeyUltra;	/* Only for Ultrascale */
	/**
	 * Following is the define to select if the user wants to select
	 * User key for Ultrascale
	 */
	u32 ProgUserKeyUltra;	/* Only for Ultrascale */
	/**
	 * Following is the define to select if the user wants to select
	 * RSA key for Ultrascale
	 */
	u32 ProgRSAKeyUltra;	/* Only for Ultrascale */
	/**
	 * Following is the define to select if the user wants to program
	 * 128 bit User key for Ultrascale
	 */
	u32 ProgUser128BitUltra;	/* Only for Ultrascale */
	/**
	 * Following is the define to select if the user wants to read
	 * AES key for Ultrascale
	 */
	u32 CheckAESKeyUltra;	/* Only for Ultrascale */
	/**
	 * Following is the define to select if the user wants to read
	 * User key for Ultrascale
	 */
	u32 ReadUserKeyUltra;	/* Only for Ultrascale */
	/**
	 * Following is the define to select if the user wants to read
	 * RSA key for Ultrascale
	 */
	u32 ReadRSAKeyUltra;	/* Only for Ultrascale */
	/**
	 * Following is the define to select if the user wants to read
	 * 128 bit User key for Ultrascale
	 */
	u32 ReadUser128BitUltra;	/* Only for Ultrascale */
	/**
	 * This is the REF_CLK value in Hz
	 */
	/*u32	RefClk;*/
	/**
	 * This is for the aes_key value
	 */
	u8 AESKey[XSK_EFUSEPL_AES_KEY_SIZE_IN_BYTES]; /* for both Zynq and Ultrascale */
	/**
	 * This is for the user_key value
	 */
	u8 UserKey[XSK_EFUSEPL_USER_KEY_SIZE_IN_BYTES]; /* for both Zynq and Ultrascale */
	/**
	 * This is for the rsa_key value for Ultrascale
	 */
	u8 RSAKeyHash[XSK_EFUSEPL_RSA_KEY_HASH_SIZE_IN_BYTES]; /* Only for Ultrascale */
	/**
	 * This is for the User 128 bit key value for Ultrascale
	 */
	u8 User128Bit[XSK_EFUSEPL_128BIT_USERKEY_SIZE_IN_BYTES];
						/* Only for Ultrascale */
	/**
	 * TDI MIO Pin Number for ZYNQ
	 */
	u32 JtagMioTDI;		/* Only for ZYNQ */
	/**
	 * TDO MIO Pin Number for ZYNQ
	 */
	u32 JtagMioTDO;		/* Only for ZYNQ */
	/**
	 * TCK MIO Pin Number for ZYNQ
	 */
	u32 JtagMioTCK;		/* Only for ZYNQ */
	/**
	 * TMS MIO Pin Number for ZYNQ
	 */
	u32 JtagMioTMS;		/* Only for ZYNQ */
	/**
	 * MUX Selection MIO Pin Number for ZYNQ
	 */
	u32 JtagMioMuxSel;	/* Only for ZYNQ */
	/**
	 * Value on the MUX Selection line for ZYNQ
	 */
	u32 JtagMuxSelLineDefVal;/* Only for ZYNQ */
	/**
     * GPIO device ID
     */
	u32 JtagGpioID; /* Only for Ultrascale*/
	/**
	 * Hardware module Start signal's GPIO pin
	 * number
	 */
	u32 HwmGpioStart; /* Only for Ultrascale*/
	/**
	 * Hardware module Ready signal's GPIO pin
	 * number
	 */
	u32 HwmGpioReady; /* Only for Ultrascale*/
	/**
	 * Hardware module End signal's GPIO pin
	 * number
	 */
	u32 HwmGpioEnd; /* Only for Ultrascale*/
	/**
	 * TDI AXI GPIO pin number for Ultrascale
	 */
	u32 JtagGpioTDI;	/* Only for Ultrascale */
	/**
	 *  TDO AXI GPIO pin number for Ultrascale
	 */
	u32 JtagGpioTDO;	/* Only for Ultrascale */
	/**
	 *  TMS AXI GPIO pin number for Ultrascale
	 */
	u32 JtagGpioTMS;	/* Only for Ultrascale */
	/**
	 *  TCK AXI GPIO pin number for Ultrascale
	 */
	u32 JtagGpioTCK;	/* Only for Ultrascale */
	/**
	 *  AXI GPIO Channel number of all Inputs TDO
	 */
	u32 GpioInputCh;	/* Only for Ultrascale */
	/**
	 *  AXI GPIO Channel number for all Outputs TDI/TMS/TCK
	 */
	u32 GpioOutPutCh;	/* Only for Ultrascale */
	/**
	 * AES key read only for Zynq
	 */
	u8 AESKeyReadback[XSK_EFUSEPL_AES_KEY_SIZE_IN_BYTES];
	/**
	 * User key read in Ultrascale and Zynq
	 */
	u8 UserKeyReadback[XSK_EFUSEPL_USER_KEY_SIZE_IN_BYTES];
				/* for Ultrascale and Zynq */
	/**
	 * Expected AES key's CRC for Ultrascale here we can't read AES
	 * key directly
	 */
	u32 CrcOfAESKey;	/* Only for Ultrascale */
	/**
	 *  Flag is True is AES's CRC is matched, otherwise False
	 */
	 u8 AESKeyMatched;	/* Only for Ultrascale */
	/**
	 *  RSA key read back for Ultrascale
	 */
	u8 RSAHashReadback[XSK_EFUSEPL_RSA_KEY_HASH_SIZE_IN_BYTES];
				/* Only for Ultrascale */
	/**
	 * User 128 bit key read back for Ultrascale
	 */
	u8 User128BitReadBack[XSK_EFUSEPL_128BIT_USERKEY_SIZE_IN_BYTES];
			/* Only for Ultrascale */
	/**
	 * Internal variable to check if timer, XADC and JTAG are initialized.
	 */
	u32 SystemInitDone;
	/**
	 *  Stores Fpga series of Efuse
	 */
	XSKEfusePl_Fpga FpgaFlag;
	/**
	 *  CRC of AES key to verify programmed AES key
	 */
	u32 CrcToVerify; /* Only for Ultrascale */
	/**
	 *  Number of SLRs to iterate through
	 */
	u32 NumSlr;
	/**
	 *  Current SLR to iterate through
	 */
	u32 MasterSlr;
	/**
	 * Master SLR
	 */
	u32 SlrConfigOrderIndex;
	/**
	 * SLR Config Order Index
	 */

}XilSKey_EPl;
/** @}
@endcond */
/************************** Function Prototypes *****************************/
/************************** Constant Definitions *****************************/

u32 XilSKey_EfusePl_SystemInit(XilSKey_EPl *InstancePtr);

u32 XilSKey_EfusePl_Program(XilSKey_EPl *InstancePtr);

u32 XilSKey_EfusePl_ReadStatus(XilSKey_EPl *InstancePtr, u32 *StatusBits);

u32 XilSKey_EfusePl_ReadKey(XilSKey_EPl *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif	/* End of XILSKEY_EPL_H */
/**@}*/
