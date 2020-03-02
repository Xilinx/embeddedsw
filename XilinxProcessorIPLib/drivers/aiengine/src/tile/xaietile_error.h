/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaietile_error.h
* @{
*
*  Header file for error handling
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Wendy   01/09/2020  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIETILE_ERROR_H
#define XAIETILE_ERROR_H

/***************************** Include Files *********************************/

/***************************** Constant Definitions **************************/
/***************************** Macro Definitions *****************************/

#define XAIETILE_ERROR_BROADCAST	0U /**< Broadcast signal to broadcast errors */
#define XAIETILE_ERROR_SHIM_INTEVENT	0x10U /**< SHIM Internal event for errors */
#define XAIETILE_ERROR_ALL		0x0U /**< All errors except those are
						  set as poll only (no logging) */
/***************************** Type Definitions ******************************/

/************************** Function Prototypes  *****************************/
int XAieTile_ErrorRegisterNotification(XAieGbl *AieInst, u8 Module, u8 Error, XAieTile_ErrorCallBack Cb, void *Arg);
void XAieTile_ErrorUnregisterNotification(XAieGbl *AieInst, u8 Module, u8 Error, u8 Logging);
void XAieTile_ErrorsSetupDefaultHandler(XAieGbl *AieInst);
int XAieTile_ErrorsHandlingInitialize(XAieGbl *AieInst);
#endif		/* end of protection macro */

/** @} */
