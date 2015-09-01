/******************************************************************************
*
* Copyright (C) 2014 - 2015 Xilinx, Inc. All rights reserved.
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
* @file mpu.c
*
* This file contains initial configuration of the MPU.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 5.00 	pkp  02/20/14 First release
* </pre>
*
* @note
*
* None.
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xreg_cortexr5.h"
#include "xil_mpu.h"
#include "xpseudo_asm.h"

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
void Init_MPU(void);
static void Xil_SetAttribute(u32 addr, u32 reg_size,s32 reg_num, u32 attrib);
static void Xil_DisableMPURegions(void);

/*****************************************************************************
*
* Initialize MPU for a given address map and Enabled the background Region in
* MPU with default memory attributes for rest of address range for Cortex R5
* processor.
*
* @param	None.
*
* @return	None.
*
*
******************************************************************************/

void Init_MPU(void)
{
	u32 Addr;
	u32 RegSize;
	u32 Attrib;
	u32 RegNum = 0;

	Xil_DisableMPURegions();

	Addr = 0x00000000U;
	RegSize = REGION_2G;
	Attrib = NORM_NSHARED_WB_WA | PRIV_RW_USER_RW;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	Addr = 0xC0000000U;
	RegSize = REGION_512M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	Addr = 0xF0000000U;
	RegSize = REGION_128M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	Addr = 0xF8000000U;
	RegSize = REGION_64M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	Addr = 0xFC000000U;
	RegSize = REGION_32M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	Addr = 0xFE000000U;
	RegSize = REGION_16M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	Addr = 0xFF000000U;
	RegSize = REGION_16M;
	Attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW   ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);
	RegNum++;

	Addr = 0xFFFC0000U;
	RegSize = REGION_256K;
	Attrib = NORM_NSHARED_WB_WA| PRIV_RW_USER_RW  ;
	Xil_SetAttribute(Addr,RegSize,RegNum, Attrib);

}

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
static void Xil_SetAttribute(u32 addr, u32 reg_size,s32 reg_num, u32 attrib)
{
	u32 Local_reg_size = reg_size;

	Local_reg_size = Local_reg_size<<1U;
	Local_reg_size |= REGION_EN;
	dsb();
	mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER,reg_num);
	isb();
	mtcp(XREG_CP15_MPU_REG_BASEADDR,addr); 		/* Set base address of a region */
	mtcp(XREG_CP15_MPU_REG_ACCESS_CTRL,attrib); 	/* Set the control attribute */
	mtcp(XREG_CP15_MPU_REG_SIZE_EN,Local_reg_size);	/* set the region size and enable it*/
	dsb();
	isb();						/* synchronize context on this processor */
}


/*****************************************************************************
*
* Disable all the MPU regions if any of them is enabled
*
* @param	None.
*
* @return	None.
*
*
******************************************************************************/
static void Xil_DisableMPURegions(void)
{
	u32 Temp;
	u32 Index;
	for (Index = 0; Index <= 15; Index++) {
		mtcp(XREG_CP15_MPU_MEMORY_REG_NUMBER,Index);
		Temp = mfcp(XREG_CP15_MPU_REG_SIZE_EN);
		Temp &= (~REGION_EN);
		dsb();
		mtcp(XREG_CP15_MPU_REG_SIZE_EN,Temp);
		dsb();
		isb();
	}

}