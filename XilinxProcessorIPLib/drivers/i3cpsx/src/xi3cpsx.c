
/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xi3cpsx.c
* @addtogroup Overview
* @{
*
* Handles init functions.
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---  -------- ---------------------------------------------
* 1.00 	sd   11/21/21 First release
* 1.2   sd    2/12/23 Remove the hardcoding devices
*       	      Copy the input clock
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xi3cpsx.h"
#include "xi3cpsx_pr.h"
#include "sleep.h"

XI3cPsx_Cmd DAA_Cmd[2];

s32 XI3cPsx_CfgInitialize(XI3cPsx *InstancePtr, XI3cPsx_Config *ConfigPtr,
						u32 EffectiveAddr)
{
	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	if (InstancePtr->IsReady == XIL_COMPONENT_IS_READY) {
		return XST_DEVICE_IS_STARTED;
	}

	/* Set the values read from the device config and the base address. */
	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;
	InstancePtr->Config.BaseAddress = EffectiveAddr;
	InstancePtr->Config.DeviceCount = ConfigPtr->DeviceCount;
	InstancePtr->Config.InputClockHz = ConfigPtr->InputClockHz;

	/* Indicate the instance is now ready to use, initialized without error */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	/*
	 * Reset the I3C controller to get it into its initial state. It is expected
	 * that device configuration will take place after this initialization
	 * is done, but before the device is started.
	 */
	XI3cPsx_Reset(InstancePtr);
	XI3cPsx_ResetFifos(InstancePtr);

	if (ConfigPtr->DeviceCount != 0) {
		XI3cPsx_SetSClk(InstancePtr);
		XI3cPsx_BusInit(InstancePtr);
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief
* This function sends the transfer command.
*
*
* @param        InstancePtr is a pointer to the XI3cPsx instance.
* @param        CmdCCC is a pointer to the Command Info.
*
* @return
*               - XST_SUCCESS if everything went well.
*               - XST_FAILURE if any error.
*
*
******************************************************************************/
s32 XI3cPsx_SendTransferCmd(XI3cPsx *InstancePtr, struct CmdInfo * CmdCCC)
{
	s32 ret = 0;

	/* RSTDAA */
	DAA_Cmd[0].TransCmd = COMMAND_PORT_ARG_DATA_LEN(CmdCCC->RxLen) | COMMAND_PORT_TRANSFER_ARG;
	/* Refer Transfer command data structure in DWC MIPI I3C Master and Slave controller Databook */
	DAA_Cmd[0].TransArg = (COMMAND_PORT_SPEED(0) |	/* 21 - 23 Selecting SDR0 */
			COMMAND_PORT_DEV_INDEX(CmdCCC->SlaveAddr) | /* 16 - 20 */
			COMMAND_PORT_CMD(CmdCCC->Cmd) |	/* 7 - 14 Selecting broadcast*/
			COMMAND_PORT_CP |	/* 15 */
			COMMAND_PORT_TOC |	/* 30 */
			COMMAND_PORT_ROC);	/* 26 */;
	if (CmdCCC->RxLen) {
		DAA_Cmd[0].TransArg |= COMMAND_PORT_READ_TRANSFER;	/* 28 - For read */
	}

	xil_printf("E Arg 0x%x\t Cmd 0x%x\n", DAA_Cmd[0].TransCmd, DAA_Cmd[0].TransArg);
	if (CmdCCC->RxLen) {
		ret = XI3cPsx_MasterRecvPolled(InstancePtr, CmdCCC->RxBuff, CmdCCC->RxLen, &DAA_Cmd[0]);
	} else {
		ret = XI3cPsx_MasterSendPolled(InstancePtr, NULL, 0, DAA_Cmd[0]);
	}

	return ret;

}

/*****************************************************************************/
/**
* @brief
* This function sends the Address Assignment command.
*
*
* @param        InstancePtr is a pointer to the XI3cPsx instance.
* @param        CmdCCC is a pointer to the Command Info.
*
* @return
*               - XST_SUCCESS if everything went well.
*               - XST_FAILURE if any error.
*
*
******************************************************************************/
s32 XI3cPsx_SendAddrAssignCmd(XI3cPsx *InstancePtr, struct CmdInfo * CmdCCC)
{
	s32 ret = 0;

	/* ENTDAA */
	DAA_Cmd[0].TransCmd = COMMAND_PORT_TRANSFER_ARG;
	/* Refer Address assignment command data structure in DWC MIPI I3C Master and Slave controller Databook */
	DAA_Cmd[0].TransArg = ((InstancePtr->Config.DeviceCount << 21) |
				COMMAND_PORT_DEV_INDEX(CmdCCC->SlaveAddr) | /* 16 - 20 */
				COMMAND_PORT_CMD(CmdCCC->Cmd) |	/* 7 - 14 */
				COMMAND_PORT_ADDR_ASSGN_CMD |	/* 0 - 2 */
				COMMAND_PORT_TOC |	/* 30 */
				COMMAND_PORT_ROC);	/* 26 */
	xil_printf("E Arg 0x%x\t Cmd 0x%x\n", DAA_Cmd[0].TransCmd, DAA_Cmd[0].TransArg);
	ret = XI3cPsx_MasterSendPolled(InstancePtr, NULL, 0, DAA_Cmd[0]);

	return ret;
}
