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
* 1.01  kc   12/02/2019 Added performance timestamps
*       kc   12/17/2019 Added deferred error mechanism for mask poll
*       bsv  01/12/2020 Changes related to bitstream loading
*       ma   02/18/2020 Made performance measurement functions generic
*       bsv  04/03/2020 Code clean up Xilpdi
* 1.02  kc   06/12/2020 Added IPI mask to PDI CDO commands to get
* 						subsystem information
*       kc   06/23/2020 Added code print command details for errors
*       bsv  07/07/2020 Made functions used in single transaltion unit as
*						static
*       bsv  09/30/2020 Fix typo
*       bm   10/14/2020 Code clean up
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
#include "xplmi_cmd.h"
#include "xplmi_debug.h"
#include "xplmi_status.h"

/************************** Constant Definitions *****************************/
/** CDO Header definitions */
#define XPLMI_CDO_HDR_IDN_WRD		(0x004F4443U)
#define XPLMI_CDO_HDR_LEN		(0x5U)

/* Commands defined */
#define XPLMI_CMD_END			(0x01FFU)

#define XPLMI_CMD_STATE_START		(0U)
#define XPLMI_CMD_STATE_RESUME		(1U)

/* Define for Max short command length */
#define XPLMI_MAX_SHORT_CMD_LEN		(255U)

/* Define for Long command header length */
#define XPLMI_LONG_CMD_HDR_LEN		(2U)

/* Define for Short command length shift */
#define XPLMI_SHORT_CMD_LEN_SHIFT	(16U)

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
	u32 TempCmdBuf[8U];	/**< Temporary buffer to store commands
				 between iterations */
	XPlmi_Cmd Cmd;		/**< Pointer to the cmd */
	u8 CmdEndDetected;	/**< Flag to detect end of commands */
	u8 Cdo1stChunk;		/**< This is used for first time to validate
				CDO header*/
	u32 ImgId;		/**< Info about which Image this belongs to */
	u32 PrtnId;		/**< Info about which partition this belongs to*/
	u32 IpiMask;		/**< Info about which master has sent the request*/
	u32 DeferredError;	/**< Defer the error for any command till the
				  end of CDO processing */
} XPlmiCdo;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPlmi_InitCdo(XPlmiCdo *CdoPtr);
int XPlmi_ProcessCdo(XPlmiCdo *CdoPtr);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* end of XPLMI_CDO_H */
