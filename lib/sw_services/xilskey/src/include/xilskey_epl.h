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
*
* @file
*
* 		xilskey_epl.h
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
*
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
#define XSK_EFUSEPL_AES_KEY_SIZE_IN_BYTES				(32)
/**
 *  User Key size in Bytes
 */
#define XSK_EFUSEPL_USER_KEY_SIZE_IN_BYTES				(4)
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/
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
	 * If XTRUE then part has to be power cycled to be able to be reconfigured
	 */
	u32	ForcePowerCycle;
	/**
	 * If XTRUE will disable eFUSE write to FUSE_AES and FUSE_USER blocks
	 */
	u32 KeyWrite;
	/**
	 * If XTRUE will disable eFUSE read to FUSE_AES block and also disables
	 * eFUSE write to FUSE_AES and FUSE_USER blocks
	 */
	u32 AESKeyRead;
	/**
	 * If XTRUE will disable eFUSE read to FUSE_USER block and also disables
	 * eFUSE write to FUSE_AES and FUSE_USER blocks
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
	 * Following is the define to select if the user wants to select AES key
	 * and User Low Ley
	 */
	u32 ProgAESandUserLowKey;
	/**
	 * Following is the define to select if the user wants to select
	 * User Low Ley
	 */
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
	/**
	 * AES key read
	 */
	u8 AESKeyReadback[XSK_EFUSEPL_AES_KEY_SIZE_IN_BYTES];
	/**
	 * User key read
	 */
	u8 UserKeyReadback[XSK_EFUSEPL_USER_KEY_SIZE_IN_BYTES];
	/**
	 * Internal variable to check if timer, XADC and JTAG are initialized.
	 */
	u32 SystemInitDone;

}XilSKey_EPl;
/************************** Function Prototypes *****************************/
/************************** Constant Definitions *****************************/

u32 XilSKey_EfusePl_Program(XilSKey_EPl *PlInstancePtr);

u32 XilSKey_EfusePl_ReadStatus(XilSKey_EPl *InstancePtr, u32 *StatusBits);

u32 XilSKey_EfusePl_ReadKey(XilSKey_EPl *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif	/* End of XILSKEY_EPL_H */
