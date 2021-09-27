/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_ipi.c
*
* This file contains the implementation of the client interface functions for
* IPI driver. Refer to the header file xsecure_ipi.h for more
* detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/23/21 Initial release
* 4.5   kal  03/23/20 Updated file version to sync with library version
*       har  04/14/21 Renamed XSecure_ConfigIpi as XSecure_SetIpi
*                     Added XSecure_InitializeIpi
*       am   05/22/21 Resolved MISRA C violations
* 4.6   har  07/14/21 Fixed doxygen warnings
*       kpt  09/27/21 Fixed compilation warnings
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_ipi.h"
#include "xsecure_defs.h"

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Constant Definitions *****************************/
static XIpiPsu *IpiPtr;

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/****************************************************************************/
/**
 * @brief  This function sends IPI request to the target module and gets the
 * response from it
 *
 * @param	Arg0		Payload argument 0
 * @param	Arg1		Payload argument 1
 * @param	Arg2		Payload argument 2
 * @param	Arg3		Payload argument 3
 * @param	Arg4		Payload argument 4
 * @param	Arg5		Payload argument 5
 *
 * @return	- XST_SUCCESS - If the IPI send and receive is successful
 * 		- XST_FAILURE - If there is a failure
 *
 * @note	Payload  consists of API id and call arguments to be written
 * 		in IPI buffer
 *
 ****************************************************************************/
int XSecure_ProcessIpi(u32 Arg0, u32 Arg1, u32 Arg2, u32 Arg3,
	u32 Arg4, u32 Arg5)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	Payload[0] = (u32)Arg0;
	Payload[1] = (u32)Arg1;
	Payload[2] = (u32)Arg2;
	Payload[3] = (u32)Arg3;
	Payload[4] = (u32)Arg4;
	Payload[5] = (u32)Arg5;

	Status = XSecure_IpiSend(Payload);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_IpiReadBuff32();

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
int XSecure_ProcessIpiWithPayload0(u32 ApiId)
{
	return XSecure_ProcessIpi(HEADER(0UL, ApiId), 0U, 0U, 0U,
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
int XSecure_ProcessIpiWithPayload1(u32 ApiId, u32 Arg1)
{
	return XSecure_ProcessIpi(HEADER(0UL, ApiId), Arg1, 0U, 0U,
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
int XSecure_ProcessIpiWithPayload2(u32 ApiId, u32 Arg1, u32 Arg2)
{
	return XSecure_ProcessIpi(HEADER(0UL, ApiId), Arg1, Arg2, 0U,
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
int XSecure_ProcessIpiWithPayload3(u32 ApiId, u32 Arg1, u32 Arg2, u32 Arg3)
{
	return XSecure_ProcessIpi(HEADER(0UL, ApiId), Arg1, Arg2, Arg3,
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
int XSecure_ProcessIpiWithPayload4(u32 ApiId, u32 Arg1, u32 Arg2, u32 Arg3,
	u32 Arg4)
{
	return XSecure_ProcessIpi(HEADER(0UL, ApiId), Arg1, Arg2, Arg3,
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
int XSecure_ProcessIpiWithPayload5(u32 ApiId, u32 Arg1, u32 Arg2, u32 Arg3,
	u32 Arg4, u32 Arg5)
{
	return XSecure_ProcessIpi(HEADER(0UL, ApiId), Arg1, Arg2, Arg3,
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
int XSecure_IpiSend(u32 *Payload)
{
	int Status = XST_FAILURE;

	if (NULL == IpiPtr) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"Passing NULL pointer to %s\r\n", __func__);
		goto END;
	}
	Status = XIpiPsu_PollForAck(IpiPtr, TARGET_IPI_INT_MASK,
				    XSECURE_IPI_TIMEOUT);
	if (Status != XST_SUCCESS) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"IPI Timeout expired in %s\n", __func__);
		goto END;
	}

	Status = XIpiPsu_WriteMessage(IpiPtr, TARGET_IPI_INT_MASK, Payload,
				      PAYLOAD_ARG_CNT, XIPIPSU_BUF_TYPE_MSG);
	if (Status != XST_SUCCESS) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
				"Writing to IPI request buffer failed\n");
		goto END;
	}

	Status = XIpiPsu_TriggerIpi(IpiPtr, TARGET_IPI_INT_MASK);

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
int XSecure_IpiReadBuff32(void)
{
	u32 Response[RESPONSE_ARG_CNT] = {0U};
	int Status = XST_FAILURE;

	/* Wait until current IPI interrupt is handled by target module */
	Status = XIpiPsu_PollForAck(IpiPtr, TARGET_IPI_INT_MASK,
				    XSECURE_IPI_TIMEOUT);
	if (XST_SUCCESS != Status) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
				"IPI Timeout expired in %s\n", __func__);
		goto END;
	}

	Status = XIpiPsu_ReadMessage(IpiPtr, TARGET_IPI_INT_MASK, Response,
				     RESPONSE_ARG_CNT, XIPIPSU_BUF_TYPE_RESP);
	if (XST_SUCCESS != Status) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
				"Reading from IPI response buffer failed\n");
		goto END;
	}

	Status = (int)Response[0];

END:
	return Status;
}

/****************************************************************************/
/**
 * @brief  	Sets Ipi instance for xilsecure library
 *
 * @param  	IpiInst 	Pointer to IPI driver instance
 *
 * @return	- XST_SUCCESS - If the IPI poniter is successfully set
 * 		- XST_FAILURE - If there is a failure
 *
 ****************************************************************************/
int XSecure_SetIpi(XIpiPsu* const IpiInst)
{
	int Status = XST_FAILURE;

	if (NULL == IpiInst) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
				"Passing NULL pointer to %s\r\n", __func__);
		goto END;
	}

	IpiPtr = IpiInst;

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
int XSecure_InitializeIpi(XIpiPsu* const IpiInstPtr)
{
	int Status = XST_FAILURE;
	XIpiPsu_Config *IpiCfgPtr;

	/* Look Up the config data */
	IpiCfgPtr = XIpiPsu_LookupConfig(XPAR_XIPIPSU_0_DEVICE_ID);
	if (NULL == IpiCfgPtr) {
		Status = XST_FAILURE;
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"%s ERROR in getting CfgPtr\n");
		goto END;
	}

	/* Init with the Cfg Data */
	Status = XIpiPsu_CfgInitialize(IpiInstPtr, IpiCfgPtr,
		IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
			"%s ERROR #%d in configuring IPI\n",Status);
		goto END;;
	}

END:
	return Status;
}
