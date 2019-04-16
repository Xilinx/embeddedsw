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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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
		XPciePsu_Config *ConfigPtr){
	Xil_AssertVoid(ConfigPtr != NULL);
	memset(PciePsuPtr, 0, sizeof(XPciePsu));
	memcpy(&(PciePsuPtr->Config), ConfigPtr, sizeof(XPciePsu_Config));
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
			PciePsuPtr->Config.BrigReg);
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			XPCIEPSU_E_BREG_BASE_HI, 0U);

	Val = XPciePsu_ReadReg(PciePsuPtr->Config.BrigReg,
			XPCIEPSU_E_BREG_CONTROL);
	Val &= ~(BREG_SIZE_MASK | BREG_ENABLE_FORCE);
	Val |= (BREG_SIZE << BREG_SIZE_SHIFT);
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
* This function waits for Pcie link to come up.
*
* @param   PciePsuPtr pointer to XPciePsu_Config Instance Pointer
*
*******************************************************************************/
void XPciePsu_EP_WaitForLinkup(XPciePsu *PciePsuPtr)
{
	Xil_AssertVoid(PciePsuPtr != NULL);
	int Val;
	do {
		Val = XPciePsu_ReadReg(PciePsuPtr->Config.PciReg,
				XPCIEPSU_PS_LINKUP_OFFSET);
	} while (!(Val & PCIE_LINK_UP));
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
	int Val;
	do {
			Val = XPciePsu_ReadReg(PciePsuPtr->Config.NpMemBaseAddr,
					COMMAND_REG);
			usleep(ENUMERATION_WAIT_TIME);
	} while (!(Val & PCIE_ENUMERATED_STATUS));
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
void XPciePSU_ReadBar(XPciePsu *PciePsuPtr, u32 BarNum, u32 *BarLo,
		u32 *BarHi)
{
	Xil_AssertVoid(PciePsuPtr != NULL);
	u32 Offset =  (BAR0_OFFSET_LO + (BarNum * 0x4));
	*BarLo = XPciePsu_ReadReg(PciePsuPtr->Config.NpMemBaseAddr, Offset);
	Offset = (BAR0_OFFSET_HI + (BarNum * 0x4));
	*BarHi = XPciePsu_ReadReg(PciePsuPtr->Config.NpMemBaseAddr, Offset);

	XPciePsu_Dbg("BAR%d LO configured by host 0x%08X\r\n", BarNum, *BarLo);
	XPciePsu_Dbg("BAR%d HI configured by host 0x%08X\r\n", BarNum, *BarHi);
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
* @retun	XST_SUCCESS if setup is successful
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
	if (IngressNum > 7) {
		return XST_FAILURE;
	}

	XPciePSU_ReadBar(PciePsuPtr, BarNum, &SrcLo, &SrcHi);

	/*
	 * Using Ingress Address Translation 0 to setup translation
	 * to PS DDR
	 */
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			(INGRESS0_SRC_BASE_LO + (IngressNum * INGRESS_SIZE)),
			SrcLo & ~0xf);
	XPciePsu_WriteReg(PciePsuPtr->Config.BrigReg,
			(INGRESS0_SRC_BASE_HI +
			(IngressNum * INGRESS_SIZE)), SrcHi);

	XPciePsu_Dbg("Done writing the Ingress Src registers\r\n");

	DestLo = XPCIEPSU_LOWER32BITS(Dst);
	DestHi = XPCIEPSU_UPPER32BITS(Dst);
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
	return XST_SUCCESS;
}
