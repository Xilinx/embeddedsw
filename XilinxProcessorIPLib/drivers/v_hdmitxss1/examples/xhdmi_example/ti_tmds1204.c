/******************************************************************************
* Copyright (C) 2018 – 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file TI_TMDS1204.c
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
* 1.00  GM     19/05/14 Initial release.
* 1.01	ssh    17/02/25 Updated compliance values.
* </pre>
*
******************************************************************************/

#include "ti_tmds1204.h"

#if defined (XPS_BOARD_VEK280) || \
	defined (XPS_BOARD_VEK385)
#if (defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190))
#define I2C_REPEATED_START 0x01
#define I2C_STOP 0x00
#else
#define I2C_REPEATED_START XIIC_REPEATED_START
#define I2C_STOP XIIC_STOP
#endif

/************************** Constant Definitions ******************************/

/**
* This table contains the values to be programmed to TI device.
* Each entry is of the format:
* 1) Device Type
* 2) Register Address
* 3) Values
*/
const Ti_RegisterField TiRegisterFields[] = {

	{TX_TI_R1_INIT, 0x0A, 0x8E}, //lanes swap
	{TX_TI_R1_INIT, 0x0B, 0x43},
	{TX_TI_R1_INIT, 0x0C, 0x70},
	{TX_TI_R1_INIT, 0x0D, 0x22}, //was e3
	{TX_TI_R1_INIT, 0x0E, 0x97},
	{TX_TI_R1_INIT, 0x11, 0x0F},
	{TX_TI_R1_INIT, 0x09, 0x02},

	{TX_TI_TMDS_14_L_R1, 0x11, 0x00},
	{TX_TI_TMDS_14_L_R1, 0x0D, 0x22}, //was e3
	{TX_TI_TMDS_14_L_R1, 0x12, 0x03},
	{TX_TI_TMDS_14_L_R1, 0x13, 0x00},
	{TX_TI_TMDS_14_L_R1, 0x14, 0x03},
	{TX_TI_TMDS_14_L_R1, 0x15, 0x05}, // value
	{TX_TI_TMDS_14_L_R1, 0x16, 0x03},
	{TX_TI_TMDS_14_L_R1, 0x17, 0x05}, //value
	{TX_TI_TMDS_14_L_R1, 0x18, 0x03},
	{TX_TI_TMDS_14_L_R1, 0x19, 0x05}, //value
	{TX_TI_TMDS_14_L_R1, 0x20, 0x00},
	{TX_TI_TMDS_14_L_R1, 0x31, 0x00},
	{TX_TI_TMDS_14_L_R1, 0x11, 0x0F},

	{TX_TI_TMDS_14_H_R1, 0x11, 0x00},
	{TX_TI_TMDS_14_H_R1, 0x0D, 0x22}, //was e3
	{TX_TI_TMDS_14_H_R1, 0x12, 0x03},
	{TX_TI_TMDS_14_H_R1, 0x13, 0x00},
	{TX_TI_TMDS_14_H_R1, 0x14, 0x03},
	{TX_TI_TMDS_14_H_R1, 0x15, 0x05}, // value
	{TX_TI_TMDS_14_H_R1, 0x16, 0x03},
	{TX_TI_TMDS_14_H_R1, 0x17, 0x05}, //value
	{TX_TI_TMDS_14_H_R1, 0x18, 0x03},
	{TX_TI_TMDS_14_H_R1, 0x19, 0x05}, //value
	{TX_TI_TMDS_14_H_R1, 0x20, 0x00},
	{TX_TI_TMDS_14_H_R1, 0x31, 0x00},
	{TX_TI_TMDS_14_H_R1, 0x11, 0x0F},

	{TX_TI_TMDS_20_R1, 0x11, 0x00},
	{TX_TI_TMDS_20_R1, 0x0D, 0x22}, //was e3
	{TX_TI_TMDS_20_R1, 0x12, 0x03},
	{TX_TI_TMDS_20_R1, 0x13, 0x00},
	{TX_TI_TMDS_20_R1, 0x14, 0x03},
	{TX_TI_TMDS_20_R1, 0x15, 0x05}, // value
	{TX_TI_TMDS_20_R1, 0x16, 0x03},
	{TX_TI_TMDS_20_R1, 0x17, 0x05}, //value
	{TX_TI_TMDS_20_R1, 0x18, 0x03},
	{TX_TI_TMDS_20_R1, 0x19, 0x05}, //value
	{TX_TI_TMDS_20_R1, 0x20, 0x02},
	{TX_TI_TMDS_20_R1, 0x31, 0x00},
	{TX_TI_TMDS_20_R1, 0x11, 0x0F},

	{TX_TI_FRL_3G_R1, 0x11, 0x00},
	{TX_TI_FRL_3G_R1, 0x0D, 0x22},
	{TX_TI_FRL_3G_R1, 0x12, 0x03},
	{TX_TI_FRL_3G_R1, 0x13, 0x00},
	{TX_TI_FRL_3G_R1, 0x14, 0x03},
	{TX_TI_FRL_3G_R1, 0x15, 0x05}, // value
	{TX_TI_FRL_3G_R1, 0x16, 0x03},
	{TX_TI_FRL_3G_R1, 0x17, 0x05}, //value
	{TX_TI_FRL_3G_R1, 0x18, 0x03},
	{TX_TI_FRL_3G_R1, 0x19, 0x05}, //value
	{TX_TI_FRL_3G_R1, 0x20, 0x00},
	{TX_TI_FRL_3G_R1, 0x31, 0x01}, // 3gx3
	{TX_TI_FRL_3G_R1, 0x11, 0x0F},

	{TX_TI_FRL_6G_3_R1, 0x11, 0x00},
	{TX_TI_FRL_6G_3_R1, 0x0D, 0x22},
	{TX_TI_FRL_6G_3_R1, 0x12, 0x03},
	{TX_TI_FRL_6G_3_R1, 0x13, 0x00},
	{TX_TI_FRL_6G_3_R1, 0x14, 0x03},
	{TX_TI_FRL_6G_3_R1, 0x15, 0x05}, // value
	{TX_TI_FRL_6G_3_R1, 0x16, 0x03},
	{TX_TI_FRL_6G_3_R1, 0x17, 0x05}, //value
	{TX_TI_FRL_6G_3_R1, 0x18, 0x03},
	{TX_TI_FRL_6G_3_R1, 0x19, 0x05}, //value
	{TX_TI_FRL_6G_3_R1, 0x20, 0x00},
	{TX_TI_FRL_6G_3_R1, 0x31, 0x02}, // 6gx3
	{TX_TI_FRL_6G_3_R1, 0x11, 0x0F},

	{TX_TI_FRL_6G_4_R1, 0x11, 0x00},
	{TX_TI_FRL_6G_4_R1, 0x0D, 0x22},
	{TX_TI_FRL_6G_4_R1, 0x12, 0x03},
	{TX_TI_FRL_6G_4_R1, 0x13, 0x05}, //value
	{TX_TI_FRL_6G_4_R1, 0x14, 0x03},
	{TX_TI_FRL_6G_4_R1, 0x15, 0x05}, // value
	{TX_TI_FRL_6G_4_R1, 0x16, 0x03},
	{TX_TI_FRL_6G_4_R1, 0x17, 0x05}, //value
	{TX_TI_FRL_6G_4_R1, 0x18, 0x03},
	{TX_TI_FRL_6G_4_R1, 0x19, 0x05}, //value
	{TX_TI_FRL_6G_4_R1, 0x20, 0x00},
	{TX_TI_FRL_6G_4_R1, 0x31, 0x03}, // 6gx4
	{TX_TI_FRL_6G_4_R1, 0x11, 0x0F},

	{TX_TI_FRL_8G_R1, 0x11, 0x00},
	{TX_TI_FRL_8G_R1, 0x0D, 0x22},
	{TX_TI_FRL_8G_R1, 0x12, 0x03},
	{TX_TI_FRL_8G_R1, 0x13, 0x05}, //value
	{TX_TI_FRL_8G_R1, 0x14, 0x03},
	{TX_TI_FRL_8G_R1, 0x15, 0x05}, // value
	{TX_TI_FRL_8G_R1, 0x16, 0x03},
	{TX_TI_FRL_8G_R1, 0x17, 0x05}, //value
	{TX_TI_FRL_8G_R1, 0x18, 0x03},
	{TX_TI_FRL_8G_R1, 0x19, 0x05}, //value
	{TX_TI_FRL_8G_R1, 0x20, 0x00},
	{TX_TI_FRL_8G_R1, 0x31, 0x04}, // 8gx4
	{TX_TI_FRL_8G_R1, 0x11, 0x0F},

	{TX_TI_FRL_10G_R1, 0x11, 0x00},
	{TX_TI_FRL_10G_R1, 0x0D, 0x22},//22
	{TX_TI_FRL_10G_R1, 0x12, 0x03},
	{TX_TI_FRL_10G_R1, 0x13, 0x05}, //value
	{TX_TI_FRL_10G_R1, 0x14, 0x03},
	{TX_TI_FRL_10G_R1, 0x15, 0x05}, // value
	{TX_TI_FRL_10G_R1, 0x16, 0x03},
	{TX_TI_FRL_10G_R1, 0x17, 0x05}, //value
	{TX_TI_FRL_10G_R1, 0x18, 0x03},
	{TX_TI_FRL_10G_R1, 0x19, 0x05}, //value
	{TX_TI_FRL_10G_R1, 0x20, 0x00},
	{TX_TI_FRL_10G_R1, 0x31, 0x05}, // 10gx4
	{TX_TI_FRL_10G_R1, 0x11, 0x0F},

	{TX_TI_FRL_12G_R1, 0x11, 0x00},
	{TX_TI_FRL_12G_R1, 0x0D, 0x22}, //22
#if defined (XPS_BOARD_ZCU102)
	{TX_TI_FRL_12G_R1, 0x12, 0x02},
#elif defined (XPS_BOARD_ZCU106)
	{TX_TI_FRL_12G_R1, 0x12, 0x02},
#else //vcu118
	{TX_TI_FRL_12G_R1, 0x12, 0x03},
#endif
	{TX_TI_FRL_12G_R1, 0x13, 0x05}, //value
#if defined (XPS_BOARD_ZCU102)
	{TX_TI_FRL_12G_R1, 0x14, 0x02},
#elif defined (XPS_BOARD_ZCU106)
	{TX_TI_FRL_12G_R1, 0x14, 0x02},
#else //vcu118
	{TX_TI_FRL_12G_R1, 0x14, 0x03},
#endif
	{TX_TI_FRL_12G_R1, 0x15, 0x05}, // value
#if defined (XPS_BOARD_ZCU102)
	{TX_TI_FRL_12G_R1, 0x16, 0x02},
#elif defined (XPS_BOARD_ZCU106)
	{TX_TI_FRL_12G_R1, 0x16, 0x02},
#else //vcu118
	{TX_TI_FRL_12G_R1, 0x16, 0x03},
#endif
	{TX_TI_FRL_12G_R1, 0x17, 0x05}, //value
#if defined (XPS_BOARD_ZCU102)
	{TX_TI_FRL_12G_R1, 0x18, 0x02},
#elif defined (XPS_BOARD_ZCU106)
	{TX_TI_FRL_12G_R1, 0x18, 0x02},
#else //vcu118
	{TX_TI_FRL_12G_R1, 0x18, 0x03},
#endif
	{TX_TI_FRL_12G_R1, 0x19, 0x05}, //value
	{TX_TI_FRL_12G_R1, 0x20, 0x00},
	{TX_TI_FRL_12G_R1, 0x31, 0x06}, // 12gx4
	{TX_TI_FRL_12G_R1, 0x11, 0x0F},

	{RX_TI_R1_INIT, 0x0A, 0x4E}, //B}, // enabling RXOUTCLK
	{RX_TI_R1_INIT, 0x0B, 0x43},
	{RX_TI_R1_INIT, 0x0C, 0x70},
	{RX_TI_R1_INIT, 0x0D, 0xE3}, //E3
	{RX_TI_R1_INIT, 0x0E, 0x17}, //97
	{RX_TI_R1_INIT, 0x1E, 0x00},
	{RX_TI_R1_INIT, 0x11, 0x0F},
	{RX_TI_R1_INIT, 0x09, 0x02}, //00
	{RX_TI_R1_INIT, 0xF8, 0x03},

//	{RX_TI_TMDS_14_L_R1, 0x11, 0x00},
	{RX_TI_TMDS_14_L_R1, 0x0A, 0x4E}, //B}, // enabling RXOUTCLK
	{RX_TI_TMDS_14_L_R1, 0x0D, 0xE3},
	{RX_TI_TMDS_14_L_R1, 0x0E, 0x17},
	{RX_TI_TMDS_14_L_R1, 0x12, 0x03},
	{RX_TI_TMDS_14_L_R1, 0x13, 0x00}, //value
	{RX_TI_TMDS_14_L_R1, 0x14, 0x03},
	{RX_TI_TMDS_14_L_R1, 0x15, 0x05}, // value
	{RX_TI_TMDS_14_L_R1, 0x16, 0x03},
	{RX_TI_TMDS_14_L_R1, 0x17, 0x05}, //value
	{RX_TI_TMDS_14_L_R1, 0x18, 0x03},
	{RX_TI_TMDS_14_L_R1, 0x19, 0x05}, //value
	{RX_TI_TMDS_14_L_R1, 0x20, 0x00}, //2 was also working
	{RX_TI_TMDS_14_L_R1, 0x31, 0x00},
//	{RX_TI_TMDS_14_L_R1, 0x11, 0x0F},

//	{RX_TI_TMDS_14_H_R1, 0x11, 0x00},
	{RX_TI_TMDS_14_H_R1, 0x0A, 0x4E}, //B}, // enabling RXOUTCLK
	{RX_TI_TMDS_14_H_R1, 0x0D, 0xE3},
	{RX_TI_TMDS_14_H_R1, 0x0E, 0x17},
	{RX_TI_TMDS_14_H_R1, 0x12, 0x03},
	{RX_TI_TMDS_14_H_R1, 0x13, 0x00}, //value
	{RX_TI_TMDS_14_H_R1, 0x14, 0x03},
	{RX_TI_TMDS_14_H_R1, 0x15, 0x05}, // value
	{RX_TI_TMDS_14_H_R1, 0x16, 0x03},
	{RX_TI_TMDS_14_H_R1, 0x17, 0x05}, //value
	{RX_TI_TMDS_14_H_R1, 0x18, 0x03},
	{RX_TI_TMDS_14_H_R1, 0x19, 0x05}, //value
	{RX_TI_TMDS_14_H_R1, 0x20, 0x00},
	{RX_TI_TMDS_14_H_R1, 0x31, 0x00},
//	{RX_TI_TMDS_14_H_R1, 0x11, 0x0F},

//	{RX_TI_TMDS_20_R1, 0x11, 0x00},
	{RX_TI_TMDS_20_R1, 0x0A, 0x4E}, //B}, // enabling RXOUTCLK
	{RX_TI_TMDS_20_R1, 0x0D, 0xE3},
	{RX_TI_TMDS_20_R1, 0x0E, 0x17},
	{RX_TI_TMDS_20_R1, 0x12, 0x03},
	{RX_TI_TMDS_20_R1, 0x13, 0x00}, //value
	{RX_TI_TMDS_20_R1, 0x14, 0x03},
	{RX_TI_TMDS_20_R1, 0x15, 0x05}, // value
	{RX_TI_TMDS_20_R1, 0x16, 0x03},
	{RX_TI_TMDS_20_R1, 0x17, 0x05}, //value
	{RX_TI_TMDS_20_R1, 0x18, 0x03},
	{RX_TI_TMDS_20_R1, 0x19, 0x05}, //value
	{RX_TI_TMDS_20_R1, 0x20, 0x02},
	{RX_TI_TMDS_20_R1, 0x31, 0x00},
//	{RX_TI_TMDS_20_R1, 0x11, 0x0F},

//	{RX_TI_FRL_3G_R1, 0x11, 0x00},
	{RX_TI_FRL_3G_R1, 0x0A, 0x0E}, // disabling RXOUTCLK
	{RX_TI_FRL_3G_R1, 0x0D, 0xE3},
	{RX_TI_FRL_3G_R1, 0x0E, 0x17},
	{RX_TI_FRL_3G_R1, 0x12, 0x03},
	{RX_TI_FRL_3G_R1, 0x13, 0x05},//value
	{RX_TI_FRL_3G_R1, 0x14, 0x03},
	{RX_TI_FRL_3G_R1, 0x15, 0x05}, // value
	{RX_TI_FRL_3G_R1, 0x16, 0x03},
	{RX_TI_FRL_3G_R1, 0x17, 0x05}, //value
	{RX_TI_FRL_3G_R1, 0x18, 0x03},
	{RX_TI_FRL_3G_R1, 0x19, 0x05}, //value
	{RX_TI_FRL_3G_R1, 0x20, 0x00},
	{RX_TI_FRL_3G_R1, 0x31, 0x01}, // 3gx3
//	{RX_TI_FRL_3G_R1, 0x11, 0x0F},

//	{RX_TI_FRL_6G_3_R1, 0x11, 0x00},
	{RX_TI_FRL_6G_3_R1, 0x0A, 0x0E}, // disabling RXOUTCLK
	{RX_TI_FRL_6G_3_R1, 0x0D, 0xE3},
	{RX_TI_FRL_6G_3_R1, 0x0E, 0x17},
	{RX_TI_FRL_6G_3_R1, 0x12, 0x02},
	{RX_TI_FRL_6G_3_R1, 0x13, 0x05}, //value
	{RX_TI_FRL_6G_3_R1, 0x14, 0x02},
	{RX_TI_FRL_6G_3_R1, 0x15, 0x05}, // value
	{RX_TI_FRL_6G_3_R1, 0x16, 0x02},
	{RX_TI_FRL_6G_3_R1, 0x17, 0x05}, //value
	{RX_TI_FRL_6G_3_R1, 0x18, 0x02},
	{RX_TI_FRL_6G_3_R1, 0x19, 0x05}, //value
	{RX_TI_FRL_6G_3_R1, 0x20, 0x00},
	{RX_TI_FRL_6G_3_R1, 0x31, 0x02}, // 6gx3
//	{RX_TI_FRL_6G_3_R1, 0x11, 0x0F},

//	{RX_TI_FRL_6G_4_R1, 0x11, 0x00},
	{RX_TI_FRL_6G_4_R1, 0x0A, 0x0E}, // disabling RXOUTCLK
	{RX_TI_FRL_6G_4_R1, 0x0D, 0xE3},
	{RX_TI_FRL_6G_4_R1, 0x0E, 0x17},
	{RX_TI_FRL_6G_4_R1, 0x12, 0x02},
	{RX_TI_FRL_6G_4_R1, 0x13, 0x05}, //value
	{RX_TI_FRL_6G_4_R1, 0x14, 0x02},
	{RX_TI_FRL_6G_4_R1, 0x15, 0x05}, // value
	{RX_TI_FRL_6G_4_R1, 0x16, 0x02},
	{RX_TI_FRL_6G_4_R1, 0x17, 0x05}, //value
	{RX_TI_FRL_6G_4_R1, 0x18, 0x02},
	{RX_TI_FRL_6G_4_R1, 0x19, 0x05}, //value
	{RX_TI_FRL_6G_4_R1, 0x20, 0x00},
	{RX_TI_FRL_6G_4_R1, 0x31, 0x03}, // 6gx4
//	{RX_TI_FRL_6G_4_R1, 0x11, 0x0F},

//	{RX_TI_FRL_8G_R1, 0x11, 0x00},
	{RX_TI_FRL_8G_R1, 0x0A, 0x0E}, // disabling RXOUTCLK
	{RX_TI_FRL_8G_R1, 0x0D, 0xF3},
	{RX_TI_FRL_8G_R1, 0x0E, 0x17},
	{RX_TI_FRL_8G_R1, 0x12, 0x02},
	{RX_TI_FRL_8G_R1, 0x13, 0x05}, //value
	{RX_TI_FRL_8G_R1, 0x14, 0x02},
	{RX_TI_FRL_8G_R1, 0x15, 0x05}, // value
	{RX_TI_FRL_8G_R1, 0x16, 0x02},
	{RX_TI_FRL_8G_R1, 0x17, 0x05}, //value
	{RX_TI_FRL_8G_R1, 0x18, 0x02},
	{RX_TI_FRL_8G_R1, 0x19, 0x05}, //value
	{RX_TI_FRL_8G_R1, 0x20, 0x00},
	{RX_TI_FRL_8G_R1, 0x31, 0x04}, // 8gx4
//	{RX_TI_FRL_8G_R1, 0x11, 0x0F},

//    {RX_TI_FRL_10G_R1, 0x11, 0x00},
    {RX_TI_FRL_10G_R1, 0x0A, 0x0E}, // disabling RXOUTCLK
    {RX_TI_FRL_10G_R1, 0x0D, 0xF3},
    {RX_TI_FRL_10G_R1, 0x12, 0x02},
    {RX_TI_FRL_10G_R1, 0x13, 0x00}, //value
    {RX_TI_FRL_10G_R1, 0x14, 0x01},
    {RX_TI_FRL_10G_R1, 0x15, 0x00}, // value
    {RX_TI_FRL_10G_R1, 0x16, 0x00},
    {RX_TI_FRL_10G_R1, 0x17, 0x01}, //value
    {RX_TI_FRL_10G_R1, 0x18, 0x02},
    {RX_TI_FRL_10G_R1, 0x19, 0x00}, //value
    {RX_TI_FRL_10G_R1, 0x20, 0x00},
    {RX_TI_FRL_10G_R1, 0x31, 0x05}, // 10gx4
//    {RX_TI_FRL_10G_R1, 0x11, 0x0F},

//	{RX_TI_FRL_12G_R1, 0x11, 0x00},
	{RX_TI_FRL_12G_R1, 0x0A, 0x0E}, // disabling RXOUTCLK
#if defined (XPS_BOARD_ZCU102)
	{RX_TI_FRL_12G_R1, 0x0D, 0xF3}, //63}, //63
#elif defined (XPS_BOARD_ZCU106)
	{RX_TI_FRL_12G_R1, 0x0D, 0xF3}, //63
#else //vcu118
	{RX_TI_FRL_12G_R1, 0x0D, 0xF3}, //63
#endif
	{RX_TI_FRL_12G_R1, 0x12, 0x01}, //
#if defined (XPS_BOARD_ZCU102)
	{RX_TI_FRL_12G_R1, 0x13, 0x00}, //
#elif defined (XPS_BOARD_ZCU106)
	{RX_TI_FRL_12G_R1, 0x13, 0x05}, //
#else //vcu118
	{RX_TI_FRL_12G_R1, 0x13, 0x01}, //value
#endif
	{RX_TI_FRL_12G_R1, 0x14, 0x01}, //
	{RX_TI_FRL_12G_R1, 0x15, 0x01}, //
#if defined (XPS_BOARD_ZCU106)
	{RX_TI_FRL_12G_R1, 0x16, 0x00},
	{RX_TI_FRL_12G_R1, 0x17, 0x03}, //value
#else
	{RX_TI_FRL_12G_R1, 0x16, 0x01},
	{RX_TI_FRL_12G_R1, 0x17, 0x01}, //value
#endif

#if defined (XPS_BOARD_ZCU102)
	{RX_TI_FRL_12G_R1, 0x18, 0x02},
#elif defined (XPS_BOARD_ZCU106)
	{RX_TI_FRL_12G_R1, 0x18, 0x02},
#else
	{RX_TI_FRL_12G_R1, 0x18, 0x01},
#endif
	{RX_TI_FRL_12G_R1, 0x19, 0x01}, //value
	{RX_TI_FRL_12G_R1, 0x20, 0x00},
	{RX_TI_FRL_12G_R1, 0x31, 0x06}, // 12gx4
};

/************************** Function Prototypes ******************************/

#if 0
static void TI_TMDS1204_I2cReset(void *IicPtr);
#endif
static unsigned TI_TMDS1204_I2cSend(void *IicPtr, u16 SlaveAddr,
		u8 *MsgPtr, unsigned ByteCount, u8 Option);
static unsigned TI_TMDS1204_I2cRecv(void *IicPtr, u16 SlaveAddr,
		u8 *BufPtr, unsigned ByteCount, u8 Option);
static u8 TI_TMDS1204_GetRegister(void *IicPtr, u8 I2CSlaveAddress,
		u8 RegisterAddress);
static int TI_TMDS1204_SetRegister(void *IicPtr, u8 I2CSlaveAddress,
		u8 RegisterAddress, u8 Value);
#if 0
static int TI_TMDS1204_ModifyRegister(void *IicPtr, u8 I2CSlaveAddress,
		u16 RegisterAddress, u8 Value, u8 Mask);
#endif

/************************** Function Definitions *****************************/

#if 0
/*****************************************************************************/
/**
*
* This function resets the IIC instance for TI_TMDS1204
*
* @param  IicPtr IIC instance pointer.

*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void TI_TMDS1204_I2cReset(void *IicPtr)
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
* This function send the IIC data to TI_TMDS1204
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
static unsigned TI_TMDS1204_I2cSend(void *IicPtr, u16 SlaveAddr,
		u8 *MsgPtr, unsigned ByteCount, u8 Option)
{
#if (defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190))
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
* This function send the IIC data to TI_TMDS1204
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
static unsigned TI_TMDS1204_I2cRecv(void *IicPtr, u16 SlaveAddr,
		u8 *BufPtr, unsigned ByteCount, u8 Option)
{
#if (defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190))
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
* This function send a single byte to the TI TMDS1204
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
static int TI_TMDS1204_SetRegister(void *IicPtr, u8 I2CSlaveAddress,
		u8 RegisterAddress, u8 Value)
{
	u32 ByteCount = 0;
	u8 Buffer[2];

	Buffer[0] = RegisterAddress;
	Buffer[1] = Value;
	ByteCount = TI_TMDS1204_I2cSend(IicPtr, I2CSlaveAddress,
			(u8*)Buffer, 2, I2C_STOP);

	if (ByteCount != 2) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function send a single byte to the TI TMDS1204
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
static u8 TI_TMDS1204_GetRegister(void *IicPtr, u8 I2CSlaveAddress,
			u8 RegisterAddress)
{
	u8 Buffer[2];

	Buffer[0] = RegisterAddress;
	TI_TMDS1204_I2cSend(IicPtr, I2CSlaveAddress, (u8*)Buffer,
			1, I2C_REPEATED_START);
	TI_TMDS1204_I2cRecv(IicPtr, I2CSlaveAddress,
					(u8*)Buffer, 1, I2C_STOP);
	return Buffer[0];
}

#if 0
/*****************************************************************************/
/**
*
* This function modifies a single byte to the TI TMDS1204
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
static int TI_TMDS1204_ModifyRegister(void *IicPtr, u8 I2CSlaveAddress,
				u16 RegisterAddress, u8 Value, u8 Mask)
{
	u8 Data;
	int Result;

	/* Read data */
	Data = TI_TMDS1204_GetRegister(IicPtr, I2CSlaveAddress,
				       RegisterAddress);

	/* Clear masked bits */
	Data &= ~Mask;

	/* Update */
	Data |= (Value & Mask);

	/* Write data */
	Result = TI_TMDS1204_SetRegister(IicPtr, I2CSlaveAddress,
					 RegisterAddress, Data);

	return Result;
}
#endif

/*****************************************************************************/
/**
*
* This function initializes the TI TMDS1204 with default values
* for use with the Video FMC.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
* @param Revision is the revision number of the TI device.
* @param IsTx specifies if the configuration is for TX or RX.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
int TI_TMDS1204_Init(void *IicPtr, u8 I2CSlaveAddress,
		u8 Revision, u8 IsTx)
{
	int Result = XST_FAILURE;
	u16 DeviceType = 0xFFFF;
	u32 i = 0;

	if (IsTx == 1) {
		switch (Revision) {
		case 1:
			DeviceType = TX_TI_R1_INIT;
			break;

		default:
			break;
		}
	} else {
		switch (Revision) {
		case 1:
			DeviceType = RX_TI_R1_INIT;
			break;

		default:
			break;
		}
	}

	i = DeviceType;

	while (DeviceType == TiRegisterFields[i].DeviceType) {
		Result = TI_TMDS1204_SetRegister(IicPtr,
				I2CSlaveAddress,
				TiRegisterFields[i].Address,
				TiRegisterFields[i].Values);

		if (Result != XST_SUCCESS) {
			return Result;
		}

		i++;
	}
	xil_printf ("success\r\n");

	return Result;
}

/*****************************************************************************/
/**
*
* This function checks the TI TMDS1204 device ID
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
int TI_TMDS1204_CheckDeviceID(void *IicPtr, u8 I2CSlaveAddress)
{
	u16 DeviceId;
	u8 Data;

	Data = TI_TMDS1204_GetRegister(IicPtr, I2CSlaveAddress, 0x08);

	/* Copy */
	DeviceId = Data;

	/* Check */
	if (DeviceId == 0x3)
		return XST_SUCCESS;
	else
		return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function checks the TI TMDS1204 Device Revision Number
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
u8 TI_TMDS1204_CheckDeviceVersion(void *IicPtr, u8 I2CSlaveAddress)
{
	u8 RevisionVersion;
	u8 Data;

	Data = TI_TMDS1204_GetRegister(IicPtr, I2CSlaveAddress, 0x08);

	if (Data == 0x00) {
		RevisionVersion = 0x00;
	} else {
		/* Register 0x08 of pass 1 and subsequent active cards =
		 * Device Revision Number */
		RevisionVersion = Data;
	}

	return RevisionVersion;
}

/*****************************************************************************/
/**
*
* This function reconfigures the TI TMDS1204 cable redriver.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
* @param Revision is the revision number of the TI device.
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
int TI_TMDS1204_LineRateReconfig(void *IicPtr, u8 I2CSlaveAddress,
		u8 Revision, u8 IsFRL, u64 LineRate, u8 Lanes, u8 IsTx)
{
	int Result = XST_FAILURE;
	u16 DeviceType = 0xFFFF;
	u32 LineRateMbps;
	u32 i = 0;
	u8 lanes = 4;
	u8 Data;

	LineRateMbps = (u32)((u64) LineRate / 1000000);

	/* TX */
	if (IsTx == 1) {
		switch (Revision) {
		case 1:
			if (IsFRL == 1) {
				if (LineRateMbps == 12000) {
					DeviceType = TX_TI_FRL_12G_R1;
				} else if (LineRateMbps == 10000) {
					DeviceType = TX_TI_FRL_10G_R1;
				} else if (LineRateMbps == 8000) {
					DeviceType = TX_TI_FRL_8G_R1;
				} else if (LineRateMbps == 6000) {
					if (lanes == 4) {
						DeviceType = TX_TI_FRL_6G_4_R1;
					} else {
						DeviceType = TX_TI_FRL_6G_3_R1;
					}
				} else if (LineRateMbps == 3000) {
					DeviceType = TX_TI_FRL_3G_R1;
				}
			} else {
				if (LineRateMbps <= 1650) {
					DeviceType = TX_TI_TMDS_14_L_R1;
				} else if (LineRateMbps > 1650 &&
						LineRateMbps <= 3400) {
					DeviceType = TX_TI_TMDS_14_H_R1;
				} else {
					DeviceType = TX_TI_TMDS_20_R1;
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
		case 1:
			if (IsFRL == 1) {
				if (LineRateMbps == 12000) {
					DeviceType = RX_TI_FRL_12G_R1;
				} else if (LineRateMbps == 10000) {
					DeviceType = RX_TI_FRL_10G_R1;
				} else if (LineRateMbps == 8000) {
					DeviceType = RX_TI_FRL_8G_R1;
				} else if (LineRateMbps == 6000) {
					if (lanes == 4) {
						DeviceType = RX_TI_FRL_6G_4_R1;
					} else {
						DeviceType = RX_TI_FRL_6G_3_R1;
					}
				} else if (LineRateMbps == 3000) {
					DeviceType = RX_TI_FRL_3G_R1;
				}
			} else {
				if (LineRateMbps > 3400) {
					DeviceType = RX_TI_TMDS_20_R1;
				} else {
					DeviceType = RX_TI_TMDS_14_H_R1;
				}
			}
			break;

		default:

			break;
		}
	}

	i = DeviceType;

	while (DeviceType == TiRegisterFields[i].DeviceType) {
		Result = TI_TMDS1204_SetRegister(IicPtr,
				I2CSlaveAddress,
				TiRegisterFields[i].Address,
				TiRegisterFields[i].Values);

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
* This function prints out all the contents from TiRegisterFields array.
*
* @param None
*
* @return None
*
* @note None.
*
******************************************************************************/
void TI_TMDS1204_RegisterLibraryDump(void)
{
	u32 Index;

	Index = sizeof(TiRegisterFields) / sizeof(Ti_RegisterField);

	for (u32 i = 0; i < Index; i++) {
		xil_printf(">Device: %d, i:%i, A:0x%X, V:0x%X\r\n",
				TiRegisterFields[i].DeviceType,
				i,
				TiRegisterFields[i].Address,
				TiRegisterFields[i].Values);
	}
}

/*****************************************************************************/
/**
*
* This function displays a registerdump of the TI TMDS1204 device.
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return None
*
* @note None.
*
******************************************************************************/
void TI_TMDS1204_RegisterDump(void *IicPtr, u8 I2CSlaveAddress)
{
	u8 Data;
	u32 i;
	int Result;

	xil_printf("-----------------------------\r\n");
	xil_printf("- TI_TMDS1204 I2C dump:\r\n");
	xil_printf("-----------------------------\r\n");

	Result = TI_TMDS1204_CheckDeviceID(IicPtr, I2CSlaveAddress);

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
			Data = TI_TMDS1204_GetRegister(IicPtr,
							I2CSlaveAddress, i);
			xil_printf("%02x ", Data);
		}

		xil_printf("\r\n");
	}

	else {
		xil_printf("TI TMDS1204 not found!\r\n");
	}
}

#endif
