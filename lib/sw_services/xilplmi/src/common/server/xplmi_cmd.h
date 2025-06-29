/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
*       bm   10/14/2020 Code clean up
* 1.03  ma   03/04/2021 Removed duplicate defines
*       ma   03/03/2021 Added IpiReqType member in XPlmi_Cmd structure
*       bsv  04/13/2021 Added support for variable Keyhole sizes in
*                       DmaWriteKeyHole command
* 1.04  bsv  07/05/2021 Added code to handle case where bitstream data starts
*                       at 32K boundary
*       bsv  07/16/2021 Fix doxygen warnings
*       bsv  08/02/2021 Change type of variables to reduce size
* 1.05  bsv  10/26/2021 Code clean up
*       ma   01/31/2022 Removed unused defines
*       rj   05/25/2022 Added a new AckInPLM member in XPlmi_Cmd structure
*       bm   07/06/2022 Refactor versal and versal_net code
*       bm   08/24/2022 Support Begin, Break and End commands across chunk
*                       boundaries
* 1.8   skg  10/04/2022 Added masks for SLR ID and Zeriozing the SLR ID
* 1.9   bm   07/11/2023 Added XPlmi_ClearEndStack member to XPlmi_Cmd structure
*       pre  09/18/2024 Added XPLMI_SLR_INDEX_SHIFT, SLR index macros
*       obs  02/04/2025 Updated XPLMI_CMD_RESP_SIZE macro value
*       pre  03/02/2025 Added BufIndex as member in cmd structure
*       ng   03/26/2025 Include xplm config header file for max user modules macro definition
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
#include "xplmi_status.h"
#include "xplmi_config.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/
#define XPLMI_CMD_API_ID_MASK			(0xFFU)
#define XPLMI_CMD_MODULE_ID_MASK		(0xFF00U)
#define XPLMI_CMD_LEN_MASK			(0xFF0000U)
#define XPLMI_CMD_RESP_SIZE			(7U)
#define XPLMI_CMD_RESUME_DATALEN		(8U)
#define XPLMI_CMD_MODULE_ID_SHIFT		(8U)
#define XPLMI_CMD_SLR_ID_MASK           (0X000000C0U)              /* Mask for extracting SlrIndex */
#define XPLMI_SLR_ID_ZEROISE            ~(XPLMI_CMD_SLR_ID_MASK)  /* Mask for making SlrIndex Zero after use*/
#define XPLMI_SLR_INDEX_SHIFT           (6U)                       /* Shift value for extracting SLRIndex */
#define XPLMI_BEGIN_OFFSET_STACK_SIZE		(10U)
/**
 * SSIT SLR Index
 */
#define XPLMI_SSIT_MASTER_SLR_INDEX		(0U)
#define XPLMI_SSIT_SLAVE0_SLR_INDEX		(1U)
#define XPLMI_SSIT_SLAVE1_SLR_INDEX		(2U)
#define XPLMI_SSIT_SLAVE2_SLR_INDEX		(3U)
#define XPLMI_SSIT_INVALID_SLR_INDEX	(4U)

/**************************** Type Definitions *******************************/
typedef struct XPlmi_Cmd XPlmi_Cmd;
typedef struct XPlmi_KeyHoleParams XPlmi_KeyHoleParams;


typedef struct {
	u32 OffsetList[XPLMI_BEGIN_OFFSET_STACK_SIZE];
	int OffsetListTop;
} XPlmi_CdoParamsStack;

struct XPlmi_KeyHoleParams {
	u64 SrcAddr; /**< Boot Source address */
	u32 ExtraWords; /**< Words that are directly DMAed to CFI */
	int (*Func) (u64 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);
	u8 IsNextChunkCopyStarted; /**< Used to check if next chunk is copied or not */
};

struct XPlmi_Cmd {
	u32 SubsystemId;
	u32 BufIndex;
	u32 IpiMask;
	u32 CmdId;
	u32 Len;
	u32 ProcessedLen;
	u32 PayloadLen;
	u32 *Payload;
	u32 Response[XPLMI_CMD_RESP_SIZE];
	int (*ResumeHandler)(XPlmi_Cmd * CmdPtr);
	u32 ResumeData[XPLMI_CMD_RESUME_DATALEN];
	XPlmi_KeyHoleParams KeyHoleParams;
	XPlmi_CdoParamsStack CdoParamsStack;
	u32 IpiReqType;
	u32 BreakLength;
	u32 ProcessedCdoLen;
	u8 DeferredError;
	u8 AckInPLM;
};

/***************** Macros (Inline Functions) Definitions *********************/
/**
 * @brief	Inline function to determine the Cdo error type based on the
 * 			command Id
 *
 * @param	CmdId is the Command ID
 *
 * @return 	CDO Error Type in
 * 			 - 0x2xyz format for CDO Error
 * 			 - 0x3xyz format for User Module CDO Error
 *
 *****************************************************************************/
static inline u32 XPlmi_GetCdoErr(u32 CmdId) {
	u32 CdoErrType = (u32)XPLMI_ERR_CDO_CMD;

#if ( XPAR_MAX_USER_MODULES > 0U )
	u32 ModuleId = (CmdId & XPLMI_CMD_MODULE_ID_MASK) >> XPLMI_CMD_MODULE_ID_SHIFT;
		if ( ModuleId >= XPLMI_USER_MODULE_START_INDEX ) {
			CdoErrType = XPLMI_ERR_USER_MODULE_CDO_CMD;
		}
#endif
		return (CdoErrType + (CmdId & XPLMI_ERR_CDO_CMD_MASK));
}

/************************** Function Prototypes ******************************/
int XPlmi_CmdExecute(XPlmi_Cmd * CmdPtr);
int XPlmi_CmdResume(XPlmi_Cmd * CmdPtr);


/**
 * @}
 * @endcond
 */

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* end of XPLMI_CMD_H */
