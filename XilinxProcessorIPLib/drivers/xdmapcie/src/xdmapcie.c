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
* @file xdmapcie.c
*
* Implements all of functions for XDmaPcie IP driver except interrupts and
* initialization.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0	tk	01/30/2019	First release
* </pre>
*
*****************************************************************************/

/****************************** Include Files *******************************/
#include "xdmapcie.h"
#include "xdmapcie_common.h"

/*************************** Constant Definitions ***************************/

/***************************** Type Definitions *****************************/

/****************** Macros (Inline Functions) Definitions *******************/

/*************************** Variable Definitions ***************************/

/*************************** Function Prototypes ****************************/



/****************************************************************************/
/**
* Initialize the XDmaPcie instance provided by the caller based on the
* given Config structure.
*
*
* @param	InstancePtr is the XDmaPcie instance to operate on.The memory
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
int XDmaPcie_CfgInitialize(XDmaPcie *InstancePtr, XDmaPcie_Config *CfgPtr,
							 UINTPTR EffectiveAddress)
{
	u32 Data;

	/* Assert arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);

	/* Clear instance memory and make copy of configuration */
	memset(InstancePtr, 0, sizeof(XDmaPcie));
	memcpy(&InstancePtr->Config, CfgPtr, sizeof(XDmaPcie_Config));

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	InstancePtr->Config.BaseAddress = EffectiveAddress;

	/* Disable all interrupts */
	XDmaPcie_DisableInterrupts(InstancePtr, XDMAPCIE_IM_DISABLE_ALL_MASK);

	/* Max number of buses */
	Data = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XDMAPCIE_BI_OFFSET);
	InstancePtr->MaxNumOfBuses = (u16)((Data & XDMAPCIE_BI_ECAM_SIZE_MASK) >>
					XDMAPCIE_BI_ECAM_SIZE_SHIFT);

	return (XST_SUCCESS);
}

/******************************************************************************/
/**
* This function reserves bar memory address.
*
* @param   InstancePtr pointer to XDmaPcie Instance Pointer
* @param   mem_type type of bar memory. address mem or IO.
* @param   mem_as bar memory tpye 32 or 64 bit
* @param   size	u64 size to increase
*
* @return  bar address
*
*******************************************************************************/
static u64 XDmaPcie_ReserveBarMem(XDmaPcie *InstancePtr, u8 MemType,
		u8 MemBarArdSize, u64 Size)
{
	u64 Ret = 0;

	if (MemType == XDMAPCIE_BAR_IO_MEM){
		Ret = XST_FAILURE;
		goto End;
	}

	if (MemBarArdSize == XDMAPCIE_BAR_MEM_TYPE_64) {
		Ret = InstancePtr->Config.PMemBaseAddr;
		InstancePtr->Config.PMemBaseAddr = InstancePtr->Config.PMemBaseAddr
							+ Size;
		Xil_AssertNonvoid(InstancePtr->Config.PMemBaseAddr <=
				InstancePtr->Config.PMemMaxAddr);
	} else {
		Ret = InstancePtr->Config.NpMemBaseAddr;
		InstancePtr->Config.NpMemBaseAddr = InstancePtr->Config.NpMemBaseAddr
							+ Size;
		Xil_AssertNonvoid(InstancePtr->Config.NpMemBaseAddr <=
				InstancePtr->Config.NpMemMaxAddr);
	}

End:
	return Ret;
}

static int XDmaPcie_PositionRightmostSetbit(u64 Size)
{
	int Position = 0;
	int Bit = 1;

	/* ignore 4 bits */
	Size = Size & (~(0xf));

	while (!(Size & Bit)) {
		Bit = Bit << 1;
		Position++;
	}

	return Position;
}
/******************************************************************************/
/**
* This function increments to next 1Mb page starting position of
* non prefetchable memory
*
* @param   	InstancePtr pointer to XDmaPcie Instance Pointer
*
*******************************************************************************/
static void XDmaPcie_IncreamentNpMem(XDmaPcie *InstancePtr)
{
	InstancePtr->Config.NpMemBaseAddr >>= MB_SHIFT;
	InstancePtr->Config.NpMemBaseAddr++;
	InstancePtr->Config.NpMemBaseAddr <<= MB_SHIFT;
}

/******************************************************************************/
/**
* This function Composes configuration space location
*
* @param   InstancePtr pointer to XDmaPcie Instance Pointer
* @param   headerType u32 type0 or type1 header
* @param   Bus
* @param   Device
* @param   Function
*
* @return  int XST_SUCCESS on success
*          err on fail
*
*******************************************************************************/
static int XDmaPcie_AllocBarSpace(XDmaPcie *InstancePtr, u32 Headertype, u8 Bus,
                           u8 Device, u8 Function)
{
	u32 Data = DATA_MASK_32;
	u32 Location = 0, Location_1 = 0;
	u32 Size = 0, Size_1 = 0, TestWrite;
	u8 MemAs, MemType;
	u64 BarAddr;
	u32 Tmp, *PPtr;
	u8 BarNo;

	u8 MaxBars = 0;

	if (Headertype == XDMAPCIE_CFG_HEADER_O_TYPE) {
		/* For endpoints */
		MaxBars = 6;
	} else {
		/* For Bridge*/
		MaxBars = 2;
	}

	for (BarNo = 0; BarNo < MaxBars; BarNo++) {
		/* Compose function configuration space location */
		Location = XDmaPcie_ComposeExternalConfigAddress(
			Bus, Device, Function,
			XDMAPCIE_CFG_BAR_BASE_OFFSET + BarNo);

		/* Write data to that location */
		XDmaPcie_WriteReg((InstancePtr->Config.Ecam), Location, Data);

		Size = XDmaPcie_ReadReg((InstancePtr->Config.Ecam), Location);
		if ((Size & (~(0xf))) == 0x00) {
			/* return saying that BAR is not implemented */
			XDmaPcie_Dbg(
				"bus: %d, device: %d, function: %d: BAR %d is "
				"not implemented\r\n",
				Bus, Device, Function, BarNo);
			continue;
		}

		/* check for IO space or memory space */
		if (Size & XDMAPCIE_CFG_BAR_MEM_TYPE_MASK) {
			/* Device required IO address space */
			MemType = XDMAPCIE_BAR_IO_MEM;
			XDmaPcie_Dbg(
				"bus: %d, device: %d, function: %d: BAR %d "
				"required IO space; it is unassigned\r\n",
				Bus, Device, Function, BarNo);
			continue;
		} else {
			/* Device required memory address space */
			MemType = XDMAPCIE_BAR_ADDR_MEM;
		}

		/* check for 32 bit AS or 64 bit AS */
		if ((Size & 0x6) == 0x4) {
			/* 64 bit AS is required */
			MemAs = XDMAPCIE_BAR_MEM_TYPE_64;

			/* Compose function configuration space location */
			Location_1 = XDmaPcie_ComposeExternalConfigAddress(
				Bus, Device, Function,
				XDMAPCIE_CFG_BAR_BASE_OFFSET + (BarNo + 1));

			/* Write data to that location */
			XDmaPcie_WriteReg((InstancePtr->Config.Ecam),
						Location_1, Data);

			/* get next bar if 64 bit address is required */
			Size_1 = XDmaPcie_ReadReg((InstancePtr->Config.Ecam),
						Location_1);

			/* Merge two bars for size */
			PPtr = (u32 *)&BarAddr;
			*PPtr = Size;
			*(PPtr + 1) = Size_1;

			TestWrite = XDmaPcie_PositionRightmostSetbit(BarAddr);

			/* actual bar size is 2 << TestWrite */
			BarAddr =
				XDmaPcie_ReserveBarMem(InstancePtr, MemType, MemAs,
						(2 << (TestWrite - 1)));

			Tmp = (u32)BarAddr;

			/* Write actual bar address here */
			XDmaPcie_WriteReg((InstancePtr->Config.Ecam), Location,
					  Tmp);

			Tmp = (u32)(BarAddr >> 32);

			/* Write actual bar address here */
			XDmaPcie_WriteReg((InstancePtr->Config.Ecam),
						Location_1, Tmp);
			XDmaPcie_Dbg(
				"bus: %d, device: %d, function: %d: BAR %d, "
				"ADDR: 0x%p size : %dK\r\n",
				Bus, Device, Function, BarNo, BarAddr,
				((2 << (TestWrite - 1)) / 1024));
		} else {
			/* 32 bit AS is required */
			MemAs = XDMAPCIE_BAR_MEM_TYPE_32;

			TestWrite = XDmaPcie_PositionRightmostSetbit(Size);

			/* actual bar size is 2 << TestWrite */
			BarAddr =
				XDmaPcie_ReserveBarMem(InstancePtr, MemType, MemAs,
						(2 << (TestWrite - 1)));

			Tmp = (u32)BarAddr;

			/* Write actual bar address here */
			XDmaPcie_WriteReg((InstancePtr->Config.Ecam), Location,
					Tmp);
			XDmaPcie_Dbg(
				"bus: %d, device: %d, function: %d: BAR %d, "
				"ADDR: 0x%p size : %dK\r\n",
				Bus, Device, Function, BarNo, BarAddr,
				((2 << (TestWrite - 1)) / 1024));
		}
		/* no need to probe next bar if present BAR requires 64 bit AS
		 */
		if ((Size & 0x6) == 0x4)
			BarNo = BarNo + 1;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
* This function increments to next 1Mb block starting position of
* prefetchable memory
*
* @param  	InstancePtr pointer to XDmaPcie Instance
*
*******************************************************************************/
static void XDmaPcie_IncreamentPMem(XDmaPcie *InstancePtr)
{
	InstancePtr->Config.PMemBaseAddr >>= MB_SHIFT;
	InstancePtr->Config.PMemBaseAddr++;
	InstancePtr->Config.PMemBaseAddr <<= MB_SHIFT;
}

/******************************************************************************/
/**
* This function starts enumeration of PCIe Fabric on the system.
* Assigns primary, secondary and subordinate bus numbers.
* Assigns memory to prefetchable and non-prefetchable memory locations.
* enables end-points and bridges.
*
* @param   	InstancePtr pointer to XDmaPcie Instance Pointer
* @param   	bus_num	to scans for connected bridges/endpoints on it.
*
* @return  	none
*
*******************************************************************************/
static void XDmaPcie_FetchDevicesInBus(XDmaPcie *InstancePtr, u32 BusNum)
{
	u32 ConfigData;
	static u32 LastBusNum;

	u16 PCIeVendorID;
	u16 PCIeDeviceID;
	u32 PCIeHeaderType;
	u32 PCIeMultiFun;

	u32 Adr06; /* Latency timer */
	u32 Adr08;
	u32 Adr09;
	u32 Adr0A;
	u32 Adr0B;

	int Ret;

	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (BusNum > InstancePtr->MaxNumOfBuses) {
		/* End of bus size */
		return;
	}

	for (u32 PCIeDevNum = 0; PCIeDevNum < XDMAPCIE_CFG_MAX_NUM_OF_DEV;
	     PCIeDevNum++) {
		for (u32 PCIeFunNum = 0; PCIeFunNum < XDMAPCIE_CFG_MAX_NUM_OF_FUN;
		     PCIeFunNum++) {

			/* Vendor ID */
			XDmaPcie_ReadRemoteConfigSpace(
				InstancePtr, BusNum, PCIeDevNum, PCIeFunNum,
				XDMAPCIE_CFG_ID_REG, &ConfigData);

			PCIeVendorID = (u16)(ConfigData & 0xFFFF);
			PCIeDeviceID = (u16)((ConfigData >> 16) & 0xFFFF);

			if (PCIeVendorID == XDMAPCIE_CFG_FUN_NOT_IMP_MASK) {
				if (PCIeFunNum == 0)
					/*
					 * We don't need to look
					 * any further on this device.
					 */
					break;
			} else {
				XDmaPcie_Dbg(
					"\n\rPCIeBus is %02X\r\nPCIeDev is "
					"%02X\r\nPCIeFunc is %02X\r\n",
					BusNum, PCIeDevNum, PCIeFunNum);

				XDmaPcie_Dbg(
					"Vendor ID is %04X \r\nDevice ID is "
					"%04X\r\n",
					PCIeVendorID, PCIeDeviceID);

				/* Header Type */
				XDmaPcie_ReadRemoteConfigSpace(
					InstancePtr, BusNum, PCIeDevNum,
					PCIeFunNum, XDMAPCIE_CFG_CAH_LAT_HD_REG,
					&ConfigData);

				PCIeHeaderType =
					ConfigData & XDMAPCIE_CFG_HEADER_TYPE_MASK;
				PCIeMultiFun =
					ConfigData & XDMAPCIE_CFG_MUL_FUN_DEV_MASK;

				if (PCIeHeaderType == XDMAPCIE_CFG_HEADER_O_TYPE) {
					/* This is an End Point */
					XDmaPcie_Dbg("This is an End Point\r\n");

					/*
					 * Write Address to PCIe BAR
					 */
					Ret = XDmaPcie_AllocBarSpace(
						InstancePtr, PCIeHeaderType,
						BusNum, PCIeDevNum,
						PCIeFunNum);
					if (Ret != 0)
						return;

					/*
					 * Initialize this end point
					 * and return.
					 */
					XDmaPcie_ReadRemoteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XDMAPCIE_CFG_CMD_STATUS_REG,
						&ConfigData);

					ConfigData |= (XDMAPCIE_CFG_CMD_BUSM_EN
						       | XDMAPCIE_CFG_CMD_MEM_EN);

					XDmaPcie_WriteRemoteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XDMAPCIE_CFG_CMD_STATUS_REG,
						ConfigData);

					XDmaPcie_Dbg(
						"End Point has been "
						"enabled\r\n");

					XDmaPcie_IncreamentNpMem(InstancePtr);
					XDmaPcie_IncreamentPMem(InstancePtr);

				} else {
					/* This is a bridge */
					XDmaPcie_Dbg("This is a Bridge\r\n");

					/* alloc bar space and configure bridge
					 */
					Ret = XDmaPcie_AllocBarSpace(
						InstancePtr, PCIeHeaderType,
						BusNum, PCIeDevNum,
						PCIeFunNum);

					if (Ret != 0)
						continue;

					Adr06 = 0x0; /* Latency timer */
					Adr08 = 0x0;
					Adr09 = 0x0;
					Adr0A = 0x0;
					Adr0B = 0x0;

					/* Sets primary and secondary bus
					 * numbers */
					Adr06 <<= TWO_HEX_NIBBLES;
					Adr06 |= 0xFF; /* sub ordinate bus no 0xF
						     */
					Adr06 <<= TWO_HEX_NIBBLES;
					Adr06 |= (++LastBusNum); /* secondary
							      bus no */
					Adr06 <<= TWO_HEX_NIBBLES;
					Adr06 |= BusNum; /* Primary bus no */
					XDmaPcie_WriteRemoteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XDMAPCIE_CFG_BUS_NUMS_T1_REG,
						Adr06);

					/* Update start values of P and NP MMIO
					 * base */
					Adr08 |= ((InstancePtr->Config.NpMemBaseAddr
						   & 0xFFF00000)
						  >> FOUR_HEX_NIBBLES);
					XDmaPcie_WriteRemoteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XDMAPCIE_CFG_NP_MEM_T1_REG, Adr08);

					Adr09 |= ((InstancePtr->Config.PMemBaseAddr
						   & 0xFFF00000)
						  >> FOUR_HEX_NIBBLES);
					XDmaPcie_WriteRemoteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XDMAPCIE_CFG_P_MEM_T1_REG, Adr09);
					Adr0A |= (InstancePtr->Config.PMemBaseAddr
						  >> EIGHT_HEX_NIBBLES);
					XDmaPcie_WriteRemoteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XDMAPCIE_CFG_P_UPPER_MEM_T1_REG,
						Adr0A);

					/* Searches secondary bus devices. */
					XDmaPcie_FetchDevicesInBus(InstancePtr,
							  LastBusNum);

					/*
					 * update subordinate bus no
					 * clearing subordinate bus no
					 */
					Adr06 &= (~(0xFF << FOUR_HEX_NIBBLES));
					/* setting subordinate bus no */
					Adr06 |= (LastBusNum
						  << FOUR_HEX_NIBBLES);
					XDmaPcie_WriteRemoteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XDMAPCIE_CFG_BUS_NUMS_T1_REG,
						Adr06);

					/*
					 * Update end values of MMIO limit
					 */

					/*
					 * Align memory to 1 Mb boundry.
					 *
					 * eg. 0xE000 0000 is the base address. Increments
					 * 1 Mb which gives 0xE010 0000 and writes to limit.
					 * So the final value at DW08(in pcie type 1 header)
					 * is 0xE010 E000.
					 * So the range is 0xE000 0000 to 0xE01F FFFF.
					 *
					 */
					XDmaPcie_IncreamentNpMem(InstancePtr);
					Adr08 |= (InstancePtr->Config.NpMemBaseAddr
						  & 0xFFF00000);
					XDmaPcie_WriteRemoteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XDMAPCIE_CFG_NP_MEM_T1_REG, Adr08);

					XDmaPcie_IncreamentPMem(InstancePtr);
					Adr09 |= (InstancePtr->Config.PMemBaseAddr
						  & 0xFFF00000);
					XDmaPcie_WriteRemoteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XDMAPCIE_CFG_P_MEM_T1_REG, Adr09);
					Adr0B |= (InstancePtr->Config.PMemBaseAddr
						  >> EIGHT_HEX_NIBBLES);
					XDmaPcie_WriteRemoteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XDMAPCIE_CFG_P_LIMIT_MEM_T1_REG,
						Adr0B);

					/* Increment P & NP mem to next aligned starting address.
					 *
					 * Eg: As the range is 0xE000 0000 to 0xE01F FFFF.
					 * the next starting address should be 0xE020 0000.
					 */
					XDmaPcie_IncreamentNpMem(InstancePtr);
					XDmaPcie_IncreamentPMem(InstancePtr);

					/*
					 * Enable configuration
					 */
					XDmaPcie_ReadRemoteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XDMAPCIE_CFG_CMD_STATUS_REG,
						&ConfigData);

					ConfigData |= (XDMAPCIE_CFG_CMD_BUSM_EN
						       | XDMAPCIE_CFG_CMD_MEM_EN);

					XDmaPcie_WriteRemoteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XDMAPCIE_CFG_CMD_STATUS_REG,
						ConfigData);
				}
			}
			if ((!PCIeFunNum) && (!PCIeMultiFun)) {
				/*
				 * If it is function 0 and it is not a
				 * multi function device, we don't need
				 * to look any further on this devie
				 */
				break;
			}
		}
	}
}

/******************************************************************************/
/**
* This function starts PCIe enumeration.
*
* @param    InstancePtr pointer to XDmaPcie Instance Pointer
*
* @return 	none
*
*******************************************************************************/
void XDmaPcie_EnumerateFabric(XDmaPcie *InstancePtr)
{
	XDmaPcie_FetchDevicesInBus(InstancePtr, 0);
}

/****************************************************************************/
/**
* This API is used to read the VSEC Capability Register.
*
* @param	InstancePtr is the XDmaPcie instance to operate on.
* @param	VsecNum is a VSEC register number as there are two registers.
*		Possible values are.
*			- XDMAPCIE_VSEC1 (0)
*			- XDMAPCIE_VSEC2 (1)
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
void XDmaPcie_GetVsecCapability(XDmaPcie *InstancePtr, u8 VsecNum,
			 u16 *VsecIdPtr, u8 *VersionPtr, u16 *NextCapPtr)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(VsecIdPtr != NULL);
	Xil_AssertVoid(VersionPtr != NULL);
	Xil_AssertVoid(NextCapPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
			(XDMAPCIE_VSECC_OFFSET +
			(XDMAPCIE_VSEC2_OFFSET_WRT_VSEC1 * VsecNum)));

	*VsecIdPtr = (u16)(Data & XDMAPCIE_VSECC_ID_MASK);

	*VersionPtr = (u8)((Data & XDMAPCIE_VSECC_VER_MASK) >>
						XDMAPCIE_VSECC_VER_SHIFT);

	*NextCapPtr = (u16)((Data & XDMAPCIE_VSECC_NEXT_MASK) >>
						XDMAPCIE_VSECC_NEXT_SHIFT);


}

/****************************************************************************/
/**
* This API is used to read the VSEC Header Register.
*
* @param	InstancePtr is the XDmaPcie instance to operate on.
* @param	VsecNum is a VSEC register number as there are two registers.
*		Possible values are.
*			- XDMAPCIE_VSEC1 (0)
*			- XDMAPCIE_VSEC2 (1)
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
void XDmaPcie_GetVsecHeader(XDmaPcie *InstancePtr, u8 VsecNum, u16 *VsecIdPtr,
				 u8 *RevisionPtr, u16 *LengthPtr)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(VsecIdPtr != NULL);
	Xil_AssertVoid(RevisionPtr != NULL);
	Xil_AssertVoid(LengthPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	Data = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
			(XDMAPCIE_VSECH_OFFSET +
			(XDMAPCIE_VSEC2_OFFSET_WRT_VSEC1 * VsecNum)));

	*VsecIdPtr = (u16)(Data & XDMAPCIE_VSECH_ID_MASK);

	*RevisionPtr = (u8)((Data & XDMAPCIE_VSECH_REV_MASK) >>
						XDMAPCIE_VSECH_REV_SHIFT);

	*LengthPtr = (u16)((Data & XDMAPCIE_VSECH_LEN_MASK) >>
						XDMAPCIE_VSECH_LEN_SHIFT);


}

/****************************************************************************/
/**
* This API Reads the Bridge info register.
*
* @param	InstancePtr is the XDmaPcie instance to operate on.
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
void XDmaPcie_GetBridgeInfo(XDmaPcie *InstancePtr, u8 *Gen2Ptr,
					 u8 *RootPortPtr, u8 *ECAMSizePtr)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Gen2Ptr != NULL);
	Xil_AssertVoid(RootPortPtr != NULL);
	Xil_AssertVoid(ECAMSizePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XDMAPCIE_BI_OFFSET);

	*Gen2Ptr = (u8)(Data & XDMAPCIE_BI_GEN2_MASK);

	*RootPortPtr = (u8)((Data & XDMAPCIE_BI_RP_MASK) >>
							XDMAPCIE_BI_RP_SHIFT);

	*ECAMSizePtr = (u16)((Data & XDMAPCIE_BI_ECAM_SIZE_MASK) >>
					XDMAPCIE_BI_ECAM_SIZE_SHIFT);


}

/****************************************************************************/
/**
* Read the Bus Location register.
*
* @param	InstancePtr is the XDmaPcie instance to operate on.
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
void XDmaPcie_GetRequesterId(XDmaPcie *InstancePtr, u8 *BusNumPtr,
				 u8 *DevNumPtr, u8 *FunNumPtr, u8 *PortNumPtr)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(BusNumPtr != NULL);
	Xil_AssertVoid(DevNumPtr != NULL);
	Xil_AssertVoid(FunNumPtr != NULL);
	Xil_AssertVoid(PortNumPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XDMAPCIE_BL_OFFSET);

	*BusNumPtr = (u8)((Data & XDMAPCIE_BL_BUS_MASK) >>
							XDMAPCIE_BL_BUS_SHIFT);

	*DevNumPtr = (u8)((Data & XDMAPCIE_BL_DEV_MASK) >>
							XDMAPCIE_BL_DEV_SHIFT);

	*FunNumPtr = (u8)(Data & XDMAPCIE_BL_FUNC_MASK);


	*PortNumPtr = (u8)((Data & XDMAPCIE_BL_PORT_MASK) >>
						XDMAPCIE_BL_PORT_SHIFT);

}

/****************************************************************************/
/**
* This API is used to read the Phy Status/Control Register.
*
* @param	InstancePtr is the XDmaPcie instance to operate on.
* @param	PhyState is a pointer to a variable where the driver will
* 		pass back Current physical status.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XDmaPcie_GetPhyStatusCtrl(XDmaPcie *InstancePtr, u32 *PhyState)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(PhyState != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
				XDMAPCIE_PHYSC_OFFSET);

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
void XDmaPcie_GetRootPortStatusCtrl(XDmaPcie *InstancePtr, u32 *StatusPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(StatusPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeRootComplex ==
							XDMAPCIE_IS_RC);

	*StatusPtr = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XDMAPCIE_RPSC_OFFSET);
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
void XDmaPcie_SetRootPortStatusCtrl(XDmaPcie *InstancePtr, u32 StatusData)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeRootComplex ==
							XDMAPCIE_IS_RC);


	XDmaPcie_WriteReg((InstancePtr->Config.BaseAddress),
			XDMAPCIE_RPSC_OFFSET,StatusData & XDMAPCIE_RPSC_MASK);

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
int XDmaPcie_SetRootPortMSIBase(XDmaPcie *InstancePtr,
						 unsigned long long MsiBase)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->Config.IncludeRootComplex ==
							XDMAPCIE_IS_RC);

	Data = (u32)((MsiBase >> XDMAPCIE_RPMSIB_UPPER_SHIFT) &
						XDMAPCIE_RPMSIB_UPPER_MASK);

	XDmaPcie_WriteReg((InstancePtr->Config.BaseAddress),
					XDMAPCIE_RPMSIB_UPPER_OFFSET, Data);

	/* Check 4kB alignment of supplied MSI base address */
	if(MsiBase & ~(ALIGN_4KB))
		return XST_FAILURE;

	Data = (u32)(MsiBase & XDMAPCIE_RPMSIB_LOWER_MASK);
	XDmaPcie_WriteReg((InstancePtr->Config.BaseAddress),
					XDMAPCIE_RPMSIB_LOWER_OFFSET, Data);


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
void XDmaPcie_GetRootPortErrFIFOMsg(XDmaPcie *InstancePtr, u16 *ReqIdPtr,
					 u8 *ErrType, u8 *ErrValid)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ReqIdPtr != NULL);
	Xil_AssertVoid(ErrType != NULL);
	Xil_AssertVoid(ErrValid != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeRootComplex ==
							XDMAPCIE_IS_RC);

	Data = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XDMAPCIE_RPEFR_OFFSET);

	*ReqIdPtr = (u16)(Data & XDMAPCIE_RPEFR_REQ_ID_MASK);

	*ErrType = (u8)((Data & XDMAPCIE_RPEFR_ERR_TYPE_MASK) >>
						XDMAPCIE_RPEFR_ERR_TYPE_SHIFT);

	*ErrValid = (u8)((Data & XDMAPCIE_RPEFR_ERR_VALID_MASK) >>
					XDMAPCIE_RPEFR_ERR_VALID_SHIFT);
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
void XDmaPcie_ClearRootPortErrFIFOMsg(XDmaPcie *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeRootComplex ==
							XDMAPCIE_IS_RC);

	XDmaPcie_WriteReg((InstancePtr->Config.BaseAddress),
					XDMAPCIE_RPEFR_OFFSET, 0x7FFF);

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
int XDmaPcie_GetRootPortIntFIFOReg(XDmaPcie *InstancePtr, u16 *ReqIdPtr,
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
							XDMAPCIE_IS_RC);

	*MsiMsgData = 0;

	Data = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
						XDMAPCIE_RPIFR1_OFFSET);

	*ReqIdPtr = (u16)(Data & XDMAPCIE_RPIFR1_REQ_ID_MASK);

	*MsiAddr = (u8)((Data & XDMAPCIE_RPIFR1_MSI_ADDR_MASK) >>
					XDMAPCIE_RPIFR1_MSI_ADDR_SHIFT);

	*MsiInt = (u8)((Data & XDMAPCIE_RPIFR1_MSIINTR_VALID_MASK) >>
					XDMAPCIE_RPIFR1_MSIINTR_VALID_SHIFT);

	*IntValid = (u8)((Data &  XDMAPCIE_RPIFR1_INTR_VALID_MASK) >>
					XDMAPCIE_RPIFR1_INTR_VALID_SHIFT);

	if(*MsiInt & *IntValid)
	{

		Data = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
						XDMAPCIE_RPIFR2_OFFSET);

		*MsiMsgData = (u16)(Data & XDMAPCIE_RPIFR2_MSG_DATA_MASK);
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
void XDmaPcie_ClearRootPortIntFIFOReg(XDmaPcie *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeRootComplex ==
							XDMAPCIE_IS_RC);

	XDmaPcie_WriteReg((InstancePtr->Config.BaseAddress),
					XDMAPCIE_RPIFR1_OFFSET,0xA7FFFFFF);

}

/****************************************************************************/
/**
* Read PCIe address translation vector that corresponds to one of AXI local
* bus bars passed by the caller.
*
* @param 	InstancePtr is the XDmaPcie instance to operate on.
* @param 	BarNumber is AXI bar number (0 - 5) passed by caller.
* @param 	BarAddrPtr is a pointer to a variable where the driver will
*.		pass back translation vector.
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
void XDmaPcie_GetLocalBusBar2PcieBar(XDmaPcie *InstancePtr, u8 BarNumber,
						 XDmaPcie_BarAddr *BarAddrPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(BarAddrPtr != NULL);
	Xil_AssertVoid(BarNumber < InstancePtr->Config.LocalBarsNum);
	Xil_AssertVoid(InstancePtr->Config.IncludeBarOffsetReg != FALSE);

	BarAddrPtr->LowerAddr =
		XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
		(XDMAPCIE_AXIBAR2PCIBAR_0L_OFFSET +
		(BarNumber * (sizeof(u32) * 2))));

	BarAddrPtr->UpperAddr =
		XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
		(XDMAPCIE_AXIBAR2PCIBAR_0U_OFFSET +
		(BarNumber * (sizeof(u32) * 2))));

}

/****************************************************************************/
/**
* Write PCIe address translation vector that corresponds to one of AXI local
* bus bars passed by the caller.
*
* @param 	InstancePtr is the XDmaPcie instance to operate on.
* @param 	BarNumber is AXI bar number (0 - 5) passed by caller.
* @param 	BarAddrPtr is a pointer to a variable where the driver will
* 		pass back translation vector.
*
* @return 	None.
*
* @note		None.
*
*****************************************************************************/
void XDmaPcie_SetLocalBusBar2PcieBar(XDmaPcie *InstancePtr, u8 BarNumber,
						 XDmaPcie_BarAddr *BarAddrPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(BarAddrPtr != NULL);
	Xil_AssertVoid(BarNumber < InstancePtr->Config.LocalBarsNum);
	Xil_AssertVoid(InstancePtr->Config.IncludeBarOffsetReg != FALSE);

	XDmaPcie_WriteReg(InstancePtr->Config.BaseAddress,
		(XDMAPCIE_AXIBAR2PCIBAR_0L_OFFSET +
		(BarNumber * (sizeof(u32) * 2))), (BarAddrPtr->LowerAddr));

	XDmaPcie_WriteReg(InstancePtr->Config.BaseAddress,
		(XDMAPCIE_AXIBAR2PCIBAR_0U_OFFSET +
		(BarNumber * (sizeof(u32) * 2))), (BarAddrPtr->UpperAddr));

}

/****************************************************************************/
/**
* Read 32-bit value from one of this IP own configuration space.
* Location is identified by its offset from the beginning of the
* configuration space.
*
* @param 	InstancePtr is the XDmaPcie instance to operate on.
* @param 	Offset from beginning of IP own configuration space.
* @param 	DataPtr is a pointer to a variable where the driver will pass
* 		back the value read from the specified location.
*
* @return 	None
*
* @note 	None
*
*****************************************************************************/
void XDmaPcie_ReadLocalConfigSpace(XDmaPcie *InstancePtr, u16 Offset,
								 u32 *DataPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(DataPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	*DataPtr = XDmaPcie_ReadReg((InstancePtr->Config.BaseAddress),
			(XDMAPCIE_PCIE_CORE_OFFSET + ((u32) (Offset * 4))));

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
void XDmaPcie_WriteLocalConfigSpace(XDmaPcie *InstancePtr, u16 Offset,
								 u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeRootComplex ==
							XDMAPCIE_IS_RC);

	XDmaPcie_WriteReg((InstancePtr->Config.BaseAddress),
		(XDMAPCIE_PCIE_CORE_OFFSET + ((u32) (Offset * 4))), Data);

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
u32 XDmaPcie_ComposeExternalConfigAddress(u8 Bus, u8 Device, u8 Function,
								 u16 Offset)
{
	u32 Location = 0;

	Location |= ((((u32)Bus) << XDMAPCIE_ECAM_BUS_SHIFT) &
						XDMAPCIE_ECAM_BUS_MASK);

	Location |= ((((u32)Device) << XDMAPCIE_ECAM_DEV_SHIFT) &
						XDMAPCIE_ECAM_DEV_MASK);

	Location |= ((((u32)Function) << XDMAPCIE_ECAM_FUN_SHIFT) &
						XDMAPCIE_ECAM_FUN_MASK);

	Location |= ((((u32)Offset) << XDMAPCIE_ECAM_REG_SHIFT) &
						XDMAPCIE_ECAM_REG_MASK);

	Location &= XDMAPCIE_ECAM_MASK;

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
*		root complex. The XDmaPcie_ReadLocalConfigSpace API should
*		be used for reading the local config space.
*
*****************************************************************************/
void XDmaPcie_ReadRemoteConfigSpace(XDmaPcie *InstancePtr, u8 Bus, u8 Device,
		 u8 Function, u16 Offset, u32 *DataPtr)
{
	u32 Location = 0;
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(DataPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeRootComplex ==
							XDMAPCIE_IS_RC);

	if (((Bus == 0) && !((Device == 0) && (Function == 0))) ||
		(Bus > InstancePtr->MaxNumOfBuses)) {
		*DataPtr = 0xFFFFFFFF;
		return;
	}

	/* Compose function configuration space location */
	Location = XDmaPcie_ComposeExternalConfigAddress (Bus, Device,
							Function, Offset);

	while(XDmaPcie_IsEcamBusy(InstancePtr));

	/* Read data from that location */
	Data = XDmaPcie_ReadReg((InstancePtr->Config.BaseAddress),
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
*		root complex. The XDmaPcie_WriteLocalConfigSpace should be
*		used for writing to local config space.
*
*****************************************************************************/
void XDmaPcie_WriteRemoteConfigSpace(XDmaPcie *InstancePtr, u8 Bus, u8 Device,
					 u8 Function, u16 Offset, u32 Data)
{
	u32 Location = 0;
	u32 TestWrite = 0;
	u8 Count = 3;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeRootComplex ==
							XDMAPCIE_IS_RC);

	if ((Bus == 0) || (Bus > InstancePtr->MaxNumOfBuses)) {
		return;
	}

	/* Compose function configuration space location */
	Location = XDmaPcie_ComposeExternalConfigAddress (Bus, Device,
							Function, Offset);
	while(XDmaPcie_IsEcamBusy(InstancePtr));


	/* Write data to that location */
	XDmaPcie_WriteReg((InstancePtr->Config.BaseAddress),
				Location , Data);


	/* Read data from that location to verify write */
	while (Count) {

		TestWrite =
			XDmaPcie_ReadReg((InstancePtr->Config.BaseAddress),
								Location);

		if (TestWrite == Data) {
			break;
		}

		Count--;
	}
}
