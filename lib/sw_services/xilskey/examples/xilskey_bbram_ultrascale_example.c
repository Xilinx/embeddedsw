/******************************************************************************
* Copyright (c) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 6.7   psl     03/20/19 Added BBRAM key write support for SSIT devices.
*       psl     03/29/19 Added Support for user configurable GPIO for
*                        jtag control.
* 6.8   psl     05/21/19 Initialized SystemInitDone ,to indicate
*                        XilSKey_Bbram_JTAGServerInit status.
*                        And passed BbramInstancePtr instead of InstancePtr in
*                        XilSKey_Bbram_JTAGServerInit.
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
int XilSKey_Bbram_ProgramSLR(XilSKey_Bbram *BbramInstancePtr);
void Bbram_Close_Ultra(void);

/*****************************************************************************/

int main()
{
	int Status;

	Status = XilSKey_Bbram_InitData(&InstancePtr);
	if(Status != XST_SUCCESS) {
		xil_printf("App: Initialization failed \r\n");
		return XST_FAILURE;
	}

	/**
	 * Verify the SLR requested to program against target
	 * validity
	 */
	switch(InstancePtr.NumSlr)
	{

		case XSK_TARGET_MAX_1_SLRS:
			if(((XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_1 == TRUE) ||
					(XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_2 == TRUE) ||
					(XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_3 == TRUE) ||
					(XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_1 == TRUE) ||
					(XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_2 == TRUE) ||
					(XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_3 == TRUE)))
			{
				xil_printf("App: Only %d SLR(s) present in the target"
						",example failed\r\n",
						InstancePtr.NumSlr);
				xil_printf("App:Disable input for SLR2/SLR3/SLR4 \r\n");
				Status = (u32)XST_FAILURE;
			}
			break;
		case XSK_TARGET_MAX_2_SLRS:
			if(((XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_2 == TRUE) ||
					(XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_3 == TRUE) ||
					(XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_2 == TRUE) ||
					(XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_3 == TRUE)))
			{
				xil_printf("App: Only %d SLR(s) present in the target"
						",example failed\r\n",
						InstancePtr.NumSlr);
				xil_printf("App:Disable input for SLR3/SLR4 \r\n");
				Status = (u32)XST_FAILURE;
			}
			break;
		case XSK_TARGET_MAX_3_SLRS:
			if(((XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_3 == TRUE) ||
					(XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_3 == TRUE))) {
				xil_printf("App: Only %d SLR(s) present in the target"
						",example failed\r\n",
						InstancePtr.NumSlr);
				xil_printf("App:Disable input for SLR4\r\n");
				Status = (u32)XST_FAILURE;
			}
			break;
		case XSK_TARGET_MAX_4_SLRS:
			xil_printf("App: SLR Count verification passed\r\n");
			Status = (u32)XST_SUCCESS;
			break;
		default:
			xil_printf("App: SLR Count not matched for the device\r\n");
			Status = (u32)XST_FAILURE;
			break;
	}

	if(Status != (u32)XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	/*************************************************************************/

	if(XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_0 == TRUE)
	{
		/*
		 * Convert key given in xilskey_input.h and
		 * assign it to the variable in instance.
		 */
		XilSKey_Efuse_ConvertStringToHexLE(XSK_BBRAM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_0,
					&(InstancePtr.AESKey[0]),
					XSK_BBRAM_AES_KEY_SIZE_IN_BITS);
	}
	else
	{
		/*
		 * Convert key given in xilskey_input.h and
		 * assign it to the variable in instance.
		 */
		XilSKey_Efuse_ConvertStringToHexLE(XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_0,
					&(InstancePtr.AESKey[0]),
					XSK_BBRAM_AES_KEY_SIZE_IN_BITS);
	}


	if((XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_0 == TRUE) ||
			(XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_0 == TRUE))
	{

		InstancePtr.SlrConfigOrderIndex = XSK_SLR_CONFIG_ORDER_0;
		Status = XilSKey_Bbram_ProgramSLR(&InstancePtr);
		if(Status != XST_SUCCESS)
		{
			return XST_FAILURE;
		}
	}

	/*************************************************************************/
	if(XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_1 == TRUE)
	{
		/*
		 * Convert key given in xilskey_input.h and
		 * assign it to the variable in instance.
		 */
		XilSKey_Efuse_ConvertStringToHexLE(XSK_BBRAM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_1,
					&(InstancePtr.AESKey[0]),
					XSK_BBRAM_AES_KEY_SIZE_IN_BITS);
	}
	else
	{
		/*
		 * Convert key given in xilskey_input.h and
		 * assign it to the variable in instance.
		 */
		XilSKey_Efuse_ConvertStringToHexLE(XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_1,
					&(InstancePtr.AESKey[0]),
					XSK_BBRAM_AES_KEY_SIZE_IN_BITS);
	}

	if((XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_1 	== TRUE) ||
			(XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_1 == TRUE))	{

		InstancePtr.SlrConfigOrderIndex = XSK_SLR_CONFIG_ORDER_1;
		Status = XilSKey_Bbram_ProgramSLR(&InstancePtr);
		if(Status != XST_SUCCESS)
		{
			return XST_FAILURE;
		}
	}
	/*************************************************************************/
	if(XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_2 == TRUE)
	{
		/*
		 * Convert key given in xilskey_input.h and
		 * assign it to the variable in instance.
		 */
		XilSKey_Efuse_ConvertStringToHexLE(XSK_BBRAM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_2,
					&(InstancePtr.AESKey[0]),
					XSK_BBRAM_AES_KEY_SIZE_IN_BITS);
	}
	else
	{
		/*
		 * Convert key given in xilskey_input.h and
		 * assign it to the variable in instance.
		 */
		XilSKey_Efuse_ConvertStringToHexLE(XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_2,
					&(InstancePtr.AESKey[0]),
					XSK_BBRAM_AES_KEY_SIZE_IN_BITS);
	}

	if((XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_2 	== TRUE) ||
			(XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_2 == TRUE))
	{
		InstancePtr.SlrConfigOrderIndex = XSK_SLR_CONFIG_ORDER_2;
		Status = XilSKey_Bbram_ProgramSLR(&InstancePtr);
		if(Status != XST_SUCCESS)
		{
			return XST_FAILURE;
		}
	}
	/*************************************************************************/
	if(XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_3 == TRUE)
	{
		/*
		 * Convert key given in xilskey_input.h and
		 * assign it to the variable in instance.
		 */
		XilSKey_Efuse_ConvertStringToHexLE(XSK_BBRAM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_3,
					&(InstancePtr.AESKey[0]),
					XSK_BBRAM_AES_KEY_SIZE_IN_BITS);
	}
	else
	{
		/*
		 * Convert key given in xilskey_input.h and
		 * assign it to the variable in instance.
		 */
		XilSKey_Efuse_ConvertStringToHexLE(XSK_BBRAM_AES_KEY_SLR_CONFIG_ORDER_3,
					&(InstancePtr.AESKey[0]),
					XSK_BBRAM_AES_KEY_SIZE_IN_BITS);
	}

	if((XSK_BBRAM_PGM_AES_KEY_SLR_CONFIG_ORDER_3== TRUE) ||
			(XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_3 == TRUE))
	{
		InstancePtr.SlrConfigOrderIndex = XSK_SLR_CONFIG_ORDER_3;
		Status = XilSKey_Bbram_ProgramSLR(&InstancePtr);
		if(Status != XST_SUCCESS)
		{
			return XST_FAILURE;
		}
	}

	Bbram_Close_Ultra();

	return XST_SUCCESS;

}
/*****************************************************************************/
/**
*  Function to program the key for specific SLR
*
*
* @param	SLR number
*
* @return
* 		- XST_SUCCESS - In case of Success
*		- XST_FAILURE - If initialization fails
*
* @note		None.
*
******************************************************************************/
int XilSKey_Bbram_ProgramSLR(XilSKey_Bbram *BbramInstancePtr)
{
	u32 Status;

	xil_printf("******** Processing SLR of config order %d ********\r\n",
			(BbramInstancePtr->SlrConfigOrderIndex));
	Status = XilSKey_Bbram_Program(BbramInstancePtr);
	if(Status != XST_SUCCESS) {
		xil_printf("Ultrascale BBRAM key write failed for SLR of"
					" config order index %d\r\n",
				(BbramInstancePtr->SlrConfigOrderIndex));
		return XST_FAILURE;
	}
	xil_printf("Successfully programmed and verified Ultrascale"
						" BBRAM key for SLR of config oder index %d\r\n",
				(BbramInstancePtr->SlrConfigOrderIndex));
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

	BbramInstancePtr->JtagGpioID  = XSK_BBRAM_AXI_GPIO_DEVICE_ID;
	BbramInstancePtr->JtagGpioTDI = XSK_BBRAM_AXI_GPIO_JTAG_TDI;
	BbramInstancePtr->JtagGpioTMS = XSK_BBRAM_AXI_GPIO_JTAG_TMS;
	BbramInstancePtr->JtagGpioTCK = XSK_BBRAM_AXI_GPIO_JTAG_TCK;
	BbramInstancePtr->JtagGpioTDO = XSK_BBRAM_AXI_GPIO_JTAG_TDO;
	BbramInstancePtr->GpioInputCh = XSK_BBRAM_GPIO_INPUT_CH;
	BbramInstancePtr->GpioOutPutCh = XSK_BBRAM_GPIO_OUTPUT_CH;

	BbramInstancePtr->SystemInitDone = 0;

	if(XilSKey_Bbram_JTAGServerInit(BbramInstancePtr) != XST_SUCCESS) {
		xil_printf("JTAG Sever Init failed \r\n");
		return XST_FAILURE;
	}


	if((XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_0 	== TRUE) ||
		(XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_1		== TRUE) ||
		(XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_2		== TRUE) ||
		(XSK_BBRAM_PGM_OBFUSCATED_KEY_SLR_CONFIG_ORDER_3		== TRUE))
	{
		BbramInstancePtr->IsKeyObfuscated = TRUE;

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
