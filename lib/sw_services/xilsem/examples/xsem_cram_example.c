/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file xsem_cram_example.c
 *
 * This file has XilSEM CRAM error injection example
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----  ----  ----------  ---------------------------------------------------
 * 0.1   gm    03/16/2021  Initial Creation
 * </pre>
 *
 *****************************************************************************/
#include "xil_printf.h"
#include "sleep.h"
#include "xsem_client_api.h"

/****************** Macros (Inline Functions) Definitions ********************/
#define TARGET_IPI_INT_DEVICE_ID	(XPAR_XIPIPSU_0_DEVICE_ID)
#define DataMaskShift(Data, Mask, Shift)	(((Data) & (Mask)) >> (Shift))
#define CFR_INIT_STATUS			(0x10005U)
#define CFR_CORECC_DETECTED_STATUS	(0x1U)
#define CFR_CORECC_DETECTED_MASK	(0x800U)
#define CFR_CORECC_DETECTED_SHIFT	(11U)
#define NO_MASK 			(0xFFFFFFFFU)
#define NO_SHIFT 			(0x0U)
#define TIMEOUT 			(1000000U)

/************************** Function Prototypes ******************************/
static XStatus XSem_CmpCfrStatus(u32 ExpectedStatus, u32 Mask, u32 Shift);
static XStatus XSem_ChkCfrStatus(u32 ExpectedVal, u32 TimeOut, u32 Mask, \
		u32 Shift);

/*****************************************************************************/
/**
*
* @brief	Main function to call the XilSEM Cram example.
*
* @return	XST_SUCCESS
*
* @note		This example will work for immediate start-up configuration.
*
******************************************************************************/
int main()
{
	XStatus Status = XST_FAILURE;
	XIpiPsu_Config *IpiCfgPtr = NULL;
	XIpiPsu IpiInst = {0};
	XSemIpiResp IpiResp = {0};
	XSemCfrErrInjData ErrData = {0x7, 0x0, 0x0, 0x0};
	XSemCfrStatus CfrStatusInfo = {0};
	u32 Index = 0U;

	/*
	 * If Immediate Start-up configured
	 * - Need to wait for CRAM Init to complete
	 * If deferred start-up configured
	 * - Perform initialization using Init IPI command.
	 */

	/* Need to wait for CRAM Init to complete */
	Status = XSem_ChkCfrStatus(CFR_INIT_STATUS, TIMEOUT, NO_MASK, NO_SHIFT);
	if( Status == XST_SUCCESS) {
		xil_printf("Cram Init successful\n");
	} else {
		xil_printf("Cram Init failed\n");
		Status = XSem_CmdCfrGetStatus(&CfrStatusInfo);
		if ( XST_SUCCESS == Status) {
			xil_printf("Cram Status [0x%08x]\n\r", \
					CfrStatusInfo.Status);
		}
		goto END;
	}

	/* Load Config for Processor IPI Channel */
	IpiCfgPtr = XIpiPsu_LookupConfig(TARGET_IPI_INT_DEVICE_ID);
	if (NULL == IpiCfgPtr) {
		xil_printf("[%s] ERROR: IPI LookupConfig failed\n\r", \
				__func__, Status);
		goto END;
	}

	/* Initialize the Ipi Instance pointer */
	Status = XIpiPsu_CfgInitialize(&IpiInst, IpiCfgPtr,
			IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		xil_printf("[%s] ERROR: IPI instance initialize failed\n\r", \
				__func__, Status);
		goto END;
	}

	/* For Error injection follow below sequence
	 * - Stop Scan
	 * - Inject Error
	 * - Start Scan again to detect/correct injected error
	 */

	/* Stop Scan */
	Status = XSem_CmdCfrStopScan(&IpiInst, &IpiResp);
	if ((XST_SUCCESS == Status) && \
			(CMD_ACK_CFR_STOP_SCAN == IpiResp.RespMsg1) && \
			(XST_SUCCESS == IpiResp.RespMsg2)) {
		xil_printf("[%s] Success: Stop\n\r", __func__);
	} else {
		xil_printf("[%s] Error: Stop Status 0x%x Ack 0x%x, Ret 0x%x", \
				__func__, Status, IpiResp.RespMsg1,
				IpiResp.RespMsg2);
		goto END;
	}

	/* Inject 1-bit correctable error */
	Status = XSem_CmdCfrNjctErr(&IpiInst, &ErrData, &IpiResp);
	if ((XST_SUCCESS == Status) && \
			(CMD_ACK_CFR_NJCT_ERR == IpiResp.RespMsg1) && \
			(XST_SUCCESS == IpiResp.RespMsg2)) {
		xil_printf("[%s] Success: Inject\n\r", __func__);
	} else {
		xil_printf("[%s] Error: Inject Status 0x%x Ack 0x%x, Ret 0x%x",\
				__func__, Status, IpiResp.RespMsg1,
				IpiResp.RespMsg2);
		goto END;
	}

	/* Start Scan */
	Status = XSem_CmdCfrStartScan(&IpiInst, &IpiResp);
	if ((XST_SUCCESS == Status) && \
			(CMD_ACK_CFR_START_SCAN == IpiResp.RespMsg1) && \
			(XST_SUCCESS == IpiResp.RespMsg2)) {
		xil_printf("[%s] Success: Start\n\r", __func__);
	} else {
		xil_printf("[%s] Error: Start Status 0x%x Ack 0x%x, Ret 0x%x", \
				__func__, Status, IpiResp.RespMsg1,
				IpiResp.RespMsg2);
		goto END;
	}

	/*Wait for XilSEM to detect correctable error*/
	Status = XSem_ChkCfrStatus(CFR_CORECC_DETECTED_STATUS, TIMEOUT,\
			CFR_CORECC_DETECTED_MASK, CFR_CORECC_DETECTED_SHIFT);
	if ( Status == XST_SUCCESS) {
		Status = XSem_CmdCfrGetStatus(&CfrStatusInfo);
		if (XST_SUCCESS == Status) {
			xil_printf("Cram Status [0x%08x]\n\r", \
					CfrStatusInfo.Status);
			xil_printf("Cor Ecc Cnt [0x%08x]\n\r", \
					CfrStatusInfo.ErrCorCnt);
			xil_printf("Error address details are as\n\r");
			for (Index = 0U ; Index < MAX_CRAMERR_REGISTER_CNT; Index++) {
				xil_printf("\nErrAddrL%d [0x%08x]\n\r", \
						Index,
						CfrStatusInfo.ErrAddrL[Index]);
				xil_printf("ErrAddrH%d [0x%08x]\n\n\r", \
						Index,
						CfrStatusInfo.ErrAddrH[Index]);
			}
		} else {
			xil_printf("XSem_ChkCfrStatus failed with err 0x%x", \
					Status);
		}
	} else {
		xil_printf("XSem_ChkCfrStatus failed with errcode 0x%x", \
				Status);
	}
END:
	return 0;
}

/*****************************************************************************/
/**
 *
 * @brief    	Function to compare cram status.
 *
 * @param    	ExpectedVal : Expected value of cram status
 * @param    	Mask : Mask value
 * @param    	Shift : Shift value
 *
 * @return    	XST_SUCCESS : Success
 *     		XST_FAILURE : Failed
 *
 ******************************************************************************/
static XStatus XSem_CmpCfrStatus(u32 ExpectedStatus, u32 Mask, u32 Shift)
{
	XSemCfrStatus CfrStatusInfo = {0};
	XStatus Status = XST_FAILURE;
	u32 Value = 0U;

	Status = XSem_CmdCfrGetStatus(&CfrStatusInfo);
	if (XST_SUCCESS == Status) {
		Value = DataMaskShift(CfrStatusInfo.Status, Mask, Shift);
		if (Value != ExpectedStatus) {
			Status = XST_FAILURE;
		}
	}
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief    	Function to wait for cram status to update till timeout
 *     		passed as argument.
 *
 * @param    	ExpectedVal : Expected value of cram status
 * @param    	TimeOut : Timeout in usec
 * @param    	Mask : Mask value
 * @param    	Shift : Shift value
 *
 * @return    	XST_SUCCESS : Success
 *     		XST_FAILURE : Failed
 *
 ******************************************************************************/
static XStatus XSem_ChkCfrStatus(u32 ExpectedVal, u32 TimeOut, u32 Mask, \
		u32 Shift)
{
	XStatus Status = XST_FAILURE;
	u32 TimeLapsed = 0U;

	/*
	 * Read and compare trace register value
	 */
	Status = XSem_CmpCfrStatus(ExpectedVal, Mask, Shift);

	/*
	 * Loop while the Mask is not set or we timeout
	 */
	while (((XST_SUCCESS != Status) && (TimeLapsed < TimeOut))) {
		usleep(1U);
		/*
		 * * Read and compare trace register value again
		 */
		Status = XSem_CmpCfrStatus(ExpectedVal, Mask, Shift);
		/*
		 * Decrement the TimeOut Count
		 */
		TimeLapsed++;
	}

	/* Sleep for 50 msec so that R5 and XilSEM logs will not mix up */
	if (XST_SUCCESS == Status) {
		usleep(50000);
	}

	return Status;
}
