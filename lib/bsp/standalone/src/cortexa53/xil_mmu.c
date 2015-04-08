/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information  of Xilinx, Inc.
* and is protected under U.S. and  international copyright and other
* intellectual property  laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any  rights to the
* materials distributed herewith. Except as  otherwise provided in a valid
* license issued to you by  Xilinx, and to the maximum extent permitted by
* applicable law:
* (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND  WITH ALL FAULTS, AND
* XILINX HEREBY DISCLAIMS ALL WARRANTIES  AND CONDITIONS, EXPRESS, IMPLIED,
* OR STATUTORY, INCLUDING  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and
* (2) Xilinx shall not be liable (whether in contract or tort,  including
* negligence, or under any other theory of liability) for any loss or damage of
* any kind or nature  related to, arising under or in connection with these
* materials, including for any direct, or any indirect,  special, incidental,
* or consequential loss or damage  (including loss of data, profits, goodwill,
* or any type of  loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was  reasonably foreseeable
* or Xilinx had been advised of the  possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe  performance, such as life-support or
* safety devices or  systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any  other applications
* that could lead to death, personal  injury, or severe property or environmental
* damage  (individually and collectively, "Critical  Applications").
* Customer assumes the sole risk and liability of any use of Xilinx products in
* Critical  Applications, subject only to applicable laws and  regulations
* governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xil_mmu.c
*
* This file provides APIs for enabling/disabling MMU and setting the memory
* attributes for sections, in the MMU translation table.
* MMU APIs are yet to be implemented. They are left blank to avoid any
* compilation error
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 5.00 	pkp  05/29/14 First release
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
#include "xil_mmu.h"

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

extern INTPTR MMUTableL1;
extern INTPTR MMUTableL2;
/************************** Function Prototypes ******************************/
/*****************************************************************************
*
* Set the memory attributes for a section, in the translation table.
*
* @param	addr is the address for which attributes are to be set.
* @param	attrib specifies the attributes for that memory region.
*
* @return	None.
*
* @note		The MMU and D-cache need not be disabled before changing an
*			translation table attribute.
*
******************************************************************************/

void Xil_SetTlbAttributes(INTPTR Addr, u64 attrib)
{
	INTPTR *ptr;
	INTPTR section;
	/* if region is less than 4GB MMUTable level 2 need to be modified */
	if(Addr<0x100000000){
	section = Addr / 0x00200000U;
	ptr = &MMUTableL2 + section;
	*ptr = (Addr & (~0x001FFFFFU)) | attrib;
	}
	/* if region is greater than 4GB MMUTable level 1 need to be modified */
	else{
	section = Addr / 0x40000000U;
	ptr = &MMUTableL1 + section;
	*ptr = (Addr & (~0x3FFFFFFFU)) | attrib;
	}

	Xil_DCacheFlush();
	mtcptlbi(ALLE3);

	dsb(); /* ensure completion of the BP and TLB invalidation */
    isb(); /* synchronize context on this processor */

}

/*****************************************************************************
*
* Invalidate the caches and then enable MMU for Cortex A53 processor.
*
* @param	None.
* @return	None.
*
******************************************************************************/
void Xil_EnableMMU(void)
{
	u32 Reg;
	Xil_DCacheInvalidate();
	Xil_ICacheInvalidate();

	Reg = mfcp(SCTLR_EL1);

	Reg |= 0x00000001U;
	/* Enable MMU */
	mtcp(SCTLR_EL1, Reg);

	dsb(); /* ensure completion of the BP and TLB invalidation */
    isb(); /* synchronize context on this processor */
}

/*****************************************************************************
*
* Disable MMU for Cortex A53 processors. This function invalidates the TLBs
* and flushed the D Caches before disabling the MMU and D cache.
*
* @param	None.
*
* @return	None.
*
******************************************************************************/
void Xil_DisableMMU(void)
{

	u32 Reg;

	mtcptlbi(ALLE3);
	Xil_DCacheFlush();

	Reg = mfcp(SCTLR_EL1);
	Reg &= ~(0x00000001U);
	/*Disable mmu*/
	mtcp(SCTLR_EL1, Reg);
	dsb(); /* ensure completion of the BP and TLB invalidation */
    isb(); /* synchronize context on this processor */
}
