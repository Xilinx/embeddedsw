/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* @file xpciepsu.c
*
* Implements all of functions for psu_pci IP driver except interrupts and
* initialization
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0	bs	08/21/2018	First release
* </pre>
*
*******************************************************************************/
/******************************** Include Files *******************************/
#include "xpciepsu.h"
#include "xpciepsu_common.h"
#include "sleep.h"
/**************************** Constant Definitions ****************************/

/****************************** Type Definitions ******************************/

/******************** Macros (Inline Functions) Definitions *******************/

/**************************** Variable Definitions ****************************/


/***************************** Function Prototypes ****************************/

/******************************************************************************/
/**
* This function looks for phy is ready or not
*
* @param   InstancePtr pointer to XPciePsu_Config Instance Pointer
*
* @return  1 if phy is ready
*          0 if phy is not ready
*
*******************************************************************************/
static int XPciePsu_PhyReady(XPciePsu_Config *InstancePtr)
{
	s32 Status;
	if (XPciePsu_ReadReg(InstancePtr->PciReg, XPCIEPSU_PS_LINKUP_OFFSET)
	    & XPCIEPSU_XPHY_RDY_LINKUP_BIT){
		Status = XPCIEPSU_LINKUP_SUCCESS;
	} else {
		Status = XPCIEPSU_LINKUP_FAIL;
	}
	return Status;
}

/******************************************************************************/
/**
* This function looks for PCIe link is up or not
*
* @param   InstancePtr pointer to XPciePsu_Config Instance Pointer
*
* @return  1 if link is up
*          0 if link is down
*
*******************************************************************************/
static int XPciePsu_PcieLinkUp(XPciePsu_Config *InstancePtr)
{
	s32 Status;
	if (XPciePsu_ReadReg(InstancePtr->PciReg, XPCIEPSU_PS_LINKUP_OFFSET)
	    & XPCIEPSU_PHY_LINKUP_BIT){
		Status =  XPCIEPSU_LINKUP_SUCCESS;
	} else {
		Status =  XPCIEPSU_LINKUP_FAIL;
	}
	return Status;
}

/******************************************************************************/
/**
* This function waits for phy link to come up till
* XPCIEPSU_LINK_WAIT_MAX_RETRIES times.
*
* @param   InstancePtr pointer to XPciePsu_Config Instance Pointer
*
* @return  1 if link is up
*          0 if link up fail
*
*******************************************************************************/
static int XPciePsu_PcieLinkUpTimeout(XPciePsu_Config *InstancePtr)
{
	int Retries;
	s32 Status = XPCIEPSU_LINKUP_FAIL;
	/* check if the link is up or not */
	for (Retries = 0; Retries < XPCIEPSU_LINK_WAIT_MAX_RETRIES; Retries++) {
		if (XPciePsu_PhyReady(InstancePtr)){
			Status =  XPCIEPSU_LINKUP_SUCCESS;
			goto End;
		}
		usleep(XPCIEPSU_LINK_WAIT_USLEEP_MIN);
	}

	XPciePsu_Err("PHY never cames up\r\n");
End:
	return Status;
}

/******************************************************************************/
/**
* This function initializes PCIe bridge. Enables/Disables required channels,
* checks for links, ECAM, Interrupts.
*
* @param   InstancePtr pointer to XPciePsu Instance Pointer
*
* @return  0 if success
*          error value on failure
*
*******************************************************************************/
static int XPciePsu_BridgeInit(XPciePsu *InstancePtr)
{
	XPciePsu_Config *CfgPtr;
	s32 Status;
	u32 BRegVal, ECamVal, FirstBusNo = 0, ECamValMin, ECamSize;
	int Err;

	CfgPtr = &(InstancePtr->Config);

	Xil_AssertNonvoid(CfgPtr != NULL);

	BRegVal = XPciePsu_ReadReg(CfgPtr->BrigReg, XPCIEPSU_E_BREG_CAPABILITIES)
		   & BREG_PRESENT;
	if (!BRegVal) {
		XPciePsu_Err("BREG is not present\r\n");
		Status = XST_FAILURE;
		goto End;
	}

	/* Write bridge_off to breg base */
	XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_E_BREG_BASE_LO,
			  LOWER_32_BITS(CfgPtr->BrigReg));

	XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_E_BREG_BASE_HI,
			  UPPER_32_BITS(CfgPtr->BrigReg));

	/* Enable BREG */
	XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_E_BREG_CONTROL,
			  ((~BREG_ENABLE_FORCE) & BREG_ENABLE));

	/* Disable DMA channel registers */
	XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_BRCFG_RX0,
			  (XPciePsu_ReadReg(CfgPtr->BrigReg, XPCIEPSU_BRCFG_RX0)
			   | CFG_DMA_REG_BAR));

	/* Enable Ingress subtractive decode translation */
	XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_I_ISUB_CONTROL,
			SET_ISUB_CONTROL);

	/* Enable msg filtering details */
	XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_BRCFG_RX_MSG_FILTER,
			  CFG_ENABLE_MSG_FILTER_MASK);

	/* Check for linkup */
	Err = XPciePsu_PcieLinkUpTimeout(CfgPtr);
	if (Err != XPCIEPSU_LINKUP_SUCCESS){
		Status = XST_FAILURE;
		goto End;
	}

	ECamVal = XPciePsu_ReadReg(CfgPtr->BrigReg,
				XPCIEPSU_E_ECAM_CAPABILITIES) & E_ECAM_PRESENT;
	if (!ECamVal) {
		XPciePsu_Err("ECAM is not present\r\n");
		Status = XST_FAILURE;
		goto End;
	}

	/* Enable ECAM */
	XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_E_ECAM_CONTROL,
			XPciePsu_ReadReg(CfgPtr->BrigReg,
					XPCIEPSU_E_ECAM_CONTROL) |
					E_ECAM_CR_ENABLE);

	XPciePsu_WriteReg(
		CfgPtr->BrigReg, XPCIEPSU_E_ECAM_CONTROL,
				XPciePsu_ReadReg(CfgPtr->BrigReg,
					XPCIEPSU_E_ECAM_CONTROL) |
					(PSU_ECAM_VALUE_DEFAULT << E_ECAM_SIZE_SHIFT));

	XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_E_ECAM_BASE_LO,
			  LOWER_32_BITS(CfgPtr->Ecam));

	XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_E_ECAM_BASE_HI,
			  UPPER_32_BITS(CfgPtr->Ecam));

	/* Get bus range */
	ECamVal = XPciePsu_ReadReg(CfgPtr->BrigReg, XPCIEPSU_E_ECAM_CONTROL);
	ECamValMin = XPciePsu_ReadReg(CfgPtr->BrigReg,
			XPCIEPSU_E_ECAM_CAPABILITIES);

	ECamSize = 0x1 << (((ECamVal & E_ECAM_SIZE_LOC) >> E_ECAM_SIZE_SHIFT) +
                          ((ECamValMin & E_ECAM_SIZE_MIN) >>
                           E_ECAM_SIZE_SHIFT));

	/* Configure last bus numbers as max possible bus number */
	InstancePtr->MaxSupportedBusNo = GET_MAX_BUS_NO(ECamSize);

	/* Write primary, secondary and subordinate bus numbers */
	ECamVal = FirstBusNo;
	ECamVal |= (FirstBusNo + 1) << 8;
	ECamVal |= (InstancePtr->MaxSupportedBusNo << E_ECAM_SIZE_SHIFT);

	XPciePsu_WriteReg(CfgPtr->Ecam, XPCIEPSU_PRIMARY_BUS, ECamVal);

	/* check link up */
	if (XPciePsu_PcieLinkUp(CfgPtr) == XPCIEPSU_LINKUP_SUCCESS) {
		XPciePsu_Dbg("Link is UP\r\n");
	} else {
		XPciePsu_Err("Link is DOWN\r\n");
		Status = XST_FAILURE;
		goto End;
	}

	/* Disable all misc interrupts */
	XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_MSGF_MISC_MASK,
			  (u32)~MSGF_MISC_SR_MASKALL);

	/* Disable all legacy interrupts */
	XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_MSGF_LEG_MASK,
			  (u32)~MSGF_LEG_SR_MASKALL);

	Status = XST_SUCCESS;
End:
	return Status;
}

/******************************************************************************/
/**
* This function Composes configuration space location
*
* @param   Bus
* @param   Device
* @param   Function
* @param   Offset
*
* @return  location address of the composed address
*
*******************************************************************************/
u32 XPciePsu_ComposeExternalConfigAddress(u8 Bus, u8 Device, u8 Function,
					  u16 Offset)
{
	u32 Location = 0;

	Location |= ((((u32)Bus) << XPCIEPSU_ECAM_BUS_SHIFT) &
				 XPCIEPSU_ECAM_BUS_MASK);

	Location |= ((((u32)Device) << XPCIEPSU_ECAM_DEV_SHIFT) &
				 XPCIEPSU_ECAM_DEV_MASK);

	Location |= ((((u32)Function) << XPCIEPSU_ECAM_FUN_SHIFT) &
				 XPCIEPSU_ECAM_FUN_MASK);

	Location |= ((((u32)Offset) << XPCIEPSU_ECAM_REG_SHIFT) &
				 XPCIEPSU_ECAM_REG_MASK);

	Location &= XPCIEPSU_ECAM_MASK;

	return Location;
}

/******************************************************************************/
/**
* This function read from remote configuration space location
*
* @param   InstancePtr pointer to XPciePsu Instance Pointer
* @param   Bus
* @param   Device
* @param   Function
* @param   Offset	location of the address to read data from.
* @param   DataPtr	pointer store date available in the offset
*
* @return  XST_SUCCESS on success
* XST_FAILURE on failure.
*
*******************************************************************************/
u8 XPciePsu_ReadConfigSpace(XPciePsu *InstancePtr, u8 Bus, u8 Device,
				    u8 Function, u16 Offset, u32 *DataPtr)
{
	u32 Location = 0;
	u32 Data;
	u8	Ret = XST_SUCCESS;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DataPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (((Bus == 0) && !((Device == 0) && (Function == 0)))
	    || (Bus > InstancePtr->MaxSupportedBusNo)) {
		*DataPtr = DATA_MASK_32;
		Ret = XST_FAILURE;
		goto End;
	}

	/* Compose function configuration space location */
	Location = XPciePsu_ComposeExternalConfigAddress(Bus, Device, Function,
							 Offset);

	/* Read data from that location */
	Data = XPciePsu_ReadReg((InstancePtr->Config.Ecam), Location);

	*DataPtr = Data;
End:
	return Ret;
}

/******************************************************************************/
/**
* This function write to remote configuration space location
*
* @param   InstancePtr pointer to XPciePsu Instance Pointer
* @param   Bus
* @param   Device
* @param   Function
* @param   Offset	location of the address to write data.
* @param   Data to be written on to the offset
*
* @return  XST_SUCCESS on success
* XST_FAILURE on failure.
*
*******************************************************************************/
u8 XPciePsu_WriteConfigSpace(XPciePsu *InstancePtr, u8 Bus, u8 Device,
				     u8 Function, u16 Offset, u32 Data)
{
	u32 Location = 0;
	u32 TestWrite = 0;
	u8 Count = 3;
	u8 Ret = XST_SUCCESS;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if ((Bus == 0) || (Bus > InstancePtr->MaxSupportedBusNo)){
		Ret = XST_FAILURE;
		goto End;
	}

	/* Compose function configuration space location */
        Location = XPciePsu_ComposeExternalConfigAddress(Bus, Device, Function,
                                                         Offset);

	/* Write data to that location */
	XPciePsu_WriteReg((InstancePtr->Config.Ecam), Location, Data);

	/* Read data from that location to verify write */
	while (Count) {
		TestWrite =
			XPciePsu_ReadReg((InstancePtr->Config.Ecam), Location);

		if (TestWrite == Data)
			break;

		Count--;
	}
End:
	return Ret;
}

static int XPciePsu_PositionRightmostSetbit(u64 Size)
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
* This function reserves bar memory address.
*
* @param   InstancePtr pointer to XPciePsu Instance Pointer
* @param   mem_type type of bar memory. address mem or IO.
* @param   mem_as bar memory tpye 32 or 64 bit
* @param   size	u64 size to increase
*
* @return  bar address
*
*******************************************************************************/
static u64 XPciePsu_ReserveBarMem(XPciePsu *InstancePtr, u8 MemType,
		u8 MemBarArdSize, u64 Size)
{
	u64 Ret = 0;

	if (MemType == XPCIEPSU_BAR_IO_MEM){
		Ret = XST_FAILURE;
		goto End;
	}

	if (MemBarArdSize == XPCIEPSU_BAR_MEM_TYPE_64) {
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

/******************************************************************************/
/**
* This function Composes configuration space location
*
* @param   InstancePtr pointer to XPciePsu Instance Pointer
* @param   headerType u32 type0 or type1 header
* @param   Bus
* @param   Device
* @param   Function
*
* @return  int XST_SUCCESS on success
*          err on fail
*
*******************************************************************************/
static int XPciePsu_AllocBarSpace(XPciePsu *InstancePtr, u32 Headertype, u8 Bus,
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

	if (Headertype == XPCIEPSU_CFG_HEADER_O_TYPE) {
		/* For endpoints */
		MaxBars = 6;
	} else {
		/* For Bridge*/
		MaxBars = 2;
	}

	for (BarNo = 0; BarNo < MaxBars; BarNo++) {
		/* Compose function configuration space location */
		Location = XPciePsu_ComposeExternalConfigAddress(
			Bus, Device, Function,
			XPCIEPSU_CFG_BAR_BASE_OFFSET + BarNo);

		/* Write data to that location */
		XPciePsu_WriteReg((InstancePtr->Config.Ecam), Location, Data);

		Size = XPciePsu_ReadReg((InstancePtr->Config.Ecam), Location);
		if ((Size & (~(0xf))) == 0x00) {
			/* return saying that BAR is not implemented */
			XPciePsu_Dbg(
				"bus: %d, device: %d, function: %d: BAR %d is "
				"not implemented\r\n",
				Bus, Device, Function, BarNo);
			continue;
		}

		/* check for IO space or memory space */
		if (Size & XPCIEPSU_CFG_BAR_MEM_TYPE_MASK) {
			/* Device required IO address space */
			MemType = XPCIEPSU_BAR_IO_MEM;
			XPciePsu_Dbg(
				"bus: %d, device: %d, function: %d: BAR %d "
				"required IO space; it is unassigned\r\n",
				Bus, Device, Function, BarNo);
			continue;
		} else {
			/* Device required memory address space */
			MemType = XPCIEPSU_BAR_ADDR_MEM;
		}

		/* check for 32 bit AS or 64 bit AS */
		if ((Size & 0x6) == 0x4) {
			/* 64 bit AS is required */
			MemAs = XPCIEPSU_BAR_MEM_TYPE_64;

			/* Compose function configuration space location */
			Location_1 = XPciePsu_ComposeExternalConfigAddress(
				Bus, Device, Function,
				XPCIEPSU_CFG_BAR_BASE_OFFSET + (BarNo + 1));

			/* Write data to that location */
			XPciePsu_WriteReg((InstancePtr->Config.Ecam),
						Location_1, Data);

			/* get next bar if 64 bit address is required */
			Size_1 = XPciePsu_ReadReg((InstancePtr->Config.Ecam),
						Location_1);

			/* Merge two bars for size */
			PPtr = (u32 *)&BarAddr;
			*PPtr = Size;
			*(PPtr + 1) = Size_1;

			TestWrite = XPciePsu_PositionRightmostSetbit(BarAddr);

			/* actual bar size is 2 << TestWrite */
			BarAddr =
				XPciePsu_ReserveBarMem(InstancePtr, MemType, MemAs,
						(2 << (TestWrite - 1)));

			Tmp = (u32)BarAddr;

			/* Write actual bar address here */
			XPciePsu_WriteReg((InstancePtr->Config.Ecam), Location,
					  Tmp);

			Tmp = (u32)(BarAddr >> 32);

			/* Write actual bar address here */
			XPciePsu_WriteReg((InstancePtr->Config.Ecam),
						Location_1, Tmp);
			XPciePsu_Dbg(
				"bus: %d, device: %d, function: %d: BAR %d, "
				"ADDR: 0x%p size : %dK\r\n",
				Bus, Device, Function, BarNo, BarAddr,
				((2 << (TestWrite - 1)) / 1024));
		} else {
			/* 32 bit AS is required */
			MemAs = XPCIEPSU_BAR_MEM_TYPE_32;

			TestWrite = XPciePsu_PositionRightmostSetbit(Size);

			/* actual bar size is 2 << TestWrite */
			BarAddr =
				XPciePsu_ReserveBarMem(InstancePtr, MemType, MemAs,
						(2 << (TestWrite - 1)));

			Tmp = (u32)BarAddr;

			/* Write actual bar address here */
			XPciePsu_WriteReg((InstancePtr->Config.Ecam), Location,
					Tmp);
			XPciePsu_Dbg(
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
* This function increments to next 1Mb page starting position of
* non prefetchable memory
*
* @param   	InstancePtr pointer to XPciePsu Instance Pointer
*
*******************************************************************************/
static void XPciePsu_IncreamentNpMem(XPciePsu *InstancePtr)
{
	InstancePtr->Config.NpMemBaseAddr >>= MB_SHIFT;
	InstancePtr->Config.NpMemBaseAddr++;
	InstancePtr->Config.NpMemBaseAddr <<= MB_SHIFT;
}

/******************************************************************************/
/**
* This function increments to next 1Mb block starting position of
* prefetchable memory
*
* @param  	InstancePtr pointer to XPciePsu Instance
*
*******************************************************************************/
static void XPciePsu_IncreamentPMem(XPciePsu *InstancePtr)
{
	InstancePtr->Config.PMemBaseAddr >>= MB_SHIFT;
	InstancePtr->Config.PMemBaseAddr++;
	InstancePtr->Config.PMemBaseAddr <<= MB_SHIFT;
}

/******************************************************************************/
/**
* This function discovers the system topology and assigns bus
* numbers and system resources.
* Assigns primary, secondary and subordinate bus numbers.
* Assigns memory to prefetchable and non-prefetchable memory locations.
* enables end-points and bridges.
*
* @param   	InstancePtr pointer to XPciePsu Instance Pointer
* @param   	bus_num	to scans for connected bridges/endpoints on it.
*
* @return  	none
*
*******************************************************************************/
static void XPciePsu_FetchDevicesInBus(XPciePsu *InstancePtr, u32 BusNum)
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

	if (BusNum > InstancePtr->MaxSupportedBusNo) {
		/* End of bus size */
		return;
	}

	for (u32 PCIeDevNum = 0; PCIeDevNum < XPCIEPSU_CFG_MAX_NUM_OF_DEV;
	     PCIeDevNum++) {
		for (u32 PCIeFunNum = 0; PCIeFunNum < XPCIEPSU_CFG_MAX_NUM_OF_FUN;
		     PCIeFunNum++) {

			/* Vendor ID */
			XPciePsu_ReadConfigSpace(
				InstancePtr, BusNum, PCIeDevNum, PCIeFunNum,
				XPCIEPSU_CFG_ID_REG, &ConfigData);

			PCIeVendorID = (u16)(ConfigData & 0xFFFF);
			PCIeDeviceID = (u16)((ConfigData >> 16) & 0xFFFF);

			if (PCIeVendorID == XPCIEPSU_CFG_FUN_NOT_IMP_MASK) {
				if (PCIeFunNum == 0)
					/*
					 * We don't need to look
					 * any further on this device.
					 */
					break;
			} else {
				XPciePsu_Dbg(
					"\n\rPCIeBus is %02X\r\nPCIeDev is "
					"%02X\r\nPCIeFunc is %02X\r\n",
					BusNum, PCIeDevNum, PCIeFunNum);

				XPciePsu_Dbg(
					"Vendor ID is %04X \r\nDevice ID is "
					"%04X\r\n",
					PCIeVendorID, PCIeDeviceID);

				/* Header Type */
				XPciePsu_ReadConfigSpace(
					InstancePtr, BusNum, PCIeDevNum,
					PCIeFunNum, XPCIEPSU_CFG_CAH_LAT_HD_REG,
					&ConfigData);

				PCIeHeaderType =
					ConfigData & XPCIEPSU_CFG_HEADER_TYPE_MASK;
				PCIeMultiFun =
					ConfigData & XPCIEPSU_CFG_MUL_FUN_DEV_MASK;

				if (PCIeHeaderType == XPCIEPSU_CFG_HEADER_O_TYPE) {
					/* This is an End Point */
					XPciePsu_Dbg("This is an End Point\r\n");

					/*
					 * Write Address to PCIe BAR
					 */
					Ret = XPciePsu_AllocBarSpace(
						InstancePtr, PCIeHeaderType,
						BusNum, PCIeDevNum,
						PCIeFunNum);
					if (Ret != 0)
						return;

					/*
					 * Initialize this end point
					 * and return.
					 */
					XPciePsu_ReadConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_CMD_STATUS_REG,
						&ConfigData);

					ConfigData |= (XPCIEPSU_CFG_CMD_BUSM_EN
						       | XPCIEPSU_CFG_CMD_MEM_EN);

					XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_CMD_STATUS_REG,
						ConfigData);

					XPciePsu_Dbg(
						"End Point has been "
						"enabled\r\n");

					XPciePsu_IncreamentNpMem(InstancePtr);
					XPciePsu_IncreamentPMem(InstancePtr);

				} else {
					/* This is a bridge */
					XPciePsu_Dbg("This is a Bridge\r\n");

					/* alloc bar space and configure bridge
					 */
					Ret = XPciePsu_AllocBarSpace(
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
					XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_BUS_NUMS_T1_REG,
						Adr06);

					/* Update start values of P and NP MMIO
					 * base */
					Adr08 |= ((InstancePtr->Config.NpMemBaseAddr
						   & 0xFFF00000)
						  >> FOUR_HEX_NIBBLES);
					XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_NP_MEM_T1_REG, Adr08);

					Adr09 |= ((InstancePtr->Config.PMemBaseAddr
						   & 0xFFF00000)
						  >> FOUR_HEX_NIBBLES);
					XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_P_MEM_T1_REG, Adr09);
					Adr0A |= (InstancePtr->Config.PMemBaseAddr
						  >> EIGHT_HEX_NIBBLES);
					XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_P_UPPER_MEM_T1_REG,
						Adr0A);

					/* Searches secondary bus devices. */
					XPciePsu_FetchDevicesInBus(InstancePtr,
							  LastBusNum);

					/*
					 * update subordinate bus no
					 * clearing subordinate bus no
					 */
					Adr06 &= (~(0xFF << FOUR_HEX_NIBBLES));
					/* setting subordinate bus no */
					Adr06 |= (LastBusNum
						  << FOUR_HEX_NIBBLES);
					XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_BUS_NUMS_T1_REG,
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
					XPciePsu_IncreamentNpMem(InstancePtr);
					Adr08 |= (InstancePtr->Config.NpMemBaseAddr
						  & 0xFFF00000);
					XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_NP_MEM_T1_REG, Adr08);

					XPciePsu_IncreamentPMem(InstancePtr);
					Adr09 |= (InstancePtr->Config.PMemBaseAddr
						  & 0xFFF00000);
					XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_P_MEM_T1_REG, Adr09);
					Adr0B |= (InstancePtr->Config.PMemBaseAddr
						  >> EIGHT_HEX_NIBBLES);
					XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_P_LIMIT_MEM_T1_REG,
						Adr0B);

					/* Increment P & NP mem to next aligned starting address.
					 *
					 * Eg: As the range is 0xE000 0000 to 0xE01F FFFF.
					 * the next starting address should be 0xE020 0000.
					 */
					XPciePsu_IncreamentNpMem(InstancePtr);
					XPciePsu_IncreamentPMem(InstancePtr);

					/*
					 * Enable configuration
					 */
					XPciePsu_ReadConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_CMD_STATUS_REG,
						&ConfigData);

					ConfigData |= (XPCIEPSU_CFG_CMD_BUSM_EN
						       | XPCIEPSU_CFG_CMD_MEM_EN);

					XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_CMD_STATUS_REG,
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
* @param    InstancePtr pointer to XPciePsu Instance Pointer
*
* @return 	1 if success
* 0 if fails
*
*******************************************************************************/
u8 XPciePsu_EnumerateBus(XPciePsu *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	XPciePsu_FetchDevicesInBus(InstancePtr, 0);
	return XST_SUCCESS;
}

/******************************************************************************/
/**
* This function initializes the config space and PCIe bridge.
*
* @param   InstancePtr pointer to XPciePsu Instance Pointer
* @param   CfgPtr pointer to XPciePsu_Config instrance Pointer.
* @param   EffectiveBrgAddress config brigReg address
*
* @return  XST_SUCCESS on success
*          err on failure
*
*******************************************************************************/
u32 XPciePsu_CfgInitialize(XPciePsu *InstancePtr, XPciePsu_Config *CfgPtr,
			   UINTPTR EffectiveBrgAddress)
{
	s32 Status;

	/* Assert arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);

	/* Clear instance memory and make copy of configuration */
	memset(InstancePtr, 0, sizeof(XPciePsu));
	memcpy(&(InstancePtr->Config), CfgPtr, sizeof(XPciePsu_Config));

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	InstancePtr->Config.BrigReg = EffectiveBrgAddress;

	/* Initialize PCIe PSU bridge */
	Status = XPciePsu_BridgeInit(InstancePtr);
	if (Status != XST_SUCCESS) {
		XPciePsu_Err("Bridge init failed\r\n");
	}

	return Status;
}
