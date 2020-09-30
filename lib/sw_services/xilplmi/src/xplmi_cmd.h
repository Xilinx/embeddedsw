/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 1.01  bsv  01/09/2020 Changes related to bitsteram loading
*       bsv  02/13/2020 Added code to prevent Plmi generic commands from
*						getting executed via IPI
*       ma   02/28/2020 Added code to prevent EM commands from getting executed
*						via IPI
*       bsv  04/03/2020 Code clean up Xilpdi
* 1.02  kc   06/22/2020 Updated command handler error codes to include command
*                       IDs
*       skd  07/14/2020 Function pointer Func prototype changed
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       bsv  09/30/2020 Added parallel DMA support for SBI, JTAG, SMAP and PCIE
*                       boot modes
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
#include "xil_assert.h"
#include "xil_types.h"
#include "xplmi_status.h"

/************************** Constant Definitions *****************************/
#define XPLMI_CMD_API_ID_MASK			(0xFFU)
#define XPLMI_CMD_MODULE_ID_MASK		(0xFF00U)
#define XPLMI_CMD_LEN_MASK			(0xFF0000U)
#define XPLMI_CMD_RESP_SIZE			(8U)
#define XPLMI_CMD_RESUME_DATALEN			(8U)
#define XPLMI_CMD_HNDLR_MASK				(0xFF00U)
#define XPLMI_CMD_HNDLR_PLM_VAL				(0x100U)
#define XPLMI_CMD_HNDLR_EM_VAL				(0x800U)

/**************************** Type Definitions *******************************/
typedef struct XPlmi_Cmd XPlmi_Cmd;
typedef struct XPlmi_KeyHoleParams XPlmi_KeyHoleParams;

struct XPlmi_KeyHoleParams {
	/** < True implies copied in chunks of 64K */
	/** < False implies complete bitstream is copied in one chunk */
	u8 InChunkCopy;
	u64 SrcAddr; /**< Boot Source address */
	u32 ExtraWords; /**< Words that are directly DMAed to CFI */
	int (*Func) (u64 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);
};

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
	u32 ResumeData[XPLMI_CMD_RESUME_DATALEN];
	u32 DeferredError;
	XPlmi_KeyHoleParams KeyHoleParams;
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
