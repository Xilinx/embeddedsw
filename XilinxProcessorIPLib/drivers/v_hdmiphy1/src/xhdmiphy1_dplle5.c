/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xhdmiphy1_dplle5.c
 *
 * Contains a minimal set of functions for the XHdmiphy1 driver that allow
 * access to all of the Video PHY core's functionality. See xhdmiphy1.h for a
 * detailed description of the driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 *            dd/mm/yy
 * ----- ---- -------- -----------------------------------------------
 * 1.0   ssh  10/09/24 Initial release.
 *
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xhdmiphy1.h"
#include "xhdmiphy1_i.h"
#if (XPAR_HDMIPHY_SS_0_HDMI_GT_CONTROLLER_TX_CLK_PRIMITIVE == 1 || \
		XPAR_HDMIPHY_SS_0_HDMI_GT_CONTROLLER_RX_CLK_PRIMITIVE == 1)
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE5) || \
     (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYP))

#define XHDMIPHY1_MMCM_DRP_CLKFBOUT_1_REG 0x0E//0x38
#define XHDMIPHY1_MMCM_DRP_DIVCLK_DIVIDE_REG 0x19//0x64
#define XHDMIPHY1_MMCM_DRP_CLKOUT0_REG1 0xF//0x3C
#define XHDMIPHY1_MMCM_DRP_CLKOUT1_REG1 0x11//0x44


#define XHDMIPHY1_MMCM_WRITE_VAL     	0xFFFF
#define XHDMIPHY1_MMCM_MULT_MASK	    0x7FC0
#define XHDMIPHY1_MMCM_CLKOUT_MASK	    0x01FF
#define XHDMIPHY1_MMCM_CLKOUTEN_MASK    0x0800
#define XHDMIPHY1_MMCM_DIVCLKDIV_MASK	0x01FF
#define XHDMIPHY1_MMCM_Lock_MASK	    0x0001



/**************************** Function Prototypes *****************************/

/************************** Constant Definitions ******************************/

/*****************************************************************************/
/**
* This function will write the mixed-mode clock manager (MMCM) values currently
* stored in the driver's instance structure to hardware .
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
*
* @return
*		- XST_SUCCESS if the MMCM write was successful.
*		- XST_FAILURE otherwise, if the configuration success bit did
*		  not go low.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_MmcmWriteParameters(XHdmiphy1 *InstancePtr, u8 QuadId,
							XHdmiphy1_DirectionType Dir)
{
	u8 ChId;
	u16 DrpRdVal;
	u16 DrpRdVal_temp;

	XHdmiphy1_Mmcm *MmcmParams;

    ChId = (Dir == XHDMIPHY1_DIR_TX) ?
						XHDMIPHY1_CHANNEL_ID_TXMMCM :
						XHDMIPHY1_CHANNEL_ID_RXMMCM;

	MmcmParams = &InstancePtr->Quads[QuadId].Mmcm[Dir];

	/* Check Parameters if has been Initialized */
	if (!MmcmParams->DivClkDivide && !MmcmParams->ClkFbOutMult &&
			!MmcmParams->ClkOut0Div && !MmcmParams->ClkOut1Div &&
			!MmcmParams->ClkOut2Div) {
		return XST_FAILURE;
	}





	/* Write CLKFBOUT_1 & CLKFBOUT_2 Values */
	DrpRdVal_temp = MmcmParams->ClkFbOutMult;
	DrpRdVal_temp <<= 6;



	XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XHDMIPHY1_MMCM_DRP_CLKFBOUT_1_REG, &DrpRdVal);

	DrpRdVal &= ~XHDMIPHY1_MMCM_MULT_MASK;
	DrpRdVal |= DrpRdVal_temp;

	XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XHDMIPHY1_MMCM_DRP_CLKFBOUT_1_REG,
						(u16)(DrpRdVal));




	/* Write DIVCLK_DIVIDE */
	DrpRdVal_temp = MmcmParams->DivClkDivide;


	XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XHDMIPHY1_MMCM_DRP_DIVCLK_DIVIDE_REG, &DrpRdVal);

	DrpRdVal &= ~XHDMIPHY1_MMCM_DIVCLKDIV_MASK;
	DrpRdVal |= DrpRdVal_temp;

	XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XHDMIPHY1_MMCM_DRP_DIVCLK_DIVIDE_REG,
						(u16)((DrpRdVal)));

	XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XHDMIPHY1_MMCM_DRP_DIVCLK_DIVIDE_REG, &DrpRdVal);


	/* Write CLKOUT0_1 & CLKOUT0_2 Values */
	DrpRdVal_temp = MmcmParams->ClkOut0Div;
	XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XHDMIPHY1_MMCM_DRP_CLKOUT0_REG1, &DrpRdVal);



	DrpRdVal &= ~XHDMIPHY1_MMCM_CLKOUT_MASK;
		DrpRdVal |= DrpRdVal_temp;

	XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XHDMIPHY1_MMCM_DRP_CLKOUT0_REG1,
						(u16)(DrpRdVal));


	/* Write CLKOUT1_1 & CLKOUT1_2 Values */
	DrpRdVal_temp = MmcmParams->ClkOut1Div;

	XHdmiphy1_DrpRd(InstancePtr, QuadId, ChId, XHDMIPHY1_MMCM_DRP_CLKOUT1_REG1, &DrpRdVal);



	DrpRdVal &= ~XHDMIPHY1_MMCM_CLKOUT_MASK;
			DrpRdVal |= DrpRdVal_temp;


		XHdmiphy1_DrpWr(InstancePtr, QuadId, ChId, XHDMIPHY1_MMCM_DRP_CLKOUT1_REG1,
							(u32)(DrpRdVal));



	return XST_SUCCESS;
}

#endif
#endif
