/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
* @file
* 	xilskey_bbram_ultrascale_example.c
* @note
*	Example illustrates the method of programming the AES key to Ultrascale
*	BBRAM.
*	The AES key to be programmed should be mentioned in
*	xilskey_bbram_ultrascale_input.h.
*	BBRAM is a battery backed RAM and there is no restriction on the
*	number of times key can programmed.
*	This algorithm only works when program and verify key are done
*	together and in that order.
*
*	Accessing Ultrascale microblaze 's BBRAM is done by using block RAM
*	initialization.
*	Master Jtag primitive has to added to design i.e MASTER_JTAG_inst
*	instantiation have to performed and AXI GPIO pins has to be connected
*	to TDO, TDI, TMS and TCK signals of MASTER_JTAG	primitive.
*	All Inputs(TDO) and All Outputs(TDI, TMS, TCK) of MASTER_JTAG can be
*	connected as
*	1) All Inputs to one channel
*	All Outputs to other channel
*	Valid example: All Outputs connected to Channel 1
*	Input signal TDO also connected to channel 2
*	2) All Inputs and All Outputs to same channel.
*	Valid example: All Outputs connected to Channel 1
*	Input signal TDO also connected to channel 1
*	3) But some of the Outputs in one channel and some of them in different
*	channel is not accepted.
*	Invalid example: All Outputs connected to Channel 1
*	Input signals (TDI, TMS) connected to Channel 1
*	Input signal TCK also connected to channel 2
*
*	The design should only contain AXI bram Ctrl memory mapped(1MB).
*	Note: MASTER_JTAG will disable all other JTAGs
*	Procedure to access BBRAM of Ultrascale:
*	1) After providing the required inputs in
*	xilskey_bbram_ultrascale_input.h, compile the project.
*	2) Generate a memory mapped interface file using TCL command
*	write_mem_info $Outfilename
*	3) Update memory has to be done using the tcl command updatemem.
*	updatemem -meminfo $file.mmi -data $Outfilename.elf -bit $design.bit
*			-proc design_1_i/microblaze_0 -out $Final.bit
*	4) Program the board using $Final.bit bitstream
*	5) Output can be seen in UART terminal.
*
*
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ----    -------- ------------------------------------------------------
* 5.0   vns     09/01/16 First Release.
* 6.0   vns     07/28/16 Updated example to allow counting configuration
*                        feature and to program Obfuscated key.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "stdio.h"
#include "xil_io.h"
#include "xstatus.h"
#include "xilskey_utils.h"
#include "xilskey_bbram.h"
#include "xilskey_bbram_ultrascale_input.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Variable Definitions *****************************/

XilSKey_Bbram InstancePtr;

/************************** Function Prototypes ******************************/

int XilSKey_Bbram_InitData(XilSKey_Bbram *BbramInstancePtr);

/*****************************************************************************/

int main()
{
	int Status;

	Status = XilSKey_Bbram_InitData(&InstancePtr);
	if(Status != XST_SUCCESS) {
		xil_printf("Ultrascale BBRAM Example failed \r\n");
		return XST_FAILURE;
	}

	Status = XilSKey_Bbram_Program(&InstancePtr);
	if(Status != XST_SUCCESS) {
		xil_printf("Ultrascale BBRAM Example failed \r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully programmed and verified Ultrascale"
							" BBRAM key \r\n");
	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*  Function to initialize BBRAM instance
*
*
* @param	BBRAM instance pointer
*
* @return
* 		- XST_SUCCESS - In case of Success
*		- XST_FAILURE - If initialization fails
*
* @note		None.
*
******************************************************************************/
int XilSKey_Bbram_InitData(XilSKey_Bbram *BbramInstancePtr)
{

	u32 Status;

	BbramInstancePtr->JtagGpioTDI = XSK_BBRAM_AXI_GPIO_JTAG_TDI;
	BbramInstancePtr->JtagGpioTMS = XSK_BBRAM_AXI_GPIO_JTAG_TMS;
	BbramInstancePtr->JtagGpioTCK = XSK_BBRAM_AXI_GPIO_JTAG_TCK;
	BbramInstancePtr->JtagGpioTDO = XSK_BBRAM_AXI_GPIO_JTAG_TDO;
	BbramInstancePtr->GpioInputCh = XSK_BBRAM_GPIO_INPUT_CH;
	BbramInstancePtr->GpioOutPutCh = XSK_BBRAM_GPIO_OUTPUT_CH;

	if (XSK_BBRAM_PGM_OBFUSCATED_KEY == TRUE) {
		BbramInstancePtr->IsKeyObfuscated =
					XSK_BBRAM_PGM_OBFUSCATED_KEY;
		/*
		 * Convert key given in xilskey_input.h and
		 * assign it to the variable in instance.
		 */
		XilSKey_Efuse_ConvertStringToHexLE(XSK_BBRAM_OBFUSCATED_KEY,
					&(BbramInstancePtr->AESKey[0]),
					XSK_BBRAM_AES_KEY_SIZE_IN_BITS);
		if (XSK_BBRAM_DPA_PROTECT_ENABLE == TRUE) {
			xil_printf("WARNING: DPA_PROTECT is not supported "
				"for Obfucated key, DPA count and mode "
				"will be programmed with default values\n\r");
		}
		BbramInstancePtr->Enable_DpaProtect = 0;
		BbramInstancePtr->Dpa_Count = 0;
		BbramInstancePtr->Dpa_Mode = XSK_BBRAM_INVALID_CONFIGURATIONS;
	}
	else {
		/*
		 * Convert key given in xilskey_input.h and
		 * assign it to the variable in instance.
		 */
		XilSKey_Efuse_ConvertStringToHexLE(XSK_BBRAM_AES_KEY,
					&(BbramInstancePtr->AESKey[0]),
					XSK_BBRAM_AES_KEY_SIZE_IN_BITS);
#if (XSK_BBRAM_DPA_PROTECT_ENABLE == TRUE)
		if (XSK_BBRAM_DPA_COUNT == 0) {
			xil_printf("ERROR: To enable DPA protection "
			"DPA count can't be zero valid values are "
			"1 to 255\n\r");
			return XST_FAILURE;
		}
		BbramInstancePtr->Enable_DpaProtect =
					XSK_BBRAM_DPA_PROTECT_ENABLE;
		BbramInstancePtr->Dpa_Count = XSK_BBRAM_DPA_COUNT;
		BbramInstancePtr->Dpa_Mode = XSK_BBRAM_DPA_MODE;
#else
		BbramInstancePtr->Enable_DpaProtect = 0;
		BbramInstancePtr->Dpa_Count = 0;
		BbramInstancePtr->Dpa_Mode = XSK_BBRAM_INVALID_CONFIGURATIONS;
#endif
	}

	Status = XST_SUCCESS;

	return Status;

}
