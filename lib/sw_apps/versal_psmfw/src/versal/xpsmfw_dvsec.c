/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
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
CpmParam_t CpmParam;

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
 * @brief	Initializes necessary DVSEC PCSR registers
 *
 * @param	None
 *
 * @return	None
 *
 * @note	None
 *
 ****************************************************************************/
static void XPsmFw_DvsecInit(void)
{
	u32 Value;

	Value = Dvsec_Rd32(CpmParam.PcieaAttrib0, PCIE_PF0_PDVSEC_VID_OFF);
	DvsecPcsrProtocol[DVSEC_PCSR_PROT_DVSEC_HDR_IDX].Val =
			DvsecPcsrProtocol[DVSEC_PCSR_PROT_DVSEC_HDR_IDX].Val |
			(Value & DVSEC_LOW_16B_MASK);

	Value = Dvsec_Rd32(CpmParam.PcieaAttrib0, PCIE_PF0_PCSR_SIZE_OFF);
	DvsecPcsrProtocol[DVSEC_PCSR_PROT_PL_CAPSTAT_IDX].Val =
			DvsecPcsrProtocol[DVSEC_PCSR_PROT_PL_CAPSTAT_IDX].Val |
			(Value & DVSEC_LOW_11B_MASK);
	Value = Dvsec_Rd32(CpmParam.PcieaAttrib0, PCIE_PF0_PCR_SIZE_OFF);
	DvsecPcsrProtocol[DVSEC_PCSR_PROT_PL_CNTRL_IDX].Val =
			DvsecPcsrProtocol[DVSEC_PCSR_PROT_PL_CNTRL_IDX].Val |
			(Value & DVSEC_LOW_11B_MASK);
}

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
	if (((Xil_In32(CpmParam.CpmSlcr + CPM_MISC_IR_STA_OFF)
		& CPM_SLCR_DVSEC_CFG_RD_MASK) == CPM_SLCR_DVSEC_CFG_RD_MASK) &&
	     (PlIntrRcvd == FALSE_VALUE)) {
		static u8 DvsecInit;
		u32 RegNum = 0U;
		u32 Index = 0U;

		if (DvsecInit == 0U) {
			XPsmFw_DvsecInit();
			DvsecInit = 1U;
		}
		RegNum = (((Xil_In32(CpmParam.PcieaAttrib0 + PCIE_CFG_ADDR_OFF)) >>
			   PCIE_PDVSEC_REG_NO_SHIFT) << PCIE_PDVSEC_REG_FIX_SHIFT);

		if ((RegNum >= DvsecPcsrPrimary[0].DvsecOff) &&
		    (RegNum <= DvsecPcsrPrimary[DVSEC_PCSR_PRIM_LEN - 1U].DvsecOff)) {
			Index = DVSEC_CALC_IDX(RegNum, DvsecPcsrPrimary);

			Dvsec_Wr_M32(CpmParam.PcieaDvsec0, RegNum,
				    ~(DVSEC_DEV_ID_MASK << DVSEC_DEV_ID_SHIFT),
				    DvsecPcsrPrimary[Index].Val);
		} else if ((RegNum >= DvsecPcsrProtocol[0].DvsecOff) &&
			 (RegNum <= DvsecPcsrProtocol[DVSEC_PCSR_PROT_LEN - 1U].DvsecOff)) {
			Index = DVSEC_CALC_IDX(RegNum, DvsecPcsrProtocol);
			Dvsec_Wr32(CpmParam.PcieaDvsec0, RegNum,
				      DvsecPcsrProtocol[Index].Val);
		} else {
			Dvsec_Wr32(CpmParam.PcieaDvsec0, RegNum, 0U);
		}

		Xil_Out32(CpmParam.PcieaAttrib0 + PCIE_CFG_ADDR_OFF, PCIE_CFG_MASK);
		Xil_Out32(CpmParam.CpmSlcr + CPM_MISC_IR_STA_OFF,
			  Xil_In32(CpmParam.CpmSlcr + CPM_MISC_IR_STA_OFF));
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
	if (((Xil_In32(CpmParam.CpmSlcr + CPM_CORR_IR_STA_OFF) &
		CPM_SLCR_DVSEC_CFG_WR_MASK) == CPM_SLCR_DVSEC_CFG_WR_MASK) &&
	    (PlIntrRcvd == FALSE_VALUE)) {
		Xil_Out32(CpmParam.CpmSlcr + CPM_CORR_IR_STA_OFF,
			  Xil_In32(CpmParam.CpmSlcr + CPM_CORR_IR_STA_OFF));
		Xil_Out32(CpmParam.PcieaAttrib0 + PCIE_CFG_ADDR_OFF, PCIE_CFG_MASK);
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
	Xil_Out32(CpmParam.CpmSlcr + CPM_PL_MISC_IRQ_ENA_OFF,
		  CPM_MISC_IRQ_UNMASK);
	Xil_Out32(CpmParam.CpmSlcr + CPM_PL_CORR_IRQ_ENA_OFF,
		  CPM_CORR_IRQ_UNMASK);

	/* Disable PS CPM SLCR MISC/CORR interrupts */
	Xil_Out32(CpmParam.CpmSlcr + CPM_PS_MISC_IRQ_DIS_OFF,
			CPM_MISC_IRQ_UNMASK);
	Xil_Out32(CpmParam.CpmSlcr + CPM_PS_CORR_IRQ_DIS_OFF,
			CPM_CORR_IRQ_UNMASK);

	/* Disable PSM GIC interrupts */
	XPsmFw_GicP2IrqDisable();
}

/****************************************************************************/
/**
 * @brief	Initializes necessary CPM5 DVSEC PCSR registers
 *
 * @param	None
 *
 * @return	None
 *
 * @note	None
 *
 ****************************************************************************/
static void XPsmFw_Cpm5DvsecInit(void)
{
	u32 Value;

	Value = Dvsec_Rd32(CpmParam.PcieaAttrib0, CPM5_PCIE_PF0_PDVSEC_VID_OFF);
	DvsecPcsrProtocol[DVSEC_PCSR_PROT_DVSEC_HDR_IDX].Val =
			DvsecPcsrProtocol[DVSEC_PCSR_PROT_DVSEC_HDR_IDX].Val |
			(Value & DVSEC_LOW_16B_MASK);

	Value = Dvsec_Rd32(CpmParam.PcieaAttrib0, CPM5_PCIE_PF0_PCSR_SIZE_OFF);
	DvsecPcsrProtocol[DVSEC_PCSR_PROT_PL_CAPSTAT_IDX].Val =
			DvsecPcsrProtocol[DVSEC_PCSR_PROT_PL_CAPSTAT_IDX].Val |
			(Value & DVSEC_LOW_11B_MASK);

	Value = Dvsec_Rd32(CpmParam.PcieaAttrib0, CPM5_PCIE_PF0_PCR_SIZE_OFF);
	DvsecPcsrProtocol[DVSEC_PCSR_PROT_PL_CNTRL_IDX].Val =
			DvsecPcsrProtocol[DVSEC_PCSR_PROT_PL_CNTRL_IDX].Val |
			(Value & DVSEC_LOW_11B_MASK);
}

/****************************************************************************/
/**
 * @brief	CPM5 DVSEC Config Read handling. This function also invokes
 * DvsecInit on its first call.
 *
 * @param	None
 *
 * @return	None
 *
 * @note	None
 *
 ****************************************************************************/
static void XPsmFw_Cpm5DvsecRead(void)
{
	static u8 Cpm5DvsecInit;
	u32 RegNum = 0U;
	u32 Index = 0U;

	if (Cpm5DvsecInit == 0U) {
		XPsmFw_Cpm5DvsecInit();
		Cpm5DvsecInit = 1U;
	}
	RegNum = ((((Xil_In32(CpmParam.PcieCsr0 + CPM5_CFG_MGMT_DVSEC_OFF)) &
		    CPM5_PCIE_PDVSEC_REG_MASK ) >> CPM5_PCIE_PDVSEC_REG_NO_SHIFT)
		  << CPM5_PCIE_PDVSEC_REG_FIX_SHIFT);

	if ((RegNum >= DvsecPcsrPrimary[0].DvsecOff) &&
	    (RegNum <= DvsecPcsrPrimary[DVSEC_PCSR_PRIM_LEN - 1U].DvsecOff)) {
		Index = DVSEC_CALC_IDX(RegNum, DvsecPcsrPrimary);

		Dvsec_Wr_M32(CpmParam.PcieaDvsec0, RegNum,
			    ~(DVSEC_DEV_ID_MASK << DVSEC_DEV_ID_SHIFT),
			    DvsecPcsrPrimary[Index].Val);
	} else if ((RegNum >= DvsecPcsrProtocol[0].DvsecOff) &&
		 (RegNum <= DvsecPcsrProtocol[DVSEC_PCSR_PROT_LEN - 1U].DvsecOff)) {
		Index = DVSEC_CALC_IDX(RegNum, DvsecPcsrProtocol);
		Dvsec_Wr32(CpmParam.PcieaDvsec0, RegNum,
			      DvsecPcsrProtocol[Index].Val);
	} else {
		Dvsec_Wr32(CpmParam.PcieaDvsec0, RegNum, 0U);
	}

	/* Acknowledging Status registers */
	Xil_Out32(CpmParam.CpmSlcr + CPM5_MISC_IR_STA_OFF,
		  Xil_In32(CpmParam.CpmSlcr + CPM5_MISC_IR_STA_OFF));
	Xil_Out32(CpmParam.CpmSlcr + CPM5_PCIE0_IR_STA_OFF,
			  Xil_In32(CpmParam.CpmSlcr + CPM5_PCIE0_IR_STA_OFF));

	/* Send Completion */
	Xil_Out32(CpmParam.PcieCsr0 + CPM5_CFG_MGMT_DVSEC_OFF, PCIE_CFG_MASK);
}

/****************************************************************************/
/**
 * @brief	CPM5 DVSEC Config Write handling
 *
 * @param	None
 *
 * @return	None
 *
 * @note	None
 *
 ****************************************************************************/
static void XPsmFw_Cpm5DvsecWrite(void)
{
	/* Acknowledging Status registers */
	Xil_Out32(CpmParam.CpmSlcr + CPM5_CORR_IR_STA_OFF,
		  Xil_In32(CpmParam.CpmSlcr + CPM5_CORR_IR_STA_OFF));
	Xil_Out32(CpmParam.CpmSlcr + CPM5_PCIE0_IR_STA_OFF,
			  Xil_In32(CpmParam.CpmSlcr + CPM5_PCIE0_IR_STA_OFF));

	/* Send Completion */
	Xil_Out32(CpmParam.PcieCsr0 + CPM5_CFG_MGMT_DVSEC_OFF, PCIE_CFG_MASK);
}

/****************************************************************************/
/**
 * @brief	Top handler function for CPM5 DVSEC Config accesses
 *
 * @param	None
 *
 * @return	None
 *
 * @note	None
 *
 ****************************************************************************/
void XPsmFw_Cpm5DvsecHandler(void)
{
	if (((Xil_In32(CpmParam.CpmSlcr + CPM5_PCIE0_IR_STA_OFF) &
		CPM5_SLCR_DVSEC_CFG_RD_MASK) != 0U) &&
	    (PlIntrRcvd == FALSE_VALUE)) {
		XPsmFw_Cpm5DvsecRead();
	}

	if (((Xil_In32(CpmParam.CpmSlcr + CPM5_PCIE0_IR_STA_OFF) &
		CPM5_SLCR_DVSEC_CFG_WR_MASK) != 0U) &&
	    (PlIntrRcvd == FALSE_VALUE)) {
		XPsmFw_Cpm5DvsecWrite();
	}
}

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
void XPsmFw_Cpm5DvsecPLHandler(void)
{
	PlIntrRcvd = TRUE_VALUE;

	/* Enable PL CPM5 DVSEC interrupts */
	Xil_Out32(CpmParam.CpmSlcr + CPM_PL_MISC_IRQ_ENA_OFF,
		  CPM5_MISC_IRQ_UNMASK);
	Xil_Out32(CpmParam.CpmSlcr + CPM_PL_CORR_IRQ_ENA_OFF,
		  CPM5_CORR_IRQ_UNMASK);

	/* Disable PS CPM5 DVSEC interrupts */
	Xil_Out32(CpmParam.CpmSlcr + CPM_PS_MISC_IRQ_DIS_OFF,
			CPM5_MISC_IRQ_UNMASK);
	Xil_Out32(CpmParam.CpmSlcr + CPM_PS_CORR_IRQ_DIS_OFF,
			CPM5_CORR_IRQ_UNMASK);

	/* Disable PSM GIC interrupts */
	XPsmFw_GicP2IrqDisable();
}

/****************************************************************************/
/**
 * @brief	Initialize CPM PowerId and Base address params
 *
 * @param	CpmPowerId	The PowerId of CPM domain
 * @param	CpmSlcrAddr	BaseAddress of CPM_SLCR module
 *
 * @return	XST_SUCCESS or error code
 *
 * @note	None
 *
 ****************************************************************************/
static XStatus XPsmFw_CpmInit(u32 CpmPowerId, u32 CpmSlcrAddr)
{
	XStatus Status = XST_FAILURE;

	Status = Xil_SMemSet(&CpmParam, sizeof(CpmParam), 0, sizeof(CpmParam));
	if (XST_SUCCESS != Status) {
		goto done;
	}

	CpmParam.CpmPowerId = CpmPowerId;
	CpmParam.CpmSlcr = CpmSlcrAddr;

	switch(CpmParam.CpmPowerId) {
	case XPSMFW_POWER_CPM5:
		CpmParam.PcieaDvsec0 = CpmParam.CpmSlcr + CPM5_PCIEA_DVSEC0_SLCR_OFF;
		CpmParam.PcieaAttrib0 = CpmParam.CpmSlcr + CPM5_PCIEA_ATTRIB0_SLCR_OFF;
		CpmParam.PcieCsr0 = CpmParam.CpmSlcr + CPM5_PCIEA_CSR0_SLCR_OFF;
		Status = XST_SUCCESS;
		break;
	case XPSMFW_POWER_CPM:
		CpmParam.PcieaDvsec0 = CpmParam.CpmSlcr + PCIEA_DVSEC0_SLCR_OFF;
		CpmParam.PcieaAttrib0 = CpmParam.CpmSlcr + PCIEA_ATTRIB0_SLCR_OFF;
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Enable DVSEC functionality in PSM
 *
 * @param	CpmPowerId	The PowerId of CPM domain
 * @param	CpmSlcrAddr	BaseAddress of CPM_SLCR module
 *
 * @return	XST_SUCCESS or error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_DvsecEnable(u32 CpmPowerId, u32 CpmSlcrAddr)
{
	XStatus Status = XST_FAILURE;

	/* Initialize CPM power id and base address params */
	Status = XPsmFw_CpmInit(CpmPowerId, CpmSlcrAddr);

	/* Enable PSM GIC interrupts */
	if (Status == XST_SUCCESS) {
		XPsmFw_GicP2IrqEnable();
	}

	return Status;
}
