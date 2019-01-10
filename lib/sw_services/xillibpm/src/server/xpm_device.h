/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#ifndef XPM_DEVICE_H_
#define XPM_DEVICE_H_

#include "xpm_node.h"
#include "xpm_power.h"
#include "xpm_clock.h"
#include "xpm_reset.h"
#include "xpm_requirement.h"


/* Processor core device IDs */
#define APU_DEVID(IDX)		NODEID(XPM_NODECLASS_DEVICE, \
	XPM_NODESUBCL_DEV_CORE, XPM_NODETYPE_DEV_CORE_APU, (IDX))
#define RPU_DEVID(IDX)		NODEID(XPM_NODECLASS_DEVICE, \
	XPM_NODESUBCL_DEV_CORE, XPM_NODETYPE_DEV_CORE_RPU, (IDX))

#define XPM_DEVID_PSM		NODEID(XPM_NODECLASS_DEVICE, \
	XPM_NODESUBCL_DEV_CORE, XPM_NODETYPE_DEV_CORE_PSM, XPM_NODEIDX_DEV_PSM_PROC)
#define XPM_DEVID_PMC		NODEID(XPM_NODECLASS_DEVICE, \
	XPM_NODESUBCL_DEV_CORE, XPM_NODETYPE_DEV_CORE_PMC, XPM_NODEIDX_DEV_PMC_PROC)
#define XPM_DEVID_ACPU_0	APU_DEVID(XPM_NODEIDX_DEV_ACPU_0)
#define XPM_DEVID_ACPU_1	APU_DEVID(XPM_NODEIDX_DEV_ACPU_1)
#define XPM_DEVID_R50_0		RPU_DEVID(XPM_NODEIDX_DEV_RPU0_0)
#define XPM_DEVID_R50_1		RPU_DEVID(XPM_NODEIDX_DEV_RPU0_1)

/* Memory bank device IDs */
#define OCM_DEVID(IDX)		NODEID(XPM_NODECLASS_DEVICE, \
	XPM_NODESUBCL_DEV_MEM, XPM_NODETYPE_DEV_OCM, (IDX))
#define TCM_DEVID(IDX)		NODEID(XPM_NODECLASS_DEVICE, \
	XPM_NODESUBCL_DEV_MEM, XPM_NODETYPE_DEV_TCM, (IDX))

#define XPM_DEVID_OCM_0		OCM_DEVID(XPM_NODEIDX_DEV_OCM_0)
#define XPM_DEVID_OCM_1		OCM_DEVID(XPM_NODEIDX_DEV_OCM_1)
#define XPM_DEVID_OCM_2		OCM_DEVID(XPM_NODEIDX_DEV_OCM_2)
#define XPM_DEVID_OCM_3		OCM_DEVID(XPM_NODEIDX_DEV_OCM_3)
#define XPM_DEVID_TCM_0_A	TCM_DEVID(XPM_NODEIDX_DEV_TCM_0_A)
#define XPM_DEVID_TCM_0_B	TCM_DEVID(XPM_NODEIDX_DEV_TCM_0_B)
#define XPM_DEVID_TCM_1_A	TCM_DEVID(XPM_NODEIDX_DEV_TCM_1_A)
#define XPM_DEVID_TCM_1_B	TCM_DEVID(XPM_NODEIDX_DEV_TCM_1_B)
#define XPM_DEVID_L2CACHE	NODEID(XPM_NODECLASS_DEVICE, \
	XPM_NODESUBCL_DEV_MEM, XPM_NODETYPE_DEV_L2CACHE, XPM_NODEIDX_DEV_L2_BANK_0)

/* Peripheral device IDs */
#define PERIPH_DEVID(IDX)	NODEID(XPM_NODECLASS_DEVICE, \
	XPM_NODESUBCL_DEV_PERIPH, XPM_NODETYPE_DEV_PERIPH, (IDX))

#define XPM_DEVID_USB_0		PERIPH_DEVID(XPM_NODEIDX_DEV_USB_0)
#define XPM_DEVID_GEM_0		PERIPH_DEVID(XPM_NODEIDX_DEV_GEM_0)
#define XPM_DEVID_GEM_1		PERIPH_DEVID(XPM_NODEIDX_DEV_GEM_1)
#define XPM_DEVID_SPI_0		PERIPH_DEVID(XPM_NODEIDX_DEV_SPI_0)
#define XPM_DEVID_SPI_1		PERIPH_DEVID(XPM_NODEIDX_DEV_SPI_1)
#define XPM_DEVID_I2C_0		PERIPH_DEVID(XPM_NODEIDX_DEV_I2C_0)
#define XPM_DEVID_I2C_1		PERIPH_DEVID(XPM_NODEIDX_DEV_I2C_1)
#define XPM_DEVID_CAN_FD_0	PERIPH_DEVID(XPM_NODEIDX_DEV_CAN_FD_0)
#define XPM_DEVID_CAN_FD_1	PERIPH_DEVID(XPM_NODEIDX_DEV_CAN_FD_1)
#define XPM_DEVID_UART_0	PERIPH_DEVID(XPM_NODEIDX_DEV_UART_0)
#define XPM_DEVID_UART_1	PERIPH_DEVID(XPM_NODEIDX_DEV_UART_1)
#define XPM_DEVID_GPIO		PERIPH_DEVID(XPM_NODEIDX_DEV_GPIO)
#define XPM_DEVID_TTC_0		PERIPH_DEVID(XPM_NODEIDX_DEV_TTC_0)
#define XPM_DEVID_TTC_1		PERIPH_DEVID(XPM_NODEIDX_DEV_TTC_1)
#define XPM_DEVID_TTC_2		PERIPH_DEVID(XPM_NODEIDX_DEV_TTC_2)
#define XPM_DEVID_TTC_3		PERIPH_DEVID(XPM_NODEIDX_DEV_TTC_3)
#define XPM_DEVID_SWDT_LPD	PERIPH_DEVID(XPM_NODEIDX_DEV_SWDT_LPD)
#define XPM_DEVID_SWDT_FPD	PERIPH_DEVID(XPM_NODEIDX_DEV_SWDT_FPD)
#define XPM_DEVID_OSPI		PERIPH_DEVID(XPM_NODEIDX_DEV_OSPI)
#define XPM_DEVID_QSPI		PERIPH_DEVID(XPM_NODEIDX_DEV_QSPI)
#define XPM_DEVID_GPIO_PMC	PERIPH_DEVID(XPM_NODEIDX_DEV_GPIO_PMC)
#define XPM_DEVID_I2C_PMC	PERIPH_DEVID(XPM_NODEIDX_DEV_I2C_PMC)
#define XPM_DEVID_SDIO_0	PERIPH_DEVID(XPM_NODEIDX_DEV_SDIO_0)
#define XPM_DEVID_SDIO_1	PERIPH_DEVID(XPM_NODEIDX_DEV_SDIO_1)

/* Device states */
typedef enum {
	XPM_DEVSTATE_UNUSED,
	XPM_DEVSTATE_RUNNING,
	XPM_DEVSTATE_PWR_ON,	/* Power up the island/domain */
	XPM_DEVSTATE_CLK_ON,	/* Enable clock */
	XPM_DEVSTATE_RST_OFF,	/* De-assert reset */
	XPM_DEVSTATE_RST_ON,	/* Assert reset */
	XPM_DEVSTATE_CLK_OFF,	/* Disable clock */
	XPM_DEVSTATE_PWR_OFF,	/* Power down */
	XPM_DEVSTATE_SUSPENDING,
} XPm_DeviceState;

/* Device events */
typedef enum {
	XPM_DEVEVENT_BRINGUP_ALL,
	XPM_DEVEVENT_BRINGUP_CLKRST,
	XPM_DEVEVENT_SHUTDOWN,
	XPM_DEVEVENT_TIMER,
} XPm_DeviceEvent;

typedef struct XPm_Subsystem XPm_Subsystem;
typedef struct XPm_Requirement XPm_Requirement;
typedef struct XPm_Device XPm_Device;
typedef struct XPm_DeviceOps XPm_DeviceOps;
typedef struct XPm_DeviceStatus XPm_DeviceStatus;

/* Device Operations */
struct XPm_DeviceOps {
	XStatus (*Request)(XPm_Device *Device,
		XPm_Subsystem *Subsystem,
		u32 Capabilities, const u32 Latency, const u32 QoS);
		/**< Request: Request the device */

	XStatus (*SetRequirement)(XPm_Device *Device,
		XPm_Subsystem *Subsystem,
		u32 Capabilities, const u32 Latency, const u32 QoS);
		/**< SetRequirement: Set the device requirement */

	XStatus (*Release)(XPm_Device *Device,
		XPm_Subsystem *Subsystem);
		/**< Release: Release the device */
};

/* Reset node list allocated to device */
typedef struct XPm_ResetNodeList {
	XPm_ResetNode *Reset; /**< Reset pointer for current node */
	struct XPm_ResetNodeList *NextNode; /**< Pointer to next node in list */
} XPm_ResetNodeList;

/**
 * The device class.  This is the base class for all the processor core,
 * memory bank and peripheral classes.
 */
struct XPm_Device {
	XPm_Node Node; /**< Node: Base class */
	XPm_Power *Power; /**< Device power node */
	XPm_ClockHandle *ClkHandles; /**< Head of the list of device clocks */
	XPm_ResetNodeList *ResetList; /**< Device reset node list */
	XPm_Requirement *Requirements;
		/**< Head of the list of requirements for all subsystems */

	XPm_Requirement *PendingReqm; /**< Requirement being updated */
	u8 WfDealloc; /**< Deallocation is pending */
	u8 WfPwrUseCnt; /**< Pending power use count */

	XPm_DeviceOps *DeviceOps; /**< Device operations */
};

typedef struct XPm_Mem {
	XPm_Device Device; /**< Device: Base class */
	u32 StartAddress;
	u32 EndAddress;
} XPm_Mem;

extern XPm_Device *PmDevices[];

/************************** Function Prototypes ******************************/
XStatus XPmDevice_Init(XPm_Device *Device,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset);

XStatus XPmDevice_SetPower(XPm_Device *Device, XPm_Power *Power);

XStatus XPmDevice_AddClock(XPm_Device *Device, XPm_ClockNode *Clock);

XStatus XPmDevice_AddReset(XPm_Device *Device, XPm_ResetNode *Reset);

XStatus XPmDevice_Reset(XPm_Device *Device, const XPm_ResetActions Action);

u8 XPmDevice_IsAllocated(u32 DeviceId, XPm_Subsystem *Subsystem);

XPm_Device *XPmDevice_GetById(const u32 DeviceId);

XStatus XPm_CheckCapabilities(XPm_Device *Device, XPm_Subsystem *Subsystem, u32 Capabilities);

XStatus XPmDevice_Request(const u32 TargetSubsystemId,
			const u32 DeviceId,
			const u32 Capabilities,
			const u32 Latency,
			const u32 QoS);

XStatus XPmDevice_Release(const u32 SubsystemId, const u32 DeviceId);

XStatus XPmDevice_SetRequirement(const u32 SubsystemId,
			const u32 DeviceId,
			const u32 Capabilities,
			const u32 Latency,
			const u32 QoS);

XStatus XPmDevice_GetStatus(const u32 SubsystemId,
			const u32 DeviceId,
			XPm_DeviceStatus *const DeviceStatus);

XStatus XPmDevice_AddParent(u32 Id, u32 *Parents, u32 NumParents);
XStatus XPmDevice_GetPermissions(XPm_Device *Device, u32 *PermissionMask);

/** @} */
#endif /* XPM_DEVICE_H_ */
