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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_cdo.h
*
* This is the file which contains cdo related code.
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
#ifndef XPLMI_CDO_H
#define XPLMI_CDO_H

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xil_types.h"
#include "xplmi_cmd.h"
#include "xplmi_debug.h"
#include "xplmi_status.h"

/************************** Constant Definitions *****************************/
/** CDO Header definitions */
#define XPLMI_CDO_HDR_IDN_WRD			(0x004F4443U)
#define XPLMI_CDO_HDR_LEN			(0x5U)

/** commands defined */
#define XPLMI_CMD_END				(0x00100U)

#define XPLMI_CMD_STATE_RESUME		(1U)

/**************************** Type Definitions *******************************/
/**
 * The XPlmiCdo is instance data. The user is required to allocate a
 * variable of this type for every Cdo processing. A pointer
 * to a variable of this type is then passed to the PLMI CDO API functions.
 */
typedef struct {
	u32 *BufPtr;		/**< CDO Buffer */
	u32 BufLen;		/**< Buffer length */
	u32 CdoLen;		/**< CDO length */
	u32 ProcessedCdoLen;	/**< Processed CDO length */
	u32 CmdState;		/**< Cmd processing state */
	u32 CopiedCmdLen;	/**< Copied Command length */
	u32 TempCmdBuf[8];	/**< temperary buffer to store commands
				 between iterations */
	XPlmi_Cmd Cmd;		/**< Pointer to the cmd */
	u32 CmdEndDetected;	/**< Flag to detect end of commands */
	u32 Cdo1stChunk;	/**< This is used for first time to validate
				CDO header*/
	u32 ImgId;		/** Info about which Image this belongs to */
	u32 PrtnId;		/** Info about which partition this belongs to*/
} XPlmiCdo;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void XPlmi_InitCdo(XPlmiCdo *CdoPtr);
int XPlmi_ProcessCdo(XPlmiCdo *CdoPtr);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* end of XPLMI_CDO_H */
