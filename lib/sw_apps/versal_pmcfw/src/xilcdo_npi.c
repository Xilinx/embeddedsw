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
 * @file xilcdo_npi.c
 *
 * This is the file which contains CDO NPI functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  kc   03/20/2017 Initial release
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xilcdo_npi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
extern XilCdo_Prtn XilCdoPrtnInst;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

XILCDO_NPI_SEQ* XilCdoNpiSeqInstPtr = NULL;
XILCDO_NPI_SEQ XilCdoNpiSeqInstance;

/*****************************************************************************/
/**
 * This function writes PCSR control register
 *
 * @param	BaseAddr is base address of the block
 * @param	Mask is the bit field to be updated
 * @param   Value is the value to be updated
 * @return	none
 *
 *****************************************************************************/
void XilCdo_WritePcsrCtrlReg(u32 BaseAddr, u32 Mask, u32 Value)
{
	XilCdo_Printf(DEBUG_DETAILED,"Write Address: 0x%08x, Mask: 0x%08x,"
		"Value: 0x%08x\n\r", BaseAddr, Mask, Value);
	Xil_Out32(BaseAddr + XILCDO_NPI_PCSR_MASK_OFFSET, Mask);
	Xil_Out32(BaseAddr + XILCDO_NPI_PCSR_CONTROL_OFFSET, Value);

	return;
}

/*****************************************************************************/
/**
 * This function clears the gate register bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ClearGateReg(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_GATEREG_MASK, 0x0U);
}

/*****************************************************************************/
/**
 * This function sets the gate register bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_SetGateReg(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_GATEREG_MASK,
			PCSR_MASK_GATEREG_MASK);
}

/*****************************************************************************/
/**
 * This function sets the complete state bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_SetCompleteState(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_PCOMPLETE_MASK,
				PCSR_MASK_PCOMPLETE_MASK);
}

/*****************************************************************************/
/**
 * This function clears the complete state bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ClearCompleteState(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_PCOMPLETE_MASK, 0U);
}

/*****************************************************************************/
/**
 * This function sets the Deskew state bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_SetDeskew(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_DESKEW_MASK,
                            PCSR_MASK_DESKEW_MASK);
}

/*****************************************************************************/
/**
 * This function sets the APB enable bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_SetApbEnable(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_APBEN_MASK,
				PCSR_MASK_APBEN_MASK);
}

/*****************************************************************************/
/**
 * This function clears the APB enable bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ClearApbEnable(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_APBEN_MASK, 0x0U);
}

/*****************************************************************************/
/**
 * This function clears the hold state bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ClearHoldState(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_HOLDSTATE_MASK, 0x0U);
}

/*****************************************************************************/
/**
 * This function sets the hold state bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_SetHoldState(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_HOLDSTATE_MASK,
			PCSR_MASK_HOLDSTATE_MASK);
}

/*****************************************************************************/
/**
 * This function clears the UBI INIT state bit for the block
 *
 * @param       BaseAddr is base address of the block
 * @param       NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return      none
 *
 *****************************************************************************/
void XilCdo_ClearUBInitState(u32 BaseAddr)
{
        XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_UB_INIT_MASK, 0x0U);
}

/*****************************************************************************/
/**
 * This function sets the UBI INIT state bit for the block
 *
 * @param       BaseAddr is base address of the block
 * @param       NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return      none
 *
 *****************************************************************************/
void XilCdo_SetUBInitState(u32 BaseAddr)
{
        XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_UB_INIT_MASK,
				PCSR_MASK_UB_INIT_MASK);
}

/*****************************************************************************/
/**
 * This function clears DCI OFC reset bit for the block
 *
 * @param       BaseAddr is base address of the block
 * @param       NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return      none
 *
 *****************************************************************************/
void XilCdo_ClearDCIOfcReset(u32 BaseAddr)
{
        XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_DCI_OFC_RST_MASK, 0x0U);
}

/*****************************************************************************/
/**
 * This function sets DCI OFC reset bit for the block
 *
 * @param       BaseAddr is base address of the block
 * @param       NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return      none
 *
 *****************************************************************************/
void XilCdo_SetDCIOfcReset(u32 BaseAddr)
{
        XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_DCI_OFC_RST_MASK,
				PCSR_MASK_DCI_OFC_RST_MASK);
}

/*****************************************************************************/
/**
 * This function clears the init state bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ClearInitState(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_INITSTATE_MASK, 0x0U);
}

/*****************************************************************************/
/**
 * This function sets the init state bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_SetInitState(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_INITSTATE_MASK,
			PCSR_MASK_INITSTATE_MASK);
}
/*****************************************************************************/
/**
 * This function clears the Deskew state bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ClearDeskew(u32 BaseAddr)
{
        XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_DESKEW_MASK, 0x0U);
}

/*****************************************************************************/
/**
 * This function sets the Fabric Enable state bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_SetFabricEnable(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_FABRICEN_MASK,
					PCSR_MASK_FABRICEN_MASK);
}

/*****************************************************************************/
/**
 * This function clears the Fabric Enable state bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ClearFabricEnable(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_FABRICEN_MASK, 0x0U);
}

/*****************************************************************************/
/**
 * This function clears the ODisable state bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ClearODisable(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_ODISABLE_MASK, 0x0U);
}

/*****************************************************************************/
/**
 * This function sets the ODisable state bit for the block
 *
 * @param       BaseAddr is base address of the block
 * @param       NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return      none
 *
 *****************************************************************************/
void XilCdo_SetODisable(u32 BaseAddr)
{
        XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_ODISABLE_MASK,
		PCSR_MASK_ODISABLE_MASK);
}

/*****************************************************************************/
/**
 * This function runs the calibration for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_RunCalibration(u32 BaseAddr, u32 Deskew)
{
	XilCdo_SetStartCal(BaseAddr);

	if(Deskew==TRUE)
	{
		XilCdo_RunDeskew(BaseAddr);
	}
	/* Wait for Calibration to complete */
	while((Xil_In32(BaseAddr+XILCDO_NPI_PCSR_STATUS_OFFSET)
				& PCSR_STATUS_CALDONE_MASK)
			!= PCSR_STATUS_CALDONE_MASK);
	XilCdo_ClearStartCal(BaseAddr);
}

/*****************************************************************************/
/**
 * This function checks if the calibration is complete or not
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	Status of calibration, XST_SUCCESS indicates Calibration is complete.
 *			Any other value indicates calibration is running.
 *
 *****************************************************************************/
XStatus XilCdo_CheckCalibration(u32 BaseAddr)
{
	/* Check if Calibration is complete */
	XStatus Status = ((Xil_In32(BaseAddr + XILCDO_NPI_PCSR_STATUS_OFFSET)
				& PCSR_STATUS_CALDONE_MASK)
			== PCSR_STATUS_CALDONE_MASK);

	if(Status == TRUE)
	{
		XilCdo_ClearStartCal(BaseAddr);
		Status = XST_SUCCESS;
	}
	else
	{
		Status = XST_FAILURE;
	}
	return Status;
}

/*****************************************************************************/
/**
 * This function runs deskew for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_RunDeskew(u32 BaseAddr)
{
	XilCdo_SetDeskew(BaseAddr);

	/* Wait for Calibration to complete */
	usleep(100U);
	XilCdo_ClearDeskew(BaseAddr);
}

/*****************************************************************************/
/**
 * This function sets the StartCal bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_SetStartCal(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_STARTCAL_MASK,
				 PCSR_MASK_STARTCAL_MASK);
}

/*****************************************************************************/
/**
 * This function clears the StartCal bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ClearStartCal(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_STARTCAL_MASK, 0x0U);
}

/*****************************************************************************/
/**
 * This function sets the ShutDown bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_SetShutDown(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_SHTDN_MASK,
				 PCSR_MASK_SHTDN_MASK);
}

/*****************************************************************************/
/**
 * This function clears the ShutDown bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ClearShutDown(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_SHTDN_MASK, 0x0U);
}

/*****************************************************************************/
/**
 * This function clears the TriState bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ClearTriState(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_TRISTATE_MASK, 0x0U);
}

/*****************************************************************************/
/**
 * This function sets the TriState bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_SetTriState(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_TRISTATE_MASK,
			PCSR_MASK_TRISTATE_MASK);
}

/*****************************************************************************/
/**
 * This function clears the lock state bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ClearLockState(u32 BaseAddr)
{
	Xil_Out32(BaseAddr + XILCDO_NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
}

/*****************************************************************************/
/**
 * This function sets the lock state bit for the block
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_SetLockState(u32 BaseAddr)
{
	Xil_Out32(BaseAddr + XILCDO_NPI_PCSR_LOCK_OFFSET, 0x0U);
}

/*****************************************************************************/
/**
 * This function sets the PR Mode bit for NOC NSU blocks
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_SetPRMode(u32 BaseAddr)
{
	u32 RegVal;
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_NSU_PR_MASK,
				PCSR_MASK_NSU_PR_MASK);
}

/*****************************************************************************/
/**
 * This function clears the PR Mode bit for NOC NSU blocks
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ClearPRMode(u32 BaseAddr)
{
	u32 RegVal;
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_NSU_PR_MASK, 0x0U);
}

/*****************************************************************************/
/**
 * This function sets the AXI_REQ_REJECT bit for NOC NMU blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_SetAxiReqRejectMode(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_AXI_REQ_REJECT_MASK,
				PCSR_MASK_AXI_REQ_REJECT_MASK);
}

/*****************************************************************************/
/**
 * This function clears the AXI_REQ_REJECT bit for NOC NMU blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ClearAxiReqRejectMode(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_AXI_REQ_REJECT_MASK, 0x0U);
}

/*****************************************************************************/
/**
 * This function sets the AXI_REQ_REJECT bit for NOC NMU blocks and waits on
 * busy status
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_RunAxiReqRejectMode(u32 BaseAddr)
{
	u32 RegVal;
	XilCdo_SetAxiReqRejectMode(BaseAddr);
	while(1)
	{
		usleep(5U);
		RegVal = Xil_In32(BaseAddr + XILCDO_NPI_NMU_BUSY_OFFSET);
		if(RegVal==0U)
		{
			break;
		}
	}
}

/*****************************************************************************/
/**
 * This function runs the PR Mode for NOC NSU blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_RunNsuPRMode(u32 BaseAddr)
{
	u32 RegVal;
	XilCdo_SetPRMode(BaseAddr);
	while(1)
	{
		usleep(5U);
		RegVal = Xil_In32(BaseAddr + XILCDO_NPI_PENDING_BURST_OFFSET);
		if(RegVal==0U)
		{
			break;
		}
	}
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for the block
 *
 * @param	CmdArgs is the pointer to argument data
 *
 * @return	returns the error codes described in xilcdo.h
 *
 *****************************************************************************/
XStatus XilCdo_NpiSeq(u32 CmdArgs[10U])
{
	u32 BaseAddr = CmdArgs[0U];
	u32 NpiParam = CmdArgs[1U];
	u32 BlockType;
	u32 DdrPresent;
	XStatus Status;

	CmdArgs[9U] = CMD_NPI_SEQ_ARGS;
	if(XilCdoNpiSeqInstPtr == NULL)
	{
	    /** Initialization of instance variable */
		XilCdoNpiSeqInstPtr = &XilCdoNpiSeqInstance;
		memset(XilCdoNpiSeqInstPtr->WaitQueue,0,
					4U * XILCDO_TOTAL_NPI_BLKS);
		XilCdoNpiSeqInstPtr->XPIO_BaseAddrCnt = 0U;
		XilCdoNpiSeqInstPtr->HSR_BUFGS_BaseAddrCnt = 0U;
        XilCdoNpiSeqInstPtr->VNOC_PS_BaseAddrCnt = 0U;
        XilCdoNpiSeqInstPtr->VNOC_BaseAddrCnt = 0U;
        XilCdoNpiSeqInstPtr->CLK_GT_BaseAddrCnt = 0U;
	}
	BlockType = NpiParam & XILCDO_NPI_BLK_MASK;
	DdrPresent =  NpiParam & XILCDO_NPI_DDRMC_PRESENT_MASK;

	if(BlockType != XILCDO_NPI_BLK_VREF)
	{
		Status = XilCdo_RunPendingNpiSeq(BlockType);
		if(Status != XST_SUCCESS)
		{
			goto END;
		}
	}

	switch(BlockType)
	{
		case XILCDO_NPI_BLK_VREF:
		{
			XilCdo_ProcBlkVREF(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_XPIO:
		{
			XilCdoNpiSeqInstPtr->XPIO_BaseAddr
			[XilCdoNpiSeqInstPtr->XPIO_BaseAddrCnt++] = BaseAddr;
			XilCdo_PreProcBlkXPIO(BaseAddr);
			XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_XPIO]=1U;
		}
		break;
		case XILCDO_NPI_BLK_XPIO_IOMISC:
		{
			XilCdo_ProcBlkXPIO_IOMISC(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_VERT_TO_HSR:
		case XILCDO_NPI_BLK_VERT_TO_HSR_GT:
		case XILCDO_NPI_BLK_REBUF_VRT:
		case XILCDO_NPI_BLK_REBUF_VRT_GT:
		{
			XilCdo_ProcBlkHSR_VRT(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME:
		{
			XilCdo_ProcBlkHSR_TNOC_ME(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_HSR_BUFGS:
		{
			XilCdoNpiSeqInstPtr->HSR_BUFGS_BaseAddr
			[XilCdoNpiSeqInstPtr->HSR_BUFGS_BaseAddrCnt++] = BaseAddr;
			XilCdo_PreProcBlkHSR_BUFGS(BaseAddr);
			XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_HSR_BUFGS]
				= 1U;
		}
		break;
		case XILCDO_NPI_BLK_VNOC_PS:
		{
			XilCdoNpiSeqInstPtr->VNOC_PS_BaseAddr
			[XilCdoNpiSeqInstPtr->VNOC_PS_BaseAddrCnt++] = BaseAddr;
			XilCdo_PreProcBlkVNOC_PS(BaseAddr);
			XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_VNOC_PS] = 1U;
		}
		break;
		case XILCDO_NPI_BLK_VNOC:
		{
			XilCdoNpiSeqInstPtr->VNOC_BaseAddr
			[XilCdoNpiSeqInstPtr->VNOC_BaseAddrCnt++] = BaseAddr;
			XilCdo_PreProcBlkVNOC(BaseAddr);
			XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_VNOC]=1U;
		}
		break;
		case XILCDO_NPI_BLK_CLK_GT:
		{
			XilCdoNpiSeqInstPtr->CLK_GT_BaseAddr
			[XilCdoNpiSeqInstPtr->CLK_GT_BaseAddrCnt++] = BaseAddr;
			XilCdo_PreProcBlkCLK_GT(BaseAddr);
			XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_CLK_GT]=1;
		}
		break;
		case XILCDO_NPI_BLK_MMCM:
		case XILCDO_NPI_BLK_DPLL:
		{
			XilCdo_ProcBlkMMCM_DPLL(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_XPLL:
		{
			XilCdo_ProcBlkXPLL(BaseAddr, DdrPresent);
		}
		break;
		case XILCDO_NPI_BLK_XPHY:
		{
			XilCdo_ProcBlkXPHY(BaseAddr, DdrPresent);
		}
		break;
		case XILCDO_NPI_BLK_DDRMC:
		{
			XilCdo_ProcBlkDDRMC(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_AMS_SAT:
		{
			XilCdo_ProcBlkAMS_SAT(BaseAddr);
		}
		case XILCDO_NPI_BLK_AMS_ROOT:
		{
			XilCdo_ProcBlkAMS_ROOT(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_XPIPE:
		{
			XilCdo_ProcBlkXPIPE(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_GT:
		{
			XilCdo_ProcBlkGT(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_ME_NPI:
		{
			XilCdo_ProcBlkME_NPI(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_NOC_NPS:
		{
			XilCdo_ProcBlkNOC_NPS(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_NOC_IDB:
		{
			XilCdo_ProcBlkNOC_IDB(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_NOC_NCRB:
		{
			XilCdo_ProcBlkNOC_NCRB(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_NOC_NSU:
		{
			XilCdo_ProcBlkNOC_NSU(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_NOC_NMU:
		{
			XilCdo_ProcBlkNOC_NMU(BaseAddr);
		}
		break;
		default:
		{
			Status = XST_FAILURE;
			goto END;
		}
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function executes pending NPI sequences for the blocks processed before
 *
 * @param	BlockType of the recent most executed block
 *
 * @return	returns the error codes described in xilcdo.h
 *
 *****************************************************************************/
XStatus XilCdo_RunPendingNpiSeq(u32 BlockType)
{
	XStatus Status;
	u32 Index;

	switch(BlockType)
	{
		case XILCDO_NPI_BLK_VREF:
		case XILCDO_NPI_BLK_XPIO:
			break;
		case XILCDO_NPI_BLK_XPIO_IOMISC:
		case XILCDO_NPI_BLK_VERT_TO_HSR:
		case XILCDO_NPI_BLK_VERT_TO_HSR_GT:
		case XILCDO_NPI_BLK_REBUF_VRT:
		case XILCDO_NPI_BLK_REBUF_VRT_GT:
		case XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME:
		case XILCDO_NPI_BLK_HSR_BUFGS:
		case XILCDO_NPI_BLK_VNOC_PS:
		case XILCDO_NPI_BLK_VNOC:
		case XILCDO_NPI_BLK_CLK_GT:
		{
			/** Process pending addresses of XPIO block types*/
			for(Index = 0U; Index <
			    XilCdoNpiSeqInstPtr->XPIO_BaseAddrCnt;
							++Index)
			{
				XilCdo_PostProcBlkXPIO
				(XilCdoNpiSeqInstPtr->XPIO_BaseAddr[Index]);
			}
		}
			break;
		case XILCDO_NPI_BLK_MMCM:
		case XILCDO_NPI_BLK_DPLL:
		case XILCDO_NPI_BLK_XPLL:
		case XILCDO_NPI_BLK_XPHY:
		case XILCDO_NPI_BLK_DDRMC:
		case XILCDO_NPI_BLK_AMS_SAT:
		case XILCDO_NPI_BLK_AMS_ROOT:
		case XILCDO_NPI_BLK_XPIPE:
		case XILCDO_NPI_BLK_GT:
		case XILCDO_NPI_BLK_NOC_NPS:
		case XILCDO_NPI_BLK_NOC_IDB:
		case XILCDO_NPI_BLK_NOC_NCRB:
		case XILCDO_NPI_BLK_NOC_NSU:
		case XILCDO_NPI_BLK_NOC_NMU:
		case XILCDO_NPI_BLK_ME_NPI:
		{
			for(Index = 0U; Index <
			    XilCdoNpiSeqInstPtr->XPIO_BaseAddrCnt;
								++Index)
			{
				XilCdo_PostProcBlkXPIO
				(XilCdoNpiSeqInstPtr->XPIO_BaseAddr[Index]);
			}
			/** Processing of pending addresses for HSR_BUFGS,
			 *  VNOC_PS,VNOC and CLK_GT BLOCK TYPES */
			while(!((XilCdoNpiSeqInstPtr->HSR_BUFGS_BaseAddrCnt==0)
			&& (XilCdoNpiSeqInstPtr->VNOC_PS_BaseAddrCnt == 0U) 
			&& (XilCdoNpiSeqInstPtr->VNOC_BaseAddrCnt == 0U) 
			&& (XilCdoNpiSeqInstPtr->CLK_GT_BaseAddrCnt == 0U)))
			{
				if(((XilCdoNpiSeqInstPtr->WaitQueue
					[XILCDO_NPI_BLK_VNOC_PS] == 0U) ||
					(XilCdoNpiSeqInstPtr->WaitQueue
					[XILCDO_NPI_BLK_VNOC_PS] >= 3U)) &&
					((XilCdoNpiSeqInstPtr->WaitQueue
					[XILCDO_NPI_BLK_VNOC] == 0U) ||
					(XilCdoNpiSeqInstPtr->WaitQueue
					[XILCDO_NPI_BLK_VNOC] >= 3U)) &&
					((XilCdoNpiSeqInstPtr->WaitQueue
					[XILCDO_NPI_BLK_CLK_GT] == 0U) ||
					(XilCdoNpiSeqInstPtr->WaitQueue
					[XILCDO_NPI_BLK_CLK_GT] >= 3U)))
				{
					/** Processing of HSR_BUFGS is done 
					 *  only after VNOC_PS, VNOC and CLK_GT
					 *  blocks, if present are Deskewed.*/
					Index = 0U; 
					for(; Index < XilCdoNpiSeqInstPtr->
					HSR_BUFGS_BaseAddrCnt; ++Index)
					{
						XilCdo_PostProcBlkHSR_BUFGS
						(XilCdoNpiSeqInstPtr->
						HSR_BUFGS_BaseAddr[Index]);
					}
				}
				for(Index = 0U; Index < XilCdoNpiSeqInstPtr->
				VNOC_PS_BaseAddrCnt; ++Index)
				{
					/** Processing of pending addresses for
					 * VNOC_PS */
					XilCdo_PostProcBlkVNOC_PS
					(XilCdoNpiSeqInstPtr->
					VNOC_PS_BaseAddr[Index]);
				}
				for(Index = 0U; Index < XilCdoNpiSeqInstPtr->
				VNOC_BaseAddrCnt; ++Index)
				{
					/** Processing of pending addresses for
					 * VNOC */
					XilCdo_PostProcBlkVNOC
					(XilCdoNpiSeqInstPtr->
					VNOC_BaseAddr[Index]);
				}
				for(Index = 0U; Index < XilCdoNpiSeqInstPtr->
				CLK_GT_BaseAddrCnt; ++Index)
				{
					/** Processing of pending addresses for 
					 *CLK_GT */
					XilCdo_PostProcBlkCLK_GT
					(XilCdoNpiSeqInstPtr->
					CLK_GT_BaseAddr[Index]);
				}
			}
		}
		break;
		default:
		{
			Status = XST_FAILURE;
			goto END;
		}
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for NPI blocks
 *
 * @param	BaseAddr is base address of the block
 * @param	NpiParam is NPI parameters as mentioned in CDO document
 *
 * @return	returns the error codes described in xilcdo.h
 *
 *****************************************************************************/
XStatus XilCdo_NpiShutDown(u32 CmdArgs[10U])
{
	u32 BlockType;
	XStatus Status;
	u32 BaseAddr = CmdArgs[0U];
	u32 NpiParam = CmdArgs[1U];

	CmdArgs[9U] = CMD_NPI_SEQ_ARGS;
	BlockType = NpiParam & XILCDO_NPI_BLK_MASK;

	switch(BlockType)
	{
		case XILCDO_NPI_BLK_VREF:
		{
			XilCdo_ShutBlkVREF(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_XPIO:
		{
			XilCdo_ShutBlkXPIO(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_XPIO_IOMISC:
		{
			XilCdo_ShutBlkDCI(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_VERT_TO_HSR:
		case XILCDO_NPI_BLK_VERT_TO_HSR_GT:
		case XILCDO_NPI_BLK_REBUF_VRT:
		case XILCDO_NPI_BLK_REBUF_VRT_GT:
		{
			XilCdo_ShutBlkHSR_VRT(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME:
		{
			XilCdo_ShutBlkHSR_TNOC_ME(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_HSR_BUFGS:
		{
			XilCdo_ShutBlkHSR_BUFGS(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_VNOC_PS:
		{
			XilCdo_ShutBlkVNOC_PS(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_VNOC:
		{
			XilCdo_ShutBlkVNOC(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_CLK_GT:
		{
			XilCdo_ShutBlkCLK_GT(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_MMCM:
		case XILCDO_NPI_BLK_DPLL:
		{
			XilCdo_ShutBlkMMCM_DPLL(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_XPHY:
		{
			XilCdo_ShutBlkXPHY(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_DDRMC:
		{
			XilCdo_ShutBlkDDRMC(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_XPIPE:
		{
			XilCdo_ShutBlkXPIPE(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_GT:
		{
			XilCdo_ShutBlkGT(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_NOC_NSU:
		{
			XilCdo_ShutBlkNSU(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_NOC_NMU:
		{
			XilCdo_ShutBlkNMU(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_NOC_NCRB:
		{
			XilCdo_ShutBlkNCRB(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_XPLL:
		{
			XilCdo_ShutBlkXPLL(BaseAddr);
		}
		break;
		case XILCDO_NPI_BLK_ME_NPI:
		case XILCDO_NPI_BLK_NOC_NPS:
		case XILCDO_NPI_BLK_AMS_SAT:
		case XILCDO_NPI_BLK_AMS_ROOT:
		case XILCDO_NPI_BLK_NOC_IDB:
		break;
		default:
		{
			Status = XST_FAILURE;
			goto END;
		}
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for VREF blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkVREF(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for XPIO1 blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_PreProcBlkXPIO(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_ClearODisable(BaseAddr);
	if(BaseAddr == XilCdoNpiSeqInstPtr->XPIO_BaseAddr[0U])
	{
		XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_XPIO] = 1U;
	}
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for XPIO2 blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_PostProcBlkXPIO(u32 BaseAddr)
{
	XilCdo_ClearTriState(BaseAddr);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
	if(BaseAddr == XilCdoNpiSeqInstPtr->XPIO_BaseAddr
			[XilCdoNpiSeqInstPtr->XPIO_BaseAddrCnt - 1U])
	{
		XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_XPIO] = 0U;
		XilCdoNpiSeqInstPtr->XPIO_BaseAddrCnt = 0U;
	}
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for DCI blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkXPIO_IOMISC(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_ClearTriState(BaseAddr);
	XilCdo_ClearInitState(BaseAddr);
	XilCdo_ClearDCIOfcReset(BaseAddr);
	XilCdo_ClearHoldState(BaseAddr);
	XilCdo_RunCalibration(BaseAddr,0U);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for HSR/VRT blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkHSR_VRT(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_ClearODisable(BaseAddr);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for HSR_TNOC_ME blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkHSR_TNOC_ME(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_ClearODisable(BaseAddr);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does NPI startup sequence pre-processing for HSR_BUFGS blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_PreProcBlkHSR_BUFGS(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_ClearInitState(BaseAddr);
	XilCdo_ClearODisable(BaseAddr);
	XilCdo_SetStartCal(BaseAddr);
	XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_HSR_BUFGS] = 1U;
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for HSR_BUFGS blocks
 * This is called after PreProcessing function is called.
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_PostProcBlkHSR_BUFGS(u32 BaseAddr)
{
	u32 Status;
	u32 RegVal;
	if(XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_HSR_BUFGS] == 1U)
	{
		if(BaseAddr == XilCdoNpiSeqInstPtr->HSR_BUFGS_BaseAddr[0U])
		{
			RegVal = Xil_In32(CFU_APB_CFU_FGCR);
			if((RegVal & (1U<<CFU_APB_CFU_FGCR_GCLK_CAL_SHIFT))==0U)
			{
				RegVal = RegVal | (1U << 
				CFU_APB_CFU_FGCR_GCLK_CAL_SHIFT);
				Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
			}
		}
		Status = XilCdo_CheckCalibration(BaseAddr);
		if(Status == XST_SUCCESS)
		{
			if(BaseAddr == XilCdoNpiSeqInstPtr->HSR_BUFGS_BaseAddr
			[XilCdoNpiSeqInstPtr->HSR_BUFGS_BaseAddrCnt -1U])
			{
				XilCdoNpiSeqInstPtr->WaitQueue
				[XILCDO_NPI_BLK_HSR_BUFGS] = 2U;
			}
		}
	}
	else
	{
		if(BaseAddr == XilCdoNpiSeqInstPtr->HSR_BUFGS_BaseAddr[0U])
		{
			RegVal = Xil_In32(CFU_APB_CFU_FGCR);
			if((RegVal & (1U<<CFU_APB_CFU_FGCR_GCLK_CAL_SHIFT))==1U)
			{
				RegVal = RegVal & ~(1U << 
				CFU_APB_CFU_FGCR_GCLK_CAL_SHIFT);
				Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
			}
			Xil_Out32(CFU_APB_CFU_FGCR, RegVal | (1U <<
					CFU_APB_CFU_FGCR_GWE_SHIFT));
		}
		XilCdo_SetCompleteState(BaseAddr);
		XilCdo_ClearHoldState(BaseAddr);
		XilCdo_SetLockState(BaseAddr);
		if(BaseAddr == XilCdoNpiSeqInstPtr->HSR_BUFGS_BaseAddr
			[XilCdoNpiSeqInstPtr->HSR_BUFGS_BaseAddrCnt -1U])
		{
			XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_HSR_BUFGS]
									 = 0U;
			XilCdoNpiSeqInstPtr->HSR_BUFGS_BaseAddrCnt = 0U;
		}
	}
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for VNOC_PS blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_PreProcBlkVNOC_PS(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_ClearInitState(BaseAddr);
	XilCdo_ClearODisable(BaseAddr);
	XilCdo_SetStartCal(BaseAddr);
	XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_VNOC_PS] = 1U;
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for VNOC_PS blocks
 * This is called after PreProcessing function is called.
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_PostProcBlkVNOC_PS(u32 BaseAddr)
{
	u32 RegVal;

	if(XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_VNOC_PS] == 1U)
	{

		if(BaseAddr == XilCdoNpiSeqInstPtr->VNOC_PS_BaseAddr[0U])
		{
			RegVal = Xil_In32(CFU_APB_CFU_FGCR);
			if((RegVal & (1U<<CFU_APB_CFU_FGCR_GCLK_CAL_SHIFT))==0U)
			{
				RegVal = RegVal | (1U <<
				CFU_APB_CFU_FGCR_GCLK_CAL_SHIFT);
				Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
			}
		}
		XilCdo_SetDeskew(BaseAddr);
		if(BaseAddr == XilCdoNpiSeqInstPtr->VNOC_PS_BaseAddr
				[XilCdoNpiSeqInstPtr->VNOC_PS_BaseAddrCnt -1U])
		{
			XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_VNOC_PS]
									= 2U;
		}
	}
	else if(XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_VNOC_PS] == 2U)
	{
		if(BaseAddr == XilCdoNpiSeqInstPtr->VNOC_PS_BaseAddr
			[XilCdoNpiSeqInstPtr->VNOC_PS_BaseAddrCnt -1U])
		{
			XilCdoNpiSeqInstPtr->WaitQueue
			[XILCDO_NPI_BLK_VNOC_PS] = 3U;
		}
	}
	else if(XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_VNOC_PS] == 3U)
	{
		XilCdo_ClearDeskew(BaseAddr);
		XilCdo_SetCompleteState(BaseAddr);
		if(BaseAddr == XilCdoNpiSeqInstPtr->VNOC_PS_BaseAddr
			[XilCdoNpiSeqInstPtr->VNOC_PS_BaseAddrCnt -1U])
		{
			XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_VNOC_PS]
									= 4U;
		}
	}
	else
	{
		if(BaseAddr == XilCdoNpiSeqInstPtr->VNOC_PS_BaseAddr[0U])
		{
			RegVal = Xil_In32(CFU_APB_CFU_FGCR);
			if((RegVal & (1U<<CFU_APB_CFU_FGCR_GCLK_CAL_SHIFT))==1U)
			{
				RegVal = RegVal & ~(1U <<
				CFU_APB_CFU_FGCR_GCLK_CAL_SHIFT);
				Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
			}
			Xil_Out32(CFU_APB_CFU_FGCR, RegVal | (1U <<
				CFU_APB_CFU_FGCR_GWE_SHIFT));
		}
		XilCdo_ClearStartCal(BaseAddr);
		XilCdo_ClearHoldState(BaseAddr);
		XilCdo_SetLockState(BaseAddr);
		if(BaseAddr == XilCdoNpiSeqInstPtr->VNOC_PS_BaseAddr
			[XilCdoNpiSeqInstPtr->VNOC_PS_BaseAddrCnt -1U])
		{
			XilCdoNpiSeqInstPtr->WaitQueue
				[XILCDO_NPI_BLK_VNOC_PS] = 0U;
			XilCdoNpiSeqInstPtr->VNOC_PS_BaseAddrCnt = 0U;
		}
	}
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for VNOC blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_PreProcBlkVNOC(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_ClearInitState(BaseAddr);
	XilCdo_ClearODisable(BaseAddr);
	XilCdo_SetStartCal(BaseAddr);
	XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_VNOC] = 1U;
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for VNOC blocks
 * This is called after PreProcessing function is called.
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_PostProcBlkVNOC(u32 BaseAddr)
{
	u32 RegVal;

	if(XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_VNOC] == 1U)
	{
		if(BaseAddr == XilCdoNpiSeqInstPtr->VNOC_BaseAddr[0U])
		{
			RegVal = Xil_In32(CFU_APB_CFU_FGCR);
			if((RegVal & (1U<<CFU_APB_CFU_FGCR_GCLK_CAL_SHIFT))==0U)
			{
				RegVal = RegVal | (1U <<
				CFU_APB_CFU_FGCR_GCLK_CAL_SHIFT);
				Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
			}
		}
		XilCdo_SetDeskew(BaseAddr);
		if(BaseAddr == XilCdoNpiSeqInstPtr->VNOC_BaseAddr
			[XilCdoNpiSeqInstPtr->VNOC_BaseAddrCnt -1U])
		{
			XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_VNOC]=2U;
		}
	}
	else if(XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_VNOC] == 2U)
	{
		if(BaseAddr == XilCdoNpiSeqInstPtr->VNOC_BaseAddr
			[XilCdoNpiSeqInstPtr->VNOC_BaseAddrCnt -1U])
		{
			XilCdoNpiSeqInstPtr->WaitQueue
			[XILCDO_NPI_BLK_VNOC] = 3U;
		}
	}
	else if(XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_VNOC] == 3U)
	{
		XilCdo_ClearDeskew(BaseAddr);
		XilCdo_SetCompleteState(BaseAddr);
		XilCdo_SetLockState(BaseAddr);
		if(BaseAddr == XilCdoNpiSeqInstPtr->VNOC_BaseAddr
			[XilCdoNpiSeqInstPtr->VNOC_BaseAddrCnt -1U])
		{
			XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_VNOC] = 4U;
		}
	}
	else
	{
		if(BaseAddr == XilCdoNpiSeqInstPtr->VNOC_BaseAddr[0U])
		{
			RegVal = Xil_In32(CFU_APB_CFU_FGCR);
			if((RegVal & (1U<<CFU_APB_CFU_FGCR_GCLK_CAL_SHIFT))==1U)
			{
				RegVal = RegVal & ~(1U <<
				CFU_APB_CFU_FGCR_GCLK_CAL_SHIFT);
				Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
			}
			Xil_Out32(CFU_APB_CFU_FGCR, RegVal | (1U <<
					CFU_APB_CFU_FGCR_GWE_SHIFT));
		}
		XilCdo_ClearHoldState(BaseAddr);
		XilCdo_SetLockState(BaseAddr);
		if(BaseAddr == XilCdoNpiSeqInstPtr->VNOC_BaseAddr
		[XilCdoNpiSeqInstPtr->VNOC_BaseAddrCnt -1U])
		{
			XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_VNOC]=0U;
			XilCdoNpiSeqInstPtr->VNOC_BaseAddrCnt = 0U;
		}
	}
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for CLK_GT blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_PreProcBlkCLK_GT(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_ClearInitState(BaseAddr);
	XilCdo_ClearODisable(BaseAddr);
	XilCdo_SetStartCal(BaseAddr);
	XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_CLK_GT] = 1U;
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for CLK_GT blocks
 * This is called after PreProcessing function is called.
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_PostProcBlkCLK_GT(u32 BaseAddr)
{
	u32 RegVal;

	if(XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_CLK_GT] == 1U)
	{
		if(BaseAddr == XilCdoNpiSeqInstPtr->CLK_GT_BaseAddr[0U])
		{
			RegVal = Xil_In32(CFU_APB_CFU_FGCR);
			if((RegVal & (1U<<CFU_APB_CFU_FGCR_GCLK_CAL_SHIFT))==0U)
			{
				RegVal = RegVal | (1U <<
				CFU_APB_CFU_FGCR_GCLK_CAL_SHIFT);
				Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
			}
		}
		XilCdo_SetDeskew(BaseAddr);
		if(BaseAddr == XilCdoNpiSeqInstPtr->CLK_GT_BaseAddr
			[XilCdoNpiSeqInstPtr->CLK_GT_BaseAddrCnt -1U])
		{
			XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_CLK_GT]
									= 2U;
		}
	}
	else if(XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_CLK_GT] == 2U)
	{
		if(BaseAddr == XilCdoNpiSeqInstPtr->CLK_GT_BaseAddr
				[XilCdoNpiSeqInstPtr->CLK_GT_BaseAddrCnt -1U])
		{
			XilCdoNpiSeqInstPtr->WaitQueue
				[XILCDO_NPI_BLK_CLK_GT] = 3U;
		}
	}
	else if(XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_CLK_GT] == 3U)
	{
		XilCdo_ClearDeskew(BaseAddr);
		XilCdo_ClearStartCal(BaseAddr);
		XilCdo_SetCompleteState(BaseAddr);
		XilCdo_ClearHoldState(BaseAddr);
		XilCdo_SetLockState(BaseAddr);
		if(BaseAddr == XilCdoNpiSeqInstPtr->CLK_GT_BaseAddr
			[XilCdoNpiSeqInstPtr->CLK_GT_BaseAddrCnt -1U])
		{
			XilCdoNpiSeqInstPtr->WaitQueue
			[XILCDO_NPI_BLK_CLK_GT] = 4U;
		}
	}
	else
	{
		if(BaseAddr == XilCdoNpiSeqInstPtr->CLK_GT_BaseAddr[0U])
		{
			RegVal = Xil_In32(CFU_APB_CFU_FGCR);
			if((RegVal & (1U<<CFU_APB_CFU_FGCR_GCLK_CAL_SHIFT))==1U)
			{
				RegVal = RegVal & ~(1U <<
				CFU_APB_CFU_FGCR_GCLK_CAL_SHIFT);
				Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
			}
			Xil_Out32(CFU_APB_CFU_FGCR, RegVal | (1U << 
					CFU_APB_CFU_FGCR_GWE_SHIFT));
		}
		XilCdo_ClearHoldState(BaseAddr);
		XilCdo_SetLockState(BaseAddr);
		if(BaseAddr == XilCdoNpiSeqInstPtr->CLK_GT_BaseAddr
			[XilCdoNpiSeqInstPtr->CLK_GT_BaseAddrCnt -1U])
		{
			XilCdoNpiSeqInstPtr->WaitQueue[XILCDO_NPI_BLK_CLK_GT] 
									= 0U;
			XilCdoNpiSeqInstPtr->CLK_GT_BaseAddrCnt = 0U;
		}

	}
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for MMCM_DPLL blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkMMCM_DPLL(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_ClearInitState(BaseAddr);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for XPLL blocks
 *
 * @param	BaseAddr is base address of the block
 * @param	DdrPresent indicates presence of DDR
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkXPLL(u32 BaseAddr, u32 DdrPresent)
{
	XilCdo_ClearLockState(BaseAddr);
	if(DdrPresent)
	{
		XilCdo_ClearGateReg(BaseAddr);
		XilCdo_ClearInitState(BaseAddr);
		XilCdo_ClearODisable(BaseAddr);
		XilCdo_SetApbEnable(BaseAddr);
		XilCdo_SetCompleteState(BaseAddr);
	}
	else
	{
		XilCdo_ClearGateReg(BaseAddr);
		XilCdo_ClearInitState(BaseAddr);
		XilCdo_ClearODisable(BaseAddr);
		XilCdo_RunCalibration(BaseAddr,0U);
		XilCdo_SetFabricEnable(BaseAddr);
		XilCdo_SetCompleteState(BaseAddr);
	}
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for XPHY blocks
 *
 * @param	BaseAddr is base address of the block
 * @param	DdrPresent indicates presence of DDR
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkXPHY(u32 BaseAddr, u32 DdrPresent)
{
	XilCdo_ClearLockState(BaseAddr);
	if(DdrPresent)
	{
		XilCdo_ClearGateReg(BaseAddr);
		XilCdo_ClearInitState(BaseAddr);
		XilCdo_ClearODisable(BaseAddr);
		XilCdo_ClearHoldState(BaseAddr);
		XilCdo_SetCompleteState(BaseAddr);
		XilCdo_ClearTriState(BaseAddr);
	}
	else
	{
		XilCdo_ClearGateReg(BaseAddr);
		XilCdo_ClearInitState(BaseAddr);
		XilCdo_RunCalibration(BaseAddr,0U);
		XilCdo_SetFabricEnable(BaseAddr);
		XilCdo_ClearODisable(BaseAddr);
		XilCdo_ClearHoldState(BaseAddr);
		XilCdo_ClearTriState(BaseAddr);
		XilCdo_SetCompleteState(BaseAddr);
	}
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for DDRMC blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkDDRMC(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearInitState(BaseAddr);
	XilCdo_ClearUBInitState(BaseAddr);
	XilCdo_RunCalibration(BaseAddr,0U);
	XilCdo_ClearODisable(BaseAddr);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for SYSMON_SATELLITE blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkAMS_SAT(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_ClearInitState(BaseAddr);
	XilCdo_ClearHoldState(BaseAddr);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for AMS_ROOT blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkAMS_ROOT(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_ClearInitState(BaseAddr);
	XilCdo_ClearODisable(BaseAddr);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for XPIPE blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkXPIPE(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_SetFabricEnable(BaseAddr);
	XilCdo_ClearInitState(BaseAddr);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for GT blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkGT(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_SetFabricEnable(BaseAddr);
	XilCdo_ClearTriState(BaseAddr);
	XilCdo_ClearInitState(BaseAddr);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for NOC_NPS blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkNOC_NPS(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for NOC_IDB blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkNOC_IDB(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_ClearODisable(BaseAddr);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for NOC_NCRB blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkNOC_NCRB(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_RunCalibration(BaseAddr,0U);
	XilCdo_ClearODisable(BaseAddr);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for NOC_NSU blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkNOC_NSU(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_ClearHoldState(BaseAddr);
	XilCdo_ClearInitState(BaseAddr);
	XilCdo_SetFabricEnable(BaseAddr);
	XilCdo_ClearODisable(BaseAddr);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for NOC_NMU blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkNOC_NMU(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_ClearHoldState(BaseAddr);
	XilCdo_ClearInitState(BaseAddr);
	XilCdo_SetFabricEnable(BaseAddr);
	XilCdo_ClearODisable(BaseAddr);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI startup sequence for ME_NPI blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ProcBlkME_NPI(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_ClearHoldState(BaseAddr);
	XilCdo_ClearInitState(BaseAddr);
	XilCdo_ClearODisable(BaseAddr);
	XilCdo_RunCalibration(BaseAddr,0U);
	XilCdo_SetCompleteState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
}

/*****************************************************************************/
/**
 * @param	CmdArgs is pointer to argument data
 *
 * @return	returns success or the error codes described in xilcdo.h
 *
 ******************************************************************************/
XStatus XilCdo_NpiWrite(u32 CmdArgs[10U])
{
	u32 RegVal;
	u32 Count = 0U;
	XStatus Status;
	u32 DestAddr = CmdArgs[0U];
	u32 Len = CmdArgs[NPI_WRITE_LEN_INDEX];
	u32 SrcAddr = CmdArgs[9U];
	u32 Offset;
	u32 ProcWords=0U;

	if(SrcAddr >= XILCDO_PMCRAM_ENDADDR)
	{
		SrcAddr = XPMCFW_PMCRAM_BASEADDR;
	}
	/** Unlock the address space */
	XilCdo_ClearLockState(DestAddr & ~(0xFFFFU));
	
	/** For NPI WRITE command, the destination address needs to be
	 * 16 byte aligned. Use Xil_Out32 till the destination address
	 *becomes 16 byte aligned. */
	while((ProcWords<Len)&&(((DestAddr+(ProcWords<<2U))&(0xFU)) != 0U))
	{
		if(SrcAddr + Count>= XILCDO_PMCRAM_ENDADDR)
		{
			XilCdo_CopyCdoBuf();
			SrcAddr = XPMCFW_PMCRAM_BASEADDR;
			Count = 0U;
		}
		RegVal = Xil_In32(SrcAddr + Count);
		Xil_Out32(DestAddr + ProcWords<<2U, RegVal);
		Count+=4U;
		++ProcWords;
	}
	Status = XilCdo_DmaTransfer((u64)(SrcAddr+Count),
		(u64)(DestAddr+(ProcWords<<2U)), (Len - ProcWords)&(~(0x3U)));
	if(Status != XST_SUCCESS)
	{
		goto END;
	}

	/** For NPI_WRITE command, Offset variable should 
	 *  be updated with the unaligned bytes. */
	ProcWords = ((Len - ProcWords)&(~(0x3U))) + ProcWords;
	Offset = (ProcWords<<2U);
	SrcAddr = CmdArgs[9U] + Offset;
	SrcAddr = SrcAddr%XILCDO_MAX_PARTITION_LENGTH + XPMCFW_PMCRAM_BASEADDR;
	Count = 0U;
	if(SrcAddr >= XILCDO_PMCRAM_ENDADDR)
	{
		XilCdo_CopyCdoBuf();
		if (XST_SUCCESS != Status)
		{
			goto END;
		}
		SrcAddr = XPMCFW_PMCRAM_BASEADDR;
	}
	while(ProcWords < Len)
	{
		if(SrcAddr + Count >= XILCDO_PMCRAM_ENDADDR)
		{
			SrcAddr = XPMCFW_PMCRAM_BASEADDR;
			Count=0;
		}
		RegVal = Xil_In32(SrcAddr + Count);
		Xil_Out32(DestAddr + Offset, RegVal);
		Count+=4U;
		Offset += 4U;
		ProcWords++;
	}
	CmdArgs[9U] = Len + CMD_NPI_WRITE_ARGS;
	
	/** Lock the address space */
	XilCdo_SetLockState(DestAddr & ~(0xFFFFU));
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for VREF blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkVREF(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_SetTriState(BaseAddr);
	XilCdo_ClearFabricEnable(BaseAddr);
	XilCdo_SetInitState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for XPIO blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkXPIO(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_SetTriState(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_ClearFabricEnable(BaseAddr);
	XilCdo_SetInitState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for DCI blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkDCI(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetInitState(BaseAddr);
	XilCdo_SetDCIOfcReset(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
	XilCdo_SetHoldState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for
 * VERT_TO_HSR, VERT_TO_HSR_GT, REBUF_VRT and REBUF_VRT_GT blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkHSR_VRT(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for HSR_TNOC_ME blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkHSR_TNOC_ME(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for HSR_BUFGS blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkHSR_BUFGS(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
	XilCdo_SetHoldState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for VNOC_PS blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkVNOC_PS(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
	XilCdo_SetHoldState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for VNOC blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkVNOC(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for CLK_GT blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkCLK_GT(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
	XilCdo_SetHoldState(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for MMCM/DPLL blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkMMCM_DPLL(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_SetInitState(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_ClearFabricEnable(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for XPHY blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkXPHY(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_SetTriState(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
	XilCdo_SetInitState(BaseAddr);
	XilCdo_SetHoldState(BaseAddr);

}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for XPLL blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkXPLL(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_SetInitState(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_ClearApbEnable(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for DDRMC blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkDDRMC(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetInitState(BaseAddr);
	XilCdo_SetUBInitState(BaseAddr);
	XilCdo_SetHoldState(BaseAddr);
	XilCdo_ClearFabricEnable(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for XPIPE blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkXPIPE(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_SetInitState(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
	XilCdo_ClearFabricEnable(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for GT blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkGT(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_SetShutDown(BaseAddr);
	XilCdo_SetTriState(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_SetTriState(BaseAddr);
	XilCdo_ClearFabricEnable(BaseAddr);
	XilCdo_SetInitState(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
	XilCdo_ClearShutDown(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for NOC_NSU blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkNSU(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_RunNsuPRMode(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_ClearFabricEnable(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetInitState(BaseAddr);
	XilCdo_SetHoldState(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
	XilCdo_ClearPRMode(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for NOC_NMU blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkNMU(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_RunAxiReqRejectMode(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_ClearFabricEnable(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetInitState(BaseAddr);
	XilCdo_SetHoldState(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
	XilCdo_ClearAxiReqRejectMode(BaseAddr);
}

/*****************************************************************************/
/**
 * This function does the required NPI shutdown sequence for NOC_NCRB blocks
 *
 * @param	BaseAddr is base address of the block
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_ShutBlkNCRB(u32 BaseAddr)
{
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_SetODisable(BaseAddr);
	XilCdo_ClearCompleteState(BaseAddr);
	XilCdo_SetInitState(BaseAddr);
	XilCdo_SetHoldState(BaseAddr);
	XilCdo_SetGateReg(BaseAddr);
}
