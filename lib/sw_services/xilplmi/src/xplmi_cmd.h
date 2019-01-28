/******************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc. All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_cmd.h
*
* This is the file which contains command execution code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/23/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLMI_CMD_H
#define XPLMI_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include <xil_assert.h>
#include "xil_types.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/
#define XPLMI_CMD_API_ID_MASK			(0xFFU)
#define XPLMI_CMD_MODULE_ID_MASK		(0xFF00U)
#define XPLMI_CMD_LEN_MASK			(0xFF0000U)
#define XPLMI_CMD_RESP_SIZE			(4U)

/**************************** Type Definitions *******************************/
typedef struct XPlmi_Cmd XPlmi_Cmd;

struct XPlmi_Cmd {
	u32 SubsystemId;
	u32 IpiMask;
	u32 CmdId;
	u32 Len;
	u32 ProcessedLen;
	u32 PayloadLen;
	u32 *Payload;
	u32 Response[XPLMI_CMD_RESP_SIZE];
	int (*ResumeHandler)(XPlmi_Cmd * CmdPtr);
	void *ResumeData;
};

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPlmi_CmdExecute(XPlmi_Cmd * CmdPtr);
int XPlmi_CmdResume(XPlmi_Cmd * CmdPtr);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* end of XPLMI_CMD_H */
