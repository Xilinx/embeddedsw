/******************************************************************************
* Copyright (C) 2018 – 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file ONSEMI_NB7NQ621M.c
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
* 1.00  GM     19/05/14 Initial release.
* </pre>
*
******************************************************************************/

#include "onsemi_nb7nq621m.h"

#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190)
#define I2C_REPEATED_START 0x01
#define I2C_STOP 0x00
#else
#define I2C_REPEATED_START XIIC_REPEATED_START
#define I2C_STOP XIIC_STOP
#endif

/************************** Constant Definitions ******************************/

/**
* This table contains the values to be programmed to ONSEMI device.
* Each entry is of the format:
* 1) Device Type
* 2) Register Address
* 3) Values
*/
const Onsemi_RegisterField OnsemiRegisterFields[] = {
	{TX_R0_TMDS, 0x04, 0x18},
	{TX_R0_TMDS, 0x05, 0x0B},
	{TX_R0_TMDS, 0x06, 0x00},
	{TX_R0_TMDS, 0x07, 0x00},
	{TX_R0_TMDS, 0x08, 0x03},
	{TX_R0_TMDS, 0x09, 0x20},
	{TX_R0_TMDS, 0x0A, 0x05},
	{TX_R0_TMDS, 0x0B, 0x0F},
	{TX_R0_TMDS, 0x0C, 0xAA},
	{TX_R0_TMDS, 0x0D, 0x00},
	{TX_R0_TMDS, 0x0E, 0x03},
	{TX_R0_TMDS, 0x0F, 0x00},
	{TX_R0_TMDS, 0x10, 0x00},
	{TX_R0_TMDS, 0x11, 0x03},
	{TX_R0_TMDS, 0x12, 0x00},
	{TX_R0_TMDS, 0x13, 0x00},
	{TX_R0_TMDS, 0x14, 0x03},
	{TX_R0_TMDS, 0x15, 0x00},
	{TX_R0_TMDS, 0x16, 0x00},
	{TX_R0_TMDS, 0x17, 0x03},
	{TX_R0_TMDS, 0x18, 0x00},

	{TX_R0_TMDS_14_L, 0x04, 0xB0},
	{TX_R0_TMDS_14_L, 0x09, 0x00},
	{TX_R0_TMDS_14_L, 0x0A, 0x03},
	{TX_R0_TMDS_14_L, 0x0D, 0x02},
	{TX_R0_TMDS_14_L, 0x0E, 0x0F},
	{TX_R0_TMDS_14_L, 0x10, 0x02},
	{TX_R0_TMDS_14_L, 0x11, 0x0F},
	{TX_R0_TMDS_14_L, 0x13, 0x02},
	{TX_R0_TMDS_14_L, 0x14, 0x0F},
	{TX_R0_TMDS_14_L, 0x16, 0x02},
	{TX_R0_TMDS_14_L, 0x17, 0x63},
	{TX_R0_TMDS_14_L, 0x18, 0x0B},

	{TX_R0_TMDS_14_H, 0x04, 0xA0},
	{TX_R0_TMDS_14_H, 0x09, 0x00},
	{TX_R0_TMDS_14_H, 0x0A, 0x03},
	{TX_R0_TMDS_14_H, 0x0D, 0x30},
	{TX_R0_TMDS_14_H, 0x0E, 0x0F},
	{TX_R0_TMDS_14_H, 0x10, 0x30},
	{TX_R0_TMDS_14_H, 0x11, 0x0F},
	{TX_R0_TMDS_14_H, 0x13, 0x30},
	{TX_R0_TMDS_14_H, 0x14, 0x0F},
	{TX_R0_TMDS_14_H, 0x16, 0x02},
	{TX_R0_TMDS_14_H, 0x17, 0x63},
	{TX_R0_TMDS_14_H, 0x18, 0x0B},

	{TX_R0_TMDS_20, 0x04, 0xA0},
	{TX_R0_TMDS_20, 0x09, 0x00},
	{TX_R0_TMDS_20, 0x0A, 0x03},
	{TX_R0_TMDS_20, 0x0D, 0x31},
	{TX_R0_TMDS_20, 0x0E, 0x0F},
	{TX_R0_TMDS_20, 0x10, 0x31},
	{TX_R0_TMDS_20, 0x11, 0x0F},
	{TX_R0_TMDS_20, 0x13, 0x31},
	{TX_R0_TMDS_20, 0x14, 0x0F},
	{TX_R0_TMDS_20, 0x16, 0x02},
	{TX_R0_TMDS_20, 0x17, 0x63},
	{TX_R0_TMDS_20, 0x18, 0x0B},

	{TX_R0_FRL, 0x04, 0x18},
	{TX_R0_FRL, 0x09, 0x20},
	{TX_R0_FRL, 0x0A, 0x05},
	{TX_R0_FRL, 0x0D, 0x00},
	{TX_R0_FRL, 0x0E, 0x03},
	{TX_R0_FRL, 0x10, 0x00},
	{TX_R0_FRL, 0x11, 0x03},
	{TX_R0_FRL, 0x13, 0x00},
	{TX_R0_FRL, 0x14, 0x03},
	{TX_R0_FRL, 0x16, 0x00},
	{TX_R0_FRL, 0x17, 0x03},
	{TX_R0_FRL, 0x18, 0x00},

	{RX_R0, 0x04, 0xB0},
	{RX_R0, 0x05, 0x0D},
	{RX_R0, 0x06, 0x00},
	{RX_R0, 0x07, 0x32},
	{RX_R0, 0x08, 0x0B},
	{RX_R0, 0x09, 0x32},
	{RX_R0, 0x0A, 0x0B},
	{RX_R0, 0x0B, 0x0F},
	{RX_R0, 0x0C, 0xAA},
	{RX_R0, 0x0D, 0x00},
	{RX_R0, 0x0E, 0x03},
	{RX_R0, 0x0F, 0x00},
	{RX_R0, 0x10, 0x00},
	{RX_R0, 0x11, 0x03},
	{RX_R0, 0x12, 0x00},
	{RX_R0, 0x13, 0x00},
	{RX_R0, 0x14, 0x03},
	{RX_R0, 0x15, 0x00},
	{RX_R0, 0x16, 0x00},
	{RX_R0, 0x17, 0x03},
	{RX_R0, 0x18, 0x00},

	/* <= 74.25Mbps */
	{TX_R1_TMDS_14_LL, 0x0A, 0x18},
	{TX_R1_TMDS_14_LL, 0x0B, 0x1F},
	{TX_R1_TMDS_14_LL, 0x0C, 0x00},
	{TX_R1_TMDS_14_LL, 0x0D, 0x30},
	{TX_R1_TMDS_14_LL, 0x0E, 0x05},
	{TX_R1_TMDS_14_LL, 0x0F, 0x20},
	{TX_R1_TMDS_14_LL, 0x10, 0x43},
	{TX_R1_TMDS_14_LL, 0x11, 0x0F},
	{TX_R1_TMDS_14_LL, 0x12, 0xAA},

	/* <= 99Mbps */
	{TX_R1_TMDS_14_L, 0x0A, 0x00},
	{TX_R1_TMDS_14_L, 0x0B, 0x1F},
	{TX_R1_TMDS_14_L, 0x0C, 0x00},
	{TX_R1_TMDS_14_L, 0x0D, 0x10},
	{TX_R1_TMDS_14_L, 0x0E, 0x2A},
	{TX_R1_TMDS_14_L, 0x0F, 0x11},
	{TX_R1_TMDS_14_L, 0x10, 0x43},
	{TX_R1_TMDS_14_L, 0x11, 0x0F},
	{TX_R1_TMDS_14_L, 0x12, 0xAA},

	/* <= 1.48Gbps */
	{TX_R1_TMDS_14, 0x0A, 0x18},
	{TX_R1_TMDS_14, 0x0B, 0x1F},
	{TX_R1_TMDS_14, 0x0C, 0x0D},
	{TX_R1_TMDS_14, 0x0D, 0x10},
	{TX_R1_TMDS_14, 0x0E, 0x2A},
	{TX_R1_TMDS_14, 0x0F, 0x11},
	{TX_R1_TMDS_14, 0x10, 0x43},
	{TX_R1_TMDS_14, 0x11, 0x0F},
	{TX_R1_TMDS_14, 0x12, 0xAA},

	/* <= 5.94 */
	{TX_R1_TMDS_20, 0x0A, 0x18},
	{TX_R1_TMDS_20, 0x0B, 0x0F},
	{TX_R1_TMDS_20, 0x0C, 0x00},
	{TX_R1_TMDS_20, 0x0D, 0x10},
	{TX_R1_TMDS_20, 0x0E, 0x2A},
	{TX_R1_TMDS_20, 0x0F, 0x33},
	{TX_R1_TMDS_20, 0x10, 0x0A},
	{TX_R1_TMDS_20, 0x11, 0x0F},
	{TX_R1_TMDS_20, 0x12, 0xAA},

	{TX_R1_FRL, 0x0A, 0x20},
	{TX_R1_FRL, 0x0B, 0x0F},
	{TX_R1_FRL, 0x0C, 0x00},
	{TX_R1_FRL, 0x0D, 0x10},
	{TX_R1_FRL, 0x0E, 0x2A},
	{TX_R1_FRL, 0x0F, 0x11},
	{TX_R1_FRL, 0x10, 0x0A},
	{TX_R1_FRL, 0x11, 0x0F},
	{TX_R1_FRL, 0x12, 0xAA},

	{TX_R1_FRL_10G, 0x0A, 0x20},
	{TX_R1_FRL_10G, 0x0B, 0x0F},
	{TX_R1_FRL_10G, 0x0C, 0x00},
	{TX_R1_FRL_10G, 0x0D, 0x00},
	{TX_R1_FRL_10G, 0x0E, 0x03},
	{TX_R1_FRL_10G, 0x0F, 0x21},
	{TX_R1_FRL_10G, 0x10, 0x0A},
	{TX_R1_FRL_10G, 0x11, 0x0F},
	{TX_R1_FRL_10G, 0x12, 0xAA},

	{TX_R1_FRL_12G, 0x0A, 0x20},
	{TX_R1_FRL_12G, 0x0B, 0x0F},
	{TX_R1_FRL_12G, 0x0C, 0x00},
	{TX_R1_FRL_12G, 0x0D, 0x00},
	{TX_R1_FRL_12G, 0x0E, 0x03},
#ifdef zcu106
	{TX_R1_FRL_12G, 0x0F, 0x21},
#else
	{TX_R1_FRL_12G, 0x0F, 0x31},
#endif
	{TX_R1_FRL_12G, 0x10, 0x0A},
	{TX_R1_FRL_12G, 0x11, 0x0F},
	{TX_R1_FRL_12G, 0x12, 0xAA},

	{RX_R1_TMDS_14, 0x0A, 0x20},
	{RX_R1_TMDS_14, 0x0B, 0x0F},
	{RX_R1_TMDS_14, 0x0C, 0x00},
	{RX_R1_TMDS_14, 0x0D, 0x00},
	{RX_R1_TMDS_14, 0x0E, 0x03},
	{RX_R1_TMDS_14, 0x0F, 0x21},
	{RX_R1_TMDS_14, 0x10, 0x2A},
	{RX_R1_TMDS_14, 0x11, 0x0F},
	{RX_R1_TMDS_14, 0x12, 0xAA},

	{RX_R1_TMDS_20, 0x0A, 0x20},
	{RX_R1_TMDS_20, 0x0B, 0x0F},
	{RX_R1_TMDS_20, 0x0C, 0x00},
	{RX_R1_TMDS_20, 0x0D, 0x00},
	{RX_R1_TMDS_20, 0x0E, 0x03},
	{RX_R1_TMDS_20, 0x0F, 0x00},
	{RX_R1_TMDS_20, 0x10, 0x00},
	{RX_R1_TMDS_20, 0x11, 0x0F},
	{RX_R1_TMDS_20, 0x12, 0xAA},

	{RX_R1_FRL, 0x0A, 0x20},
	{RX_R1_FRL, 0x0B, 0x0F},
	{RX_R1_FRL, 0x0C, 0x00},
	{RX_R1_FRL, 0x0D, 0x00},
	{RX_R1_FRL, 0x0E, 0x07},
	{RX_R1_FRL, 0x0F, 0x20},
	{RX_R1_FRL, 0x10, 0x01},
	{RX_R1_FRL, 0x11, 0x0F},
	{RX_R1_FRL, 0x12, 0xAA},

	{TX_R2_TMDS_14_L, 0x09, 0x7C},
	{TX_R2_TMDS_14_L, 0x0A, 0x00},
	{TX_R2_TMDS_14_L, 0x0B, 0x0F},
	{TX_R2_TMDS_14_L, 0x0C, 0x00},
	{TX_R2_TMDS_14_L, 0x0D, 0x20},
	{TX_R2_TMDS_14_L, 0x0E, 0x43},
	{TX_R2_TMDS_14_L, 0x0F, 0x20},
	{TX_R2_TMDS_14_L, 0x10, 0x43},
	{TX_R2_TMDS_14_L, 0x11, 0x0F},
	{TX_R2_TMDS_14_L, 0x12, 0xAA},
	{TX_R2_TMDS_14_L, 0x13, 0x02},
	{TX_R2_TMDS_14_L, 0x14, 0x0F},
	{TX_R2_TMDS_14_L, 0x15, 0x00},
	{TX_R2_TMDS_14_L, 0x16, 0x02},
	{TX_R2_TMDS_14_L, 0x17, 0x63},
	{TX_R2_TMDS_14_L, 0x18, 0x0B},

	{TX_R2_TMDS_14_H, 0x09, 0x7C},
	{TX_R2_TMDS_14_H, 0x0A, 0x18},
	{TX_R2_TMDS_14_H, 0x0B, 0x0F},
	{TX_R2_TMDS_14_H, 0x0D, 0x00},
	{TX_R2_TMDS_14_H, 0x0E, 0x43},
	{TX_R2_TMDS_14_H, 0x0F, 0x00},
	{TX_R2_TMDS_14_H, 0x10, 0x47},
	{TX_R2_TMDS_14_H, 0x13, 0x30},
	{TX_R2_TMDS_14_H, 0x14, 0x0F},
	{TX_R2_TMDS_14_H, 0x16, 0x02},
	{TX_R2_TMDS_14_H, 0x17, 0x63},
	{TX_R2_TMDS_14_H, 0x18, 0x0B},

	{TX_R2_TMDS_20, 0x09, 0x7C},
	{TX_R2_TMDS_20, 0x0A, 0x18},
	{TX_R2_TMDS_20, 0x0B, 0x0F},
	{TX_R2_TMDS_20, 0x0D, 0x00},
	{TX_R2_TMDS_20, 0x0E, 0x43},
	{TX_R2_TMDS_20, 0x0F, 0x11},
	{TX_R2_TMDS_20, 0x10, 0x28},
	{TX_R2_TMDS_20, 0x13, 0x30},
	{TX_R2_TMDS_20, 0x14, 0x0F},
	{TX_R2_TMDS_20, 0x16, 0x02},
	{TX_R2_TMDS_20, 0x17, 0x63},
	{TX_R2_TMDS_20, 0x18, 0x0B},

	{TX_R2_FRL, 0x09, 0x7C},
	{TX_R2_FRL, 0x0A, 0x20},
	{TX_R2_FRL, 0x0B, 0x0F},
#ifdef XPS_BOARD_ZCU106
	{TX_R2_FRL, 0x0D, 0x00}, /* Onsemi 0x10}, */
	{TX_R2_FRL, 0x0E, 0x0A}, /* Onsemi 0x2A}, */
	{TX_R2_FRL, 0x0F, 0x31}, /* Onsemi 0x02}, */
	{TX_R2_FRL, 0x10, 0x05},
#elif defined XPS_BOARD_VCK190
	{TX_R2_FRL, 0x0D, 0x00}, /* Onsemi 0x10}, */
	{TX_R2_FRL, 0x0E, 0x0A}, /* Onsemi 0x2A}, */
	{TX_R2_FRL, 0x0F, 0x31}, /* Onsemi 0x02}, */
	{TX_R2_FRL, 0x10, 0x00},
#else
	{TX_R2_FRL, 0x0D, 0x33}, /* Onsemi 0x10}, */
	{TX_R2_FRL, 0x0E, 0x0A}, /* Onsemi 0x2A}, */
	{TX_R2_FRL, 0x0F, 0x33}, /* Onsemi 0x02}, */
	{TX_R2_FRL, 0x10, 0x05},
#endif
	{TX_R2_FRL, 0x13, 0x00},
	{TX_R2_FRL, 0x14, 0x03},
	{TX_R2_FRL, 0x16, 0x00},
	{TX_R2_FRL, 0x17, 0x03},
	{TX_R2_FRL, 0x18, 0x00},

	{RX_R2_TMDS_14, 0x0A, 0x20},
	{RX_R2_TMDS_14, 0x0B, 0x0F},
	{RX_R2_TMDS_14, 0x0C, 0x00},
	{RX_R2_TMDS_14, 0x0D, 0x00},
	{RX_R2_TMDS_14, 0x0E, 0x03},
	{RX_R2_TMDS_14, 0x0F, 0x21},
	{RX_R2_TMDS_14, 0x10, 0x2A},
	{RX_R2_TMDS_14, 0x11, 0x0F},
	{RX_R2_TMDS_14, 0x12, 0xAA},

	{RX_R2_TMDS_20, 0x0A, 0x20},
	{RX_R2_TMDS_20, 0x0B, 0x0F},
	{RX_R2_TMDS_20, 0x0C, 0x00},
	{RX_R2_TMDS_20, 0x0D, 0x00},
	{RX_R2_TMDS_20, 0x0E, 0x03},
	{RX_R2_TMDS_20, 0x0F, 0x00},
	{RX_R2_TMDS_20, 0x10, 0x00},
	{RX_R2_TMDS_20, 0x11, 0x0F},
	{RX_R2_TMDS_20, 0x12, 0xAA},

	{RX_R2_FRL, 0x0A, 0xA0},
	{RX_R2_FRL, 0x0B, 0x0F},
	{RX_R2_FRL, 0x0C, 0x00},
	{RX_R2_FRL, 0x0D, 0x20},
	{RX_R2_FRL, 0x0E, 0x07},
	{RX_R2_FRL, 0x0F, 0x20},
	{RX_R2_FRL, 0x10, 0x00},
	{RX_R2_FRL, 0x11, 0x0F},
	{RX_R2_FRL, 0x12, 0xAA},
	{RX_R2_FRL, 0x13, 0x20},
	{RX_R2_FRL, 0x14, 0x00},
	{RX_R2_FRL, 0x15, 0x00},
	{RX_R2_FRL, 0x16, 0x21},
	{RX_R2_FRL, 0x17, 0x00},
	{RX_R2_FRL, 0x18, 0x00},
	{RX_R2_FRL, 0x19, 0x20},
	{RX_R2_FRL, 0x1A, 0x00},
	{RX_R2_FRL, 0x1B, 0x00},
#ifdef XPS_BOARD_ZCU106
	{RX_R2_FRL, 0x1C, 0x03},
	{RX_R2_FRL, 0x1D, 0x00},
#else
	{RX_R2_FRL, 0x1C, 0x20},
	{RX_R2_FRL, 0x1D, 0x07},
#endif
	{RX_R2_FRL, 0x1E, 0x00},

	{TX_R3_TMDS_14_L, 0x0A, 0x1C},
	{TX_R3_TMDS_14_L, 0x0B, 0x0F},
	{TX_R3_TMDS_14_L, 0x0C, 0x0B},
	{TX_R3_TMDS_14_L, 0x0D, 0x30},
	{TX_R3_TMDS_14_L, 0x0E, 0x4A},
	{TX_R3_TMDS_14_L, 0x0F, 0x30},
	{TX_R3_TMDS_14_L, 0x10, 0x4A},
	{TX_R3_TMDS_14_L, 0x11, 0x0F},
	{TX_R3_TMDS_14_L, 0x12, 0xAA},
	{TX_R3_TMDS_14_L, 0x13, 0x30},
	{TX_R3_TMDS_14_L, 0x14, 0x0F},
	{TX_R3_TMDS_14_L, 0x15, 0x00},
	{TX_R3_TMDS_14_L, 0x16, 0x02},
	{TX_R3_TMDS_14_L, 0x17, 0x63},
	{TX_R3_TMDS_14_L, 0x18, 0x0B},
	{TX_R3_TMDS_14_L, 0x19, 0x00},
	{TX_R3_TMDS_14_L, 0x1A, 0x03},
	{TX_R3_TMDS_14_L, 0x1B, 0x00},
	{TX_R3_TMDS_14_L, 0x1C, 0x00},
	{TX_R3_TMDS_14_L, 0x1D, 0x03},
	{TX_R3_TMDS_14_L, 0x1E, 0x00},

	{TX_R3_TMDS_14_H, 0x0A, 0x1C},
	{TX_R3_TMDS_14_H, 0x0B, 0x0F},
	{TX_R3_TMDS_14_H, 0x0C, 0x0B},
	{TX_R3_TMDS_14_H, 0x0D, 0x30},
	{TX_R3_TMDS_14_H, 0x0E, 0x4A},
	{TX_R3_TMDS_14_H, 0x0F, 0x30},
	{TX_R3_TMDS_14_H, 0x10, 0x4A},
	{TX_R3_TMDS_14_H, 0x11, 0x0F},
	{TX_R3_TMDS_14_H, 0x12, 0xAA},
	{TX_R3_TMDS_14_H, 0x13, 0x30},
	{TX_R3_TMDS_14_H, 0x14, 0x0F},
	{TX_R3_TMDS_14_H, 0x15, 0x00},
	{TX_R3_TMDS_14_H, 0x16, 0x02},
	{TX_R3_TMDS_14_H, 0x17, 0x63},
	{TX_R3_TMDS_14_H, 0x18, 0x0B},
	{TX_R3_TMDS_14_H, 0x19, 0x00},
	{TX_R3_TMDS_14_H, 0x1A, 0x03},
	{TX_R3_TMDS_14_H, 0x1B, 0x00},
	{TX_R3_TMDS_14_H, 0x1C, 0x00},
	{TX_R3_TMDS_14_H, 0x1D, 0x03},
	{TX_R3_TMDS_14_H, 0x1E, 0x00},

	{TX_R3_TMDS_20, 0x0A, 0x1C},
	{TX_R3_TMDS_20, 0x0B, 0x0F},
	{TX_R3_TMDS_20, 0x0C, 0x00},
	{TX_R3_TMDS_20, 0x0D, 0x30},
	{TX_R3_TMDS_20, 0x0E, 0x4A},
	{TX_R3_TMDS_20, 0x0F, 0x30},
	{TX_R3_TMDS_20, 0x10, 0x4A},
	{TX_R3_TMDS_20, 0x11, 0x0F},
	{TX_R3_TMDS_20, 0x12, 0xAA},
	{TX_R3_TMDS_20, 0x13, 0x02},
	{TX_R3_TMDS_20, 0x14, 0x0F},
	{TX_R3_TMDS_20, 0x15, 0x00},
	{TX_R3_TMDS_20, 0x16, 0x02},
	{TX_R3_TMDS_20, 0x17, 0x63},
	{TX_R3_TMDS_20, 0x18, 0x0B},
	{TX_R3_TMDS_20, 0x19, 0x00},
	{TX_R3_TMDS_20, 0x1A, 0x03},
	{TX_R3_TMDS_20, 0x1B, 0x00},
	{TX_R3_TMDS_20, 0x1C, 0x00},
	{TX_R3_TMDS_20, 0x1D, 0x03},
	{TX_R3_TMDS_20, 0x1E, 0x00},

	{TX_R3_FRL, 0x0A, 0x24},
	{TX_R3_FRL, 0x0B, 0x0D},
	{TX_R3_FRL, 0x0C, 0x00},
#ifdef XPS_BOARD_ZCU106
	{TX_R3_FRL, 0x0D, 0x31}, /*Onsemi 0x10},*/
	{TX_R3_FRL, 0x0E, 0x0A}, /*Onsemi 0x2A},*/
	{TX_R3_FRL, 0x0F, 0x31}, /*Onsemi 0x02},*/
	{TX_R3_FRL, 0x10, 0x05},
#elif defined (XPS_BOARD_ZCU102)
	{TX_R3_FRL, 0x0D, 0x10}, /*Onsemi 0x10},*/
	{TX_R3_FRL, 0x0E, 0x2A}, /*Onsemi 0x2A},*/
	{TX_R3_FRL, 0x0F, 0x31}, /*Onsemi 0x02},*/
	{TX_R3_FRL, 0x10, 0x05},
#elif defined (XPS_BOARD_VCU118)
	{TX_R3_FRL, 0x0D, 0x30}, /*Onsemi 0x10},*/
	{TX_R3_FRL, 0x0E, 0x00}, /*Onsemi 0x2A},*/
	{TX_R3_FRL, 0x0F, 0x30}, /*Onsemi 0x02},*/
	{TX_R3_FRL, 0x10, 0x00},
#elif defined XPS_BOARD_VCK190
	{TX_R3_FRL, 0x0D, 0x31}, /*Onsemi 0x10},*/
	{TX_R3_FRL, 0x0E, 0x0A}, /*Onsemi 0x2A},*/
	{TX_R3_FRL, 0x0F, 0x31}, /*Onsemi 0x02},*/
	{TX_R3_FRL, 0x10, 0x05},
#endif
	{TX_R3_FRL, 0x11, 0x0F},
	{TX_R3_FRL, 0x12, 0xAA},
	{TX_R3_FRL, 0x13, 0x00},
	{TX_R3_FRL, 0x14, 0x03},
	{TX_R3_FRL, 0x15, 0x00},
	{TX_R3_FRL, 0x16, 0x00},
	{TX_R3_FRL, 0x17, 0x03},
	{TX_R3_FRL, 0x18, 0x00},
	{TX_R3_FRL, 0x19, 0x00},
	{TX_R3_FRL, 0x1A, 0x03},
	{TX_R3_FRL, 0x1B, 0x00},
	{TX_R3_FRL, 0x1C, 0x00},
	{TX_R3_FRL, 0x1D, 0x03},
	{TX_R3_FRL, 0x1E, 0x00},

	{RX_R3_TMDS_14, 0x0A, 0x34},
	{RX_R3_TMDS_14, 0x0B, 0x0D},
	{RX_R3_TMDS_14, 0x0C, 0x00},
	{RX_R3_TMDS_14, 0x0D, 0x00},
	{RX_R3_TMDS_14, 0x0E, 0x03},
	{RX_R3_TMDS_14, 0x0F, 0x21},
	{RX_R3_TMDS_14, 0x10, 0x2A},
	{RX_R3_TMDS_14, 0x11, 0x0F},
	{RX_R3_TMDS_14, 0x12, 0x00},
	{RX_R3_TMDS_14, 0x13, 0x00},
	{RX_R3_TMDS_14, 0x14, 0x03},
	{RX_R3_TMDS_14, 0x15, 0x00},
	{RX_R3_TMDS_14, 0x16, 0x00},
	{RX_R3_TMDS_14, 0x17, 0x03},
	{RX_R3_TMDS_14, 0x18, 0x00},
	{RX_R3_TMDS_14, 0x19, 0x00},
	{RX_R3_TMDS_14, 0x1A, 0x03},
	{RX_R3_TMDS_14, 0x1B, 0x00},
	{RX_R3_TMDS_14, 0x1C, 0x00},
	{RX_R3_TMDS_14, 0x1D, 0x03},
	{RX_R3_TMDS_14, 0x1E, 0x00},

	{RX_R3_TMDS_20, 0x0A, 0x34},
	{RX_R3_TMDS_20, 0x0B, 0x0D},
	{RX_R3_TMDS_20, 0x0C, 0x00},
	{RX_R3_TMDS_20, 0x0D, 0x00},
	{RX_R3_TMDS_20, 0x0E, 0x03},
	{RX_R3_TMDS_20, 0x0F, 0x00},
	{RX_R3_TMDS_20, 0x10, 0x00},
	{RX_R3_TMDS_20, 0x11, 0x0F},
	{RX_R3_TMDS_20, 0x12, 0x00},
	{RX_R3_TMDS_20, 0x13, 0x00},
	{RX_R3_TMDS_20, 0x14, 0x03},
	{RX_R3_TMDS_20, 0x15, 0x00},
	{RX_R3_TMDS_20, 0x16, 0x00},
	{RX_R3_TMDS_20, 0x17, 0x03},
	{RX_R3_TMDS_20, 0x18, 0x00},
	{RX_R3_TMDS_20, 0x19, 0x00},
	{RX_R3_TMDS_20, 0x1A, 0x03},
	{RX_R3_TMDS_20, 0x1B, 0x00},
	{RX_R3_TMDS_20, 0x1C, 0x00},
	{RX_R3_TMDS_20, 0x1D, 0x03},
	{RX_R3_TMDS_20, 0x1E, 0x00},

	{RX_R3_FRL, 0x0A, 0x24},
	{RX_R3_FRL, 0x0B, 0x0D},
	{RX_R3_FRL, 0x0C, 0x00},
	{RX_R3_FRL, 0x0D, 0x20},
	{RX_R3_FRL, 0x0E, 0x07},
	{RX_R3_FRL, 0x0F, 0x20},
	{RX_R3_FRL, 0x10, 0x00},
	{RX_R3_FRL, 0x11, 0x0F},
	{RX_R3_FRL, 0x12, 0xAA},
	{RX_R3_FRL, 0x13, 0x21},
	{RX_R3_FRL, 0x14, 0x00},
	{RX_R3_FRL, 0x15, 0x00},
	{RX_R3_FRL, 0x16, 0x21},
	{RX_R3_FRL, 0x17, 0x00},
	{RX_R3_FRL, 0x18, 0x00},
	{RX_R3_FRL, 0x19, 0x21},
	{RX_R3_FRL, 0x1A, 0x00},
	{RX_R3_FRL, 0x1B, 0x00},
#ifdef XPS_BOARD_ZCU106
	{RX_R3_FRL, 0x1C, 0x21},
	{RX_R3_FRL, 0x1D, 0x00},
#else
	{RX_R3_FRL, 0x1C, 0x20},
	{RX_R3_FRL, 0x1D, 0x07},
#endif
	{RX_R3_FRL, 0x1E, 0x00},

};

/************************** Function Prototypes ******************************/

#if 0
static void ONSEMI_NB7NQ621M_I2cReset(void *IicPtr);
#endif
static unsigned ONSEMI_NB7NQ621M_I2cSend(void *IicPtr, u16 SlaveAddr,
		u8 *MsgPtr, unsigned ByteCount, u8 Option);
static unsigned ONSEMI_NB7NQ621M_I2cRecv(void *IicPtr, u16 SlaveAddr,
		u8 *BufPtr, unsigned ByteCount, u8 Option);
static u8 ONSEMI_NB7NQ621M_GetRegister(void *IicPtr, u8 I2CSlaveAddress,
		u8 RegisterAddress);
static int ONSEMI_NB7NQ621M_SetRegister(void *IicPtr, u8 I2CSlaveAddress,
		u8 RegisterAddress, u8 Value);
#if 0
static int ONSEMI_NB7NQ621M_ModifyRegister(void *IicPtr, u8 I2CSlaveAddress,
		u16 RegisterAddress, u8 Value, u8 Mask);
#endif

/************************** Function Definitions *****************************/

#if 0
/*****************************************************************************/
/**
*
* This function resets the IIC instance for ONSEMI_NB7NQ621M
*
* @param  IicPtr IIC instance pointer.

*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void ONSEMI_NB7NQ621M_I2cReset(void *IicPtr)
{
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106)
	/* Do nothing
	XIicPs *Iic_Ptr = IicPtr;
	XIicPs_Reset(Iic_Ptr);*/
#else
	XIic *Iic_Ptr = IicPtr;
	XIic_WriteReg(Iic_Ptr->BaseAddress, XIIC_RESETR_OFFSET,
				  XIIC_RESET_MASK);
#endif
}
#endif

/*****************************************************************************/
/**
*
* This function send the IIC data to ONSEMI_NB7NQ621M
*
* @param  IicPtr IIC instance pointer.
* @param  SlaveAddr contains the 7 bit IIC address of the device to send the
*		   specified data to.
* @param MsgPtr points to the data to be sent.
* @param ByteCount is the number of bytes to be sent.
* @param Option indicates whether to hold or free the bus after
* 		  transmitting the data.
*
* @return	The number of bytes sent.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static unsigned ONSEMI_NB7NQ621M_I2cSend(void *IicPtr, u16 SlaveAddr,
		u8 *MsgPtr, unsigned ByteCount, u8 Option)
{
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190)
	XIicPs *Iic_Ptr = IicPtr;
	u32 Status;

	/* Set operation to 7-bit mode */
	XIicPs_SetOptions(Iic_Ptr, XIICPS_7_BIT_ADDR_OPTION);
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_10_BIT_ADDR_OPTION);

	/* Set Repeated Start option */
	if (Option == I2C_REPEATED_START) {
		XIicPs_SetOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	} else {
		XIicPs_ClearOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	}

	Status = XIicPs_MasterSendPolled(Iic_Ptr, MsgPtr, ByteCount, SlaveAddr);

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	if (!(Iic_Ptr->IsRepeatedStart)) {
		while (XIicPs_BusIsBusy(Iic_Ptr));
	}

	if (Status == XST_SUCCESS) {
		return ByteCount;
	} else {
		return 0;
	}
#else
	XIic *Iic_Ptr = IicPtr;
	/* This delay prevents IIC access from hanging */
	usleep(1000);
	return XIic_Send(Iic_Ptr->BaseAddress, SlaveAddr, MsgPtr,
					ByteCount, Option);
#endif
}

/*****************************************************************************/
/**
*
* This function send the IIC data to ONSEMI_NB7NQ621M
*
* @param  IicPtr IIC instance pointer.
* @param  SlaveAddr contains the 7 bit IIC address of the device to send the
*		   specified data to.
* @param BufPtr points to the memory to write the data.
* @param ByteCount is the number of bytes to be sent.
* @param Option indicates whether to hold or free the bus after
* 		  transmitting the data.
*
* @return	The number of bytes sent.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static unsigned ONSEMI_NB7NQ621M_I2cRecv(void *IicPtr, u16 SlaveAddr,
		u8 *BufPtr, unsigned ByteCount, u8 Option)
{
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190)
	XIicPs *Iic_Ptr = IicPtr;
	u32 Status;

	XIicPs_SetOptions(Iic_Ptr, XIICPS_7_BIT_ADDR_OPTION);
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_10_BIT_ADDR_OPTION);
	if (Option == I2C_REPEATED_START) {
		XIicPs_SetOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	} else {
		XIicPs_ClearOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	}

	Status = XIicPs_MasterRecvPolled(Iic_Ptr, BufPtr, ByteCount, SlaveAddr);

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	if (!(Iic_Ptr->IsRepeatedStart)) {
		while (XIicPs_BusIsBusy(Iic_Ptr));
	}

	if (Status == XST_SUCCESS) {
		return ByteCount;
	} else {
		return 0;
	}
#else
	XIic *Iic_Ptr = IicPtr;
	return XIic_Recv(Iic_Ptr->BaseAddress, SlaveAddr, BufPtr,
					ByteCount, Option);
#endif
}

/*****************************************************************************/
/**
*
* This function send a single byte to the ONSEMI NB7NQ621M
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
static int ONSEMI_NB7NQ621M_SetRegister(void *IicPtr, u8 I2CSlaveAddress,
		u8 RegisterAddress, u8 Value)
{
	u32 ByteCount = 0;
	u8 Buffer[2];

	Buffer[0] = RegisterAddress;
	Buffer[1] = Value;
	ByteCount = ONSEMI_NB7NQ621M_I2cSend(IicPtr, I2CSlaveAddress,
			(u8*)Buffer, 2, I2C_STOP);
	if (ByteCount != 2) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function send a single byte to the ONSEMI NB7NQ621M
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
static u8 ONSEMI_NB7NQ621M_GetRegister(void *IicPtr, u8 I2CSlaveAddress,
			u8 RegisterAddress)
{
	u8 Buffer[2];

	Buffer[0] = RegisterAddress;
	ONSEMI_NB7NQ621M_I2cSend(IicPtr, I2CSlaveAddress, (u8*)Buffer,
			1, I2C_REPEATED_START);
	ONSEMI_NB7NQ621M_I2cRecv(IicPtr, I2CSlaveAddress,
					(u8*)Buffer, 1, I2C_STOP);
	return Buffer[0];
}

#if 0
/*****************************************************************************/
/**
*
* This function modifies a single byte to the ONSEMI NB7NQ621M
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
static int ONSEMI_NB7NQ621M_ModifyRegister(void *IicPtr, u8 I2CSlaveAddress,
				u16 RegisterAddress, u8 Value, u8 Mask)
{
	u8 Data;
	int Result;

	/* Read data */
	Data = ONSEMI_NB7NQ621M_GetRegister(IicPtr, I2CSlaveAddress,
				       RegisterAddress);

	/* Clear masked bits */
	Data &= ~Mask;

	/* Update */
	Data |= (Value & Mask);

	/* Write data */
	Result = ONSEMI_NB7NQ621M_SetRegister(IicPtr, I2CSlaveAddress,
					 RegisterAddress, Data);

	return Result;
}
#endif

/*****************************************************************************/
/**
*
* This function initializes the ONSEMI NB7NQ621M with default values
* for use with the Video FMC.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
* @param Revision is the revision number of the ONSEMI device.
* @param IsTx specifies if the configuration is for TX or RX.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
int ONSEMI_NB7NQ621M_Init(void *IicPtr, u8 I2CSlaveAddress,
		u8 Revision, u8 IsTx)
{
	int Result = XST_FAILURE;
	u16 DeviceType = 0xFFFF;
	u32 i = 0;

	if (IsTx == 1) {
		switch (Revision) {
		case 0:
			DeviceType = TX_R0_TMDS;
			break;
		case 1:
			DeviceType = TX_R1_TMDS_14;
			break;
		case 2:
			DeviceType = TX_R2_TMDS_14_L;
			break;
		case 3:
			DeviceType = TX_R3_TMDS_14_L;
			break;

		default:
			break;
		}
	} else {
		switch (Revision) {
		case 0:
			DeviceType = RX_R0;
			break;
		case 1:
			DeviceType = RX_R1_TMDS_14;
			break;
		case 2:
			DeviceType = RX_R2_TMDS_14;
			break;
		case 3:
			DeviceType = RX_R3_TMDS_14;
			break;

		default:
			break;
		}
	}

	i = DeviceType;

	while (DeviceType == OnsemiRegisterFields[i].DeviceType) {
		Result = ONSEMI_NB7NQ621M_SetRegister(IicPtr,
				I2CSlaveAddress,
				OnsemiRegisterFields[i].Address,
				OnsemiRegisterFields[i].Values);

		if (Result != XST_SUCCESS) {
			return Result;
		}

		i++;
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function checks the ONSEMI NB7NQ621M device ID
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
int ONSEMI_NB7NQ621M_CheckDeviceID(void *IicPtr, u8 I2CSlaveAddress)
{
	u16 DeviceId;
	u8 Data;

	Data = ONSEMI_NB7NQ621M_GetRegister(IicPtr, I2CSlaveAddress, 0x0001);

	/* Copy */
	DeviceId = Data;

	/* Shift */
	DeviceId <<= 8;

	Data = ONSEMI_NB7NQ621M_GetRegister(IicPtr, I2CSlaveAddress, 0x0000);

	/* Copy */
	DeviceId |= Data;

	Data = ONSEMI_NB7NQ621M_GetRegister(IicPtr, I2CSlaveAddress, 0x0002);
	Data = ONSEMI_NB7NQ621M_GetRegister(IicPtr, I2CSlaveAddress, 0x0003);
	Data = ONSEMI_NB7NQ621M_GetRegister(IicPtr, I2CSlaveAddress, 0x0004);
	Data = ONSEMI_NB7NQ621M_GetRegister(IicPtr, I2CSlaveAddress, 0x0005);
	Data = ONSEMI_NB7NQ621M_GetRegister(IicPtr, I2CSlaveAddress, 0x0006);
	Data = ONSEMI_NB7NQ621M_GetRegister(IicPtr, I2CSlaveAddress, 0x0007);

	/* Check */
	if (DeviceId == 0x4E4F)
		return XST_SUCCESS;
	else
		return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function checks the ONSEMI NB7NQ621M Device Revision Number
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
u8 ONSEMI_NB7NQ621M_CheckDeviceVersion(void *IicPtr, u8 I2CSlaveAddress)
{
	u8 RevisionVersion;
	u8 Data;

	/* Register 0x02 of pass 0 active card (Revision) = 0,
	 * Register 0x02 of pass 1 active card (Byte 2 of Device ID) = 0x20 */
	Data = ONSEMI_NB7NQ621M_GetRegister(IicPtr, I2CSlaveAddress, 0x0002);

	if (Data == 0x00) {
		RevisionVersion = 0x00;
	} else {
		/* Register 0x08 of pass 1 and subsequent active cards =
		 * Device Revision Number */
		RevisionVersion = ONSEMI_NB7NQ621M_GetRegister(IicPtr,
				I2CSlaveAddress, 0x0008);
	}

	return RevisionVersion;
}

/*****************************************************************************/
/**
*
* This function reconfigures the ONSEMI NB7NQ621M cable redriver.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
* @param Revision is the revision number of the ONSEMI device.
* @param IsFRL specifies if the mode is FRL or TMDS.
* @param LineRate specifies the linerate.
* @param IsTx specifies if the configuration is for TX or RX.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
int ONSEMI_NB7NQ621M_LineRateReconfig(void *IicPtr, u8 I2CSlaveAddress,
		u8 Revision, u8 IsFRL, u64 LineRate, u8 IsTx)
{
	int Result = XST_FAILURE;
	u16 DeviceType = 0xFFFF;
	u32 LineRateMbps;
	u32 i = 0;

	LineRateMbps = (u32)((u64) LineRate / 1000000);

	/* TX */
	if (IsTx == 1) {
		switch (Revision) {
		case 0:
			if (IsFRL == 1) {
				DeviceType = TX_R0_FRL;
			} else {
				/* HDMI 2.0 */
				if ((LineRateMbps > 3400) &&
						(LineRateMbps <= 6000)) {
					DeviceType = TX_R0_TMDS_20;
				}
				/* HDMI 1.4 1.65-3.4 Gbps */
				else if ((LineRateMbps > 1650) &&
						(LineRateMbps <= 3400)) {
					DeviceType = TX_R0_TMDS_14_H;
				}
				/* HDMI 1.4 0.25-1.65 Gbps */
				else {
					DeviceType = TX_R0_TMDS_14_L;
				}
			}
			break;
		case 1:
			if (IsFRL == 1) {
				if (LineRateMbps >= 12000) {
					DeviceType = TX_R1_FRL_12G;
				} else if (LineRateMbps >= 10000) {
					DeviceType = TX_R1_FRL_10G;
				} else {
					DeviceType = TX_R1_FRL;
				}
			} else {
				/* HDMI 2.0 */
				if (LineRateMbps > 3400) {
					DeviceType = TX_R1_TMDS_20;
				}
				/* HDMI 1.4 */
				else if ((LineRateMbps > 99) &&
						(LineRateMbps <= 3400)) {
					DeviceType = TX_R1_TMDS_14;
				}
				else if ((LineRateMbps > 74.25) &&
						(LineRateMbps <= 99)) {
					DeviceType = TX_R1_TMDS_14_L;
				} else {
					DeviceType = TX_R1_TMDS_14_LL;
				}
			}
			break;
		case 2:
			if (IsFRL == 1) {
				DeviceType = TX_R2_FRL;
			} else {
				/* HDMI 2.0 */
				if ((LineRateMbps > 3400) &&
						(LineRateMbps <= 6000)) {
					DeviceType = TX_R2_TMDS_20;
				}
				/* HDMI 1.4 1.65-3.4 Gbps */
				else if ((LineRateMbps > 1650) &&
						(LineRateMbps <= 3400)) {
					DeviceType = TX_R2_TMDS_14_H;
				}
				/* HDMI 1.4 0.25-1.65 Gbps */
				else {
					DeviceType = TX_R2_TMDS_14_L;
				}
			}
			break;
		case 3:
			if (IsFRL == 1) {
				DeviceType = TX_R3_FRL;
			} else {
				/* HDMI 2.0 */
				if ((LineRateMbps > 3400) &&
						(LineRateMbps <= 6000)) {
					DeviceType = TX_R3_TMDS_20;
				}
				/* HDMI 1.4 1.65-3.4 Gbps */
				else if ((LineRateMbps > 1650) &&
						(LineRateMbps <= 3400)) {
					DeviceType = TX_R3_TMDS_14_H;
				}
				/* HDMI 1.4 0.25-1.65 Gbps */
				else {
					DeviceType = TX_R3_TMDS_14_L;
				}
			}
			break;

		default:
			break;
		}
	}
	/* RX */
	else {
		switch (Revision) {
		case 0:
			/* DeviceType = RX_R0; */
			break;
		case 1:
			if (IsFRL == 1) {
				DeviceType = RX_R1_FRL;
			} else {
				if (LineRateMbps > 3400) {
					DeviceType = RX_R1_TMDS_20;
				} else {
					DeviceType = RX_R1_TMDS_14;
				}
			}
			break;
		case 2:
			if (IsFRL == 1) {
				DeviceType = RX_R2_FRL;
			} else {

				if (LineRateMbps > 3400) {
					DeviceType = RX_R2_TMDS_20;
				} else {
					DeviceType = RX_R2_TMDS_14;
				}
			}
			break;
		case 3:
			if (IsFRL == 1) {
				DeviceType = RX_R3_FRL;
			} else {

				if (LineRateMbps > 3400) {
					DeviceType = RX_R3_TMDS_20;
				} else {
					DeviceType = RX_R3_TMDS_14;
				}
			}
			break;

		default:

			break;
		}
	}

	i = DeviceType;

	while (DeviceType == OnsemiRegisterFields[i].DeviceType) {
		Result = ONSEMI_NB7NQ621M_SetRegister(IicPtr,
				I2CSlaveAddress,
				OnsemiRegisterFields[i].Address,
				OnsemiRegisterFields[i].Values);

		if (Result != XST_SUCCESS) {
			return Result;
		}

		i++;
	}

	return Result;
}

/*****************************************************************************/
/**
*
* This function prints out all the contents from OnsemiRegisterFields array.
*
* @param None
*
* @return None
*
* @note None.
*
******************************************************************************/
void ONSEMI_NB7NQ621M_RegisterLibraryDump(void)
{
	u32 Index;

	Index = sizeof(OnsemiRegisterFields) / sizeof(Onsemi_RegisterField);

	for (u32 i = 0; i < Index; i++) {
		xil_printf(">Device: %d, i:%i, A:0x%X, V:0x%X\r\n",
				OnsemiRegisterFields[i].DeviceType,
				i,
				OnsemiRegisterFields[i].Address,
				OnsemiRegisterFields[i].Values);
	}
}

/*****************************************************************************/
/**
*
* This function displays a registerdump of the ONSEMI NB7NQ621M device.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return None
*
* @note None.
*
******************************************************************************/
void ONSEMI_NB7NQ621M_RegisterDump(void *IicPtr, u8 I2CSlaveAddress)
{
	u8 Data;
	u32 i;
	int Result;

	xil_printf("-----------------------------\r\n");
	xil_printf("- ONSEMI_NB7NQ621M I2C dump:\r\n");
	xil_printf("-----------------------------\r\n");

	Result = ONSEMI_NB7NQ621M_CheckDeviceID(IicPtr, I2CSlaveAddress);

	if (Result == XST_SUCCESS) {
		xil_printf("     ");
		for (i=0; i<8; i++)
			xil_printf("+%01x ", i);

		xil_printf("\r\n     ");
		for (i=0; i<8; i++)
			xil_printf("---");

		for (i=0; i<31; i++) {
			if ((i % 8) == 0) {
				xil_printf("\r\n%02x : ", i);
			}
			Data = ONSEMI_NB7NQ621M_GetRegister(IicPtr,
							I2CSlaveAddress, i);
			xil_printf("%02x ", Data);
		}

		xil_printf("\r\n");
	}

	else {
		xil_printf("ONSEMI NB7NQ621M not found!\r\n");
	}
}
