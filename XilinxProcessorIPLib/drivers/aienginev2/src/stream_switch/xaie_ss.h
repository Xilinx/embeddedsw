/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_ss.h
* @{
*
* Header file for stream switch implementations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   03/20/2020  Remove range apis
* 1.2   Tejus   03/21/2020  Add stream switch packet switch mode apis
* </pre>
*
******************************************************************************/
#ifndef XAIESS_H
#define XAIESS_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaiegbl_defs.h"
#include "xaie_helper.h"

/**************************** Type Definitions *******************************/
/* Typedef to capture Packet drop header */
typedef enum {
	XAIE_SS_PKT_DONOT_DROP_HEADER,
	XAIE_SS_PKT_DROP_HEADER
} XAie_StrmSwPktHeader;

/************************** Function Prototypes  *****************************/
AieRC XAie_StrmConnCctEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum);

AieRC XAie_StrmConnCctDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum);

AieRC XAie_StrmPktSwMstrPortEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Master, u8 MstrPortNum,
		XAie_StrmSwPktHeader DropHeader, u8 Arbitor, u8 MSelEn);

AieRC XAie_StrmPktSwMstrPortDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Master, u8 MstrPortNum);

AieRC XAie_StrmPktSwSlavePortEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum);

AieRC XAie_StrmPktSwSlavePortDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum);

AieRC XAie_StrmPktSwSlaveSlotEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, u8 SlotNum,
		XAie_Packet Pkt, u8 Mask, u8 MSel, u8 Arbitor);

AieRC XAie_StrmPktSwSlaveSlotDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, u8 SlotNum);
AieRC XAie_StrmSwLogicalToPhysicalPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_StrmPortIntf Port, StrmSwPortType PortType, u8 PortNum,
		u8 *PhyPortId);
AieRC XAie_StrmSwPhysicalToLogicalPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_StrmPortIntf Port, u8 PhyPortId, StrmSwPortType *PortType,
		u8 *PortNum);
AieRC XAie_StrmSwDeterministicMergeConfig(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Arbitor, StrmSwPortType Slave, u8 PortNum,
		u8 PktCount, u8 Position);
AieRC XAie_StrmSwDeterministicMergeEnable(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Arbitor);
AieRC XAie_StrmSwDeterministicMergeDisable(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Arbitor);

#endif		/* end of protection macro */
