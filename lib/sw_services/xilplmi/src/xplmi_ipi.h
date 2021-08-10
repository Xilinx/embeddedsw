/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
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
* 1.00  ma   10/09/2018 Initial release
* 1.01  kc   04/09/2019 Added code to register/enable/disable interrupts
*       kc   05/21/2019 Updated IPI error code to response buffer
*       ma   08/01/2019 Added LPD init code
*       rv   02/04/2020 Set the 1st element of response array always to status
*       bsv  02/13/2020 XilPlmi generic commands should not be supported
*                       via IPI
*       ma   02/21/2020 Added code to allow event logging command via IPI
*       ma   02/28/2020 Added code to disallow EM commands over IPI
*       bsv  03/09/2020 Added code to support CDO features command
*       ma   03/19/2020 Added features command for EM module
*       bsv  04/04/2020 Code clean up
* 1.02  bsv  06/02/2020 Added code to support GET BOARD command and disallow
*                       SET BOARD command via IPI
*       bm   10/14/2020 Code clean up
* 1.03  ma   03/04/2021 Added IPI secure related defines
*       bsv  04/16/2021 Added provision to store Subsystem Id in XilPlmi
*       ma   08/09/2021 Added IPI_PMC_IMR register define
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
#define XPLMI_MAX_IPI_CMD_LEN		(6U)

/* IPI defines */
#define IPI_BASEADDR				(0xFF300000U)
#define IPI_PMC_ISR					(IPI_BASEADDR + 0x20010U)
#define IPI_PMC_IMR					(IPI_BASEADDR + 0x20014U)
#define IPI_PMC_ISR_PSM_BIT_MASK	(0x1U)
#define IPI_PMC_ISR_IPI5_BIT_MASK	(0x80U)
#define IPI_NO_BUF_CHANNEL_INDEX	(0xFFFFU)

/* Command header secure bit defines */
#define IPI_CMD_HDR_SECURE_BIT_MASK		(0x1000000U)

/* IPI Aperture TZ register base address */
#define IPI_APER_TZ_000_ADDR			(0xFF3000BCU)
#define IPI_APER_TZ_PMC_REQ_BUF_MASK	(0x4U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
typedef u32 (*XPlmi_SubsystemHandler)(u32 IpiMask);

/************************** Function Prototypes ******************************/
int XPlmi_IpiInit(XPlmi_SubsystemHandler SubsystemHandler);
int XPlmi_IpiDispatchHandler(void *Data);
int XPlmi_IpiWrite(u32 DestCpuMask, u32 *MsgPtr, u32 MsgLen, u8 Type);
int XPlmi_IpiRead(u32 SrcCpuMask, u32 *MsgPtr, u32 MsgLen, u8 Type);
int XPlmi_IpiTrigger(u32 DestCpuMask);
int XPlmi_IpiPollForAck(u32 DestCpuMask, u32 TimeOutCount);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
#endif /* XPAR_XIPIPSU_0_DEVICE_ID */

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_IPI_H */
