/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf_ipi.c
*
* This file contains the implementation of the client interface functions for
* IPI driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt  01/04/22 Initial release
*       kpt  01/13/22 Removed hardcoded IPI device id
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xpuf_ipi.h"
#include "xpuf_defs.h"

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Constant Definitions *****************************/
static XIpiPsu *PufIpiPtr = NULL;

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/****************************************************************************/
/**
 * @brief  This function sends IPI request to the target module and gets the
 * response from it
 *
 * @param	Arg0 		Payload argument 0
 * 		Arg1 		Payload argument 1
 *		Arg2 		Payload argument 2
 *		Arg3 		Payload argument 3
 *		Arg4 		Payload argument 4
 *		Arg5 		Payload argument 5
 *
 * @return	- XST_SUCCESS - If the IPI send and receive is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ****************************************************************************/
int XPuf_ProcessIpi(u32 Arg0, u32 Arg1, u32 Arg2, u32 Arg3,
	u32 Arg4, u32 Arg5)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	Payload[0U] = Arg0;
	Payload[1U] = Arg1;
	Payload[2U] = Arg2;
	Payload[3U] = Arg3;
	Payload[4U] = Arg4;
	Payload[5U] = Arg5;

	Status = XPuf_IpiSend(Payload);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPuf_IpiReadBuff32();

END:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function processes the IPI with 1 payload argument
 *
 * @param	ApiId 		API id of the IPI command
 *
 * @return	- XST_SUCCESS - If the IPI send and receive is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ****************************************************************************/
int XPuf_ProcessIpiWithPayload0(u32 ApiId)
{
	return XPuf_ProcessIpi(PufHeader(0U, ApiId), 0U, 0U, 0U,
		0U, 0U);
}

/****************************************************************************/
/**
 * @brief  This function processes the IPI with 2 payload arguments
 *
 * @param	ApiId 		API id of the IPI command
 * 		Arg1 		Payload argument 1
 *
 * @return	- XST_SUCCESS - If the IPI send and receive is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ****************************************************************************/
int XPuf_ProcessIpiWithPayload1(u32 ApiId, u32 Arg1)
{
	return XPuf_ProcessIpi(PufHeader(0U, ApiId), Arg1, 0U, 0U,
		0U, 0U);
}

/****************************************************************************/
/**
 * @brief  This function processes IPI request with 3 payload arguments
 *
 * @param	ApiId 		API id of the IPI command
 * 		Arg1 		Payload argument 1
 *		Arg2 		Payload argument 2
 *
 * @return	- XST_SUCCESS - If the IPI send and receive is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ****************************************************************************/
int XPuf_ProcessIpiWithPayload2(u32 ApiId, u32 Arg1, u32 Arg2)
{
	return XPuf_ProcessIpi(PufHeader(0U, ApiId), Arg1, Arg2, 0U,
		0U, 0U);
}

/****************************************************************************/
/**
 * @brief  This function processes IPI request with 4 payload arguments
 *
 * @param	ApiId 		API id of the IPI command
 * 		Arg1 		Payload argument 1
 *		Arg2 		Payload argument 2
 *		Arg3 		Payload argument 3
 *
 * @return	- XST_SUCCESS - If the IPI send and receive is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ****************************************************************************/
int XPuf_ProcessIpiWithPayload3(u32 ApiId, u32 Arg1, u32 Arg2, u32 Arg3)
{
	return XPuf_ProcessIpi(PufHeader(0U, ApiId), Arg1, Arg2, Arg3,
		0U, 0U);
}

/****************************************************************************/
/**
 * @brief  This function processes IPI request with 5 payload arguments
 *
 * @param	ApiId 		API id of the IPI command
 * 		Arg1 		Payload argument 1
 *		Arg2 		Payload argument 2
 *		Arg3 		Payload argument 3
 *		Arg4 		Payload argument 4
 *
 * @return	- XST_SUCCESS - If the IPI send and receive is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ****************************************************************************/
int XPuf_ProcessIpiWithPayload4(u32 ApiId, u32 Arg1, u32 Arg2, u32 Arg3,
	u32 Arg4)
{
	return XPuf_ProcessIpi(PufHeader(0U, ApiId), Arg1, Arg2, Arg3,
		Arg4, 0U);
}

/****************************************************************************/
/**
 * @brief  This function processes IPI request with 6 payload arguments
 *
 * @param	ApiId 		API id of the IPI command
 * 		Arg1 		Payload argument 1
 *		Arg2 		Payload argument 2
 *		Arg3 		Payload argument 3
 *		Arg4 		Payload argument 4
 *		Arg5 		Payload argument 5
 *
 * @return	- XST_SUCCESS - If the IPI send and receive is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ****************************************************************************/
int XPuf_ProcessIpiWithPayload5(u32 ApiId, u32 Arg1, u32 Arg2, u32 Arg3,
	u32 Arg4, u32 Arg5)
{
	return XPuf_ProcessIpi(PufHeader(0U, ApiId), Arg1, Arg2, Arg3,
		Arg4, Arg5);
}

/****************************************************************************/
/**
 * @brief  This function sends IPI request to the target module
 *
 * @param	Payload 	API id and call arguments to be written
 * 				in IPI buffer
 *
 * @return	- XST_SUCCESS - If the IPI send is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ****************************************************************************/
int XPuf_IpiSend(u32 *Payload)
{
	int Status = XST_FAILURE;

	if (NULL == PufIpiPtr) {
		XPuf_Printf(XPUF_DEBUG_GENERAL,
			"Passing NULL pointer to %s\r\n", __func__);
		goto END;
	}
	Status = XIpiPsu_PollForAck(PufIpiPtr, XPUF_TARGET_IPI_INT_MASK,
				    XPUF_IPI_TIMEOUT);
	if (Status != XST_SUCCESS) {
		XPuf_Printf(XPUF_DEBUG_GENERAL,
			"IPI Timeout expired in %s\n", __func__);
		goto END;
	}

	Status = XIpiPsu_WriteMessage(PufIpiPtr, XPUF_TARGET_IPI_INT_MASK, Payload,
				      PAYLOAD_ARG_CNT, XIPIPSU_BUF_TYPE_MSG);
	if (Status != XST_SUCCESS) {
		XPuf_Printf(XPUF_DEBUG_GENERAL,
				"Writing to IPI request buffer failed\n");
		goto END;
	}

	Status = XIpiPsu_TriggerIpi(PufIpiPtr, XPUF_TARGET_IPI_INT_MASK);

END:
	return Status;
}

/****************************************************************************/
/**
 * @brief	This function reads IPI Response after target module
 * 		has handled interrupt
 *
 * @return	- XST_SUCCESS - If the IPI send is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ****************************************************************************/
int XPuf_IpiReadBuff32(void)
{
	u32 Response[RESPONSE_ARG_CNT] = {0U};
	int Status = XST_FAILURE;

	if (NULL == PufIpiPtr) {
		XPuf_Printf(XPUF_DEBUG_GENERAL,
				"Passing NULL pointer to %s\r\n", __func__);
		goto END;
	}

	/* Wait until current IPI interrupt is handled by target module */
	Status = XIpiPsu_PollForAck(PufIpiPtr, XPUF_TARGET_IPI_INT_MASK,
				    XPUF_IPI_TIMEOUT);
	if (XST_SUCCESS != Status) {
		XPuf_Printf(XPUF_DEBUG_GENERAL,
				"IPI Timeout expired in %s\n", __func__);
		goto END;
	}

	Status = XIpiPsu_ReadMessage(PufIpiPtr, XPUF_TARGET_IPI_INT_MASK, Response,
				     RESPONSE_ARG_CNT, XIPIPSU_BUF_TYPE_RESP);
	if (XST_SUCCESS != Status) {
		XPuf_Printf(XPUF_DEBUG_GENERAL,
				"Reading from IPI response buffer failed\n");
		goto END;
	}

	Status = (int)Response[0U];

END:
	return Status;
}

/****************************************************************************/
/**
 * @brief  	Sets Ipi instance for xilpuf library
 *
 * @param  	IpiInst 	Pointer to IPI driver instance
 *
 * @return	- XST_SUCCESS - If the IPI poniter is successfully set
 * 		- XST_FAILURE - If there is a failure
 *
 ****************************************************************************/
int XPuf_SetIpi(XIpiPsu* const IpiInst)
{
	int Status = XST_FAILURE;

	if (NULL == IpiInst) {
		XPuf_Printf(XPUF_DEBUG_GENERAL,
				"Passing NULL pointer to %s\r\n", __func__);
		goto END;
	}

	PufIpiPtr = IpiInst;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
*
* This function configures and initializes the IPI
*
* @param	IpiInstPtr	Pointer to IPI instance
*
* @return
* 		- XST_SUCCESS if Ipi configuration is successful
*		- XST_FAILURE if Ipi configuration is failed.
*
******************************************************************************/
int XPuf_InitializeIpi(XIpiPsu* const IpiInstPtr)
{
	int Status = XST_FAILURE;
	XIpiPsu_Config *IpiCfgPtr;

	/* Look Up the config data */
	IpiCfgPtr = XIpiPsu_LookupConfig(XPUF_IPI_CHANNEL);
	if (NULL == IpiCfgPtr) {
		Status = XST_FAILURE;
		XPuf_Printf(XPUF_DEBUG_GENERAL,
			"%s ERROR in getting CfgPtr\n");
		goto END;
	}

	/* Init with the Cfg Data */
	Status = XIpiPsu_CfgInitialize(IpiInstPtr, IpiCfgPtr,
		IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		XPuf_Printf(XPUF_DEBUG_GENERAL,
			"%s ERROR #%d in configuring IPI\n", Status);
		goto END;;
	}

END:
	return Status;
}