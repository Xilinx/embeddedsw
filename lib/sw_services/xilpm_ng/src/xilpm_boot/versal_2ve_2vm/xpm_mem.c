/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_defs.h"
#include "xpm_device.h"
#include "xpm_powerdomain.h"
#include "xpm_mem.h"
#include "xpm_alloc.h"
#include "xpm_debug.h"
#include "xpm_regs.h"

/**
 * Dummy Runtime function which can be overridden by the Runtime Library
 * We need to define this function here to avoid linking error in case
 */

/****************************************************************************/
/**
 * @brief  weak function for boot to check accessible subsystems
 *
 * @param  SubsystemId 	checks whether given subsystem is accessible during boot
 * @param  RegionAddr 	don't care
 * @param  RegionSize 	don't care
 * @param  IsPLMem 	don't care
 *
 * @return Status of the operation: XPM_SUCCESS, XST_INVALID_PARAM
 *
 ****************************************************************************/
XStatus __attribute__((weak, noinline)) XPm_IsMemRegnAddressValid(u32 SubsystemId, u64 RegionAddr, u64 RegionSize, u8 *IsPLMem) {

	volatile XStatus Status = XPM_FAILURE;
	(void)(RegionAddr);
	(void)(RegionSize);
	*IsPLMem = 0U;

	/**
	 * Supported Subsystem for boot:
	 * - PM_SUBSYS_DEFAULT
	 * - PM_SUBSYS_PMC
	 * - PM_SUBSYS_ASU
	 */
	if (!IS_BUILTIN_SUBSYSTEM(SubsystemId)) {
		PmErr("Unexpected SubsystemId [0x%x]\r\n", SubsystemId);
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XPM_SUCCESS;

done:
	return Status;
};

/*************************************************************************************************/

XStatus XPmMemDevice_Init(XPm_MemDevice *MemDevice,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset,
		u32 MemStartAddress, u32 MemEndAddress)
{
	XStatus Status = XST_FAILURE;

	Status = XPmDevice_Init(&MemDevice->Device, Id, BaseAddress, Power, Clock,
				Reset);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	MemDevice->StartAddress = MemStartAddress;
	MemDevice->EndAddress = MemEndAddress;


done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Add mem-range device to internal data-structure map
 *
 * @param  MemRegnDevice 	Pointer to store (struct XPm_MemRegnDevice)
 * @param  Id 				Device Id
 * @param  Address 			Address associated with device
 * @param  Size 			Length of the range
 *
 * @return Status of the operation.
 *
 ****************************************************************************/
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

/****************************************************************************/
/**
 * @brief  Add mem-range device to internal data-structure map
 *
 * @param  DeviceId 	Variable stored device id extracted from cdo
 * @param  Address 		Address associated with device
 * @param  Size 		Length of the range
 *
 * @return Status of the operation.
 *
 ****************************************************************************/
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
		ASU_RAM = 0x1,
		PPU_INSTR_CNTLR_MEM = 0x2,
		PPU_DATA_CNTLR_MEM = 0x3,
		RTCA_IMAGE_STORE_REGN = 0x4
	};

	static struct XPm_ExclusionRegion {
		u64 StartAddress;
		u64 EndAddress;
	} XPm_ExcludedList[] = {
		[PMC_RAM] = {XPLMI_PMCRAM_BASEADDR, (XPLMI_PMCRAM_BASEADDR + XPLMI_TOTAL_CHUNK_SIZE - 1ULL)},
		[ASU_RAM] = {XPLMI_ASU_RAM_BASE_ADDR, XPLMI_ASU_RAM_HIGH_ADDR},
		[PPU_INSTR_CNTLR_MEM] = {PPU_RAM_INSTR_CNTLR_BASEADDR, PPU_RAM_INSTR_CNTLR_HIGHADDR},
		[PPU_DATA_CNTLR_MEM] = {PPU_RAM_DATA_CNTLR_BASEADDR, PPU_RAM_DATA_CNTLR_HIGHADDR},
		[RTCA_IMAGE_STORE_REGN] = {0x0ULL, 0x0ULL}
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

	/* check security concerns for address overflow */
	if (XPm_ExcludedList[RTCA_IMAGE_STORE_REGN].StartAddress > XPm_ExcludedList[RTCA_IMAGE_STORE_REGN].EndAddress) {
		PmErr("RTCA Image Store Address Range Overflow\r\n");
		Status = XST_INVALID_PARAM;
		goto done;
	}

	for (u32 index=0U; index<ARRAY_SIZE(XPm_ExcludedList); index++) {
		u64 excludedStartAddress = XPm_ExcludedList[index].StartAddress;
		u64 excludedEndAddress = XPm_ExcludedList[index].EndAddress;

		/* size for any exclusion region must be non-zero */
		if (0x0ULL == (excludedEndAddress - excludedStartAddress)) {
			continue;
		}

		AddrRangeStatus isRangeOverlapExclusion = ADDR_OUT_OF_RANGE;
		AddrRangeStatus isExclusionInRange = ADDR_OUT_OF_RANGE;

		/* condition to check whether entire excluded range falls in given address range (address + size) */
		isExclusionInRange = IsAddrWithinRange(excludedStartAddress, excludedEndAddress,\
								RegionStartAddr, RegionEndAddr);
		/* condition to check whether there is any overlap of given address range (address + size) with excluded range */
		if (((RegionStartAddr >= excludedStartAddress) && (RegionStartAddr <= excludedEndAddress)) || \
			((RegionEndAddr >= excludedStartAddress) && (RegionEndAddr <= excludedEndAddress))) {
			isRangeOverlapExclusion = ADDR_IN_RANGE;
		}

		/* If there is any overlap of provided range and excluded range, not allowed to access! */
		/**
		 * Excluded Range: |--------------------|
		 * Ex Range1:  |-------| 			(given range overlaps excluded range partially at the start)
		 * Ex Range2:                       |-------| 	(given range overlaps excluded range partially at the end)
		 * Ex Range3:             |-------| 		(given range completely inside excluded range)
		 * Ex Range4: |-------------------------------| (given range fully contains excluded range)
		 */
		if ((ADDR_IN_RANGE == isRangeOverlapExclusion) || (ADDR_IN_RANGE == isExclusionInRange)) {
			PmInfo("Range falls in Exclusion List [%d]\r\n", index);
			Status = XPM_FAILURE;
			goto done;
		}
	}
	Status = XPM_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Checks whether given address range (addr + size) is read/write accessible
 * 		by a given subsystem id.
 *
 * @param  SubsystemId 	The Subsystem to check for given addr range
 * @param  RegionAddr 	Start Address of the range
 * @param  RegionSize 	Size of the range
 *
 * @return Status of the operation : XPM_INVALID_SUBSYSID, XST_INVALID_PARAM, XPM_FAILURE,
 * 				XPM_PM_NO_ACCESS, XPM_SUCCESS
 *
 ****************************************************************************/
XStatus XPm_IsMemAddressValid(u32 SubsystemId, u64 RegionAddr, u64 RegionSize) {
	XStatus Status = XPM_FAILURE;
	const XPm_MemDevice *MemDevice = NULL;
	const XPm_MemCtrlrDevice *MCDev = NULL;
	u32 DeviceId;

	if (RegionSize < 1U) {
		Status = XST_INVALID_PARAM;
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

	/**
	 * XPm_IsMemRegnAddressValid() for boot and runtime-eemi, check whether given subsystem is accessible during boot-time
	 * XPm_IsMemRegnAddressValid() for runtime-subsys, check whether given address range falls in mem-regn
	 * 	- For runtime, IsPLMem flag is used to also check PL Power Domain ( IsPLMem flag is don't care for boot & eemi )
	 */
	u8 IsPLMem = 0U;
	Status = XPm_IsMemRegnAddressValid(SubsystemId, RegionAddr, RegionSize, &IsPLMem);
	if (XPM_SUCCESS != Status) {
		goto done;
	}
	/** For PL Mem-Regn, we have already checked the PLD Power Domain in XPm_IsMemRegnAddressValid()
	 *  Hence, for XPM_SUCCESS and IsPLMem, we can skip rest everything! ( as PL doesn't have parent node )
	*/
	if (IsPLMem) {
		goto done;
	}

	/** manually over-write the status to failure
	 *  this is for Non-PL Mem-Regns ( as we further check parent controller's state )
	*/
	Status = XPM_FAILURE;

	/**
	 * We iterate over OCM/TCM Regions first to avoid the overlapping corner cases with DDR
	 */
	for (u32 idx = (u32)XPM_NODEIDX_DEV_MIN; idx <= (u32)XPM_NODEIDX_DEV_MAX; idx++) {
		/* OCM/TCM Case */
		const XPm_Device *Device = XPmDevice_GetByIndex(idx);
		if (NULL == Device) {
			continue;
		}
		DeviceId = Device->Node.Id;

		/**
		 * Currently, we only support checking OCM/TCM Devices
		 */
		if (!IS_MEM_DEV_OCM(DeviceId) && !IS_MEM_DEV_TCM(DeviceId)) {
			continue;
		}

		MemDevice = (XPm_MemDevice *)Device;
		if (NULL == MemDevice) {
			continue;
		}
		u64 StartAddress = (u64)MemDevice->StartAddress;
		u64 EndAddress = (u64)MemDevice->EndAddress;
		if (ADDR_IN_RANGE == IsAddrWithinRange(RegionAddr, (RegionAddr + RegionSize - 1U), StartAddress, EndAddress)) {
			if ((u8)XPM_DEVSTATE_RUNNING == MemDevice->Device.Node.State) {
				Status = XPM_SUCCESS;
			}
			else {
				PmInfo("Node [0x%x] not running...!\r\n", DeviceId);
				Status = XPM_PM_NO_ACCESS;
			}
			goto done;
		}
	}

	/**
	 * Now we check DDR devices at the very end ( this avoids any address overlapping )
	 */
	for (u32 idx = (u32)XPM_NODEIDX_DEV_MIN; idx <= (u32)XPM_NODEIDX_DEV_MAX; idx++) {
		/* DDR Case */
		const XPm_Device *Device = XPmDevice_GetByIndex(idx);
		if (NULL == Device) {
			continue;
		}
		DeviceId = Device->Node.Id;

		if (!IS_MEM_DEV_DDRMC(DeviceId)) {
			continue;
		}

		MCDev = (XPm_MemCtrlrDevice *)Device;
		if (NULL == MCDev) {
			continue;
		}

		for (u32 Cnt = 0U; Cnt < MCDev->RegionCount; Cnt++) {
			u64 StartAddress = MCDev->Region[Cnt].Address;
			u64 EndAddress = MCDev->Region[Cnt].Address + MCDev->Region[Cnt].Size - 1U;
			if ( ADDR_IN_RANGE == IsAddrWithinRange(RegionAddr, (RegionAddr + RegionSize - 1U), StartAddress, EndAddress)) {
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