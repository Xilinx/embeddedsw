/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

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

#define SIZE_64K				(0x10000U)
#define SIZE_1M					(0x100000U)
#define SIZE_512M				(0x20000000U)

/**
 * Following macros represent staring and ending aperture offsets
 * as defined in the XPPU HW Spec.
 * 	64k Apertures: 0-255
 * 	1m Apertures: 384-399
 * 	512m Aperture: 400
 */
#define APER_64K_START				(0U)
#define APER_64K_END				(255U)
#define APER_1M_START				(384U)
#define APER_1M_END				(399U)
#define APER_512M_START				(400U)
#define APER_512M_END				(400U)

#define MAX_PERM_REGS				(13U)
#define MAX_APER_PARITY_FIELDS			(4U)
#define APER_PARITY_FIELD_WIDTH			(5U)
#define APER_PARITY_FIELD_MASK			(0x1FU)

/** Refer: Section "XPPU protection for IPI" from XPPU Spec */
#define APER_IPI_MIN				(49U)
#define APER_IPI_MAX				(63U)

/**
 * XPPU Macros
 */
#define PERM_MASK		(XPPU_APERTURE_TRUSTZONE_MASK | XPPU_APERTURE_PERMISSION_MASK)
#define PERM(ApertureAddr)	((ApertureAddr) & XPPU_APERTURE_PERMISSION_MASK)
#define TZ(ApertureAddr)	((ApertureAddr) & XPPU_APERTURE_TRUSTZONE_MASK)
#define APER_PERM(Tz, Perms)	(TZ(((Tz) << XPPU_APERTURE_TRUSTZONE_OFFSET)) | PERM((Perms)))

#define APERIDX_TO_OFFSET(AperIdx, IdxStart)	((AperIdx) - (IdxStart))
#define APERIDX_64K_TO_OFFSET(AperIdx)		(APERIDX_TO_OFFSET(AperIdx, APER_64K_START))
#define APERIDX_1M_TO_OFFSET(AperIdx)		(APERIDX_TO_OFFSET(AperIdx, APER_1M_START))

#define OFFSET_TO_APERIDX(Offset, IdxStart)	((Offset) + (IdxStart))
#define OFFSET_TO_APERIDX_64K(Offset)		(OFFSET_TO_APERIDX(Offset, APER_64K_START))
#define OFFSET_TO_APERIDX_1M(Offset)		(OFFSET_TO_APERIDX(Offset, APER_1M_START))

#define ADDR_TO_OFFSET(DevAddr, Start, Size)	(((DevAddr) - (Start))/(Size))

#define APER_ADDR(Base, Start, Offset)		((Base) + (Start) + ((Offset) * 4U))
#define APERADDR_64K(Base, Offset)		(APER_ADDR(Base, XPPU_APERTURE_0_OFFSET, Offset))
#define APERADDR_1M(Base, Offset)		(APER_ADDR(Base, XPPU_APERTURE_384_OFFSET, Offset))
#define APERADDR_512M(Base)			(APER_ADDR(Base, XPPU_APERTURE_400_OFFSET, 0U))

#define PERM_CHECK_REG_ADDR(Base, AperIdx)	\
		((Base) + \
		 (((AperIdx) / 32U) * 4U) + \
		 XPPU_ENABLE_PERM_CHECK_REG00_OFFSET)
#define PERM_CHECK_REG_MASK(AperIdx)		\
		((u32)1U << ((AperIdx) % 32U))

#define XPM_NODEIDX_XPPU_MIN			((u32)XPM_NODEIDX_PROT_XPPU_LPD)
#define XPM_NODEIDX_XPPU_MAX			((u32)XPM_NODEIDX_PROT_XPPU_PMC_NPI)

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
 * @brief  Write to a XPPU Aperture with given value (with or without parity)
 *
 * @param  Ppu: An instance of XPPU
 * @param  AperAddr: Aperture address
 * @param  AperVal: Aperture Value to be written
 *
 * @return N/A
 *
 * @note   This function computes or skips computing the parity bits
 *         for Aperture depending on the parity enable flag
 *
 ****************************************************************************/
static void XPmProt_XppuSetAperture(const XPm_ProtPpu *Ppu,
				    u32 AperAddr,
				    u32 AperVal)
{
	u32 i, RegVal, Field, FieldParity, Tz;
	u32 Parity = 0;

	/* Clear parity bits */
	RegVal = AperVal & ~XPPU_APERTURE_PARITY_MASK;

	if (0U == Ppu->AperParityEn) {
		goto done;
	}

	/* Extract TrustZone bit */
	Tz = (RegVal >> XPPU_APERTURE_TRUSTZONE_OFFSET) & 0x1U;

	/**
	 * Enabling aperture parity provides a benefit that in terms of
	 * security of the XPPU module itself. Anytime an aperture entry
	 * is fetched from the local RAM, HW and SW computed parities are
	 * compared; if they mismatch then transaction fails and parity error
	 * is flagged.
	 *
	 * Parity for bits in this register.
	 * bit 28: [4:0].
	 * bit 29: [9:5].
	 * bit 30: [14:10].
	 * bit 31: [27],[19:15].
	 */
	for (i = 0; i < MAX_APER_PARITY_FIELDS; i++)
	{
		Field = (RegVal >> (i * APER_PARITY_FIELD_WIDTH));
		FieldParity = XPm_ComputeParity(Field & APER_PARITY_FIELD_MASK);
		if (i == (MAX_APER_PARITY_FIELDS - 1U)) {
			FieldParity ^= Tz;
		}
		Parity |= (FieldParity << i);
	}

	/* Set parity bits */
	RegVal |= (Parity << XPPU_APERTURE_PARITY_SHIFT);

done:
	PmOut32(AperAddr, RegVal);
}

/****************************************************************************/
/**
 * @brief  Enable XPPU
 *
 * @param  Ppu: An instance of XPPU
 * @param  ApertureInitVal: Default permission mask for all apertures
 *
 * @return N/A
 *
 ****************************************************************************/
static void XPmProt_XppuEnable(XPm_ProtPpu *Ppu, u32 ApertureInitVal)
{
	u32 i = 0U;
	u32 Address = 0U;
	u32 RegVal = 0U;
	u32 Platform = XPm_GetPlatform();
	u32 PlatformVersion = XPm_GetPlatformVersion();
	u32 IdCode = XPm_GetIdCode();
	u32 BaseAddr = Ppu->Node.BaseAddress;

	if ((u8)XPM_PROT_ENABLED == Ppu->Node.State) {
		goto done;
	}

	/* Set default aperture permission mask */
	Ppu->AperPermInitMask = ApertureInitVal & PERM_MASK;

	if ((PLATFORM_VERSION_SILICON == Platform) &&
	    (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion) &&
	    (PMC_TAP_IDCODE_DEV_SBFMLY_VC1902 == (IdCode & PMC_TAP_IDCODE_DEV_SBFMLY_MASK))) {
		/* Disable permission checks for all apertures */
		Address = BaseAddr + XPPU_ENABLE_PERM_CHECK_REG00_OFFSET;
		for (i = 0; i < MAX_PERM_REGS; i++)
		{
			PmOut32(Address, 0x0U);
			Address = Address + 0x4U;
		}
	}

	/* Get Number of Apertures supported */
	PmIn32(BaseAddr + XPPU_M_APERTURE_64KB_OFFSET, RegVal);
	Ppu->A64k.Total = RegVal;
	PmIn32(BaseAddr + XPPU_M_APERTURE_1MB_OFFSET, RegVal);
	Ppu->A1m.Total = RegVal;
	PmIn32(BaseAddr + XPPU_M_APERTURE_512MB_OFFSET, RegVal);
	Ppu->A512m.Total = RegVal;

	/* Store parity bits settings */
	PmIn32(BaseAddr + XPPU_CTRL_OFFSET, RegVal);
	Ppu->MIDParityEn = (u8)((RegVal >> XPPU_CTRL_MID_PARITY_EN_SHIFT) & 0x1U);
	Ppu->AperParityEn = (u8)((RegVal >> XPPU_CTRL_APER_PARITY_EN_SHIFT) & 0x1U);

	/* Initialize all apertures for default value */
	Address = BaseAddr + XPPU_APERTURE_0_OFFSET;
	for (i = APER_64K_START; i <= APER_64K_END; i++) {
		/**
		 * In Versal, message buffer protection is moved out of XPPU, to IPI.
		 * Therefore, XPPU permissions for IPI specific apertures (aperture 49 to 63)
		 * should be configured to allow all. This is applicable to LPD XPPU only.
		 *
		 * Refer "XPPU protection for IPI" from XPPU DID.
		 */
		if (((u32)XPM_NODEIDX_PROT_XPPU_LPD == NODEINDEX(Ppu->Node.Id))
			&& ((i >= APER_IPI_MIN) && (i <= APER_IPI_MAX))) {
			XPmProt_XppuSetAperture(Ppu, Address,
					(ApertureInitVal | XPPU_APERTURE_PERMISSION_MASK));
		} else {
			XPmProt_XppuSetAperture(Ppu, Address, ApertureInitVal);
		}
		Address = Address + 0x4U;
	}
	Address = BaseAddr + XPPU_APERTURE_384_OFFSET;
	for (i = APER_1M_START; i <= APER_1M_END; i++) {
		XPmProt_XppuSetAperture(Ppu, Address, ApertureInitVal);
		Address = Address + 0x4U;
	}
	Address = BaseAddr + XPPU_APERTURE_400_OFFSET;
	for (i = APER_512M_START; i <= APER_512M_END; i++) {
		XPmProt_XppuSetAperture(Ppu, Address, ApertureInitVal);
		Address = Address + 0x4U;
	}

	if ((PLATFORM_VERSION_SILICON == Platform) &&
	    (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion) &&
	    (PMC_TAP_IDCODE_DEV_SBFMLY_VC1902 == (IdCode & PMC_TAP_IDCODE_DEV_SBFMLY_MASK))) {
		/* Enable permission checks for all apertures */
		Address = BaseAddr + XPPU_ENABLE_PERM_CHECK_REG00_OFFSET;
		for (i = 0; i < MAX_PERM_REGS; i++)
		{
			PmOut32(Address, 0xFFFFFFFFU);
			Address = Address + 0x4U;
		}
	}

	/* Get Aperture start and end addresses */
	PmIn32(BaseAddr + XPPU_BASE_64KB_OFFSET, RegVal);
	Ppu->A64k.Start = RegVal;
	Ppu->A64k.End = Ppu->A64k.Start + (Ppu->A64k.Total * SIZE_64K) - 1U;

	PmIn32(BaseAddr + XPPU_BASE_1MB_OFFSET, RegVal);
	Ppu->A1m.Start = RegVal;
	Ppu->A1m.End = Ppu->A1m.Start + (Ppu->A1m.Total * SIZE_1M) - 1U;

	PmIn32(BaseAddr + XPPU_BASE_512MB_OFFSET, RegVal);
	Ppu->A512m.Start = RegVal;
	Ppu->A512m.End = Ppu->A512m.Start + (Ppu->A512m.Total * SIZE_512M) - 1U;

	/* Enable Xppu */
	PmRmw32(BaseAddr + XPPU_CTRL_OFFSET,
			XPPU_CTRL_ENABLE_MASK, XPPU_CTRL_ENABLE_MASK);

	/* Enable SW state */
	Ppu->Node.State = (u8)XPM_PROT_ENABLED;
done:
	return;
}

/****************************************************************************/
/**
 * @brief  Disable XPPU
 *
 * @param  Ppu: An instance of XPPU
 *
 * @return N/A
 *
 ****************************************************************************/
static void XPmProt_XppuDisable(XPm_ProtPpu *Ppu)
{
	u32 Address, idx;
	u32 Platform =  XPm_GetPlatform();
	u32 PlatformVersion = XPm_GetPlatformVersion();
	u32 IdCode = XPm_GetIdCode();
	u32 PpuBase = Ppu->Node.BaseAddress;

	if ((PLATFORM_VERSION_SILICON == Platform) &&
	    (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion) &&
	    (PMC_TAP_IDCODE_DEV_SBFMLY_VC1902 == (IdCode & PMC_TAP_IDCODE_DEV_SBFMLY_MASK))) {
		/* Disable permission checks for all apertures */
		Address = PpuBase + XPPU_ENABLE_PERM_CHECK_REG00_OFFSET;
		for (idx = 0; idx < MAX_PERM_REGS; idx++)
		{
			PmOut32(Address, 0x0);
			Address = Address + 0x4U;
		}
	}

	/* Disable Xppu */
	PmRmw32(PpuBase + XPPU_CTRL_OFFSET,
			XPPU_CTRL_ENABLE_MASK, ~XPPU_CTRL_ENABLE_MASK);

	/* Disable SW state */
	Ppu->Node.State = (u8)XPM_PROT_DISABLED;
}

/****************************************************************************/
/**
 * @brief  Run Dynamic Re-Configuration of XPPU
 *
 * @param  Ppu: An instance of XPPU
 * @param  PermCheckAddr: Aperture permission check address (Only used for ES1)
 * @param  PermCheckMask: Aperture permission check mask (Only used for ES1)
 * @param  AperIdx: Aperture index
 * @param  AperAddr: Aperture address
 * @param  AperVal: Aperture Value to be written
 *
 * @return N/A
 *
 * @note   This function runs the configuration sequence to dynamically
 * modify the XPPU configuration
 *
 ****************************************************************************/
static void XPmProt_XppuDynReconfig(const XPm_ProtPpu *Ppu,
				    u32 PermCheckAddr,
				    u32 PermCheckMask,
				    u32 AperIdx,
				    u32 AperAddr,
				    u32 AperVal)
{
	u32 Platform = XPm_GetPlatform();
	u32 PlatformVersion = XPm_GetPlatformVersion();
	u32 IdCode = XPm_GetIdCode();
	u32 PpuBaseAddr = Ppu->Node.BaseAddress;

	if ((PLATFORM_VERSION_SILICON == Platform) &&
	    (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion) &&
	    (PMC_TAP_IDCODE_DEV_SBFMLY_VC1902 == (IdCode & PMC_TAP_IDCODE_DEV_SBFMLY_MASK))) {
		/* Disable XPPU */
		PmRmw32(PpuBaseAddr + XPPU_CTRL_OFFSET,
				XPPU_CTRL_ENABLE_MASK,
				~XPPU_CTRL_ENABLE_MASK);

		/* Disable permission check of the required aperture */
		PmRmw32(PermCheckAddr, PermCheckMask, 0);

		/* Program permissions of the aperture to be reconfigured */
		XPmProt_XppuSetAperture(Ppu, AperAddr, AperVal);

		/* Enable permission check of the required aperture */
		PmRmw32(PermCheckAddr, PermCheckMask, PermCheckMask);

		/* Enable XPPU */
		PmRmw32(PpuBaseAddr + XPPU_CTRL_OFFSET,
				XPPU_CTRL_ENABLE_MASK,
				XPPU_CTRL_ENABLE_MASK);

	} else {
		/* Program aperture index to be reconfigured */
		PmOut32(PpuBaseAddr + XPPU_DYNAMIC_RECONFIG_APER_ADDR_OFFSET, AperIdx);

		/* Program permissions of the aperture to be reconfigured */
		XPmProt_XppuSetAperture(Ppu,
				PpuBaseAddr + XPPU_DYNAMIC_RECONFIG_APER_PERM_OFFSET,
				AperVal);

		/* Indicate new permission are available in dynamic reconfig registers */
		PmOut32(PpuBaseAddr + XPPU_DYNAMIC_RECONFIG_EN_OFFSET, 1);

		/* Update aperture to be reconfigured with new permissions */
		XPmProt_XppuSetAperture(Ppu, AperAddr, AperVal);

		/* Indicate new permission are available in permission memory */
		PmOut32(PpuBaseAddr + XPPU_DYNAMIC_RECONFIG_EN_OFFSET, 0);
	}
}

/****************************************************************************/
/**
 * @brief  Reconfigure an aperture with given permissions
 *
 * @param  Ppu: An instance of XPPU
 * @param  AperIdx: Aperture index
 * @param  AperVal: Aperture value to be written

 * @return XST_SUCCESS if successful else appropriate failure code
 *
 ****************************************************************************/
static XStatus XPmProt_XppuReconfig(const XPm_ProtPpu *Ppu,
				    u32 AperIdx,
				    u32 AperVal)
{
	XStatus Status = XST_FAILURE;
	u32 Offset = 0;
	u32 PermCheckAddr = 0;
	u32 PermCheckMask = 0;
	u32 AperAddr = 0;
	u32 PpuBaseAddr = Ppu->Node.BaseAddress;

	if ((u8)XPM_PROT_ENABLED != Ppu->Node.State) {
		Status = XPM_INVALID_STATE;
		goto done;
	}

	if (APER_64K_END >= AperIdx) {
		Offset = APERIDX_64K_TO_OFFSET(AperIdx);
		AperAddr = APERADDR_64K(PpuBaseAddr, Offset);
	} else if ((APER_1M_START <= AperIdx) && (APER_1M_END >= AperIdx)) {
		Offset = APERIDX_1M_TO_OFFSET(AperIdx);
		AperAddr = APERADDR_1M(PpuBaseAddr, Offset);
	} else if (APER_512M_START == AperIdx) {
		AperAddr = APERADDR_512M(PpuBaseAddr);
	} else {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	PermCheckAddr = PERM_CHECK_REG_ADDR(PpuBaseAddr, AperIdx);
	PermCheckMask = PERM_CHECK_REG_MASK(AperIdx);

	XPmProt_XppuDynReconfig(Ppu, PermCheckAddr, PermCheckMask,
				AperIdx, AperAddr, AperVal);

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Dynamically reconfigure permissions for the given aperture
 *
 * @param  Reqm: Requirement for the given device from a subsystem
 * @param  Ppu: An instance of XPPU
 * @param  AperIdx: Aperture index
 * @param  AperAddr: Aperture address
 * @param  Enable: Enable(1)/Disable(0) masters in permission ram
 *
 * @return XST_SUCCESS on success; else XST_FAILURE
 *
 * @note   This is a helper function for XPmProt_PpuControl()
 *         It derives the permission value for the given aperture by looking
 *         at the sharing policy for the given device node. This derived
 *         permission value is then used to dynamically reconfigure the aperture
 *
 ****************************************************************************/
static XStatus XPmProt_PpuPermReconfig(const XPm_Requirement *Reqm,
				       const XPm_ProtPpu *Ppu,
				       u32 AperIdx, u32 AperAddr,
				       u32 Enable)
{
	XStatus Status = XST_FAILURE;
	const XPm_Requirement *DevReqm = NULL;
	u32 AperPerm = 0U;
	u32 CurrPerms = 0U;
	u32 PermCheckAddr, PermCheckMask;
	u32 PpuBase = Ppu->Node.BaseAddress;
	u32 Security = SECURITY_POLICY((u32)Reqm->Flags);
	u16 Usage = USAGE_POLICY(Reqm->Flags);

	/* Perm check enable bit for derived aperture */
	PermCheckAddr = PERM_CHECK_REG_ADDR(PpuBase, AperIdx);
	PermCheckMask = PERM_CHECK_REG_MASK(AperIdx);

	/* Get permission mask configured for this aperture as of now */
	PmIn32(AperAddr, AperPerm);

	if (0U != Enable) {
		switch (Usage) {
		case (u16)REQ_NONSHARED:
		case (u16)REQ_TIME_SHARED:
			/* Replace existing perms if device requires exclusive access */
			AperPerm = APER_PERM(Security, Reqm->AperPerm);
			Status = XST_SUCCESS;
			break;
		case (u16)REQ_SHARED:
		case (u16)REQ_NO_RESTRICTION:
			/* Add to existing perms if device is shared or using def policy */
			AperPerm |= APER_PERM(Security, Reqm->AperPerm);
			Status = XST_SUCCESS;
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
		}
	} else {
		switch (Usage) {
		case (u16)REQ_NONSHARED:
		case (u16)REQ_TIME_SHARED:
			/* Disable masters belonging to this subsystem */
			AperPerm &= ~PERM(Reqm->AperPerm);
			Status = XST_SUCCESS;
			break;
		case (u16)REQ_SHARED:
		case (u16)REQ_NO_RESTRICTION:
			/* Get current perms for this shared device from other subsystems */
			DevReqm = Reqm->Device->Requirements;
			while (NULL != DevReqm) {
				if  (1U == DevReqm->Allocated) {
					CurrPerms |= DevReqm->AperPerm;
				}
				DevReqm = DevReqm->NextSubsystem;
			}
			/* Only disable masters belonging to this subsystem */
			AperPerm = (AperPerm & ~PERM(Reqm->AperPerm)) | CurrPerms;
			Status = XST_SUCCESS;
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
		}
	}

	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Add default permissions mask to desired aperture permission */
	AperPerm |= (Ppu->AperPermInitMask & PERM_MASK);

	/* Run the dynamic reconfiguration sequence */
	XPmProt_XppuDynReconfig(Ppu, PermCheckAddr, PermCheckMask, AperIdx,
				AperAddr, AperPerm);

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Configure XPPU according to access control policies from
 *         given subsystem requirement for the peripheral device
 *
 * @param  Reqm: peripheral device requirement
 *               imposed by caller subsystem
 * @param  AddrToProt: Address to protect, typically base address of the device
 * @param  Enable: enable(1)/disable(0) masters in permission ram
 *
 * @return XST_SUCCESS if successful else appropriate failure code
 *
 * @note  This function is called from device request-release infrastructure;
 *        with the requirement specified from the subsystem requesting
 *        the device.
 *        Dynamic reconfiguration of 512m region is only supported for pmc_xppu;
 *        because it is mapped to QSPI/OSPI linear memory address range.
 *        There are no devices modeled from lpd_xppu's 512m region. In addition,
 *        this region overlaps with other regions from all other xppu instances.
 *        Due to the nature of its complexities and no requirement to support it,
 *        its support will be added in future on a need basis.
 *
 ****************************************************************************/
XStatus XPmProt_PpuControl(const XPm_Requirement *Reqm,
			   const u32 AddrToProt,
			   const u32 Enable)
{
	XStatus Status = XST_FAILURE;
	const XPm_ProtPpu *Ppu = NULL;
	u32 SubsystemId = Reqm->Subsystem->Id;
	u32 AperAddr = 0U;
	u32 AperIdx = 0U;
	u32 PpuBase = 0U, i;

	/*
	 * For PMC/Default Subsystem with default requirements,
	 * do not re-configure any apertures since:
	 *   - PMC is a part of default permission mask
	 *   - Default subsystem does not support dynamic protection
	 */
	if (((u32)PM_SUBSYS_PMC == SubsystemId) ||
	    ((u32)PM_SUBSYS_DEFAULT == SubsystemId)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Find PPU and desired aperture */
	for (i = XPM_NODEIDX_XPPU_MIN; i <= XPM_NODEIDX_XPPU_MAX; i++) {
		u32 Offset = 0U;

		Ppu = (XPm_ProtPpu *)XPmProt_GetByIndex(i);
		if ((NULL == Ppu) ||
		    ((u8)XPM_PROT_ENABLED != Ppu->Node.State)) {
			continue;
		}
		PpuBase = Ppu->Node.BaseAddress;

		/* 64k */
		if ((AddrToProt >= Ppu->A64k.Start) &&
		    (AddrToProt <= Ppu->A64k.End)) {
			Offset = ADDR_TO_OFFSET(AddrToProt, Ppu->A64k.Start, SIZE_64K);
			AperAddr = APERADDR_64K(PpuBase, Offset);
			AperIdx = OFFSET_TO_APERIDX_64K(Offset);
		/* 1m */
		} else if ((AddrToProt >= Ppu->A1m.Start) &&
			   (AddrToProt <= Ppu->A1m.End)) {
			Offset = ADDR_TO_OFFSET(AddrToProt, Ppu->A1m.Start, SIZE_1M);
			AperAddr = APERADDR_1M(PpuBase, Offset);
			AperIdx = OFFSET_TO_APERIDX_1M(Offset);
		/* 512m (512m region is only supported for pmc_xppu) */
		} else if ((i == (u32)XPM_NODEIDX_PROT_XPPU_PMC) &&
			   (AddrToProt >= Ppu->A512m.Start) &&
			   (AddrToProt <= Ppu->A512m.End)) {
			AperAddr = APERADDR_512M(PpuBase);
			AperIdx = APER_512M_START;
		} else {
			/* not found anywhere, continue */
			continue;
		}
		/* found */
		break;
	}

	/* Nothing to protect */
	if ((NULL == Ppu) || (0U == AperAddr)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Derive the permissions for this aperture and reconfigure it */
	Status = XPmProt_PpuPermReconfig(Reqm, Ppu, AperIdx, AperAddr, Enable);

done:
	return Status;
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
 * @brief  Common handler for XPPU init node ops
 *
 * @param  Ppu: Xppu node object reference
 * @param  Func: Xppu InitNode Function
 * @param  Args: Args list
 * @param  NumArgs: total number of args present in Args list
 *
 * @return XST_SUCCESS if successful else appropriate failure code
 *
 ****************************************************************************/
static XStatus XPmProt_XppuInitCtrl(XPm_ProtPpu *Ppu, u32 Func,
				    const u32 *Args,
				    u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	switch (Func) {
	case (u32)FUNC_XPPU_ENABLE:
		if ((NULL == Args) || (NumArgs != 1U)) {
			DbgErr = XPM_INT_ERR_INVALID_ARGS;
			goto done;
		}
		XPmProt_XppuEnable(Ppu, Args[0]);
		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_XPPU_DISABLE:
		XPmProt_XppuDisable(Ppu);
		Status = XST_SUCCESS;
		break;
	case (u32)FUNC_XPPU_RECONFIG:
		if ((NULL == Args) || (NumArgs != 2U)) {
			DbgErr = XPM_INT_ERR_INVALID_ARGS;
			goto done;
		}
		Status = XPmProt_XppuReconfig(Ppu, Args[0], Args[1]);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_XPPU_RECONFIG;
			goto done;
		}
		break;
	default:
		Status = XST_NO_FEATURE;
		DbgErr = XPM_INT_ERR_NO_FEATURE;
		break;
	}

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
	Ppu->Ops = &XPmProt_XppuInitCtrl;

	/* Init apertures */
	(void)memset(&Ppu->A64k, 0, sizeof(Ppu->A64k));
	(void)memset(&Ppu->A1m, 0, sizeof(Ppu->A1m));
	(void)memset(&Ppu->A512m, 0, sizeof(Ppu->A512m));

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
