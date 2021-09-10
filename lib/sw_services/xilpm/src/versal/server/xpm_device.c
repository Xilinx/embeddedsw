/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi_dma.h"
#include "xplmi.h"
#include "xpm_device.h"
#include "xpm_core.h"
#include "xpm_regs.h"
#include "xpm_prot.h"
#include "xpm_rpucore.h"
#include "xpm_notifier.h"
#include "xpm_api.h"
#include "xpm_pmc.h"
#include "xpm_mem.h"
#include "xpm_pslpdomain.h"
#include "xpm_psfpdomain.h"
#include "xpm_requirement.h"
#include "xpm_debug.h"
#include "xpm_pldevice.h"
#include "xpm_aiedevice.h"

/** PSM RAM Base address */
#define XPM_PSM_RAM_BASE_ADDR           (0xFFC00000U)
#define XPM_PSM_RAM_SIZE                (0x40000U)

#define SD_DLL_DIV_MAP_RESET_VAL	(0x50505050U)

/* Device security bit in register */
#define DEV_NONSECURE			(1U)
#define DEV_SECURE			(0U)

static const char *PmDevStates[] = {
	"UNUSED",
	"RUNNING",
	"PWR_ON",
	"CLK_ON",
	"RST_OFF",
	"RST_ON",
	"CLK_OFF",
	"PWR_OFF",
	"SUSPENDING",
	"RUNTIME_SUSPEND",
	"INITIALIZING",
};

static const char *PmDevEvents[] = {
	"BRINGUP_ALL",
	"BRINGUP_CLKRST",
	"SHUTDOWN",
	"TIMER",
};

static const u32 IpiMasks[][2] = {
	{ PM_DEV_IPI_0, IPI_0_MASK },
	{ PM_DEV_IPI_1, IPI_1_MASK },
	{ PM_DEV_IPI_2, IPI_2_MASK },
	{ PM_DEV_IPI_3, IPI_3_MASK },
	{ PM_DEV_IPI_4, IPI_4_MASK },
	{ PM_DEV_IPI_5, IPI_5_MASK },
	{ PM_DEV_IPI_6, IPI_6_MASK },
};

static XPm_DeviceOps PmDeviceOps;
static XPm_Device *PmDevices[(u32)XPM_NODEIDX_DEV_MAX];
static XPm_Device *PmPlDevices[(u32)XPM_NODEIDX_DEV_PLD_MAX];
static XPm_Device *PmAieDevices[(u32)XPM_NODEIDX_DEV_AIE_MAX];
static XPm_Device *PmOcmMemRegnDevices[(u32)MEM_REGN_DEV_NODE_MAX];
static XPm_Device *PmDdrMemRegnDevices[(u32)MEM_REGN_DEV_NODE_MAX];
static XPm_Device *PmVirtualDevices[(u32)XPM_NODEIDX_DEV_VIRT_MAX];
static XPm_Device *PmHbMonDevices[(u32)XPM_NODEIDX_DEV_HB_MON_MAX];
static u32 PmNumDevices;
static u32 PmNumPlDevices;
static u32 PmNumOcmMemRegnDevices;
static u32 PmNumDdrMemRegnDevices;
static u32 PmNumVirtualDevices;
static u32 PmNumHbMonDevices;
static u32 PmNumAieDevices;
static u32 PmSysmonAddresses[(u32)XPM_NODEIDX_MONITOR_MAX];

static const XPm_StateCap XPmGenericDeviceStates[] = {
	{
		.State = (u8)XPM_DEVSTATE_UNUSED,
		.Cap = XPM_MIN_CAPABILITY,
	}, {
		.State = (u8)XPM_DEVSTATE_RUNTIME_SUSPEND,
		.Cap = (u32)PM_CAP_UNUSABLE,
	}, {
		.State = (u8)XPM_DEVSTATE_RUNNING,
		.Cap = XPM_MAX_CAPABILITY | (u32)PM_CAP_UNUSABLE,
	},
};

static const XPm_StateTran XPmGenericDevTransitions[] = {
	{
		.FromState = (u32)XPM_DEVSTATE_RUNNING,
		.ToState = (u32)XPM_DEVSTATE_UNUSED,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_UNUSED,
		.ToState = (u32)XPM_DEVSTATE_RUNNING,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_RUNTIME_SUSPEND,
		.ToState = (u32)XPM_DEVSTATE_UNUSED,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_RUNTIME_SUSPEND,
		.ToState = (u32)XPM_DEVSTATE_RUNNING,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_UNUSED,
		.ToState = (u32)XPM_DEVSTATE_RUNTIME_SUSPEND,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_RUNNING,
		.ToState = (u32)XPM_DEVSTATE_RUNTIME_SUSPEND,
		.Latency = XPM_DEF_LATENCY,
	},
};

static XPm_Requirement *FindReqm(const XPm_Device *Device, const XPm_Subsystem *Subsystem)
{
	XPm_Requirement *Reqm = NULL;

	Reqm = Device->Requirements;
	while (NULL != Reqm) {
		if (Reqm->Subsystem == Subsystem) {
			break;
		}
		Reqm = Reqm->NextSubsystem;
	}

	return Reqm;
}

struct XPm_Reqm *XPmDevice_FindRequirement(const u32 DeviceId, const u32 SubsystemId)
{
	const XPm_Device *Device = XPmDevice_GetById(DeviceId);
	const XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);
	XPm_Requirement *Reqm = NULL;

	if ((NULL == Device) || (NULL == Subsystem)) {
		goto done;
	}

	Reqm = FindReqm(Device, Subsystem);
done:
	return Reqm;
}

static XStatus SetDeviceNode(u32 Id, XPm_Device *Device)
{
	XStatus Status = XST_INVALID_PARAM;
	u32 NodeIndex = NODEINDEX(Id);

	/*
	 * We assume that the Node ID class, subclass and type has _already_
	 * been validated before, so only check bounds here against index
	 */
	if ((NULL != Device) && ((u32)XPM_NODEIDX_DEV_MAX > NodeIndex)) {
		PmDevices[NodeIndex] = Device;
		PmNumDevices++;
		Status = XST_SUCCESS;
	}

	return Status;
}

static XStatus SetPlDeviceNode(u32 Id, XPm_Device *Device)
{
	XStatus Status = XST_INVALID_PARAM;
	u32 NodeIndex = NODEINDEX(Id);

	/*
	 * Node ID class, subclass and type should _already_ been validated
	 * before, so only check bounds here against index.
	 */
	if ((NULL != Device) && ((u32)XPM_NODEIDX_DEV_PLD_MAX > NodeIndex)) {
		PmPlDevices[NodeIndex] = Device;
		PmNumPlDevices++;
		Status = XST_SUCCESS;
	}

	return Status;
}

static XStatus SetVirtDeviceNode(u32 Id, XPm_Device *Device)
{
	XStatus Status = XST_INVALID_PARAM;
	u32 NodeIndex = NODEINDEX(Id);

	if ((NULL != Device) && ((u32)XPM_NODEIDX_DEV_VIRT_MAX > NodeIndex)) {
		PmVirtualDevices[NodeIndex] = Device;
		PmNumVirtualDevices++;
		Status = XST_SUCCESS;
	}

	return Status;
}

static XStatus SetHbMonDeviceNode(u32 Id, XPm_Device *Device)
{
	XStatus Status = XST_INVALID_PARAM;
	u32 NodeIndex = NODEINDEX(Id);

	if ((NULL != Device) && ((u32)XPM_NODEIDX_DEV_HB_MON_MAX > NodeIndex)) {
		PmHbMonDevices[NodeIndex] = Device;
		PmNumHbMonDevices++;
		Status = XST_SUCCESS;
	}

	return Status;
}

static XStatus SetMemRegnDeviceNode(u32 Id, XPm_Device *Device)
{
	XStatus Status = XST_INVALID_PARAM;
	u32 NodeType = NODETYPE(Id);
	u32 NodeIndex = NODEINDEX(Id);
	XPm_Device **MemRegDevices = NULL;
	u32 *NumMemRegDevices = NULL;

	switch (NodeType) {
	case (u8)XPM_NODETYPE_DEV_OCM_REGN:
		MemRegDevices = PmOcmMemRegnDevices;
		NumMemRegDevices = &PmNumOcmMemRegnDevices;
		break;
	case (u8)XPM_NODETYPE_DEV_DDR_REGN:
		MemRegDevices = PmDdrMemRegnDevices;
		NumMemRegDevices = &PmNumDdrMemRegnDevices;
		break;
	default:
		PmDbg("Memory type other than OCM or DDR\r\n");
		break;
	}

	if (NULL == NumMemRegDevices) {
		goto done;
	}

	/*
	 * We assume that the Node ID class, subclass and type has _already_
	 * been validated before, so only check bounds here against index
	 */
	if ((NULL != Device) && ((u32)MEM_REGN_DEV_NODE_MAX > NodeIndex)) {

		MemRegDevices[NodeIndex] = Device;
		(*NumMemRegDevices)++;
		Status = XST_SUCCESS;
	}

done:
	return Status;
}

static XStatus SetAieDeviceNode(u32 Id, XPm_Device *Device)
{
	XStatus Status = XST_FAILURE;
	u32 NodeIdx = NODEINDEX(Id);

	if ((NULL != Device) && ((u32)XPM_NODEIDX_DEV_AIE_MAX > NodeIdx)) {
		PmAieDevices[NodeIdx] = Device;
		PmNumAieDevices++;
		Status = XST_SUCCESS;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief	Get subsystem ID of processor
 *
 * @param  Device	Processor whose subsystem needs to found
 *
 * @return	Subsystem ID of that processor
 *
 * @note	Core must be requested from single subsystem. If it is
 *		requested from multiple subsystems then it returns only one
 *		subsystem ID and if it is not requested from any subsystem
 *		then this function returns maximum subsystem ID which is
 *		invalid.
 *
 ****************************************************************************/
u32 XPmDevice_GetSubsystemIdOfCore(const XPm_Device *Device)
{
	const XPm_Requirement *Reqm;
	const XPm_Subsystem *Subsystem = NULL;
	u32 Idx, SubSystemId;
	u32 SubsysIdx = XPmSubsystem_GetMaxSubsysIdx();

	for (Idx = 0; Idx <= SubsysIdx; Idx++) {
		Subsystem = XPmSubsystem_GetByIndex(Idx);
		if (NULL != Subsystem) {
			Reqm = FindReqm(Device, Subsystem);
			if ((NULL != Reqm) && (1U == Reqm->Allocated)) {
				break;
			}
		}
	}

	if (SubsysIdx < Idx) {
		SubSystemId = INVALID_SUBSYSID;
	} else {
		SubSystemId = Subsystem->Id;
	}

	return SubSystemId;
}

/****************************************************************************/
/**
 * @brief	Get maximum of all requested capabilities of device
 * @param Device	Device whose maximum required capabilities should be
 *			determined
 *
 * @return	32bit value encoding the capabilities
 *
 * @note	None
 *
 ****************************************************************************/
static u32 GetMaxCapabilities(const XPm_Device* const Device)
{
	const XPm_Requirement* Reqm = Device->Requirements;
	u32 MaxCaps = 0U;

	while (NULL != Reqm) {
		MaxCaps |= Reqm->Curr.Capabilities;
		Reqm = Reqm->NextSubsystem;
	}

	return MaxCaps;
}

/****************************************************************************/
/**
 * @brief  This function checks device capability
 *
 * @param Device	Device for capability check
 * @param Caps		Capability
 *
 * @return XST_SUCCESS if desired Caps is available in Device
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPm_CheckCapabilities(const XPm_Device *Device, u32 Caps)
{
	u32 Idx;
	XStatus Status = XST_FAILURE;

	if (NULL == Device->DeviceFsm) {
		goto done;
	}

	for (Idx = 0U; Idx < Device->DeviceFsm->StatesCnt; Idx++) {
		/* Find the first state that contains all capabilities */
		if ((Caps & Device->DeviceFsm->States[Idx].Cap) == Caps) {
			Status = XST_SUCCESS;
			break;
		}
	}

done:
	if (Status != XST_SUCCESS) {
		Status = XST_NO_FEATURE;
	}
	return Status;
}

static u32 IsRunning(const XPm_Device *Device)
{
	u32 Running = 0;
	const XPm_Requirement *Reqm = Device->Requirements;

	while (NULL != Reqm) {
		if (Reqm->Allocated > 0U) {
			if (Reqm->Curr.Capabilities > 0U) {
				Running = 1;
				break;
			}
		}
		Reqm = Reqm->NextSubsystem;
	}

	return Running;
}

XStatus XPmDevice_BringUp(XPm_Device *Device)
{
	XStatus Status = XPM_ERR_DEVICE_BRINGUP;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (NULL == Device->Power) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		goto done;
	}

	/* Check if device is already up and running */
	if (Device->Node.State == (u8)XPM_DEVSTATE_RUNNING) {
		Status = XST_SUCCESS;
		goto done;
	}

	Device->WfPwrUseCnt = Device->Power->UseCount + 1U;
	Status = Device->Power->HandleEvent(&Device->Power->Node,
					    XPM_POWER_EVENT_PWR_UP);
	if (XST_SUCCESS == Status) {
		Device->Node.State = (u8)XPM_DEVSTATE_PWR_ON;
		/* Todo: Start timer to poll the power node */
		/* Hack */
		Status = Device->HandleEvent(&Device->Node, XPM_DEVEVENT_TIMER);
	} else {
		DbgErr = XPM_INT_ERR_DEVICE_PWR_PARENT_UP;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);

	return Status;
}

static XStatus SetClocks(const XPm_Device *Device, u32 Enable)
{
	XStatus Status = XST_FAILURE;
	u32 NodeId;

	/*
	 * Clocks for PLD and AIE is handled by CDO, hence do not handle in
	 * firmware. Skip for PLD & AIE Devices
	*/
	NodeId = Device->Node.Id;
	if (((u32)XPM_NODESUBCL_DEV_PL == NODESUBCLASS(NodeId)) ||
	    ((u32)XPM_NODESUBCL_DEV_AIE == NODESUBCLASS(NodeId))) {
		Status = XST_SUCCESS;
		goto done;
	}

	const XPm_ClockHandle *ClkHandle = Device->ClkHandles;

	/* Enable all the clock gates, skip over others */
	if (1U == Enable) {
		Status = XPmClock_Request(ClkHandle);
	} else {
		Status = XPmClock_Release(ClkHandle);
	}

done:
	return Status;
}

static XStatus ResetSdDllRegs(const XPm_Device *Device)
{
	XStatus Status = XST_FAILURE;
	u32 Value;
	u32 BaseAddress;
	const XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	BaseAddress = Pmc->PmcIouSlcrBaseAddr;
	if (PM_DEV_SDIO_0 == Device->Node.Id) {
		PmIn32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP0_OFFSET,
		       Value);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP0_OFFSET,
			SD_DLL_DIV_MAP_RESET_VAL);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP0_OFFSET,
			Value);
		PmIn32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP1_OFFSET,
		       Value);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP1_OFFSET,
			SD_DLL_DIV_MAP_RESET_VAL);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP1_OFFSET,
			Value);
	} else if (PM_DEV_SDIO_1 == Device->Node.Id) {
		PmIn32(BaseAddress + PMC_IOU_SLCR_SD1_DLL_DIV_MAP0_OFFSET,
		       Value);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD1_DLL_DIV_MAP0_OFFSET,
			SD_DLL_DIV_MAP_RESET_VAL);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD1_DLL_DIV_MAP0_OFFSET,
			Value);
		PmIn32(BaseAddress + PMC_IOU_SLCR_SD1_DLL_DIV_MAP1_OFFSET,
		       Value);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD1_DLL_DIV_MAP1_OFFSET,
			SD_DLL_DIV_MAP_RESET_VAL);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD1_DLL_DIV_MAP1_OFFSET,
			Value);
	} else {
		/* Required by MISRA */
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus HandleDeviceEvent(XPm_Node *Node, u32 Event)
{
	XStatus Status = XST_FAILURE;
	XPm_Device *Device = (XPm_Device *)Node;
	XPm_Core *Core;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	PmDbg("ID=0x%x State=%s, Event=%s\n\r", Node->Id, PmDevStates[Node->State], PmDevEvents[Event]);

	switch(Node->State)
	{
		case (u8)XPM_DEVSTATE_UNUSED:
			if ((u32)XPM_DEVEVENT_BRINGUP_ALL == Event) {
				Status = Device->DeviceFsm->EnterState(Device, XPM_DEVSTATE_RUNNING);
			} else if ((u32)XPM_DEVEVENT_SHUTDOWN == Event) {
				Status = XST_SUCCESS;
			} else {
				DbgErr = XPM_INT_ERR_INVALID_EVENT;
			}
			break;
		case (u8)XPM_DEVSTATE_PWR_ON:
			if ((u32)XPM_DEVEVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				if (Device->WfPwrUseCnt == Device->Power->UseCount) {
					Node->State = (u8)XPM_DEVSTATE_CLK_ON;
					/* Enable clock */
					Status = SetClocks(Device, 1U);
					if (XST_SUCCESS != Status) {
						DbgErr = XPM_INT_ERR_CLK_ENABLE;
						break;
					}
					/* Todo: Start timer to poll the clock node */
					/* Hack */
					Status = Device->HandleEvent(Node, (u32)XPM_DEVEVENT_TIMER);
				} else {
					/* Todo: Start timer to poll the power node */
				}
			} else {
				DbgErr = XPM_INT_ERR_DEVICE_BUSY;
				Status = XST_DEVICE_BUSY;
			}
			break;
		case (u8)XPM_DEVSTATE_CLK_ON:
			if ((u32)XPM_DEVEVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Check if clock is enabled */
				if (TRUE /* Hack: Clock enabled */) {
					Node->State = (u8)XPM_DEVSTATE_RST_OFF;

					const XPm_PsLpDomain *PsLpd;
					PsLpd = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);
					if (NULL == PsLpd) {
						DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
						Status = XST_FAILURE;
						break;
					}

					/*
					 * Configure ADMA as non-secure so Linux
					 * can use it.
					 * TODO: Remove this when security config
					 * support is added through CDO
					 */
					if ((Device->Node.Id >= PM_DEV_ADMA_0) &&
							(Device->Node.Id <= PM_DEV_ADMA_7)) {
						XPm_Out32(PsLpd->LpdSlcrSecureBaseAddr +
							  LPD_SLCR_SECURE_WPROT0_OFFSET, 0x0);
						XPm_Out32(PsLpd->LpdSlcrSecureBaseAddr +
							  LPD_SLCR_SECURE_ADMA_0_OFFSET +
							  ((Device->Node.Id - PM_DEV_ADMA_0) * 4U), 0x1);
						XPm_Out32(PsLpd->LpdSlcrSecureBaseAddr +
							  LPD_SLCR_SECURE_WPROT0_OFFSET, 0x1);
					}

					/* De-assert reset for peripheral devices */
					if ((u32)XPM_NODESUBCL_DEV_PERIPH ==
						NODESUBCLASS(Device->Node.Id)) {
						Status = XPmDevice_Reset(Device,
							PM_RESET_ACTION_RELEASE);
						if (XST_SUCCESS != Status) {
							DbgErr = XPM_INT_ERR_RST_RELEASE;
							break;
						}

						/*
						 * As per EDT-997700 SD/eMMC DLL modes are failing after
						 * SD controller reset. Reset SD_DLL_MAP registers after
						 * reset release as a workaround.
						 */
						if ((PM_DEV_SDIO_0 == Device->Node.Id) ||
						    (PM_DEV_SDIO_1 == Device->Node.Id)) {
							Status = ResetSdDllRegs(Device);
							if (XST_SUCCESS != Status) {
								DbgErr = XPM_INT_ERR_RST_SD_DLL_REGS;
								break;
							}
						}
					} else if((Node->Id == PM_DEV_RPU0_0) || (Node->Id == PM_DEV_RPU0_1)) {
						/*RPU has a special handling */
						Status = XPmRpuCore_Halt(Device);
						if (XST_SUCCESS != Status) {
							DbgErr = XPM_INT_ERR_RPU_CORE_HALT;
							break;
						}
					} else if(Node->Id == PM_DEV_PSM_PROC) {
						/* Ecc initialize PSM RAM*/
						Status = XPlmi_EccInit(XPM_PSM_RAM_BASE_ADDR, XPM_PSM_RAM_SIZE);
						if (XST_SUCCESS != Status) {
							DbgErr = XPM_INT_ERR_ECC_INIT_PSM_RAM;
							break;
						}
					} else {
						DbgErr = XPM_INT_ERR_INVALID_NODE;
					}
					/* Todo: Start timer to poll the reset node */
					/* Hack */
					Status = Device->HandleEvent(Node, (u32)XPM_DEVEVENT_TIMER);
				} else {
					/* Todo: Start timer to poll the clock node */
				}
			} else {
				Status = XST_DEVICE_BUSY;
			}
			break;
		case (u8)XPM_DEVSTATE_RST_OFF:
			if ((u32)XPM_DEVEVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Check if reset is de-asserted */
				if (TRUE /* Hack: Reset de-asserted */) {
					XPm_RequiremntUpdate(Device->PendingReqm);
					Node->State = (u8)XPM_DEVSTATE_RUNNING;
					Device->PendingReqm = NULL;
				} else {
					/* Todo: Start timer to poll the reset node */
				}
			} else {
				DbgErr = XPM_INT_ERR_DEVICE_BUSY;
				Status = XST_DEVICE_BUSY;
			}
			break;
		case (u8)XPM_DEVSTATE_RUNNING:
			if ((u32)XPM_DEVEVENT_BRINGUP_ALL == Event) {
				Status = XPmDevice_BringUp(Device);
			} else if ((u32)XPM_DEVEVENT_BRINGUP_CLKRST == Event) {
				Node->State = (u8)XPM_DEVSTATE_CLK_ON;
				/* Enable all clocks */
				Status = SetClocks(Device, 1U);
				if (XST_SUCCESS != Status) {
					DbgErr = XPM_INT_ERR_CLK_ENABLE;
					break;
				}
				/* Todo: Start timer to poll the clock node */
				/* Hack */
				Status = Device->HandleEvent(Node, (u32)XPM_DEVEVENT_TIMER);
			} else if ((u32)XPM_DEVEVENT_SHUTDOWN == Event) {
				if (((u32)XPM_NODECLASS_DEVICE == NODECLASS(Device->Node.Id)) &&
				    ((u32)XPM_NODESUBCL_DEV_CORE == NODESUBCLASS(Device->Node.Id))) {
					Core = (XPm_Core *)XPmDevice_GetById(Device->Node.Id);
					if ((NULL != Core) && (NULL != Core->CoreOps)
					    && (NULL != Core->CoreOps->PowerDown)) {
						Status = Core->CoreOps->PowerDown(Core);
						break;
					}
				}
				Node->State = (u8)XPM_DEVSTATE_RST_ON;
				/* Assert reset for peripheral devices */
				if ((u32)XPM_NODESUBCL_DEV_PERIPH ==
						NODESUBCLASS(Device->Node.Id)) {
					Status = XPmDevice_Reset(Device,
							PM_RESET_ACTION_ASSERT);
					if (XST_SUCCESS != Status) {
						DbgErr = XPM_INT_ERR_RST_ASSERT;
						break;
					}
				}
				/* Todo: Start timer to poll reset node */
				/* Hack */
				Status = Device->HandleEvent(Node, (u32)XPM_DEVEVENT_TIMER);
			} else if ((u32)XPM_DEVEVENT_RUNTIME_SUSPEND == Event) {
				Node->State = (u8)XPM_DEVSTATE_RUNTIME_SUSPEND;
				/* Disable all clocks */
				Status = SetClocks(Device, 0U);
				if (XST_SUCCESS != Status) {
					DbgErr = XPM_INT_ERR_CLK_DISABLE;
					break;
				}
			} else {
				DbgErr = XPM_INT_ERR_INVALID_EVENT;
				/* Required by MISRA */
			}
			break;
		case (u8)XPM_DEVSTATE_RST_ON:
			if ((u32)XPM_DEVEVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Check if reset is asserted */
				if (TRUE /* Hack: asserted */) {
					Node->State = (u8)XPM_DEVSTATE_CLK_OFF;
					/* Disable all clocks */
					Status = SetClocks(Device, 0U);
					if (XST_SUCCESS != Status) {
						DbgErr = XPM_INT_ERR_CLK_DISABLE;
						break;
					}
					/* Todo: Start timer to poll clock node */
					/* Hack */
					Status = Device->HandleEvent(Node, (u32)XPM_DEVEVENT_TIMER);
				} else {
					/* Todo: Start timer to poll reset node */
				}
			}
			break;
		case (u8)XPM_DEVSTATE_CLK_OFF:
			if ((u32)XPM_DEVEVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				/* Todo: Check if clock is disabled */
				if (TRUE /* Hack: Clock disabled */) {
					Node->State = (u8)XPM_DEVSTATE_PWR_OFF;
					Device->WfPwrUseCnt = (u8)(Device->Power->UseCount - 1U);
					Status = Device->Power->HandleEvent(
						 &Device->Power->Node, (u32)XPM_POWER_EVENT_PWR_DOWN);
					/* Todo: Start timer to poll power node use count */
					/* Hack */
					Status = Device->HandleEvent(Node, (u32)XPM_DEVEVENT_TIMER);
				} else {
					/* Todo: Start timer to poll clock node */
				}
			}
			break;
		case (u8)XPM_DEVSTATE_PWR_OFF:
			if ((u32)XPM_DEVEVENT_TIMER == Event) {
				Status = XST_SUCCESS;
				Device->Node.Flags &= (u8)(~NODE_IDLE_DONE);
				if (Device->WfPwrUseCnt == Device->Power->UseCount) {
					if (1U == Device->WfDealloc) {
						Device->PendingReqm->Allocated = 0;
						Device->WfDealloc = 0;
					}
					if(Device->PendingReqm != NULL) {
						XPm_RequiremntUpdate(Device->PendingReqm);
						Device->PendingReqm = NULL;
					}
					if (0U == IsRunning(Device)) {
						Node->State = (u8)XPM_DEVSTATE_UNUSED;
					} else {
						Node->State = (u8)XPM_DEVSTATE_RUNNING;
					}
				} else {
					/* Todo: Start timer to poll power node use count */
				}
			} else if ((u32)XPM_DEVEVENT_SHUTDOWN == Event) {
				/* Device is already in power off state */
				Status = XST_SUCCESS;
			} else {
				DbgErr = XPM_INT_ERR_INVALID_EVENT;
			}
			break;
		case (u8)XPM_DEVSTATE_RUNTIME_SUSPEND:
			if ((u32)XPM_DEVEVENT_SHUTDOWN == Event) {
				/* Assert reset for peripheral devices */
				if ((u32)XPM_NODESUBCL_DEV_PERIPH ==
						NODESUBCLASS(Device->Node.Id)) {
					Status = XPmDevice_Reset(Device,
							PM_RESET_ACTION_ASSERT);
					if (XST_SUCCESS != Status) {
						DbgErr = XPM_INT_ERR_RST_ASSERT;
						break;
					}
				}
				/*
				 * Change device's state to clock off since all
				 * clocks are disabled during runtime suspend.
				 */
				Node->State = (u8)XPM_DEVSTATE_CLK_OFF;
				Status = Device->HandleEvent(Node, (u32)XPM_DEVEVENT_TIMER);
			} else if ((u32)XPM_DEVEVENT_BRINGUP_ALL == Event) {
				/* Enable all clocks */
				Status = SetClocks(Device, 1U);
				if (XST_SUCCESS != Status) {
					DbgErr = XPM_INT_ERR_CLK_ENABLE;
					break;
				}
				Node->State = (u8)XPM_DEVSTATE_RUNNING;
			} else {
				DbgErr = XPM_INT_ERR_INVALID_EVENT;
			}
			break;
		default:
			DbgErr = XPM_INT_ERR_INVALID_STATE;
			Status = XPM_INVALID_STATE;
			break;
	}

	if (XST_SUCCESS != Status) {
		PmErr("ID=0x%x State=%s Event=%s Err=%x\r\n", Node->Id,
		      Node->State, Event, DbgErr);
	}

	return Status;
}

static XStatus CheckSecurityAccess(const XPm_Requirement *Reqm, u32 ReqCaps,
				   u32 CmdType)
{
	XStatus Status = XST_SUCCESS;

	/**
	 * Check if requirement policy allows non-secure master and
	 * non-secure master can only make device to non-secure
	 */
	if ((XPLMI_CMD_SECURE != CmdType) &&
	    (((u16)REQ_ACCESS_SECURE == SECURITY_POLICY(Reqm->Flags)) ||
	     (0U != (ReqCaps & (u32)(PM_CAP_SECURE))))) {
		Status = XPM_PM_NO_ACCESS;
	}

	return Status;
}

static XStatus SetSecurityAttr(XPm_Requirement *Reqm, u32 ReqCaps, u32 PrevState)
{
	XStatus Status = XST_FAILURE;
	u32 CurrSecState;
	u32 Is_CapSecure = 0U;
	u32 BaseAddr, Offset1, Mask1, Offset2, Mask2;
	const XPm_DeviceAttr *DevAttr = Reqm->Device->DevAttr;
	const XPm_PsLpDomain *Lpd = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);
	const XPm_PsFpDomain *Fpd = (XPm_PsFpDomain *)XPmPower_GetById(PM_POWER_FPD);

	/**
	 * Skip if device does not have any security attributes.
	 * Security[1] attribute is optional.
	 */
	if ((NULL == DevAttr) || (0U == DevAttr->Security[0].Mask) ||
	    (0U == DevAttr->SecurityBaseAddr)) {
		Status = XST_SUCCESS;
		goto done;
	}

	if ((NULL == Lpd) || (NULL == Fpd)) {
		Status = XST_FAILURE;
		goto done;
	}

	BaseAddr = DevAttr->SecurityBaseAddr;
	Offset1 = DevAttr->Security[0].Offset;
	Mask1 = DevAttr->Security[0].Mask;
	Offset2 = DevAttr->Security[1].Offset;
	Mask2 = DevAttr->Security[1].Mask;

	if (0U != (ReqCaps & (u32)(PM_CAP_SECURE))) {
		Is_CapSecure = 1U;
	}

	/**
	 * Do not touch the LPD/FPD registers if domain is not ON.
	 * This is the generalized solution but applicable during the last
	 * device release of the domain.
	 */
	if (BaseAddr == Lpd->LpdSlcrSecureBaseAddr) {
		if ((u8)XPM_POWER_STATE_ON != Lpd->Domain.Power.Node.State) {
			Status = XST_SUCCESS;
			goto done;
		}
	} else if (BaseAddr == Fpd->FpdSlcrSecureBaseAddr) {
		if ((u8)XPM_POWER_STATE_ON != Fpd->Domain.Power.Node.State) {
			Status = XST_SUCCESS;
			goto done;
		}
	} else {
		/* Required due to MISRA */
	}

	/* Here 1 value in bit corresponds to non-secure config */
	CurrSecState = XPm_In32(BaseAddr + Offset1) & Mask1;

	/* Do nothing if device is still ON after release */
	if ((0U == Reqm->Allocated) &&
	    ((u8)XPM_DEVSTATE_RUNNING == Reqm->Device->Node.State)) {
		Status = XST_SUCCESS;
		goto done;
	}

	if ((1U == Reqm->Allocated) &&
	    ((u32)XPM_DEVSTATE_RUNNING == PrevState)) {
		/**
		 * Return error if current device state does not match
		 * with the received request.
		 */
		if (((1U == Is_CapSecure) && (DEV_NONSECURE == CurrSecState)) ||
		    ((0U == Is_CapSecure) && (DEV_SECURE == CurrSecState))) {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		} else {
			Status = XST_SUCCESS;
			goto done;
		}
	}

	if (BaseAddr == Lpd->LpdSlcrSecureBaseAddr) {
		XPm_Out32(Lpd->LpdSlcrSecureBaseAddr + LPD_SLCR_SECURE_WPROT0_OFFSET, 0x0U);
	} else if (BaseAddr == Fpd->FpdSlcrSecureBaseAddr) {
		XPm_Out32(Fpd->FpdSlcrSecureBaseAddr + FPD_SLCR_SECURE_WPROT0_OFFSET, 0x0U);
	} else {
		/* Required due to MISRA */
	}

	if (1U == Is_CapSecure) {
		PmRmw32(BaseAddr + Offset1, Mask1, 0);
		if (0U != Mask2) {
			PmRmw32(BaseAddr + Offset2, Mask2, 0);
		}
		Reqm->AttrCaps |= (u8)(PM_CAP_SECURE);
	} else {
		PmRmw32(BaseAddr + Offset1, Mask1, Mask1);
		if (0U != Mask2) {
			PmRmw32(BaseAddr + Offset2, Mask2, Mask2);
		}
	}

	if (BaseAddr == Lpd->LpdSlcrSecureBaseAddr) {
		XPm_Out32(Lpd->LpdSlcrSecureBaseAddr + LPD_SLCR_SECURE_WPROT0_OFFSET, 0x1U);
	} else if (BaseAddr == Fpd->FpdSlcrSecureBaseAddr) {
		XPm_Out32(Fpd->FpdSlcrSecureBaseAddr + FPD_SLCR_SECURE_WPROT0_OFFSET, 0x1U);
	} else {
		/* Required due to MISRA */
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus SetDevCohVirtAttr(XPm_Requirement *Reqm, u32 ReqCaps,
				 u8 Capability, u32 Enable)
{
	XStatus Status = XST_FAILURE;
	u8 CurrCaps = 0U;
	u32 BaseAddr, Offset, Mask;
	XPm_Power *FpdPower = NULL;
	const XPm_Requirement *DevReqm = NULL;
	const XPm_DeviceAttr *DevAttr = Reqm->Device->DevAttr;

	/* Skip if device does not have any attributes */
	if (NULL == DevAttr) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Get base address, mask and offset based on capability */
	BaseAddr = DevAttr->CohVirtBaseAddr;
	if ((u8)(PM_CAP_COHERENT) == Capability) {
		Offset = DevAttr->Coherency.Offset;
		Mask = DevAttr->Coherency.Mask;
	} else {
		Offset = DevAttr->Virtualization.Offset;
		Mask = DevAttr->Virtualization.Mask;
	}

	/* Do nothing if required attributes are not present */
	if ((0U == Mask) || (0U == BaseAddr)) {
		Status = XST_SUCCESS;
                goto done;
	}

	if (1U == Enable) {
		/* Do nothing if capability is not requested */
		if (0U == (ReqCaps & Capability)) {
			Status = XST_SUCCESS;
			goto done;
		}
	} else {
		/* Update AttrCaps flag of requirement */
		if (0U != (Reqm->AttrCaps & Capability)) {
			Reqm->AttrCaps &= (u8)(~Capability);
		} else {
			Status = XST_SUCCESS;
			goto done;
		}
	}

	FpdPower = XPmPower_GetById(PM_POWER_FPD);
	if (NULL == FpdPower) {
		goto done;
	}

	DevReqm = Reqm->Device->Requirements;
	while (NULL != DevReqm) {
		if (1U == DevReqm->Allocated) {
			CurrCaps |= DevReqm->AttrCaps;
		}
		DevReqm = DevReqm->NextSubsystem;
	}
	/* Do nothing if attribute is already set */
	if (0U != (CurrCaps & Capability)) {
		Status = XST_SUCCESS;
		goto done;
	}

	if (1U == Enable) {
		Status = FpdPower->HandleEvent(&FpdPower->Node,
						XPM_POWER_EVENT_PWR_UP);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		PmRmw32(BaseAddr + Offset, Mask, Mask);
		Reqm->AttrCaps |= Capability;
	} else {
		PmRmw32(BaseAddr + Offset, Mask, 0U);
		Status = FpdPower->HandleEvent(&FpdPower->Node,
						XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

done:
	return Status;
}

static XStatus HandleDeviceAttr(XPm_Requirement *Reqm, u32 ReqCaps,
				u32 PrevState, u32 Enable)
{
	XStatus Status = XST_FAILURE;

	Status = SetDevCohVirtAttr(Reqm, ReqCaps, (u8)PM_CAP_COHERENT, Enable);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = SetDevCohVirtAttr(Reqm, ReqCaps, (u8)PM_CAP_VIRTUALIZED, Enable);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = SetSecurityAttr(Reqm, ReqCaps, PrevState);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
}

static XStatus HandleDeviceState(XPm_Device* const Device, const u32 NextState)
{
	XStatus Status = XST_FAILURE;

	switch (Device->Node.State) {
	case (u8)XPM_DEVSTATE_UNUSED:
		if ((u32)XPM_DEVSTATE_RUNNING == NextState) {
			Status = XPmDevice_BringUp(Device);
		} else {
			Status = XST_SUCCESS;
		}
		break;
	case (u8)XPM_DEVSTATE_RUNNING:
		if ((u32)XPM_DEVSTATE_UNUSED == NextState) {
			Status = Device->HandleEvent(&Device->Node,
						     (u32)XPM_DEVEVENT_SHUTDOWN);
		} else if ((u32)XPM_DEVSTATE_RUNTIME_SUSPEND == NextState) {
			Status = Device->HandleEvent(&Device->Node,
						     (u32)XPM_DEVEVENT_RUNTIME_SUSPEND);
		} else {
			Status = XST_SUCCESS;
		}
		break;
	case (u8)XPM_DEVSTATE_RUNTIME_SUSPEND:
		if ((u32)XPM_DEVSTATE_RUNNING == NextState) {
			Status = Device->HandleEvent(&Device->Node,
						     (u32)XPM_DEVEVENT_BRINGUP_ALL);
		} else if ((u32)XPM_DEVSTATE_UNUSED == NextState) {
			Status = Device->HandleEvent(&Device->Node,
						     (u32)XPM_DEVEVENT_SHUTDOWN);
		} else {
			Status = XST_SUCCESS;
		}
		break;
	default:
		Status = XPM_INVALID_STATE;
		break;
	}

	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

static const XPm_DeviceFsm XPmGenericDeviceFsm = {
	DEFINE_DEV_STATES(XPmGenericDeviceStates),
	DEFINE_DEV_TRANS(XPmGenericDevTransitions),
	.EnterState = HandleDeviceState,
};

static const XPm_StateTran XPmVirtDev_VirtDevTransitions[] = {
	{
		.FromState = (u32)XPM_DEVSTATE_RUNNING,
		.ToState = (u32)XPM_DEVSTATE_UNUSED,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_UNUSED,
		.ToState = (u32)XPM_DEVSTATE_RUNNING,
		.Latency = XPM_DEF_LATENCY,
	},
};

static const XPm_DeviceFsm XPmVirtDevFsm = {
	DEFINE_DEV_STATES(XPmGenericDeviceStates),
	DEFINE_DEV_TRANS(XPmVirtDev_VirtDevTransitions),
	.EnterState = HandleDeviceState,
};

XStatus XPmVirtDev_DeviceInit(XPm_Device *Device, u32 Id, XPm_Power *Power)
{
	XStatus Status = XST_FAILURE;

	Status = XPmDevice_Init(Device, Id, 0U, Power, NULL, NULL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/*
	 * Here have to creata a new FSM. Ideally just setting the transition
	 * states and transition count would be better but these fields are
	 * const in the XPm_DeviceFsm struct.
	 */
	Device->DeviceFsm = &XPmVirtDevFsm;

done:
	return Status;
}

static XStatus DevRequest(XPm_Device *Device, const XPm_Subsystem *Subsystem,
		       u32 Capabilities, u32 QoS, u32 CmdType)
{
	XStatus Status = XPM_ERR_DEVICE_REQ;
	XPm_Requirement *Reqm = NULL;
	u16 UsagePolicy = 0;
	u32 PrevState;

	if (((u8)XPM_DEVSTATE_UNUSED != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_RUNNING != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_RUNTIME_SUSPEND != Device->Node.State)) {
			Status = XST_DEVICE_BUSY;
			goto done;
	}
	PrevState = Device->Node.State;

	/* Check whether this device assigned to the subsystem */
	Reqm = FindReqm(Device, Subsystem);
	if (NULL == Reqm) {
		goto done;
	}

	Status = CheckSecurityAccess(Reqm, Capabilities, CmdType);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (1U == Reqm->Allocated) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Check whether this device is shareable */
	UsagePolicy = USAGE_POLICY(Reqm->Flags);
	if ((UsagePolicy == (u16)REQ_TIME_SHARED) || (UsagePolicy == (u16)REQ_NONSHARED)) {
			//Check if it already requested by other subsystem. If yes, return
			const XPm_Requirement *NextReqm = Reqm->NextSubsystem;
			while (NULL != NextReqm) {
				if (1U == NextReqm->Allocated) {
					Status = XPM_PM_NODE_USED;
					goto done;
				}
				NextReqm = NextReqm->NextSubsystem;
			}
	}

	/* Allocated device for the subsystem */
	Reqm->Allocated = 1U;

	Status = Device->DeviceOps->SetRequirement(Device, Subsystem,
						   Capabilities, QoS);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* (Re)Configure device protection if handler is present */
	if ((PLATFORM_VERSION_SILICON == XPm_GetPlatform()) &&
	    (NULL != Device->HandleProtection)) {
		Status = Device->HandleProtection(Reqm, 1U);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = HandleDeviceAttr(Reqm, Capabilities, PrevState, 1U);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	if (XST_SUCCESS != Status) {
		XPmRequirement_Clear(Reqm);
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

static XStatus SetDevRequirement(XPm_Device *Device, const XPm_Subsystem *Subsystem,
			      u32 Capabilities, const u32 QoS)
{
	XStatus Status = XPM_ERR_SET_REQ;;
	XPm_ReqmInfo TempReqm;

	if (((u8)XPM_DEVSTATE_UNUSED != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_RUNNING != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_RUNTIME_SUSPEND != Device->Node.State) &&
		((u8)XPM_DEVSTATE_INITIALIZING != Device->Node.State)) {
			Status = XST_DEVICE_BUSY;
			goto done;
	}

	Device->PendingReqm = FindReqm(Device, Subsystem);
	if (NULL == Device->PendingReqm) {
		goto done;
	}

	/*
	 * If subsystem state is suspending then do not change device's state
	 * according to capabilities, only schedule requirements by setting
	 * device's next requirements.
	 */
	if ((u8)SUSPENDING == Subsystem->State) {
		Device->PendingReqm->Next.Capabilities =
			(Capabilities & BITMASK(REQ_INFO_CAPS_BIT_FIELD_SIZE));
		Device->PendingReqm->Next.QoS = QoS;
		Status = XST_SUCCESS;
		goto done;
	} else {
		/*
		 * Store current requirements as a backup in case something
		 * fails.
		 */
		TempReqm.Capabilities = Device->PendingReqm->Curr.Capabilities;
		TempReqm.QoS = Device->PendingReqm->Curr.QoS;

		Device->PendingReqm->Curr.Capabilities =
			(Capabilities & BITMASK(REQ_INFO_CAPS_BIT_FIELD_SIZE));
		Device->PendingReqm->Curr.QoS = QoS;
	}

	Status = XPmDevice_UpdateStatus(Device);

	if (XST_SUCCESS != Status) {
		Device->PendingReqm->Curr.Capabilities = TempReqm.Capabilities;
		Device->PendingReqm->Curr.QoS = TempReqm.QoS;
	} else if ((u32)PM_CAP_UNUSABLE == Capabilities) {
		/* Schedule next requirement to 0 */
		Device->PendingReqm->Next.Capabilities = 0U;
		Device->PendingReqm->Next.QoS = QoS;
	} else {
		XPm_RequiremntUpdate(Device->PendingReqm);
	}

done:
	if (XST_SUCCESS != Status) {
		Device->PendingReqm = NULL;
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

static XStatus DevRelease(XPm_Device *Device, const XPm_Subsystem *Subsystem, u32 CmdType)
{
	XStatus Status = XPM_ERR_DEVICE_RELEASE;
	XPm_Requirement *Reqm;
	u32 PrevState;

	if (((u8)XPM_DEVSTATE_UNUSED != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_RUNNING != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_RUNTIME_SUSPEND != Device->Node.State) &&
		((u8)XPM_DEVSTATE_INITIALIZING != Device->Node.State)) {
			Status = XST_DEVICE_BUSY;
			goto done;
	}
	PrevState = Device->Node.State;

	Device->PendingReqm = FindReqm(Device, Subsystem);
	if (NULL == Device->PendingReqm) {
		goto done;
	}
	Reqm = Device->PendingReqm;

	Status = CheckSecurityAccess(Reqm, 0U, CmdType);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (0U == Device->PendingReqm->Allocated) {
		Status = XST_SUCCESS;
		goto done;
	}

	Device->WfDealloc = 1U;

	Status = Device->DeviceOps->SetRequirement(Device, Subsystem, 0,
						   XPM_DEF_QOS);
	if (XST_SUCCESS != Status) {
		Status = XPM_ERR_DEVICE_RELEASE;
		goto done;
	}

	XPmRequirement_Clear(Reqm);
	Device->WfDealloc = 0;

	/* (Re)Configure device protection if handler is present */
	if ((PLATFORM_VERSION_SILICON == XPm_GetPlatform()) &&
	    (NULL != Device->HandleProtection)) {
		Status = Device->HandleProtection(Reqm, 0U);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = HandleDeviceAttr(Reqm, 0U, PrevState, 0U);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	if (XST_SUCCESS != Status) {
		Device->PendingReqm = NULL;
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

XStatus XPmDevice_Init(XPm_Device *Device,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode * Clock, XPm_ResetNode *Reset)
{
	XStatus Status = XPM_ERR_DEVICE_INIT;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if ((NULL != XPmDevice_GetById(Id)) &&
	    (((u32)XPM_NODESUBCL_DEV_PL != NODESUBCLASS(Id)) &&
	    ((u32)XPM_NODESUBCL_DEV_AIE != NODESUBCLASS(Id)))){
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	XPmNode_Init(&Device->Node, Id, (u8)XPM_DEVSTATE_UNUSED, BaseAddress);

	/**
	 * Add requirement by default for PMC subsystem;
	 * for all the devices except memory region devices.
	 */
	if (0U == (u8)IS_MEM_REGN(Id)) {
		Status = XPmRequirement_Add(
				XPmSubsystem_GetByIndex((u32)XPM_NODEIDX_SUBSYS_PMC),
				Device,
				(u32)REQUIREMENT_FLAGS(0U, 0U, 0U, 0U,
					(u32)REQ_ACCESS_SECURE_NONSECURE,
					(u32)REQ_NO_RESTRICTION),
				0U, 0U, XPM_DEF_QOS);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_ADD_REQUIREMENT;
			goto done;
		}
	}

	Device->Power = Power;
	Device->PendingReqm = NULL;
	Device->WfDealloc = 0;
	Device->WfPwrUseCnt = 0;

	/* No protection handler is registered by default */
	Device->HandleProtection = NULL;

	Status = XPmDevice_AddClock(Device, Clock);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_ADD_CLK;
		goto done;
	}

	Status = XPmDevice_AddReset(Device, Reset);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_ADD_RST;
		goto done;
	}

	Device->HandleEvent = HandleDeviceEvent;

	PmDeviceOps.Request = DevRequest;
	PmDeviceOps.SetRequirement = SetDevRequirement;
	PmDeviceOps.Release = DevRelease;
	Device->DeviceOps = &PmDeviceOps;
	if (NULL == Device->DeviceFsm) {
		Device->DeviceFsm = &XPmGenericDeviceFsm;
	}

	if ((u32)XPM_NODESUBCL_DEV_PL == NODESUBCLASS(Id)) {
		Status = SetPlDeviceNode(Id, Device);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SET_PL_DEV;
			goto done;
		}
	}  else if (IS_MEM_REGN(Id)) {
		Status = SetMemRegnDeviceNode(Id, Device);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SET_MEM_REG_DEV;
			goto done;
		}
	} else if (((u32)XPM_NODESUBCL_DEV_PERIPH == NODESUBCLASS(Id)) &&
		  (((u32)XPM_NODETYPE_DEV_GGS == NODETYPE(Id)) ||
		   ((u32)XPM_NODETYPE_DEV_PGGS == NODETYPE(Id)))) {
		Status = SetVirtDeviceNode(Id, Device);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SET_VIRT_DEV;
			goto done;
		}
	} else if (((u32)XPM_NODESUBCL_DEV_PERIPH == NODESUBCLASS(Id)) &&
		   ((u32)XPM_NODETYPE_DEV_HB_MON == NODETYPE(Id))) {
		Status = SetHbMonDeviceNode(Id, Device);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SET_HB_MON_DEV;
		}
	} else if (IS_DEV_AIE(Id)) {
		Status = SetAieDeviceNode(Id, Device);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SET_AIE_DEV;
			goto done;
		}
	} else {
		Status = SetDeviceNode(Id, Device);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SET_DEV_NODE;
			goto done;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}


XStatus XPmDevice_AddClock(XPm_Device *Device, XPm_ClockNode *Clock)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockHandle *ClkHandle;

	if (NULL == Device) {
		Status = XPM_ERR_DEVICE;
		goto done;
	}

	if (NULL == Clock) {
		Status = XST_SUCCESS;
		goto done;
	}

	ClkHandle = (XPm_ClockHandle *)XPm_AllocBytes(sizeof(XPm_ClockHandle));
	if (NULL == ClkHandle) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	ClkHandle->Clock = Clock;
	ClkHandle->Device = Device;

	/* Prepend the new handle to the device's clock handle list */
	ClkHandle->NextClock = Device->ClkHandles;
	Device->ClkHandles = ClkHandle;

	/* Prepend the new handle to the clock's device handle list */
	ClkHandle->NextDevice = Clock->ClkHandles;
	Clock->ClkHandles = ClkHandle;

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

XStatus XPmDevice_AddReset(XPm_Device *Device, XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;
	XPm_ResetHandle *RstHandle;

	if (NULL == Device) {
		Status = XPM_ERR_DEVICE;
		goto done;
	}

	if (NULL == Reset) {
		Status = XST_SUCCESS;
		goto done;
	}

	RstHandle = (XPm_ResetHandle *)XPm_AllocBytes(sizeof(XPm_ResetHandle));
	if (NULL == RstHandle) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	RstHandle->Reset = Reset;
	RstHandle->Device = Device;

	/* Prepend the new handle to the device's reset handle list */
	RstHandle->NextReset = Device->RstHandles;
	Device->RstHandles = RstHandle;

	/* Prepend the new handle to the reset's device handle list */
	RstHandle->NextDevice = Reset->RstHandles;
	Reset->RstHandles = RstHandle;

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

XStatus XPmDevice_Reset(const XPm_Device *Device, const XPm_ResetActions Action)
{
	XStatus Status = XST_FAILURE;
	const XPm_ResetHandle *RstHandle;
	const XPm_ResetHandle *DeviceHandle;
	XPm_ResetNode *Reset;

	if (NULL == Device) {
		Status = XPM_ERR_DEVICE;
		goto done;
	}

	/* TODO: Skip handling for PL resets until PL topology is available */
	if ((u32)XPM_NODESUBCL_DEV_PL == NODESUBCLASS(Device->Node.Id)) {
		Status = XST_SUCCESS;
		goto done;
	}

#ifdef DEBUG_UART_PS
	/* Reset LPD init flag to stop debug prints which is using UART */
	if ((NODE_UART == Device->Node.Id) &&
	    (PM_RESET_ACTION_ASSERT == Action)) {
		PmDbg("Disabling UART prints\r\n");
		/* Wait for UART buffer to flush */
		usleep(10000);
		XPlmi_ResetLpdInitialized();
	}
#endif

	RstHandle = Device->RstHandles;
	if (PM_RESET_ACTION_RELEASE != Action) {
		while (NULL != RstHandle) {
			Reset = RstHandle->Reset;
			DeviceHandle = Reset->RstHandles;
			while (NULL != DeviceHandle) {
				if ((Device->Node.Id !=
				    DeviceHandle->Device->Node.Id) &&
				    ((u32)XPM_DEVSTATE_RUNNING ==
				    DeviceHandle->Device->Node.State)) {
					break;
				}
				DeviceHandle = DeviceHandle->NextDevice;
			}
			if (NULL == DeviceHandle) {
				Status = Reset->Ops->SetState(Reset, Action);
				if (XST_SUCCESS != Status) {
					goto done;
				}
			}
			RstHandle = RstHandle->NextReset;
		}
	} else {
		while (NULL != RstHandle) {
			Status = RstHandle->Reset->Ops->SetState(RstHandle->Reset, Action);
			if (XST_SUCCESS != Status) {
				goto done;
			}
			RstHandle = RstHandle->NextReset;
		}
	}

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

XStatus XPmDevice_CheckPermissions(const XPm_Subsystem *Subsystem, u32 DeviceId)
{
	XStatus Status = XPM_PM_NO_ACCESS;
	const XPm_Requirement *Reqm;
	const XPm_Device *Device = XPmDevice_GetById(DeviceId);

	if (NULL == Device) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Reqm = FindReqm(Device, Subsystem);
	if (NULL == Reqm) {
		goto done;
	}

	if (1U == Reqm->Allocated) {
		Status = XST_SUCCESS;
		goto done;
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief	Get handle to requested device node by "complete" Node ID
 *
 * @param DeviceId	Device Node ID
 *
 * @return	Pointer to requested XPm_Device
 *              NULL otherwise
 *
 * @note	Requires Complete Node ID
 *
 ****************************************************************************/
XPm_Device *XPmDevice_GetById(const u32 DeviceId)
{
	XPm_Device *Device = NULL;
	XPm_Device **DevicesHandle = NULL;

	if ((u32)XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		goto done;
	}

	if ((u32)XPM_NODESUBCL_DEV_PL == NODESUBCLASS(DeviceId)) {
		if ((u32)XPM_NODEIDX_DEV_PLD_MAX <= NODEINDEX(DeviceId)) {
			goto done;
		}
		DevicesHandle = PmPlDevices;

	} else if (((u32)XPM_NODESUBCL_DEV_MEM == NODESUBCLASS(DeviceId))
		&& (IS_MEM_REGN_TYPE(DeviceId))) {
		if ((u32)MEM_REGN_DEV_NODE_MAX <= NODEINDEX(DeviceId)) {
			goto done;
		}

		switch (NODETYPE(DeviceId)) {
		case (u8)XPM_NODETYPE_DEV_OCM_REGN:
			DevicesHandle = PmOcmMemRegnDevices;
			break;
		case (u8)XPM_NODETYPE_DEV_DDR_REGN:
			DevicesHandle = PmDdrMemRegnDevices;
			break;
		default:
			/* Should never reach here */
			PmDbg("Memory type other than OCM or DDR\r\n");
			break;
		}
	} else if (((u32)XPM_NODETYPE_DEV_GGS == NODETYPE(DeviceId)) ||
		   ((u32)XPM_NODETYPE_DEV_PGGS == NODETYPE(DeviceId))) {
		if ((u32)XPM_NODEIDX_DEV_VIRT_MAX <= NODEINDEX(DeviceId)) {
			goto done;
		}
		DevicesHandle = PmVirtualDevices;
	} else if (((u32)XPM_NODESUBCL_DEV_PERIPH == NODESUBCLASS(DeviceId)) &&
		   ((u32)XPM_NODETYPE_DEV_HB_MON == NODETYPE(DeviceId))) {
		if ((u32)XPM_NODEIDX_DEV_HB_MON_MAX <= NODEINDEX(DeviceId)) {
			goto done;
		}
		DevicesHandle = PmHbMonDevices;
	} else if ((u32)XPM_NODESUBCL_DEV_AIE == NODESUBCLASS(DeviceId)) {
		if ((u32)XPM_NODEIDX_DEV_AIE_MAX <= NODEINDEX(DeviceId)) {
			goto done;
		}
		DevicesHandle = PmAieDevices;
	} else {
		if ((u32)XPM_NODEIDX_DEV_MAX <= NODEINDEX(DeviceId)) {
			goto done;
		}
		DevicesHandle = PmDevices;
	}

	if (NULL == DevicesHandle) {
		goto done;
	}

	/* Retrieve the device */
	Device = DevicesHandle[NODEINDEX(DeviceId)];
	/* Check that Device's ID is same as given ID or not. */
	if ((NULL != Device) && (DeviceId != Device->Node.Id)) {
		Device = NULL;
	}

done:
	return Device;
}

/****************************************************************************/
/**
 * @brief	Get handle to requested device node by "only" Node INDEX
 *
 * @param DeviceIndex	Device Node Index
 *
 * @return	Pointer to requested XPm_Device, NULL otherwise
 *
 * @note	Requires ONLY Node Index
 *
 * Caller should be _careful_ while using this function as it skips the checks
 * for validating the class, subclass and type of the device before and after
 * retrieving the node from the database. Use this only where it is absolutely
 * necessary, otherwise use XPmDevice_GetById() which is more strict
 * and requires 'complete' Node ID for retrieving the handle.
 *
 ****************************************************************************/
XPm_Device *XPmDevice_GetByIndex(const u32 DeviceIndex)
{
	XPm_Device *Device = NULL;
	/* Make sure we are working with only Index. */
	u32 Index = (DeviceIndex & NODE_INDEX_MASK);

	if ((u32)XPM_NODEIDX_DEV_MAX <= Index) {
		goto done;
	}

	Device = PmDevices[Index];
	/* Check that Device's Index is same as given Index or not. */
	if ((NULL != Device) && (Index != NODEINDEX(Device->Node.Id))) {
		Device = NULL;
	}

done:
	return Device;
}

/****************************************************************************/
/**
 * @brief	Get PLD device node by node INDEX
 *
 * @param DeviceIndex	Device Node Index
 *
 * @return	Pointer to requested XPm_Device, NULL otherwise
 *
 * @note	Requires Node Index
 *
 * Caller should be _careful_ while using this function as it skips the checks
 * for validating the class, subclass and type of the device before and after
 * retrieving the node from the database. Use this only where it is absolutely
 * necessary, otherwise use XPmDevice_GetById() which is more strict and
 * requires 'complete' Node ID for retrieving the handle.
 *
 ****************************************************************************/
XPm_Device *XPmDevice_GetPlDeviceByIndex(const u32 DeviceIndex)
{
	XPm_Device *Device = NULL;
	/* Make sure we are working with only Index. */
	u32 Index = (DeviceIndex & NODE_INDEX_MASK);

	if ((u32)XPM_NODEIDX_DEV_PLD_MAX <= Index) {
		goto done;
	}

	Device = PmPlDevices[Index];
	/* Check that Device's Index is same as given Index or not. */
	if ((NULL != Device) && (Index != NODEINDEX(Device->Node.Id))) {
		Device = NULL;
	}

done:
	return Device;
}

/****************************************************************************/
/**
 * @brief	Get Healthy Boot Monitor device node by node INDEX
 *
 * @param DeviceIndex	Device Node Index
 *
 * @return	Pointer to requested XPm_Device, NULL otherwise
 *
 * @note	Requires Node Index
 *
 * Caller should be _careful_ while using this function as it skips the checks
 * for validating the class, subclass and type of the device before and after
 * retrieving the node from the database. Use this only where it is absolutely
 * necessary, otherwise use XPmDevice_GetById() which is more strict and
 * requires 'complete' Node ID for retrieving the handle.
 *
 ****************************************************************************/
XPm_Device *XPmDevice_GetHbMonDeviceByIndex(const u32 DeviceIndex)
{
	XPm_Device *Device = NULL;
	/* Make sure we are working with only Index. */
	u32 Index = (DeviceIndex & NODE_INDEX_MASK);

	if ((u32)XPM_NODEIDX_DEV_HB_MON_MAX <= Index) {
		goto done;
	}

	Device = PmHbMonDevices[Index];
	/* Check that Device's Index is same as given Index or not. */
	if ((NULL != Device) && (Index != NODEINDEX(Device->Node.Id))) {
		Device = NULL;
	}

done:
	return Device;
}

XStatus XPmDevice_Request(const u32 SubsystemId, const u32 DeviceId,
			  const u32 Capabilities, const u32 QoS, const u32 CmdType)
{
	XStatus Status = XPM_ERR_DEVICE_REQ;
	XPm_Device *Device;
	XPm_Subsystem *Subsystem;
	u32 Idx;

	/* Todo: Check if policy allows this request */
	/* If not allowed XPM_PM_NO_ACCESS error should be returned */

	if ((u32)XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}


	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if ((Subsystem == NULL) || (Subsystem->State != (u8)ONLINE)) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Status = Device->DeviceOps->Request(Device, Subsystem, Capabilities,
					    QoS, CmdType);
	if (XST_SUCCESS == Status) {
		/**
		 * More than one IPI channels can be associated with a subsystem;
		 * Update its IPI mask accordingly when an IPI channel is requested.
		 */
		for (Idx = 0; Idx < ARRAY_SIZE(IpiMasks); Idx++) {
			if (IpiMasks[Idx][0] == Device->Node.Id) {
				Subsystem->IpiMask |= IpiMasks[Idx][1];
				break;
			}
		}
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

XStatus XPmDevice_Release(const u32 SubsystemId, const u32 DeviceId,
			  const u32 CmdType)
{
	XStatus Status = XPM_ERR_DEVICE_RELEASE;
	XPm_Device *Device;
	const XPm_Subsystem *Subsystem;

	/* Todo: Check if subsystem has permission */

	if ((u32)XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if ((Subsystem == NULL) || (Subsystem->State == (u8)OFFLINE)) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Status = Device->DeviceOps->Release(Device, Subsystem, CmdType);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

XStatus XPmDevice_SetRequirement(const u32 SubsystemId, const u32 DeviceId,
				 const u32 Capabilities, const u32 QoS)
{
	XStatus Status = XPM_ERR_SET_REQ;
	XPm_Device *Device;
	const XPm_Subsystem *Subsystem;

	/* Todo: Check if subsystem has permission */

	if ((u32)XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}


	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if ((Subsystem == NULL) || (Subsystem->State == (u8)OFFLINE)) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Status = Device->DeviceOps->SetRequirement(Device, Subsystem,
						   Capabilities, QoS);

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

XStatus XPmDevice_GetStatus(const u32 SubsystemId,
			const u32 DeviceId,
			XPm_DeviceStatus *const DeviceStatus)
{
	XStatus Status = XPM_ERR_DEVICE_STATUS;
	const XPm_Subsystem *Subsystem;
	const XPm_Device *Device;
	const XPm_Requirement *Reqm;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if ((Subsystem == NULL) || (Subsystem->State != (u8)ONLINE)) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	DeviceStatus->Status = Device->Node.State;

	Reqm = FindReqm(Device, Subsystem);
	if (NULL != Reqm) {
		DeviceStatus->Requirement = Reqm->Curr.Capabilities;
	}

	DeviceStatus->Usage = XPmDevice_GetUsageStatus(Subsystem, Device);

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

XStatus XPmDevice_AddParent(u32 Id, const u32 *Parents, u32 NumParents)
{
	XStatus Status = XST_FAILURE;
	u32 i = 0;
	XPm_Device *DevPtr = XPmDevice_GetById(Id);

	if ((DevPtr == NULL) || (NumParents == 0U)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	for(i=0;i<NumParents;i++)
	{
		if ((u32)XPM_NODECLASS_CLOCK == NODECLASS(Parents[i])) {
			XPm_ClockNode *Clk = XPmClock_GetById(Parents[i]);
			if (NULL == Clk) {
				Status = XST_INVALID_PARAM;
				goto done;
			}

			Status = XPmDevice_AddClock(DevPtr, Clk);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else if ((u32)XPM_NODECLASS_RESET == NODECLASS(Parents[i])) {
			XPm_ResetNode *Rst = XPmReset_GetById(Parents[i]);
			if (NULL == Rst) {
				Status = XST_INVALID_PARAM;
				goto done;
			}

			Status = XPmDevice_AddReset(DevPtr, Rst);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else if ((u32)XPM_NODECLASS_POWER == NODECLASS(Parents[i])) {
			if (DevPtr->Power != NULL) {
				Status = XST_INVALID_PARAM;
				goto done;
			} else {
				DevPtr->Power = XPmPower_GetById(Parents[i]);
				if (NULL == DevPtr->Power) {
					Status = XST_DEVICE_NOT_FOUND;
					goto done;
				}
				Status = XST_SUCCESS;
			}
		} else if (((u32)XPM_NODECLASS_DEVICE == NODECLASS(Parents[i])) &&
			   ((u32)XPM_NODESUBCL_DEV_PL == NODESUBCLASS(Parents[i]))) {
			if (((u32)XPM_NODECLASS_DEVICE == NODECLASS(Id)) &&
			    ((u32)XPM_NODESUBCL_DEV_PL == NODESUBCLASS(Id))) {
				XPm_PlDevice *PlDevice = (XPm_PlDevice *)DevPtr;
				XPm_PlDevice *Parent = (XPm_PlDevice *)XPmDevice_GetById(Parents[i]);
				/*
				 * Along with checking validity of parent, check if parent has
				 * a parent with exception being PLD_0. This is to prevent
				 * broken trees
				 */
				Status = XPmPlDevice_IsValidPld(Parent);
				if (XST_SUCCESS != Status) {
					goto done;
				}

				PlDevice->Parent = Parent;
				PlDevice->NextPeer = Parent->Child;
				Parent->Child = PlDevice;
				Status = XST_SUCCESS;
			} else if (((u32)XPM_NODECLASS_DEVICE == NODECLASS(Id)) &&
			           ((u32)XPM_NODESUBCL_DEV_AIE == NODESUBCLASS(Id))) {
				XPm_AieDevice *AieDevice = (XPm_AieDevice *)DevPtr;
				XPm_PlDevice *Parent = (XPm_PlDevice *)XPmDevice_GetById(Parents[i]);
				/*
				 * Along with checking validity of parent, check if parent has
				 * a parent with exception being PLD_0. This is to prevent
				 * broken trees
				 */
				Status = XPmPlDevice_IsValidPld(Parent);
				if (XST_SUCCESS != Status) {
					goto done;
				}

				AieDevice->Parent = Parent;
				Parent->AieDevice = AieDevice;
				Status = XST_SUCCESS;
			} else {
				Status = XPM_INVALID_DEVICEID;
				goto done;
			}
		} else {
			Status = XST_INVALID_PARAM;
			goto done;
		}
	}
done:
	return Status;
}

XStatus XPmDevice_GetPermissions(const XPm_Device *Device, u32 *PermissionMask)
{
	XStatus Status = XST_FAILURE;
	const XPm_Requirement *Reqm;
	u32 Idx;
	u32 SubsysIdx = XPmSubsystem_GetMaxSubsysIdx();

	if ((NULL == Device) || (NULL == PermissionMask)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Reqm = Device->Requirements;
	while (NULL != Reqm) {
		if (1U == Reqm->Allocated) {
			for (Idx = 0; Idx <= SubsysIdx; Idx++) {
				if (Reqm->Subsystem == XPmSubsystem_GetByIndex(Idx)) {
					*PermissionMask |= ((u32)1U << Idx);
				}
			}
		}
		Reqm = Reqm->NextSubsystem;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Set maximum allowed latency for the device
 *
 * @param  SubsystemId	Initiator of the request who must previously requested
 *			the device
 * @param  DeviceId	Device whose latency is specified
 * @param  Latency	Maximum allowed latency in microseconds
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 ****************************************************************************/
XStatus XPmDevice_SetMaxLatency(const u32 SubsystemId, const u32 DeviceId,
			    const u32 Latency)
{
	XStatus Status = XST_FAILURE;
	XPm_Requirement *Reqm;
	const XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);
	XPm_Device *Device = XPmDevice_GetById(DeviceId);

	if ((NULL == Subsystem) || (NULL == Device)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Reqm = FindReqm(Device, Subsystem);
	if (NULL == Reqm) {
		Status = XPM_ERR_DEVICE_REQ;
		goto done;
	}

	Reqm->Next.Latency =
		(Latency & BITMASK(REQ_INFO_LATENCY_BIT_FIELD_SIZE));
	Reqm->SetLatReq = 1;

	Status = XPmDevice_UpdateStatus(Device);
	if (XST_SUCCESS != Status) {
		Reqm->SetLatReq = 0;
		goto done;
	}

	Reqm->Curr.Latency =
		(Latency & BITMASK(REQ_INFO_LATENCY_BIT_FIELD_SIZE));

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Change state of a device
 *
 * @param Device	Device pointer whose state should be changed
 * @param NextState		New state
 *
 * @return	XST_SUCCESS if transition was performed successfully.
 *              Error otherwise.
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPmDevice_ChangeState(XPm_Device *Device, const u32 NextState)
{
	XStatus Status = XPM_ERR_SETSTATE;
	const XPm_DeviceFsm* Fsm = Device->DeviceFsm;
	u32 OldState = Device->Node.State;
	u32 Trans;

	if (0U == Fsm->TransCnt) {
		/* Device's FSM has no transitions when it has only one state */
		Status = XST_SUCCESS;
		goto done;
	}

	for (Trans = 0U; Trans < Fsm->TransCnt; Trans++) {
		/* Find transition from current state to next state */
		if ((Fsm->Trans[Trans].FromState != Device->Node.State) ||
			(Fsm->Trans[Trans].ToState != NextState)) {
			continue;
		}

		if (NULL != Device->DeviceFsm->EnterState) {
			/* Execute transition action of device's FSM */
			Status = Device->DeviceFsm->EnterState(Device, NextState);
		} else {
			Status = XST_SUCCESS;
		}

		break;
	}

	if ((OldState != NextState) && (XST_SUCCESS == Status)) {
		Device->Node.State = (u8)NextState;

		/* Send notification about device state change */
		XPmNotifier_Event(Device->Node.Id, (u32)EVENT_STATE_CHANGE);
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Get state with provided capabilities
 *
 * @param Device	Device whose states are searched
 * @param Caps		Capabilities the state must have
 * @param State		Pointer to a u32 variable where the result is put if
 *			state is found
 *
 * @return	Status of the operation
 *		- XST_SUCCESS if state is found
 *
 * @note	None
 *
 ****************************************************************************/
static XStatus GetStateWithCaps(const XPm_Device* const Device, const u32 Caps,
				u32* const State)
{
	u32 Idx;
	XStatus Status = XPM_PM_CONFLICT;

	for (Idx = 0U; Idx < Device->DeviceFsm->StatesCnt; Idx++) {
		/* Find the first state that contains all capabilities */
		if ((Caps & Device->DeviceFsm->States[Idx].Cap) == Caps) {
			Status = XST_SUCCESS;
			if (NULL != State) {
				*State = Device->DeviceFsm->States[Idx].State;
			}
			break;
		}
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  Find minimum of all latency requirements
 *
 * @Param  Device	Device whose min required latency is requested
 *
 * @return Latency in microseconds
 *
 ****************************************************************************/
static u32 GetMinRequestedLatency(const XPm_Device *const Device)
{
	const XPm_Requirement *Reqm = Device->Requirements;
	u32 MinLatency = XPM_MAX_LATENCY;

	while (NULL != Reqm) {
		if ((1U == Reqm->SetLatReq) &&
		    (MinLatency > Reqm->Next.Latency)) {
			MinLatency = Reqm->Next.Latency;
		}
		Reqm = Reqm->NextSubsystem;
	}

	return MinLatency;
}

/****************************************************************************/
/**
 * @brief  Get latency from given state to the highest state
 *
 * @param  Device	Pointer to the device whose states are in question
 * @param  State	State from which the latency is calculated
 *
 * @return Return value for the found latency
 *
 ****************************************************************************/
static u32 GetLatencyFromState(const XPm_Device *const Device, const u32 State)
{
	u32 Idx;
	u32 Latency = 0U;
	u32 HighestStateIdx = Device->DeviceFsm->StatesCnt - (u32)1U;
	u32 HighestState = Device->DeviceFsm->States[HighestStateIdx].State;

	for (Idx = 0U; Idx < Device->DeviceFsm->TransCnt; Idx++) {
		if ((State == Device->DeviceFsm->Trans[Idx].FromState) &&
		    (HighestState == Device->DeviceFsm->Trans[Idx].ToState)) {
			Latency = Device->DeviceFsm->Trans[Idx].Latency;
			break;
		}
	}

	return Latency;
}

/****************************************************************************/
/**
 * @brief  Find a higher power state which satisfies latency requirements
 *
 * @param  Device	Device whose state may be constrained
 * @param  State	Chosen state which does not satisfy latency requirements
 * @param  CapsToSet	Capabilities that the state must have
 * @param  MinLatency	Latency requirements to be satisfied
 *
 * @return Status showing whether the higher power state is found or not.
 * State may not be found if multiple subsystem have contradicting requirements,
 * then XST_FAILURE is returned. Otherwise, function returns success.
 *
 ****************************************************************************/
static XStatus ConstrainStateByLatency(const XPm_Device *const Device,
				   u32 *const State, const u32 CapsToSet,
				   const u32 MinLatency)
{
	XStatus Status = XST_FAILURE;
	u32 WkupLat;
	u32 Idx = 0;

	/*
	 * Need to find higher power state, so ignore lower power states
	 * and find index for chosen state
	 */
	while (Device->DeviceFsm->States[Idx].State != *State)
	{
		Idx++;
	}

	for (; Idx < Device->DeviceFsm->StatesCnt; Idx++) {
		if ((CapsToSet & Device->DeviceFsm->States[Idx].Cap) != CapsToSet) {
			/* State candidate has no required capabilities */
			continue;
		}
		WkupLat = GetLatencyFromState(Device, Device->DeviceFsm->States[Idx].State);
		if (WkupLat > MinLatency) {
			/* State does not satisfy latency requirement */
			continue;
		}

		Status = XST_SUCCESS;
		*State = Device->DeviceFsm->States[Idx].State;
		break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  Device updates its power parent about latency req
 *
 * @param  Device	Device whose latency requirement have changed
 *
 * @return If the change of the latency requirement caused the power up of the
 * power parent, the status of performing power up operation is returned,
 * otherwise XST_SUCCESS is returned.
 *
 ****************************************************************************/
static XStatus UpdatePwrLatencyReq(const XPm_Device *const Device)
{
	XStatus Status = XST_FAILURE;
	XPm_Power* Power = Device->Power;

	if ((u8)XPM_POWER_STATE_ON == Power->Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Power is down, check if latency requirements trigger the power up */
	if (Device->Node.LatencyMarg <
	    (Power->PwrDnLatency + Power->PwrUpLatency)) {
		Power->Node.LatencyMarg = 0U;
		Status = Power->HandleEvent(&Power->Node, XPM_POWER_EVENT_PWR_UP);
	} else {
		Status = XST_SUCCESS;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Update the device's state according to the current requirements
 *		from all subsystems
 * @param Device	Device whose state is about to be updated
 *
 * @return      Status of operation of updating device's state.
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPmDevice_UpdateStatus(XPm_Device *Device)
{
	XStatus Status = XPM_ERR_DEVICE_STATUS;
	u32 Caps = GetMaxCapabilities(Device);
	u32 WkupLat, MinLat;
	u32 State = 0;

	if (((u8)XPM_DEVSTATE_UNUSED != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_RUNNING != Device->Node.State) &&
	    ((u8)XPM_DEVSTATE_RUNTIME_SUSPEND != Device->Node.State) &&
		((u8)XPM_DEVSTATE_INITIALIZING != Device->Node.State)) {
			Status = XST_DEVICE_BUSY;
			goto done;
	}

	Status = GetStateWithCaps(Device, Caps, &State);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	MinLat = GetMinRequestedLatency(Device);
	WkupLat = GetLatencyFromState(Device, State);
	if (WkupLat > MinLat) {
		/* State does not satisfy latency requirement, find another */
		Status = ConstrainStateByLatency(Device, &State, Caps, MinLat);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		WkupLat = GetLatencyFromState(Device, State);
	}

	Device->Node.LatencyMarg = (u16)(MinLat - WkupLat);

	if (State != Device->Node.State) {
		Status = XPmDevice_ChangeState(Device, State);
	} else {
		if (((u8)XPM_DEVSTATE_UNUSED == Device->Node.State) &&
		    (NULL != Device->Power)) {
			/* Notify power parent (changed latency requirement) */
			Status = UpdatePwrLatencyReq(Device);
		}
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Get the current usage status for a given device.
 * @param  Subsystem   Subsystem for which usage status is in query
 * @slave  Device      Device for which usage status need to be calculated
 *
 * @return  Usage status:
 *          - 0: No subsystem is currently using the device
 *          - 1: Only requesting subsystem is currently using the device
 *          - 2: Only other subsystems are currently using the device
 *          - 3: Both the current and at least one other subsystem is currently
 *               using the device
 *
 ****************************************************************************/
u32 XPmDevice_GetUsageStatus(const XPm_Subsystem *Subsystem, const XPm_Device *Device)
{
	u32 UsageStatus = 0;
	const XPm_Requirement *Reqm = Device->Requirements;

	while (NULL != Reqm) {
		if (1U == Reqm->Allocated) {
			/* This subsystem is currently using this device */
			if (Subsystem == Reqm->Subsystem) {
				UsageStatus |= (u32)PM_USAGE_CURRENT_SUBSYSTEM;
			} else {
				UsageStatus |= (u32)PM_USAGE_OTHER_SUBSYSTEM;
			}
		}
		Reqm = Reqm->NextSubsystem;
	}

	return UsageStatus;
}

/****************************************************************************/
/**
 * @brief  Check if any clock for a given device is active
 * @param  Device      Device whose clocks need to be checked
 *
 * @return XST_SUCCESS if any one clock for given device is active
 *         XST_FAILURE if all clocks for given device are inactive
 *
 ****************************************************************************/
XStatus XPmDevice_IsClockActive(const XPm_Device *Device)
{
	XStatus Status = XST_FAILURE;
	const XPm_ClockHandle *ClkHandle = Device->ClkHandles;
	const XPm_OutClockNode *Clk;
	u32 Enable;

	while (NULL != ClkHandle) {
		if ((NULL != ClkHandle->Clock) &&
		    (ISOUTCLK(ClkHandle->Clock->Node.Id))) {
			Clk = (XPm_OutClockNode *)ClkHandle->Clock;
			Status = XPmClock_GetClockData(Clk, (u32)TYPE_GATE, &Enable);
			if (XST_SUCCESS == Status) {
				if (1U == Enable) {
					Status = XST_SUCCESS;
					goto done;
				}
			} else if (XPM_INVALID_CLK_SUBNODETYPE == Status) {
				PmDbg("Clock 0x%x does not have Clock Gate\n\r",
						ClkHandle->Clock->Node.Id);
				Status = XST_SUCCESS;
			} else {
				PmDbg("XPmClock_GetClockData failed with Status 0x%x"
						" for clock id: 0x%x\r\n",
						Status,
						ClkHandle->Clock->Node.Id);
			}
		}
		ClkHandle = ClkHandle->NextClock;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Check if any subsystem requested perticular device or not.
 *
 * @param  DeviceId	Device ID
 * @param  SubsystemId	Subsystem ID
 *
 * @return XST_SUCCESS if device is requested from subsystem
 *         XST_FAILURE if device is not requested or error code
 *
 ****************************************************************************/
XStatus XPmDevice_IsRequested(const u32 DeviceId, const u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device = XPmDevice_GetById(DeviceId);
	const XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);
	const XPm_Requirement *Reqm;

	if ((NULL == Device) || (NULL == Subsystem)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Reqm = FindReqm(Device, Subsystem);
	if ((NULL != Reqm) && (1U == Reqm->Allocated)) {
		Status = XST_SUCCESS;
	}

done:
	return Status;
}

XStatus XPmDevice_GetWakeupLatency(const u32 DeviceId, u32 *Latency)
{
	XStatus Status = XST_SUCCESS;
	const XPm_Device *Device = XPmDevice_GetById(DeviceId);
	u32 Lat = 0;

	*Latency = 0;

	if (NULL == Device) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if ((u8)XPM_DEVSTATE_RUNNING == Device->Node.State) {
		goto done;
	}

	*Latency = GetLatencyFromState(Device, Device->Node.State);

	if (NULL != Device->Power) {
		Status = XPmPower_GetWakeupLatency(Device->Power->Node.Id, &Lat);
		if (XST_SUCCESS != Status) {
			*Latency += Lat;
		}
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function stores the sysmon addresses so that they can be retrieved
 * by index.
 *
 * @param Id: Sysmon node ID
 * @param BaseAddress: Sysmon address
 *
 * @return XST_SUCCESS if successful else XST_FAILURE
 *
 ****************************************************************************/
XStatus XPm_SetSysmonNode(u32 Id, u32 BaseAddress)
{
	XStatus Status = XST_FAILURE;
	u32 NodeIndex = NODEINDEX(Id);

	/*
	 * We assume that the Node ID class, subclass and type has already
	 * been validated before, so only check bounds here against index
	 */
	if ((u32)XPM_NODEIDX_MONITOR_MAX > NodeIndex) {
		PmSysmonAddresses[NodeIndex] = BaseAddress;
		Status = XST_SUCCESS;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief Returns address to sysmon node from given Node Index
 *
 * @param SysmonIndex: Node Index assigned to a Sysmon node
 *
 * @return Sysmon address to given node; else 0
 *
 ****************************************************************************/
u32 XPm_GetSysmonByIndex(const u32 SysmonIndex)
{
	u32 BaseAddress = 0U;

	/* Make sure we are working with only Index. */
	u32 Index = (SysmonIndex & NODE_INDEX_MASK);
	if ((u32)XPM_NODEIDX_MONITOR_MAX <= Index) {
		goto done;
	}

	BaseAddress = PmSysmonAddresses[Index];

done:
	return BaseAddress;
}

/****************************************************************************/
/**
 * @brief  Add device security, virtualization and coherency attributes
 *
 * @param  Args		CDO command arguments
 * @param  NumArgs	Total number of arguments
 *
 * @return Status of the operation.
 *
 ****************************************************************************/
XStatus AddDevAttributes(const u32 *Args, const u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_DeviceAttr *DevAttr = NULL;
	XPm_Device *Dev = XPmDevice_GetById(Args[0]);

	/* Check for device presence and sufficient arguments */
	if ((NULL == Dev) || (NumArgs < 9U)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	DevAttr = (XPm_DeviceAttr *)XPm_AllocBytes(sizeof(XPm_DeviceAttr));
	if (NULL == DevAttr) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	/* Store the security attributes */
	DevAttr->SecurityBaseAddr = Args[6];
	DevAttr->Security[0].Offset = (u16)((Args[7] >> 16U) & 0xFFFFU);
	DevAttr->Security[0].Mask = (u16)(Args[7] & 0xFFFFU);
	DevAttr->Security[1].Offset = (u16)((Args[8] >> 16U) & 0xFFFFU);
	DevAttr->Security[1].Mask = (u16)(Args[8] & 0xFFFFU);

	/* Check for the coherency and virtualization attributes */
	if (NumArgs > 9U) {
		if (NumArgs < 12U) {
			Status = XST_INVALID_PARAM;
			goto done;
		}

		DevAttr->CohVirtBaseAddr = Args[9];
		DevAttr->Coherency.Offset = (u16)((Args[10] >> 16U) & 0xFFFFU);
		DevAttr->Coherency.Mask = (u16)(Args[10] & 0xFFFFU);
		DevAttr->Virtualization.Offset = (u16)((Args[11] >> 16U) & 0xFFFFU);
		DevAttr->Virtualization.Mask = (u16)(Args[11] & 0xFFFFU);
	}

	Dev->DevAttr = DevAttr;
	Status = XST_SUCCESS;

done:
	return Status;
}
