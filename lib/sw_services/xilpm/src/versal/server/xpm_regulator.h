/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_REGULATOR_H_
#define XPM_REGULATOR_H_

#include "xstatus.h"
#include "xpm_power.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	XPM_METHODTYPE_I2C=1,
} XPm_ControlMethod;

typedef struct XPm_Regulator {
	XPm_Node Node; 			/** Node base class */
	XPm_ControlMethod CtrlMethod; 	/** Control method. Pmbus/I2c:1, no other value supported */
	u32 ParentId;			/** Parent Node ID */
	struct XPm_I2cCmd Config;	/** i2c commands to configure this regulator */
} XPm_Regulator;


/************************** Function Prototypes ******************************/

XPm_Regulator *XPmRegulator_GetById(u32 Id);
XStatus XPmRegulator_Init(XPm_Regulator *Regulator, u32 Id, const u32 *Args,
			  u32 NumArgs);

#ifdef __cplusplus
}
#endif

#endif /* XPM_REGULATOR_H_ */
