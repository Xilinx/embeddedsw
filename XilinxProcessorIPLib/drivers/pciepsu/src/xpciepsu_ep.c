/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
* @file xpciepsu_ep.c
*
* Implements all of functions for psu_pci IP EndPoint driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0	tk	02/13/2019	First release
* </pre>
*
*******************************************************************************/
/******************************** Include Files *******************************/
#include "xpciepsu_ep.h"
#include "xpciepsu_common.h"
#include "xpciepsu_hw.h"
#include "sleep.h"
/**************************** Constant Definitions ****************************/

/****************************** Type Definitions ******************************/

/******************** Macros (Inline Functions) Definitions *******************/

/**************************** Variable Definitions ****************************/

/***************************** Function Prototypes ****************************/

/******************************************************************************/
/**
* This function initializes the config space
*
* @param   PciePsuPtr pointer to XPciePsu Instance Pointer
* @param   ConfigPtr pointer to XPciePsu_Config instrance Pointer.
*
* @return  XST_SUCCESS on success
*          err on failure
*
*******************************************************************************/
void XPciePsu_EP_CfgInitialize(XPciePsu *PciePsuPtr,
		const XPciePsu_Config *ConfigPtr){
	Xil_AssertVoid(ConfigPtr != NULL);
	(void)memset(PciePsuPtr, 0, sizeof(XPciePsu));
	(void)memcpy(&(PciePsuPtr->Config), ConfigPtr, sizeof(XPciePsu_Config));
}
/******************************************************************************/
/**
* This function initializes PCIe bridge.
*
* @param   PciePsuPtr pointer to XPciePsu Instance Pointer
*
*******************************************************************************/
void XPciePsu_EP_BridgeInitialize(XPciePsu *PciePsuPtr)
{
	u32 Val;
	Xil_AssertVoid(PciePsuPtr != NULL);

	/* Bridge Configurations */
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg, XPCIEPSU_E_BREG_BASE_LO,
			LOWER_32_BITS(PciePsuPtr->Config.BrigReg));
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg, XPCIEPSU_E_BREG_BASE_HI,
			UPPER_32_BITS(PciePsuPtr->Config.BrigReg));

	Val = XPciePsu_ReadReg(PciePsuPtr->Config.BrigReg,
			XPCIEPSU_E_BREG_CONTROL);
	Val &= ~(BREG_SIZE_MASK | BREG_ENABLE_FORCE);
	Val |= ((u32)BREG_SIZE) << BREG_SIZE_SHIFT;
	Val |= BREG_ENABLE;
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			XPCIEPSU_E_BREG_CONTROL, Val);

	/* DMA regs Configurations */
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg, DREG_BASE_LO,
			PciePsuPtr->Config.DmaBaseAddr);
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg, DREG_BASE_HI, 0U);

	Val = XPciePsu_ReadReg(PciePsuPtr->Config.BrigReg, DREG_CONTROL);
	Val |= DMA_ENABLE;
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg, DREG_CONTROL, Val);

	/* ECAM Configurations */
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg, XPCIEPSU_E_ECAM_BASE_LO,
			PciePsuPtr->Config.NpMemBaseAddr);
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			XPCIEPSU_E_ECAM_BASE_HI, 0U);

	/* ECAM Enable */
	Val = XPciePsu_ReadReg(PciePsuPtr->Config.BrigReg,
			XPCIEPSU_E_ECAM_CONTROL);
	Val |= ECAM_ENABLE;
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			XPCIEPSU_E_ECAM_CONTROL, Val);

	/* Enable AXI domain interrupt */
	XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
			DMA0_CHAN_AXI_INTR, AXI_INTR_ENABLE);
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg, MSGF_DMA_MASK,
			MSGF_DMA_INTR_ENABLE);
}

/******************************************************************************/
/**
* This function initializes Egress PCIe bridge.
*
* @param   PciePsuPtr pointer to XPciePsu Instance Pointer
*
*******************************************************************************/
void XPciePsu_Egress_EP_BridgeInitialize(XPciePsu *PciePsuPtr)
{
	u32 Val;
	Xil_AssertVoid(PciePsuPtr != NULL);

	Val = XPciePsu_ReadReg(PciePsuPtr->Config.BrigReg,0U);
	Val &= (u32)(~PCIE_DMA_REG_ACCESS_DISABLE);
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,0U, Val);

	/* Bridge Configurations */
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg, XPCIEPSU_E_BREG_BASE_LO,
			LOWER_32_BITS(PciePsuPtr->Config.BrigReg));
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg, XPCIEPSU_E_BREG_BASE_HI,
			UPPER_32_BITS(PciePsuPtr->Config.BrigReg));

	Val = XPciePsu_ReadReg(PciePsuPtr->Config.BrigReg,
			XPCIEPSU_E_BREG_CONTROL);
	Val &= ~(BREG_SIZE_MASK | BREG_ENABLE_FORCE);
	Val |= ((u32)BREG_SIZE) << BREG_SIZE_SHIFT;
	Val |= BREG_ENABLE;
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			XPCIEPSU_E_BREG_CONTROL, Val);

	/* DMA regs Configurations */
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg, DREG_BASE_LO,
			PciePsuPtr->Config.DmaBaseAddr);
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg, DREG_BASE_HI, 0U);

	Val = XPciePsu_ReadReg(PciePsuPtr->Config.BrigReg, DREG_CONTROL);
	Val |= DMA_ENABLE;
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg, DREG_CONTROL, Val);

	/* ECAM Configurations */
	Val = (u32)(ECAM_BASE);
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg, XPCIEPSU_E_ECAM_BASE_LO,
			Val);

	Val= UPPER_32_BITS(ECAM_BASE);
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			XPCIEPSU_E_ECAM_BASE_HI, Val);

	/* ECAM Enable */
	Val = XPciePsu_ReadReg(PciePsuPtr->Config.BrigReg,
			XPCIEPSU_E_ECAM_CONTROL);
	Val |= ECAM_ENABLE;
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			XPCIEPSU_E_ECAM_CONTROL, Val);

	 /* Enable AXI domain interrupt */
        XPciePsu_WriteReg(PciePsuPtr->Config.DmaBaseAddr,
                        DMA0_CHAN_AXI_INTR + 0x80, AXI_INTR_ENABLE);
        XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg, MSGF_DMA_MASK + 0x80,
                        MSGF_DMA_INTR_ENABLE);


}
/******************************************************************************/
/**
* This function waits for Pcie link to come up.
*
* @param   PciePsuPtr pointer to XPciePsu_Config Instance Pointer
*
*******************************************************************************/
void XPciePsu_EP_WaitForLinkup(XPciePsu *PciePsuPtr)
{
	Xil_AssertVoid(PciePsuPtr != NULL);
	u32 Val;
	do {
		Val = XPciePsu_ReadReg(PciePsuPtr->Config.PciReg,
				XPCIEPSU_PS_LINKUP_OFFSET);
	} while ((Val & PCIE_LINK_UP) == 0U);
}
/******************************************************************************/
/**
* This function waits for host to enumerate.
* On x86 BIOS sets memory enable bit
* On ARM64 it is set when driver gets inserted
*
* @param   PciePsuPtr pointer to XPciePsu_Config Instance Pointer
*
*******************************************************************************/
void XPciePsu_EP_WaitForEnumeration(XPciePsu *PciePsuPtr)
{
	Xil_AssertVoid(PciePsuPtr != NULL);
	u32 Val;
	do {
			Val = XPciePsu_ReadReg(PciePsuPtr->Config.NpMemBaseAddr,
					COMMAND_REG);
			usleep(ENUMERATION_WAIT_TIME);
	} while ((Val & PCIE_ENUMERATED_STATUS) == 0U);
}
/******************************************************************************/
/**
* This function read bar address of specific bar number
*
* @param	PciePsuPtr pointer to XPciePsu_Config Instance Pointer
* @param	BarNum
* @param	BarLo
* @param	BarHi
*
*******************************************************************************/
static void XPciePSU_ReadBar(XPciePsu *PciePsuPtr, u32 BarNum, u32 *BarLo,
		u32 *BarHi)
{
	Xil_AssertVoid(PciePsuPtr != NULL);
	u32 Offset =  (BAR0_OFFSET_LO + (BarNum * 0x4U));

	*BarLo = XPciePsu_ReadReg(PciePsuPtr->Config.NpMemBaseAddr, Offset);
	Offset = (BAR0_OFFSET_HI + (BarNum * 0x4U));
	*BarHi = XPciePsu_ReadReg(PciePsuPtr->Config.NpMemBaseAddr, Offset);

	XPciePsu_Dbg("BAR%d LO configured by host 0x%08X\r\n", BarNum, *BarLo);
	XPciePsu_Dbg("BAR%d HI configured by host 0x%08X\r\n", BarNum, *BarHi);
}
/******************************************************************************/
/**
* This function does Egress test
*
* @param	egress source address
*
****************************************************************************/

void Do_Egress_Test(u32 *egress_src_lo)
{
	u32 buf[2];

	xil_printf("Inside of egress test: egress_src_lo: %llx\n", egress_src_lo);
	buf[0] = *egress_src_lo;

	xil_printf("Pre: Value read from offset 0 => 0x%x\n", buf[0]);
	xil_printf("Pre: Value read from offset 4 => 0x%x\n", buf[1]);

	*egress_src_lo = 0xdeadbeef;
	*(egress_src_lo + 1) = 0xbeefdead;

	buf[0] = *egress_src_lo;
	buf[1] = *(egress_src_lo + 1);

	xil_printf("Value read from offset 0 => 0x%x\n", buf[0]);
	xil_printf("Value read from offset 4 => 0x%x\n", buf[1]);

	if (buf[0] == 0xdeadbeef && buf[1] == 0xbeefdead) {
		xil_printf("PCIe Egress Test Pass!\n");
	} else {
		xil_printf("PCIe Egress Test Fail\n");
		xil_printf("Values written => 0xdeadbeef, 0xbeefdead\n");
		xil_printf("Values read => 0x%x, 0x%x\n", buf[0], buf[1]);
	}
}
/******************************************************************************/
/**
* This function sets up ingress translation
*
* @param	PciePsuPtr pointer to XPciePsu_Config Instance Pointer
* @param	IngressNum ingress must be 0 to 7
* @param	BarNum bar no to setup ingress
* @param	Dst 32 or 64 bit destination address
*
* @return	XST_SUCCESS if setup is successful
*			XST_FAILURE if setup is fail
*
*******************************************************************************/
int XPciePsu_EP_SetupIngress(XPciePsu *PciePsuPtr, u32 IngressNum, u32 BarNum,
		u64 Dst){
	Xil_AssertNonvoid(PciePsuPtr != NULL);
	u32 SrcLo;
	u32 SrcHi;
	u32 Val;
	u32 DestLo;
	u32 DestHi;
	if (IngressNum > 7U) {
		return (s32)XST_FAILURE;
	}

	XPciePSU_ReadBar(PciePsuPtr, BarNum, &SrcLo, &SrcHi);

	/*
	 * Using Ingress Address Translation 0 to setup translation
	 * to PS DDR
	 */

	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			(INGRESS0_SRC_BASE_LO + (IngressNum * INGRESS_SIZE)),
			SrcLo & ~0xfU);
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			(INGRESS0_SRC_BASE_HI +
			(IngressNum * INGRESS_SIZE)), SrcHi);

	XPciePsu_Dbg("Done writing the Ingress Src registers\r\n");

	DestLo = LOWER_32_BITS(Dst);
	DestHi = UPPER_32_BITS(Dst);

	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			(INGRESS0_DST_BASE_LO +
			(IngressNum * INGRESS_SIZE)), DestLo);
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			(INGRESS0_DST_BASE_HI +
			(IngressNum * INGRESS_SIZE)), DestHi);

	XPciePsu_Dbg("Done writing the Ingress Dst registers\r\n");

	Val = XPciePsu_ReadReg(PciePsuPtr->Config.BrigReg, INGRESS0_CONTROL);

	XPciePsu_Dbg("Read Ingress Control register\r\n");

	Val &= (u32)(~INGRESS_SIZE_MASK);
	Val |= (((u32)INGRESS_SIZE_ENCODING << INGRESS_SIZE_SHIFT) |
		(u32)INGRESS_ENABLE | (u32)INGRESS_SECURITY_ENABLE);
	Val |= INGRESS_RD_WR_ATTR;

	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			(INGRESS0_CONTROL + (IngressNum * INGRESS_SIZE)), Val);

	XPciePsu_Dbg("Done setting up the ingress trasnslation registers\r\n");
	return (s32)XST_SUCCESS;
}


/******************************************************************************/
/**
* This function sets up Egress translation
*
* @param	PciePsuPtr pointer to XPciePsu_Config Instance Pointer
* @param	EgressNum ingress must be 0 to 7
*
* @return	XST_SUCCESS if setup is successful
*			XST_FAILURE if setup is fail
*
*******************************************************************************/
int XPciePsu_EP_SetupEgress(XPciePsu *PciePsuPtr, u32 EgressNum ){
	Xil_AssertNonvoid(PciePsuPtr != NULL);
	u32 SrcLo;
	u32 SrcHi;
	u32 Val;
	u32 Scratch0, Scratch1;

	/*
	 * Using Ingress Address Translation 0 to setup translation
	 * to PS DDR
	 */

	SrcLo = LOWER_32_BITS(EGRESS_SRC_LO);
	SrcHi = UPPER_32_BITS(EGRESS_SRC_HI);

	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			(EGRESS0_SRC_BASE_LO + (EgressNum * EGRESS_SIZE)),
			SrcLo & ~0xfU);

	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			(EGRESS0_SRC_BASE_HI + (EgressNum * EGRESS_SIZE)), SrcHi);

	XPciePsu_Dbg("Done writing the Ingress Src registers\r\n");

	Scratch0 = (PciePsuPtr->Config.DmaBaseAddr + EGRESS_BUF_ADDR0_OFFSET);
	Scratch1 = (PciePsuPtr->Config.DmaBaseAddr + EGRESS_BUF_ADDR1_OFFSET);

	Val = XPciePsu_ReadReg(Scratch0, 0U);

	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			(EGRESS0_DST_BASE_LO + (EgressNum * EGRESS_SIZE)), Val);

	Val = XPciePsu_ReadReg(Scratch1, 0U);
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			(EGRESS0_DST_BASE_HI +
			(EgressNum * EGRESS_SIZE)), Val);

	XPciePsu_Dbg("Done writing the Ingress Dst registers\r\n");

	Val = XPciePsu_ReadReg(PciePsuPtr->Config.BrigReg, EGRESS0_CONTROL);

	Val &= (u32)(~EGRESS_SIZE_MASK);
	Val |= (((u32)EGRESS_SIZE_ENCODING << EGRESS_SIZE_SHIFT) |
		(u32)EGRESS_ENABLE | (u32)EGRESS_SECURITY_ENABLE);

	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			(EGRESS0_CONTROL + (EgressNum * EGRESS_SIZE)) , Val);

	XPciePsu_Dbg("Done setting up the Egress trasnslation registers\r\n");

	Do_Egress_Test((u32 *) EGRESS_SRC_LO);

	return (s32)XST_SUCCESS;

}
