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

// MIO assignments
#define MIO_TDI    			g_mio_jtag_tdi
#define MIO_TDO    			g_mio_jtag_tdo
#define MIO_TCK    			g_mio_jtag_tck
#define MIO_TMS    			g_mio_jtag_tms
#define MIO_MUX_SELECT		g_mio_jtag_mux_sel

#define ZYNQ_TAP_IR_LENGTH		(6)
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
 * XEfusePl is the PL eFUSE driver instance. Using this
 * structure, user can define the eFUSE bits to be
 * blown.
 */
typedef struct {
	/**
	 * Following are the FUSE CNTRL bits[1:5, 8-10]
	 */

	/**
	 * If XTRUE then part has to be power cycled to be able to be reconfigured
	 */
	u32	ForcePowerCycle;
	/**
	 * If XTRUE will disable eFUSE write to FUSE_AES and FUSE_USER blocks
	 */
	u32 KeyWrite;
	/**
	 * If XTRUE will disable eFUSE read to FUSE_AES block and also disables eFUSE write to FUSE_AES and FUSE_USER blocks
	 */
	u32 AESKeyRead;
	/**
	 * If XTRUE will disable eFUSE read to FUSE_USER block and also disables eFUSE write to FUSE_AES and FUSE_USER blocks
	 */
	u32 UserKeyRead;
	/**
	 * If XTRUE will disable eFUSE write to FUSE_CNTRL block
	 */
	u32	CtrlWrite;
	/**
	 * If XTRUE will force eFUSE key to be used if booting Secure Image
	 */
	u32 AESKeyExclusive;
	/**
	 * If XTRUE then permanently sets the Zynq ARM DAP controller in bypass mode
	 */
	u32 JtagDisable;
	/**
	 * If XTRUE will force to use Secure boot with eFUSE key only
	 */
	u32 UseAESOnly;
	/**
	 * Following is the define to select if the user wants to select AES key and USER low key OR USER high key or BOTH
	 */
	u32 ProgAESandUserLowKey;
	u32 ProgUserHighKey;

	/**
	 * This is the REF_CLK value in Hz
	 */
	/*u32	RefClk;*/
	/**
	 * This is for the aes_key value
	 */
	u8 AESKey[XSK_EFUSEPL_AES_KEY_SIZE_IN_BYTES];
	/**
	 * This is for the user_key value
	 */
	u8 UserKey[XSK_EFUSEPL_USER_KEY_SIZE_IN_BYTES];
	/**
	 * TDI MIO Pin Number
	 */
	u32 JtagMioTDI;
	/**
	 * TDO MIO Pin Number
	 */
	u32 JtagMioTDO;
	/**
	 * TCK MIO Pin Number
	 */
	u32 JtagMioTCK;
	/**
	 * TMS MIO Pin Number
	 */
	u32 JtagMioTMS;
	/**
	 * MUX Selection MIO Pin Number
	 */
	u32 JtagMioMuxSel;
	/**
	 * Value on the MUX Selection line
	 */
	u32	JtagMuxSelLineDefVal;

}XilSKey_EPl;

#ifdef __cplusplus
}
#endif

#endif /*XILSKEY_JSCMD_H*/
