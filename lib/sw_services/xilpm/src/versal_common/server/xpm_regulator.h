/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_REGULATOR_H_
#define XPM_REGULATOR_H_

#include "xstatus.h"
#include "xpm_power.h"
#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined (RAIL_CONTROL)
#define XPM_METHODTYPE_I2C	1
#define XPM_METHODTYPE_GPIO	2

typedef enum {
	XPM_I2C_CNTRLR=0,
	XPM_GPIO_CNTRLR,
	XPM_MAX_NUM_CNTRLR,
} XPm_CntrlrType;

typedef struct {
	XPm_Node Node;			/** Node base class */
	XPm_I2cCmd Config;		/** i2c commands to configure the regulator */
	u16 I2cAddress;			/** i2c address of the regulator */
	SAVE_REGION()
	XPm_Device *Cntrlr[XPM_MAX_NUM_CNTRLR]; /** Array of supported controllers */
} XPm_Regulator;

/************************** Function Prototypes ******************************/

XPm_Regulator *XPmRegulator_GetById(u32 Id);
XStatus XPmRegulator_Init(XPm_Regulator *Regulator, u32 Id, const u32 *Args,
			  u32 NumArgs);
#endif /* RAIL_CONTROL */

#ifdef __cplusplus
}
#endif

#endif /* XPM_REGULATOR_H_ */
