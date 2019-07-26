/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
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

#define SIZE_64K 	0x10000
#define SIZE_1M		0x100000
#define SIZE_512M	0x20000000
#define APER_64K_START		0
#define APER_64K_END		255
#define APER_1M_START		384
#define APER_1M_END			389
#define APER_512M_START		400
#define APER_512M_END		400
#define MAX_PERM_REGS		13

XStatus XPmProt_Init(XPm_Prot *ProtNode, u32 Id, u32 BaseAddr)
{
	XStatus Status = XST_SUCCESS;
	u32 NodeIndex = NODEINDEX(Id);
	XPm_ProtPpu *PpuNode = (XPm_ProtPpu *)ProtNode;

	if ((NULL == ProtNode) || (XPM_NODEIDX_PROT_MAX < NodeIndex)) {
		Status = XST_FAILURE;
		goto done;
	}

	Status = XPmNode_Init(&ProtNode->Node,
		Id, XPM_PROT_DISABLED, BaseAddr);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	PmProtNodes[NodeIndex] = ProtNode;

	/* Init addresses */
	PpuNode->Aperture_64k.StartAddress = 0;
	PpuNode->Aperture_64k.EndAddress = 0;
	PpuNode->Aperture_1m.StartAddress = 0;
	PpuNode->Aperture_1m.EndAddress = 0;
	PpuNode->Aperture_512m.StartAddress = 0;
	PpuNode->Aperture_512m.EndAddress = 0;

done:
	return Status;
}

static XPm_Prot *XPmProt_GetById(const u32 Id)
{
	XPm_Prot *ProtNode = NULL;

	if ((XPM_NODECLASS_PROTECTION != NODECLASS(Id)) ||
	    (XPM_NODEIDX_PROT_MAX <= NODEINDEX(Id))) {
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
	XStatus Status = XST_SUCCESS;
	u32 NodeIndex = NODEINDEX(NodeId);
	int i = 0;
	XPm_ProtPpu *PpuNode = (XPm_ProtPpu *)XPmProt_GetById(NodeId);
	u32 Address, BaseAddr, RegVal;

	if (PpuNode == NULL) {
		Status = XST_FAILURE;
		goto done;
	}

	BaseAddr = PpuNode->ProtNode.Node.BaseAddress;

	if ((PLATFORM_VERSION_SILICON == Platform) && (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
		/* Disable permission checks for all apertures */
		Address = BaseAddr + XPPU_ENABLE_PERM_CHECK_REG00_OFFSET;
		for (i = 0; i < MAX_PERM_REGS; i++)
		{
			PmOut32(Address, 0x0);
			Address = Address + 0x4;
		}
	}

	/* Initialize all aperture for default value */
	Address = BaseAddr + XPPU_APERTURE_0_OFFSET;
	for (i = APER_64K_START; i <= APER_64K_END; i++) {
			PmRmw32(Address, 0xF80FFFFFU, ApertureInitVal);
			Address = Address + 0x4;
	}
	Address = BaseAddr + XPPU_APERTURE_384_OFFSET;
	for (i = APER_1M_START; i <= APER_1M_END; i++) {
			PmRmw32(Address, 0xF80FFFFFU, ApertureInitVal);
			Address = Address + 0x4;
	}
	Address = BaseAddr + XPPU_APERTURE_400_OFFSET;
	for (i = APER_512M_START; i <= APER_512M_END; i++) {
			PmRmw32(Address, 0xF80FFFFFU, ApertureInitVal);
			Address = Address + 0x4;
	}

	if ((PLATFORM_VERSION_SILICON == Platform) && (PLATFORM_VERSION_SILICON_ES1 == PlatformVersion)) {
		/* Enable permission checks for all apertures */
		Address = BaseAddr + XPPU_ENABLE_PERM_CHECK_REG00_OFFSET;
		for (i = 0; i < MAX_PERM_REGS; i++)
		{
			PmOut32(Address, 0xFFFFFFFF);
			Address = Address + 0x4;
		}
	}

	/* Get Aperture start and end addresses */
	PmIn32(BaseAddr + XPPU_BASE_64KB_OFFSET,RegVal);
	PpuNode->Aperture_64k.StartAddress = RegVal;
	PmIn32(BaseAddr + XPPU_M_APERTURE_64KB_OFFSET,RegVal);
	PpuNode->Aperture_64k.EndAddress = (PpuNode->Aperture_64k.StartAddress + (RegVal * SIZE_64K) - 1) ;

	PmIn32(BaseAddr + XPPU_BASE_1MB_OFFSET,RegVal);
	PpuNode->Aperture_1m.StartAddress = RegVal;
	PmIn32(BaseAddr + XPPU_M_APERTURE_1MB_OFFSET,RegVal);
	PpuNode->Aperture_1m.EndAddress = (PpuNode->Aperture_1m.StartAddress + (RegVal * SIZE_1M) - 1) ;

	PmIn32(BaseAddr + XPPU_BASE_512MB_OFFSET,RegVal);
	PpuNode->Aperture_512m.StartAddress = RegVal;
	PmIn32(BaseAddr + XPPU_M_APERTURE_512MB_OFFSET,RegVal);
	PpuNode->Aperture_512m.EndAddress = (PpuNode->Aperture_512m.StartAddress + (RegVal * SIZE_512M) - 1) ;


	/* Enable Xppu */
	PmRmw32(BaseAddr + XPPU_CTRL_OFFSET, 0x1, 0x1);

	PpuNode->ProtNode.Node.State = XPM_PROT_ENABLED;

done:
	return Status;
}

XStatus XPmProt_XppuDisable(u32 NodeId)
{
	XStatus Status = XST_SUCCESS;
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
			Address = Address + 0x4;
		}
	}

	/* Disable Xppu */
	PmRmw32(PpuNode->ProtNode.Node.BaseAddress + XPPU_CTRL_OFFSET, 0x1, 0x0);

	PpuNode->ProtNode.Node.State = XPM_PROT_DISABLED;

done:
	return Status;
}

static XStatus XPmProt_ConfigureXppu(XPm_Requirement *Reqm, u32 Enable)
{
	XStatus Status = XST_SUCCESS;
	u32 DeviceBaseAddr = Reqm->Device->Node.BaseAddress;
	XPm_ProtPpu *PpuNode = NULL;
	u32 ApertureOffset, ApertureAddress = 0;
	u32 Permissions = 0, i;
	u32 DynamicReconfigAddrOffset, PermissionRegAddress, PermissionRegMask;

        PmDbg("Xppu configure %x\r\n", Enable);
	PmDbg("Device base %x\r\n", DeviceBaseAddr);

	/* Find XPPU */
	for (i = 0; i < XPM_NODEIDX_PROT_MAX; i++)
	{
		if (PmProtNodes[i] != NULL && (XPM_NODESUBCL_PROT_XPPU == NODESUBCLASS(PmProtNodes[i]->Node.Id))) {
			PpuNode = PmProtNodes[i];
			if ((DeviceBaseAddr >= PpuNode->Aperture_64k.StartAddress) && (DeviceBaseAddr <= PpuNode->Aperture_64k.EndAddress)) {
				ApertureOffset =  (DeviceBaseAddr - PpuNode->Aperture_64k.StartAddress) / SIZE_64K;
				ApertureAddress = (PpuNode->ProtNode.Node.BaseAddress + XPPU_APERTURE_0_OFFSET) + (ApertureOffset * 4);
				DynamicReconfigAddrOffset = ApertureOffset;
				PermissionRegAddress = PpuNode->ProtNode.Node.BaseAddress + XPPU_ENABLE_PERM_CHECK_REG00_OFFSET + (((APER_64K_START + ApertureOffset) / 32) * 4);
				PermissionRegMask = 1 << ((APER_64K_START + ApertureOffset) % 32);
			} else if ((DeviceBaseAddr >= PpuNode->Aperture_1m.StartAddress) && (DeviceBaseAddr <= PpuNode->Aperture_1m.EndAddress)) {
				ApertureOffset =  (DeviceBaseAddr - PpuNode->Aperture_1m.StartAddress) / SIZE_1M;
				ApertureAddress = (PpuNode->ProtNode.Node.BaseAddress + XPPU_APERTURE_384_OFFSET) + (ApertureOffset * 4);
				DynamicReconfigAddrOffset = ApertureOffset;
				PermissionRegAddress = PpuNode->ProtNode.Node.BaseAddress + XPPU_ENABLE_PERM_CHECK_REG00_OFFSET + (((APER_1M_START + ApertureOffset) / 32) * 4);
				PermissionRegMask = 1 << ((APER_1M_START + ApertureOffset) % 32);
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
	if ((i==XPM_NODEIDX_PROT_MAX) || (PpuNode==NULL) || (ApertureAddress==0)) {
		goto done;
	}

	/* See if XPPU is enabled or not, if not, return */
	if (XPM_PROT_DISABLED == PpuNode->ProtNode.Node.State)
		goto done;

	PmDbg("Aperoffset %x AperAddress %x DynamicReconfigAddrOffset %x\r\n",ApertureOffset, ApertureAddress, DynamicReconfigAddrOffset);

	if (Enable) {
		u32 UsagePolicy = Reqm->Flags & REG_FLAGS_USAGE_MASK;
		u32 Security = (Reqm->Flags & REG_FLAGS_SECURITY_MASK) >> REG_FLAGS_SECURITY_OFFSET;

		PmIn32(ApertureAddress, Permissions);

		/* Configure XPPU Aperture */
		if ((UsagePolicy == REQ_NONSHARED) || (UsagePolicy == REQ_TIME_SHARED)) {
			Permissions = ((Security << XPPU_APERTURE_TRUSTZONE_OFFSET) | (Reqm->Params[0] & XPPU_APERTURE_PERMISSION_MASK));
		} else if (UsagePolicy == REQ_SHARED) {
			/* if device is shared, permissions need to be ored with existing */
			Permissions |= ((Security << XPPU_APERTURE_TRUSTZONE_OFFSET) | (Reqm->Params[0] & XPPU_APERTURE_PERMISSION_MASK));
		} else if (UsagePolicy == REQ_NO_RESTRICTION) {
			Permissions = (XPPU_APERTURE_PERMISSION_MASK | XPPU_APERTURE_TRUSTZONE_MASK);
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

done:
	return Status;
}

XStatus XPmProt_Configure(XPm_Requirement *Reqm, u32 Enable)
{
	XStatus Status = XST_SUCCESS;

	if (PLATFORM_VERSION_SILICON != Platform) {
		goto done;
	}

	if (XPM_NODESUBCL_DEV_PERIPH == NODESUBCLASS(Reqm->Device->Node.Id)) {
		/* TODO: Some of the FPD peripherals are protected by XMPU. so based on device's baseaddress,
		we need to find whether to configure XMPu/XPPU */
		/* TODO: Need to know which addresse range particular XMPU protects */
		Status = XPmProt_ConfigureXppu(Reqm, Enable);
	} else {
		goto done;
	}

done:
	return Status;
}
