/******************************************************************************
*
* Copyright (C) 2013 - 2019 Xilinx, Inc.  All rights reserved.
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
* @file xilskey_bbram.h
* @addtogroup xilskey_zynq_ultra_bbram BBRAM PL
* @{
* @cond xilskey_internal
* @{
* @note
*		 Contains the function prototypes, defines and macros for
*		 BBRAM functionality.
*
* MODIFICATION HISTORY:
*
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.01a hk      09/18/13 First release
* 4.0   vns     10/08/15 Added prototypes for ZynqMp BBRAM PS
* 5.0   vns     01/09/16 Added functionality of Ultrascale BBRAM
*                        programming.
* 6.0   vns     07/28/16 Added IsKeyObfuscated, Enable_DpaProtect, Dpa_Count,
*                        Dpa_Mode and CtrlWord fields to BBRAM instance.
* 6.1   vns     10/25/16 Removed ForcePowerCycle and JtagDisable from BBRAM
*                        Zynq instance, as they are not actually programming
*                        any bit They already exists in Zynq eFUSE PL instances
* 6.6   vns     06/06/18 Added doxygen tags
* 6.7   arc     25/02/19 Changed void XilSKey_ZynqMp_Bbram_Zeroise(void)
*                        prototype to u32 XilSKey_ZynqMp_Bbram_Zeroise(void)
*       psl     03/20/19 Added BBRAM key write support for SSIT devices.
*       psl     03/29/19 Added Support for user configurable GPIO for jtag
*                        control.
****************************************************************************/
#ifndef XILSKEY_BBRAM_H
#define XILSKEY_BBRAM_H

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/
/************************** Constant Definitions *****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/************************** Variable Definitions ****************************/
typedef struct {

	/**
	 * This is for the aes_key value
	 */
	u8 AESKey[32];
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
     * GPIO device ID
     */
    u32 JtagGpioID; /* Only for Ultrascale*/
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
	/* Is BBRAM key is obfuscated */
	u16 IsKeyObfuscated;	/* Only for Ultrascale */
	/* DPA protection enable */
	u16 Enable_DpaProtect;	/* Only for Ultrascale */
	/* DPA count */
	u16 Dpa_Count;	/* Only for Ultrascale */
	/* DPA configuration */
	u16 Dpa_Mode;	/* Only for Ultrascale */
	/* BBRAM control word to be programmed */
	u32 CtrlWord;	/* Only for Ultrascale */

	/* Stores Fpga series of BBRAM */
	XSKEfusePl_Fpga FpgaFlag;

	u32 Crc; /* Crc of AES key and control word of Ultrascale BBRAM */
    /* Number of SLRs to iterate through */
    u32 NumSlr;
    /* Current SLR to iterate through */
    u32 CurSlr;

}XilSKey_Bbram;
/************************** Constant Definitions *****************************/
/*
 * Constant definitions for instruction used in BBRAM key program and verify
 */
#define JPROGRAM 0x0B
#define ISC_NOOP 0x14
#define ISC_ENABLE 0x10
#define ISC_PROGRAM_KEY 0x12
#define ISC_PROGRAM 0x11
#define ISC_READ 0x15
#define ISC_DISABLE 0x16
#define BYPASS 0x3F

/*
 * Pre and post pads
 */
#define IRHEADER	0
#define IRTRAILER	4
#define DRHEADER	0
#define DRTRAILER	1

/*
 * Pre and post pads for BYPASS in de-init
 */
#define IRHEADER_BYP	0
#define IRTRAILER_BYP	0
#define DRHEADER_BYP	0
#define DRTRAILER_BYP	0

/*
 * Instruction load length
 */
#define IRLENGTH	6

/*
 * Data register lengths for program
 */
#define DRLENGTH_PROGRAM	32
/*
 * Data register lengths for verify
 */
#define DRLENGTH_VERIFY		37
/*
 * Data register lengths for data load after ISC_ENABLE
 */
#define DRLENGTH_EN		5
/*
 * Data register load after ISC_ENABLE
 */
#define DR_EN		0x15

/*
 * Timer function load for 100msec
 */
#define TIMER_100MS	1000000

/*
 * IRCAPTURE status - init complete mask
 */
#define INITCOMPLETEMASK	0x10

/*
 * Number of char's in a KEY
 */
#define NUMCHARINKEY	32
/*
 * Number of words in a KEY
 */
#define NUMWORDINKEY	8

/*
 * Data register shift before key verify
 */
#define DATAREGCLEAR	0x1FFFFFFFE0

/*
 * Number of LSB status bits in 37 bit read to shifted out
 */
#define NUMLSBSTATUSBITS	5

/*
 * Data and instruction loads in de-init
 */
#define IRDEINIT_H		0x03
#define IRDEINIT_L		0xFF
#define DRDEINIT		0x00

/*
 * Data and instruction lenghts in de-init
 */
#define IRDEINITLEN		10
#define DRDEINITLEN		2
/** @}
@endcond */
/************************** Function Prototypes *****************************/
/*
 * Function for BBRAM program and verify algorithm
 */
int XilSKey_Bbram_Program(XilSKey_Bbram *InstancePtr);
int XilSKey_Bbram_JTAGServerInit(XilSKey_Bbram *InstancePtr);
/**@}*/

/** @addtogroup xilskey_zynqMP zynqmp bbram
 @{ */

/* Functions to program AES key and function to zeroise AES key */
u32 XilSKey_ZynqMp_Bbram_Program(u32 *AesKey);
u32 XilSKey_ZynqMp_Bbram_Zeroise(void);
/**@}*/

#ifdef __cplusplus
}
#endif

#endif	/* End of XILSKEY_BBRAM_H */
