/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xil_mpu.c
*
* This file provides APIs for enabling/disabling MPU and setting the memory
* attributes for section of memory, in the CortexR52 MPU  table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 9.0   mus  04/19/23 Initial version
* </pre>
*
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_cache.h"
#include "xpseudo_asm.h"
#include "xil_types.h"
#include "xil_mpu.h"
#include "xdebug.h"
#include "xstatus.h"

/**************************** Type Definitions *******************************/
/*
 * This data structure is used internally in this file to
 * fix overlappings in MPU table.
 * It contains parameters required to fix overlapping created
 * by requested MPU region.
 */
struct XRequestMpuConfig{
	UINTPTR BaseAddress;/* MPU region base address */
	u32 Size;/* MPU region size address */
	u32 Attribute;/* MPU region size attribute */
	u32 StartRegNum;/* Existing MPU region number where requested region base address falls */
	u32 EndRegNum;/* Existing MPU region number where requested region end address falls */
	u32 RedundantRegCnt; /* Number of existing MPU regions occupied requested region(excluding start and end region)*/
	u32 Redundant_RegNum[16]; /* Array which contains existing MPU region number's occupied by requested region */
};

/***************** Static functions declarations *********************/
static u32 Xil_SetTlbAttributesR52(UINTPTR addr, u32 Size, u32 Attrib)__attribute__((__section__(".boot")));
static void Xil_AddEntryToMPUConfig(XMpu_Config MpuConfig, struct XMpuConfig NewEntry);
static u32 Xil_RemoveEntryFromMPUConfig(XMpu_Config MpuConfig, UINTPTR RemovedEntry);
static void Xil_SetMPUConfig(XMpu_Config MpuConfigNew);
static u32 Xil_GetNumOfFreeRegionsInReqConfig (XMpu_Config MpuConfig);
static u32 Xil_AdjustOverlappedRegions(struct XRequestMpuConfig ReqMPUConfig );

/************************** Constant Definitions *****************************/
#define MPU_REGION_SIZE_MIN 0x40U
/************************** Variable Definitions *****************************/
XMpu_Config Mpu_Config __attribute__((section(".bootdata")));

/************************** Function Prototypes ******************************/
 void Xil_SetAttributeBasedOnConfig(u32 IsSortingNedded)  __attribute__((__section__(".boot"))); /* To be used by only mpu_r52.c and xil_mpu_r52.c */

/*****************************************************************************/
/**
* @brief    Set the global MPU configuration data structure Mpu_Config with the
*           one passed by caller
*
* @param	MpuConfigNew: New MPU configuration to be copied to Mpu_Config
* @return	None.
*
*
******************************************************************************/
static void Xil_SetMPUConfig(XMpu_Config MpuConfigNew) {
	u32 Cnt;

	for (Cnt=0; Cnt<MAX_POSSIBLE_MPU_REGS; Cnt++) {
		Mpu_Config[Cnt] = MpuConfigNew[Cnt];
	}
}

/*****************************************************************************/
/**
* @brief    Returns the total number of free MPU regions available in new MPU
*           configuration data structure being requested by caller.
* @param	MpuConfig: This is of type XMpu_Config
*           which is an array of 16 entries of type structure representing
*           the MPU config table.
*
* @return	Number of free regions available in requested configuration.
*
*
******************************************************************************/
static u32 Xil_GetNumOfFreeRegionsInReqConfig (XMpu_Config MpuConfig) {
	u32 Index = 0U;
	u32 NumofFreeRegs = 0U;

	while (Index < MAX_POSSIBLE_MPU_REGS) {
		if (MPU_REG_DISABLED == MpuConfig[Index].RegionStatus) {
			NumofFreeRegs++;
		}
		Index++;
	}
	return NumofFreeRegs;
}

static void Xil_AddEntryToMPUConfig(XMpu_Config MpuConfig, struct XMpuConfig NewEntry)
{
	s32 RegNum, NewEntryIndex=0, LastValidIndex=0;
	u32 LastValidAddr = 0;

	/* Find last valid region base address */
	for (RegNum = 0; (RegNum < (s32)MAX_POSSIBLE_MPU_REGS) && (MpuConfig[RegNum].flags & XMPU_VALID_REGION); RegNum++)
	{
		LastValidAddr =  MpuConfig[RegNum].BaseAddress;
		LastValidIndex = RegNum;
	}

	if (NewEntry.BaseAddress > LastValidAddr)
	{
		MpuConfig[RegNum] = NewEntry;
	} else {

		for (RegNum = 0; (RegNum < (s32)MAX_POSSIBLE_MPU_REGS) && (MpuConfig[RegNum].flags & XMPU_VALID_REGION); RegNum++)
		{
			if (NewEntry.BaseAddress < MpuConfig[RegNum].BaseAddress)
			{
			NewEntryIndex = RegNum;
		break;
		}
		}

		for (RegNum = LastValidIndex+1; RegNum >= NewEntryIndex; RegNum--)
        {
			MpuConfig[RegNum] = MpuConfig[RegNum-1];
        }


        MpuConfig[NewEntryIndex] = NewEntry;
	}

}


static u32 Xil_RemoveEntryFromMPUConfig(XMpu_Config MpuConfig, UINTPTR RemovedEntry)
{
	u32 RegNum, RemovedEntryIndex=0xFF;
	u32 Status = XST_FAILURE;


	for (RegNum = 0; (RegNum < MAX_POSSIBLE_MPU_REGS) && (Mpu_Config[RegNum].flags & XMPU_VALID_REGION); RegNum++)
	{
		if (RemovedEntry == MpuConfig[RegNum].BaseAddress)
		{
			RemovedEntryIndex = RegNum;
			break;
		}
	}

	if (0xFF == RemovedEntryIndex)
	{
		return Status;
	}


	if(RemovedEntryIndex != (MAX_POSSIBLE_MPU_REGS - 1))
	{
		for (RegNum = 0; RegNum < (MAX_POSSIBLE_MPU_REGS-1-RemovedEntryIndex); RegNum++)
		{
			MpuConfig[RemovedEntryIndex + RegNum ] = MpuConfig[RemovedEntryIndex + RegNum + 1];
		}
	}

	MpuConfig[MAX_POSSIBLE_MPU_REGS-1].BaseAddress = 0;
	MpuConfig[MAX_POSSIBLE_MPU_REGS-1].RegionStatus = 0;
	MpuConfig[MAX_POSSIBLE_MPU_REGS-1].Size = 0;
	MpuConfig[MAX_POSSIBLE_MPU_REGS-1].Attribute = 0;
	MpuConfig[MAX_POSSIBLE_MPU_REGS-1].flags = 0;

	Status = XST_SUCCESS;

	return Status;

}

/*****************************************************************************/
/**
*
* @brief   Program the MPU regions listed in Mpu_Config data
*          structure.
* @param   IsSortingNedded: Set it to 1 if Mpu_Config needes to be sorted
*          before programming the MPU.
*
* @return	None.
* @note    Only for internal use, supposed to be called only from mpu_r52.c
*          and xil_mpu_r52.c
*
*
******************************************************************************/
 void Xil_SetAttributeBasedOnConfig(u32 IsSortingNedded)
{
	u32 Cnt, Cnt2;
	struct XMpuConfig Temp;
	u32 Local_reg_size;
	u32 LocalAddr;

	if (IsSortingNedded) {
		/* Sort MPU region from low to high base address */
		for (Cnt=0; (Cnt<MAX_POSSIBLE_MPU_REGS) && (Mpu_Config[Cnt].flags & XMPU_VALID_REGION); Cnt++)
	{
			for (Cnt2=Cnt+1; (Cnt2<MAX_POSSIBLE_MPU_REGS) && (Mpu_Config[Cnt2].flags & XMPU_VALID_REGION); Cnt2++)
			{

			if (Mpu_Config[Cnt2].BaseAddress < Mpu_Config[Cnt].BaseAddress)
		{
			Temp = Mpu_Config[Cnt];
					Mpu_Config[Cnt] = Mpu_Config[Cnt2];
					Mpu_Config[Cnt2] = Temp;
		}
		}
		}
	}

	/* Program MPU regions based on regions listed in Mpu_Config */
	for (Cnt=0; (Cnt<MAX_POSSIBLE_MPU_REGS) && (Mpu_Config[Cnt].flags & XMPU_VALID_REGION); Cnt++)
	{

		LocalAddr = (Mpu_Config[Cnt].BaseAddress & XMPU_PBAR_REG_BASEADDR_MASK);

		Local_reg_size = (Mpu_Config[Cnt].Size + LocalAddr);
	Local_reg_size &= XMPU_PRLAR_REG_ENDADDR_MASK;
	Local_reg_size |= ((Mpu_Config[Cnt].Attribute >> XMPU_PRLAR_ATTRIBUTE_SHIFT) & XMPU_PRLAR_REG_ATTRIBUTE_MASK);
		Local_reg_size |= REGION_EN;
		dsb();

		mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER,Cnt);
		isb();

		mtcp(XREG_CP15_MPU_REG_BASEADDR,(LocalAddr | (Mpu_Config[Cnt].Attribute & XMPU_PBAR_REG_ATTRIBUTE_MASK))); /* Set base address of a region */
		mtcp(XREG_CP15_MPU_REG_SIZE_EN,Local_reg_size);	/* set the region size and enable it*/
		dsb();
		isb();	/* synchronize context on this processor */
	}
}

static u32 Xil_SetTlbAttributesR52(UINTPTR addr, u32 Size, u32 Attrib)
{
	u32 Cnt,AttrChange = 0, NumFreeRegion = 0, Status = XST_FAILURE ;
    u32 StartRegNum = 0xFF, EndRegNum = 0xFF;
	u32 Redundant_RegNum[16], RedundantRegCnt=0;
	u32 Endaddr, ReqEndAddr;
	XMpu_Config MpuConfig, MpuConfigNew;
	struct XRequestMpuConfig ReqMPUConfig;
	struct XMpuConfig Temp;
	u32 Localaddr = addr;

	Xil_GetMPUConfig(MpuConfig);
	Xil_GetMPUConfig(MpuConfigNew);

	ReqEndAddr =  (((addr & XMPU_PBAR_REG_BASEADDR_MASK) + Size) | 0x3F);


	for (Cnt=0; (Cnt<MAX_POSSIBLE_MPU_REGS) && (MpuConfig[Cnt].flags & XMPU_VALID_REGION); Cnt++)
    {
		Endaddr =  MpuConfig[Cnt].Size + MpuConfig[Cnt].BaseAddress;
		Endaddr |=  0x3F;

		/*
		 * Possible scenarios are listed below
		 * scenario1: Requested region is not overlapping any existig
		 *            MPU region
		 * scenario2: Requested region is overlappling one of the
		 *            existing MPU region (start address and end address
		 *            of requested region falls in single existing region)
		 * scenario3: Requested region is overlapping 2 existing MPU regions
		 *            (start address of requested region falls in one region
		 *            and end address in other one)
		 * scenario4: Requested region is overlapping multiple existing MPU
		 *            regions (start address of requested region falls in
		 *            one region, and end address in other region, whereas
		 *            multiple existing MPU regions are covered in-between
		 *            start and end address range)
		 * Note: Theoretically there could be scenarios where,
		 *            - start address falls in region which is not covered
		 *              by any existing region and end address covered by
		 *              existing region
		 *            - start address falls in region which is covered
		 *              by existing region and end address not covered by
		 *              existing region
		 *        Existing API does not handle these scenarios.
		 *
		 */

		if ( ((MpuConfig[Cnt].BaseAddress <= addr) && (Endaddr >= addr)))
		{

			StartRegNum = Cnt;
		}


		if ( ((MpuConfig[Cnt].BaseAddress <= ReqEndAddr) && (Endaddr >= ReqEndAddr)))
        {
				EndRegNum = Cnt;
				if (StartRegNum == EndRegNum)
                {
			if ((MpuConfig[Cnt].BaseAddress == (addr & XMPU_PBAR_REG_BASEADDR_MASK)) && (ReqEndAddr == Endaddr) )
                    {
                        AttrChange = 1;
                    }
                }

				break;
		}

		/* Check for scenario4 */
		if (StartRegNum != 0xFF && StartRegNum != Cnt)
		{
			/*
			 * MPU regions other than StartRegNum and EndRegNum in scenario4
			 * will be included in Redundant_RegNum array.
			 */
			Redundant_RegNum[RedundantRegCnt] = Cnt;
			RedundantRegCnt++;
		}


    }

	if (AttrChange) {
		/* Requested region size and base address is same as that of one of the existig MPU region */
		Status = Xil_RemoveEntryFromMPUConfig(MpuConfigNew, MpuConfig[StartRegNum].BaseAddress);
		if (Status != XST_SUCCESS)
		{
			return Status;
		}

		Temp.BaseAddress = (Localaddr & XMPU_PBAR_REG_BASEADDR_MASK);
		Temp.Size = (Size - 1);
		Temp.RegionStatus = MPU_REG_ENABLED;
		Temp.Attribute = Attrib;
		Temp.flags = XMPU_VALID_REGION;
		Xil_AddEntryToMPUConfig(MpuConfigNew, Temp);
		Xil_SetMPUConfig(MpuConfigNew);

	}else if (StartRegNum == 0xFF && EndRegNum == 0xFF) {
		/* No overlapping, new region has been requested*/
		NumFreeRegion = Xil_GetNumOfFreeRegionsInReqConfig (MpuConfigNew);
		if (NumFreeRegion == 0)
			return XST_FAILURE;

		Temp.BaseAddress = (Localaddr & XMPU_PBAR_REG_BASEADDR_MASK);
		Temp.Size = (Size - 1);
		Temp.RegionStatus = MPU_REG_ENABLED;
		Temp.Attribute = Attrib;
		Temp.flags = XMPU_VALID_REGION;

		Xil_AddEntryToMPUConfig(MpuConfigNew, Temp);
		Xil_SetMPUConfig(MpuConfigNew);
	} else {
		ReqMPUConfig.BaseAddress = (Localaddr & XMPU_PBAR_REG_BASEADDR_MASK);
		ReqMPUConfig.Size = (Size - 1);
		ReqMPUConfig.Attribute = Attrib;
		ReqMPUConfig.StartRegNum = StartRegNum;
		ReqMPUConfig.EndRegNum = EndRegNum;
		ReqMPUConfig.RedundantRegCnt = RedundantRegCnt;
		for (Cnt =0 ; Cnt < RedundantRegCnt; Cnt++)
		{
			ReqMPUConfig.Redundant_RegNum[Cnt] = Redundant_RegNum[Cnt];
		}

		Status = Xil_AdjustOverlappedRegions(ReqMPUConfig);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

	}
	dsb();
	isb();
	Xil_DisableMPU();
	Xil_DisableMPURegions();
	Xil_SetAttributeBasedOnConfig(0x0);
	Xil_EnableMPU();
	return XST_SUCCESS;
}

u32 Xil_AdjustOverlappedRegions(struct XRequestMpuConfig ReqMPUConfig )
{
	u32 RegNum, Cnt, Status, NumFreeRegion;
	XMpu_Config MpuConfig, MpuConfigNew;
	struct XMpuConfig Temp;
	u32 StartRegNum = ReqMPUConfig.StartRegNum, EndRegNum = ReqMPUConfig.EndRegNum;
	u32 Attrib =  ReqMPUConfig.Attribute, Size = ReqMPUConfig.Size;
	u32 ReqEndAddr, Endaddr;
	u32 Localaddr = ReqMPUConfig.BaseAddress;

	Xil_GetMPUConfig(MpuConfig);
	Xil_GetMPUConfig(MpuConfigNew);

	Status = Xil_RemoveEntryFromMPUConfig(MpuConfigNew, MpuConfig[StartRegNum].BaseAddress);
	if (Status != XST_SUCCESS)
	{
		return Status;
	}

	if (StartRegNum != EndRegNum) {
		/* More than one region impacted */

		for (Cnt = 0; Cnt < ReqMPUConfig.RedundantRegCnt; Cnt++)
		{
			Status = Xil_RemoveEntryFromMPUConfig(MpuConfigNew, MpuConfig[ReqMPUConfig.Redundant_RegNum[Cnt]].BaseAddress);
			if (Status != XST_SUCCESS)
			{
				return Status;
			}
		}

		Status = Xil_RemoveEntryFromMPUConfig(MpuConfigNew, MpuConfig[EndRegNum].BaseAddress);
		if (Status != XST_SUCCESS)
		{
			return Status;
		}
	}

	ReqEndAddr =  (((ReqMPUConfig.BaseAddress & XMPU_PBAR_REG_BASEADDR_MASK) + Size) | 0x3F);
	Endaddr =  MpuConfig[EndRegNum].Size + MpuConfig[EndRegNum].BaseAddress;
	Endaddr |=  0x3F;
	if ( (StartRegNum != EndRegNum) && \
		 (MpuConfig[StartRegNum].BaseAddress == (ReqMPUConfig.BaseAddress & XMPU_PBAR_REG_BASEADDR_MASK))  && \
		   ((MpuConfig[EndRegNum].BaseAddress + MpuConfig[EndRegNum].Size) | 0x3F) == ReqEndAddr)
	{
		/* 1 free region needed */
		Temp.BaseAddress = MpuConfig[StartRegNum].BaseAddress;
		Temp.Size = (Size - 1);
		Temp.RegionStatus = MPU_REG_ENABLED;
		Temp.Attribute = Attrib;
		Temp.flags = XMPU_VALID_REGION;

		Xil_AddEntryToMPUConfig(MpuConfigNew, Temp);

	} else if ( MpuConfig[StartRegNum].BaseAddress == (Localaddr & XMPU_PBAR_REG_BASEADDR_MASK)) {
		/* 2 free regions needed */
		NumFreeRegion = Xil_GetNumOfFreeRegionsInReqConfig (MpuConfigNew);
		if (NumFreeRegion < 2)
		{
			return XST_FAILURE;
		}

		Temp.BaseAddress = MpuConfig[StartRegNum].BaseAddress;
		Temp.Size = (Size - 1);
		Temp.RegionStatus = MPU_REG_ENABLED;
		Temp.Attribute = Attrib;
		Temp.flags = XMPU_VALID_REGION;

		Xil_AddEntryToMPUConfig(MpuConfigNew, Temp);

		if (StartRegNum != EndRegNum) {
			RegNum = EndRegNum;
		} else {
			RegNum = StartRegNum;
		}
		Temp.BaseAddress = ((MpuConfig[RegNum].BaseAddress + (Size - 1) ) | 0x3F) + 1;
		Temp.Size =  ((MpuConfig[RegNum].BaseAddress + MpuConfig[RegNum].Size) | 0x3F) - Temp.BaseAddress - 1 ;
		Temp.RegionStatus = MPU_REG_ENABLED;
		Temp.Attribute =  MpuConfig[RegNum].Attribute;
		Temp.flags = XMPU_VALID_REGION;

		Xil_AddEntryToMPUConfig(MpuConfigNew, Temp);
	} else if  (Endaddr == ReqEndAddr) {
			/* 2 free regions needed */
			NumFreeRegion = Xil_GetNumOfFreeRegionsInReqConfig (MpuConfigNew);
			if (NumFreeRegion < 2)
			{
				return XST_FAILURE;
			}

			Temp.BaseAddress = MpuConfig[StartRegNum].BaseAddress;
			Temp.Size = ((MpuConfig[StartRegNum].BaseAddress + MpuConfig[StartRegNum].Size) | 0x3F) - (Localaddr & XMPU_PBAR_REG_BASEADDR_MASK) - 1;
			Temp.RegionStatus = MPU_REG_ENABLED;
			Temp.Attribute = MpuConfig[StartRegNum].Attribute;
			Temp.flags = XMPU_VALID_REGION;

			Xil_AddEntryToMPUConfig(MpuConfigNew, Temp);

			Temp.BaseAddress = (ReqMPUConfig.BaseAddress & XMPU_PBAR_REG_BASEADDR_MASK);
			Temp.Size = (Size - 1);
			Temp.RegionStatus = MPU_REG_ENABLED;
			Temp.Attribute = Attrib;
			Temp.flags = XMPU_VALID_REGION;

			Xil_AddEntryToMPUConfig(MpuConfigNew, Temp);
	} else {
			/* 3 free regions needed */
			NumFreeRegion = Xil_GetNumOfFreeRegionsInReqConfig (MpuConfigNew);
			if (NumFreeRegion < 3)
			{
				return XST_FAILURE;
			}

			Temp.BaseAddress = MpuConfig[StartRegNum].BaseAddress;
			Temp.Size = (ReqMPUConfig.BaseAddress & XMPU_PBAR_REG_BASEADDR_MASK) - Temp.BaseAddress - 1;
			Temp.RegionStatus = MPU_REG_ENABLED;
			Temp.Attribute = MpuConfig[StartRegNum].Attribute;
			Temp.flags = XMPU_VALID_REGION;

			Xil_AddEntryToMPUConfig(MpuConfigNew, Temp);

			Temp.BaseAddress = (ReqMPUConfig.BaseAddress & XMPU_PBAR_REG_BASEADDR_MASK);
			Temp.Size = (Size - 1);
			Temp.RegionStatus = MPU_REG_ENABLED;
			Temp.Attribute = Attrib;
			Temp.flags = XMPU_VALID_REGION;

			Xil_AddEntryToMPUConfig(MpuConfigNew, Temp);

			if (StartRegNum != EndRegNum) {
				RegNum = EndRegNum;
			} else {
				RegNum = StartRegNum;
			}

			Temp.BaseAddress = (((Temp.BaseAddress + Temp.Size) | 0x3F) + 1) & XMPU_PBAR_REG_BASEADDR_MASK;
			Temp.Size =  ((MpuConfig[RegNum].BaseAddress + MpuConfig[RegNum].Size) | 0x3F) - Temp.BaseAddress - 1 ;
			Temp.RegionStatus = MPU_REG_ENABLED;
			Temp.Attribute =  MpuConfig[RegNum].Attribute;
			Temp.flags = XMPU_VALID_REGION;

			Xil_AddEntryToMPUConfig(MpuConfigNew, Temp);
		}
		Xil_SetMPUConfig(MpuConfigNew);

		return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief    This function sets the memory attributes for a section covering
*           1MB, of memory in the translation table.
*
* @param	Addr : 32-bit address for which memory attributes need to be set.
* @param	attrib : Attribute for the given memory region.
* @return	None.
* @note     In RTOS/OS environment, this API must be called by disabling
*           scheduler and interrupts.
*
******************************************************************************/
u32 Xil_SetTlbAttributes(UINTPTR addr, u32 attrib)
{
	/*
	 * Sets attribute of section of 1 MB memory, if user wants to set it
	 * for different size of memory section, Xil_SetMPURegion API needs
	 * to be used.
	 */
	return (Xil_SetTlbAttributesR52(addr, 0x100000, attrib));
}

/*****************************************************************************/
/**
* @brief    Set the memory attributes for a section of memory in the
*           translation table.
*
* @param	addr: 32-bit address for which memory attributes need to be set..
* @param	size: size is the size of the region.
* @param	attrib: Attribute for the given memory region.
* @return	None.
* @note     In RTOS/OS environment, this API must be called by disabling
*           scheduler and interrupts.
*
******************************************************************************/
u32 Xil_SetMPURegion(UINTPTR addr, u64 size, u32 attrib)
{
    return (Xil_SetTlbAttributesR52(addr, size, attrib));
}


/*****************************************************************************/
/**
* @brief    Enable MPU for Cortex R52 processor. This function invalidates I
*           cache and flushes the D Caches, and then enables the MPU.
*
* @return	None.
*
******************************************************************************/
void Xil_EnableMPU(void)
{
	u32 CtrlReg, Reg;
	s32 DCacheStatus=0, ICacheStatus=0;
	/* enable caches only if they are disabled */
	CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);

	if ((CtrlReg & XREG_CP15_CONTROL_C_BIT) != 0x00000000U) {
		DCacheStatus=1;
	}
	if ((CtrlReg & XREG_CP15_CONTROL_I_BIT) != 0x00000000U) {
		ICacheStatus=1;
	}

	if(DCacheStatus != 0) {
		Xil_DCacheDisable();
	}
	if(ICacheStatus != 0){
		Xil_ICacheDisable();
	}

	Reg = mfcp(XREG_CP15_SYS_CONTROL);

	Reg |= 0x00000001U;
	dsb();
	mtcp(XREG_CP15_SYS_CONTROL, Reg);
	isb();
	/* enable caches only if they are disabled in routine*/
	if(DCacheStatus != 0) {
		Xil_DCacheEnable();
	}
	if(ICacheStatus != 0) {
		Xil_ICacheEnable();
	}
}

/*****************************************************************************/
/**
* @brief    Disable MPU for Cortex R52 processors. This function invalidates I
*           cache and flushes the D Caches, and then disabes the MPU.
*
* @return	None.
*
******************************************************************************/
void Xil_DisableMPU(void)
{
	u32 CtrlReg, Reg;
	s32 DCacheStatus=0, ICacheStatus=0;
	/* enable caches only if they are disabled */
	CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
	if ((CtrlReg & XREG_CP15_CONTROL_C_BIT) != 0x00000000U) {
		DCacheStatus=1;
	}
	if ((CtrlReg & XREG_CP15_CONTROL_I_BIT) != 0x00000000U) {
		ICacheStatus=1;
	}

	if(DCacheStatus != 0) {
		Xil_DCacheDisable();
	}
	if(ICacheStatus != 0){
		Xil_ICacheDisable();
	}

	mtcp(XREG_CP15_INVAL_BRANCH_ARRAY, 0);
	Reg = mfcp(XREG_CP15_SYS_CONTROL);
	Reg &= ~(0x00000001U);
	dsb();
	mtcp(XREG_CP15_SYS_CONTROL, Reg);
	isb();
	/* enable caches only if they are disabled in routine*/
	if(DCacheStatus != 0) {
		Xil_DCacheEnable();
	}
	if(ICacheStatus != 0) {
		Xil_ICacheEnable();
	}
}


/*****************************************************************************/
/**
* @brief    The MPU configuration table is passed to the caller.
*
* @param	mpuconfig: This is of type XMpu_Config which is an array of
* 			16 entries of type structure representing the MPU config table
* @return	none
*
*
******************************************************************************/
void Xil_GetMPUConfig (XMpu_Config mpuconfig) {
	u32 Index = 0U;

	while (Index < MAX_POSSIBLE_MPU_REGS) {
		mpuconfig[Index] = Mpu_Config[Index];
		Index++;
	}
}

/*****************************************************************************/
/**
* @brief    Returns the total number of free MPU regions available.
*
* @return	Number of free regions available to users
*
*
******************************************************************************/
u32 Xil_GetNumOfFreeRegions (void) {
	u32 Index = 0U;
	u32 NumofFreeRegs = 0U;

	while (Index < MAX_POSSIBLE_MPU_REGS) {
		if (MPU_REG_DISABLED == Mpu_Config[Index].RegionStatus) {
			NumofFreeRegs++;
		}
		Index++;
	}
	return NumofFreeRegs;
}

/*****************************************************************************/
/**
* @brief    Returns the total number of free MPU regions available in the form
*           of a mask. A bit of 1 in the returned 16 bit value represents the
*           corresponding region number to be available.
*           For example, if this function returns 0xC0000, this would mean, the
*           regions 14 and 15 are available to users.
*
* @return	The free region mask as a 16 bit value
*
*
******************************************************************************/
u16 Xil_GetMPUFreeRegMask (void) {
	u32 Index = 0U;
	u16 FreeRegMask = 0U;

	while (Index < MAX_POSSIBLE_MPU_REGS) {
		if (MPU_REG_DISABLED == Mpu_Config[Index].RegionStatus) {
			FreeRegMask |= ((u16)1U << Index);
		}
		Index++;
	}
	return FreeRegMask;
}

/*****************************************************************************/
/**
* @brief    Disables the corresponding region number as passed by the user.
*
* @param	reg_num: The region number to be disabled
* @return	XST_SUCCESS: If the region could be disabled successfully
* 			XST_FAILURE: If the requested region number is 16 or more.
*
*
******************************************************************************/
u32 Xil_DisableMPURegionByRegNum (u32 reg_num) {
	u32 ReturnVal = XST_FAILURE;
	u32 Status;
	XMpu_Config MpuConfig;

	if (reg_num >= 16U) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Invalid region number\r\n");
		goto exit1;
	}

	Xil_GetMPUConfig(MpuConfig);

	Status = Xil_RemoveEntryFromMPUConfig(MpuConfig, MpuConfig[reg_num].BaseAddress);
    ReturnVal = Status;

    if (XST_SUCCESS != ReturnVal)
    {
	return ReturnVal;
	}
	Xil_SetMPUConfig(MpuConfig);
	dsb();
	isb();
	Xil_DisableMPU();
	Xil_DisableMPURegions();
	Xil_SetAttributeBasedOnConfig(0x0);
	Xil_EnableMPU();


exit1:
	return ReturnVal;
}

/*****************************************************************************/
/**
* @brief    Returns the next available free MPU region
*
* @return	The free MPU region available
*
*
******************************************************************************/
u32 Xil_GetNextMPURegion(void)
{
	u32 Index = 0U;
	u32 NextAvailableReg = 0xFF;
	while (Index < MAX_POSSIBLE_MPU_REGS) {
		if (Mpu_Config[Index].RegionStatus != MPU_REG_ENABLED) {
			NextAvailableReg = Index;
			break;
		}
		Index++;
	}
	return NextAvailableReg;
}

#define u32overflow(a, b) ({typeof(a) s; __builtin_uadd_overflow(a, b, &s); })

/*****************************************************************************/
/**
* @brief    Memory mapping for Cortex-R52. If successful, the mapped
*           region will include all of the memory requested, but may
*           truncate the PhysAddr by making it 64 byte aligned, since Cortex R52
*           MPU region needs to be strictly 64 byte aligned.
*
* @param       PhysAddr is base physical address at which to start mapping.
*                   NULL in Physaddr masks possible mapping errors.
* @param       size of region to be mapped.
* @param       flags used to set translation table.
*
* @return      Physaddr on success, NULL on error. Ambiguous if Physaddr==NULL
*
* @cond Xil_MemMap_internal
* @note:    u32overflow() is defined for readability and (for __GNUC__) to
*           - force the type of the check to be the same as the first argument
*           - hide the otherwise unused third argument of the builtin
*           - improve safety by choosing the explicit _uadd_ version.
*           Consider __builtin_add_overflow_p() when available.
*           Use an alternative (less optimal?) for compilers w/o the builtin.
* @endcond
******************************************************************************/
void *Xil_MemMap(UINTPTR PhysAddr, size_t size, u32 flags)
{

	if (flags == 0U) {
		return (void *)PhysAddr;
	}
	if (u32overflow(PhysAddr, size)) {
		return NULL;
	}

	return ((Xil_SetMPURegion(PhysAddr,
                                        size, flags) == XST_SUCCESS) ?
                                        (void *)(PhysAddr & XMPU_PBAR_REG_BASEADDR_MASK) : NULL);
}
