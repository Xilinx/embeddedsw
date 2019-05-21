/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplmi_ipi.c
 *
 * This is the file which contains ipi manager code.
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
/***************************** Include Files *********************************/
#include "xplmi_ipi.h"
#include "xplmi_proc.h"
#ifdef XPAR_XIPIPSU_0_DEVICE_ID
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/* Instance of IPI Driver */
static XIpiPsu IpiInst;
static XIpiPsu *IpiInstPtr = &IpiInst;
u32 IpiMaskList[XPLMI_IPI_MASK_COUNT] = {0U};

/*****************************************************************************/
/**
 * @brief This function initializes the IPI
 *
 * @param	void
 *
 * @return	Status	IPI initialization status
 *
 *****************************************************************************/
int XPlmi_IpiInit(void)
{
	int Status;
	XIpiPsu_Config *IpiCfgPtr;
	u32 i;

	/* Load Config for Processor IPI Channel */
	IpiCfgPtr = XIpiPsu_LookupConfig(XPAR_XIPIPSU_0_DEVICE_ID);
	if (IpiCfgPtr == NULL) {
		Status = XST_FAILURE;
		goto END;
	}

	/* Init Mask Lists */
	for (i = 0U; i < XPLMI_IPI_MASK_COUNT; i++) {
		IpiMaskList[i] = IpiCfgPtr->TargetList[i].Mask;
	}

	/* Initialize the Instance pointer */
	Status = XIpiPsu_CfgInitialize(IpiInstPtr, IpiCfgPtr,
			IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		goto END;
	}

	/* Enable IPI from all Masters */
	for (i = 0U; i < XPLMI_IPI_MASK_COUNT; i++) {
		XIpiPsu_InterruptEnable(IpiInstPtr,
				IpiCfgPtr->TargetList[i].Mask);
	}

	/**
	 * Enable the IPI IRQ
	 */
	XPlmi_PlmIntrEnable(XPLMI_IPI_IRQ);
END:
	XPlmi_Printf(DEBUG_DETAILED,
		    "%s: IPI init status: 0x%x\n\r", __func__, Status);
	return Status;
}

/*****************************************************************************/
/**
 * @brief This is the handler for IPI interrupts
 *
 * @param	void
 *
 * @return	Status	Status of received IPI processing
 *
 *****************************************************************************/
int XPlmi_IpiDispatchHandler(void *Data)
{
	int Status = XST_FAILURE;
	u32 SrcCpuMask;
	u32 Payload[XPLMI_IPI_MAX_MSG_LEN];
	u32 MaskIndex;
	XPlmi_Cmd Cmd;

	/* For MISRA C */
	(void )Data;

	SrcCpuMask = Xil_In32(IPI_PMC_ISR);

	for (MaskIndex = 0; MaskIndex < XPLMI_IPI_MASK_COUNT; MaskIndex++) {
		if ((SrcCpuMask & IpiMaskList[MaskIndex]) != 0U) {
			Status = XPlmi_IpiRead(IpiMaskList[MaskIndex], &Payload[0], XPLMI_IPI_MAX_MSG_LEN, XIPIPSU_BUF_TYPE_MSG);
			Cmd.CmdId = Payload[0U];
			Cmd.IpiMask = IpiMaskList[MaskIndex];
			Cmd.Len = (Cmd.CmdId >> 16) & 255;
			if (Cmd.Len > 6U) {
				Cmd.Len = Payload[1U];
				Cmd.Payload = (u32 *)Payload[2U];
			} else {
				Cmd.Payload = &Payload[1U];
			}
			Status = XPlmi_CmdExecute(&Cmd);
			Cmd.Response[0] = Status;

			/* Send response to caller */
			XPlmi_IpiWrite(Cmd.IpiMask, Cmd.Response, XPLMI_CMD_RESP_SIZE, XIPIPSU_BUF_TYPE_RESP);
		}
	}

	XPlmi_Printf(DEBUG_DETAILED, "%s: IPI processed.\n\r", __func__);

	if (XST_SUCCESS != Status) {
		XPlmi_Printf(DEBUG_GENERAL, "%s: Error: Unhandled IPI received\n\r", __func__);
	}

	Xil_Out32(IPI_PMC_ISR, SrcCpuMask);

	return Status;
}

/*****************************************************************************/
/**
 * @brief This function writes the IPI message or response to destination CPU
 *
 * @param	DestCpuMask Destination CPU IPI mask
 * 			MsgPtr		Message to be written
 * 			MsgLen		IPI message length
 * 			Type		IPI buffer type
 *
 * @return	Status		IPI message write status
 *
 *****************************************************************************/
int XPlmi_IpiWrite(u32 DestCpuMask, u32 *MsgPtr, u32 MsgLen, u32 Type)
{
	int Status;

	if ((NULL == MsgPtr) ||
			((MsgLen <= 0) || (MsgLen > XPLMI_IPI_MAX_MSG_LEN)) ||
			((XIPIPSU_BUF_TYPE_MSG != Type) && (XIPIPSU_BUF_TYPE_RESP != Type))) {
		Status = XST_FAILURE;
	} else {

		Status = XIpiPsu_WriteMessage(IpiInstPtr, DestCpuMask, MsgPtr, MsgLen,
				Type);
	}

	XPlmi_Printf(DEBUG_DETAILED, "%s: IPI write status: 0x%x\r\n", __func__, Status);

	return Status;
}

/*****************************************************************************/
/**
 * @brief This function read the IPI message or response from source CPU
 *
 * @param	SrcCpuMask	Source CPU IPI mask
 * 			MsgPtr 		IPI read message buffer
 * 			MsgLen		IPI message length
 * 			Type		IPI buffer type
 *
 * @return	Status		IPI message read status
 *
 *****************************************************************************/
int XPlmi_IpiRead(u32 SrcCpuMask, u32 *MsgPtr, u32 MsgLen, u32 Type)
{
	int Status;

	if ((NULL == MsgPtr) ||
			((MsgLen <= 0) || (MsgLen > XPLMI_IPI_MAX_MSG_LEN)) ||
			((XIPIPSU_BUF_TYPE_MSG != Type) && (XIPIPSU_BUF_TYPE_RESP != Type))) {
		Status = XST_FAILURE;
	} else {
		/* Read Entire Message to Buffer */
		Status = XIpiPsu_ReadMessage(IpiInstPtr, SrcCpuMask, MsgPtr, MsgLen,
				Type);
	}
	XPlmi_Printf(DEBUG_DETAILED, "%s: IPI read status: 0x%x\r\n", __func__, Status);

	return Status;
}

/*****************************************************************************/
/**
 * @brief This function triggers the IPI interrupt to destination CPU
 *
 * @param	DestCpuMask Destination CPU IPI mask
 *
 * @return	int		Status of IPI message trigger
 *
 *****************************************************************************/
inline int XPlmi_IpiTrigger(u32 DestCpuMask)
{
	return XIpiPsu_TriggerIpi(IpiInstPtr, DestCpuMask);
}

/*****************************************************************************/
/**
 * @brief This function polls for IPI acknowledgment from destination CPU
 *
 * @param	DestCpuMask Destination CPU IPI mask
 * 			TimeOutCount Timeout value
 *
 * @return	int		Status of IPI poll for acknowledgment
 *
 *****************************************************************************/
inline int XPlmi_IpiPollForAck(u32 DestCpuMask, u32 TimeOutCount)
{
	return XIpiPsu_PollForAck(IpiInstPtr, DestCpuMask, TimeOutCount);
}
#endif /* XPAR_XIPIPSU_0_DEVICE_ID */
