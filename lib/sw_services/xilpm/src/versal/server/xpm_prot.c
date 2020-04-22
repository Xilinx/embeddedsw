/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/



#include "xpm_common.h"
#include "xpm_node.h"
#include "xpm_regs.h"
#include "xpm_subsystem.h"
#include "xpm_device.h"
#include "xpm_prot.h"

static XPm_Prot *PmProtNodes[XPM_NODEIDX_PROT_MAX];

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

static XStatus XPmProt_Init(XPm_Prot *ProtNode, u32 Id, u32 BaseAddr)
{
	XStatus Status = XST_FAILURE;
	u32 NodeIndex = NODEINDEX(Id);

	if ((u32)XPM_NODEIDX_PROT_MAX <= NodeIndex) {
		goto done;
	}

	XPmNode_Init(&ProtNode->Node, Id, (u8)XPM_PROT_DISABLED, BaseAddr);

	PmProtNodes[NodeIndex] = ProtNode;
	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmProtPpu_Init(XPm_ProtPpu *PpuNode, u32 Id, u32 BaseAddr)
{
	XStatus Status = XST_FAILURE;

	Status = XPmProt_Init((XPm_Prot *)PpuNode, Id, BaseAddr);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Parity status bits */
	PpuNode->MIDParityEn = 0;
	PpuNode->AperParityEn = 0;

	/* Init addresses - 64k */
	PpuNode->Aperture_64k.NumSupported = 0;
	PpuNode->Aperture_64k.StartAddress = 0;
	PpuNode->Aperture_64k.EndAddress = 0;

	/* Init addresses - 1m */
	PpuNode->Aperture_1m.NumSupported = 0;
	PpuNode->Aperture_1m.StartAddress = 0;
	PpuNode->Aperture_1m.EndAddress = 0;

	/* Init addresses - 512mb */
	PpuNode->Aperture_512m.NumSupported = 0;
	PpuNode->Aperture_512m.StartAddress = 0;
	PpuNode->Aperture_512m.EndAddress = 0;

done:
	return Status;
}

XStatus XPmProtMpu_Init(XPm_ProtMpu *MpuNode, u32 Id, u32 BaseAddr)
{
	XStatus Status = XST_FAILURE;

	Status = XPmProt_Init((XPm_Prot *)MpuNode, Id, BaseAddr);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* TODO: XMPU Init addresses */

done:
	return Status;
}

static XPm_Prot *XPmProt_GetById(const u32 Id)
{
	XPm_Prot *ProtNode = NULL;

	if (((u32)XPM_NODECLASS_PROTECTION != NODECLASS(Id)) ||
	    ((u32)XPM_NODEIDX_PROT_MAX <= NODEINDEX(Id))) {
		goto done;
	}

	ProtNode = PmProtNodes[NODEINDEX(Id)];
	/* Check that internal ID is same as given ID or not. */
	if ((NULL != ProtNode) && (Id != ProtNode->Node.Id)) {
		ProtNode = NULL;
	}

done:
	return ProtNode;
}

static void XPmProt_XppuSetAperture(const XPm_ProtPpu *PpuNode, u32 AperAddr, u32 AperVal)
{
	u32 i, RegVal, Field, FieldParity, Tz;
	u32 Parity = 0;

	/* Clear parity bits */
	RegVal = AperVal & ~XPPU_APERTURE_PARITY_MASK;

	if (0x0U == PpuNode->AperParityEn) {
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
		if (i == MAX_APER_PARITY_FIELDS - 1U) {
			FieldParity ^= Tz;
		}
		Parity |= (FieldParity << i);
	}

	/* Set parity bits */
	RegVal |= (Parity << XPPU_APERTURE_PARITY_SHIFT);

done:
	PmOut32(AperAddr, RegVal);
}

XStatus XPmProt_XppuEnable(u32 NodeId, u32 ApertureInitVal)
{
	XStatus Status = XST_FAILURE;
	u32 i = 0;
	XPm_ProtPpu *PpuNode = (XPm_ProtPpu *)XPmProt_GetById(NodeId);
	u32 Address, BaseAddr, RegVal;

	if (PpuNode == NULL) {
		goto done;
	}

	/* XPPU Base Address */
	BaseAddr = PpuNode->ProtNode.Node.BaseAddress;

	if ((PLATFORM_VERSION_SILICON == Platform) && (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
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
	PpuNode->Aperture_64k.NumSupported = RegVal;
	Xil_AssertNonvoid((APER_64K_END - APER_64K_START + 1U) == PpuNode->Aperture_64k.NumSupported);

	PmIn32(BaseAddr + XPPU_M_APERTURE_1MB_OFFSET, RegVal);
	PpuNode->Aperture_1m.NumSupported = RegVal;
	Xil_AssertNonvoid((APER_1M_END - APER_1M_START + 1U) == PpuNode->Aperture_1m.NumSupported);

	PmIn32(BaseAddr + XPPU_M_APERTURE_512MB_OFFSET, RegVal);
	PpuNode->Aperture_512m.NumSupported = RegVal;
	Xil_AssertNonvoid((APER_512M_END - APER_512M_START + 1U) == PpuNode->Aperture_512m.NumSupported);

	/* Store parity bits settings */
	PmIn32(BaseAddr + XPPU_CTRL_OFFSET, RegVal);
	PpuNode->MIDParityEn = (u8)((RegVal >> XPPU_CTRL_MID_PARITY_EN_SHIFT) & 0x1U);
	PpuNode->AperParityEn = (u8)((RegVal >> XPPU_CTRL_APER_PARITY_EN_SHIFT) & 0x1U);

	/* Initialize all apertures for default value */
	Address = BaseAddr + XPPU_APERTURE_0_OFFSET;
	for (i = APER_64K_START; i <= APER_64K_END; i++) {
		/**
		 * In Versal, message buffer protection is moved out of XPPU, to IPI.
		 * Therefore, XPPU permissions for IPI specific apertures (aperture 49 to aperture 63)
		 * should be configured to allow all. This is applicable to LPD XPPU only.
		 *
		 * Refer "XPPU protection for IPI" from XPPU Spec
		 */
		if (((u32)XPM_NODEIDX_PROT_XPPU_LPD == NODEINDEX(NodeId))
			&& ((i >= APER_IPI_MIN) && (i <= APER_IPI_MAX))) {
			XPmProt_XppuSetAperture(PpuNode, Address, (ApertureInitVal | XPPU_APERTURE_PERMISSION_MASK));
		} else {
			XPmProt_XppuSetAperture(PpuNode, Address, ApertureInitVal);
		}
		Address = Address + 0x4U;
	}
	Address = BaseAddr + XPPU_APERTURE_384_OFFSET;
	for (i = APER_1M_START; i <= APER_1M_END; i++) {
		XPmProt_XppuSetAperture(PpuNode, Address, ApertureInitVal);
		Address = Address + 0x4U;
	}
	Address = BaseAddr + XPPU_APERTURE_400_OFFSET;
	for (i = APER_512M_START; i <= APER_512M_END; i++) {
		XPmProt_XppuSetAperture(PpuNode, Address, ApertureInitVal);
		Address = Address + 0x4U;
	}

	if ((PLATFORM_VERSION_SILICON == Platform) && (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
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
	PpuNode->Aperture_64k.StartAddress = RegVal;
	PpuNode->Aperture_64k.EndAddress =
		(PpuNode->Aperture_64k.StartAddress + (PpuNode->Aperture_64k.NumSupported * SIZE_64K) - 1U);

	PmIn32(BaseAddr + XPPU_BASE_1MB_OFFSET, RegVal);
	PpuNode->Aperture_1m.StartAddress = RegVal;
	PpuNode->Aperture_1m.EndAddress =
		(PpuNode->Aperture_1m.StartAddress + (PpuNode->Aperture_1m.NumSupported * SIZE_1M) - 1U);

	PmIn32(BaseAddr + XPPU_BASE_512MB_OFFSET, RegVal);
	PpuNode->Aperture_512m.StartAddress = RegVal;
	PpuNode->Aperture_512m.EndAddress =
		(PpuNode->Aperture_512m.StartAddress + (PpuNode->Aperture_512m.NumSupported * SIZE_512M) - 1U);

	/* Enable Xppu */
	PmRmw32(BaseAddr + XPPU_CTRL_OFFSET, XPPU_CTRL_ENABLE_MASK, XPPU_CTRL_ENABLE_MASK);

	PpuNode->ProtNode.Node.State = (u8)XPM_PROT_ENABLED;

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmProt_XppuDisable(u32 NodeId)
{
	XStatus Status = XST_FAILURE;
	XPm_ProtPpu *PpuNode = (XPm_ProtPpu *)XPmProt_GetById(NodeId);
	u32 Address, idx;

	if (PpuNode == NULL) {
		Status = XST_FAILURE;
		goto done;
	}

	if ((PLATFORM_VERSION_SILICON == Platform) && (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
		/* Disable permission checks for all apertures */
		Address = PpuNode->ProtNode.Node.BaseAddress + XPPU_ENABLE_PERM_CHECK_REG00_OFFSET;
		for (idx = 0; idx < MAX_PERM_REGS; idx++)
		{
			PmOut32(Address, 0x0);
			Address = Address + 0x4U;
		}
	}

	/* Disable Xppu */
	PmRmw32(PpuNode->ProtNode.Node.BaseAddress + XPPU_CTRL_OFFSET,
			XPPU_CTRL_ENABLE_MASK, ~XPPU_CTRL_ENABLE_MASK);

	PpuNode->ProtNode.Node.State = (u8)XPM_PROT_DISABLED;

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPmProt_ConfigureXppu(XPm_Requirement *Reqm, u32 Enable)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceBaseAddr = Reqm->Device->Node.BaseAddress;
	XPm_ProtPpu *PpuNode = NULL;
	u32 ApertureOffset = 0, ApertureAddress = 0;
	u32 Permissions = 0, i;
	u32 DynamicReconfigAddrOffset = 0;
	u32	PermissionRegAddress = 0;
	u32	PermissionRegMask = 0;

        PmDbg("Xppu configure %x\r\n", Enable);
	PmDbg("Device base %x\r\n", DeviceBaseAddr);

	/* Find XPPU */
	for (i = 0; i < (u32)XPM_NODEIDX_PROT_MAX; i++)
	{
		if ((PmProtNodes[i] != NULL)
		&& ((u32)XPM_NODESUBCL_PROT_XPPU == NODESUBCLASS(PmProtNodes[i]->Node.Id))) {
			PpuNode = (XPm_ProtPpu *)PmProtNodes[i];
			if ((DeviceBaseAddr >= PpuNode->Aperture_64k.StartAddress) && (DeviceBaseAddr <= PpuNode->Aperture_64k.EndAddress)) {
				ApertureOffset =  (DeviceBaseAddr - PpuNode->Aperture_64k.StartAddress) / SIZE_64K;
				ApertureAddress = (PpuNode->ProtNode.Node.BaseAddress + XPPU_APERTURE_0_OFFSET) + (ApertureOffset * 4U);
				DynamicReconfigAddrOffset = ApertureOffset;
				PermissionRegAddress = PpuNode->ProtNode.Node.BaseAddress + XPPU_ENABLE_PERM_CHECK_REG00_OFFSET + (((APER_64K_START + ApertureOffset) / 32U) * 4U);
				PermissionRegMask = (u32)1U << ((APER_64K_START + ApertureOffset) % 32U);
			} else if ((DeviceBaseAddr >= PpuNode->Aperture_1m.StartAddress) && (DeviceBaseAddr <= PpuNode->Aperture_1m.EndAddress)) {
				ApertureOffset =  (DeviceBaseAddr - PpuNode->Aperture_1m.StartAddress) / SIZE_1M;
				ApertureAddress = (PpuNode->ProtNode.Node.BaseAddress + XPPU_APERTURE_384_OFFSET) + (ApertureOffset * 4U);
				DynamicReconfigAddrOffset = ApertureOffset;
				PermissionRegAddress = PpuNode->ProtNode.Node.BaseAddress + XPPU_ENABLE_PERM_CHECK_REG00_OFFSET + (((APER_1M_START + ApertureOffset) / 32U) * 4U);
				PermissionRegMask = (u32)1U << ((APER_1M_START + ApertureOffset) % 32U);
			/*TODO: 512M start and end address need to be validated */
			/*} else if ((DeviceBaseAddr >= PpuNode->Aperture_512m.StartAddress) && (DeviceBaseAddr <= PpuNode->Aperture_512m.EndAddress)) {
				ApertureOffset =  (DeviceBaseAddr - PpuNode->Aperture_512m.StartAddress) / SIZE_512M;
				ApertureAddress = (PpuNode->ProtNode.Node.BaseAddress + XPPU_APERTURE_400_OFFSET) + (ApertureOffset * 4);
				DynamicReconfigAddrOffset = APER_512M_START;
				PermissionRegAddress = PpuNode->ProtNode.Node.BaseAddress + XPPU_ENABLE_PERM_CHECK_REG00_OFFSET + (((APER_512M_START + ApertureOffset) / 32) * 4);
				PermissionRegMask = 1 << ((APER_512M_START + ApertureOffset) % 32); */
			} else {
				continue;
			}
			break;
		}
	}
	if ((i == (u32)XPM_NODEIDX_PROT_MAX) || (NULL == PpuNode) || (0U == ApertureAddress)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* See if XPPU is enabled or not, if not, return */
	if ((u8)XPM_PROT_DISABLED == PpuNode->ProtNode.Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	PmDbg("Aperoffset %x AperAddress %x DynamicReconfigAddrOffset %x\r\n",ApertureOffset, ApertureAddress, DynamicReconfigAddrOffset);

	if (0U != Enable) {
		u8 UsagePolicy = Reqm->Flags & REG_FLAGS_USAGE_MASK;
		u32 Security = (Reqm->Flags & (u32)REG_FLAGS_SECURITY_MASK) >> REG_FLAGS_SECURITY_OFFSET;

		PmIn32(ApertureAddress, Permissions);

		/* Configure XPPU Aperture */
		if ((UsagePolicy == (u8)REQ_NONSHARED) || (UsagePolicy == (u8)REQ_TIME_SHARED)) {
			Permissions = ((Security << XPPU_APERTURE_TRUSTZONE_OFFSET) | (Reqm->Params[0] & XPPU_APERTURE_PERMISSION_MASK));
		} else if (UsagePolicy == (u8)REQ_SHARED) {
			/* if device is shared, permissions need to be ored with existing */
			Permissions |= ((Security << XPPU_APERTURE_TRUSTZONE_OFFSET) | (Reqm->Params[0] & XPPU_APERTURE_PERMISSION_MASK));
		} else if (UsagePolicy == (u8)REQ_NO_RESTRICTION) {
			Permissions = (XPPU_APERTURE_PERMISSION_MASK | XPPU_APERTURE_TRUSTZONE_MASK);
		} else {
			/* Required due to MISRA */
			PmDbg("Invalid UsagePolicy %d\r\n", UsagePolicy);
		}
	} else {
		/* Configure XPPU to disable masters belonging to this subsystem */
		Permissions = (Permissions | XPPU_APERTURE_TRUSTZONE_MASK);
		Permissions = (Permissions & (~(Reqm->Params[0] & XPPU_APERTURE_PERMISSION_MASK)));
	}

	PmDbg("PermissionRegAddress %x Permissions %x RegMask %x \r\n",PermissionRegAddress, Permissions, PermissionRegMask);

	if ((PLATFORM_VERSION_SILICON == Platform) && (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
		/* Set XPPU control to 0 */
		PmRmw32(PpuNode->ProtNode.Node.BaseAddress + XPPU_CTRL_OFFSET,
				XPPU_CTRL_ENABLE_MASK, ~XPPU_CTRL_ENABLE_MASK);

		/* Set Enable Permission check of the required apertures to 0 */
		PmRmw32(PermissionRegAddress, PermissionRegMask, 0);

		/* Program permissions of the apertures that need to be �reconfigured� */
		XPmProt_XppuSetAperture(PpuNode, ApertureAddress, Permissions);

		/* Enable back permission check of the apertures */
		PmRmw32(PermissionRegAddress, PermissionRegMask, PermissionRegMask);

		/* Set XPPU control to 1 */
		PmRmw32(PpuNode->ProtNode.Node.BaseAddress + XPPU_CTRL_OFFSET,
				XPPU_CTRL_ENABLE_MASK, XPPU_CTRL_ENABLE_MASK);

	} else {
		/* Configure Dynamic reconfig enable registers before changing XPPU config */
		PmOut32(PpuNode->ProtNode.Node.BaseAddress + XPPU_DYNAMIC_RECONFIG_APER_ADDR_OFFSET, DynamicReconfigAddrOffset);
		PmOut32(PpuNode->ProtNode.Node.BaseAddress + XPPU_DYNAMIC_RECONFIG_APER_PERM_OFFSET, Permissions);
		PmOut32(PpuNode->ProtNode.Node.BaseAddress + XPPU_DYNAMIC_RECONFIG_EN_OFFSET, 1);

		/* Write values to Aperture */
		XPmProt_XppuSetAperture(PpuNode, ApertureAddress, Permissions);

		/* Disable dynamic reconfig enable once done */
		PmOut32(PpuNode->ProtNode.Node.BaseAddress + XPPU_DYNAMIC_RECONFIG_EN_OFFSET, 0);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmProt_Configure(XPm_Requirement *Reqm, u32 Enable)
{
	XStatus Status = XST_FAILURE;

	if (PLATFORM_VERSION_SILICON != Platform) {
		Status = XST_SUCCESS;
		goto done;
	}

	if ((u32)XPM_NODESUBCL_DEV_PERIPH == NODESUBCLASS(Reqm->Device->Node.Id)) {
		/* TODO: Some of the FPD peripherals are protected by XMPU. so based on device's baseaddress,
		we need to find whether to configure XMPu/XPPU */
		/* TODO: Need to know which addresse range particular XMPU protects */
		Status = XPmProt_ConfigureXppu(Reqm, Enable);
	} else {
		Status = XST_SUCCESS;
		goto done;
	}

done:
	return Status;
}
