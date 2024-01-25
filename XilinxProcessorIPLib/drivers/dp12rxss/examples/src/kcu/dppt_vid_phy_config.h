/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file dppt_vid_phy_config.h
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
//#include "xbasic_types.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xvphy.h"

typedef enum {
        ONBOARD_REF_CLK = 1,
        DP159_FORWARDED_CLK,
} XVphy_User_GT_RefClk_Src;

typedef struct {
        u8 Index;
        XVphy_PllType  TxPLL;
        XVphy_PllType  RxPLL;
        XVphy_ChannelId TxChId;
        XVphy_ChannelId RxChId;
        u32 LineRate;
        u64 LineRateHz;
        XVphy_User_GT_RefClk_Src QPLLRefClkSrc;
        XVphy_User_GT_RefClk_Src CPLLRefClkSrc;
        u64 QPLLRefClkFreqHz;
        u64 CPLLRefClkFreqHz;
} XVphy_User_Config;



void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate);
u32 PHY_Configuration_Rx(XVphy *InstancePtr,
							XVphy_User_Config PHY_User_Config_Table);
u32 PHY_Configuration_Tx(XVphy *InstancePtr,
							XVphy_User_Config PHY_User_Config_Table);
u32 PHY_tx_reconfig (XVphy *InstancePtr,
					XVphy_ChannelId ChId, XVphy_DirectionType Dir, u8 QuadId);
void Dppt_Tx_SetRefClocks(u8 DPLinkRate_Value, u8 Prog, u8 is_TX_CPLL);
void Dppt_Rx_SetRefClocks(u8 DPLinkRate_Value, u8 Prog);
void Two_byte_set (XVphy *InstancePtr, u8 Tx_to_two_byte, u8 Rx_to_two_byte);
void lmk();
