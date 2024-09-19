/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
#include <stdlib.h>
/**************************** Constant Definitions ****************************/

/****************************** Type Definitions ******************************/
/* Structure for to sort the Data */
typedef struct {
        u32 BarAdd;
        u32 BarNo;
} XPciePsu_BarAlloc;

/******************** Macros (Inline Functions) Definitions *******************/

/**************************** Variable Definitions ****************************/


/***************************** Function Prototypes ****************************/

/******************************************************************************/
/**
* This function reads a register value from specified offset
*
* @param   BaseAddr   BaseAddr of the register
* @param   RegOffset  Offset from the base address to be read
*
* @return  Register value
*
*******************************************************************************/
u32 XPciePsu_ReadReg(UINTPTR BaseAddr, u32 RegOffset) {
	return (Xil_In32(BaseAddr + RegOffset));
}

/******************************************************************************/
/**
* This function writes a register value to specified offset
*
* @param   BaseAddr   Base Address of the register
* @param   RegOffset  Offset from the base address to be written
* @param   Val        Value to be written
*
* @return  none
*
*******************************************************************************/
void XPciePsu_WriteReg(UINTPTR BaseAddr, u32 RegOffset, u32 Val) {
	Xil_Out32((BaseAddr + RegOffset), Val);
}

/******************************************************************************/
/**
* This function writes a register value to specified offset
*
* @param   BaseAddr   Base Address of the register
* @param   RegOffset  Offset from the base address to be written
* @param   Val        Value to be written
*
* @return  none
*
*******************************************************************************/
#if defined(__aarch64__) || defined(__arch64__)
void XPciePsu_WriteReg64(UINTPTR BaseAddr, u64 RegOffset, u64 Val) {
	Xil_Out64((BaseAddr + RegOffset), Val);
}
#endif

/******************************************************************************/
/**
* This function gets the Maximum supported Bus Number
*
* @param   EcamSize   Size of the ECAM space
*
* @return  Maximum supported number of buses
*
*******************************************************************************/
static u32 XPciePsu_GetMaxBusNo(u32 EcamSize) {
/* each bus required 1 MB ecam space */
	return (((EcamSize) / (1024U * 1024U)) - 1U);
}

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
	if ((XPciePsu_ReadReg(InstancePtr->PciReg, XPCIEPSU_PS_LINKUP_OFFSET)
	    & XPCIEPSU_XPHY_RDY_LINKUP_BIT) != 0U) {
		Status = (s32)XPCIEPSU_LINKUP_SUCCESS;
	} else {
		Status = (s32)XPCIEPSU_LINKUP_FAIL;
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

	if ((XPciePsu_ReadReg(InstancePtr->PciReg, XPCIEPSU_PS_LINKUP_OFFSET)
	    & XPCIEPSU_PHY_LINKUP_BIT) != 0U) {
		Status = (s32)XPCIEPSU_LINKUP_SUCCESS;
	} else {
		Status = (s32)XPCIEPSU_LINKUP_FAIL;
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
	u32 Retries;
	s32 Status = (s32)XPCIEPSU_LINKUP_FAIL;
	/* check if the link is up or not */
	for (Retries = 0; Retries < XPCIEPSU_LINK_WAIT_MAX_RETRIES; Retries++) {
		if (XPciePsu_PhyReady(InstancePtr) == (s32)XPCIEPSU_LINKUP_SUCCESS) {
			Status =  (s32)XPCIEPSU_LINKUP_SUCCESS;
			break;
		}
		usleep(XPCIEPSU_LINK_WAIT_USLEEP_MIN);
	}

	if (Retries == XPCIEPSU_LINK_WAIT_MAX_RETRIES ) {
		XPciePsu_Err("PHY never came up\r\n");
	}

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
	if (BRegVal == 0U) {
		XPciePsu_Err("BREG is not present\r\n");
		Status = (s32)XST_FAILURE;
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
			   | (u32)CFG_DMA_REG_BAR));

	/* Enable Ingress subtractive decode translation */
	XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_I_ISUB_CONTROL,
			SET_ISUB_CONTROL);

	/* Enable msg filtering details */
	XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_BRCFG_RX_MSG_FILTER,
			  CFG_ENABLE_MSG_FILTER_MASK);

	/* Check for linkup */
	Err = XPciePsu_PcieLinkUpTimeout(CfgPtr);
	if (Err != (s32)XPCIEPSU_LINKUP_SUCCESS){
		Status = (s32)XST_FAILURE;
		goto End;
	}

	ECamVal = XPciePsu_ReadReg(CfgPtr->BrigReg,
				XPCIEPSU_E_ECAM_CAPABILITIES) & E_ECAM_PRESENT;
	if (ECamVal == 0U) {
		XPciePsu_Err("ECAM is not present\r\n");
		Status = (s32)XST_FAILURE;
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
					(((u32)PSU_ECAM_VALUE_DEFAULT) << E_ECAM_SIZE_SHIFT));

	XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_E_ECAM_BASE_LO,
			  LOWER_32_BITS(CfgPtr->Ecam));

	XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_E_ECAM_BASE_HI,
			  UPPER_32_BITS(CfgPtr->Ecam));

	/* Get bus range */
	ECamVal = XPciePsu_ReadReg(CfgPtr->BrigReg, XPCIEPSU_E_ECAM_CONTROL);
	ECamValMin = XPciePsu_ReadReg(CfgPtr->BrigReg,
			XPCIEPSU_E_ECAM_CAPABILITIES);

	ECamSize = (u32)(0x1UL << (((ECamVal & E_ECAM_SIZE_LOC) >> E_ECAM_SIZE_SHIFT) +
                          ((ECamValMin & E_ECAM_SIZE_MIN) >>
                           E_ECAM_SIZE_SHIFT)));

	/* Configure last bus numbers as max possible bus number */
	InstancePtr->MaxSupportedBusNo = XPciePsu_GetMaxBusNo(ECamSize);

	/* Write primary, secondary and subordinate bus numbers */
	ECamVal = FirstBusNo;
	ECamVal |= (FirstBusNo + 1U) << 8U;
	ECamVal |= (InstancePtr->MaxSupportedBusNo << E_ECAM_SIZE_SHIFT);

	XPciePsu_WriteReg(CfgPtr->Ecam, XPCIEPSU_PRIMARY_BUS, ECamVal);

	/* check link up */
	if (XPciePsu_PcieLinkUp(CfgPtr) == (s32)XPCIEPSU_LINKUP_SUCCESS) {
		XPciePsu_Dbg("Link is UP\r\n");

		/* Disable all misc interrupts */
		XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_MSGF_MISC_MASK,
				  (u32)~MSGF_MISC_SR_MASKALL);

		/* Disable all legacy interrupts */
		XPciePsu_WriteReg(CfgPtr->BrigReg, XPCIEPSU_MSGF_LEG_MASK,
				  (u32)~MSGF_LEG_SR_MASKALL);

		Status = (s32)XST_SUCCESS;
	} else {
		XPciePsu_Err("Link is DOWN\r\n");
		Status = (s32)XST_FAILURE;
	}

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
	u32 Location = 0U;
	u32 Data;
	u8 Ret = XST_SUCCESS;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DataPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if ((Bus == 0U) && !((Device == 0U) && (Function == 0U))) {
		*DataPtr = DATA_MASK_32;
	} else if (Bus > InstancePtr->MaxSupportedBusNo) {
		XPciePsu_Dbg("Bus:%d greater than Max supported buses:%d\n",
				Bus, InstancePtr->MaxSupportedBusNo);
		Ret = XST_FAILURE;
	} else {
		/* Compose function configuration space location */
		Location = XPciePsu_ComposeExternalConfigAddress(Bus, Device, Function,
								 Offset);

		/* Read data from that location */
		Data = XPciePsu_ReadReg((InstancePtr->Config.Ecam), Location);

		*DataPtr = Data;
	}

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
	u32 Location = 0U;
	u32 TestWrite = 0U;
	u8 Count = 3U;
	u8 Ret = XST_SUCCESS;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (Bus == 0U) {
		goto End;
	}

	if (Bus > InstancePtr->MaxSupportedBusNo) {
		XPciePsu_Dbg("Bus:%d greater than Max supported buses:%d\n",
				Bus, InstancePtr->MaxSupportedBusNo);
		Ret = XST_FAILURE;
		goto End;
	}

	/* Compose function configuration space location */
        Location = XPciePsu_ComposeExternalConfigAddress(Bus, Device, Function,
                                                         Offset);

	/* Write data to that location */
	XPciePsu_WriteReg((InstancePtr->Config.Ecam), Location, Data);

	/* Read data from that location to verify write */
	while (Count != 0U) {
		TestWrite =
			XPciePsu_ReadReg((InstancePtr->Config.Ecam), Location);

		if (TestWrite == Data) {
			break;
		}

		Count--;
	}
End:
	return Ret;
}

/****************************************************************************/
/**
* Read 32-bit value from one of this IP own configuration space.
* Location is identified by its offset from the beginning of the
* configuration space.
*
* @param 	InstancePtr is the XPciePsu instance to operate on.
* @param 	Offset from beginning of IP own configuration space.
* @param 	DataPtr is a pointer to a variable where the driver will pass
* 		back the value read from the specified location.
*
* @return	XST_SUCCESS on success
*		XST_FAILURE on failure.
*
* @note 	None
*
*****************************************************************************/
u8 XPciePsu_ReadLocalConfigSpace(XPciePsu *InstancePtr, u16 Offset,
					u32 *DataPtr)
{
	u8 Ret = XST_SUCCESS;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DataPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	*DataPtr = XPciePsu_ReadReg((InstancePtr->Config.Ecam),
			(XPCIEPSU_PCIE_CORE_OFFSET + ((u32) (Offset * 4))));

	return Ret;
}

/****************************************************************************/
/**
* Write 32-bit value to one of this IP own configuration space.
* Location is identified by its offset from the beginning of the
* configuration space.
*
* @param 	InstancePtr is the PCIe component to operate on.
* @param 	Offset from beggininng of IP own configuration space.
* @param 	Data to be written to the specified location.
*
* @return	XST_SUCCESS on success
*		XST_FAILURE on failure.
*
* @note 	This function is valid only when IP is configured as a
*		root complex.
*
*****************************************************************************/
u8 XPciePsu_WriteLocalConfigSpace(XPciePsu *InstancePtr, u16 Offset,
					u32 Data)
{
	u8 Ret = XST_SUCCESS;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XPciePsu_WriteReg((InstancePtr->Config.Ecam),
		(XPCIEPSU_PCIE_CORE_OFFSET + ((u32) (Offset * 4))), Data);

	return Ret;
}

static u32 XPciePsu_PositionRightmostSetbit(u64 Size)
{
	u32 Position = 0U;
	u32 Bit = 1U;
	/* ignore 4 bits */
	u64 Size_1 = Size & (~(0xfU));

	while ((Size_1 & Bit) == 0U) {
		Bit = Bit << 1U;
		Position++;
	}

	return Position;
}
/******************************************************************************/
/**
* This function reserves bar memory address.
*
* @param   InstancePtr pointer to XPciePsu Instance Pointer
* @param   BarType differentiate 32 or 64 bit bar type
* @param   Size of the Bar Request
*
* @return  bar address
*
*******************************************************************************/
#if defined(__aarch64__) || defined(__arch64__)
static u64 XPciePsu_ReserveBarMem(XPciePsu *InstancePtr,
				  u8 BarType, u64 Size)
{
	u64 Ret = 0;

        if (BarType & XPCIEPSU_BAR_MEM_TYPE_64) {
                if (BarType & (XPCIEPSU_BAR_MEM_TYPE_64 << 1)) {
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
        } else {

                Ret = InstancePtr->Config.NpMemBaseAddr;
                InstancePtr->Config.NpMemBaseAddr = InstancePtr->Config.NpMemBaseAddr
                                                        + Size;
                Xil_AssertNonvoid(InstancePtr->Config.NpMemBaseAddr <=
                                InstancePtr->Config.NpMemMaxAddr);
        }

	return Ret;
}
#else
static u32 XPciePsu_ReserveBarMem(XPciePsu *InstancePtr, u32 Size)
{
	u32 Ret = 0;

	Ret = InstancePtr->Config.NpMemBaseAddr;
	InstancePtr->Config.NpMemBaseAddr = InstancePtr->Config.NpMemBaseAddr
						+ Size;
	Xil_AssertNonvoid(InstancePtr->Config.NpMemBaseAddr <=
			InstancePtr->Config.NpMemMaxAddr);

	return Ret;
}
#endif
/******************************************************************************/
/**
* This function Compare two different sizes
*
* @param   FirstReqSize is First Variable to compare size
* @param   SecondReqSize is Second Variable to compare size

* @return  Difference of sizes, Zero for Equal
*
*******************************************************************************/

int XPciePsu_CompareSizes(const void* FirstReqSize, const void* SecondReqSize)
{
	u32 FirstSizeAddr = ((XPciePsu_BarAlloc*)FirstReqSize) ->BarAdd;
	u32 SecondSizeAddr = ((XPciePsu_BarAlloc*)SecondReqSize) ->BarAdd;
	/* Compare in descending order (return reverse result) */
	return SecondSizeAddr - FirstSizeAddr;
}

/******************************************************************************/
/**
* This function Aligns BAR Resources
*
* @param   Value Combination of Location and Offset value
* @param   MaxBars Maximum Requested BARs
* @param   Index is a Bar Numbers
*
* @return  void
*
*******************************************************************************/

static void XPciePsu_AlignBarResources(u64* Value,u8 MaxBars, u32* Index)
{

	XPciePsu_BarAlloc BarAllocs[REQ_SIZE];

	for (u8 Num = 0; Num < MaxBars; Num++) {
		BarAllocs[Num].BarAdd = Value[Num];
		BarAllocs[Num].BarNo = Index[Num];
	}

	/* Sort based on Sizes */
	qsort(BarAllocs, MaxBars, sizeof(XPciePsu_BarAlloc), XPciePsu_CompareSizes);

	for (u8 Num = 0; Num < MaxBars; Num++) {
		Value[Num] = BarAllocs[Num].BarAdd;
		Index[Num] = BarAllocs[Num].BarNo;
	}
}

/******************************************************************************/
/**
* This function Allocates Bar Memory
*
* @param   InstancePtr pointer to XPciePsu Instance Pointer
* @param   Bus
* @param   Device
* @param   Function
* @param   Value is a Combination of Location and Offset value
* @param   MaxBars Maximum Requested BARs,6 for EP and 2 for bridge
* @param   Index is a BAR Number
* @param   Size of the Bar Request
* @param   BarAllocControl is to differentiate Unsorted and Sorted Bar Sizes
*
* @return  Void
*
*******************************************************************************/
static void XPciePsu_BarMemoryAlloc(XPciePsu *InstancePtr, u8 Bus,u8 Device,u8 Function,
				u64* Value, u8 MaxBars, u32* Index, u64* Size, u8 BarAllocControl)
{
	u32 Data = DATA_MASK_32;
	u32 Location = 0, Location_1 = 0;
	u32 TestWrite;
	u32 Size_1 = 0;
	u8 BarNo = 0;
	u64 MaxBarSize = 0;
#if defined(__aarch64__) || defined(__arch64__)
	u64 BarAddr;
	u8 MemAs;
#else
	u32 BarAddr;
#endif
	u64 Tmp;
	u8 BarIndex;
	u64 Prefetchable_Size;

	for (BarIndex = 0; BarIndex < MaxBars; BarIndex++) {

		if (BarAllocControl != 0) {
			BarNo = Index[BarIndex];

		} else {

			BarNo = BarIndex;
		}

		/* Compose function configuration space location */
		Location = XPciePsu_ComposeExternalConfigAddress(
			Bus, Device, Function,
			XPCIEPSU_CFG_BAR_BASE_OFFSET + (u16)BarNo);

		if (BarAllocControl == 0) {
			Index[BarNo] = BarNo;
		}

		/* Write Data to that location */
		XPciePsu_WriteReg((InstancePtr->Config.Ecam), Location, Data);

		Size[BarNo] = XPciePsu_ReadReg((InstancePtr->Config.Ecam), Location);

		if ((Size[BarNo] & XPCIEPSU_CFG_BAR_MEM_AS_MASK ) && (Size[BarNo] != (~((u64)0x0)))) {

			Location_1 = XPciePsu_ComposeExternalConfigAddress(
				Bus, Device, Function,
				XPCIEPSU_CFG_BAR_BASE_OFFSET + ((u16)BarNo + 1U));

			XPciePsu_WriteReg((InstancePtr->Config.Ecam), Location_1, Data);

			Size_1 = XPciePsu_ReadReg((InstancePtr->Config.Ecam), Location_1);
			Prefetchable_Size = (((u64)Size_1 << 32) | Size[BarNo]);
			Size[BarNo] = Prefetchable_Size;
		}

		/* return saying that BAR is not implemented */
		if ((Size[BarNo] & (~((u64)0xfU))) == 0x00U) {
			continue;
		}

		/* check for IO space or memory space */
		if ((Size[BarNo] & XPCIEPSU_CFG_BAR_MEM_TYPE_MASK) ==
			XPCIEPSU_BAR_MEM_TYPE_IO) {
			continue;
		}

		 /* check for 32 bit AS or 64 bit AS  */
		if ((Size[BarNo] & XPCIEPSU_CFG_BAR_MEM_AS_MASK) != 0U) {
#if defined(__aarch64__) || defined(__arch64__)
			/* 64 bit AS is required */
			MemAs = Size[BarNo];

			MaxBarSize = InstancePtr->Config.PMemMaxAddr - InstancePtr->Config.PMemBaseAddr;
			TestWrite = XPciePsu_PositionRightmostSetbit(Size[BarNo]);

			/* Store the Data into Array */
			Value[BarNo] = 2 << (TestWrite-1);

			if (BarAllocControl != 0) {
				if(Value[BarNo] > MaxBarSize) {

					XPciePsu_Dbg(
						"Requested BAR size of %uK for bus: %02X, dev: %02X, "
						"function: %02X is out of range \n",
						(Value[BarNo] / 1024),Bus,Device,Function);

					return XST_SUCCESS;
				}
				/* actual bar size is 2 << TestWrite */
				BarAddr =
					XPciePsu_ReserveBarMem(
						InstancePtr, MemAs,
						((u64)2 << (TestWrite - 1U)));

				Tmp = (u32)BarAddr;

				/* Write actual bar address here */
				XPciePsu_WriteReg((InstancePtr->Config.Ecam), Location,
						Tmp);

				Tmp = (u32)(BarAddr >> 32U);

				/* Write actual bar address here */
				XPciePsu_WriteReg((InstancePtr->Config.Ecam),
					Location_1, Tmp);

			}
#else
			TestWrite = XPciePsu_PositionRightmostSetbit(Size[BarNo]);
			Value[BarNo] = 2 << (TestWrite-1);

			if (BarAllocControl != 0) {
			/* actual bar size is 2 << TestWrite */
			BarAddr =
				XPciePsu_ReserveBarMem(
						InstancePtr,
						((u32)2U << (TestWrite - 1U)));

			Tmp = (u32)BarAddr;

			/* Write actual bar address here */
			XPciePsu_WriteReg((InstancePtr->Config.Ecam), Location,
					Tmp);
		}
#endif
		} else {

			MaxBarSize = InstancePtr->Config.NpMemMaxAddr - InstancePtr->Config.NpMemBaseAddr;
			TestWrite = XPciePsu_PositionRightmostSetbit(Size[BarNo]);
			Value[BarNo] = 2 << (TestWrite-1);

#if defined(__aarch64__) || defined(__arch64__)

			/* 32 bit AS is required */
			MemAs = Size[BarNo];

			if (BarAllocControl != 0) {
				if(Value[BarNo] > MaxBarSize) {

					XPciePsu_Dbg(
						"Requested BAR size of %uK for bus: %02X, dev: %02X, "
						"function: %02X is out of range \n",
						(Value[BarNo] / 1024),Bus,Device,Function);
					return XST_SUCCESS;
				}

			/* actual bar size is 2 << TestWrite */
			BarAddr =
				XPciePsu_ReserveBarMem(
					InstancePtr, MemAs,
					((u64)2U << (TestWrite - 1U)));
			}
#else
			if (BarAllocControl != 0 ) {
			/* actual bar size is 2 << TestWrite */
			BarAddr =
				XPciePsu_ReserveBarMem(
					InstancePtr,
					((u32)2U << (TestWrite - 1U)));
			}
#endif
			if (BarAllocControl != 0) {
				Tmp = (u32)BarAddr;

				/* Write actual bar address here */
				XPciePsu_WriteReg((InstancePtr->Config.Ecam), Location,
						Tmp);
			}
		}
		/* no need to probe next bar if present BAR requires 64 bit AS */
		if ((Size[BarNo] & XPCIEPSU_CFG_BAR_MEM_AS_MASK) != 0U) {
			BarIndex = BarIndex + 1U;
		}
	}
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
	u8 BarAllocControl = 0;
	u8 MaxBars = 0;
	u64 ReqSize;
	u32 Location;
	u32 Position;
#if defined(__aarch64__) || defined(__arch64__)
	u64 BarAddr, ReqAddr;
	u32 Location_1;
	u32 *PPtr;
	u64 ReqBar, ReqBar_1;
	u64 MaxBarSize;
#else
	u32 ReqAddr;
	u32 MaxBarSize;
#endif

	/*Static array declarations for BAR Alignment */
	u64 Value[REQ_SIZE];
	u32 Index[REQ_SIZE];
	u64 Size[REQ_SIZE];

	/* Initialize memory with 0*/
	(void)memset(Value, 0, sizeof(Value));
	(void)memset(Index, 0, sizeof(Index));
	(void)memset(Size, 0, sizeof(Size));

	if (Headertype == XPCIEPSU_CFG_HEADER_O_TYPE) {
		/* For endpoints */
		MaxBars = 6;
	} else {
		/* For Bridge*/
		MaxBars = 2;
	}

	/* get original memory allocation based on location and Size */
	XPciePsu_BarMemoryAlloc(InstancePtr, Bus, Device, Function, Value,
			MaxBars, Index, Size, BarAllocControl);

	/* align values and index */
	XPciePsu_AlignBarResources(Value, MaxBars , Index);

	/* differentiate sorted sizes and unsorted sizes */
	BarAllocControl = 1;

	/* allocates bar address after sorted, based on Index and Size */
	XPciePsu_BarMemoryAlloc(InstancePtr, Bus, Device, Function, Value,
			MaxBars, Index, Size, BarAllocControl);

	/* prints all BARs that have been allocated memory */
	for (u8 Bar=0 ; Bar < MaxBars; Bar++ ) {

		if ((Size[Bar] & (~((u64)0xfU))) == 0x00U) {
		   /* return saying that BAR is not implemented */
			XPciePsu_Dbg(
				"bus: %02X, device: %02X, function: %02X: BAR %d is "
				"not implemented\r\n",
				Bus, Device, Function, Bar);
			continue;
		}

		/* check for IO space or memory space */
		if ((Size[Bar] & XPCIEPSU_CFG_BAR_MEM_TYPE_MASK) ==
				XPCIEPSU_BAR_MEM_TYPE_IO) {
			/* Device required IO address space */
			XPciePsu_Dbg(
				"bus: %02X, device: %02X, function: %02X: BAR %d "
				"required IO space; it is unassigned\r\n",
				Bus, Device, Function, Bar);
			continue;
		}

		/* check for 32 bit AS or 64 bit AS */
		if ((Size[Bar] & XPCIEPSU_CFG_BAR_MEM_AS_MASK) != 0U) {
#if defined(__aarch64__) || defined(__arch64__)

			MaxBarSize = InstancePtr->Config.PMemMaxAddr - InstancePtr->Config.PMemBaseAddr;
			Location = XPciePsu_ComposeExternalConfigAddress(Bus, Device, Function,
					XPCIEPSU_CFG_BAR_BASE_OFFSET + (u16)Bar);

			ReqBar = XPciePsu_ReadReg((InstancePtr->Config.Ecam), Location);

			Location_1 = XPciePsu_ComposeExternalConfigAddress(Bus, Device, Function,
					XPCIEPSU_CFG_BAR_BASE_OFFSET + ((u16)Bar + 1U));

			ReqBar_1 = XPciePsu_ReadReg((InstancePtr->Config.Ecam), Location_1);

			/* Merge two bars for size */
			PPtr = (u32 *)&BarAddr;
			*PPtr = ReqBar;
			*(PPtr + 1) = ReqBar_1;
			ReqAddr = BarAddr;
			ReqAddr = ReqAddr & (~0xF);

			Position = XPciePsu_PositionRightmostSetbit(Size[Bar]);
			ReqSize = (2 << (Position -1));

			/* Compare Bar request size with max Bar size */
			if(ReqSize > MaxBarSize) {
				return XST_SUCCESS;
			}
#endif
		} else {

			MaxBarSize = InstancePtr->Config.NpMemMaxAddr - InstancePtr->Config.NpMemBaseAddr;
			Position = XPciePsu_PositionRightmostSetbit(Size[Bar]);
			ReqSize = (2 << (Position -1));
			Location = XPciePsu_ComposeExternalConfigAddress(Bus, Device, Function,
						XPCIEPSU_CFG_BAR_BASE_OFFSET + (u16)Bar);

			ReqAddr = XPciePsu_ReadReg((InstancePtr->Config.Ecam), Location);
			ReqAddr = ReqAddr & (~0xF);

			/* Compare Bar request size with max Bar size */
			if(ReqSize > MaxBarSize) {
				return XST_SUCCESS;
			}

		}

		XPciePsu_Dbg("bus: %02X, device: %02X, function: %02X: BAR %d, "
				"ADDR: 0x%p size : %dK\r\n",
				Bus, Device, Function, Bar, ReqAddr, (ReqSize / 1024UL));

		if ((Size[Bar] & XPCIEPSU_CFG_BAR_MEM_AS_MASK) != 0U) {
			Bar = Bar + 1U;
		}
	}

	return (s32)XST_SUCCESS;
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

#if defined(__aarch64__) || defined(__arch64__)
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
#endif

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
static void XPciePsu_FetchDevicesInBus(XPciePsu *InstancePtr, u8 BusNum)
{
	u32 ConfigData = 0;
	static u8 LastBusNum;

	u16 PCIeVendorID;
	u16 PCIeDeviceID;
	u32 PCIeHeaderType;
	u32 PCIeMultiFun;

	u32 Adr06; /* Latency timer */
	u32 Adr08;
#if defined(__aarch64__) || defined(__arch64__)
	u32 Adr09;
	u32 Adr10;
	u32 Adr11;
#endif

	int Ret;

	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (BusNum > InstancePtr->MaxSupportedBusNo) {
		/* End of bus size */
		return;
	}

	for (u8 PCIeDevNum = 0U; PCIeDevNum < XPCIEPSU_CFG_MAX_NUM_OF_DEV;
	     PCIeDevNum++) {
		for (u8 PCIeFunNum = 0U; PCIeFunNum < XPCIEPSU_CFG_MAX_NUM_OF_FUN;
		     PCIeFunNum++) {

			/* Vendor ID */
			if (XPciePsu_ReadConfigSpace(
				InstancePtr, BusNum, PCIeDevNum, PCIeFunNum,
				XPCIEPSU_CFG_ID_REG, &ConfigData) != (u8)XST_SUCCESS) {
				return;
			}

			PCIeVendorID = (u16)(ConfigData & 0xFFFFU);
			PCIeDeviceID = (u16)((ConfigData >> 16U) & 0xFFFFU);

			if (PCIeVendorID == XPCIEPSU_CFG_FUN_NOT_IMP_MASK) {
				if (PCIeFunNum == 0U) {
					/*
					 * We don't need to look
					 * any further on this device.
					 */
					break;
				}
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
				if (XPciePsu_ReadConfigSpace(
					InstancePtr, BusNum, PCIeDevNum,
					PCIeFunNum, XPCIEPSU_CFG_CAH_LAT_HD_REG,
					&ConfigData) != (u8)XST_SUCCESS) {
					return;
				}

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
					if (Ret != (s32)XST_SUCCESS) {
						return;
					}

					/*
					 * Initialize this end point
					 * and return.
					 */
					if (XPciePsu_ReadConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_CMD_STATUS_REG,
						&ConfigData) != (u8)XST_SUCCESS) {
						return;
					}

					ConfigData |= (XPCIEPSU_CFG_CMD_BUSM_EN
						       | XPCIEPSU_CFG_CMD_MEM_EN);

					if (XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_CMD_STATUS_REG,
						ConfigData) != (u8)XST_SUCCESS) {
						return;
					}

					XPciePsu_Dbg(
						"End Point has been "
						"enabled\r\n");

					XPciePsu_IncreamentNpMem(InstancePtr);
#if defined(__aarch64__) || defined(__arch64__)
					XPciePsu_IncreamentPMem(InstancePtr);
#endif

				} else {
					/* This is a bridge */
					XPciePsu_Dbg("This is a Bridge\r\n");

					/* alloc bar space and configure bridge
					 */
					Ret = XPciePsu_AllocBarSpace(
						InstancePtr, PCIeHeaderType,
						BusNum, PCIeDevNum,
						PCIeFunNum);

					if (Ret != (s32)XST_SUCCESS) {
						continue;
					}

					Adr06 = 0x0; /* Latency timer */
					Adr08 = 0x0;
#if defined(__aarch64__) || defined(__arch64__)
					Adr09 = 0x0;
					Adr10 = 0x0;
					Adr11 = 0x0;
#endif

					/* Sets primary and secondary bus
					 * numbers */
					Adr06 <<= TWO_HEX_NIBBLES;
					Adr06 |= 0xFFU; /* sub ordinate bus no 0xF
						     */
					Adr06 <<= TWO_HEX_NIBBLES;
					LastBusNum++;
					Adr06 |= LastBusNum; /* secondary
							      bus no */
					Adr06 <<= TWO_HEX_NIBBLES;
					Adr06 |= BusNum; /* Primary bus no */
					if (XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_BUS_NUMS_T1_REG,
						Adr06) != (u8)XST_SUCCESS) {
						return;
					}

					/* Update start values of P and NP MMIO
					 * base */
					Adr08 |= ((InstancePtr->Config.NpMemBaseAddr
						   & 0xFFF00000U)
						  >> FOUR_HEX_NIBBLES);
					if (XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_NP_MEM_T1_REG, Adr08) != (u8)XST_SUCCESS) {
						return;
					}

#if defined(__aarch64__) || defined(__arch64__)
					Adr09 |= (u32)((InstancePtr->Config.PMemBaseAddr
						   & 0xFFF00000U)
						  >> FOUR_HEX_NIBBLES);
					if (XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_P_MEM_T1_REG, Adr09) != (u8)XST_SUCCESS) {
						return;
					}

					Adr10 |= (u32)(InstancePtr->Config.PMemBaseAddr
						  >> EIGHT_HEX_NIBBLES);
					if (XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_P_UPPER_MEM_T1_REG,
						Adr10) != (u8)XST_SUCCESS) {
						return;
					}
#endif

					/* Searches secondary bus devices. */
					XPciePsu_FetchDevicesInBus(InstancePtr,
							  LastBusNum);

					/*
					 * update subordinate bus no
					 * clearing subordinate bus no
					 */
					Adr06 &= (~(((u32)0xFFU) << FOUR_HEX_NIBBLES));
					/* setting subordinate bus no */
					Adr06 |= (u32)LastBusNum
						  << FOUR_HEX_NIBBLES;
					if (XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_BUS_NUMS_T1_REG,
						Adr06) != (u8)XST_SUCCESS) {
						return;
					}

					/*
					 * Update end values of MMIO limit
					 */

					/*
					 * Align memory to 1 Mb boundary.
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
						  & 0xFFF00000U);
					if (XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_NP_MEM_T1_REG, Adr08) != (u8)XST_SUCCESS) {
						return;
					}

#if defined(__aarch64__) || defined(__arch64__)
					XPciePsu_IncreamentPMem(InstancePtr);
					Adr09 |= (u32)(InstancePtr->Config.PMemBaseAddr
						  & 0xFFF00000U);
					if (XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_P_MEM_T1_REG, Adr09) != (u8)XST_SUCCESS) {
						return;
					}
					Adr11 |= (u32)(InstancePtr->Config.PMemBaseAddr
						  >> EIGHT_HEX_NIBBLES);
					if (XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_P_LIMIT_MEM_T1_REG,
						Adr11) != (u8)XST_SUCCESS) {
						return;
					}
#endif

					/* Increment P & NP mem to next aligned starting address.
					 *
					 * Eg: As the range is 0xE000 0000 to 0xE01F FFFF.
					 * the next starting address should be 0xE020 0000.
					 */
					XPciePsu_IncreamentNpMem(InstancePtr);
#if defined(__aarch64__) || defined(__arch64__)
					XPciePsu_IncreamentPMem(InstancePtr);
#endif

					/*
					 * Enable configuration
					 */
					if (XPciePsu_ReadConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_CMD_STATUS_REG,
						&ConfigData) != (u8)XST_SUCCESS) {
						return;
					}

					ConfigData |= (XPCIEPSU_CFG_CMD_BUSM_EN
						       | XPCIEPSU_CFG_CMD_MEM_EN);

					if (XPciePsu_WriteConfigSpace(
						InstancePtr, BusNum,
						PCIeDevNum, PCIeFunNum,
						XPCIEPSU_CFG_CMD_STATUS_REG,
						ConfigData) != (u8)XST_SUCCESS) {
						return;
					}
				}
			}
			if ((PCIeFunNum == 0U) && (PCIeMultiFun == 0U)) {
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
	XPciePsu_FetchDevicesInBus(InstancePtr, 0U);

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
u32 XPciePsu_CfgInitialize(XPciePsu *InstancePtr, const XPciePsu_Config *CfgPtr,
			   UINTPTR EffectiveBrgAddress)
{
	s32 Status;

	/* Assert arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);

	/* Clear instance memory and make copy of configuration */
	(void)memset(InstancePtr, 0, sizeof(XPciePsu));
	(void)memcpy(&(InstancePtr->Config), CfgPtr, sizeof(XPciePsu_Config));

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	InstancePtr->Config.BrigReg = EffectiveBrgAddress;

	/* Initialize PCIe PSU bridge */
	Status = XPciePsu_BridgeInit(InstancePtr);
	if (Status != XST_SUCCESS) {
		XPciePsu_Err("Bridge init failed\r\n");
	}

	if (InstancePtr->Config.Ecam == InstancePtr->Config.NpMemBaseAddr) {
		InstancePtr->Config.NpMemBaseAddr += XPCIEPSU_ECAM_MEMSIZE;
	}

	return (u32)Status;
}
