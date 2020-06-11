/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
#define XPLMI_CDO_HDR_IDN_WRD		(0x004F4443U)
#define XPLMI_CDO_HDR_LEN			(0x5U)

/* Commands defined */
#define XPLMI_CMD_END				(0x00100U)

#define XPLMI_CMD_STATE_START		(0U)
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
	u32 ImgId;		/**< Info about which Image this belongs to */
	u32 PrtnId;		/**< Info about which partition this belongs to*/
	u32 IpiMask;		/**< Info about which master has sent the request*/
	u32 DeferredError;	/**< Defer the error for any command till the
				  end of CDO processing */
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
