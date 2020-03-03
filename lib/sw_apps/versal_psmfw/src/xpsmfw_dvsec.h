/******************************************************************************
*
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
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
#define CPM_CMN_BASE			0xFC000000U
#define CPM_ADDR_REMAP			0xFCF30000U

#define CMN_N40_BASE_REG		CPM_CMN_BASE + 0x500000U
#define CMN_N72_RA_BASE_REG		CPM_CMN_BASE + 0x900000U
#define CMN_N72_HA_BASE_REG		CPM_CMN_BASE + 0x910000U
#define CMN_N72_LA_BASE_REG		CPM_CMN_BASE + 0x920000U
#define CMN_N72_BASE_REG		CPM_CMN_BASE + 0x950000U
#define CMN_N76_BASE_REG		CPM_CMN_BASE + 0x968000U

#define CPM_CORR_IR_STA_OFF		0x300U
#define CPM_MISC_IR_STA_OFF		0x340U

#define PCIE_PF0_PCSR_SIZE_OFF		0xA6CU
#define PCIE_PF0_PCR_SIZE_OFF		0xA74U
#define PCIE_PF0_PDVSEC_VID_OFF		0xA58U
#define PCIE_CFG_ADDR_OFF		0xE78U

#define CMN_PORT_SRCID_CTRL_OFF		0x214U
#define CMN_HAID_OFF			0x8U
#define CMN_HNF_RA_PHY_ID1_OFF		0xD30U
#define CMN_HNF_RA_PHY_ID16_OFF		0xDA8U
#define CMN_LA_CCIX_PROP_OFF		0xC08U
#define CMN_NH_MEM_REGION0_OFF		0xC08U
#define CMN_NH_MEM_REGION1_OFF		0xC0CU
#define CMN_RASAM_REG0_1_OFF		0xDA8U
#define CMN_RASAM_REG0_2_OFF		0xDACU
#define CMN_NH_TGT_NID0_OFF		0xC30U
#define CMN_NH_TGT_NID1_OFF		0xC34U
#define CMN_RNSAM_STATUS_OFF		0xC00U
#define CMN_HA_RAID_LDID_REG_OFF	0xC00U
#define CMN_HA_RAID_LDID_VAL_OFF	0xD08U
#define CMN_HA_AGENTID_LINKID_VAL_OFF 	0xD00U
#define CMN_LA_AGENTID_LINKID_VAL_OFF	0xC70U
#define CMN_LDID_RAID_REG0_1_OFF	0xEA0U
#define CMN_LINKID_PCIE_BUSNUM_OFF	0xC78U
#define CMN_LINK0_CTL_1_OFF		0x1000U
#define CMN_LINK0_EN_REQ_VAL_OFF	0x3U
#define CMN_LA_TLP_HDR_1_OFF		0xC80U
#define CMN_LA_TLP_HDR_2_OFF		0xC84U
#define CMN_RA_AGENTID_LINKID_VAL_OFF	0xF20U
#define CMN_LDID_RAID_STAT_OFF		0xF28U
#define CMN_RA_AGENTID_LINKID_REG0_OFF	0xE60U
#define CMN_HNF_SAM_CONTROL_OFF		0xD00U
#define CMN_HNF_SAM_PROP_OFF		0xD18U

#define CPM_AR_SRC_ADDR0_H_OFF		0x24U
#define CPM_AR_APER_SIZE0_H_OFF		0x2CU
#define CPM_AR_ADDRREMAP_CTL_OFF	0xE4U
#define CPM_AR_DST_ADDR0_H_OFF		0x34U

#define DVSEC_TRANSPORT_HDR_OFF		0xA38U
#define DVSEC_PROTOCOL_HDR_OFF		0xA58U

#define PCIE_CFG_MASK			0x8000000U
#define CMN_TC_TLP_MASK			0x47000U
#define CMN_PARTIAL_CACHE_ADDR_MASK	0x7AU
#define CMN_RAID_MASK			0x3FU
#define CMN_CXPRTCTL_VAL_MASK		0xFU
#define CMN_LINK_EN_VAL_MASK		0x1U
#define CMN_LA_PKT_HDR_VAL_MASK		0x40U
#define	CMN_RAID_STAT_MASK		0x2U
#define CMN_REG_VALID_MASK		0x1U
#define CMN_HNF_RN_PHY_ID_VAL_MASK 	0x80000000U
#define CMN_NOMSGPACK_MASK		0x400U
#define CMN_RSAM_STATUS_MASK		0x3U
#define CMN_RSAM_UNSTALL_MASK		0x2U
#define CMN_RSAM_REGION_VALID_MASK	0x80000000U
#define CMN_NH_MEM_REGION_MASK		0x7FFFFFF1U
#define CMN_NODE_TYPE_CXRA_MASK		0x8U
#define CMN_NODE_TYPE_HNF_MASK		0x0U
#define CMN_REMOTE_RN_MASK		0x10000U
#define CMN_HAID_MASK			0x1FU
#define CMN_SN0_128B_WIDTH_MASK		0x1U
#define CMN_SN1_128B_WIDTH_MASK		0x40U
#define CMN_SN1_PCMO_PROP_MASK		0x100U
#define CMN_SN0_PCMO_PROP_MASK		0x4U
#define CMN_APER_SIZE0_EN_MASK		0x1U
#define CMN_CLEAR_MASK			0x0U
#define DVSEC_PRIM_PRT_CTRL_SAM_MASK	0x000000E0U
#define DVSEC_RSAM_VALID_MASK		0x00000040U
#define DVSEC_SAM_PID_MASK		0xF0U
#define DVSEC_IDM_VALID_MASK		0x00000020U
#define DVSEC_SAM_IDM_EN_MASK		0x1U
#define DVSEC_RA_EN_MASK		0x1U
#define DVSEC_SAM_VALID_MASK		0x1U
#define DVSEC_LINK_CREDIT_SND_END_MASK	0x1U
#define DVSEC_DEV_ID_MASK		0xFFU
#define DVSEC_PKT_HEADER_MASK		0x1U
#define DVSEC_PARTIAL_CACHE_MASK	0x1U
#define DVSEC_CACHE_LINE_SIZE_MASK	0x1U
#define DVSEC_ADDR_WIDTH_EN_MASK	0x7U
#define DVSEC_LOW_16B_MASK		0xFFFFU
#define DVSEC_LOW_11B_MASK		0xFFFU
#define DVSEC_32B_MASK			~0x0U
#define DVSEC_VENDOR_ID_MASK		0xFFFF0000U
#define DVSEC_LINK_ID_MASK		0x3U
#define DVSEC_SAM_DEST_MASK		0x2U
#define DVSEC_HBAT_VALID_MASK		0x1U
#define DVSEC_PRSNT_RAID_VAL_MASK	0x1U

#define CPM_SLCR_LINK0_BDF_SHIFT	0x0U
#define PCIE_PDVSEC_REG_NO_SHIFT	0x9U
#define PCIE_PDVSEC_REG_FIX_SHIFT	0x2U
#define CMN_RAID_SHIFT			0x1AU
#define CMN_LDID1_RAID_REG0_1_SHIFT	0x8U
#define CMN_LA_LINK0_BUS_SHIFT		0x8U
#define CMN_NH_TGT_NID0_SHIFT 		0x0U
#define CMN_NH_TGT_NID1_SHIFT		0xCU
#define CMN_NH_TGT_NID2_SHIFT		0x18U
#define CMN_NH_TGT_NID3_SHIFT		0x4U
#define CMN_PARTIAL_CACHE_SHIFT		0x1U
#define CMN_CACHE_LINE_SIZE_SHIFT	0x3U
#define CMN_ADDR_WIDTH_EN_SHIFT		0x4U
#define CMN_RSAM_REGN_TID_SHIFT		0x14U
#define CMN_NH_MEM_BASE_ADDR_SHIFT	0xFU
#define CMN_NH_MEM_LEN_SHIFT		0x4U
#define CMN_RAID_IS_RNF_SHIFT		0x7U

#define DVSEC_SAM_PID_SHIFT		0x4U
#define DVSEC_LINK_CREDIT_SND_END_SHIFT	0x1U
#define DVSEC_DEV_ID_SHIFT		0x18U
#define DVSEC_PKT_HEADER_SHIFT		0x1U
#define DVSEC_PARTIAL_CACHE_SHIFT	0x1U
#define DVSEC_CACHE_LINE_SIZE_SHIFT	0x3U
#define DVSEC_ADDR_WIDTH_EN_SHIFT	0x4U
#define DVSEC_SHIFT_16BIT		0x10U
#define DVSEC_VENDOR_ID_SHIFT		0x10U
#define DVSEC_LINK_ID_SHIFT		0xFU

#define DVSEC_NUM_SAM_ENTRIES		0x8U
#define DVSEC_NUM_IDM_ENTRIES		0x40U
#define DVSEC_PRSNT_RAID_LEN		0x20U
#define DVSEC_REG_SIZE			0x4U
#define DVSEC_BYTE_SIZE			0x8U
#define CMN_64B_REG_SIZE		0x8U
#define CMN_NUM_RA_RLID_REG		0x8U
#define CMN_TRAFFIC_CLASS		0x7000U
#define CMN_CML0_NID			0x48U
#define CMN_NH_TGT_NID0			0x28U
#define CMN_NH_TGT_NID1			0x24U
#define CMN_NH_TGT_NID2			0x48U
#define CMN_NH_TGT_NID3			0x44U
#define CMN_SN0_NID			0x2CU
#define CPM_AR_DST_ADDR_H		0x8U
#define CPM_AR_APER_SIZE_H		0xFFEU
#define CMN_NH_MEM_SIZE_8G_MASK		0x70U

#define DVSEC_CCID_LEN			0x5U
#define DVSEC_PCR_PRIM_LEN		0x6U
#define DVSEC_PCR_PORT_LEN		0xBU
#define DVSEC_PCR_LINK_LEN		0x8U
#define DVSEC_PCR_RA_LEN		0x4U
#define DVSEC_PCR_HA_LEN		0xFU
#define DVSEC_SAM_LEN			0x3U
#define DVSEC_PCSR_PROT_LEN		0x5U
#define DVSEC_PCSR_PRIM_LEN		0x8U
#define DVSEC_PCSR_PORT_LEN		0x5U
#define DVSEC_PCSR_LINK_LEN		0x6U
#define DVSEC_PCSR_HA_LEN		0xBU
#define DVSEC_PCSR_RA_LEN		0x3U

#define DVSEC_PCSR_PRIM_CAP_STAT_IDX 	0x1U
#define DVSEC_PCSR_PROT_DVSEC_HDR_IDX	0x1U
#define DVSEC_PCSR_PROT_PL_CAPSTAT_IDX	0x3U
#define DVSEC_PCSR_PROT_PL_CNTRL_IDX	0x4U
#define DVSEC_CCID_OVERRIDE_4_IDX	0x4U
#define DVSEC_PCR_PRIM_START_IDX	0x0U
#define DVSEC_PCR_PRIM_COMMON_CTL1_IDX	0x1U
#define DVSEC_PCR_PRIM_COMMON_CTL2_IDX	0x2U
#define DVSEC_PCR_PORT_CTL_IDX		0x1U
#define DVSEC_PCR_PORT_BDF_IDX		0x4U
#define DVSEC_PCR_LINK_AT_CTL_IDX	0x1U
#define DVSEC_PCR_SAM_START_IDX		0x0U
#define DVSEC_SAM_0_IDX			0x0U
#define DVSEC_SAM_1_IDX			0x1U
#define DVSEC_PCR_RA_CTL_IDX		0x1U
#define DVSEC_PCR_HA_PRSNT_RAID0_IDX	0x2U
#define DVSEC_PCR_HA_PRSNT_RAID1_IDX	0x3U
#define DVSEC_PCR_HA_IDTBL_ENTRY_IDX	0x6U
#define DVSEC_PCR_HBAT0_IDX		0x7U
#define DVSEC_PCR_HBAT1_IDX		0x8U
#define DVSEC_SAM_SADDR_IDX		0x1U
#define DVSEC_SAM_EADDR_IDX		0x2U


#define Dvsec_Wr_M32(Reg, Off, Mask, Val) \
	Xil_Out32((Reg) + (Off), (Xil_In32((Reg) + (Off)) & (Mask)) | (Val))

#define Dvsec_Wr32(Reg, Off, Val) Xil_Out32((Reg) + (Off), (Val))

#define Dvsec_Rd32(Reg, Off) Xil_In32((Reg) + (Off))

#define DVSEC_CALC_IDX(Reg, DvsecStruct) \
	((Reg) - (DvsecStruct[0].DvsecOff)) / (DVSEC_REG_SIZE)

#define DVSEC_CALC_IDX_VAL(Index, Val, Reg, DvsecStruct) \
	Index = DVSEC_CALC_IDX((Reg), (DvsecStruct)); \
	Val = Dvsec_Rd32((PCIEA_DVSEC_0), (Reg)) & (DvsecStruct[Index].Mask);

#define CMN_MEMORY_LEN(EndAddr, StartAddr) \
	log2((((EndAddr) - (StartAddr)) * (4 * 1024))/64);

typedef struct{
	u32 DvsecOff;
	u32 Val;
	u32 Mask;
}DvsecReg;

typedef struct{
	u32 DvsecOff;
	u32 Val;
} DvsecPcsr;

void XPsmFw_DvsecInit(void);
void XPsmFW_DvsecSetVal(u32 Off, u32 Val);

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_DVSEC_H */
