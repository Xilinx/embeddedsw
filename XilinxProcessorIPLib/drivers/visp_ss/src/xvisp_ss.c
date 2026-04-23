#include <string.h>
/******************************************************************************
* Copyright (C) 2025 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xvisp_ss.c
*
* This file contains the implementation of the interface functions for the
* VISP SS driver. See xvisp_ss.h for a description of the driver.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xvisp_ss.h"
#include "xstatus.h"
#include "xil_printf.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/
#define ISPMAP2NONE	-1

#define MAPISP2RPU0	0
#define MAPISP2RPU1	1
#define MAPISP2RPU2	2
#define MAPISP2RPU3	3
#define MAPISP2RPU4	4
#define MAPISP2RPU5	5

/** @cond INTERNAL */
/* Hardware RPU IPI target IDs from xparameters */
#define HW_RPU_ID_0	6
#define HW_RPU_ID_1	7
#define HW_RPU_ID_2	8
#define HW_RPU_ID_3	9
#define HW_RPU_ID_4	10
#define HW_RPU_ID_5	11
/** @endcond */

/**************************** Type Definitions *******************************/
typedef struct isp2rpu_mapping {
	int32_t reg[XPAR_XVISP_SS_NUM_INSTANCES];
} isp2rpu_mapping_t;


/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
isp2rpu_mapping_t isp2rpu_map;
u32 dest_cpu_id, src_cpu_id;
/*****************************************************************************/
/**
*
* Initialize the VISP SS instance provided by the caller based on the
* given configuration data.
*
* @param    InstancePtr is the device instance to operate on.
* @param    CfgPtr is the device configuration structure containing information
*           about a specific VISP SS device.
* @param    EffectiveAddr is the base address of the device.
*
* @return   XST_SUCCESS if the initialization is successful.
*           XST_DEVICE_IS_STARTED if the device has already been started.
*
* @note     None.
*
******************************************************************************/
int XVisp_Ss_CfgInitialize(XVisp_Ss *InstancePtr, xvisp_ss_Config *CfgPtr,
			   UINTPTR EffectiveAddr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != 0);

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XVisp_Ss));
	(void)memcpy((void *) & (InstancePtr->Config), (const void *)CfgPtr,
		     sizeof(xvisp_ss_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/*
	 * Set all handlers to stub values, let user configure this data later
	 */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

void reset_isp2rpu_mapping()
{
	for (int i = 0; i < XPAR_XVISP_SS_NUM_INSTANCES; i++)
		isp2rpu_map.reg[i] = ISPMAP2NONE;
}
void init_isp2rpu_mapping(u32 rpu_id, u32 isp_id)
{

	switch (rpu_id) {
		case HW_RPU_ID_0:
			isp2rpu_map.reg[isp_id] = MAPISP2RPU0;
			break;
		case HW_RPU_ID_1:
			isp2rpu_map.reg[isp_id] = MAPISP2RPU1;
			break;
		case HW_RPU_ID_2:
			isp2rpu_map.reg[isp_id] = MAPISP2RPU2;
			break;
		case HW_RPU_ID_3:
			isp2rpu_map.reg[isp_id] = MAPISP2RPU3;
			break;
		case HW_RPU_ID_4:
			isp2rpu_map.reg[isp_id] = MAPISP2RPU4;
			break;
		case HW_RPU_ID_5:
			isp2rpu_map.reg[isp_id] = MAPISP2RPU5;
			break;
		default:
			xil_printf("------------ERRROR-------------");
	}

}

/*****************************************************************************/
/**
*
* Select the destination RPU core for the given ISP instance.
*
* @param    ispid is the ISP instance ID used to look up the mapped RPU core.
*
* @return   XST_SUCCESS if the destination core is successfully selected.
*           XST_FAILURE if the ISP ID is out of range or not mapped.
*
* @note     None.
*
******************************************************************************/
int selectDestinationCore(u32 ispid)
{
	if (ispid >= XPAR_XVISP_SS_NUM_INSTANCES ||
	    isp2rpu_map.reg[ispid] == ISPMAP2NONE) {
		xil_printf("Failed to Map ISP =%x to RPU core...\n\r", ispid);
		return XST_FAILURE;
	}
	dest_cpu_id = isp2rpu_map.reg[ispid];
	return XST_SUCCESS;
}
