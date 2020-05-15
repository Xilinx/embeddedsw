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
#include <math.h>

static DvsecReg DvsecCcid[DVSEC_CCID_LEN] = {
	{0x9E0U, 0xF2E8CA31U, 0xFFFFFFFFU},
	{0x9E4U, 0x9B68D271U, 0xFFFFFFFFU},
	{0x9E8U, 0x02C4436FU, 0xFFFFFFFFU},
	{0x9ECU, 0xC3CB99CBU, 0xFFFFFFFFU},
	{0x9F0U, 0x0U, 0xFFFF000FU},
},
DvsecPcrPrimary[DVSEC_PCR_PRIM_LEN] = {
	{0x800U, 0x82000003U, 0xFFF0FFFFU},
	{0x804U, 0x0U, 0xFF3F01F7U},
	{0x808U, 0x0U, 0xFFF0407AU},
	{0x80cU, 0x0U, 0xFFFFFFFFU},
	{0x810U, 0x0U, 0xFFFFFFFFU},
	{0x814U, 0x0U, 0xFFFFFFFFU},
},
DvsecPcrPort[DVSEC_PCR_PORT_LEN] = {
	{0x820U, 0x8B000004U, 0xFFF0FFFFU},
	{0x824U, 0x0U, 0x7FF8BU},
	{0x828U, 0x0U, 0xFFFFFFFFU},
	{0x82cU, 0x0U, 0xFFFFU},
	{0x830U, 0x0U, 0xFFFFU},
	{0x834U, 0x0U, 0xFE01U},
	{0x838U, 0x0U, 0xFFFFFFFFU},
	{0x83cU, 0x0U, 0xFFFFFFFFU},
	{0x840U, 0x0U, 0xFU},
	{0x844U, 0x0U, 0xFFFFFFFFU},
	{0x848U, 0x0U, 0xFFFFFFFFU}
},
DvsecPcrLink[DVSEC_PCR_LINK_LEN] = {
	{0x8B0U, 0x92000005U, 0xFFF0FFFFU},
	{0x8B4U, 0x0U, 0x7CFU},
	{0x8B8U, 0x0U, 0x3FFFFFFFU},
	{0x8BCU, 0x0U, 0x3FFFFFFFU},
	{0x8C0U, 0x0U, 0xFFFFFU},
	{0x8C4U, 0x0U, 0xFFFFFFFFU},
	{0x8C8U, 0x0U, 0xFFFFFFFFU},
	{0x8CCU, 0x0U, 0xFFFFFFFFU},
},
DvsecPcrRa0[DVSEC_PCR_RA_LEN] = {
	{0x920U, 0x94000008U, 0xFFF0FFFFU},
	{0x924U, 0x0U, 0xFC06C00BU},
	{0x928U, 0x0U, 0xFFFFFFFFU},
	{0x92CU, 0x0U, 0xFFFFFFFFU},
},
DvsecPcrHa0[DVSEC_PCR_HA_LEN] = {
	{0x940U, 0x00000006U, 0xFFF0FFFFU},
	{0x944U, 0x0U, 0x057F01U},
	{0x948U, 0x0U, 0xFFFFFFFFU},
	{0x94CU, 0x0U, 0xFFFFFFFFU},
	{0x950U, 0x0U, 0xFFFFFFFFU},
	{0x954U, 0x0U, 0xFFFFFFFFU},
	{0x958U, 0x0U, 0x3F3F3F3FU},
	{0x95CU, 0x0U, 0x7U},
	{0x960U, 0x0U, 0xFFFFFFFFU},
	{0x964U, 0x0U, 0x7U},
	{0x968U, 0x0U, 0xFFFFFFFFU},
	{0x96CU, 0x0U, 0x7U},
	{0x970U, 0x0U, 0xFFFFFFFFU},
	{0x974U, 0x0U, 0x7U},
	{0x978U, 0x0U, 0xFFFFFFFFU},

},
DvsecPcrSa0[DVSEC_PCR_SA_LEN] = {
	{0x940U, 0x00000006U, 0xFFF0FFFFU},
	{0x944U, 0x0U, 0xFFFFFFFFU},
	{0x948U, 0x0U, 0xFFFFFFFFU},
	{0x94CU, 0x0U, 0xFFFFFFFFU},
	{0x950U, 0x0U, 0xFFFFFFFFU},
	{0x954U, 0x0U, 0x7U},
	{0x958U, 0x0U, 0xFFFFFFFFU},
	{0x95CU, 0x0U, 0x7U},
	{0x960U, 0x0U, 0xFFFFFFFFU},
	{0x964U, 0x0U, 0x7U},
	{0x968U, 0x0U, 0xFFFFFFFFU},
	{0x96CU, 0x0U, 0x7U},
	{0x970U, 0x0U, 0xFFFFFFFFU},

},
DvsecIdm[DVSEC_NUM_IDM_ENTRIES] = {
	{0xC00U, 0x0U, 0x1FBCF3U},
	{0xC04U, 0x0U, 0x1FBCF3U},
	{0xC08U, 0x0U, 0x1FBCF3U},
	{0xC0CU, 0x0U, 0x1FBCF3U},
	{0xC10U, 0x0U, 0x1FBCF3U},
	{0xC14U, 0x0U, 0x1FBCF3U},
	{0xC18U, 0x0U, 0x1FBCF3U},
	{0xC1CU, 0x0U, 0x1FBCF3U},
	{0xC20U, 0x0U, 0x1FBCF3U},
	{0xC24U, 0x0U, 0x1FBCF3U},
	{0xC28U, 0x0U, 0x1FBCF3U},
	{0xC2CU, 0x0U, 0x1FBCF3U},
	{0xC30U, 0x0U, 0x1FBCF3U},
	{0xC34U, 0x0U, 0x1FBCF3U},
	{0xC38U, 0x0U, 0x1FBCF3U},
	{0xC3CU, 0x0U, 0x1FBCF3U},
	{0xC40U, 0x0U, 0x1FBCF3U},
	{0xC44U, 0x0U, 0x1FBCF3U},
	{0xC48U, 0x0U, 0x1FBCF3U},
	{0xC4CU, 0x0U, 0x1FBCF3U},
	{0xC50U, 0x0U, 0x1FBCF3U},
	{0xC54U, 0x0U, 0x1FBCF3U},
	{0xC58U, 0x0U, 0x1FBCF3U},
	{0xC5CU, 0x0U, 0x1FBCF3U},
	{0xC60U, 0x0U, 0x1FBCF3U},
	{0xC64U, 0x0U, 0x1FBCF3U},
	{0xC68U, 0x0U, 0x1FBCF3U},
	{0xC6CU, 0x0U, 0x1FBCF3U},
	{0xC70U, 0x0U, 0x1FBCF3U},
	{0xC74U, 0x0U, 0x1FBCF3U},
	{0xC78U, 0x0U, 0x1FBCF3U},
	{0xC7CU, 0x0U, 0x1FBCF3U},
	{0xC80U, 0x0U, 0x1FBCF3U},
	{0xC84U, 0x0U, 0x1FBCF3U},
	{0xC88U, 0x0U, 0x1FBCF3U},
	{0xC8CU, 0x0U, 0x1FBCF3U},
	{0xC90U, 0x0U, 0x1FBCF3U},
	{0xC94U, 0x0U, 0x1FBCF3U},
	{0xC98U, 0x0U, 0x1FBCF3U},
	{0xC9CU, 0x0U, 0x1FBCF3U},
	{0xCA0U, 0x0U, 0x1FBCF3U},
	{0xCA4U, 0x0U, 0x1FBCF3U},
	{0xCA8U, 0x0U, 0x1FBCF3U},
	{0xCACU, 0x0U, 0x1FBCF3U},
	{0xCB0U, 0x0U, 0x1FBCF3U},
	{0xCB4U, 0x0U, 0x1FBCF3U},
	{0xCB8U, 0x0U, 0x1FBCF3U},
	{0xCBCU, 0x0U, 0x1FBCF3U},
	{0xCC0U, 0x0U, 0x1FBCF3U},
	{0xCC4U, 0x0U, 0x1FBCF3U},
	{0xCC8U, 0x0U, 0x1FBCF3U},
	{0xCCCU, 0x0U, 0x1FBCF3U},
	{0xCD0U, 0x0U, 0x1FBCF3U},
	{0xCD4U, 0x0U, 0x1FBCF3U},
	{0xCD8U, 0x0U, 0x1FBCF3U},
	{0xCDCU, 0x0U, 0x1FBCF3U},
	{0xCE0U, 0x0U, 0x1FBCF3U},
	{0xCE4U, 0x0U, 0x1FBCF3U},
	{0xCE8U, 0x0U, 0x1FBCF3U},
	{0xCECU, 0x0U, 0x1FBCF3U},
	{0xCF0U, 0x0U, 0x1FBCF3U},
	{0xCF4U, 0x0U, 0x1FBCF3U},
	{0xCF8U, 0x0U, 0x1FBCF3U},
	{0xCFCU, 0x0U, 0x1FBCF3U},
};

static struct DvsecSamEntryMap {
	DvsecReg DvsecPcrSam[DVSEC_SAM_LEN];
} DvsecSamEntry[DVSEC_NUM_SAM_ENTRIES] = {
	{{{0xA00U, 0x0U, 0x3CF3U}, {0xA04U, 0x0U, 0xFFFFFFFFU}, {0xA08U, 0x0U, 0xFFFFFFFFU} }},
	{{{0xA0cU, 0x0U, 0x3CF3U}, {0xA10U, 0x0U, 0xFFFFFFFFU}, {0xA14U, 0x0U, 0xFFFFFFFFU} }},
	{{{0xA18U, 0x0U, 0x3CF3U}, {0xA1cU, 0x0U, 0xFFFFFFFFU}, {0xA20U, 0x0U, 0xFFFFFFFFU} }},
	{{{0xA24U, 0x0U, 0x3CF3U}, {0xA28U, 0x0U, 0xFFFFFFFFU}, {0xA2cU, 0x0U, 0xFFFFFFFFU} }},
	{{{0xA30U, 0x0U, 0x3CF3U}, {0xA34U, 0x0U, 0xFFFFFFFFU}, {0xA38U, 0x0U, 0xFFFFFFFFU} }},
	{{{0xA3cU, 0x0U, 0x3CF3U}, {0xA40U, 0x0U, 0xFFFFFFFFU}, {0xA44U, 0x0U, 0xFFFFFFFFU} }},
	{{{0xA48U, 0x0U, 0x3CF3U}, {0xA4cU, 0x0U, 0xFFFFFFFFU}, {0xA50U, 0x0U, 0xFFFFFFFFU} }},
	{{{0xA54U, 0x0U, 0x3CF3U}, {0xA58U, 0x0U, 0xFFFFFFFFU}, {0xA5cU, 0x0U, 0xFFFFFFFFU} }},
};

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
	{0x688U, 0x1FF80001U},
	{0x68cU, 0x00000000U},
	{0x690U, 0xD0000000U},
	{0x694U, 0xC0000000U},
	{0x698U, 0xA0000020U},
	{0x69CU, 0xD0000080U},
},
DvsecPcsrPort[DVSEC_PCSR_PORT_LEN] = {
	{0x6A0U, 0x6B400004U},
	{0x6A4U, 0x4083U},
	{0x6A8U, 0x0U},
	{0x6ACU, 0x00000000U},
	{0x6B0U, 0xD2000000U},
},
DvsecPcsrLink[DVSEC_PCSR_LINK_LEN] = {
	{0x6B4U, 0x6D000005U},
	{0x6B8U, 0x5U},
	{0x6BCU, 0x1004010U},
	{0x6C0U, 0x00000000U},
	{0x6C4U, 0x00000000U},
	{0x6C8U, 0x00000000U},
},
DvsecPcsrRa0[DVSEC_PCSR_RA_LEN] = {
	{0x6D0U, 0x6F000008U},
	{0x6D4U, 0x00000000U},
	{0x6D8U, 0x00000000U},
},
DvsecPcsrHa0[DVSEC_PCSR_HA_LEN] = {
	{0x6F0U, 0x00000006U},
	{0x6F4U, 0x80000103U},
	{0x6F8U, 0x9A000000U},
	{0x6FCU, 0xFFFF0001U},
	{0x700U, 0x00000001U},
	{0x704U, 0x00000000U},
	{0x708U, 0x00000000U},
	{0x70CU, 0x00000000U},
	{0x710U, 0x00000000U},
	{0x714U, 0x00000000U},
	{0x718U, 0x00000000U},
};

XStatus XPsmFw_DvsecRead(void)
{
	static u8 dvsec_init;
	u32 RegNum = 0U;
	u32 Index = 0U;

	if ((Xil_In32(CPM_SLCR_BASE + CPM_MISC_IR_STA_OFF)
		& CPM_SLCR_DVSEC_CFG_RD_MASK) == 0U) {
		return XST_SUCCESS;
	}

	if (dvsec_init == 0U) {
		XPsmFw_DvsecInit();
		dvsec_init = 1U;
	}

	RegNum = (((Xil_In32(PCIEA_ATTRIB_0 + PCIE_CFG_ADDR_OFF)) >>
		   PCIE_PDVSEC_REG_NO_SHIFT) << PCIE_PDVSEC_REG_FIX_SHIFT);

	if ((RegNum >= DvsecPcsrPrimary[0].DvsecOff) &&
	    (RegNum <= DvsecPcsrPrimary[DVSEC_PCSR_PRIM_LEN - 1].DvsecOff)) {
		Index = DVSEC_CALC_IDX(RegNum, DvsecPcsrPrimary);
		/*Update DID Value from primary_port_ctl to primary_port*/
		if (Index == DVSEC_PCSR_PRIM_CAP_STAT_IDX) {
			DvsecPcsrPrimary[Index].Val =
			  DvsecPcsrPrimary[Index].Val |
			  (Dvsec_Rd32(PCIEA_DVSEC_0,
				      DvsecPcrPrimary[Index].DvsecOff) &
				      (u32)(DVSEC_DEV_ID_MASK << DVSEC_DEV_ID_SHIFT));
		}
		Dvsec_Wr_M32(PCIEA_DVSEC_0, RegNum,
			    ~(DVSEC_DEV_ID_MASK << DVSEC_DEV_ID_SHIFT),
			    DvsecPcsrPrimary[Index].Val);
	} else if ((RegNum >= DvsecPcsrProtocol[0].DvsecOff) &&
		 (RegNum <= DvsecPcsrProtocol[DVSEC_PCSR_PROT_LEN - 1].DvsecOff)) {
		Index = DVSEC_CALC_IDX(RegNum, DvsecPcsrProtocol);
		Dvsec_Wr32(PCIEA_DVSEC_0, RegNum,
			      DvsecPcsrProtocol[Index].Val);
	} else if ((RegNum >= DvsecPcsrPort[0].DvsecOff) &&
		 (RegNum <= DvsecPcsrPort[DVSEC_PCSR_PORT_LEN - 1].DvsecOff)) {
		Index = DVSEC_CALC_IDX(RegNum, DvsecPcsrPort);
		Dvsec_Wr32(PCIEA_DVSEC_0, RegNum, DvsecPcsrPort[Index].Val);
	} else if ((RegNum >= DvsecPcsrLink[0].DvsecOff) &&
		 (RegNum <= DvsecPcsrLink[DVSEC_PCSR_LINK_LEN - 1].DvsecOff)) {
		Index = DVSEC_CALC_IDX(RegNum, DvsecPcsrLink);
		Dvsec_Wr32(PCIEA_DVSEC_0, RegNum, DvsecPcsrLink[Index].Val);
	} else if ((RegNum >= DvsecPcsrRa0[0].DvsecOff) &&
		 (RegNum <= DvsecPcsrRa0[DVSEC_PCSR_RA_LEN - 1].DvsecOff)) {
		Index = DVSEC_CALC_IDX(RegNum, DvsecPcsrRa0);
		Dvsec_Wr32(PCIEA_DVSEC_0, RegNum, DvsecPcsrRa0[Index].Val);
	} else if ((RegNum >= DvsecPcsrHa0[0].DvsecOff) &&
		 (RegNum <= DvsecPcsrHa0[DVSEC_PCSR_HA_LEN - 1].DvsecOff)) {
		Index = DVSEC_CALC_IDX(RegNum, DvsecPcsrHa0);
		Dvsec_Wr32(PCIEA_DVSEC_0, RegNum, DvsecPcsrHa0[Index].Val);
	} else if ((RegNum >= DvsecCcid[0].DvsecOff) &&
		 (RegNum <= DvsecCcid[DVSEC_CCID_LEN - 1].DvsecOff)) {
		Index = DVSEC_CALC_IDX(RegNum, DvsecCcid);
		Dvsec_Wr32(PCIEA_DVSEC_0, RegNum, DvsecCcid[Index].Val);
	} else if (RegNum == DvsecPcrPrimary[0].DvsecOff) {
		Dvsec_Wr32(PCIEA_DVSEC_0, RegNum, DvsecPcrPrimary[0].Val);
	} else if (RegNum == DvsecPcrPort[0].DvsecOff) {
		Dvsec_Wr32(PCIEA_DVSEC_0, RegNum, DvsecPcrPort[0].Val);
	} else if (RegNum == DvsecPcrLink[0].DvsecOff) {
		Dvsec_Wr32(PCIEA_DVSEC_0, RegNum, DvsecPcrLink[0].Val);
	} else if (RegNum == DvsecPcrRa0[0].DvsecOff) {
		Dvsec_Wr32(PCIEA_DVSEC_0, RegNum, DvsecPcrRa0[0].Val);
	} else if (RegNum == DvsecPcrHa0[0].DvsecOff) {
		Dvsec_Wr32(PCIEA_DVSEC_0, RegNum, DvsecPcrHa0[0].Val);
	} else if (((RegNum >= DvsecPcrPrimary[1].DvsecOff) &&
		    (RegNum <= DvsecPcrPrimary[DVSEC_PCR_PRIM_LEN - 1].DvsecOff)) ||
		   ((RegNum >= DvsecPcrPort[1].DvsecOff) &&
		    (RegNum <= DvsecPcrPort[DVSEC_PCR_PORT_LEN - 1].DvsecOff)) ||
		   ((RegNum >= DvsecPcrLink[1].DvsecOff) &&
		    (RegNum <= DvsecPcrLink[DVSEC_PCR_LINK_LEN - 1].DvsecOff)) ||
		   ((RegNum >= DvsecPcrRa0[1].DvsecOff) &&
		    (RegNum <= DvsecPcrRa0[DVSEC_PCR_RA_LEN - 1].DvsecOff)) ||
		   ((RegNum >= DvsecPcrHa0[1].DvsecOff) &&
		    (RegNum <= DvsecPcrHa0[DVSEC_PCR_HA_LEN - 1].DvsecOff)) ||
		   ((RegNum >= DvsecPcrSa0[1].DvsecOff) &&
		    (RegNum <= DvsecPcrSa0[DVSEC_PCR_SA_LEN - 1].DvsecOff)) ||
		   ((RegNum >= DvsecSamEntry[DVSEC_SAM_0_IDX].DvsecPcrSam[0].DvsecOff) &&
		    (RegNum <= DvsecSamEntry[DVSEC_SAM_0_IDX].DvsecPcrSam[DVSEC_SAM_LEN - 1].DvsecOff)) ||
		   ((RegNum >= DvsecSamEntry[DVSEC_SAM_1_IDX].DvsecPcrSam[0].DvsecOff) &&
		    (RegNum <= DvsecSamEntry[DVSEC_SAM_1_IDX].DvsecPcrSam[DVSEC_SAM_LEN - 1].DvsecOff)) ||
		   ((RegNum >= DvsecIdm[0].DvsecOff) &&
		    (RegNum <= DvsecIdm[DVSEC_NUM_IDM_ENTRIES - 1].DvsecOff)) ||
		   ((RegNum >= DvsecCcid[0].DvsecOff) &&
		    (RegNum <= DvsecCcid[DVSEC_CCID_LEN - 1].DvsecOff))) {
			goto done;
	} else {
		Dvsec_Wr32(PCIEA_DVSEC_0, RegNum, 0U);
	}

done:
	Xil_Out32(PCIEA_ATTRIB_0 + PCIE_CFG_ADDR_OFF, PCIE_CFG_MASK);
	Xil_Out32(CPM_SLCR_BASE + CPM_MISC_IR_STA_OFF,
		  Xil_In32(CPM_SLCR_BASE + CPM_MISC_IR_STA_OFF));

	return XST_SUCCESS;
}

void XPsmFW_DvsecSetVal(u32 Off, u32 Val)
{
	u32 Reg1Val = 0U;
	u32 Reg2Val = 0U;
	u32 StartAddr;
	u32 EndAddr;
	u32 IdmPid;
	u32 SamVal;
	u32 SamPid;
	u32 IdmVal;
	u32 TgtId;
	u32 i;
	u32 j;

	if (Off == DvsecCcid[DVSEC_CCID_OVERRIDE_4_IDX].DvsecOff) {
		u32 Vid;

		Vid = (Val & DVSEC_VENDOR_ID_MASK) >> DVSEC_VENDOR_ID_SHIFT;
		if (Vid == 0U) {
			return;
		}
		Dvsec_Wr_M32(PCIEA_ATTRIB_0, DVSEC_TRANSPORT_HDR_OFF,
			     ~DVSEC_LOW_16B_MASK, Vid);
		Dvsec_Wr_M32(PCIEA_ATTRIB_0, DVSEC_PROTOCOL_HDR_OFF,
			     ~DVSEC_LOW_16B_MASK, Vid);
		Dvsec_Wr_M32(CMN_N72_LA_BASE_REG, CMN_LA_TLP_HDR_2_OFF,
			     ~DVSEC_LOW_16B_MASK, Vid);
	}

	if (Off == DvsecPcrPrimary[DVSEC_PCR_PRIM_COMMON_CTL2_IDX].DvsecOff) {
		u32 RegVal;
		RegVal =
		  (((Val >> DVSEC_PARTIAL_CACHE_SHIFT) &
		    DVSEC_PARTIAL_CACHE_MASK) << CMN_PARTIAL_CACHE_SHIFT)     |
		  (((Val >> DVSEC_CACHE_LINE_SIZE_SHIFT) &
		    DVSEC_CACHE_LINE_SIZE_MASK) << CMN_CACHE_LINE_SIZE_SHIFT) |
		  (((Val >> DVSEC_ADDR_WIDTH_EN_SHIFT) &
		    DVSEC_ADDR_WIDTH_EN_MASK) << CMN_ADDR_WIDTH_EN_SHIFT);

		Dvsec_Wr_M32(CMN_N72_LA_BASE_REG, CMN_LA_CCIX_PROP_OFF,
			     ~CMN_PARTIAL_CACHE_ADDR_MASK, RegVal);
	}

	if (Off == DvsecPcrPrimary[DVSEC_PCR_PRIM_COMMON_CTL1_IDX].DvsecOff) {
		if ((SingleHA == 1) || (SingleSA == 1) || (HARA == 1)) {
			Dvsec_Wr_M32(CMN_N72_BASE_REG, CMN_RNSAM_STATUS_OFF,
				     ~CMN_RSAM_STATUS_MASK,
				     CMN_RSAM_UNSTALL_MASK);
		}
		if ((SingleRA == 1) || (HARA == 1)) {
			Dvsec_Wr_M32(CMN_N76_BASE_REG, CMN_RNSAM_STATUS_OFF,
				     ~CMN_RSAM_STATUS_MASK,
				     CMN_RSAM_UNSTALL_MASK);
		}
	}

	if (((Val & DVSEC_IDM_VALID_MASK) == 0U) ||
	    ((Val & DVSEC_RSAM_VALID_MASK) == 0U)) {
		return;
	}

	for (i = 0U; i < DVSEC_NUM_SAM_ENTRIES; i++) {
		SamVal = Dvsec_Rd32(PCIEA_DVSEC_0,
			 DvsecSamEntry[i].DvsecPcrSam[DVSEC_PCR_SAM_START_IDX].DvsecOff)
			 & DvsecSamEntry[i].DvsecPcrSam[DVSEC_PCR_SAM_START_IDX].Mask;
		if ((SamVal & DVSEC_SAM_IDM_EN_MASK) == 0U) {
			continue;
		}
		SamPid = (SamVal & DVSEC_SAM_PID_MASK) >> DVSEC_SAM_PID_SHIFT;
		for (j = 0U; j < DVSEC_NUM_IDM_ENTRIES; j++) {
			IdmVal = Dvsec_Rd32(PCIEA_DVSEC_0,
					    DvsecIdm[j].DvsecOff);
			IdmPid = (IdmVal & DVSEC_SAM_PID_MASK) >>
				  DVSEC_SAM_PID_SHIFT;
			if ((IdmPid != SamPid) ||
			    ((IdmVal & DVSEC_SAM_IDM_EN_MASK) == 0U)) {
				continue;
			}

			StartAddr = Dvsec_Rd32(PCIEA_DVSEC_0,
				    DvsecSamEntry[i].DvsecPcrSam[DVSEC_SAM_SADDR_IDX].DvsecOff)
				    & DvsecSamEntry[i].DvsecPcrSam[DVSEC_SAM_SADDR_IDX].Mask;
			EndAddr = Dvsec_Rd32(PCIEA_DVSEC_0,
				  DvsecSamEntry[i].DvsecPcrSam[DVSEC_SAM_EADDR_IDX].DvsecOff)
				  & DvsecSamEntry[i].DvsecPcrSam[DVSEC_SAM_EADDR_IDX].Mask;

			Reg1Val = CMN_SAM_LEN(EndAddr, StartAddr);

			if (((IdmVal & DVSEC_SAM_DEST_MASK) == 0U) &&
			    ((SamVal & DVSEC_SAM_DEST_MASK) == DVSEC_SAM_DEST_MASK)) {
				TgtId = CMN_TGT_HAID_0;
			 } else {
				/* Assigning IDM entry num as a target ID */
				TgtId = j;
			}

			Reg2Val = StartAddr				|
				  (TgtId << CMN_RSAM_REGN_TID_SHIFT)	|
				  CMN_RSAM_REGION_VALID_MASK;

			Dvsec_Wr32(CMN_N72_RA_BASE_REG,
			   CMN_RASAM_REG0_1_OFF + (i * CMN_64B_REG_SIZE),
			   Reg1Val);
			Dvsec_Wr32(CMN_N72_RA_BASE_REG,
			   CMN_RASAM_REG0_2_OFF + (i * CMN_64B_REG_SIZE),
			   Reg2Val);
		}
	}
}

static u32 XPsmFW_DvsecCalcAperH(u32 Addr, u32 Size)
{
	u32 AperMask = 0U;
	u32 Pos = 0U;
	u32 i = 0U;

	/* Calculate position of MSB, set all 1's till MSB */
	while (Addr != 0U) {
		Addr >>= 1U;
		Pos++;
	}

	while (i < Pos) {
		AperMask |= (u32)1U << i;
		i++;
	}

	return ((~Size) & AperMask);
}

static void XPsmFW_DvsecPgmMP(u32 SrcAddr, u32 ARSrcHOff, u32 MPBaseHOff,
			      u32 MPBaseLOff, u32 MPSizeOff)
{
	u32 MPSizeH = 0U;
	u32 MPSizeL = 0U;
	u32 AperH = 0U;
	u32 AperL = 0U;
	u32 Len;

	Dvsec_Wr32(CPM_ADDR_REMAP, ARSrcHOff, SrcAddr);
	Dvsec_Wr32(CPM_ADDR_REMAP, ARSrcHOff + CPM_AR_DST_ADDR_H_OFF,
		   Dvsec_Rd32(PSM_DVSEC_RAM, MPBaseHOff));
	Dvsec_Wr32(CPM_ADDR_REMAP, ARSrcHOff + CPM_AR_DST_ADDR_L_OFF,
		   Dvsec_Rd32(PSM_DVSEC_RAM, MPBaseLOff));

	MPSizeH = (Dvsec_Rd32(PSM_DVSEC_RAM, MPSizeOff) & DVSEC_UP_16B_MASK)
		   >> DVSEC_SHIFT_16BIT;
	MPSizeL = ((Dvsec_Rd32(PSM_DVSEC_RAM, MPSizeOff) & DVSEC_LOW_16B_MASK)
		   << DVSEC_SHIFT_16BIT) | DVSEC_LOW_16B_MASK;

	AperH = XPsmFW_DvsecCalcAperH((SrcAddr + MPSizeH), MPSizeH);
	AperL = ~MPSizeL;

	Dvsec_Wr32(CPM_ADDR_REMAP, ARSrcHOff + CPM_AR_APER_SIZE_H_OFF, AperH);
	Dvsec_Wr32(CPM_ADDR_REMAP, ARSrcHOff + CPM_AR_APER_SIZE_L_OFF, AperL);

	Len = CMN_MEMORY_LEN((SrcAddr + MPSizeH + 1U), SrcAddr);
	Dvsec_Wr_M32(CMN_N72_BASE_REG, CMN_NH_MEM_REGION1_OFF,
		     ~(CMN_NH_MEM_REGION_MASK),
		     (SrcAddr << CMN_NH_MEM_BASE_ADDR_SHIFT) |
		     CMN_NODE_TYPE_HNF_MASK		     |
		     (Len << CMN_NH_MEM_LEN_SHIFT)	     |
		     CMN_REG_VALID_MASK);
}

static void XPsmFw_DvsecSetupMem(u32 SrcAddr)
{
	u32 mem_en;

	mem_en = Dvsec_Rd32(PSM_DVSEC_RAM, PSM_DVSEC_MP_PROP0_OFF) &
		 PSM_DVSEC_MEM_REG_EN_MASK;
	if ((mem_en & PSM_DVSEC_MPSEL_0_MASK) != 0U) {
		XPsmFW_DvsecPgmMP(SrcAddr, CPM_AR_SRC_ADDR0_H_OFF,
		  PSM_DVSEC_MP_BASE_ADDR0_H_OFF, PSM_DVSEC_MP_BASE_ADDR0_L_OFF,
		  PSM_DVSEC_MP_SIZE0_OFF);
	} else if ((mem_en & PSM_DVSEC_MPSEL_1_MASK) != 0U) {
		XPsmFW_DvsecPgmMP(SrcAddr, CPM_AR_SRC_ADDR1_H_OFF,
		  PSM_DVSEC_MP_BASE_ADDR1_H_OFF, PSM_DVSEC_MP_BASE_ADDR1_L_OFF,
		  PSM_DVSEC_MP_SIZE1_OFF);
	} else if ((mem_en & PSM_DVSEC_MPSEL_2_MASK) != 0U) {
		XPsmFW_DvsecPgmMP(SrcAddr, CPM_AR_SRC_ADDR2_H_OFF,
		  PSM_DVSEC_MP_BASE_ADDR2_H_OFF, PSM_DVSEC_MP_BASE_ADDR2_L_OFF,
		  PSM_DVSEC_MP_SIZE2_OFF);
	} else if ((mem_en & PSM_DVSEC_MPSEL_3_MASK) != 0U) {
		XPsmFW_DvsecPgmMP(SrcAddr, CPM_AR_SRC_ADDR3_H_OFF,
		  PSM_DVSEC_MP_BASE_ADDR3_H_OFF, PSM_DVSEC_MP_BASE_ADDR3_L_OFF,
		  PSM_DVSEC_MP_SIZE3_OFF);
	} else if ((mem_en & PSM_DVSEC_MPSEL_4_MASK) != 0U) {
		XPsmFW_DvsecPgmMP(SrcAddr, CPM_AR_SRC_ADDR4_H_OFF,
		  PSM_DVSEC_MP_BASE_ADDR4_H_OFF, PSM_DVSEC_MP_BASE_ADDR4_L_OFF,
		  PSM_DVSEC_MP_SIZE4_OFF);
	} else if ((mem_en & PSM_DVSEC_MPSEL_5_MASK) != 0U) {
		XPsmFW_DvsecPgmMP(SrcAddr, CPM_AR_SRC_ADDR5_H_OFF,
		  PSM_DVSEC_MP_BASE_ADDR5_H_OFF, PSM_DVSEC_MP_BASE_ADDR5_L_OFF,
		  PSM_DVSEC_MP_SIZE5_OFF);
	} else if ((mem_en & PSM_DVSEC_MPSEL_6_MASK) != 0U) {
		XPsmFW_DvsecPgmMP(SrcAddr, CPM_AR_SRC_ADDR6_H_OFF,
		  PSM_DVSEC_MP_BASE_ADDR6_H_OFF, PSM_DVSEC_MP_BASE_ADDR6_L_OFF,
		  PSM_DVSEC_MP_SIZE6_OFF);
	} else if ((mem_en & PSM_DVSEC_MPSEL_7_MASK) != 0U) {
		XPsmFW_DvsecPgmMP(SrcAddr, CPM_AR_SRC_ADDR7_H_OFF,
		  PSM_DVSEC_MP_BASE_ADDR7_H_OFF, PSM_DVSEC_MP_BASE_ADDR7_L_OFF,
		  PSM_DVSEC_MP_SIZE7_OFF);
	} else {
		XPsmFW_DvsecPgmMP(SrcAddr, CPM_AR_SRC_ADDR0_H_OFF,
		  PSM_DVSEC_MP_BASE_ADDR0_H_OFF, PSM_DVSEC_MP_BASE_ADDR0_L_OFF,
		  PSM_DVSEC_MP_SIZE0_OFF);
	}

	Dvsec_Wr32(CPM_ADDR_REMAP, CPM_AR_ADDRREMAP_CTL_OFF,
		   CMN_APER_SIZE0_EN_MASK);
}

XStatus XPsmFw_DvsecWrite(void)
{
	u32 RegNum = 0U;
	u32 Index = 0U;
	u8 Check = 0U;
	u32 Val = 0U;

	if ((Xil_In32(CPM_SLCR_BASE + CPM_CORR_IR_STA_OFF) &
		CPM_SLCR_DVSEC_CFG_WR_MASK) == 0U) {
		return XST_SUCCESS;
	}

	RegNum = (((Xil_In32(PCIEA_ATTRIB_0 + PCIE_CFG_ADDR_OFF)) >>
		   PCIE_PDVSEC_REG_NO_SHIFT) << PCIE_PDVSEC_REG_FIX_SHIFT);

	if ((RegNum >= DvsecPcrPrimary[0].DvsecOff) &&
	    (RegNum <= DvsecPcrPrimary[DVSEC_PCR_PRIM_LEN - 1].DvsecOff)) {
		Check = 1U;
		DVSEC_CALC_IDX_VAL(Index, Val, RegNum, DvsecPcrPrimary);
		if (Index != DVSEC_PCR_PRIM_START_IDX) {
			XPsmFW_DvsecSetVal(RegNum, Val);
		}
	} else if ((RegNum >= DvsecPcrPort[0].DvsecOff) &&
		   (RegNum <= DvsecPcrPort[DVSEC_PCR_PORT_LEN - 1].DvsecOff)) {
		Check = 1U;
		DVSEC_CALC_IDX_VAL(Index, Val, RegNum, DvsecPcrPort);

		/* Updating Packet Header in the cxla_ccix_prop 6th Bit */
		if (Index == DVSEC_PCR_PORT_CTL_IDX) {
			if (((Val >> DVSEC_PKT_HEADER_SHIFT) &
			    DVSEC_PKT_HEADER_MASK) != 0U) {
				Dvsec_Wr_M32(CMN_N72_LA_BASE_REG,
					     CMN_LA_CCIX_PROP_OFF,
					     ~(CMN_LA_PKT_HDR_VAL_MASK),
					     (CMN_LA_PKT_HDR_VAL_MASK));
			} else {
				Dvsec_Wr_M32(CMN_N72_LA_BASE_REG,
					     CMN_LA_CCIX_PROP_OFF,
					     ~(CMN_LA_PKT_HDR_VAL_MASK),
					     CMN_CLEAR_MASK);
			}
		}

		/*Loading from source BDF into Link0_BDF*/
		if (Index != DVSEC_PCR_PORT_BDF_IDX)
			goto done;
		if ((SingleHA == 1) || (SingleSA == 1) || (HARA == 1)) {
			Dvsec_Wr32(CMN_N72_LA_BASE_REG,
				     CMN_LINKID_PCIE_BUSNUM_OFF,
				     Val >> CMN_LA_LINK0_BUS_SHIFT);
			Dvsec_Wr32(CPM_SLCR_BASE, CMN_PORT_SRCID_CTRL_OFF,
				     Val >> CPM_SLCR_LINK0_BDF_SHIFT);
		}
		Dvsec_Wr32(CPM_SLCR_BASE, CMN_PORT_SRCID_CTRL_OFF, Val);
	} else if ((RegNum >= DvsecPcrLink[0].DvsecOff) &&
		   (RegNum <= DvsecPcrLink[DVSEC_PCR_LINK_LEN - 1].DvsecOff)) {
		Check = 1U;
		DVSEC_CALC_IDX_VAL(Index, Val, RegNum, DvsecPcrLink);
		if ((Index != DVSEC_PCR_LINK_AT_CTL_IDX) ||
		    (((Val >> DVSEC_LINK_CREDIT_SND_END_SHIFT) &
		       DVSEC_LINK_CREDIT_SND_END_MASK) == 0U)) {
			goto done;
		}

		if ((Val & DVSEC_MSGPACK_EN_MASK) == 0U) {
			Dvsec_Wr_M32(CMN_N72_LA_BASE_REG, CMN_LA_CCIX_PROP_OFF,
				     ~CMN_NOMSGPACK_MASK, CMN_NOMSGPACK_MASK);
		} else {
			/* Clear the nomsgpack bit */
			Dvsec_Wr_M32(CMN_N72_LA_BASE_REG, CMN_LA_CCIX_PROP_OFF,
				     ~CMN_NOMSGPACK_MASK, CMN_CLEAR_MASK);
		}
		/* Link-Up Sequence
		 * Enabling Snoop Response for RA
		 */
		if ((SingleRA == 1) || (HARA == 1)) {
			Dvsec_Wr_M32(CMN_N72_RA_BASE_REG, CMN_LINK0_CTL_1_OFF,
				     ~(CMN_LINK0_EN_REQ_VAL_OFF),
				     CMN_LINK0_EN_REQ_VAL_OFF);
			if (((Dvsec_Rd32(CMN_N72_RA_BASE_REG,
			    CMN_LINK0_CTL_1_OFF)) & CMN_LINK_EN_VAL_MASK) != 0U) {
				Dvsec_Wr_M32(CMN_N72_RA_BASE_REG,
				 CMN_LINK0_CTL_1_OFF,
				 ~CMN_CXPRTCTL_VAL_MASK, CMN_CXPRTCTL_VAL_MASK);
			}
		}

		/* Link-Up Sequence
		 * Enabling Snoop Response for HA
		 */
		if ((SingleHA == 1) || (SingleSA == 1) || (HARA == 1)) {
			Dvsec_Wr_M32(CMN_N72_HA_BASE_REG, CMN_LINK0_CTL_1_OFF,
				     ~(CMN_LINK0_EN_REQ_VAL_OFF),
				     CMN_LINK0_EN_REQ_VAL_OFF);
			if (((Dvsec_Rd32(CMN_N72_HA_BASE_REG,
			    CMN_LINK0_CTL_1_OFF)) & CMN_LINK_EN_VAL_MASK) != 0U) {
				Dvsec_Wr_M32(CMN_N72_HA_BASE_REG,
				 CMN_LINK0_CTL_1_OFF,
				 ~CMN_CXPRTCTL_VAL_MASK, CMN_CXPRTCTL_VAL_MASK);
			}
		}
	} else if ((RegNum >= DvsecSamEntry[DVSEC_SAM_0_IDX].DvsecPcrSam[0].DvsecOff) &&
		   (RegNum <= DvsecSamEntry[DVSEC_SAM_0_IDX].DvsecPcrSam[DVSEC_SAM_LEN - 1].DvsecOff)) {
		u32 StartAddr;
		u32 EndAddr;
		u32 Remote;
		u32 Len;

		Check = 1U;
		DVSEC_CALC_IDX_VAL(Index, Val, RegNum,
				   DvsecSamEntry[DVSEC_SAM_0_IDX].DvsecPcrSam);

		if ((Index != DVSEC_PCR_SAM_START_IDX) ||
		     ((Val & DVSEC_SAM_VALID_MASK) == 0U)) {
			goto done;
		}

		Remote = (Val & DVSEC_SAM_DEST_MASK);
		StartAddr =  Dvsec_Rd32(PCIEA_DVSEC_0,
			     DvsecSamEntry[DVSEC_SAM_0_IDX].DvsecPcrSam[DVSEC_SAM_SADDR_IDX].DvsecOff)
			     & DvsecSamEntry[DVSEC_SAM_0_IDX].DvsecPcrSam[DVSEC_SAM_SADDR_IDX].Mask;
		EndAddr = Dvsec_Rd32(PCIEA_DVSEC_0,
			  DvsecSamEntry[DVSEC_SAM_0_IDX].DvsecPcrSam[DVSEC_SAM_EADDR_IDX].DvsecOff)
			  & DvsecSamEntry[DVSEC_SAM_0_IDX].DvsecPcrSam[DVSEC_SAM_EADDR_IDX].Mask;
		Len = CMN_MEMORY_LEN(EndAddr, StartAddr);

		/* Assigning Base addr, Length & Valid Bit High of
		 * Region-0 in Node-76.
		 * Host provides SAM address bits 63:32. Need to program
		 * bits 47:26 of this address in bits 30:9 of memory register.
		 */
		Dvsec_Wr_M32(CMN_N76_BASE_REG, CMN_NH_MEM_REGION0_OFF,
		~(CMN_NH_MEM_REGION_MASK),
		(StartAddr << CMN_NH_MEM_BASE_ADDR_SHIFT)		       |
		(Remote ? ((u32)CMN_NODE_TYPE_CXRA_MASK) : (u32)CMN_NODE_TYPE_HNF_MASK)  |
		(Len << CMN_NH_MEM_LEN_SHIFT) | CMN_REG_VALID_MASK);

		Dvsec_Wr_M32(CMN_N72_BASE_REG, CMN_NH_MEM_REGION0_OFF,
		~(CMN_NH_MEM_REGION_MASK),
		(StartAddr << CMN_NH_MEM_BASE_ADDR_SHIFT)		      |
		(Remote ? ((u32)CMN_NODE_TYPE_CXRA_MASK) : ((u32)CMN_NODE_TYPE_HNF_MASK)) |
		(Len << CMN_NH_MEM_LEN_SHIFT) | CMN_REG_VALID_MASK);
	} else if ((RegNum >= DvsecSamEntry[DVSEC_SAM_1_IDX].DvsecPcrSam[0].DvsecOff) &&
		   (RegNum <= DvsecSamEntry[DVSEC_SAM_1_IDX].DvsecPcrSam[DVSEC_SAM_LEN - 1].DvsecOff)) {
		u32 StartAddr;
		u32 EndAddr;
		u32 Remote;
		u32 Len;

		Check = 1U;
		DVSEC_CALC_IDX_VAL(Index, Val, RegNum,
				   DvsecSamEntry[DVSEC_SAM_1_IDX].DvsecPcrSam);
		if ((Index != DVSEC_PCR_SAM_START_IDX) ||
		    ((Val & DVSEC_SAM_VALID_MASK) == 0U)) {
			goto done;
		}

		Remote = (Val & DVSEC_SAM_DEST_MASK);

		StartAddr =  Dvsec_Rd32(PCIEA_DVSEC_0,
			     DvsecSamEntry[DVSEC_SAM_1_IDX].DvsecPcrSam[DVSEC_SAM_SADDR_IDX].DvsecOff)
			     & DvsecSamEntry[DVSEC_SAM_1_IDX].DvsecPcrSam[DVSEC_SAM_SADDR_IDX].Mask;
		EndAddr = Dvsec_Rd32(PCIEA_DVSEC_0,
			  DvsecSamEntry[DVSEC_SAM_1_IDX].DvsecPcrSam[DVSEC_SAM_EADDR_IDX].DvsecOff)
			  & DvsecSamEntry[DVSEC_SAM_1_IDX].DvsecPcrSam[DVSEC_SAM_EADDR_IDX].Mask;
		Len = CMN_MEMORY_LEN(EndAddr, StartAddr);
		/* Assigning Base addr, Length & Valid Bit High of
		 * Region-0 in Node-76.
		 */
		Dvsec_Wr_M32(CMN_N76_BASE_REG, CMN_NH_MEM_REGION1_OFF,
		~(CMN_NH_MEM_REGION_MASK),
		(StartAddr << CMN_NH_MEM_BASE_ADDR_SHIFT)		      |
		(Remote ? ((u32)CMN_NODE_TYPE_CXRA_MASK) : (u32)CMN_NODE_TYPE_HNF_MASK) |
		(Len << CMN_NH_MEM_LEN_SHIFT) | CMN_REG_VALID_MASK);
	} else if ((RegNum >= DvsecIdm[0].DvsecOff) &&
		   (RegNum <= DvsecIdm[DVSEC_NUM_IDM_ENTRIES - 1].DvsecOff)) {
		u32 RegShift;
		u32 AgentId;
		u32 LinkId;
		u32 Reg;

		Check = 1U;
		DVSEC_CALC_IDX_VAL(Index, Val, RegNum, DvsecIdm);
		AgentId = Index;
		if ((Val & DVSEC_SAM_IDM_EN_MASK) == 0U) {
			goto done;
		}

		Reg = ((AgentId / DVSEC_REG_SIZE) * DVSEC_REG_SIZE);
		RegShift = (AgentId % DVSEC_REG_SIZE) * DVSEC_BYTE_SIZE;
		LinkId   = (Val >> DVSEC_LINK_ID_SHIFT) & DVSEC_LINK_ID_MASK;
		if ((SingleRA == 1) || (HARA == 1)) {
			/* Updating Link ID Value in agent id to link id reg. */
			Dvsec_Wr_M32(CMN_N72_RA_BASE_REG,
				     CMN_RA_AGENTID_LINKID_REG0_OFF + Reg,
				     ~((u32)1U << RegShift), (LinkId << RegShift));
			Dvsec_Wr32(CMN_N72_RA_BASE_REG,
				   CMN_RA_AGENTID_LINKID_VAL_OFF, (u32)0x1U);
		}
		if (SingleRA == 1) {
			Dvsec_Wr_M32(CMN_N72_LA_BASE_REG,
				     CMN_LA_AGENTID_LINKID_VAL_OFF, ~((u32)0x0U), (u32)0x1U);
		}
		if (HARA == 1) {
			if ((Val & DVSEC_SAM_DEST_MASK) == 0U) {
				Dvsec_Wr_M32(CMN_N72_HA_BASE_REG,
				  CMN_HA_RAID_LDID_REG_OFF + ((AgentId / DVSEC_REG_SIZE) * DVSEC_REG_SIZE),
				  ~(CMN_RAID_MASK),
				  CMN_LOC_RNF0_LDID | ((u32)0x1U << CMN_RAID_IS_RNF_SHIFT));
				Dvsec_Wr_M32(CMN_N72_HA_BASE_REG,
				  CMN_HA_RAID_LDID_VAL_OFF,
				  (~((u32)0x1U << CMN_LOC_RNF0_LDID)), ((u32)0x1U << CMN_LOC_RNF0_LDID));
				}
		}
	} else if ((RegNum >= DvsecCcid[0].DvsecOff) &&
		   (RegNum <= DvsecCcid[DVSEC_CCID_LEN - 1].DvsecOff)) {
		Check = 1U;
		DVSEC_CALC_IDX_VAL(Index, Val, RegNum, DvsecCcid);
		if (Index != DVSEC_CCID_OVERRIDE_4_IDX) {
			goto done;
		}
		XPsmFW_DvsecSetVal(RegNum, Val);
		Val = Dvsec_Rd32(PCIEA_DVSEC_0, RegNum) & DvsecCcid[Index].Mask;
		DvsecCcid[Index].Val = Val;
	} else if ((RegNum >= DvsecPcrRa0[0].DvsecOff) &&
		   (RegNum <= DvsecPcrRa0[DVSEC_PCR_RA_LEN - 1].DvsecOff)) {
		u32 RaidVal;

		Check = 1U;
		DVSEC_CALC_IDX_VAL(Index, Val, RegNum, DvsecPcrRa0);
		if ((Index != DVSEC_PCR_RA_CTL_IDX) ||
		    ((Val & DVSEC_RA_EN_MASK) == 0U)) {
			goto done;
		}
		RaidVal = Val >> CMN_RAID_SHIFT;
		Dvsec_Wr_M32(CMN_N72_RA_BASE_REG, CMN_LDID_RAID_REG0_1_OFF,
			     (u32)(~(CMN_RAID_MASK << CMN_LDID1_RAID_REG0_1_SHIFT)),
			     RaidVal << CMN_LDID1_RAID_REG0_1_SHIFT);

		Dvsec_Wr_M32(CMN_N72_RA_BASE_REG, CMN_LDID_RAID_STAT_OFF,
			     ~CMN_RAID_STAT_MASK, CMN_RAID_STAT_MASK);
	} else if ((RegNum >= DvsecPcrHa0[0].DvsecOff) &&
		 (RegNum <= DvsecPcrHa0[DVSEC_PCR_HA_LEN - 1].DvsecOff)) {
		static u32 RegCount;

		Check = 1U;
		DVSEC_CALC_IDX_VAL(Index, Val, RegNum, DvsecPcrHa0);
		if (Index == DVSEC_PCR_HA_IDTBL_ENTRY_IDX) {
			/* Storing the HAID Value for Node 72 */
			Dvsec_Wr32(CMN_N72_HA_BASE_REG, CMN_HAID_OFF,
				   (Val & CMN_HAID_MASK));
		} else if (Index == DVSEC_PCR_HBAT1_IDX) {
			u32 SrcAddr = 0U;
			u32 BATValid;

			BATValid  = Dvsec_Rd32(PCIEA_DVSEC_0,
				    DvsecPcrHa0[DVSEC_PCR_HBAT0_IDX].DvsecOff) &
				    DvsecPcrHa0[DVSEC_PCR_HBAT0_IDX].Mask;
			if ((BATValid & DVSEC_HBAT_VALID_MASK) == 0U) {
				goto done;
			}

			SrcAddr = Dvsec_Rd32(PCIEA_DVSEC_0,
				   DvsecPcrHa0[DVSEC_PCR_HBAT1_IDX].DvsecOff) &
				   DvsecPcrHa0[DVSEC_PCR_HBAT1_IDX].Mask;

			XPsmFw_DvsecSetupMem(SrcAddr);

		} else if (Index == DVSEC_PCR_HA_PRSNT_RAID0_IDX) {
			u32 BitShift;
			u32 LDIDInc;
			u32 LDIDVal;
			u32 RegVal;
			u32 i;

			/*  i  is bit position in the Present RAID Vector. */
			for (i = 0U; i < DVSEC_PRSNT_RAID_LEN; i++) {
				if ((((Val >> i) & DVSEC_PRSNT_RAID_VAL_MASK) == 0U) ||
				    ((Dvsec_Rd32(PCIEA_DVSEC_0,
					DvsecIdm[i].DvsecOff) & DVSEC_SAM_DEST_MASK) == 0U)) {
					RegCount++;
					continue;
				}

				BitShift = ((RegCount) % DVSEC_REG_SIZE) *
					    CMN_NUM_RA_RLID_REG;
				RegVal = (RegCount / DVSEC_REG_SIZE) *
					  DVSEC_REG_SIZE;
				LDIDInc = (RegCount / DVSEC_PRSNT_RAID_LEN) *
					  DVSEC_REG_SIZE;
				LDIDVal = (i + 2U) % DVSEC_PRSNT_RAID_LEN;

				Dvsec_Wr32(CMN_N40_BASE_REG,
				 (CMN_HNF_RA_PHY_ID1_OFF + (RegCount * DVSEC_REG_SIZE)),
				 (CMN_CML0_NID | CMN_REMOTE_RN_MASK |
				  CMN_HNF_RN_PHY_ID_VAL_MASK));

				Dvsec_Wr_M32(CMN_N72_HA_BASE_REG,
				  CMN_HA_RAID_LDID_REG_OFF + RegVal,
				  ~((u32)CMN_RAID_MASK << BitShift),
				  (LDIDVal << BitShift) |
				  ((u32)0x1U << (BitShift + CMN_RAID_IS_RNF_SHIFT)));

				Dvsec_Wr_M32(CMN_N72_HA_BASE_REG,
				  CMN_HA_RAID_LDID_VAL_OFF + LDIDInc,
				  (~((u32)0x1U << LDIDVal)), ((u32)0x1U << LDIDVal));

				Dvsec_Wr_M32(CMN_N72_HA_BASE_REG,
					     CMN_HA_AGENTID_LINKID_VAL_OFF,
					     (~((u32)0x1U << i)), ((u32)0x1U << i));
				Dvsec_Wr_M32(CMN_N72_LA_BASE_REG,
					     CMN_LA_AGENTID_LINKID_VAL_OFF,
					     (~((u32)0x1U << i)), ((u32)0x1U << i));
				RegCount++;
			}
		} else if (Index == DVSEC_PCR_HA_PRSNT_RAID1_IDX) {
			u32 BitShift;
			u32 LDIDInc;
			u32 LDIDVal;
			u32 RegVal;
			u32 i;

			for (i = 0U; i < DVSEC_PRSNT_RAID_LEN; i++) {
				if ((((Val >> i) & DVSEC_PRSNT_RAID_VAL_MASK) == 0U) ||
				    ((Dvsec_Rd32(PCIEA_DVSEC_0,
					DvsecIdm[i].DvsecOff) & DVSEC_SAM_DEST_MASK) == 0U)) {
					RegCount++;
					continue;
				}

				BitShift = ((RegCount) % DVSEC_REG_SIZE) *
					    CMN_NUM_RA_RLID_REG;
				RegVal = (RegCount / DVSEC_REG_SIZE) *
					  DVSEC_REG_SIZE;
				LDIDInc = (RegCount / DVSEC_PRSNT_RAID_LEN) *
					  DVSEC_REG_SIZE;
				LDIDVal = (i + 2U) % DVSEC_PRSNT_RAID_LEN;

				Dvsec_Wr32(CMN_N40_BASE_REG,
				 (CMN_HNF_RA_PHY_ID16_OFF + (RegCount * DVSEC_REG_SIZE)),
				 (CMN_CML0_NID | CMN_REMOTE_RN_MASK |
				 CMN_HNF_RN_PHY_ID_VAL_MASK));

				Dvsec_Wr_M32(CMN_N72_HA_BASE_REG,
				  (CMN_HA_RAID_LDID_REG_OFF + RegVal),
				  ~((u32)CMN_RAID_MASK << BitShift),
				  ((LDIDVal << BitShift) |
				  ((u32)0x1U << (BitShift + CMN_RAID_IS_RNF_SHIFT))));

				Dvsec_Wr_M32(CMN_N72_HA_BASE_REG,
				  CMN_HA_RAID_LDID_VAL_OFF + LDIDInc,
				  (~((u32)0x1U << LDIDVal)), ((u32)0x1U << LDIDVal));

				Dvsec_Wr_M32(CMN_N72_HA_BASE_REG,
					     CMN_HA_AGENTID_LINKID_VAL_OFF + DVSEC_REG_SIZE,
					     ~((u32)0x1U << i), ((u32)0x1U << i));

				Dvsec_Wr_M32(CMN_N72_LA_BASE_REG,
					     CMN_LA_AGENTID_LINKID_VAL_OFF + DVSEC_REG_SIZE,
					     ~((u32)0x1U << i), ((u32)0x1U << i));

				RegCount++;
			}
		} else {
			goto done;
		}
	} else if ((RegNum >= DvsecPcrSa0[0].DvsecOff) &&
		 (RegNum <= DvsecPcrSa0[DVSEC_PCR_SA_LEN - 1].DvsecOff)) {

		Check = 1U;
		DVSEC_CALC_IDX_VAL(Index, Val, RegNum, DvsecPcrSa0);
		if (Index == DVSEC_PCR_SA_CTL_IDX) {
			/* Storing the SAID Value for Node 72 */
			Dvsec_Wr32(CMN_N72_HA_BASE_REG, CMN_HAID_OFF,
				    (Val & CMN_SAID_MASK) >> CMN_SAID_SHIFT);
		} else if (Index == DVSEC_PCR_SBAT1_IDX) {
			u32 SrcAddr = 0U;
			u32 BATValid;

			BATValid  = Dvsec_Rd32(PCIEA_DVSEC_0,
				    DvsecPcrHa0[DVSEC_PCR_SBAT0_IDX].DvsecOff) &
				    DvsecPcrHa0[DVSEC_PCR_SBAT0_IDX].Mask;
			if ((BATValid & DVSEC_HBAT_VALID_MASK) == 0U) {
				goto done;
			}

			SrcAddr = Dvsec_Rd32(PCIEA_DVSEC_0,
				   DvsecPcrHa0[DVSEC_PCR_SBAT1_IDX].DvsecOff) &
				   DvsecPcrHa0[DVSEC_PCR_SBAT1_IDX].Mask;

			XPsmFw_DvsecSetupMem(SrcAddr);

		} else {
			goto done;
		}
	} else {
		/* Acknowledge the interrupt */
		goto done;
	}

done:
	Xil_Out32(CPM_SLCR_BASE + CPM_CORR_IR_STA_OFF,
		  Xil_In32(CPM_SLCR_BASE + CPM_CORR_IR_STA_OFF));
	Xil_Out32(PCIEA_ATTRIB_0 + PCIE_CFG_ADDR_OFF, PCIE_CFG_MASK);

	if (Check == 1) {
		Dvsec_Wr32(PCIEA_DVSEC_0, RegNum, Val);
	}

	return XST_SUCCESS;
}

XStatus XPsmFw_DvsecATSHandler(void)
{
	static u8 ATSInit = 0U;
	u32 IntrStatus;
	u32 ATSStatus;
	u32 PRIStatus;
	u32 ATSBaseReg;
	u32 SCBaseReg;
	u32 PASIDStatus;

	SCBaseReg = Dvsec_Rd32(PSM_DVSEC_RAM, PSM_DVSEC_SC_BASE_ADDR_OFF);
	ATSBaseReg = SCBaseReg - ATS_BASE_OFF;

	if (ATSInit  == 0U) {
		Dvsec_Wr32(ATSBaseReg, ATS_SC1_TAG_OFF, ATS_SC1_TAG_VAL);
		Dvsec_Wr32(ATSBaseReg, ATS_SC1_AXI0_BDF_OFF, ATS_SC1_AXI0_BDF_VAL);
		Dvsec_Wr32(ATSBaseReg, ATS_AXI_PCI_BDF_OFF, ATS_AXI_PCI_BDF_VAL);
		Dvsec_Wr32(ATSBaseReg, ATS_AXI_PCI_TAG_OFF, ATS_AXI_PCI_TAG_VAL);
		Dvsec_Wr32(ATSBaseReg, ATS_CTRL_REG_OFF, ATS_CTRL_REG_VAL);

		Dvsec_Wr32(SCBaseReg, SC_ATS_PCIE_CTRL_OFF, SC_ATS_PCIE_CTRL_VAL);
		Dvsec_Wr32(SCBaseReg, SC_ATS_PCIE_CTRL1_OFF, SC_ATS_PCIE_CTRL1_VAL);
		Dvsec_Wr32(SCBaseReg, SC_PASID_CAP_REG_OFF, SC_PASID_CAP_REG_VAL);

		ATSInit = 1U;
	}

	IntrStatus = Dvsec_Rd32(PL_INTR_STATUS_REG, 0U);
	ATSStatus = IntrStatus & PL_ATS_INTR_STATUS_MASK;
	PRIStatus = (IntrStatus & PL_PRI_INTR_STATUS_MASK)
		    >> PL_PRI_INTR_STATUS_SHIFT;
	PASIDStatus = (IntrStatus & PL_PASID_INTR_STATUS_MASK)
		    >> PL_PASID_INTR_STATUS_SHIFT;

	Dvsec_Wr_M32(SCBaseReg, SC_ATS_PRI_CTRL_OFF,
		     ~((u32)1U << SC_ATS_STATUS_SHIFT),
		     ATSStatus << SC_ATS_STATUS_SHIFT);
	Dvsec_Wr_M32(SCBaseReg, SC_PRI_CTRL_OFF,
		     ~((u32)1U << SC_PRI_STATUS_SHIFT),
		     PRIStatus << SC_PRI_STATUS_SHIFT);
	Dvsec_Wr32(SCBaseReg, SC_PASID_CTRL_OFF,
		   PASIDStatus << SC_PASID_STATUS_SHIFT);
	Dvsec_Wr32(SCBaseReg, SC_PASID_CTRL1_OFF, SC_PASID_CTRL1_VAL);

	return XST_SUCCESS;
}

void XPsmFw_DvsecInit(void)
{
	u32 Value;

	Value = Dvsec_Rd32(PSM_DVSEC_RAM, PSM_DVSEC_DYN_CONTROL_OFF);
	if ((Value & PSM_DVSEC_RA0_EN_MASK) != 0U) {
		SingleRA = 1U;
	}

	if ((Value & PSM_DVSEC_AGENT_SEL_HA_MASK) != 0U) {
		Value = Dvsec_Rd32(PSM_DVSEC_RAM, PSM_DVSEC_MP_PROP0_OFF);
		/* Atleast one pool should be enabled for HA */
		if ((Value & PSM_DVSEC_MEM_REG_EN_MASK) != 0U) {
			SingleHA = 1U;
		}
	}
	if ((Value & PSM_DVSEC_AGENT_SEL_SA_MASK) != 0U) {
		Value = Dvsec_Rd32(PSM_DVSEC_RAM, PSM_DVSEC_MP_PROP0_OFF);
		/* Atleast one pool should be enabled for SA */
		if ((Value & PSM_DVSEC_MEM_REG_EN_MASK) != 0U) {
			SingleSA = 1U;
		}
	}

	if ((SingleHA == 1) && (SingleRA == 1)) {
		HARA = 1U;
		SingleHA = 0U;
		SingleRA = 0U;
	}

	if (SingleSA == 1) {
		HARA = 0U;
		SingleHA = 0U;
		SingleRA = 0U;
	}

	if ((SingleHA == 1) || (SingleSA == 1)) {
		DvsecPcrLink[0].Val = DVSEC_PCR_LINK_START_HA_VAL;
		DvsecPcsrLink[0].Val = DVSEC_PCSR_LINK_START_HA_VAL;
		DvsecPcsrPrimary[DVSEC_PCSR_RSAM_OFF_IDX].Val = 0x0U;
		DvsecPcsrPrimary[DVSEC_PCSR_HSAM_OFF_IDX].Val =
							DVSEC_PCSR_HSAM_OFF_VAL;
	 }

	if ((SingleRA == 1)) {
		DvsecPcsrRa0[0].Val = DVSEC_PCSR_RA0_START_RA_VAL;
		DvsecPcsrPrimary[DVSEC_PCSR_RSAM_OFF_IDX].Val =
							DVSEC_PCSR_RSAM_OFF_VAL;
		DvsecPcsrPrimary[DVSEC_PCSR_HSAM_OFF_IDX].Val = 0x0U;
		/* RA only NODEID = 72*/
		Dvsec_Wr_M32(CMN_N76_BASE_REG, CMN_NH_TGT_NID0_OFF,
			     DVSEC_32B_MASK,
			     (u32)(CMN_CML0_NID << CMN_NH_TGT_NID0_SHIFT));
		Dvsec_Wr_M32(CMN_N72_BASE_REG, CMN_NH_TGT_NID0_OFF,
			     DVSEC_32B_MASK,
			     (u32)(CMN_NH_TGT_NID0 << CMN_NH_TGT_NID0_SHIFT));
	}

	if ((HARA == 1)) {
		Dvsec_Wr_M32(CMN_N76_BASE_REG, CMN_NH_TGT_NID0_OFF,
			     DVSEC_32B_MASK,
			     (u32)((CMN_CML0_NID << CMN_NH_TGT_NID0_SHIFT) |
			     (CMN_NH_TGT_NID1 << CMN_NH_TGT_NID1_SHIFT)));
	}

	if ((SingleHA == 1) || (HARA == 1) || (SingleSA == 1)) {
		DVSEC_SET_MEM_CAP_L(DVSEC_PCSR_MP0_CAP0_IDX,
				    PSM_DVSEC_MP_SIZE0_OFF);
		DVSEC_SET_MEM_CAP_H(DVSEC_PCSR_MP0_CAP1_IDX,
				    PSM_DVSEC_MP_SIZE0_OFF);
		DVSEC_SET_MEM_CAP_L(DVSEC_PCSR_MP1_CAP0_IDX,
				    PSM_DVSEC_MP_SIZE1_OFF);
		DVSEC_SET_MEM_CAP_H(DVSEC_PCSR_MP1_CAP1_IDX,
				    PSM_DVSEC_MP_SIZE1_OFF);
		DVSEC_SET_MEM_CAP_L(DVSEC_PCSR_MP2_CAP0_IDX,
				    PSM_DVSEC_MP_SIZE2_OFF);
		DVSEC_SET_MEM_CAP_H(DVSEC_PCSR_MP2_CAP1_IDX,
				    PSM_DVSEC_MP_SIZE2_OFF);
		DVSEC_SET_MEM_CAP_L(DVSEC_PCSR_MP3_CAP0_IDX,
				    PSM_DVSEC_MP_SIZE3_OFF);
		DVSEC_SET_MEM_CAP_H(DVSEC_PCSR_MP3_CAP1_IDX,
				    PSM_DVSEC_MP_SIZE3_OFF);

		Dvsec_Wr_M32(CMN_N72_BASE_REG, CMN_NH_TGT_NID0_OFF,
			     DVSEC_32B_MASK,
			     (u32)((CMN_NH_TGT_NID0 << CMN_NH_TGT_NID0_SHIFT) |
				  (CMN_NH_TGT_NID1 << CMN_NH_TGT_NID1_SHIFT)));
		Dvsec_Wr32(CMN_N40_BASE_REG, CMN_HNF_SAM_CONTROL_OFF,
			   CMN_SN0_NID);
		Dvsec_Wr32(CMN_N40_BASE_REG, CMN_HNF_SAM_PROP_OFF,
			   (u32)(CMN_SN0_128B_WIDTH_MASK | CMN_SN0_PCMO_PROP_MASK  |
				 CMN_SN1_128B_WIDTH_MASK | CMN_SN1_PCMO_PROP_MASK));
		Dvsec_Wr_M32(CMN_N40_BASE_REG, CMN_HNF_AUX_CTL_OFF,
			     ~CMN_HNF_STASH_DIS_MASK, CMN_HNF_STASH_DIS_MASK);
	}
	/* Updating Traffic Class & TLP */
	Dvsec_Wr_M32(CMN_N72_LA_BASE_REG, CMN_LA_TLP_HDR_1_OFF,
		   ~(CMN_TC_TLP_MASK), CMN_TRAFFIC_CLASS);

	Value = Dvsec_Rd32(PCIEA_ATTRIB_0, PCIE_PF0_PDVSEC_VID_OFF);
	DvsecPcsrProtocol[DVSEC_PCSR_PROT_DVSEC_HDR_IDX].Val =
			DvsecPcsrProtocol[DVSEC_PCSR_PROT_DVSEC_HDR_IDX].Val |
			(Value & DVSEC_LOW_16B_MASK);

	/* Updating VID in CXLA_TLP_HDR_FIELDS */
	Dvsec_Wr_M32(CMN_N72_LA_BASE_REG, CMN_LA_TLP_HDR_2_OFF,
		     ~DVSEC_LOW_16B_MASK, Value);


	Value = Dvsec_Rd32(PCIEA_ATTRIB_0, PCIE_PF0_PCSR_SIZE_OFF);
	DvsecPcsrProtocol[DVSEC_PCSR_PROT_PL_CAPSTAT_IDX].Val =
			DvsecPcsrProtocol[DVSEC_PCSR_PROT_PL_CAPSTAT_IDX].Val |
			(Value & DVSEC_LOW_11B_MASK);
	Value = Dvsec_Rd32(PCIEA_ATTRIB_0, PCIE_PF0_PCR_SIZE_OFF);
	DvsecPcsrProtocol[DVSEC_PCSR_PROT_PL_CNTRL_IDX].Val =
			DvsecPcsrProtocol[DVSEC_PCSR_PROT_PL_CNTRL_IDX].Val |
			(Value & DVSEC_LOW_11B_MASK);
}
