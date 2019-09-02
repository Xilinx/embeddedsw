/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
******************************************************************************/

#ifndef XPM_BOARD_H_
#define XPM_BOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xparameters.h"
#include "xstatus.h"
#include "xil_types.h"

#ifdef XPAR_XIICPS_1_DEVICE_ID
#include "xiicps.h"
#include "xpm_pmbus.h"
#endif /* XPAR_XIICPS_1_DEVICE_ID */

/************************** I2C Configurations *******************************/
#define IIC_DEVICE_ID               0
#define IIC_CLK_FREQ_HZ				100000000
#define IIC_SCLK_RATE               100000
#define IIC_BASE_ADDR				PMC_I2C_ADDR

/************************** Address Definitions ******************************/
#define PMC_I2C_ADDR			0xF1000000
#define I2C0_MUX_ADDR           0x74
#define PSFP_REGULATOR_ADDR     0x0A

/************************ MUX Channel Definitions ****************************/
/***************** Channels 5-7 are currently not configured *****************/
#define MUX_SEL_CHANNEL_0       0x01
#define MUX_SEL_CHANNEL_1       0x02
#define MUX_SEL_CHANNEL_2       0x04
#define MUX_SEL_CHANNEL_3       0x08
#define MUX_SEL_CHANNEL_4       0x10

/************************** Variable Definitions *****************************/
#define BUFFER_SIZE				1 /**< I2C Buffer size */

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
