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
* 			xilskey_bbram_example.c
* @note
*	Example illustrates the method of programming the AES key to BBRAM.
*	The user key should be mentioned in xilskey_input.h.
*	BBRAM is a battery backed RAM and there is no restriction on the
*	number of times key can programmed.
*	This algorithm only wokrs when program and verify key are done
*	together and in that order.
.
* Procedure:
* 1. Create a hardware project using the appropriate design.
* 2. Create a BSP.
* 3. Create fsbl
* 4. Create an application project using bbram example and xilskey_input.h
* 5. The 256 bit key to be programmed has to be mentioned
* 6. A hardware setup which dedicates four MIO pins for JTAG signals
*    should be used and the MIO pins should be mentioned in xilskey_input.h
*    There should be a method to download this example and have the
*    MIO pins connected to JTAG before running this application.
* 7. Run the application which will program and verify the BBRAM key
* 8. After programming and veifying the key, power off.
* 9. Place a BBRAM key encrypted boot image in one of the boot devices,
*    set the appropriate boot pins and power on.
* 10. The bbram example will be run from DDR with the default linker.
* 11. To run from OCM, modify the linker to map all sections to
*     ps7_ram_0_S_AXI_BASEADDR instead of ps7_ddr_0_S_AXI_BASEADDR.
*
*
* MODIFICATION HISTORY:
*
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.01a hk      09/18/13 First release
*
*
****************************************************************************/
/***************************** Include Files *********************************/
#include "stdio.h"
#include "xil_io.h"
#include "xstatus.h"
#include "xilskey_utils.h"
#include "xilskey_bbram.h"
#include "xilskey_input.h"
/************************** Constant Definitions *****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/************************** Variable Definitions ****************************/
XilSKey_Bbram InstancePtr;
/************************** Function Prototypes *****************************/
int XilSKey_Bbram_InitData(XilSKey_Bbram *BbramInstancePtr);
/***************************************************************************/

int main()
{
	int Status;
	u8 StartProg;

	xil_printf(" BBRAM Example start \r\n");

	Status = XilSKey_Bbram_InitData(&InstancePtr);
	if(Status != XST_SUCCESS) {
		xil_printf(" BBRAM Example failed \r\n");
		return XST_FAILURE;
	}

	Status = XilSKey_Bbram_Program(&InstancePtr);
	if(Status != XST_SUCCESS) {
		xil_printf(" BBRAM Example failed \r\n");
		return XST_FAILURE;
	}

	xil_printf(" Successfully programmed and verified BBRAM key \r\n");
	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
*
*  Function to initialize BBRAM instance
*
*
* @param	BBRAM instance pointer
*
* @return
*
*	- XST_SUCCESS - In case of Success
*	- XST_FAILURE - If initialization fails
*
* @note
*
*****************************************************************************/
int XilSKey_Bbram_InitData(XilSKey_Bbram *BbramInstancePtr)
{

	u32 Status;

	BbramInstancePtr->ForcePowerCycle		= 	XSK_BBRAM_FORCE_PCYCLE_RECONFIG;
	BbramInstancePtr->JtagDisable			= 	XSK_BBRAM_DISABLE_JTAG_CHAIN;
	BbramInstancePtr->JtagMioTDI 			=	XSK_BBRAM_MIO_JTAG_TDI;
	BbramInstancePtr->JtagMioTDO			=	XSK_BBRAM_MIO_JTAG_TDO;
	BbramInstancePtr->JtagMioTCK			=	XSK_BBRAM_MIO_JTAG_TCK;
	BbramInstancePtr->JtagMioTMS			=	XSK_BBRAM_MIO_JTAG_TMS;
	BbramInstancePtr->JtagMioMuxSel 		=	XSK_BBRAM_MIO_JTAG_MUX_SELECT;
	BbramInstancePtr->JtagMuxSelLineDefVal	=  XSK_BBRAM_MIO_MUX_SEL_DEFAULT_VAL;

	/*
	 * Convert key given in xilskey_input.h and
	 * assign it to the variable in instance.
	 */
	XilSKey_Efuse_ConvertStringToHexBE(XSK_BBRAM_AES_KEY,
				&(BbramInstancePtr->AESKey[0]),
				XSK_BBRAM_AES_KEY_SIZE_IN_BITS);

	Status = XST_SUCCESS;
	return Status;

}
