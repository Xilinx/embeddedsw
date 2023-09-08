/******************************************************************************
* Copyright (C) 2018 – 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file onsemi_nb7nq621m.h
* @addtogroup ONSEMI_NB7NQ621M
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

#ifndef ONSEMI_NB7NQ621M_H
#define ONSEMI_NB7NQ621M_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xil_types.h"
#include "sleep.h"
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
	defined (XPS_BOARD_VCK190) || \
	defined (XPS_BOARD_VEK280)
#include "xiicps.h"
#include "xiic.h"
#else
#include "xiic.h"
#endif

#if (XPAR_HDMIPHY1_0_TRANSCEIVER == 6) /*GTYE4*/
#define XPS_BOARD_VCU118
#else
/* Place-holder for other boards in future */
#endif

/************************** Constant Definitions *****************************/
#define REG04_BIT7_LANE_CTRL_GLOBAL		0
#define REG04_BIT7_LANE_CTRL_INDIVIDUAL	1

#define REG04_BIT6_LANE_SWAP_DISABLE	0
#define REG04_BIT6_LANE_SWAP_ENABLE		1

#define REG04_BIT53_MODE_TMDS_HIZ		0
#define REG04_BIT53_MODE_TMDS_300		1
#define REG04_BIT53_MODE_TMDS_200		2
#define REG04_BIT53_MODE_TMDS_100		3
#define REG04_BIT53_MODE_FRL_DC			4
#define REG04_BIT53_MODE_FRL_AC			6
#define REG04_BIT53_MODE_ML				7

/**************************** Type Definitions *******************************/
/**
* FRL Character Rate Enumeration
*/
typedef enum {
	TX_R0_TMDS = 0,
	TX_R0_TMDS_14_L = 21,
	TX_R0_TMDS_14_H = 33,
	TX_R0_TMDS_20 = 45,
	TX_R0_FRL = 57,
	RX_R0 = 69,
	TX_R1_TMDS_14_LL = 90,
	TX_R1_TMDS_14_L = 99,
	TX_R1_TMDS_14 = 108,	/* HDMI 1.4 */
	TX_R1_TMDS_20 = 117,	/* HDMI 2.0 */
	TX_R1_FRL = 126,
	TX_R1_FRL_10G = 135,
	TX_R1_FRL_12G = 144,
	RX_R1_TMDS_14 = 153,
	RX_R1_TMDS_20 = 162,
	RX_R1_FRL = 171,
	TX_R2_TMDS_14_L = 180,
	TX_R2_TMDS_14_H = 196,
	TX_R2_TMDS_20 = 208,
	TX_R2_FRL = 220,
	RX_R2_TMDS_14 = 232,
	RX_R2_TMDS_20 = 241,
	RX_R2_FRL = 250,
	/* Above these were all early versions of
	 * OnSemi re-driver
	 * All the 21 write registers are added for flexibility
	 */
	TX_R3_TMDS_14_L = 271,
	TX_R3_TMDS_14_H = TX_R3_TMDS_14_L + 21,
	TX_R3_TMDS_20 = TX_R3_TMDS_14_H + 21,
	TX_R3_FRL = TX_R3_TMDS_20 + 21,
	RX_R3_TMDS_14 = TX_R3_FRL + 21,
	RX_R3_TMDS_20 = RX_R3_TMDS_14 + 21,
	RX_R3_FRL = RX_R3_TMDS_20 + 21,
} Onsemi_DeviceType;

/**
* This typedef contains translations of FRL_Rate to Lanes and Line Rates.
*/
typedef struct {
	u16 DeviceType;		/**< Device Type */
	u8 Address;		/**< Line Rate */
	u8 Values;
} Onsemi_RegisterField;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int ONSEMI_NB7NQ621M_Init(void *IicPtr, u8 I2CSlaveAddress,
		u8 Revision, u8 IsTx);
int ONSEMI_NB7NQ621M_CheckDeviceID(void *IicPtr, u8 I2CSlaveAddress);
u8 ONSEMI_NB7NQ621M_CheckDeviceVersion(void *IicPtr, u8 I2CSlaveAddress);
int ONSEMI_NB7NQ621M_LineRateReconfig(void *IicPtr, u8 I2CSlaveAddress,
		u8 Revision, u8 IsFRL, u64 LineRate, u8 IsTx);
void ONSEMI_NB7NQ621M_RegisterDump(void *IicPtr, u8 I2CSlaveAddress);
void ONSEMI_NB7NQ621M_RegisterLibraryDump(void);

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* ONSEMI_NB7NQ621M_H */
/** @} */
