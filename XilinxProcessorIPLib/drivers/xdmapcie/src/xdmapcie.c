/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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

#include <stdlib.h>
/*************************** Constant Definitions ***************************/

/* Structure for to sort the Data */
typedef struct {
        u32 BarAdd;
        u32 BarNo;
} XDmaPcie_BarAlloc;
/***************************** Type Definitions *****************************/

/****************** Macros (Inline Functions) Definitions *******************/
#ifdef QDMA_PCIE_BRIDGE
#define BDF_ENTRY_ADDR_LO              0x2420
#define BDF_ENTRY_ADDR_HI              0x2424
#define BDF_ENTRY_PASID                        0x2428
#define BDF_ENTRY_FUNCTION             0x242C
#define BDF_ENTRY_WINDOW               0x2430
#define BDF_ENTRY_REG                  0x2434

#define BDF_NUM_WINDOWS                        8
#define BDF_ADDR_BOUNDARY              4096
#define BDF_TABLE_ENTRY_OFF            0x20
#define BDF_ACCESS_PERM                        0xC0000000

/*************************** Variable Definitions ***************************/

/*************************** Function Prototypes ****************************/


/**
* Additional programming for QDMA bridges for BAR access
*/
void XDmaPcie_QdmaAddPgm(XDmaPcie *InstancePtr)
{
       u32 i, Size;

       Size = InstancePtr->Config.NpMemMaxAddr - InstancePtr->Config.NpMemBaseAddr;

       for (i = 0; i < BDF_NUM_WINDOWS; i++) {
               Xil_Out32(InstancePtr->Config.BaseAddress + BDF_ENTRY_ADDR_LO + (i * BDF_TABLE_ENTRY_OFF),
                         InstancePtr->Config.NpMemBaseAddr + (i * (Size/BDF_NUM_WINDOWS)));
               Xil_Out32(InstancePtr->Config.BaseAddress + BDF_ENTRY_ADDR_HI + (i * BDF_TABLE_ENTRY_OFF), 0x0);
               Xil_Out32(InstancePtr->Config.BaseAddress + BDF_ENTRY_ADDR_HI + (i * BDF_TABLE_ENTRY_OFF), 0x0);
               Xil_Out32(InstancePtr->Config.BaseAddress + BDF_ENTRY_PASID + (i * BDF_TABLE_ENTRY_OFF), 0x0);
               Xil_Out32(InstancePtr->Config.BaseAddress + BDF_ENTRY_FUNCTION + (i * BDF_TABLE_ENTRY_OFF), 0x0);
               Xil_Out32(InstancePtr->Config.BaseAddress + BDF_ENTRY_WINDOW + (i * BDF_TABLE_ENTRY_OFF),
                         BDF_ACCESS_PERM + (Size/(BDF_NUM_WINDOWS * BDF_ADDR_BOUNDARY)));
               Xil_Out32(InstancePtr->Config.BaseAddress + BDF_ENTRY_REG + (i * BDF_TABLE_ENTRY_OFF), 0x0);
       }

#if defined(__aarch64__) || defined(__arch64__)
       if (InstancePtr->Config.PMemBaseAddr == 0x0) {
               return;
       }

       Size = InstancePtr->Config.PMemMaxAddr - InstancePtr->Config.PMemBaseAddr;

       for (i = 0; i < BDF_NUM_WINDOWS; i++) {
               Xil_Out32(InstancePtr->Config.BaseAddress + BDF_ENTRY_ADDR_LO + (i * BDF_TABLE_ENTRY_OFF),
                         InstancePtr->Config.PMemBaseAddr + (i * (Size/BDF_NUM_WINDOWS)));
               Xil_Out32(InstancePtr->Config.BaseAddress + BDF_ENTRY_ADDR_HI + (i * BDF_TABLE_ENTRY_OFF),
                         ((InstancePtr->Config.PMemBaseAddr >> 16) >> 16));
               Xil_Out32(InstancePtr->Config.BaseAddress + BDF_ENTRY_ADDR_HI + (i * BDF_TABLE_ENTRY_OFF),
                         ((InstancePtr->Config.PMemBaseAddr >> 16) >> 16));
               Xil_Out32(InstancePtr->Config.BaseAddress + BDF_ENTRY_PASID + (i * BDF_TABLE_ENTRY_OFF), 0x0);
               Xil_Out32(InstancePtr->Config.BaseAddress + BDF_ENTRY_FUNCTION + (i * BDF_TABLE_ENTRY_OFF), 0x0);
               Xil_Out32(InstancePtr->Config.BaseAddress + BDF_ENTRY_WINDOW + (i * BDF_TABLE_ENTRY_OFF),
                         BDF_ACCESS_PERM + (Size/(BDF_NUM_WINDOWS * BDF_ADDR_BOUNDARY)));
               Xil_Out32(InstancePtr->Config.BaseAddress + BDF_ENTRY_REG + (i * BDF_TABLE_ENTRY_OFF), 0x0);
       }
#endif
}
#endif
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
#if !defined(versal) && !defined(QDMA_PCIE_BRIDGE)
	u32 Data;
#endif

	/* Assert arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);

	/* Clear instance memory and make copy of configuration */
	memset(InstancePtr, 0, sizeof(XDmaPcie));
	memcpy(&InstancePtr->Config, CfgPtr, sizeof(XDmaPcie_Config));

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	InstancePtr->Config.BaseAddress = EffectiveAddress;

#if defined(QDMA_PCIE_BRIDGE)
	#if defined(versal)
		InstancePtr->Config.BaseAddress = InstancePtr->Config.Ecam;
		InstancePtr->Config.Ecam = 0x80000000;
        #else
		InstancePtr->Config.Ecam = 0xA0000000;
        #endif
		XDmaPcie_QdmaAddPgm(InstancePtr);
#endif

#if defined(SDT) && defined(versal) && !defined(QDMA_PCIE_BRIDGE) && !defined(XDMA_PCIE_BRIDGE) && !defined(versal_2ve_2vm)
	InstancePtr->Config.BaseAddress= InstancePtr->Config.Ecam;
	InstancePtr->Config.Ecam= EffectiveAddress;
#endif

	if (InstancePtr->Config.Ecam == InstancePtr->Config.NpMemBaseAddr)
		InstancePtr->Config.NpMemBaseAddr += XDMAPCIE_ECAM_MEMSIZE;

	/* Disable all interrupts */
	XDmaPcie_DisableInterrupts(InstancePtr, XDMAPCIE_IM_DISABLE_ALL_MASK);

	/* Max number of buses */
#if defined(versal) || defined(QDMA_PCIE_BRIDGE) || defined(versal_2ve_2vm)
	InstancePtr->MaxNumOfBuses = XDMAPCIE_NUM_BUSES;
#else
	Data = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XDMAPCIE_BI_OFFSET);
	InstancePtr->MaxNumOfBuses = (u16)((Data & XDMAPCIE_BI_ECAM_SIZE_MASK) >>
					XDMAPCIE_BI_ECAM_SIZE_SHIFT);
#endif

	return (XST_SUCCESS);
}

#if defined(__aarch64__) || defined(__arch64__)
static u8 XdmaPcie_IsValidAddr(u64 Addr)
{
	if (Addr == 0U) {
		return FALSE;
	} else {
		return TRUE;
	}
}

/******************************************************************************/
/**
* This function reserves bar memory address.
*
* @param   InstancePtr pointer to XDmaPcie Instance Pointer
* @param   BarType differentiate 32 or 64 bit bar type
* @param   Size of the Bar Request
*
* @return  bar address
*
*******************************************************************************/
static u64 XDmaPcie_ReserveBarMem(XDmaPcie *InstancePtr,
		u8 BarType, u64 Size)
{
	u64 Ret = 0;

	if ((BarType & XDMAPCIE_BAR_MEM_TYPE_64) &&
		(BarType & (XDMAPCIE_BAR_MEM_TYPE_64 << 1))) {
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
	return Ret;
}
#else
static u32 XDmaPcie_ReserveBarMem(XDmaPcie *InstancePtr, u32 Size)
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
* This function Compare two different sizes
*
* @param   FirstReqSize is First Variable to compare size
* @param   SecondReqSize is Second Variable to compare size

* @return  Difference of sizes, Zero for Equal
*
*******************************************************************************/

int XDmaPcie_CompareSizes(const void* FirstReqSize, const void* SecondReqSize)
{
	u32 FirstSizeAddr = ((XDmaPcie_BarAlloc*)FirstReqSize) ->BarAdd;
	u32 SecondSizeAddr = ((XDmaPcie_BarAlloc*)SecondReqSize) ->BarAdd;
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

static void XDmaPcie_AlignBarResources(u64* Value,u8 MaxBars, u32* Index)
{

	XDmaPcie_BarAlloc BarAllocs[REQ_SIZE];

	for (u8 Num = 0; Num < MaxBars; Num++) {
		BarAllocs[Num].BarAdd = Value[Num];
		BarAllocs[Num].BarNo = Index[Num];
	}

	/* Sort based on Sizes */
	qsort(BarAllocs, MaxBars, sizeof(XDmaPcie_BarAlloc), XDmaPcie_CompareSizes);

	for (u8 Num = 0; Num < MaxBars; Num++) {
		Value[Num] = BarAllocs[Num].BarAdd;
		Index[Num] = BarAllocs[Num].BarNo;
	}

}
/******************************************************************************/
/**
* This function calculates the size of the BAR requested by the endpoint and
* allocates appropriate memory space from the available non-prefetchable memory
* window. It supports both AArch64 and other architectures via conditional compilation.
*
* If the requested BAR size exceeds the available non-prefetchable memory, the
* function logs an error and returns without allocation.
*
* @param   InstancePtr pointer to XDmaPcie Instance Pointer
* @param   Location BAR configuration offset within the ECAM space.
* @param   Bus is PCIe Bus number of the Endpoint
* @param   Device is PCIe Device number of the Endpoint.
* @param   Function is PCIe Function number of the Endpoint.
* @param   Value is a Combination of Location and Offset value
* @param   Size of the Bar Request
* @param   BarAllocControl is to differentiate Unsorted and Sorted Bar Sizes
* @param   BarNo is the BAR index (0 to 5).
*
* @return  Void
*
*******************************************************************************/
static void XDmaPcie_Assign32bBarMem(XDmaPcie *InstancePtr, u32 Location, u8 Bus, u8 Device,
	u8 Function, u64* Value, u64* Size, u8 BarAllocControl, u8 BarNo)
{
	u32 BarAddr = 0;
	u32 MaxBarSize = 0;
	u32 TestWrite = 0;
	u32 BarAddrLo = 0;
	#if defined(__aarch64__) || defined(__arch64__)
	u8 MemAs = 0;
	#endif

	MaxBarSize = InstancePtr->Config.NpMemMaxAddr - InstancePtr->Config.NpMemBaseAddr;
	TestWrite = XDmaPcie_PositionRightmostSetbit(Size[BarNo]);
	Value[BarNo] = 2<<(TestWrite-1);

	/* Store the Data into Array */
	if (BarAllocControl != 0) {
		if(Value[BarNo] > MaxBarSize) {
			XDmaPcie_Dbg(
				"Requested BAR size of %uK for bus: %02X, dev: %02X, "
				"function: %02X is out of range \n",
				(Value[BarNo] / 1024),Bus,Device,Function);
				return;
		}
	}

	if (BarAllocControl != 0) {
#if defined(__aarch64__) || defined(__arch64__)
		MemAs = Size[BarNo];
		/* actual bar size is 2 << TestWrite */
		BarAddr =
			XDmaPcie_ReserveBarMem(InstancePtr, MemAs,
				((u64)2 << (TestWrite - 1U)));
#else
		BarAddr =
			XDmaPcie_ReserveBarMem(InstancePtr,
				((u64)2 << (TestWrite - 1U)));
#endif

		BarAddrLo = (u32)BarAddr;
		/* Write actual bar address here */
		XDmaPcie_WriteReg((InstancePtr->Config.Ecam), Location,
				BarAddrLo);
	}
}
/******************************************************************************/
/**
* This function Allocates Bar Memory
*
* @param   InstancePtr pointer to XDmaPcie Instance Pointer
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
static void XDmaPcie_BarMemoryAlloc(XDmaPcie *InstancePtr, u8 Bus,u8 Device,u8 Function,
			u64* Value, u8 MaxBars, u32* Index, u64* Size, u8 BarAllocControl)
{
	u32 Data = DATA_MASK_32;
	u32 Location = 0, Location_1 = 0;
	u32 Size_1 = 0;
	u8  BarNo = 0;
#if defined(__aarch64__) || defined(__arch64__)
	u64 BarAddr = 0;
	u64 MaxBarSize = 0;
	u8 MemAs = 0;
	u64 BarAddrLo = 0;
	u32 TestWrite = 0;
#endif
	u8  BarIndex;
	u64 Prefetchable_Size;

	for (BarIndex = 0; BarIndex < MaxBars; BarIndex++) {

		if (BarAllocControl != 0) {
			BarNo = Index[BarIndex];

			if (((Size[BarNo] & XDMAPCIE_CFG_BAR_MEM_AS_MASK) != 0U) &&
				((Size[BarNo] & XDMAPCIE_CFG_BAR_MEM_TYPE_MASK) != 0x1)) {

				BarIndex = BarNo;
			}
		} else {
			BarNo = BarIndex;
		}

		/* Compose function configuration space location */
		Location = XDmaPcie_ComposeExternalConfigAddress(
			Bus, Device, Function,
			XDMAPCIE_CFG_BAR_BASE_OFFSET + BarNo);

		/* Write data to that location */
		XDmaPcie_WriteReg((InstancePtr->Config.Ecam), Location, Data);

		Size[BarNo] = XDmaPcie_ReadReg((InstancePtr->Config.Ecam), Location);

		if (BarAllocControl == 0) {
			Index[BarNo] = BarNo;
		}

		if((BarIndex + 1) < MaxBars) {
			if ((Size[BarNo] & XDMAPCIE_CFG_BAR_MEM_TYPE_MASK) != 1U) {
				if (((Size[BarNo] & XDMAPCIE_CFG_BAR_MEM_AS_MASK) != 0) &&
					(Size[BarNo] != (~((u64)0x0)))) {

				Location_1 = XDmaPcie_ComposeExternalConfigAddress(
					Bus, Device, Function,
					XDMAPCIE_CFG_BAR_BASE_OFFSET + (BarNo + 1U));
				XDmaPcie_WriteReg((InstancePtr->Config.Ecam), Location_1, Data);

				Size_1 = XDmaPcie_ReadReg((InstancePtr->Config.Ecam), Location_1);
				Prefetchable_Size = ((u64)Size_1 << 32) | Size[BarNo];
				Size[BarNo] = Prefetchable_Size;
				}
			}
		}

		/* return saying that BAR is not implemented */
		if (XDMAPCIE_IS_BAR_UNIMPLEMENTED(Size[BarNo])) {
			continue;
		}

		/* check for IO space or memory space */
		if (Size[BarNo] & XDMAPCIE_CFG_BAR_MEM_TYPE_MASK) {
			continue;
		}

#if defined(__aarch64__) || defined(__arch64__)
		 /* check for 32 bit AS or 64 bit AS  */
		if ((Size[BarNo] & XDMAPCIE_CFG_BAR_MEM_AS_MASK) != 0U) {
			/* 64 bit AS is required */
			MemAs = Size[BarNo];
			MaxBarSize = InstancePtr->Config.PMemMaxAddr - InstancePtr->Config.PMemBaseAddr;

			TestWrite = XDmaPcie_PositionRightmostSetbit(Size[BarNo]);

			/* Store the Data into Array */
			Value[BarNo] = 2<<(TestWrite-1);

			if (BarAllocControl != 0) {

				if(Value[BarNo] > MaxBarSize) {
					XDmaPcie_Dbg(
						"Requested BAR size of %uK for bus: %02X, dev: %02X, "
						"function: %02X is out of range \n",
						(Value[BarNo] / 1024),Bus,Device,Function);
					return;
				}

				/* actual bar size is 2 << TestWrite */
				BarAddr =
					XDmaPcie_ReserveBarMem(InstancePtr, MemAs,
						((u64)2 << (TestWrite - 1U)));

				BarAddrLo = (u32)BarAddr;

				/* Write actual bar address here */
				XDmaPcie_WriteReg((InstancePtr->Config.Ecam), Location,
					  BarAddrLo);

				BarAddrLo = (u32)(BarAddr >> 32U);

				/* Write actual bar address here */
				XDmaPcie_WriteReg((InstancePtr->Config.Ecam),
						Location_1, BarAddrLo);
			}

		} else {
			XDmaPcie_Assign32bBarMem(InstancePtr, Location, Bus,Device,Function,
				Value,Size, BarAllocControl,BarNo);
		}
#else
		XDmaPcie_Assign32bBarMem(InstancePtr, Location, Bus,Device,Function,
			Value,Size, BarAllocControl,BarNo);
#endif
		/* no need to probe next bar if present BAR requires 64 bit AS */
		if ((Size[BarNo] & XDMAPCIE_CFG_BAR_MEM_AS_MASK) != 0U) {
			BarIndex = BarIndex + 1U;
		}
	}
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
	u8  BarAllocControl = 0;
	u8  MaxBars = 0;
	u64 ReqSize = 0;
	u32 Location = 0;
	u32 Position = 0;
#if defined(__aarch64__) || defined(__arch64__)
	u64 BarAddr, ReqAddr;
	u32 Location_1;
	u32 *PPtr;
	u64 ReqBar, ReqBar_1;
	u64 MaxBarSize = 0;
#else
	u32 MaxBarSize = 0;
	u32 ReqAddr = 0;

#endif

	/*Static array declarations for BAR Alignment */
	u64 Value[REQ_SIZE];
	u32 Index[REQ_SIZE];
	u64 Size[REQ_SIZE];

	/* Initialize memory with 0*/
	(void)memset(Value, 0, sizeof(Value));
	(void)memset(Index, 0, sizeof(Index));
	(void)memset(Size, 0, sizeof(Size));

	if (Headertype == XDMAPCIE_CFG_HEADER_O_TYPE) {
		/* For endpoints */
		MaxBars = 6;
	} else {
		/* For Bridge*/
		MaxBars = 2;
	}

	/* get original memory allocation based on location and Size */
	XDmaPcie_BarMemoryAlloc(InstancePtr, Bus, Device, Function, Value,
			MaxBars, Index, Size, BarAllocControl);

	/* align values and index */
	XDmaPcie_AlignBarResources(Value, MaxBars , Index);

	/* differentiate sorted sizes and unsorted sizes */
	BarAllocControl = 1;


	/* allocates bar address after sorted, based on Index and Size */
	XDmaPcie_BarMemoryAlloc(InstancePtr, Bus, Device, Function, Value,
			MaxBars, Index, Size, BarAllocControl);

	/* prints all BARs that have been allocated memory */
	for (u8 Bar=0 ; Bar < MaxBars; Bar++ ) {

		if (XDMAPCIE_IS_BAR_UNIMPLEMENTED(Size[Bar])) {
		   /* return saying that BAR is not implemented */
			XDmaPcie_Dbg(
				"bus: %02X, device: %02X, function: %02X: BAR %d is "
				"not implemented\r\n",
				Bus, Device, Function, Bar);
			continue;
		}

		/* check for IO space or memory space */
		if (Size[Bar] & XDMAPCIE_CFG_BAR_MEM_TYPE_MASK) {
			/* Device required IO address space */
			XDmaPcie_Dbg(
				"bus: %02X, device: %02X, function: %02X: BAR %d"
				"required IO space; it is unassigned\r\n",
				Bus, Device, Function, Bar);
			continue;
		}

		/* check for 32 bit AS or 64 bit AS */
		if ((Size[Bar] & XDMAPCIE_CFG_BAR_MEM_AS_MASK) != 0U) {
#if defined(__aarch64__) || defined(__arch64__)

			MaxBarSize = InstancePtr->Config.PMemMaxAddr - InstancePtr->Config.PMemBaseAddr;
			Location = XDmaPcie_ComposeExternalConfigAddress(Bus, Device, Function,
					XDMAPCIE_CFG_BAR_BASE_OFFSET + Bar);
			ReqBar = XDmaPcie_ReadReg((InstancePtr->Config.Ecam), Location);
			Location_1 = XDmaPcie_ComposeExternalConfigAddress(Bus, Device, Function,
					XDMAPCIE_CFG_BAR_BASE_OFFSET + (Bar + 1U));
			ReqBar_1 = XDmaPcie_ReadReg((InstancePtr->Config.Ecam), Location_1);

			/* Merge two bars for size */
			PPtr = (u32 *)&BarAddr;
			*PPtr = ReqBar;
			*(PPtr + 1) = ReqBar_1;
			ReqAddr = BarAddr;
			ReqAddr = ReqAddr & (~0xF);

			Position = XDmaPcie_PositionRightmostSetbit(Size[Bar]);
			ReqSize = (2 << (Position -1));

			/* Compare Bar request size with max Bar size */
			if(ReqSize > MaxBarSize) {
				return XST_SUCCESS;
			}
#endif
		} else {

			MaxBarSize = InstancePtr->Config.NpMemMaxAddr - InstancePtr->Config.NpMemBaseAddr;
			Position = XDmaPcie_PositionRightmostSetbit(Size[Bar]);
			ReqSize = (2 << (Position -1));
			Location = XDmaPcie_ComposeExternalConfigAddress(Bus, Device, Function,
					XDMAPCIE_CFG_BAR_BASE_OFFSET + Bar );
			ReqAddr = XDmaPcie_ReadReg((InstancePtr->Config.Ecam), Location);
			ReqAddr = ReqAddr & (~0xF);

			/* Compare Bar request size with max Bar size */
			if(ReqSize > MaxBarSize) {
				return XST_SUCCESS;
			}
		}

		XDmaPcie_Dbg(
			"bus: %02X, device: %02X, function: %02X: BAR %d, "
			"ADDR: 0x%p size : %dK\r\n",
			Bus, Device, Function, Bar, ReqAddr, (ReqSize / 1024UL));

		if ((Size[Bar] & XDMAPCIE_CFG_BAR_MEM_AS_MASK) != 0U) {
			Bar = Bar + 1U;
		}
	}

	return XST_SUCCESS;
}

#if defined(__aarch64__) || defined(__arch64__)
/******************************************************************************/
/**
* This function increments to next 1Mb block starting position of
* prefetchable memory
*
* @param  	InstancePtr pointer to XDmaPcie Instance
*
*******************************************************************************/
static void XDmaPcie_IncrementPMem(XDmaPcie *InstancePtr)
{
	InstancePtr->Config.PMemBaseAddr >>= MB_SHIFT;
	InstancePtr->Config.PMemBaseAddr++;
	InstancePtr->Config.PMemBaseAddr <<= MB_SHIFT;
}
#endif
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
#if defined(__aarch64__) || defined(__arch64__)
	u32 Adr09;
	u32 Adr0A;
	u32 Adr0B;
#endif

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
#if defined(__aarch64__) || defined(__arch64__)
					XDmaPcie_IncrementPMem(InstancePtr);
#endif

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
#if defined(__aarch64__) || defined(__arch64__)
					Adr09 = 0x0;
					Adr0A = 0x0;
					Adr0B = 0x0;
#endif

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

#if defined(__aarch64__) || defined(__arch64__)
					if (XdmaPcie_IsValidAddr(InstancePtr->Config.PMemBaseAddr) == TRUE) {
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
					}
#endif

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
					 * Align memory to 1 Mb boundary.
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

#if defined(__aarch64__) || defined(__arch64__)
					if (XdmaPcie_IsValidAddr(InstancePtr->Config.PMemBaseAddr) == TRUE) {
						XDmaPcie_IncrementPMem(InstancePtr);
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
					}
#endif

					/* Increment P & NP mem to next aligned starting address.
					 *
					 * Eg: As the range is 0xE000 0000 to 0xE01F FFFF.
					 * the next starting address should be 0xE020 0000.
					 */
					XDmaPcie_IncreamentNpMem(InstancePtr);
#if defined(__aarch64__) || defined(__arch64__)
					if (XdmaPcie_IsValidAddr(InstancePtr->Config.PMemBaseAddr) == TRUE) {
						XDmaPcie_IncrementPMem(InstancePtr);
					}
#endif

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
*		MSI address for which interrupt message received.
* @param	MsiInt is a variable where the driver will pass back the
*		type of interrupt message received (MSI/INTx).
* @param	IntValid is a variable where the driver will pass back the
*		status of read operation of interrupt message.
* @param	MsiMsgData is a variable where the driver will pass back the
*		MSI data received.
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
#if !defined(versal) || defined(QDMA_PCIE_BRIDGE)
	Xil_AssertVoid(InstancePtr->Config.IncludeBarOffsetReg != FALSE);
#endif

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
#if !defined(versal) || defined(QDMA_PCIE_BRIDGE)
	Xil_AssertVoid(InstancePtr->Config.IncludeBarOffsetReg != FALSE);
#endif

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
* Location is identified by its offset from the beginning of the
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
/*
* Enable IATU Programming for MMI PCI Node
*
* @param 	InstancePtr is the PCIe component to operate on.
* @param        Bus is the external PCIe function's Bus number.
* @param        ATUConfigDataType is the TYPE to Modify Outgoing TLP's
* @param        ATURegionCntrl is the Region EN for Address Transalation
* @param        ATUAddress is the IATU Base Address
* @param        ATULimitAddress is the IATU Limit Address
* @param        PCIeAddress is the Target address to be mapped
* @param        RegionSize is the IATU Address Transalation Size

* @return       None
*
* @note         This function is valid only when IP is configured as a
*               root complex.
*
*****************************************************************************/
void XDmaPcie_ConfigureIATURegion(XDmaPcie *InstancePtr, u8 Bus, u8 ATUConfigDataType,
		u32 ATURegionCntrl, u64 ATUAddress, u32 ATULimitAddress,
		u64 PCIeAddress, u32 RegionSize)
{
	 if(Bus>=1) {
                Xil_Out32(InstancePtr->Config.IATUAddress , ATUConfigDataType);

                Xil_Out32((InstancePtr->Config.IATUAddress +
			XDMAPCIE_IATU_REGION_CNTRL_OFFSET) , ATURegionCntrl);

                Xil_Out32((InstancePtr->Config.IATUAddress +
                        XDMAPCIE_IATU_LWR_BASE_ADDR_OFFSET) , LOWER_32_BITS(ATUAddress));

                Xil_Out32((InstancePtr->Config.IATUAddress +
                        XDMAPCIE_IATU_UPPER_BASE_ADDR_OFFSET) , UPPER_32_BITS(ATUAddress));

                Xil_Out32((InstancePtr->Config.IATUAddress +
                        XDMAPCIE_IATU_LIMIT_ADDR_OFFSET) , ATULimitAddress);

                Xil_Out32((InstancePtr->Config.IATUAddress +
                        XDMAPCIE_IATU_PCIE_LWR_ADDR_OFFSET) ,LOWER_32_BITS(PCIeAddress));

                Xil_Out32((InstancePtr->Config.IATUAddress +
                        XDMAPCIE_IATU_PCIE_UPPER_ADDR_OFFSET) ,UPPER_32_BITS(PCIeAddress));

                Xil_Out32((InstancePtr->Config.IATUAddress +
                        XDMAPCIE_IATU_MAX_ATU_SIZE_OFFSET) ,RegionSize);

	 }
}

/****************************************************************************/
/**
* Read 32-bit value from external PCIe Function's configuration space.
* External PCIe function is identified by its Requester ID (Bus#, Device#,
* Function#). Location is identified by its offset from the beginning of the
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
	u32 Data = 0;
#if defined(versal_2ve_2vm)
	u64 PCIeAddr = 0;
	u64 IATUAddr = 0;
#endif
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
#if defined(versal_2ve_2vm)
	PCIeAddr = InstancePtr->Config.Ecam + Location;

	if ((Bus == 1) && (Device == 0))
		IATUAddr = InstancePtr->Config.Ecam + Location;

	if((Bus == 1) && (Device == 0)) {
		XDmaPcie_ConfigureIATURegion(InstancePtr, Bus, XDMAPCIE_CFG_TLP_TYPE0,
			XDMAPCIE_REGION_EN, IATUAddr, XDMAPCIE_ATU_LIMIT_ADDR, PCIeAddr,
			XDMAPCIE_ATU_REGION_SIZE);

	} else {
		XDmaPcie_ConfigureIATURegion(InstancePtr, Bus, XDMAPCIE_CFG_TLP_TYPE1,
			XDMAPCIE_REGION_EN, IATUAddr, XDMAPCIE_ATU_LIMIT_ADDR, PCIeAddr,
			XDMAPCIE_ATU_REGION_SIZE);
	}
#endif
	while(XDmaPcie_IsEcamBusy(InstancePtr));

	/* Read data from that location */
	Data = XDmaPcie_ReadReg((InstancePtr->Config.Ecam),
								Location);
	*DataPtr = Data;

}

/****************************************************************************/
/**
* Write 32-bit value to external PCIe function's configuration space.
* External PCIe function is identified by its Requester ID (Bus#, Device#,
* Function#). Location is identified by its offset from the beginning of the
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

	if (((Bus == 0) && !((Device == 0) && (Function == 0))) ||
		(Bus > InstancePtr->MaxNumOfBuses)) {
		return;
	}

	/* Compose function configuration space location */
	Location = XDmaPcie_ComposeExternalConfigAddress (Bus, Device,
							Function, Offset);
	while(XDmaPcie_IsEcamBusy(InstancePtr));


	/* Write data to that location */
	XDmaPcie_WriteReg((InstancePtr->Config.Ecam),
				Location , Data);


	/* Read data from that location to verify write */
	while (Count) {

		TestWrite =
			XDmaPcie_ReadReg((InstancePtr->Config.Ecam),
								Location);

		if (TestWrite == Data) {
			break;
		}

		Count--;
	}
}
