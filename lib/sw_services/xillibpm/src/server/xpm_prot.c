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
