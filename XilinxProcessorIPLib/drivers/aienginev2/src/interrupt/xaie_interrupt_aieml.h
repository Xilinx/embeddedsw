/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_interrupt_aieml.h
* @{
*
* Internal header file to capture interrupt APIs specific to AIE ML.
*
******************************************************************************/
#ifndef XAIE_INTERRUPT_AIEML_H
#define XAIE_INTERRUPT_AIEML_H

/***************************** Include Files *********************************/
/**************************** Type Definitions *******************************/
/************************** Function Prototypes  *****************************/
u8 _XAieMl_IntrCtrlL1IrqId(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch);

#endif		/* end of protection macro */
