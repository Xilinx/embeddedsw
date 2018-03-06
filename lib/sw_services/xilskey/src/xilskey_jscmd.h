/******************************************************************************
*
* Copyright (C) 2013 - 2018 Xilinx, Inc.  All rights reserved.
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

#ifndef XILSKEY_JSCMD_H
#define XILSKEY_JSCMD_H

#include "xilskey_js.h"

#ifdef __cplusplus
extern "C" {
#endif

extern js_server_t *js_init_zynq(void);
void initGPIO ();
int jtag_setPreAndPostPads (js_port_t *port_arg, int irPrePadBits, int irPostPadBits, int drPrePadBits, int drPostPadBits);
int jtag_navigate (js_port_t *port, js_state_t state);
int jtag_shift (js_port_t *port, unsigned char mode, int bits, unsigned char* wrBuffer, unsigned char* rdBuffer, js_state_t state);
unsigned int g_mio_jtag_tdi;
unsigned int g_mio_jtag_tdo;
unsigned int g_mio_jtag_tck;
unsigned int g_mio_jtag_tms;
unsigned int g_mio_jtag_mux_sel;
unsigned int g_mux_sel_def_val;

u32 GpioPinMasterJtagTDI;
u32 GpioPinMasterJtagTDO;
u32 GpioPinMasterJtagTMS;
u32 GpioPinMasterJtagTCK;

u32 GpioPinHwmStart;
u32 GpioPinHwmEnd;
u32 GpioPinHwmReady;

u32 GpioInPutCh;
u32 GpioOutPutCh;

// MIO assignments
#ifdef XSK_MICROBLAZE_PLATFORM
#define MIO_TDI    			GPIO_TDI
#define MIO_TDO    			GPIO_TDO
#define MIO_TCK    			GPIO_TCK
#define MIO_TMS    			GPIO_TMS
#else
#define MIO_TDI    			g_mio_jtag_tdi
#define MIO_TDO    			g_mio_jtag_tdo
#define MIO_TCK    			g_mio_jtag_tck
#define MIO_TMS    			g_mio_jtag_tms
#define MIO_MUX_SELECT		g_mio_jtag_mux_sel
#endif
#define GPIO_TDI			GpioPinMasterJtagTDI
#define GPIO_TDO			GpioPinMasterJtagTDO
#define GPIO_TMS			GpioPinMasterJtagTMS
#define GPIO_TCK			GpioPinMasterJtagTCK

#define GPIO_HWM_START		GpioPinHwmStart
#define GPIO_HWM_READY		GpioPinHwmReady
#define GPIO_HWM_END		GpioPinHwmEnd

#define TAP_IR_LENGTH		(6)
#define ZYNQ_DAP_IR_LENGTH		(4)

#define ATOMIC_DR_SCAN                 0x40
#define ATOMIC_IR_SCAN                 0x50

#define GPIO_BASE_ADDR		0xF8000700
#define GPIO_MASK_VAL		0x00003FFFU
#define GPIO_TDI_VAL		0x00001300U
#define GPIO_TDO_VAL		0x00001301U
#define GPIO_TCK_VAL		0x00001300U
#define GPIO_TMS_VAL		0x00001300U
#define GPIO_MUX_SEL_VAL	0x00001300U

/**
 * GPIO channel numbers of Ultrascale
 */
#define XSK_EFUSEPL_GPIO_CH1	(1)	/**< GPIO channel 1 */
#define XSK_EFUSEPL_GPIO_CH2	(2)	/**< GPIO channel 2 */

/* Maximum valid GPIO number */
#define XSK_GPIO_PIN_MAX	(31)	/**< Maximum valid
					  * gpio pin number */

/**
 * Option selection among efuse and bbram
 */
typedef enum {
	XSK_EFUSE,		/* For eFUSE */
	XSK_BBRAM		/* For BBRAM */
} XilSKey_ModuleSelection;

/**
 * XEfusePl is the PL eFUSE driver instance. Using this
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
	/* If XTRUE will disable eFUSE write to FUSE_USER block in Ultrascale */
	u32 UserKeyWrite;	/* only For Ultrascale */
	/* If XTRUE will disable eFUSE write to FUSE_SEC block in Ultrascale */
	u32 SecureWrite;	/* only For Ultrascale */
	/* If XTRUE will disable eFUSE write to FUSE_RSA block in Ultrascale */
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
	 * If XTRUE will force eFUSE key to be used if booting Secure Image In Zynq
	 */
	u32 AESKeyExclusive;	/* Only for Zynq */
	/**
	 * If XTRUE then permanently sets the Zynq ARM DAP controller in bypass mode
	 * in both zynq and ultrascale.
	 */
	u32 JtagDisable;	/* for Zynq and Ultrascale */
	/**
	 * If XTRUE will force to use Secure boot with eFUSE key only for both Zynq and Ultrascale
	 */
	u32 UseAESOnly;		/* For Zynq and Ultrascale */
	/**
	 * If XTRUE will only allow encrypted bitstreams only
	 */
	 u32 EncryptOnly;	/* For Ultrascale only */
	/**
	 * If XTRUE then sets the disable's Xilinx internal test access in Ultrascale
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
	/*
	 * Hardware module Start signal's GPIO pin
	 * number
	 */
	u32 HwmGpioStart; /* Only for Ultrascale*/
	/*
	 * Hardware module Ready signal's GPIO pin
	 * number
	 */
	u32 HwmGpioReady; /* Only for Ultrascale*/
	/*
	 * Hardware module End signal's GPIO pin
	 * number
	 */
	u32 HwmGpioEnd; /* Only for Ultrascale*/
	/* TDI AXI GPIO pin number for Ultrascale */
	u32 JtagGpioTDI;	/* Only for Ultrascale */
	/* TDO AXI GPIO pin number for Ultrascale */
	u32 JtagGpioTDO;	/* Only for Ultrascale */
	/* TMS AXI GPIO pin number for Ultrascale */
	u32 JtagGpioTMS;	/* Only for Ultrascale */
	/* TCK AXI GPIO pin number for Ultrascale */
	u32 JtagGpioTCK;	/* Only for Ultrascale */
	/* AXI GPIO Channel number of all Inputs TDO */
	u32 GpioInputCh;	/* Only for Ultrascale */
	/* AXI GPIO Channel number for all Outputs TDI/TMS/TCK */
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
	/* Flag is True is AES's CRC is matched, otherwise False */
	 u8 AESKeyMatched;	/* Only for Ultrascale */
	/* RSA key read back for Ultrascale */
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
	/* Stores Fpga series of Efuse */
	XSKEfusePl_Fpga FpgaFlag;
	/* CRC of AES key to verify programmed AES key */
    u32 CrcToVerify; /* Only for Ultrascale */
}XilSKey_EPl;

#ifdef __cplusplus
}
#endif

#endif /*XILSKEY_JSCMD_H*/
