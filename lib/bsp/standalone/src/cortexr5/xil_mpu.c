/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc. All rights reserved.
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
/*****************************************************************************/
/**
* @file xil_mmu.c
*
* This file provides APIs for enabling/disabling MMU and setting the memory
* attributes for sections, in the MMU translation table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 5.00  pkp  02/10/14 Initial version
* </pre>
*
* @note
*
* None.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_cache.h"
#include "xpseudo_asm.h"
#include "xil_types.h"
#include "xil_mpu.h"
#include "xil_printf.h"

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/*****************************************************************************
*
* Set the memory attributes for a section of memory with starting address addr
* of the region size defined by reg_size having attributes attrib of region number
* reg_num
*
* @param	addr is the address for which attributes are to be set.
* @param	attrib specifies the attributes for that memory region.
* @param	reg_size specifies the size for that memory region.
* @param	reg_num specifies the number for that memory region.
* @return	None.
*
*
******************************************************************************/
void Xil_SetAttribute(u32 addr, u32 reg_size,s32 reg_num, u32 attrib)
{
	u32 CtrlReg, Alignment_Check=0x1U;
	u32 Index;
	u32 Local_reg_size = reg_size;
	s32 DCacheStatus=0, ICacheStatus=0;
	/* disable caches only if they are enabled */
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
	for (Index=0U; Index<=Local_reg_size;Index++) {
		Alignment_Check*=2U;
	}

	/*If address is aligned with region size then it is configured*/

	if((!(addr%Alignment_Check)) != 0x00000000U){
		Local_reg_size = Local_reg_size<<1U;
		Local_reg_size |= REGION_EN;
		mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER,reg_num);
		mtcp(XREG_CP15_MPU_REG_BASEADDR,addr); 			/* Set base address of a region */
		mtcp(XREG_CP15_MPU_REG_ACCESS_CTRL,attrib); 	/* Set the control attribute */
		mtcp(XREG_CP15_MPU_REG_SIZE_EN,Local_reg_size); 		/* set the region size and enable it*/
		dsb(); 											/* ensure completion of the BP invalidation */
		isb();											/* synchronize context on this processor */
	}
	else {
		xil_printf("Address %x is not aligned with region size \n",addr);
	}

	/* enable caches only if they are disabled in routine*/
	if(DCacheStatus != 0) {
		Xil_DCacheEnable();
	}
	if(ICacheStatus != 0) {
		Xil_ICacheEnable();
	}
}

/*****************************************************************************
*
* Enable MPU for Cortex R5 processor. This function invalidates I cache and
* flush the D Caches before enabling the MPU.
*
*
* @param	None.
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

/*****************************************************************************
*
* Disable MPU for Cortex R5 processors. This function invalidates I cache and
* flush the D Caches before disabling the MPU.
*
* @param	None.
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

/*****************************************************************************
*
* Disable a Region in MPU for Cortex R5 processors. This function invalidates
* I cache and flush the D Caches before disabling the MPU region.
*
* @param	reg_num defines region number which is to be disabled.
*
* @return	None.
*
******************************************************************************/

void Xil_DisableRegion(s32 reg_num)
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
	mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER,reg_num);

	Reg = mfcp(XREG_CP15_MPU_REG_SIZE_EN);

	Reg &= ~((u32)REGION_EN);
	dsb();
	mtcp(XREG_CP15_MPU_REG_SIZE_EN, Reg);
	isb();
	/* enable caches only if they are disabled in routine*/
	if(DCacheStatus != 0) {
		Xil_DCacheEnable();
	}
	if(ICacheStatus != 0) {
		Xil_ICacheEnable();
	}
}
/*****************************************************************************
*
* Disable a SubRegion in MPU for Cortex R5 processors. This function invalidates
* I cache and flush the D Caches before disabling
* the MPU subregion.
*
* @param	reg_num defines region number in which particular subregion
*			is to be disabled.
* @param	subreg_num defines the subregion number which is to be disabled.
* @return	None.
*
******************************************************************************/

void Xil_DisableSubRegion(s32 reg_num, u32 subreg_num)
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
	mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER,reg_num);

	Reg = mfcp(XREG_CP15_MPU_REG_SIZE_EN);
	Reg |= ((0x00000001U<<subreg_num)<<8);
	dsb();
	mtcp(XREG_CP15_MPU_REG_SIZE_EN, Reg);
	isb();
	/* enable caches only if they are disabled in routine*/
	if(DCacheStatus != 0) {
		Xil_DCacheEnable();
	}
	if(ICacheStatus != 0) {
		Xil_ICacheEnable();
	}
}

/*****************************************************************************
*
* Enable a SubRegion in MPU for Cortex R5 processors. This function invalidates
* I cache and flush the D Caches before enabling the MPU subregion.
*
* @param	reg_num defines region number in which particular subregion
*			is to be enabled.
* @param	subreg_num defines the subregion number which is to be enabled.
* @return	None.
*
******************************************************************************/

void Xil_EnableSubRegion(s32 reg_num, u32 subreg_num)
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
	mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER,reg_num);
	Reg = mfcp(XREG_CP15_MPU_REG_SIZE_EN);
	Reg &=~((0x00000001U<<subreg_num)<<8U);
	dsb();
	mtcp(XREG_CP15_MPU_REG_SIZE_EN, Reg);
	isb();
	/* enable caches only if they are disabled in routine*/
	if(DCacheStatus != 0) {
		Xil_DCacheEnable();
	}
	if(ICacheStatus != 0) {
		Xil_ICacheEnable();
	}
}
/*****************************************************************************
*
* Enable a background Region in MPU with default memory attributes for Cortex R5
* processor. This function invalidates I cache and flush the D Caches before
* enabling a background Region.
*
* @param	None.
*
* @return	None.
*
*
******************************************************************************/

void Xil_EnableBackgroundRegion(void)
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
	Reg=mfcp(XREG_CP15_SYS_CONTROL);
	Reg |= (0x00000001U<<17U);

	dsb();
	mtcp(XREG_CP15_SYS_CONTROL,Reg);

	isb();
	/* enable caches only if they are disabled in routine*/
	if(DCacheStatus != 0) {
		Xil_DCacheEnable();
	}
	if(ICacheStatus != 0) {
		Xil_ICacheEnable();
	}
}
/*****************************************************************************
*
* Disable a background Region in MPU for Cortex R5 processor. This function
* invalidates I cache and flush the D Caches before
* disabling a background Region.
*
* @param	None.
*
* @return	None.
*
*
******************************************************************************/

void Xil_DisableBackgroundRegion(void)
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
	Xil_DCacheFlush();
	Reg=mfcp(XREG_CP15_SYS_CONTROL);
	Reg &= ~(0x00000001U<<17U);

	dsb();
	mtcp(XREG_CP15_SYS_CONTROL,Reg);

	isb();
	/* enable caches only if they are disabled in routine*/
	if(DCacheStatus != 0) {
		Xil_DCacheEnable();
	}
	if(ICacheStatus != 0) {
		Xil_ICacheEnable();
	}
}
