/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file dppt_vid_phy_config.c
*
* This file contains functions to configure Video Pattern Generator core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  KI    07/13/17 Initial release.
* </pre>
*
******************************************************************************/

#include <stdio.h>
#include "xbasic_types.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xvphy.h"
#include "xspi.h"
#include "dppt_vid_phy_config.h"

int PLL_init_Seting(XSpi *SPI_LMK04906);
void CLK135MHz_Out (XSpi *SPI_LMK04906 , u32 CLKout_Num);
void CLK162MHz_Out (XSpi *SPI_LMK04906 , u32 CLKout_Num);
void LMK04906_init(XSpi *SPI_LMK04906);

	XSpi SPI_LMK04906;  /* SPI Device Point*/
void lmk() {
	LMK04906_init(&SPI_LMK04906);
}


void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate)
{
	switch (link_rate) {
		case 0x6:
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
						ONBOARD_REF_CLK, 135000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
						DP159_FORWARDED_CLK, 81000000);
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
						XVPHY_DP_LINK_RATE_HZ_162GBPS);
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN,
						XVPHY_DP_LINK_RATE_HZ_162GBPS);
			break;
		case 0x14:
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
						ONBOARD_REF_CLK, 135000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
						DP159_FORWARDED_CLK, 270000000);
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
						XVPHY_DP_LINK_RATE_HZ_540GBPS);
			break;
		default:
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
						ONBOARD_REF_CLK, 135000000);
			XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
						DP159_FORWARDED_CLK, 135000000);
			XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
						XVPHY_DP_LINK_RATE_HZ_270GBPS);
			break;
	}
}


u32 PHY_Configuration_Tx(XVphy *InstancePtr,
						XVphy_User_Config PHY_User_Config_Table) {

	XVphy_PllRefClkSelType QpllRefClkSel;
	XVphy_PllRefClkSelType CpllRefClkSel;
	XVphy_PllType TxPllSelect;
	XVphy_PllType RxPllSelect;
	XVphy_ChannelId TxChId;
	//XVphy_ChannelId RxChId;
	u8 QuadId = 0;
	u32 Status;
	u32 Status1;
	u32 Status2;

	QpllRefClkSel   = PHY_User_Config_Table.QPLLRefClkSrc;
	CpllRefClkSel   = PHY_User_Config_Table.CPLLRefClkSrc;
	TxPllSelect     = PHY_User_Config_Table.TxPLL;
	RxPllSelect     = PHY_User_Config_Table.RxPLL;
	TxChId          = PHY_User_Config_Table.TxChId;

	//Set the Ref Clock Frequency and line rates
	XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId, QpllRefClkSel,
								PHY_User_Config_Table.QPLLRefClkFreqHz);
	XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId, CpllRefClkSel,
								PHY_User_Config_Table.CPLLRefClkFreqHz);
	XVphy_CfgLineRate(InstancePtr, QuadId, TxChId,
								PHY_User_Config_Table.LineRateHz);
	XVphy_PllInitialize(InstancePtr, QuadId, TxChId,
						QpllRefClkSel, CpllRefClkSel,
						TxPllSelect, RxPllSelect);

	// Initiate the DRP configuration of GT based on above parameters
	Status = XVphy_ClkInitialize(InstancePtr, QuadId, TxChId, XVPHY_DIR_TX);
	XVphy_ResetGtPll(InstancePtr, QuadId, TxChId, XVPHY_DIR_TX,(TRUE));
	XVphy_ResetGtPll(InstancePtr, QuadId, TxChId, XVPHY_DIR_TX,(FALSE));
	XVphy_ResetGtPll(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX,(FALSE));
	XVphy_ResetGtPll(InstancePtr, 0,
						XVPHY_CHANNEL_ID_CMN, XVPHY_DIR_TX,(FALSE));
	Status1 = XVphy_WaitForPllLock(InstancePtr, QuadId, TxChId);
	Status2 = XVphy_WaitForResetDone(InstancePtr, QuadId, TxChId, XVPHY_DIR_TX);

//        XVphy_DpDebugInfo(&VPhy_Instance, 0, XVPHY_CHANNEL_ID_CH1);
//	xil_printf ("%d %d %d\r\n", Status, Status1, Status2);
	return (Status | Status1 | Status2);
}

u32 PHY_Configuration_Rx(XVphy *InstancePtr,
							XVphy_User_Config PHY_User_Config_Table){

	XVphy_PllRefClkSelType QpllRefClkSel;
	XVphy_PllRefClkSelType CpllRefClkSel;
	XVphy_PllType TxPllSelect;
	XVphy_PllType RxPllSelect;
//	XVphy_ChannelId TxChId;
	XVphy_ChannelId RxChId;
	u8 QuadId = 0;
	u32 Status;

	QpllRefClkSel   = PHY_User_Config_Table.QPLLRefClkSrc;
	CpllRefClkSel   = PHY_User_Config_Table.CPLLRefClkSrc;
	TxPllSelect     = PHY_User_Config_Table.TxPLL;
	RxPllSelect     = PHY_User_Config_Table.RxPLL;
//	TxChId          = PHY_User_Config_Table.TxChId;
	RxChId          = PHY_User_Config_Table.RxChId;

	//Set the Ref Clock Frequency
	XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId, QpllRefClkSel,
								PHY_User_Config_Table.QPLLRefClkFreqHz);
	XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId, CpllRefClkSel,
								PHY_User_Config_Table.CPLLRefClkFreqHz);
	XVphy_CfgLineRate(InstancePtr, QuadId, RxChId,
								PHY_User_Config_Table.LineRateHz);
	// Initialize GT with ref clock and PLL selects
	XVphy_PllInitialize(InstancePtr, QuadId, RxChId,
									QpllRefClkSel, CpllRefClkSel,
									TxPllSelect, RxPllSelect);
	// Initialize GT with ref clock and PLL selects
	Status = XVphy_ClkInitialize(InstancePtr, QuadId, RxChId, XVPHY_DIR_RX);
//	XVphy_ResetGtPll(InstancePtr, QuadId, RxChId, XVPHY_DIR_RX,(TRUE));
	XVphy_ResetGtPll(InstancePtr, QuadId, RxChId, XVPHY_DIR_RX,(FALSE));

	return Status;
}

//Board specific - Ref Clock settings
void Dppt_Tx_SetRefClocks(u8 DPLinkRate_Value, u8 is_TX_CPLL)
{

//	XSpi SPI_LMK04906;  /* SPI Device Point*/
//	LMK04906_init(&SPI_LMK04906);
	PLL_init_Seting(&SPI_LMK04906);
//        // Programs LMK04960B - GTREFCLK0
	if (is_TX_CPLL == 1) {
		CLK135MHz_Out(&SPI_LMK04906, 0);
		xil_printf ("Programming fixed clock of 135Mhz..\r\n");
	} else {
		CLK162MHz_Out(&SPI_LMK04906, 0);
		xil_printf ("Programming fixed clock of 162Mhz..\r\n");
	}
}

void Two_byte_set (XVphy *InstancePtr, u8 Tx_to_two_byte, u8 Rx_to_two_byte)
{
        u16 DrpVal;
        u32 WriteVal;
        u32 Status;

        // Modifying TX data width to 2 byte
        if (Tx_to_two_byte == 1) {
				Status = XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
					0x6B, &DrpVal);
                DrpVal &= ~0x17;
                WriteVal = 0x0;
                WriteVal = DrpVal | 0x3;
				Status = XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
						0x6B, WriteVal);
				Status += XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2,
						0x6B, WriteVal);
				Status += XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3,
						0x6B, WriteVal);
				Status += XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4,
						0x6B, WriteVal);
				if(Status == 0)
                xil_printf ("TX GT Channel put in 2 byte mode\r\n");
        }

        // Modifying RX data width to 2 byte
        if (Rx_to_two_byte == 1) {
				Status = XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
						0x11, &DrpVal);
                DrpVal &= ~0x7800;
                WriteVal = 0x0;
                WriteVal = DrpVal | 0x1800;
				Status = XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
						0x11, WriteVal);
				Status = XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2,
						0x11, WriteVal);
				Status = XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3,
						0x11, WriteVal);
				Status = XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4,
						0x11, WriteVal);
                xil_printf ("RX GT Channel put in 2 byte mode\r\n");
        }
}
