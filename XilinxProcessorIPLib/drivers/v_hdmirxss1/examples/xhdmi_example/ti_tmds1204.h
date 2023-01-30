/******************************************************************************
* Copyright (C) 2018 – 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file ti_tmds1204.h
* @addtogroup TI_TMDS1204
* @{
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* X.XX  XX     YY/MM/DD
* 1.00  YH     18/12/24 Initial release.
* </pre>
*
******************************************************************************/
#include "xparameters.h"

#if defined (XPS_BOARD_VEK280_ES)
#ifndef TI_TMDS1204_H
#define TI_TMDS1204_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "sleep.h"
#if (defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
	defined (XPS_BOARD_VCK190))
#include "xiicps.h"
#else
#include "xiic.h"
#endif

#if (XPAR_HDMIPHY1_0_TRANSCEIVER == 6) /*GTYE4*/
#define XPS_BOARD_VCU118
#else
/* Place-holder for other boards in future */
#endif

/**************************** Type Definitions *******************************/
/**
* FRL Character Rate Enumeration
*/
typedef enum {
	TX_TI_R1_INIT = 0, // program 6 registers
	TX_TI_TMDS_14_L_R1 = TX_TI_R1_INIT + 7, // 13 registers are programmed
	TX_TI_TMDS_14_H_R1 = TX_TI_TMDS_14_L_R1 + 13,
	TX_TI_TMDS_20_R1 = TX_TI_TMDS_14_H_R1 + 13,
	TX_TI_FRL_3G_R1 = TX_TI_TMDS_20_R1 + 13,
	TX_TI_FRL_6G_3_R1 = TX_TI_FRL_3G_R1 + 13,
	TX_TI_FRL_6G_4_R1 = TX_TI_FRL_6G_3_R1 + 13,
	TX_TI_FRL_8G_R1 = TX_TI_FRL_6G_4_R1 + 13,
	TX_TI_FRL_10G_R1 = TX_TI_FRL_8G_R1 + 13,
	TX_TI_FRL_12G_R1 = TX_TI_FRL_10G_R1 + 13,

	RX_TI_R1_INIT = TX_TI_FRL_12G_R1 + 13,
	RX_TI_TMDS_14_L_R1 = RX_TI_R1_INIT + 9,
	RX_TI_TMDS_14_H_R1 = RX_TI_TMDS_14_L_R1 + 14 - 1,
	RX_TI_TMDS_20_R1 = RX_TI_TMDS_14_H_R1 + 14 - 1,
	RX_TI_FRL_3G_R1 = RX_TI_TMDS_20_R1 + 14 - 1,
	RX_TI_FRL_6G_3_R1 = RX_TI_FRL_3G_R1 + 14 - 1,
	RX_TI_FRL_6G_4_R1 = RX_TI_FRL_6G_3_R1 + 14 - 1,
	RX_TI_FRL_8G_R1 = RX_TI_FRL_6G_4_R1 + 14 - 1,
	RX_TI_FRL_10G_R1 = RX_TI_FRL_8G_R1 + 14 - 1,
	RX_TI_FRL_12G_R1 = RX_TI_FRL_10G_R1 + 14 - 1,

} Ti_DeviceType;

/**
* This typedef contains translations of FRL_Rate to Lanes and Line Rates.
*/
typedef struct {
	u16 DeviceType;		/**< Device Type */
	u8 Address;		/**< Line Rate */
	u8 Values;
} Ti_RegisterField;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int TI_TMDS1204_Init(void *IicPtr, u8 I2CSlaveAddress,
		u8 Revision, u8 IsTx);
int TI_TMDS1204_CheckDeviceID(void *IicPtr, u8 I2CSlaveAddress);
u8 TI_TMDS1204_CheckDeviceVersion(void *IicPtr, u8 I2CSlaveAddress);
int TI_TMDS1204_LineRateReconfig(void *IicPtr, u8 I2CSlaveAddress,
		u8 Revision, u8 IsFRL, u64 LineRate, u8 Lanes, u8 IsTx);
void TI_TMDS1204_RegisterDump(void *IicPtr, u8 I2CSlaveAddress);
void TI_TMDS1204_RegisterLibraryDump(void);

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif
#endif /* TI_TMDS1204_H */
/** @} */
