/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_DEVICE_PLAT_H_
#define XPM_DEVICE_PLAT_H_

#include "xpm_node.h"
#include "xpm_power.h"
#include "xpm_clock.h"
#include "xpm_reset.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IS_DEV_AIE(ID)			(((u32)XPM_NODECLASS_DEVICE == NODECLASS(ID)) && \
					 ((u32)XPM_NODESUBCL_DEV_AIE == NODESUBCLASS(ID)) && \
					 ((u32)XPM_NODEIDX_DEV_AIE != NODEINDEX(ID)))

/** PSM RAM Base address */
#define XPM_PSM_RAM_BASE_ADDR		(0xFFC00000U)
#define XPM_PSM_RAM_SIZE		(0x40000U)

#define DDRMC_DEVID(IDX)	NODEID((u32)XPM_NODECLASS_DEVICE, \
				       (u32)XPM_NODESUBCL_DEV_MEM_CTRLR, \
				       (u32)XPM_NODETYPE_DEV_DDR, (IDX))

#define GT_DEVID(IDX)		NODEID((u32)XPM_NODECLASS_DEVICE, \
				       (u32)XPM_NODESUBCL_DEV_PHY, \
				       (u32)XPM_NODETYPE_DEV_GT, (IDX))

typedef struct XPm_DeviceNode XPm_Device;

/************************** Function Prototypes ******************************/
maybe_unused static u8 XPmDevice_IsRequestable(u32 NodeId)
{
	u8 Requestable = 0U;

	switch (NODESUBCLASS(NodeId)) {
	case (u32)XPM_NODESUBCL_DEV_CORE:
	case (u32)XPM_NODESUBCL_DEV_PERIPH:
	case (u32)XPM_NODESUBCL_DEV_MEM:
	case (u32)XPM_NODESUBCL_DEV_MEM_CTRLR:
	case (u32)XPM_NODESUBCL_DEV_PL:
	case (u32)XPM_NODESUBCL_DEV_AIE:
		Requestable = 1U;
		break;
	default:
		Requestable = 0U;
		break;
	}

	return Requestable;
}

/************************** Function Prototypes ******************************/
XStatus AddDevAttributes(const u32 *Args, const u32 NumArgs);
XStatus XPmDevice_ConfigureADMA(const u32 Id);
XStatus XPmDevice_SdResetWorkaround(const XPm_Device *Device);
XStatus HandleDeviceAttr(struct XPm_Reqm *Reqm, u32 ReqCaps,
				u32 PrevState, u32 Enable);
XStatus XPmDevice_PlatAddParent(const u32 Id, const u32 ParentId);
struct XPm_Reqm *XPmDevice_GetAieReqm(XPm_Device *Device, XPm_Subsystem *Subsystem);
void PlatDevRequest(const XPm_Device *Device, const XPm_Subsystem *Subsystem, const u32 QoS, XStatus *Status);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_DEVICE_PLAT_H_ */
