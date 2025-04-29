/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_defs.h"
#include "xpm_device.h"
#include "xpm_powerdomain.h"
#include "xpm_mem.h"
#include "xpm_debug.h"
#include "xpm_regs.h"

static const XPm_StateCap XPmMemDeviceStates[] = {
	{
		.State = (u8)XPM_DEVSTATE_UNUSED,
		.Cap = XPM_MIN_CAPABILITY,
	}, {
		.State = (u8)XPM_DEVSTATE_RUNNING,
		.Cap = PM_CAP_ACCESS | PM_CAP_CONTEXT,
	},
};

static const XPm_StateTran XPmMemDevTransitions[] = {
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

static XStatus HandleMemDeviceState(XPm_Device* const Device, const u32 NextState)
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
						     XPM_DEVEVENT_SHUTDOWN);
		} else {
			Status = XST_SUCCESS;
		}
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

	return Status;
}

static const XPm_DeviceFsm XPmMemDeviceFsm = {
	DEFINE_DEV_STATES(XPmMemDeviceStates),
	DEFINE_DEV_TRANS(XPmMemDevTransitions),
	.EnterState = &HandleMemDeviceState,
};

static void TcmEccInit(const XPm_MemDevice *Tcm, u32 Mode)
{
	u32 Size = Tcm->EndAddress - Tcm->StartAddress + 1U;
	u32 Id = Tcm->Device.Node.Id;
	u32 Base = Tcm->StartAddress;

	Base -= XPm_CombTcm(Id,Mode);

	if (0U != Size) {
		s32 Status = XPlmi_EccInit(Base, Size);
		if (XST_SUCCESS != Status) {
			PmWarn("Error %d in EccInit of 0x%x\r\n", Status, Tcm->Device.Node.Id);
		}
	}
	return;
}

static XStatus HandleTcmDeviceState(XPm_Device* const Device, u32 const NextState)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Rpu0Device = NULL,*Rpu1Device = NULL;
	u32 Id = Device->Node.Id;
	u32 Mode;

	Status = XPm_GetRpuDevice(&Rpu0Device, &Rpu1Device, Id);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if ((NULL == Rpu0Device) || (NULL == Rpu1Device))
	{
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	switch (Device->Node.State) {
	case (u8)XPM_DEVSTATE_UNUSED:
		if ((u32)XPM_DEVSTATE_RUNNING == NextState) {
			Status = XPmDevice_BringUp(Device);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			/* Request the RPU clocks. Here both core having same RPU clock */
			Status = XPmClock_Request(Rpu0Device->ClkHandles);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			/* TCM is only accessible when the RPU is powered on and out of reset and is in halted state
			 * so bring up RPU too when TCM is requested*/
			Status = HaltRpuCore(Rpu0Device, Rpu1Device, Id, &Mode);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			/* Tcm should be ecc initialized */
			TcmEccInit((XPm_MemDevice *)Device, Mode);
		}
		Status = XST_SUCCESS;
		break;
	case (u8)XPM_DEVSTATE_RUNNING:
		if ((u32)XPM_DEVSTATE_UNUSED == NextState) {
			Status = Device->HandleEvent(&Device->Node,
						     XPM_DEVEVENT_SHUTDOWN);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			/* Release the RPU clocks. Here both core having same RPU clock */
			Status = XPmClock_Release(Rpu0Device->ClkHandles);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

done:
	return Status;
}

static const XPm_DeviceFsm XPmTcmDeviceFsm = {
	DEFINE_DEV_STATES(XPmMemDeviceStates),
	DEFINE_DEV_TRANS(XPmMemDevTransitions),
	.EnterState = &HandleTcmDeviceState,
};

XStatus XPmMemDevice_Init(XPm_MemDevice *MemDevice,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset,
		u32 MemStartAddress, u32 MemEndAddress)
{
	XStatus Status = XST_FAILURE;
	u32 Type = NODETYPE(Id);

	Status = XPmDevice_Init(&MemDevice->Device, Id, BaseAddress, Power, Clock,
				Reset);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	MemDevice->StartAddress = MemStartAddress;
	MemDevice->EndAddress = MemEndAddress;

	switch (Type) {
	case (u32)XPM_NODETYPE_DEV_DDR:
		XPm_AssignDdrFsm(MemDevice);
		break;
	case (u32)XPM_NODETYPE_DEV_TCM:
		MemDevice->Device.DeviceFsm = &XPmTcmDeviceFsm;
		break;
	default:
		MemDevice->Device.DeviceFsm = &XPmMemDeviceFsm;
		break;
	}

	if (NULL == MemDevice->Device.DeviceFsm) {
		MemDevice->Device.DeviceFsm = &XPmMemDeviceFsm;
	}

done:
	return Status;
}

static XStatus XPmMemRegnDevice_Init(XPm_MemRegnDevice *MemRegnDevice, u32 Id,
			     u64 Address, u64 Size)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Power;
	const u32 PowerId = PM_POWER_NOC;

	Power = XPmPower_GetById(PowerId);
	if (NULL == Power) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	Status = XPmDevice_Init(&MemRegnDevice->Device, Id, 0U, Power,
				NULL, NULL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	MemRegnDevice->AddrRegion.Address = Address;
	MemRegnDevice->AddrRegion.Size = Size;

done:
	return Status;
}

XStatus XPm_AddMemRegnDevice(u32 DeviceId, u64 Address, u64 Size)
{
	XStatus Status = XST_FAILURE;
	XPm_MemRegnDevice *MemRegnDevice;

	MemRegnDevice  = (XPm_MemRegnDevice *)XPm_AllocBytes(sizeof(XPm_MemRegnDevice));
	if (NULL == MemRegnDevice) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}
	Status = XPmMemRegnDevice_Init(MemRegnDevice, DeviceId, Address, Size);

done:
	return Status;
}

/**
 * @brief Checks whether a given address range is not in any of the exclusion regions
 * @param RegionAddr 	Range Start Address
 * @param RegionSize 	Range Size
 *
 * @note This application for Subsystem != PM_SUBSYS_PMC
 * @return ==XPM_FAILURE (if range in exclusion list), ==XPM_SUCCESS (if range not in exclusion list)
 *************************************************************************************/
static XStatus IsRangeOutsideExclusionList(u64 RegionAddr, u64 RegionSize) {
	XStatus Status = XPM_FAILURE;
	u64 imgStoreRegionAddress = 0x0ULL;
	u64 imgStoreRegionSize    = 0x0ULL;

	if (RegionSize < 1U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	u64 RegionStartAddr = RegionAddr;
	u64 RegionEndAddr = RegionAddr + (RegionSize - 1ULL);

	enum ExclusionRegionList {
		PMC_RAM = 0x0,
		PSM_RAM = 0x1,
		PPU_INSTR_CNTLR_MEM = 0x2,
		PPU_DATA_CNTLR_MEM = 0x3,
		RTCA_IMAGE_STORE_REGN = 0x4,
		RTCFG_PLM_INPLACE_UPDATE = 0x5
	};

	static struct XPm_ExclusionRegion {
		u64 StartAddress;
		u64 EndAddress;
	} XPm_ExcludedList[] = {
		[PMC_RAM] = {XPLMI_PMCRAM_BASEADDR, (XPLMI_PMCRAM_BASEADDR + XPLMI_TOTAL_CHUNK_SIZE - 1ULL)},
		[PSM_RAM] = {XPLMI_PSM_RAM_BASE_ADDR, XPLMI_PSM_RAM_HIGH_ADDR},
		[PPU_INSTR_CNTLR_MEM] = {PPU_RAM_INSTR_CNTLR_BASEADDR, PPU_RAM_INSTR_CNTLR_HIGHADDR},
		[PPU_DATA_CNTLR_MEM] = {PPU_RAM_DATA_CNTLR_BASEADDR, PPU_RAM_DATA_CNTLR_HIGHADDR},
		[RTCA_IMAGE_STORE_REGN] = {0x0ULL, 0x0ULL},
		[RTCFG_PLM_INPLACE_UPDATE] = {0x0ULL, 0x0ULL}
	};
	/** extract RTCA Image Store Address Range
	 * 32-bit higher store address: 0xF2014288
	 * 32-bit lower store address:  0xf201428C
	 * 32-bit size: 0xF2014290
	*/
	imgStoreRegionAddress = (RTCA_IMAGE_STORE_ADDR_HIGH << 32U) | RTCA_IMAGE_STORE_ADDR_LOW;
	imgStoreRegionSize = (u64)(RTCA_IMAGE_STORE_ADDR_SIZE);
	/* only initialize array parameters if size is non-zero */
	if (imgStoreRegionSize >= 1U) {
		XPm_ExcludedList[RTCA_IMAGE_STORE_REGN].StartAddress = imgStoreRegionAddress;
		XPm_ExcludedList[RTCA_IMAGE_STORE_REGN].EndAddress = imgStoreRegionAddress + imgStoreRegionSize - 1ULL;
	}

#if defined(VERSAL_NET)
	/**
	 * We only have INPLACE_UPDATE feature for versalnet
	 */
	u64 inplaceUpdateRegionAddress = XPm_In32((u32)XPLMI_RTCFG_PLM_RSVD_DDR_ADDR);
	u64 inplaceUpdateRegionSize = XPm_In32((u32)XPLMI_RTCFG_PLM_RSVD_DDR_SIZE);
	/* only initialize array parameters if size is non-zero */
	if (inplaceUpdateRegionSize >= 1U) {
		XPm_ExcludedList[RTCFG_PLM_INPLACE_UPDATE].StartAddress = inplaceUpdateRegionAddress;
		XPm_ExcludedList[RTCFG_PLM_INPLACE_UPDATE].EndAddress = inplaceUpdateRegionAddress + inplaceUpdateRegionSize - 1U;
	}
#endif
	for (u32 index=0U; index<ARRAY_SIZE(XPm_ExcludedList); index++) {
		u64 excludedStartAddress = XPm_ExcludedList[index].StartAddress;
		u64 excludedEndAddress = XPm_ExcludedList[index].EndAddress;

		/* size for any exclusion region must be non-zero */
		if (0x0ULL == excludedEndAddress) {
			continue;
		}

		u32 isRangeOverlapExclusion = 0U;
		u32 isExclusionInRange = 0U;

		/* condition to check whether entire excluded range falls in given address range (address + size) */
		isExclusionInRange = IsAddrWithinRange(excludedStartAddress, excludedEndAddress,\
								RegionStartAddr, RegionEndAddr);
		/* condition to check whether there is any overlap of given address range (address + size) with excluded range */
		if (((RegionStartAddr >= excludedStartAddress) && (RegionStartAddr <= excludedEndAddress)) || \
			((RegionEndAddr >= excludedStartAddress) && (RegionEndAddr <= excludedEndAddress))) {
			isRangeOverlapExclusion = 1U;
		}

		/* If there is any overlap of provided range and excluded range, not allowed to access! */
		/**
		 * Excluded Range: |--------------------|
		 * Ex Range1:  |-------| 			(given range overlaps excluded range partially at the start)
		 * Ex Range2:                       |-------| 	(given range overlaps excluded range partially at the end)
		 * Ex Range3:             |-------| 		(given range completely inside excluded range)
		 * Ex Range4: |-------------------------------| (given range fully contains excluded range)
		 */
		if ((1U == isRangeOverlapExclusion) || (1U == isExclusionInRange)) {
			PmInfo("Range falls in Exclusion List [%d]\r\n", index);
			Status = XPM_FAILURE;
			goto done;
		}
	}
	Status = XPM_SUCCESS;

done:
	return Status;
}

#if defined(XPM_ENABLE_MEM_REGN_CHECKING)
static XStatus IsMemRegnAddressValid(u32 SubsystemId, u64 RegionAddr, u64 RegionSize, u8 *IsPLMem) {
	XStatus Status = XPM_FAILURE;
	const XPm_Requirement *Reqm = NULL;
	const XPm_Subsystem *Subsystem = NULL;
	const XPm_MemRegnDevice *MemRegnDevice = NULL;
	u32 DeviceId;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	if (RegionSize < 1U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/**
	 * For built-in subsystems, we don't have any Mem-Regns
	 * and the below algorithm checking is not required!
	*/
	if (IS_BUILTIN_SUBSYSTEM(SubsystemId)) {
		Status = XPM_SUCCESS;
		goto done;
	}

	*IsPLMem = 0U;
	Reqm = Subsystem->Requirements;

	while (NULL != Reqm) {
		DeviceId = Reqm->Device->Node.Id;

		if ((u32)XPM_NODESUBCL_DEV_MEM_REGN != NODESUBCLASS(DeviceId)) {
			Reqm = Reqm->NextDevice;
			continue;
		}

		MemRegnDevice  = (XPm_MemRegnDevice *)Reqm->Device;
		u64 StartAddress = MemRegnDevice->AddrRegion.Address;
		u64 MemSize = MemRegnDevice->AddrRegion.Size;
		u32 Flags = 0U;
		if (IS_PL_MEM_REGN(MemSize)) {
			Flags = PL_MEM_REGN_FLAGS(MemSize);
			/* zero-ing out upper flag bits[31:28] from 64bit size */
			MemSize &= ~PL_MEM_REGN_FLAGS_MASK_64;
		}
		u64 EndAddress = StartAddress + MemSize - 1U;

		if (IsAddrWithinRange(RegionAddr, (RegionAddr + RegionSize - 1U), StartAddress, EndAddress)) {
			/* Check whether PL mem or not */
			if (PL_MEM_REGN == Flags) {
				*IsPLMem = 1U;
			}
			Status = XPM_SUCCESS;
			break;
		}

		Reqm = Reqm->NextDevice;
	}

	if (XPM_SUCCESS != Status) {
		PmInfo("Range not in Mem-Regn: [0x%x]\r\n", SubsystemId);
		goto done;
	}
	if (1U == *IsPLMem) {
		/* Address Range provided falls in PL Region */
		/**
		 * Check whether PL Power Domain is up ( because no parent node for PL )
		 * Note: For any other Mem-Regn, power checking is done by retrieving
		 * their parent's controller node in XPm_IsMemAddressValid()
		 */
		const XPm_Power *Power = XPmPower_GetById(PM_POWER_PLD);
		if ((NULL != Power) && ((u8)XPM_POWER_STATE_ON == Power->Node.State)) {
			Status = XPM_SUCCESS;
		}
		else {
			Status = XPM_FAILURE;
		}
	}

done:
	return Status;
}
#endif

XStatus XPm_IsMemAddressValid(u32 SubsystemId, u64 RegionAddr, u64 RegionSize) {
	XStatus Status = XPM_FAILURE;
	const XPm_MemDevice *MemDevice = NULL;
	const XPm_MemCtrlrDevice *MCDev = NULL;
	u32 DeviceId;

	if (RegionSize < 1U) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	if ((INVALID_SUBSYSID == SubsystemId) ||
		((MAX_NUM_SUBSYSTEMS) <= NODEINDEX(SubsystemId))) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	/**
	 * For any subsystem other than PMC, we must check exclusion list
	 * If range is in exclusion list, it's not allowed and we return
	 */
	Status = IsRangeOutsideExclusionList(RegionAddr, RegionSize);
	if (XPM_SUCCESS != Status) {
		if (PM_SUBSYS_PMC == SubsystemId) {
			/* If range is in exclusion list and caller is PMC_SUBSYS, then it's accessible */
			Status = XPM_SUCCESS;
		}
		goto done;
	}

#if defined(XPM_ENABLE_MEM_REGN_CHECKING)
	u8 IsPlMem = 0U;
	Status = IsMemRegnAddressValid(SubsystemId, RegionAddr, RegionSize, &IsPlMem);
	if (XPM_SUCCESS != Status) {
		goto done;
	}
	/** For PL Mem-Regn, we have already checked the PLD Power Domain in IsMemRegnAddressValid()
	 *  Hence, for XPM_SUCCESS and IsPlMem, we can skip rest everything! ( as PL doesn't have parent node )
	*/
	if (IsPlMem) {
		goto done;
	}
#endif
	/* manually over-write the status to failure */
	Status = XPM_FAILURE;

	/**
	* We iterate over OCM/TCM Regions first to avoid the overlapping corner cases with DDR
	*/
	for (u32 i = (u32)XPM_NODEIDX_DEV_MIN; i <= (u32)XPM_NODEIDX_DEV_MAX; i++) {

		const XPm_Device *Device = XPmDevice_GetByIndex(i);
		if (NULL == Device) {
			continue;
		}
		DeviceId = Device->Node.Id;

		if (!IS_MEM_DEV_OCM(DeviceId) && !IS_MEM_DEV_TCM(DeviceId)) {
			continue;
		}

		/* OCM/TCM Case */
		MemDevice = (XPm_MemDevice *)Device;
		if (NULL == MemDevice) {
			continue;
		}
		u64 StartAddress = (u64)MemDevice->StartAddress;
		u64 EndAddress = (u64)MemDevice->EndAddress;
		if (IsAddrWithinRange(RegionAddr, (RegionAddr + RegionSize - 1U), StartAddress, EndAddress)) {
			if ((u8)XPM_DEVSTATE_RUNNING == MemDevice->Device.Node.State) {
				Status = XPM_SUCCESS;
			}
			else {
				PmInfo("Node [0x%x] parent controller not running...!\r\n", DeviceId);
				Status = XPM_PM_NO_ACCESS;
			}
			goto done;
		}
	}

	for (u32 i = (u32)XPM_NODEIDX_DEV_MIN; i <= (u32)XPM_NODEIDX_DEV_MAX; i++) {

		const XPm_Device *Device = XPmDevice_GetByIndex(i);
		if (NULL == Device) {
			continue;
		}
		DeviceId = Device->Node.Id;

		if (!IS_MEM_DEV_DDRMC(DeviceId)) {
			continue;
		}

		/* DDR Case */
		MCDev = (XPm_MemCtrlrDevice *)Device;
		if (NULL == MCDev) {
			continue;
		}

		for (u32 Cnt = 0U; Cnt < MCDev->RegionCount; Cnt++) {
			u64 StartAddress = MCDev->Region[Cnt].Address;
			u64 EndAddress = MCDev->Region[Cnt].Address + MCDev->Region[Cnt].Size - 1U;
			if (IsAddrWithinRange(RegionAddr, (RegionAddr + RegionSize - 1U), StartAddress, EndAddress)) {
				/*
				* the memory controller should be in running state
				* for the address to be accessible
				*/
				if ((u8)XPM_DEVSTATE_RUNNING != MCDev->Device.Node.State) {
					PmInfo("DDRMC [0x%x] not running...!\r\n", DeviceId);
					Status = XPM_PM_NO_ACCESS;
					goto done;
				}
				Status = XPM_SUCCESS;
				break;
			}
		}
		/*
		* If the address is valid and the memory controller is interleaved,
		* then keep going to check for paired memory controller.
		*/
		if ((XPM_SUCCESS == Status) && (0U == MCDev->IntlvIndex)) {
			break;
		}

	}

done:
	return Status;
}