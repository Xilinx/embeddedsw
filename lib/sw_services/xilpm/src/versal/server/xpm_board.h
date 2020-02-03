/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_BOARD_H_
#define XPM_BOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xparameters.h"
#include "xstatus.h"
#include "xil_types.h"

/************************** I2C Configurations *******************************/
#define IIC_DEVICE_ID			0
#define IIC_CLK_FREQ_HZ			100000000
#define IIC_SCLK_RATE			400000
#define IIC_BASE_ADDR			PMC_I2C_ADDR

/************************** Address Definitions ******************************/
#define PMC_I2C_ADDR			0xF1000000U
#define I2C0_MUX_ADDR			0x74
#define PSFP_REGULATOR_ADDR		0x0A
#define PSLP_REGULATOR_ADDR		0x09

/************************ MUX Channel Definitions ****************************/
/***************** Channels 5-7 are currently not configured *****************/
#define MUX_SEL_CHANNEL_0		0x01
#define MUX_SEL_CHANNEL_1		0x02
#define MUX_SEL_CHANNEL_2		0x04
#define MUX_SEL_CHANNEL_3		0x08
#define MUX_SEL_CHANNEL_4		0x10

/************************** Variable Definitions *****************************/

enum power_rail_id {
	POWER_RAIL_FPD,
	POWER_RAIL_LPD,
};

enum power_rail_function {
	RAIL_POWER_UP,
	RAIL_POWER_DOWN,
};

/************************** Function Prototypes ******************************/
XStatus XPmBoard_ControlRail(const enum power_rail_function Function,
			const enum power_rail_id PowerRegulatorId);

#ifdef __cplusplus
}
#endif

#endif /* XPM_BOARD_H_ */
