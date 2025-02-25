/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi_dma.h"
#include "xplmi.h"
#include "xpm_device.h"
#include "xpm_core.h"
#include "xpm_regs.h"
#include "xpm_rpucore.h"

#include "xpm_api.h"
#include "xpm_pmc.h"
#include "xpm_mem.h"
#include "xpm_pslpdomain.h"
#include "xpm_psfpdomain.h"
#include "xpm_debug.h"
#include "xpm_pldevice.h"
#include "xpm_reset.h"
#include "xpm_alloc.h"
#include "xpm_aiedevice.h"

#define SD_DLL_DIV_MAP_RESET_VAL	(0x50505050U)

XPm_Device *PmDevices[(u32)XPM_NODEIDX_DEV_MAX];
static XPm_Device *PmPlDevices[(u32)XPM_NODEIDX_DEV_PLD_MAX];
static XPm_Device *PmAieDevices[(u32)XPM_NODEIDX_DEV_AIE_MAX];
static XPm_Device *PmVirtualDevices[(u32)XPM_NODEIDX_DEV_VIRT_MAX];
static XPm_Device *PmHbMonDevices[(u32)XPM_NODEIDX_DEV_HB_MON_MAX];
static XPm_Device *PmMemRegnDevices[(u32)XPM_NODEIDX_DEV_MEM_REGN_MAX];
static u32 PmNumPlDevices;
static u32 PmNumVirtualDevices;
static u32 PmNumHbMonDevices;
static u32 PmNumMemRegnDevices;
static u32 PmSysmonAddresses[(u32)XPM_NODEIDX_MONITOR_MAX];
static u32 PmNumDevices;
static u32 PmNumAieDevices;
//static XStatus SetClocks(const XPm_Device *Device, u32 Enable);

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

static XStatus SetMemRegnDeviceNode(u32 Id, XPm_Device *Device)
{
	XStatus Status = XST_INVALID_PARAM;
	u32 NodeIndex = NODEINDEX(Id);

	if ((NULL != Device) && ((u32)XPM_NODEIDX_DEV_MEM_REGN_MAX > NodeIndex)) {
		PmMemRegnDevices[NodeIndex] = Device;
		PmNumMemRegnDevices++;
		Status = XST_SUCCESS;
	}

	return Status;
}

static XStatus SetAieDeviceNode(u32 Id, XPm_Device *Device)
{
	XStatus Status = XST_INVALID_PARAM;
	u32 NodeIndex = NODEINDEX(Id);

	/*
	 * We assume that the Node ID class, subclass and type has _already_
	 * been validated before, so only check bounds here against index
	 */
	if ((NULL != Device) && ((u32)XPM_NODEIDX_DEV_AIE_MAX > NodeIndex)) {
		PmAieDevices[NodeIndex] = Device;
		PmNumAieDevices++;
		Status = XST_SUCCESS;
	}

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
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmDevice_SdResetWorkaround(const XPm_Device *Device)
{
	XStatus Status = XST_FAILURE;
	/*
	 * As per EDT-1054057 SD/eMMC DLL modes are failing after
	 * SD controller reset. Reset SD_DLL_MAP registers after
	 * reset release as a workaround.
	 */
	/* SDIO1 is EMMC and there is no DLL_RESET for SDIO1 */
	if ((PM_DEV_SDIO_0 == Device->Node.Id)) {
		Status = ResetSdDllRegs(Device);
	} else {
		Status = XST_SUCCESS;
	}

	return Status;
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
		Device->Node.State = (u8)XPM_DEVSTATE_RUNNING;
	} else {
		DbgErr = XPM_INT_ERR_DEVICE_PWR_PARENT_UP;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);

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

	Device->Power = Power;
	Device->WfPwrUseCnt = 0;
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

	if ((u32)XPM_NODESUBCL_DEV_PL == NODESUBCLASS(Id)) {
		Status = SetPlDeviceNode(Id, Device);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SET_PL_DEV;
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
	} else if ((u32)XPM_NODESUBCL_DEV_AIE == NODESUBCLASS(Id)){
		Status = SetAieDeviceNode(Id, Device);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SET_AIE_DEV;
		}
	} else if (((u32)XPM_NODESUBCL_DEV_MEM_REGN == NODESUBCLASS(Id)) &&
			((u32)XPM_NODETYPE_DEV_MEM_REGN == NODETYPE(Id))) {
		Status = SetMemRegnDeviceNode(Id, Device);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SET_MEM_REGN_DEV;
			goto done;
		}
	} else {
		Status = SetDeviceNode(Id, Device);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SET_DEV_NODE;
		}
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmVirtDev_DeviceInit(XPm_Device *Device, u32 Id, XPm_Power *Power)
{
	XStatus Status = XST_FAILURE;

	Status = XPmDevice_Init(Device, Id, 0U, Power, NULL, NULL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
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

XStatus __attribute__((weak)) XPmDevice_Reset(const XPm_Device *Device, const XPm_ResetActions Action)
{
	(void)Device;
	(void)Action;
	PmWarn("XPmDevice_Reset is not implemented in XilPmBoot\r\n");
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
	} else if (((u32)XPM_NODESUBCL_DEV_MEM_REGN == NODESUBCLASS(DeviceId)) &&
			((u32)XPM_NODETYPE_DEV_MEM_REGN == NODETYPE(DeviceId))) {
		if ((u32)XPM_NODEIDX_DEV_MEM_REGN_MAX <= NODEINDEX(DeviceId)) {
			goto done;
		}
		DevicesHandle = PmMemRegnDevices;
	} else {
		if ((u32)XPM_NODEIDX_DEV_MAX <= NODEINDEX(DeviceId)) {
			goto done;
		}
		DevicesHandle = PmDevices;
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



XStatus __attribute__((weak)) XPmDevice_Release(const u32 SubsystemId, const u32 DeviceId,
			  const u32 CmdType)
{
	(void)SubsystemId;
	(void)DeviceId;
	(void)CmdType;
	return XST_SUCCESS;
}

XStatus XPmDevice_GetState(const u32 DeviceId,
			u32 *const DeviceState)
{
	XStatus Status = XPM_ERR_DEVICE_STATUS;

	*DeviceState = XPM_DEVSTATE_UNUSED;
	XPm_Device* Device = XPmDevice_GetById(DeviceId);
	if (NULL == Device) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	*DeviceState = Device->Node.State;

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
				PmErr("Expecting AIE or PL device ID\r\n");
				Status = XPM_INVALID_DEVICEID;
				if (XST_SUCCESS != Status) {
					goto done;
				}
			}
		} else {
			PmErr("Expecting PL device or Power Node parent Id\r\n");
			Status = XPM_INVALID_DEVICEID;
			goto done;
		}
	}
done:
	return Status;
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
