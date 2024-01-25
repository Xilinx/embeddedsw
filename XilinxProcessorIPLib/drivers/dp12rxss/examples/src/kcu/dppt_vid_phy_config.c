/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
* 1.00  KI   07/13/17 Initial release.
* </pre>
*
******************************************************************************/

#include <stdio.h>
//#include "xbasic_types.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xvphy.h"
#include "xspi.h"
#include "dppt_vid_phy_config.h"

#define CLK135MHz_DIVIDER 18
#define CLK270MHz_DIVIDER 9
#define CLK162MHz_DIVIDER 15

XSpi SPI_LMK04906;  /* SPI Device Point*/

void LMK04906_init(XSpi *SPI_LMK04906);
int PLL_init_Seting(XSpi *SPI_LMK04906);

void lmk() {
       LMK04906_init(&SPI_LMK04906);
}



void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate) {
switch (link_rate) {
case 0x6:
        XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_REF_CLK, 270000000);
        XVphy_CfgQuadRefClkFreq(InstancePtr, 0, DP159_FORWARDED_CLK, 81000000);
        XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
						XVPHY_DP_LINK_RATE_HZ_162GBPS);
        XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1,
						XVPHY_DP_LINK_RATE_HZ_162GBPS);
        break;
case 0x14:
        XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_REF_CLK, 270000000);
        XVphy_CfgQuadRefClkFreq(InstancePtr, 0, DP159_FORWARDED_CLK, 270000000);
        XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
						XVPHY_DP_LINK_RATE_HZ_540GBPS);
        XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1,
						XVPHY_DP_LINK_RATE_HZ_540GBPS);
        break;
default:
        XVphy_CfgQuadRefClkFreq(InstancePtr, 0, ONBOARD_REF_CLK, 270000000);
        XVphy_CfgQuadRefClkFreq(InstancePtr, 0, DP159_FORWARDED_CLK, 135000000);
        XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
						XVPHY_DP_LINK_RATE_HZ_270GBPS);
        XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1,
						XVPHY_DP_LINK_RATE_HZ_270GBPS);
        break;
}
}


u32 PHY_Configuration_Tx(XVphy *InstancePtr,
							XVphy_User_Config PHY_User_Config_Table){

	XVphy_PllRefClkSelType QpllRefClkSel;
	XVphy_PllRefClkSelType CpllRefClkSel;
	XVphy_PllType TxPllSelect;
	XVphy_PllType RxPllSelect;
	XVphy_ChannelId TxChId;
	//XVphy_ChannelId RxChId;
	u8 QuadId = 0;
	u32 Status = XST_FAILURE;
	u32 Status1 = XST_FAILURE;
	u32 Status2 = XST_FAILURE;
	u32 retries = 0;

	QpllRefClkSel   		= PHY_User_Config_Table.QPLLRefClkSrc;
	CpllRefClkSel   		= PHY_User_Config_Table.CPLLRefClkSrc;
	TxPllSelect             = PHY_User_Config_Table.TxPLL;
	RxPllSelect             = PHY_User_Config_Table.RxPLL;
	TxChId                  = PHY_User_Config_Table.TxChId;
//      RxChId                  = PHY_User_Config_Table.RxChId;

			//Set the Ref Clock Frequency
	XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId, QpllRefClkSel,
									PHY_User_Config_Table.QPLLRefClkFreqHz);
	XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId, CpllRefClkSel,
									PHY_User_Config_Table.CPLLRefClkFreqHz);
	XVphy_CfgLineRate(InstancePtr, QuadId, TxChId,
									PHY_User_Config_Table.LineRateHz);

	XVphy_PllInitialize(InstancePtr, QuadId, TxChId,
							QpllRefClkSel, CpllRefClkSel,
							TxPllSelect, RxPllSelect);

	// Initialize GT with ref clock and PLL selects
	// GT DRPs may not get completed if GT is busy doing something else
	// hence this is run in loop and retried 100 times
	while (Status != XST_SUCCESS) {
		Status = XVphy_ClkInitialize(InstancePtr, QuadId, TxChId, XVPHY_DIR_TX);
		if (retries > 100) {
			retries = 0;
			break;
		}
		retries++;
	}
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, 0x14, 0x6);
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, 0x14, 0x0);
	XVphy_ResetGtPll(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CHA,
													XVPHY_DIR_TX,(FALSE));
	XVphy_WaitForPmaResetDone(InstancePtr, QuadId, TxChId, XVPHY_DIR_TX);
	Status1 = XVphy_WaitForPllLock(InstancePtr, QuadId, TxChId);
	Status2 = XVphy_WaitForResetDone(InstancePtr, QuadId, TxChId, XVPHY_DIR_TX);
//xil_printf ("%d %d %d\r\n", Status, Status1, Status2);
	if ((Status | Status1 | Status2) != 0) {
		xil_printf ("++++TX GT config encountered error++++\r\n");
	}
	return (Status | Status1 | Status2);
}

u32 PHY_tx_reconfig (XVphy *InstancePtr, XVphy_ChannelId ChId,
									XVphy_DirectionType Dir, u8 QuadId)
{
    u32 Status = XST_FAILURE;
    u32 Status1 = XST_FAILURE;
    u32 Status2 = XST_FAILURE;
    u32 retries = 0;

	// GT DRPs may not get completed if GT is busy doing something else
	// hence this is run in loop and retried 100 times
	while (Status != XST_SUCCESS) {
		Status = XVphy_ClkInitialize(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CHA,
																XVPHY_DIR_TX);
		if (retries > 100) {
			retries = 0;
			break;
		}
		retries++;
	}

    XVphy_BufgGtReset(InstancePtr, XVPHY_DIR_TX,(TRUE));
    XVphy_ResetGtPll(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX,
					(TRUE));
    XVphy_ResetGtPll(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX,
					(FALSE));
    XVphy_BufgGtReset(InstancePtr, XVPHY_DIR_TX,(FALSE));
    XVphy_ResetGtTxRx(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX,
					FALSE);
    Status1 = XVphy_WaitForPllLock(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CHA);
    Status2 = XVphy_WaitForResetDone(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CHA,
											XVPHY_DIR_TX);
//    xil_printf ("%x %x %x\r\n", Status, Status1, Status2);
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
	TxPllSelect             = PHY_User_Config_Table.TxPLL;
	RxPllSelect             = PHY_User_Config_Table.RxPLL;
//	TxChId                  = PHY_User_Config_Table.TxChId;
	RxChId                  = PHY_User_Config_Table.RxChId;

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
// The following can also be run in loop to ensure that DRPs happen properly
	Status = XVphy_ClkInitialize(InstancePtr, QuadId, RxChId, XVPHY_DIR_RX);
	XVphy_BufgGtReset(InstancePtr, XVPHY_DIR_RX,(TRUE));
	XVphy_ResetGtPll(InstancePtr, QuadId, RxChId, XVPHY_DIR_RX,(TRUE));
	XVphy_ResetGtPll(InstancePtr, QuadId, RxChId, XVPHY_DIR_RX,(FALSE));
	XVphy_BufgGtReset(InstancePtr, XVPHY_DIR_RX,(FALSE));

	return Status;
}

//Board specific - Ref Clock settings
void Dppt_Tx_SetRefClocks(u8 DPLinkRate_Value, u8 Prog, u8 is_TX_CPLL)
{
//	XSpi SPI_LMK04906;  /* SPI Device Point*/

//	PLL_init_Seting(&SPI_LMK04906, CLK270MHz_DIVIDER);
	PLL_init_Seting(&SPI_LMK04906);
	xil_printf("Programming 270 MHz Clock for GTREFCLK0...\r\n");

}

void Two_byte_set (XVphy *InstancePtr, u8 Tx_to_two_byte, u8 Rx_to_two_byte)
{

	u16 DrpVal;
	u32 WriteVal;
	u32 Status;
    if (Tx_to_two_byte == 1) {
		Status = XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
				0x7A, &DrpVal);
        DrpVal &= ~0xF;
        WriteVal = 0x0;
        WriteVal = DrpVal | 0x3;
		Status = XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
				0x7A, WriteVal);
		Status += XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2,
				0x7A, WriteVal);
		Status += XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3,
				0x7A, WriteVal);
		Status += XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4,
				0x7A, WriteVal);
		if(Status == 0)
        xil_printf ("TX GT Channel put in 2 byte mode\r\n");


		Status = XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
				0x85, &DrpVal);
		DrpVal &= ~0xC00;
		WriteVal = 0x0;
        WriteVal = DrpVal | 0x0;
		Status = XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
				0x85, WriteVal);
		Status += XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2,
				0x85, WriteVal);
		Status += XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3,
				0x85, WriteVal);
		Status += XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4,
				0x85, WriteVal);


		xil_printf ("TX Channel configured for 2byte mode\r\n");
    }
    if (Rx_to_two_byte == 1) {
		Status = XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
				0x03, &DrpVal);

		DrpVal &= ~0x1E0;
		WriteVal = 0x0;
		WriteVal = DrpVal | 0x60;
		Status = XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
				0x03, WriteVal);
		Status += XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2,
				0x03, WriteVal);
		Status += XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3,
				0x03, WriteVal);
		Status += XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4,
				0x03, WriteVal);

		Status = XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
				0x66, &DrpVal);

		DrpVal &= ~0x3;
		WriteVal = 0x0;
		WriteVal = DrpVal | 0x0;
		Status = XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
				0x66, WriteVal);
		Status += XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2,
				0x66, WriteVal);
		Status += XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3,
				0x66, WriteVal);
		Status += XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4,
				0x66, WriteVal);

		xil_printf ("RX Channel configured for 2byte mode\r\n");
    }
}
