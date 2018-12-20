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
#include "xpmcfw_util.h"
#include "xpmcfw_hw.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
extern XilCdo_Prtn XilCdoPrtnInst;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

XILCDO_NPI_SEQ* XilCdoNpiSeqInstPtr = NULL;
XILCDO_NPI_SEQ XilCdoNpiSeqInstance;
u32 ScanClearDone;
u32 NpiCmd;
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

void XilCdo_EnableCFUWrite(void)
{

	u32 RegVal = Xil_In32(CFU_APB_CFU_PROTECT);
	RegVal &= ~CFU_APB_CFU_PROTECT_ACTIVE_MASK;
	Xil_Out32(CFU_APB_CFU_PROTECT, RegVal);
}

void XilCdo_DisableCFUWrite(void)
{

	u32 RegVal = Xil_In32(CFU_APB_CFU_PROTECT);
	RegVal |= CFU_APB_CFU_PROTECT_ACTIVE_MASK;
	Xil_Out32(CFU_APB_CFU_PROTECT, RegVal);
}

void XilCdo_ResetGlobalSignals(void)
{
	u32 RegVal = Xil_In32(CFU_APB_CFU_FGCR);

	XilCdo_EnableCFUWrite();
	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_GSCWE_MASK |
			CFU_APB_CFU_FGCR_GMC_B_MASK | CFU_APB_CFU_FGCR_GRESTORE_MASK |
			CFU_APB_CFU_FGCR_GHIGH_B_MASK | CFU_APB_CFU_FGCR_GCLK_CAL_MASK |
			CFU_APB_CFU_FGCR_GTS_CFG_B_MASK | CFU_APB_CFU_FGCR_GWE_MASK |
			CFU_APB_CFU_FGCR_EOS_MASK | CFU_APB_CFU_FGCR_EN_GLOBS_B_MASK);
	RegVal &= ~CFU_APB_CFU_FGCR_GSCWE_MASK;
	RegVal &= ~CFU_APB_CFU_FGCR_GMC_B_MASK;
	RegVal &= ~CFU_APB_CFU_FGCR_GRESTORE_MASK;
	RegVal &= ~CFU_APB_CFU_FGCR_GHIGH_B_MASK;
	RegVal &= ~CFU_APB_CFU_FGCR_GCLK_CAL_MASK;
	RegVal &= ~CFU_APB_CFU_FGCR_GTS_CFG_B_MASK;
	RegVal &= ~CFU_APB_CFU_FGCR_GWE_MASK;
	RegVal &= ~CFU_APB_CFU_FGCR_EOS_MASK;
	RegVal &= ~CFU_APB_CFU_FGCR_EN_GLOBS_B_MASK;
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
	XilCdo_DisableCFUWrite();
}

void XilCdo_ResetGlobalSignals_ME(void)
{
	u32 RegVal = Xil_In32(CFU_APB_CFU_FGCR);

	XilCdo_EnableCFUWrite();
	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_GSCWE_MASK |
			CFU_APB_CFU_FGCR_GMC_B_MASK | CFU_APB_CFU_FGCR_GRESTORE_MASK |
			CFU_APB_CFU_FGCR_GHIGH_B_MASK | CFU_APB_CFU_FGCR_GTS_CFG_B_MASK
			| CFU_APB_CFU_FGCR_GWE_MASK | CFU_APB_CFU_FGCR_EOS_MASK |
			CFU_APB_CFU_FGCR_EN_GLOBS_B_MASK);
	RegVal &= ~CFU_APB_CFU_FGCR_GSCWE_MASK;
	RegVal |= CFU_APB_CFU_FGCR_GMC_B_MASK;
	RegVal &= ~CFU_APB_CFU_FGCR_GRESTORE_MASK;
	RegVal |= CFU_APB_CFU_FGCR_GHIGH_B_MASK;
	RegVal |= CFU_APB_CFU_FGCR_GTS_CFG_B_MASK;
	RegVal |= CFU_APB_CFU_FGCR_GWE_MASK;
	RegVal |= CFU_APB_CFU_FGCR_EOS_MASK;
	RegVal |= CFU_APB_CFU_FGCR_EN_GLOBS_B_MASK;
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
	XilCdo_DisableCFUWrite();
}

void XilCdo_SetGlobalSignals(void)
{
	u32 RegVal = Xil_In32(CFU_APB_CFU_FGCR);

	XilCdo_EnableCFUWrite();

	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_GMC_B_MASK);
	RegVal |= CFU_APB_CFU_FGCR_GMC_B_MASK;
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);

	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_GRESTORE_MASK);
	RegVal |= CFU_APB_CFU_FGCR_GRESTORE_MASK;
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);


	RegVal &= ~CFU_APB_CFU_FGCR_GRESTORE_MASK;
	Xil_Out32(CFU_APB_CFU_MASK,CFU_APB_CFU_FGCR_GRESTORE_MASK);
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);

	RegVal |= CFU_APB_CFU_FGCR_GHIGH_B_MASK;
	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_GHIGH_B_MASK);
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);

	usleep(100U);

	RegVal |= CFU_APB_CFU_FGCR_GTS_CFG_B_MASK;
	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_GTS_CFG_B_MASK);
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);

	XilCdo_DisableCFUWrite();
}

void XilCdo_ClearGlobalSignals(void)
{
	u32 RegVal = Xil_In32(CFU_APB_CFU_FGCR);

	XilCdo_EnableCFUWrite();
	XilCdo_SetGSCWE();
	XilCdo_ClearEOS();
	XilCdo_ClearGWE();

	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_GRESTORE_MASK);
	RegVal &= ~CFU_APB_CFU_FGCR_GRESTORE_MASK;
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);


	RegVal |= CFU_APB_CFU_FGCR_GRESTORE_MASK;
	Xil_Out32(CFU_APB_CFU_MASK,CFU_APB_CFU_FGCR_GRESTORE_MASK);
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);

	RegVal &= ~CFU_APB_CFU_FGCR_GTS_CFG_B_MASK;
	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_GTS_CFG_B_MASK);
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);

	RegVal &= ~CFU_APB_CFU_FGCR_GHIGH_B_MASK;
	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_GHIGH_B_MASK);
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);

	usleep(100U);

	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_GMC_B_MASK);
	RegVal &= ~CFU_APB_CFU_FGCR_GMC_B_MASK;
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);

	XilCdo_DisableCFUWrite();
}

void XilCdo_SetEOS(void)
{
	u32 RegVal = Xil_In32(CFU_APB_CFU_FGCR);
	RegVal |= CFU_APB_CFU_FGCR_EOS_MASK;
	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_EOS_MASK);
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
}

void XilCdo_ClearEOS(void)
{
	u32 RegVal = Xil_In32(CFU_APB_CFU_FGCR);
	RegVal &= ~CFU_APB_CFU_FGCR_EOS_MASK;
	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_EOS_MASK);
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
}

void XilCdo_SetGWE(void)
{
	u32 RegVal = Xil_In32(CFU_APB_CFU_FGCR);
	RegVal |= CFU_APB_CFU_FGCR_GWE_MASK;
	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_GWE_MASK);
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
}

void XilCdo_ClearGWE(void)
{
	u32 RegVal = Xil_In32(CFU_APB_CFU_FGCR);
	RegVal &= ~CFU_APB_CFU_FGCR_GWE_MASK;
	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_GWE_MASK);
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
}

void XilCdo_SetGSCWE(void)
{
	u32 RegVal = Xil_In32(CFU_APB_CFU_FGCR);
	RegVal |= CFU_APB_CFU_FGCR_GSCWE_MASK;
	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_GSCWE_MASK);
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
}

void XilCdo_ClearGSCWE(void)
{
	u32 RegVal = Xil_In32(CFU_APB_CFU_FGCR);
	RegVal &= ~CFU_APB_CFU_FGCR_GSCWE_MASK;
	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_GSCWE_MASK);
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
}

void XilCdo_ClearEnGlob(void)
{
	u32 RegVal = Xil_In32(CFU_APB_CFU_FGCR);
	RegVal |= CFU_APB_CFU_FGCR_EN_GLOBS_B_MASK;
	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_EN_GLOBS_B_MASK);
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal);
}

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
 *
 * @return	none
 *
 *****************************************************************************/
void XilCdo_CheckCalibration(u32 BaseAddr)
{
	/*Wait for Calibration to complete */
	while((Xil_In32(BaseAddr + XILCDO_NPI_PCSR_STATUS_OFFSET)
				& PCSR_STATUS_CALDONE_MASK) != PCSR_STATUS_CALDONE_MASK)
	{
		;
	}


	XilCdo_ClearStartCal(BaseAddr);

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

void XilCdo_CheckShutDown(u32 BaseAddr)
{
	while((Xil_In32(BaseAddr + XILCDO_NPI_PCSR_STATUS_OFFSET) & PCSR_STATUS_SHUTDN_COMP_MASK)==0U);
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

void XilCdo_CheckRegBusy(u32 BaseAddr)
{
	while(Xil_In32(BaseAddr + XILCDO_REG_BUSY_OFFSET) & XILCDO_REG_BUSY_MASK);
}

void XilCdo_CheckRegPendingBurst(u32 BaseAddr)
{
	while(Xil_In32(BaseAddr + XILCDO_REG_PEND_BURST_OFFSET) &
							XILCDO_REG_PEND_BURST_OFFSET_MASK);
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

void XilCdo_ClearInitCtrl(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_INIT_CTRL_MASK, 0U);
}

void XilCdo_SetInitCtrl(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_INIT_CTRL_MASK, PCSR_MASK_INIT_CTRL_MASK);
}

void XilCdo_SetBISRTrigger(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_BISR_TRIGGER_MASK,
			PCSR_MASK_BISR_TRIGGER_MASK);
}

u32 XilCdo_CheckBISRPass(u32 BaseAddr)
{
	/* Check for BISR success */
	return ((Xil_In32(BaseAddr+XILCDO_NPI_PCSR_STATUS_OFFSET)
				& PCSR_STATUS_BISR_PASS_MASK)
			!= PCSR_STATUS_BISR_PASS_MASK);
}

void XilCdo_WaitForBISRDone(u32 BaseAddr)
{
	/* Wait for BISR to complete */
	while((Xil_In32(BaseAddr+XILCDO_NPI_PCSR_STATUS_OFFSET)
				& PCSR_STATUS_BISRDONE_MASK)
			!= PCSR_STATUS_BISRDONE_MASK);
}

u32 XilCdo_ScanClear(u32 NpiParam)
{
	u32 Status;
	if((NpiParam & PCSR_PRECFG_SCANCLR_MASK) == 0U)
	{
		Status = XST_SUCCESS;
		goto END;
	}
	if(ScanClearDone == FALSE)
	{
		XPmcFw_UtilRMW(PMC_ANALOG_SCAN_CLEAR_TRIGGER,
				PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK,
				PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK);

		XilCdo_WaitForScanClearDone();
		ScanClearDone = TRUE;
		Status = XilCdo_CheckScanClearPass();
	}
	else
	{
		Status = XST_SUCCESS;
	}
END:
	return Status;
}

u32 XilCdo_CheckScanClearPass(void)
{
	/* Check for Scan Clear success */
	return ((Xil_In32(PMC_ANALOG_SCAN_CLEAR_DONE)
				& PMC_ANALOG_SCAN_CLEAR_PASS_PMC_MASK)
			!= PMC_ANALOG_SCAN_CLEAR_PASS_PMC_MASK);
}

void XilCdo_WaitForScanClearDone(void)
{
	/* Wait for Scan Clear to complete */
	while((Xil_In32(PMC_ANALOG_SCAN_CLEAR_DONE)
				& PMC_ANALOG_SCAN_CLEAR_DONE_PMC_MASK)
			!= PMC_ANALOG_SCAN_CLEAR_DONE_PMC_MASK);
}

u32 XilCdo_ScanClearME(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_SCANCLR_MASK,
			PCSR_MASK_SCANCLR_MASK);
	XilCdo_WaitForScanClearMEDone(BaseAddr);
	return XilCdo_CheckScanClearMEPass(BaseAddr);
}

u32 XilCdo_CheckScanClearMEPass(u32 BaseAddr)
{
	/* Check for MBIST success */
	return ((Xil_In32(BaseAddr+XILCDO_NPI_PCSR_STATUS_OFFSET)
				& PCSR_STATUS_SCAN_PASS_MASK)
			!= PCSR_STATUS_SCAN_PASS_MASK);
}

void XilCdo_WaitForScanClearMEDone(u32 BaseAddr)
{
	/* Wait for MBIST to complete */
	while((Xil_In32(BaseAddr+XILCDO_NPI_PCSR_STATUS_OFFSET)
				& PCSR_STATUS_SCANDONE_MASK)
			!= PCSR_STATUS_SCANDONE_MASK);
}

void XilCdo_SetMBISTTrigger(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_MBISTCLR_MASK,
			PCSR_MASK_MBISTCLR_MASK);
}

void XilCdo_ClearMBISTTrigger(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_MBISTCLR_MASK, 0U);
}

u32 XilCdo_CheckMBISTPass(u32 BaseAddr)
{
	/* Check for MBIST success */
	return ((Xil_In32(BaseAddr+XILCDO_NPI_PCSR_STATUS_OFFSET)
				& PCSR_STATUS_MBIST_PASS_MASK)
			!= PCSR_STATUS_MBIST_PASS_MASK);
}

void XilCdo_WaitForMBISTDone(u32 BaseAddr)
{
	/* Wait for MBIST to complete */
	while((Xil_In32(BaseAddr+XILCDO_NPI_PCSR_STATUS_OFFSET)
				& PCSR_STATUS_MBISTDONE_MASK)
			!= PCSR_STATUS_MBISTDONE_MASK);
}

u32 XilCdo_ChkMEPwrSupply(u32 BaseAddr)
{
	return ((Xil_In32(BaseAddr+XILCDO_NPI_PCSR_STATUS_OFFSET)
				& PCSR_STATUS_ME_PWR_SUPPLY_MASK)
			== PCSR_STATUS_ME_PWR_SUPPLY_MASK);
}

void XilCdo_ClearME_POR(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_ME_IPOR_MASK, 0x0U);
}

void XilCdo_ClearODisable0(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_ODISABLE0_MASK, 0x0U);
}

void XilCdo_ClearODisable1(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_ODISABLE1_MASK, 0x0U);
}

void XilCdo_SetODisableAXI(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_ODISABLE_AXI_MASK, PCSR_MASK_ODISABLE_AXI_MASK);
}

void XilCdo_ClearODisableNPP(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_ODISABLE_NPP_MASK, 0U);
}

void XilCdo_ClearODisableAXI(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_ODISABLE_AXI_MASK, 0U);
}

void XilCdo_SetODisableNPP(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_ODISABLE_NPP_MASK, PCSR_MASK_ODISABLE_NPP_MASK);
}

void XilCdo_ClearMEArrayReset(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_ME_ARRAY_RESET_MASK, 0x0U);
}

void XilCdo_SetMemClearEnAll(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_MEM_CLEAR_EN_ALL,
			PCSR_MASK_MEM_CLEAR_EN_ALL);
}

void XilCdo_ClearOD_MBIST_ASYNC_RESET(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_OD_MBIST_ASYNC_RESET, 0x0U);
}

void XilCdo_SetOD_MBIST_ASYNC_RESET(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_OD_MBIST_ASYNC_RESET,
			PCSR_MASK_OD_MBIST_ASYNC_RESET);
}

void XilCdo_ClearOD_BIST_SETUP1(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_OD_BIST_SETUP1, 0x0U);
}

void XilCdo_SetOD_BIST_SETUP1(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_OD_BIST_SETUP1,
			PCSR_MASK_OD_BIST_SETUP1);
}

void XilCdo_SetPRFreeze(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_PR_FREEZE_MASK,
			PCSR_MASK_PR_FREEZE_MASK);
}

void XilCdo_ClearPRFreeze(u32 BaseAddr)
{
	XilCdo_WritePcsrCtrlReg(BaseAddr, PCSR_MASK_PR_FREEZE_MASK, 0U);
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
	XStatus Status;

	CmdArgs[9U] = CMD_NPI_SEQ_ARGS;
	if((NpiCmd != 0U) &&(NpiCmd != CMD_NPI_SEQ))
	{
		XilCdo_RunPendingNpiSeq();
	}
	NpiCmd = CMD_NPI_SEQ;
	Status = XilCdo_StoreNpiParams(CmdArgs);
	return Status;
}

XStatus XilCdo_StoreNpiParams(u32 CmdArgs[10U])
{
	u32 BaseAddr = CmdArgs[0U];
	u32 NpiParam = CmdArgs[1U];
	u32 BlockType;
	XStatus Status;
	if(XilCdoNpiSeqInstPtr == NULL)
	{
		/** Initialization of instance variable */
		memset(&XilCdoNpiSeqInstance, 0U, sizeof(XilCdoNpiSeqInstance));
		XilCdoNpiSeqInstPtr = &XilCdoNpiSeqInstance;
	}
	BlockType = NpiParam & XILCDO_NPI_BLK_MASK;
	switch(BlockType)
	{
		case XILCDO_NPI_BLK_DDRMC_MAIN:
			{
				//Do nothing
			}
			break;
		case XILCDO_NPI_BLK_VREF:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VREF_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]] = BaseAddr;
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VREF_NpiParam
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = NpiParam;
			}
			break;
		case XILCDO_NPI_BLK_XPIO:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]] = BaseAddr;
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_NpiParam
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = NpiParam;
			}
			break;
		case XILCDO_NPI_BLK_XPIO_IOMISC:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]] = BaseAddr;
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_NpiParam
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = NpiParam;
			}
			break;
		case XILCDO_NPI_BLK_VERT_TO_HSR:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_VERT_TO_HSR_GT:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_GT_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_REBUF_VRT:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_REBUF_VRT_GT:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_GT_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_HSR_BUFGS:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_HSR_BUFGS_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_VNOC_PS:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_PS_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_VNOC:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_CLK_GT:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_CLK_GT_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_MMCM:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_MMCM_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_DPLL:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DPLL_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_XPLL:
			{
				if(BaseAddr < XILCDO_NPI_BLK_XPLL_START_ADDR)
				{
					XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr
						[XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_PLL_PHY]] = BaseAddr;
					XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_NpiParam
						[XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_PLL_PHY]++] = NpiParam;
				}
				else
				{
					XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr
						[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]] = BaseAddr;
					XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_NpiParam
						[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = NpiParam;
				}
			}
			break;
		case XILCDO_NPI_BLK_XPHY:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]] = BaseAddr;
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_NpiParam
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = NpiParam;
			}
			break;
		case XILCDO_NPI_BLK_DDRMC:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DDRMC_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_AMS_SAT:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_AMS_SAT_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_AMS_ROOT:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_AMS_ROOT_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_XPIPE:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIPE_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]] = BaseAddr;
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIPE_NpiParam
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = NpiParam;
			}
			break;
		case XILCDO_NPI_BLK_GT:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]] = BaseAddr;
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_NpiParam
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = NpiParam;
			}
			break;
		case XILCDO_NPI_BLK_ME_NPI:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_ME_NPI_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_NOC_NPS:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NPS_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_NOC_IDB:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_IDB_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_NOC_NCRB:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NCRB_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = BaseAddr;
			}
			break;
		case XILCDO_NPI_BLK_NOC_NSU:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]] = BaseAddr;
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_NpiParam
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = NpiParam;
			}
			break;
		case XILCDO_NPI_BLK_NOC_NMU:
			{
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]] = BaseAddr;
				XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_NpiParam
					[XilCdoNpiSeqInstPtr->BaseAddrCnt[BlockType]++] = NpiParam;
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
 * @return  SUCCESS
 *
 *****************************************************************************/
XStatus XilCdo_RunPendingNpiSeq(void)
{
	XStatus Status;
	u32 Count;

	if(XilCdoNpiSeqInstPtr == NULL)
	{
		Status = XST_SUCCESS;
		goto END;
	}
	if(NpiCmd == CMD_NPI_SEQ)
	{
		XPmcFw_Printf(DEBUG_DETAILED,"\n Running all pending Startup/PR sequences \n\r");
		for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_GT]; ++Count)
		{
			XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
		}
		for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_GT]; ++Count)
		{
			XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
			XilCdo_ClearInitCtrl(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
		}
		XilCdo_SetGlobalSignals();
		XilCdo_ProcIOModules();
		XilCdoProcNocModules();
		XilCdoProcAMSModules();
		XilCdoProcGTModules();
		XilCdoProcMMCM_CLK();
		XilCdo_AssertGlobalSignals();
	}
	else if(NpiCmd == CMD_NPI_SHUTDN)
	{
		XPmcFw_Printf(DEBUG_DETAILED,"\n Running all pending shut down sequences \n\r");
		XilCdo_ShutNocModules();
		XilCdo_ShutIOModules();
		XilCdo_ShutGTModules();
		XilCdo_ShutMMCM_CLK();
		XilCdo_ClearGlobalSignals();
	}
	else
	{

	}
	memset(&XilCdoNpiSeqInstance, 0U, sizeof(XilCdoNpiSeqInstance));
	XilCdoNpiSeqInstPtr = NULL;
	NpiCmd = 0U;
	Status = XST_SUCCESS;
END:
	return Status;
}

void XilCdo_ProcIOModules(void)
{
	u32 Count;
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VREF]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VREF_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_PLL_PHY]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPHY]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DDRMC]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DDRMC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO_IOMISC]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO_IOMISC]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr[Count]);
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_NpiParam[Count] &
					XILCDO_NPI_DDRMC_PRESENT_MASK) == XILCDO_NPI_DDRMC_PRESENT_MASK)
		{
			XilCdo_ClearTriState(XilCdoNpiSeqInstPtr->
					XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VREF]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VREF_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VREF_BaseAddr[Count]);
	}

	usleep(10);

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO]; ++Count)
	{
		XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO_IOMISC]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_NpiParam[Count] &
					XILCDO_NPI_DDRMC_PRESENT_MASK) == 0U)
		{
			XilCdo_ClearTriState(XilCdoNpiSeqInstPtr->
					XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO_IOMISC]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr[Count]);
		XilCdo_ClearDCIOfcReset(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr[Count]);
	}

	usleep(100U);

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO_IOMISC]; ++Count)
	{
		XilCdo_CheckCalibration(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO_IOMISC]; ++Count)
	{
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr[Count]);
	}

	/* DDR related changes */
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_NpiParam[Count] &
				XILCDO_NPI_DDRMC_PRESENT_MASK) == XILCDO_NPI_DDRMC_PRESENT_MASK)
		{
			XilCdo_ClearTriState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
			XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_PLL_PHY]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		if(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_NpiParam[Count] & XILCDO_NPI_DDRMC_PRESENT_MASK)
		{
			XilCdo_SetApbEnable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
			XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
		}
		else
		{
			XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
			XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_NpiParam[Count] & XILCDO_NPI_DDRMC_PRESENT_MASK)==0U)
		{
			XilCdo_CheckCalibration(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
		}

	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_PLL_PHY]; ++Count)
	{
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_NpiParam[Count] & XILCDO_NPI_DDRMC_PRESENT_MASK)==0U)
		{
			if(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_NpiParam[Count] & XILCDO_NPI_FABRICEN_MASK)
			{
				XilCdo_SetFabricEnable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
			}

			XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPHY]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPHY]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPHY]; ++Count)
	{
		XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPHY]; ++Count)
	{
		XilCdo_ClearHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPHY]; ++Count)
	{
		XilCdo_ClearTriState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_BaseAddr[Count]);
	}
	/** Workaround for EDT-990664 */
#if 0
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DDRMC]; ++Count)
	{
		XilCdo_ClearHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DDRMC_BaseAddr[Count]);
	}
#endif
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DDRMC]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DDRMC_BaseAddr[Count]);
		XilCdo_ClearUBInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DDRMC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DDRMC]; ++Count)
	{
		XilCdo_SetStartCal(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DDRMC_BaseAddr[Count]);
	}
	usleep(100U);

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		if(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_NpiParam[Count] & XILCDO_NPI_DDRMC_PRESENT_MASK)
		{
			XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		if(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_NpiParam[Count] & XILCDO_NPI_DDRMC_PRESENT_MASK)
		{
			XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
		}
	}
	usleep(100U);

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_NpiParam[Count] &
						XILCDO_NPI_DDRMC_PRESENT_MASK) == XILCDO_NPI_DDRMC_PRESENT_MASK)
		{
			XilCdo_CheckCalibration(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DDRMC]; ++Count)
	{
		XilCdo_CheckCalibration(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DDRMC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DDRMC]; ++Count)
	{
		XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DDRMC_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DDRMC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_NpiParam[Count] &
					XILCDO_NPI_PR_FREEZE_MASK) == XILCDO_NPI_PR_FREEZE_MASK)
		{
			XilCdo_ClearPRFreeze(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO]; ++Count)
	{
		XilCdo_ClearTriState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
	}


	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VREF]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VREF_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO_IOMISC]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPHY]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DDRMC]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DDRMC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_PLL_PHY]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr[Count]);
	}
}

void XilCdoProcMMCM_CLK(void)
{
	u32 Count;
	u32 RegVal;

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VERT_TO_HSR]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VERT_TO_HSR_GT]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_VRT]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_VRT_GT]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_HSR_BUFGS]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_HSR_BUFGS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC_PS]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_PS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_CLK_GT]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_CLK_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_MMCM]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_MMCM_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DPLL]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_PLL_PHY]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr[Count]);
	}

	/* Deassert GateReg for all CLK elements */
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VERT_TO_HSR]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VERT_TO_HSR_GT]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_VRT]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_VRT_GT]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_HSR_BUFGS]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_HSR_BUFGS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC_PS]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_PS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_CLK_GT]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_CLK_GT_BaseAddr[Count]);
	}

	/* Init state de-asserted in BUFGS and active deskew state machines */
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_HSR_BUFGS]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_HSR_BUFGS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC_PS]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_PS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_CLK_GT]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_CLK_GT_BaseAddr[Count]);
	}

	/* Clear ODisable for all CLK elements */
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VERT_TO_HSR]; ++Count)
	{
		XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VERT_TO_HSR_GT]; ++Count)
	{
		XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_VRT]; ++Count)
	{
		XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_VRT_GT]; ++Count)
	{
		XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME]; ++Count)
	{
		XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_HSR_BUFGS]; ++Count)
	{
		XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_HSR_BUFGS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC_PS]; ++Count)
	{
		XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_PS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC]; ++Count)
	{
		XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_CLK_GT]; ++Count)
	{
		XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_CLK_GT_BaseAddr[Count]);
	}

	/* Assert Start Cal */

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_HSR_BUFGS]; ++Count)
	{
		XilCdo_SetStartCal(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_HSR_BUFGS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC_PS]; ++Count)
	{
		XilCdo_SetStartCal(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_PS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_CLK_GT]; ++Count)
	{
		XilCdo_SetStartCal(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_CLK_GT_BaseAddr[Count]);
	}

	XilCdo_EnableCFUWrite();
	RegVal = Xil_In32(CFU_APB_CFU_FGCR);
	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_GCLK_CAL_MASK);
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal | CFU_APB_CFU_FGCR_GCLK_CAL_MASK);

	/* Set Deskew */
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_VRT]; ++Count)
	{
		XilCdo_SetDeskew(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_VRT_GT]; ++Count)
	{
		XilCdo_SetDeskew(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC_PS]; ++Count)
	{
		XilCdo_SetDeskew(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_PS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC]; ++Count)
	{
		XilCdo_SetDeskew(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_CLK_GT]; ++Count)
	{
		XilCdo_SetDeskew(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_CLK_GT_BaseAddr[Count]);
	}

	usleep(100U);

	/* Clear Deskew */
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_VRT]; ++Count)
	{
		XilCdo_ClearDeskew(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_VRT_GT]; ++Count)
	{
		XilCdo_ClearDeskew(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_GT_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC_PS]; ++Count)
	{
		XilCdo_ClearDeskew(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_PS_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_PS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC]; ++Count)
	{
		XilCdo_ClearDeskew(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_CLK_GT]; ++Count)
	{
		XilCdo_ClearDeskew(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_CLK_GT_BaseAddr[Count]);
	}

	/* Clear Start Cal */

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_HSR_BUFGS]; ++Count)
	{
		XilCdo_ClearStartCal(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_HSR_BUFGS_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_HSR_BUFGS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC_PS]; ++Count)
	{
		XilCdo_ClearStartCal(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_PS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_CLK_GT]; ++Count)
	{
		XilCdo_ClearStartCal(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_CLK_GT_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_CLK_GT_BaseAddr[Count]);
	}

	/* Set Complete State */
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VERT_TO_HSR]; ++Count)
	{
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VERT_TO_HSR_GT]; ++Count)
	{
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME]; ++Count)
	{
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME_BaseAddr[Count]);
	}

	RegVal = Xil_In32(CFU_APB_CFU_FGCR);
	Xil_Out32(CFU_APB_CFU_MASK, CFU_APB_CFU_FGCR_GCLK_CAL_MASK);
	Xil_Out32(CFU_APB_CFU_FGCR, RegVal & ~(CFU_APB_CFU_FGCR_GCLK_CAL_MASK));

	/* Clear Hold State */
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_HSR_BUFGS]; ++Count)
	{
		XilCdo_ClearHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_HSR_BUFGS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC_PS]; ++Count)
	{
		XilCdo_ClearHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_PS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_CLK_GT]; ++Count)
	{
		XilCdo_ClearHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_CLK_GT_BaseAddr[Count]);
	}

	XilCdo_SetGWE();

	/* Clear Gate Registers for CMT Blocks */
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_MMCM]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_MMCM_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DPLL]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_PLL_PHY]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr[Count]);
	}

	/* Clear Init State for MMCM/DPLL and clear ODisable for DPLL/XPLL */
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_MMCM]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_MMCM_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DPLL]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DPLL_BaseAddr[Count]);
		XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
		XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
	}

	usleep(100U);
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		XilCdo_CheckCalibration(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DPLL]; ++Count)
	{
		XilCdo_CheckCalibration(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_MMCM]; ++Count)
	{
		XilCdo_CheckCalibration(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_MMCM_BaseAddr[Count]);
	}

	/* Set Complete state */
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_MMCM]; ++Count)
	{
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_MMCM_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DPLL]; ++Count)
	{
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_PLL_PHY]; ++Count)
	{
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr[Count]);
	}

	XilCdo_SetEOS();
	XilCdo_ClearEnGlob();

	/* Lock all blocks */
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VERT_TO_HSR]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VERT_TO_HSR_GT]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_VRT]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_VRT_GT]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_HSR_BUFGS]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_HSR_BUFGS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC_PS]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_PS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_CLK_GT]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_CLK_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_MMCM]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_MMCM_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DPLL]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_PLL_PHY]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr[Count]);
	}
}

void XilCdoProcNocModules()
{
	u32 Count;
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NPS]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NPS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_IDB]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_IDB_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NCRB]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NCRB_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NSU]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NMU]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NSU]; ++Count)
	{
		XilCdo_SetODisableNPP(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NMU]; ++Count)
	{
		XilCdo_SetODisableNPP(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NPS]; ++Count)
	{
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NPS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NCRB]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NCRB_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NCRB]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NCRB_BaseAddr[Count]);
		XilCdo_ClearHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NCRB_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NCRB]; ++Count)
	{
		XilCdo_SetStartCal(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NCRB_BaseAddr[Count]);
	}
	usleep(100U);

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NCRB]; ++Count)
	{
		XilCdo_CheckCalibration(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NCRB_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NCRB]; ++Count)
	{
		XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NCRB_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NCRB_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_IDB]; ++Count)
	{
		XilCdo_ClearODisableNPP(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_IDB_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_IDB_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NSU]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NSU]; ++Count)
	{
		XilCdo_ClearHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NSU]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NSU]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_NpiParam[Count] &
					XILCDO_NPI_FABRICEN_MASK) == XILCDO_NPI_FABRICEN_MASK)
		{
			XilCdo_SetFabricEnable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NSU]; ++Count)
	{
		XilCdo_ClearODisableNPP(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
		XilCdo_ClearODisableAXI(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NMU]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NMU]; ++Count)
	{
		XilCdo_ClearHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NMU]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NMU]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_NpiParam[Count] &
					XILCDO_NPI_FABRICEN_MASK) == XILCDO_NPI_FABRICEN_MASK)
		{
			XilCdo_SetFabricEnable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NMU]; ++Count)
	{
		XilCdo_ClearODisableNPP(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
		XilCdo_ClearODisableAXI(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NPS]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NPS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_IDB]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_IDB_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NCRB]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NCRB_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NSU]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NMU]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
	}
}

void XilCdoProcAMSModules(void)
{
	u32 Count;

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_AMS_ROOT]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_AMS_ROOT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_AMS_SAT]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_AMS_SAT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_AMS_SAT]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_AMS_SAT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_AMS_SAT]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_AMS_SAT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_AMS_SAT]; ++Count)
	{
		XilCdo_ClearHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_AMS_SAT_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_AMS_SAT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_AMS_ROOT]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_AMS_ROOT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_AMS_ROOT]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_AMS_ROOT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_AMS_ROOT]; ++Count)
	{
		XilCdo_ClearODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_AMS_ROOT_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_AMS_ROOT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_AMS_ROOT]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_AMS_ROOT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_AMS_SAT]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_AMS_SAT_BaseAddr[Count]);
	}
}


void XilCdoProcGTModules(void)
{
	u32 Count;

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIPE]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIPE_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIPE]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIPE_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIPE]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIPE_NpiParam[Count] &
					XILCDO_NPI_FABRICEN_MASK) == XILCDO_NPI_FABRICEN_MASK)
		{
			XilCdo_SetFabricEnable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIPE_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIPE]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIPE_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIPE_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_GT]; ++Count)
	{
		XilCdo_ClearGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_GT]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_NpiParam[Count] &
					XILCDO_NPI_FABRICEN_MASK) == 0U)
		{
			XilCdo_SetFabricEnable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
		}

		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_NpiParam[Count] &
					XILCDO_NPI_APBEN_MASK) == XILCDO_NPI_APBEN_MASK)
		{
			XilCdo_SetApbEnable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_GT]; ++Count)
	{
		XilCdo_ClearTriState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_GT]; ++Count)
	{
		XilCdo_ClearInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_GT]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIPE]; ++Count)
	{
		XilCdo_SetLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIPE_BaseAddr[Count]);
	}
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
	XStatus Status;
	if((NpiCmd != 0U) &&( NpiCmd != CMD_NPI_SHUTDN))
	{
		XilCdo_RunPendingNpiSeq();
	}
	NpiCmd = CMD_NPI_SHUTDN;
	Status = XilCdo_StoreNpiParams(CmdArgs);
	CmdArgs[9U] = CMD_NPI_SHUTDN_ARGS;
	return Status;
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
		Xil_Out32(DestAddr + (ProcWords<<2U), RegVal);
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
 * This function does the required Pre Configration for NPI blocks
 *
 * @param       Command Args
 *
 * @return      none
 *
 *****************************************************************************/
XStatus XilCdo_NpiPreCfg(u32 CmdArgs[10U])
{
	u32 BaseAddr = CmdArgs[0U];
	u32 NpiParam = CmdArgs[1U];
	u32 BlockType;
	XStatus Status;

	CmdArgs[9U] = CMD_NPI_PRECFG_ARGS;

	BlockType = NpiParam & XILCDO_NPI_BLK_MASK;

	switch(BlockType)
	{
		case XILCDO_NPI_BLK_GT:
			{
				Status = XilCdo_NpiPreCfg_GTY(BaseAddr, NpiParam);
			}
			break;

		case XILCDO_NPI_BLK_DDRMC:
			{
				Status = XilCdo_ScanClear(NpiParam);
				if(Status != XST_SUCCESS)
				{
					goto END;
				}

				Status = XilCdo_NpiPreCfg_DDRMC(BaseAddr, NpiParam);
			}
			break;

		case XILCDO_NPI_BLK_ME_NPI:
			{
				Status = XilCdo_NpiPreCfg_ME(BaseAddr, NpiParam);
			}
			break;

		case XILCDO_NPI_BLK_NOC_NPS:
		case XILCDO_NPI_BLK_NOC_NCRB:
		case XILCDO_NPI_BLK_NOC_NSU:
		case XILCDO_NPI_BLK_NOC_IDB:
			{
				Status = XilCdo_ScanClear(NpiParam);
			}
			break;

		case XILCDO_NPI_BLK_NOC_NMU:
			{
				Status = XilCdo_ScanClear(NpiParam);
				if(Status != XST_SUCCESS)
				{
					goto END;
				}

				Status = XilCdo_NpiPreCfg_NOC_NMU(BaseAddr, NpiParam);
			}
			break;

		default:
			{
				Status = XST_SUCCESS;
			}
			break;
	}
END:
	return Status;
}

XStatus XilCdo_NpiPreCfg_GTY(u32 BaseAddr, u32 NpiParam)
{
	u32 Status;
	XilCdo_ClearLockState(BaseAddr);
	XilCdo_ClearInitCtrl(BaseAddr);
#if XPMCFW_BISR_ENABLED
	//to do : Load efuse data into BISR cache
	XilCdo_SetBISRTrigger(BaseAddr);
	XilCdo_WaitForBISRDone(BaseAddr);
	Status = XilCdo_CheckBISRPass(BaseAddr);
	if(Status != XST_SUCCESS)
	{
		goto END;
	}
#endif
	if((NpiParam & XILCDO_NPI_MEMCLR_MASK) == XILCDO_NPI_MEMCLR_MASK)
	{
		XilCdo_SetMBISTTrigger(BaseAddr);
		XilCdo_WaitForMBISTDone(BaseAddr);
		Status = XilCdo_CheckMBISTPass(BaseAddr);
		if(Status != XST_SUCCESS)
		{
			goto END;
		}
	}
	else
	{
		Status = XST_SUCCESS;
	}
	XilCdo_ClearGateReg(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
END:
	return Status;
}

XStatus XilCdo_NpiPreCfg_DDRMC(u32 BaseAddr, u32 NpiParam)
{
	u32 Status;
	XilCdo_ClearLockState(BaseAddr);
#if XPMCFW_BISR_ENABLED
	//to do : Load efuse data into BISR cache
	XilCdo_SetBISRTrigger(BaseAddr);
#endif

	if((NpiParam & XILCDO_NPI_MEMCLR_MASK) == XILCDO_NPI_MEMCLR_MASK)
	{
		XilCdo_SetMBISTTrigger(BaseAddr);
		XilCdo_WaitForMBISTDone(BaseAddr);
		Status = XilCdo_CheckMBISTPass(BaseAddr);
		if(Status != XST_SUCCESS)
		{
			goto END;
		}
	}
	else
	{
		Status = XST_SUCCESS;
	}
/** Workaround for EDT-990664 */
//	XilCdo_SetHoldState(BaseAddr);
	XilCdo_ClearUBInitState(BaseAddr);
	XilCdo_SetLockState(BaseAddr);
END:
	return Status;
}

XStatus XilCdo_NpiPreCfg_ME(u32 BaseAddr, u32 NpiParam)
{
	u32 Status = XilCdo_ChkMEPwrSupply(BaseAddr);
	if(Status)
	{
		XilCdo_ClearLockState(BaseAddr);
		XilCdo_ClearME_POR(BaseAddr);
#if XPMCFW_BISR_ENABLED
		//to do : Load efuse data into BISR cache
#endif
		XilCdo_ClearODisable1(BaseAddr);
		Status = XilCdo_ScanClearME(BaseAddr);
		if(Status != XST_SUCCESS)
		{
			goto END;
		}
		XilCdo_ClearODisable0(BaseAddr);
		XilCdo_ClearGateReg(BaseAddr);
		XilCdo_ClearInitState(BaseAddr);
		XilCdo_ClearMEArrayReset(BaseAddr);
		XilCdo_SetMemClearEnAll(BaseAddr);
		XilCdo_ClearOD_MBIST_ASYNC_RESET(BaseAddr);
		XilCdo_SetOD_BIST_SETUP1(BaseAddr);
		if((NpiParam & XILCDO_NPI_MEMCLR_MASK) == XILCDO_NPI_MEMCLR_MASK)
		{
			XilCdo_SetMBISTTrigger(BaseAddr);
			XilCdo_WaitForMBISTDone(BaseAddr);
			Status = XilCdo_CheckMBISTPass(BaseAddr);
			if(Status != XST_SUCCESS)
			{
				goto END;
			}
		}
		else
		{
			Status = XST_SUCCESS;
		}
		XilCdo_SetOD_MBIST_ASYNC_RESET(BaseAddr);
		XilCdo_ClearOD_BIST_SETUP1(BaseAddr);
		XilCdo_ClearMBISTTrigger(BaseAddr);
		XilCdo_SetLockState(BaseAddr);
	}
END:
	return Status;
}

XStatus XilCdo_NpiPreCfg_NOC_NMU(u32 BaseAddr, u32 NpiParam)
{
	u32 Status;
	XilCdo_ClearLockState(BaseAddr);
	if((NpiParam & XILCDO_NPI_MEMCLR_MASK) == XILCDO_NPI_MEMCLR_MASK)
	{
		XilCdo_SetMBISTTrigger(BaseAddr);
		XilCdo_WaitForMBISTDone(BaseAddr);
		Status = XilCdo_CheckMBISTPass(BaseAddr);
		if(Status != XST_SUCCESS)
		{
			goto END;
		}
	}
	else
	{
		Status = XST_SUCCESS;
	}
	XilCdo_SetLockState(BaseAddr);
END:
	return Status;
}

void XilCdo_AssertGlobalSignals()
{
	XilCdo_EnableCFUWrite();
	XilCdo_SetGWE();
	XilCdo_SetEOS();
	XilCdo_ClearEnGlob();
	XilCdo_DisableCFUWrite();
}

void XilCdo_ShutIOModules(void)
{
	u32 Count;
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VREF]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VREF_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DDRMC]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DDRMC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPHY]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO_IOMISC]; ++Count)
	{

		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_PLL_PHY]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_NpiParam[Count] &
					XILCDO_NPI_PR_FREEZE_MASK) == XILCDO_NPI_PR_FREEZE_MASK)
		{
			XilCdo_SetPRFreeze(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
		XilCdo_SetInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);

		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_NpiParam[Count] &
			XILCDO_NPI_DDRMC_PRESENT_MASK) == XILCDO_NPI_DDRMC_PRESENT_MASK)
		{
			XilCdo_ClearApbEnable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
		}
		else
		{
			XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_PLL_PHY]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_NpiParam[Count] &
			XILCDO_NPI_DDRMC_PRESENT_MASK) == XILCDO_NPI_DDRMC_PRESENT_MASK)
		{
			XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr[Count]);
			XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_PLL_PHY]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_NpiParam[Count] &
			XILCDO_NPI_DDRMC_PRESENT_MASK) == 0U)
		{
			XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr[Count]);
			XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DDRMC]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DDRMC_BaseAddr[Count]);
		XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DDRMC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DDRMC]; ++Count)
	{
		XilCdo_SetInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DDRMC_BaseAddr[Count]);
		XilCdo_SetUBInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DDRMC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DDRMC]; ++Count)
	{
		XilCdo_SetStartCal(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DDRMC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
		XilCdo_SetTriState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_NpiParam[Count] &
				XILCDO_NPI_DDRMC_PRESENT_MASK) == XILCDO_NPI_DDRMC_PRESENT_MASK)
		{
			XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_NpiParam[Count] & XILCDO_NPI_DDRMC_PRESENT_MASK) == 0U)
		{
			XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
			XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VREF]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VREF_NpiParam[Count] &
					XILCDO_NPI_DDRMC_PRESENT_MASK) == XILCDO_NPI_DDRMC_PRESENT_MASK)
		{
			XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VREF_BaseAddr[Count]);
			XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VREF_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO_IOMISC]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_NpiParam[Count] &
					XILCDO_NPI_DDRMC_PRESENT_MASK) == 0U)
		{
			XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr[Count]);
			XilCdo_SetInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr[Count]);
			XilCdo_SetDCIOfcReset(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO_IOMISC]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_NpiParam[Count] &
					XILCDO_NPI_DDRMC_PRESENT_MASK) == 0U)
		{
			XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr[Count]);
			XilCdo_SetHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VREF]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VREF_NpiParam[Count] &
					XILCDO_NPI_DDRMC_PRESENT_MASK) == 0U)
		{
			XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VREF_BaseAddr[Count]);
			XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VREF_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIO]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_NpiParam[Count] &
						XILCDO_NPI_DDRMC_PRESENT_MASK) == XILCDO_NPI_DDRMC_PRESENT_MASK)
		{
			XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIO_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPHY]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_BaseAddr[Count]);
		XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_BaseAddr[Count]);
		XilCdo_SetTriState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPHY]; ++Count)
	{
		XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_BaseAddr[Count]);
		XilCdo_SetInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_BaseAddr[Count]);
		XilCdo_SetHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPHY_BaseAddr[Count]);
	}
}

void XilCdo_ShutMMCM_CLK(void)
{
	u32 Count;

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VERT_TO_HSR]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VERT_TO_HSR_GT]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_VRT]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_VRT_GT]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_HSR_BUFGS]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_HSR_BUFGS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC_PS]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_PS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_CLK_GT]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_CLK_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_MMCM]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_MMCM_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DPLL]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_PLL_PHY]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr[Count]);
	}

	/* Init state de-asserted in BUFGS and active deskew state machines */
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_HSR_BUFGS]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_HSR_BUFGS_BaseAddr[Count]);
		XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_HSR_BUFGS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC_PS]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_PS_BaseAddr[Count]);
		XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_PS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_CLK_GT]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_CLK_GT_BaseAddr[Count]);
		XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_CLK_GT_BaseAddr[Count]);
	}
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_HSR_BUFGS]; ++Count)
	{
		XilCdo_SetHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_HSR_BUFGS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC_PS]; ++Count)
	{
		XilCdo_SetHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_PS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_CLK_GT]; ++Count)
	{
		XilCdo_SetHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_CLK_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_MMCM]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_MMCM_BaseAddr[Count]);
		XilCdo_SetInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_MMCM_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DPLL]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DPLL_BaseAddr[Count]);
		XilCdo_SetInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DPLL_BaseAddr[Count]);
		XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
		XilCdo_SetInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
		XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_PLL_PHY]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_PLL_PHY]; ++Count)
	{
		XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_PLL_PHY_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_MMCM]; ++Count)
	{
		XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_MMCM_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_DPLL]; ++Count)
	{
		XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_DPLL_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPLL]; ++Count)
	{
		XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPLL_BaseAddr[Count]);
	}



	/* Set ODisable for all CLK distribution elements */
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VERT_TO_HSR]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_BaseAddr[Count]);
		XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VERT_TO_HSR_GT]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_GT_BaseAddr[Count]);
		XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VERT_TO_HSR_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_VRT]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_BaseAddr[Count]);
		XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_VRT_GT]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_GT_BaseAddr[Count]);
		XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_VRT_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME_BaseAddr[Count]);
		XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_VNOC]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_BaseAddr[Count]);
		XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_VNOC_BaseAddr[Count]);
	}
}

void XilCdo_ShutNocModules(void)
{
	u32 Count;
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NPS]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NPS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_IDB]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_IDB_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NCRB]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NCRB_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NSU]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NMU]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NPS]; ++Count)
	{
		XilCdo_SetCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NPS_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NMU]; ++Count)
	{
		XilCdo_SetAxiReqRejectMode(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
	}
	usleep(5U);

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NMU]; ++Count)
	{
		XilCdo_CheckRegBusy(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NMU]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
		XilCdo_SetODisableAXI(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
		XilCdo_SetODisableNPP(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_NpiParam[Count] &
					XILCDO_NPI_FABRICEN_MASK) == XILCDO_NPI_FABRICEN_MASK)
		{
			XilCdo_ClearFabricEnable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NMU]; ++Count)
	{
		XilCdo_SetInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
		XilCdo_SetHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NMU]; ++Count)
	{
		XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NMU]; ++Count)
	{
		XilCdo_ClearAxiReqRejectMode(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NMU_BaseAddr[Count]);
	}
	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NSU]; ++Count)
	{
		XilCdo_SetPRMode(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
	}

	usleep(5U);

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NSU]; ++Count)
	{
		XilCdo_CheckRegPendingBurst(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NSU]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
		XilCdo_SetODisableAXI(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
		XilCdo_SetODisableNPP(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_NpiParam[Count] &
					XILCDO_NPI_FABRICEN_MASK) == XILCDO_NPI_FABRICEN_MASK)
		{
			XilCdo_ClearFabricEnable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NSU]; ++Count)
	{
		XilCdo_SetInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
		XilCdo_SetHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NSU]; ++Count)
	{
		XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NSU]; ++Count)
	{
		XilCdo_ClearPRMode(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NSU_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NCRB]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NCRB_BaseAddr[Count]);
		XilCdo_SetODisable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NCRB_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NCRB]; ++Count)
	{
		XilCdo_SetInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NCRB_BaseAddr[Count]);
		XilCdo_SetHoldState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NCRB_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_NOC_NCRB]; ++Count)
	{
		XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_NOC_NCRB_BaseAddr[Count]);
	}
}

void XilCdo_ShutGTModules(void)
{
	u32 Count;

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIPE]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIPE_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_GT]; ++Count)
	{
		XilCdo_ClearLockState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_GT]; ++Count)
	{
		XilCdo_SetShutDown(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_GT]; ++Count)
	{
		XilCdo_CheckShutDown(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_GT]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
		XilCdo_SetTriState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_GT]; ++Count)
	{
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_NpiParam[Count] &
					XILCDO_NPI_FABRICEN_MASK) == XILCDO_NPI_FABRICEN_MASK)
		{
			XilCdo_ClearFabricEnable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
		}
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_NpiParam[Count] &
					XILCDO_NPI_APBEN_MASK) == XILCDO_NPI_APBEN_MASK)
		{
			XilCdo_ClearApbEnable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
		}
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_GT]; ++Count)
	{
		XilCdo_SetInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
		XilCdo_SetInitCtrl(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
		XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
		XilCdo_ClearShutDown(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_GT_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIPE]; ++Count)
	{
		XilCdo_ClearCompleteState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIPE_BaseAddr[Count]);
		XilCdo_SetInitState(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIPE_BaseAddr[Count]);
	}

	for(Count =0; Count < XilCdoNpiSeqInstPtr->BaseAddrCnt[XILCDO_NPI_BLK_XPIPE]; ++Count)
	{
		XilCdo_SetGateReg(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIPE_BaseAddr[Count]);
		if((XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIPE_NpiParam[Count] &
					XILCDO_NPI_FABRICEN_MASK) == XILCDO_NPI_FABRICEN_MASK)
		{
			XilCdo_ClearFabricEnable(XilCdoNpiSeqInstPtr->XILCDO_NPI_BLK_XPIPE_BaseAddr[Count]);
		}
	}

}
