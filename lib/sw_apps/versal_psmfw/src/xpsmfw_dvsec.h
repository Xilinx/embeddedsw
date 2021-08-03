/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_dvsec.h
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ---- ---- -------- ------------------------------
* 1.00	av	19/2/2020	Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_DVSEC_H_
#define XPSMFW_DVSEC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpsmfw_dvsec_common.h"

#define PCIEA_DVSEC0_SLCR_OFF		0x5A0000U
#define PCIEA_ATTRIB0_SLCR_OFF		0x40000U

#define CPM5_PCIEA_DVSEC0_SLCR_OFF	0x70000U
#define CPM5_PCIEA_ATTRIB0_SLCR_OFF	0x38000U
#define CPM5_PCIEA_CSR0_SLCR_OFF	0x30000U

#define CPM_CORR_IR_STA_OFF		0x300U
#define CPM_MISC_IR_STA_OFF		0x340U

#define CPM5_CORR_IR_STA_OFF		0x360U
#define CPM5_MISC_IR_STA_OFF		0x3A0U
#define CPM5_PCIE0_IR_STA_OFF		0x2A0U

#define PCIE_PF0_PCSR_SIZE_OFF		0xA6CU
#define PCIE_PF0_PCR_SIZE_OFF		0xA74U
#define PCIE_PF0_PDVSEC_VID_OFF		0xA58U
#define PCIE_CFG_ADDR_OFF		0xE78U

#define CPM5_PCIE_PF0_PCSR_SIZE_OFF	0x1E34U
#define CPM5_PCIE_PF0_PCR_SIZE_OFF	0x1E3CU
#define CPM5_PCIE_PF0_PDVSEC_VID_OFF	0x1E20U

#define CPM5_CFG_MGMT_DVSEC_OFF		0x200U

#define PCIE_CFG_MASK			(u32)(0x08000000U)
#define CPM_SLCR_DVSEC_CFG_RD_MASK	(u32)(0x00020000U)
#define CPM_SLCR_DVSEC_CFG_WR_MASK	(u32)(0x00040000U)
#define DVSEC_LOW_16B_MASK		(u32)(0x0000FFFFU)
#define DVSEC_LOW_11B_MASK		(u32)(0x00000FFFU)
#define DVSEC_DEV_ID_MASK		(u32)(0x000000FFU)

#define CPM_PL_MISC_IRQ_ENA_OFF	0x3A8U
#define CPM_PL_CORR_IRQ_ENA_OFF	0x368U

#define CPM_MISC_IRQ_UNMASK	0xA0002U
#define CPM_CORR_IRQ_UNMASK	0x140002U

#define CPM5_MISC_IRQ_UNMASK	0x2U
#define CPM5_CORR_IRQ_UNMASK	0x2U

#define CPM_PS_MISC_IRQ_DIS_OFF	0x34CU
#define CPM_PS_CORR_IRQ_DIS_OFF	0x30CU

#define CPM5_PCIE_PDVSEC_REG_MASK	0x7FEU
#define CPM5_SLCR_DVSEC_CFG_RD_MASK	0x10U
#define CPM5_SLCR_DVSEC_CFG_WR_MASK	0x20U

#define PCIE_PDVSEC_REG_NO_SHIFT	0x9U
#define PCIE_PDVSEC_REG_FIX_SHIFT	0x2U
#define CPM5_PCIE_PDVSEC_REG_NO_SHIFT	0x1U
#define CPM5_PCIE_PDVSEC_REG_FIX_SHIFT	0x2U

#define DVSEC_DEV_ID_SHIFT		0x18U

#define DVSEC_REG_SIZE			0x4U

#define DVSEC_PCSR_PROT_LEN		0x5U
#define DVSEC_PCSR_PRIM_LEN		0x3U

#define DVSEC_PCSR_PRIM_CAP_STAT_IDX 	0x1U
#define DVSEC_PCSR_PROT_DVSEC_HDR_IDX	0x1U
#define DVSEC_PCSR_PROT_PL_CAPSTAT_IDX	0x3U
#define DVSEC_PCSR_PROT_PL_CNTRL_IDX	0x4U

/**
Register write after masking original value of the register with specified mask.

@param Reg	Register base address
@param Off	Register address offset
@param Mask	Mask for the value
@param Val	Value to be written
*/
#define Dvsec_Wr_M32(Reg, Off, Mask, Val) \
	Xil_Out32((Reg) + (Off), (Xil_In32((Reg) + (Off)) & (Mask)) | (Val))

/**
Register write operation.

@param Reg	Register base address
@param Off	Register address offset
@param Val	Value to be written
*/
#define Dvsec_Wr32(Reg, Off, Val) Xil_Out32((Reg) + (Off), (Val))

/**
Register read operation.

@param Reg	Register base address
@param Off	Register address offset
*/
#define Dvsec_Rd32(Reg, Off) Xil_In32((Reg) + (Off))

/**
Calculate index of register from the specified DVSEC struct

@param Reg		Register offset
@param DvsecStruct	Dvsec Structure
*/
#define DVSEC_CALC_IDX(Reg, DvsecStruct) \
	((Reg) - (DvsecStruct[0].DvsecOff)) / (DVSEC_REG_SIZE)

typedef struct{
	u32 DvsecOff;
	u32 Val;
} DvsecPcsr;

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_DVSEC_H */
