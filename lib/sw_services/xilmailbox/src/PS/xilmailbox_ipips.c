/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilmailbox_ipips.c
 * @addtogroup xilmailbox Overview
 * @{
 * @details
 *
 * This file contains the definitions for ZynqMP and versal IPI implementation.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   adk  14/02/19    Initial Release
 * 1.1   sd   16/08/19    Initialise status variable
 * 1.3   sd   03/03/21    Doxygen Fixes
 * 1.4   sd   23/06/21    Fix MISRA-C warnings
 * 1.5   dp   22/11/21    Update XIpiPs_RegisterIrq() to check whether GIC has
 *                        already been setup or not and if it was setup skip
 *                        initializing GIC again and just register handlers.
 * 1.6   sd   28/02/21    Add support for microblaze
 *       kpt  03/16/21    Fixed compilation warning on microblaze
 * 1.7   sd   01/04/22    Replace memset with Xil_SMemSet
 * 1.8   ana  05/02/23	  Updated XIpiPs_PollforDone logic to improve
 *						  AES client performance
 *	 ht   05/30/23	  Added support for system device-tree flow.
 *	 ht   06/12/23	  Fix MISRA-C warnings
 *	 ht   07/24/23	  Restructure the code for more modularity
 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xilmailbox.h"
#include "xilmailbox_ipips_control.h"
#include "xil_util.h"

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/****************************************************************************/
/**
 * Initialize the XMailbox Instance
 *
 * @param	InstancePtr is a pointer to the instance to be worked on
 * @param	DeviceId is the IPI Instance to be worked on
 *
 * @return	XST_SUCCESS if initialization was successful
 * 		XST_FAILURE in case of failure
 */
/****************************************************************************/
#ifndef SDT
u32 XMailbox_Initialize(XMailbox *InstancePtr, u8 DeviceId)
#else
u32 XMailbox_Initialize(XMailbox *InstancePtr, UINTPTR BaseAddress)
#endif
{
	u32 Status = XST_FAILURE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Wrapper function to memset function */
	Status = (u32) Xil_SMemSet((void *)InstancePtr, (u32)sizeof(XMailbox), 0, sizeof(XMailbox));
	if (Status != XST_SUCCESS) {
		return Status;
	}

	InstancePtr->XMbox_IPI_SendData = XIpiPs_SendData;
	InstancePtr->XMbox_IPI_Send = XIpiPs_Send;
	InstancePtr->XMbox_IPI_Recv = XIpiPs_RecvData;

	/* Initialize the InstancePtr */
#ifndef SDT
	Status = XIpiPs_Init(InstancePtr, DeviceId);
#else
	Status = XIpiPs_Init(InstancePtr, BaseAddress);
#endif
	return Status;
}

