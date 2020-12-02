/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_dvsec.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ---- ---- -------- ------------------------------
* 1.00	av	19/02/2020	Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#include "xpsmfw_default.h"
#include "psm_global.h"
#include "pmc_global.h"
#include "xpsmfw_dvsec.h"

static u8 PlIntrRcvd = FALSE_VALUE;

static DvsecPcsr DvsecPcsrProtocol[DVSEC_PCSR_PROT_LEN] = {
	{0x644U, 0x00000023U},
	{0x648U, 0x66000000U},
	{0x64cU, 0x00000002U},
	{0x650U, 0x68000000U},
	{0x654U, 0x80000000U},
},
DvsecPcsrPrimary[DVSEC_PCSR_PRIM_LEN] = {
	{0x680U, 0x6A000003U},
	{0x684U, 0x00000002U},
	{0x688U, 0x58000000U},
};

/****************************************************************************/
/**
 * @brief	Handler function for DVSEC Config Reads. This function also
 * invokes DvsecInit on its first call.
 *
 * @param	None
 *
 * @return	None
 *
 * @note	None
 *
 ****************************************************************************/
void XPsmFw_DvsecRead(void)
{
	if (((Xil_In32(CPM_SLCR_BASE + CPM_MISC_IR_STA_OFF)
		& CPM_SLCR_DVSEC_CFG_RD_MASK) == CPM_SLCR_DVSEC_CFG_RD_MASK) &&
	     (PlIntrRcvd == FALSE_VALUE)) {
		static u8 DvsecInit;
		u32 RegNum = 0U;
		u32 Index = 0U;

		if (DvsecInit == 0U) {
			XPsmFw_DvsecInit();
			DvsecInit = 1U;
		}
		RegNum = (((Xil_In32(PCIEA_ATTRIB_0 + PCIE_CFG_ADDR_OFF)) >>
			   PCIE_PDVSEC_REG_NO_SHIFT) << PCIE_PDVSEC_REG_FIX_SHIFT);

		if ((RegNum >= DvsecPcsrPrimary[0].DvsecOff) &&
		    (RegNum <= DvsecPcsrPrimary[DVSEC_PCSR_PRIM_LEN - 1U].DvsecOff)) {
			Index = DVSEC_CALC_IDX(RegNum, DvsecPcsrPrimary);

			Dvsec_Wr_M32(PCIEA_DVSEC_0, RegNum,
				    ~(DVSEC_DEV_ID_MASK << DVSEC_DEV_ID_SHIFT),
				    DvsecPcsrPrimary[Index].Val);
		} else if ((RegNum >= DvsecPcsrProtocol[0].DvsecOff) &&
			 (RegNum <= DvsecPcsrProtocol[DVSEC_PCSR_PROT_LEN - 1U].DvsecOff)) {
			Index = DVSEC_CALC_IDX(RegNum, DvsecPcsrProtocol);
			Dvsec_Wr32(PCIEA_DVSEC_0, RegNum,
				      DvsecPcsrProtocol[Index].Val);
		} else {
			Dvsec_Wr32(PCIEA_DVSEC_0, RegNum, 0U);
		}

		Xil_Out32(PCIEA_ATTRIB_0 + PCIE_CFG_ADDR_OFF, PCIE_CFG_MASK);
		Xil_Out32(CPM_SLCR_BASE + CPM_MISC_IR_STA_OFF,
			  Xil_In32(CPM_SLCR_BASE + CPM_MISC_IR_STA_OFF));
	}
}

/****************************************************************************/
/**
 * @brief	Handler function for DVSEC Config Writes
 *
 * @param	None
 *
 * @return	None
 *
 * @note	None
 *
 ****************************************************************************/
void XPsmFw_DvsecWrite(void)
{
	if (((Xil_In32(CPM_SLCR_BASE + CPM_CORR_IR_STA_OFF) &
		CPM_SLCR_DVSEC_CFG_WR_MASK) == CPM_SLCR_DVSEC_CFG_WR_MASK) &&
	    (PlIntrRcvd == FALSE_VALUE)) {
		Xil_Out32(CPM_SLCR_BASE + CPM_CORR_IR_STA_OFF,
			  Xil_In32(CPM_SLCR_BASE + CPM_CORR_IR_STA_OFF));
		Xil_Out32(PCIEA_ATTRIB_0 + PCIE_CFG_ADDR_OFF, PCIE_CFG_MASK);
	}
}

void XPsmFw_DvsecPLHandler(void)
/****************************************************************************/
/**
 * @brief	Execute PSM->PL handoff on recieving a PL Ready interrupt
 *
 * @param	None
 *
 * @return	None
 *
 * @note	None
 *
 ****************************************************************************/
{
	PlIntrRcvd = TRUE_VALUE;

	/* Enable PL CPM SLCR MISC/CORR interrupts */
	Xil_Out32(CPM_SLCR_BASE + CPM_PL_MISC_IRQ_ENA_OFF,
		  CPM_MISC_IRQ_UNMASK);
	Xil_Out32(CPM_SLCR_BASE + CPM_PL_CORR_IRQ_ENA_OFF,
		  CPM_CORR_IRQ_UNMASK);

	/* Disable PS CPM SLCR MISC/CORR interrupts */
	Xil_Out32(CPM_SLCR_BASE + CPM_PS_MISC_IRQ_DIS_OFF,
			CPM_MISC_IRQ_UNMASK);
	Xil_Out32(CPM_SLCR_BASE + CPM_PS_CORR_IRQ_DIS_OFF,
			CPM_CORR_IRQ_UNMASK);

	/* Disable PSM GIC interrupts */
	XPsmFw_GicP2IrqDisable();
}

/****************************************************************************/
/**
 * @brief	Initializes necessary DVSEC PCSR registers
 *
 * @param	None
 *
 * @return	None
 *
 * @note	None
 *
 ****************************************************************************/
void XPsmFw_DvsecInit(void)
{
	u32 Value;

	Value = Dvsec_Rd32(PCIEA_ATTRIB_0, PCIE_PF0_PDVSEC_VID_OFF);
	DvsecPcsrProtocol[DVSEC_PCSR_PROT_DVSEC_HDR_IDX].Val =
			DvsecPcsrProtocol[DVSEC_PCSR_PROT_DVSEC_HDR_IDX].Val |
			(Value & DVSEC_LOW_16B_MASK);

	Value = Dvsec_Rd32(PCIEA_ATTRIB_0, PCIE_PF0_PCSR_SIZE_OFF);
	DvsecPcsrProtocol[DVSEC_PCSR_PROT_PL_CAPSTAT_IDX].Val =
			DvsecPcsrProtocol[DVSEC_PCSR_PROT_PL_CAPSTAT_IDX].Val |
			(Value & DVSEC_LOW_11B_MASK);
	Value = Dvsec_Rd32(PCIEA_ATTRIB_0, PCIE_PF0_PCR_SIZE_OFF);
	DvsecPcsrProtocol[DVSEC_PCSR_PROT_PL_CNTRL_IDX].Val =
			DvsecPcsrProtocol[DVSEC_PCSR_PROT_PL_CNTRL_IDX].Val |
			(Value & DVSEC_LOW_11B_MASK);
}
