/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
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
#define CPM_SLCR_BASE			0xFCA10000U
#define PCIEA_DVSEC_0			0xFCFB0000U
#define PCIEA_ATTRIB_0			0xFCA50000U
#define PL_INTR_STATUS_REG		0xA8000000U

#define CPM_CORR_IR_STA_OFF		0x300U
#define CPM_MISC_IR_STA_OFF		0x340U

#define PCIE_PF0_PCSR_SIZE_OFF		0xA6CU
#define PCIE_PF0_PCR_SIZE_OFF		0xA74U
#define PCIE_PF0_PDVSEC_VID_OFF		0xA58U
#define PCIE_CFG_ADDR_OFF		0xE78U

#define PCIE_CFG_MASK			0x8000000U
#define CPM_SLCR_DVSEC_CFG_RD_MASK	0x20000U
#define CPM_SLCR_DVSEC_CFG_WR_MASK	0x40000U
#define DVSEC_LOW_16B_MASK		0xFFFFU
#define DVSEC_LOW_11B_MASK		0xFFFU
#define DVSEC_DEV_ID_MASK		0xFFU

#define CPM_PL_MISC_IRQ_ENA_OFF	0x3A8U
#define CPM_PL_CORR_IRQ_ENA_OFF	0x368U

#define CPM_MISC_IRQ_UNMASK	0xA0002U
#define CPM_CORR_IRQ_UNMASK	0x140002U

#define CPM_PS_MISC_IRQ_DIS_OFF	0x34CU
#define CPM_PS_CORR_IRQ_DIS_OFF	0x30CU

#define PCIE_PDVSEC_REG_NO_SHIFT	0x9U
#define PCIE_PDVSEC_REG_FIX_SHIFT	0x2U

#define DVSEC_DEV_ID_SHIFT		0x18U

#define DVSEC_REG_SIZE			0x4U

#define DVSEC_PCSR_PROT_LEN		0x5U
#define DVSEC_PCSR_PRIM_LEN		0x3U

#define DVSEC_PCSR_PRIM_CAP_STAT_IDX 	0x1U
#define DVSEC_PCSR_PROT_DVSEC_HDR_IDX	0x1U
#define DVSEC_PCSR_PROT_PL_CAPSTAT_IDX	0x3U
#define DVSEC_PCSR_PROT_PL_CNTRL_IDX	0x4U

#define Dvsec_Wr_M32(Reg, Off, Mask, Val) \
	Xil_Out32((Reg) + (Off), (Xil_In32((Reg) + (Off)) & (Mask)) | (Val))

#define Dvsec_Wr32(Reg, Off, Val) Xil_Out32((Reg) + (Off), (Val))

#define Dvsec_Rd32(Reg, Off) Xil_In32((Reg) + (Off))

#define DVSEC_CALC_IDX(Reg, DvsecStruct) \
	((Reg) - (DvsecStruct[0].DvsecOff)) / (DVSEC_REG_SIZE)

typedef struct{
	u32 DvsecOff;
	u32 Val;
} DvsecPcsr;

void XPsmFw_DvsecInit(void);

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_DVSEC_H */
