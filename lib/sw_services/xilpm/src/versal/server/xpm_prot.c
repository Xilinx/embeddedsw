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
	Mpu->Ops = NULL;

done:
	return Status;
}
