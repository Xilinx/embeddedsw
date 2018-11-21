/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_ipi.h
*
* This is the header file which contains definitions for the ipi manager
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  mg   10/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_IPI_H
#define XPLMI_IPI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xplmi_modules.h"
#include "xplmi_cmd.h"
#include "xil_assert.h"
#ifdef XPAR_XIPIPSU_0_DEVICE_ID
#include "xipipsu.h"

/************************** Constant Definitions *****************************/
#define XPLMI_IPI_MASK_COUNT		XIPIPSU_MAX_TARGETS
#define XPLMI_IPI_MAX_MSG_LEN		XIPIPSU_MAX_MSG_LEN

/* IPI defines */
#define IPI_BASEADDR				(0xFF300000U)
#define IPI_PMC_ISR					(IPI_BASEADDR + 0x20010U)
#define IPI_PMC_ISR_PSM_BIT_MASK	(0x1U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPlmi_IpiInit(void);
int XPlmi_IpiDispatchHandler(void *Data);
int XPlmi_IpiWrite(u32 DestCpuMask, u32 *MsgPtr, u32 MsgLen, u32 Type);
int XPlmi_IpiRead(u32 SrcCpuMask, u32 *MsgPtr, u32 MsgLen, u32 Type);
int XPlmi_IpiTrigger(u32 DestCpuMask);
int XPlmi_IpiPollForAck(u32 DestCpuMask, u32 TimeOutCount);
int XPlmi_ValidateIpiCmd(u32 CmdId);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
#endif /* XPAR_XIPIPSU_0_DEVICE_ID */

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_IPI_H */
