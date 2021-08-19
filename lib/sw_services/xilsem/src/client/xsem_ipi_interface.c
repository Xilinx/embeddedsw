/******************************************************************************
* Copyright (c) 2020-2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
* @file xsem_ipi_interface.c
*
* @cond xsem_internal
* This file has XilSEM IPI Interface implementation
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date         Changes
* ----  ---  ----------   --------------------------------------------------
* 0.1   gm   08/28/2020   Initial Creation
* 0.2   gm   09/03/2020   Updation of XSem_IpiPlmRespMsg to support
*                         complete response message.
* 0.3   hv   03/11/2021   Doxygen changes
* 0.4   hb   03/15/2021   MISRA fixes and formatted code
* 0.5   rb   04/07/2021   Doxygen changes
* 0.6	hv   08/18/2021   Fix Doxygen warnings
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
/**
 * @cond xsem_internal
 * @{
 */
#include "xsem_ipi_interface.h"

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	Sends IPI request to the PLM
 *
 * @param[in]	IpiInst : Pointer to ipi instance
 * @param[in]	Payload : API ID and call arguments to be written in IPI buffer
 *
 * @return	XST_SUCCESS : If IPI request is successful
 * 		XST_FAILURE : If IPI request is failed
 *****************************************************************************/
XStatus XSem_IpiSendReqPlm(XIpiPsu *IpiInst,
			u32 Payload[PAYLOAD_ARG_CNT])
{
	XStatus Status = XST_FAILURE;

	if (NULL == IpiInst) {
		XSem_Dbg("[%s] ERROR: NULL input\n\r", __func__);
		goto END;
	}

	Status = XIpiPsu_PollForAck(IpiInst, TARGET_IPI_INT_MASK,
				    SEM_IPI_TIMEOUT);
	if (XST_SUCCESS != Status) {
		XSem_Dbg("[%s] ERROR: Timeout expired\n\r", __func__);
		goto END;
	}

	Status = XIpiPsu_WriteMessage(IpiInst, TARGET_IPI_INT_MASK,
				      Payload, PAYLOAD_ARG_CNT,
				      XIPIPSU_BUF_TYPE_MSG);
	if (XST_SUCCESS != Status) {
		XSem_Dbg("[%s] ERROR: writing to IPI request buffer\n",
				__func__);
		goto END;
	}

	(void)XIpiPsu_TriggerIpi(IpiInst, TARGET_IPI_INT_MASK);
	XSem_Dbg("[%s] SUCCESS: 0x%x\n\r", __func__, Status);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	Reads IPI response after PLM has handled interrupt
 *
 * @param[in]	IpiInst : IPI instance pointer
 * @param[out]	RespMsg : Structure Pointer with IPI response
 *
 * @return	XST_SUCCESS : On reading IPI reasponse successfully
 * 		XST_FAILURE : On failing to read IPI response
 *****************************************************************************/
XStatus XSem_IpiPlmRespMsg(XIpiPsu *IpiInst,
				u32 RespMsg[RESPONSE_ARG_CNT])
{
	XStatus Status = XST_FAILURE;

	if (NULL == IpiInst) {
		XSem_Dbg("[%s] ERROR: NULL input\n\r", __func__);
		goto END;
	}

	/* Wait until current IPI interrupt is handled by PLM */
	Status = XIpiPsu_PollForAck(IpiInst, TARGET_IPI_INT_MASK,
				  SEM_IPI_TIMEOUT);

	if (Status != XST_SUCCESS) {
		XSem_Dbg("[%s] ERROR: Timeout expired\n\r", __func__);
		goto END;
	}

	Status = XIpiPsu_ReadMessage(IpiInst, TARGET_IPI_INT_MASK,
					 RespMsg, RESPONSE_ARG_CNT,
					 XIPIPSU_BUF_TYPE_RESP);

	if (Status != XST_SUCCESS) {
		XSem_Dbg("[%s] ERROR: reading from IPI response buffer\n\r",
				__func__);
		goto END;
	}

	Status = (XStatus)RespMsg[0];

	XSem_Dbg("[%s] SUCCESS: 0x%x\n\r", __func__, Status);

END:
	return Status;
}
/**
 * @}
 * @endcond
 */
