/******************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file xplmi_usermodule_example.c
*
* This example demonstrates the usage of UserModule API's.
* Each IPI channel can trigger an interrupt to itself and can exchange messages
* through the message buffer. This feature is used here to exercise the usermodule API's
* Example control flow:
* - Initialize the XMailbox instance
* - Write a Message and Trigger IPI to Self in Blocking mode.
* - Interrupt handler receives IPI and Sends back response.
*
*This Example is supported for Versal devices.
*
* Procedure to Run the example.
*------------------------------------------------------------------------------
* Load the Pdi.
* Select the target.
* Download the example elf into the target.
*
* Changes Required in PLM to make use of the example.
*------------------------------------------------------------------------------
* https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/2037088327/Versal+Platform+Loader+and+Manager#User-Modules
* Refer the above link to Initialize and utilize usermodules.
* These Changes are Required in PLM to run this example.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   AA      05/26/25 Intial Release
* </pre>
*
*
******************************************************************************/

/**
 * @addtogroup xplmi_client_example_apis XilPlmi Client Example APIs
 * @{
 */

/***************************** Include Files *********************************/
#include <stdlib.h>
#include <xil_printf.h>
#include "xparameters.h"
#ifndef SDT
#include "xscugic.h"
#endif
#include "xilmailbox.h"
#include "xdebug.h"
#ifdef SDT
#include "xilmailbox_hwconfig.h"
#endif

/************************** Constant Definitions *****************************/
/* IPI device ID to use for this test */
#ifdef SDT
#define IPI_CHANNEL	XMAILBOX_IPI_BASEADDRESS
#define PMC_IPI_MASK	XMAILBOX_IPI_CHANNEL_ID
#else
#define IPI_CHANNEL	XPAR_XIPIPSU_0_DEVICE_ID
#define PMC_IPI_MASK	XPAR_XIPIPSU_0_BIT_MASK
#endif

/* Test message length in words. Max is 8 words (32 bytes) */
#define MSG_LEN		8U

/* User Module ID  and Command ID*/
#define USER_MODULE_ID	0x80U
#define CMD_SEND	0x0U
#define CMD_SEND_RECV	0x1U
#define RECV_STATUS	0x1U

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static u32 UserModuleSend(void);
static u32 UserModuleSendRecv(void);

static void DoneHandler(void *CallBackRef);
static void ErrorHandler(void *CallBackRef, u32 ErrorStatus);

/************************** Variable Definitions *****************************/
static XMailbox XMboxInstance;
static volatile u32 RecvDone	= 0U;		/**< Done flag */
static volatile u32 Error	= 0U;		/**< Error flag*/

/*****************************************************************************/

/*************************************************************************************************/
/**
 * @brief	Main function to call the user module example functions.
 *
 * @return
 *		 - XST_SUCCESS on success.
 *		 - XST_FAILURE on failure.
 *
 *************************************************************************************************/
u32 main(void)
{
	/*status flag */
	u32 Status = (u32)XST_FAILURE;

	Status = UserModuleSend();
	if (Status != (u32)XST_SUCCESS) {
		xil_printf("User_module_send Failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran user module send\n\r");

	Status = UserModuleSendRecv();
	if (Status != (u32)XST_SUCCESS) {
		xil_printf("User_module_recv Failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran user module recv\n\r");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function will trigger the interrupt and send messages.
 *
 * @return
 * 		 - XST_SUCCESS on success.
 * 		 - XST_FAILURE on failure.
 *
 *****************************************************************************/
static u32 UserModuleSend(void)
{
	/*status flag */
	u32 Status = (u32)XST_FAILURE;

	/* Used to store the module id and cmd id in LSB and message to send*/
	u32 ReqBuffer[MSG_LEN];

	Status = XMailbox_Initialize(&XMboxInstance,IPI_CHANNEL);

	if(Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Register callbacks for Error and Read handlers*/
	(void)XMailbox_SetCallBack(&XMboxInstance, XMAILBOX_RECV_HANDLER,
	(void *)DoneHandler, (void *)&XMboxInstance);

	(void)XMailbox_SetCallBack(&XMboxInstance, XMAILBOX_ERROR_HANDLER,
	(void *)ErrorHandler, (void *)&XMboxInstance);

	/* xilplmi expects MODULE_ID | CMD_ID in LSB*/
	ReqBuffer[0] = (USER_MODULE_ID << 8) | CMD_SEND ;

	/* Send an IPI Req Message */
	Status = XMailbox_SendData(&XMboxInstance, PMC_IPI_MASK, ReqBuffer,
	MSG_LEN, XILMBOX_MSG_TYPE_REQ, (u8)1);

	if(Status != (u32)XST_SUCCESS){
		xil_printf("Sending Req Message Failed\n\r");
		goto END;
	}

	xil_printf("Request message sent sucessfully\n\r");

	if(Error){
		xil_printf("Error Occurred During IPI Transfer\n\r");
		Status =(u32)XST_FAILURE;
		goto END;
	}

	xil_printf("DONE\n\r");

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function will trigger the interrupt and exchange messages.
 *
 * @return
 * 		 - XST_SUCCESS on success.
 * 		 - XST_FAILURE on failure.
 *
 *****************************************************************************/
static u32 UserModuleSendRecv(void)
{
	/*status flag */
	u32 Status = (u32)XST_FAILURE;

	/* Used to store the module id and cmd id in LSB and message to send*/
	u32 ReqBuffer[MSG_LEN];

	/*used to store Received message*/
	u32 RespBuffer[MSG_LEN];

	Status = XMailbox_Initialize(&XMboxInstance,IPI_CHANNEL);

	if(Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Register callbacks for Error and Read handlers*/
	(void)XMailbox_SetCallBack(&XMboxInstance, XMAILBOX_RECV_HANDLER,
	(void *)DoneHandler, (void *)&XMboxInstance);

	(void)XMailbox_SetCallBack(&XMboxInstance, XMAILBOX_ERROR_HANDLER,
	(void *)ErrorHandler, (void *)&XMboxInstance);

	/* xilplmi expects MODULE_ID | CMD_ID in LSB*/
	ReqBuffer[0] = (USER_MODULE_ID << 8) | CMD_SEND_RECV ;

	/* Send an IPI Req Message */
	Status = XMailbox_SendData(&XMboxInstance, PMC_IPI_MASK, ReqBuffer,
	MSG_LEN, XILMBOX_MSG_TYPE_REQ, 1);

	if(Status != (u32)XST_SUCCESS){
		xil_printf("Sending Req Message Failed\n\r");
		goto END;
	}

	xil_printf("Request message sent sucessfully\n\r");

	while(!Error && !RecvDone);

	if(Error){
		xil_printf("Error Occurred During IPI Transfer\n\r");
		Status =(u32)XST_FAILURE;
		goto END;
	}

	/* Read an IPI Resp Message */
	Status = XMailbox_Recv(&XMboxInstance,PMC_IPI_MASK, RespBuffer,
	MSG_LEN, XILMBOX_MSG_TYPE_RESP);

	if (Status != (u32)XST_SUCCESS) {
		xil_printf("Reading an IPI Resp message Failed\n\r");
		goto END;
	}
	xil_printf("Response Message Read sucessfully\r\n");

	xil_printf("DONE\n\r");

END:
	return Status;
}
/*************************************************************************************************/
/**
 * @brief	When IPI channel Sends Response message from PLM which intialized in user module this
			function will be called and changes the status of RecvDone to 1 .
 *
 * @param	CallBackRef is the reference pointer to callback function.
 *
 *************************************************************************************************/
static void DoneHandler(void *CallBackRef)
{
	RecvDone = (u32)RECV_STATUS;
}

/*************************************************************************************************/
/**
 * @brief	This Function will be called if any error occur during IPI transfer.
 *
 * @param	CallBackRef is the reference pointer to callback function.
 * @param	ErrorStatus is the Error occured during IPI transfer.
 *
 *************************************************************************************************/
static void ErrorHandler(void *CallBackRef, u32 ErrorStatus)
{
	Error = ErrorStatus;
}
