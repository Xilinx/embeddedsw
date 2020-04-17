/******************************************************************************
* Copyright (c) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 6.1   vns     10/25/16 Removed ForcePowerCycle and JtagDisable from Zynq
*                        BBRAM configuration as they are not actually
*                        programming any control bit.
* 6.7   psl     03/20/19 Added BBRAM jtag server init function.
*       psl     03/15/19 Moved XilSKey_Bbram_JTAGServerInit function from
*                        example to library.
* 6.8   psl     05/21/19 Initialized SystemInitDone ,to indicate
*                        XilSKey_Bbram_JTAGServerInit status.
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

	BbramInstancePtr->JtagMioTDI 			=	XSK_BBRAM_MIO_JTAG_TDI;
	BbramInstancePtr->JtagMioTDO			=	XSK_BBRAM_MIO_JTAG_TDO;
	BbramInstancePtr->JtagMioTCK			=	XSK_BBRAM_MIO_JTAG_TCK;
	BbramInstancePtr->JtagMioTMS			=	XSK_BBRAM_MIO_JTAG_TMS;
	BbramInstancePtr->JtagMioMuxSel 		=	XSK_BBRAM_MIO_JTAG_MUX_SELECT;
	BbramInstancePtr->JtagMuxSelLineDefVal	=  XSK_BBRAM_MIO_MUX_SEL_DEFAULT_VAL;

	BbramInstancePtr->SystemInitDone = 0;
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
