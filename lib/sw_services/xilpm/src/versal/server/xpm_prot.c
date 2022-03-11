/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_util.h"
#include "xpm_common.h"
#include "xpm_debug.h"
#include "xpm_mem.h"
#include "xpm_node.h"
#include "xpm_regs.h"
#include "xpm_prot.h"

/**
 * Protection nodes (XPPUs + XMPUs)
 */
static XPm_Prot *PmProtections[XPM_NODEIDX_PROT_MAX];

/**
 * Max XMPU regions count
 */
#define MAX_MEM_REGIONS				(16U)

/**
 * Memory region type to XMPU map.
 */
#define MEMIDX(Idx) \
	((u32)(Idx) - (u32)XPM_NODETYPE_DEV_OCM_REGN)

static const u8 MemToXMPUMap[2] = {
	[MEMIDX(XPM_NODETYPE_DEV_OCM_REGN)] = (u8)XPM_NODEIDX_PROT_XMPU_OCM,
	[MEMIDX(XPM_NODETYPE_DEV_DDR_REGN)] = (u8)XPM_NODEIDX_PROT_XMPU_PMC,
};

/**
 * Get relevant XMPU instance index from corresponding memory region type.
 */
#define MEM_TYPE_TO_XMPU(MemRegnNodeId)	\
	(MemToXMPUMap[MEMIDX(NODETYPE(MemRegnNodeId))])

/**
 * For XMPU Region-12 (0-based):
 *   Offset step size is different
 */
static const u8 RegnCfgOffsets[MAX_MEM_REGIONS] = {
	0x00, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x18,
	0x20, 0x18, 0x18, 0x18,
};

/**
 * For XMPU Region-13 (0-based):
 *   Offset step size is different
 */
static const u8 RegnAddrOffsets[MAX_MEM_REGIONS] = {
	0x00, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x18,
	0x18, 0x20, 0x18, 0x18,
};

/**
 * Returns XMPU Region "config" offset for given region number.
 */
#define REGN_CONFIG_OFFSET(region)	((u32)RegnCfgOffsets[(region)])

/**
 * Returns XMPU Region "address" offset for given region number.
 * For:
 *  - Start Lo/Hi addr registers
 *  - End lo/hi addr registers
 */
#define REGN_ADDR_OFFSET(region)	((u32)RegnAddrOffsets[(region)])

/****************************************************************************/
/**
 * @brief  Returns a handle to protection node from given Node Id
 *
 * @param  Id: Node Id assigned to a XPPU/XMPU node
 *
 * @return A handle to given node; else NULL
 *
 ****************************************************************************/
XPm_Prot *XPmProt_GetById(const u32 Id)
{
	XPm_Prot *Prot = NULL;

	if (((u32)XPM_NODECLASS_PROTECTION != NODECLASS(Id)) ||
	    ((u32)XPM_NODEIDX_PROT_MAX <= NODEINDEX(Id))) {
		goto done;
	}

	Prot = PmProtections[NODEINDEX(Id)];
	/* Check that internal ID is same as given ID or not. */
	if ((NULL != Prot) && (Id != Prot->Id)) {
		Prot = NULL;
	}

done:
	return Prot;
}

/****************************************************************************/
/**
 * @brief  Returns a handle to protection node from given Node Id/Index
 *
 * @param  Idx: Node Id/Index assigned to a XPPU/XMPU node
 *
 * @return A handle to given node; else NULL
 *
 ****************************************************************************/
static XPm_Prot *XPmProt_GetByIndex(const u32 Idx)
{
	XPm_Prot *Prot = NULL;

	if ((u32)XPM_NODEIDX_PROT_MAX <= NODEINDEX(Idx)) {
		goto done;
	}

	Prot = PmProtections[NODEINDEX(Idx)];
	/* Check that internal Index is same as given Index or not. */
	if ((NULL != Prot) && (Idx != NODEINDEX(Prot->Id))) {
		Prot = NULL;
	}

done:
	return Prot;
}

/****************************************************************************/
/**
 * @brief  Enable XMPU
 *
 * @param  Mpu: An instance of XMPU
 *
 * @return N/A
 *
 ****************************************************************************/
static void XPmProt_XmpuEnable(XPm_ProtMpu *Mpu)
{
	u32 BaseAddr;
	u32 RegVal;

	if ((u8)XPM_PROT_ENABLED == Mpu->Node.State) {
		goto done;
	}

	/* XMPU base address */
	BaseAddr = Mpu->Node.BaseAddress;

	/* Read region alignment */
	PmIn32(BaseAddr + XMPU_CTRL_OFFSET, RegVal);
	Mpu->AlignCfg = (u8)(RegVal >> XMPU_CTRL_ALIGN_CFG_SHIFT) & 0x1U;

	/* Disable default RD/WR configuration on protected regions */
	PmRmw32(BaseAddr + XMPU_CTRL_OFFSET,
			XMPU_CTRL_DISABLE_DEFAULT_S_REGION_MASK,
			XMPU_CTRL_DISABLE_DEFAULT_S_REGION_MASK);

	PmIn32(BaseAddr + XMPU_CTRL_OFFSET, RegVal);

	/* Enable SW state */
	Mpu->Node.State = (u8)XPM_PROT_ENABLED;
done:
	return;
}

/****************************************************************************/
/**
 * @brief  Disable XMPU
 *
 * @param  Mpu: An instance of XMPU
 *
 * @return N/A
 *
 ****************************************************************************/
static void XPmProt_XmpuDisable(XPm_ProtMpu *Mpu)
{
	u32 BaseAddr, Region;
	u32 RegnCfgAddr = 0;

	/* XMPU base address */
	BaseAddr = Mpu->Node.BaseAddress;

	/* Starting region config address */
	RegnCfgAddr = BaseAddr + XMPU_R00_CONFIG_OFFSET;

	/* Disable all the regions */
	for (Region = 0; Region < MAX_MEM_REGIONS; Region++) {
		/* Disable all memory regions */
		RegnCfgAddr += REGN_CONFIG_OFFSET(Region);
		PmRmw32(RegnCfgAddr,
			XMPU_RXX_CONFIG_ENABLE_MASK,
			~XMPU_RXX_CONFIG_ENABLE_MASK);
	}

	/* Disable SW state */
	Mpu->Node.State = (u8)XPM_PROT_DISABLED;
}

/****************************************************************************/
/**
 * @brief  Setup a XMPU region with given subsystem requirements and enable it
 *
 * @param  Reqm: device requirement imposed by caller subsystem
 * @param  RegionId: Region Id (0-15)
 * @param  Enable: enable(1)/disable(0) region
 *
 * @return XST_SUCCESS if successful else appropriate failure code
 *
 * @note   An assumption: @Reqm and @Mpu is Non-NULL
 *
 ****************************************************************************/
static XStatus XPmProt_XmpuSetupRegion(const XPm_Requirement *Reqm,
				const XPm_ProtMpu *Mpu,
				u32 RegionId,
				u32 Enable)
{
	XStatus Status = XST_FAILURE;
	u32 CfgToWr, RegnCfgAddr;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u16 Usage, Security, RdAllowed, WrAllowed, NSRegnCheck;

	if (MAX_MEM_REGIONS <= RegionId) {
		DbgErr = XPM_INT_ERR_INVALID_REGION;
		Status = XST_INVALID_PARAM;
		goto done;
	}
	RegnCfgAddr = Mpu->Node.BaseAddress
		+ XMPU_R00_CONFIG_OFFSET
		+ REGN_CONFIG_OFFSET(RegionId);

	/* Requirement flags */
	Usage = USAGE_POLICY(Reqm->Flags);
	Security = SECURITY_POLICY(Reqm->Flags);
	RdAllowed = RD_POLICY(Reqm->Flags);
	WrAllowed = WR_POLICY(Reqm->Flags);
	NSRegnCheck = REGN_CHECK_POLICY(Reqm->Flags);

	PmDbg("Configuring region: %u\r\n", RegionId);
	PmDbg("Usage: 0x%x, Security: 0x%x, Rd: 0x%x, Wr: 0x%x, Check: 0x%x\r\n",
			Usage, Security, RdAllowed, WrAllowed, NSRegnCheck);

	/* Setup config to be written; except enable */
	CfgToWr = ((((u32)NSRegnCheck << XMPU_RXX_CONFIG_NSCHECKTYPE_SHIFT)
			& XMPU_RXX_CONFIG_NSCHECKTYPE_MASK)
		 | (((u32)Security << XMPU_RXX_CONFIG_REGIONNS_SHIFT)
			 & XMPU_RXX_CONFIG_REGIONNS_MASK)
		 | (((u32)WrAllowed << XMPU_RXX_CONFIG_WRALLOWED_SHIFT)
			 & XMPU_RXX_CONFIG_WRALLOWED_MASK)
		 | (((u32)RdAllowed << XMPU_RXX_CONFIG_RDALLOWED_SHIFT)
			 & XMPU_RXX_CONFIG_RDALLOWED_MASK));

	/* Setup memory region config */
	if ((u16)REQ_NONSHARED != Usage) {
		/**
		 * TODO: Add support for following usage modes:
		 *   - REQ_SHARED
		 *   - REQ_TIME_SHARED
		 *   - REQ_NO_RESTRICTION
		 */
		DbgErr = XPM_INT_ERR_NO_FEATURE;
		Status = XST_NO_FEATURE;
		goto done;
	} else {
		/**
		 * Only REQ_NONSHARED usage mode is supported for now;
		 * so no reconfiguration of Masters is required.
		 * Therefore, allow all the configured masters belonging
		 * to this subsystem to access this region.
		 */
		CfgToWr |= (Enable & XMPU_RXX_CONFIG_ENABLE_MASK);
		PmOut32(RegnCfgAddr, CfgToWr);
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief  Configure XMPU according to access control policies from
 *         subsystem requirement for given memory region
 *
 * @param  Reqm: memory region device requirement
 *               imposed by caller subsystem
 * @param  Enable: enable(1)/disable(0) region
 *
 * @return XST_SUCCESS if successful else appropriate failure code
 *
 ****************************************************************************/
XStatus XPmProt_MpuControl(const XPm_Requirement *Reqm, u32 Enable)
{
	XStatus Status = XST_FAILURE;
	u64 RegnStart, RegnEnd, DevStart, DevEnd;
	u32 BaseAddr, DeviceId, Region;
	u32 RegnStartLo, RegnStartHi, RegnStartLoVal, RegnStartHiVal;
	u32 RegnEndLo, RegnEndHi, RegnEndLoVal, RegnEndHiVal;
	const XPm_ProtMpu *Mpu = NULL;
	const XPm_MemDevice *MemDevice = NULL;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u8 XmpuIdx;

	if ((NULL == Reqm) || (NULL == Reqm->Device)) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		Status = XPM_ERR_DEVICE;
		goto done;
	}
	MemDevice = (XPm_MemDevice *)Reqm->Device;
	DevStart  = (u64)(MemDevice->StartAddress);
	DevEnd    = (u64)(MemDevice->EndAddress);

	PmDbg("StartAddress: 0x%08x - EndAddress: 0x%08x\r\n",
			MemDevice->StartAddress,
			MemDevice->EndAddress);
	PmDbg("DevStart: 0x%08x - DevEnd: 0x%08x\r\n",
			(u32)DevStart, (u32)DevEnd);

	/* Get relevant XMPU index */
	DeviceId = Reqm->Device->Node.Id;
	XmpuIdx = MEM_TYPE_TO_XMPU(DeviceId);

	PmDbg("DeviceId: 0x%08x | XMPU IDX: %d\r\n", DeviceId, XmpuIdx);

	Mpu = (XPm_ProtMpu *)XPmProt_GetByIndex(XmpuIdx);
	if (NULL == Mpu) {
		DbgErr = XPM_INT_ERR_INVALID_NODE;
		goto done;
	}

	/* Return if XMPU is not enabled */
	if ((u8)XPM_PROT_ENABLED != Mpu->Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}
	BaseAddr = Mpu->Node.BaseAddress;

	/* Region-0 addresses */
	RegnStartLo = BaseAddr + XMPU_R00_START_LO_OFFSET;
	RegnStartHi = BaseAddr + XMPU_R00_START_HI_OFFSET;
	RegnEndLo   = BaseAddr + XMPU_R00_END_LO_OFFSET;
	RegnEndHi   = BaseAddr + XMPU_R00_END_HI_OFFSET;

	/**
	 * Iterate over XMPU regions
	 */
	for (Region = 0; Region < MAX_MEM_REGIONS; Region++) {
		/* compute region addresses */
		RegnStartLo += REGN_ADDR_OFFSET(Region);
		RegnStartHi += REGN_ADDR_OFFSET(Region);
		RegnEndLo   += REGN_ADDR_OFFSET(Region);
		RegnEndHi   += REGN_ADDR_OFFSET(Region);

		/* retrieve region start and end addresses */
		PmIn32(RegnStartLo, RegnStartLoVal);
		PmIn32(RegnStartHi, RegnStartHiVal);
		PmIn32(RegnEndLo, RegnEndLoVal);
		PmIn32(RegnEndHi, RegnEndHiVal);

		RegnStart = (((u64)RegnStartHiVal) << 32U) | RegnStartLoVal;
		RegnEnd   = (((u64)RegnEndHiVal) << 32U) | RegnEndLoVal;

		/**
		 * Enable/Disable all the regions which are configured.
		 */
		if ((DevStart < DevEnd)
		&&  (DevStart >= RegnStart)
		&&  (DevEnd <= RegnEnd)) {
			/* Configure and enable/disable the region based on requirements */
			Status = XPmProt_XmpuSetupRegion(Reqm, Mpu, Region, Enable);
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_SETUP_REGION;
				goto done;
			}
		}
	}

	/**
	 * Memory region is not found in configured XMPU regions,
	 * however just return with success since HW will handle the case.
	 */
	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief  Common handler for XMPU enable/disable control
 *
 * @param  Mpu: Xmpu node object reference
 * @param  Func: Xmpu InitNode Function
 * @param  Args: Args list
 * @param  NumArgs: total number of args present in Args list
 *
 * @return XST_SUCCESS if successful else appropriate failure code
 *
 ****************************************************************************/
static XStatus XPmProt_XmpuInitCtrl(XPm_ProtMpu *Mpu, u32 Func,
				    const u32 *Args,
				    u32 NumArgs)
{
	XStatus Status = XST_FAILURE;

	(void)Args;
	(void)NumArgs;

	switch (Func) {
	case (u32)FUNC_XMPU_ENABLE:
		XPmProt_XmpuEnable(Mpu);
		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_XMPU_DISABLE:
		XPmProt_XmpuDisable(Mpu);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_NO_FEATURE;
		break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  Initialize protection base class struct and add it to database
 *
 * @param  Prot: Pointer to an uninitialized protection class struct
 * @param  Id: Node Id assigned to a XPPU/XMPU node
 * @param  BaseAddr: Base address of the given XPPU/XMPU
 *
 * @return XST_SUCCESS if successful; appropriate failure code otherwise
 *
 * @note   This is internally called from XMPU/XPPU initialization routines
 *
 ****************************************************************************/
static XStatus XPmProt_Init(XPm_Prot *Prot, u32 Id, u32 BaseAddr)
{
	XStatus Status = XST_FAILURE;
	u32 NodeIndex = NODEINDEX(Id);

	if ((u32)XPM_NODEIDX_PROT_MAX <= NodeIndex) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	XPmNode_Init(Prot, Id, (u8)XPM_PROT_DISABLED, BaseAddr);

	PmProtections[NodeIndex] = Prot;
	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Initialize and add a XPPU instance to database
 *
 * @param  Ppu: Pointer to an uninitialized XPPU instance
 * @param  Id: Node Id assigned to a XPPU node
 * @param  BaseAddr: Base address of the given XPPU
 * @param  Power: Node Id of the parent power domain node
 *
 * @return XST_SUCCESS if successful; appropriate failure code otherwise
 *
 ****************************************************************************/
XStatus XPmProt_PpuInit(XPm_ProtPpu *Ppu,
			u32 Id,
			u32 BaseAddr,
			XPm_Power *Power)
{
	XStatus Status = XST_FAILURE;

	Status = XPmProt_Init(&Ppu->Node, Id, BaseAddr);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Parity status bits */
	Ppu->MIDParityEn = 0U;
	Ppu->AperParityEn = 0U;

	/* Default aperture mask */
	Ppu->AperPermInitMask = 0U;

	/* Parent power domain */
	Ppu->Power = Power;

	/* Node Ops */
	Ppu->Ops = NULL;

	/* Init apertures */
	Status = Xil_SMemSet(&Ppu->A64k, sizeof(Ppu->A64k), 0, sizeof(Ppu->A64k));
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = Xil_SMemSet(&Ppu->A1m, sizeof(Ppu->A1m), 0, sizeof(Ppu->A1m));
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = Xil_SMemSet(&Ppu->A512m, sizeof(Ppu->A512m), 0, sizeof(Ppu->A512m));

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Initialize and add a XMPU instance to database
 *
 * @param  Mpu: Pointer to an uninitialized XMPU instance
 * @param  Id: Node Id assigned to a XMPU node
 * @param  BaseAddr: Base address of the given XMPU
 * @param  Power: Node Id of the parent power domain node
 *
 * @return XST_SUCCESS if successful; appropriate failure code otherwise
 *
 ****************************************************************************/
XStatus XPmProt_MpuInit(XPm_ProtMpu *Mpu,
			u32 Id,
			u32 BaseAddr,
			XPm_Power *Power)
{
	XStatus Status = XST_FAILURE;

	Status = XPmProt_Init(&Mpu->Node, Id, BaseAddr);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Init */
	Mpu->AlignCfg = 0;
	Mpu->Power = Power;

	/* Node Ops */
	Mpu->Ops = &XPmProt_XmpuInitCtrl;

done:
	return Status;
}
