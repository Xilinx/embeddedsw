/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xpmonpsv.h
* @addtogroup pmonpsv_v1_0
* @{
* @details
*
* The XpsvPmon driver supports the Xilinx Performance Monitor device.
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
* XpsvPmon_CfgInitialize() API is used to initialize the AXI Performance Monitor
* device. The user needs to first call the XpsvPmon_LookupConfig() API which
* returns the Configuration structure pointer which is passed as a parameter to
* the XAxiPmon_CfgInitialize() API.
*
*
* <b>Interrupts</b>
*
* The AXI Performance Monitor does not support Interrupts
*
* <b> How to read the counters </b>
* XpsvPmon_GetReadCounter returns the contents of the Read response and request counters.
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
} XPmonpsv_Config;


/**
 * The driver's instance data. The user is required to allocate a variable
 * of this type for every Performance Monitor device in system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	XPmonpsv_Config Config;	/**< XPmonpsv_Config of current device */
	u32  IsReady;		/**< Device is initialized and ready  */
	u32  RequestedCounters[2];	/**< Number of counters requested in each domain */
} XpsvPmon;

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

/**
 * Functions in xpmonpsv_sinit.c
 */
XPmonpsv_Config *XpsvPmon_LookupConfig(u16 DeviceId);

/**
 * Functions in xpmonpsv.c
 */
s32 XpsvPmon_CfgInitialize(XpsvPmon *InstancePtr,
		const XPmonpsv_Config *ConfigPtr, UINTPTR EffectiveAddr);

u32 XpsvPmon_ResetCounter(const XpsvPmon *InstancePtr, u32 Domain, u32 CounterNum);

u32 XpsvPmon_SetMetrics(const XpsvPmon *InstancePtr, u32 StatPeriod, u32 Domain, u32 CounterNum);

s32 XpsvPmon_GetMetrics(const XpsvPmon *InstancePtr, u32 CounterNum, u8 *MainCtl,
						u8 *StatPeriod ,u32 Domain);
s32 XpsvPmon_RequestCounter(XpsvPmon *InstancePtr,u32 Domain, u32 *CounterNum);

s32 XpsvPmon_GetWriteCounter(const XpsvPmon *InstancePtr,u32 *WriteRequestValue,
				u32 *WriteRespValue, u32 Domain, u32 CounterNum);

s32 XpsvPmon_GetReadCounter(const XpsvPmon *InstancePtr,u32 *ReadRequestValue,
				u32 *ReadRespValue, u32 Domain, u32 CounterNum);

s32 XpsvPmon_EnableCounters(const XpsvPmon *InstancePtr, u32 Domain, u32 CounterNum);

s32 XpsvPmon_StopCounter(const XpsvPmon *InstancePtr, u32 Domain, u32 CounterNum);
s32 XpsvPmon_SetPort(const XpsvPmon *InstancePtr, u32 PortSel, u32 Domain, u32 CounterNum);
s32 XpsvPmon_SetSrc(const XpsvPmon *InstancePtr, u32 SrcSel , u32 Domain, u32 CounterNum);

s32 XpsvPmon_Unlock(const XpsvPmon *InstancePtr);
s32 XpsvPmon_Lock(const XpsvPmon *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif  /* End of protection macro. */
/** @} */
