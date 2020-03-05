/******************************************************************************
*
* Copyright (C) 2019-2020 Xilinx, Inc.  All rights reserved.
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
#define APER_64K_START				(0U)
#define APER_64K_END				(255U)
#define APER_1M_START				(384U)
#define APER_1M_END				(399U)
#define APER_512M_START				(400U)
#define APER_512M_END				(400U)
#define MAX_PERM_REGS				(13U)

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

	/* TODO: XMPU Init adddresses */

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
			PmOut32(Address, 0x0);
			Address = Address + 0x4U;
		}
	}

	/* Get Number of Apertures supported */
	PmIn32(BaseAddr + XPPU_M_APERTURE_64KB_OFFSET, RegVal);
	PpuNode->Aperture_64k.NumSupported = RegVal;
	Xil_AssertNonvoid((APER_64K_END - APER_64K_START + 1) == PpuNode->Aperture_64k.NumSupported);

	PmIn32(BaseAddr + XPPU_M_APERTURE_1MB_OFFSET, RegVal);
	PpuNode->Aperture_1m.NumSupported = RegVal;
	Xil_AssertNonvoid((APER_1M_END - APER_1M_START + 1) == PpuNode->Aperture_1m.NumSupported);

	PmIn32(BaseAddr + XPPU_M_APERTURE_512MB_OFFSET, RegVal);
	PpuNode->Aperture_512m.NumSupported = RegVal;
	Xil_AssertNonvoid((APER_512M_END - APER_512M_START + 1) == PpuNode->Aperture_512m.NumSupported);

	/* Initialize all apertures for default value */
	Address = BaseAddr + XPPU_APERTURE_0_OFFSET;
	for (i = APER_64K_START; i <= APER_64K_END; i++) {
			PmRmw32(Address, 0xF80FFFFFU, ApertureInitVal);
			Address = Address + 0x4U;
	}
	Address = BaseAddr + XPPU_APERTURE_384_OFFSET;
	for (i = APER_1M_START; i <= APER_1M_END; i++) {
			PmRmw32(Address, 0xF80FFFFFU, ApertureInitVal);
			Address = Address + 0x4U;
	}
	Address = BaseAddr + XPPU_APERTURE_400_OFFSET;
	for (i = APER_512M_START; i <= APER_512M_END; i++) {
			PmRmw32(Address, 0xF80FFFFFU, ApertureInitVal);
			Address = Address + 0x4U;
	}

	if ((PLATFORM_VERSION_SILICON == Platform) && (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
		/* Enable permission checks for all apertures */
		Address = BaseAddr + XPPU_ENABLE_PERM_CHECK_REG00_OFFSET;
		for (i = 0; i < MAX_PERM_REGS; i++)
		{
			PmOut32(Address, 0xFFFFFFFF);
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
	PmRmw32(BaseAddr + XPPU_CTRL_OFFSET, 0x1, 0x1);

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
	PmRmw32(PpuNode->ProtNode.Node.BaseAddress + XPPU_CTRL_OFFSET, 0x1, 0x0);

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
	u32 ApertureOffset, ApertureAddress = 0;
	u32 Permissions = 0, i;
	u32 DynamicReconfigAddrOffset, PermissionRegAddress, PermissionRegMask;

        PmDbg("Xppu configure %x\r\n", Enable);
	PmDbg("Device base %x\r\n", DeviceBaseAddr);

	/* Find XPPU */
	for (i = 0; i < (u32)XPM_NODEIDX_PROT_MAX; i++)
	{
		if (PmProtNodes[i] != NULL && ((u32)XPM_NODESUBCL_PROT_XPPU == NODESUBCLASS(PmProtNodes[i]->Node.Id))) {
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
		PmRmw32(PpuNode->ProtNode.Node.BaseAddress + XPPU_CTRL_OFFSET, 0x1, 0x0);

		/* Set Enable Permission check of the required apertures to 0 */
		PmRmw32(PermissionRegAddress, PermissionRegMask, 0);

		/* Program permissions of the apertures that need to be �reconfigured� */
		PmOut32(ApertureAddress, Permissions);

		/* Enable back permission check of the apertures */
		PmRmw32(PermissionRegAddress, PermissionRegMask, PermissionRegMask);

		/* Set XPPU control to 1 */
		PmRmw32(PpuNode->ProtNode.Node.BaseAddress + XPPU_CTRL_OFFSET, 0x1, 0x1);

	} else {
		/* Configure Dynamic reconfig enable registers before changing XPPU config */
		PmOut32(PpuNode->ProtNode.Node.BaseAddress + XPPU_DYNAMIC_RECONFIG_APER_ADDR_OFFSET, DynamicReconfigAddrOffset);
		PmOut32(PpuNode->ProtNode.Node.BaseAddress + XPPU_DYNAMIC_RECONFIG_APER_PERM_OFFSET, Permissions);
		PmOut32(PpuNode->ProtNode.Node.BaseAddress + XPPU_DYNAMIC_RECONFIG_EN_OFFSET, 1);

		/* Write values to Aperture */
		PmOut32(ApertureAddress, Permissions);

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
