/******************************************************************************
*
* Copyright (C) 2011 - 2014 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
******************************************************************************/
/*****************************************************************************
**
* @file xaxipcie.c
* @addtogroup axipcie_v3_1
* @{
*
* Implements all of functions for XAxiPcie IP driver except interrupts and
* initialization.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.00a rkv  03/03/11  Original code.
* 2.00a nm   10/19/11  Added support of pcie root complex functionality.
*		       Changed these functions
* 		       	-renamed function XAxiPcie_GetRequestId to
*		        XAxiPcie_GetRequesterId
*		       	-added two functions arguments RootPortPtr &
*			ECAMSizePtr to XAxiPcie_GetBridgeInfo API
*		       Added these new API for root complex support
*		       	- XAxiPcie_SetRequesterId
*			- XAxiPcie_GetRootPortStatusCtrl
*			- XAxiPcie_SetRootPortStatusCtrl
*			- XAxiPcie_SetRootPortMSIBase
*			- XAxiPcie_GetRootPortErrFIFOMsg
*			- XAxiPcie_ClearRootPortErrFIFOMsg
*			- XAxiPcie_GetRootPortIntFIFOReg
*			- XAxiPcie_ClearRootPortIntFIFOReg
*			- XAxiPcie_WriteLocalConfigSpace
*			- XAxiPcie_ComposeExternalConfigAddress
*			- XAxiPcie_ReadRemoteConfigSpace
*			- XAxiPcie_WriteRemoteConfigSpace
*
* 2.01a nm   04/01/12  Removed XAxiPcie_SetRequesterId and
*		       XAxiPcie_SetBlPortNumber APIs as these are writing
*		       to Read Only bits for CR638299.
* 2.02a nm   08/01/12  Updated for removing compilation errors with C++,
*		       changed XCOMPONENT_IS_READY to XIL_COMPONENT_IS_READY
*		       Removed the Endian Swap in
*		       XAxiPcie_ReadRemoteConfigSpace and
*		       XAxiPcie_WriteRemoteConfigSpace APIs as the HW
*		       has been fixed and the swapping is not required
*		       in  the driver (CR 657412)
* 2.03a srt  04/13/13  Removed Warnings (CR 705004).
* 3.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototype of XAxiPcie_CfgInitialize API.
*
* </pre>
*
*****************************************************************************/

/****************************** Include Files *******************************/
#include "xaxipcie.h"

/*************************** Constant Definitions ***************************/

/***************************** Type Definitions *****************************/

/****************** Macros (Inline Functions) Definitions *******************/

/*************************** Variable Definitions ***************************/

/*************************** Function Prototypes ****************************/

/****************************************************************************/
/**
* Initialize the XAxiPcie instance provided by the caller based on the
* given Config structure.
*
*
* @param	InstancePtr is the XAxiPcie instance to operate on.The memory
*		of the pointer references must be pre-allocated by the caller.
* @param	CfgPtr is the device configuration structure containing
* 		required HW build data.
* @param	EffectiveAddress is the Physical address of the hardware in a
* 		Virtual Memory operating system environment.It is the Base
* 		Address in a stand alone environment.
*
* @return
*
* 		- XST_SUCCESS Initialization was successful.
*
* @note		None.
*
******************************************************************************/
int XAxiPcie_CfgInitialize(XAxiPcie *InstancePtr, XAxiPcie_Config *CfgPtr,
							 UINTPTR EffectiveAddress)
{
	u32 Data;

	/* Assert arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);

	/* Clear instance memory and make copy of configuration */
	memset(InstancePtr, 0, sizeof(XAxiPcie));
	memcpy(&InstancePtr->Config, CfgPtr, sizeof(XAxiPcie_Config));

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	InstancePtr->Config.BaseAddress = EffectiveAddress;

	/* Disable all interrupts */
	XAxiPcie_DisableInterrupts(InstancePtr, XAXIPCIE_IM_DISABLE_ALL_MASK);

	/* Max number of buses */
	Data = XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XAXIPCIE_BI_OFFSET);
	InstancePtr->MaxNumOfBuses = (u16)((Data & XAXIPCIE_BI_ECAM_SIZE_MASK) >>
					XAXIPCIE_BI_ECAM_SIZE_SHIFT);

	return (XST_SUCCESS);
}

/****************************************************************************/
/**
* This API is used to read the VSEC Capability Register.
*
* @param	InstancePtr is the XAxiPcie instance to operate on.
* @param	VsecNum is a VSEC register number as there are two registers.
*		Possible values are.
*			- XAXIPCIE_VSEC1 (0)
*			- XAXIPCIE_VSEC2 (1)
* @param	VsecIdPtr is a pointer to a variable where the driver will pass
*		back the Vendor Specific Enhanced Capability ID.
* @param	VersionPtr is a pointer to a variable where the driver will
*.		pass back the Version of VSEC.
* @param	NextCapPtr is a pointer to a variable where the driver will
*		pass back the Next Capability offset.
*
* @return	None.
*
* @note 	None
*
*****************************************************************************/
void XAxiPcie_GetVsecCapability(XAxiPcie *InstancePtr, u8 VsecNum,
			 u16 *VsecIdPtr, u8 *VersionPtr, u16 *NextCapPtr)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(VsecIdPtr != NULL);
	Xil_AssertVoid(VersionPtr != NULL);
	Xil_AssertVoid(NextCapPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
			(XAXIPCIE_VSECC_OFFSET +
			(XAXIPCIE_VSEC2_OFFSET_WRT_VSEC1 * VsecNum)));

	*VsecIdPtr = (u16)(Data & XAXIPCIE_VSECC_ID_MASK);

	*VersionPtr = (u8)((Data & XAXIPCIE_VSECC_VER_MASK) >>
						XAXIPCIE_VSECC_VER_SHIFT);

	*NextCapPtr = (u16)((Data & XAXIPCIE_VSECC_NEXT_MASK) >>
						XAXIPCIE_VSECC_NEXT_SHIFT);


}

/****************************************************************************/
/**
* This API is used to read the VSEC Header Register.
*
* @param	InstancePtr is the XAxiPcie instance to operate on.
* @param	VsecNum is a VSEC register number as there are two registers.
*		Possible values are.
*			- XAXIPCIE_VSEC1 (0)
*			- XAXIPCIE_VSEC2 (1)
* @param	VsecIdPtr is a pointer to a variable where the driver will pass
*		back the VSEC header structure Id.
* @param	RevisionPtr is a pointer to a variable where the driver will
* 		pass back the Revision of VSEC capability Structure.
* @param	LengthPtr is a pointer to a variable where the driver will pass
*.		back the length of the VSEC capability structure.
*
* @return	None.
*
* @note 	None
*
*****************************************************************************/
void XAxiPcie_GetVsecHeader(XAxiPcie *InstancePtr, u8 VsecNum, u16 *VsecIdPtr,
				 u8 *RevisionPtr, u16 *LengthPtr)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(VsecIdPtr != NULL);
	Xil_AssertVoid(RevisionPtr != NULL);
	Xil_AssertVoid(LengthPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	Data = XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
			(XAXIPCIE_VSECH_OFFSET +
			(XAXIPCIE_VSEC2_OFFSET_WRT_VSEC1 * VsecNum)));

	*VsecIdPtr = (u16)(Data & XAXIPCIE_VSECH_ID_MASK);

	*RevisionPtr = (u8)((Data & XAXIPCIE_VSECH_REV_MASK) >>
						XAXIPCIE_VSECH_REV_SHIFT);

	*LengthPtr = (u16)((Data & XAXIPCIE_VSECH_LEN_MASK) >>
						XAXIPCIE_VSECH_LEN_SHIFT);


}

/****************************************************************************/
/**
* This API Reads the Bridge info register.
*
* @param	InstancePtr is the XAxiPcie instance to operate on.
* @param	Gen2Ptr is a pointer to a variable indicating whether
*		underlying PCIe block support PCIe Gen2 Speed.
* @param	RootPortPtr is a pointer to a variable indication whether
*		underlying PCIe block is root port.
* @param	ECAMSizePtr is a pointer to a variable where it indicates ECAM
*		size. Value is between 1 to 8. Total address bits dedicated to
*		ECAM is 20 + ECAM size.
*
* @return	None.
*
* @note 	None
*
*****************************************************************************/
void XAxiPcie_GetBridgeInfo(XAxiPcie *InstancePtr, u8 *Gen2Ptr,
					 u8 *RootPortPtr, u8 *ECAMSizePtr)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Gen2Ptr != NULL);
	Xil_AssertVoid(RootPortPtr != NULL);
	Xil_AssertVoid(ECAMSizePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XAXIPCIE_BI_OFFSET);

	*Gen2Ptr = (u8)(Data & XAXIPCIE_BI_GEN2_MASK);

	*RootPortPtr = (u8)((Data & XAXIPCIE_BI_RP_MASK) >>
							XAXIPCIE_BI_RP_SHIFT);

	*ECAMSizePtr = (u16)((Data & XAXIPCIE_BI_ECAM_SIZE_MASK) >>
					XAXIPCIE_BI_ECAM_SIZE_SHIFT);


}

/****************************************************************************/
/**
* Read the Bus Location register.
*
* @param	InstancePtr is the XAxiPcie instance to operate on.
* @param	BusNumPtr is a pointer to a variable where the driver will pass
* 		back the bus number of requester ID assigned to IP.
* @param	DevNumPtr is a pointer to a variable where the driver will pass
* 		back the device number of requester ID assigned to IP.
* @param	FunNumPtr is a pointer to a variable where the driver will pass
* 		back the function number of requester ID assigned to IP.
* @param	PortNumPtr is a pointer to a variable where the driver will
* 		pass back the Port number of requester ID assigned to IP.
*
* @return	None.
*
* @note 	None
*
*****************************************************************************/
void XAxiPcie_GetRequesterId(XAxiPcie *InstancePtr, u8 *BusNumPtr,
				 u8 *DevNumPtr, u8 *FunNumPtr, u8 *PortNumPtr)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(BusNumPtr != NULL);
	Xil_AssertVoid(DevNumPtr != NULL);
	Xil_AssertVoid(FunNumPtr != NULL);
	Xil_AssertVoid(PortNumPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XAXIPCIE_BL_OFFSET);

	*BusNumPtr = (u8)((Data & XAXIPCIE_BL_BUS_MASK) >>
							XAXIPCIE_BL_BUS_SHIFT);

	*DevNumPtr = (u8)((Data & XAXIPCIE_BL_DEV_MASK) >>
							XAXIPCIE_BL_DEV_SHIFT);

	*FunNumPtr = (u8)(Data & XAXIPCIE_BL_FUNC_MASK);


	*PortNumPtr = (u8)((Data & XAXIPCIE_BL_PORT_MASK) >>
						XAXIPCIE_BL_PORT_SHIFT);

}

/****************************************************************************/
/**
* This API is used to read the Phy Status/Control Register.
*
* @param	InstancePtr is the XAxiPcie instance to operate on.
* @param	PhyState is a pointer to a variable where the driver will
* 		pass back Current physical status.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XAxiPcie_GetPhyStatusCtrl(XAxiPcie *InstancePtr, u32 *PhyState)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(PhyState != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
				XAXIPCIE_PHYSC_OFFSET);

	*PhyState = Data;
}

/****************************************************************************/
/**
* Read Root Port Status/Control Register.
*
* @param	InstancePtr is the PCIe component to operate on.
* @param	StatusPtr is a pointer to a variable where the driver will
*		pass back the root port status.
*
* @return	None.
*
* @note		This function is valid only when IP is configured as a
*		root complex.
*
*****************************************************************************/
void XAxiPcie_GetRootPortStatusCtrl(XAxiPcie *InstancePtr, u32 *StatusPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(StatusPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeRootComplex ==
							XAXIPCIE_IS_RC);

	*StatusPtr = XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XAXIPCIE_RPSC_OFFSET);
}

/****************************************************************************/
/**
* Write Value in Root Port Status/Control Register.
*
* @param 	InstancePtr is the PCIe component to operate on.
* @param 	StatusData is data to set.
*
* @return 	None.
*
* @note		This function is valid only when IP is configured as a
*		root complex.
*
*****************************************************************************/
void XAxiPcie_SetRootPortStatusCtrl(XAxiPcie *InstancePtr, u32 StatusData)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeRootComplex ==
							XAXIPCIE_IS_RC);


	XAxiPcie_WriteReg((InstancePtr->Config.BaseAddress),
	        	XAXIPCIE_RPSC_OFFSET,StatusData & XAXIPCIE_RPSC_MASK);

}

/****************************************************************************/
/**
* Write MSI Base Address to Root Port MSI Base Address Register.
*
* @param	InstancePtr is the PCIe component to operate on.
* @param	MsiBase is 64 bit base address for MSI.This address should be
*		4kB aligned always.
*
* @return 	XST_SUCCESS if success or XST_FAILURE if failure .
*
* @note		This function is valid only when IP is configured as a
*		root complex.
*
*****************************************************************************/
int XAxiPcie_SetRootPortMSIBase(XAxiPcie *InstancePtr,
						 unsigned long long MsiBase)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->Config.IncludeRootComplex ==
							XAXIPCIE_IS_RC);

	Data = (u32)((MsiBase >> XAXIPCIE_RPMSIB_UPPER_SHIFT) &
						XAXIPCIE_RPMSIB_UPPER_MASK);

	XAxiPcie_WriteReg((InstancePtr->Config.BaseAddress),
					XAXIPCIE_RPMSIB_UPPER_OFFSET, Data);

	/* Check 4kB alignment of supplied MSI base address */
	if(MsiBase & ~(ALIGN_4KB))
		return XST_FAILURE;

	Data = (u32)(MsiBase & XAXIPCIE_RPMSIB_LOWER_MASK);
	XAxiPcie_WriteReg((InstancePtr->Config.BaseAddress),
					XAXIPCIE_RPMSIB_LOWER_OFFSET, Data);


	return XST_SUCCESS;
}

/****************************************************************************/
/**
* Read Root Port Error FIFO Message
*
* @param 	InstancePtr is the PCIe component to operate on.
* @param	ReqIdPtr is a variable where the driver will pass back the
*		requester Id of error message.
* @param	ErrType is a variable where the driver will pass back the
*		type of error message
* @param	ErrValid is a variable where the driver will pass back the
*		status of read operation of error message.
*
* @return	None.
*
* @note		This function is valid only when IP is configured as a
*		root complex.
*
*****************************************************************************/
void XAxiPcie_GetRootPortErrFIFOMsg(XAxiPcie *InstancePtr, u16 *ReqIdPtr,
					 u8 *ErrType, u8 *ErrValid)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ReqIdPtr != NULL);
	Xil_AssertVoid(ErrType != NULL);
	Xil_AssertVoid(ErrValid != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeRootComplex ==
							XAXIPCIE_IS_RC);

	Data = XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XAXIPCIE_RPEFR_OFFSET);

	*ReqIdPtr = (u16)(Data & XAXIPCIE_RPEFR_REQ_ID_MASK);

	*ErrType = (u8)((Data & XAXIPCIE_RPEFR_ERR_TYPE_MASK) >>
						XAXIPCIE_RPEFR_ERR_TYPE_SHIFT);

	*ErrValid = (u8)((Data & XAXIPCIE_RPEFR_ERR_VALID_MASK) >>
					XAXIPCIE_RPEFR_ERR_VALID_SHIFT);
}

/****************************************************************************/
/**
* Clear Root Port Error FIFO Message
*
* @param 	InstancePtr is the PCIe component to operate on.
*
* @return 	None.
*
* @note		This function is valid only when IP is configured as a
*		root complex.
*
*****************************************************************************/
void XAxiPcie_ClearRootPortErrFIFOMsg(XAxiPcie *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeRootComplex ==
							XAXIPCIE_IS_RC);

	XAxiPcie_WriteReg((InstancePtr->Config.BaseAddress),
					XAXIPCIE_RPEFR_OFFSET, 0x7FFF);

}

/****************************************************************************/
/**
* Read Root Port Interrupt FIFO message Register 1 & 2.
*
* @param 	InstancePtr is the PCIe component to operate on.
* @param	ReqIdPtr is a variable where the driver will pass back the
*		requester Id of error message.
* @param	MsiAddr is a variable where the driver will pass back the
*		MSI address for which interrupt message recieved.
* @param	MsiInt is a variable where the driver will pass back the
*		type of interrupt message recieved (MSI/INTx).
* @param	IntValid is a variable where the driver will pass back the
*		status of read operation of interrupt message.
* @param	MsiMsgData is a variable where the driver will pass back the
*		MSI data recieved.
*
* @return 	MsiMsgData if MSI interrupt is observed or
*		0 if there is no MSI interrupt.
*
* @note 	This function is valid only when IP is configured as a
*		root complex.
*
*****************************************************************************/
int XAxiPcie_GetRootPortIntFIFOReg(XAxiPcie *InstancePtr, u16 *ReqIdPtr,
	 u16 *MsiAddr, u8 *MsiInt, u8 *IntValid, u16 *MsiMsgData)
{
	u32 Data = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ReqIdPtr != NULL);
	Xil_AssertNonvoid(MsiAddr != NULL);
	Xil_AssertNonvoid(MsiInt != NULL);
	Xil_AssertNonvoid(IntValid != NULL);
	Xil_AssertNonvoid(MsiMsgData != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->Config.IncludeRootComplex ==
							XAXIPCIE_IS_RC);

	*MsiMsgData = 0;

	Data = XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
						XAXIPCIE_RPIFR1_OFFSET);

	*ReqIdPtr = (u16)(Data & XAXIPCIE_RPIFR1_REQ_ID_MASK);

	*MsiAddr = (u8)((Data & XAXIPCIE_RPIFR1_MSI_ADDR_MASK) >>
					XAXIPCIE_RPIFR1_MSI_ADDR_SHIFT);

	*MsiInt = (u8)((Data & XAXIPCIE_RPIFR1_MSIINTR_VALID_MASK) >>
					XAXIPCIE_RPIFR1_MSIINTR_VALID_SHIFT);

	*IntValid = (u8)((Data &  XAXIPCIE_RPIFR1_INTR_VALID_MASK) >>
					XAXIPCIE_RPIFR1_INTR_VALID_SHIFT);

	if(*MsiInt & *IntValid)
	{

		Data = XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
						XAXIPCIE_RPIFR2_OFFSET);

		*MsiMsgData = (u16)(Data & XAXIPCIE_RPIFR2_MSG_DATA_MASK);
	}

	return *MsiMsgData;
}

/****************************************************************************/
/**
* Clear Root Port FIFO Interrupt message Register 1 & 2.
*
* @param 	InstancePtr is the PCIe component to operate on
*
* @return 	None.
*
* @note		This function is valid only when IP is configured as a
*		root complex.Clearing any one Interrupt FIFO register clears
*		both registers.
*
*****************************************************************************/
void XAxiPcie_ClearRootPortIntFIFOReg(XAxiPcie *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeRootComplex ==
							XAXIPCIE_IS_RC);

	XAxiPcie_WriteReg((InstancePtr->Config.BaseAddress),
					XAXIPCIE_RPIFR1_OFFSET,0xA7FFFFFF);

}

/****************************************************************************/
/**
* Read PCIe address translation vector that corresponds to one of AXI local
* bus bars passed by the caller.
*
* @param 	InstancePtr is the XAxiPcie instance to operate on.
* @param 	BarNumber is AXI bar number (0 - 5) passed by caller.
* @param 	BarAddrPtr is a pointer to a variable where the driver will
*.		pass back translation vector.
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
void XAxiPcie_GetLocalBusBar2PcieBar(XAxiPcie *InstancePtr, u8 BarNumber,
						 XAxiPcie_BarAddr *BarAddrPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(BarAddrPtr != NULL);
	Xil_AssertVoid(BarNumber < InstancePtr->Config.LocalBarsNum);
	Xil_AssertVoid(InstancePtr->Config.IncludeBarOffsetReg != FALSE);

	BarAddrPtr->LowerAddr =
		XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
		(XAXIPCIE_AXIBAR2PCIBAR_0L_OFFSET +
		(BarNumber * (sizeof(u32) * 2))));

	BarAddrPtr->UpperAddr =
		XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
		(XAXIPCIE_AXIBAR2PCIBAR_0U_OFFSET +
		(BarNumber * (sizeof(u32) * 2))));

}

/****************************************************************************/
/**
* Write PCIe address translation vector that corresponds to one of AXI local
* bus bars passed by the caller.
*
* @param 	InstancePtr is the XAxiPcie instance to operate on.
* @param 	BarNumber is AXI bar number (0 - 5) passed by caller.
* @param 	BarAddrPtr is a pointer to a variable where the driver will
* 		pass back translation vector.
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
void XAxiPcie_SetLocalBusBar2PcieBar(XAxiPcie *InstancePtr, u8 BarNumber,
						 XAxiPcie_BarAddr *BarAddrPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(BarAddrPtr != NULL);
	Xil_AssertVoid(BarNumber < InstancePtr->Config.LocalBarsNum);
	Xil_AssertVoid(InstancePtr->Config.IncludeBarOffsetReg != FALSE);

	XAxiPcie_WriteReg(InstancePtr->Config.BaseAddress,
		(XAXIPCIE_AXIBAR2PCIBAR_0L_OFFSET +
		(BarNumber * (sizeof(u32) * 2))), (BarAddrPtr->LowerAddr));

	XAxiPcie_WriteReg(InstancePtr->Config.BaseAddress,
		(XAXIPCIE_AXIBAR2PCIBAR_0U_OFFSET +
		(BarNumber * (sizeof(u32) * 2))), (BarAddrPtr->UpperAddr));

}

/****************************************************************************/
/**
* Read 32-bit value from one of this IP own configuration space.
* Location is identified by its offset from the beginning of the
* configuration space.
*
* @param 	InstancePtr is the XAxiPcie instance to operate on.
* @param 	Offset from beginning of IP own configuration space.
* @param 	DataPtr is a pointer to a variable where the driver will pass
* 		back the value read from the specified location.
*
* @return 	None
*
* @note 	None
*
*****************************************************************************/
void XAxiPcie_ReadLocalConfigSpace(XAxiPcie *InstancePtr, u16 Offset,
								 u32 *DataPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(DataPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	*DataPtr = XAxiPcie_ReadReg((InstancePtr->Config.BaseAddress),
			(XAXIPCIE_PCIE_CORE_OFFSET + ((u32) (Offset * 4))));

}

/****************************************************************************/
/**
* Write 32-bit value to one of this IP own configuration space.
* Location is identified by its offset from the begginning of the
* configuration space.
*
* @param 	InstancePtr is the PCIe component to operate on.
* @param 	Offset from beggininng of IP own configuration space.
* @param 	Data to be written to the specified location.
*
* @return 	None
*
* @note 	This function is valid only when IP is configured as a
*		root complex.
*
*****************************************************************************/
void XAxiPcie_WriteLocalConfigSpace(XAxiPcie *InstancePtr, u16 Offset,
								 u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeRootComplex ==
							XAXIPCIE_IS_RC);

	XAxiPcie_WriteReg((InstancePtr->Config.BaseAddress),
		(XAXIPCIE_PCIE_CORE_OFFSET + ((u32) (Offset * 4))), Data);

}

/****************************************************************************/
/*
* Compose an address to be written to configuration address port
*
* @param 	Bus is the external PCIe function's Bus number.
* @param 	Device is the external PCIe function's Device number.
* @param 	Function is the external PCIe function's Function number.
* @param 	Offset from beggininng of PCIe function's configuration space.

* @return 	32 bit composed value (address).
*
* @note 	This function is valid only when IP is configured as a
*		root complex.
*
*****************************************************************************/
u32 XAxiPcie_ComposeExternalConfigAddress(u8 Bus, u8 Device, u8 Function,
								 u16 Offset)
{
	u32 Location = 0;

	Location |= ((((u32)Bus) << XAXIPCIE_ECAM_BUS_SHIFT) &
						XAXIPCIE_ECAM_BUS_MASK);

	Location |= ((((u32)Device) << XAXIPCIE_ECAM_DEV_SHIFT) &
						XAXIPCIE_ECAM_DEV_MASK);

	Location |= ((((u32)Function) << XAXIPCIE_ECAM_FUN_SHIFT) &
						XAXIPCIE_ECAM_FUN_MASK);

	Location |= ((((u32)Offset) << XAXIPCIE_ECAM_REG_SHIFT) &
						XAXIPCIE_ECAM_REG_MASK);

	Location &= XAXIPCIE_ECAM_MASK;

	return Location;
}

/****************************************************************************/
/**
* Read 32-bit value from external PCIe Function's configuration space.
* External PCIe function is identified by its Requester ID (Bus#, Device#,
* Function#). Location is identified by its offset from the begginning of the
* configuration space.
*
* @param 	InstancePtr is the PCIe component to operate on.
* @param 	Bus is the external PCIe function's Bus number.
* @param 	Device is the external PCIe function's Device number.
* @param 	Function is the external PCIe function's Function number.
* @param 	Offset from beggininng of PCIe function's configuration space.
* @param 	DataPtr is a pointer to a variable where the driver will pass
* 		back the value read from the specified location.
*
* @return 	None
*
* @note 	This function is valid only when IP is configured as a
*		root complex. The XAxiPcie_ReadLocalConfigSpace API should
*		be used for reading the local config space.
*
*****************************************************************************/
void XAxiPcie_ReadRemoteConfigSpace(XAxiPcie *InstancePtr, u8 Bus, u8 Device,
		 u8 Function, u16 Offset, u32 *DataPtr)
{
	u32 Location = 0;
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(DataPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeRootComplex ==
							XAXIPCIE_IS_RC);

	if (((Bus == 0) && !((Device == 0) && (Function == 0))) ||
		(Bus > InstancePtr->MaxNumOfBuses)) {
		*DataPtr = 0xFFFFFFFF;
		return;
	}

	/* Compose function configuration space location */
	Location = XAxiPcie_ComposeExternalConfigAddress (Bus, Device,
							Function, Offset);

	while(XAxiPcie_IsEcamBusy(InstancePtr));

	/* Read data from that location */
	Data = XAxiPcie_ReadReg((InstancePtr->Config.BaseAddress),
								Location);
	*DataPtr = Data;

}

/****************************************************************************/
/**
* Write 32-bit value to external PCIe function's configuration space.
* External PCIe function is identified by its Requester ID (Bus#, Device#,
* Function#). Location is identified by its offset from the begginning of the
* configuration space.
*
* @param 	InstancePtr is the PCIe component to operate on.
* @param 	Bus is the external PCIe function's Bus number.
* @param 	Device is the external PCIe function's Device number.
* @param 	Function is the external PCIe function's Function number.
* @param 	Offset from beggininng of PCIe function's configuration space.
* @param 	Data to be written to the specified location.
*
* @return 	None
*
* @note 	This function is valid only when IP is configured as a
*		root complex. The XAxiPcie_WriteLocalConfigSpace should be
*		used for writing to local config space.
*
*****************************************************************************/
void XAxiPcie_WriteRemoteConfigSpace(XAxiPcie *InstancePtr, u8 Bus, u8 Device,
					 u8 Function, u16 Offset, u32 Data)
{
	u32 Location = 0;
	u32 TestWrite = 0;
	u8 Count = 3;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeRootComplex ==
							XAXIPCIE_IS_RC);

	if ((Bus == 0) || (Bus > InstancePtr->MaxNumOfBuses)) {
		return;
	}

	/* Compose function configuration space location */
	Location = XAxiPcie_ComposeExternalConfigAddress (Bus, Device,
							Function, Offset);
	while(XAxiPcie_IsEcamBusy(InstancePtr));


	/* Write data to that location */
	XAxiPcie_WriteReg((InstancePtr->Config.BaseAddress),
				Location , Data);


	/* Read data from that location to verify write */
	while (Count) {

		TestWrite =
			XAxiPcie_ReadReg((InstancePtr->Config.BaseAddress),
								Location);

		if (TestWrite == Data) {
			break;
		}

		Count--;
	}
}

/** @} */
