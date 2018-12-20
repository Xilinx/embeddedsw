/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
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
 * @file xpmcfw_handoff.c
 *
 * This is the main file which contains handoff code for the PMCFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  kc   02/21/2017 Initial release
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xpmcfw_main.h"
#include "xpmcfw_fabric.h"
#include "xilcdo_npi.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static XStatus XPmcFw_HandoffCpus(XPmcFw * PmcFwInstancePtr);

/************************** Variable Definitions *****************************/
extern u32 NpiFabricEnabled;
/*****************************************************************************/
/**
 * This function starts the whole system
 *
 * @param	PmcFwInstancePtr is pointer to the XPmcFw Instance
 *
 * @return	returns the error codes described in xpmcfw_error.h on any error
 *
 * @note	This function should not return incase of success
 *****************************************************************************/

XStatus XPmcFw_Handoff (XPmcFw * PmcFwInstancePtr)
{
	XStatus Status;

	XPmcFw_Printf(DEBUG_DETAILED, "In %s\n\r", __func__);

	Status = PmcFwInstancePtr->DeviceOps.Release();
	if (XPMCFW_SUCCESS != Status) {
	goto END;
    }

	if ((PmcFwInstancePtr->PlCfiPresent == TRUE) || (NpiFabricEnabled == 1U))
	{
		XilCdo_SetGlobalSignals();
	}

	XilCdo_RunPendingNpiSeq();
	/* Hook before handoff */
	Status = XPmcFw_HookBeforeHandoff();
	if (XPMCFW_SUCCESS != Status) {
		goto END;
	}

	/* PL Global Sequence END if only cfi or npi bit stream is loaded */
	if ((PmcFwInstancePtr->PlCfiPresent == TRUE) || (NpiFabricEnabled == 1U))
	{
		XilCdo_AssertGlobalSignals();
#if 1
	/* Enable Readback */
	XPmcFw_ReadFabricData((u32 *)XPMCFW_PMCRAM_BASEADDR, 100*4);
#endif
	}

	/* Handoff to the CPUs */
	Status = XPmcFw_HandoffCpus(PmcFwInstancePtr);
	if (XPMCFW_SUCCESS != Status) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function handoff the images to the respective cpu's
 *
 * @param	PmcFwInstancePtr is pointer to the XPmcFw Instance
 *
 * @return	returns the error codes described in xpmcfw_error.h on any error
 *****************************************************************************/
static XStatus XPmcFw_HandoffCpus(XPmcFw * PmcFwInstancePtr)
{
	XStatus Status;
	u32 Index;
	u32 CpuId;
	u32 ExecState;
	u64 HandoffAddr;
	u32 VInitHi;
	XCrx Crx={0};

	/* Handoff to the cpus */
	for (Index=0U;Index<PmcFwInstancePtr->NoOfHandoffCpus;Index++)
	{
		CpuId = PmcFwInstancePtr->HandoffParam[Index].CpuSettings
			& XIH_PH_ATTRB_DSTN_CPU_MASK;
		ExecState = PmcFwInstancePtr->HandoffParam[Index].CpuSettings &
						XIH_PH_ATTRB_A72_EXEC_ST_MASK;
		HandoffAddr = PmcFwInstancePtr->HandoffParam[Index].HandoffAddr;
		VInitHi = PmcFwInstancePtr->HandoffParam[Index].CpuSettings &
						XIH_PH_ATTRB_HIVEC_MASK;

		switch (CpuId)
		{
			case XIH_PH_ATTRB_DSTN_CPU_A72_0:
			case XIH_PH_ATTRB_DSTN_CPU_A72_1:
			{
				// TODO PSM IPI communication is required here
				Crx.RvbarAddr = HandoffAddr;
				if (ExecState == XIH_PH_ATTRB_A72_EXEC_ST_AA64)
				{
					Crx.AA64nAA32 = XCRX_APU_AA64;
				} else {
					Crx.AA64nAA32 = XCRX_APU_AA32;
					/* TODO set VINITHI setting here */
				}
				if (CpuId == XIH_PH_ATTRB_DSTN_CPU_A72_0){
					Crx.CpuNo = XCRX_APU_CPU_0;
					XCrx_RelApu0(&Crx);
				} else {
					Crx.CpuNo = XCRX_APU_CPU_1;
					XCrx_RelApu1(&Crx);
				}
			}break;
			case XIH_PH_ATTRB_DSTN_CPU_R5_0:
			case XIH_PH_ATTRB_DSTN_CPU_R5_1:
			case XIH_PH_ATTRB_DSTN_CPU_R5_L:
			{
				// TODO PSM IPI communication is required here
				/* Vector setting */
				if (VInitHi == XIH_PH_ATTRB_HIVEC_MASK)
				{
					Crx.VInitHi = XCRX_CPU_VINITHI_HIVEC;
				} else {
					Crx.VInitHi = XCRX_CPU_VINITHI_LOVEC;
				}

				/* Update CPU No */
				if (CpuId == XIH_PH_ATTRB_DSTN_CPU_R5_L)
				{
					Crx.CpuNo = XCRX_RPU_CPU_L;
				} else if (CpuId == XIH_PH_ATTRB_DSTN_CPU_R5_0) {
					Crx.CpuNo = XCRX_RPU_CPU_0;
				} else {
					Crx.CpuNo = XCRX_RPU_CPU_1;
				}
				Crx.Halt = XCRX_CPU_NO_HALT;
				XCrx_RelRpu(&Crx);
			}break;
			case XIH_PH_ATTRB_DSTN_CPU_PSM:
			{
				XCrx_WakeUpPsm();
			}break;
			default:
			{
			}break;
		}

	}

	Status = XPMCFW_SUCCESS;
	return Status;
}
