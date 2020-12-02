/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xpmonpsv.h
* @addtogroup pmonpsv_v2_1
* @{
* @details
*
* The XPmonPsv driver supports the Xilinx Performance Monitor device.
*
* For a full description of Performance Monitor features, please see the hardware spec. This
* driver supports the following features:
*
* Computes performance metrics for Agents connected to slots
*   - Read request and response counters
*   - Lock and unlock the performance monitor.
*   - Setting metrics like the statperiod.
*   - Readback of the metrics.
*
* <b> Initialization and Configuration </b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the Performance Monitor device.
*
* XPmonPsv_CfgInitialize() API is used to initialize the AXI Performance Monitor
* device. The user needs to first call the XPmonPsv_LookupConfig() API which
* returns the Configuration structure pointer which is passed as a parameter to
* the XAxiPmon_CfgInitialize() API.
*
*
* <b>Interrupts</b>
*
* The AXI Performance Monitor does not support Interrupts
*
* <b> How to read the counters </b>
* XPmonPsv_GetReadCounter returns the contents of the Read response and request counters.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* TODO:
*    Add the filter support
*
* <br><br>
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0 sd   01/20/19 First release
* 2.0 sd   04/22/20  Rename the APIs
* </pre>
*
*****************************************************************************/
#ifndef XPSVPMON_H /* Prevent circular inclusions */
#define XPSVPMON_H /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xpmonpsv_hw.h"
/************************** Constant Definitions ****************************/
#define XPMONPSV_R5_DOMAIN		0U
#define XPMONPSV_LPD_MAIN_DOMAIN	1U
#define XPMONPSV_MAX_COUNTERS		10U

/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for the Performance
 * Monitor device.
 */
typedef struct {
	u16 DeviceId;			/**< Unique ID of device */
	UINTPTR BaseAddress;		/**< Device base address */
} XPmonPsv_Config;


/**
 * The driver's instance data. The user is required to allocate a variable
 * of this type for every Performance Monitor device in system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	XPmonPsv_Config Config;	/**< XPmonPsv_Config of current device */
	u32  IsReady;		/**< Device is initialized and ready  */
	u32  RequestedCounters[2];	/**< Number of counters requested in each domain */
} XPmonPsv;

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

/**
 * Functions in xpmonpsv_sinit.c
 */
XPmonPsv_Config *XPmonPsv_LookupConfig(u16 DeviceId);

/**
 * Functions in xpmonpsv.c
 */
u32 XPmonPsv_CfgInitialize(XPmonPsv *InstancePtr,
		const XPmonPsv_Config *ConfigPtr, UINTPTR EffectiveAddr);

u32 XPmonPsv_ResetCounter(const XPmonPsv *InstancePtr, u32 Domain, u32 CounterNum);

u32 XPmonPsv_SetMetrics(const XPmonPsv *InstancePtr, u32 StatPeriod, u32 Domain, u32 CounterNum);

u32 XPmonPsv_GetMetrics(const XPmonPsv *InstancePtr, u32 CounterNum, u8 *MainCtl,
						u8 *StatPeriod ,u32 Domain);
u32 XPmonPsv_RequestCounter(XPmonPsv *InstancePtr,u32 Domain, u32 *CounterNum);

u32 XPmonPsv_GetWriteCounter(const XPmonPsv *InstancePtr,u32 *WriteRequestValue,
				u32 *WriteRespValue, u32 Domain, u32 CounterNum);

u32 XPmonPsv_GetReadCounter(const XPmonPsv *InstancePtr,u32 *ReadRequestValue,
				u32 *ReadRespValue, u32 Domain, u32 CounterNum);

u32 XPmonPsv_EnableCounters(const XPmonPsv *InstancePtr, u32 Domain, u32 CounterNum);

u32 XPmonPsv_StopCounter(const XPmonPsv *InstancePtr, u32 Domain, u32 CounterNum);
u32 XPmonPsv_SetPort(const XPmonPsv *InstancePtr, u32 PortSel, u32 Domain, u32 CounterNum);
u32 XPmonPsv_SetSrc(const XPmonPsv *InstancePtr, u32 SrcSel , u32 Domain, u32 CounterNum);

void XPmonPsv_Unlock(const XPmonPsv *InstancePtr);
void XPmonPsv_Lock(const XPmonPsv *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif  /* End of protection macro. */
/** @} */
